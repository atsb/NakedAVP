/* KJL 99/2/12 - Statistics of current game */
#include "3dc.h"
#include "inline.h"
#include "module.h"
#include "stratdef.h"
#include "gamedef.h"
#include "game_statistics.h"
#include "bh_types.h"
#include "bh_marin.h"
#include "bh_alien.h"
#include "bh_corpse.h"
#include "bh_debri.h"
#include "bh_weap.h"
#include "pldghost.h"
#include "opengl.h"

#define UseLocalAssert Yes
#include "ourasert.h"

#include "avp_envinfo.h"
#include "avp_userprofile.h"

#define FIXED_MINUTE (ONE_FIXED*60)

#define TABPOINT1	(ScreenDescriptorBlock.SDB_Width/10)
#define TABPOINT1A	(ScreenDescriptorBlock.SDB_Width/4)
#define TABPOINT2	(ScreenDescriptorBlock.SDB_Width/2)
#define TABPOINT3	((ScreenDescriptorBlock.SDB_Width/6)+(ScreenDescriptorBlock.SDB_Width/2))
#define TABPOINT4	(((ScreenDescriptorBlock.SDB_Width/6)*2)+(ScreenDescriptorBlock.SDB_Width/2))

extern int DebuggingCommandsActive;
#define NotCheating ((CheatMode_Active==CHEATMODE_NONACTIVE)&&!DebuggingCommandsActive)

AvP_GameStats CurrentGameStatistics;
extern SCREENDESCRIPTORBLOCK ScreenDescriptorBlock;
extern int NormalFrameTime;

extern int MarineEpisodeToPlay;
extern int PredatorEpisodeToPlay;
extern int AlienEpisodeToPlay;
extern char LevelName[];

extern AvP_Level_Target_Desc LevelStatsTargets[I_MaxDifficulties][AVP_ENVIRONMENT_END_OF_LIST];

/* Default structure: */
AvP_GameStats_Stored DefaultLevelGameStats = {
	{0,0,0,0,0,0,0,0,0,0},
	{0,0,0,0,0,0,0,0,0,0},
	{0,0,0,0,0,0,0,0,0,0}, /* TrophiesOrLiveHeadBites */
	{0,0,0,0,0,0,0,0,0,0},
	10000,
	0,
	100,
	0,
	0,
	12,
	0,
	0,
	12,
	999,
	999,
	0,
	999,
	0,
	/* Padding! */
	{
		0,0,0,0,0,0,0,0,0,0,
		0,0,0,0,0,0,0,0,0,0,
		0,0,0,0,0,0,0,0,0,0,
		0,0,0,0,0,0,0,0,0,0
	}

};

#define COLOUR_WHITE	(0xffffffff)
#define COLOUR_RED		(0xffff0000)
#define COLOUR_GREEN	(0xff00ff00)

#define NEWLINE_SPACING	((ScreenDescriptorBlock.SDB_Height<400)? 12:15)

void CurrentGameStats_Initialise(void)
{
	int i;
	for (i=0; i<STATS_VICTIM_MAXIMUM; i++)
	{
		CurrentGameStatistics.Killed[i]=0;
		CurrentGameStatistics.Decapitated[i]=0;
		CurrentGameStatistics.Trophies[i]=0;
		CurrentGameStatistics.LiveHeadBites[i]=0;
		CurrentGameStatistics.DeadHeadBites[i]=0;
	}
	for (i=0; i<MAX_NO_OF_WEAPON_SLOTS; i++) {
		CurrentGameStatistics.WeaponTimes[i]=0;
		CurrentGameStatistics.ShotsFired[i]=0;
		CurrentGameStatistics.ShotsHit[i]=0;
	}
	for (i=0; i<NUMBER_OF_VISION_MODES; i++) {
		CurrentGameStatistics.VisionModeTimes[i]=0;
	}
	CurrentGameStatistics.Spotted=0;
	
	CurrentGameStatistics.Cloak_ElapsedSeconds=0;
	CurrentGameStatistics.Cloak_ElapsedMinutes=0;
	CurrentGameStatistics.Cloak_ElapsedHours=0;
	CurrentGameStatistics.FieldChargeUsed=0;

	CurrentGameStatistics.HealthDamage=0;
	CurrentGameStatistics.ArmourDamage=0;

	CurrentGameStatistics.IntegralSpeed=0;

	AvP.ElapsedSeconds = 0;
	AvP.ElapsedMinutes = 0;
	AvP.ElapsedHours = 0;

}

extern void CurrentGameStats_SpeedSample(unsigned int speed,unsigned int time) {

	if (PlayerStatusPtr->IsAlive==0) {
		return;
	}
	
	CurrentGameStatistics.IntegralSpeed+=(MUL_FIXED(speed,time));

}

extern void CurrentGameStats_DamageTaken(unsigned int health,unsigned int armour) {

	if (PlayerStatusPtr->IsAlive==0) {
		return;
	}

	CurrentGameStatistics.HealthDamage+=health;
	CurrentGameStatistics.ArmourDamage+=armour;

}

extern void CurrentGameStats_ChargeUsed(unsigned int charge) {

	if (PlayerStatusPtr->IsAlive==0) {
		return;
	}

	CurrentGameStatistics.FieldChargeUsed+=charge;

}

extern void CurrentGameStats_VisionMode(enum VISION_MODE_ID mode) {
	
	if (PlayerStatusPtr->IsAlive==0) {
		return;
	}

	CurrentGameStatistics.VisionModeTimes[mode]+=NormalFrameTime;

}

extern void CurrentGameStats_CloakOn(void) {

	if (PlayerStatusPtr->IsAlive==0) {
		return;
	}

	CurrentGameStatistics.Cloak_ElapsedSeconds += NormalFrameTime;
	
	if(CurrentGameStatistics.Cloak_ElapsedSeconds  >= FIXED_MINUTE)
	{
		CurrentGameStatistics.Cloak_ElapsedSeconds -= FIXED_MINUTE;
		CurrentGameStatistics.Cloak_ElapsedMinutes ++;
	}
	
	if(CurrentGameStatistics.Cloak_ElapsedMinutes >= 60)
	{
		CurrentGameStatistics.Cloak_ElapsedMinutes -= 60;
		CurrentGameStatistics.Cloak_ElapsedHours ++;
	}		

}

extern void CurrentGameStats_WeaponFired(enum WEAPON_SLOT slot,unsigned int rounds) {

	if (PlayerStatusPtr->IsAlive==0) {
		return;
	}

	if ((slot>=0)&&(slot<=MAX_NO_OF_WEAPON_SLOTS)) {
		CurrentGameStatistics.ShotsFired[slot]+=rounds;
	}

}

extern void CurrentGameStats_WeaponHit(enum WEAPON_SLOT slot,unsigned int rounds) {

	if (PlayerStatusPtr->IsAlive==0) {
		return;
	}

	if ((slot>=0)&&(slot<=MAX_NO_OF_WEAPON_SLOTS)) {
		CurrentGameStatistics.ShotsHit[slot]+=rounds;
	}

}

extern void CurrentGameStats_UsingWeapon(enum WEAPON_SLOT slot) {

	if (PlayerStatusPtr->IsAlive==0) {
		return;
	}
	
	if ((slot>=0)&&(slot<=MAX_NO_OF_WEAPON_SLOTS)) {
		CurrentGameStatistics.WeaponTimes[slot]+=NormalFrameTime;
	}

}

extern void CurrentGameStats_Spotted(void) {

	if (PlayerStatusPtr->IsAlive==0) {
		return;
	}
	
	CurrentGameStatistics.Spotted++;

}

extern void PrintSpottedNumber(void) {
	
	/* It must be in this file... */

	textprint("Spotted: %d\n",CurrentGameStatistics.Spotted);
	textprint("Size %d\n",sizeof(AvP_GameStats_Stored));

}

extern void CurrentGameStats_HeadBitten(STRATEGYBLOCK *sbPtr) {

	if (PlayerStatusPtr->IsAlive==0) {
		return;
	}

	switch (sbPtr->I_SBtype) {
		case I_BehaviourAlien:
		{
			ALIEN_STATUS_BLOCK *alienStatus = (ALIEN_STATUS_BLOCK *)sbPtr->SBdataptr;

			switch (alienStatus->Type)
			{
				case 0:
				default:
					CurrentGameStatistics.LiveHeadBites[STATS_VICTIM_XENOMORPH]++;
					break;
				case 1:
					CurrentGameStatistics.LiveHeadBites[STATS_VICTIM_PREDALIEN]++;
					break;
				case 2:
					CurrentGameStatistics.LiveHeadBites[STATS_VICTIM_PRAETORIAN]++;
					break;
			}

			break;
		}
		case I_BehaviourMarine:
		{
			MARINE_STATUS_BLOCK *marineStatusPointer;    

			LOCALASSERT(sbPtr);
			LOCALASSERT(sbPtr->containingModule); 
			marineStatusPointer = (MARINE_STATUS_BLOCK *)(sbPtr->SBdataptr);    
			
			if (marineStatusPointer->My_Weapon->Android)
			{
				CurrentGameStatistics.LiveHeadBites[STATS_VICTIM_ANDROID]++;
			}
			else if (marineStatusPointer->My_Weapon->ARealMarine)
			{
				CurrentGameStatistics.LiveHeadBites[STATS_VICTIM_MARINE]++;
			}
			else
			{
				CurrentGameStatistics.LiveHeadBites[STATS_VICTIM_CIVILIAN]++;
			}
			break;
		}
		case I_BehaviourQueenAlien:
		{
			CurrentGameStatistics.LiveHeadBites[STATS_VICTIM_QUEEN]++;
			break;
		}
		case I_BehaviourFaceHugger:
		{
			CurrentGameStatistics.LiveHeadBites[STATS_VICTIM_FACEHUGGER]++;
			break;
		}
		case I_BehaviourXenoborg:
		{
			CurrentGameStatistics.LiveHeadBites[STATS_VICTIM_XENOBORG]++;
			break;
		}
		case I_BehaviourPredator:
		{
			CurrentGameStatistics.LiveHeadBites[STATS_VICTIM_PREDATOR]++;
			break;
		}
		case I_BehaviourNetCorpse:
		{
			NETCORPSEDATABLOCK *corpseDataPtr;

		    LOCALASSERT(sbPtr);
			corpseDataPtr = (NETCORPSEDATABLOCK *)sbPtr->SBdataptr;
			LOCALASSERT(corpseDataPtr);

			switch (corpseDataPtr->Type)
			{
				case I_BehaviourAlien:
				{
					switch (corpseDataPtr->subtype)
					{
						case 0:
						default:
							CurrentGameStatistics.DeadHeadBites[STATS_VICTIM_XENOMORPH]++;
							break;
						case 1:
							CurrentGameStatistics.DeadHeadBites[STATS_VICTIM_PREDALIEN]++;
							break;
						case 2:
							CurrentGameStatistics.DeadHeadBites[STATS_VICTIM_PRAETORIAN]++;
							break;
					}
					break;
				}
				case I_BehaviourMarine:
				{
					
					if (corpseDataPtr->Android)
					{
						CurrentGameStatistics.DeadHeadBites[STATS_VICTIM_ANDROID]++;
					}
					else if (corpseDataPtr->ARealMarine)
					{
						CurrentGameStatistics.DeadHeadBites[STATS_VICTIM_MARINE]++;
					}
					else
					{
						CurrentGameStatistics.DeadHeadBites[STATS_VICTIM_CIVILIAN]++;
					}
					break;
				}
				case I_BehaviourQueenAlien:
				{
					CurrentGameStatistics.DeadHeadBites[STATS_VICTIM_QUEEN]++;
					break;
				}
				case I_BehaviourFaceHugger:
				{
					CurrentGameStatistics.DeadHeadBites[STATS_VICTIM_FACEHUGGER]++;
					break;
				}
				case I_BehaviourXenoborg:
				{
					CurrentGameStatistics.DeadHeadBites[STATS_VICTIM_XENOBORG]++;
					break;
				}


				case I_BehaviourPredator:
				{
					CurrentGameStatistics.DeadHeadBites[STATS_VICTIM_PREDATOR]++;
					break;
				}
				default:
				{
					break;
				}
			}
			break;
		}
		case I_BehaviourNetGhost:
		{
			NETGHOSTDATABLOCK *ghostData;
	
			LOCALASSERT(sbPtr);
			ghostData = (NETGHOSTDATABLOCK *)sbPtr->SBdataptr;
			LOCALASSERT(ghostData);
			
			switch (ghostData->type) {
				case I_BehaviourMarinePlayer:
					CurrentGameStatistics.LiveHeadBites[STATS_VICTIM_MARINE]++;
					break;
				case I_BehaviourAlienPlayer:
					CurrentGameStatistics.LiveHeadBites[STATS_VICTIM_XENOMORPH]++;
					break;
				case I_BehaviourPredatorPlayer:
					CurrentGameStatistics.LiveHeadBites[STATS_VICTIM_PREDATOR]++;
					break;
				case I_BehaviourNetCorpse:
				{
					switch (ghostData->subtype) {
						case I_BehaviourAlien:
						{
							switch (ghostData->IOType) {
								/* Hey ho, it was free... */
								case 0:
								default:
									CurrentGameStatistics.DeadHeadBites[STATS_VICTIM_XENOMORPH]++;
									break;
								case 1:
									CurrentGameStatistics.DeadHeadBites[STATS_VICTIM_PREDALIEN]++;
									break;
								case 2:
									CurrentGameStatistics.DeadHeadBites[STATS_VICTIM_PRAETORIAN]++;
									break;
							}
							break;
						}
						case I_BehaviourMarinePlayer:
						{
							CurrentGameStatistics.DeadHeadBites[STATS_VICTIM_MARINE]++;
							break;
						}
						case I_BehaviourAlienPlayer:
						{
							CurrentGameStatistics.DeadHeadBites[STATS_VICTIM_XENOMORPH]++;
							break;
						}
						case I_BehaviourPredatorPlayer:
						{
							CurrentGameStatistics.DeadHeadBites[STATS_VICTIM_PREDATOR]++;
							break;
						}
						default:
						{
							break;
						}
					}
					break;
				}
				default:
				{
					break;
				}
			}
			break;
		}
		default:
		{
			break;
		}
	}

}

extern void CurrentGameStats_TrophyCollected(STRATEGYBLOCK *sbPtr) {

	if (PlayerStatusPtr->IsAlive==0) {
		return;
	}

	/* It's always going to be a corpse or a ghost, isn't it? */

	switch (sbPtr->I_SBtype) {
		case I_BehaviourNetCorpse:
		{
			NETCORPSEDATABLOCK *corpseDataPtr;

		    LOCALASSERT(sbPtr);
			corpseDataPtr = (NETCORPSEDATABLOCK *)sbPtr->SBdataptr;
			LOCALASSERT(corpseDataPtr);

			switch (corpseDataPtr->Type)
			{
				case I_BehaviourAlien:
				{
					switch (corpseDataPtr->subtype)
					{
						case 0:
						default:
							CurrentGameStatistics.Trophies[STATS_VICTIM_XENOMORPH]++;
							break;
						case 1:
							CurrentGameStatistics.Trophies[STATS_VICTIM_PREDALIEN]++;
							break;
						case 2:
							CurrentGameStatistics.Trophies[STATS_VICTIM_PRAETORIAN]++;
							break;
					}
					break;
				}
				case I_BehaviourMarine:
				{
					
					if (corpseDataPtr->Android)
					{
						CurrentGameStatistics.Trophies[STATS_VICTIM_ANDROID]++;
					}
					else if (corpseDataPtr->ARealMarine)
					{
						CurrentGameStatistics.Trophies[STATS_VICTIM_MARINE]++;
					}
					else
					{
						CurrentGameStatistics.Trophies[STATS_VICTIM_CIVILIAN]++;
					}
					break;
				}
				case I_BehaviourQueenAlien:
				{
					CurrentGameStatistics.Trophies[STATS_VICTIM_QUEEN]++;
					break;
				}
				case I_BehaviourFaceHugger:
				{
					CurrentGameStatistics.Trophies[STATS_VICTIM_FACEHUGGER]++;
					break;
				}
				case I_BehaviourXenoborg:
				{
					CurrentGameStatistics.Trophies[STATS_VICTIM_XENOBORG]++;
					break;
				}


				case I_BehaviourPredator:
				{
					CurrentGameStatistics.Trophies[STATS_VICTIM_PREDATOR]++;
					break;
				}
				default:
				{
					break;
				}
			}
			break;
		}
		case I_BehaviourNetGhost:
		{
			NETGHOSTDATABLOCK *ghostData;
	
			LOCALASSERT(sbPtr);
			ghostData = (NETGHOSTDATABLOCK *)sbPtr->SBdataptr;
			LOCALASSERT(ghostData);
			
			switch (ghostData->type) {
				case I_BehaviourNetCorpse:
				{
					switch (ghostData->subtype) {
						case I_BehaviourAlien:
						{
							switch (ghostData->IOType) {
								/* Hey ho, it was free... */
								case 0:
								default:
									CurrentGameStatistics.Trophies[STATS_VICTIM_XENOMORPH]++;
									break;
								case 1:
									CurrentGameStatistics.Trophies[STATS_VICTIM_PREDALIEN]++;
									break;
								case 2:
									CurrentGameStatistics.Trophies[STATS_VICTIM_PRAETORIAN]++;
									break;
							}
							break;
						}
						case I_BehaviourMarinePlayer:
						{
							CurrentGameStatistics.Trophies[STATS_VICTIM_MARINE]++;
							break;
						}
						case I_BehaviourAlienPlayer:
						{
							CurrentGameStatistics.Trophies[STATS_VICTIM_XENOMORPH]++;
							break;
						}
						case I_BehaviourPredatorPlayer:
						{
							CurrentGameStatistics.Trophies[STATS_VICTIM_PREDATOR]++;
							break;
						}
						default:
						{
							break;
						}
					}
					break;
				}
				default:
				{
					break;
				}
			}
			break;
		}
		case I_BehaviourHierarchicalFragment:
		{
			HDEBRIS_BEHAV_BLOCK *debrisDataPtr;

		    LOCALASSERT(sbPtr);
			debrisDataPtr = (HDEBRIS_BEHAV_BLOCK *)sbPtr->SBdataptr;
			LOCALASSERT(debrisDataPtr);

			switch (debrisDataPtr->Type) {
				case I_BehaviourAlien:
				{
					switch (debrisDataPtr->SubType)
					{
						case 0:
						default:
							CurrentGameStatistics.Trophies[STATS_VICTIM_XENOMORPH]++;
							break;
						case 1:
							CurrentGameStatistics.Trophies[STATS_VICTIM_PREDALIEN]++;
							break;
						case 2:
							CurrentGameStatistics.Trophies[STATS_VICTIM_PRAETORIAN]++;
							break;
					}
					break;
				}
				case I_BehaviourMarine:
				{
					
					if (debrisDataPtr->Android)
					{
						CurrentGameStatistics.Trophies[STATS_VICTIM_ANDROID]++;
					}
					else if (debrisDataPtr->SubType) /* Means ARealMarine for fragments of marines */
					{
						CurrentGameStatistics.Trophies[STATS_VICTIM_MARINE]++;
					}
					else
					{
						CurrentGameStatistics.Trophies[STATS_VICTIM_CIVILIAN]++;
					}
					break;
				}
				case I_BehaviourQueenAlien:
				{
					CurrentGameStatistics.Trophies[STATS_VICTIM_QUEEN]++;
					break;
				}
				case I_BehaviourFaceHugger:
				{
					CurrentGameStatistics.Trophies[STATS_VICTIM_FACEHUGGER]++;
					break;
				}
				case I_BehaviourXenoborg:
				{
					CurrentGameStatistics.Trophies[STATS_VICTIM_XENOBORG]++;
					break;
				}


				case I_BehaviourPredator:
				{
					CurrentGameStatistics.Trophies[STATS_VICTIM_PREDATOR]++;
					break;
				}
				default:
				{
					break;
				}
			}
			break;
		}
		case I_BehaviourSpeargunBolt:
		{
			SPEAR_BEHAV_BLOCK *spearDataPtr;

		    LOCALASSERT(sbPtr);
			spearDataPtr = (SPEAR_BEHAV_BLOCK *)sbPtr->SBdataptr;
			LOCALASSERT(spearDataPtr);

			switch (spearDataPtr->Type) {
				case I_BehaviourAlien:
				{
					switch (spearDataPtr->SubType)
					{
						case 0:
						default:
							CurrentGameStatistics.Trophies[STATS_VICTIM_XENOMORPH]++;
							break;
						case 1:
							CurrentGameStatistics.Trophies[STATS_VICTIM_PREDALIEN]++;
							break;
						case 2:
							CurrentGameStatistics.Trophies[STATS_VICTIM_PRAETORIAN]++;
							break;
					}
					break;
				}
				case I_BehaviourMarine:
				{
					
					if (spearDataPtr->Android)
					{
						CurrentGameStatistics.Trophies[STATS_VICTIM_ANDROID]++;
					}
					else if (spearDataPtr->SubType) /* Means ARealMarine for fragments of marines */
					{
						CurrentGameStatistics.Trophies[STATS_VICTIM_MARINE]++;
					}
					else
					{
						CurrentGameStatistics.Trophies[STATS_VICTIM_CIVILIAN]++;
					}
					break;
				}
				case I_BehaviourQueenAlien:
				{
					CurrentGameStatistics.Trophies[STATS_VICTIM_QUEEN]++;
					break;
				}
				case I_BehaviourFaceHugger:
				{
					CurrentGameStatistics.Trophies[STATS_VICTIM_FACEHUGGER]++;
					break;
				}
				case I_BehaviourXenoborg:
				{
					CurrentGameStatistics.Trophies[STATS_VICTIM_XENOBORG]++;
					break;
				}


				case I_BehaviourPredator:
				{
					CurrentGameStatistics.Trophies[STATS_VICTIM_PREDATOR]++;
					break;
				}
				default:
				{
					break;
				}
			}
			break;
		}
		default:
		{
			break;
		}
	}
}

extern void CurrentGameStats_CreatureKilled(STRATEGYBLOCK *sbPtr,SECTION_DATA *sectionDataPtr)
{

	if (PlayerStatusPtr->IsAlive==0) {
		return;
	}

	switch (sbPtr->I_SBtype)
	{
		case I_BehaviourAlien:
		{
			ALIEN_STATUS_BLOCK *alienStatus = (ALIEN_STATUS_BLOCK *)sbPtr->SBdataptr;

			switch (alienStatus->Type)
			{
				case 0:
				default:
					CurrentGameStatistics.Killed[STATS_VICTIM_XENOMORPH]++;
					break;
				case 1:
					CurrentGameStatistics.Killed[STATS_VICTIM_PREDALIEN]++;
					break;
				case 2:
					CurrentGameStatistics.Killed[STATS_VICTIM_PRAETORIAN]++;
					break;
			}
				
			if (sectionDataPtr)
			{
				if (sectionDataPtr->sempai)
				{
					if (sectionDataPtr->sempai->Section_Name)
					{
						if (!_stricmp(sectionDataPtr->sempai->Section_Name,"head"))
						{
							CurrentGameStatistics.Decapitated[STATS_VICTIM_XENOMORPH]++;
						}
					}
				}
			}

			break;
		}
		case I_BehaviourMarine:
		{
			MARINE_STATUS_BLOCK *marineStatusPointer;    

			LOCALASSERT(sbPtr);
			LOCALASSERT(sbPtr->containingModule); 
			marineStatusPointer = (MARINE_STATUS_BLOCK *)(sbPtr->SBdataptr);    
			
			if (marineStatusPointer->My_Weapon->Android)
			{
				CurrentGameStatistics.Killed[STATS_VICTIM_ANDROID]++;
			}
			else if (marineStatusPointer->My_Weapon->ARealMarine)
			{
				CurrentGameStatistics.Killed[STATS_VICTIM_MARINE]++;
			}
			else
			{
				CurrentGameStatistics.Killed[STATS_VICTIM_CIVILIAN]++;
			}
			break;
		}
		case I_BehaviourQueenAlien:
		{
			CurrentGameStatistics.Killed[STATS_VICTIM_QUEEN]++;
			break;
		}
		case I_BehaviourFaceHugger:
		{
			CurrentGameStatistics.Killed[STATS_VICTIM_FACEHUGGER]++;
			break;
		}
		case I_BehaviourXenoborg:
		{
			CurrentGameStatistics.Killed[STATS_VICTIM_XENOBORG]++;
			break;
		}


		case I_BehaviourPredator:
		{
			CurrentGameStatistics.Killed[STATS_VICTIM_PREDATOR]++;
			break;
		}
		
		default: ;
	}
}


void DoFailedLevelStatisticsScreen(void)
{
	extern int deathFadeLevel;
	D3D_FadeDownScreen(deathFadeLevel,0);
	DoStatisticsScreen(0);
	
	if (!deathFadeLevel) 
	{
		RenderStringCentred(GetTextString(TEXTSTRING_MULTIPLAYER_PRESSKEYTORESTARTGAME),ScreenDescriptorBlock.SDB_Width/2,ScreenDescriptorBlock.SDB_Height-20,0xffffffff);
	}

}

enum TEXTSTRING_ID TemporaryNameStore[] =
{
	TEXTSTRING_GAMESTATS_FACEHUGGER_PL,
	TEXTSTRING_GAMESTATS_XENOMORPH_PL,
	TEXTSTRING_GAMESTATS_PRAETORIAN_PL,
	TEXTSTRING_GAMESTATS_QUEEN_PL,
	TEXTSTRING_GAMESTATS_XENOBORG_PL,
	TEXTSTRING_GAMESTATS_PREDALIEN_PL,
	TEXTSTRING_GAMESTATS_PREDATOR_PL,
	TEXTSTRING_GAMESTATS_MARINE_PL,
	TEXTSTRING_GAMESTATS_CIVILIAN_PL,
	TEXTSTRING_GAMESTATS_ANDROID_PL,
};

enum TEXTSTRING_ID TemporaryNameStore2[] =
{
	TEXTSTRING_GAMESTATS_FACEHUGGER,
	TEXTSTRING_GAMESTATS_XENOMORPH,
	TEXTSTRING_GAMESTATS_PRAETORIAN,
	TEXTSTRING_GAMESTATS_QUEEN,
	TEXTSTRING_GAMESTATS_XENOBORG,
	TEXTSTRING_GAMESTATS_PREDALIEN,
	TEXTSTRING_GAMESTATS_PREDATOR,
	TEXTSTRING_GAMESTATS_MARINE,
	TEXTSTRING_GAMESTATS_CIVILIAN,
	TEXTSTRING_GAMESTATS_ANDROID,
};

enum TEXTSTRING_ID VisionModeNames[] =
{
	TEXTSTRING_GAMESTATS_VM_NORMAL,
	TEXTSTRING_GAMESTATS_VM_NAVSENSE,
	TEXTSTRING_GAMESTATS_VM_INTENSIFIER,
	TEXTSTRING_GAMESTATS_VM_THERMAL,
	TEXTSTRING_GAMESTATS_VM_ELECTRICAL,
	TEXTSTRING_GAMESTATS_VM_PREDTECH,
};


extern void DoStatisticsScreen(int completed_level)
{
	// content to be finalised. Language localisation issues: time formats etc. 
	int y;
	char buffer[100];
	int level_num;
	int colour_to_draw,best;

	int targets,targetspassed;

	NPC_DATA *NpcData = NULL;
	
	switch (AvP.PlayerType)
	{
		case I_Marine:
			switch (AvP.Difficulty) {
				case I_Easy:
					NpcData=GetThisNpcData(I_PC_Marine_Easy);
					break;
				default:
				case I_Medium:
					NpcData=GetThisNpcData(I_PC_Marine_Medium);
					break;
				case I_Hard:
					NpcData=GetThisNpcData(I_PC_Marine_Hard);
					break;
				case I_Impossible:
					NpcData=GetThisNpcData(I_PC_Marine_Impossible);
					break;
			}
			break;
		case I_Alien:
			switch (AvP.Difficulty) {
				case I_Easy:
					NpcData=GetThisNpcData(I_PC_Alien_Easy);
					break;
				default:
				case I_Medium:
					NpcData=GetThisNpcData(I_PC_Alien_Medium);
					break;
				case I_Hard:
					NpcData=GetThisNpcData(I_PC_Alien_Hard);
					break;
				case I_Impossible:
					NpcData=GetThisNpcData(I_PC_Alien_Impossible);
					break;
			}
			break;
		case I_Predator:
			switch (AvP.Difficulty) {
				case I_Easy:
					NpcData=GetThisNpcData(I_PC_Predator_Easy);
					break;
				default:
				case I_Medium:
					NpcData=GetThisNpcData(I_PC_Predator_Medium);
					break;
				case I_Hard:
					NpcData=GetThisNpcData(I_PC_Predator_Hard);
					break;
				case I_Impossible:
					NpcData=GetThisNpcData(I_PC_Predator_Impossible);
					break;
			}
			break;
		default:
			LOCALASSERT(0);
	}

	if (PlayerStatusPtr->soundHandle5!=SOUND_NOACTIVEINDEX) {
		Sound_Stop(PlayerStatusPtr->soundHandle5);
	}

	level_num=NumberForCurrentLevel();
	targets=0;
	targetspassed=0;

	/* Print their name. */
	sprintf(buffer,"%s",UserProfilePtr->Name);
	RenderString(buffer,TABPOINT1,20,COLOUR_WHITE);

	if (completed_level) {
		RenderStringCentred(GetTextString(TEXTSTRING_GAMESTATS_LEVELCOMPLETED),
			ScreenDescriptorBlock.SDB_Width/2,20,COLOUR_GREEN);
	} else {
		RenderStringCentred(GetTextString(TEXTSTRING_GAMESTATS_LEVELNOTCOMPLETED),
			ScreenDescriptorBlock.SDB_Width/2,20,COLOUR_RED);
	}
	
	#if 0
	RenderStringCentred(GetTextString(TEXTSTRING_GAMESTATS_NAME),TABPOINT1A,40,COLOUR_WHITE);
	#endif
	RenderStringCentred(GetTextString(TEXTSTRING_GAMESTATS_YOUR),TABPOINT2,40,COLOUR_WHITE);
	RenderStringCentred(GetTextString(TEXTSTRING_GAMESTATS_BEST),TABPOINT3,40,COLOUR_WHITE);
	RenderStringCentred(GetTextString(TEXTSTRING_GAMESTATS_TARGET),TABPOINT4,40,COLOUR_WHITE);

	y = 55;

	colour_to_draw=COLOUR_WHITE;

	if ((completed_level)&&(level_num<AVP_ENVIRONMENT_END_OF_LIST)&&(NotCheating)) {
		/* Is it a new best? */
		best=0;
		if (UserProfilePtr->PersonalBests[AvP.Difficulty][level_num].Total_ElapsedHours>AvP.ElapsedHours) {
			best=1;
		} else if (UserProfilePtr->PersonalBests[AvP.Difficulty][level_num].Total_ElapsedHours==AvP.ElapsedHours) {
			if (UserProfilePtr->PersonalBests[AvP.Difficulty][level_num].Total_ElapsedMinutes>AvP.ElapsedMinutes) {
				best=1;
			} else if (UserProfilePtr->PersonalBests[AvP.Difficulty][level_num].Total_ElapsedMinutes==AvP.ElapsedMinutes) {
				if (UserProfilePtr->PersonalBests[AvP.Difficulty][level_num].Total_ElapsedSeconds>AvP.ElapsedSeconds) {
					best=1;
				} else if (UserProfilePtr->PersonalBests[AvP.Difficulty][level_num].Total_ElapsedSeconds==AvP.ElapsedSeconds) {
					best=1;
				}		
			}		
		}
		if (best) {
			colour_to_draw=COLOUR_GREEN;
			UserProfilePtr->PersonalBests[AvP.Difficulty][level_num].Total_ElapsedHours=AvP.ElapsedHours;
			UserProfilePtr->PersonalBests[AvP.Difficulty][level_num].Total_ElapsedMinutes=AvP.ElapsedMinutes;
			UserProfilePtr->PersonalBests[AvP.Difficulty][level_num].Total_ElapsedSeconds=AvP.ElapsedSeconds;
		}
	}

	RenderString(GetTextString(TEXTSTRING_GAMESTATS_TIMEELAPSED),TABPOINT1,y,colour_to_draw);

	sprintf(buffer,"%dh %02dm %02ds",AvP.ElapsedHours,AvP.ElapsedMinutes,AvP.ElapsedSeconds/65536);
	RenderStringCentred(buffer,TABPOINT2,y,colour_to_draw); //y was 50!

	if ((level_num<AVP_ENVIRONMENT_END_OF_LIST)&&(NotCheating)) {
		sprintf(buffer,"%dh %02dm %02ds",
			UserProfilePtr->PersonalBests[AvP.Difficulty][level_num].Total_ElapsedHours,
			UserProfilePtr->PersonalBests[AvP.Difficulty][level_num].Total_ElapsedMinutes,
			UserProfilePtr->PersonalBests[AvP.Difficulty][level_num].Total_ElapsedSeconds/65536);
		RenderStringCentred(buffer,TABPOINT3,y,colour_to_draw); //y was 50!
	} else {
		RenderStringCentred("---",TABPOINT3,y,COLOUR_WHITE);
	}

	colour_to_draw=COLOUR_WHITE;

	if ((completed_level)&&(level_num<AVP_ENVIRONMENT_END_OF_LIST)) {
		/* Is it a completed target? */
		best=0;
		if (LevelStatsTargets[AvP.Difficulty][level_num].StatTargets.Total_ElapsedHours>-1) {
			targets++;
			if (LevelStatsTargets[AvP.Difficulty][level_num].StatTargets.Total_ElapsedHours>AvP.ElapsedHours) {
				best=1;
			} else if (LevelStatsTargets[AvP.Difficulty][level_num].StatTargets.Total_ElapsedHours==AvP.ElapsedHours) {
				if (LevelStatsTargets[AvP.Difficulty][level_num].StatTargets.Total_ElapsedMinutes>AvP.ElapsedMinutes) {
					best=1;
				} else if (LevelStatsTargets[AvP.Difficulty][level_num].StatTargets.Total_ElapsedMinutes==AvP.ElapsedMinutes) {
					if (LevelStatsTargets[AvP.Difficulty][level_num].StatTargets.Total_ElapsedSeconds>AvP.ElapsedSeconds) {
						best=1;
					} else if (LevelStatsTargets[AvP.Difficulty][level_num].StatTargets.Total_ElapsedSeconds==AvP.ElapsedSeconds) {
						best=1;
					}		
				}		
			}
			if (best) {
				targetspassed++;
				colour_to_draw=COLOUR_RED;
			}
		}
	}
	if ((level_num<AVP_ENVIRONMENT_END_OF_LIST)&&(NotCheating)) {
		if (UserProfilePtr->CheatMode[LevelStatsTargets[AvP.Difficulty][level_num].CheatModeToActivate]!=1) {
			if (LevelStatsTargets[AvP.Difficulty][level_num].StatTargets.Total_ElapsedHours>-1) {
				sprintf(buffer,"%dh %02dm %02ds",
					LevelStatsTargets[AvP.Difficulty][level_num].StatTargets.Total_ElapsedHours,
					LevelStatsTargets[AvP.Difficulty][level_num].StatTargets.Total_ElapsedMinutes,
					LevelStatsTargets[AvP.Difficulty][level_num].StatTargets.Total_ElapsedSeconds/65536
				);
				RenderStringCentred(buffer,TABPOINT4,y,colour_to_draw);
			} else {
				RenderStringCentred("---",TABPOINT4,y,COLOUR_WHITE);
			}
		} else {
			RenderStringCentred("---",TABPOINT4,y,COLOUR_WHITE);
		}
	} else {
		RenderStringCentred("---",TABPOINT4,y,COLOUR_WHITE);
	}

	y+=NEWLINE_SPACING;
	colour_to_draw=COLOUR_WHITE;

	if (AvP.PlayerType==I_Predator) {

		if ((completed_level)&&(level_num<AVP_ENVIRONMENT_END_OF_LIST)&&(NotCheating)) {
			/* Is it a new best? */
			best=0;
			if (UserProfilePtr->PersonalBests[AvP.Difficulty][level_num].Cloak_ElapsedHours>CurrentGameStatistics.Cloak_ElapsedHours) {
				best=1;
			} else if (UserProfilePtr->PersonalBests[AvP.Difficulty][level_num].Cloak_ElapsedHours==CurrentGameStatistics.Cloak_ElapsedHours) {
				if (UserProfilePtr->PersonalBests[AvP.Difficulty][level_num].Cloak_ElapsedMinutes>CurrentGameStatistics.Cloak_ElapsedMinutes) {
					best=1;
				} else if (UserProfilePtr->PersonalBests[AvP.Difficulty][level_num].Cloak_ElapsedMinutes==CurrentGameStatistics.Cloak_ElapsedMinutes) {
					if (UserProfilePtr->PersonalBests[AvP.Difficulty][level_num].Cloak_ElapsedSeconds>CurrentGameStatistics.Cloak_ElapsedSeconds) {
						best=1;
					} else if (UserProfilePtr->PersonalBests[AvP.Difficulty][level_num].Cloak_ElapsedSeconds==CurrentGameStatistics.Cloak_ElapsedSeconds) {
						best=1;
					}		
				}		
			}
			if (best) {
				colour_to_draw=COLOUR_GREEN;
				UserProfilePtr->PersonalBests[AvP.Difficulty][level_num].Cloak_ElapsedHours=CurrentGameStatistics.Cloak_ElapsedHours;
				UserProfilePtr->PersonalBests[AvP.Difficulty][level_num].Cloak_ElapsedMinutes=CurrentGameStatistics.Cloak_ElapsedMinutes;
				UserProfilePtr->PersonalBests[AvP.Difficulty][level_num].Cloak_ElapsedSeconds=CurrentGameStatistics.Cloak_ElapsedSeconds;
			}
		}

		RenderString(GetTextString(TEXTSTRING_GAMESTATS_TIMECLOAKED),TABPOINT1,y,colour_to_draw);

		sprintf(buffer,"%dh %02dm %02ds",CurrentGameStatistics.Cloak_ElapsedHours,CurrentGameStatistics.Cloak_ElapsedMinutes,CurrentGameStatistics.Cloak_ElapsedSeconds/65536);
		RenderStringCentred(buffer,TABPOINT2,y,colour_to_draw);

		if ((level_num<AVP_ENVIRONMENT_END_OF_LIST)&&(NotCheating)) {
			sprintf(buffer,"%dh %02dm %02ds",
				UserProfilePtr->PersonalBests[AvP.Difficulty][level_num].Cloak_ElapsedHours,
				UserProfilePtr->PersonalBests[AvP.Difficulty][level_num].Cloak_ElapsedMinutes,
				UserProfilePtr->PersonalBests[AvP.Difficulty][level_num].Cloak_ElapsedSeconds/65536);
			RenderStringCentred(buffer,TABPOINT3,y,colour_to_draw); //y was 50!
		} else {
			RenderStringCentred("---",TABPOINT3,y,COLOUR_WHITE);
		}

		colour_to_draw=COLOUR_WHITE;

		if ((completed_level)&&(level_num<AVP_ENVIRONMENT_END_OF_LIST)) {
			/* Is it a completed target? */
			best=0;
			if (LevelStatsTargets[AvP.Difficulty][level_num].StatTargets.Cloak_ElapsedHours>-1) {
				targets++;
				if (LevelStatsTargets[AvP.Difficulty][level_num].StatTargets.Cloak_ElapsedHours>AvP.ElapsedHours) {
					best=1;
				} else if (LevelStatsTargets[AvP.Difficulty][level_num].StatTargets.Cloak_ElapsedHours==AvP.ElapsedHours) {
					if (LevelStatsTargets[AvP.Difficulty][level_num].StatTargets.Cloak_ElapsedMinutes>AvP.ElapsedMinutes) {
						best=1;
					} else if (LevelStatsTargets[AvP.Difficulty][level_num].StatTargets.Cloak_ElapsedMinutes==AvP.ElapsedMinutes) {
						if (LevelStatsTargets[AvP.Difficulty][level_num].StatTargets.Cloak_ElapsedSeconds>AvP.ElapsedSeconds) {
							best=1;
						} else if (LevelStatsTargets[AvP.Difficulty][level_num].StatTargets.Cloak_ElapsedSeconds==AvP.ElapsedSeconds) {
							best=1;
						}		
					}		
				}
				if (best) {
					targetspassed++;
					colour_to_draw=COLOUR_RED;
				}
			}
		}
		if ((level_num<AVP_ENVIRONMENT_END_OF_LIST)&&(NotCheating)) {
			if (UserProfilePtr->CheatMode[LevelStatsTargets[AvP.Difficulty][level_num].CheatModeToActivate]!=1) {
				if (LevelStatsTargets[AvP.Difficulty][level_num].StatTargets.Cloak_ElapsedHours>-1) {
					sprintf(buffer,"%dh %02dm %02ds",
						LevelStatsTargets[AvP.Difficulty][level_num].StatTargets.Cloak_ElapsedHours,
						LevelStatsTargets[AvP.Difficulty][level_num].StatTargets.Cloak_ElapsedMinutes,
						LevelStatsTargets[AvP.Difficulty][level_num].StatTargets.Cloak_ElapsedSeconds/65536
					);
					RenderStringCentred(buffer,TABPOINT4,y,colour_to_draw);
				} else {
					RenderStringCentred("---",TABPOINT4,y,COLOUR_WHITE);
				}
			} else {
				RenderStringCentred("---",TABPOINT4,y,COLOUR_WHITE);
			}
		} else {
			RenderStringCentred("---",TABPOINT4,y,COLOUR_WHITE);
		}

		y+=NEWLINE_SPACING;
		colour_to_draw=COLOUR_WHITE;
	}

	{
		int i;
		for (i=0; i<STATS_VICTIM_MAXIMUM; i++)
		{
			if ((CurrentGameStatistics.Killed[i])
				||(LevelStatsTargets[AvP.Difficulty][level_num].StatTargets.Killed[i]>-1)
				||((AvP.PlayerType==I_Predator)&&(LevelStatsTargets[AvP.Difficulty][level_num].StatTargets.TrophiesOrLiveHeadBites[i]>-1))
				||((AvP.PlayerType==I_Alien)&&(LevelStatsTargets[AvP.Difficulty][level_num].StatTargets.TrophiesOrLiveHeadBites[i]>-1))
				||((AvP.PlayerType==I_Alien)&&(LevelStatsTargets[AvP.Difficulty][level_num].StatTargets.DeadHeadBites[i]>-1))
				) {
				if ((completed_level)&&(level_num<AVP_ENVIRONMENT_END_OF_LIST)&&(NotCheating)) {
					/* Is it a new best? */
					if (UserProfilePtr->PersonalBests[AvP.Difficulty][level_num].Killed[i]<=CurrentGameStatistics.Killed[i]) {
						UserProfilePtr->PersonalBests[AvP.Difficulty][level_num].Killed[i]=CurrentGameStatistics.Killed[i];
						colour_to_draw=COLOUR_GREEN;
					}
				}

				sprintf(buffer,"%s %s",GetTextString(TemporaryNameStore[i]),GetTextString(TEXTSTRING_GAMESTATS_KILLED));
				RenderString(buffer,TABPOINT1,y,colour_to_draw);

				sprintf(buffer,"%d",CurrentGameStatistics.Killed[i]);
				RenderStringCentred(buffer,TABPOINT2,y,colour_to_draw);

				if ((level_num<AVP_ENVIRONMENT_END_OF_LIST)&&(NotCheating)) {
					sprintf(buffer,"%d",
						UserProfilePtr->PersonalBests[AvP.Difficulty][level_num].Killed[i]
					);
					RenderStringCentred(buffer,TABPOINT3,y,colour_to_draw);
				} else {
					RenderStringCentred("---",TABPOINT3,y,COLOUR_WHITE);
				}

				colour_to_draw=COLOUR_WHITE;
				if ((completed_level)&&(level_num<AVP_ENVIRONMENT_END_OF_LIST)) {
					/* Is it a completed target? */
					if (LevelStatsTargets[AvP.Difficulty][level_num].StatTargets.Killed[i]>-1) {
						targets++;
						if (LevelStatsTargets[AvP.Difficulty][level_num].StatTargets.Killed[i]<=CurrentGameStatistics.Killed[i]) {
							colour_to_draw=COLOUR_RED;
							targetspassed++;
						}
					}
				}
				if ((level_num<AVP_ENVIRONMENT_END_OF_LIST)&&(NotCheating)) {
					if (UserProfilePtr->CheatMode[LevelStatsTargets[AvP.Difficulty][level_num].CheatModeToActivate]!=1) {
						if (LevelStatsTargets[AvP.Difficulty][level_num].StatTargets.Killed[i]>-1) {
							sprintf(buffer,"%d",
								LevelStatsTargets[AvP.Difficulty][level_num].StatTargets.Killed[i]
							);
							RenderStringCentred(buffer,TABPOINT4,y,colour_to_draw);
						} else {
							RenderStringCentred("---",TABPOINT4,y,COLOUR_WHITE);
						}
					} else {
						RenderStringCentred("---",TABPOINT4,y,COLOUR_WHITE);
					}
				} else {
					RenderStringCentred("---",TABPOINT4,y,COLOUR_WHITE);
				}

				y+=NEWLINE_SPACING;
				colour_to_draw=COLOUR_WHITE;
				if (AvP.PlayerType==I_Predator) {
					if ((i!=STATS_VICTIM_XENOBORG)
						&&(i!=STATS_VICTIM_QUEEN)
						&&(i!=STATS_VICTIM_FACEHUGGER)
						&&(i!=STATS_VICTIM_PREDATOR)
						){

						if ((completed_level)&&(level_num<AVP_ENVIRONMENT_END_OF_LIST)&&(NotCheating)) {
							/* Is it a new best? */
							if (UserProfilePtr->PersonalBests[AvP.Difficulty][level_num].TrophiesOrLiveHeadBites[i]<=CurrentGameStatistics.Trophies[i]) {
								UserProfilePtr->PersonalBests[AvP.Difficulty][level_num].TrophiesOrLiveHeadBites[i]=CurrentGameStatistics.Trophies[i];
								colour_to_draw=COLOUR_GREEN;
							}
						}

						sprintf(buffer,"%s %s",GetTextString(TemporaryNameStore2[i]),GetTextString(TEXTSTRING_GAMESTATS_TROPHIESCOLLECTED));
						RenderString(buffer,TABPOINT1,y,colour_to_draw);
						
						sprintf(buffer,"%d",CurrentGameStatistics.Trophies[i]);
						RenderStringCentred(buffer,TABPOINT2,y,colour_to_draw);
						
						if ((level_num<AVP_ENVIRONMENT_END_OF_LIST)&&(NotCheating)) {
							sprintf(buffer,"%d",
								UserProfilePtr->PersonalBests[AvP.Difficulty][level_num].TrophiesOrLiveHeadBites[i]
							);
							RenderStringCentred(buffer,TABPOINT3,y,colour_to_draw);
						} else {
							RenderStringCentred("---",TABPOINT3,y,COLOUR_WHITE);
						}

						colour_to_draw=COLOUR_WHITE;
						if ((completed_level)&&(level_num<AVP_ENVIRONMENT_END_OF_LIST)) {
							/* Is it a completed target? */
							if (LevelStatsTargets[AvP.Difficulty][level_num].StatTargets.TrophiesOrLiveHeadBites[i]>-1) {
								targets++;
								if (LevelStatsTargets[AvP.Difficulty][level_num].StatTargets.TrophiesOrLiveHeadBites[i]<=CurrentGameStatistics.Trophies[i]) {
									colour_to_draw=COLOUR_RED;
									targetspassed++;
								}
							}
						}
						if ((level_num<AVP_ENVIRONMENT_END_OF_LIST)&&(NotCheating)) {
							if (UserProfilePtr->CheatMode[LevelStatsTargets[AvP.Difficulty][level_num].CheatModeToActivate]!=1) {
								if (LevelStatsTargets[AvP.Difficulty][level_num].StatTargets.TrophiesOrLiveHeadBites[i]>-1) {
									sprintf(buffer,"%d",
										LevelStatsTargets[AvP.Difficulty][level_num].StatTargets.TrophiesOrLiveHeadBites[i]
									);
									RenderStringCentred(buffer,TABPOINT4,y,colour_to_draw);
								} else {
									RenderStringCentred("---",TABPOINT4,y,COLOUR_WHITE);
								}
							} else {
								RenderStringCentred("---",TABPOINT4,y,COLOUR_WHITE);
							}
						} else {
							RenderStringCentred("---",TABPOINT4,y,COLOUR_WHITE);
						}

						y+=NEWLINE_SPACING;
						colour_to_draw=COLOUR_WHITE;
					}
				} else if (AvP.PlayerType==I_Alien) {
					if ((i!=STATS_VICTIM_ANDROID)
						&&(i!=STATS_VICTIM_XENOMORPH)
						&&(i!=STATS_VICTIM_PREDALIEN)
						&&(i!=STATS_VICTIM_PRAETORIAN)
						&&(i!=STATS_VICTIM_QUEEN)
						&&(i!=STATS_VICTIM_XENOBORG)
						&&(i!=STATS_VICTIM_FACEHUGGER)
						){

						if ((completed_level)&&(level_num<AVP_ENVIRONMENT_END_OF_LIST)&&(NotCheating)) {
							/* Is it a new best? */
							if (UserProfilePtr->PersonalBests[AvP.Difficulty][level_num].TrophiesOrLiveHeadBites[i]<=CurrentGameStatistics.LiveHeadBites[i]) {
								UserProfilePtr->PersonalBests[AvP.Difficulty][level_num].TrophiesOrLiveHeadBites[i]=CurrentGameStatistics.LiveHeadBites[i];
								colour_to_draw=COLOUR_GREEN;
							}
						}

						sprintf(buffer,"%s %s",GetTextString(TemporaryNameStore2[i]),GetTextString(TEXTSTRING_GAMESTATS_LIVEHEADBITES));
						RenderString(buffer,TABPOINT1,y,colour_to_draw);

						sprintf(buffer,"%d",CurrentGameStatistics.LiveHeadBites[i]);
						RenderStringCentred(buffer,TABPOINT2,y,colour_to_draw);

						if ((level_num<AVP_ENVIRONMENT_END_OF_LIST)&&(NotCheating)) {
							sprintf(buffer,"%d",
								UserProfilePtr->PersonalBests[AvP.Difficulty][level_num].TrophiesOrLiveHeadBites[i]
							);
							RenderStringCentred(buffer,TABPOINT3,y,colour_to_draw);
						} else {
							RenderStringCentred("---",TABPOINT3,y,COLOUR_WHITE);
						}

						colour_to_draw=COLOUR_WHITE;
						if ((completed_level)&&(level_num<AVP_ENVIRONMENT_END_OF_LIST)) {
							/* Is it a completed target? */
							if (LevelStatsTargets[AvP.Difficulty][level_num].StatTargets.TrophiesOrLiveHeadBites[i]>-1) {
								targets++;
								if (LevelStatsTargets[AvP.Difficulty][level_num].StatTargets.TrophiesOrLiveHeadBites[i]<=CurrentGameStatistics.LiveHeadBites[i]) {
									colour_to_draw=COLOUR_RED;
									targetspassed++;
								}
							}
						}
						if ((level_num<AVP_ENVIRONMENT_END_OF_LIST)&&(NotCheating)) {
							if (UserProfilePtr->CheatMode[LevelStatsTargets[AvP.Difficulty][level_num].CheatModeToActivate]!=1) {
								if (LevelStatsTargets[AvP.Difficulty][level_num].StatTargets.TrophiesOrLiveHeadBites[i]>-1) {
									sprintf(buffer,"%d",
										LevelStatsTargets[AvP.Difficulty][level_num].StatTargets.TrophiesOrLiveHeadBites[i]
									);
									RenderStringCentred(buffer,TABPOINT4,y,colour_to_draw);
								} else {
									RenderStringCentred("---",TABPOINT4,y,COLOUR_WHITE);
								}
							} else {
								RenderStringCentred("---",TABPOINT4,y,COLOUR_WHITE);
							}
						} else {
							RenderStringCentred("---",TABPOINT4,y,COLOUR_WHITE);
						}

						y+=NEWLINE_SPACING;
						colour_to_draw=COLOUR_WHITE;

						if ((completed_level)&&(level_num<AVP_ENVIRONMENT_END_OF_LIST)&&(NotCheating)) {
							/* Is it a new best? */
							if (UserProfilePtr->PersonalBests[AvP.Difficulty][level_num].DeadHeadBites[i]<=CurrentGameStatistics.DeadHeadBites[i]) {
								UserProfilePtr->PersonalBests[AvP.Difficulty][level_num].DeadHeadBites[i]=CurrentGameStatistics.DeadHeadBites[i];
								colour_to_draw=COLOUR_GREEN;
							}
						}

						sprintf(buffer,"%s %s",GetTextString(TemporaryNameStore2[i]),GetTextString(TEXTSTRING_GAMESTATS_DEADHEADBITES));
						RenderString(buffer,TABPOINT1,y,colour_to_draw);

						sprintf(buffer,"%d",CurrentGameStatistics.DeadHeadBites[i]);
						RenderStringCentred(buffer,TABPOINT2,y,colour_to_draw);

						if ((level_num<AVP_ENVIRONMENT_END_OF_LIST)&&(NotCheating)) {
							sprintf(buffer,"%d",
								UserProfilePtr->PersonalBests[AvP.Difficulty][level_num].DeadHeadBites[i]
							);
							RenderStringCentred(buffer,TABPOINT3,y,colour_to_draw);
						} else {
							RenderStringCentred("---",TABPOINT3,y,COLOUR_WHITE);
						}

						colour_to_draw=COLOUR_WHITE;
						if ((completed_level)&&(level_num<AVP_ENVIRONMENT_END_OF_LIST)) {
							/* Is it a completed target? */
							if (LevelStatsTargets[AvP.Difficulty][level_num].StatTargets.DeadHeadBites[i]>-1) {
								targets++;
								if (LevelStatsTargets[AvP.Difficulty][level_num].StatTargets.DeadHeadBites[i]<=CurrentGameStatistics.DeadHeadBites[i]) {
									colour_to_draw=COLOUR_RED;
									targetspassed++;
								}
							}
						}
						if ((level_num<AVP_ENVIRONMENT_END_OF_LIST)&&(NotCheating)) {
							if (UserProfilePtr->CheatMode[LevelStatsTargets[AvP.Difficulty][level_num].CheatModeToActivate]!=1) {
								if (LevelStatsTargets[AvP.Difficulty][level_num].StatTargets.DeadHeadBites[i]>-1) {
									sprintf(buffer,"%d",
										LevelStatsTargets[AvP.Difficulty][level_num].StatTargets.DeadHeadBites[i]
									);
									RenderStringCentred(buffer,TABPOINT4,y,colour_to_draw);
								} else {
									RenderStringCentred("---",TABPOINT4,y,COLOUR_WHITE);
								}
							} else {
								RenderStringCentred("---",TABPOINT4,y,COLOUR_WHITE);
							}
						} else {
							RenderStringCentred("---",TABPOINT4,y,COLOUR_WHITE);
						}

						y+=NEWLINE_SPACING;
						colour_to_draw=COLOUR_WHITE;
					}
				}
			}
		}
	}

	/* Speed? */
	{
		unsigned int total_time;
		unsigned int average_speed;

		float float_speed;

		total_time=(AvP.ElapsedSeconds>>ONE_FIXED_SHIFT);
		total_time+=(AvP.ElapsedMinutes*60);
		total_time+=((AvP.ElapsedHours*60)*60);

		if (total_time) {
			average_speed=(CurrentGameStatistics.IntegralSpeed/total_time);

			if ((completed_level)&&(level_num<AVP_ENVIRONMENT_END_OF_LIST)&&(NotCheating)) {
				/* Is it a new best? */
				if (UserProfilePtr->PersonalBests[AvP.Difficulty][level_num].Speed<=average_speed) {
					UserProfilePtr->PersonalBests[AvP.Difficulty][level_num].Speed=average_speed;
					colour_to_draw=COLOUR_GREEN;
				}
			}

			RenderString(GetTextString(TEXTSTRING_GAMESTATS_AVERAGESPEED),TABPOINT1,y,colour_to_draw);

			float_speed=(float)average_speed;
			float_speed/=1000;

			sprintf(buffer,"%.1f m/s",float_speed);
			RenderStringCentred(buffer,TABPOINT2,y,colour_to_draw);

			if ((level_num<AVP_ENVIRONMENT_END_OF_LIST)&&(NotCheating)) {
				float_speed=(float)UserProfilePtr->PersonalBests[AvP.Difficulty][level_num].Speed;
				float_speed/=1000;

				sprintf(buffer,"%.1f m/s",float_speed);

				RenderStringCentred(buffer,TABPOINT3,y,colour_to_draw);
			} else {
				RenderStringCentred("---",TABPOINT3,y,COLOUR_WHITE);
			}

			colour_to_draw=COLOUR_WHITE;
			if ((completed_level)&&(level_num<AVP_ENVIRONMENT_END_OF_LIST)) {
				/* Is it a completed target? */
				if (LevelStatsTargets[AvP.Difficulty][level_num].StatTargets.Speed>-1) {
					targets++;
					if (LevelStatsTargets[AvP.Difficulty][level_num].StatTargets.Speed<=average_speed) {
						colour_to_draw=COLOUR_RED;
						targetspassed++;
					}
				}
			}
			if ((level_num<AVP_ENVIRONMENT_END_OF_LIST)&&(NotCheating)) {
				if (UserProfilePtr->CheatMode[LevelStatsTargets[AvP.Difficulty][level_num].CheatModeToActivate]!=1) {
					if (LevelStatsTargets[AvP.Difficulty][level_num].StatTargets.Speed>-1) {
						float_speed=(float)LevelStatsTargets[AvP.Difficulty][level_num].StatTargets.Speed;
						float_speed/=1000;
						sprintf(buffer,"%.1f m/s",float_speed);
						RenderStringCentred(buffer,TABPOINT4,y,colour_to_draw);
					} else {
						RenderStringCentred("---",TABPOINT4,y,COLOUR_WHITE);
					}
				} else {
					RenderStringCentred("---",TABPOINT4,y,COLOUR_WHITE);
				}
			} else {
				RenderStringCentred("---",TABPOINT4,y,COLOUR_WHITE);
			}
		}
	}
	y+=NEWLINE_SPACING;
	colour_to_draw=COLOUR_WHITE;
	{
		unsigned int percentage;
		/* Health and armour. */
		if (NpcData->StartingStats.Health) {
			percentage=((CurrentGameStatistics.HealthDamage>>ONE_FIXED_SHIFT)*100)/(NpcData->StartingStats.Health);

			if ((completed_level)&&(level_num<AVP_ENVIRONMENT_END_OF_LIST)&&(NotCheating)) {
				/* Is it a new best? */
				if (UserProfilePtr->PersonalBests[AvP.Difficulty][level_num].HealthDamage>=percentage) {
					UserProfilePtr->PersonalBests[AvP.Difficulty][level_num].HealthDamage=percentage;
					colour_to_draw=COLOUR_GREEN;
				}
			}

			RenderString(GetTextString(TEXTSTRING_GAMESTATS_HEALTHDAMAGETAKEN),TABPOINT1,y,colour_to_draw);

			sprintf(buffer,"%03d%%",percentage);
			RenderStringCentred(buffer,TABPOINT2,y,colour_to_draw);

			if ((level_num<AVP_ENVIRONMENT_END_OF_LIST)&&(NotCheating)) {
				sprintf(buffer,"%03d%%",
					UserProfilePtr->PersonalBests[AvP.Difficulty][level_num].HealthDamage
				);
				RenderStringCentred(buffer,TABPOINT3,y,colour_to_draw);
			} else {
				RenderStringCentred("---",TABPOINT3,y,COLOUR_WHITE);
			}

			colour_to_draw=COLOUR_WHITE;
			if ((completed_level)&&(level_num<AVP_ENVIRONMENT_END_OF_LIST)) {
				/* Is it a completed target? */
				if (LevelStatsTargets[AvP.Difficulty][level_num].StatTargets.HealthDamage>-1) {
					targets++;
					if (LevelStatsTargets[AvP.Difficulty][level_num].StatTargets.HealthDamage>=percentage) {
						colour_to_draw=COLOUR_RED;
						targetspassed++;
					}
				}
			}
			if ((level_num<AVP_ENVIRONMENT_END_OF_LIST)&&(NotCheating)) {
				if (UserProfilePtr->CheatMode[LevelStatsTargets[AvP.Difficulty][level_num].CheatModeToActivate]!=1) {
					if (LevelStatsTargets[AvP.Difficulty][level_num].StatTargets.HealthDamage>-1) {
						sprintf(buffer,"%03d%%",
							LevelStatsTargets[AvP.Difficulty][level_num].StatTargets.HealthDamage
						);
						RenderStringCentred(buffer,TABPOINT4,y,colour_to_draw);
					} else {
						RenderStringCentred("---",TABPOINT4,y,COLOUR_WHITE);
					}
				} else {
					RenderStringCentred("---",TABPOINT4,y,COLOUR_WHITE);
				}
			} else {
				RenderStringCentred("---",TABPOINT4,y,COLOUR_WHITE);
			}

			y+=NEWLINE_SPACING;
			colour_to_draw=COLOUR_WHITE;
		}
		if ((NpcData->StartingStats.Armour)&&(AvP.PlayerType==I_Marine)) {
			percentage=((CurrentGameStatistics.ArmourDamage>>ONE_FIXED_SHIFT)*100)/(NpcData->StartingStats.Armour);

			if ((completed_level)&&(level_num<AVP_ENVIRONMENT_END_OF_LIST)&&(NotCheating)) {
				/* Is it a new best? */
				if (UserProfilePtr->PersonalBests[AvP.Difficulty][level_num].ArmourDamage>=percentage) {
					UserProfilePtr->PersonalBests[AvP.Difficulty][level_num].ArmourDamage=percentage;
					colour_to_draw=COLOUR_GREEN;
				}
			}

			RenderString(GetTextString(TEXTSTRING_GAMESTATS_ARMOURDAMAGETAKEN),TABPOINT1,y,colour_to_draw);

			sprintf(buffer,"%03d%%",percentage);
			RenderStringCentred(buffer,TABPOINT2,y,colour_to_draw);

			if ((level_num<AVP_ENVIRONMENT_END_OF_LIST)&&(NotCheating)) {
				sprintf(buffer,"%03d%%",
					UserProfilePtr->PersonalBests[AvP.Difficulty][level_num].ArmourDamage
				);
				RenderStringCentred(buffer,TABPOINT3,y,colour_to_draw);
			} else {
				RenderStringCentred("---",TABPOINT3,y,COLOUR_WHITE);
			}

			colour_to_draw=COLOUR_WHITE;
			if ((completed_level)&&(level_num<AVP_ENVIRONMENT_END_OF_LIST)) {
				/* Is it a completed target? */
				if (LevelStatsTargets[AvP.Difficulty][level_num].StatTargets.ArmourDamage>-1) {
					targets++;
					if (LevelStatsTargets[AvP.Difficulty][level_num].StatTargets.ArmourDamage>=percentage) {
						colour_to_draw=COLOUR_RED;
						targetspassed++;
					}
				}
			}
			if ((level_num<AVP_ENVIRONMENT_END_OF_LIST)&&(NotCheating)) {
				if (UserProfilePtr->CheatMode[LevelStatsTargets[AvP.Difficulty][level_num].CheatModeToActivate]!=1) {
					if (LevelStatsTargets[AvP.Difficulty][level_num].StatTargets.ArmourDamage>-1) {
						sprintf(buffer,"%03d%%",
							LevelStatsTargets[AvP.Difficulty][level_num].StatTargets.ArmourDamage
						);
						RenderStringCentred(buffer,TABPOINT4,y,colour_to_draw);
					} else {
						RenderStringCentred("---",TABPOINT4,y,COLOUR_WHITE);
					}
				} else {
					RenderStringCentred("---",TABPOINT4,y,COLOUR_WHITE);
				}
			} else {
				RenderStringCentred("---",TABPOINT4,y,COLOUR_WHITE);
			}

			y+=NEWLINE_SPACING;
			colour_to_draw=COLOUR_WHITE;
		}

	}
	/* Creature specific stats... */
	if (AvP.PlayerType==I_Marine) {
		int headshots=0,kills=0,i;

		for (i=0; i<STATS_VICTIM_MAXIMUM; i++)
		{
			headshots += CurrentGameStatistics.Decapitated[i];
			kills += CurrentGameStatistics.Killed[i];
		}
		if (kills)
		{
			headshots = (100*headshots)/kills;
		}
		else
		{
			headshots = 0;
		}

		if ((completed_level)&&(level_num<AVP_ENVIRONMENT_END_OF_LIST)&&(NotCheating)) {
			/* Is it a new best? */
			if (UserProfilePtr->PersonalBests[AvP.Difficulty][level_num].HeadShotPercentage<=headshots) {
				UserProfilePtr->PersonalBests[AvP.Difficulty][level_num].HeadShotPercentage=headshots;
				colour_to_draw=COLOUR_GREEN;
			}
		}
	
		RenderString(GetTextString(TEXTSTRING_GAMESTATS_HEADSHOTS),TABPOINT1,y,colour_to_draw);

		sprintf(buffer,"%03d%%",headshots);
		RenderStringCentred(buffer,TABPOINT2,y,colour_to_draw);

		if ((level_num<AVP_ENVIRONMENT_END_OF_LIST)&&(NotCheating)) {
			sprintf(buffer,"%03d%%",
				UserProfilePtr->PersonalBests[AvP.Difficulty][level_num].HeadShotPercentage
			);
			RenderStringCentred(buffer,TABPOINT3,y,colour_to_draw);
		} else {
			RenderStringCentred("---",TABPOINT3,y,COLOUR_WHITE);
		}

		colour_to_draw=COLOUR_WHITE;
		if ((completed_level)&&(level_num<AVP_ENVIRONMENT_END_OF_LIST)) {
			/* Is it a completed target? */
			if (LevelStatsTargets[AvP.Difficulty][level_num].StatTargets.HeadShotPercentage>-1) {
				targets++;
				if (LevelStatsTargets[AvP.Difficulty][level_num].StatTargets.HeadShotPercentage<=headshots) {
					colour_to_draw=COLOUR_RED;
					targetspassed++;
				}
			}
		}
		if ((level_num<AVP_ENVIRONMENT_END_OF_LIST)&&(NotCheating)) {
			if (UserProfilePtr->CheatMode[LevelStatsTargets[AvP.Difficulty][level_num].CheatModeToActivate]!=1) {
				if (LevelStatsTargets[AvP.Difficulty][level_num].StatTargets.HeadShotPercentage>-1) {
					sprintf(buffer,"%03d%%",
						LevelStatsTargets[AvP.Difficulty][level_num].StatTargets.HeadShotPercentage
					);
					RenderStringCentred(buffer,TABPOINT4,y,colour_to_draw);
				} else {
					RenderStringCentred("---",TABPOINT4,y,COLOUR_WHITE);
				}
			} else {
				RenderStringCentred("---",TABPOINT4,y,COLOUR_WHITE);
			}
		} else {
			RenderStringCentred("---",TABPOINT4,y,COLOUR_WHITE);
		}

		y+=NEWLINE_SPACING;
		colour_to_draw=COLOUR_WHITE;
		/* Shots fired... */
		{
			unsigned int total_shots;

			total_shots=0;
			
			for (i=0; i<MAX_NO_OF_WEAPON_SLOTS; i++) {
				PLAYER_WEAPON_DATA *weaponPtr;
				TEMPLATE_WEAPON_DATA *twPtr;

			    weaponPtr = &(PlayerStatusPtr->WeaponSlot[i]);
			    twPtr = &TemplateWeapon[weaponPtr->WeaponIDNumber];
				
				if (twPtr->LogShots) {
					total_shots+=CurrentGameStatistics.ShotsFired[i];
				}
			}

			if ((completed_level)&&(level_num<AVP_ENVIRONMENT_END_OF_LIST)&&(NotCheating)) {
				/* Is it a new best? */
				if (UserProfilePtr->PersonalBests[AvP.Difficulty][level_num].ShotsFired>=total_shots) {
					UserProfilePtr->PersonalBests[AvP.Difficulty][level_num].ShotsFired=total_shots;
					colour_to_draw=COLOUR_GREEN;
				}
			}

			RenderString(GetTextString(TEXTSTRING_GAMESTATS_TOTALSHOTSFIRED),TABPOINT1,y,colour_to_draw);
			sprintf(buffer,"%d\n",(total_shots));
			RenderStringCentred(buffer,TABPOINT2,y,colour_to_draw);

			if ((level_num<AVP_ENVIRONMENT_END_OF_LIST)&&(NotCheating)) {
				sprintf(buffer,"%d",
					UserProfilePtr->PersonalBests[AvP.Difficulty][level_num].ShotsFired
				);
				RenderStringCentred(buffer,TABPOINT3,y,colour_to_draw);
			} else {
				RenderStringCentred("---",TABPOINT3,y,COLOUR_WHITE);
			}

			colour_to_draw=COLOUR_WHITE;
			if ((completed_level)&&(level_num<AVP_ENVIRONMENT_END_OF_LIST)) {
				/* Is it a completed target? */
				if (LevelStatsTargets[AvP.Difficulty][level_num].StatTargets.ShotsFired>-1) {
					targets++;
					if (LevelStatsTargets[AvP.Difficulty][level_num].StatTargets.ShotsFired>=total_shots) {
						colour_to_draw=COLOUR_RED;
						targetspassed++;
					}
				}
			}
			if ((level_num<AVP_ENVIRONMENT_END_OF_LIST)&&(NotCheating)) {
				if (UserProfilePtr->CheatMode[LevelStatsTargets[AvP.Difficulty][level_num].CheatModeToActivate]!=1) {
					if (LevelStatsTargets[AvP.Difficulty][level_num].StatTargets.ShotsFired>-1) {
						sprintf(buffer,"%d",
							LevelStatsTargets[AvP.Difficulty][level_num].StatTargets.ShotsFired
						);
						RenderStringCentred(buffer,TABPOINT4,y,colour_to_draw);
					} else {
						RenderStringCentred("---",TABPOINT4,y,COLOUR_WHITE);
					}
				} else {
					RenderStringCentred("---",TABPOINT4,y,COLOUR_WHITE);
				}
			} else {
				RenderStringCentred("---",TABPOINT4,y,COLOUR_WHITE);
			}

			y+=NEWLINE_SPACING;
			colour_to_draw=COLOUR_WHITE;
		}
		/* Accuracy... */
		{
			unsigned int total_shots,total_hits,percentage;

			total_shots=0;
			total_hits=0;
			
			for (i=0; i<MAX_NO_OF_WEAPON_SLOTS; i++) {
				PLAYER_WEAPON_DATA *weaponPtr;
				TEMPLATE_WEAPON_DATA *twPtr;

			    weaponPtr = &(PlayerStatusPtr->WeaponSlot[i]);
			    twPtr = &TemplateWeapon[weaponPtr->WeaponIDNumber];
				
				if (twPtr->LogAccuracy) {
					total_shots+=CurrentGameStatistics.ShotsFired[i];
					total_hits+=CurrentGameStatistics.ShotsHit[i];
				}
			}
			if (total_shots) {
				percentage=(100*total_hits)/total_shots;
			} else {
				percentage=0;
			}

			if ((completed_level)&&(level_num<AVP_ENVIRONMENT_END_OF_LIST)&&(NotCheating)) {
				/* Is it a new best? */
				if (UserProfilePtr->PersonalBests[AvP.Difficulty][level_num].Accuracy<=percentage) {
					UserProfilePtr->PersonalBests[AvP.Difficulty][level_num].Accuracy=percentage;
					colour_to_draw=COLOUR_GREEN;
				}
			}

			RenderString(GetTextString(TEXTSTRING_GAMESTATS_ACCURACY),TABPOINT1,y,colour_to_draw);

			sprintf(buffer,"%03d%%",percentage);
			RenderStringCentred(buffer,TABPOINT2,y,colour_to_draw);

			if ((level_num<AVP_ENVIRONMENT_END_OF_LIST)&&(NotCheating)) {
				sprintf(buffer,"%03d%%",
					UserProfilePtr->PersonalBests[AvP.Difficulty][level_num].Accuracy
				);
				RenderStringCentred(buffer,TABPOINT3,y,colour_to_draw);
			} else {
				RenderStringCentred("---",TABPOINT3,y,COLOUR_WHITE);
			}

			colour_to_draw=COLOUR_WHITE;
			if ((completed_level)&&(level_num<AVP_ENVIRONMENT_END_OF_LIST)) {
				/* Is it a completed target? */
				if (LevelStatsTargets[AvP.Difficulty][level_num].StatTargets.Accuracy>-1) {
					targets++;
					if (LevelStatsTargets[AvP.Difficulty][level_num].StatTargets.Accuracy<=percentage) {
						colour_to_draw=COLOUR_RED;
						targetspassed++;
					}
				}
			}
			if ((level_num<AVP_ENVIRONMENT_END_OF_LIST)&&(NotCheating)) {
				if (UserProfilePtr->CheatMode[LevelStatsTargets[AvP.Difficulty][level_num].CheatModeToActivate]!=1) {
					if (LevelStatsTargets[AvP.Difficulty][level_num].StatTargets.Accuracy>-1) {
						sprintf(buffer,"%03d%%",
							LevelStatsTargets[AvP.Difficulty][level_num].StatTargets.Accuracy
						);
						RenderStringCentred(buffer,TABPOINT4,y,colour_to_draw);
					} else {
						RenderStringCentred("---",TABPOINT4,y,COLOUR_WHITE);
					}
				} else {
					RenderStringCentred("---",TABPOINT4,y,COLOUR_WHITE);
				}
			} else {
				RenderStringCentred("---",TABPOINT4,y,COLOUR_WHITE);
			}

			y+=NEWLINE_SPACING;
			colour_to_draw=COLOUR_WHITE;
		}
		/* Preferred weapon... */
		{
			unsigned int maxtime=0;
			int preferred_slot=-1;
			PLAYER_WEAPON_DATA *weaponPtr;

			for (i=0; i<MAX_NO_OF_WEAPON_SLOTS; i++) {
				if (CurrentGameStatistics.WeaponTimes[i]>=maxtime) {
					maxtime=CurrentGameStatistics.WeaponTimes[i];
					preferred_slot=i;
				}
			}
			GLOBALASSERT(preferred_slot!=-1);
		
		    weaponPtr = &(PlayerStatusPtr->WeaponSlot[preferred_slot]);

			RenderString(GetTextString(TEXTSTRING_GAMESTATS_PREFERREDWEAPON),TABPOINT1,y,colour_to_draw);
			sprintf(buffer,"%s\n",GetTextString(TemplateWeapon[weaponPtr->WeaponIDNumber].Name));			
			RenderStringCentred(buffer,TABPOINT2,y,colour_to_draw);

			RenderStringCentred("---",TABPOINT3,y,colour_to_draw);
			RenderStringCentred("---",TABPOINT4,y,COLOUR_WHITE);

			y+=NEWLINE_SPACING;
			colour_to_draw=COLOUR_WHITE;
		}
	} else if (AvP.PlayerType==I_Alien) {

		if ((completed_level)&&(level_num<AVP_ENVIRONMENT_END_OF_LIST)&&(NotCheating)) {
			/* Is it a new best? */
			if (UserProfilePtr->PersonalBests[AvP.Difficulty][level_num].Spotted>=CurrentGameStatistics.Spotted) {
				UserProfilePtr->PersonalBests[AvP.Difficulty][level_num].Spotted=CurrentGameStatistics.Spotted;
				colour_to_draw=COLOUR_GREEN;
			}
		}
		
		RenderString(GetTextString(TEXTSTRING_GAMESTATS_SPOTTED),TABPOINT1,y,colour_to_draw);

		sprintf(buffer,"%d",CurrentGameStatistics.Spotted);
		RenderStringCentred(buffer,TABPOINT2,y,colour_to_draw);

		if ((level_num<AVP_ENVIRONMENT_END_OF_LIST)&&(NotCheating)) {
			sprintf(buffer,"%d",
				UserProfilePtr->PersonalBests[AvP.Difficulty][level_num].Spotted
			);
			RenderStringCentred(buffer,TABPOINT3,y,colour_to_draw);
		} else {
			RenderStringCentred("---",TABPOINT3,y,COLOUR_WHITE);
		}

		colour_to_draw=COLOUR_WHITE;
		if ((completed_level)&&(level_num<AVP_ENVIRONMENT_END_OF_LIST)) {
			/* Is it a completed target? */
			if (LevelStatsTargets[AvP.Difficulty][level_num].StatTargets.Spotted>-1) {
				targets++;
				if (LevelStatsTargets[AvP.Difficulty][level_num].StatTargets.Spotted>=CurrentGameStatistics.Spotted) {
					colour_to_draw=COLOUR_RED;
					targetspassed++;
				}
			}
		}
		if ((level_num<AVP_ENVIRONMENT_END_OF_LIST)&&(NotCheating)) {
			if (UserProfilePtr->CheatMode[LevelStatsTargets[AvP.Difficulty][level_num].CheatModeToActivate]!=1) {
				if (LevelStatsTargets[AvP.Difficulty][level_num].StatTargets.Spotted>-1) {
					sprintf(buffer,"%d",
						LevelStatsTargets[AvP.Difficulty][level_num].StatTargets.Spotted
					);
					RenderStringCentred(buffer,TABPOINT4,y,colour_to_draw);
				} else {
					RenderStringCentred("---",TABPOINT4,y,COLOUR_WHITE);
				}
			} else {
				RenderStringCentred("---",TABPOINT4,y,COLOUR_WHITE);
			}
		} else {
			RenderStringCentred("---",TABPOINT4,y,COLOUR_WHITE);
		}

		y+=NEWLINE_SPACING;
		colour_to_draw=COLOUR_WHITE;

	} else if (AvP.PlayerType==I_Predator) {

		int i;

		if ((completed_level)&&(level_num<AVP_ENVIRONMENT_END_OF_LIST)&&(NotCheating)) {
			/* Is it a new best? */
			if (UserProfilePtr->PersonalBests[AvP.Difficulty][level_num].Spotted>=CurrentGameStatistics.Spotted) {
				UserProfilePtr->PersonalBests[AvP.Difficulty][level_num].Spotted=CurrentGameStatistics.Spotted;
				colour_to_draw=COLOUR_GREEN;
			}
		}

		RenderString(GetTextString(TEXTSTRING_GAMESTATS_SPOTTED),TABPOINT1,y,colour_to_draw);

		sprintf(buffer,"%d",CurrentGameStatistics.Spotted);
		RenderStringCentred(buffer,TABPOINT2,y,colour_to_draw);

		if ((level_num<AVP_ENVIRONMENT_END_OF_LIST)&&(NotCheating)) {
			sprintf(buffer,"%d",
				UserProfilePtr->PersonalBests[AvP.Difficulty][level_num].Spotted
			);
			RenderStringCentred(buffer,TABPOINT3,y,colour_to_draw);
		} else {
			RenderStringCentred("---",TABPOINT3,y,COLOUR_WHITE);
		}

		colour_to_draw=COLOUR_WHITE;
		if ((completed_level)&&(level_num<AVP_ENVIRONMENT_END_OF_LIST)) {
			/* Is it a completed target? */
			if (LevelStatsTargets[AvP.Difficulty][level_num].StatTargets.Spotted>-1) {
				targets++;
				if (LevelStatsTargets[AvP.Difficulty][level_num].StatTargets.Spotted>=CurrentGameStatistics.Spotted) {
					colour_to_draw=COLOUR_RED;
					targetspassed++;
				}
			}
		}
		if ((level_num<AVP_ENVIRONMENT_END_OF_LIST)&&(NotCheating)) {
			if (UserProfilePtr->CheatMode[LevelStatsTargets[AvP.Difficulty][level_num].CheatModeToActivate]!=1) {
				if (LevelStatsTargets[AvP.Difficulty][level_num].StatTargets.Spotted>-1) {
					sprintf(buffer,"%d",
						LevelStatsTargets[AvP.Difficulty][level_num].StatTargets.Spotted
					);
					RenderStringCentred(buffer,TABPOINT4,y,colour_to_draw);
				} else {
					RenderStringCentred("---",TABPOINT4,y,COLOUR_WHITE);
				}
			} else {
				RenderStringCentred("---",TABPOINT4,y,COLOUR_WHITE);
			}
		} else {
			RenderStringCentred("---",TABPOINT4,y,COLOUR_WHITE);
		}

		y+=NEWLINE_SPACING;
		colour_to_draw=COLOUR_WHITE;

		/* FieldCharge... */
		{
			unsigned int percentage;

			percentage=CurrentGameStatistics.FieldChargeUsed/(PLAYERCLOAK_MAXENERGY/100);

			if ((completed_level)&&(level_num<AVP_ENVIRONMENT_END_OF_LIST)&&(NotCheating)) {
				/* Is it a new best? */
				if (UserProfilePtr->PersonalBests[AvP.Difficulty][level_num].FieldChargeUsed>=percentage) {
					UserProfilePtr->PersonalBests[AvP.Difficulty][level_num].FieldChargeUsed=percentage;
					colour_to_draw=COLOUR_GREEN;
				}
			}

			RenderString(GetTextString(TEXTSTRING_GAMESTATS_FIELDCHARGEUSED),TABPOINT1,y,colour_to_draw);

			sprintf(buffer,"%03d%%",percentage);
			RenderStringCentred(buffer,TABPOINT2,y,colour_to_draw);

			if ((level_num<AVP_ENVIRONMENT_END_OF_LIST)&&(NotCheating)) {
				sprintf(buffer,"%03d%%",
					UserProfilePtr->PersonalBests[AvP.Difficulty][level_num].FieldChargeUsed
				);
				RenderStringCentred(buffer,TABPOINT3,y,colour_to_draw);
			} else {
				RenderStringCentred("---",TABPOINT3,y,COLOUR_WHITE);
			}

			colour_to_draw=COLOUR_WHITE;
			if ((completed_level)&&(level_num<AVP_ENVIRONMENT_END_OF_LIST)) {
				/* Is it a completed target? */
				if (LevelStatsTargets[AvP.Difficulty][level_num].StatTargets.FieldChargeUsed>-1) {
					targets++;
					if (LevelStatsTargets[AvP.Difficulty][level_num].StatTargets.FieldChargeUsed>=percentage) {
						colour_to_draw=COLOUR_RED;
						targetspassed++;
					}
				}
			}
			if ((level_num<AVP_ENVIRONMENT_END_OF_LIST)&&(NotCheating)) {
				if (UserProfilePtr->CheatMode[LevelStatsTargets[AvP.Difficulty][level_num].CheatModeToActivate]!=1) {
					if (LevelStatsTargets[AvP.Difficulty][level_num].StatTargets.FieldChargeUsed>-1) {
						sprintf(buffer,"%03d%%",
							LevelStatsTargets[AvP.Difficulty][level_num].StatTargets.FieldChargeUsed
						);
						RenderStringCentred(buffer,TABPOINT4,y,colour_to_draw);
					} else {
						RenderStringCentred("---",TABPOINT4,y,COLOUR_WHITE);
					}
				} else {
					RenderStringCentred("---",TABPOINT4,y,COLOUR_WHITE);
				}
			} else {
				RenderStringCentred("---",TABPOINT4,y,COLOUR_WHITE);
			}

			y+=NEWLINE_SPACING;
			colour_to_draw=COLOUR_WHITE;

		}
		/* Shots fired... */
		{
			unsigned int total_shots;

			total_shots=0;
			
			for (i=0; i<MAX_NO_OF_WEAPON_SLOTS; i++) {
				PLAYER_WEAPON_DATA *weaponPtr;
				TEMPLATE_WEAPON_DATA *twPtr;

			    weaponPtr = &(PlayerStatusPtr->WeaponSlot[i]);
			    twPtr = &TemplateWeapon[weaponPtr->WeaponIDNumber];
				
				if (twPtr->LogShots) {
					total_shots+=CurrentGameStatistics.ShotsFired[i];
				}
			}

			if ((completed_level)&&(level_num<AVP_ENVIRONMENT_END_OF_LIST)&&(NotCheating)) {
				/* Is it a new best? */
				if (UserProfilePtr->PersonalBests[AvP.Difficulty][level_num].ShotsFired>=total_shots) {
					UserProfilePtr->PersonalBests[AvP.Difficulty][level_num].ShotsFired=total_shots;
					colour_to_draw=COLOUR_GREEN;
				}
			}

			RenderString(GetTextString(TEXTSTRING_GAMESTATS_TOTALSHOTSFIRED),TABPOINT1,y,colour_to_draw);

			sprintf(buffer,"%d",(total_shots));
			RenderStringCentred(buffer,TABPOINT2,y,colour_to_draw);

			if ((level_num<AVP_ENVIRONMENT_END_OF_LIST)&&(NotCheating)) {
				sprintf(buffer,"%d",
					UserProfilePtr->PersonalBests[AvP.Difficulty][level_num].ShotsFired
				);
				RenderStringCentred(buffer,TABPOINT3,y,colour_to_draw);
			} else {
				RenderStringCentred("---",TABPOINT3,y,COLOUR_WHITE);
			}

			colour_to_draw=COLOUR_WHITE;
			if ((completed_level)&&(level_num<AVP_ENVIRONMENT_END_OF_LIST)) {
				/* Is it a completed target? */
				if (LevelStatsTargets[AvP.Difficulty][level_num].StatTargets.ShotsFired>-1) {
					targets++;
					if (LevelStatsTargets[AvP.Difficulty][level_num].StatTargets.ShotsFired>=total_shots) {
						colour_to_draw=COLOUR_RED;
						targetspassed++;
					}
				}
			}
			if ((level_num<AVP_ENVIRONMENT_END_OF_LIST)&&(NotCheating)) {
				if (UserProfilePtr->CheatMode[LevelStatsTargets[AvP.Difficulty][level_num].CheatModeToActivate]!=1) {
					if (LevelStatsTargets[AvP.Difficulty][level_num].StatTargets.ShotsFired>-1) {
						sprintf(buffer,"%d",
							LevelStatsTargets[AvP.Difficulty][level_num].StatTargets.ShotsFired
						);
						RenderStringCentred(buffer,TABPOINT4,y,colour_to_draw);
					} else {
						RenderStringCentred("---",TABPOINT4,y,COLOUR_WHITE);
					}
				} else {
					RenderStringCentred("---",TABPOINT4,y,COLOUR_WHITE);
				}
			} else {
				RenderStringCentred("---",TABPOINT4,y,COLOUR_WHITE);
			}

			y+=NEWLINE_SPACING;
			colour_to_draw=COLOUR_WHITE;
		}
		/* Accuracy... */
		{
			unsigned int total_shots,total_hits,percentage;

			total_shots=0;
			total_hits=0;
			
			for (i=0; i<MAX_NO_OF_WEAPON_SLOTS; i++) {
				PLAYER_WEAPON_DATA *weaponPtr;
				TEMPLATE_WEAPON_DATA *twPtr;

			    weaponPtr = &(PlayerStatusPtr->WeaponSlot[i]);
			    twPtr = &TemplateWeapon[weaponPtr->WeaponIDNumber];
				
				if (twPtr->LogAccuracy) {
					total_shots+=CurrentGameStatistics.ShotsFired[i];
					total_hits+=CurrentGameStatistics.ShotsHit[i];
				}
			}
			if (total_shots) {
				percentage=(100*total_hits)/total_shots;
			} else {
				percentage=0;
			}

			if ((completed_level)&&(level_num<AVP_ENVIRONMENT_END_OF_LIST)&&(NotCheating)) {
				/* Is it a new best? */
				if (UserProfilePtr->PersonalBests[AvP.Difficulty][level_num].Accuracy<=percentage) {
					UserProfilePtr->PersonalBests[AvP.Difficulty][level_num].Accuracy=percentage;
					colour_to_draw=COLOUR_GREEN;
				}
			}

			RenderString(GetTextString(TEXTSTRING_GAMESTATS_ACCURACY),TABPOINT1,y,colour_to_draw);

			sprintf(buffer,"%03d%%",percentage);
			RenderStringCentred(buffer,TABPOINT2,y,colour_to_draw);

			if ((level_num<AVP_ENVIRONMENT_END_OF_LIST)&&(NotCheating)) {
				sprintf(buffer,"%03d%%",
					UserProfilePtr->PersonalBests[AvP.Difficulty][level_num].Accuracy
				);
				RenderStringCentred(buffer,TABPOINT3,y,colour_to_draw);
			} else {
				RenderStringCentred("---",TABPOINT3,y,COLOUR_WHITE);
			}

			colour_to_draw=COLOUR_WHITE;
			if ((completed_level)&&(level_num<AVP_ENVIRONMENT_END_OF_LIST)) {
				/* Is it a completed target? */
				if (LevelStatsTargets[AvP.Difficulty][level_num].StatTargets.Accuracy>-1) {
					targets++;
					if (LevelStatsTargets[AvP.Difficulty][level_num].StatTargets.Accuracy<=percentage) {
						colour_to_draw=COLOUR_RED;
						targetspassed++;
					}
				}
			}
			if ((level_num<AVP_ENVIRONMENT_END_OF_LIST)&&(NotCheating)) {
				if (UserProfilePtr->CheatMode[LevelStatsTargets[AvP.Difficulty][level_num].CheatModeToActivate]!=1) {
					if (LevelStatsTargets[AvP.Difficulty][level_num].StatTargets.Accuracy>-1) {
						sprintf(buffer,"%03d%%",
							LevelStatsTargets[AvP.Difficulty][level_num].StatTargets.Accuracy
						);
						RenderStringCentred(buffer,TABPOINT4,y,colour_to_draw);
					} else {
						RenderStringCentred("---",TABPOINT4,y,COLOUR_WHITE);
					}
				} else {
					RenderStringCentred("---",TABPOINT4,y,COLOUR_WHITE);
				}
			} else {
				RenderStringCentred("---",TABPOINT4,y,COLOUR_WHITE);
			}

			y+=NEWLINE_SPACING;
			colour_to_draw=COLOUR_WHITE;
		}
		/* Preferred weapon... */
		{
			unsigned int maxtime=0;
			int preferred_slot=-1;
			PLAYER_WEAPON_DATA *weaponPtr;

			for (i=0; i<MAX_NO_OF_WEAPON_SLOTS; i++) {
				if (CurrentGameStatistics.WeaponTimes[i]>=maxtime) {
					maxtime=CurrentGameStatistics.WeaponTimes[i];
					preferred_slot=i;
				}
			}
			GLOBALASSERT(preferred_slot!=-1);
		
		    weaponPtr = &(PlayerStatusPtr->WeaponSlot[preferred_slot]);

			RenderString(GetTextString(TEXTSTRING_GAMESTATS_PREFERREDWEAPON),TABPOINT1,y,colour_to_draw);

			sprintf(buffer,"%s",GetTextString(TemplateWeapon[weaponPtr->WeaponIDNumber].Name));			
			RenderStringCentred(buffer,TABPOINT2,y,colour_to_draw);

			RenderStringCentred("---",TABPOINT3,y,colour_to_draw);
			RenderStringCentred("---",TABPOINT4,y,COLOUR_WHITE);

			y+=NEWLINE_SPACING;
			colour_to_draw=COLOUR_WHITE;
		}
	}

	/* Vision Mode... */
	{
		int i;
		unsigned int maxtime=0;
		int preferred_slot=-1;

		for (i=0; i<NUMBER_OF_VISION_MODES; i++) {
			if (CurrentGameStatistics.VisionModeTimes[i]>=maxtime) {
				maxtime=CurrentGameStatistics.VisionModeTimes[i];
				preferred_slot=i;
			}
		}
		GLOBALASSERT(preferred_slot!=-1);

		RenderString(GetTextString(TEXTSTRING_GAMESTATS_PREFERREDVISIONMODE),TABPOINT1,y,colour_to_draw);
		sprintf(buffer,"%s\n",GetTextString(VisionModeNames[preferred_slot]));
		RenderStringCentred(buffer,TABPOINT2,y,colour_to_draw);

		RenderStringCentred("---",TABPOINT3,y,colour_to_draw);
		RenderStringCentred("---",TABPOINT4,y,COLOUR_WHITE);

	}
	y+=NEWLINE_SPACING;
	colour_to_draw=COLOUR_WHITE;

	/* Unlock a cheat mode? */
	
	if (NotCheating) {
		if (targets) {
			if (targets==targetspassed) {
				if (LevelStatsTargets[AvP.Difficulty][level_num].CheatModeToActivate!=CHEATMODE_NONACTIVE) {
					if (UserProfilePtr->CheatMode[LevelStatsTargets[AvP.Difficulty][level_num].CheatModeToActivate]!=1) {
						UserProfilePtr->CheatMode[LevelStatsTargets[AvP.Difficulty][level_num].CheatModeToActivate]=2;
						RenderStringCentred(GetTextString(TEXTSTRING_GAMESTATS_CHEATMODEENABLED),ScreenDescriptorBlock.SDB_Width/2,y,COLOUR_GREEN);
					}
				}
			}
		}
	}
}
