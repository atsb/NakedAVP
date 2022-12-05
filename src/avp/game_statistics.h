#ifndef _game_stats_h_
#define _game_stats_h_ 1

#ifdef __cplusplus
extern "C" {
#endif

/* KJL 99/2/12 - Statistics of current game */
#include "equipmnt.h"

enum STATS_VICTIM_ID
{
	STATS_VICTIM_FACEHUGGER,
	STATS_VICTIM_XENOMORPH,
	STATS_VICTIM_PRAETORIAN,
	STATS_VICTIM_QUEEN,
	STATS_VICTIM_XENOBORG,
	STATS_VICTIM_PREDALIEN,
	STATS_VICTIM_PREDATOR,
	STATS_VICTIM_MARINE,
	STATS_VICTIM_CIVILIAN,
	STATS_VICTIM_ANDROID,
	STATS_VICTIM_MAXIMUM
};

typedef struct
{
	int Killed[STATS_VICTIM_MAXIMUM];
	int Decapitated[STATS_VICTIM_MAXIMUM];
	int Trophies[STATS_VICTIM_MAXIMUM];
	int LiveHeadBites[STATS_VICTIM_MAXIMUM];
	int DeadHeadBites[STATS_VICTIM_MAXIMUM];
	unsigned int WeaponTimes[MAX_NO_OF_WEAPON_SLOTS];
	unsigned int ShotsFired[MAX_NO_OF_WEAPON_SLOTS];
	unsigned int ShotsHit[MAX_NO_OF_WEAPON_SLOTS];
	unsigned int VisionModeTimes[NUMBER_OF_VISION_MODES];
	int Spotted;
	/* Cloak time... */
	unsigned int Cloak_ElapsedSeconds;
	unsigned int Cloak_ElapsedMinutes;
	unsigned int Cloak_ElapsedHours;
	unsigned int FieldChargeUsed;
	/* Damage... */
	unsigned int HealthDamage;
	unsigned int ArmourDamage;
	/* Speed? */
	unsigned int IntegralSpeed;
} AvP_GameStats;

typedef struct
{
	int Killed[STATS_VICTIM_MAXIMUM];
	int Decapitated[STATS_VICTIM_MAXIMUM];

//	union {
		int TrophiesOrLiveHeadBites[STATS_VICTIM_MAXIMUM];
//		int Trophies[STATS_VICTIM_MAXIMUM];
//		int LiveHeadBites[STATS_VICTIM_MAXIMUM];
//	};

	int DeadHeadBites[STATS_VICTIM_MAXIMUM];
	int ShotsFired;
	int Accuracy;
	int Spotted;
	/* Total time... */
	unsigned int Total_ElapsedSeconds;
	unsigned int Total_ElapsedMinutes;
	int Total_ElapsedHours;
	/* Cloak time... */
	unsigned int Cloak_ElapsedSeconds;
	unsigned int Cloak_ElapsedMinutes;
	int Cloak_ElapsedHours;
	/* Damage... */
	int HealthDamage;
	int ArmourDamage;
	/* Some day the predator might have armour. */

	/* Speed in mm/s */
	int Speed;
	/* For a predator. */
	int FieldChargeUsed;

	int HeadShotPercentage;

	unsigned char Padding[40];
} AvP_GameStats_Stored;

extern AvP_GameStats_Stored DefaultLevelGameStats;
extern AvP_GameStats CurrentGameStatistics;
extern void CurrentGameStats_SpeedSample(unsigned int speed,unsigned int time);
extern void CurrentGameStats_DamageTaken(unsigned int health,unsigned int armour);
extern void CurrentGameStats_ChargeUsed(unsigned int charge);
extern void CurrentGameStats_VisionMode(enum VISION_MODE_ID mode);
extern void CurrentGameStats_CloakOn(void);
extern void CurrentGameStats_WeaponFired(enum WEAPON_SLOT slot,unsigned int rounds);
extern void CurrentGameStats_WeaponHit(enum WEAPON_SLOT slot,unsigned int rounds);
extern void CurrentGameStats_UsingWeapon(enum WEAPON_SLOT slot);
extern void CurrentGameStats_Spotted(void);
extern void CurrentGameStats_CreatureKilled(STRATEGYBLOCK *sbPtr,SECTION_DATA *sectionDataPtr);
extern void CurrentGameStats_TrophyCollected(STRATEGYBLOCK *sbPtr);
extern void CurrentGameStats_HeadBitten(STRATEGYBLOCK *sbPtr);
extern void InitialiseCurrentGameStatistics(void);
extern void CurrentGameStats_Initialise(void);
extern void DoFailedLevelStatisticsScreen(void);
extern void DoStatisticsScreen(int completed_level);

#ifdef __cplusplus
}
#endif

#endif
