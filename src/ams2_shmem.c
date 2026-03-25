// This is opencode generated, and used only for debugging. I didn't check for correctness - it can have mistakes
#include "ams2_shmem.h"
#include <stdbool.h>
#include <stdio.h>

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Waddress-of-packed-member"

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
  printf("  mIsActive: %s\n", info->mIsActive ? "true" : "false");
  printf("  mName: %.64s\n", info->mName);
  printf("  mWorldPosition: [%.2f, %.2f, %.2f]\n", info->mWorldPosition[0], info->mWorldPosition[1],
         info->mWorldPosition[2]);
  printf("  mCurrentLapDistance: %.2f\n", info->mCurrentLapDistance);
  printf("  mRacePosition: %u\n", info->mRacePosition);
  printf("  mLapsCompleted: %u\n", info->mLapsCompleted);
  printf("  mCurrentLap: %u\n", info->mCurrentLap);
  printf("  mCurrentSector: %d\n", info->mCurrentSector);
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
  printf("=== Shared Memory Data ===\n\n");

  printf("Version:\n");
  printf("  mVersion: %u\n", data->mVersion);
  printf("  mBuildVersionNumber: %u\n", data->mBuildVersionNumber);

  printf("\nGame States:\n");
  printf("  mGameState: %d (%s)\n", data->mGameState,
         game_state_names[data->mGameState] ? game_state_names[data->mGameState] : "UNKNOWN");
  printf("  mSessionState: %d (%s)\n", data->mSessionState,
         session_state_names[data->mSessionState] ? session_state_names[data->mSessionState] : "UNKNOWN");
  printf("  mRaceState: %d (%s)\n", data->mRaceState,
         race_state_names[data->mRaceState] ? race_state_names[data->mRaceState] : "UNKNOWN");

  printf("\nParticipant Info:\n");
  printf("  mViewedParticipantIndex: %d\n", data->mViewedParticipantIndex);
  printf("  mNumParticipants: %d\n", data->mNumParticipants);
  for (int i = 0; i < data->mNumParticipants && i < STORED_PARTICIPANTS_MAX; i++) {
    printf("  Participant[%d]:\n", i);
    print_participant_info(&data->mParticipantInfo[i]);
  }

  printf("\nUnfiltered Input:\n");
  printf("  mUnfilteredThrottle: %.2f\n", data->mUnfilteredThrottle);
  printf("  mUnfilteredBrake: %.2f\n", data->mUnfilteredBrake);
  printf("  mUnfilteredSteering: %.2f\n", data->mUnfilteredSteering);
  printf("  mUnfilteredClutch: %.2f\n", data->mUnfilteredClutch);

  printf("\nVehicle Info:\n");
  printf("  mCarName: %.64s\n", data->mCarName);
  printf("  mCarClassName: %.64s\n", data->mCarClassName);

  printf("\nEvent Info:\n");
  printf("  mLapsInEvent: %u\n", data->mLapsInEvent);
  printf("  mTrackLocation: %.64s\n", data->mTrackLocation);
  printf("  mTrackVariation: %.64s\n", data->mTrackVariation);
  printf("  mTrackLength: %.2f\n", data->mTrackLength);

  printf("\nTimings:\n");
  printf("  mNumSectors: %d\n", data->mNumSectors);
  printf("  mLapInvalidated: %s\n", data->mLapInvalidated ? "true" : "false");
  printf("  mBestLapTime: %.3f\n", data->mBestLapTime);
  printf("  mLastLapTime: %.3f\n", data->mLastLapTime);
  printf("  mCurrentTime: %.3f\n", data->mCurrentTime);
  printf("  mSplitTimeAhead: %.3f\n", data->mSplitTimeAhead);
  printf("  mSplitTimeBehind: %.3f\n", data->mSplitTimeBehind);
  printf("  mSplitTime: %.3f\n", data->mSplitTime);
  printf("  mEventTimeRemaining: %.3f\n", data->mEventTimeRemaining);
  printf("  mPersonalFastestLapTime: %.3f\n", data->mPersonalFastestLapTime);
  printf("  mWorldFastestLapTime: %.3f\n", data->mWorldFastestLapTime);
  printf("  mCurrentSector1Time: %.3f\n", data->mCurrentSector1Time);
  printf("  mCurrentSector2Time: %.3f\n", data->mCurrentSector2Time);
  printf("  mCurrentSector3Time: %.3f\n", data->mCurrentSector3Time);
  printf("  mFastestSector1Time: %.3f\n", data->mFastestSector1Time);
  printf("  mFastestSector2Time: %.3f\n", data->mFastestSector2Time);
  printf("  mFastestSector3Time: %.3f\n", data->mFastestSector3Time);
  printf("  mPersonalFastestSector1Time: %.3f\n", data->mPersonalFastestSector1Time);
  printf("  mPersonalFastestSector2Time: %.3f\n", data->mPersonalFastestSector2Time);
  printf("  mPersonalFastestSector3Time: %.3f\n", data->mPersonalFastestSector3Time);
  printf("  mWorldFastestSector1Time: %.3f\n", data->mWorldFastestSector1Time);
  printf("  mWorldFastestSector2Time: %.3f\n", data->mWorldFastestSector2Time);
  printf("  mWorldFastestSector3Time: %.3f\n", data->mWorldFastestSector3Time);

  printf("\nFlags:\n");
  printf("  mHighestFlagColour: %d (%s)\n", data->mHighestFlagColour,
         flag_colour_names[data->mHighestFlagColour] ? flag_colour_names[data->mHighestFlagColour] : "UNKNOWN");
  printf("  mHighestFlagReason: %d\n", data->mHighestFlagReason);

  printf("\nPit Info:\n");
  printf("  mPitMode: %d (%s)\n", data->mPitMode,
         pit_mode_names[data->mPitMode] ? pit_mode_names[data->mPitMode] : "UNKNOWN");
  printf("  mPitSchedule: %d (%s)\n", data->mPitSchedule,
         pit_schedule_names[data->mPitSchedule] ? pit_schedule_names[data->mPitSchedule] : "UNKNOWN");

  printf("\nCar State:\n");
  printf("  mCarFlags: 0x%08x\n", data->mCarFlags);
  printf("  mOilTempCelsius: %.2f\n", data->mOilTempCelsius);
  printf("  mOilPressureKPa: %.2f\n", data->mOilPressureKPa);
  printf("  mWaterTempCelsius: %.2f\n", data->mWaterTempCelsius);
  printf("  mWaterPressureKPa: %.2f\n", data->mWaterPressureKPa);
  printf("  mFuelPressureKPa: %.2f\n", data->mFuelPressureKPa);
  printf("  mFuelLevel: %.2f\n", data->mFuelLevel);
  printf("  mFuelCapacity: %.2f\n", data->mFuelCapacity);
  printf("  mSpeed: %.2f m/s\n", data->mSpeed);
  printf("  mRpm: %.0f\n", data->mRpm);
  printf("  mMaxRPM: %.0f\n", data->mMaxRPM);
  printf("  mBrake: %.2f\n", data->mBrake);
  printf("  mThrottle: %.2f\n", data->mThrottle);
  printf("  mClutch: %.2f\n", data->mClutch);
  printf("  mSteering: %.2f\n", data->mSteering);
  printf("  mGear: %d\n", data->mGear);
  printf("  mNumGears: %d\n", data->mNumGears);
  printf("  mOdometerKM: %.2f\n", data->mOdometerKM);
  printf("  mAntiLockActive: %s\n", data->mAntiLockActive ? "true" : "false");
  printf("  mLastOpponentCollisionIndex: %d\n", data->mLastOpponentCollisionIndex);
  printf("  mLastOpponentCollisionMagnitude: %.2f\n", data->mLastOpponentCollisionMagnitude);
  printf("  mBoostActive: %s\n", data->mBoostActive ? "true" : "false");
  printf("  mBoostAmount: %.2f\n", data->mBoostAmount);

  printf("\nMotion:\n");
  print_float_array("mOrientation", data->mOrientation, VEC_MAX);
  print_float_array("mLocalVelocity", data->mLocalVelocity, VEC_MAX);
  print_float_array("mWorldVelocity", data->mWorldVelocity, VEC_MAX);
  print_float_array("mAngularVelocity", data->mAngularVelocity, VEC_MAX);
  print_float_array("mLocalAcceleration", data->mLocalAcceleration, VEC_MAX);
  print_float_array("mWorldAcceleration", data->mWorldAcceleration, VEC_MAX);
  print_float_array("mExtentsCentre", data->mExtentsCentre, VEC_MAX);

  printf("\nWheels / Tyres:\n");
  print_tyre_flags_array("mTyreFlags", data->mTyreFlags);
  print_terrain_array("mTyreTerrain", data->mTyreTerrain);
  print_tyre_array("mTyreY", data->mTyreY);
  print_tyre_array("mTyreRPS", data->mTyreRPS);
  print_tyre_array("mTyreSlipSpeed", data->mTyreSlipSpeed);
  print_tyre_array("mTyreTemp", data->mTyreTemp);
  print_tyre_array("mTyreGrip", data->mTyreGrip);
  print_tyre_array("mTyreHeightAboveGround", data->mTyreHeightAboveGround);
  print_tyre_array("mTyreLateralStiffness", data->mTyreLateralStiffness);
  print_tyre_array("mTyreWear", data->mTyreWear);
  print_tyre_array("mBrakeDamage", data->mBrakeDamage);
  print_tyre_array("mSuspensionDamage", data->mSuspensionDamage);
  print_tyre_array("mBrakeTempCelsius", data->mBrakeTempCelsius);
  print_tyre_array("mTyreTreadTemp", data->mTyreTreadTemp);
  print_tyre_array("mTyreLayerTemp", data->mTyreLayerTemp);
  print_tyre_array("mTyreCarcassTemp", data->mTyreCarcassTemp);
  print_tyre_array("mTyreRimTemp", data->mTyreRimTemp);
  print_tyre_array("mTyreInternalAirTemp", data->mTyreInternalAirTemp);

  printf("\nCar Damage:\n");
  printf("  mCrashState: %d (%s)\n", data->mCrashState,
         crash_damage_names[data->mCrashState] ? crash_damage_names[data->mCrashState] : "UNKNOWN");
  printf("  mAeroDamage: %.2f\n", data->mAeroDamage);
  printf("  mEngineDamage: %.2f\n", data->mEngineDamage);

  printf("\nWeather:\n");
  printf("  mAmbientTemperature: %.2f C\n", data->mAmbientTemperature);
  printf("  mTrackTemperature: %.2f C\n", data->mTrackTemperature);
  printf("  mRainDensity: %.2f\n", data->mRainDensity);
  printf("  mWindSpeed: %.2f\n", data->mWindSpeed);
  printf("  mWindDirection: [%.2f, %.2f]\n", data->mWindDirectionX, data->mWindDirectionY);
  printf("  mCloudBrightness: %.2f\n", data->mCloudBrightness);

  printf("\nSequence:\n");
  printf("  mSequenceNumber: %u\n", data->mSequenceNumber);

  printf("\nAdditional Car Variables:\n");
  print_tyre_array("mWheelLocalPositionY", data->mWheelLocalPositionY);
  print_tyre_array("mSuspensionTravel", data->mSuspensionTravel);
  print_tyre_array("mSuspensionVelocity", data->mSuspensionVelocity);
  print_tyre_array("mAirPressure", data->mAirPressure);
  printf("  mEngineSpeed: %.2f rad/s\n", data->mEngineSpeed);
  printf("  mEngineTorque: %.2f Nm\n", data->mEngineTorque);
  printf("  mWings: [%.2f, %.2f]\n", data->mWings[0], data->mWings[1]);
  printf("  mHandBrake: %.2f\n", data->mHandBrake);

  printf("\nRace Variables:\n");
  printf("  mEnforcedPitStopLap: %d\n", data->mEnforcedPitStopLap);
  printf("  mTranslatedTrackLocation: %.64s\n", data->mTranslatedTrackLocation);
  printf("  mTranslatedTrackVariation: %.64s\n", data->mTranslatedTrackVariation);
  printf("  mBrakeBias: %.2f\n", data->mBrakeBias);
  printf("  mTurboBoostPressure: %.2f\n", data->mTurboBoostPressure);
  printf("  mSnowDensity: %.2f\n", data->mSnowDensity);
  printf("  mSessionDuration: %.2f min\n", data->mSessionDuration);
  printf("  mSessionAdditionalLaps: %d\n", data->mSessionAdditionalLaps);

  printf("\nTyres (AMS2):\n");
  print_tyre_array("mTyreTempLeft", data->mTyreTempLeft);
  print_tyre_array("mTyreTempCenter", data->mTyreTempCenter);
  print_tyre_array("mTyreTempRight", data->mTyreTempRight);
  print_tyre_array("mRideHeight", data->mRideHeight);

  printf("\nDRS:\n");
  printf("  mDrsState: 0x%08x\n", data->mDrsState);

  printf("\nInput:\n");
  printf("  mJoyPad0: 0x%08x\n", data->mJoyPad0);
  printf("  mDPad: 0x%08x\n", data->mDPad);
  printf("  mAntiLockSetting: %d\n", data->mAntiLockSetting);
  printf("  mTractionControlSetting: %d\n", data->mTractionControlSetting);

  printf("\nERS:\n");
  printf("  mErsDeploymentMode: %d (%s)\n", data->mErsDeploymentMode,
         ers_deployment_names[data->mErsDeploymentMode] ? ers_deployment_names[data->mErsDeploymentMode] : "UNKNOWN");
  printf("  mErsAutoModeEnabled: %s\n", data->mErsAutoModeEnabled ? "true" : "false");

  printf("\nClutch:\n");
  printf("  mClutchTemp: %.2f K\n", data->mClutchTemp);
  printf("  mClutchWear: %.2f\n", data->mClutchWear);
  printf("  mClutchOverheated: %s\n", data->mClutchOverheated ? "true" : "false");
  printf("  mClutchSlipping: %s\n", data->mClutchSlipping ? "true" : "false");

  printf("\nSession:\n");
  printf("  mYellowFlagState: %d (%s)\n", data->mYellowFlagState,
         yellow_flag_state_names[data->mYellowFlagState + 1] ? yellow_flag_state_names[data->mYellowFlagState + 1]
                                                             : "UNKNOWN");
  printf("  mSessionIsPrivate: %s\n", data->mSessionIsPrivate ? "true" : "false");
  printf("  mLaunchStage: %d (%s)\n", data->mLaunchStage,
         launch_stage_names[data->mLaunchStage + 1] ? launch_stage_names[data->mLaunchStage + 1] : "UNKNOWN");

  printf("\n=== End ===\n");
}
#pragma GCC diagnostic pop
