// This is opencode generated, and used only for debugging. I didn't check for correctness - it might be missing fields
// or have mistakes
#include <stdio.h>

#include "ams2_telemetry.h"

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
         info->name, info->isActive, info->racePosition, info->worldPosition[0], info->worldPosition[1],
         info->worldPosition[2], info->lapsCompleted, info->currentLap, info->currentLapDistance, info->currentSector);
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
  printf("ProtocolVersion: %u  BuildVersionNumber: %u\n", tele->version, tele->buildVersionNumber);
  printf("GameState: %d (%s) SessionState: %d (%s) RaceState: %d (%s)\n", tele->gameState,
         game_state_names[tele->gameState], tele->sessionState, session_state_names[tele->sessionState],
         tele->raceState, race_state_names[tele->raceState]);

  printf("\nParticipants:\n");
  printf("  ViewedParticipantIndex: %d\n", tele->viewedParticipantIndex);
  for (int i = 0; i < tele->numParticipants; i++) {
    printf("  Participant[%d]: ", i);
    print_participant_info(&tele->participantInfo[i]);
  }

  printf("\nUnfiltered Input: Throttle: %.2f  Brake: %.2f  Steering: %.2f  Clutch: %.2f\n", tele->unfilteredThrottle,
         tele->unfilteredBrake, tele->unfilteredSteering, tele->unfilteredClutch);

  printf("\nCar Name: %.64s (class: %.64s)\n", tele->carName, tele->carClassName);
  printf("Track: %.64s Variation: %.64s (%.2fkm) LapsInEvent: %u\n", tele->trackLocation, tele->trackVariation,
         tele->trackLength, tele->lapsInEvent);

  printf("\nTimings:\n");
  printf("  NumSectors: %d LapInvalidated: %d\n", tele->numSectors, tele->lapInvalidated);
  printf("  LapTime: Best: %.3f Last: %.3f Current: %.3f PersonalFastest: %.3f WorldFastest: %.3f\n", tele->bestLapTime,
         tele->lastLapTime, tele->currentTime, tele->personalFastestLapTime, tele->worldFastestLapTime);
  printf("  SplitTime: %.3f Ahead: %.3f Behind: %.3f EventTimeRemaining: %.3f\n", tele->splitTime, tele->splitTimeAhead,
         tele->splitTimeBehind, tele->eventTimeRemaining);
  printf("  SectorTime: Current: [%.3f, %.3f, %.3f] Fastest: [%.3f, %.3f, %.3f] PersonalFastest: [%.3f, %.3f, %.3f] "
         "WorldFastest: [%.3f, %.3f, %.3f]\n",
         tele->currentSector1Time, tele->currentSector2Time, tele->currentSector3Time, tele->fastestSector1Time,
         tele->fastestSector2Time, tele->fastestSector3Time, tele->personalFastestSector1Time,
         tele->personalFastestSector2Time, tele->personalFastestSector3Time, tele->worldFastestSector1Time,
         tele->worldFastestSector2Time, tele->worldFastestSector3Time);

  printf("\nFlags: HighestFlagColour: %d (%s) Reason: %d\n", tele->highestFlagColour,
         flag_colour_names[tele->highestFlagColour], tele->highestFlagReason);

  printf("\nPit Info: Mode: %d (%s) Schedule: %d (%s)\n", tele->pitMode, pit_mode_names[tele->pitMode],
         tele->pitSchedule, pit_schedule_names[tele->pitSchedule]);

  printf("\nCar State:\n");
  printf("  CarFlags: 0x%08x\n", tele->carFlags);
  printf("  Oil: Temp: %.2fC Pressure: %.2fKPa\n", tele->oilTempCelsius, tele->oilPressureKPa);
  printf("  Water: Temp: %.2fC Pressure: %.2fKPa\n", tele->waterTempCelsius, tele->waterPressureKPa);
  printf("  Fuel: %.2f/%.2f Pressure: %.2fKPa\n", tele->fuelLevel, tele->fuelCapacity, tele->fuelPressureKPa);
  printf("  Speed: %.2f m/s Rpm: %.0f/%.0f\n", tele->speed, tele->rpm, tele->maxRPM);
  printf("  Brake: %.2f Throttle: %.2f Clutch: %.2f Steering: %.2f\n", tele->brake, tele->throttle, tele->clutch,
         tele->steering);
  printf("  Gear: %d/%d\n", tele->gear, tele->numGears);
  printf("  OdometerKM: %.2f\n", tele->odometerKM);
  printf("  AntiLockActive: %d\n", tele->antiLockActive);
  printf("  LastOpponentCollisionIndex: %d Magnitude: %.2f\n", tele->lastOpponentCollisionIndex,
         tele->lastOpponentCollisionMagnitude);
  printf("  Boost: Active: %d Amount: %.2f\n", tele->boostActive, tele->boostAmount);

  printf("\nMotion:\n");
  print_float_array("Orientation", tele->orientation, VEC_MAX);
  print_float_array("LocalVelocity", tele->localVelocity, VEC_MAX);
  print_float_array("WorldVelocity", tele->worldVelocity, VEC_MAX);
  print_float_array("AngularVelocity", tele->angularVelocity, VEC_MAX);
  print_float_array("LocalAcceleration", tele->localAcceleration, VEC_MAX);
  print_float_array("WorldAcceleration", tele->worldAcceleration, VEC_MAX);
  print_float_array("ExtentsCentre", tele->extentsCentre, VEC_MAX);

  printf("\nWheels / Tyres:\n");
  print_tyre_flags_array("TyreFlags", tele->tyreFlags);
  print_terrain_array("TyreTerrain", tele->tyreTerrain);
  print_tyre_array("TyreY", tele->tyreY);
  print_tyre_array("TyreRPS", tele->tyreRPS);
  print_tyre_array("TyreSlipSpeed", tele->tyreSlipSpeed);
  print_tyre_array("TyreTemp", tele->tyreTemp);
  print_tyre_array("TyreGrip", tele->tyreGrip);
  print_tyre_array("TyreHeightAboveGround", tele->tyreHeightAboveGround);
  print_tyre_array("TyreLateralStiffness", tele->tyreLateralStiffness);
  print_tyre_array("TyreWear", tele->tyreWear);
  print_tyre_array("BrakeDamage", tele->brakeDamage);
  print_tyre_array("SuspensionDamage", tele->suspensionDamage);
  print_tyre_array("BrakeTempCelsius", tele->brakeTempCelsius);
  print_tyre_array("TyreTreadTemp", tele->tyreTreadTemp);
  print_tyre_array("TyreLayerTemp", tele->tyreLayerTemp);
  print_tyre_array("TyreCarcassTemp", tele->tyreCarcassTemp);
  print_tyre_array("TyreRimTemp", tele->tyreRimTemp);
  print_tyre_array("TyreInternalAirTemp", tele->tyreInternalAirTemp);

  printf("\nCar Damage: CrashState: %d (%s) AeroDamage: %.2f EngineDamage: %.2f\n", tele->crashState,
         crash_damage_names[tele->crashState], tele->aeroDamage, tele->engineDamage);

  printf("\nWeather:\n");
  printf("  Temperature: Ambient: %.2fC Track: %.2fC\n", tele->ambientTemperature, tele->trackTemperature);
  printf("  RainDensity: %.2f\n", tele->rainDensity);
  printf("  Wind: Speed: %.2f Direction: [%.2f, %.2f]\n", tele->windSpeed, tele->windDirectionX, tele->windDirectionY);
  printf("  CloudBrightness: %.2f\n", tele->cloudBrightness);

  printf("\nSequenceNumber: %u\n", tele->sequenceNumber);

  printf("\nAdditional Car Variables:\n");
  print_tyre_array("WheelLocalPositionY", tele->wheelLocalPositionY);
  print_tyre_array("SuspensionTravel", tele->suspensionTravel);
  print_tyre_array("SuspensionVelocity", tele->suspensionVelocity);
  print_tyre_array("AirPressure", tele->airPressure);
  printf("  EngineSpeed: %.2f rad/s EngineTorque: %.2f Nm\n", tele->engineSpeed, tele->engineTorque);
  printf("  Wings: [%.2f, %.2f]\n", tele->wings[0], tele->wings[1]);
  printf("  HandBrake: %.2f\n", tele->handBrake);

  printf("\nRace Variables:\n");
  printf("  EnforcedPitStopLap: %d\n", tele->enforcedPitStopLap);
  printf("  TranslatedTrack: Location: %.64s Variation: %.64s\n", tele->translatedTrackLocation,
         tele->translatedTrackVariation);
  printf("  BrakeBias: %.2f TurboBoostPressure: %.2f\n", tele->brakeBias, tele->turboBoostPressure);
  printf("  SnowDensity: %.2f\n", tele->snowDensity);
  printf("  Session: Duration: %.2fmin AdditionalLaps: %d\n", tele->sessionDuration, tele->sessionAdditionalLaps);

  printf("\nTyres:\n");
  print_tyre_array("TempLeft", tele->tyreTempLeft);
  print_tyre_array("TempCenter", tele->tyreTempCenter);
  print_tyre_array("TempRight", tele->tyreTempRight);
  print_tyre_array("RideHeight", tele->rideHeight);

  printf("\nDRSState: 0x%08x\n", tele->drsState);

  printf("\nInput: JoyPad0: 0x%08x DPad: 0x%08x AntiLockSetting: %d TractionControlSetting: %d\n", tele->joyPad0,
         tele->dPad, tele->antiLockSetting, tele->tractionControlSetting);

  printf("\nERS: DeploymentMode: %d (%s) AutoModeEnabled: %d\n", tele->ersDeploymentMode,
         ers_deployment_names[tele->ersDeploymentMode], tele->ersAutoModeEnabled);

  printf("\nClutch: Temp: %.2fK Wear: %.2f Overheated: %d Slipping: %d\n", tele->clutchTemp, tele->clutchWear,
         tele->clutchOverheated, tele->clutchSlipping);

  printf("\nSession: YellowFlagState: %d (%s) SessionIsPrivate: %d LaunchStage: %d (%s)\n", tele->yellowFlagState,
         yellow_flag_state_names[tele->yellowFlagState + 1], tele->sessionIsPrivate, tele->launchStage,
         launch_stage_names[tele->launchStage + 1]);
}
