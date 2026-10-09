/* KJL 16:23:20 10/25/96 - I'm moved all the weapon stuff to the newly created weapon.c,
so player.c is looking a bit bare at the moment. */
#include "3dc.h"
#include "module.h"
#include "inline.h"

#include "stratdef.h"
#include "gamedef.h"
#include "bh_types.h"
#include "inventry.h"
#include "gameplat.h"
#include "dynblock.h"
#include "dynamics.h"
#include "comp_shp.h"
#include "weapons.h"
#include "vision.h"
#include "pheromon.h"
#include "avpview.h"
#include "particle.h"
#include "scream.h"
#include "savegame.h"
#include "game_statistics.h"
#include "pfarlocs.h"
#include "bh_ais.h"

#define UseLocalAssert Yes
#include "ourasert.h"

#include "psnd.h"
#include "psndplat.h"
#include "player.h"

/* for win 95 net support */
#include "pldnet.h"
#include "pldghost.h"
//#include "dp_func.h"

#include "showcmds.h"
#include "bonusabilities.h"

extern DPID AVPDPNetID;

#define PLAYER_HMODEL 0

/*KJL****************************************************************************************
*  										G L O B A L S 	            					    *
****************************************************************************************KJL*/
VECTORCH PlayerStartLocation;
MATRIXCH PlayerStartMat;
extern int NormalFrameTime;
extern ACTIVESOUNDSAMPLE ActiveSounds[];
extern int PlayerDamagedOverlayIntensity;
extern int playerNoise;
extern int predHUDSoundHandle;
extern int predOVision_SoundHandle;

extern int AIModuleArraySize;

int GimmeChargeCalls;
int HtoHStrikes;
int CurrentLightAtPlayer;
int TauntSoundPlayed;

int TrickleCharge=9000;
int CloakDrain=12000;
int CloakThreshold=(5*ONE_FIXED);
int CloakPowerOnDrain=(2*ONE_FIXED);

extern DPID myNetworkKillerId;
extern DPID myIgniterId;
extern int MyHitBodyPartId;
extern HMODELCONTROLLER PlayersWeaponHModelController;
extern SECTION_DATA *PWMFSDP; /* PlayersWeaponMuzzleFlashSectionDataPointer */

/*KJL****************************************************************************************
*                                    P R O T O T Y P E S	                                *
****************************************************************************************KJL*/
void InitPlayer(STRATEGYBLOCK* sbPtr, int sb_type);
void MaintainPlayer(void);
void PlayerIsDamaged(STRATEGYBLOCK *sbPtr, DAMAGE_PROFILE *damage, int multiplier,VECTORCH* incoming);
static void PlayerIsDead(DAMAGE_PROFILE *damage,int multiplier,VECTORCH* incoming);

extern int LightIntensityAtPoint(VECTORCH *pointPtr);
extern SECTION * GetNamedHierarchyFromLibrary(const char * rif_name, const char * hier_name);
extern int SlotForThisWeapon(enum WEAPON_ID weaponID);
extern void PointAlert(int level, VECTORCH *point);
extern void RemoveAllThisPlayersDiscs(void);
void ShowAdjacencies(void);

extern int ShowAdj;

/*KJL****************************************************************************************
*                                     F U N C T I O N S	                                    *
****************************************************************************************KJL*/

PLAYER_STATUS* PlayerStatusPtr = NULL;
static PLAYER_STATUS PlayerStatusBlock;
int ShowPredoStats=0;
int Observer=0;

/* Patrick 22/8/97------------------------------------------------
Cloaking stuff
------------------------------------------------------------------*/
void InitPlayerCloakingSystem(void);
static void DoPlayerCloakingSystem(void);

void InitPlayer(STRATEGYBLOCK* sbPtr, int sb_type) 
{
	/*KJL**************************************************************************************
	* InitPlayer() was written by me. It attaches the extra player data to the strategy block *
	* and fills in some initial values.                                                       *
	**************************************************************************************KJL*/

#if 0
	SECTION *root_section;
#endif
	PLAYER_STATUS *psPtr = &PlayerStatusBlock;
	GLOBALASSERT(psPtr);
 	GLOBALASSERT(sbPtr);
	
	// set up our global

	PlayerStatusPtr = psPtr;


	sbPtr->I_SBtype = sb_type;
	sbPtr->SBdataptr = (void*)psPtr;

	InitialisePlayersInventory(psPtr);

	/* Initialise Player's stats */
	{
		NPC_DATA *NpcData;
		NPC_TYPES PlayerType;

		switch(AvP.PlayerType) 
		{
			case(I_Marine):
			{
				switch (AvP.Difficulty) {
					case I_Easy:
						PlayerType=I_PC_Marine_Easy;
						break;
					default:
					case I_Medium:
						PlayerType=I_PC_Marine_Medium;
						break;
					case I_Hard:
						PlayerType=I_PC_Marine_Hard;
						break;
					case I_Impossible:
						PlayerType=I_PC_Marine_Impossible;
						break;
				}

				#if 0  //this hmodel isn't being set up for the moment - Richard
				root_section=GetNamedHierarchyFromLibrary("hnpcmarine","Template");
				if (!root_section) {
					GLOBALASSERT(0);
					/* Sorry, there's just no bouncing back from this one.  Fix it. */
					return;
				}
				#if PLAYER_HMODEL
				Create_HModel(&psPtr->HModelController,root_section);
				InitHModelSequence(&psPtr->HModelController,0,0,ONE_FIXED);
				#endif
				/* Doesn't matter what the sequence is... */
				#endif
				break;
			}
			case(I_Predator):
			{
				switch (AvP.Difficulty) {
					case I_Easy:
						PlayerType=I_PC_Predator_Easy;
						break;
					default:
					case I_Medium:
						PlayerType=I_PC_Predator_Medium;
						break;
					case I_Hard:
						PlayerType=I_PC_Predator_Hard;
						break;
					case I_Impossible:
						PlayerType=I_PC_Predator_Impossible;
						break;
				}

				#if 0  //this hmodel isn't being set up for the moment - Richard
				root_section=GetNamedHierarchyFromLibrary("hnpcpredator","Template");
				if (!root_section) {
					GLOBALASSERT(0);
					/* Sorry, there's just no bouncing back from this one.  Fix it. */
					return;
				}
				#if PLAYER_HMODEL
				Create_HModel(&psPtr->HModelController,root_section);
				InitHModelSequence(&psPtr->HModelController,0,0,ONE_FIXED);
				#endif
				/* Doesn't matter what the sequence is... */
				#endif
				break;
			}
			case(I_Alien):
			{
				switch (AvP.Difficulty) {
					case I_Easy:
						PlayerType=I_PC_Alien_Easy;
						break;
					default:
					case I_Medium:
						PlayerType=I_PC_Alien_Medium;
						break;
					case I_Hard:
						PlayerType=I_PC_Alien_Hard;
						break;
					case I_Impossible:
						PlayerType=I_PC_Alien_Impossible;
						break;
				}

				#if 0  //this hmodel isn't being set up for the moment - Richard
				root_section=GetNamedHierarchyFromLibrary("hnpcalien","alien");
				if (!root_section) {
					GLOBALASSERT(0);
					/* Sorry, there's just no bouncing back from this one.  Fix it. */
					return;
				}
				#if PLAYER_HMODEL
				Create_HModel(&psPtr->HModelController,root_section);
				InitHModelSequence(&psPtr->HModelController,0,0,ONE_FIXED);
				#endif
				/* Doesn't matter what the sequence is... */
				#endif
				break;

			}
			default:
			{
				LOCALASSERT(1==0);
				break;
			}
		}

		NpcData=GetThisNpcData(PlayerType);
		LOCALASSERT(NpcData);
		sbPtr->SBDamageBlock.Health=NpcData->StartingStats.Health<<ONE_FIXED_SHIFT;
		sbPtr->SBDamageBlock.Armour=NpcData->StartingStats.Armour<<ONE_FIXED_SHIFT;
		sbPtr->SBDamageBlock.SB_H_flags=NpcData->StartingStats.SB_H_flags;
		sbPtr->SBDamageBlock.IsOnFire=0;

		//{
		//	int *nptr,i;
		//	nptr=(int *)sbPtr->SBname;
		//	for (i=0; i<(SB_NAME_LENGTH>>2); i++) {
		//		*nptr=FastRandom();
		//		nptr++;
		//	}
		//	sbPtr->SBname[SB_NAME_LENGTH-1]=3; /* Just to make sure... */
		//}
		AssignNewSBName(sbPtr);
	}

    //psPtr->Health=STARTOFGAME_MARINE_HEALTH;
    psPtr->Energy=STARTOFGAME_MARINE_ENERGY;
    //psPtr->Armour=STARTOFGAME_MARINE_ARMOUR;

	psPtr->Encumberance.MovementMultiple=ONE_FIXED;
	psPtr->Encumberance.TurningMultiple=ONE_FIXED;
	psPtr->Encumberance.JumpingMultiple=ONE_FIXED;
	psPtr->Encumberance.CanCrouch=1;
	psPtr->Encumberance.CanRun=1;

	psPtr->incidentFlag=0;
	psPtr->incidentTimer=0;

	/* CDF 16/9/97 Now, those health and armour stats are those of the last cycle. */

	psPtr->Health=sbPtr->SBDamageBlock.Health;
	psPtr->Armour=sbPtr->SBDamageBlock.Armour;

	psPtr->IsAlive = 1;
	psPtr->IHaveAPlacedAutogun = 0;
	psPtr->MyFaceHugger=NULL;
	psPtr->MyCorpse=NULL;
	psPtr->tauntTimer=0;
	TauntSoundPlayed=0;
	psPtr->fireTimer=0;
	psPtr->invulnerabilityTimer=0;
	/* Better safe than sorry. */
	psPtr->soundHandle=SOUND_NOACTIVEINDEX;
	psPtr->soundHandle3=SOUND_NOACTIVEINDEX;
	psPtr->soundHandle4=SOUND_NOACTIVEINDEX;
	psPtr->soundHandle5=SOUND_NOACTIVEINDEX;
	psPtr->soundHandleForPredatorCloakDamaged=SOUND_NOACTIVEINDEX;
	InitPlayerCloakingSystem();/* Patrick 22/8/97 : Cloaking stuff */

    /* KJL 12:06:01 11/14/96 - allocate dynamics block & fill from template */
	sbPtr->DynPtr = AllocateDynamicsBlock(DYNAMICS_TEMPLATE_MARINE_PLAYER);
	/* for the time being get world position and orientation from the displayblock */
	{
		DISPLAYBLOCK *dPtr = sbPtr->SBdptr;
		DYNAMICSBLOCK *dynPtr = sbPtr->DynPtr;
		GLOBALASSERT(dPtr);
		GLOBALASSERT(dynPtr);

		dynPtr->Position = PlayerStartLocation;
		dynPtr->OrientMat = PlayerStartMat;
		MatrixToEuler(&dynPtr->OrientMat,&dynPtr->OrientEuler);
		//dynPtr->OrientEuler = dPtr->ObEuler;
		
		
		dynPtr->PrevPosition = dynPtr->Position;
		dynPtr->PrevOrientMat = dynPtr->OrientMat;
		dynPtr->PrevOrientEuler = dynPtr->OrientEuler;
																 				
		/* let alien walk on walls & ceiling */
		if (AvP.PlayerType == I_Alien)
		{
			dynPtr->ToppleForce = TOPPLE_FORCE_ALIEN;	
		}
		
		/* KJL 10:56:57 11/24/97 - set ObRadius to a sensible value */
		dPtr->ObRadius = 1200;

										/*
		if (AvP.PlayerType == I_Alien) sbPtr->I_SBtype = I_BehaviourAlienPlayer;
		if (AvP.PlayerType == I_Predator) sbPtr->I_SBtype = I_BehaviourPredatorPlayer;*/
		/* KJL 18:30:09 11/11/98 - datum used for falling damage */
		{
			extern int PlayersMaxHeightWhilstNotInContactWithGround;
			PlayersMaxHeightWhilstNotInContactWithGround=dynPtr->Position.vy;
		}
	}   

	/* zero inertia values */
	psPtr->ForwardInertia=0;
	psPtr->StrafeInertia=0; 
	psPtr->TurnInertia=0; 	
	psPtr->IsMovingInWater = 0;
    PlayerDamagedOverlayIntensity = 0;
	
	/* a little addition by patrick */
	InitPlayerMovementData(sbPtr);
	
	/* security clearance */
	psPtr->securityClearances = 0;

	/* thou art mortal */
	psPtr->IsImmortal = 0;
	
	if (AvP.Network==I_No_Network)
	{
		SoundSys_FadeIn();
	}
	else
	{
		SoundSys_ResetFadeLevel();
	}

	//restore the number of saves allowed
	ResetNumberOfSaves();

	//choosing a start position now occurs later on
//	if(AvP.Network!=I_No_Network) TeleportNetPlayerToAStartingPosition(sbPtr, 1);
}

void ChangeToMarine()
{
	if(AvP.Network!=I_No_Network)
	{
		AvP.PlayerType=I_Marine;
		NetPlayerRespawn(Player->ObStrategyBlock);
		InitPlayerMovementData(Player->ObStrategyBlock);
		Player->ObStrategyBlock->DynPtr->ToppleForce=TOPPLE_FORCE_NONE;
		netGameData.myCharacterType=netGameData.myNextCharacterType=NGCT_Marine;

		//reorient the player
		{
			EULER e;
			MatrixToEuler(&Player->ObStrategyBlock->DynPtr->OrientMat,&e);
			e.EulerX=0;
			e.EulerZ=0;

			CreateEulerMatrix(&e,&Player->ObStrategyBlock->DynPtr->OrientMat);
			TransposeMatrixCH(&Player->ObStrategyBlock->DynPtr->OrientMat);

			Player->ObStrategyBlock->DynPtr->UseStandardGravity=1;
		}

		/* CDF 15/3/99, delete all discs... */
		RemoveAllThisPlayersDiscs();

	}
}
void ChangeToAlien()
{
	if(AvP.Network!=I_No_Network)
	{
		AvP.PlayerType=I_Alien;
		NetPlayerRespawn(Player->ObStrategyBlock);
		InitPlayerMovementData(Player->ObStrategyBlock);
		Player->ObStrategyBlock->DynPtr->ToppleForce=TOPPLE_FORCE_ALIEN;

		netGameData.myCharacterType=netGameData.myNextCharacterType=NGCT_Alien;

		/* CDF 15/3/99, delete all discs... */
		RemoveAllThisPlayersDiscs();
	}
}
void ChangeToPredator()
{
	if(AvP.Network!=I_No_Network)
	{
		AvP.PlayerType=I_Predator;
		NetPlayerRespawn(Player->ObStrategyBlock);
		InitPlayerMovementData(Player->ObStrategyBlock);
		Player->ObStrategyBlock->DynPtr->ToppleForce=TOPPLE_FORCE_NONE;
		netGameData.myCharacterType=netGameData.myNextCharacterType=NGCT_Predator;
		
		//reorient the player
		{
			EULER e;
			MatrixToEuler(&Player->ObStrategyBlock->DynPtr->OrientMat,&e);
			e.EulerX=0;
			e.EulerZ=0;

			CreateEulerMatrix(&e,&Player->ObStrategyBlock->DynPtr->OrientMat);
			TransposeMatrixCH(&Player->ObStrategyBlock->DynPtr->OrientMat);

			Player->ObStrategyBlock->DynPtr->UseStandardGravity=1;
		}

		/* CDF 15/3/99, delete all discs... */
		RemoveAllThisPlayersDiscs();
	}
}

void MaintainPlayer(void)
{
	int rand = FastRandom();
	PLAYER_STATUS *playerStatusPtr= (PLAYER_STATUS *) (Player->ObStrategyBlock->SBdataptr);
    
    if (playerStatusPtr->IsAlive)
	{
	    MaintainPlayersInventory();
	}

	if (ShowAdj) {
		ShowAdjacencies();
	}

	/* Set here, as first point. */
	playerNoise=0;	
	
	/* Incident handling. */
	playerStatusPtr->incidentFlag=0;

	playerStatusPtr->incidentTimer-=NormalFrameTime;
	
	if (playerStatusPtr->incidentTimer<0) {
		playerStatusPtr->incidentFlag=1;
		playerStatusPtr->incidentTimer=32767+(FastRandom()&65535);
	}

	/* CDF 9/6/98 - I can't believe this isn't done!!! */
  	Player->ObStrategyBlock->containingModule = playerPherModule;

	if (Observer) {
		textprint("Observer Mode...\n");
	}
	textprint("HtoH Strikes %d\n",HtoHStrikes);

	DoPlayerCloakingSystem();/* Patrick 22/8/97 : Cloaking stuff */
   //	HandlePredatorVisionModes();

	CurrentLightAtPlayer=LightIntensityAtPoint(&Player->ObStrategyBlock->DynPtr->Position);

	#if 1
	textprint("PlayerLight %d\n",CurrentLightAtPlayer);
	#endif

	if(AvP.Network==I_No_Network)
	{
		#if 1
		if(playerStatusPtr->Mvt_InputRequests.Flags.Rqst_PauseGame)
		{
			// go to start menu
			AvP.MainLoopRunning = 0;
		}
		#endif
	}
	else
	if(playerStatusPtr->Mvt_InputRequests.Flags.Rqst_PauseGame)
	{
		if (AvP.Network == I_Host) 
		{
			TransmitEndOfGameNetMsg();
			netGameData.myGameState = NGS_EndGame;
		}
		else if	(AvP.Network == I_Peer)	
		{
			TransmitPlayerLeavingNetMsg();
			netGameData.myGameState = NGS_Leaving;
		}		
		// go to start menu
		AvP.MainLoopRunning = 0;
	}

	//Update the player's invulnerabilty timer
	if(playerStatusPtr->invulnerabilityTimer>0)
	{
		playerStatusPtr->invulnerabilityTimer-=NormalFrameTime;
				
		if(playerStatusPtr->invulnerabilityTimer<=0)
		{
			playerStatusPtr->invulnerabilityTimer=0;
		}	
		//lose invulnerability if player is firing

		if(playerStatusPtr->Mvt_InputRequests.Flags.Rqst_FirePrimaryWeapon)
		{
			PLAYER_WEAPON_DATA *weaponPtr = &(playerStatusPtr->WeaponSlot[playerStatusPtr->SelectedWeaponSlot]);
			if(weaponPtr->WeaponIDNumber!=WEAPON_PRED_MEDICOMP)
			{
				playerStatusPtr->invulnerabilityTimer=0;
			}
		}
		if(playerStatusPtr->Mvt_InputRequests.Flags.Rqst_FireSecondaryWeapon)
		{
			//not many weapons have an offensive secondary fire
			PLAYER_WEAPON_DATA *weaponPtr = &(playerStatusPtr->WeaponSlot[playerStatusPtr->SelectedWeaponSlot]);
			if(weaponPtr->WeaponIDNumber == WEAPON_PULSERIFLE ||
			   weaponPtr->WeaponIDNumber == WEAPON_CUDGEL ||
			   weaponPtr->WeaponIDNumber == WEAPON_MARINE_PISTOL ||
			   weaponPtr->WeaponIDNumber == WEAPON_TWO_PISTOLS ||
			   weaponPtr->WeaponIDNumber == WEAPON_PRED_WRISTBLADE ||
			   weaponPtr->WeaponIDNumber == WEAPON_ALIEN_CLAW)
			{
				playerStatusPtr->invulnerabilityTimer=0;
			}
		}
	}


	if (AvP.DestructTimer>0) {
		extern int NormalFrameTime;

		AvP.DestructTimer-=NormalFrameTime;
		if (AvP.DestructTimer<0) AvP.DestructTimer=0;

	} else if (AvP.DestructTimer==0) {
		// ...Destruct?
		CauseDamageToObject(Player->ObStrategyBlock,&TemplateAmmo[AMMO_SADAR_TOW].MaxDamage[AvP.Difficulty], 25*ONE_FIXED,NULL);
		// That'll learn 'em.
	}

	/* Take speed sample. */
	{
		int speed;

		speed=Approximate3dMagnitude(&Player->ObStrategyBlock->DynPtr->LinVelocity);
		CurrentGameStats_SpeedSample(speed,NormalFrameTime);
	}

	/* Is the player on fire? */
	if (Player->ObStrategyBlock->SBDamageBlock.IsOnFire) {

		myNetworkKillerId=myIgniterId;
		CauseDamageToObject(Player->ObStrategyBlock,&firedamage,NormalFrameTime,NULL);
		myNetworkKillerId=AVPDPNetID;

		if (playerStatusPtr->soundHandle3!=SOUND_NOACTIVEINDEX) {
			if (ActiveSounds[playerStatusPtr->soundHandle3].soundIndex!=SID_FIRE) {
				Sound_Stop(playerStatusPtr->soundHandle3);
			 	Sound_Play(SID_FIRE,"dlev",&(Player->ObStrategyBlock->DynPtr->Position),&playerStatusPtr->soundHandle3,127);
			} else {
				Sound_Update3d(playerStatusPtr->soundHandle3,&(Player->ObStrategyBlock->DynPtr->Position));
			}
		} else if (playerStatusPtr->IsAlive) {
		 	Sound_Play(SID_FIRE,"dlev",&(Player->ObStrategyBlock->DynPtr->Position),&playerStatusPtr->soundHandle3,127);
		}
		
		/* Put the fire out... */

		#if 1
		{
			int speed;
			/* Go out? */
			speed=Approximate3dMagnitude(&Player->ObStrategyBlock->DynPtr->LinVelocity);

			if (speed>22000) {
				/* Jumping alien. */
				playerStatusPtr->fireTimer-=(NormalFrameTime*6);
			} else if (speed>15000) {
				/* Running alien. */
				playerStatusPtr->fireTimer-=(NormalFrameTime<<2);
			} else {
				/* Normal bloke. */
				playerStatusPtr->fireTimer-=NormalFrameTime;
			}
			
			if(playerStatusPtr->invulnerabilityTimer>0)
			{
				//player is invulnerable, so put him out.
				playerStatusPtr->fireTimer=0;
			}

			if (playerStatusPtr->fireTimer<=0) {
				/* Go out. */
				Player->ObStrategyBlock->SBDamageBlock.IsOnFire=0;
				playerStatusPtr->fireTimer=0;
			}
		}
		#else
		if (playerStatusPtr->incidentFlag) {
			int speed;
			/* Go out? */
			speed=Approximate3dMagnitude(&Player->ObStrategyBlock->DynPtr->LinVelocity);

			if (speed>15000) {
				/* Running alien. */
				if ((FastRandom()&65535)<13107) {
					Player->ObStrategyBlock->SBDamageBlock.IsOnFire=0;
				}
			} else {
				/* Normal bloke. */
				if ((FastRandom()&65535)<3000) {
					Player->ObStrategyBlock->SBDamageBlock.IsOnFire=0;
				}
			}
		}
		#endif
	} else {
		if (playerStatusPtr->soundHandle3!=SOUND_NOACTIVEINDEX) {
			Sound_Stop(playerStatusPtr->soundHandle3);
		}
	}
	
	if (playerStatusPtr->IsMovingInWater)
	{
		#if 0
		if (playerStatusPtr->soundHandle4==SOUND_NOACTIVEINDEX) {
	 		switch (rand % 4) {
				case 0:
				 	Sound_Play(SID_SPLASH1,"dev",&(Player->ObStrategyBlock->DynPtr->Position),&playerStatusPtr->soundHandle4,127);
					break;
				case 1:
				 	Sound_Play(SID_SPLASH2,"dev",&(Player->ObStrategyBlock->DynPtr->Position),&playerStatusPtr->soundHandle4,127);
					break;
				case 2:
				 	Sound_Play(SID_SPLASH3,"dev",&(Player->ObStrategyBlock->DynPtr->Position),&playerStatusPtr->soundHandle4,127);
					break;
				default:
				 	Sound_Play(SID_SPLASH4,"dev",&(Player->ObStrategyBlock->DynPtr->Position),&playerStatusPtr->soundHandle4,127);
					break;
			}
		}
		#else
		/* KJL 19:07:57 25/05/98 - make a noise at most every 1/4 of a sec */
		if (playerStatusPtr->soundHandle4<=0)
		{
	 		switch (rand&3)
	 		{
				case 0:
				 	Sound_Play(SID_SPLASH1,"d",&(Player->ObStrategyBlock->DynPtr->Position));
					break;
				case 1:
				 	Sound_Play(SID_SPLASH2,"d",&(Player->ObStrategyBlock->DynPtr->Position));
					break;
				case 2:
				 	Sound_Play(SID_SPLASH3,"d",&(Player->ObStrategyBlock->DynPtr->Position));
					break;
				default:
				 	Sound_Play(SID_SPLASH4,"d",&(Player->ObStrategyBlock->DynPtr->Position));
					break;

			}
			playerStatusPtr->soundHandle4=16384;
		}
		else
		{
			playerStatusPtr->soundHandle4-=NormalFrameTime;
		}
		#endif
	}


	/* KJL 14:54:48 25/05/98 - reset water flag to zero for next frame */
	playerStatusPtr->IsMovingInWater = 0;

	/* Taunt effects. */
	if (playerStatusPtr->tauntTimer) {
		int ex,ey,ez;
	
		playerNoise=1;
		/* An actual noise, too, would probably be good. */

		if (AvP.PlayerType==I_Alien) {

			//if (playerStatusPtr->tauntTimer>(TAUNT_LENGTH>>1)) {
			if (TauntSoundPlayed==0) {
				/* That should make sure we don't get more than one. */
				if (playerStatusPtr->soundHandle==SOUND_NOACTIVEINDEX) {
					#if 0
					Sound_Play(SID_ALIEN_SCREAM,"de",&(Player->ObStrategyBlock->DynPtr->Position),&playerStatusPtr->soundHandle);
					#else
					PlayAlienSound(0,ASC_Taunt,0,&playerStatusPtr->soundHandle,&(Player->ObStrategyBlock->DynPtr->Position));
					if(AvP.Network!=I_No_Network) netGameData.myLastScream=ASC_Taunt;
					#endif
					TauntSoundPlayed=1;
				}
			}

			/* Wave the head around? */
			ex=0;
			ey=0;
			ez=0;

			ex=MUL_FIXED(64,GetSin(((playerStatusPtr->tauntTimer>>6)&wrap360)));
			ey=MUL_FIXED(128,GetSin(((playerStatusPtr->tauntTimer>>5)&wrap360)));
			ez=MUL_FIXED(-64,GetSin(((playerStatusPtr->tauntTimer>>5)&wrap360)));
 
			ex&=wrap360;
			ey&=wrap360;
			ez&=wrap360;

			HeadOrientation.EulerX=ex;
			HeadOrientation.EulerY=ey;
			HeadOrientation.EulerZ=ez;
		} else if (AvP.PlayerType==I_Marine) {
			if (TauntSoundPlayed==0) {
				/* That should make sure we don't get more than one. */
				if (playerStatusPtr->soundHandle==SOUND_NOACTIVEINDEX) {
					PlayMarineScream(0,SC_Taunt,0,&playerStatusPtr->soundHandle,NULL);
					if(AvP.Network!=I_No_Network) netGameData.myLastScream=SC_Taunt;
					TauntSoundPlayed=1;
				}
			}
		} else if (AvP.PlayerType==I_Predator) {
			if (TauntSoundPlayed==0) {
				/* That should make sure we don't get more than one. */
				if (playerStatusPtr->soundHandle==SOUND_NOACTIVEINDEX) {
					PlayPredatorSound(0,PSC_Taunt,0,&playerStatusPtr->soundHandle,NULL);
					if(AvP.Network!=I_No_Network) netGameData.myLastScream=PSC_Taunt;
					TauntSoundPlayed=1;
				}
			}
		} else {
			GLOBALASSERT(0);
		}

	}

	/* Decay alien superhealth. */
	if (AvP.PlayerType==I_Alien) {
		NPC_DATA *NpcData;

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
		LOCALASSERT(NpcData);
		
		if (Player->ObStrategyBlock->SBDamageBlock.Health>(NpcData->StartingStats.Health<<ONE_FIXED_SHIFT)) {
			/* Decay health a bit. */
			Player->ObStrategyBlock->SBDamageBlock.Health-=(NormalFrameTime);
			if (Player->ObStrategyBlock->SBDamageBlock.Health<(NpcData->StartingStats.Health<<ONE_FIXED_SHIFT)) {
				Player->ObStrategyBlock->SBDamageBlock.Health=NpcData->StartingStats.Health<<ONE_FIXED_SHIFT;
			}
			PlayerStatusPtr->Health=Player->ObStrategyBlock->SBDamageBlock.Health;
		}
	}
}	

void PlayerIsDamaged(STRATEGYBLOCK *sbPtr, DAMAGE_PROFILE *damage, int multiplier,VECTORCH* incoming)
{
	int rand = FastRandom();
	int pitch = (rand & 255) - 128;
 	int deltaHealth;
 	int deltaArmour;
	
	/* access the extra data hanging off the strategy block */
	PLAYER_STATUS *playerStatusPtr= (PLAYER_STATUS *) (sbPtr->SBdataptr);
  GLOBALASSERT(playerStatusPtr);

	deltaHealth=playerStatusPtr->Health-sbPtr->SBDamageBlock.Health;
	deltaArmour=playerStatusPtr->Armour-sbPtr->SBDamageBlock.Armour;

	CurrentGameStats_DamageTaken(deltaHealth,deltaArmour);
	
	/* Patrick 4/8/97--------------------------------------------------
	A little hack-et to make the predator tougher in multiplayer games
	------------------------------------------------------------------*/
	//if((AvP.Network!=I_No_Network)&&(AvP.PlayerType==I_Predator)) damage>>=1;
	/* ChrisF 16/9/97 No, predators are now... wait for it... tough. */

	if (playerStatusPtr->IsAlive)
	{
		#if 0
		damage <<= 16;
		if (playerStatusPtr->Armour > 0)
		{
			if (playerStatusPtr->Armour >= damage/2)
			{
				playerStatusPtr->Armour -= damage/2;
				playerStatusPtr->Health -= damage/4;
			}
			else
			{
				damage -= playerStatusPtr->Armour*2;
				playerStatusPtr->Health -= playerStatusPtr->Armour/2 + damage;
				playerStatusPtr->Armour = 0;
			}
		}
		else
		{
			playerStatusPtr->Health -= damage;
		}
		#endif
		{
			int maxTilt = deltaHealth>>12; 
			int halfTilt = maxTilt/2;
			if (maxTilt)
			{
				HeadOrientation.EulerX = (FastRandom()%maxTilt)-halfTilt;
				HeadOrientation.EulerY = (FastRandom()%maxTilt)-halfTilt;
				HeadOrientation.EulerZ = (FastRandom()%maxTilt)-halfTilt;

				if (HeadOrientation.EulerX < 0) HeadOrientation.EulerX += 4096;
				if (HeadOrientation.EulerY < 0) HeadOrientation.EulerY += 4096;
				if (HeadOrientation.EulerZ < 0) HeadOrientation.EulerZ += 4096;
			}
		}	
	 	
		if ((playerStatusPtr->soundHandle==SOUND_NOACTIVEINDEX)&&(deltaHealth||deltaArmour)) {
	 		switch (AvP.PlayerType)
		 	{
				case I_Alien:
				{
					if ((damage->Impact==0) 		
						&&(damage->Cutting==0)  	
						&&(damage->Penetrative==0)
						&&(damage->Fire>0)
						&&(damage->Electrical==0)
						&&(damage->Acid==0)
						) {
						PlayAlienSound(0,ASC_PC_OnFire,pitch,&playerStatusPtr->soundHandle,NULL);
						if(AvP.Network!=I_No_Network) netGameData.myLastScream=ASC_PC_OnFire;
					} else {
						PlayAlienSound(0,ASC_Scream_Hurt,pitch,&playerStatusPtr->soundHandle,NULL);
						if(AvP.Network!=I_No_Network) netGameData.myLastScream=ASC_Scream_Hurt;
					}
					break;
				}
	
				case I_Marine:
				{
					if (damage->Id==AMMO_FACEHUGGER) {
						PlayMarineScream(0,SC_Facehugged,pitch,&playerStatusPtr->soundHandle,NULL);
					} else if (damage->Id==AMMO_FALLING_POSTMAX) {
						PlayMarineScream(0,SC_Falling,pitch,&playerStatusPtr->soundHandle,NULL);
					} else if ((damage->Impact==0)
						&&(damage->Cutting==0)  	
						&&(damage->Penetrative==0)
						&&(damage->Fire==0)
						&&(damage->Electrical==0)
						&&(damage->Acid>0)
						) {
						PlayMarineScream(0,SC_Acid,pitch,&playerStatusPtr->soundHandle,NULL);
						if(AvP.Network!=I_No_Network) netGameData.myLastScream=SC_Acid;
					} else if ((damage->Impact==0) 		
						&&(damage->Cutting==0)  	
						&&(damage->Penetrative==0)
						&&(damage->Fire>0)
						&&(damage->Electrical==0)
						&&(damage->Acid==0)
						) {
						PlayMarineScream(0,SC_PC_OnFire,pitch,&playerStatusPtr->soundHandle,NULL);
						if(AvP.Network!=I_No_Network) netGameData.myLastScream=SC_PC_OnFire;
					} else {
						PlayMarineScream(0,SC_Pain,pitch,&playerStatusPtr->soundHandle,NULL);
						if(AvP.Network!=I_No_Network) netGameData.myLastScream=SC_Pain;
					}
					break;
				}
	
				case I_Predator:
				{
					if (damage->Id==AMMO_FACEHUGGER) {
						PlayPredatorSound(0,PSC_Facehugged,pitch,&playerStatusPtr->soundHandle,NULL);
					} else if ((damage->Impact==0) 		
						&&(damage->Cutting==0)  	
						&&(damage->Penetrative==0)
						&&(damage->Fire==0)
						&&(damage->Electrical==0)
						&&(damage->Acid>0)
						) {
						PlayPredatorSound(0,PSC_Acid,pitch,&playerStatusPtr->soundHandle,NULL);
						if(AvP.Network!=I_No_Network) netGameData.myLastScream=PSC_Acid;
					} else if ((damage->Impact==0) 		
						&&(damage->Cutting==0)  	
						&&(damage->Penetrative==0)
						&&(damage->Fire>0)
						&&(damage->Electrical==0)
						&&(damage->Acid==0)
						) {
						PlayPredatorSound(0,PSC_PC_OnFire,pitch,&playerStatusPtr->soundHandle,NULL);
						if(AvP.Network!=I_No_Network) netGameData.myLastScream=PSC_PC_OnFire;
					} else {
						PlayPredatorSound(0,PSC_Scream_Hurt,pitch,&playerStatusPtr->soundHandle,NULL);
						if(AvP.Network!=I_No_Network) netGameData.myLastScream=PSC_Scream_Hurt;
					}
					break;
				}
	
				default:
				{
					break;
				}
			}
			playerNoise=1;
			/* Alert marines, pretty much whoever you are. */
			if (AvP.PlayerType==I_Marine) {
				PointAlert(3, &Player->ObStrategyBlock->DynPtr->Position);
			} else {
				#if 0
				PointAlert(2, &Player->ObStrategyBlock->DynPtr->Position);
				#endif
			}
		}

		{
			NPC_DATA *NpcData;
			int scaled_DeltaHealth;
			
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
					break;
			}
			/* Compute scaled deltaHealth... */
			scaled_DeltaHealth=MUL_FIXED(deltaHealth,100);
			scaled_DeltaHealth=DIV_FIXED(scaled_DeltaHealth,NpcData->StartingStats.Health);

			if (scaled_DeltaHealth>PlayerDamagedOverlayIntensity)
			{
				PlayerDamagedOverlayIntensity=scaled_DeltaHealth;
			}
		}

		if (deltaHealth>ONE_FIXED)
		{
			VECTORCH abovePosition = Global_VDB_Ptr->VDB_World;
			abovePosition.vy-=1000;
	 		switch (AvP.PlayerType)
		 	{
				case I_Alien:
					MakeBloodExplosion(&Global_VDB_Ptr->VDB_World, 127, &abovePosition, deltaHealth/ONE_FIXED, PARTICLE_ALIEN_BLOOD);
					break;
				case I_Marine:
					MakeBloodExplosion(&Global_VDB_Ptr->VDB_World, 127, &abovePosition, deltaHealth/ONE_FIXED, PARTICLE_HUMAN_BLOOD);
					break;
				case I_Predator:
					MakeBloodExplosion(&Global_VDB_Ptr->VDB_World, 127, &abovePosition, deltaHealth/ONE_FIXED, PARTICLE_PREDATOR_BLOOD);
					break;
			}
		}

		if (sbPtr->SBDamageBlock.Health <= 0)
		{
			if (playerStatusPtr->IsImmortal) sbPtr->SBDamageBlock.Health=0;
			else
			{
				PlayerIsDead(damage,multiplier,incoming);
			} 
		}

		playerStatusPtr->Health=sbPtr->SBDamageBlock.Health;
		playerStatusPtr->Armour=sbPtr->SBDamageBlock.Armour;
		
	}
}

extern EULER deathTargetOrientation;
extern int deathFadeLevel;
extern int weaponHandle;

static void PlayerIsDead(DAMAGE_PROFILE* damage,int multiplier,VECTORCH* incoming)
{
	STRATEGYBLOCK *sbPtr = Player->ObStrategyBlock;
	PLAYER_STATUS *playerStatusPtr= (PLAYER_STATUS *) (sbPtr->SBdataptr);
	DYNAMICSBLOCK *dynPtr = sbPtr->DynPtr;

	/* Stop pred hud sound. */
	if(predHUDSoundHandle != SOUND_NOACTIVEINDEX) {
		Sound_Stop(predHUDSoundHandle);
	}
	if(predOVision_SoundHandle != SOUND_NOACTIVEINDEX) {
		Sound_Stop(predOVision_SoundHandle);
	}
	if(weaponHandle != SOUND_NOACTIVEINDEX) {
		Sound_Stop(weaponHandle);
	}

  // Do some death sounds

	switch (AvP.PlayerType)
	{
		case I_Alien:
		{
			PlayAlienSound(0,ASC_Scream_Dying,0,NULL,NULL);
		  break;
		}
		case I_Marine:
		{
			if (damage->Id==AMMO_FACEHUGGER) {
				PlayMarineScream(0,SC_Facehugged,0,NULL,NULL);
			} else {
				PlayMarineScream(0,SC_Death,0,NULL,NULL);
			}
			break;
		}  
		case I_Predator:
		{
			if (damage->Id==AMMO_FACEHUGGER) {
				PlayPredatorSound(0,PSC_Facehugged,0,NULL,NULL);
			} else {
				PlayPredatorSound(0,PSC_Scream_Dying,0,NULL,NULL);
			}

			{
				VECTORCH abovePosition = Global_VDB_Ptr->VDB_World;
				abovePosition.vy-=1000;
				MakeBloodExplosion(&Global_VDB_Ptr->VDB_World, 127, &abovePosition, 200, PARTICLE_PREDATOR_BLOOD);
			}
			break;
		}
		default:
		{
			break;  
		}  
	}

	/* Well, it was nice knowing you. */
	//playerStatusPtr->Health = 0;
	sbPtr->SBDamageBlock.Health=0;
	playerStatusPtr->IsAlive = 0;

	/* make player fall to the ground */
//	Player->ObShape=I_ShapeMarinePlayerLieDown;
  //	MapBlockInit(Player);

	/* give player a little bounce */
	dynPtr->Elasticity = 16384;
	/* but a lot of friction */
//	dynPtr->Friction = 1<<20;
//	dynPtr->Mass = 1<<20;

	/* cancel any velocity */
	dynPtr->LinVelocity.vx = 0;
	dynPtr->LinVelocity.vy = 0;
	dynPtr->LinVelocity.vz = 0;

	deathTargetOrientation.EulerX = 0;//1024-(FastRandom()&255);
	deathFadeLevel=65536;
	
	/* if in single player, fade out sound... */
	if (AvP.Network==I_No_Network)
	{
		SoundSys_FadeOut();
	}

	if (AvP.PlayerType == I_Alien)
	{
		sbPtr->DynPtr->UseStandardGravity=1;
	}

	/* network support... */
	if(AvP.Network!=I_No_Network) 
	{
		playerStatusPtr->MyCorpse=MakeNewCorpse();
		AddNetMsg_PlayerKilled((*((int *)(&(playerStatusPtr->MyCorpse->SBname[4])))),damage);
		
		/*---------------------------------------------------------**
		** 		handle various special body being torn apart cases **
		**---------------------------------------------------------*/

		/*Note that we need to apply the damage to the corpse before we can determine a
		suitable death anim sequence*/
		if(damage)
		{
			HMODELCONTROLLER* hmodel=playerStatusPtr->MyCorpse->SBdptr->HModelControlBlock;
			//was the player killed by a jaw attack?
			if(damage->Id==AMMO_ALIEN_BITE_KILLSECTION && AvP.PlayerType!=I_Alien)
			{
				//knock the head of the corpse
				SECTION_DATA *head;
				head=GetThisSectionData(playerStatusPtr->MyCorpse->SBdptr->HModelControlBlock->section_data,"head");
				//do lots of damage to it
				if(head)
				{
					CauseDamageToHModel(playerStatusPtr->MyCorpse->SBdptr->HModelControlBlock,playerStatusPtr->MyCorpse, &TemplateAmmo[AMMO_ALIEN_BITE_KILLSECTION].MaxDamage[AvP.Difficulty],
						ONE_FIXED,head,NULL,NULL,0);
				}
				//play the head being bitten off sound
				Sound_Play(SID_ALIEN_JAW_ATTACK,"d",&(Player->ObStrategyBlock->DynPtr->Position));
				
			}
			//chopped by predator disc?
			else if(damage->Id==AMMO_PRED_DISC && AvP.PlayerType!=I_Predator)
			{
				SECTION_DATA *firstSectionPtr;
				SECTION_DATA *chest_section=0;
		
				firstSectionPtr=hmodel->section_data;
				LOCALASSERT(firstSectionPtr);
				LOCALASSERT(firstSectionPtr->flags&section_data_initialised);
		
				/* look for the object's torso in preference */
				chest_section =GetThisSectionData(hmodel->section_data,"chest");
		
				if (chest_section)
				{
					CauseDamageToHModel(hmodel,playerStatusPtr->MyCorpse,damage, ONE_FIXED, chest_section,incoming,&chest_section->World_Offset,0);
				}
				else
				{
					CauseDamageToObject(playerStatusPtr->MyCorpse,damage, ONE_FIXED,incoming);
				}
			}
			//blown up by shoulder cannon?
			else if(damage->Id==AMMO_PLASMACASTER_PCKILL && AvP.PlayerType!=I_Predator)
			{
				SECTION_DATA *firstSectionPtr;
				SECTION_DATA *chest_section=0;
		
				firstSectionPtr=hmodel->section_data;
				LOCALASSERT(firstSectionPtr);
				LOCALASSERT(firstSectionPtr->flags&section_data_initialised);
		
				/* look for the object's torso in preference */
				chest_section =GetThisSectionData(hmodel->section_data,"chest");
		
				if (chest_section)
				{
					//spherical blood explosion for aliens
					if(AvP.PlayerType==I_Alien)
						CauseDamageToHModel(hmodel,playerStatusPtr->MyCorpse,damage, ONE_FIXED, chest_section,NULL,&chest_section->World_Offset,0);
					else
						CauseDamageToHModel(hmodel,playerStatusPtr->MyCorpse,damage, ONE_FIXED, chest_section,incoming,&chest_section->World_Offset,0);
				}
				else
				{
					CauseDamageToObject(playerStatusPtr->MyCorpse,damage, ONE_FIXED,NULL);
				}
			}
			else
			{
				SECTION_DATA *section_data=0;
				if(MyHitBodyPartId!=-1)
				{
					//we have the id of the section that was hit when we died
					//so try to find the section
					section_data=GetThisSectionData_FromID(hmodel->section_data,MyHitBodyPartId);
				}
				//damage the corpse with the fatal blow
				if(section_data)
				{
					DISPLAYBLOCK *fragged_section=0;
					fragged_section=CauseDamageToHModel(hmodel,playerStatusPtr->MyCorpse,damage,
										multiplier,section_data,incoming,NULL,0);
					
					if(fragged_section && incoming && damage->Id==AMMO_PRED_RIFLE)
					{
					//a speargun has fragged off a body part , so we need to create a spear
						VECTORCH direction=*incoming;
						RotateVector(&direction,&Player->ObMat);
						CreateSpearPossiblyWithFragment(fragged_section,&fragged_section->ObWorld,&direction);

					}
				}
				else
				{
					CauseDamageToObject(playerStatusPtr->MyCorpse,damage,multiplier,incoming);
				}
			}
		}
		/*---------------------------------**
		** 		Choose death anim sequence **
		**---------------------------------*/
		{
			int deathId;
			switch(AvP.PlayerType)
			{
				case I_Marine :
					deathId = Deduce_PlayerMarineDeathSequence(playerStatusPtr->MyCorpse,damage,multiplier,incoming);
					break;

				case I_Alien :
					deathId = Deduce_PlayerAlienDeathSequence(playerStatusPtr->MyCorpse,damage,multiplier,incoming);
					break;

				case I_Predator :
					deathId = Deduce_PlayerPredatorDeathSequence(playerStatusPtr->MyCorpse,damage,multiplier,incoming);
					break;
			}
			//apply the animation
			ApplyCorpseDeathAnim(playerStatusPtr->MyCorpse,deathId);
			//tell everyone else about the chosen death
			AddNetMsg_PlayerDeathAnim(deathId,*(int*)&playerStatusPtr->MyCorpse->SBname[4]);
			
		}

//		if(AvP.Network==I_Host) DoNetScoresForHostDeath();

		//does this death require a change in character type?
		if(netGameData.gameType==NGT_LastManStanding)
		{
			//Am I a marine or predator?
			if(AvP.PlayerType!=I_Alien)
			{
				//was I killed by an alien?
				if(myNetworkKillerId && myNetworkKillerId!=AVPDPNetID)
				{
					int killer_index=PlayerIdInPlayerList(myNetworkKillerId);
					GLOBALASSERT(killer_index!=NET_IDNOTINPLAYERLIST);
					if(netGameData.playerData[killer_index].characterType==NGCT_Alien)
					{
						//set  the next character to be an alien then
						netGameData.myNextCharacterType=NGCT_Alien;
					}
					
				}
				else
				{
					//suicide , so become an alien anyway
						netGameData.myNextCharacterType=NGCT_Alien;
				}
			}
		}
		else if(netGameData.gameType==NGT_PredatorTag || netGameData.gameType==NGT_AlienTag)
		{
			//we may need to change character
			extern void SpeciesTag_DetermineMyNextCharacterType();
		    SpeciesTag_DetermineMyNextCharacterType();
		}
	}

	if (playerStatusPtr->soundHandle!=SOUND_NOACTIVEINDEX) {
 		Sound_Stop(playerStatusPtr->soundHandle);
	}
	if (playerStatusPtr->soundHandle3!=SOUND_NOACTIVEINDEX) {
 		Sound_Stop(playerStatusPtr->soundHandle3);
	}

	/* KJL 15:36:41 10/09/98 - don't hang around on my behalf */
	DisengageGrapplingHook();

}

void ActivateSelfDestructSequence (int seconds)
{
	// Currently does nothing, called when sequence is activated

	if (AvP.DestructTimer==-1) {
		AvP.DestructTimer=ONE_FIXED*seconds;	//2 1/2 mins.
	}
}

void DeInitialisePlayer(void) {
	/* I thought it would be logical to put it here... */
	
  	int slot = MAX_NO_OF_WEAPON_SLOTS;
#if 0
	PLAYER_STATUS *playerStatusPtr= (PLAYER_STATUS *) (Player->ObStrategyBlock->SBdataptr);
#endif

    do {
		TXACTRLBLK *txactrl,*txactrl_next;
    	PLAYER_WEAPON_DATA *wdPtr = &PlayerStatusBlock.WeaponSlot[--slot];

		txactrl_next = wdPtr->TxAnimCtrl;

		while(txactrl_next)
		{
			txactrl = txactrl_next;
			txactrl_next = txactrl->tac_next;
			DeallocateMem((void*)txactrl);
		}
	
		wdPtr->TxAnimCtrl=NULL;
	} while(slot);

	#if 0  //this hmodel isn't being set up for the moment - Richard
	Dispel_HModel(&playerStatusPtr->HModelController);
	#endif

}

static int cloakDebounce = 1;

void InitPlayerCloakingSystem(void)
{
	PLAYER_STATUS *playerStatusPtr= (PLAYER_STATUS *) (Player->ObStrategyBlock->SBdataptr);
	LOCALASSERT(playerStatusPtr);
	
	playerStatusPtr->cloakOn = 0;
	playerStatusPtr->cloakPositionGivenAway = 0;
	playerStatusPtr->CloakingEffectiveness = 0;

	playerStatusPtr->FieldCharge = PLAYERCLOAK_MAXENERGY;
	playerStatusPtr->cloakPositionGivenAwayTimer = 0;
	playerStatusPtr->PlasmaCasterCharge=0;

	GimmeChargeCalls=0;
	HtoHStrikes=0;
	CurrentLightAtPlayer=0;
}

static void DoPlayerCloakingSystem(void)
{
	extern int NormalFrameTime;
	PLAYER_STATUS *playerStatusPtr= (PLAYER_STATUS *) (Player->ObStrategyBlock->SBdataptr);
	LOCALASSERT(playerStatusPtr);

	if(AvP.PlayerType!=I_Predator) return;
	if(!(playerStatusPtr->IsAlive)) return;

	/* Handle controls. */
	if (playerStatusPtr->Mvt_InputRequests.Flags.Rqst_ChangeVision)
	{
		if (cloakDebounce)
		{
			cloakDebounce = 0;
			if (playerStatusPtr->cloakOn)
			{
				Sound_Play(SID_PRED_CLOAKOFF,"h");
				playerStatusPtr->cloakOn = 0;
				//playerNoise=1;
			} else {
				/* Check validity. */
				if ((playerStatusPtr->FieldCharge>CloakThreshold)
					&&(playerStatusPtr->FieldCharge>CloakPowerOnDrain)) {
				
					PLAYER_WEAPON_DATA *weaponPtr;
					TEMPLATE_WEAPON_DATA *twPtr;

				    weaponPtr = &(playerStatusPtr->WeaponSlot[playerStatusPtr->SelectedWeaponSlot]);
				    twPtr = &TemplateWeapon[weaponPtr->WeaponIDNumber];
					if (!(((weaponPtr->CurrentState==WEAPONSTATE_FIRING_PRIMARY)||(weaponPtr->CurrentState==WEAPONSTATE_FIRING_SECONDARY))
						&&(twPtr->FireWhenCloaked==0)))
					{
						playerStatusPtr->FieldCharge-=CloakPowerOnDrain;
						CurrentGameStats_ChargeUsed(CloakPowerOnDrain);
						playerStatusPtr->cloakOn = 1;
						playerStatusPtr->CloakingEffectiveness = 0;
						Sound_Play(SID_PRED_CLOAKON,"h");
						//playerNoise=1;
					}
				}
			}
		}
	}	
	else
	{
		cloakDebounce = 1;

		if (playerStatusPtr->cloakOn)
		{
			int maxPossibleEffectiveness;
			VECTORCH velocity;
			DYNAMICSBLOCK *dynPtr=Player->ObStrategyBlock->DynPtr;

			velocity.vx = dynPtr->Position.vx - dynPtr->PrevPosition.vx;
			velocity.vy = dynPtr->Position.vy - dynPtr->PrevPosition.vy;
			velocity.vz = dynPtr->Position.vz - dynPtr->PrevPosition.vz;

			if (playerStatusPtr->IsMovingInWater)
			{
				maxPossibleEffectiveness = 0;
				if(playerStatusPtr->soundHandleForPredatorCloakDamaged==SOUND_NOACTIVEINDEX)
					Sound_Play(SID_PREDATOR_CLOAKING_DAMAGED,"el",playerStatusPtr->soundHandleForPredatorCloakDamaged);
			}
			else
			{
				if(playerStatusPtr->soundHandleForPredatorCloakDamaged!=SOUND_NOACTIVEINDEX)
					Sound_Stop(playerStatusPtr->soundHandleForPredatorCloakDamaged);
				
				maxPossibleEffectiveness = ONE_FIXED - DIV_FIXED(Magnitude(&velocity)*2,NormalFrameTime);
				if (maxPossibleEffectiveness<0) maxPossibleEffectiveness = 0;
			}

			playerStatusPtr->CloakingEffectiveness += NormalFrameTime;
			if (playerStatusPtr->CloakingEffectiveness>maxPossibleEffectiveness)
			{
				playerStatusPtr->CloakingEffectiveness = maxPossibleEffectiveness;
			}
		}
		else
		{
			playerStatusPtr->CloakingEffectiveness -= NormalFrameTime;
			if (playerStatusPtr->CloakingEffectiveness<0)
			{
				playerStatusPtr->CloakingEffectiveness=0;
			}
			if(playerStatusPtr->soundHandleForPredatorCloakDamaged!=SOUND_NOACTIVEINDEX)
				Sound_Stop(playerStatusPtr->soundHandleForPredatorCloakDamaged);
		}

	}
	#if 1

	/* position-given-away-timer runs whatever state the cloak is in */
	if(playerStatusPtr->cloakPositionGivenAway)
	{
		playerStatusPtr->cloakPositionGivenAwayTimer-=NormalFrameTime;
		if(playerStatusPtr->cloakPositionGivenAwayTimer<=0)
		{
			playerStatusPtr->cloakPositionGivenAwayTimer = 0;
			playerStatusPtr->cloakPositionGivenAway = 0;
			playerStatusPtr->CloakingEffectiveness = 0;
		}
	}

	/* now sort out energy discharge/recharge */
	if(playerStatusPtr->cloakOn)
	{
		int chargeUsed;

		chargeUsed=MUL_FIXED(NormalFrameTime,CloakDrain);
		
		if (playerStatusPtr->IsMovingInWater) {
			chargeUsed<<=2;
		}

		if (chargeUsed>playerStatusPtr->FieldCharge) {
			chargeUsed=playerStatusPtr->FieldCharge;
		}
		playerStatusPtr->FieldCharge -= chargeUsed;
		CurrentGameStats_ChargeUsed(chargeUsed);
		if(playerStatusPtr->FieldCharge <= 0)
		{
			playerStatusPtr->FieldCharge = 0;
			playerStatusPtr->cloakOn = 0;
			playerStatusPtr->Mvt_InputRequests.Flags.Rqst_ChangeVision = 1;
			Sound_Play(SID_PRED_CLOAKOFF,"h");
			//playerNoise=1;
		}

		/* TrickleCharge Difficulty Variation! */
		if ((AvP.Difficulty==I_Hard)||(AvP.Difficulty==I_Impossible)) {
			playerStatusPtr->FieldCharge += MUL_FIXED(NormalFrameTime,(TrickleCharge>>1));
		} else {
			playerStatusPtr->FieldCharge += MUL_FIXED(NormalFrameTime,TrickleCharge);
		}

		if(playerStatusPtr->FieldCharge > PLAYERCLOAK_MAXENERGY) {
			playerStatusPtr->FieldCharge = PLAYERCLOAK_MAXENERGY;
		}

	}
	else
	{
		if(playerStatusPtr->FieldCharge < PLAYERCLOAK_MAXENERGY)
		{

			/* TrickleCharge Difficulty Variation! */
			if ((AvP.Difficulty==I_Hard)||(AvP.Difficulty==I_Impossible)) {
				playerStatusPtr->FieldCharge += MUL_FIXED(NormalFrameTime,(TrickleCharge>>1));
			} else {
				playerStatusPtr->FieldCharge += MUL_FIXED(NormalFrameTime,TrickleCharge);
			}

			if(playerStatusPtr->FieldCharge > PLAYERCLOAK_MAXENERGY) {
				playerStatusPtr->FieldCharge = PLAYERCLOAK_MAXENERGY;
			}
			
			#if 0
			/* Infinite field charge? */
			playerStatusPtr->FieldCharge = PLAYERCLOAK_MAXENERGY;
			#endif
		}	
	}
	#endif

	if (ShowPredoStats) {
		/* for testing */
		PrintDebuggingText("Cloak on: %d \n",playerStatusPtr->cloakOn);
		PrintDebuggingText("Field Charge: %d \n",playerStatusPtr->FieldCharge);
		{
			int a;
			/* Speargun ammo count */
			a=SlotForThisWeapon(WEAPON_PRED_RIFLE);
			if (a!=-1) {
				PrintDebuggingText("Speargun Rounds: %d Clips: %d\n",(playerStatusPtr->WeaponSlot[a].PrimaryRoundsRemaining>>ONE_FIXED_SHIFT),playerStatusPtr->WeaponSlot[a].PrimaryMagazinesRemaining);
			} else {
				PrintDebuggingText("Speargun not possessed.\n");
			}
            
		}
		PrintDebuggingText("Cloak given away: %d \n", playerStatusPtr->cloakPositionGivenAway);
		PrintDebuggingText("Cloak given away timer: %d \n", playerStatusPtr->cloakPositionGivenAwayTimer);
		PrintDebuggingText("Cloaking Effectiveness: %d \n", playerStatusPtr->CloakingEffectiveness);
		PrintDebuggingText("Gimme_Charge Calls: %d \n",GimmeChargeCalls);
	}
	#if 1
	/* now, if we are cloaked, lets see if we have given away our position... 
	if we have already given away our position, we may do so again */
	if(playerStatusPtr->cloakOn)
	{
		PLAYER_WEAPON_DATA *weaponPtr;
		TEMPLATE_WEAPON_DATA *twPtr;

	    weaponPtr = &(playerStatusPtr->WeaponSlot[playerStatusPtr->SelectedWeaponSlot]);
	    twPtr = &TemplateWeapon[weaponPtr->WeaponIDNumber];

		/* weapon fire ?*/
		if(((PlayerStatusPtr->WeaponSlot[PlayerStatusPtr->SelectedWeaponSlot].CurrentState==WEAPONSTATE_FIRING_PRIMARY)&&(!twPtr->PrimaryIsMeleeWeapon))||
		   ((PlayerStatusPtr->WeaponSlot[PlayerStatusPtr->SelectedWeaponSlot].CurrentState==WEAPONSTATE_FIRING_SECONDARY)&&(!twPtr->SecondaryIsMeleeWeapon)))
		{
			playerStatusPtr->cloakPositionGivenAway = 1;
			playerStatusPtr->cloakPositionGivenAwayTimer = PLAYERCLOAK_POSTIONGIVENAWAYTIME;
		}
		/* collision with npc ?*/
		{
			struct collisionreport *nextReport;
			LOCALASSERT(Player->ObStrategyBlock->DynPtr);
			nextReport = Player->ObStrategyBlock->DynPtr->CollisionReportPtr;

			while(nextReport)
			{		
 				if(nextReport->ObstacleSBPtr)
 				{	
	 				if((nextReport->ObstacleSBPtr->I_SBtype==I_BehaviourAlien)||
					   (nextReport->ObstacleSBPtr->I_SBtype==I_BehaviourMarine)||
					   (nextReport->ObstacleSBPtr->I_SBtype==I_BehaviourXenoborg)||
					   (nextReport->ObstacleSBPtr->I_SBtype==I_BehaviourPredatorAlien)||
					   (nextReport->ObstacleSBPtr->I_SBtype==I_BehaviourQueenAlien)||
					   (nextReport->ObstacleSBPtr->I_SBtype==I_BehaviourFaceHugger)||
					   (nextReport->ObstacleSBPtr->I_SBtype==I_BehaviourSeal))
	 				{
						playerStatusPtr->cloakPositionGivenAway = 1;
						playerStatusPtr->cloakPositionGivenAwayTimer = PLAYERCLOAK_POSTIONGIVENAWAYTIME;
						break;
					}
				} 		
 				nextReport = nextReport->NextCollisionReportPtr;
			}
		}		
	}
	#endif

	if (playerStatusPtr->cloakOn) {
		CurrentGameStats_CloakOn();
	}
}

void GimmeCharge(void) {

	PLAYER_STATUS *playerStatusPtr= (PLAYER_STATUS *) (Player->ObStrategyBlock->SBdataptr);
	LOCALASSERT(playerStatusPtr);

	if(AvP.PlayerType!=I_Predator) return;

	playerStatusPtr->FieldCharge = PLAYERCLOAK_MAXENERGY;

	GimmeChargeCalls++;

}

#define AUTOSPOT_RANGE	(1000)

int AlienPCIsCurrentlyVisible(int checktime,STRATEGYBLOCK *sbPtr) {

	PLAYER_WEAPON_DATA *weaponPtr;
	int range;
	DYNAMICSBLOCK *dynPtr=(Player->ObStrategyBlock->DynPtr);
	PLAYER_STATUS *playerStatusPtr= (PLAYER_STATUS *) (Player->ObStrategyBlock->SBdataptr);
	LOCALASSERT(playerStatusPtr);
	/* sbPtr is the viewer. */

	if (AvP.PlayerType!=I_Alien) {
		/* Just to be sure. */
		return(1);
	}

    weaponPtr = &(playerStatusPtr->WeaponSlot[playerStatusPtr->SelectedWeaponSlot]);

	/* Set a default range? */
	range=-1;
	/* Maybe we also need an autospot range. */
	if (sbPtr) {
		VECTORCH offset;

		offset.vx=dynPtr->Position.vx-sbPtr->DynPtr->Position.vx;
		offset.vy=dynPtr->Position.vy-sbPtr->DynPtr->Position.vy;
		offset.vz=dynPtr->Position.vz-sbPtr->DynPtr->Position.vz;

		range=Approximate3dMagnitude(&offset);

		/* In fact, let's turn this around... */
	
		if (playerStatusPtr->ShapeState==PMph_Standing) {
			if (range>=(CurrentLightAtPlayer)+AUTOSPOT_RANGE) {
				return(0);
			}
		} else {
			if (range>=(CurrentLightAtPlayer>>1)+AUTOSPOT_RANGE) {
				/* Half that range if crouching? */
				return(0);
			}
		}

		/* I guess, if we're in _pitch_ darkness... see nothing outside that range? */
		if (CurrentLightAtPlayer<=2560) {
			if (playerStatusPtr->ShapeState==PMph_Standing) {
				if (range<=(CurrentLightAtPlayer)+AUTOSPOT_RANGE) {
					return(1);
				}
			} else {
				if (range<=(CurrentLightAtPlayer>>1)+AUTOSPOT_RANGE) {
					/* Half that range if crouching? */
					return(1);
				}
			}
			return(0);
		}	

	}


	if (playerStatusPtr->ShapeState==PMph_Standing) {
		/* Default to visible, unless stationary and in near darkness. */
		if (weaponPtr->CurrentState!=WEAPONSTATE_IDLE) {
			/* Don't even breathe. */
			return(1);
		}
		if ((dynPtr->Position.vx!=dynPtr->PrevPosition.vx)||
			(dynPtr->Position.vy!=dynPtr->PrevPosition.vy)||
			(dynPtr->Position.vz!=dynPtr->PrevPosition.vz)) {
			/* Stand perfectly still. */
			return(1);
		}
		if (checktime) {
			int chance;
			/* There is a chance of being seen regardless. */
			chance=32767; /* Lets say. */
			if ((FastRandom()&65535)<chance) {
				return(1);
			}
		}
		if (CurrentLightAtPlayer<5000) {
			return(0);
		}
		return(1);
	} else {
		int speed;

		/* Crouching.  Default to cloaked, unless they get 'lucky'. */
		if (weaponPtr->CurrentState==WEAPONSTATE_FIRING_PRIMARY) {
			/* Just don't claw. */
			return(1);
		}

		if (CurrentLightAtPlayer>17000) {
			/* It's too bright, mere crouching won't save you. */
			return(1);
		}
		#if 0
		if ((dynPtr->Position.vx!=dynPtr->PrevPosition.vx)||
			(dynPtr->Position.vy!=dynPtr->PrevPosition.vy)||
			(dynPtr->Position.vz!=dynPtr->PrevPosition.vz)) {
			/* Stand perfectly still? */
			return(1);
		}
		#else
		speed=Approximate3dMagnitude(&Player->ObStrategyBlock->DynPtr->LinVelocity);
		/* Speed: < 5000 = standing.  > 22000 = jumping.
			In practice, walk/fall = 18000 ish, jump = 27000 ish. */
		if (speed>22000) {
			/* Not getting away with that, mate. */
			return(1);
		}
		speed-=5000;
		if (speed<0) {
			speed=0;
		}
		/* Now should be 0->17000 ish. */
		speed<<=1;
		#endif
		if (checktime) {
			int chance;
			/* There is a chance of being seen. */
			chance=CurrentLightAtPlayer-5000;
			#if 1
			chance+=speed;
			#endif
			if ((FastRandom()&65535)<chance) {
				return(1);
			}
		}
		return(0);
	}
}

#if 0
static void HandlePredatorVisionModes(void)
{
	extern unsigned char DebouncedKeyboardInput[];
	
	if (DebouncedKeyboardInput[KEY_K])
	{
		ChangePredatorVisionMode();
	}
}
#endif

void ShowAdjacencies(void) {

	int moduleCounter;
	AIMODULE **AdjModuleRefPtr;
	AIMODULE *thisModule;
	AIMODULE *ModuleListPointer;	

	/* Utility function... */
	PrintDebuggingText("Adjacencies FROM Player's Module (%s):\n",
		(*(playerPherModule->m_aimodule->m_module_ptrs))->name);

	thisModule=playerPherModule->m_aimodule;

	AdjModuleRefPtr = thisModule->m_link_ptrs;
    
    if(AdjModuleRefPtr)     /* check that there is a list of adjacent modules */
    {
    	while(*AdjModuleRefPtr != 0)
	    {
			/* Probably want some validity test for the link. */
			if (AIModuleIsPhysical(*AdjModuleRefPtr)) {
				/* Probably a valid link... */
				if (CheckAdjacencyValidity((*AdjModuleRefPtr),thisModule,0)) {
					PrintDebuggingText("--AI Module of %s, general.\n",(*((*AdjModuleRefPtr)->m_module_ptrs))->name);
				} else if (CheckAdjacencyValidity((*AdjModuleRefPtr),thisModule,1)) {
					PrintDebuggingText("--AI Module of %s, alien only.\n",(*((*AdjModuleRefPtr)->m_module_ptrs))->name);
				} else {
					PrintDebuggingText("--AI Module of %s, fails validity test!\n",(*((*AdjModuleRefPtr)->m_module_ptrs))->name);
				}
			}
			/* next adjacent module reference pointer */
			AdjModuleRefPtr++;
		}
	}

	PrintDebuggingText("Adjacencies TO Player's Module:\n");

	{
		extern AIMODULE *AIModuleArray;

		ModuleListPointer = AIModuleArray;
	}

	/* go through each aimodule in the environment  */	
	for(moduleCounter = 0; moduleCounter < AIModuleArraySize; moduleCounter++)
	{

		/* get a pointer to the next current module */
		thisModule = &(ModuleListPointer[moduleCounter]); 
		LOCALASSERT(thisModule);

		AdjModuleRefPtr = thisModule->m_link_ptrs;
	    
	    if(AdjModuleRefPtr)     /* check that there is a list of adjacent modules */
	    {
	    	while(*AdjModuleRefPtr != 0)
		    {
				/* Probably want some validity test for the link. */
				if (AIModuleIsPhysical(*AdjModuleRefPtr)) {
					/* Is this the target? */
					if ((*AdjModuleRefPtr)==(playerPherModule->m_aimodule)) {

						/* Probably a valid link... */
						if (CheckAdjacencyValidity((*AdjModuleRefPtr),thisModule,0)) {
							PrintDebuggingText("--AI Module of %s, general.\n",(*(thisModule->m_module_ptrs))->name);
						} else if (CheckAdjacencyValidity((*AdjModuleRefPtr),thisModule,1)) {
							PrintDebuggingText("--AI Module of %s, alien only.\n",(*(thisModule->m_module_ptrs))->name);
						} else {
							PrintDebuggingText("--AI Module of %s, fails validity test!\n",(*(thisModule->m_module_ptrs))->name);
						}

					}
				}
				/* next adjacent module reference pointer */
				AdjModuleRefPtr++;
			}
		}

	}

}



/*---------------------------**
** Loading and saving player **
**---------------------------*/


typedef struct player_save_block
{
	SAVE_BLOCK_STRATEGY_HEADER header;

//behaviour block things
	PLAYER_WEAPON_DATA	WeaponSlot[MAX_NO_OF_WEAPON_SLOTS];

	enum WEAPON_SLOT	SelectedWeaponSlot;
	enum WEAPON_SLOT	SwapToWeaponSlot;
	enum WEAPON_SLOT	PreviouslySelectedWeaponSlot;
    
    int	Health;	 /* in 16.16 */
	int	Energy;	 /* in 16.16 */
	int	Armour;	 /* in 16.16 */
 
		
	enum player_morph_state ShapeState;		/* for controlling morphing */
	
	signed int ForwardInertia;
	signed int StrafeInertia; 
	signed int TurnInertia; 	

	int ViewPanX; /* the looking up/down value that used to be in displayblock */

	unsigned int securityClearances;

	unsigned int IsAlive :1;
	unsigned int IsImmortal :1;
	unsigned int Mvt_AnalogueTurning :1;
	unsigned int Mvt_AnaloguePitching :1;
	unsigned int Absolute_Pitching :1;
	unsigned int SwappingIsDebounced :1;
	unsigned int DemoMode :1;
	unsigned int IHaveAPlacedAutogun :1;
	unsigned int IsMovingInWater :1;
	unsigned int JetpackEnabled :1;
	unsigned int GrapplingHookEnabled :1;

	unsigned int MTrackerType;

	unsigned int cloakOn :1;
	unsigned int cloakPositionGivenAway :1;
	int FieldCharge;
	int cloakPositionGivenAwayTimer;
	int PlasmaCasterCharge;

	int CloakingEffectiveness; 

	ENCUMBERANCE_STATE Encumberance;
	int tauntTimer;

	int incidentFlag;
	int incidentTimer;
	int fireTimer;

//some stuff for the weapon displayblock
	VECTORCH Weapon_World;
	EULER Weapon_Euler;
	MATRIXCH Weapon_Matrix;


//and  globals
	enum VISION_MODE_ID CurrentVisionMode;
	SMARTGUN_MODES SmartgunMode;
	GRENADE_LAUNCHER_DATA GrenadeLauncherData;
	

//strategy block stuff
	DYNAMICSBLOCK dynamics;
	int integrity;
	DAMAGEBLOCK SBDamageBlock;

}PLAYER_SAVE_BLOCK;

//defines for load/save macros
#define SAVELOAD_BLOCK block
#define SAVELOAD_BEHAV playerStatusPtr


void SaveStrategy_Player(STRATEGYBLOCK* sbPtr)
{
	PLAYER_STATUS *playerStatusPtr= (PLAYER_STATUS *) (sbPtr->SBdataptr);
	PLAYER_SAVE_BLOCK* block;
	int i;

	GET_STRATEGY_SAVE_BLOCK(block,sbPtr);


	COPYELEMENT_SAVE(SelectedWeaponSlot)
	COPYELEMENT_SAVE(SwapToWeaponSlot)
	COPYELEMENT_SAVE(PreviouslySelectedWeaponSlot)
    
    COPYELEMENT_SAVE(Health)	 /* in 16.16 */
	COPYELEMENT_SAVE(Energy)	 /* in 16.16 */
	COPYELEMENT_SAVE(Armour)	 /* in 16.16 */
 
		
	COPYELEMENT_SAVE(ShapeState)		/* for controlling morphing */
	
	COPYELEMENT_SAVE(ForwardInertia)
	COPYELEMENT_SAVE(StrafeInertia) 
	COPYELEMENT_SAVE(TurnInertia) 	

	COPYELEMENT_SAVE(ViewPanX) /* the looking up/down value that used to be in displayblock */

	COPYELEMENT_SAVE(securityClearances)

	COPYELEMENT_SAVE(IsAlive)
	COPYELEMENT_SAVE(IsImmortal)
	COPYELEMENT_SAVE(Mvt_AnalogueTurning)
	COPYELEMENT_SAVE(Mvt_AnaloguePitching)
	COPYELEMENT_SAVE(Absolute_Pitching)
	COPYELEMENT_SAVE(SwappingIsDebounced)
	COPYELEMENT_SAVE(DemoMode)
	COPYELEMENT_SAVE(IHaveAPlacedAutogun)
	COPYELEMENT_SAVE(IsMovingInWater)
	COPYELEMENT_SAVE(JetpackEnabled)
	COPYELEMENT_SAVE(GrapplingHookEnabled )

	COPYELEMENT_SAVE(MTrackerType)

	COPYELEMENT_SAVE(cloakOn)
	COPYELEMENT_SAVE(cloakPositionGivenAway)
	COPYELEMENT_SAVE(FieldCharge)
	COPYELEMENT_SAVE(cloakPositionGivenAwayTimer)
	COPYELEMENT_SAVE(PlasmaCasterCharge)
	COPYELEMENT_SAVE(CloakingEffectiveness) 
	COPYELEMENT_SAVE(Encumberance)
	COPYELEMENT_SAVE(tauntTimer)

	COPYELEMENT_SAVE(incidentFlag)
	COPYELEMENT_SAVE(incidentTimer)
	COPYELEMENT_SAVE(fireTimer)
	
	for(i=0;i<MAX_NO_OF_WEAPON_SLOTS;i++)
	{
		block->WeaponSlot[i].WeaponIDNumber = playerStatusPtr->WeaponSlot[i].WeaponIDNumber; 		
		block->WeaponSlot[i].CurrentState = playerStatusPtr->WeaponSlot[i].CurrentState; 		
		block->WeaponSlot[i].StateTimeOutCounter = playerStatusPtr->WeaponSlot[i].StateTimeOutCounter; 		
		block->WeaponSlot[i].PrimaryRoundsRemaining = playerStatusPtr->WeaponSlot[i].PrimaryRoundsRemaining; 		
		block->WeaponSlot[i].SecondaryRoundsRemaining = playerStatusPtr->WeaponSlot[i].SecondaryRoundsRemaining; 		
		block->WeaponSlot[i].PrimaryMagazinesRemaining = playerStatusPtr->WeaponSlot[i].PrimaryMagazinesRemaining;	
		block->WeaponSlot[i].SecondaryMagazinesRemaining = playerStatusPtr->WeaponSlot[i].SecondaryMagazinesRemaining; 		
		block->WeaponSlot[i].PositionOffset = playerStatusPtr->WeaponSlot[i].PositionOffset;  		
		block->WeaponSlot[i].DirectionOffset = playerStatusPtr->WeaponSlot[i].DirectionOffset; 		
		block->WeaponSlot[i].Possessed = playerStatusPtr->WeaponSlot[i].Possessed; 		
	}

	//some stuff for the weapon displayblock
	block->Weapon_World = PlayersWeapon.ObWorld;
	block->Weapon_Euler = PlayersWeapon.ObEuler;
	block->Weapon_Matrix = PlayersWeapon.ObMat;

	//global
	block->CurrentVisionMode = CurrentVisionMode;
	block->SmartgunMode = SmartgunMode;
	block->GrenadeLauncherData = GrenadeLauncherData;

	//strategy block stuff

	block->integrity = sbPtr->integrity;
	block->SBDamageBlock = sbPtr->SBDamageBlock;
	
	block->dynamics = *sbPtr->DynPtr;
	block->dynamics.CollisionReportPtr=0;

	//save the weapon hierarchy
	SaveHierarchy(&PlayersWeaponHModelController);


	Save_SoundState(&playerStatusPtr->soundHandle);
	Save_SoundState(&playerStatusPtr->soundHandle3);
	Save_SoundState(&playerStatusPtr->soundHandle5);
	Save_SoundState(&playerStatusPtr->soundHandleForPredatorCloakDamaged);
}

void LoadStrategy_Player(SAVE_BLOCK_STRATEGY_HEADER* header)
{
	PLAYER_SAVE_BLOCK* block = (PLAYER_SAVE_BLOCK*) header;
	STRATEGYBLOCK* sbPtr = Player->ObStrategyBlock;
	PLAYER_STATUS *playerStatusPtr= (PLAYER_STATUS *) (sbPtr->SBdataptr);
	int i;

	if(block->header.size != sizeof(*block)) return;

	COPY_NAME(sbPtr->SBname,header->SBname);



	COPYELEMENT_LOAD(SelectedWeaponSlot)
	COPYELEMENT_LOAD(SwapToWeaponSlot)
	COPYELEMENT_LOAD(PreviouslySelectedWeaponSlot)
    
    COPYELEMENT_LOAD(Health)	 /* in 16.16 */
	COPYELEMENT_LOAD(Energy)	 /* in 16.16 */
	COPYELEMENT_LOAD(Armour)	 /* in 16.16 */
 
		
	COPYELEMENT_LOAD(ShapeState)		/* for controlling morphing */
	
	COPYELEMENT_LOAD(ForwardInertia)
	COPYELEMENT_LOAD(StrafeInertia) 
	COPYELEMENT_LOAD(TurnInertia) 	

	COPYELEMENT_LOAD(ViewPanX) /* the looking up/down value that used to be in displayblock */

	COPYELEMENT_LOAD(securityClearances)

	COPYELEMENT_LOAD(IsAlive)
	COPYELEMENT_LOAD(IsImmortal)
	COPYELEMENT_LOAD(Mvt_AnalogueTurning)
	COPYELEMENT_LOAD(Mvt_AnaloguePitching)
	COPYELEMENT_LOAD(Absolute_Pitching)
	COPYELEMENT_LOAD(SwappingIsDebounced)
	COPYELEMENT_LOAD(DemoMode)
	COPYELEMENT_LOAD(IHaveAPlacedAutogun)
	COPYELEMENT_LOAD(IsMovingInWater)
	COPYELEMENT_LOAD(JetpackEnabled)
	COPYELEMENT_LOAD(GrapplingHookEnabled )

	COPYELEMENT_LOAD(MTrackerType)

	COPYELEMENT_LOAD(cloakOn)
	COPYELEMENT_LOAD(cloakPositionGivenAway)
	COPYELEMENT_LOAD(FieldCharge)
	COPYELEMENT_LOAD(cloakPositionGivenAwayTimer)
	COPYELEMENT_LOAD(PlasmaCasterCharge)
	COPYELEMENT_LOAD(CloakingEffectiveness) 
	COPYELEMENT_LOAD(Encumberance)
	COPYELEMENT_LOAD(tauntTimer)

	COPYELEMENT_LOAD(incidentFlag)
	COPYELEMENT_LOAD(incidentTimer)
	COPYELEMENT_LOAD(fireTimer)

//	playerStatusPtr->SwapToWeaponSlot = block->SelectedWeaponSlot;
	


	for(i=0;i<MAX_NO_OF_WEAPON_SLOTS;i++)
	{
		playerStatusPtr->WeaponSlot[i].WeaponIDNumber = block->WeaponSlot[i].WeaponIDNumber; 		
		playerStatusPtr->WeaponSlot[i].CurrentState = block->WeaponSlot[i].CurrentState; 		
		playerStatusPtr->WeaponSlot[i].StateTimeOutCounter = block->WeaponSlot[i].StateTimeOutCounter; 		
		playerStatusPtr->WeaponSlot[i].PrimaryRoundsRemaining = block->WeaponSlot[i].PrimaryRoundsRemaining; 		
		playerStatusPtr->WeaponSlot[i].SecondaryRoundsRemaining = block->WeaponSlot[i].SecondaryRoundsRemaining; 		
		playerStatusPtr->WeaponSlot[i].PrimaryMagazinesRemaining = block->WeaponSlot[i].PrimaryMagazinesRemaining;	
		playerStatusPtr->WeaponSlot[i].SecondaryMagazinesRemaining = block->WeaponSlot[i].SecondaryMagazinesRemaining; 		
		playerStatusPtr->WeaponSlot[i].PositionOffset = block->WeaponSlot[i].PositionOffset;  		
		playerStatusPtr->WeaponSlot[i].DirectionOffset = block->WeaponSlot[i].DirectionOffset; 		
		playerStatusPtr->WeaponSlot[i].Possessed = block->WeaponSlot[i].Possessed; 		
	}
	
	//some stuff for the weapon displayblock
	PlayersWeapon.ObWorld = block->Weapon_World;
	PlayersWeapon.ObEuler = block->Weapon_Euler;
	PlayersWeapon.ObMat = block->Weapon_Matrix;

	//global
	CurrentVisionMode = block->CurrentVisionMode;
	SmartgunMode = block->SmartgunMode;
	GrenadeLauncherData = block->GrenadeLauncherData;
	
	//strategy block stuff
	*sbPtr->DynPtr = block->dynamics;
	sbPtr->integrity = block->integrity;
	sbPtr->SBDamageBlock = block->SBDamageBlock;

	
	
	
	{
		extern VIEWDESCRIPTORBLOCK* Global_VDB_Ptr;
	   	sbPtr->containingModule = ModuleFromPosition(&(sbPtr->DynPtr->Position), (MODULE*)0);
		Global_VDB_Ptr->VDB_World = sbPtr->DynPtr->Position;
	}


	//load the weapon hierarchy
	{
		SAVE_BLOCK_HEADER* hier_header = GetNextBlockIfOfType(SaveBlock_Hierarchy);
		if(hier_header)
		{
			LoadHierarchy(hier_header,&PlayersWeaponHModelController);
		}
		else
		{
			Dispel_HModel(&PlayersWeaponHModelController);
		}
	}


	PlayersWeapon.HModelControlBlock = &PlayersWeaponHModelController;

	//get the section data pointers
	PWMFSDP=GetThisSectionData(PlayersWeaponHModelController.section_data,"dum flash");
	if (PWMFSDP==NULL) {
		PWMFSDP=GetThisSectionData(PlayersWeaponHModelController.section_data,"Dum flash");
	}

	Load_SoundState(&playerStatusPtr->soundHandle);
	Load_SoundState(&playerStatusPtr->soundHandle3);
	Load_SoundState(&playerStatusPtr->soundHandle5);
	Load_SoundState(&playerStatusPtr->soundHandleForPredatorCloakDamaged);

}

