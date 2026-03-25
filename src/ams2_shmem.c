// This is opencode generated, and used only for debugging. I didn't check for correctness - it can have mistakes
#include "ams2_shmem.h"
#include <assert.h>
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

void ams2_shmem_print(const ams2_shmem *data) {
  printf("\033[2J\033[H");

  printf("ProtocolVersion: %u  BuildVersionNumber: %u\n", data->mVersion, data->mBuildVersionNumber);
  printf("GameState: %d (%s) SessionState: %d (%s) RaceState: %d (%s)\n", data->mGameState,
         game_state_names[data->mGameState] ? game_state_names[data->mGameState] : "UNKNOWN", data->mSessionState,
         session_state_names[data->mSessionState] ? session_state_names[data->mSessionState] : "UNKNOWN",
         data->mRaceState, race_state_names[data->mRaceState] ? race_state_names[data->mRaceState] : "UNKNOWN");

  printf("\nParticipants:\n");
  printf("  ViewedParticipantIndex: %d\n", data->mViewedParticipantIndex);
  assert(data->mNumParticipants <= STORED_PARTICIPANTS_MAX);
  for (int i = 0; i < data->mNumParticipants; i++) {
    printf("  Participant[%d]: ", i);
    print_participant_info(&data->mParticipantInfo[i]);
  }

  printf("\nUnfiltered Input: Throttle: %.2f  Brake: %.2f  Steering: %.2f  Clutch: %.2f\n", data->mUnfilteredThrottle,
         data->mUnfilteredBrake, data->mUnfilteredSteering, data->mUnfilteredClutch);

  printf("\nCar Name: %.64s (class: %.64s)\n", data->mCarName, data->mCarClassName);
  printf("Track: %.64s Variation: %.64s (%.2fkm) LapsInEvent: %u\n", data->mTrackLocation, data->mTrackVariation,
         data->mTrackLength, data->mLapsInEvent);

  printf("\nTimings:\n");
  printf("  NumSectors: %d\n", data->mNumSectors);
  printf("  LapInvalidated: %d\n", data->mLapInvalidated);
  printf("  LapTime: Best: %.3f Last: %.3f Current: %.3f PersonalFastest: %.3f WorldFastest: %.3f\n",
         data->mBestLapTime, data->mLastLapTime, data->mCurrentTime, data->mPersonalFastestLapTime,
         data->mWorldFastestLapTime);
  printf("  SplitTime: %.3f Ahead: %.3f Behind: %.3f\n", data->mSplitTime, data->mSplitTimeAhead,
         data->mSplitTimeBehind);
  printf("  EventTimeRemaining: %.3f\n", data->mEventTimeRemaining);
  printf("  SectorTime: Current: [%.3f, %.3f, %.3f] Fastest: [%.3f, %.3f, %.3f] PersonalFastest: [%.3f, %.3f, %.3f] "
         "WorldFastest: [%.3f, %.3f, %.3f]\n",
         data->mCurrentSector1Time, data->mCurrentSector2Time, data->mCurrentSector3Time, data->mFastestSector1Time,
         data->mFastestSector2Time, data->mFastestSector3Time, data->mPersonalFastestSector1Time,
         data->mPersonalFastestSector2Time, data->mPersonalFastestSector3Time, data->mWorldFastestSector1Time,
         data->mWorldFastestSector2Time, data->mWorldFastestSector3Time);

  printf("\nFlags: HighestFlagColour: %d (%s) Reason: %d\n", data->mHighestFlagColour,
         data->mHighestFlagColour >= 0 && data->mHighestFlagColour < FLAG_COLOUR_MAX
             ? flag_colour_names[data->mHighestFlagColour]
             : "UNKNOWN",
         data->mHighestFlagReason);

  printf("\nPit Info: Mode: %d (%s) Schedule: %d (%s)\n", data->mPitMode,
         pit_mode_names[data->mPitMode] ? pit_mode_names[data->mPitMode] : "UNKNOWN", data->mPitSchedule,
         pit_schedule_names[data->mPitSchedule] ? pit_schedule_names[data->mPitSchedule] : "UNKNOWN");

  printf("\nCar State:\n");
  printf("  CarFlags: 0x%08x\n", data->mCarFlags);
  printf("  Oil: Temp: %.2fC Pressure: %.2fKPa\n", data->mOilTempCelsius, data->mOilPressureKPa);
  printf("  Water: Temp: %.2fC Pressure: %.2fKPa\n", data->mWaterTempCelsius, data->mWaterPressureKPa);
  printf("  Fuel: %.2f/%.2f Pressure: %.2fKPa\n", data->mFuelLevel, data->mFuelCapacity, data->mFuelPressureKPa);
  printf("  Speed: %.2f m/s\n", data->mSpeed);
  printf("  Rpm: %.0f/%.0f\n", data->mRpm, data->mMaxRPM);
  printf("  Brake: %.2f\n", data->mBrake);
  printf("  Throttle: %.2f\n", data->mThrottle);
  printf("  Clutch: %.2f\n", data->mClutch);
  printf("  Steering: %.2f\n", data->mSteering);
  printf("  Gear: %d/%d\n", data->mGear, data->mNumGears);
  printf("  OdometerKM: %.2f\n", data->mOdometerKM);
  printf("  AntiLockActive: %d\n", data->mAntiLockActive);
  printf("  LastOpponentCollisionIndex: %d Magnitude: %.2f\n", data->mLastOpponentCollisionIndex,
         data->mLastOpponentCollisionMagnitude);
  printf("  Boost: Active: %d Amount: %.2f\n", data->mBoostActive, data->mBoostAmount);

  printf("\nMotion:\n");
  print_float_array("Orientation", data->mOrientation, VEC_MAX);
  print_float_array("LocalVelocity", data->mLocalVelocity, VEC_MAX);
  print_float_array("WorldVelocity", data->mWorldVelocity, VEC_MAX);
  print_float_array("AngularVelocity", data->mAngularVelocity, VEC_MAX);
  print_float_array("LocalAcceleration", data->mLocalAcceleration, VEC_MAX);
  print_float_array("WorldAcceleration", data->mWorldAcceleration, VEC_MAX);
  print_float_array("ExtentsCentre", data->mExtentsCentre, VEC_MAX);

  printf("\nWheels / Tyres:\n");
  print_tyre_flags_array("TyreFlags", data->mTyreFlags);
  print_terrain_array("TyreTerrain", data->mTyreTerrain);
  print_tyre_array("TyreY", data->mTyreY);
  print_tyre_array("TyreRPS", data->mTyreRPS);
  print_tyre_array("TyreSlipSpeed", data->mTyreSlipSpeed);
  print_tyre_array("TyreTemp", data->mTyreTemp);
  print_tyre_array("TyreGrip", data->mTyreGrip);
  print_tyre_array("TyreHeightAboveGround", data->mTyreHeightAboveGround);
  print_tyre_array("TyreLateralStiffness", data->mTyreLateralStiffness);
  print_tyre_array("TyreWear", data->mTyreWear);
  print_tyre_array("BrakeDamage", data->mBrakeDamage);
  print_tyre_array("SuspensionDamage", data->mSuspensionDamage);
  print_tyre_array("BrakeTempCelsius", data->mBrakeTempCelsius);
  print_tyre_array("TyreTreadTemp", data->mTyreTreadTemp);
  print_tyre_array("TyreLayerTemp", data->mTyreLayerTemp);
  print_tyre_array("TyreCarcassTemp", data->mTyreCarcassTemp);
  print_tyre_array("TyreRimTemp", data->mTyreRimTemp);
  print_tyre_array("TyreInternalAirTemp", data->mTyreInternalAirTemp);

  printf("\nCar Damage: CrashState: %d (%s) AeroDamage: %.2f EngineDamage: %.2f\n", data->mCrashState,
         crash_damage_names[data->mCrashState] ? crash_damage_names[data->mCrashState] : "UNKNOWN", data->mAeroDamage,
         data->mEngineDamage);

  printf("\nWeather:\n");
  printf("  Temperature: Ambient: %.2fC Track: %.2fC\n", data->mAmbientTemperature, data->mTrackTemperature);
  printf("  RainDensity: %.2f\n", data->mRainDensity);
  printf("  Wind: Speed: %.2f Direction: [%.2f, %.2f]\n", data->mWindSpeed, data->mWindDirectionX,
         data->mWindDirectionY);
  printf("  CloudBrightness: %.2f\n", data->mCloudBrightness);

  printf("\nSequence:\n");
  printf("  SequenceNumber: %u\n", data->mSequenceNumber);

  printf("\nAdditional Car Variables:\n");
  print_tyre_array("WheelLocalPositionY", data->mWheelLocalPositionY);
  print_tyre_array("SuspensionTravel", data->mSuspensionTravel);
  print_tyre_array("SuspensionVelocity", data->mSuspensionVelocity);
  print_tyre_array("AirPressure", data->mAirPressure);
  printf("  EngineSpeed: %.2f rad/s\n", data->mEngineSpeed);
  printf("  EngineTorque: %.2f Nm\n", data->mEngineTorque);
  printf("  Wings: [%.2f, %.2f]\n", data->mWings[0], data->mWings[1]);
  printf("  HandBrake: %.2f\n", data->mHandBrake);

  printf("\nRace Variables:\n");
  printf("  EnforcedPitStopLap: %d\n", data->mEnforcedPitStopLap);
  printf("  TranslatedTrack: Location: %.64s Variation: %.64s\n", data->mTranslatedTrackLocation,
         data->mTranslatedTrackVariation);
  printf("  BrakeBias: %.2f\n", data->mBrakeBias);
  printf("  TurboBoostPressure: %.2f\n", data->mTurboBoostPressure);
  printf("  SnowDensity: %.2f\n", data->mSnowDensity);
  printf("  Session: Duration: %.2fmin AdditionalLaps: %d\n", data->mSessionDuration, data->mSessionAdditionalLaps);

  printf("\nTyres:\n");
  print_tyre_array("TempLeft", data->mTyreTempLeft);
  print_tyre_array("TempCenter", data->mTyreTempCenter);
  print_tyre_array("TempRight", data->mTyreTempRight);
  print_tyre_array("RideHeight", data->mRideHeight);

  printf("\nDRS: State: 0x%08x\n", data->mDrsState);

  printf("\nInput: JoyPad0: 0x%08x DPad: 0x%08x AntiLockSetting: %d TractionControlSetting: %d\n", data->mJoyPad0,
         data->mDPad, data->mAntiLockSetting, data->mTractionControlSetting);

  printf("\nERS: DeploymentMode: %d (%s) AutoModeEnabled: %d\n", data->mErsDeploymentMode,
         ers_deployment_names[data->mErsDeploymentMode] ? ers_deployment_names[data->mErsDeploymentMode] : "UNKNOWN",
         data->mErsAutoModeEnabled);

  printf("\nClutch: Temp: %.2fK Wear: %.2f Overheated: %d Slipping: %d\n", data->mClutchTemp, data->mClutchWear,
         data->mClutchOverheated, data->mClutchSlipping);

  printf("\nSession:\n");
  printf("  YellowFlagState: %d (%s)\n", data->mYellowFlagState,
         yellow_flag_state_names[data->mYellowFlagState + 1] ? yellow_flag_state_names[data->mYellowFlagState + 1]
                                                             : "UNKNOWN");
  printf("  SessionIsPrivate: %d\n", data->mSessionIsPrivate);
  printf("  LaunchStage: %d (%s)\n", data->mLaunchStage,
         launch_stage_names[data->mLaunchStage + 1] ? launch_stage_names[data->mLaunchStage + 1] : "UNKNOWN");
}
