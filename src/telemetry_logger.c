#include "ams2_telemetry.h"
#include <stdbool.h>
#include <stdio.h>
#include <time.h>

static const char *game_state_names[] = {
    "GAME_EXITED",
    "GAME_FRONT_END",
    "GAME_INGAME_PLAYING",
    "GAME_INGAME_PAUSED",
    "GAME_INGAME_INMENU_TIME_TICKING",
    "GAME_INGAME_RESTARTING",
    "GAME_INGAME_REPLAY",
    "GAME_FRONT_END_REPLAY",
};

static const char *session_state_names[] = {
    "SESSION_INVALID",       "SESSION_PRACTICE", "SESSION_TEST",        "SESSION_QUALIFY",
    "SESSION_FORMATION_LAP", "SESSION_RACE",     "SESSION_TIME_ATTACK",
};

static const char *race_state_names[] = {
    "RACESTATE_INVALID",      "RACESTATE_NOT_STARTED", "RACESTATE_RACING", "RACESTATE_FINISHED",
    "RACESTATE_DISQUALIFIED", "RACESTATE_RETIRED",     "RACESTATE_DNF",
};

static const char *flag_colour_names[] = {
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

static const char *pit_mode_names[] = {
    "PIT_MODE_NONE",      "PIT_MODE_DRIVING_INTO_PITS",     "PIT_MODE_IN_PIT", "PIT_MODE_DRIVING_OUT_OF_PITS",
    "PIT_MODE_IN_GARAGE", "PIT_MODE_DRIVING_OUT_OF_GARAGE",
};

static const char *pit_schedule_names[] = {
    "PIT_SCHEDULE_NONE",
    "PIT_SCHEDULE_PLAYER_REQUESTED",
    "PIT_SCHEDULE_ENGINEER_REQUESTED",
    "PIT_SCHEDULE_DAMAGE_REQUESTED",
    "PIT_SCHEDULE_MANDATORY",
    "PIT_SCHEDULE_DRIVE_THROUGH",
    "PIT_SCHEDULE_STOP_GO",
    "PIT_SCHEDULE_PITSPOT_OCCUPIED",
};

static const char *crash_damage_names[] = {
    "CRASH_DAMAGE_NONE",     "CRASH_DAMAGE_OFFTRACK", "CRASH_DAMAGE_LARGE_PROP",
    "CRASH_DAMAGE_SPINNING", "CRASH_DAMAGE_ROLLING",
};

static const char *ers_deployment_names[] = {
    "ERS_DEPLOYMENT_MODE_NONE",     "ERS_DEPLOYMENT_MODE_OFF",    "ERS_DEPLOYMENT_MODE_BUILD",
    "ERS_DEPLOYMENT_MODE_BALANCED", "ERS_DEPLOYMENT_MODE_ATTACK", "ERS_DEPLOYMENT_MODE_QUAL",
};

static const char *yellow_flag_state_names[] = {
    "YFS_INVALID",   "YFS_NONE",       "YFS_PENDING",  "YFS_PITS_CLOSED", "YFS_PIT_LEAD_LAP",
    "YFS_PITS_OPEN", "YFS_PITS_OPEN2", "YFS_LAST_LAP", "YFS_RESUME",      "YFS_RACE_HALT",
};

static const char *launch_stage_names[] = {"LAUNCH_INVALID", "LAUNCH_OFF", "LAUNCH_REV", "LAUNCH_ON"};

static const char *terrain_names[] = {
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

static const char *flag_reason_names[] = {
    "FLAG_REASON_NONE",
    "FLAG_REASON_SOLO_CRASH",
    "FLAG_REASON_VEHICLE_CRASH",
    "FLAG_REASON_VEHICLE_OBSTRUCTION",
};

static const char *names_mCarFlags[] = {
    "CAR_HEADLIGHT", "CAR_ENGINE_ACTIVE", "CAR_ENGINE_WARNING", "CAR_SPEED_LIMITER", "CAR_ABS", "CAR_HANDBRAKE",
    "CAR_TCS",       "CAR_SCS",
};

static const char *tyre_flag_names[] = {
    "TYRE_ATTACHED",
    "TYRE_INFLATED",
    "TYRE_IS_ON_GROUND",
};

static const char *drs_state_names[] = {
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

static void print_f(const char *name, const float *value, int count) {
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

#define PRINT_BITFLAG(key) print_bitflag(#key, tele->key, names_##key)
#define PRINT_3F(key) print_3f(#key, tele->key)
#define PRINT_4F(key) print_4f(#key, tele->key)
#define PRINT_F(key, size) print_f(#key, tele->key, size)

static void print_terrain_array_as_json(const char *name, const int32_t *values, int count) {
  printf("\"%s\":[", name);
  for (int i = 0; i < count; i++) {
    if (i > 0)
      printf(",");
    printf("\"%s\"", terrain_names[values[i]]);
  }
  printf("],");
}

static void print_as_json(const ams2_telemetry *tele) {
  printf("{\"mVersion\":%u,\"mBuildVersionNumber\":%u,\"mGameState\":\"%s\",\"mSessionState\":\"%s\",\"mRaceState\":\"%"
         "s\",\"mViewedParticipantIndex\":%d,\"mParticipantInfo\":",
         tele->mVersion, tele->mBuildVersionNumber, game_state_names[tele->mGameState],
         session_state_names[tele->mSessionState], race_state_names[tele->mRaceState], tele->mViewedParticipantIndex);

  if (!tele->mNumParticipants) {
    printf("[],");
  } else {
    char sep = '[';
    for (int i = 0; i < tele->mNumParticipants; i++) {
      printf("%c{\"mIsActive\":%s,\"mName\":\"%.64s\",\"mWorldPosition\":[%f,%f,%f],\"mCurrentLapDistance\":%f,"
             "\"mRacePosition\":%u,\"mLapsCompleted\":%u,\"mCurrentLap\":%u,\"mCurrentSector\":%d}",
             sep, tele->mParticipantInfo[i].mIsActive ? "true" : "false", tele->mParticipantInfo[i].mName,
             tele->mParticipantInfo[i].mWorldPosition[0], tele->mParticipantInfo[i].mWorldPosition[1],
             tele->mParticipantInfo[i].mWorldPosition[2], tele->mParticipantInfo[i].mCurrentLapDistance,
             tele->mParticipantInfo[i].mRacePosition, tele->mParticipantInfo[i].mLapsCompleted,
             tele->mParticipantInfo[i].mCurrentLap, tele->mParticipantInfo[i].mCurrentSector);
      sep = ',';
    }
    printf("],");
  }

  PRINT_BITFLAG(mCarFlags);

  printf("\"mTyreFlags\":[");
  for (int i = 0; i < TYRE_MAX; ++i) {
    print_bitflag_value(tele->mTyreFlags[i], tyre_flag_names);
  }
  printf("],");

  print_terrain_array_as_json("mTyreTerrain", tele->mTyreTerrain, TYRE_MAX);
  printf(
      "\"mUnfilteredThrottle\":%f,\"mUnfilteredBrake\":%f,\"mUnfilteredSteering\":%f,\"mUnfilteredClutch\":%f,"
      "\"mCarName\":\"%.64s\",\"mCarClassName\":\"%.64s\",\"mLapsInEvent\":%u,\"mTrackLocation\":\"%.64s\","
      "\"mTrackVariation\":\"%.64s\",\"mTrackLength\":%f,\"mNumSectors\":%d,\"mLapInvalidated\":%s,\"mBestLapTime\":%f,"
      "\"mLastLapTime\":%f,\"mCurrentTime\":%f,\"mSplitTimeAhead\":%f,\"mSplitTimeBehind\":%f,\"mSplitTime\":%f,"
      "\"mEventTimeRemaining\":%f,\"mPersonalFastestLapTime\":%f,\"mWorldFastestLapTime\":%f,\"mCurrentSector1Time\":%"
      "f,\"mCurrentSector2Time\":%f,\"mCurrentSector3Time\":%f,\"mFastestSector1Time\":%f,\"mFastestSector2Time\":%f,"
      "\"mFastestSector3Time\":%f,\"mPersonalFastestSector1Time\":%f,\"mPersonalFastestSector2Time\":%f,"
      "\"mPersonalFastestSector3Time\":%f,\"mWorldFastestSector1Time\":%f,\"mWorldFastestSector2Time\":%f,"
      "\"mWorldFastestSector3Time\":%f,\"mHighestFlagColour\":\"%s\",\"mHighestFlagReason\":\"%s\",\"mPitMode\":\"%s\","
      "\"mPitSchedule\":\"%s\",\"mOilTempCelsius\":%f,\"mOilPressureKPa\":%f,\"mWaterTempCelsius\":%f,"
      "\"mWaterPressureKPa\":%f,\"mFuelPressureKPa\":%f,\"mFuelLevel\":%f,\"mFuelCapacity\":%f,\"mSpeed\":%f,\"mRpm\":%"
      "f,\"mMaxRPM\":%f,\"mBrake\":%f,\"mThrottle\":%f,\"mClutch\":%f,\"mSteering\":%f,\"mGear\":%d,\"mNumGears\":%d,"
      "\"mOdometerKM\":%f,\"mAntiLockActive\":%s,\"mLastOpponentCollisionIndex\":%d,"
      "\"mLastOpponentCollisionMagnitude\":%f,\"mBoostActive\":%s,\"mBoostAmount\":%f,",
      tele->mUnfilteredThrottle, tele->mUnfilteredBrake, tele->mUnfilteredSteering, tele->mUnfilteredClutch,
      tele->mCarName, tele->mCarClassName, tele->mLapsInEvent, tele->mTrackLocation, tele->mTrackVariation,
      tele->mTrackLength, tele->mNumSectors, tele->mLapInvalidated ? "true" : "false", tele->mBestLapTime,
      tele->mLastLapTime, tele->mCurrentTime, tele->mSplitTimeAhead, tele->mSplitTimeBehind, tele->mSplitTime,
      tele->mEventTimeRemaining, tele->mPersonalFastestLapTime, tele->mWorldFastestLapTime, tele->mCurrentSector1Time,
      tele->mCurrentSector2Time, tele->mCurrentSector3Time, tele->mFastestSector1Time, tele->mFastestSector2Time,
      tele->mFastestSector3Time, tele->mPersonalFastestSector1Time, tele->mPersonalFastestSector2Time,
      tele->mPersonalFastestSector3Time, tele->mWorldFastestSector1Time, tele->mWorldFastestSector2Time,
      tele->mWorldFastestSector3Time, flag_colour_names[tele->mHighestFlagColour],
      flag_reason_names[tele->mHighestFlagReason], pit_mode_names[tele->mPitMode],
      pit_schedule_names[tele->mPitSchedule], tele->mOilTempCelsius, tele->mOilPressureKPa, tele->mWaterTempCelsius,
      tele->mWaterPressureKPa, tele->mFuelPressureKPa, tele->mFuelLevel, tele->mFuelCapacity, tele->mSpeed, tele->mRpm,
      tele->mMaxRPM, tele->mBrake, tele->mThrottle, tele->mClutch, tele->mSteering, tele->mGear, tele->mNumGears,
      tele->mOdometerKM, tele->mAntiLockActive ? "true" : "false", tele->mLastOpponentCollisionIndex,
      tele->mLastOpponentCollisionMagnitude, tele->mBoostActive ? "true" : "false", tele->mBoostAmount);

  PRINT_3F(mOrientation);
  PRINT_3F(mLocalVelocity);
  PRINT_3F(mWorldVelocity);
  PRINT_3F(mAngularVelocity);
  PRINT_3F(mLocalAcceleration);
  PRINT_3F(mWorldAcceleration);
  PRINT_3F(mExtentsCentre);
  PRINT_4F(mTyreY);
  PRINT_4F(mTyreRPS);
  PRINT_4F(mTyreTemp);
  PRINT_4F(mTyreHeightAboveGround);
  PRINT_4F(mTyreWear);
  PRINT_4F(mBrakeDamage);
  PRINT_4F(mSuspensionDamage);
  PRINT_4F(mBrakeTempCelsius);
  PRINT_4F(mTyreTreadTemp);
  PRINT_4F(mTyreLayerTemp);
  PRINT_4F(mTyreCarcassTemp);
  PRINT_4F(mTyreRimTemp);
  PRINT_4F(mTyreInternalAirTemp);

  printf("\"mCrashState\":\"%s\",\"mAeroDamage\":%f,\"mEngineDamage\":%f,\"mAmbientTemperature\":%f,"
         "\"mTrackTemperature\":%f,\"mRainDensity\":%f,\"mWindSpeed\":%f,\"mWindDirectionX\":%f,\"mWindDirectionY\":%f,"
         "\"mCloudBrightness\":%f,\"mSequenceNumber\":%u,",
         crash_damage_names[tele->mCrashState], tele->mAeroDamage, tele->mEngineDamage, tele->mAmbientTemperature,
         tele->mTrackTemperature, tele->mRainDensity, tele->mWindSpeed, tele->mWindDirectionX, tele->mWindDirectionY,
         tele->mCloudBrightness, tele->mSequenceNumber);

  PRINT_4F(mWheelLocalPositionY);
  PRINT_4F(mSuspensionTravel);
  PRINT_4F(mSuspensionVelocity);
  PRINT_4F(mAirPressure);
  printf("\"mEngineSpeed\":%f,\"mEngineTorque\":%f,\"mWings\":[%f,%f],\"mHandBrake\":%f,\"mEnforcedPitStopLap\":%d,"
         "\"mTranslatedTrackLocation\":\"%.64s\",\"mTranslatedTrackVariation\":\"%.64s\",\"mBrakeBias\":%f,"
         "\"mTurboBoostPressure\":%f,\"mTyreCompound\":[\"%.40s\",\"%.40s\",\"%.40s\",\"%.40s\"],\"mSnowDensity\":%f,"
         "\"mSessionDuration\":%f,\"mSessionAdditionalLaps\":%d,",
         tele->mEngineSpeed, tele->mEngineTorque, tele->mWings[0], tele->mWings[1], tele->mHandBrake,
         tele->mEnforcedPitStopLap, tele->mTranslatedTrackLocation, tele->mTranslatedTrackVariation, tele->mBrakeBias,
         tele->mTurboBoostPressure, tele->mTyreCompound[0], tele->mTyreCompound[1], tele->mTyreCompound[2],
         tele->mTyreCompound[3], tele->mSnowDensity, tele->mSessionDuration, tele->mSessionAdditionalLaps);

  print_4f("mTyreTempLeft", tele->mTyreTempLeft);
  print_4f("mTyreTempCenter", tele->mTyreTempCenter);
  print_4f("mTyreTempRight", tele->mTyreTempRight);
  print_4f("mRideHeight", tele->mRideHeight);

  print_bitflag("mDrsState", tele->mDrsState, drs_state_names);

  printf("\"mJoyPad0\":%u,\"mDPad\":%u,\"mAntiLockSetting\":%d,\"mTractionControlSetting\":%d,\"mErsDeploymentMode\":"
         "\"%s\",\"mErsAutoModeEnabled\":%s,\"mClutchTemp\":%f,\"mClutchWear\":%f,\"mClutchOverheated\":%s,"
         "\"mClutchSlipping\":%s,\"mYellowFlagState\":\"%s\",\"mSessionIsPrivate\":%s,\"mLaunchStage\":\"%s\",",
         tele->mJoyPad0, tele->mDPad, tele->mAntiLockSetting, tele->mTractionControlSetting,
         ers_deployment_names[tele->mErsDeploymentMode], tele->mErsAutoModeEnabled ? "true" : "false",
         tele->mClutchTemp, tele->mClutchWear, tele->mClutchOverheated ? "true" : "false",
         tele->mClutchSlipping ? "true" : "false", yellow_flag_state_names[tele->mYellowFlagState],
         tele->mSessionIsPrivate ? "true" : "false", launch_stage_names[tele->mLaunchStage + 1]);

  PRINT_F(mCurrentSector1Times, tele->mNumParticipants);
  PRINT_F(mCurrentSector2Times, tele->mNumParticipants);
  PRINT_F(mCurrentSector3Times, tele->mNumParticipants);
  PRINT_F(mFastestSector1Times, tele->mNumParticipants);
  PRINT_F(mFastestSector2Times, tele->mNumParticipants);
  PRINT_F(mFastestSector3Times, tele->mNumParticipants);

  printf("\"mFastestLapTimes\":[");
  for (int i = 0; i < tele->mNumParticipants; i++) {
    if (i > 0)
      printf(",");
    printf("%f", tele->mFastestLapTimes[i]);
  }
  printf("],");
  printf("\"mLastLapTimes\":[");
  for (int i = 0; i < tele->mNumParticipants; i++) {
    if (i > 0)
      printf(",");
    printf("%f", tele->mLastLapTimes[i]);
  }
  printf("],");
  printf("\"mLapsInvalidated\":[");
  for (int i = 0; i < tele->mNumParticipants; i++) {
    if (i > 0)
      printf(",");
    printf("%s", tele->mLapsInvalidated[i] ? "true" : "false");
  }
  printf("],");
  printf("\"mRaceStates\":[");
  for (int i = 0; i < tele->mNumParticipants; i++) {
    if (i > 0)
      printf(",");
    printf("\"%s\"", race_state_names[tele->mRaceStates[i]]);
  }
  printf("],");
  printf("\"mPitModes\":[");
  for (int i = 0; i < tele->mNumParticipants; i++) {
    if (i > 0)
      printf(",");
    printf("\"%s\"", pit_mode_names[tele->mPitModes[i]]);
  }
  printf("],");
  printf("\"mOrientations\":[");
  for (int i = 0; i < tele->mNumParticipants; i++) {
    if (i > 0)
      printf(",");
    printf("[%f,%f,%f]", tele->mOrientations[i][0], tele->mOrientations[i][1], tele->mOrientations[i][2]);
  }
  printf("],");
  printf("\"mSpeeds\":[");
  for (int i = 0; i < tele->mNumParticipants; i++) {
    if (i > 0)
      printf(",");
    printf("%f", tele->mSpeeds[i]);
  }
  printf("],");
  printf("\"mCarNames\":[");
  for (int i = 0; i < tele->mNumParticipants; i++) {
    if (i > 0)
      printf(",");
    printf("\"%.64s\"", tele->mCarNames[i]);
  }
  printf("],");
  printf("\"mCarClassNames\":[");
  for (int i = 0; i < tele->mNumParticipants; i++) {
    if (i > 0)
      printf(",");
    printf("\"%.64s\"", tele->mCarClassNames[i]);
  }
  printf("],");
  printf("\"mPitSchedules\":[");
  for (int i = 0; i < tele->mNumParticipants; i++) {
    if (i > 0)
      printf(",");
    printf("\"%s\"", pit_schedule_names[tele->mPitSchedules[i]]);
  }
  printf("],");
  printf("\"mHighestFlagColours\":[");
  for (int i = 0; i < tele->mNumParticipants; i++) {
    if (i > 0)
      printf(",");
    printf("\"%s\"", flag_colour_names[tele->mHighestFlagColours[i]]);
  }
  printf("],");
  printf("\"mHighestFlagReasons\":[");
  for (int i = 0; i < tele->mNumParticipants; i++) {
    if (i > 0)
      printf(",");
    printf("\"%s\"", flag_reason_names[tele->mHighestFlagReasons[i]]);
  }
  printf("],");
  printf("\"mNationalities\":[");
  for (int i = 0; i < tele->mNumParticipants; i++) {
    if (i > 0)
      printf(",");
    printf("%u", tele->mNationalities[i]);
  }
  printf("]");

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
    if (seq_num && tele.mSequenceNumber != seq_num + 2) {
      fprintf(stderr, "mSequenceNumber jumped from %u to %u - some updates were missed\n", seq_num,
              tele.mSequenceNumber);
    }
    seq_num = tele.mSequenceNumber;
    print_as_json(&tele);

    nanosleep(&long_sleep, NULL);
  }

  return 0;
}
