#include <inttypes.h>
#include <signal.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include <carquet/carquet.h>

#include "ams2_telemetry.h"

#define BATCH_SIZE (1024 * 1024)

sig_atomic_t signal_received = 0;

static void handle_signal(int) { signal_received = 1; }

// RecordBatch to be written
//
// Field order mirrors the AMS2 shared memory struct.
typedef struct {
  // Game States
  int32_t game_state[BATCH_SIZE];
  int32_t session_state[BATCH_SIZE];
  int32_t race_state[BATCH_SIZE];

  // Unfiltered Input
  float unfiltered_throttle[BATCH_SIZE];
  float unfiltered_brake[BATCH_SIZE];
  float unfiltered_steering[BATCH_SIZE];
  float unfiltered_clutch[BATCH_SIZE];

  // Timings
  float best_lap_time[BATCH_SIZE];
  float last_lap_time[BATCH_SIZE];
  float current_time[BATCH_SIZE];

  // Car State
  float oil_temp[BATCH_SIZE];
  float oil_pressure[BATCH_SIZE];
  float water_temp[BATCH_SIZE];
  float water_pressure[BATCH_SIZE];
  float fuel_pressure[BATCH_SIZE];
  float fuel_level[BATCH_SIZE];
  float fuel_capacity[BATCH_SIZE];
  float speed[BATCH_SIZE];
  float rpm[BATCH_SIZE];
  float max_rpm[BATCH_SIZE];
  float brake[BATCH_SIZE];
  float throttle[BATCH_SIZE];
  float clutch[BATCH_SIZE];
  float steering[BATCH_SIZE];
  int32_t gear[BATCH_SIZE];

  // Motion
  float orient_x[BATCH_SIZE];
  float orient_y[BATCH_SIZE];
  float orient_z[BATCH_SIZE];
  float local_vel_x[BATCH_SIZE];
  float local_vel_y[BATCH_SIZE];
  float local_vel_z[BATCH_SIZE];
  float world_vel_x[BATCH_SIZE];
  float world_vel_y[BATCH_SIZE];
  float world_vel_z[BATCH_SIZE];
  float local_accel_x[BATCH_SIZE];
  float local_accel_y[BATCH_SIZE];
  float local_accel_z[BATCH_SIZE];
  float world_accel_x[BATCH_SIZE];
  float world_accel_y[BATCH_SIZE];
  float world_accel_z[BATCH_SIZE];

  // Wheels / Tyres
  float tyre_temp_fl[BATCH_SIZE];
  float tyre_temp_fr[BATCH_SIZE];
  float tyre_temp_rl[BATCH_SIZE];
  float tyre_temp_rr[BATCH_SIZE];
  float tyre_wear_fl[BATCH_SIZE];
  float tyre_wear_fr[BATCH_SIZE];
  float tyre_wear_rl[BATCH_SIZE];
  float tyre_wear_rr[BATCH_SIZE];
  float brake_temp_fl[BATCH_SIZE];
  float brake_temp_fr[BATCH_SIZE];
  float brake_temp_rl[BATCH_SIZE];
  float brake_temp_rr[BATCH_SIZE];

  // Car Damage
  float aero_damage[BATCH_SIZE];
  float engine_damage[BATCH_SIZE];

  // Weather
  float ambient_temp[BATCH_SIZE];
  float track_temp[BATCH_SIZE];
  float rain_density[BATCH_SIZE];
  float wind_speed[BATCH_SIZE];
  float wind_dir_x[BATCH_SIZE];
  float wind_dir_y[BATCH_SIZE];

  // PCars2 additions
  float susp_travel_fl[BATCH_SIZE];
  float susp_travel_fr[BATCH_SIZE];
  float susp_travel_rl[BATCH_SIZE];
  float susp_travel_rr[BATCH_SIZE];
  float air_pressure_fl[BATCH_SIZE];
  float air_pressure_fr[BATCH_SIZE];
  float air_pressure_rl[BATCH_SIZE];
  float air_pressure_rr[BATCH_SIZE];
  float engine_speed[BATCH_SIZE];
  float engine_torque[BATCH_SIZE];
  float wing_front[BATCH_SIZE];
  float wing_rear[BATCH_SIZE];
  float handbrake[BATCH_SIZE];

  // More race variables
  float brake_bias[BATCH_SIZE];

  // AMS2 additions
  float ride_height_fl[BATCH_SIZE];
  float ride_height_fr[BATCH_SIZE];
  float ride_height_rl[BATCH_SIZE];
  float ride_height_rr[BATCH_SIZE];

  int32_t count;
} row_buffer_t;

// --- Column metadata table ---

typedef struct {
  const char *name;
  carquet_physical_type_t type;
  size_t offset;
} column_info_t;

#define FLOAT_COL(field, col_name) {col_name, CARQUET_PHYSICAL_FLOAT, offsetof(row_buffer_t, field)}
#define INT32_COL(field, col_name) {col_name, CARQUET_PHYSICAL_INT32, offsetof(row_buffer_t, field)}

// clang-format off
static const column_info_t columns[] = {
    // Game States
    INT32_COL(game_state, "GameState"),
    INT32_COL(session_state, "SessionState"),
    INT32_COL(race_state, "RaceState"),

    // Unfiltered Input
    FLOAT_COL(unfiltered_throttle, "UnfilteredThrottle"),
    FLOAT_COL(unfiltered_brake, "UnfilteredBrake"),
    FLOAT_COL(unfiltered_steering, "UnfilteredSteering"),
    FLOAT_COL(unfiltered_clutch, "UnfilteredClutch"),

    // Timings
    FLOAT_COL(best_lap_time, "BestLapTime"),
    FLOAT_COL(last_lap_time, "LastLapTime"),
    FLOAT_COL(current_time, "CurrentTime"),

    // Car State
    FLOAT_COL(oil_temp, "OilTempCelsius"),
    FLOAT_COL(oil_pressure, "OilPressureKPa"),
    FLOAT_COL(water_temp, "WaterTempCelsius"),
    FLOAT_COL(water_pressure, "WaterPressureKPa"),
    FLOAT_COL(fuel_pressure, "FuelPressureKPa"),
    FLOAT_COL(fuel_level, "FuelLevel"),
    FLOAT_COL(fuel_capacity, "FuelCapacity"),
    FLOAT_COL(speed, "Speed"),
    FLOAT_COL(rpm, "Rpm"),
    FLOAT_COL(max_rpm, "MaxRpm"),
    FLOAT_COL(brake, "Brake"),
    FLOAT_COL(throttle, "Throttle"),
    FLOAT_COL(clutch, "Clutch"),
    FLOAT_COL(steering, "Steering"),
    INT32_COL(gear, "Gear"),

    // Motion
    FLOAT_COL(orient_x, "OrientationX"),
    FLOAT_COL(orient_y, "OrientationY"),
    FLOAT_COL(orient_z, "OrientationZ"),
    FLOAT_COL(local_vel_x, "LocalVelocityX"),
    FLOAT_COL(local_vel_y, "LocalVelocityY"),
    FLOAT_COL(local_vel_z, "LocalVelocityZ"),
    FLOAT_COL(world_vel_x, "WorldVelocityX"),
    FLOAT_COL(world_vel_y, "WorldVelocityY"),
    FLOAT_COL(world_vel_z, "WorldVelocityZ"),
    FLOAT_COL(local_accel_x, "LocalAccelerationX"),
    FLOAT_COL(local_accel_y, "LocalAccelerationY"),
    FLOAT_COL(local_accel_z, "LocalAccelerationZ"),
    FLOAT_COL(world_accel_x, "WorldAccelerationX"),
    FLOAT_COL(world_accel_y, "WorldAccelerationY"),
    FLOAT_COL(world_accel_z, "WorldAccelerationZ"),

    // Wheels / Tyres
    FLOAT_COL(tyre_temp_fl, "TyreTempFl"),
    FLOAT_COL(tyre_temp_fr, "TyreTempFr"),
    FLOAT_COL(tyre_temp_rl, "TyreTempRl"),
    FLOAT_COL(tyre_temp_rr, "TyreTempRr"),
    FLOAT_COL(tyre_wear_fl, "TyreWearFl"),
    FLOAT_COL(tyre_wear_fr, "TyreWearFr"),
    FLOAT_COL(tyre_wear_rl, "TyreWearRl"),
    FLOAT_COL(tyre_wear_rr, "TyreWearRr"),
    FLOAT_COL(brake_temp_fl, "BrakeTempFl"),
    FLOAT_COL(brake_temp_fr, "BrakeTempFr"),
    FLOAT_COL(brake_temp_rl, "BrakeTempRl"),
    FLOAT_COL(brake_temp_rr, "BrakeTempRr"),

    // Car Damage
    FLOAT_COL(aero_damage, "AeroDamage"),
    FLOAT_COL(engine_damage, "EngineDamage"),

    // Weather
    FLOAT_COL(ambient_temp, "AmbientTemperature"),
    FLOAT_COL(track_temp, "TrackTemperature"),
    FLOAT_COL(rain_density, "RainDensity"),
    FLOAT_COL(wind_speed, "WindSpeed"),
    FLOAT_COL(wind_dir_x, "WindDirectionX"),
    FLOAT_COL(wind_dir_y, "WindDirectionY"),

    // PCars2 additions
    FLOAT_COL(susp_travel_fl, "SuspensionTravelFl"),
    FLOAT_COL(susp_travel_fr, "SuspensionTravelFr"),
    FLOAT_COL(susp_travel_rl, "SuspensionTravelRl"),
    FLOAT_COL(susp_travel_rr, "SuspensionTravelRr"),
    FLOAT_COL(air_pressure_fl, "AirPressureFl"),
    FLOAT_COL(air_pressure_fr, "AirPressureFr"),
    FLOAT_COL(air_pressure_rl, "AirPressureRl"),
    FLOAT_COL(air_pressure_rr, "AirPressureRr"),
    FLOAT_COL(engine_speed, "EngineSpeed"),
    FLOAT_COL(engine_torque, "EngineTorque"),
    FLOAT_COL(wing_front, "WingFront"),
    FLOAT_COL(wing_rear, "WingRear"),
    FLOAT_COL(handbrake, "HandBrake"),

    // More race variables
    FLOAT_COL(brake_bias, "BrakeBias"),

    // AMS2 additions
    FLOAT_COL(ride_height_fl, "RideHeightFl"),
    FLOAT_COL(ride_height_fr, "RideHeightFr"),
    FLOAT_COL(ride_height_rl, "RideHeightRl"),
    FLOAT_COL(ride_height_rr, "RideHeightRr"),
};
// clang-format on

#define NUM_COLUMNS (sizeof(columns) / sizeof(columns[0]))

// --- Schema creation ---

static carquet_schema_t *create_schema(carquet_error_t *err) {
  carquet_schema_t *schema = carquet_schema_create(err);
  if (!schema) {
    return NULL;
  }

  for (size_t i = 0; i < NUM_COLUMNS; i++) {
    carquet_status_t st =
        carquet_schema_add_column(schema, columns[i].name, columns[i].type, NULL, CARQUET_REPETITION_REQUIRED, 0, 0);
    if (st != CARQUET_OK) {
      carquet_schema_free(schema);
      carquet_error_set(err, st, __FILE__, __LINE__, __func__, "failed to add column '%s'", columns[i].name);
      return NULL;
    }
  }
  return schema;
}

// --- Buffer operations ---

static void buffer_add_row(row_buffer_t *buf, const ams2_telemetry *tele) {
  int i = buf->count++;

  // Game States
  buf->game_state[i] = (int32_t)tele->mGameState;
  buf->session_state[i] = (int32_t)tele->mSessionState;
  buf->race_state[i] = (int32_t)tele->mRaceState;

  // Unfiltered Input
  buf->unfiltered_throttle[i] = tele->mUnfilteredThrottle;
  buf->unfiltered_brake[i] = tele->mUnfilteredBrake;
  buf->unfiltered_steering[i] = tele->mUnfilteredSteering;
  buf->unfiltered_clutch[i] = tele->mUnfilteredClutch;

  // Timings
  buf->best_lap_time[i] = tele->mBestLapTime;
  buf->last_lap_time[i] = tele->mLastLapTime;
  buf->current_time[i] = tele->mCurrentTime;

  // Car State
  buf->oil_temp[i] = tele->mOilTempCelsius;
  buf->oil_pressure[i] = tele->mOilPressureKPa;
  buf->water_temp[i] = tele->mWaterTempCelsius;
  buf->water_pressure[i] = tele->mWaterPressureKPa;
  buf->fuel_pressure[i] = tele->mFuelPressureKPa;
  buf->fuel_level[i] = tele->mFuelLevel;
  buf->fuel_capacity[i] = tele->mFuelCapacity;
  buf->speed[i] = tele->mSpeed;
  buf->rpm[i] = tele->mRpm;
  buf->max_rpm[i] = tele->mMaxRPM;
  buf->brake[i] = tele->mBrake;
  buf->throttle[i] = tele->mThrottle;
  buf->clutch[i] = tele->mClutch;
  buf->steering[i] = tele->mSteering;
  buf->gear[i] = tele->mGear;

  // Motion
  buf->orient_x[i] = tele->mOrientation[VEC_X];
  buf->orient_y[i] = tele->mOrientation[VEC_Y];
  buf->orient_z[i] = tele->mOrientation[VEC_Z];
  buf->local_vel_x[i] = tele->mLocalVelocity[VEC_X];
  buf->local_vel_y[i] = tele->mLocalVelocity[VEC_Y];
  buf->local_vel_z[i] = tele->mLocalVelocity[VEC_Z];
  buf->world_vel_x[i] = tele->mWorldVelocity[VEC_X];
  buf->world_vel_y[i] = tele->mWorldVelocity[VEC_Y];
  buf->world_vel_z[i] = tele->mWorldVelocity[VEC_Z];
  buf->local_accel_x[i] = tele->mLocalAcceleration[VEC_X];
  buf->local_accel_y[i] = tele->mLocalAcceleration[VEC_Y];
  buf->local_accel_z[i] = tele->mLocalAcceleration[VEC_Z];
  buf->world_accel_x[i] = tele->mWorldAcceleration[VEC_X];
  buf->world_accel_y[i] = tele->mWorldAcceleration[VEC_Y];
  buf->world_accel_z[i] = tele->mWorldAcceleration[VEC_Z];

  // Wheels / Tyres
  buf->tyre_temp_fl[i] = tele->mTyreTemp[TYRE_FRONT_LEFT];
  buf->tyre_temp_fr[i] = tele->mTyreTemp[TYRE_FRONT_RIGHT];
  buf->tyre_temp_rl[i] = tele->mTyreTemp[TYRE_REAR_LEFT];
  buf->tyre_temp_rr[i] = tele->mTyreTemp[TYRE_REAR_RIGHT];
  buf->tyre_wear_fl[i] = tele->mTyreWear[TYRE_FRONT_LEFT];
  buf->tyre_wear_fr[i] = tele->mTyreWear[TYRE_FRONT_RIGHT];
  buf->tyre_wear_rl[i] = tele->mTyreWear[TYRE_REAR_LEFT];
  buf->tyre_wear_rr[i] = tele->mTyreWear[TYRE_REAR_RIGHT];
  buf->brake_temp_fl[i] = tele->mBrakeTempCelsius[TYRE_FRONT_LEFT];
  buf->brake_temp_fr[i] = tele->mBrakeTempCelsius[TYRE_FRONT_RIGHT];
  buf->brake_temp_rl[i] = tele->mBrakeTempCelsius[TYRE_REAR_LEFT];
  buf->brake_temp_rr[i] = tele->mBrakeTempCelsius[TYRE_REAR_RIGHT];

  // Car Damage
  buf->aero_damage[i] = tele->mAeroDamage;
  buf->engine_damage[i] = tele->mEngineDamage;

  // Weather
  buf->ambient_temp[i] = tele->mAmbientTemperature;
  buf->track_temp[i] = tele->mTrackTemperature;
  buf->rain_density[i] = tele->mRainDensity;
  buf->wind_speed[i] = tele->mWindSpeed;
  buf->wind_dir_x[i] = tele->mWindDirectionX;
  buf->wind_dir_y[i] = tele->mWindDirectionY;

  // PCars2 additions
  buf->susp_travel_fl[i] = tele->mSuspensionTravel[TYRE_FRONT_LEFT];
  buf->susp_travel_fr[i] = tele->mSuspensionTravel[TYRE_FRONT_RIGHT];
  buf->susp_travel_rl[i] = tele->mSuspensionTravel[TYRE_REAR_LEFT];
  buf->susp_travel_rr[i] = tele->mSuspensionTravel[TYRE_REAR_RIGHT];
  buf->air_pressure_fl[i] = tele->mAirPressure[TYRE_FRONT_LEFT];
  buf->air_pressure_fr[i] = tele->mAirPressure[TYRE_FRONT_RIGHT];
  buf->air_pressure_rl[i] = tele->mAirPressure[TYRE_REAR_LEFT];
  buf->air_pressure_rr[i] = tele->mAirPressure[TYRE_REAR_RIGHT];
  buf->engine_speed[i] = tele->mEngineSpeed;
  buf->engine_torque[i] = tele->mEngineTorque;
  buf->wing_front[i] = tele->mWings[0];
  buf->wing_rear[i] = tele->mWings[1];
  buf->handbrake[i] = tele->mHandBrake;

  // More race variables
  buf->brake_bias[i] = tele->mBrakeBias;

  // AMS2 additions
  buf->ride_height_fl[i] = tele->mRideHeight[TYRE_FRONT_LEFT];
  buf->ride_height_fr[i] = tele->mRideHeight[TYRE_FRONT_RIGHT];
  buf->ride_height_rl[i] = tele->mRideHeight[TYRE_REAR_LEFT];
  buf->ride_height_rr[i] = tele->mRideHeight[TYRE_REAR_RIGHT];
}

static void buffer_flush(row_buffer_t *buf, carquet_writer_t *writer) {
  if (buf->count == 0) {
    return;
  }

  for (size_t col = 0; col < NUM_COLUMNS; col++) {
    const void *data = (const uint8_t *)buf + columns[col].offset;
    carquet_status_t st = carquet_writer_write_batch(writer, (int32_t)col, data, buf->count, NULL, NULL);
    if (st != CARQUET_OK) {
      fprintf(stderr, "Failed to write column '%s': %s\n", columns[col].name, carquet_status_string(st));
    }
  }
  buf->count = 0;
}

// --- Main ---

int main(int argc, char *argv[]) {
  const char *output_path = "telemetry.parquet";
  if (argc > 1) {
    output_path = argv[1];
  }

  signal(SIGINT, handle_signal);
  signal(SIGTERM, handle_signal);

  fprintf(stderr, "Waiting for AMS2...\n");
  int pid = wait_for_ams2_pid();
  const void *remote_addr = wait_for_ams2_telemetry_address(pid);
  fprintf(stderr, "Connected. Logging telemetry to %s\n", output_path);

  carquet_error_t err = CARQUET_ERROR_INIT;
  carquet_schema_t *schema = create_schema(&err);
  if (!schema) {
    fprintf(stderr, "Failed to create schema: %s\n", carquet_error_message(&err));
    return 1;
  }

  carquet_writer_options_t opts;
  carquet_writer_options_init(&opts);
  opts.compression = CARQUET_COMPRESSION_ZSTD;
  opts.write_statistics = true;

  carquet_writer_t *writer = carquet_writer_create(output_path, schema, &opts, &err);
  if (!writer) {
    fprintf(stderr, "Failed to create writer: %s\n", carquet_error_message(&err));
    carquet_schema_free(schema);
    return 1;
  }

  if (carquet_writer_add_metadata(writer, "app", "ams2_parquet_logger") != CARQUET_OK) {
    fprintf(stderr, "Warning: failed to add file metadata\n");
  }

  row_buffer_t buf = {0};

  struct timespec short_sleep = {.tv_sec = 0, .tv_nsec = 1 * 1000 * 1000};
  struct timespec long_sleep = {.tv_sec = 0, .tv_nsec = 18 * 1000 * 1000};

  unsigned seq_num = 0;
  int64_t total_rows = 0;

  while (!signal_received) {
    ams2_telemetry tele;
    while (!signal_received && !read_ams2_telemetry(pid, &tele, remote_addr)) {
      nanosleep(&short_sleep, NULL);
    }
    if (signal_received) {
      break;
    }

    if (seq_num && tele.mSequenceNumber != seq_num + 2) {
      fprintf(stderr, "Skipped updates: %u -> %u\n", seq_num, tele.mSequenceNumber);
    }
    seq_num = tele.mSequenceNumber;

    buffer_add_row(&buf, &tele);
    total_rows++;

    if (buf.count >= BATCH_SIZE) {
      buffer_flush(&buf, writer);
      fprintf(stderr, "Flushed row group (%" PRId64 " total rows)\n", total_rows);
    }

    nanosleep(&long_sleep, NULL);
  }

  buffer_flush(&buf, writer);

  carquet_status_t st = carquet_writer_close(writer);
  if (st != CARQUET_OK) {
    fprintf(stderr, "Failed to close writer: %s\n", carquet_status_string(st));
  }

  carquet_schema_free(schema);
  fprintf(stderr, "Wrote %" PRId64 " rows to %s\n", total_rows, output_path);
  return 0;
}
