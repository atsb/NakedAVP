/*-------------- Patrick 15/10/96 ------------------
	Source file for Player Movement ...
----------------------------------------------------*/
#include "3dc.h"
#include "inline.h"
#include "module.h"

#include "gamedef.h"
#include "stratdef.h"
#include "dynblock.h"
#include "dynamics.h"
#include "gameplat.h"

#include "bh_types.h"

#define UseLocalAssert 1
#include "ourasert.h"
#include "comp_shp.h"

#include "pmove.h"
#include "usr_io.h"
#include "bh_far.h"
#include "triggers.h"
#include "pvisible.h"
#include "inventry.h"
#include "pfarlocs.h"
#include "weapons.h"
#include "pheromon.h"
#include "bh_pred.h"
#include "psnd.h"
#include "bh_weap.h"
#include "equipmnt.h"
#include "bh_agun.h"
#include "los.h"
#include "pldnet.h"
#include "bonusabilities.h"
#include "avp_menus.h"
#include "lighting.h"
#include "scream.h"
#include "player.h"
#include "avp_userprofile.h"


#define ALIEN_CONTACT_WEAPON 0
#if ALIEN_CONTACT_WEAPON
static void AlienContactWeapon(void);
#endif

#ifdef AVP_DEBUG_VERSION
	#define FLY_MODE_CHEAT_ON 1
#else
	#ifdef AVP_DEBUG_FOR_FOX
		#define FLY_MODE_CHEAT_ON 1
	#else
		#define FLY_MODE_CHEAT_ON 0
	#endif
#endif
//!(PREDATOR_DEMO||MARINE_DEMO||ALIEN_DEMO||DEATHMATCH_DEMO)
#if FLY_MODE_CHEAT_ON
extern unsigned char KeyboardInput[];
#endif
extern int DebouncedGotAnyKey;

/*KJL*****************************************************
* If the define below is set to non-zero then the player *
* movement values will be loaded in from movement.txt	 *
*****************************************************KJL*/
#define LOAD_IN_MOVEMENT_VALUES 0

#if LOAD_IN_MOVEMENT_VALUES	

static int AlienForwardSpeed;
static int AlienStrafeSpeed;
static int AlienTurnSpeed;	
static int AlienJumpSpeed;
static int PredatorForwardSpeed;
static int PredatorStrafeSpeed;
static int PredatorTurnSpeed;	
static int PredatorJumpSpeed;
static int MarineForwardSpeed;
static int MarineStrafeSpeed;
static int MarineTurnSpeed;	
static int MarineJumpSpeed;

static void LoadInMovementValues(void);
#endif

/* Globals */
int CrouchIsToggleKey;
char CrouchKeyDebounced;
int executeDemo;

/* Global Externs */
extern DISPLAYBLOCK* Player;
extern int NormalFrameTime;
extern int predHUDSoundHandle;
extern int predOVision_SoundHandle;
extern int TauntSoundPlayed;

extern unsigned char GotAnyKey;

static char FlyModeOn = 0;			
#if FLY_MODE_CHEAT_ON
static char FlyModeDebounced = 0;
#endif

#if 0
static char BonusAbilityDebounced = 0;
static void MakePlayerLieDown(STRATEGYBLOCK* sbPtr);
#endif

extern int deathFadeLevel;
extern VIEWDESCRIPTORBLOCK *Global_VDB_Ptr;

// DISPLAYBLOCK *playerdb;

extern void DeInitialisePlayer(void);

/* some prototypes for this source file */
static void MakePlayerCrouch(STRATEGYBLOCK* sbPtr);
static void MaintainPlayerShape(STRATEGYBLOCK* sbPtr);
static void NetPlayerDeadProcessing(STRATEGYBLOCK* sbPtr);
static void CorpseMovement(STRATEGYBLOCK *sbPtr);

extern SECTION * GetNamedHierarchyFromLibrary(const char * rif_name, const char * hier_name);
extern void NewOnScreenMessage(unsigned char *messagePtr);
extern void RemoveAllThisPlayersDiscs(void);

int timeInContactWithFloor;

extern int weaponHandle;

extern int PlayerDamagedOverlayIntensity;


#define JETPACK_MAX_SPEED 10000
#define JETPACK_THRUST 40000

/*----------------------------------------------------------- 
Initialise player movement data
-------------------------------------------------------------*/
void InitPlayerMovementData(STRATEGYBLOCK* sbPtr)
{
	InitPlayerGameInput(sbPtr);
	
	/* set the player's morph control block and state*/
	{
		PLAYER_STATUS *playerStatusPtr = (PLAYER_STATUS *)(sbPtr->SBdataptr);    
    	LOCALASSERT(playerStatusPtr);

		playerStatusPtr->ShapeState = PMph_Standing;
		playerStatusPtr->ViewPanX = 0;
	
		playerStatusPtr->DemoMode = 0;
	}
	
	/* KJL 13:35:13 16/03/98 - make sure fly mode is off */
	FlyModeOn = 0;
	
	timeInContactWithFloor=(ONE_FIXED/10);

	#if LOAD_IN_MOVEMENT_VALUES	
	LoadInMovementValues();
	#endif

}

void StartPlayerTaunt(void) {

	PLAYER_STATUS *playerStatusPtr;
    
	/* get the player status block ... */
	playerStatusPtr = (PLAYER_STATUS *) (Player->ObStrategyBlock->SBdataptr);
    GLOBALASSERT(playerStatusPtr);
	
	if (playerStatusPtr->tauntTimer) {
		return;
	}

	playerStatusPtr->tauntTimer=-1; /* Cue to start. */
	TauntSoundPlayed=0;
}

/*-------------- Patrick 15/10/96 ----------------
--------------------------------------------------*/
void PlayerBehaviour(STRATEGYBLOCK* sbPtr)
{
	PLAYER_STATUS *playerStatusPtr;
    
	/* get the player status block ... */
	playerStatusPtr = (PLAYER_STATUS *) (sbPtr->SBdataptr);
    GLOBALASSERT(playerStatusPtr);
  
    /* KJL 18:05:55 03/10/97 - is anybody there? */
    if (playerStatusPtr->IsAlive)
	{
		if (playerStatusPtr->tauntTimer>0) {
			playerStatusPtr->tauntTimer-=NormalFrameTime;
			if (playerStatusPtr->tauntTimer<0) {
				playerStatusPtr->tauntTimer=0;
			}
		} else if (AvP.Network==I_No_Network) {
			/* *Might* need to monitor this... */
			if (playerStatusPtr->tauntTimer==-1) {
				/* Begin taunt. */
				playerStatusPtr->tauntTimer=TAUNT_LENGTH;
			} else if (playerStatusPtr->tauntTimer>0) {
				playerStatusPtr->tauntTimer-=NormalFrameTime;
				if (playerStatusPtr->tauntTimer<0) {
					playerStatusPtr->tauntTimer=0;
				}
			}
		}
		ExecuteFreeMovement(sbPtr);
	}
	else CorpseMovement(sbPtr);

	if(playerStatusPtr->IsAlive)
	{
		if ((sbPtr->containingModule)&&(!Observer)) {
			/* Update pheromone system. If there's no containing module,           *
			 * well... I sigh with despair at the system.  But I cannot change it. */
			
			switch(AvP.PlayerType)
			{
				case I_Marine:
					AddMarinePheromones(sbPtr->containingModule->m_aimodule);
					break;
				case I_Predator:
					/* Ah well, for the moment... */
					AddMarinePheromones(sbPtr->containingModule->m_aimodule);
					break;
				case I_Alien:
					break;
				default:
					GLOBALASSERT(0);
					break;
			}
		}
	}

}




/*------------------------Patrick 21/10/96------------------------
  Newer cleaned up version, supporting new input functions
  ----------------------------------------------------------------*/
#define ALIEN_MOVESCALE 18000
#define PREDATOR_MOVESCALE 16000
#define MARINE_MOVESCALE 15000

#define TURNSCALE 2000
#define JUMPVELOCITY 9000

#define FASTMOVESCALE 12000
#define SLOWMOVESCALE 8000
#define FASTTURNSCALE 2000
#define SLOWTURNSCALE 1000
#define FASTSTRAFESCALE 10000
#define SLOWSTRAFESCALE 6000

/* KJL 14:39:45 01/14/97 - Camera stuff */
#define	PANRATESHIFT 6	
#define TIMEBEFOREAUTOCENTREVIEW 16384

/* patrick 9/7/97: these are for testing AI pre-calculated values... */
#define PATTEST_EPS	0
#define PATTEST_AUXLOCS 0
#if (PATTEST_EPS&&PATTEST_AUXLOCS)
	#error Cannot have both
#endif 
#if PATTEST_EPS
	void EpLocationTest(void);
#endif
#if PATTEST_AUXLOCS
	void AuxLocationTest(void);
#endif

void ExecuteFreeMovement(STRATEGYBLOCK* sbPtr)
{
	DYNAMICSBLOCK *dynPtr = sbPtr->DynPtr;

	/* access the extra data hanging off the strategy block */
	PLAYER_STATUS *playerStatusPtr= (PLAYER_STATUS *) (sbPtr->SBdataptr);

	if (dynPtr->IsInContactWithFloor) {
		timeInContactWithFloor+=NormalFrameTime;
	} else {
		timeInContactWithFloor=0;
	}
	
	/*------------------------------------------------------ 
	GAME INPUTS 
	Call the (platform dependant) game input reading fn.
	------------------------------------------------------*/ 
	ReadPlayerGameInput(sbPtr);
 
	/* KJL 11:07:42 10/09/98 - Bonus Abilities */
	switch (AvP.PlayerType)
	{
		case I_Alien:
			break;
		#if 0
		case I_Predator: /* KJL 11:08:19 10/09/98 - Grappling Hook */
		{
			if (playerStatusPtr->Mvt_InputRequests.Flags.Rqst_BonusAbility)
			{
				if(BonusAbilityDebounced)
				{
					ActivateGrapplingHook();
					BonusAbilityDebounced = 0;
				}
			}
			else BonusAbilityDebounced = 1;
			
			break;
		}
		#endif
		case I_Predator: /* KJL 11:08:19 10/09/98 - Cycle Vision Mode */
		{
			if (playerStatusPtr->Mvt_InputRequests.Flags.Rqst_CycleVisionMode)
			{
				ChangePredatorVisionMode();
			}
			if (playerStatusPtr->Mvt_InputRequests.Flags.Rqst_GrapplingHook && 
				playerStatusPtr->GrapplingHookEnabled)
			{
				ActivateGrapplingHook();
			}

			break;
		}
		case I_Marine:
			break;
	}

	if (playerStatusPtr->Mvt_InputRequests.Flags.Rqst_Operate)
		OperateObjectInLineOfSight();
			
	/* patrick 9/7/97: these are for testing AI pre-calculated values... */
	#if PATTEST_EPS
		EpLocationTest();
	#endif
	#if PATTEST_AUXLOCS
		AuxLocationTest();
	#endif
	

	/* Alien damages things by being in contact with them */
	#if ALIEN_CONTACT_WEAPON
	if (AvP.PlayerType == I_Alien) AlienContactWeapon();
	#endif

	/*------------------------------------------------------ 
	MOVEMENT

	NB player must be standing for faster movement
	------------------------------------------------------*/ 
	
	/* KJL 16:59:53 01/07/97 - New 3d strategy code	*/
	{
		int MaxSpeed;
		int forwardSpeed;
		int strafeSpeed; 
		int turnSpeed; 	
		int jumpSpeed;

		#if LOAD_IN_MOVEMENT_VALUES	
		switch (AvP.PlayerType)
		{
			case I_Alien:
				forwardSpeed = AlienForwardSpeed;
				strafeSpeed  = AlienStrafeSpeed;
				turnSpeed    = AlienTurnSpeed;	
				jumpSpeed    = AlienJumpSpeed;
				break;
			
			case I_Predator:
				forwardSpeed = PredatorForwardSpeed;
				strafeSpeed  = PredatorStrafeSpeed;
				turnSpeed    = PredatorTurnSpeed;	
				jumpSpeed    = PredatorJumpSpeed;
				break;
			
			case I_Marine:
				forwardSpeed = MarineForwardSpeed;
				strafeSpeed  = MarineStrafeSpeed;
				turnSpeed    = MarineTurnSpeed;	
				jumpSpeed    = MarineJumpSpeed;
				break;
		}
		#else
		switch (AvP.PlayerType)
		{
			case I_Alien:
				forwardSpeed = ALIEN_MOVESCALE;
				strafeSpeed = ALIEN_MOVESCALE;
				turnSpeed =	TURNSCALE;
				jumpSpeed = JUMPVELOCITY;
				break;
			case I_Predator:
				forwardSpeed = PREDATOR_MOVESCALE;
				strafeSpeed = PREDATOR_MOVESCALE;
				turnSpeed =	TURNSCALE;
				jumpSpeed = JUMPVELOCITY;
				break;
			case I_Marine:
				forwardSpeed = MARINE_MOVESCALE;
				strafeSpeed = MARINE_MOVESCALE;
				turnSpeed =	TURNSCALE;
				jumpSpeed = JUMPVELOCITY;
				break;
		}
		#endif

		MaxSpeed=forwardSpeed;

		if((playerStatusPtr->Mvt_InputRequests.Flags.Rqst_Strafe)&&(playerStatusPtr->Mvt_SideStepIncrement==0))
		{
			strafeSpeed	= MUL_FIXED(strafeSpeed,playerStatusPtr->Mvt_TurnIncrement);
		}
		else
		{
			strafeSpeed	= MUL_FIXED(strafeSpeed,playerStatusPtr->Mvt_SideStepIncrement);
		}
		forwardSpeed = MUL_FIXED(forwardSpeed,playerStatusPtr->Mvt_MotionIncrement);
		turnSpeed    = MUL_FIXED(turnSpeed,playerStatusPtr->Mvt_TurnIncrement);
		
		if (MIRROR_CHEATMODE)
		{
			turnSpeed = -turnSpeed;
			strafeSpeed = -strafeSpeed;
		}
		
		{
			extern int CameraZoomLevel;
			if(CameraZoomLevel)
			{
				turnSpeed >>= CameraZoomLevel;
				playerStatusPtr->Mvt_PitchIncrement >>= CameraZoomLevel;
			}
		}
		
		if( ((AvP.PlayerType == I_Alien) || (playerStatusPtr->ShapeState == PMph_Standing))
			&& (playerStatusPtr->Mvt_InputRequests.Flags.Rqst_Faster) && (playerStatusPtr->Encumberance.CanRun) )
		{	
			/* Test - half backward speed for predators */
			if (AvP.PlayerType==I_Predator) {
				if (playerStatusPtr->Mvt_MotionIncrement<0) {
					forwardSpeed = (forwardSpeed)/2;
				}
			}
		}
		else 
		{
			/* walk = half speed */
			strafeSpeed = (strafeSpeed)/2;
			forwardSpeed = (forwardSpeed)/2;
			turnSpeed = (turnSpeed)/2;
		}	
		
		/* Marker */

		strafeSpeed=MUL_FIXED(strafeSpeed,playerStatusPtr->Encumberance.MovementMultiple);
		forwardSpeed=MUL_FIXED(forwardSpeed,playerStatusPtr->Encumberance.MovementMultiple);
		turnSpeed=MUL_FIXED(turnSpeed,playerStatusPtr->Encumberance.TurningMultiple);
		jumpSpeed=MUL_FIXED(jumpSpeed,playerStatusPtr->Encumberance.JumpingMultiple);
		
		/* KJL 17:45:03 9/9/97 - inertia means it's difficult to stop */			
	  	if (forwardSpeed*playerStatusPtr->ForwardInertia<0) playerStatusPtr->ForwardInertia = 0;
	  	if (strafeSpeed*playerStatusPtr->StrafeInertia<0) playerStatusPtr->StrafeInertia = 0;
	  	
	  	if (!forwardSpeed)
		{
			int deltaForward = (FASTMOVESCALE*NormalFrameTime)>>14;
			if (playerStatusPtr->ForwardInertia>0)
			{
				forwardSpeed = playerStatusPtr->ForwardInertia - deltaForward;
				if (forwardSpeed<0) forwardSpeed=0;
			}
			else if (playerStatusPtr->ForwardInertia<0)
			{
				forwardSpeed = playerStatusPtr->ForwardInertia + deltaForward;
				if (forwardSpeed>0) forwardSpeed=0;
			}
		}
		else
		{
			int deltaForward = MUL_FIXED(forwardSpeed*4,NormalFrameTime);
			{
				int a = playerStatusPtr->ForwardInertia + deltaForward;
				if (forwardSpeed>0)
				{
					if (a<forwardSpeed) forwardSpeed = a;
				}
				else
				{
					if (a>forwardSpeed) forwardSpeed = a;
				}
			}
		}

		if (!strafeSpeed)
		{
			int deltaStrafe = (FASTSTRAFESCALE*NormalFrameTime)>>14;
			if (playerStatusPtr->StrafeInertia>0)
			{
				strafeSpeed = playerStatusPtr->StrafeInertia - deltaStrafe;
				if (strafeSpeed<0) strafeSpeed=0;
			}
			else if (playerStatusPtr->StrafeInertia<0)
			{
				strafeSpeed = playerStatusPtr->StrafeInertia + deltaStrafe;
				if (strafeSpeed>0) strafeSpeed=0;
			}
		}
		else
		{
			int deltaForward = MUL_FIXED(strafeSpeed*4,NormalFrameTime);
			{
				int a = playerStatusPtr->StrafeInertia + deltaForward;
				if (strafeSpeed>0)
				{
					if (a<strafeSpeed) strafeSpeed = a;
				}
				else
				{
					if (a>strafeSpeed) strafeSpeed = a;
				}
			}
		}

		/* inertia on turning - currently off */
		#if 0
		if(!turnSpeed)
		{
			int deltaTurn = (FASTTURNSCALE*NormalFrameTime)>>15;
			if (playerStatusPtr->TurnInertia>0)
			{
				turnSpeed = playerStatusPtr->TurnInertia - deltaTurn;
				if (turnSpeed<0) turnSpeed=0;
			}
			else if (playerStatusPtr->TurnInertia<0)
			{
				turnSpeed = playerStatusPtr->TurnInertia + deltaTurn;
				if (turnSpeed>0) turnSpeed=0;
			}
		}
		#endif

		/* Hold it! Correct forwardSpeed vs. strafeSpeed? */

		#if 0
		{
			int mag,angle;

			mag=(forwardSpeed*forwardSpeed)+(strafeSpeed*strafeSpeed);
			if (mag>(MaxSpeed*MaxSpeed)) {

				angle=ArcTan(forwardSpeed,strafeSpeed);

				forwardSpeed=MUL_FIXED(GetSin(angle),MaxSpeed);
				strafeSpeed=MUL_FIXED(GetCos(angle),MaxSpeed);
			
			}
		}
		#endif
		
		if (playerStatusPtr->Mvt_InputRequests.Flags.Rqst_Jetpack &&
			playerStatusPtr->JetpackEnabled)
		{
			if (dynPtr->LinImpulse.vy>-JETPACK_MAX_SPEED)
			{
				dynPtr->LinImpulse.vy-=MUL_FIXED(JETPACK_THRUST,NormalFrameTime);
			}
			AddLightingEffectToObject(Player,LFX_OBJECTONFIRE);
			/* Sound handling. */
			if (playerStatusPtr->soundHandle5==SOUND_NOACTIVEINDEX) {
				Sound_Play(SID_ED_JETPACK_START,"h");
				Sound_Play(SID_ED_JETPACK_MID,"el",&playerStatusPtr->soundHandle5);
			}

		} else {
			/* Sound handling. */
			if (playerStatusPtr->soundHandle5!=SOUND_NOACTIVEINDEX) {
				Sound_Play(SID_ED_JETPACK_END,"h");
				Sound_Stop(playerStatusPtr->soundHandle5);
			}
		}

		#if FLY_MODE_CHEAT_ON
		dynPtr->GravityOn=1;
		if (KeyboardInput[KEY_F6]&&(!(playerStatusPtr->DemoMode)))
		{
			if(FlyModeDebounced)
			{
				FlyModeOn = !FlyModeOn;			
				FlyModeDebounced = 0;
			}
		}
		else FlyModeDebounced = 1;

		if(FlyModeOn)
		{
			dynPtr->LinVelocity.vx = 0;
			dynPtr->LinVelocity.vy = 0;
			dynPtr->LinVelocity.vz = forwardSpeed;
//			dynPtr->IsNetGhost=1;
			if(playerStatusPtr->Mvt_InputRequests.Flags.Rqst_Strafe)
			{
				dynPtr->LinVelocity.vx = strafeSpeed;
			}
			else if((playerStatusPtr->Mvt_InputRequests.Flags.Rqst_SideStepLeft)
				|| (playerStatusPtr->Mvt_InputRequests.Flags.Rqst_SideStepRight))
			{
				dynPtr->LinVelocity.vx = strafeSpeed;
			}

		   	/* rotate LinVelocity along camera view */
			{
				MATRIXCH mat = Global_VDB_Ptr->VDB_Mat;
				TransposeMatrixCH(&mat);
				RotateVector(&dynPtr->LinVelocity,&mat);
			}
			dynPtr->GravityOn=0;
			dynPtr->LinImpulse.vx=0;
			dynPtr->LinImpulse.vy=0;
			dynPtr->LinImpulse.vz=0;
		}
		else
		#endif
		/* KJL 12:28:48 14/04/98 - if we're not in contact with the floor, but we've hit
		something, set our velocity to zero (otherwise leave it alone) */
		if(!dynPtr->IsInContactWithFloor)
		{
			if (playerStatusPtr->Mvt_InputRequests.Flags.Rqst_Jetpack &&
				playerStatusPtr->JetpackEnabled)
			{
				dynPtr->LinVelocity.vx = 0;
				dynPtr->LinVelocity.vy = 0;
				if (forwardSpeed>0)
				{
					dynPtr->LinVelocity.vz = forwardSpeed/2;
				}
				else
				{
					dynPtr->LinVelocity.vz = forwardSpeed/4;
				}
	//			dynPtr->IsNetGhost=1;
				if(playerStatusPtr->Mvt_InputRequests.Flags.Rqst_Strafe)
				{
					dynPtr->LinVelocity.vx = strafeSpeed/4;
				}
				else if((playerStatusPtr->Mvt_InputRequests.Flags.Rqst_SideStepLeft)
					|| (playerStatusPtr->Mvt_InputRequests.Flags.Rqst_SideStepRight))
				{
					dynPtr->LinVelocity.vx = strafeSpeed/4;
				}

				/* rotate LinVelocity into world space */
				RotateVector(&dynPtr->LinVelocity,&dynPtr->OrientMat);
			}
			else if (dynPtr->CollisionReportPtr)
			{
	  			dynPtr->LinVelocity.vx = 0;
	  			dynPtr->LinVelocity.vy = 0;
	  			dynPtr->LinVelocity.vz = forwardSpeed/8;
				/* rotate LinVelocity into world space */
				RotateVector(&dynPtr->LinVelocity,&dynPtr->OrientMat);
				
			}	
		}
		/* this bit sets the velocity: don't do it in demo mode, though
		as we set our own velocity... */
		else if((dynPtr->IsInContactWithFloor)&&(!(playerStatusPtr->DemoMode)))
		{
			dynPtr->LinVelocity.vx = 0;
			dynPtr->LinVelocity.vy = 0;
			dynPtr->LinVelocity.vz = forwardSpeed;
		
			if(playerStatusPtr->Mvt_InputRequests.Flags.Rqst_Strafe)
			{
				dynPtr->LinVelocity.vx = strafeSpeed;
			}
			else if((playerStatusPtr->Mvt_InputRequests.Flags.Rqst_SideStepLeft)
				|| (playerStatusPtr->Mvt_InputRequests.Flags.Rqst_SideStepRight))
			{
				dynPtr->LinVelocity.vx = strafeSpeed;
			}

			if(playerStatusPtr->Mvt_InputRequests.Flags.Rqst_Jump)
			{
				COLLISIONREPORT *reportPtr = Player->ObStrategyBlock->DynPtr->CollisionReportPtr;
				int notTooSteep = 0;
				
				while (reportPtr) /* while there is a valid report */
				{
					int dot = DotProduct(&(reportPtr->ObstacleNormal),&(dynPtr->GravityDirection));

					if (dot<-60000) 
					{
						notTooSteep = 1;
						break;
					}
					/* skip to next report */
					reportPtr = reportPtr->NextCollisionReportPtr;
				}
						
				if (notTooSteep)
				{
					/* alien can jump in the direction it's looking */									
					if (AvP.PlayerType == I_Alien)
					{
						VECTORCH viewDir;

						viewDir.vx = Global_VDB_Ptr->VDB_Mat.mat13;
						viewDir.vy = Global_VDB_Ptr->VDB_Mat.mat23;
						viewDir.vz = Global_VDB_Ptr->VDB_Mat.mat33;
						if ((playerStatusPtr->ShapeState == PMph_Crouching) && (DotProduct(&viewDir,&dynPtr->GravityDirection)<-32768))
						{
							dynPtr->LinImpulse.vx += MUL_FIXED(viewDir.vx,jumpSpeed*3);
							dynPtr->LinImpulse.vy += MUL_FIXED(viewDir.vy,jumpSpeed*3);
							dynPtr->LinImpulse.vz += MUL_FIXED(viewDir.vz,jumpSpeed*3);
						}
						else
						{
							dynPtr->LinImpulse.vx -= MUL_FIXED(dynPtr->GravityDirection.vx,jumpSpeed);
							dynPtr->LinImpulse.vy -= MUL_FIXED(dynPtr->GravityDirection.vy,jumpSpeed);
							dynPtr->LinImpulse.vz -= MUL_FIXED(dynPtr->GravityDirection.vz,jumpSpeed);
						  	dynPtr->LinVelocity.vz += jumpSpeed;	
						}
						dynPtr->TimeNotInContactWithFloor = -1;
					}
					else
					{
						dynPtr->LinImpulse.vx -= MUL_FIXED(dynPtr->GravityDirection.vx,jumpSpeed);
						dynPtr->LinImpulse.vy -= MUL_FIXED(dynPtr->GravityDirection.vy,jumpSpeed);
						dynPtr->LinImpulse.vz -= MUL_FIXED(dynPtr->GravityDirection.vz,jumpSpeed);
						dynPtr->TimeNotInContactWithFloor = 0;
					}

					switch(AvP.PlayerType)
					{
						case I_Marine:
						{
							#if 0
							if (playerStatusPtr->soundHandle==SOUND_NOACTIVEINDEX) {
								int rand=(FastRandom()%4);

								switch (rand) {
									case 0:
										Sound_Play(SID_MARINE_JUMP_START,"he",&playerStatusPtr->soundHandle);
										break;
									case 1:
										Sound_Play(SID_MARINE_JUMP_START_2,"he",&playerStatusPtr->soundHandle);
										break;
									case 2:
										Sound_Play(SID_MARINE_JUMP_START_3,"he",&playerStatusPtr->soundHandle);
										break;
									default:
										Sound_Play(SID_MARINE_JUMP_START_4,"he",&playerStatusPtr->soundHandle);
										break;
								}
							}
							#else
							if (playerStatusPtr->soundHandle==SOUND_NOACTIVEINDEX) {
								PlayMarineScream(0,SC_Jump,0,&playerStatusPtr->soundHandle,NULL);
								if(AvP.Network!=I_No_Network) netGameData.myLastScream=SC_Jump;
							}
							#endif
							break;
						}
						case I_Alien:
							break;
						case I_Predator:
						{
							#if 0
							if (playerStatusPtr->soundHandle==SOUND_NOACTIVEINDEX) {
								int rand=(FastRandom()%3);

								switch (rand) {
									case 0:
										Sound_Play(SID_PRED_JUMP_START_1,"he",&playerStatusPtr->soundHandle);
										break;
									case 1:
										Sound_Play(SID_PRED_JUMP_START_2,"he",&playerStatusPtr->soundHandle);
										break;
									default:
										Sound_Play(SID_PRED_JUMP_START_3,"he",&playerStatusPtr->soundHandle);
										break;
								}
							}
							#else
							if (playerStatusPtr->soundHandle==SOUND_NOACTIVEINDEX) {
								PlayPredatorSound(0,PSC_Jump,0,&playerStatusPtr->soundHandle,NULL);
								if(AvP.Network!=I_No_Network) netGameData.myLastScream=PSC_Jump;
							}
							#endif
							break;
						}
						default:
							break;

					}
				}
			}
			/* rotate LinVelocity into world space */
			RotateVector(&dynPtr->LinVelocity,&dynPtr->OrientMat);
		}

		/* zero angular velocity */
		dynPtr->AngVelocity.EulerX = 0;
		dynPtr->AngVelocity.EulerZ = 0;
		
		if(playerStatusPtr->Mvt_InputRequests.Flags.Rqst_Strafe)
		{
			dynPtr->AngVelocity.EulerY = 0;
		}
		else
		{
		 	dynPtr->AngVelocity.EulerY = turnSpeed;                       
		}
		
		playerStatusPtr->ForwardInertia = forwardSpeed;
		playerStatusPtr->StrafeInertia = strafeSpeed; 
		playerStatusPtr->TurnInertia = turnSpeed; 	
	}
	/*KJL****************************************************************************************
	* The player's AngVelocity as set by the above code is only valid in the player's object    *
	* space, and so has to be rotated into world space. So aliens can walk on the ceiling, etc. *
	****************************************************************************************KJL*/
	if (dynPtr->AngVelocity.EulerY)
	{
		MATRIXCH mat;
   	
   		int angle = MUL_FIXED(NormalFrameTime,dynPtr->AngVelocity.EulerY)&4095;
 	  	int cos = GetCos(angle);
 	  	int sin = GetSin(angle);
 	  	mat.mat11 = cos;		 
 	  	mat.mat12 = 0;
 	  	mat.mat13 = -sin;
 	  	mat.mat21 = 0;	  	
 	  	mat.mat22 = 65536;	  	
 	  	mat.mat23 = 0;	  	
 	  	mat.mat31 = sin;	  	
 	  	mat.mat32 = 0;	  	
 	  	mat.mat33 = cos;	  	

		MatrixMultiply(&dynPtr->OrientMat,&mat,&dynPtr->OrientMat);
	 	MatrixToEuler(&dynPtr->OrientMat, &dynPtr->OrientEuler);

	}
	/*------------------------------------------------------ 
	CROUCHING, LYING DOWN, ETC.
	------------------------------------------------------*/ 
	MaintainPlayerShape(sbPtr);
	
	/* Alien's wall-crawling abilities */
	if (AvP.PlayerType == I_Alien)
	{
		/* let alien walk on walls & ceiling */
		if ( (playerStatusPtr->ShapeState == PMph_Crouching)
		   &&(!dynPtr->RequestsToStandUp) )
		{
			dynPtr->UseStandardGravity=0;
		}
		else
		{
			dynPtr->UseStandardGravity=1;
		}
	}


	

    /*------------------------------------------------------ 
	WEAPON FIRING
	Kevin: The player input functions now interface directly
	with the weapons state machine.	I hope.
	------------------------------------------------------*/

	/*------------------------------------------------------ 
	CAMERA Controls
	------------------------------------------------------*/ 
	
	/* If AbsolutePitch is set, view angle comes direct from Mvt_PitchIncrement,
	   which takes values -65536 to +65536. */
	
	
	if (playerStatusPtr->Absolute_Pitching)
	{
		playerStatusPtr->ViewPanX = MUL_FIXED(playerStatusPtr->Mvt_PitchIncrement,1024-128);
		playerStatusPtr->ViewPanX &= wrap360;
	}
	else
	{
		static int timeBeenContinuouslyMoving=0;
		int AllowedLookDownAngle;
		int AllowedLookUpAngle;

		if (AvP.PlayerType==I_Alien)
		{
			AllowedLookUpAngle = 0;
			AllowedLookDownAngle = 2048;
		}
		else
		{
			AllowedLookUpAngle = 128;
			AllowedLookDownAngle = 2048-128;
		}

		if (!ControlMethods.AutoCentreOnMovement)
		{
			timeBeenContinuouslyMoving = 0;
		}

		if (playerStatusPtr->Mvt_MotionIncrement == 0)
		{
			timeBeenContinuouslyMoving=0;
		}
		else
		{
			if (timeBeenContinuouslyMoving>TIMEBEFOREAUTOCENTREVIEW
			&& !playerStatusPtr->Mvt_InputRequests.Flags.Rqst_LookUp
			&& !playerStatusPtr->Mvt_InputRequests.Flags.Rqst_LookDown)
			{
				playerStatusPtr->Mvt_InputRequests.Flags.Rqst_CentreView =1;
			}
			else
			{
				timeBeenContinuouslyMoving+=NormalFrameTime;	
			}
		}
		
		if(playerStatusPtr->Mvt_InputRequests.Flags.Rqst_LookUp)
		{
        	playerStatusPtr->ViewPanX += 1024;
			playerStatusPtr->ViewPanX &= wrap360;
                               
			playerStatusPtr->ViewPanX += MUL_FIXED
									(
										playerStatusPtr->Mvt_PitchIncrement,
										NormalFrameTime>>PANRATESHIFT
									);

			if (playerStatusPtr->ViewPanX < AllowedLookUpAngle) playerStatusPtr->ViewPanX=AllowedLookUpAngle; 

        	playerStatusPtr->ViewPanX -= 1024;
			playerStatusPtr->ViewPanX &= wrap360;
		}
		else if(playerStatusPtr->Mvt_InputRequests.Flags.Rqst_LookDown)
		{
        	playerStatusPtr->ViewPanX += 1024;
			playerStatusPtr->ViewPanX &= wrap360;
                               
			playerStatusPtr->ViewPanX += MUL_FIXED
									(
										playerStatusPtr->Mvt_PitchIncrement,
										NormalFrameTime>>PANRATESHIFT
									);

			if (playerStatusPtr->ViewPanX > AllowedLookDownAngle) playerStatusPtr->ViewPanX=AllowedLookDownAngle; 

        	playerStatusPtr->ViewPanX -= 1024;
			playerStatusPtr->ViewPanX &= wrap360;
		} 
		if(playerStatusPtr->Mvt_InputRequests.Flags.Rqst_CentreView)
		{
        	playerStatusPtr->ViewPanX += 1024;
			playerStatusPtr->ViewPanX &= wrap360;
            
            if (playerStatusPtr->ViewPanX > 1024)
            {                  
				playerStatusPtr->ViewPanX -= (NormalFrameTime>>PANRATESHIFT)*2;
				if (playerStatusPtr->ViewPanX < 1024) playerStatusPtr->ViewPanX=1024; 
			}
            else if (playerStatusPtr->ViewPanX < 1024)
            {                  
				playerStatusPtr->ViewPanX += (NormalFrameTime>>PANRATESHIFT)*2;
				if (playerStatusPtr->ViewPanX > 1024) playerStatusPtr->ViewPanX=1024; 
			}

        	playerStatusPtr->ViewPanX -= 1024;
			playerStatusPtr->ViewPanX &= wrap360;
		}
	}

	HandleGrapplingHookForces();
}


/*------------------------------------------------------ 
Crouch and Lie down support fns.
------------------------------------------------------*/ 

static void MaintainPlayerShape(STRATEGYBLOCK* sbPtr)
{
	PLAYER_STATUS *playerStatusPtr= (PLAYER_STATUS *) (sbPtr->SBdataptr);

	/* maintain play morphing state */
	switch (playerStatusPtr->ShapeState)
	{
		case(PMph_Standing):
		{
			/* if we're standing, check inputs for a request to 
			   crouch or lie down */
			if (playerStatusPtr->Encumberance.CanCrouch)
			{
				if(playerStatusPtr->Mvt_InputRequests.Flags.Rqst_Crouch) 
				{
					if (CrouchKeyDebounced)
					{
						MakePlayerCrouch(sbPtr);
						CrouchKeyDebounced = 0;
					}
				}
				else
				{
					CrouchKeyDebounced = 1;
				}
			
			}


			sbPtr->DynPtr->RequestsToStandUp=0;
					   
			break;
		}
		case(PMph_Crouching):
		{
			/* if we're crouching, then check inputs for crouch request.
			   if there isn't one, stand up again */
			if(sbPtr->DynPtr->RequestsToStandUp)
			{
				//currently crouching , but have had a request to stand up.
				//cancel request if the crouch key is pressed again
				if (playerStatusPtr->Encumberance.CanCrouch)
				{
					if(playerStatusPtr->Mvt_InputRequests.Flags.Rqst_Crouch) 
					{
						if (CrouchKeyDebounced)
						{
							sbPtr->DynPtr->RequestsToStandUp = 0;
							CrouchKeyDebounced = 0;
						}
					}
					else
					{
						CrouchKeyDebounced = 1;
					}
			
				}
			}
			else
			{
				if (!(playerStatusPtr->Encumberance.CanCrouch)) 
				{
					sbPtr->DynPtr->RequestsToStandUp=1;
				}
			
				if (CrouchIsToggleKey)
				{
					if(playerStatusPtr->Mvt_InputRequests.Flags.Rqst_Crouch)
					{
						if (CrouchKeyDebounced)
						{
							sbPtr->DynPtr->RequestsToStandUp=1;
							CrouchKeyDebounced = 0;
						}
					}
					else
					{
						CrouchKeyDebounced = 1;
					}
				}
				else if(!(playerStatusPtr->Mvt_InputRequests.Flags.Rqst_Crouch))
				{
					sbPtr->DynPtr->RequestsToStandUp=1;
				}
			}
			break;
		}
		case(PMph_Lying):
		{
			/* if we're lying, then check inputs for lie request.
			if there isn't one, stand up again */
			break;
		}
		default:
		{
			/* should never get here */
			GLOBALASSERT(1==0);
		}
	
	}

}

static void MakePlayerCrouch(STRATEGYBLOCK* sbPtr)
{	
	PLAYER_STATUS *playerStatusPtr= (PLAYER_STATUS *) (sbPtr->SBdataptr);

	/* set player state */
	playerStatusPtr->ShapeState = PMph_Crouching;

	return;
}

#if 0
static void MakePlayerLieDown(STRATEGYBLOCK* sbPtr)
{	
	PLAYER_STATUS *playerStatusPtr= (PLAYER_STATUS *) (sbPtr->SBdataptr);
	

	/* set player state */
	playerStatusPtr->ShapeState = PMph_Lying;

	return;
}
#endif


int deathFadeLevel;

static void CorpseMovement(STRATEGYBLOCK *sbPtr)
{
	extern int RealFrameTime;

	/* only fade non-net game */
	if(AvP.Network == I_No_Network)
	{
		if(deathFadeLevel>0)
		{
			/* fade screen to black */
			//SetPaletteFadeLevel(deathFadeLevel);
			deathFadeLevel-= RealFrameTime/4;
			if (deathFadeLevel<0) deathFadeLevel = 0;

		}
		else
		{
			deathFadeLevel = 0;
			/* KJL 15:44:10 03/11/97 - game over, quit main loop */
			/* restart level instead -Richard*/
		  	if (DebouncedGotAnyKey)
			{
			  	AvP.RestartLevel = 1;
			}
		}
	}
	else
	{
		if(deathFadeLevel>0)
		{
			deathFadeLevel-= RealFrameTime/2;	
		}
		else
		{
			deathFadeLevel = 0;
			NetPlayerDeadProcessing(sbPtr);
		}
	}
}

/*-------------------Patrick 14/4/97--------------------
  This function does necessary processing for a dead
  network player...
  ------------------------------------------------------*/
static void NetPlayerDeadProcessing(STRATEGYBLOCK *sbPtr)
{
	PLAYER_STATUS *psPtr= (PLAYER_STATUS *) (sbPtr->SBdataptr);

	/* call the read input function so that we can still respawn/quit, etc */
	ReadPlayerGameInput(sbPtr);

	/* check for re-spawn */
	if(psPtr->Mvt_InputRequests.Flags.Rqst_Operate)
	{
		if(AreThereAnyLivesLeft())
		{
			//check for change of character
			if(netGameData.myCharacterType!=netGameData.myNextCharacterType)
			{
				switch(netGameData.myNextCharacterType)
				{
					case (NGCT_Marine) :
						ChangeToMarine();
						break;

					case (NGCT_Alien) :
						ChangeToAlien();
						break;

					case (NGCT_Predator) :
						ChangeToPredator();
						break;

					default :
						GLOBALASSERT("dodgy character type"==0);
						break;
						
				}

				netGameData.myCharacterType=netGameData.myNextCharacterType;
			}
			else
			{
				/* CDF 15/3/99, delete all discs... */
				RemoveAllThisPlayersDiscs();

				NetPlayerRespawn(sbPtr);
			}

			/* dynamics block stuff... */
			{
				EULER zeroEuler = {0,0,0};
				VECTORCH zeroVec = {0,0,0};
				DYNAMICSBLOCK *dynPtr = sbPtr->DynPtr;

				dynPtr->Position = zeroVec;
				dynPtr->OrientEuler = zeroEuler;
				dynPtr->LinVelocity = zeroVec;
				dynPtr->LinImpulse = zeroVec;

				CreateEulerMatrix(&dynPtr->OrientEuler, &dynPtr->OrientMat);
				TransposeMatrixCH(&dynPtr->OrientMat);

				//Need to get rid of collisions for this frame , so player doesn't pick up
				//his dropped weapon when he respawns.
				dynPtr->CollisionReportPtr=0;
			}
			TeleportNetPlayerToAStartingPosition(sbPtr,0);
		}
		else
		{
			//no lives left , so have to act as an observer
			GetNextMultiplayerObservedPlayer();

			//The player's dropped weapon (if there was one) can now be drawn
			MakePlayersWeaponPickupVisible();
			
		}
	}
}

extern void InitPlayerCloakingSystem(void);
//make the player into new healthy character
void NetPlayerRespawn(STRATEGYBLOCK *sbPtr)
{
	extern int LeanScale;
#if 0
	SECTION *root_section;
#endif

	PLAYER_STATUS *psPtr= (PLAYER_STATUS *) (sbPtr->SBdataptr);


	/* Turn on corpse. */
	if (psPtr->MyCorpse) {
		if (psPtr->MyCorpse->SBdptr) {
			psPtr->MyCorpse->SBdptr->ObFlags&=~ObFlag_NotVis;
		}
	}
	psPtr->MyCorpse=NULL;
	DeInitialisePlayer();
	/* When you're going to respawn... you might change */
	/* character class, after all. */
	InitialisePlayersInventory(psPtr);
    /* psPtr->Health=STARTOFGAME_MARINE_HEALTH; */
    /* psPtr->Armour=STARTOFGAME_MARINE_ARMOUR; */
	psPtr->IsAlive = 1;
	psPtr->MyFaceHugger=NULL;
    psPtr->Energy=STARTOFGAME_MARINE_ENERGY;
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
				LeanScale=ONE_FIXED;

				#if 0  //this hmodel isn't being set up for the moment - Richard
				root_section=GetNamedHierarchyFromLibrary("hnpcmarine","Template");
				if (!root_section) {
					GLOBALASSERT(0);
					/* Sorry, there's just no bouncing back from this one.  Fix it. */
					return;
				}
				Create_HModel(&psPtr->HModelController,root_section);
				InitHModelSequence(&psPtr->HModelController,0,0,ONE_FIXED);
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
				LeanScale=ONE_FIXED;

				#if 0  //this hmodel isn't being set up for the moment - Richard
				root_section=GetNamedHierarchyFromLibrary("hnpcpredator","Template");
				if (!root_section) {
					GLOBALASSERT(0);
					/* Sorry, there's just no bouncing back from this one.  Fix it. */
					return;
				}
				Create_HModel(&psPtr->HModelController,root_section);
				InitHModelSequence(&psPtr->HModelController,0,0,ONE_FIXED);
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
				LeanScale=ONE_FIXED*3;

				#if 0  //this hmodel isn't being set up for the moment - Richard
				root_section=GetNamedHierarchyFromLibrary("hnpcalien","alien");
				if (!root_section) {
					GLOBALASSERT(0);
					/* Sorry, there's just no bouncing back from this one.  Fix it. */
					return;
				}
				Create_HModel(&psPtr->HModelController,root_section);
				InitHModelSequence(&psPtr->HModelController,0,0,ONE_FIXED);
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

		NpcData = GetThisNpcData(PlayerType);
		LOCALASSERT(NpcData);
		sbPtr->SBDamageBlock.Health=NpcData->StartingStats.Health<<ONE_FIXED_SHIFT;
		sbPtr->SBDamageBlock.Armour=NpcData->StartingStats.Armour<<ONE_FIXED_SHIFT;			
		sbPtr->SBDamageBlock.SB_H_flags=NpcData->StartingStats.SB_H_flags;
		sbPtr->SBDamageBlock.IsOnFire=0;
	}
	
	psPtr->Encumberance.MovementMultiple=ONE_FIXED;
	psPtr->Encumberance.TurningMultiple=ONE_FIXED;
	psPtr->Encumberance.JumpingMultiple=ONE_FIXED;
	psPtr->Encumberance.CanCrouch=1;
	psPtr->Encumberance.CanRun=1;
	psPtr->Health=sbPtr->SBDamageBlock.Health;
	psPtr->Armour=sbPtr->SBDamageBlock.Armour;

	psPtr->ForwardInertia=0;
	psPtr->StrafeInertia=0; 
	psPtr->TurnInertia=0; 	
	psPtr->IsMovingInWater = 0;

	psPtr->incidentFlag=0;
	psPtr->incidentTimer=0;

	if (psPtr->soundHandle!=SOUND_NOACTIVEINDEX) {
 		Sound_Stop(psPtr->soundHandle);
	}
	if (psPtr->soundHandle3!=SOUND_NOACTIVEINDEX) {
 		Sound_Stop(psPtr->soundHandle3);
	}
	
	if (weaponHandle!=SOUND_NOACTIVEINDEX) {
 		Sound_Stop(weaponHandle);
	}

	if (predHUDSoundHandle!=SOUND_NOACTIVEINDEX) {
		Sound_Stop(predHUDSoundHandle);
	}

	if (predOVision_SoundHandle!=SOUND_NOACTIVEINDEX) {
		Sound_Stop(predOVision_SoundHandle);
	}

	//reset the player's elasticity (which gets altered upon death)
	sbPtr->DynPtr->Elasticity = 0;
	

	InitPlayerCloakingSystem();
		
	SetupVision();

    PlayerDamagedOverlayIntensity = 0;

	//no longer acting as an observer
	TurnOffMultiplayerObserveMode();
	
	//The player's dropped weapon (if there was one) can now be drawn
	MakePlayersWeaponPickupVisible();
}


/* Patrick 9/7/97 ---------------------------------------------------
These two functions are used for testing the pre-processed AI 
locations... (either entry points or auxilary locs)
They teleport the player to the next location in the sequence, 
in response to the player pressing 'unused3' (currently the U key).
--------------------------------------------------------------------*/
#if PATTEST_EPS
static int pF_ModuleIndex = 0;
static int pF_EpIndex = 0;
static int pF_HaveStarted = 0;
static int pF_CanMove = 0;

void EpLocationTest(void)
{
	extern SCENE Global_Scene;
	extern SCENEMODULE **Global_ModulePtr;
	extern int ModuleArraySize;

	SCENEMODULE *ScenePtr;
	MODULE **moduleListPointer;
	DYNAMICSBLOCK *dynPtr = Player->ObStrategyBlock->DynPtr;
	MODULE *thisModulePtr;

	LOCALASSERT(Global_ModulePtr);
	ScenePtr = Global_ModulePtr[Global_Scene];
	moduleListPointer = ScenePtr->sm_marray;		

	if(PlayerStatusPtr->Mvt_InputRequests.Flags.Rqst_Unused3)
	{			
		if(pF_CanMove == 1)
		{
			/* move to the next one */
			pF_EpIndex++;
			if(pF_EpIndex >= FALLP_EntryPoints[pF_ModuleIndex].numEntryPoints)
			{
				pF_EpIndex=0;
				do
				{
					pF_ModuleIndex++;
					if(pF_ModuleIndex>=ModuleArraySize) pF_ModuleIndex = 0;
				}
				while(FALLP_EntryPoints[pF_ModuleIndex].numEntryPoints==0);
			}

			/* now move to the new location */
			thisModulePtr = moduleListPointer[pF_ModuleIndex];
			dynPtr->Position = FALLP_EntryPoints[pF_ModuleIndex].entryPointsList[(pF_EpIndex)].position;
			dynPtr->Position.vx += thisModulePtr->m_world.vx;
			dynPtr->Position.vy += thisModulePtr->m_world.vy;
			dynPtr->Position.vz += thisModulePtr->m_world.vz;

			dynPtr->PrevPosition = dynPtr->Position;	
			
			pF_HaveStarted = 1;
			pF_CanMove = 0;
		}			
	}
	else pF_CanMove = 1;
					
	if (pF_HaveStarted)
	{
		textprint("CURRENT FAR MODULE %d \n", pF_ModuleIndex);
		textprint("EP number %d from module %d \n", pF_EpIndex, FALLP_EntryPoints[pF_ModuleIndex].entryPointsList[(pF_EpIndex)].donorIndex);
	}	
}

#endif
#if PATTEST_AUXLOCS
static int pF_ModuleIndex = 0;
static int pF_AuxIndex = 0;
static int pF_HaveStarted = 0;
static int pF_CanMove = 0;

void AuxLocationTest(void)
{
	extern SCENE Global_Scene;
	extern SCENEMODULE **Global_ModulePtr;
	extern int ModuleArraySize;

	SCENEMODULE *ScenePtr;
	MODULE **moduleListPointer;
	DYNAMICSBLOCK *dynPtr=Player->ObStrategyBlock->DynPtr;
	MODULE *thisModulePtr;

	LOCALASSERT(Global_ModulePtr);
	ScenePtr = Global_ModulePtr[Global_Scene];
	moduleListPointer = ScenePtr->sm_marray;		

	/* dynPtr->GravityOn = 0; */

	if(PlayerStatusPtr->Mvt_InputRequests.Flags.Rqst_Unused3)
	{			
		if(pF_CanMove == 1)
		{
			/* move to the next one */
			pF_AuxIndex++;
			if(pF_AuxIndex >= FALLP_AuxLocs[pF_ModuleIndex].numLocations)
			{
				pF_AuxIndex=0;
				do
				{
					pF_ModuleIndex++;
					if(pF_ModuleIndex>=ModuleArraySize) pF_ModuleIndex = 0;
				}
				while(FALLP_AuxLocs[pF_ModuleIndex].numLocations==0);
			}

			/* now move to the new location */
			thisModulePtr = moduleListPointer[pF_ModuleIndex];
			dynPtr->Position = FALLP_AuxLocs[pF_ModuleIndex].locationsList[pF_AuxIndex];
			dynPtr->Position.vx += thisModulePtr->m_world.vx;
			dynPtr->Position.vy += thisModulePtr->m_world.vy;
			dynPtr->Position.vz += thisModulePtr->m_world.vz;
			dynPtr->Position.vy -= 1000;

			dynPtr->PrevPosition = dynPtr->Position;				
			pF_HaveStarted = 1;
			pF_CanMove = 0;
		}			
	}
	else pF_CanMove = 1;
					
	if (pF_HaveStarted)
	{
		textprint("CURRENT FAR MODULE %d \n", pF_ModuleIndex);
		textprint("AUX number %d \n", pF_AuxIndex);
	}	
}

#endif





/* KJL 10:34:54 8/5/97 - The alien can damage things by merely touching them 

   This will need work to get the values right - the damage done could be
   scaled by the alien's experience points, the relative velocities of the
   objects, and so on.
*/
#define ALIEN_CONTACT_WEAPON_DAMAGE 50
#define ALIEN_CONTACT_WEAPON_DELAY 65536

#if ALIEN_CONTACT_WEAPON
static void AlienContactWeapon(void)
{
	COLLISIONREPORT *reportPtr = Player->ObStrategyBlock->DynPtr->CollisionReportPtr;
	static int contactWeaponTimer = 0;

	if (contactWeaponTimer<=0)
	{
		contactWeaponTimer = ALIEN_CONTACT_WEAPON_DELAY;

		while (reportPtr) /* while there is a valid report */
		{
			if (reportPtr->ObstacleSBPtr)
			{
				switch(reportPtr->ObstacleSBPtr->I_SBtype)
				{
					case I_BehaviourMarinePlayer:
					case I_BehaviourAlienPlayer:
					case I_BehaviourPredatorPlayer:
					case I_BehaviourPredator:
					case I_BehaviourMarine:
					case I_BehaviourSeal:
					case I_BehaviourNetGhost:
					{
						/* make alienesque noise */
						Sound_Play(SID_HIT_FLESH,"h");

						/* damage unfortunate object */
						CauseDamageToObject(reportPtr->ObstacleSBPtr,ALIEN_CONTACT_WEAPON_DAMAGE,NULL);
						break;
					}
					default:
						break;
				}
			}								 
			/* skip to next report */
			reportPtr = reportPtr->NextCollisionReportPtr;
		}
	}
	else 
	{
		contactWeaponTimer -= NormalFrameTime;
	}

}
#endif

/* Demo code removed, CDF 28/9/98, by order of Kevin */

#if LOAD_IN_MOVEMENT_VALUES	
static void LoadInMovementValues(void)
{

	FILE *fpInput;

	fpInput = fopen("movement.txt","rb");

	while(fgetc(fpInput) != '#');
	while(fgetc(fpInput) != '#');
	fscanf(fpInput, "%d",&AlienForwardSpeed);
	while(fgetc(fpInput) != '#');
	fscanf(fpInput, "%d",&AlienStrafeSpeed);
	while(fgetc(fpInput) != '#');
	fscanf(fpInput, "%d",&AlienTurnSpeed);
	while(fgetc(fpInput) != '#');
	fscanf(fpInput, "%d",&AlienJumpSpeed);

	while(fgetc(fpInput) != '#');
	fscanf(fpInput, "%d",&PredatorForwardSpeed);
	while(fgetc(fpInput) != '#');
	fscanf(fpInput, "%d",&PredatorStrafeSpeed);
	while(fgetc(fpInput) != '#');
	fscanf(fpInput, "%d",&PredatorTurnSpeed);
	while(fgetc(fpInput) != '#');
	fscanf(fpInput, "%d",&PredatorJumpSpeed);

	while(fgetc(fpInput) != '#');
	fscanf(fpInput, "%d",&MarineForwardSpeed);
	while(fgetc(fpInput) != '#');
	fscanf(fpInput, "%d",&MarineStrafeSpeed);
	while(fgetc(fpInput) != '#');
	fscanf(fpInput, "%d",&MarineTurnSpeed);
	while(fgetc(fpInput) != '#');
	fscanf(fpInput, "%d",&MarineJumpSpeed);

	fclose(fpInput);
}
#endif


void ThrowAFlare(void)
{
	extern int NumberOfFlaresActive;
	
	if (NumberOfFlaresActive<4)
	{
		extern VIEWDESCRIPTORBLOCK *ActiveVDBList[];
		VIEWDESCRIPTORBLOCK *VDBPtr = ActiveVDBList[0];
 		MATRIXCH mat = VDBPtr->VDB_Mat;
		VECTORCH position = VDBPtr->VDB_World;

		TransposeMatrixCH(&mat);

		CreateGrenadeKernel(I_BehaviourFlareGrenade,&position,&mat,1);
	   	Sound_Play(SID_THROW_FLARE,"h");
	}

}
