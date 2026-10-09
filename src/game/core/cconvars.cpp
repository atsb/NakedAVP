/* KJL 11:10:15 28/01/98 - 

	This file contains game-specific console variables
	
 */
#include "3dc.h"
#include "inline.h"
#include "module.h"
#include "stratdef.h"
#include "gamedef.h"
#include "davehook.h"
#include "r2base.h"
#include "gadget.h"
#include "daemon.h"
#include "rentrntq.h"

#include "bh_types.h"
#include "consvar.hpp"
#include "conscmnd.hpp"
#include "equipmnt.h"
#include "weapons.h"
#include "bh_queen.h"
#include "bh_gener.h"
#include "dxlog.h"
#include "avp_menus.h"
#include "pheromon.h"
#include "showcmds.h"
#include "pfarlocs.h"

#define UseLocalAssert Yes
#include "ourasert.h"

#include "game_statistics.h"
#include "avp_envinfo.h"
#include "avp_userprofile.h"

extern "C"
{

extern int Simplify_HModel_Rendering;

extern int TrickleCharge;
extern int CloakDrain;
extern int CloakThreshold;
extern int CloakPowerOnDrain;
extern int GlobalGoreRate;
extern int PredPistol_ShotCost;
extern int PredPistolBoltSpeed;
extern int PredPistolBoltGravity;
extern int Caster_Jumpstart;
extern int Caster_Chargetime;
extern int Caster_TrickleRate;
extern int Caster_TrickleLevel;
extern int Caster_ChargeRatio;
extern int Caster_NPCKill;
extern int Caster_PCKill;
extern int Caster_BlastRadius;
extern int Caster_MinCharge;
extern int RecallDisc_Charge;
extern void Recall_Disc(void);

extern int Marine_Skill;
extern int Marine_Terminal_Velocity;

extern int ShowPredoStats;
extern int ShowSquadState;
extern int ShowNearSquad;
extern int ShowHiveState;
extern int ShowXenoStats;
extern int ShowSlack;
extern void ActivateHive(void);
extern void DeActivateHive(void);
extern void ForceAGenerator_Shell(void);
extern void StrikeTime(int time);
extern void AlienStrikeTime(int time);
extern void CastSentrygun(void);
extern void CastDummy(void);
extern void StartPlayerTaunt(void);
extern void Console_ZoneAlert(int input);
extern void ZapSlack(void);
/* Queen commands */
extern int Queen_Charge_Rate;
extern int Queen_Step_Time;
extern int Queen_Step_Speed;
extern int Queen_Step_Mode;
extern int Queen_Turn_Rate;
extern void QComm_Stop(void);
extern void QComm_StepForward(void);
extern void QComm_StepBack(void);
extern void QComm_TurnLeft(void);
extern void QComm_TurnRight(void);
extern void QComm_Heel(void);
extern void QComm_Taunt(void);
extern void QComm_Hiss(void);
extern void QComm_LeftSwipe(void);
extern void QComm_RightSwipe(void);
extern void QComm_Route(void);
extern void QComm_Charge(void);
/* Tail calibration */
extern int tail_xcal;
extern int tail_ycal;
/* Molotov Callibration! */
extern int mx,my,mz;

extern int RATweak;

extern TEMPLATE_WEAPON_DATA	TemplateWeapon[MAX_NO_OF_WEAPON_TEMPLATES];
extern void NewOnScreenMessage(unsigned char *messagePtr);

extern int GlobalAmbience;
int Old_GlobalAmbience=ONE_FIXED;

int ShowAdj=0;

char ccv_tempstring[128];

void Toggle_Ambience(void) {

	int buffer;

	buffer=GlobalAmbience;
	GlobalAmbience=Old_GlobalAmbience;
	Old_GlobalAmbience=buffer;	

}

void ActivateAllGenerators(void) {
	int a;
	STRATEGYBLOCK *candidate;

	for (a=0; a<NumActiveStBlocks; a++) {
		candidate=ActiveStBlockList[a];
		if (candidate->I_SBtype==I_BehaviourGenerator) {
			GENERATOR_BLOCK *genBlock;

			genBlock=(GENERATOR_BLOCK *)candidate->SBdataptr;
			genBlock->Active=1;
		}
	}
}

void KillAllInanimates(void) {
	int a;
	STRATEGYBLOCK *candidate;

	for (a=0; a<NumActiveStBlocks; a++) {
		candidate=ActiveStBlockList[a];
		if (candidate->I_SBtype==I_BehaviourInanimateObject) {
			candidate->SBflags.please_destroy_me=1;
		}
	}
}

void KillAllMarines(void) {
	int a;
	STRATEGYBLOCK *candidate;

	for (a=0; a<NumActiveStBlocks; a++) {
		candidate=ActiveStBlockList[a];
		if ((candidate->I_SBtype==I_BehaviourMarine)
		 ||(candidate->I_SBtype==I_BehaviourSeal)) {
			CauseDamageToObject(candidate,&console_nuke,ONE_FIXED,NULL);
		}
	}
}

void KillAllAliens(void) {
	int a;
	STRATEGYBLOCK *candidate;

	for (a=0; a<NumActiveStBlocks; a++) {
		candidate=ActiveStBlockList[a];
		if (candidate->I_SBtype==I_BehaviourAlien) {
			CauseDamageToObject(candidate,&console_nuke,ONE_FIXED,NULL);
		}
	}
}

void KillAllPreds(void) {
	int a;
	STRATEGYBLOCK *candidate;

	for (a=0; a<NumActiveStBlocks; a++) {
		candidate=ActiveStBlockList[a];
		if (candidate->I_SBtype==I_BehaviourPredator) {
			CauseDamageToObject(candidate,&console_nuke,ONE_FIXED,NULL);
		}
	}
}

void KillAllDummies(void) {
	int a;
	STRATEGYBLOCK *candidate;

	for (a=0; a<NumActiveStBlocks; a++) {
		candidate=ActiveStBlockList[a];
		if (candidate->I_SBtype==I_BehaviourDummy) {
			DestroyAnyStrategyBlock(candidate);
		}
	}
}

void ToggleShapeRender(void) {
	Simplify_HModel_Rendering=(~Simplify_HModel_Rendering);
}

void Show_PredOStats(void) {
	ShowPredoStats=(~ShowPredoStats);
}

void Show_XenoStats(void) {
	ShowXenoStats=(~ShowXenoStats);
}

void Show_SquadState(void) {
	ShowSquadState=(~ShowSquadState);
}

void Show_NearSquad(void) {
	ShowNearSquad=(~ShowNearSquad);
}

void Show_HiveState(void) {
	ShowHiveState=(~ShowHiveState);
}

void Show_Slack(void) {
	ShowSlack=(~ShowSlack);
}

void Show_Adj(void) {
	ShowAdj=(~ShowAdj);
}

void Toggle_Observer(void) {
	Observer=(~Observer);
}

void ShowRecoilMaxXTilt(void) {

	PLAYER_WEAPON_DATA *weaponPtr;
	TEMPLATE_WEAPON_DATA *twPtr;
	PLAYER_STATUS *playerStatusPtr= (PLAYER_STATUS *) (Player->ObStrategyBlock->SBdataptr);
    GLOBALASSERT(playerStatusPtr);
    weaponPtr = &(playerStatusPtr->WeaponSlot[playerStatusPtr->SelectedWeaponSlot]);
    twPtr = &TemplateWeapon[weaponPtr->WeaponIDNumber];
	
	sprintf(ccv_tempstring,"RECOILMAXXTILT = %d\n",twPtr->RecoilMaxXTilt);

	NewOnScreenMessage((unsigned char *)&ccv_tempstring[0]);
}

void ShowRecoilMaxYTilt(void) {

	PLAYER_WEAPON_DATA *weaponPtr;
	TEMPLATE_WEAPON_DATA *twPtr;
	PLAYER_STATUS *playerStatusPtr= (PLAYER_STATUS *) (Player->ObStrategyBlock->SBdataptr);
    GLOBALASSERT(playerStatusPtr);
    weaponPtr = &(playerStatusPtr->WeaponSlot[playerStatusPtr->SelectedWeaponSlot]);
    twPtr = &TemplateWeapon[weaponPtr->WeaponIDNumber];

	sprintf(ccv_tempstring,"RECOILMAXYTILT = %d\n",twPtr->RecoilMaxYTilt);

	NewOnScreenMessage((unsigned char *)&ccv_tempstring[0]);
}

void ShowRecoilMaxZ(void) {

	PLAYER_WEAPON_DATA *weaponPtr;
	TEMPLATE_WEAPON_DATA *twPtr;
	PLAYER_STATUS *playerStatusPtr= (PLAYER_STATUS *) (Player->ObStrategyBlock->SBdataptr);
    GLOBALASSERT(playerStatusPtr);
    weaponPtr = &(playerStatusPtr->WeaponSlot[playerStatusPtr->SelectedWeaponSlot]);
    twPtr = &TemplateWeapon[weaponPtr->WeaponIDNumber];

	sprintf(ccv_tempstring,"RECOILMAXZ = %d\n",twPtr->RecoilMaxZ);

	NewOnScreenMessage((unsigned char *)&ccv_tempstring[0]);
}

void ShowRecoilMaxRandomZ(void) {

	PLAYER_WEAPON_DATA *weaponPtr;
	TEMPLATE_WEAPON_DATA *twPtr;
	PLAYER_STATUS *playerStatusPtr= (PLAYER_STATUS *) (Player->ObStrategyBlock->SBdataptr);
    GLOBALASSERT(playerStatusPtr);
    weaponPtr = &(playerStatusPtr->WeaponSlot[playerStatusPtr->SelectedWeaponSlot]);
    twPtr = &TemplateWeapon[weaponPtr->WeaponIDNumber];

	sprintf(ccv_tempstring,"RECOILMAXRANDOMZ = %d\n",twPtr->RecoilMaxRandomZ);

	NewOnScreenMessage((unsigned char *)&ccv_tempstring[0]);
}

void SetRecoilMaxXTilt(int value) {

	PLAYER_WEAPON_DATA *weaponPtr;
	TEMPLATE_WEAPON_DATA *twPtr;
	PLAYER_STATUS *playerStatusPtr= (PLAYER_STATUS *) (Player->ObStrategyBlock->SBdataptr);
    GLOBALASSERT(playerStatusPtr);
    weaponPtr = &(playerStatusPtr->WeaponSlot[playerStatusPtr->SelectedWeaponSlot]);
    twPtr = &TemplateWeapon[weaponPtr->WeaponIDNumber];
	
	twPtr->RecoilMaxXTilt=value;

	sprintf(ccv_tempstring,"RECOILMAXXTILT = %d\n",twPtr->RecoilMaxXTilt);

	NewOnScreenMessage((unsigned char *)&ccv_tempstring[0]);
}

void SetRecoilMaxYTilt(int value) {

	PLAYER_WEAPON_DATA *weaponPtr;
	TEMPLATE_WEAPON_DATA *twPtr;
	PLAYER_STATUS *playerStatusPtr= (PLAYER_STATUS *) (Player->ObStrategyBlock->SBdataptr);
    GLOBALASSERT(playerStatusPtr);
    weaponPtr = &(playerStatusPtr->WeaponSlot[playerStatusPtr->SelectedWeaponSlot]);
    twPtr = &TemplateWeapon[weaponPtr->WeaponIDNumber];

	twPtr->RecoilMaxYTilt=value;

	sprintf(ccv_tempstring,"RECOILMAXYTILT = %d\n",twPtr->RecoilMaxYTilt);

	NewOnScreenMessage((unsigned char *)&ccv_tempstring[0]);
}

void SetRecoilMaxZ(int value) {

	PLAYER_WEAPON_DATA *weaponPtr;
	TEMPLATE_WEAPON_DATA *twPtr;
	PLAYER_STATUS *playerStatusPtr= (PLAYER_STATUS *) (Player->ObStrategyBlock->SBdataptr);
    GLOBALASSERT(playerStatusPtr);
    weaponPtr = &(playerStatusPtr->WeaponSlot[playerStatusPtr->SelectedWeaponSlot]);
    twPtr = &TemplateWeapon[weaponPtr->WeaponIDNumber];

	twPtr->RecoilMaxZ=value;

	sprintf(ccv_tempstring,"RECOILMAXZ = %d\n",twPtr->RecoilMaxZ);

	NewOnScreenMessage((unsigned char *)&ccv_tempstring[0]);
}

void SetRecoilMaxRandomZ(int value) {

	PLAYER_WEAPON_DATA *weaponPtr;
	TEMPLATE_WEAPON_DATA *twPtr;
	PLAYER_STATUS *playerStatusPtr= (PLAYER_STATUS *) (Player->ObStrategyBlock->SBdataptr);
    GLOBALASSERT(playerStatusPtr);
    weaponPtr = &(playerStatusPtr->WeaponSlot[playerStatusPtr->SelectedWeaponSlot]);
    twPtr = &TemplateWeapon[weaponPtr->WeaponIDNumber];

	twPtr->RecoilMaxRandomZ=value;

	sprintf(ccv_tempstring,"RECOILMAXRANDOMZ = %d\n",twPtr->RecoilMaxRandomZ);

	NewOnScreenMessage((unsigned char *)&ccv_tempstring[0]);
}

void SetPredPistolRecoilTime(int value) {

	TEMPLATE_WEAPON_DATA *twPtr;
	int timeOutRate;

    twPtr = &TemplateWeapon[WEAPON_PRED_PISTOL];

	if (value==0) {
		int time;

		GLOBALASSERT(twPtr->TimeOutRateForState[WEAPONSTATE_RECOIL_PRIMARY]);
		time=DIV_FIXED(ONE_FIXED,twPtr->TimeOutRateForState[WEAPONSTATE_RECOIL_PRIMARY]);
		sprintf(ccv_tempstring,"PREDPISTOL_RECOILTIME = %d\n",time);

		NewOnScreenMessage((unsigned char *)&ccv_tempstring[0]);
		return;
	}

	timeOutRate=DIV_FIXED(ONE_FIXED,value);

   	twPtr->TimeOutRateForState[WEAPONSTATE_RECOIL_PRIMARY]=timeOutRate;

}

void SetPredPistolMaxDamage(int value) {

	if (value<0) {
		sprintf(ccv_tempstring,"PREDPISTOL_BLASTDAMAGE = %d\n",TemplateAmmo[AMMO_PRED_PISTOL].MaxDamage[AvP.Difficulty].Electrical);

		NewOnScreenMessage((unsigned char *)&ccv_tempstring[0]);
		return;
	}

	if (value>1000) return;

	TemplateAmmo[AMMO_PRED_PISTOL].MaxDamage[AvP.Difficulty].Electrical=value;

}

void SetPredPistolBlastRange(int value) {

	if (value<0) {
		sprintf(ccv_tempstring,"PREDPISTOL_BLASTRANGE = %d\n",TemplateAmmo[AMMO_PRED_PISTOL].MaxRange);

		NewOnScreenMessage((unsigned char *)&ccv_tempstring[0]);
		return;
	}

	if (value>65536) return;

	TemplateAmmo[AMMO_PRED_PISTOL].MaxRange=value;

}

void SetPredPistolStrikeDamage(int value) {

	if (value<0) {
		sprintf(ccv_tempstring,"PREDPISTOL_STRIKEDAMAGE = %d\n",TemplateAmmo[AMMO_PREDPISTOL_STRIKE].MaxDamage[AvP.Difficulty].Electrical);

		NewOnScreenMessage((unsigned char *)&ccv_tempstring[0]);
		return;
	}

	if (value>1000) return;

	TemplateAmmo[AMMO_PREDPISTOL_STRIKE].MaxDamage[AvP.Difficulty].Electrical=value;

}

void PredPistol_FullAuto(void) {

	TEMPLATE_WEAPON_DATA *twPtr;

    twPtr = &TemplateWeapon[WEAPON_PRED_PISTOL];

   	twPtr->PrimaryIsAutomatic=1;

}

void PredPistol_SemiAuto(void) {

	TEMPLATE_WEAPON_DATA *twPtr;

    twPtr = &TemplateWeapon[WEAPON_PRED_PISTOL];

   	twPtr->PrimaryIsAutomatic=0;

}

void SetPlayerStartingHealth(int value) {

	NPC_DATA *NpcData = NULL;
	PLAYER_STATUS *playerStatusPtr= (PLAYER_STATUS *) (Player->ObStrategyBlock->SBdataptr);
	
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
		default:
			LOCALASSERT(0);
	}
	GLOBALASSERT(NpcData);

	if (value<=0) {
		sprintf(ccv_tempstring,"PLAYER STARTING HEALTH = %d\n",NpcData->StartingStats.Health);

		NewOnScreenMessage((unsigned char *)&ccv_tempstring[0]);
		return;
	}

	if (value>1000) return;

	NpcData->StartingStats.Health=value;

	Player->ObStrategyBlock->SBDamageBlock.Health=NpcData->StartingStats.Health<<ONE_FIXED_SHIFT;
	playerStatusPtr->Health=Player->ObStrategyBlock->SBDamageBlock.Health;

}

void SetPlayerStartingArmour(int value) {

	NPC_DATA *NpcData = NULL;
	PLAYER_STATUS *playerStatusPtr= (PLAYER_STATUS *) (Player->ObStrategyBlock->SBdataptr);
	
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
		default:
			LOCALASSERT(0);
	}
	GLOBALASSERT(NpcData);

	if (value<=0) {
		sprintf(ccv_tempstring,"PLAYER STARTING ARMOUR = %d\n",NpcData->StartingStats.Armour);

		NewOnScreenMessage((unsigned char *)&ccv_tempstring[0]);
		return;
	}

	if (value>1000) return;

	NpcData->StartingStats.Armour=value;

	Player->ObStrategyBlock->SBDamageBlock.Armour=NpcData->StartingStats.Armour<<ONE_FIXED_SHIFT;
	playerStatusPtr->Armour=Player->ObStrategyBlock->SBDamageBlock.Armour;

}

void ResetPersonalBests(void) {

	{
		int a,b;

		for (a=0; a<I_MaxDifficulties; a++) {
			for (b=0; b<AVP_ENVIRONMENT_END_OF_LIST; b++) {
				UserProfilePtr->PersonalBests[a][b]=DefaultLevelGameStats;
			}
		}
	}

}

void CreateMoreGameSpecificConsoleVariables(void)
{
	#if CONSOLE_DEBUGGING_COMMANDS_ACTIVATED
	/* Calibrate Tail Hack */

	ConsoleVariable :: MakeSimpleConsoleVariable_Int
	(
		tail_xcal, // int& Value_ToUse,
		"TAIL-XCAL", // ProjChar* pProjCh_ToUse,
		"CLASSIFIED", // ProjChar* pProjCh_Description_ToUse
		0, // int MinVal_New,
		1000  // int MaxVal_New
	);
	ConsoleVariable :: MakeSimpleConsoleVariable_Int
	(
		tail_ycal, // int& Value_ToUse,
		"TAIL-YCAL", // ProjChar* pProjCh_ToUse,
		"CLASSIFIED", // ProjChar* pProjCh_Description_ToUse
		0, // int MinVal_New,
		1000  // int MaxVal_New
	);

	#if 0
	ConsoleVariable :: MakeSimpleConsoleVariable_Int
	(
		RATweak, // int& Value_ToUse,
		"RATWEAK", // ProjChar* pProjCh_ToUse,
		"CLASSIFIED", // ProjChar* pProjCh_Description_ToUse
		-512, // int MinVal_New,
		512  // int MaxVal_New
	);
	#endif

	ConsoleVariable :: MakeSimpleConsoleVariable_Int
	(
		GlobalGoreRate, // int& Value_ToUse,
		"GLOBALGORERATE", // ProjChar* pProjCh_ToUse,
		"(TAKES VALUES 0 -> 65536, LESS IS MORE: 0 = DISABLED)", // ProjChar* pProjCh_Description_ToUse
		0, // int MinVal_New,
		65536  // int MaxVal_New
	);

	ConsoleVariable :: MakeSimpleConsoleVariable_Int
	(
		Marine_Skill, // int& Value_ToUse,
		"MARINE-SKILL", // ProjChar* pProjCh_ToUse,
		"(NPC SKILL, 65536 WILL RARELY MISS)", // ProjChar* pProjCh_Description_ToUse
		0, // int MinVal_New,
		131072  // int MaxVal_New
	);

	ConsoleVariable :: MakeSimpleConsoleVariable_Int
	(
		Marine_Terminal_Velocity, // int& Value_ToUse,
		"MARINE-TERMINAL-VELOCITY", // ProjChar* pProjCh_ToUse,
		"(SPEED WHICH KILLS A FALLING MARINE)", // ProjChar* pProjCh_Description_ToUse
		0, // int MinVal_New,
		131072  // int MaxVal_New
	);

	/* Cloaking */

	ConsoleVariable :: MakeSimpleConsoleVariable_Int
	(
		TrickleCharge, // int& Value_ToUse,
		"TRICKLECHARGE", // ProjChar* pProjCh_ToUse,
		"(PRED TRICKLE CHARGE RATE)", // ProjChar* pProjCh_Description_ToUse
		-131072, // int MinVal_New,
		131072  // int MaxVal_New
	);

	ConsoleVariable :: MakeSimpleConsoleVariable_Int
	(
		CloakDrain, // int& Value_ToUse,
		"CLOAKDRAIN", // ProjChar* pProjCh_ToUse,
		"(PRED CLOAK DRAIN RATE)", // ProjChar* pProjCh_Description_ToUse
		-131072, // int MinVal_New,
		131072  // int MaxVal_New
	);

	ConsoleVariable :: MakeSimpleConsoleVariable_Int
	(
		CloakThreshold, // int& Value_ToUse,
		"CLOAKTHRESHOLD", // ProjChar* pProjCh_ToUse,
		"(CANNOT CLOAK WITHOUT THIS MUCH ENERGY)", // ProjChar* pProjCh_Description_ToUse
		0, // int MinVal_New,
		PLAYERCLOAK_MAXENERGY  // int MaxVal_New
	);

	ConsoleVariable :: MakeSimpleConsoleVariable_Int
	(
		CloakPowerOnDrain, // int& Value_ToUse,
		"CLOAKPOWERONDRAIN", // ProjChar* pProjCh_ToUse,
		"(CLOAKING USES THIS MUCH POWER)", // ProjChar* pProjCh_Description_ToUse
		0, // int MinVal_New,
		PLAYERCLOAK_MAXENERGY  // int MaxVal_New
	);

	/* Pred Pistol */

	ConsoleVariable :: MakeSimpleConsoleVariable_Int
	(
		PredPistol_ShotCost, // int& Value_ToUse,
		"PREDPISTOL_SHOTCOST", // ProjChar* pProjCh_ToUse,
		"(ONE SHOT USES THIS MUCH POWER)", // ProjChar* pProjCh_Description_ToUse
		0, // int MinVal_New,
		PLAYERCLOAK_MAXENERGY  // int MaxVal_New
	);

	ConsoleVariable :: MakeSimpleConsoleVariable_Int
	(
		PredPistolBoltSpeed, // int& Value_ToUse,
		"PREDPISTOL_BOLTSPEED", // ProjChar* pProjCh_ToUse,
		"(SPEED OF BOLT)", // ProjChar* pProjCh_Description_ToUse
		0, // int MinVal_New,
		262144  // int MaxVal_New
	);

	ConsoleVariable :: MakeSimpleConsoleVariable_Int
	(
		PredPistolBoltGravity, // int& Value_ToUse,
		"PREDPISTOL_BOLTGRAVITY", // ProjChar* pProjCh_ToUse,
		"(GGRAVITY APPLIED TO BOLT)", // ProjChar* pProjCh_Description_ToUse
		-131072, // int MinVal_New,
		131072  // int MaxVal_New
	);

	ConsoleCommand :: Make
	(
		"RESET_PERSONAL_BESTS",
		"RESETS PERSONAL BESTS STRUCTURES",
		ResetPersonalBests
	);

	ConsoleCommand :: Make
	(
		"PREDPISTOL_BLASTRANGE",
		"ALTERS PRED PISTOL EXPLOSIVE RANGE",
		SetPredPistolBlastRange
	);

	ConsoleCommand :: Make
	(
		"PREDPISTOL_BLASTDAMAGE",
		"ALTERS PRED PISTOL EXPLOSIVE DAMAGE",
		SetPredPistolMaxDamage
	);

	ConsoleCommand :: Make
	(
		"PREDPISTOL_STRIKEDAMAGE",
		"ALTERS PRED PISTOL DIRECT DAMAGE",
		SetPredPistolStrikeDamage
	);

	ConsoleCommand :: Make
	(
		"PREDPISTOL_RECOILTIME",
		"ALTERS PRED PISTOL RECOIL TIME",
		SetPredPistolRecoilTime
	);

	ConsoleCommand :: Make
	(
		"PREDPISTOL_FULLAUTO",
		"MAKES PRED PISTOL FULLY AUTOMATIC",
		PredPistol_FullAuto
	);

	ConsoleCommand :: Make
	(
		"PREDPISTOL_SEMIAUTO",
		"MAKES PRED PISTOL SEMI AUTOMATIC",
		PredPistol_SemiAuto
	);

	/* Plasmacaster */

	ConsoleVariable :: MakeSimpleConsoleVariable_Int
	(
		Caster_Jumpstart, // int& Value_ToUse,
		"CASTER_JUMPSTART", // ProjChar* pProjCh_ToUse,
		"(INSTANT CHARGE WHEN FIRED)", // ProjChar* pProjCh_Description_ToUse
		0, // int MinVal_New,
		65536  // int MaxVal_New
	);

	ConsoleVariable :: MakeSimpleConsoleVariable_Int
	(
		Caster_Chargetime, // int& Value_ToUse,
		"CASTER_CHARGETIME", // ProjChar* pProjCh_ToUse,
		"(TIME TO CHARGE UP WITH FIRE)", // ProjChar* pProjCh_Description_ToUse
		0, // int MinVal_New,
		(65536*60)  // int MaxVal_New
	);

	ConsoleVariable :: MakeSimpleConsoleVariable_Int
	(
		Caster_TrickleRate, // int& Value_ToUse,
		"CASTER_TRICKLERATE", // ProjChar* pProjCh_ToUse,
		"(ONE FIXED PER SECOND)", // ProjChar* pProjCh_Description_ToUse
		0, // int MinVal_New,
		65536  // int MaxVal_New
	);

	ConsoleVariable :: MakeSimpleConsoleVariable_Int
	(
		Caster_TrickleLevel, // int& Value_ToUse,
		"CASTER_TRICKLELEVEL", // ProjChar* pProjCh_ToUse,
		"(TRICKLES TO THIS LEVEL)", // ProjChar* pProjCh_Description_ToUse
		0, // int MinVal_New,
		65536  // int MaxVal_New
	);

	ConsoleVariable :: MakeSimpleConsoleVariable_Int
	(
		Caster_ChargeRatio, // int& Value_ToUse,
		"CASTER_CHARGERATIO", // ProjChar* pProjCh_ToUse,
		"(FIELD CHARGE FOR A FULL SHOT)", // ProjChar* pProjCh_Description_ToUse
		0, // int MinVal_New,
		(65536*30)  // int MaxVal_New
	);

	ConsoleVariable :: MakeSimpleConsoleVariable_Int
	(
		Caster_NPCKill, // int& Value_ToUse,
		"CASTER_NPCKILL", // ProjChar* pProjCh_ToUse,
		"(CASTER CHARGE TO KILL AN NPC)", // ProjChar* pProjCh_Description_ToUse
		0, // int MinVal_New,
		65536  // int MaxVal_New
	);

	ConsoleVariable :: MakeSimpleConsoleVariable_Int
	(
		Caster_PCKill, // int& Value_ToUse,
		"CASTER_PCKILL", // ProjChar* pProjCh_ToUse,
		"(CASTER CHARGE TO KILL A PC)", // ProjChar* pProjCh_Description_ToUse
		0, // int MinVal_New,
		65536  // int MaxVal_New
	);

	ConsoleVariable :: MakeSimpleConsoleVariable_Int
	(
		Caster_BlastRadius, // int& Value_ToUse,
		"CASTER_BLASTRADIUS", // ProjChar* pProjCh_ToUse,
		"(BLAST RADIUS OF A FULL BOLT)", // ProjChar* pProjCh_Description_ToUse
		0, // int MinVal_New,
		65536  // int MaxVal_New
	);

	ConsoleVariable :: MakeSimpleConsoleVariable_Int
	(
		Caster_MinCharge, // int& Value_ToUse,
		"CASTER_MINCHARGE", // ProjChar* pProjCh_ToUse,
		"(MINIMUM CASTER CHARGE TO FIRE)", // ProjChar* pProjCh_Description_ToUse
		0, // int MinVal_New,
		65536  // int MaxVal_New
	);

	/* Disc */

	ConsoleVariable :: MakeSimpleConsoleVariable_Int
	(
		RecallDisc_Charge, // int& Value_ToUse,
		"RECALL_DISC_CHARGE", // ProjChar* pProjCh_ToUse,
		"(COST TO RECALL A DISC)", // ProjChar* pProjCh_Description_ToUse
		0, // int MinVal_New,
		PLAYERCLOAK_MAXENERGY  // int MaxVal_New
	);

	ConsoleCommand :: Make
	(
		"RECALL_DISC",
		"RECALLS A STUCK DISC",
		Recall_Disc
	);

	/* Player Stats */

	ConsoleCommand :: Make
	(
		"PLAYER_STARTING_HEALTH",
		"CHANGES CURRENT PLAYER STARTING HEALTH",
		SetPlayerStartingHealth
	);

	ConsoleCommand :: Make
	(
		"PLAYER_STARTING_ARMOUR",
		"CHANGES CURRENT PLAYER STARTING ARMOUR",
		SetPlayerStartingArmour
	);

	/* Debugging */
	#if 1
	ConsoleCommand :: Make
	(
		"TOGGLE_SHAPERENDER",
		"SWITCHES ALL HIERARCHY SECTIONS FOR SHELLS",
		ToggleShapeRender
	);
	#endif
	/* TEMPORARY */
	ConsoleCommand :: Make
	(
		"STRIKETIME",
		"ALTERS WRISTBLADE STRIKE TIME",
		StrikeTime
	);

	ConsoleCommand :: Make
	(
		"ALIEN-STRIKETIME",
		"ALTERS ALIEN CLAW STRIKE TIME",
		AlienStrikeTime
	);

	ConsoleCommand :: Make
	(
		"SHOWPREDOSTATS",
		"TOGGLES PREDATOR ENERGY PRINT",
		Show_PredOStats
	);

	ConsoleCommand :: Make
	(
		"SHOWXENOSTATS",
		"TOGGLES XENOBORG BEHAVIOUR PRINT",
		Show_XenoStats
	);


	ConsoleCommand :: Make
	(
		"SHOWSQUADSTATE",
		"TOGGLES MARINE SQUAD STATUS PRINT",
		Show_SquadState
	);

	ConsoleCommand :: Make
	(
		"SHOWNEARSQUAD",
		"TOGGLES NEAR MARINE STATUS PRINT",
		Show_NearSquad
	);

	ConsoleCommand :: Make
	(
		"SHOWHIVESTATE",
		"TOGGLES HIVE STATUS PRINT",
		Show_HiveState
	);
	#if 0
	ConsoleCommand :: Make
	(
		"TOGGLEAMBIENCE",
		"TOGGLES THE AMBIENT LIGHT",
		Toggle_Ambience
	);
	#endif
	ConsoleCommand :: Make
	(
		"SHOWSLACK",
		"TOGGLES SLACK STATUS PRINT",
		Show_Slack
	);

	ConsoleCommand :: Make
	(
		"SHOWADJACENCIES",
		"TOGGLES ADJACENCY STATUS PRINT",
		Show_Adj
	);

	ConsoleCommand :: Make
	(
		"ZAPSLACK",
		"ZEROES SLACK COUNTERS",
		ZapSlack
	);

	ConsoleCommand :: Make
	(
		"ZONEALERT",
		"TRIGGERS A MARINE ALERT",
		Console_ZoneAlert
	);
	/* TEMPORARY */

	ConsoleCommand :: Make
	(
		"TAUNT",
		"STARTS TAUNT SEQUENCE",
		StartPlayerTaunt
	);

	ConsoleCommand :: Make
	(
		"ACTIVATE-HIVE",
		"ACTIVATES ALIEN/NPC HIVE",
		ActivateHive
	);

	ConsoleCommand :: Make
	(
		"DEACTIVATE-HIVE",
		"DEACTIVATES ALIEN/NPC HIVE",
		DeActivateHive
	);

	ConsoleCommand :: Make
	(
		"FORCEGEN",
		"FORCES GENERATION OF AN NPC - SOMEWHERE",
		ForceAGenerator_Shell
	);

	ConsoleCommand :: Make
	(
		"DUMMY",
		"CREATES PLAYER DUMMY.",
		CastDummy
	);

	#if 0
	ConsoleCommand :: Make
	(
		"SENTRYGUN",
		"DROPS A SENTRY GUN",
		CastSentrygun
	);
	#endif
	ConsoleCommand :: Make
	(
		"ACTIVATE_GENERATORS",
		"ACTIVATES ALL GENERATORS.",
		ActivateAllGenerators
	);

	ConsoleCommand :: Make
	(
		"NUKEINANIMATES",
		"DESTROYS ALL INANIMATE OBJECTS.",
		KillAllInanimates
	);

	ConsoleCommand :: Make
	(
		"NUKEMARINES",
		"KILLS ALL MARINES.",
		KillAllMarines
	);

	ConsoleCommand :: Make
	(
		"NUKEALIENS",
		"KILLS ALL ALIENS.",
		KillAllAliens
	);

	ConsoleCommand :: Make
	(
		"NUKEPREDS",
		"KILLS ALL PREDATORS.",
		KillAllPreds
	);

	ConsoleCommand :: Make
	(
		"NUKEDUMMIES",
		"KILLS ALL PLAYER DUMMIES.",
		KillAllDummies
	);

	ConsoleCommand :: Make
	(
		"OBSERVER",
		"TOGGLES OBSERVER MODE.",
		Toggle_Observer
	);

	ConsoleCommand :: Make
	(
		"SHOWRECOILMAXXTILT",
		"SHOWS THAT VALUE.",
		ShowRecoilMaxXTilt
	);

	ConsoleCommand :: Make
	(
		"SHOWRECOILMAXYTILT",
		"SHOWS THAT VALUE.",
		ShowRecoilMaxYTilt
	);

	ConsoleCommand :: Make
	(
		"SHOWRECOILMAXZ",
		"SHOWS THAT VALUE.",
		ShowRecoilMaxZ
	);

	ConsoleCommand :: Make
	(
		"SHOWRECOILMAXRANDOMZ",
		"SHOWS THAT VALUE.",
		ShowRecoilMaxRandomZ
	);

	ConsoleCommand :: Make
	(
		"SETRECOILMAXXTILT",
		"SETS THAT VALUE.",
		SetRecoilMaxXTilt
	);

	ConsoleCommand :: Make
	(
		"SETRECOILMAXYTILT",
		"SETS THAT VALUE.",
		SetRecoilMaxYTilt
	);

	ConsoleCommand :: Make
	(
		"SETRECOILMAXZ",
		"SETS THAT VALUE.",
		SetRecoilMaxZ
	);

	ConsoleCommand :: Make
	(
		"SETRECOILMAXRANDOMZ",
		"SETS THAT VALUE.",
		SetRecoilMaxRandomZ
	);
	
	#if 0
	ConsoleVariable :: MakeSimpleConsoleVariable_Int
	(
		mx,
		"MX",
		"MOLOTOV X IMPULSE",
		-ONE_FIXED,
		ONE_FIXED
	);

	ConsoleVariable :: MakeSimpleConsoleVariable_Int
	(
		my,
		"MY",
		"MOLOTOV Y IMPULSE",
		-ONE_FIXED,
		ONE_FIXED
	);

	ConsoleVariable :: MakeSimpleConsoleVariable_Int
	(
		mz,
		"MZ",
		"MOLOTOV Z IMPULSE",
		-ONE_FIXED,
		ONE_FIXED
	);
	#endif

	#if 0
	/* Queen commands */

	ConsoleVariable :: MakeSimpleConsoleVariable_Int
	(
		Queen_Step_Time,
		"QSTEP-TIME",
		"QUEEN STEP FORWARD TIME",
		0,
		(ONE_FIXED<<8)
	);

	ConsoleVariable :: MakeSimpleConsoleVariable_Int
	(
		Queen_Charge_Rate,
		"QCHARGE_RATE",
		"QUEEN CHARGE PACE",
		0,
		(ONE_FIXED<<8)
	);

	ConsoleVariable :: MakeSimpleConsoleVariable_Int
	(
		Queen_Step_Speed,
		"QSTEP-SPEED",
		"QUEEN STEP FORWARD SPEED",
		0,
		(ONE_FIXED)
	);

	ConsoleVariable :: MakeSimpleConsoleVariable_Int
	(
		Queen_Step_Mode,
		"QSTEP-MODE",
		"QUEEN STEP FORWARD MODE",
		0,
		1
	);

	ConsoleVariable :: MakeSimpleConsoleVariable_Int
	(
		Queen_Turn_Rate,
		"QTURN-RATE",
		"QUEEN TURN RATE",
		0,
		65536
	);

	ConsoleCommand :: Make
	(
		"QCOMM-STOP",
		"STOPS SLAVE QUEEN.",
		QComm_Stop
	);

	ConsoleCommand :: Make
	(
		"QCOMM-SF",
		"ORDERS SLAVE QUEEN ONE STEP FORWARD.",
		QComm_StepForward
	);

	ConsoleCommand :: Make
	(
		"QCOMM-SB",
		"ORDERS SLAVE QUEEN ONE STEP Back.",
		QComm_StepBack
	);

	ConsoleCommand :: Make
	(
		"QCOMM-TL",
		"ORDERS SLAVE QUEEN TO TURN LEFT.",
		QComm_TurnLeft
	);

	ConsoleCommand :: Make
	(
		"QCOMM-TR",
		"ORDERS SLAVE QUEEN TO TURN RIGHT.",
		QComm_TurnRight
	);

	ConsoleCommand :: Make
	(
		"QCOMM-HEEL",
		"ORDERS SLAVE QUEEN TO COME TO PLAYER.",
		QComm_Heel
	);

	ConsoleCommand :: Make
	(
		"QCOMM-TAUNT",
		"ORDERS SLAVE QUEEN TO TAUNT.",
		QComm_Taunt
	);
	
	ConsoleCommand :: Make
	(
		"QCOMM-HISS",
		"ORDERS SLAVE QUEEN TO HISS.",
		QComm_Hiss
	);

	ConsoleCommand :: Make
	(
		"QCOMM-LS",
		"ORDERS SLAVE QUEEN TO SWIPE (LEFT).",
		QComm_LeftSwipe
	);

	ConsoleCommand :: Make
	(
		"QCOMM-RS",
		"ORDERS SLAVE QUEEN TO SWIPE (Right).",
		QComm_RightSwipe
	);

	ConsoleCommand :: Make
	(
		"QCOMM-ROUTE",
		"ORDERS SLAVE QUEEN TO FOLLOW THE PROGRAMMED ROUTE.",
		QComm_Route
	);

	ConsoleCommand :: Make
	(
		"QCOMM-CHARGE",
		"ORDERS SLAVE QUEEN TO CHARGE.",
		QComm_Charge
	);
	#endif

	#endif
}

}; // extern "C"

