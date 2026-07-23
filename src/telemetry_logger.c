#include <stdbool.h>
#include <stdio.h>
#include <time.h>

#include "ams2_telemetry.h"

static const char *names_gameState[] = {
    "GAME_EXITED",
    "GAME_FRONT_END",
    "GAME_INGAME_PLAYING",
    "GAME_INGAME_PAUSED",
    "GAME_INGAME_INMENU_TIME_TICKING",
    "GAME_INGAME_RESTARTING",
    "GAME_INGAME_REPLAY",
    "GAME_FRONT_END_REPLAY",
};

static const char *names_sessionState[] = {
    "SESSION_INVALID",       "SESSION_PRACTICE", "SESSION_TEST",        "SESSION_QUALIFY",
    "SESSION_FORMATION_LAP", "SESSION_RACE",     "SESSION_TIME_ATTACK",
};

static const char *names_raceStates[] = {
    "RACESTATE_INVALID",      "RACESTATE_NOT_STARTED", "RACESTATE_RACING", "RACESTATE_FINISHED",
    "RACESTATE_DISQUALIFIED", "RACESTATE_RETIRED",     "RACESTATE_DNF",
};

static const char *names_highestFlagColours[] = {
    "FLAG_COLOUR_NONE",
    "FLAG_COLOUR_GREEN",
    "FLAG_COLOUR_BLUE",
    "FLAG_COLOUR_WHITE_SLOW_CAR",
    "FLAG_COLOUR_WHITE_FINAL_LAP",
    "FLAG_COLOUR_RED",
    "FLAG_COLOUR_YELLOW",
    "FLAG_COLOUR_DOUBLE_YELLOW",
    "FLAG_COLOUR_BLACK_AND_WHITE",
    "FLAG_COLOUR_BLACK_ORANGE_CIRCLE",
    "FLAG_COLOUR_BLACK",
    "FLAG_COLOUR_CHEQUERED",
};

static const char *names_pitModes[] = {
    "PIT_MODE_NONE",      "PIT_MODE_DRIVING_INTO_PITS",     "PIT_MODE_IN_PIT", "PIT_MODE_DRIVING_OUT_OF_PITS",
    "PIT_MODE_IN_GARAGE", "PIT_MODE_DRIVING_OUT_OF_GARAGE",
};

static const char *names_pitSchedules[] = {
    "PIT_SCHEDULE_NONE",
    "PIT_SCHEDULE_PLAYER_REQUESTED",
    "PIT_SCHEDULE_ENGINEER_REQUESTED",
    "PIT_SCHEDULE_DAMAGE_REQUESTED",
    "PIT_SCHEDULE_MANDATORY",
    "PIT_SCHEDULE_DRIVE_THROUGH",
    "PIT_SCHEDULE_STOP_GO",
    "PIT_SCHEDULE_PITSPOT_OCCUPIED",
};

static const char *names_crashState[] = {
    "CRASH_DAMAGE_NONE",     "CRASH_DAMAGE_OFFTRACK", "CRASH_DAMAGE_LARGE_PROP",
    "CRASH_DAMAGE_SPINNING", "CRASH_DAMAGE_ROLLING",
};

static const char *names_ersDeploymentMode[] = {
    "ERS_DEPLOYMENT_MODE_NONE",     "ERS_DEPLOYMENT_MODE_OFF",    "ERS_DEPLOYMENT_MODE_BUILD",
    "ERS_DEPLOYMENT_MODE_BALANCED", "ERS_DEPLOYMENT_MODE_ATTACK", "ERS_DEPLOYMENT_MODE_QUAL",
};

static const char *names_yellowFlagState[] = {
    "YFS_INVALID",   "YFS_NONE",       "YFS_PENDING",  "YFS_PITS_CLOSED", "YFS_PIT_LEAD_LAP",
    "YFS_PITS_OPEN", "YFS_PITS_OPEN2", "YFS_LAST_LAP", "YFS_RESUME",      "YFS_RACE_HALT",
};

static const char *names_launchStage[] = {"LAUNCH_INVALID", "LAUNCH_OFF", "LAUNCH_REV", "LAUNCH_ON"};

static const char *names_tyreTerrain[] = {
    "TERRAIN_ROAD",
    "TERRAIN_LOW_GRIP_ROAD",
    "TERRAIN_BUMPY_ROAD1",
    "TERRAIN_BUMPY_ROAD2",
    "TERRAIN_BUMPY_ROAD3",
    "TERRAIN_MARBLES",
    "TERRAIN_GRASSY_BERMS",
    "TERRAIN_GRASS",
    "TERRAIN_GRAVEL",
    "TERRAIN_BUMPY_GRAVEL",
    "TERRAIN_RUMBLE_STRIPS",
    "TERRAIN_DRAINS",
    "TERRAIN_TYREWALLS",
    "TERRAIN_CEMENTWALLS",
    "TERRAIN_GUARDRAILS",
    "TERRAIN_SAND",
    "TERRAIN_BUMPY_SAND",
    "TERRAIN_DIRT",
    "TERRAIN_BUMPY_DIRT",
    "TERRAIN_DIRT_ROAD",
    "TERRAIN_BUMPY_DIRT_ROAD",
    "TERRAIN_PAVEMENT",
    "TERRAIN_DIRT_BANK",
    "TERRAIN_WOOD",
    "TERRAIN_DRY_VERGE",
    "TERRAIN_EXIT_RUMBLE_STRIPS",
    "TERRAIN_GRASSCRETE",
    "TERRAIN_LONG_GRASS",
    "TERRAIN_SLOPE_GRASS",
    "TERRAIN_COBBLES",
    "TERRAIN_SAND_ROAD",
    "TERRAIN_BAKED_CLAY",
    "TERRAIN_ASTROTURF",
    "TERRAIN_SNOWHALF",
    "TERRAIN_SNOWFULL",
    "TERRAIN_DAMAGED_ROAD1",
    "TERRAIN_TRAIN_TRACK_ROAD",
    "TERRAIN_BUMPYCOBBLES",
    "TERRAIN_ARIES_ONLY",
    "TERRAIN_ORION_ONLY",
    "TERRAIN_B1RUMBLES",
    "TERRAIN_B2RUMBLES",
    "TERRAIN_ROUGH_SAND_MEDIUM",
    "TERRAIN_ROUGH_SAND_HEAVY",
    "TERRAIN_SNOWWALLS",
    "TERRAIN_ICE_ROAD",
    "TERRAIN_RUNOFF_ROAD",
    "TERRAIN_ILLEGAL_STRIP",
    "TERRAIN_PAINT_CONCRETE",
    "TERRAIN_PAINT_CONCRETE_ILLEGAL",
    "TERRAIN_RALLY_TARMAC",
};

static const char *names_highestFlagReasons[] = {
    "FLAG_REASON_NONE",
    "FLAG_REASON_SOLO_CRASH",
    "FLAG_REASON_VEHICLE_CRASH",
    "FLAG_REASON_VEHICLE_OBSTRUCTION",
};

static const char *names_carFlags[] = {
    "CAR_HEADLIGHT", "CAR_ENGINE_ACTIVE", "CAR_ENGINE_WARNING", "CAR_SPEED_LIMITER", "CAR_ABS", "CAR_HANDBRAKE",
    "CAR_TCS",       "CAR_SCS",
};

static const char *names_tyreFlags[] = {
    "TYRE_ATTACHED",
    "TYRE_INFLATED",
    "TYRE_IS_ON_GROUND",
};

static const char *names_drsState[] = {
    "DRS_INSTALLED", "DRS_ZONE_RULES", "DRS_AVAILABLE_NEXT", "DRS_AVAILABLE_NOW", "DRS_ACTIVE",
};

static void print_bitflag_value(int32_t value, const char **names) {
  if (!value) {
    printf("[],");
    return;
  }

  char sep = '[';
  for (int i = 0; value; i++) {
    if (value & (1 << i)) {
      value &= ~(1 << i);
      printf("%c\"%s\"", sep, names[i]);
      sep = ',';
    }
  }
  printf("],");
}

static void print_bitflag(const char *name, int32_t value, const char **names) {
  printf("\"%s\":", name);
  print_bitflag_value(value, names);
}

static void print_3f(const char *name, const float value[3]) {
  printf("\"%s\":[%f,%f,%f],", name, value[0], value[1], value[2]);
}

static void print_4f(const char *name, const float value[4]) {
  printf("\"%s\":[%f,%f,%f,%f],", name, value[0], value[1], value[2], value[3]);
}

static void print_array_f(const char *name, const float *value, int count) {
  printf("\"%s\":", name);
  if (!count) {
    printf("[],");
    return;
  }

  char sep = '[';
  for (int i = 0; i < count; i++) {
    printf("%c%f", sep, value[i]);
    sep = ',';
  }
  printf("],");
}

static void print_array_s(const char *name, const char values[64][64], int count) {
  printf("\"%s\":", name);
  if (!count) {
    printf("[],");
    return;
  }

  char sep = '[';
  for (int i = 0; i < count; i++) {
    printf("%c\"%.64s\"", sep, values[i]);
    sep = ',';
  }
  printf("],");
}

static void print_array_enum(const char *name, const int *indices, const char **enum_values, int count) {
  printf("\"%s\":", name);
  if (!count) {
    printf("[],");
    return;
  }

  char sep = '[';
  for (int i = 0; i < count; i++) {
    printf("%c\"%s\"", sep, enum_values[indices[i]]);
    sep = ',';
  }
  printf("],");
}

#define PRINT_BITFLAG(key) print_bitflag(#key, tele->key, names_##key)
#define PRINT_3F(key) print_3f(#key, tele->key)
#define PRINT_4F(key) print_4f(#key, tele->key)
#define PRINT_ARRAY_F(key, size) print_array_f(#key, tele->key, size)
#define PRINT_ARRAY_S(key, size) print_array_s(#key, tele->key, size)
#define PRINT_ARRAY_ENUM(key, size) print_array_enum(#key, tele->key, names_##key, size)

static void print_as_json(const ams2_telemetry *tele) {
  printf("{\"Version\":%u,\"BuildVersionNumber\":%u,\"GameState\":\"%s\",\"SessionState\":\"%s\",\"RaceState\":\"%"
         "s\",\"ViewedParticipantIndex\":%d,",
         tele->version, tele->buildVersionNumber, names_gameState[tele->gameState],
         names_sessionState[tele->sessionState], names_raceStates[tele->raceState], tele->viewedParticipantIndex);

  if (tele->numParticipants) {
    printf("\"ParticipantInfo\":");
    char sep = '[';
    for (int i = 0; i < tele->numParticipants; i++) {
      printf("%c{\"IsActive\":%s,\"Name\":\"%.64s\",\"WorldPosition\":[%f,%f,%f],\"CurrentLapDistance\":%f,"
             "\"RacePosition\":%u,\"LapsCompleted\":%u,\"CurrentLap\":%u,\"CurrentSector\":%d}",
             sep, tele->participantInfo[i].isActive ? "true" : "false", tele->participantInfo[i].name,
             tele->participantInfo[i].worldPosition[0], tele->participantInfo[i].worldPosition[1],
             tele->participantInfo[i].worldPosition[2], tele->participantInfo[i].currentLapDistance,
             tele->participantInfo[i].racePosition, tele->participantInfo[i].lapsCompleted,
             tele->participantInfo[i].currentLap, tele->participantInfo[i].currentSector);
      sep = ',';
    }
    printf("],");
  }

  PRINT_BITFLAG(carFlags);
  printf("\"TyreFlags\":[");
  for (int i = 0; i < TYRE_MAX; ++i) {
    print_bitflag_value(tele->tyreFlags[i], names_tyreFlags);
  }
  printf("],");

  PRINT_ARRAY_ENUM(tyreTerrain, 4);
  printf("\"UnfilteredThrottle\":%f,\"UnfilteredBrake\":%f,\"UnfilteredSteering\":%f,\"UnfilteredClutch\":%f,"
         "\"CarName\":\"%.64s\",\"CarClassName\":\"%.64s\",\"LapsInEvent\":%u,\"TrackLocation\":\"%.64s\","
         "\"TrackVariation\":\"%.64s\",\"TrackLength\":%f,\"NumSectors\":%d,\"LapInvalidated\":%s,\"BestLapTime\":%f,"
         "\"LastLapTime\":%f,\"CurrentTime\":%f,\"SplitTimeAhead\":%f,\"SplitTimeBehind\":%f,\"SplitTime\":%f,"
         "\"EventTimeRemaining\":%f,\"PersonalFastestLapTime\":%f,\"WorldFastestLapTime\":%f,\"CurrentSector1Time\":%"
         "f,\"CurrentSector2Time\":%f,\"CurrentSector3Time\":%f,\"FastestSector1Time\":%f,\"FastestSector2Time\":%f,"
         "\"FastestSector3Time\":%f,\"PersonalFastestSector1Time\":%f,\"PersonalFastestSector2Time\":%f,"
         "\"PersonalFastestSector3Time\":%f,\"WorldFastestSector1Time\":%f,\"WorldFastestSector2Time\":%f,"
         "\"WorldFastestSector3Time\":%f,\"HighestFlagColour\":\"%s\",\"HighestFlagReason\":\"%s\",\"PitMode\":\"%s\","
         "\"PitSchedule\":\"%s\",\"OilTempCelsius\":%f,\"OilPressureKPa\":%f,\"WaterTempCelsius\":%f,"
         "\"WaterPressureKPa\":%f,\"FuelPressureKPa\":%f,\"FuelLevel\":%f,\"FuelCapacity\":%f,\"Speed\":%f,\"Rpm\":%"
         "f,\"MaxRPM\":%f,\"Brake\":%f,\"Throttle\":%f,\"Clutch\":%f,\"Steering\":%f,\"Gear\":%d,\"NumGears\":%d,"
         "\"OdometerKM\":%f,\"AntiLockActive\":%s,\"LastOpponentCollisionIndex\":%d,"
         "\"LastOpponentCollisionMagnitude\":%f,\"BoostActive\":%s,\"BoostAmount\":%f,",
         tele->unfilteredThrottle, tele->unfilteredBrake, tele->unfilteredSteering, tele->unfilteredClutch,
         tele->carName, tele->carClassName, tele->lapsInEvent, tele->trackLocation, tele->trackVariation,
         tele->trackLength, tele->numSectors, tele->lapInvalidated ? "true" : "false", tele->bestLapTime,
         tele->lastLapTime, tele->currentTime, tele->splitTimeAhead, tele->splitTimeBehind, tele->splitTime,
         tele->eventTimeRemaining, tele->personalFastestLapTime, tele->worldFastestLapTime, tele->currentSector1Time,
         tele->currentSector2Time, tele->currentSector3Time, tele->fastestSector1Time, tele->fastestSector2Time,
         tele->fastestSector3Time, tele->personalFastestSector1Time, tele->personalFastestSector2Time,
         tele->personalFastestSector3Time, tele->worldFastestSector1Time, tele->worldFastestSector2Time,
         tele->worldFastestSector3Time, names_highestFlagColours[tele->highestFlagColour],
         names_highestFlagReasons[tele->highestFlagReason], names_pitModes[tele->pitMode],
         names_pitSchedules[tele->pitSchedule], tele->oilTempCelsius, tele->oilPressureKPa, tele->waterTempCelsius,
         tele->waterPressureKPa, tele->fuelPressureKPa, tele->fuelLevel, tele->fuelCapacity, tele->speed, tele->rpm,
         tele->maxRPM, tele->brake, tele->throttle, tele->clutch, tele->steering, tele->gear, tele->numGears,
         tele->odometerKM, tele->antiLockActive ? "true" : "false", tele->lastOpponentCollisionIndex,
         tele->lastOpponentCollisionMagnitude, tele->boostActive ? "true" : "false", tele->boostAmount);

  PRINT_3F(orientation);
  PRINT_3F(localVelocity);
  PRINT_3F(worldVelocity);
  PRINT_3F(angularVelocity);
  PRINT_3F(localAcceleration);
  PRINT_3F(worldAcceleration);
  PRINT_3F(extentsCentre);
  PRINT_4F(tyreY);
  PRINT_4F(tyreRPS);
  PRINT_4F(tyreTemp);
  PRINT_4F(tyreHeightAboveGround);
  PRINT_4F(tyreWear);
  PRINT_4F(brakeDamage);
  PRINT_4F(suspensionDamage);
  PRINT_4F(brakeTempCelsius);
  PRINT_4F(tyreTreadTemp);
  PRINT_4F(tyreLayerTemp);
  PRINT_4F(tyreCarcassTemp);
  PRINT_4F(tyreRimTemp);
  PRINT_4F(tyreInternalAirTemp);

  printf("\"CrashState\":\"%s\",\"AeroDamage\":%f,\"EngineDamage\":%f,\"AmbientTemperature\":%f,"
         "\"TrackTemperature\":%f,\"RainDensity\":%f,\"WindSpeed\":%f,\"WindDirectionX\":%f,\"WindDirectionY\":%f,"
         "\"CloudBrightness\":%f,\"SequenceNumber\":%u,",
         names_crashState[tele->crashState], tele->aeroDamage, tele->engineDamage, tele->ambientTemperature,
         tele->trackTemperature, tele->rainDensity, tele->windSpeed, tele->windDirectionX, tele->windDirectionY,
         tele->cloudBrightness, tele->sequenceNumber);

  PRINT_4F(wheelLocalPositionY);
  PRINT_4F(suspensionTravel);
  PRINT_4F(suspensionVelocity);
  PRINT_4F(airPressure);
  printf("\"EngineSpeed\":%f,\"EngineTorque\":%f,\"Wings\":[%f,%f],\"HandBrake\":%f,\"EnforcedPitStopLap\":%d,"
         "\"TranslatedTrackLocation\":\"%.64s\",\"TranslatedTrackVariation\":\"%.64s\",\"BrakeBias\":%f,"
         "\"TurboBoostPressure\":%f,\"TyreCompound\":[\"%.40s\",\"%.40s\",\"%.40s\",\"%.40s\"],\"SnowDensity\":%f,"
         "\"SessionDuration\":%f,\"SessionAdditionalLaps\":%d,",
         tele->engineSpeed, tele->engineTorque, tele->wings[0], tele->wings[1], tele->handBrake,
         tele->enforcedPitStopLap, tele->translatedTrackLocation, tele->translatedTrackVariation, tele->brakeBias,
         tele->turboBoostPressure, tele->tyreCompound[0], tele->tyreCompound[1], tele->tyreCompound[2],
         tele->tyreCompound[3], tele->snowDensity, tele->sessionDuration, tele->sessionAdditionalLaps);

  PRINT_4F(tyreTempLeft);
  PRINT_4F(tyreTempCenter);
  PRINT_4F(tyreTempRight);
  PRINT_4F(rideHeight);

  PRINT_BITFLAG(drsState);

  printf("\"JoyPad0\":%u,\"DPad\":%u,\"AntiLockSetting\":%d,\"TractionControlSetting\":%d,\"ErsDeploymentMode\":"
         "\"%s\",\"ErsAutoModeEnabled\":%s,\"ClutchTemp\":%f,\"ClutchWear\":%f,\"ClutchOverheated\":%s,"
         "\"ClutchSlipping\":%s,\"YellowFlagState\":\"%s\",\"SessionIsPrivate\":%s,\"LaunchStage\":\"%s\",",
         tele->joyPad0, tele->dPad, tele->antiLockSetting, tele->tractionControlSetting,
         names_ersDeploymentMode[tele->ersDeploymentMode], tele->ersAutoModeEnabled ? "true" : "false",
         tele->clutchTemp, tele->clutchWear, tele->clutchOverheated ? "true" : "false",
         tele->clutchSlipping ? "true" : "false", names_yellowFlagState[tele->yellowFlagState],
         tele->sessionIsPrivate ? "true" : "false", names_launchStage[tele->launchStage + 1]);

  if (tele->numParticipants) {
    PRINT_ARRAY_F(currentSector1Times, tele->numParticipants);
    PRINT_ARRAY_F(currentSector2Times, tele->numParticipants);
    PRINT_ARRAY_F(currentSector3Times, tele->numParticipants);
    PRINT_ARRAY_F(fastestSector1Times, tele->numParticipants);
    PRINT_ARRAY_F(fastestSector2Times, tele->numParticipants);
    PRINT_ARRAY_F(fastestSector3Times, tele->numParticipants);
    PRINT_ARRAY_F(fastestLapTimes, tele->numParticipants);
    PRINT_ARRAY_F(lastLapTimes, tele->numParticipants);

    char sep = '[';
    printf("\"LapsInvalidated\":");
    for (int i = 0; i < tele->numParticipants; i++) {
      printf("%c%s", sep, tele->lapsInvalidated[i] ? "true" : "false");
      sep = ',';
    }
    printf("],");

    PRINT_ARRAY_ENUM(raceStates, tele->numParticipants);
    PRINT_ARRAY_ENUM(pitModes, tele->numParticipants);

    sep = '[';
    printf("\"Orientations\":[");
    for (int i = 0; i < tele->numParticipants; i++) {
      printf("%c[%f,%f,%f]", sep, tele->orientations[i][0], tele->orientations[i][1], tele->orientations[i][2]);
      sep = ',';
    }
    printf("],");

    PRINT_ARRAY_F(speeds, tele->numParticipants);
    PRINT_ARRAY_S(carNames, tele->numParticipants);
    PRINT_ARRAY_S(carClassNames, tele->numParticipants);
    PRINT_ARRAY_ENUM(pitSchedules, tele->numParticipants);
    PRINT_ARRAY_ENUM(highestFlagColours, tele->numParticipants);
    PRINT_ARRAY_ENUM(highestFlagReasons, tele->numParticipants);

    sep = '[';
    printf("\"Nationalities\":[");
    for (int i = 0; i < tele->numParticipants; i++) {
      printf("%c%u", sep, tele->nationalities[i]);
      sep = ',';
    }
    printf("]");
  }

  printf("}\n");
}

int main() {
  int pid = wait_for_ams2_pid();
  const void *remote_addr = wait_for_ams2_telemetry_address(pid);

  // Sleep interval after message is received, and next message isn't expected for another 20ms
  struct timespec long_sleep = {
      .tv_sec = 0,
      .tv_nsec = 18 * 1000 * 1000,
  };
  // Shorter sleep, when message is expected soon
  struct timespec short_sleep = {
      .tv_sec = 0,
      .tv_nsec = 1 * 1000 * 1000,
  };

  unsigned seq_num = 0;
  while (1) {
    ams2_telemetry tele;
    while (!read_ams2_telemetry(pid, &tele, remote_addr)) {
      // Next update should be close
      nanosleep(&short_sleep, NULL);
    }

    // Got new update
    if (seq_num && tele.sequenceNumber != seq_num + 2) {
      fprintf(stderr, "SequenceNumber jumped from %u to %u - some updates were missed\n", seq_num, tele.sequenceNumber);
    }
    seq_num = tele.sequenceNumber;
    print_as_json(&tele);

    nanosleep(&long_sleep, NULL);
  }

  return 0;
}
