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
  int32_t gameState[BATCH_SIZE];
  int32_t sessionState[BATCH_SIZE];
  int32_t raceState[BATCH_SIZE];

  // Unfiltered Input
  float unfilteredThrottle[BATCH_SIZE];
  float unfilteredBrake[BATCH_SIZE];
  float unfilteredSteering[BATCH_SIZE];
  float unfilteredClutch[BATCH_SIZE];

  // Timings
  float bestLapTime[BATCH_SIZE];
  float lastLapTime[BATCH_SIZE];
  float currentTime[BATCH_SIZE];

  // Car State
  float oilTempCelsius[BATCH_SIZE];
  float oilPressureKPa[BATCH_SIZE];
  float waterTempCelsius[BATCH_SIZE];
  float waterPressureKPa[BATCH_SIZE];
  float fuelPressureKPa[BATCH_SIZE];
  float fuelLevel[BATCH_SIZE];
  float fuelCapacity[BATCH_SIZE];
  float speed[BATCH_SIZE];
  float rpm[BATCH_SIZE];
  float maxRPM[BATCH_SIZE];
  float brake[BATCH_SIZE];
  float throttle[BATCH_SIZE];
  float clutch[BATCH_SIZE];
  float steering[BATCH_SIZE];
  int32_t gear[BATCH_SIZE];

  // Motion
  float orientation[BATCH_SIZE][VEC_MAX];
  float localVelocity[BATCH_SIZE][VEC_MAX];
  float worldVelocity[BATCH_SIZE][VEC_MAX];
  float localAcceleration[BATCH_SIZE][VEC_MAX];
  float worldAcceleration[BATCH_SIZE][VEC_MAX];

  // Wheels / Tyres
  float tyreTemp[BATCH_SIZE][TYRE_MAX];
  float tyreWear[BATCH_SIZE][TYRE_MAX];
  float brakeTempCelsius[BATCH_SIZE][TYRE_MAX];

  // Car Damage
  float aeroDamage[BATCH_SIZE];
  float engineDamage[BATCH_SIZE];

  // Weather
  float ambientTemperature[BATCH_SIZE];
  float trackTemperature[BATCH_SIZE];
  float rainDensity[BATCH_SIZE];
  float windSpeed[BATCH_SIZE];
  float windDirectionX[BATCH_SIZE];
  float windDirectionY[BATCH_SIZE];

  // PCars2 additions
  float suspensionTravel[BATCH_SIZE][TYRE_MAX];
  float airPressure[BATCH_SIZE][TYRE_MAX];
  float engineSpeed[BATCH_SIZE];
  float engineTorque[BATCH_SIZE];
  float wings[BATCH_SIZE][2];
  float handBrake[BATCH_SIZE];

  // More race variables
  float brakeBias[BATCH_SIZE];

  // AMS2 additions
  float rideHeight[BATCH_SIZE][TYRE_MAX];

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
    INT32_COL(gameState, "GameState"),
    INT32_COL(sessionState, "SessionState"),
    INT32_COL(raceState, "RaceState"),

    // Unfiltered Input
    FLOAT_COL(unfilteredThrottle, "UnfilteredThrottle"),
    FLOAT_COL(unfilteredBrake, "UnfilteredBrake"),
    FLOAT_COL(unfilteredSteering, "UnfilteredSteering"),
    FLOAT_COL(unfilteredClutch, "UnfilteredClutch"),

    // Timings
    FLOAT_COL(bestLapTime, "BestLapTime"),
    FLOAT_COL(lastLapTime, "LastLapTime"),
    FLOAT_COL(currentTime, "CurrentTime"),

    // Car State
    FLOAT_COL(oilTempCelsius, "OilTempCelsius"),
    FLOAT_COL(oilPressureKPa, "OilPressureKPa"),
    FLOAT_COL(waterTempCelsius, "WaterTempCelsius"),
    FLOAT_COL(waterPressureKPa, "WaterPressureKPa"),
    FLOAT_COL(fuelPressureKPa, "FuelPressureKPa"),
    FLOAT_COL(fuelLevel, "FuelLevel"),
    FLOAT_COL(fuelCapacity, "FuelCapacity"),
    FLOAT_COL(speed, "Speed"),
    FLOAT_COL(rpm, "Rpm"),
    FLOAT_COL(maxRPM, "MaxRpm"),
    FLOAT_COL(brake, "Brake"),
    FLOAT_COL(throttle, "Throttle"),
    FLOAT_COL(clutch, "Clutch"),
    FLOAT_COL(steering, "Steering"),
    INT32_COL(gear, "Gear"),

    // Motion
    FLOAT_COL_AT(orientation, VEC_X, "OrientationX"),
    FLOAT_COL_AT(orientation, VEC_Y, "OrientationY"),
    FLOAT_COL_AT(orientation, VEC_Z, "OrientationZ"),
    FLOAT_COL_AT(localVelocity, VEC_X, "LocalVelocityX"),
    FLOAT_COL_AT(localVelocity, VEC_Y, "LocalVelocityY"),
    FLOAT_COL_AT(localVelocity, VEC_Z, "LocalVelocityZ"),
    FLOAT_COL_AT(worldVelocity, VEC_X, "WorldVelocityX"),
    FLOAT_COL_AT(worldVelocity, VEC_Y, "WorldVelocityY"),
    FLOAT_COL_AT(worldVelocity, VEC_Z, "WorldVelocityZ"),
    FLOAT_COL_AT(localAcceleration, VEC_X, "LocalAccelerationX"),
    FLOAT_COL_AT(localAcceleration, VEC_Y, "LocalAccelerationY"),
    FLOAT_COL_AT(localAcceleration, VEC_Z, "LocalAccelerationZ"),
    FLOAT_COL_AT(worldAcceleration, VEC_X, "WorldAccelerationX"),
    FLOAT_COL_AT(worldAcceleration, VEC_Y, "WorldAccelerationY"),
    FLOAT_COL_AT(worldAcceleration, VEC_Z, "WorldAccelerationZ"),

    // Wheels / Tyres
    FLOAT_COL_AT(tyreTemp, TYRE_FRONT_LEFT, "TyreTempFl"),
    FLOAT_COL_AT(tyreTemp, TYRE_FRONT_RIGHT, "TyreTempFr"),
    FLOAT_COL_AT(tyreTemp, TYRE_REAR_LEFT, "TyreTempRl"),
    FLOAT_COL_AT(tyreTemp, TYRE_REAR_RIGHT, "TyreTempRr"),
    FLOAT_COL_AT(tyreWear, TYRE_FRONT_LEFT, "TyreWearFl"),
    FLOAT_COL_AT(tyreWear, TYRE_FRONT_RIGHT, "TyreWearFr"),
    FLOAT_COL_AT(tyreWear, TYRE_REAR_LEFT, "TyreWearRl"),
    FLOAT_COL_AT(tyreWear, TYRE_REAR_RIGHT, "TyreWearRr"),
    FLOAT_COL_AT(brakeTempCelsius, TYRE_FRONT_LEFT, "BrakeTempFl"),
    FLOAT_COL_AT(brakeTempCelsius, TYRE_FRONT_RIGHT, "BrakeTempFr"),
    FLOAT_COL_AT(brakeTempCelsius, TYRE_REAR_LEFT, "BrakeTempRl"),
    FLOAT_COL_AT(brakeTempCelsius, TYRE_REAR_RIGHT, "BrakeTempRr"),

    // Car Damage
    FLOAT_COL(aeroDamage, "AeroDamage"),
    FLOAT_COL(engineDamage, "EngineDamage"),

    // Weather
    FLOAT_COL(ambientTemperature, "AmbientTemperature"),
    FLOAT_COL(trackTemperature, "TrackTemperature"),
    FLOAT_COL(rainDensity, "RainDensity"),
    FLOAT_COL(windSpeed, "WindSpeed"),
    FLOAT_COL(windDirectionX, "WindDirectionX"),
    FLOAT_COL(windDirectionY, "WindDirectionY"),

    // PCars2 additions
    FLOAT_COL_AT(suspensionTravel, TYRE_FRONT_LEFT, "SuspensionTravelFl"),
    FLOAT_COL_AT(suspensionTravel, TYRE_FRONT_RIGHT, "SuspensionTravelFr"),
    FLOAT_COL_AT(suspensionTravel, TYRE_REAR_LEFT, "SuspensionTravelRl"),
    FLOAT_COL_AT(suspensionTravel, TYRE_REAR_RIGHT, "SuspensionTravelRr"),
    FLOAT_COL_AT(airPressure, TYRE_FRONT_LEFT, "AirPressureFl"),
    FLOAT_COL_AT(airPressure, TYRE_FRONT_RIGHT, "AirPressureFr"),
    FLOAT_COL_AT(airPressure, TYRE_REAR_LEFT, "AirPressureRl"),
    FLOAT_COL_AT(airPressure, TYRE_REAR_RIGHT, "AirPressureRr"),
    FLOAT_COL(engineSpeed, "EngineSpeed"),
    FLOAT_COL(engineTorque, "EngineTorque"),
    FLOAT_COL_AT(wings, 0, "WingFront"),
    FLOAT_COL_AT(wings, 1, "WingRear"),
    FLOAT_COL(handBrake, "HandBrake"),

    // More race variables
    FLOAT_COL(brakeBias, "BrakeBias"),

    // AMS2 additions
    FLOAT_COL_AT(rideHeight, TYRE_FRONT_LEFT, "RideHeightFl"),
    FLOAT_COL_AT(rideHeight, TYRE_FRONT_RIGHT, "RideHeightFr"),
    FLOAT_COL_AT(rideHeight, TYRE_REAR_LEFT, "RideHeightRl"),
    FLOAT_COL_AT(rideHeight, TYRE_REAR_RIGHT, "RideHeightRr"),
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

    if (seq_num && tele.sequenceNumber != seq_num + 2) {
      fprintf(stderr, "Skipped updates: %u -> %u\n", seq_num, tele.sequenceNumber);
    }
    seq_num = tele.sequenceNumber;

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
