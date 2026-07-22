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
// Field names and order mirror the AMS2 shared memory struct.
// Array fields use [dim][BATCH_SIZE] layout so each component is contiguous.
typedef struct {
  // Game States
  int32_t mGameState[BATCH_SIZE];
  int32_t mSessionState[BATCH_SIZE];
  int32_t mRaceState[BATCH_SIZE];

  // Unfiltered Input
  float mUnfilteredThrottle[BATCH_SIZE];
  float mUnfilteredBrake[BATCH_SIZE];
  float mUnfilteredSteering[BATCH_SIZE];
  float mUnfilteredClutch[BATCH_SIZE];

  // Timings
  float mBestLapTime[BATCH_SIZE];
  float mLastLapTime[BATCH_SIZE];
  float mCurrentTime[BATCH_SIZE];

  // Car State
  float mOilTempCelsius[BATCH_SIZE];
  float mOilPressureKPa[BATCH_SIZE];
  float mWaterTempCelsius[BATCH_SIZE];
  float mWaterPressureKPa[BATCH_SIZE];
  float mFuelPressureKPa[BATCH_SIZE];
  float mFuelLevel[BATCH_SIZE];
  float mFuelCapacity[BATCH_SIZE];
  float mSpeed[BATCH_SIZE];
  float mRpm[BATCH_SIZE];
  float mMaxRPM[BATCH_SIZE];
  float mBrake[BATCH_SIZE];
  float mThrottle[BATCH_SIZE];
  float mClutch[BATCH_SIZE];
  float mSteering[BATCH_SIZE];
  int32_t mGear[BATCH_SIZE];

  // Motion
  float mOrientation[BATCH_SIZE][VEC_MAX];
  float mLocalVelocity[BATCH_SIZE][VEC_MAX];
  float mWorldVelocity[BATCH_SIZE][VEC_MAX];
  float mLocalAcceleration[BATCH_SIZE][VEC_MAX];
  float mWorldAcceleration[BATCH_SIZE][VEC_MAX];

  // Wheels / Tyres
  float mTyreTemp[BATCH_SIZE][TYRE_MAX];
  float mTyreWear[BATCH_SIZE][TYRE_MAX];
  float mBrakeTempCelsius[BATCH_SIZE][TYRE_MAX];

  // Car Damage
  float mAeroDamage[BATCH_SIZE];
  float mEngineDamage[BATCH_SIZE];

  // Weather
  float mAmbientTemperature[BATCH_SIZE];
  float mTrackTemperature[BATCH_SIZE];
  float mRainDensity[BATCH_SIZE];
  float mWindSpeed[BATCH_SIZE];
  float mWindDirectionX[BATCH_SIZE];
  float mWindDirectionY[BATCH_SIZE];

  // PCars2 additions
  float mSuspensionTravel[BATCH_SIZE][TYRE_MAX];
  float mAirPressure[BATCH_SIZE][TYRE_MAX];
  float mEngineSpeed[BATCH_SIZE];
  float mEngineTorque[BATCH_SIZE];
  float mWings[BATCH_SIZE][2];
  float mHandBrake[BATCH_SIZE];

  // More race variables
  float mBrakeBias[BATCH_SIZE];

  // AMS2 additions
  float mRideHeight[BATCH_SIZE][TYRE_MAX];

  int32_t count;
} row_buffer_t;

// --- Column metadata table ---

typedef struct {
  const char *name;
  carquet_physical_type_t type;
  size_t buf_offset;
  size_t tele_offset;
  size_t elem_size;
} column_info_t;

#define FLOAT_COL(field, col_name)                                                                                     \
  {col_name, CARQUET_PHYSICAL_FLOAT, offsetof(row_buffer_t, field), offsetof(ams2_telemetry, field), sizeof(float)}
#define INT32_COL(field, col_name)                                                                                     \
  {col_name, CARQUET_PHYSICAL_INT32, offsetof(row_buffer_t, field), offsetof(ams2_telemetry, field), sizeof(int32_t)}
#define FLOAT_COL_AT(field, idx, col_name)                                                                             \
  {col_name, CARQUET_PHYSICAL_FLOAT, offsetof(row_buffer_t, field) + (idx) * BATCH_SIZE * sizeof(float),               \
   offsetof(ams2_telemetry, field) + (idx) * sizeof(float), sizeof(float)}

// clang-format off
static const column_info_t columns[] = {
    // Game States
    INT32_COL(mGameState, "GameState"),
    INT32_COL(mSessionState, "SessionState"),
    INT32_COL(mRaceState, "RaceState"),

    // Unfiltered Input
    FLOAT_COL(mUnfilteredThrottle, "UnfilteredThrottle"),
    FLOAT_COL(mUnfilteredBrake, "UnfilteredBrake"),
    FLOAT_COL(mUnfilteredSteering, "UnfilteredSteering"),
    FLOAT_COL(mUnfilteredClutch, "UnfilteredClutch"),

    // Timings
    FLOAT_COL(mBestLapTime, "BestLapTime"),
    FLOAT_COL(mLastLapTime, "LastLapTime"),
    FLOAT_COL(mCurrentTime, "CurrentTime"),

    // Car State
    FLOAT_COL(mOilTempCelsius, "OilTempCelsius"),
    FLOAT_COL(mOilPressureKPa, "OilPressureKPa"),
    FLOAT_COL(mWaterTempCelsius, "WaterTempCelsius"),
    FLOAT_COL(mWaterPressureKPa, "WaterPressureKPa"),
    FLOAT_COL(mFuelPressureKPa, "FuelPressureKPa"),
    FLOAT_COL(mFuelLevel, "FuelLevel"),
    FLOAT_COL(mFuelCapacity, "FuelCapacity"),
    FLOAT_COL(mSpeed, "Speed"),
    FLOAT_COL(mRpm, "Rpm"),
    FLOAT_COL(mMaxRPM, "MaxRpm"),
    FLOAT_COL(mBrake, "Brake"),
    FLOAT_COL(mThrottle, "Throttle"),
    FLOAT_COL(mClutch, "Clutch"),
    FLOAT_COL(mSteering, "Steering"),
    INT32_COL(mGear, "Gear"),

    // Motion
    FLOAT_COL_AT(mOrientation, VEC_X, "OrientationX"),
    FLOAT_COL_AT(mOrientation, VEC_Y, "OrientationY"),
    FLOAT_COL_AT(mOrientation, VEC_Z, "OrientationZ"),
    FLOAT_COL_AT(mLocalVelocity, VEC_X, "LocalVelocityX"),
    FLOAT_COL_AT(mLocalVelocity, VEC_Y, "LocalVelocityY"),
    FLOAT_COL_AT(mLocalVelocity, VEC_Z, "LocalVelocityZ"),
    FLOAT_COL_AT(mWorldVelocity, VEC_X, "WorldVelocityX"),
    FLOAT_COL_AT(mWorldVelocity, VEC_Y, "WorldVelocityY"),
    FLOAT_COL_AT(mWorldVelocity, VEC_Z, "WorldVelocityZ"),
    FLOAT_COL_AT(mLocalAcceleration, VEC_X, "LocalAccelerationX"),
    FLOAT_COL_AT(mLocalAcceleration, VEC_Y, "LocalAccelerationY"),
    FLOAT_COL_AT(mLocalAcceleration, VEC_Z, "LocalAccelerationZ"),
    FLOAT_COL_AT(mWorldAcceleration, VEC_X, "WorldAccelerationX"),
    FLOAT_COL_AT(mWorldAcceleration, VEC_Y, "WorldAccelerationY"),
    FLOAT_COL_AT(mWorldAcceleration, VEC_Z, "WorldAccelerationZ"),

    // Wheels / Tyres
    FLOAT_COL_AT(mTyreTemp, TYRE_FRONT_LEFT, "TyreTempFl"),
    FLOAT_COL_AT(mTyreTemp, TYRE_FRONT_RIGHT, "TyreTempFr"),
    FLOAT_COL_AT(mTyreTemp, TYRE_REAR_LEFT, "TyreTempRl"),
    FLOAT_COL_AT(mTyreTemp, TYRE_REAR_RIGHT, "TyreTempRr"),
    FLOAT_COL_AT(mTyreWear, TYRE_FRONT_LEFT, "TyreWearFl"),
    FLOAT_COL_AT(mTyreWear, TYRE_FRONT_RIGHT, "TyreWearFr"),
    FLOAT_COL_AT(mTyreWear, TYRE_REAR_LEFT, "TyreWearRl"),
    FLOAT_COL_AT(mTyreWear, TYRE_REAR_RIGHT, "TyreWearRr"),
    FLOAT_COL_AT(mBrakeTempCelsius, TYRE_FRONT_LEFT, "BrakeTempFl"),
    FLOAT_COL_AT(mBrakeTempCelsius, TYRE_FRONT_RIGHT, "BrakeTempFr"),
    FLOAT_COL_AT(mBrakeTempCelsius, TYRE_REAR_LEFT, "BrakeTempRl"),
    FLOAT_COL_AT(mBrakeTempCelsius, TYRE_REAR_RIGHT, "BrakeTempRr"),

    // Car Damage
    FLOAT_COL(mAeroDamage, "AeroDamage"),
    FLOAT_COL(mEngineDamage, "EngineDamage"),

    // Weather
    FLOAT_COL(mAmbientTemperature, "AmbientTemperature"),
    FLOAT_COL(mTrackTemperature, "TrackTemperature"),
    FLOAT_COL(mRainDensity, "RainDensity"),
    FLOAT_COL(mWindSpeed, "WindSpeed"),
    FLOAT_COL(mWindDirectionX, "WindDirectionX"),
    FLOAT_COL(mWindDirectionY, "WindDirectionY"),

    // PCars2 additions
    FLOAT_COL_AT(mSuspensionTravel, TYRE_FRONT_LEFT, "SuspensionTravelFl"),
    FLOAT_COL_AT(mSuspensionTravel, TYRE_FRONT_RIGHT, "SuspensionTravelFr"),
    FLOAT_COL_AT(mSuspensionTravel, TYRE_REAR_LEFT, "SuspensionTravelRl"),
    FLOAT_COL_AT(mSuspensionTravel, TYRE_REAR_RIGHT, "SuspensionTravelRr"),
    FLOAT_COL_AT(mAirPressure, TYRE_FRONT_LEFT, "AirPressureFl"),
    FLOAT_COL_AT(mAirPressure, TYRE_FRONT_RIGHT, "AirPressureFr"),
    FLOAT_COL_AT(mAirPressure, TYRE_REAR_LEFT, "AirPressureRl"),
    FLOAT_COL_AT(mAirPressure, TYRE_REAR_RIGHT, "AirPressureRr"),
    FLOAT_COL(mEngineSpeed, "EngineSpeed"),
    FLOAT_COL(mEngineTorque, "EngineTorque"),
    FLOAT_COL_AT(mWings, 0, "WingFront"),
    FLOAT_COL_AT(mWings, 1, "WingRear"),
    FLOAT_COL(mHandBrake, "HandBrake"),

    // More race variables
    FLOAT_COL(mBrakeBias, "BrakeBias"),

    // AMS2 additions
    FLOAT_COL_AT(mRideHeight, TYRE_FRONT_LEFT, "RideHeightFl"),
    FLOAT_COL_AT(mRideHeight, TYRE_FRONT_RIGHT, "RideHeightFr"),
    FLOAT_COL_AT(mRideHeight, TYRE_REAR_LEFT, "RideHeightRl"),
    FLOAT_COL_AT(mRideHeight, TYRE_REAR_RIGHT, "RideHeightRr"),
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
  for (size_t col = 0; col < NUM_COLUMNS; col++) {
    void *dst = (uint8_t *)buf + columns[col].buf_offset + (size_t)i * columns[col].elem_size;
    const void *src = (const uint8_t *)tele + columns[col].tele_offset;
    memcpy(dst, src, columns[col].elem_size);
  }
}

static void buffer_flush(row_buffer_t *buf, carquet_writer_t *writer) {
  if (buf->count == 0) {
    return;
  }

  for (size_t col = 0; col < NUM_COLUMNS; col++) {
    const void *data = (const uint8_t *)buf + columns[col].buf_offset;
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
