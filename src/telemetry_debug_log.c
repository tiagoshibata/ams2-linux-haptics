// This is opencode generated, and used only for debugging. I didn't check for correctness - it might be missing fields
// or have mistakes
#include "ams2_telemetry.h"
#include <stdio.h>

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

static const char *launch_stage_names[] = {
    "LAUNCH_INVALID",
    "LAUNCH_OFF",
    "LAUNCH_REV",
    "LAUNCH_ON",
};

static void print_participant_info(const ParticipantInfo *info) {
  printf("Name: %.64s IsActive: %d RacePosition: %u WorldPosition: [%.2f, %.2f, %.2f] Laps: Completed: %u Current: %u "
         "CurrentLapDistance: %.2f CurrentSector: %u\n",
         info->mName, info->mIsActive, info->mRacePosition, info->mWorldPosition[0], info->mWorldPosition[1],
         info->mWorldPosition[2], info->mLapsCompleted, info->mCurrentLap, info->mCurrentLapDistance,
         info->mCurrentSector);
}

static void print_float_array(const char *name, const float *arr, int len) {
  printf("  %s: [", name);
  for (int i = 0; i < len; i++) {
    printf("%.2f", arr[i]);
    if (i < len - 1)
      printf(", ");
  }
  printf("]\n");
}

static void print_tyre_array(const char *name, const float *arr) {
  printf("  %s: [", name);
  for (int i = 0; i < TYRE_MAX; i++) {
    printf("%.2f", arr[i]);
    if (i < TYRE_MAX - 1)
      printf(", ");
  }
  printf("]\n");
}

static void print_tyre_flags_array(const char *name, const TyreFlags *arr) {
  printf("  %s: [", name);
  for (int i = 0; i < TYRE_MAX; i++) {
    printf("%u", arr[i]);
    if (i < TYRE_MAX - 1)
      printf(", ");
  }
  printf("]\n");
}

static void print_terrain_array(const char *name, const Terrain *arr) {
  printf("  %s: [", name);
  for (int i = 0; i < TYRE_MAX; i++) {
    printf("%u", arr[i]);
    if (i < TYRE_MAX - 1)
      printf(", ");
  }
  printf("]\n");
}

void telemetry_debug_log(const ams2_telemetry *tele) {
  printf("ProtocolVersion: %u  BuildVersionNumber: %u\n", tele->mVersion, tele->mBuildVersionNumber);
  printf("GameState: %d (%s) SessionState: %d (%s) RaceState: %d (%s)\n", tele->mGameState,
         game_state_names[tele->mGameState], tele->mSessionState, session_state_names[tele->mSessionState],
         tele->mRaceState, race_state_names[tele->mRaceState]);

  printf("\nParticipants:\n");
  printf("  ViewedParticipantIndex: %d\n", tele->mViewedParticipantIndex);
  for (int i = 0; i < tele->mNumParticipants; i++) {
    printf("  Participant[%d]: ", i);
    print_participant_info(&tele->mParticipantInfo[i]);
  }

  printf("\nUnfiltered Input: Throttle: %.2f  Brake: %.2f  Steering: %.2f  Clutch: %.2f\n", tele->mUnfilteredThrottle,
         tele->mUnfilteredBrake, tele->mUnfilteredSteering, tele->mUnfilteredClutch);

  printf("\nCar Name: %.64s (class: %.64s)\n", tele->mCarName, tele->mCarClassName);
  printf("Track: %.64s Variation: %.64s (%.2fkm) LapsInEvent: %u\n", tele->mTrackLocation, tele->mTrackVariation,
         tele->mTrackLength, tele->mLapsInEvent);

  printf("\nTimings:\n");
  printf("  NumSectors: %d LapInvalidated: %d\n", tele->mNumSectors, tele->mLapInvalidated);
  printf("  LapTime: Best: %.3f Last: %.3f Current: %.3f PersonalFastest: %.3f WorldFastest: %.3f\n",
         tele->mBestLapTime, tele->mLastLapTime, tele->mCurrentTime, tele->mPersonalFastestLapTime,
         tele->mWorldFastestLapTime);
  printf("  SplitTime: %.3f Ahead: %.3f Behind: %.3f EventTimeRemaining: %.3f\n", tele->mSplitTime,
         tele->mSplitTimeAhead, tele->mSplitTimeBehind, tele->mEventTimeRemaining);
  printf("  SectorTime: Current: [%.3f, %.3f, %.3f] Fastest: [%.3f, %.3f, %.3f] PersonalFastest: [%.3f, %.3f, %.3f] "
         "WorldFastest: [%.3f, %.3f, %.3f]\n",
         tele->mCurrentSector1Time, tele->mCurrentSector2Time, tele->mCurrentSector3Time, tele->mFastestSector1Time,
         tele->mFastestSector2Time, tele->mFastestSector3Time, tele->mPersonalFastestSector1Time,
         tele->mPersonalFastestSector2Time, tele->mPersonalFastestSector3Time, tele->mWorldFastestSector1Time,
         tele->mWorldFastestSector2Time, tele->mWorldFastestSector3Time);

  printf("\nFlags: HighestFlagColour: %d (%s) Reason: %d\n", tele->mHighestFlagColour,
         flag_colour_names[tele->mHighestFlagColour], tele->mHighestFlagReason);

  printf("\nPit Info: Mode: %d (%s) Schedule: %d (%s)\n", tele->mPitMode, pit_mode_names[tele->mPitMode],
         tele->mPitSchedule, pit_schedule_names[tele->mPitSchedule]);

  printf("\nCar State:\n");
  printf("  CarFlags: 0x%08x\n", tele->mCarFlags);
  printf("  Oil: Temp: %.2fC Pressure: %.2fKPa\n", tele->mOilTempCelsius, tele->mOilPressureKPa);
  printf("  Water: Temp: %.2fC Pressure: %.2fKPa\n", tele->mWaterTempCelsius, tele->mWaterPressureKPa);
  printf("  Fuel: %.2f/%.2f Pressure: %.2fKPa\n", tele->mFuelLevel, tele->mFuelCapacity, tele->mFuelPressureKPa);
  printf("  Speed: %.2f m/s Rpm: %.0f/%.0f\n", tele->mSpeed, tele->mRpm, tele->mMaxRPM);
  printf("  Brake: %.2f Throttle: %.2f Clutch: %.2f Steering: %.2f\n", tele->mBrake, tele->mThrottle, tele->mClutch,
         tele->mSteering);
  printf("  Gear: %d/%d\n", tele->mGear, tele->mNumGears);
  printf("  OdometerKM: %.2f\n", tele->mOdometerKM);
  printf("  AntiLockActive: %d\n", tele->mAntiLockActive);
  printf("  LastOpponentCollisionIndex: %d Magnitude: %.2f\n", tele->mLastOpponentCollisionIndex,
         tele->mLastOpponentCollisionMagnitude);
  printf("  Boost: Active: %d Amount: %.2f\n", tele->mBoostActive, tele->mBoostAmount);

  printf("\nMotion:\n");
  print_float_array("Orientation", tele->mOrientation, VEC_MAX);
  print_float_array("LocalVelocity", tele->mLocalVelocity, VEC_MAX);
  print_float_array("WorldVelocity", tele->mWorldVelocity, VEC_MAX);
  print_float_array("AngularVelocity", tele->mAngularVelocity, VEC_MAX);
  print_float_array("LocalAcceleration", tele->mLocalAcceleration, VEC_MAX);
  print_float_array("WorldAcceleration", tele->mWorldAcceleration, VEC_MAX);
  print_float_array("ExtentsCentre", tele->mExtentsCentre, VEC_MAX);

  printf("\nWheels / Tyres:\n");
  print_tyre_flags_array("TyreFlags", tele->mTyreFlags);
  print_terrain_array("TyreTerrain", tele->mTyreTerrain);
  print_tyre_array("TyreY", tele->mTyreY);
  print_tyre_array("TyreRPS", tele->mTyreRPS);
  print_tyre_array("TyreSlipSpeed", tele->mTyreSlipSpeed);
  print_tyre_array("TyreTemp", tele->mTyreTemp);
  print_tyre_array("TyreGrip", tele->mTyreGrip);
  print_tyre_array("TyreHeightAboveGround", tele->mTyreHeightAboveGround);
  print_tyre_array("TyreLateralStiffness", tele->mTyreLateralStiffness);
  print_tyre_array("TyreWear", tele->mTyreWear);
  print_tyre_array("BrakeDamage", tele->mBrakeDamage);
  print_tyre_array("SuspensionDamage", tele->mSuspensionDamage);
  print_tyre_array("BrakeTempCelsius", tele->mBrakeTempCelsius);
  print_tyre_array("TyreTreadTemp", tele->mTyreTreadTemp);
  print_tyre_array("TyreLayerTemp", tele->mTyreLayerTemp);
  print_tyre_array("TyreCarcassTemp", tele->mTyreCarcassTemp);
  print_tyre_array("TyreRimTemp", tele->mTyreRimTemp);
  print_tyre_array("TyreInternalAirTemp", tele->mTyreInternalAirTemp);

  printf("\nCar Damage: CrashState: %d (%s) AeroDamage: %.2f EngineDamage: %.2f\n", tele->mCrashState,
         crash_damage_names[tele->mCrashState], tele->mAeroDamage, tele->mEngineDamage);

  printf("\nWeather:\n");
  printf("  Temperature: Ambient: %.2fC Track: %.2fC\n", tele->mAmbientTemperature, tele->mTrackTemperature);
  printf("  RainDensity: %.2f\n", tele->mRainDensity);
  printf("  Wind: Speed: %.2f Direction: [%.2f, %.2f]\n", tele->mWindSpeed, tele->mWindDirectionX,
         tele->mWindDirectionY);
  printf("  CloudBrightness: %.2f\n", tele->mCloudBrightness);

  printf("\nSequenceNumber: %u\n", tele->mSequenceNumber);

  printf("\nAdditional Car Variables:\n");
  print_tyre_array("WheelLocalPositionY", tele->mWheelLocalPositionY);
  print_tyre_array("SuspensionTravel", tele->mSuspensionTravel);
  print_tyre_array("SuspensionVelocity", tele->mSuspensionVelocity);
  print_tyre_array("AirPressure", tele->mAirPressure);
  printf("  EngineSpeed: %.2f rad/s EngineTorque: %.2f Nm\n", tele->mEngineSpeed, tele->mEngineTorque);
  printf("  Wings: [%.2f, %.2f]\n", tele->mWings[0], tele->mWings[1]);
  printf("  HandBrake: %.2f\n", tele->mHandBrake);

  printf("\nRace Variables:\n");
  printf("  EnforcedPitStopLap: %d\n", tele->mEnforcedPitStopLap);
  printf("  TranslatedTrack: Location: %.64s Variation: %.64s\n", tele->mTranslatedTrackLocation,
         tele->mTranslatedTrackVariation);
  printf("  BrakeBias: %.2f TurboBoostPressure: %.2f\n", tele->mBrakeBias, tele->mTurboBoostPressure);
  printf("  SnowDensity: %.2f\n", tele->mSnowDensity);
  printf("  Session: Duration: %.2fmin AdditionalLaps: %d\n", tele->mSessionDuration, tele->mSessionAdditionalLaps);

  printf("\nTyres:\n");
  print_tyre_array("TempLeft", tele->mTyreTempLeft);
  print_tyre_array("TempCenter", tele->mTyreTempCenter);
  print_tyre_array("TempRight", tele->mTyreTempRight);
  print_tyre_array("RideHeight", tele->mRideHeight);

  printf("\nDRSState: 0x%08x\n", tele->mDrsState);

  printf("\nInput: JoyPad0: 0x%08x DPad: 0x%08x AntiLockSetting: %d TractionControlSetting: %d\n", tele->mJoyPad0,
         tele->mDPad, tele->mAntiLockSetting, tele->mTractionControlSetting);

  printf("\nERS: DeploymentMode: %d (%s) AutoModeEnabled: %d\n", tele->mErsDeploymentMode,
         ers_deployment_names[tele->mErsDeploymentMode], tele->mErsAutoModeEnabled);

  printf("\nClutch: Temp: %.2fK Wear: %.2f Overheated: %d Slipping: %d\n", tele->mClutchTemp, tele->mClutchWear,
         tele->mClutchOverheated, tele->mClutchSlipping);

  printf("\nSession: YellowFlagState: %d (%s) SessionIsPrivate: %d LaunchStage: %d (%s)\n", tele->mYellowFlagState,
         yellow_flag_state_names[tele->mYellowFlagState + 1], tele->mSessionIsPrivate, tele->mLaunchStage,
         launch_stage_names[tele->mLaunchStage + 1]);
}
