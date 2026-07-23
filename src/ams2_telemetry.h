#pragma once
// Based on
// https://github.com/RangeyRover/Automobilista-2-Auto-Director/blob/main/shared_memory_struct.py and
// https://codedocs.xyz/Spacefreak18/simapi/pcars2data_8h_source.html
#include <stdint.h>

#define SHARED_MEMORY_VERSION 14
#define STRING_LENGTH_MAX 64
#define STORED_PARTICIPANTS_MAX 64
#define TYRE_COMPOUND_NAME_LENGTH_MAX 40

// Tyres
enum {
  TYRE_FRONT_LEFT = 0,
  TYRE_FRONT_RIGHT,
  TYRE_REAR_LEFT,
  TYRE_REAR_RIGHT,
  //--------------
  TYRE_MAX
};

// Vector
enum {
  VEC_X = 0,
  VEC_Y,
  VEC_Z,
  //-------------
  VEC_MAX
};

typedef enum : int32_t {
  GAME_EXITED = 0,
  GAME_FRONT_END,
  GAME_INGAME_PLAYING,
  GAME_INGAME_PAUSED,
  GAME_INGAME_INMENU_TIME_TICKING,
  GAME_INGAME_RESTARTING,
  GAME_INGAME_REPLAY,
  GAME_FRONT_END_REPLAY,
  //-------------
  GAME_MAX
} GameState;

typedef enum : int32_t {
  SESSION_INVALID = 0,
  SESSION_PRACTICE,
  SESSION_TEST,
  SESSION_QUALIFY,
  SESSION_FORMATION_LAP,
  SESSION_RACE,
  SESSION_TIME_ATTACK,
  //-------------
  SESSION_MAX
} SessionState;

typedef enum : int32_t {
  RACESTATE_INVALID,
  RACESTATE_NOT_STARTED,
  RACESTATE_RACING,
  RACESTATE_FINISHED,
  RACESTATE_DISQUALIFIED,
  RACESTATE_RETIRED,
  RACESTATE_DNF,
  //-------------
  RACESTATE_MAX
} RaceState;

typedef enum : int32_t {
  FLAG_COLOUR_NONE = 0,            // Not used for actual flags, only for some query functions
  FLAG_COLOUR_GREEN,               // End of danger zone, or race started
  FLAG_COLOUR_BLUE,                // Faster car wants to overtake the participant
  FLAG_COLOUR_WHITE_SLOW_CAR,      // Slow car in area
  FLAG_COLOUR_WHITE_FINAL_LAP,     // Final Lap
  FLAG_COLOUR_RED,                 // Huge collisions where one or more cars become wrecked and block the track
  FLAG_COLOUR_YELLOW,              // Danger on the racing surface itself
  FLAG_COLOUR_DOUBLE_YELLOW,       // Danger that wholly or partly blocks the racing surface
  FLAG_COLOUR_BLACK_AND_WHITE,     // Unsportsmanlike conduct
  FLAG_COLOUR_BLACK_ORANGE_CIRCLE, // Mechanical Failure
  FLAG_COLOUR_BLACK,               // Participant disqualified
  FLAG_COLOUR_CHEQUERED,           // Chequered flag
  //-------------
  FLAG_COLOUR_MAX
} FlagColour;

typedef enum : int32_t {
  FLAG_REASON_NONE = 0,
  FLAG_REASON_SOLO_CRASH,
  FLAG_REASON_VEHICLE_CRASH,
  FLAG_REASON_VEHICLE_OBSTRUCTION,
  //-------------
  FLAG_REASON_MAX
} FlagReason;

typedef enum : int32_t {
  PIT_MODE_NONE = 0,
  PIT_MODE_DRIVING_INTO_PITS,
  PIT_MODE_IN_PIT,
  PIT_MODE_DRIVING_OUT_OF_PITS,
  PIT_MODE_IN_GARAGE,
  PIT_MODE_DRIVING_OUT_OF_GARAGE,
  //-------------
  PIT_MODE_MAX
} PitMode;

typedef enum : int32_t {
  PIT_SCHEDULE_NONE = 0,           // Nothing scheduled
  PIT_SCHEDULE_PLAYER_REQUESTED,   // Used for standard pit sequence - requested by player
  PIT_SCHEDULE_ENGINEER_REQUESTED, // Used for standard pit sequence - requested by engineer
  PIT_SCHEDULE_DAMAGE_REQUESTED,   // Used for standard pit sequence - requested by engineer for damage
  PIT_SCHEDULE_MANDATORY,     // Used for standard pit sequence - requested by engineer from career enforced lap number
  PIT_SCHEDULE_DRIVE_THROUGH, // Used for drive-through penalty
  PIT_SCHEDULE_STOP_GO,       // Used for stop-go penalty
  PIT_SCHEDULE_PITSPOT_OCCUPIED, // Used for drive-through when pitspot is occupied
  //-------------
  PIT_SCHEDULE_MAX
} PitSchedule;

typedef enum : int32_t {
  CAR_HEADLIGHT = (1 << 0),
  CAR_ENGINE_ACTIVE = (1 << 1),
  CAR_ENGINE_WARNING = (1 << 2),
  CAR_SPEED_LIMITER = (1 << 3),
  CAR_ABS = (1 << 4),
  CAR_HANDBRAKE = (1 << 5),
  CAR_TCS = (1 << 6),
  CAR_SCS = (1 << 7),
} CarFlags;

typedef enum : int32_t {
  TYRE_ATTACHED = (1 << 0),
  TYRE_INFLATED = (1 << 1),
  TYRE_IS_ON_GROUND = (1 << 2),
} TyreFlags;

typedef enum : int32_t {
  TERRAIN_ROAD = 0,
  TERRAIN_LOW_GRIP_ROAD,
  TERRAIN_BUMPY_ROAD1,
  TERRAIN_BUMPY_ROAD2,
  TERRAIN_BUMPY_ROAD3,
  TERRAIN_MARBLES,
  TERRAIN_GRASSY_BERMS,
  TERRAIN_GRASS,
  TERRAIN_GRAVEL,
  TERRAIN_BUMPY_GRAVEL,
  TERRAIN_RUMBLE_STRIPS,
  TERRAIN_DRAINS,
  TERRAIN_TYREWALLS,
  TERRAIN_CEMENTWALLS,
  TERRAIN_GUARDRAILS,
  TERRAIN_SAND,
  TERRAIN_BUMPY_SAND,
  TERRAIN_DIRT,
  TERRAIN_BUMPY_DIRT,
  TERRAIN_DIRT_ROAD,
  TERRAIN_BUMPY_DIRT_ROAD,
  TERRAIN_PAVEMENT,
  TERRAIN_DIRT_BANK,
  TERRAIN_WOOD,
  TERRAIN_DRY_VERGE,
  TERRAIN_EXIT_RUMBLE_STRIPS,
  TERRAIN_GRASSCRETE,
  TERRAIN_LONG_GRASS,
  TERRAIN_SLOPE_GRASS,
  TERRAIN_COBBLES,
  TERRAIN_SAND_ROAD,
  TERRAIN_BAKED_CLAY,
  TERRAIN_ASTROTURF,
  TERRAIN_SNOWHALF,
  TERRAIN_SNOWFULL,
  TERRAIN_DAMAGED_ROAD1,
  TERRAIN_TRAIN_TRACK_ROAD,
  TERRAIN_BUMPYCOBBLES,
  TERRAIN_ARIES_ONLY,
  TERRAIN_ORION_ONLY,
  TERRAIN_B1RUMBLES,
  TERRAIN_B2RUMBLES,
  TERRAIN_ROUGH_SAND_MEDIUM,
  TERRAIN_ROUGH_SAND_HEAVY,
  TERRAIN_SNOWWALLS,
  TERRAIN_ICE_ROAD,
  TERRAIN_RUNOFF_ROAD,
  TERRAIN_ILLEGAL_STRIP,
  TERRAIN_PAINT_CONCRETE,
  TERRAIN_PAINT_CONCRETE_ILLEGAL,
  TERRAIN_RALLY_TARMAC,
  //-------------
  TERRAIN_MAX
} Terrain;

typedef enum : int32_t {
  CRASH_DAMAGE_NONE = 0,
  CRASH_DAMAGE_OFFTRACK,
  CRASH_DAMAGE_LARGE_PROP,
  CRASH_DAMAGE_SPINNING,
  CRASH_DAMAGE_ROLLING,
  //-------------
  CRASH_MAX
} CrashDamage;

typedef struct {
  bool isActive;
  char name[STRING_LENGTH_MAX]; // [ string ]
  float worldPosition[VEC_MAX]; // [ UNITS = World Space ]
  float currentLapDistance;     // [ UNITS = Metres ] [ RANGE = 0->... ] [ UNSET = 0 ]
  unsigned racePosition;        // [ RANGE = 1->... ] [ UNSET = 0 ]
  unsigned lapsCompleted;       // [ RANGE = 0->... ] [ UNSET = 0 ]
  unsigned currentLap;          // [ RANGE = 0->... ] [ UNSET = 0 ]
  int currentSector;            // [ RANGE = 0->... ] [ UNSET = -1 ]
} ParticipantInfo;

typedef enum : int32_t {
  DRS_INSTALLED = (1 << 0),      // Vehicle has DRS capability
  DRS_ZONE_RULES = (1 << 1),     // 1 if DRS uses F1 style rules
  DRS_AVAILABLE_NEXT = (1 << 2), // detection zone was triggered (only applies to f1 style rules)
  DRS_AVAILABLE_NOW = (1 << 3),  // detection zone triggered and we are in the zone (only applies to f1 style rules)
  DRS_ACTIVE = (1 << 4),         // Wing is in activated state
} DrsState;

typedef enum : int32_t {
  ERS_DEPLOYMENT_MODE_NONE = 0, // The vehicle does not support deployment modes
  ERS_DEPLOYMENT_MODE_OFF,      // Regen only, no deployment
  ERS_DEPLOYMENT_MODE_BUILD,    // Heavy emphasis towards regen
  ERS_DEPLOYMENT_MODE_BALANCED, // Deployment map automatically adjusted to try and maintain target SoC
  ERS_DEPLOYMENT_MODE_ATTACK,   // More aggressive deployment, no target SoC
  ERS_DEPLOYMENT_MODE_QUAL,     // Maximum deployment, no target Soc
} ErsDeploymentMode;

typedef enum : int32_t {
  YFS_INVALID = -1,
  YFS_NONE,         // No yellow flag pending on track
  YFS_PENDING,      // Flag has been thrown, but not yet taken by leader
  YFS_PITS_CLOSED,  // Flag taken by leader, pits not yet open
  YFS_PIT_LEAD_LAP, // Those on the lead lap may pit
  YFS_PITS_OPEN,    // Everyone may pit
  YFS_PITS_OPEN2,   // Everyone may pit
  YFS_LAST_LAP,     // On the last caution lap
  YFS_RESUME,       // About to restart (pace car will duck out)
  YFS_RACE_HALT,    // Safety car will lead field into pits
  //-------------
  YFS_MAXIMUM,
} YellowFlagState;

typedef enum : int32_t {
  LAUNCH_INVALID = -1, // Launch control unavailable
  LAUNCH_OFF = 0,      // Launch control inactive
  LAUNCH_REV,          // Launch control revving to optimum engine speed
  LAUNCH_ON,           // Launch control actively accelerating vehicle
} LaunchStage;

typedef struct {
  // Version Number
  unsigned version;            // [ RANGE = 0->... ]
  unsigned buildVersionNumber; // [ RANGE = 0->... ] [ UNSET = 0 ]

  // Game States
  GameState gameState;
  SessionState sessionState;
  RaceState raceState;

  // Participant Info
  int viewedParticipantIndex; // [ RANGE = 0->STORED_PARTICIPANTS_MAX ] [ UNSET = -1 ]
  int numParticipants;        // [ RANGE = 0->STORED_PARTICIPANTS_MAX ] [ UNSET = -1 ]
  ParticipantInfo participantInfo[STORED_PARTICIPANTS_MAX];

  // Unfiltered Input
  float unfilteredThrottle; // [ RANGE = 0->1 ]
  float unfilteredBrake;    // [ RANGE = 0->1 ]
  float unfilteredSteering; // [ RANGE = -1->1 ]
  float unfilteredClutch;   // [ RANGE = 0->1 ]

  // Vehicle information
  char carName[STRING_LENGTH_MAX];      // [ string ]
  char carClassName[STRING_LENGTH_MAX]; // [ string ]

  // Event information
  unsigned lapsInEvent;                   // [ RANGE = 0->... ] [ UNSET = 0 ]
  char trackLocation[STRING_LENGTH_MAX];  // [ string ] - untranslated shortened English name
  char trackVariation[STRING_LENGTH_MAX]; // [ string ] - untranslated shortened English variation
  float trackLength;                      // [ UNITS = Metres ] [ RANGE = 0->... ] [ UNSET = 0 ]

  // Timings
  int numSectors;                   // [ RANGE = 0->... ] [ UNSET = -1 ]
  bool lapInvalidated;              // [ UNSET = false ]
  float bestLapTime;                // [ UNITS = seconds ] [ RANGE = 0->... ] [ UNSET = -1 ]
  float lastLapTime;                // [ UNITS = seconds ] [ RANGE = 0->... ] [ UNSET = 0 ]
  float currentTime;                // [ UNITS = seconds ] [ RANGE = 0->... ] [ UNSET = 0 ]
  float splitTimeAhead;             // [ UNITS = seconds ] [ RANGE = 0->... ] [ UNSET = -1 ]
  float splitTimeBehind;            // [ UNITS = seconds ] [ RANGE = 0->... ] [ UNSET = -1 ]
  float splitTime;                  // [ UNITS = seconds ] [ RANGE = 0->... ] [ UNSET = 0 ]
  float eventTimeRemaining;         // [ UNITS = milli-seconds ] [ RANGE = 0->... ] [ UNSET = -1 ]
  float personalFastestLapTime;     // [ UNITS = seconds ] [ RANGE = 0->... ] [ UNSET = -1 ]
  float worldFastestLapTime;        // [ UNITS = seconds ] [ RANGE = 0->... ] [ UNSET = -1 ]
  float currentSector1Time;         // [ UNITS = seconds ] [ RANGE = 0->... ] [ UNSET = -1 ]
  float currentSector2Time;         // [ UNITS = seconds ] [ RANGE = 0->... ] [ UNSET = -1 ]
  float currentSector3Time;         // [ UNITS = seconds ] [ RANGE = 0->... ] [ UNSET = -1 ]
  float fastestSector1Time;         // [ UNITS = seconds ] [ RANGE = 0->... ] [ UNSET = -1 ]
  float fastestSector2Time;         // [ UNITS = seconds ] [ RANGE = 0->... ] [ UNSET = -1 ]
  float fastestSector3Time;         // [ UNITS = seconds ] [ RANGE = 0->... ] [ UNSET = -1 ]
  float personalFastestSector1Time; // [ UNITS = seconds ] [ RANGE = 0->... ] [ UNSET = -1 ]
  float personalFastestSector2Time; // [ UNITS = seconds ] [ RANGE = 0->... ] [ UNSET = -1 ]
  float personalFastestSector3Time; // [ UNITS = seconds ] [ RANGE = 0->... ] [ UNSET = -1 ]
  float worldFastestSector1Time;    // [ UNITS = seconds ] [ RANGE = 0->... ] [ UNSET = -1 ]
  float worldFastestSector2Time;    // [ UNITS = seconds ] [ RANGE = 0->... ] [ UNSET = -1 ]
  float worldFastestSector3Time;    // [ UNITS = seconds ] [ RANGE = 0->... ] [ UNSET = -1 ]

  // Flags
  FlagColour highestFlagColour;
  FlagReason highestFlagReason;

  // Pit Info
  PitMode pitMode;
  PitSchedule pitSchedule;

  // Car State
  CarFlags carFlags;
  float oilTempCelsius;                 // [ UNITS = Celsius ] [ UNSET = 0 ]
  float oilPressureKPa;                 // [ UNITS = Kilopascal ] [ RANGE = 0->... ] [ UNSET = 0 ]
  float waterTempCelsius;               // [ UNITS = Celsius ] [ UNSET = 0 ]
  float waterPressureKPa;               // [ UNITS = Kilopascal ] [ RANGE = 0->... ] [ UNSET = 0 ]
  float fuelPressureKPa;                // [ UNITS = Kilopascal ] [ RANGE = 0->... ] [ UNSET = 0 ]
  float fuelLevel;                      // [ RANGE = 0->1 ]
  float fuelCapacity;                   // [ UNITS = Liters ] [ RANGE = 0->1 ] [ UNSET = 0 ]
  float speed;                          // [ UNITS = Metres per-second ] [ RANGE = 0->... ]
  float rpm;                            // [ UNITS = Revolutions per minute ] [ RANGE = 0->... ] [ UNSET = 0 ]
  float maxRPM;                         // [ UNITS = Revolutions per minute ] [ RANGE = 0->... ] [ UNSET = 0 ]
  float brake;                          // [ RANGE = 0->1 ]
  float throttle;                       // [ RANGE = 0->1 ]
  float clutch;                         // [ RANGE = 0->1 ]
  float steering;                       // [ RANGE = -1->1 ]
  int gear;                             // [ RANGE = -1 (Reverse)  0 (Neutral)  1  2 ... ] [ UNSET = 0 ]
  int numGears;                         // [ RANGE = 0->... ] [ UNSET = -1 ]
  float odometerKM;                     // [ RANGE = 0->... ] [ UNSET = -1 ]
  bool antiLockActive;                  // [ UNSET = false ]
  int lastOpponentCollisionIndex;       // [ RANGE = 0->STORED_PARTICIPANTS_MAX ] [ UNSET = -1 ]
  float lastOpponentCollisionMagnitude; // [ RANGE = 0->... ]
  bool boostActive;                     // [ UNSET = false ]
  float boostAmount;                    // [ RANGE = 0->100 ]

  // Motion & Device Related
  float orientation[VEC_MAX];       // [ UNITS = Euler Angles ]
  float localVelocity[VEC_MAX];     // [ UNITS = Metres per-second ]
  float worldVelocity[VEC_MAX];     // [ UNITS = Metres per-second ]
  float angularVelocity[VEC_MAX];   // [ UNITS = Radians per-second ]
  float localAcceleration[VEC_MAX]; // [ UNITS = Metres per-second ]
  float worldAcceleration[VEC_MAX]; // [ UNITS = Metres per-second ]
  float extentsCentre[VEC_MAX];     // [ UNITS = Local Space  X  Y  Z ]

  // Wheels / Tyres
  TyreFlags tyreFlags[TYRE_MAX];
  Terrain tyreTerrain[TYRE_MAX];
  float tyreY[TYRE_MAX];                 // [ UNITS = Local Space  Y ]
  float tyreRPS[TYRE_MAX];               // [ UNITS = Revolutions per second ]
  float tyreSlipSpeed[TYRE_MAX];         // OBSOLETE, kept for backward compatibility only
  float tyreTemp[TYRE_MAX];              // [ UNITS = Celsius ] [ UNSET = 0 ]
  float tyreGrip[TYRE_MAX];              // OBSOLETE, kept for backward compatibility only
  float tyreHeightAboveGround[TYRE_MAX]; // [ UNITS = Local Space  Y ]
  float tyreLateralStiffness[TYRE_MAX];  // OBSOLETE, kept for backward compatibility only
  float tyreWear[TYRE_MAX];              // [ RANGE = 0->1 ]
  float brakeDamage[TYRE_MAX];           // [ RANGE = 0->1 ]
  float suspensionDamage[TYRE_MAX];      // [ RANGE = 0->1 ]
  float brakeTempCelsius[TYRE_MAX];      // [ UNITS = Celsius ]
  float tyreTreadTemp[TYRE_MAX];         // [ UNITS = Kelvin ]
  float tyreLayerTemp[TYRE_MAX];         // [ UNITS = Kelvin ]
  float tyreCarcassTemp[TYRE_MAX];       // [ UNITS = Kelvin ]
  float tyreRimTemp[TYRE_MAX];           // [ UNITS = Kelvin ]
  float tyreInternalAirTemp[TYRE_MAX];   // [ UNITS = Kelvin ]

  // Car Damage
  CrashDamage crashState;
  float aeroDamage;   // [ RANGE = 0->1 ]
  float engineDamage; // [ RANGE = 0->1 ]

  // Weather
  float ambientTemperature; // [ UNITS = Celsius ] [ UNSET = 25 ]
  float trackTemperature;   // [ UNITS = Celsius ] [ UNSET = 30 ]
  float rainDensity;        // [ UNITS = How much rain will fall ] [ RANGE = 0->1 ]
  float windSpeed;          // [ RANGE = 0->100 ] [ UNSET = 2 ]
  float windDirectionX;     // [ UNITS = Normalised Vector X ]
  float windDirectionY;     // [ UNITS = Normalised Vector Y ]
  float cloudBrightness;    // [ RANGE = 0->... ]

  // PCars2 additions start, version 8
  // Sequence Number to help slightly with data integrity reads
  volatile unsigned sequenceNumber; // 0 at the start, incremented at start and end of writing, so odd when Shared
                                    // Memory is being filled, even when the memory is not being touched

  // Additional car variables
  float wheelLocalPositionY[TYRE_MAX]; // [ UNITS = Local Space Y ]
  float suspensionTravel[TYRE_MAX];    // [ UNITS = meters ] [ RANGE 0 =>... ] [ UNSET = 0 ]
  float suspensionVelocity[TYRE_MAX];  // [ UNITS = Rate of change of pushrod deflection ] [ RANGE 0 =>... ]
  float airPressure[TYRE_MAX];         // [ UNITS = PSI ]  [ RANGE 0 =>... ]  [ UNSET = 0 ]
  float engineSpeed;                   // [ UNITS = Rad/s ] [UNSET = 0 ]
  float engineTorque;                  // [ UNITS = Newton Meters] [UNSET = 0 ] [ RANGE = 0->... ]
  float wings[2];                      // [ RANGE = 0->1 ] [UNSET = 0 ]
  float handBrake;                     // [ RANGE = 0->1 ] [UNSET = 0 ]

  // additional race variables
  float currentSector1Times[STORED_PARTICIPANTS_MAX]; // [ UNITS = seconds ] [ RANGE = 0->... ] [ UNSET = -1 ]
  float currentSector2Times[STORED_PARTICIPANTS_MAX]; // [ UNITS = seconds ] [ RANGE = 0->... ] [ UNSET = -1 ]
  float currentSector3Times[STORED_PARTICIPANTS_MAX]; // [ UNITS = seconds ] [ RANGE = 0->... ] [ UNSET = -1 ]
  float fastestSector1Times[STORED_PARTICIPANTS_MAX]; // [ UNITS = seconds ] [ RANGE = 0->... ] [ UNSET = -1 ]
  float fastestSector2Times[STORED_PARTICIPANTS_MAX]; // [ UNITS = seconds ] [ RANGE = 0->... ] [ UNSET = -1 ]
  float fastestSector3Times[STORED_PARTICIPANTS_MAX]; // [ UNITS = seconds ] [ RANGE = 0->... ] [ UNSET = -1 ]
  float fastestLapTimes[STORED_PARTICIPANTS_MAX];     // [ UNITS = seconds ] [ RANGE = 0->... ] [ UNSET = -1 ]
  float lastLapTimes[STORED_PARTICIPANTS_MAX];        // [ UNITS = seconds ] [ RANGE = 0->... ] [ UNSET = -1 ]
  bool lapsInvalidated[STORED_PARTICIPANTS_MAX];      // [ UNITS = boolean per participant ] [ UNSET = false ]
  RaceState raceStates[STORED_PARTICIPANTS_MAX];
  PitMode pitModes[STORED_PARTICIPANTS_MAX];
  float orientations[STORED_PARTICIPANTS_MAX][VEC_MAX];           // [ UNITS = Euler Angles ]
  float speeds[STORED_PARTICIPANTS_MAX];                          // [ UNITS = Metres per-second ] [ RANGE = 0->... ]
  char carNames[STORED_PARTICIPANTS_MAX][STRING_LENGTH_MAX];      // [ string ]
  char carClassNames[STORED_PARTICIPANTS_MAX][STRING_LENGTH_MAX]; // [ string ]

  // additional race variables
  int enforcedPitStopLap; // [ UNITS = in which lap is a mandatory pitstop] [ RANGE = 0->... ] [ UNSET = -1 ]
  char translatedTrackLocation[STRING_LENGTH_MAX];            // [ string ]
  char translatedTrackVariation[STRING_LENGTH_MAX];           // [ string ]
  float brakeBias;                                            // [ RANGE = 0->1... ] [ UNSET = -1 ]
  float turboBoostPressure;                                   // [ RANGE = 0->1... ] [ UNSET = -1 ]
  char tyreCompound[TYRE_MAX][TYRE_COMPOUND_NAME_LENGTH_MAX]; // [ strings  ]
  PitSchedule pitSchedules[STORED_PARTICIPANTS_MAX];
  FlagColour highestFlagColours[STORED_PARTICIPANTS_MAX];
  FlagReason highestFlagReasons[STORED_PARTICIPANTS_MAX];
  unsigned nationalities[STORED_PARTICIPANTS_MAX]; // [ nationality table, SP AND UNSET = 0 ]
  float snowDensity; // [ UNITS = How much snow will fall ] [ RANGE = 0->1 ] non zero only in Winter and Snow seasons

  // AMS2 Additions (v10...)
  // Session info
  float sessionDuration; // [ UNITS = minutes ] [ UNSET = 0 ] scheduled session length. Unset = laps race (lapsInEvent)
  int sessionAdditionalLaps; // The number of additional complete laps lead lap drivers must complete to finish a timed
                             // race after the session duration has elapsed.

  // Tyres
  float tyreTempLeft[TYRE_MAX];   // [ UNITS = Celsius ] [ UNSET = 0 ]
  float tyreTempCenter[TYRE_MAX]; // [ UNITS = Celsius ] [ UNSET = 0 ]
  float tyreTempRight[TYRE_MAX];  // [ UNITS = Celsius ] [ UNSET = 0 ]

  // DRS
  DrsState drsState;

  // Suspension
  float rideHeight[TYRE_MAX]; // [ UNITS = cm ]

  // Input
  unsigned joyPad0; // button mask
  unsigned dPad;    // button mask

  int antiLockSetting;        // [ UNSET = -1 ] Current ABS garage setting. Valid under player control only.
  int tractionControlSetting; // [ UNSET = -1 ] Current ABS garage setting. Valid under player control only.

  // ERS
  ErsDeploymentMode ersDeploymentMode;
  bool ersAutoModeEnabled; // true if deployment mode was selected by auto system. Valid only when ersDeploymentMode >
                           // ERS_DEPLOYMENT_MODE_NONE

  // Clutch State & Damage
  float clutchTemp;      // [ UNITS = Kelvin ] [ UNSET = -273.16 ]
  float clutchWear;      // [ RANGE = 0->1... ]
  bool clutchOverheated; // true if clutch performance is degraded due to overheating
  bool clutchSlipping;   // true if clutch is slipping (can be induced by overheating or wear)

  YellowFlagState yellowFlagState;
  bool sessionIsPrivate; // true if this is a private session where users cannot see or interact with other drivers
                         // (and so would not need positional awareness of them etc)
  LaunchStage launchStage;
} ams2_telemetry;

#ifdef __cplusplus
extern "C" {
#endif

int wait_for_ams2_pid();
const void *wait_for_ams2_telemetry_address(int pid);
bool read_ams2_telemetry(int pid, ams2_telemetry *local_addr, const void *remote_addr);

#ifdef __cplusplus
}
#endif
