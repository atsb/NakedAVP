/*----------------Patrick 15/11/96-------------------
 Source for NPC generator behavior
 ----------------------------------------------------*/
#include "3dc.h"
#include "inline.h"
#include "module.h"
#include "stratdef.h"
#include "gamedef.h"
#include "bh_types.h"

#include "bh_alien.h"
#include "bh_marin.h"
#include "pfarlocs.h"
#include "bh_gener.h"
#include "pvisible.h"
#include "pheromon.h"
#include "bh_far.h"
#include "pldghost.h"

#include "load_shp.h" 
#define UseLocalAssert Yes
#include "ourasert.h"

#include "huddefs.h"
#include "showcmds.h"

/* externs for this file */
extern int NormalFrameTime;
extern char *ModuleCurrVisArray;

extern void CreateMarineDynamic(STRATEGYBLOCK* Generator,MARINE_NPC_WEAPONS weapon_for_marine);

/* globals for this file */
HIVE_DATA NPCHive;

//generator details that are initialised with in setup_generators in the rif load
HIVELEVELPARAMS LoadedHiveData; 
static int TypeOfNPCGenerated;

/* prototypes for this file */
static void ResetGeneratorTimer(GENERATOR_BLOCK *genBlock);
static void ResetHiveStateTime(void);
int NumNPCsFromThisGenerator(STRATEGYBLOCK* gen_sbptr);

int SlackTotal;
int SlackSize;
int ShowSlack=0;

int NearAliens;
int Alt_NearAliens;
int FarAliens;
int Alt_FarAliens;
int ShowHiveState=0;

/* for testing */
#define logGenData	0  					
#if logGenData
FILE *logFile;
#endif


/*
Stuff for adjusting difficulty level according to player's performance
*/
static void GeneratorBalance_Init();
static void GeneratorBalance_PerFrameMaintenance();
static int GeneratorBalance_GlobalLimit();
static int GeneratorBalance_LocalLimit(int normal_limit);

BOOL UseGeneratorBalance = FALSE;
struct
{

	int Timer;

	int AIScore;
	int PlayerScore;

	int PlayerValue;

	int RateMultiplier;
	int MaxAIShift;

	int Counter;

	int MaxOwnSettingNpc;
}GeneratorBalance;

void ZapSlack(void) {
	
	SlackTotal=0;
	SlackSize=0;

}

/*----------------Patrick 15/11/96-------------------
 Initialise a generator.
 ----------------------------------------------------*/
void InitGenerator(void *bhdata, STRATEGYBLOCK *sbPtr)
{
 	
	GENERATOR_BLOCK *toolsData = (GENERATOR_BLOCK *)bhdata;
 	GENERATOR_BLOCK *genBlock = (GENERATOR_BLOCK *)AllocateMem(sizeof(GENERATOR_BLOCK));
	if (!genBlock)
	{
		memoryInitialisationFailure = 1;
		return;
	}

	*genBlock=*toolsData;

 	genBlock->Timer = 0;	   
	genBlock->RateIncreaseTimer=60*ONE_FIXED;
	if(genBlock->GenerationRate<=0)
		genBlock->GenerationRate=1;

 	sbPtr->SBdataptr = (void *)genBlock;
 	sbPtr->maintainVisibility = 0;
 	sbPtr->containingModule = NULL;


  	sbPtr->shapeIndex=0; //shape index not relevant when using hierarchical	models

	if(UseGeneratorBalance && genBlock->use_own_max_npc)
	{
		GeneratorBalance.MaxOwnSettingNpc+=genBlock->MaxGenNPCs;					
	}
}


/*----------------Patrick 22/1/97-------------------
  Generator Behaviour function.
 ----------------------------------------------------*/
void GeneratorBehaviour(STRATEGYBLOCK *sbPtr)
{
	GENERATOR_BLOCK *genBlock = (GENERATOR_BLOCK *)sbPtr->SBdataptr;
	LOCALASSERT(genBlock);
		
	/* don't do this for a net game */
//	textprint("GenBeh\n");
	if(AvP.Network != I_No_Network && AvP.NetworkAIServer==0) return;
//	textprint("GenBeh ok\n");

	/* if our number of generators/minute is not > 0, the generator system is
	effectively turned off... */
	if(!(NPCHive.generatorNPCsPerMinute>0)) return;
	
	/*see whether this generator is active*/
	if(!genBlock->Active)return;
		 
	/*if generator is using its own rate values , check for rate increase */
	if(genBlock->use_own_rate_values)
	{
		genBlock->RateIncreaseTimer-=NormalFrameTime;
		if(genBlock->RateIncreaseTimer<0)
		{
			genBlock->RateIncreaseTimer=ONE_FIXED*60;
			genBlock->GenerationRate+=genBlock->GenerationRateIncrease;
			genBlock->GenerationRate=min(genBlock->GenerationRate,GENSPERMINUTE_MAX*100);
			genBlock->GenerationRate=max(genBlock->GenerationRate,GENSPERMINUTE_MIN*100);
		}
	}
		
	/* check the timer */	
	if(UseGeneratorBalance && AvP.Network != I_No_Network)
	{
		genBlock->Timer -= MUL_FIXED(NormalFrameTime,GeneratorBalance.RateMultiplier);	
	}
	else
	{
		genBlock->Timer -= NormalFrameTime;	
	}
	if(genBlock->Timer > 0) return;
	
	/* reset the timer */
	ResetGeneratorTimer(genBlock);

	/* at this point, we have reset the timer, and are set to 
	generate a new npc: however, some things can still 
	prevent this: eg if the module is visible, or there are too 
	many generator npcs in the environment */
	
	/* if generator is visible, do not create a new NPC */
	if(ModuleCurrVisArray[(sbPtr->containingModule->m_index)]) 
	{	
		#if logGenData
		{
			logFile = fopen("D:/PATRICK/GENLOG.TXT","a");
			fprintf(logFile, "generator: I am visible \n \n");		 		
 			fclose(logFile);
		}	
		#endif
		return;
	}

	/*If in a network game , must also make sure the module isn't visible by other players*/
	if(AvP.Network!=I_No_Network)
	{
		/* go through the strategy blocks looking for players*/
		int sbIndex;
		for(sbIndex=0;sbIndex<NumActiveStBlocks;sbIndex++)
		{
			STRATEGYBLOCK *playerSbPtr = ActiveStBlockList[sbIndex];
			NETGHOSTDATABLOCK *ghostData;
			if(playerSbPtr->I_SBtype!=I_BehaviourNetGhost) continue;
			ghostData = (NETGHOSTDATABLOCK *)playerSbPtr->SBdataptr;

			if(ghostData->type==I_BehaviourAlienPlayer ||
			   ghostData->type==I_BehaviourMarinePlayer ||
			   ghostData->type==I_BehaviourPredatorPlayer)
			{
				/*this is another player*/
				if(playerSbPtr->containingModule)
				{
					if(IsModuleVisibleFromModule(playerSbPtr->containingModule,sbPtr->containingModule))
					{
						/*Another player can see this generator's module*/
						return;
					}
				}
			}
		}
	}

	/* if there are too many NPCs in the module, do not create a new one */
	if(PherAi_Buf[(sbPtr->containingModule->m_aimodule->m_index)] >= MAX_GENERATORNPCSPERMODULE)
	{
		#if logGenData
		{
			logFile = fopen("D:/PATRICK/GENLOG.TXT","a");
			fprintf(logFile, "generator: too many aliens in my module \n \n");		 		
 			fclose(logFile);
		}	
		#endif
		return;
	}
	/* if there are too many npcs in the env, do not create a new one */
	if(UseGeneratorBalance && AvP.Network != I_No_Network)
	{
		if(genBlock->use_own_max_npc)
		{
			//check npcs from this generator
			if (NumNPCsFromThisGenerator(sbPtr) >= GeneratorBalance_LocalLimit(genBlock->MaxGenNPCs)) 
				return;

		}
		else
		{
			//check global npc limit
			if (NumGeneratorNPCsInEnv() >= GeneratorBalance_GlobalLimit()) 
				return;
		}
	}
	else
	{
		if(genBlock->use_own_max_npc)
		{
			//check npcs from this generator
			if (NumNPCsFromThisGenerator(sbPtr) >= genBlock->MaxGenNPCs) 
				return;

		}
		else
		{
			//check global npc limit
			if (NumGeneratorNPCsInEnv() >= NPCHive.maxGeneratorNPCs) 
				return;
		}
	}		
	/* ok... create an NPC, then */
	GLOBALASSERT(genBlock->WeightingTotal);
	{
		int random=FastRandom()%genBlock->WeightingTotal;
		if(random<0) random=-random;
		
		//pulse rifle marine
		if(random<genBlock->PulseMarine_Wt)
		{
			CreateMarineDynamic(sbPtr,MNPCW_PulseRifle);
			return;
		}
		random-=genBlock->PulseMarine_Wt;
		
		//pistol marine
		if(random<genBlock->PistolMarine_Wt)
		{
			CreateMarineDynamic(sbPtr,MNPCW_PistolMarine);
			return;
		}
		random-=genBlock->PistolMarine_Wt;
				
		//flamer marine
		if(random<genBlock->FlameMarine_Wt)
		{
			CreateMarineDynamic(sbPtr,MNPCW_Flamethrower);
			return;
		}
		random-=genBlock->FlameMarine_Wt;
	
		//smartgun marine
		if(random<genBlock->SmartMarine_Wt)
		{
			CreateMarineDynamic(sbPtr,MNPCW_Smartgun);
			return;
		}
		random-=genBlock->SmartMarine_Wt;

		//sadar marine
		if(random<genBlock->SadarMarine_Wt)
		{
			CreateMarineDynamic(sbPtr,MNPCW_SADAR);
			return;
		}
		random-=genBlock->SadarMarine_Wt;

		//grenade marine
		if(random<genBlock->GrenadeMarine_Wt)
		{
			CreateMarineDynamic(sbPtr,MNPCW_GrenadeLauncher);
			return;
		}
		random-=genBlock->GrenadeMarine_Wt;


		//minigun marine
		if(random<genBlock->MinigunMarine_Wt)
		{
			CreateMarineDynamic(sbPtr,MNPCW_Minigun);
			return;
		}
		random-=genBlock->MinigunMarine_Wt;

		//shotgun civilian
		if(random<genBlock->ShotgunCiv_Wt)
		{
			CreateMarineDynamic(sbPtr,MNPCW_MShotgun);
			return;
		}
		random-=genBlock->ShotgunCiv_Wt;

		//pistol civilian
		if(random<genBlock->PistolCiv_Wt)
		{
			CreateMarineDynamic(sbPtr,MNPCW_MPistol);
			return;
		}
		random-=genBlock->PistolCiv_Wt;

		//flamer civilian
		if(random<genBlock->FlameCiv_Wt)
		{
			CreateMarineDynamic(sbPtr,MNPCW_MFlamer);
			return;
		}
		random-=genBlock->FlameCiv_Wt;

		//unarmed civilian
		if(random<genBlock->UnarmedCiv_Wt)
		{
			CreateMarineDynamic(sbPtr,MNPCW_MUnarmed);
			return;
		}
		random-=genBlock->UnarmedCiv_Wt;

		//molotov civilian
		if(random<genBlock->MolotovCiv_Wt)
		{
			CreateMarineDynamic(sbPtr,MNPCW_MMolotov);
			return;
		}
		random-=genBlock->MolotovCiv_Wt;

		//alien
		if(random<genBlock->Alien_Wt)
		{
			CreateAlienDynamic(sbPtr,AT_Standard);
			return;
		}
		random-=genBlock->Alien_Wt;
		
		//predator alien
		if(random<genBlock->PredAlien_Wt)
		{
			CreateAlienDynamic(sbPtr,AT_Predalien);
			return;
		}
		random-=genBlock->PredAlien_Wt;

		//praetorian
		if(random<genBlock->Praetorian_Wt)
		{
			CreateAlienDynamic(sbPtr,AT_Praetorian);
			return;
		}
		random-=genBlock->Praetorian_Wt;

		GLOBALASSERT(0=="Failed to select generator badguy");

	}	

}


/*----------------Patrick 21/1/97-------------------
 Initialise the hive
 ----------------------------------------------------*/
void InitHive(void)
{
	extern int NumActiveStBlocks;
	extern STRATEGYBLOCK *ActiveStBlockList[];	
	int sbIndex = 0;
	STRATEGYBLOCK *sbPtr;
		
	GeneratorBalance_Init();

	/* initialise the hive data */
	NPCHive.currentState = HS_Attack;
	NPCHive.numGenerators = 0;

	NPCHive.hiveStateTimer = 0;
	NPCHive.genRateTimer = 0;
	NPCHive.maxGeneratorNPCs = 0;
	NPCHive.generatorNPCsPerMinute = 0;
	NPCHive.deltaGeneratorNPCsPerMinute = 0;

	NPCHive.AliensCanBeGenerated = FALSE;
	NPCHive.PredAliensCanBeGenerated = FALSE;
	NPCHive.PraetoriansCanBeGenerated = FALSE;

	SlackTotal=0;
	SlackSize=0;

	NearAliens=0;
	Alt_NearAliens=0;
	FarAliens=0;
	Alt_FarAliens=0;

	/* don't do any more for a net game */
	/* actually, do - Richard */
//	if(AvP.Network != I_No_Network && AvP.NetworkAIServer==0) return;
//	if(AvP.Network != I_No_Network)	return;

	


	/* set the level parameters */
//	NPCHive.maxGeneratorNPCs = hiveLevelData[AvP.CurrentEnv].maxGeneratorNPCs;
//	NPCHive.generatorNPCsPerMinute = hiveLevelData[AvP.CurrentEnv].generatorNPCsPerMinute;
//	NPCHive.deltaGeneratorNPCsPerMinute = hiveLevelData[AvP.CurrentEnv].deltaGeneratorNPCsPerMinute;
//	NPCHive.genRateTimer = (ONE_FIXED*60);
//	
//	/* validate these parameters */
//	if(NPCHive.maxGeneratorNPCs > MAXGENNPCS_MAX) 
//		NPCHive.maxGeneratorNPCs = MAXGENNPCS_MAX;
//	if(NPCHive.maxGeneratorNPCs < MAXGENNPCS_MIN) 
//		NPCHive.maxGeneratorNPCs = MAXGENNPCS_MIN;
//	if(NPCHive.generatorNPCsPerMinute > GENSPERMINUTE_MAX) 
//		NPCHive.generatorNPCsPerMinute = GENSPERMINUTE_MAX;
//	if(NPCHive.generatorNPCsPerMinute < GENSPERMINUTE_MIN) 
//		NPCHive.generatorNPCsPerMinute = GENSPERMINUTE_MIN;
//	if(NPCHive.deltaGeneratorNPCsPerMinute > INCREASEGENSPERMINUTE_MAX) 
//		NPCHive.deltaGeneratorNPCsPerMinute = INCREASEGENSPERMINUTE_MAX;
//	if(NPCHive.deltaGeneratorNPCsPerMinute < INCREASEGENSPERMINUTE_MIN) 
//		NPCHive.deltaGeneratorNPCsPerMinute = INCREASEGENSPERMINUTE_MIN;
//
//	/* init the hive timer */
//	ResetHiveStateTime();

	/* Now in ActivateHive. */
		  
	/* Some futher generator initialisations: work out what modules the generators are
	in, and how many generators there are.
	*/	
	sbIndex = 0;
	while(sbIndex < NumActiveStBlocks)
	{	
		sbPtr = ActiveStBlockList[sbIndex++];
		if(sbPtr->I_SBtype == I_BehaviourGenerator)
		{
   			GENERATOR_BLOCK *genBlock = (GENERATOR_BLOCK *)sbPtr->SBdataptr;			
			sbPtr->containingModule = ModuleFromPosition(&genBlock->Position, (MODULE *)0);
			LOCALASSERT(sbPtr->containingModule);		
			NPCHive.numGenerators++;			
			/* init generator times to something quite small... 
			so that we get some npcs in the env quickly */
			genBlock->Timer = ONE_FIXED;

			//work out which types of alien can be generated on this level (for multiplayer)
			if(genBlock->Alien_Wt) NPCHive.AliensCanBeGenerated = TRUE;
			if(genBlock->PredAlien_Wt) NPCHive.PredAliensCanBeGenerated = TRUE;
			if(genBlock->Praetorian_Wt) NPCHive.PraetoriansCanBeGenerated = TRUE;
		}
	}
	
	#if logGenData
	{
		logFile = fopen("D:/PATRICK/GENLOG.TXT","w");
		fprintf(logFile, "GENERATOR/HIVE DATA LOG \n \n");
		fprintf(logFile, "num Geners: %d \n",NPCHive.numGenerators);		
		fprintf(logFile, "hive timer: %d \n",NPCHive.hiveStateTimer);		
		fprintf(logFile, "max npcs: %d \n",NPCHive.maxGeneratorNPCs);		
		fprintf(logFile, "npcs per min: %d \n",NPCHive.generatorNPCsPerMinute);		
		fprintf(logFile, "change in npcs per min: %d \n \n",NPCHive.deltaGeneratorNPCsPerMinute);		
 		fclose(logFile);
	}	
	#endif

	ActivateHive();

}

void ActivateHive(void) {

	/* Placed in for Jules's level... CDF 1/12/97, Deadline day! */

	NPCHive.maxGeneratorNPCs=LoadedHiveData.maxGeneratorNPCs;
	NPCHive.generatorNPCsPerMinute=LoadedHiveData.generatorNPCsPerMinute;
	NPCHive.deltaGeneratorNPCsPerMinute=LoadedHiveData.deltaGeneratorNPCsPerMinute;
	NPCHive.genRateTimer=60*ONE_FIXED;

	/* validate these parameters */
	if(NPCHive.maxGeneratorNPCs > MAXGENNPCS_MAX) 
		NPCHive.maxGeneratorNPCs = MAXGENNPCS_MAX;
	if(NPCHive.maxGeneratorNPCs < MAXGENNPCS_MIN) 
		NPCHive.maxGeneratorNPCs = MAXGENNPCS_MIN;
	if(NPCHive.generatorNPCsPerMinute > GENSPERMINUTE_MAX) 
		NPCHive.generatorNPCsPerMinute = GENSPERMINUTE_MAX;
	if(NPCHive.generatorNPCsPerMinute < GENSPERMINUTE_MIN) 
		NPCHive.generatorNPCsPerMinute = GENSPERMINUTE_MIN;
	if(NPCHive.deltaGeneratorNPCsPerMinute > INCREASEGENSPERMINUTE_MAX) 
		NPCHive.deltaGeneratorNPCsPerMinute = INCREASEGENSPERMINUTE_MAX;
	if(NPCHive.deltaGeneratorNPCsPerMinute < INCREASEGENSPERMINUTE_MIN) 
		NPCHive.deltaGeneratorNPCsPerMinute = INCREASEGENSPERMINUTE_MIN;

	/* init the hive timer */
	ResetHiveStateTime();

}

void DeActivateHive(void) {

	NPCHive.genRateTimer = 0;
	NPCHive.maxGeneratorNPCs = 0;
	NPCHive.generatorNPCsPerMinute = 0;
	NPCHive.deltaGeneratorNPCsPerMinute = 0;

	/* validate these parameters */
	if(NPCHive.maxGeneratorNPCs > MAXGENNPCS_MAX) 
		NPCHive.maxGeneratorNPCs = MAXGENNPCS_MAX;
	if(NPCHive.maxGeneratorNPCs < MAXGENNPCS_MIN) 
		NPCHive.maxGeneratorNPCs = MAXGENNPCS_MIN;
	if(NPCHive.generatorNPCsPerMinute > GENSPERMINUTE_MAX) 
		NPCHive.generatorNPCsPerMinute = GENSPERMINUTE_MAX;
	if(NPCHive.generatorNPCsPerMinute < GENSPERMINUTE_MIN) 
		NPCHive.generatorNPCsPerMinute = GENSPERMINUTE_MIN;
	if(NPCHive.deltaGeneratorNPCsPerMinute > INCREASEGENSPERMINUTE_MAX) 
		NPCHive.deltaGeneratorNPCsPerMinute = INCREASEGENSPERMINUTE_MAX;
	if(NPCHive.deltaGeneratorNPCsPerMinute < INCREASEGENSPERMINUTE_MIN) 
		NPCHive.deltaGeneratorNPCsPerMinute = INCREASEGENSPERMINUTE_MIN;

	/* init the hive timer */
	ResetHiveStateTime();

}

/*---------------------Patrick 21/1/97------------------------
 Do hive management:
 generator time is decreased at the start of an attack phase  
 ------------------------------------------------------------*/
void DoHive(void)
{
	/* don't do this for a net game */
	if(AvP.Network != I_No_Network && AvP.NetworkAIServer==0) return;
//	if(AvP.Network != I_No_Network)	return;

	if(AvP.Network != I_No_Network)
	{
		GeneratorBalance_PerFrameMaintenance();	
	}
	
	NearAliens=Alt_NearAliens;
	Alt_NearAliens=0;
	FarAliens=Alt_FarAliens;
	Alt_FarAliens=0;

	/* chack hive state timer */
	NPCHive.hiveStateTimer -= NormalFrameTime;
	if(NPCHive.hiveStateTimer <= 0)
	{
		/* state change */
		if(NPCHive.currentState == HS_Attack)
		{
			#if ULTRAVIOLENCE
			/* Hackery.  An experiment. CDF 2/12/97.  Ha. */
			NPCHive.currentState = HS_Attack;
			#else
			/* switch to regroup */
			NPCHive.currentState = HS_Regroup;
			#endif
		}
		else
		{
			/* switch to attack */
			#if ULTRAVIOLENCE
			#else
			LOCALASSERT(NPCHive.currentState == HS_Regroup);
			#endif
			NPCHive.currentState = HS_Attack;
		}
		ResetHiveStateTime();
	}
	
	/* check gen rate timer */
	NPCHive.genRateTimer -= NormalFrameTime;
	if(NPCHive.genRateTimer <= 0)
	{
		/* increase frequency */
		NPCHive.generatorNPCsPerMinute += NPCHive.deltaGeneratorNPCsPerMinute;
		/* validate this value */
		if(NPCHive.generatorNPCsPerMinute > GENSPERMINUTE_MAX) 
			NPCHive.generatorNPCsPerMinute = GENSPERMINUTE_MAX;				
		if(NPCHive.generatorNPCsPerMinute < GENSPERMINUTE_MIN) 
			NPCHive.generatorNPCsPerMinute = GENSPERMINUTE_MIN;
		
		NPCHive.genRateTimer = (ONE_FIXED*60);
	}

	/* Print hive state. */

	if(NPCHive.currentState == HS_Attack) {
		if (ShowHiveState) {
			PrintDebuggingText("Hive Attacking %d...\n",NPCHive.hiveStateTimer);
		}
	} else {
		if (ShowHiveState) {
			PrintDebuggingText("Hive Retreating %d...\n",NPCHive.hiveStateTimer);
		}
	}
	
	if (ShowHiveState) {
		PrintDebuggingText("Near Aliens = %d\nFar Aliens = %d\n",NearAliens,FarAliens);
	}

	if ((SlackSize)&&(ShowSlack)) {
		int Slack;
		
		Slack=(SlackTotal/SlackSize);
		PrintDebuggingText("Average Slack %d\n",Slack);
	}
}


/*----------------Patrick 22/1/97-------------------
  Timer functions
 ----------------------------------------------------*/
static void ResetGeneratorTimer(GENERATOR_BLOCK *genBlock)
{
	LOCALASSERT(genBlock);
		
	/* shouldn't be doing this for a net game */
//	LOCALASSERT(AvP.Network == I_No_Network);

	if(genBlock->use_own_rate_values)
	{
		LOCALASSERT(genBlock->GenerationRate);
		genBlock->Timer	= (60 * ONE_FIXED *100)/genBlock->GenerationRate;
	}
	else
	{
		/* if we get here, there must be at least one generator, and some kind of generator rate */
		LOCALASSERT(NPCHive.numGenerators>0); 	
		LOCALASSERT(NPCHive.generatorNPCsPerMinute>0);

		/* set the timer */		
		genBlock->Timer	= (((60 * ONE_FIXED)/(NPCHive.generatorNPCsPerMinute))*NPCHive.numGenerators);
	}
	/* randomise +- an eighth */
	{
		int baseTime = genBlock->Timer;
		genBlock->Timer = ((baseTime*7)/8) + (FastRandom()%(baseTime/4)); 
	}
	/* clamp */
	if(genBlock->Timer>GENERATORTIME_MAX) genBlock->Timer=GENERATORTIME_MAX;
	if(genBlock->Timer<GENERATORTIME_MIN) genBlock->Timer=GENERATORTIME_MIN;	

	#if logGenData
	{
		logFile = fopen("D:/PATRICK/GENLOG.TXT","a");
		fprintf(logFile, "Reset Gen Timer \n");		
		fprintf(logFile, "gen timer to: %d seconds \n \n",genBlock->Timer);		 		
 		fclose(logFile);
	}	
	#endif
}

static void ResetHiveStateTime(void)
{
	int baseTime; 

	/* shouldn't be doing this for a net game */
//	LOCALASSERT(AvP.Network == I_No_Network);

	/* set the timer, +- an eighth */	
	baseTime = LoadedHiveData.hiveStateBaseTime;
	NPCHive.hiveStateTimer = ((baseTime*7)/8) + (FastRandom()%(baseTime/4));

	#if logGenData
	{
		logFile = fopen("D:/PATRICK/GENLOG.TXT","a");
		fprintf(logFile, "Reset Hive Timer \n");		
		fprintf(logFile, "hive timer to: %d seconds \n \n",NPCHive.hiveStateTimer);		 		
 		fclose(logFile);
	}	
	#endif
}  					


/* Patrick 11/8/97 ---------------------------------------------------
   A couple of useful functions
   -------------------------------------------------------------------*/
int NumGeneratorNPCsInEnv(void)
{
	extern int NumActiveStBlocks;
	extern STRATEGYBLOCK *ActiveStBlockList[];	
	int sbIndex = 0;
	STRATEGYBLOCK *sbPtr;
	int numOfNPCs = 0;
		
	while(sbIndex < NumActiveStBlocks)
	{	
		sbPtr = ActiveStBlockList[sbIndex++];
		if((sbPtr->I_SBtype == I_BehaviourAlien)||(sbPtr->I_SBtype == I_BehaviourMarine))
		{
			//All placed bad guys will have the last character of the sbname as 0
			//generated badguys shoud have a non-zero last character.
			if(sbPtr->SBname[SB_NAME_LENGTH-1])
			{
				numOfNPCs++;
			}
		}
	}

	#if logGenData
	{
		logFile = fopen("D:/PATRICK/GENLOG.TXT","a");
		fprintf(logFile, "current num gener npcs: %d \n \n",numOfNPCs);		 		
 		fclose(logFile);
	}	
	#endif
	return numOfNPCs;
}
int NumNPCsFromThisGenerator(STRATEGYBLOCK* gen_sbptr)
{
	extern int NumActiveStBlocks;
	extern STRATEGYBLOCK *ActiveStBlockList[];	
	int sbIndex = 0;
	STRATEGYBLOCK *sbPtr;
	int numOfNPCs = 0;
		
	while(sbIndex < NumActiveStBlocks)
	{	
		sbPtr = ActiveStBlockList[sbIndex++];
		switch(sbPtr->I_SBtype)
		{
			case I_BehaviourAlien :
				{
					ALIEN_STATUS_BLOCK* status_block=(ALIEN_STATUS_BLOCK*)sbPtr->SBdataptr;
					GLOBALASSERT(status_block);

					if(status_block->generator_sbptr==gen_sbptr)
					{
						//this alien was produced by this generator
						numOfNPCs++;
					}
				}
				break;

			case I_BehaviourMarine :
				{
					MARINE_STATUS_BLOCK* status_block=(MARINE_STATUS_BLOCK*)sbPtr->SBdataptr;
					GLOBALASSERT(status_block);

					if(status_block->generator_sbptr==gen_sbptr)
					{
						//this marine was produced by this generator
						numOfNPCs++;
					}
				}
				break;
									
			default: ; /* do nothing */
		}
		
	}

	return numOfNPCs;
}


int NumGeneratorNPCsVisible(void)
{
	extern int NumActiveStBlocks;
	extern STRATEGYBLOCK *ActiveStBlockList[];	
	int sbIndex = 0;
	STRATEGYBLOCK *sbPtr;
	int numOfVisNPCs = 0;
		
	while(sbIndex < NumActiveStBlocks)
	{	
		sbPtr = ActiveStBlockList[sbIndex++];
		if((sbPtr->I_SBtype == I_BehaviourAlien)||(sbPtr->I_SBtype == I_BehaviourMarine))
		{	
			if(sbPtr->SBdptr)numOfVisNPCs++;
		}
	}
	#if logGenData
	{
		logFile = fopen("D:/PATRICK/GENLOG.TXT","a");
		fprintf(logFile, "current num visible npcs: %d \n \n",numOfVisNPCs);		 		
 		fclose(logFile);
	}	
	#endif
	return numOfVisNPCs;
}

void ForceAGenerator_Shell(void) {

	NewOnScreenMessage("FORCING...\n");
	ForceAGenerator();
}

void ForceAGenerator(void)
{
	extern int NumActiveStBlocks;
	extern STRATEGYBLOCK *ActiveStBlockList[];	
	int sbIndex = 0;
	STRATEGYBLOCK *sbPtr;
	#if logGenData
	{
		logFile = fopen("D:/PATRICK/GENLOG.TXT","a");
		fprintf(logFile, "forcing a generator... \n");		 		
 		fclose(logFile);
	}	
	#endif
		
	while(sbIndex < NumActiveStBlocks)
	{	
		sbPtr = ActiveStBlockList[sbIndex++];
		if(sbPtr->I_SBtype == I_BehaviourGenerator)
		{	
			GENERATOR_BLOCK *genBlock = (GENERATOR_BLOCK *)sbPtr->SBdataptr;
			LOCALASSERT(genBlock);
		
			if(genBlock->Timer>0) 
			{	
				/* found a generator with timer>0, so set it to zero and return */
				genBlock->Timer = 0;
				#if logGenData
				{
					logFile = fopen("D:/PATRICK/GENLOG.TXT","a");
					fprintf(logFile, "... forced generator ref %d \n \n",sbIndex);		 		
 					fclose(logFile);
				}	
				#endif
				return;
			}
		}
	}
	#if logGenData
	{
		logFile = fopen("D:/PATRICK/GENLOG.TXT","a");
		fprintf(logFile, "... didn't find one to force \n \n");		 		
 		fclose(logFile);
	}	
	#endif

}


void SetHiveParamaters(int enemytype,int max,int genpermin,int deltagenpermin,int time)
{
	LoadedHiveData.maxGeneratorNPCs=max;
	LoadedHiveData.generatorNPCsPerMinute=genpermin;
	LoadedHiveData.deltaGeneratorNPCsPerMinute=deltagenpermin;
	LoadedHiveData.hiveStateBaseTime=time;

	TypeOfNPCGenerated=enemytype;
}


/*--------------------**
** Loading and Saving **
**--------------------*/
#include "savegame.h"
typedef struct generator_save_block
{
	SAVE_BLOCK_STRATEGY_HEADER header;

	int Timer;
	int Active; 
	
	int GenerationRate;		//scaled up by 100
	int RateIncreaseTimer;
}GENERATOR_SAVE_BLOCK;

//defines for load/save macros
#define SAVELOAD_BLOCK block
#define SAVELOAD_BEHAV genBlock


void LoadStrategy_Generator(SAVE_BLOCK_STRATEGY_HEADER* header)
{
	STRATEGYBLOCK* sbPtr;
	GENERATOR_BLOCK *genBlock;
	GENERATOR_SAVE_BLOCK* block = (GENERATOR_SAVE_BLOCK*) header; 
	
	//check the size of the save block
	if(header->size!=sizeof(*block)) return;

	//find the existing strategy block
	sbPtr = FindSBWithName(header->SBname);
	if(!sbPtr) return;

	//make sure the strategy found is of the right type
	if(sbPtr->I_SBtype != I_BehaviourGenerator) return;

	genBlock = (GENERATOR_BLOCK*)sbPtr->SBdataptr;

	//start copying stuff

	COPYELEMENT_LOAD(Timer)
	COPYELEMENT_LOAD(Active)
	COPYELEMENT_LOAD(GenerationRate)
	COPYELEMENT_LOAD(RateIncreaseTimer)
	
}

void SaveStrategy_Generator(STRATEGYBLOCK* sbPtr)
{
	GENERATOR_SAVE_BLOCK *block;
	GENERATOR_BLOCK *genBlock;
	genBlock = (GENERATOR_BLOCK*)sbPtr->SBdataptr;
	

	GET_STRATEGY_SAVE_BLOCK(block,sbPtr);

	//start copying stuff

	COPYELEMENT_SAVE(Timer)
	COPYELEMENT_SAVE(Active)
	COPYELEMENT_SAVE(GenerationRate)
	COPYELEMENT_SAVE(RateIncreaseTimer)

}


/*----------------------------------**
** And now the global hive settings **
**----------------------------------*/


typedef struct hive_save_block
{
	SAVE_BLOCK_HEADER header;

	HIVE_STATE currentState;
	int hiveStateTimer;
	int genRateTimer;
	int generatorNPCsPerMinute;

}HIVE_SAVE_BLOCK;

#undef  SAVELOAD_BLOCK
#undef  SAVELOAD_BEHAV
//defines for load/save macros
#define SAVELOAD_BLOCK block
#define SAVELOAD_BEHAV (&NPCHive)

void LoadHiveSettings(SAVE_BLOCK_HEADER* header)
{
	HIVE_SAVE_BLOCK* block = (HIVE_SAVE_BLOCK*) header; 

	//check the size of the save block
	if(header->size!=sizeof(*block)) return;

	COPYELEMENT_LOAD(currentState)
	COPYELEMENT_LOAD(hiveStateTimer)
	COPYELEMENT_LOAD(genRateTimer)
	COPYELEMENT_LOAD(generatorNPCsPerMinute)
	
}

void SaveHiveSettings()
{
	HIVE_SAVE_BLOCK* block;
	GET_SAVE_BLOCK_POINTER(block);

	//fill in the header
	block->header.type = SaveBlock_GlobalHive;
	block->header.size = sizeof(*block);


	COPYELEMENT_SAVE(currentState)
	COPYELEMENT_SAVE(hiveStateTimer)
	COPYELEMENT_SAVE(genRateTimer)
	COPYELEMENT_SAVE(generatorNPCsPerMinute)
}








int GeneratorBalance_PlayerScoreValue = 0;

#define GENERATOR_BALANCE_DECAY  (ONE_FIXED * .01)
#define GENERATOR_BALANCE_THRESHHOLD (100000)

static void GeneratorBalance_Init()
{
	GeneratorBalance.PlayerValue = GeneratorBalance_PlayerScoreValue*100;
	
	GeneratorBalance.Timer = 0;
	GeneratorBalance.AIScore = 0;
	GeneratorBalance.PlayerScore = 0;

	GeneratorBalance.RateMultiplier = ONE_FIXED;
	GeneratorBalance.MaxAIShift = 0;
	GeneratorBalance.Counter = GENERATOR_BALANCE_THRESHHOLD/2;

	GeneratorBalance.MaxOwnSettingNpc = 0;	

}

void GeneratorBalance_NotePlayerDeath()
{
	if(!UseGeneratorBalance) return;

	GeneratorBalance.PlayerScore += GeneratorBalance.PlayerValue;
}

void GeneratorBalance_NoteAIDeath()
{
	if(!UseGeneratorBalance) return;

	GeneratorBalance.AIScore+=100;
}

static void GeneratorBalance_PerFrameMaintenance()
{
	if(GeneratorBalance.PlayerValue!=GeneratorBalance_PlayerScoreValue*100)
	{
		GeneratorBalance.PlayerValue=GeneratorBalance_PlayerScoreValue*100;
		UseGeneratorBalance = (GeneratorBalance.PlayerValue>0); 			
	}
	
	if(!UseGeneratorBalance) return;


//	PrintDebuggingText("\n\n\n\n\nAI : %d\n",GeneratorBalance.AIScore);
//	PrintDebuggingText("Player : %d\n",GeneratorBalance.PlayerScore);
//	PrintDebuggingText("Counter : %d\n",GeneratorBalance.Counter);
	PrintDebuggingText("\n\n\nAi Limit Shift : %d\n",GeneratorBalance.MaxAIShift);
	

	GeneratorBalance.Timer += NormalFrameTime;
	
	if(GeneratorBalance.Timer > 10 * ONE_FIXED)
	{
		GeneratorBalance.Timer-= 10 * ONE_FIXED;
	  
		if(GeneratorBalance.PlayerScore>0 || GeneratorBalance.AIScore>0)
		{
			
			if(GeneratorBalance.PlayerScore>GeneratorBalance.AIScore)
			{
				//make things easier
				int ratio = DIV_FIXED(GeneratorBalance.PlayerScore+GeneratorBalance.PlayerValue,GeneratorBalance.AIScore+GeneratorBalance.PlayerValue);
			
/*			
				if(ratio > (ONE_FIXED*1.1))
				{
					GeneratorBalance.RateMultiplier = DIV_FIXED(GeneratorBalance.RateMultiplier,ONE_FIXED *1.1);
				}
*/			
				{
					int decrement = ratio - ONE_FIXED;
					if(GeneratorBalance.MaxAIShift < 0)
					{
						decrement /= (-GeneratorBalance.MaxAIShift)+1;
					}
					
					GeneratorBalance.Counter-=decrement;
					if(GeneratorBalance.Counter<0)
					{
						GeneratorBalance.Counter = GENERATOR_BALANCE_THRESHHOLD/2;
						GeneratorBalance.MaxAIShift--;							
						GeneratorBalance.PlayerScore = 	GeneratorBalance.AIScore;

					}

				}
			}
			else
			{
				//make things harder
				int ratio = DIV_FIXED(GeneratorBalance.AIScore+GeneratorBalance.PlayerValue,GeneratorBalance.PlayerScore+GeneratorBalance.PlayerValue);
  /*				
				if(ratio > (ONE_FIXED*1.1))
				{
					GeneratorBalance.RateMultiplier = MUL_FIXED(GeneratorBalance.RateMultiplier,ONE_FIXED *1.1);
				}
*/


				{
					int increment = ratio - ONE_FIXED;
					if(GeneratorBalance.MaxAIShift > 0)
					{
						increment /= GeneratorBalance.MaxAIShift+1;
					}
					
					GeneratorBalance.Counter+=increment;
					if(GeneratorBalance.Counter>GENERATOR_BALANCE_THRESHHOLD)
					{
						GeneratorBalance.Counter = GENERATOR_BALANCE_THRESHHOLD/2;
						GeneratorBalance.MaxAIShift++;							
						GeneratorBalance.AIScore = 	GeneratorBalance.PlayerScore;

					}

				}
			}
	   		GeneratorBalance.AIScore -= MUL_FIXED(GENERATOR_BALANCE_DECAY,GeneratorBalance.AIScore);
			GeneratorBalance.PlayerScore -= MUL_FIXED(GENERATOR_BALANCE_DECAY,GeneratorBalance.PlayerScore);
		}

		if(GeneratorBalance.RateMultiplier > 4*ONE_FIXED) GeneratorBalance.RateMultiplier = 4*ONE_FIXED;
		if(GeneratorBalance.RateMultiplier < ONE_FIXED/4) GeneratorBalance.RateMultiplier = ONE_FIXED/4;



   		
	}
}


static int GeneratorBalance_GlobalLimit()
{
	int limit = NPCHive.maxGeneratorNPCs + GeneratorBalance.MaxAIShift;

	if(limit < 2) limit = 2;
	if(limit > NPCHive.maxGeneratorNPCs +4) limit = NPCHive.maxGeneratorNPCs +4;
	return(limit);
}

static int GeneratorBalance_LocalLimit(int normal_limit)
{
	if(GeneratorBalance.MaxAIShift == 0) return(normal_limit);

	if(GeneratorBalance.MaxAIShift < 0)
	{
		int limit = GeneratorBalance.MaxOwnSettingNpc + GeneratorBalance.MaxAIShift;

		if(limit < 2) limit = 2;

		if(NumGeneratorNPCsInEnv() >= limit) return(0);
		return(normal_limit);
	}
	else
	{
		int shift = min(GeneratorBalance.MaxAIShift,4);
		int limit = GeneratorBalance.MaxOwnSettingNpc + shift;
		int	alien_shortfall = limit - NumGeneratorNPCsInEnv();
		
		return(normal_limit + min(alien_shortfall,shift));
		
	}
}
