/*----------------Patrick 15/11/96-------------------
 Header for NPC generators
 ----------------------------------------------------*/

#ifndef _bhgenerator_h_
	#define _bhgenerator_h_ 1

	#ifdef __cplusplus
	extern "C" {
	#endif

	
	/*----------------Patrick 15/11/96-------------------
 	Generator behaviour data block. NB generators
	have their own module ptr, as they are not included
	in the visibility system.
 	----------------------------------------------------*/
	typedef struct generatorblock
	{
		int PulseMarine_Wt;
		int FlameMarine_Wt;
		int SmartMarine_Wt;
		int SadarMarine_Wt;
		int GrenadeMarine_Wt;
		int MinigunMarine_Wt;
		int ShotgunCiv_Wt;
		int PistolCiv_Wt;
		int FlameCiv_Wt;
		int UnarmedCiv_Wt;
		int MolotovCiv_Wt;
		
		int Alien_Wt;
		int PredAlien_Wt;
		int Praetorian_Wt;
		
		int PistolMarine_Wt;
		
		int WeightingTotal;
				
		/* Pathfinder parameters */
		/* If these values aren't -1 , then the generator produces path following creatures*/
		int path;
		int stepnumber;
		
		
		struct vectorch Position;
		int Timer;
		/* generator can be switched on and off via the request state function */
		int Active; 

		int GenerationRate;		//scaled up by 100
		int GenerationRateIncrease;  //scaled up by 100
		int RateIncreaseTimer;
		int MaxGenNPCs;//limit for this generator

		/*if use_own_rate_values is false then the generator will use the global 
		generation rates*/
		unsigned int use_own_rate_values :1;
		unsigned int use_own_max_npc :1;
	} GENERATOR_BLOCK;
	

	/*----------------Patrick 22/1/97-------------------
	Hive enums and data structure.
	Note that only one hive exists in the game.
 	----------------------------------------------------*/
	typedef enum hive_state
	{ 
		HS_Attack,
		HS_Regroup,
			
	}HIVE_STATE;

	typedef struct hive_data
	{ 
		HIVE_STATE currentState;
		int numGenerators;
		int hiveStateTimer;
		int genRateTimer;
		int maxGeneratorNPCs;
		int generatorNPCsPerMinute;
		int deltaGeneratorNPCsPerMinute;

		BOOL AliensCanBeGenerated;
		BOOL PredAliensCanBeGenerated;
		BOOL PraetoriansCanBeGenerated;

	} HIVE_DATA;

	typedef struct hivelevelparams
	{ 
		int maxGeneratorNPCs;
		int generatorNPCsPerMinute;
		int deltaGeneratorNPCsPerMinute;
		int hiveStateBaseTime;
	} HIVELEVELPARAMS;

	extern int ShowSlack;
	/* prototypes */
	extern void InitGenerator(void *posn, STRATEGYBLOCK *sbPtr);
	extern void GeneratorBehaviour(STRATEGYBLOCK *sbPtr);
	extern void InitHive(void);
	extern void DoHive(void);
	extern int NumGeneratorNPCsInEnv(void);
	extern int NumGeneratorNPCsVisible(void);
	extern void ForceAGenerator(void);
	extern void ActivateHive(void);

	/* defines */
	#define DEFAULTHIVESTATETIME		(60*ONE_FIXED)

	#define MAXGENNPCS_MAX				(255)
	#define MAXGENNPCS_MIN				(0)
	#define GENSPERMINUTE_MAX			(255)
	#define GENSPERMINUTE_MIN			(0)
	#define INCREASEGENSPERMINUTE_MAX	(255)
	#define INCREASEGENSPERMINUTE_MIN	(0)

	#define GENERATORTIME_MAX			(120*ONE_FIXED)
	#define GENERATORTIME_MIN			(3*ONE_FIXED)

	/* globals */
	extern HIVE_DATA NPCHive;
	
	#ifdef __cplusplus
	}
	#endif

extern void GeneratorBalance_NoteAIDeath();
extern void GeneratorBalance_NotePlayerDeath();
#endif
