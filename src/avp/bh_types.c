#define DB_LEVEL 3

#include "3dc.h"
#include "inline.h"
#include "module.h"

#include "stratdef.h"
#include "gamedef.h"
#include "bh_types.h"
#include "bonusabilities.h"

#include "weapons.h"
#include "comp_shp.h"
#include "inventry.h"
#include "triggers.h"

#include "dynblock.h"
#include "dynamics.h"

#define UseLocalAssert Yes

#include "ourasert.h"

#include "pmove.h"
#include "bh_alien.h"
#include "bh_gener.h"
#include "pvisible.h"
#include "bh_pred.h"
#include "bh_xeno.h"
#include "bh_paq.h"
#include "bh_queen.h"
#include "bh_marin.h"
#include "bh_fhug.h"
#include "bh_swdor.h"
#include "bh_ldoor.h"
#include "bh_plift.h"
#include "load_shp.h"
#include "bh_weap.h"
#include "bh_debri.h"
#include "lighting.h"
#include "bh_lnksw.h"
#include "bh_binsw.h"
#include "bh_spcl.h"
#include "bh_agun.h"
#include "bh_lift.h"
#include "bh_ltfx.h"
#include "bh_snds.h"
#include "bh_mission.h"
#include "bh_track.h"
#include "bh_fan.h"
#include "bh_rubberduck.h"
#include "bh_plachier.h"
#include "bh_light.h"
#include "bh_cable.h"
#include "bh_deathvol.h"
#include "bh_selfdest.h"
#include "bh_dummy.h"
#include "bh_pargen.h"
#include "bh_videoscreen.h"

#include "psnd.h"
#include "plat_shp.h"
#include "savegame.h"

#include "db.h"

/* for win95 net game support */
#include "pldghost.h"

#include "bh_corpse.h"

/* 
	our functions for doing stuff to objects 
  	can be called in two different ways, call the function
  	that processes the list (ObjectBehaviours) or call
  	ExecuteBehaviour with the strategyblock
*/


/**** extern globals ****/

extern int NormalFrameTime;

// Standard Behaviours - note others are in relevent files

static void* DoorProxBehaveInit(void* bhdata, STRATEGYBLOCK* sbptr);
static void DoorProxBehaveFun(STRATEGYBLOCK* sbptr);
static void* SimpleAnimationBehaveInit(void* bhdata, STRATEGYBLOCK* sbptr);
static void SimpleAnimBehaveFun(STRATEGYBLOCK* sbptr);
static void* InitDatabase(void* bhdata, STRATEGYBLOCK* sbptr);

// support functions - others are extened in bh_types.h

static int AnythingNearProxDoor(MODULE *doorModulePtr,PROXDOOR_BEHAV_BLOCK *doorbhv);
//static void CountToDeallocateTemporaryObject(STRATEGYBLOCK *);

/***** extern functions *****/

extern void SmokeGeneratorBehaviour(STRATEGYBLOCK *sptr);
extern void InitPlayer(STRATEGYBLOCK* sbPtr, int sb_type); 
extern void AlienFragFun(STRATEGYBLOCK* sptr);
extern void SetupSimpleAnimation(int counter, STRATEGYBLOCK *sbPtr);
extern void HierarchicalFragmentBehaviour(STRATEGYBLOCK *sptr);

extern void Xeno_Enter_PowerUp_State(STRATEGYBLOCK *sbPtr);
extern void Xeno_Enter_PowerDown_State(STRATEGYBLOCK *sbPtr);

/************************ FUNCTIONS TO FILL OUT SBS ***************/
/* essentially these are the old entity type functions ******/

/* IMPORTANT IMPORTANT!!!
	
	If you write an allocater for a behaviour that AllocatesMem BESIDES
	that for the BehaviourBlock e.g. the simpleanimations generates a
	linked list of texture animation control blocks, one for every item 
	with an animation, THEN YOU MUST ALWAYS WRITE A SPECIFIC DEALLOCATER
*/


static NPC_DATA NpcDataList[I_NPC_End]= {
	{
		I_NPC_Civilian,
		{
			20,	/* Health */
			0,	/* Armour */
			0, /* IsOnFire */
			{
				0,	/* Acid Resistant */
				0,	/* Fire Resistant */
				0,	/* Electric Resistant */
				0,	/* Perfect Armour */
				0,	/* Electric Sensitive */
				1,	/* Combustability */
				0,	/* Indestructable */
			},
		},
	},
	{
		I_NPC_FaceHugger,
		{
			5,	/* Health */
			0,	/* Armour */
			0, /* IsOnFire */
			{
				1,	/* Acid Resistant */
				0,	/* Fire Resistant */
				0,	/* Electric Resistant */
				0,	/* Perfect Armour */
				1,	/* Electric Sensitive */
				3,	/* Combustability */
				0,	/* Indestructable */
			},
		},
	},
	{
		I_NPC_ChestBurster,
		{
			15,	/* Health */
			0,	/* Armour */
			0, /* IsOnFire */
			{
				1,	/* Acid Resistant */
				0,	/* Fire Resistant */
				0,	/* Electric Resistant */
				0,	/* Perfect Armour */
				1,	/* Electric Sensitive */
				1,	/* Combustability */
				0,	/* Indestructable */
			},
		},
	},
	{
		I_NPC_Alien,
		{
			30,	/* Health */
			5,	/* Armour */
			0, /* IsOnFire */
			{
				1,	/* Acid Resistant */
				0,	/* Fire Resistant */
				0,	/* Electric Resistant */
				1,	/* Perfect Armour */
				1,	/* Electric Sensitive */
				1,	/* Combustability */
				0,	/* Indestructable */
			},
		},
	},
	{
		I_NPC_Xenoborg,
		{
			750,/* Health */
			120,/* Armour */
			0, /* IsOnFire */
			{
				1,	/* Acid Resistant */
				0,	/* Fire Resistant */
				0,	/* Electric Resistant */
				0,	/* Perfect Armour */
				0,	/* Electric Sensitive */
				0,	/* Combustability */
				0,	/* Indestructable */
			},
		},
	},
	{
		I_NPC_Marine,
		{
			25,	/* Health */
			8,	/* Armour */
			0, /* IsOnFire */
			{
				0,	/* Acid Resistant */
				0,	/* Fire Resistant */
				0,	/* Electric Resistant */
				0,	/* Perfect Armour */
				0,	/* Electric Sensitive */
				1,	/* Combustability */
				0,	/* Indestructable */
			},
		},
	},
	{
		I_NPC_PredatorAlien,
		{
			200,	/* Health */
			40,	/* Armour */
			0, /* IsOnFire */
			{
				1,	/* Acid Resistant */
				0,	/* Fire Resistant */
				0,	/* Electric Resistant */
				1,	/* Perfect Armour */
				1,	/* Electric Sensitive */
				1,	/* Combustability */
				0,	/* Indestructable */
			},
		},
	},
	{
		I_NPC_SFMarine,
		{
			40,	/* Health */
			10,	/* Armour */
			0, /* IsOnFire */
			{
				0,	/* Acid Resistant */
				0,	/* Fire Resistant */
				0,	/* Electric Resistant */
				0,	/* Perfect Armour */
				0,	/* Electric Sensitive */
				1,	/* Combustability */
				0,	/* Indestructable */
			},
		},
	},
	{
		I_NPC_Predator,
		{
			450,	/* Health */
			200,	/* Armour */
			0, /* IsOnFire */
			{
				0,	/* Acid Resistant */
				0,	/* Fire Resistant */
				0,	/* Electric Resistant */
				0,	/* Perfect Armour */
				0,	/* Electric Sensitive */
				1,	/* Combustability */
				0,	/* Indestructable */
			},
		},
	},
	{
		I_NPC_PraetorianGuard,
		{
			150,	/* Health */
			60,	/* Armour */
			0, /* IsOnFire */
			{
				1,	/* Acid Resistant */
				0,	/* Fire Resistant */
				0,	/* Electric Resistant */
				1,	/* Perfect Armour */
				1,	/* Electric Sensitive */
				1,	/* Combustability */
				0,	/* Indestructable */
			},
		},
	},
	{
		I_NPC_AlienQueen,
		{
			4000,	/* Health */
			1000,	/* Armour */
			0, /* IsOnFire */
			{
				1,	/* Acid Resistant */
				0,	/* Fire Resistant */
				0,	/* Electric Resistant */
				1,	/* Perfect Armour */
				0,	/* Electric Sensitive */
				1,	/* Combustability */
				0,	/* Indestructable */
			},
		},
	},
	{
		I_NPC_DefaultInanimate,
		{
			10,	/* Health */
			2,	/* Armour */
			0, /* IsOnFire */
			{
				1,	/* Acid Resistant */
				1,	/* Fire Resistant */
				0,	/* Electric Resistant */
				1,	/* Perfect Armour */
				0,	/* Electric Sensitive */
				1,	/* Combustability */
				0,	/* Indestructable */
			},
		},
	},
	{
		I_PC_Alien_Easy,
		{
			90,	/* Health */
			30,	/* Armour (was 6) */
			0, /* IsOnFire */
			{
				1,	/* Acid Resistant */
				0,	/* Fire Resistant */
				0,	/* Electric Resistant */
				0,	/* Perfect Armour */  /* Disabled, CDF 28/7/98 */
				1,	/* Electric Sensitive */
				1,	/* Combustability */
				0,	/* Indestructable */
			},
		},
	},
	{
		I_PC_Marine_Easy,
		{
			100,	/* Health */
			20,	/* Armour */
			0, /* IsOnFire */
			{
				1,	/* Acid Resistant */
				0,	/* Fire Resistant */
				0,	/* Electric Resistant */
				0,	/* Perfect Armour */
				0,	/* Electric Sensitive */
				1,	/* Combustability */
				0,	/* Indestructable */
			},
		},
	},
	{
		I_PC_Predator_Easy,
		{
			450,	/* Health */
			0,	/* Armour */
			0, /* IsOnFire */
			{
				1,	/* Acid Resistant */
				0,	/* Fire Resistant */
				0,	/* Electric Resistant */
				0,	/* Perfect Armour */
				0,	/* Electric Sensitive */
				1,	/* Combustability */
				0,	/* Indestructable */
			},
		},
	},
	{
		I_PC_Alien_Medium,
		{
			90,	/* Health */
			30,	/* Armour (was 6) */
			0, /* IsOnFire */
			{
				1,	/* Acid Resistant */
				0,	/* Fire Resistant */
				0,	/* Electric Resistant */
				0,	/* Perfect Armour */  /* Disabled, CDF 28/7/98 */
				1,	/* Electric Sensitive */
				1,	/* Combustability */
				0,	/* Indestructable */
			},
		},
	},
	{
		I_PC_Marine_Medium,
		{
			100,	/* Health */
			20,	/* Armour */
			0, /* IsOnFire */
			{
				0,	/* Acid Resistant */
				0,	/* Fire Resistant */
				0,	/* Electric Resistant */
				0,	/* Perfect Armour */
				0,	/* Electric Sensitive */
				1,	/* Combustability */
				0,	/* Indestructable */
			},
		},
	},
	{
		I_PC_Predator_Medium,
		{
			450,	/* Health */
			0,	/* Armour */
			0, /* IsOnFire */
			{
				0,	/* Acid Resistant */
				0,	/* Fire Resistant */
				0,	/* Electric Resistant */
				0,	/* Perfect Armour */
				0,	/* Electric Sensitive */
				1,	/* Combustability */
				0,	/* Indestructable */
			},
		},
	},
	{
		I_PC_Alien_Hard,
		{
			90,	/* Health */
			30,	/* Armour (was 6) */
			0, /* IsOnFire */
			{
				1,	/* Acid Resistant */
				0,	/* Fire Resistant */
				0,	/* Electric Resistant */
				0,	/* Perfect Armour */  /* Disabled, CDF 28/7/98 */
				1,	/* Electric Sensitive */
				1,	/* Combustability */
				0,	/* Indestructable */
			},
		},
	},
	{
		I_PC_Marine_Hard,
		{
			100,	/* Health */
			20,	/* Armour */
			0, /* IsOnFire */
			{
				0,	/* Acid Resistant */
				0,	/* Fire Resistant */
				0,	/* Electric Resistant */
				0,	/* Perfect Armour */
				0,	/* Electric Sensitive */
				1,	/* Combustability */
				0,	/* Indestructable */
			},
		},
	},
	{
		I_PC_Predator_Hard,
		{
			450,	/* Health */
			0,	/* Armour */
			0, /* IsOnFire */
			{
				0,	/* Acid Resistant */
				0,	/* Fire Resistant */
				0,	/* Electric Resistant */
				0,	/* Perfect Armour */
				0,	/* Electric Sensitive */
				1,	/* Combustability */
				0,	/* Indestructable */
			},
		},
	},
	{
		I_PC_Alien_Impossible,
		{
			30,	/* Health */
			5,	/* Armour */
			0, /* IsOnFire */
			{
				1,	/* Acid Resistant */
				0,	/* Fire Resistant */
				0,	/* Electric Resistant */
				1,	/* Perfect Armour */
				1,	/* Electric Sensitive */
				1,	/* Combustability */
				0,	/* Indestructable */
			},
		},
	},
	{
		I_PC_Marine_Impossible,
		{
			25,	/* Health */
			8,	/* Armour */
			0, /* IsOnFire */
			{
				0,	/* Acid Resistant */
				0,	/* Fire Resistant */
				0,	/* Electric Resistant */
				0,	/* Perfect Armour */
				0,	/* Electric Sensitive */
				1,	/* Combustability */
				0,	/* Indestructable */
			},
		},
	},
	{
		I_PC_Predator_Impossible,
		{
			450,	/* Health */
			200,	/* Armour */
			0, /* IsOnFire */
			{
				0,	/* Acid Resistant */
				0,	/* Fire Resistant */
				0,	/* Electric Resistant */
				0,	/* Perfect Armour */
				0,	/* Electric Sensitive */
				1,	/* Combustability */
				0,	/* Indestructable */
			},
		},
	},
	{
		I_PC_Alien_MaxStats,
		{
			180,	/* Health */
			30,	/* Armour (was 6) */
			0, /* IsOnFire */
			{
				1,	/* Acid Resistant */
				0,	/* Fire Resistant */
				0,	/* Electric Resistant */
				0,	/* Perfect Armour */  /* Disabled, CDF 28/7/98 */
				0,	/* Electric Sensitive */
				1,	/* Combustability */
				0,	/* Indestructable */
			},
		},
	},
	{
		I_NPC_SentryGun,
		{
			50,	/* Health */
			50,	/* Armour */
			0, /* IsOnFire */
			{
				0,	/* Acid Resistant */
				0,	/* Fire Resistant */
				0,	/* Electric Resistant */
				0,	/* Perfect Armour */
				0,	/* Electric Sensitive */
				1,	/* Combustability */
				0,	/* Indestructable */
			},
		},
	},
	{
		I_NPC_Android,
		{
			40,	/* Health */
			8,	/* Armour */
			0, /* IsOnFire */
			{
				0,	/* Acid Resistant */
				0,	/* Fire Resistant */
				0,	/* Electric Resistant */
				0,	/* Perfect Armour */
				0,	/* Electric Sensitive */
				1,	/* Combustability */
				0,	/* Indestructable */
			},
		},
	},
};

/* Interface Function! */

NPC_DATA *GetThisNpcData(NPC_TYPES NpcType) {
int a;

for (a=0; a<I_NPC_End; a++) {
	if (NpcDataList[a].Type==NpcType) break;
}

if (a==I_NPC_End) return(NULL);
else return(&NpcDataList[a]);

}

/*----------------------------------------------------------------------
  Use this function to initialise compiled in objects - redundant in final 
  ----------------------------------------------------------------------*/


void AssignRunTimeBehaviours(STRATEGYBLOCK* sbptr) 
{
	/* 
	function for assigning behaviours to objects at
	runtime. This includes any objects that are going
	to be created (grenades) AND compiled in ints such 
    as the player
	*/
			

	DISPLAYBLOCK *dptr;

	GLOBALASSERT(sbptr);
	dptr = sbptr->SBdptr;
	GLOBALASSERT(dptr);

	dptr->ObShape = I_ShapeCube;

	InitPlayer(sbptr, I_BehaviourMarinePlayer);
}

/*----------------------------------------------------------------------
  Use this function to initialise binary loaded objects
  ----------------------------------------------------------------------*/

void EnableBehaviourType(STRATEGYBLOCK* sbptr, AVP_BEHAVIOUR_TYPE sb_type, void *bhdata)
{

	GLOBALASSERT(sbptr);

	sbptr->I_SBtype = sb_type;

	switch(sb_type)
	{
		case I_BehaviourAlien:
      		InitAlienBehaviour(bhdata, sbptr);
      		break;

		case I_BehaviourFaceHugger:
			InitFacehuggerBehaviour(bhdata, sbptr);
			break;

		case I_BehaviourPredator:
			InitPredatorBehaviour(bhdata, sbptr);
			break;
		
		case I_BehaviourDormantPredator:
			InitDormantPredatorBehaviour(bhdata, sbptr);
			break;

		case I_BehaviourXenoborg:
			InitXenoborgBehaviour(bhdata, sbptr);
			break;

		case I_BehaviourQueenAlien:
			InitQueenBehaviour(bhdata, sbptr);
			break;

		case I_BehaviourPredatorAlien:
			GLOBALASSERT(0);
			//InitPredAlBehaviour(bhdata, sbptr);
			break;

		case I_BehaviourMarine:
			InitMarineBehaviour(bhdata, sbptr);
			break;

		case I_BehaviourSeal:
			InitSealBehaviour(bhdata, sbptr);
			break;

		case I_BehaviourProximityDoor:
			sbptr->SBdataptr = DoorProxBehaveInit(bhdata, sbptr);
			sbptr->SBmoptr->m_flags &= ~m_flag_open;
			break;

		case I_BehaviourLiftDoor:
			sbptr->SBdataptr = LiftDoorBehaveInit(bhdata, sbptr);
			break;

		case I_BehaviourSwitchDoor:
			InitialiseSwitchDoor(bhdata, sbptr);
			break;

		case I_BehaviourPlatform:
			InitialisePlatformLift(bhdata, sbptr);
			break;

		case I_BehaviourBinarySwitch:
			if(sbptr->shapeIndex!=-1)/*Allow for switches that have no shape*/
			{
				sbptr->DynPtr = AllocateDynamicsBlock(DYNAMICS_TEMPLATE_STATIC);
			}
			sbptr->SBdataptr = BinarySwitchBehaveInit(bhdata, sbptr);
			break;

		case I_BehaviourLinkSwitch:
			if(sbptr->shapeIndex!=-1)/*Allow for switches that have no shape*/
			{
				sbptr->DynPtr = AllocateDynamicsBlock(DYNAMICS_TEMPLATE_STATIC);
			}
			sbptr->SBdataptr = LinkSwitchBehaveInit(bhdata, sbptr);
			break;

		case I_BehaviourLift:
			sbptr->SBdataptr = LiftBehaveInit(bhdata, sbptr);
			break;

		case I_BehaviourSimpleAnimation:
			sbptr->SBdataptr = SimpleAnimationBehaveInit(bhdata, sbptr);
			break;
		
		case I_BehaviourGenerator:
			InitGenerator(bhdata, sbptr);
			break;

		case I_BehaviourAutoGun:
			AutoGunBehaveInit(bhdata, sbptr);
			break;

		case I_BehaviourInanimateObject:
			InitInanimateObject(bhdata, sbptr);
			break;

		case I_BehaviourDatabase:
			sbptr->DynPtr = AllocateDynamicsBlock(DYNAMICS_TEMPLATE_STATIC);
			sbptr->SBdataptr  = InitDatabase(bhdata, sbptr);
			break;

		case I_BehaviourXenoborgMorphRoom:
			sbptr->SBdataptr = InitXenoMorphRoom (bhdata, sbptr);
			break;
		
		case I_BehaviourLightFX:
		{
			sbptr->SBdataptr = LightFXBehaveInit (bhdata, sbptr);
			break;
		}

		case I_BehaviourPlacedSound:
		{
			sbptr->SBdataptr = SoundBehaveInit (bhdata, sbptr);
			break;
		}
					
		case I_BehaviourMissionComplete:
		{
			sbptr->SBdataptr = MissionCompleteBehaveInit (bhdata, sbptr);
			break;
		}
		case I_BehaviourMessage:
		{
			sbptr->SBdataptr = MessageBehaveInit (bhdata, sbptr);
			break;
		}
		case I_BehaviourTrackObject:
		{
			sbptr->DynPtr = AllocateDynamicsBlock(DYNAMICS_TEMPLATE_STATIC);
			sbptr->SBdataptr = TrackObjectBehaveInit(bhdata, sbptr);
			break;
		}
		case I_BehaviourFan:
		{
			sbptr->DynPtr = AllocateDynamicsBlock(DYNAMICS_TEMPLATE_STATIC);
			sbptr->SBdataptr = FanBehaveInit(bhdata, sbptr);
			break;
		}
		case I_BehaviourPlacedHierarchy:
		{
			sbptr->DynPtr = AllocateDynamicsBlock(DYNAMICS_TEMPLATE_STATIC);
			sbptr->SBdataptr = PlacedHierarchyBehaveInit(bhdata, sbptr);
			break;
		}
		
		case I_BehaviourPlacedLight:
			InitPlacedLight(bhdata, sbptr);
			break;

		case I_BehaviourVideoScreen:
			InitVideoScreen(bhdata, sbptr);
			break;

		case I_BehaviourPowerCable :
			sbptr->SBdataptr = PowerCableBehaveInit(bhdata, sbptr);
			break;
		
		case I_BehaviourDeathVolume :
			sbptr->SBdataptr = DeathVolumeBehaveInit(bhdata, sbptr);
			break;
		
		case I_BehaviourSelfDestruct :
			sbptr->SBdataptr = SelfDestructBehaveInit(bhdata, sbptr);
			break;

		case I_BehaviourParticleGenerator :
			sbptr->SBdataptr = ParticleGeneratorBehaveInit(bhdata, sbptr);
			break;
		default:
			break;
	}
}





/****************** DOOR INIT ***************************/


/* (these shift values are passed to the open/close prox door fn's,
and control the morphing speed) */


static void* DoorProxBehaveInit(void* bhdata, STRATEGYBLOCK* sbptr)
{
	PROXDOOR_BEHAV_BLOCK *doorbhv;
	PROX_DOOR_TOOLS_TEMPLATE *doortt;
	MORPHCTRL* morphctrl;	
	MORPHHEADER* morphheader;
	MORPHFRAME* morphframe;
	MODULE * my_mod;

 	GLOBALASSERT(sbptr);
	doorbhv = (PROXDOOR_BEHAV_BLOCK*)AllocateMem(sizeof(PROXDOOR_BEHAV_BLOCK));

	if(!doorbhv) 
	{
		memoryInitialisationFailure = 1;
		return ((void *)NULL);
	}

	doorbhv->bhvr_type = I_BehaviourProximityDoor;

	// Who is doing the deallocation for this - the unloaders
	// should CHECK THIS.

	doortt = (PROX_DOOR_TOOLS_TEMPLATE*)bhdata;


	// Set up a new Morph Control
	morphctrl = (MORPHCTRL*)AllocateMem(sizeof(MORPHCTRL));
	if(!morphctrl) 
	{
		memoryInitialisationFailure = 1;
		return ((void *)NULL);
	}
	morphheader = (MORPHHEADER*)AllocateMem(sizeof(MORPHHEADER));
	if(!morphheader) 
	{
		memoryInitialisationFailure = 1;
		return ((void *)NULL);
	}
	morphframe = (MORPHFRAME*)AllocateMem(sizeof(MORPHFRAME));
	if(!morphframe) 
	{
		memoryInitialisationFailure = 1;
		return ((void *)NULL);
	}

	morphframe->mf_shape1 = doortt->shape_open;
	morphframe->mf_shape2 = doortt->shape_closed;

	morphheader->mph_numframes = 1;
	morphheader->mph_maxframes = ONE_FIXED;
	morphheader->mph_frames = morphframe;

	morphctrl->ObMorphCurrFrame = 0;
	morphctrl->ObMorphFlags = 0;
	morphctrl->ObMorphSpeed = 0;
	morphctrl->ObMorphHeader = morphheader;


	// Copy the name over
	COPY_NAME (sbptr->SBname, doortt->nameID);


	// Setup module ref
	{
		MREF mref=doortt->my_module;
		ConvertModuleNameToPointer (&mref, MainSceneArray[0]->sm_marray);
		my_mod = mref.mref_ptr;
	}
	GLOBALASSERT (my_mod);

	my_mod->m_sbptr = sbptr;
	sbptr->SBmoptr = my_mod;
	sbptr->SBmomptr = my_mod->m_mapptr;
	sbptr->SBflags.no_displayblock = 1;


	doorbhv->PDmctrl = morphctrl; 
	doorbhv->lockable_door = doortt->has_lock_target;

	doorbhv->door_opening_speed=doortt->door_opening_speed;
	doorbhv->door_closing_speed=doortt->door_closing_speed;

	if(doortt->has_lock_target)
		{
			COPY_NAME(doorbhv->target_name, doortt->target_name);
		}

	sbptr->SBmorphctrl = doorbhv->PDmctrl;
	sbptr->SBmorphctrl->ObMorphCurrFrame = 1; /* this should be closed*/

	doorbhv->door_state = 	I_door_closed;
	CloseDoor(sbptr->SBmorphctrl, DOOR_CLOSEFASTSPEED);	

	if(sbptr->SBmoptr)
		{
			sbptr->SBmoptr->m_flags |= m_flag_open;
		}
	if(sbptr->SBmomptr)
		{
			sbptr->SBmomptr->MapMorphHeader = sbptr->SBmorphctrl->ObMorphHeader;
		}

	/*-----------Patrick 9/12/96--------------
	 a little addition...
	 -----------------------------------------*/
	doorbhv->alienTrigger = 0;
	doorbhv->marineTrigger = 0;
	doorbhv->triggeredByMarine = 0;
	doorbhv->alienTimer = 0;
	doorbhv->door_locked = doortt->door_is_locked;

  /* Andy 10/6/97 --------------------------
   sound stuff...
  ----------------------------------------*/
  doorbhv->SoundHandle = SOUND_NOACTIVEINDEX;
	
	{
		// Work out the door sound pitch
	
		int maxX,maxY,maxZ,doorSize;

		maxX=mainshapelist[morphframe->mf_shape2]->shapemaxx;
		maxY=mainshapelist[morphframe->mf_shape2]->shapemaxy;
		maxZ=mainshapelist[morphframe->mf_shape2]->shapemaxz;
			
		doorSize = maxX + maxY + maxZ;
		if (doorSize < 3000) doorSize = 3000;
		else if (doorSize > 8000) doorSize = 8000;
		
		doorSize = (3000 - doorSize) >> 4;

		doorbhv->doorType = doorSize; 

	}
		
	return((void*)doorbhv);
}






/******************** ANIMATION INIT *************************/


static void* SimpleAnimationBehaveInit(void* bhdata, STRATEGYBLOCK* sbptr)
{
	/**RWH 10/12/96 ***************************************

	Simple Animation intialistation. This will set up an array
	of texture control blocks. Each textures controlblock will animate only 
	one sequence per polygon.

	***********************************************************/

	SIMPLE_ANIM_BEHAV_BLOCK *sabhv;
	int item_num;
	TXACTRLBLK **pptxactrlblk;		
	SIMPLE_ANIM_TOOLS_TEMPLATE * satt = (SIMPLE_ANIM_TOOLS_TEMPLATE *)bhdata;
	int shape_num = satt->shape_num;
	SHAPEHEADER *shptr = GetShapeData(shape_num);
	MODULE * my_mod;
   	

	GLOBALASSERT(shptr);
	GLOBALASSERT(shptr->numitems > 0);

	SetupPolygonFlagAccessForShape(shptr);

	sabhv = (SIMPLE_ANIM_BEHAV_BLOCK*)AllocateMem(sizeof(SIMPLE_ANIM_BEHAV_BLOCK));
	if(!sabhv) 
	{
		memoryInitialisationFailure = 1;
		return ((void *)NULL);
	}
		
	sabhv->bhvr_type = I_BehaviourSimpleAnimation;
	
	// Copy the name over
	COPY_NAME (sbptr->SBname, satt->nameID);
	
	// Setup module ref

	if (*((int *)satt->my_module.mref_name))
	{
		{
			MREF mref=satt->my_module;
			ConvertModuleNameToPointer (&mref, MainSceneArray[0]->sm_marray);
			my_mod = mref.mref_ptr;
		}
		GLOBALASSERT (my_mod);

		my_mod->m_sbptr = sbptr;
		sbptr->SBmoptr = my_mod;
		sbptr->SBmomptr = my_mod->m_mapptr;
		sbptr->SBflags.no_displayblock = 1;
	}


	/* we need to reserve the address of where we 
		 are going to place the new animation ctrl block.

		 this is so that we can call the texture animation builder
		 in the below loop
	*/

	pptxactrlblk = &sabhv->tacbSimple;

	/*
	the bhdata is a ptr to the SHAPEHEADER each 
	animating polygon has an array of sequences, in 
	this case thers is only onr sequence per array
	*/

	for(item_num = 0; item_num < shptr->numitems; item_num ++)
		{
			POLYHEADER *poly =  (POLYHEADER*)(shptr->items[item_num]);
			LOCALASSERT(poly);
				
			if((Request_PolyFlags((void *)poly)) & iflag_txanim)
				{
					TXACTRLBLK *pnew_txactrlblk;

					pnew_txactrlblk = AllocateMem(sizeof(TXACTRLBLK));
					if (pnew_txactrlblk)
					{
 						pnew_txactrlblk->tac_flags = 0;										
						pnew_txactrlblk->tac_item = item_num;										
						pnew_txactrlblk->tac_sequence = 0;										
						pnew_txactrlblk->tac_node = 0;										
						pnew_txactrlblk->tac_txarray = GetTxAnimArrayZ(shape_num, item_num);										
						pnew_txactrlblk->tac_txah_s = GetTxAnimHeaderFromShape(pnew_txactrlblk, shape_num);

						/* set the flags in the animation header */

						/* KJL 17:00:38 04/04/97 - ChrisF complained about the next line, so
						I've turned it off to see if anything happens. */
						/*pnew_txactrlblk->tac_txah.txa_flags = txa_flag_play; */
					
						/* change the value held in pptxactrlblk
						 which point to the previous structures "next"
						 pointer*/

						*pptxactrlblk = pnew_txactrlblk;
						pptxactrlblk = &pnew_txactrlblk->tac_next;
					}
					else
					{
						memoryInitialisationFailure = 1;
					}
				}
		}
		*pptxactrlblk=0;

	return((void*)sabhv);
}



static void* InitDatabase(void* bhdata, STRATEGYBLOCK* sbptr)
{
	DATABASE_BLOCK *db;
	DATABASE_TOOLS_TEMPLATE *dbtt;

	GLOBALASSERT(sbptr);
	GLOBALASSERT(bhdata);
	dbtt = (DATABASE_TOOLS_TEMPLATE*)bhdata;

	db = (DATABASE_BLOCK*)AllocateMem(sizeof(DATABASE_BLOCK));
	if(!db) 
	{
		memoryInitialisationFailure = 1;
		return ((void *)NULL);
	}

	db->bhvr_type=I_BehaviourDatabase;

	db->num = dbtt->num;

	sbptr->DynPtr->Position = sbptr->DynPtr->PrevPosition = dbtt->position;
	sbptr->DynPtr->OrientEuler = dbtt->orientation;
	CreateEulerMatrix(&sbptr->DynPtr->OrientEuler, &sbptr->DynPtr->OrientMat);
	TransposeMatrixCH(&sbptr->DynPtr->OrientMat);	

	sbptr->shapeIndex = dbtt->shape_num;

	return(void*)db;
}





/*********************************************************************
******************************** ASSIGN SB  Names ********************/


void AssignAllSBNames()
{

	int stratblock = NumActiveStBlocks;
	GLOBALASSERT(stratblock>=0);

	while(--stratblock >= 0)
		{
			STRATEGYBLOCK* sbptr = ActiveStBlockList[stratblock];
			GLOBALASSERT(sbptr);

			switch (sbptr->I_SBtype)
			  {
					case I_BehaviourBinarySwitch:
						{
							BINARY_SWITCH_BEHAV_BLOCK *bs_bhv;
							int i;
   						bs_bhv = (BINARY_SWITCH_BEHAV_BLOCK*)sbptr->SBdataptr;
							GLOBALASSERT((bs_bhv->bhvr_type == I_BehaviourBinarySwitch));
							for (i=0; i<bs_bhv->num_targets; i++)
							{
								bs_bhv->bs_targets[i] = FindSBWithName(bs_bhv->target_names[i].name);
							}
			 				break;
						}
					case I_BehaviourLinkSwitch:
						{
							LINK_SWITCH_BEHAV_BLOCK *ls_bhv;
							int i=0;
   							ls_bhv = (LINK_SWITCH_BEHAV_BLOCK*)sbptr->SBdataptr;
							GLOBALASSERT((ls_bhv->bhvr_type == I_BehaviourLinkSwitch));

							for(i=0;i<ls_bhv->num_linked_switches;i++)
							{
								ls_bhv->lswitch_list[i].bswitch = FindSBWithName(ls_bhv->lswitch_list[i].bs_name);
							}

							for(i=0;i<ls_bhv->num_targets;i++)
							{
								ls_bhv->ls_targets[i].sbptr=FindSBWithName(ls_bhv->ls_targets[i].name);
							}

			 				break;
						}
					case I_BehaviourSwitchDoor:
					{
						SWITCH_DOOR_BEHAV_BLOCK *switchDoorBehaviourPtr = (SWITCH_DOOR_BEHAV_BLOCK *)sbptr->SBdataptr;
						if(*((int *)switchDoorBehaviourPtr->linkedDoorName) + *( ((int *)switchDoorBehaviourPtr->linkedDoorName)+1 ) )
							switchDoorBehaviourPtr->linkedDoorPtr = FindSBWithName(switchDoorBehaviourPtr->linkedDoorName);
			 			/* may or may not be linked to another door */
			 			if(switchDoorBehaviourPtr->linkedDoorPtr)
						{
			 				GLOBALASSERT(switchDoorBehaviourPtr->linkedDoorPtr->I_SBtype == I_BehaviourSwitchDoor);
						}
			 			break;
					}
					case I_BehaviourProximityDoor:
						{
							PROXDOOR_BEHAV_BLOCK *doorbhv;
						 	doorbhv = (PROXDOOR_BEHAV_BLOCK*)sbptr->SBdataptr;
							GLOBALASSERT((doorbhv->bhvr_type == I_BehaviourProximityDoor));
							if(doorbhv->lockable_door)
								{
								 	doorbhv->door_lock_target = FindSBWithName(doorbhv->target_name);
								}
							break;
						}
					case I_BehaviourLift:
					{
						LIFT_BEHAV_BLOCK *lift_bhv;	
						LIFT_STATION *curr_stn;
						lift_bhv = (LIFT_BEHAV_BLOCK*)sbptr->SBdataptr;
						GLOBALASSERT((lift_bhv->bhvr_type == I_BehaviourLift));
						curr_stn = &lift_bhv->lift_station;
						GLOBALASSERT(curr_stn);
						curr_stn->lift_call_switch = FindSBWithName(curr_stn->lift_call_switch_name);
						if((*(int*)&curr_stn->lift_floor_switch_name[0]) ||	(*(int*)&curr_stn->lift_floor_switch_name[4]))
							curr_stn->lift_floor_switch = FindSBWithName(curr_stn->lift_floor_switch_name); //lifts with switches that don't teleport,don't know about their floor switches
						curr_stn->lift_door = FindSBWithName(curr_stn->lift_door_name);
						
						// find my module pointer

						{
							STRATEGYBLOCK *my_sb;
							my_sb = FindSBWithName(curr_stn->my_sb_name);
							if(my_sb)							
								curr_stn->lift_module = my_sb->SBmoptr;
								
						}							
							
						// find the controlling lift behave block
						lift_bhv->control_sb = FindSBWithName(lift_bhv->control_sb_name);
						
						GLOBALASSERT(lift_bhv->control_sb);

						{
							LIFT_STATION **lift_array;
							LIFT_BEHAV_BLOCK *cont_bhv = lift_bhv->control_sb->SBdataptr;
	
							lift_bhv->lift_control = cont_bhv->lift_control;
							lift_array = cont_bhv->lift_control->lift_stations;

							lift_array[curr_stn->num_floor] = curr_stn;
						}

						if(curr_stn->starting_station)
						{
							// the door on this floor is open
							// therefore, since we are all perfect
							// i can say

							GLOBALASSERT(lift_bhv->lift_control->curr_station == -1);
							
							lift_bhv->lift_control->curr_station = curr_stn->num_floor;
							if(curr_stn->env == AvP.CurrentEnv)
							{
								if(curr_stn->lift_call_switch)
								{
									// turn on the floor lights
									RequestState(curr_stn->lift_call_switch, 1, 0);
									if((*(int*)&curr_stn->lift_floor_switch_name[0]) ||	(*(int*)&curr_stn->lift_floor_switch_name[4]))
										RequestState(curr_stn->lift_floor_switch, 1, 0);
								}
							}
							
							
						}

							

						break;
					}
					
					case I_BehaviourXenoborgMorphRoom:
					{
						XENO_MORPH_ROOM_DATA * xmrd = (XENO_MORPH_ROOM_DATA *)sbptr->SBdataptr;

						GLOBALASSERT (xmrd->bhvr_type == I_BehaviourXenoborgMorphRoom);

						xmrd->DoorToRoom = FindSBWithName(xmrd->doorID);
						break;
					}
					
					default:
						break;
					
					case I_BehaviourAlien:
						{
							ALIEN_STATUS_BLOCK *alienStatus = (ALIEN_STATUS_BLOCK *)sbptr->SBdataptr;
							alienStatus->death_target_sbptr = FindSBWithName(alienStatus->death_target_ID);
						
			 				break;
						}
					case I_BehaviourMarine:
						{
							MARINE_STATUS_BLOCK *marineStatus = (MARINE_STATUS_BLOCK *)sbptr->SBdataptr;
							marineStatus->death_target_sbptr = FindSBWithName(marineStatus->death_target_ID);
						
			 				break;
						}
					case I_BehaviourPredator:
						{
							PREDATOR_STATUS_BLOCK *predatorStatus = (PREDATOR_STATUS_BLOCK *)sbptr->SBdataptr;
							predatorStatus->death_target_sbptr = FindSBWithName(predatorStatus->death_target_ID);
						
			 				break;
						}
					case I_BehaviourXenoborg:
						{
							XENO_STATUS_BLOCK *xenoStatus = (XENO_STATUS_BLOCK *)sbptr->SBdataptr;
							xenoStatus->death_target_sbptr = FindSBWithName(xenoStatus->death_target_ID);
						
			 				break;
						}
					case I_BehaviourAutoGun:
						{
							AUTOGUN_STATUS_BLOCK *agunStatus = (AUTOGUN_STATUS_BLOCK *)sbptr->SBdataptr;
							agunStatus->death_target_sbptr = FindSBWithName(agunStatus->death_target_ID);
						
			 				break;
						}
					case I_BehaviourQueenAlien:
						{
							QUEEN_STATUS_BLOCK *queenStatus = (QUEEN_STATUS_BLOCK *)sbptr->SBdataptr;
							queenStatus->death_target_sbptr = FindSBWithName(queenStatus->death_target_ID);
						
			 				break;
						}
					case I_BehaviourFaceHugger:
						{
							FACEHUGGER_STATUS_BLOCK *facehuggerStatus = (FACEHUGGER_STATUS_BLOCK *)sbptr->SBdataptr;
							facehuggerStatus->death_target_sbptr = FindSBWithName(facehuggerStatus->death_target_ID);
					
			 				break;
						}
					case I_BehaviourTrackObject:
						{
							int i;
							TRACK_OBJECT_BEHAV_BLOCK *to_bhv = (TRACK_OBJECT_BEHAV_BLOCK *)sbptr->SBdataptr;
							for(i=0;i<to_bhv->num_special_track_points;i++)
							{
								int j;
								SPECIAL_TRACK_POINT* stp=&to_bhv->special_track_points[i];
								for(j=0;j<stp->num_targets;j++)
								{
									stp->targets[j].target_sbptr=FindSBWithName(stp->targets[j].target_name);
								}
							}
							
							to_bhv->destruct_target_sbptr=FindSBWithName(to_bhv->destruct_target_ID);
								
			 				break;
						}
					case I_BehaviourPlacedHierarchy:
						{
							int i;
							PLACED_HIERARCHY_BEHAV_BLOCK *ph_bhv = (PLACED_HIERARCHY_BEHAV_BLOCK *)sbptr->SBdataptr;
							for(i=0;i<ph_bhv->num_special_track_points;i++)
							{
								int j;
								SPECIAL_TRACK_POINT* stp=&ph_bhv->special_track_points[i];
								for(j=0;j<stp->num_targets;j++)
								{
									stp->targets[j].target_sbptr=FindSBWithName(stp->targets[j].target_name);
								}
							}
							
			 				break;
						}
					
					case I_BehaviourInanimateObject:
						{
							INANIMATEOBJECT_STATUSBLOCK* objectstatusptr=(INANIMATEOBJECT_STATUSBLOCK*)sbptr->SBdataptr;
							if(objectstatusptr)
							{
								if(objectstatusptr->event_target)
								{
									objectstatusptr->event_target->event_target_sbptr=FindSBWithName(objectstatusptr->event_target->event_target_ID);
								}
							}
							break;
						
						}
					
					case I_BehaviourPlacedLight:
						{
							PLACED_LIGHT_BEHAV_BLOCK* pl_bhv=(PLACED_LIGHT_BEHAV_BLOCK*)sbptr->SBdataptr;
							if(pl_bhv)
							{
								pl_bhv->destruct_target_sbptr=FindSBWithName(pl_bhv->destruct_target_ID);
								
							}
							break;
						
						}
					case I_BehaviourVideoScreen:
						{
							VIDEO_SCREEN_BEHAV_BLOCK* videoScreen=(VIDEO_SCREEN_BEHAV_BLOCK*)sbptr->SBdataptr;
							if(videoScreen)
							{
								videoScreen->destruct_target_sbptr=FindSBWithName(videoScreen->destruct_target_ID);
								
							}
							break;
						
						}
			  }
		}
}


/**************************************************************************************/
/******************************* STUFF TO DO BEHAVIOURS *******************************/
							 					
							 																 
void ObjectBehaviours(void)
{
	int i;	

#ifdef AVP_DEBUG_VERSION
	for (i=0; i<NumActiveStBlocks; i++)
	{
		if (ActiveStBlockList[i]->SBdptr)
		{
			int j = i+1;

			for (; j<NumActiveStBlocks; j++)
			{
				if (ActiveStBlockList[i]->SBdptr == ActiveStBlockList[j]->SBdptr)
				{
					GLOBALASSERT (0);
				}
			}
		}
	}
#endif

	RequestEnvChangeViaLift	= 0;

	i = 0;
	
	while(i < NumActiveStBlocks)
	{
		ExecuteBehaviour(ActiveStBlockList[i++]);
  }

}			


void ExecuteBehaviour(STRATEGYBLOCK* sbptr)
{
	GLOBALASSERT(sbptr);

	switch(sbptr->I_SBtype)
	{
		case I_BehaviourInanimateObject:
			InanimateObjectBehaviour(sbptr);
			break;
	
		case I_BehaviourProximityDoor:
			DoorProxBehaveFun(sbptr);
			break;

		case I_BehaviourBinarySwitch:
			BinarySwitchBehaveFun(sbptr);
			break;

		case I_BehaviourLiftDoor:
			LiftDoorBehaveFun(sbptr);
			break;

		case I_BehaviourLift:
			LiftBehaveFun(sbptr);
			break;

    case I_BehaviourGenerator:
			GeneratorBehaviour(sbptr);
			break;

		case I_BehaviourMarinePlayer:
			PlayerBehaviour(sbptr);
			break;

  	case I_BehaviourAlien:
			AlienBehaviour(sbptr);
			break;

		case I_BehaviourSwitchDoor:
			SwitchDoorBehaviour(sbptr);
			break;
	  
		case I_BehaviourLinkSwitch:
			LinkSwitchBehaveFun(sbptr);
			break;
			
		case I_BehaviourPlatform:
			PlatformLiftBehaviour(sbptr);
			break;

		case I_BehaviourSimpleAnimation:
			SimpleAnimBehaveFun(sbptr);
			break;
		
		case I_BehaviourAutoGun:
			AutoGunBehaveFun(sbptr);
			break;			

	
		case I_BehaviourFragmentationGrenade:
		case I_BehaviourGrenade:
			GrenadeBehaviour(sbptr);
			break;
		case I_BehaviourMolotov:
			MolotovBehaviour(sbptr);
			break;
    	
    case I_BehaviourProximityGrenade:
			ProximityGrenadeBehaviour(sbptr);
    	break;

    case I_BehaviourFlareGrenade:
			FlareGrenadeBehaviour(sbptr);
    	break;
				
		case I_BehaviourClusterGrenade:
			ClusterGrenadeBehaviour(sbptr);
			break;
	 
		case I_BehaviourFrisbee:
			FrisbeeBehaviour(sbptr);
			break;

		case I_BehaviourRocket:
			RocketBehaviour(sbptr);
			break;

		case I_BehaviourPulseGrenade:
			PulseGrenadeBehaviour(sbptr);
			break;
		
		case I_BehaviourFaceHugger:
			FacehuggerBehaviour(sbptr);
			break;

		case I_BehaviourPredator:
			PredatorBehaviour(sbptr);
			break;

		case I_BehaviourXenoborg:
			XenoborgBehaviour(sbptr);
			break;

		case I_BehaviourQueenAlien:
			QueenBehaviour(sbptr);
			break;

		case I_BehaviourPredatorAlien:
			GLOBALASSERT(0);
			//PAQBehaviour(sbptr);
			break;

		case I_BehaviourMarine:
			MarineBehaviour(sbptr);
			break;

		case I_BehaviourSeal:
			MarineBehaviour(sbptr);
			break;
					
		
		case I_BehaviourHierarchicalFragment:
			{
				#if 0
				HDEBRIS_BEHAV_BLOCK *hbbptr=(HDEBRIS_BEHAV_BLOCK *)sbptr->SBdataptr;
				LOCALASSERT(hbbptr);
				if (hbbptr->HModelController.Root_Section->flags&section_sprays_acid) {
					AlienFragFun(sbptr);
				} else {
					OneShotBehaveFun(sbptr);
				}
				#else
				HierarchicalFragmentBehaviour(sbptr);
				#endif
			}
			break;
		case I_BehaviourAlienFragment:
			AlienFragFun(sbptr);
			break;

  	case I_BehaviourFragment:
		case I_BehaviourAutoGunMuzzleFlash:
			OneShotBehaveFun(sbptr);
	  	break;

		case I_BehaviourOneShotAnim:
			OneShot_Anim_BehaveFun(sbptr);
			break;			
  		
		case I_BehaviourPPPlasmaBolt:
			PPPlasmaBoltBehaviour(sbptr); 
			break;

		case I_BehaviourSpeargunBolt:
			SpeargunBoltBehaviour(sbptr);
			break;

		case I_BehaviourPredatorEnergyBolt:
			PredatorEnergyBoltBehaviour(sbptr); 
			break;

		case I_BehaviourFrisbeeEnergyBolt:
			FrisbeeEnergyBoltBehaviour(sbptr); 
			break;

		case I_BehaviourXenoborgEnergyBolt:
			XenoborgEnergyBoltBehaviour(sbptr); 
			break;
		
		case I_BehaviourAlienSpit:
			AlienSpitBehaviour(sbptr); 
			break;

		case I_BehaviourNPCPredatorDisc:
			NPCDiscBehaviour(sbptr);
			break;
		
		case I_BehaviourPredatorDisc_SeekTrack:
			DiscBehaviour_SeekTrack(sbptr);
			break;
		
		#if 0
		/* KJL 17:07:53 02/24/97 - I've turned these off for now since they
		always seem to cause crashes. */
		case I_BehaviourFlameProjectile:
			FlameProjectileFunction(sbptr);
			break;
		#endif
		
		case I_BehaviourDatabase:
			/* KJL 16:30:21 03/13/97 - no behaviour required */
			break;
		
 		case I_BehaviourNetGhost:
			NetGhostBehaviour(sbptr);
			break;

		case I_BehaviourSmokeGenerator:
			SmokeGeneratorBehaviour(sbptr);
			break;

		case I_BehaviourXenoborgMorphRoom:
			XenoMorphRoomBehaviour (sbptr);
			break;
    
		case I_BehaviourLightFX:
			LightFXBehaveFun (sbptr);
			break;

		case I_BehaviourPlacedSound:
		{
			SoundBehaveFun (sbptr);
			break;
		}
		case I_BehaviourTrackObject:
		{
			TrackObjectBehaveFun (sbptr);
			break;
		}
		case I_BehaviourFan:
		{
			FanBehaveFun (sbptr);
			break;
		}
		case I_BehaviourNetCorpse:
		{
			CorpseBehaveFun(sbptr);
			break;
		}
		case I_BehaviourRubberDuck:
		{	
			RubberDuckBehaviour(sbptr);
			break;
		}
		case I_BehaviourPlacedHierarchy :
		{
			PlacedHierarchyBehaveFun(sbptr);
			break;
		}
				
		case I_BehaviourPlacedLight:
			PlacedLightBehaviour(sbptr);
			break;

		case I_BehaviourVideoScreen:
			VideoScreenBehaviour(sbptr);
			break;
	
		case I_BehaviourPowerCable:
			PowerCableBehaveFun(sbptr);
			break;

		case I_BehaviourDeathVolume:
			DeathVolumeBehaveFun(sbptr);
			break;

		case I_BehaviourSelfDestruct:
			SelfDestructBehaveFun(sbptr);
			break;

		case I_BehaviourGrapplingHook:
			GrapplingHookBehaviour(sbptr);
			break;

		case I_BehaviourDummy:
			DummyBehaviour(sbptr);
			break;

		case I_BehaviourParticleGenerator:
			ParticleGeneratorBehaveFun(sbptr);
			break;
    default:
			break;
	}
}





/********************** OTHER FUNCTIONS **********************/





/************************** DOOR FUNCTIONS *************************/




/*------------Patrick 6/1/97--------------------
  I have made some significant modifications to
  the proximity door behaviour function, to
  facilitate opening and closing by 'far'aliens
  ----------------------------------------------*/

void UnlockThisProxdoor(STRATEGYBLOCK* sbptr)
{
	PROXDOOR_BEHAV_BLOCK *doorbhv;
	MORPHCTRL *mctrl;
	DISPLAYBLOCK* dptr;
	MODULE *mptr;

 	GLOBALASSERT(sbptr);
	doorbhv = (PROXDOOR_BEHAV_BLOCK*)sbptr->SBdataptr;
	GLOBALASSERT((doorbhv->bhvr_type == I_BehaviourProximityDoor));
	mctrl = doorbhv->PDmctrl;
	GLOBALASSERT(mctrl);
	mptr = sbptr->SBmoptr;
	GLOBALASSERT(mptr);
	dptr = sbptr->SBdptr;

	/* Unlock door! */
	doorbhv->door_locked = 0;

}

static void DoorProxBehaveFun(STRATEGYBLOCK* sbptr)
{
	PROXDOOR_BEHAV_BLOCK *doorbhv;
	MORPHCTRL *mctrl;
	DISPLAYBLOCK* dptr;
	MODULE *mptr;
	BOOL open_door = No;

 	GLOBALASSERT(sbptr);
	doorbhv = (PROXDOOR_BEHAV_BLOCK*)sbptr->SBdataptr;
	GLOBALASSERT((doorbhv->bhvr_type == I_BehaviourProximityDoor));
	mctrl = doorbhv->PDmctrl;
	GLOBALASSERT(mctrl);
	mptr = sbptr->SBmoptr;
	GLOBALASSERT(mptr);
	dptr = sbptr->SBdptr;
	
	
	/* update morphing.... */
	UpdateMorphing(mctrl);

	/*-----------------Patrick 29/4/97-------------------------
	 Little patch for networking: doors should run the
	 AnythingNearProxDoor() test even when they are not visible
	 ----------------------------------------------------------*/ 
	 
	if(AvP.Network==I_No_Network)
	{
		/* if door is visible check for proximity of player, aliens, and whatever... */
		if(dptr) open_door = AnythingNearProxDoor(mptr,doorbhv);
	}
	else open_door = AnythingNearProxDoor(mptr,doorbhv);
	


 	/* check for an alien trigger:
 	NB Door could be either visible or not when triggered */
 	if ((doorbhv->alienTrigger == 1)||((doorbhv->marineTrigger == 1)))
	{
		open_door = Yes;
		if (doorbhv->marineTrigger==1) {
			doorbhv->triggeredByMarine = 1;
		}
		doorbhv->alienTimer = DOOR_FAROPENTIME;
		doorbhv->alienTrigger = 0;
		doorbhv->marineTrigger = 0;
	}

	if (doorbhv->door_locked)
	{
		open_door = No;
	}

 	switch(doorbhv->door_state)
	{
		case I_door_opening:
		{	
			mptr->m_flags |= m_flag_open;
			if(mctrl->ObMorphFlags & mph_flag_finished)		
			{
		        if (doorbhv->SoundHandle!=SOUND_NOACTIVEINDEX)
		        {
		          if (doorbhv->triggeredByMarine) {
			          Sound_Play(SID_DOOREND,"dpm",&mptr->m_world,doorbhv->doorType);
				  } else {
			          Sound_Play(SID_DOOREND,"dp",&mptr->m_world,doorbhv->doorType);
				  }
		          Sound_Stop(doorbhv->SoundHandle);
		        }

				doorbhv->door_state = I_door_open;
			}
			break;
		}

		case I_door_closing:
		{
			if(open_door)
			{				
      								
				if(dptr) 
				{
					OpenDoor(mctrl, doorbhv->door_opening_speed);
					if (doorbhv->SoundHandle==SOUND_NOACTIVEINDEX)
					{
		  		         if (doorbhv->triggeredByMarine) {
		 	 				Sound_Play(SID_DOORSTART,"dpm",&mptr->m_world,doorbhv->doorType);
					 		Sound_Play(SID_DOORMID,"delpm",&mptr->m_world,&doorbhv->SoundHandle,doorbhv->doorType);
						 } else {
		 	 				Sound_Play(SID_DOORSTART,"dp",&mptr->m_world,doorbhv->doorType);
					 		Sound_Play(SID_DOORMID,"delp",&mptr->m_world,&doorbhv->SoundHandle,doorbhv->doorType);
						 }
				 	}
		    }	
				else 
				{
				  OpenDoor(mctrl, DOOR_OPENFASTSPEED);
				}

				doorbhv->door_state = I_door_opening;
			}
			else if(mctrl->ObMorphFlags & mph_flag_finished)
			{
		        if (doorbhv->SoundHandle!=SOUND_NOACTIVEINDEX)
		        {
	  		         if (doorbhv->triggeredByMarine) {
  				 		Sound_Play(SID_DOOREND,"dpm",&mptr->m_world,doorbhv->doorType);
					 } else {
  				 		Sound_Play(SID_DOOREND,"dp",&mptr->m_world,doorbhv->doorType);
					 }
		       		Sound_Stop(doorbhv->SoundHandle);
		        }
        
				doorbhv->door_state = I_door_closed;
				mptr->m_flags &= ~m_flag_open;
			}
				
			break;
		}

		case I_door_open:
		{
			mptr->m_flags |= m_flag_open;
			if(!open_door)
			{
          
 				if(dptr) 
				{
  		         if (doorbhv->triggeredByMarine) {
			 		Sound_Play(SID_DOORSTART,"dpm",&mptr->m_world,doorbhv->doorType);
			 		Sound_Play(SID_DOORMID,"delpm",&mptr->m_world,&doorbhv->SoundHandle,doorbhv->doorType);
				 } else {
			 		Sound_Play(SID_DOORSTART,"dp",&mptr->m_world,doorbhv->doorType);
			 		Sound_Play(SID_DOORMID,"delp",&mptr->m_world,&doorbhv->SoundHandle,doorbhv->doorType);
				 }			 			  				 							
					doorbhv->alienTimer = 0;
					CloseDoor(mctrl,doorbhv->door_closing_speed);
					doorbhv->door_state = I_door_closing;
				}
				else 
				{
					doorbhv->alienTimer -= NormalFrameTime;
					if(!(doorbhv->alienTimer>0))
					{
						doorbhv->alienTimer = 0;
						CloseDoor(mctrl, DOOR_CLOSEFASTSPEED);
						doorbhv->door_state = I_door_closing;
					}
				}
			}
				
			break;
		}
			
		case I_door_closed:
		{
			mptr->m_flags &= ~m_flag_open;
			if(open_door)
			{
		 		if(dptr) 
				{
	  		         if (doorbhv->triggeredByMarine) {
						Sound_Play(SID_DOORSTART,"dpm",&mptr->m_world,doorbhv->doorType);
				 		Sound_Play(SID_DOORMID,"delpm",&mptr->m_world,&doorbhv->SoundHandle,doorbhv->doorType);
				 	 } else {
						Sound_Play(SID_DOORSTART,"dp",&mptr->m_world,doorbhv->doorType);
				 		Sound_Play(SID_DOORMID,"delp",&mptr->m_world,&doorbhv->SoundHandle,doorbhv->doorType);
					 }
					OpenDoor(mctrl,doorbhv->door_opening_speed);
				}
				else OpenDoor(mctrl, DOOR_OPENFASTSPEED);
				doorbhv->door_state = I_door_opening;
				mptr->m_flags |= m_flag_open;
			} else {
				doorbhv->triggeredByMarine = 0;
			}
			
			break;
		}
		
		default:
				LOCALASSERT(1==0);
	}
}


void OpenDoor(MORPHCTRL *mctrl, int speed)
{
	LOCALASSERT(mctrl);

	mctrl->ObMorphFlags = mph_flag_play;
	mctrl->ObMorphFlags |= mph_flag_noloop;
	mctrl->ObMorphFlags |= mph_flag_reverse;
	mctrl->ObMorphSpeed = speed;
}


void CloseDoor(MORPHCTRL *mctrl, int speed)
{
	LOCALASSERT(mctrl);
	
	mctrl->ObMorphFlags = mph_flag_play;
	mctrl->ObMorphFlags |= mph_flag_noloop;
	mctrl->ObMorphFlags &= ~mph_flag_reverse;
	mctrl->ObMorphSpeed =  speed;
}

/*---------------Patrick 6/1/97-------------------
  Sorry, but I can't think of anything better than 
  searching the	entire sb list for any object that
  might trigger a door....
  ------------------------------------------------*/
static int AnythingNearProxDoor(MODULE *doorModulePtr,PROXDOOR_BEHAV_BLOCK *doorbhv)
{	
	/* KJL 18:06:59 19/02/98 - I've rewritten this check so that it can cope with very tall doors. */	
	extern int NumActiveStBlocks;
	extern STRATEGYBLOCK *ActiveStBlockList[];	

	int sbIndex = 0;
	STRATEGYBLOCK *sbPtr;
	DYNAMICSBLOCK *dynPtr;
	DISPLAYBLOCK *dPtr;
	int retval;

	int maxY;
	int minY;
	int maxX;
	int minX;
	int maxZ;
	int minZ;

	if (doorModulePtr->m_flags & MODULEFLAG_HORIZONTALDOOR)
	{
		maxY = doorModulePtr->m_world.vy + DOOR_OPENDISTANCE/2;
		minY = doorModulePtr->m_world.vy - DOOR_OPENDISTANCE/2;
		maxX = doorModulePtr->m_world.vx + doorModulePtr->m_maxx + DOOR_OPENDISTANCE/2;
		minX = doorModulePtr->m_world.vx + doorModulePtr->m_minx - DOOR_OPENDISTANCE/2;
		maxZ = doorModulePtr->m_world.vz + doorModulePtr->m_maxz + DOOR_OPENDISTANCE/2;
		minZ = doorModulePtr->m_world.vz + doorModulePtr->m_minz - DOOR_OPENDISTANCE/2;
	}
	else
	{
		maxY = doorModulePtr->m_world.vy + doorModulePtr->m_maxy;
		minY = doorModulePtr->m_world.vy + doorModulePtr->m_miny;
		maxX = doorModulePtr->m_world.vx + DOOR_OPENDISTANCE;
		minX = doorModulePtr->m_world.vx - DOOR_OPENDISTANCE;
		maxZ = doorModulePtr->m_world.vz + DOOR_OPENDISTANCE;
		minZ = doorModulePtr->m_world.vz - DOOR_OPENDISTANCE;
	}
	/* loop thro' the strategy block list... 
	looking for triggerable objects (please feel free to add any
	sensible object types) */
	retval=No;

	while(sbIndex < NumActiveStBlocks)
	{	
		sbPtr = ActiveStBlockList[sbIndex++];
		dynPtr = sbPtr->DynPtr;
		dPtr = sbPtr->SBdptr;

		if(	(dynPtr)&&(dPtr)&&
			((sbPtr->I_SBtype == I_BehaviourAlien)||
			(sbPtr->I_SBtype == I_BehaviourMarinePlayer)||
			(sbPtr->I_SBtype == I_BehaviourPredatorPlayer)||
			(sbPtr->I_SBtype == I_BehaviourAlienPlayer)||
			(sbPtr->I_SBtype == I_BehaviourPredator)||
			(sbPtr->I_SBtype ==	I_BehaviourXenoborg)||
			(sbPtr->I_SBtype ==	I_BehaviourMarine)||
			(sbPtr->I_SBtype ==	I_BehaviourSeal)||
			(sbPtr->I_SBtype ==	I_BehaviourQueenAlien)||
			(sbPtr->I_SBtype ==	I_BehaviourPredatorAlien)||
			(sbPtr->I_SBtype ==	I_BehaviourFaceHugger)||
			(sbPtr->I_SBtype == I_BehaviourGrenade)||
			(sbPtr->I_SBtype == I_BehaviourRocket)||
			(sbPtr->I_SBtype == I_BehaviourFrisbee)||
			(sbPtr->I_SBtype == I_BehaviourPulseGrenade)||
			(sbPtr->I_SBtype == I_BehaviourProximityGrenade)||
			(sbPtr->I_SBtype == I_BehaviourNetGhost)||
//			(sbPtr->I_SBtype == I_BehaviourFlareGrenade)||
			(sbPtr->I_SBtype == I_BehaviourFragmentationGrenade)||
			(sbPtr->I_SBtype == I_BehaviourPredatorDisc_SeekTrack)||		
			(sbPtr->I_SBtype == I_BehaviourNetCorpse)||
			(sbPtr->I_SBtype == I_BehaviourHierarchicalFragment)||
			(sbPtr->I_SBtype == I_BehaviourNPCPredatorDisc))		
		  )		
		{
			if (dynPtr->Position.vy > minY && dynPtr->Position.vy < maxY)
			{
				if (dynPtr->Position.vx > minX && dynPtr->Position.vx < maxX)
				{
					if (dynPtr->Position.vz > minZ && dynPtr->Position.vz < maxZ)
					{
						if ((sbPtr->I_SBtype==I_BehaviourMarine)
							||(sbPtr->I_SBtype==I_BehaviourSeal)) {
							doorbhv->triggeredByMarine = 1;
						}
						retval=Yes;	
					}
				}
			}
		}		
	}
	
	return(retval);
	
}



static void SimpleAnimBehaveFun(STRATEGYBLOCK* sbptr)
{
	/* this just copie the SB taccontrol into the
	   display bloxk if it didnt already have one*/

	SIMPLE_ANIM_BEHAV_BLOCK *sanimbhv;
  	DISPLAYBLOCK* dptr;

	sanimbhv = (SIMPLE_ANIM_BEHAV_BLOCK*)(sbptr->SBdataptr);

	GLOBALASSERT(sanimbhv->bhvr_type == I_BehaviourSimpleAnimation);

	dptr = sbptr->SBdptr;

	if(dptr)
	{
		if(!dptr->ObTxAnimCtrlBlks)
			{ 
				dptr->ObTxAnimCtrlBlks = sanimbhv->tacbSimple;
			}
	}
}


/********************* SUPPORT  FUNCTION ***********************/
/*
	these set the reqest state in a strategy block 
*/

void RequestState(STRATEGYBLOCK* sbptr, int message, STRATEGYBLOCK * SBRequester)
{
	//lowest bit of message corresponds to old on/off state
	//the rest is extended information ,the interpretation of which depends on the receiving strategy
	BOOL state=message & 1;	
	if(!sbptr)
	{
		return;
	}
	//GLOBALASSERT(sbptr);

	if(sbptr->SBflags.destroyed_but_preserved)
	{
		//target doesn't exist anymore so ignore request.
		return;	
	}

	#if DB_LEVEL >=3
	{
		//add details of request to logfile
		extern int GlobalFrameCounter;
		const char* name1=0;
		const char* name2="ANON";
		if(sbptr) name1=sbptr->name;
		if(SBRequester) name2=SBRequester->name;
		db_logf3(("Frame %d  : %s sent %s message to %s%s",GlobalFrameCounter,name2, state ? "'on'" : "'off'",name1,(message>>1)? " with extra message data" : ""));
	}
	#endif


	switch (sbptr->I_SBtype)
	  {
			case I_BehaviourNetCorpse:
				/* Ulp... */
				break;
			case I_BehaviourBinarySwitch:
				{
					// 0 = rest state
					// 1 = unusual state

					BINARY_SWITCH_BEHAV_BLOCK *bs_bhv;
					bs_bhv = (BINARY_SWITCH_BEHAV_BLOCK*)sbptr->SBdataptr;
					GLOBALASSERT((bs_bhv->bhvr_type == I_BehaviourBinarySwitch));

					if(state)
						bs_bhv->request = I_request_on;
					else
						bs_bhv->request = I_request_off;
										
					break;
				}

			case I_BehaviourLinkSwitch:
				{
					LINK_SWITCH_BEHAV_BLOCK *bs_bhv;

					// 0 = rest state
					// 1 = unusual state
					
					bs_bhv = (LINK_SWITCH_BEHAV_BLOCK*)sbptr->SBdataptr;
					GLOBALASSERT((bs_bhv->bhvr_type == I_BehaviourLinkSwitch));

					if (SBRequester) if (SBRequester->I_SBtype == I_BehaviourBinarySwitch || 
															SBRequester->I_SBtype == I_BehaviourLinkSwitch)
					{
						//if the request has come from one of the linked switches , ignore it
						int i;
						for(i=0;i<bs_bhv->num_linked_switches;i++)
						{
							if(bs_bhv->lswitch_list[i].bswitch==SBRequester)
							{
								break;
							}
						}
						if(i<bs_bhv->num_linked_switches)
						{
							break;
						}
					}


					if(state)
						bs_bhv->request = I_request_on;
					else
						bs_bhv->request = I_request_off;
										
					break;
				}

			case I_BehaviourProximityDoor:
				{
					PROXDOOR_BEHAV_BLOCK *door_bhv;
					MORPHCTRL *mctrl;
				
					door_bhv = (PROXDOOR_BEHAV_BLOCK*)sbptr->SBdataptr;
					GLOBALASSERT((door_bhv->bhvr_type == I_BehaviourProximityDoor));
					mctrl = door_bhv->PDmctrl;
					GLOBALASSERT(mctrl);

					//request previously open and closed door.Now it locks or unlocks it.
					if(state == 1)
						{
							door_bhv->door_locked = 0;
						}
					else
						{
							door_bhv->door_locked = 1;
						}
					break;
				}	

			case I_BehaviourLiftDoor:
				{
					LIFT_DOOR_BEHAV_BLOCK *door_bhv;
					door_bhv = (LIFT_DOOR_BEHAV_BLOCK*)sbptr->SBdataptr;
					GLOBALASSERT((door_bhv->bhvr_type == I_BehaviourLiftDoor));
		
					if(state == 1)
						door_bhv->request_state = I_door_open;
					else
						door_bhv->request_state = I_door_closed;					

					break;
				}

			case I_BehaviourSwitchDoor:
			{
				SWITCH_DOOR_BEHAV_BLOCK *door_bhv;
				door_bhv = (SWITCH_DOOR_BEHAV_BLOCK*)sbptr->SBdataptr;
				GLOBALASSERT((door_bhv->myBehaviourType == I_BehaviourSwitchDoor));
		
				if(state == 1) door_bhv->requestOpen = 1;
				else door_bhv->requestClose = 1;
				break;
			}

			case I_BehaviourLift:
				{
					LIFT_BEHAV_BLOCK *lift_bhv;	
					LIFT_STATION *lift_stn;
			 		GLOBALASSERT(sbptr);
					lift_bhv = (LIFT_BEHAV_BLOCK*)sbptr->SBdataptr;
					GLOBALASSERT((lift_bhv->bhvr_type == I_BehaviourLift));
					lift_stn = &lift_bhv->lift_station;
					GLOBALASSERT(lift_stn);

					if(state == 1)
						lift_stn->called = 1;
					else
						lift_stn->called = 0;					

					break;
				}

			case I_BehaviourPlatform:
			{
							
				((PLATFORMLIFT_BEHAVIOUR_BLOCK*)sbptr->SBdataptr)->Enabled = state;
				
				break;
			}

			case I_BehaviourAutoGun:
			{
				AUTOGUN_STATUS_BLOCK *agunStatusPointer;
				agunStatusPointer = (AUTOGUN_STATUS_BLOCK*)sbptr->SBdataptr;
				
				if ((agunStatusPointer->behaviourState==I_inactive)&&(state==1)) {
					agunStatusPointer->behaviourState=I_tracking;
				} else if ((agunStatusPointer->behaviourState==I_tracking)&&(state==0)) {
					agunStatusPointer->behaviourState=I_inactive;
					Sound_Play(SID_SENTRYGUN_SHUTDOWN,"d",&sbptr->DynPtr->Position);
				}

				break;
			}
			
			case I_BehaviourGenerator:
			{
				((GENERATOR_BLOCK*)sbptr->SBdataptr)->Active = state;
				break;
			}
			
			case I_BehaviourLightFX:
			{
				LIGHT_FX_BEHAV_BLOCK * lfxbb = (LIGHT_FX_BEHAV_BLOCK *)sbptr->SBdataptr;
				
				if (lfxbb->type == LFX_Switch)
				{
					switch (lfxbb->current_state)
					{
						
						case LFXS_LightOn:
						{
							if(state==0)
							{
								lfxbb->current_state = LFXS_LightFadingDown;
								lfxbb->timer = 0;
							}
							break;
						}
						
						case LFXS_LightOff:
						{
							if(state==1)
							{
								lfxbb->current_state = LFXS_LightFadingUp;
								lfxbb->timer = 0;
							}
							break;
						}
						
						case LFXS_LightFadingUp:
						{
							if(state==0)
							{
								lfxbb->timer = ONE_FIXED - lfxbb->timer;
								lfxbb->current_state = LFXS_LightFadingDown;
							}
							break;
						}
						
						case LFXS_LightFadingDown:
						{
							if(state==1)
							{
								lfxbb->timer = ONE_FIXED - lfxbb->timer;
								lfxbb->current_state = LFXS_LightFadingUp;
							}
							break;
						}
						
						default:
						{
							GLOBALASSERT (0 == "Light FX state not supported");
							break;
						}
					}
				}
				else if (lfxbb->type == LFX_FlickySwitch)
				{
					switch (lfxbb->current_state)
					{
						
						case LFXS_LightOn:
						{
							if(state==0)
							{
								lfxbb->current_state = LFXS_LightFadingDown;
								lfxbb->timer = 0;
							}
							break;
						}
						
						case LFXS_LightOff:
						{
							if(state==1)
							{
								lfxbb->current_state = LFXS_LightFadingUp;
								lfxbb->timer = 0;
							}
							break;
						}
						
						case LFXS_LightFadingUp:
						{
							if(state==0)
							{
								lfxbb->multiplier = lfxbb->timer;
								lfxbb->timer = ONE_FIXED - lfxbb->timer;
								lfxbb->current_state = LFXS_LightFadingDown;
							}
							break;
						}
						
						case LFXS_LightFadingDown:
						{
							if(state==1)
							{
								lfxbb->timer = ONE_FIXED - lfxbb->timer;
								lfxbb->current_state = LFXS_LightFadingUp;
							}
							break;
						}
						
						default:
						{
							GLOBALASSERT (0 == "Light FX state not supported");
							break;
						}
					}
				}
				else if(lfxbb->type==LFX_RandomFlicker)
				{
					switch (lfxbb->current_state)
					{
						case LFXS_Flicking:
						case LFXS_NotFlicking:
							if(state==0)
							{
								//switch light off
								lfxbb->current_state = LFXS_LightOff;
							}
							break;
							
						case LFXS_LightOff :
							if(state==1)
							{
								//switch light on
								lfxbb->current_state = LFXS_Flicking;
							}
							break;
						default: ;
					}
				}
				
				
				break;
			}
			case I_BehaviourMissionComplete:
				{
					SendRequestToMissionStrategy(sbptr,state,message>>1);
						
					break;
				}
			case I_BehaviourMessage:
				{
					SendRequestToMessageStrategy(sbptr,state,message>>1);
									
					break;
				}
			
			case I_BehaviourTrackObject:
				{

					TRACK_OBJECT_BEHAV_BLOCK *to_bhv;
					to_bhv = (TRACK_OBJECT_BEHAV_BLOCK*)sbptr->SBdataptr;
					GLOBALASSERT((to_bhv->bhvr_type == I_BehaviourTrackObject));

					if(state)
					{
						switch(message>>1)
						{
							case 0:
								to_bhv->request = track_request_start;
								break;
							case 1:
								to_bhv->request = track_request_startforward;
								break;
							case 2:
								to_bhv->request = track_request_startbackward;
								break;
						}
					}
					else
						to_bhv->request = track_request_stop;
										
					break;
				}

			case I_BehaviourFan:
				{

					FAN_BEHAV_BLOCK *f_bhv;
					f_bhv = (FAN_BEHAV_BLOCK*)sbptr->SBdataptr;
					GLOBALASSERT((f_bhv->bhvr_type == I_BehaviourFan));

					if(state)
						f_bhv->state = fan_state_go;
					else
						f_bhv->state = fan_state_stop;
										
					break;
				}

			case I_BehaviourPlacedSound:
				{
					if(state)
						StartPlacedSoundPlaying(sbptr);
					else
						StopPlacedSoundPlaying(sbptr);
					break;
				}

			case I_BehaviourInanimateObject:
				{
					SendRequestToInanimateObject(sbptr,state,message>>1);
					break;
				}

			case I_BehaviourPlacedHierarchy :
				{
					SendRequestToPlacedHierarchy(sbptr,state,message>>1);
				}
				break;

			case I_BehaviourPlacedLight:
				{
					SendRequestToPlacedLight(sbptr,state,message>>1);
						
					break;
				}
			case I_BehaviourAlien:
				{
					ALIEN_STATUS_BLOCK *alienStatusPointer;
					alienStatusPointer = (ALIEN_STATUS_BLOCK*)sbptr->SBdataptr;

					if (alienStatusPointer->BehaviourState==ABS_Dormant) {
						Alien_Awaken(sbptr);
					}
					break;
				}
			case I_BehaviourFaceHugger:
				{
					FACEHUGGER_STATUS_BLOCK *facehuggerStatusPointer;    

					facehuggerStatusPointer = (FACEHUGGER_STATUS_BLOCK *)(sbptr->SBdataptr);    
					LOCALASSERT(facehuggerStatusPointer);

					if (facehuggerStatusPointer->nearBehaviourState==FHNS_Floating) {
						Wake_Hugger(sbptr);
					}
					break;
				}
			case I_BehaviourXenoborg:
				{
					XENO_STATUS_BLOCK *xenoStatusPointer;
					xenoStatusPointer = (XENO_STATUS_BLOCK*)sbptr->SBdataptr;

					if (state) {
						if (xenoStatusPointer->behaviourState==XS_Inactive) {
							Xeno_Enter_PowerUp_State(sbptr);
						}
					} else {
						if (xenoStatusPointer->behaviourState!=XS_Inactive) {
							Xeno_Enter_PowerDown_State(sbptr);
						}
					}
					break;
				}

			case I_BehaviourDormantPredator :
				{
					if(state)
					{
						ActivateDormantPredator(sbptr);
					}
					break;
				}
			case I_BehaviourPredator :
				//do nothing
				//can't assert for this one since it might previously have been a dormant predator
				break;

			case I_BehaviourMarine :
				{
					SendRequestToMarine(sbptr,state,message>>1);
				}
				break;
			
			case I_BehaviourDeathVolume:
				{

					DEATH_VOLUME_BEHAV_BLOCK *dv_bhv;
					dv_bhv = (DEATH_VOLUME_BEHAV_BLOCK*)sbptr->SBdataptr;
					GLOBALASSERT((dv_bhv->bhvr_type == I_BehaviourDeathVolume));

					dv_bhv->active=state;
										
					break;
				}
			case I_BehaviourSelfDestruct:
				{

					SELF_DESTRUCT_BEHAV_BLOCK *sd_bhv;
					sd_bhv = (SELF_DESTRUCT_BEHAV_BLOCK*)sbptr->SBdataptr;
					GLOBALASSERT((sd_bhv->bhvr_type == I_BehaviourSelfDestruct));

					sd_bhv->active=state;
										
					break;
				}
			case I_BehaviourParticleGenerator:
				{
					SendRequestToParticleGenerator(sbptr,state,message>>1);
						
					break;
				}
			default:
			{
				GLOBALASSERT(2<1);
			}
	  }
}


BOOL GetState(STRATEGYBLOCK* sbptr)
{
	GLOBALASSERT(sbptr);
	
	switch (sbptr->I_SBtype)
	  {
			case I_BehaviourBinarySwitch:
				{
					// 0 = rest state
					// 1 = unusual state

					BINARY_SWITCH_BEHAV_BLOCK *bs_bhv;
					bs_bhv = (BINARY_SWITCH_BEHAV_BLOCK*)sbptr->SBdataptr;
					GLOBALASSERT((bs_bhv->bhvr_type == I_BehaviourBinarySwitch));

					return((BOOL)bs_bhv->state);
										
					break;
				}
			case I_BehaviourLinkSwitch:
				{
					// 0 = rest state
					// 1 = unusual state

					LINK_SWITCH_BEHAV_BLOCK *bs_bhv;
					bs_bhv = (LINK_SWITCH_BEHAV_BLOCK*)sbptr->SBdataptr;
					GLOBALASSERT((bs_bhv->bhvr_type == I_BehaviourLinkSwitch));

					return((BOOL)bs_bhv->state);
										
					break;
				}
			case I_BehaviourProximityDoor:
				{
					PROXDOOR_BEHAV_BLOCK *door_bhv;
					door_bhv = (PROXDOOR_BEHAV_BLOCK*)sbptr->SBdataptr;
					GLOBALASSERT((door_bhv->bhvr_type == I_BehaviourProximityDoor));

					// returns true only when fully open
					
					if(door_bhv->door_state == I_door_open)
						return((BOOL)1);
					else
						return((BOOL)0);
						
					break;
				}
			case I_BehaviourSwitchDoor:
				{
					/* returns true if fully open (DO NOT CHANGE THIS) */
					SWITCH_DOOR_BEHAV_BLOCK *door_bhv;
					door_bhv = (SWITCH_DOOR_BEHAV_BLOCK*)sbptr->SBdataptr;
					GLOBALASSERT(door_bhv->myBehaviourType == I_BehaviourSwitchDoor);
										
					if(door_bhv->doorState == I_door_open) return((BOOL)1);
					else return((BOOL)0);	
					break;
				}
			case I_BehaviourLiftDoor:
				{
					LIFT_DOOR_BEHAV_BLOCK *door_bhv;
					door_bhv = (LIFT_DOOR_BEHAV_BLOCK*)sbptr->SBdataptr;
					GLOBALASSERT((door_bhv->bhvr_type == I_BehaviourLiftDoor));

					// returns true only when fully open so that the lift
					// dosn't start to move when we can still see out
				
					if(door_bhv->door_state == I_door_closed)
						return((BOOL)0);
					else
						return((BOOL)1);
						
					break;
				}
			case I_BehaviourLift:
				{
					LIFT_BEHAV_BLOCK *lift_bhv;	
					LIFT_STATION *lift_stn;
			 		GLOBALASSERT(sbptr);
					lift_bhv = (LIFT_BEHAV_BLOCK*)sbptr->SBdataptr;
					GLOBALASSERT((lift_bhv->bhvr_type == I_BehaviourLift));
					lift_stn = &lift_bhv->lift_station;
					GLOBALASSERT(lift_stn);

					return(lift_stn->called);

					break;
				}
			default:
				GLOBALASSERT(2<1);
	  }

	return((BOOL)0);
}




DISPLAYBLOCK *MakeObject(AVP_BEHAVIOUR_TYPE bhvr, VECTORCH *positionPtr) 
{
  // This function creates the specified object, fully
  // initialized, WITHOUT a modulemapblock into which
  // we can collapse it again if required, and a valid
  // strategyblock,

	extern int NumActiveBlocks;

  DISPLAYBLOCK *dptr;
  STRATEGYBLOCK *sptr;
  MODULEMAPBLOCK *mmbptr;
  MODULE m_temp;
  //TXACTRLBLK *taPtr;
	
  if (NumActiveBlocks >= maxobjects) return (DISPLAYBLOCK *)NULL;
  if (NumActiveStBlocks >= maxstblocks) return (DISPLAYBLOCK *)NULL;

  // 1. Set up shape data BEFORE making the displayblock,
  // since "AllocateModuleObject()" will fill in shapeheader
  // information and extent data

  mmbptr = &TempModuleMap;
                
  switch (bhvr)
  {
    case I_BehaviourAutoGunMuzzleFlash:
		{
			CreateShapeInstance(mmbptr,"Sntrymuz");
			break;
		}
	
		case I_BehaviourRocket:
		{
			CreateShapeInstance(mmbptr,"missile");
		    break;
		}				
						
		case I_BehaviourFrisbee:
		case I_BehaviourNPCPredatorDisc:
		case I_BehaviourPredatorDisc_SeekTrack:
		{
			//CreateShapeInstance(mmbptr,"disk@hnpcpredator");
			CreateShapeInstance(mmbptr,"Shell");
		    break;
		}				

		case I_BehaviourPulseGrenade: /* need a new shape */
		case I_BehaviourGrenade:
		{
			CreateShapeInstance(mmbptr,"Shell");
			break;
		}
		case I_BehaviourFragmentationGrenade:
		{
			CreateShapeInstance(mmbptr,"Frag");
			break;
		}
		case I_BehaviourClusterGrenade:
		{
			CreateShapeInstance(mmbptr,"Cluster");
			break;
		}
		case I_BehaviourProximityGrenade:
		{
			CreateShapeInstance(mmbptr,"Proxmine");
			break;
		}		
		
    			
		case I_BehaviourFlareGrenade:
		{
			CreateShapeInstance(mmbptr,"Flare");
			break;
		}

		case I_BehaviourSpeargunBolt:
		{
			CreateShapeInstance(mmbptr,"spear");
			break;
		}				
		case I_BehaviourPPPlasmaBolt:		
		case I_BehaviourFrisbeeEnergyBolt:
		case I_BehaviourPredatorEnergyBolt:
		case I_BehaviourXenoborgEnergyBolt:
		{
			//uses special effect instead of a shape
    		mmbptr->MapShape = 0;
			mmbptr->MapType = MapType_Default;
//			CreateShapeInstance(mmbptr,"Plasbolt");
			CreateShapeInstance(mmbptr,"Shell");

			break;
		}				
		case I_BehaviourAlienSpit:
		{
			CreateShapeInstance(mmbptr,"Spit");
			break;
		}				

    case I_BehaviourTest:
    default:
		{
      // Don't call this function for undefined types
      GLOBALASSERT (1 == 0);
			break;
    }            
	}

  // And allocate the modulemapblock object

	m_temp.m_numlights = 0;
	m_temp.m_lightarray = NULL;
  m_temp.m_mapptr = mmbptr;
  m_temp.m_sbptr = (STRATEGYBLOCK*)NULL;
  AllocateModuleObject(&m_temp); 
   
  dptr = m_temp.m_dptr;

	if (dptr == 0) return(DISPLAYBLOCK*)NULL;

  dptr->ObMyModule = NULL;     /* Module that created us */
	dptr->ObWorld = *positionPtr;

 	sptr = AttachNewStratBlock((MODULE*)NULL, mmbptr, dptr);
   
  if (sptr == 0) return(DISPLAYBLOCK*)NULL;
  
  // 2. NOW set up the strategyblock-specific fields for
  // the new displayblock. We won't go through the "AttachNew
  // StrategyBlock" and "AssignRunTimeBehaviours" pair, since
  // the first switches on ObShape and the second on bhvr;
  // but, in this case, there isn't a particular connection
  // between them.

  sptr->I_SBtype = bhvr;
	    
  switch (bhvr)
  {
    case I_BehaviourAutoGunMuzzleFlash:
    // but soon it will be an animated sequence
		{
			sptr->SBdataptr = (ONE_SHOT_BEHAV_BLOCK *) AllocateMem(sizeof(ONE_SHOT_BEHAV_BLOCK ));

   		if (sptr->SBdataptr == 0) 
			{	
				// Failed to allocate a strategy block data pointer
				RemoveBehaviourStrategy(sptr);
				return (DISPLAYBLOCK*)NULL;
			}
	
			((ONE_SHOT_BEHAV_BLOCK * ) sptr->SBdataptr)->counter = ONE_FIXED/10;

			/* KJL 17:23:22 11/26/96 - here's a little something to give a random orientation */
			{
				EULER orientation;
				orientation.EulerX = 0;
				orientation.EulerY = 0;
				orientation.EulerZ = FastRandom()&4095;

				CreateEulerMatrix(&orientation, &dptr->ObMat);
				TransposeMatrixCH(&dptr->ObMat);
			}

			AddLightingEffectToObject(dptr,LFX_MUZZLEFLASH);

			break;
		}
      			

		case I_BehaviourFrisbee:
		case I_BehaviourRocket:
		case I_BehaviourPulseGrenade:
		case I_BehaviourGrenade:
		case I_BehaviourFlareGrenade:
		case I_BehaviourFragmentationGrenade:
		case I_BehaviourClusterGrenade:
		case I_BehaviourProximityGrenade:
		case I_BehaviourSpeargunBolt:
		case I_BehaviourPPPlasmaBolt:		
		case I_BehaviourFrisbeeEnergyBolt:
		case I_BehaviourPredatorEnergyBolt:
		case I_BehaviourXenoborgEnergyBolt:
		case I_BehaviourAlienSpit:
		case I_BehaviourNPCPredatorDisc:
		case I_BehaviourPredatorDisc_SeekTrack:
		{
		  AssignNewSBName(sptr);
		  break;
		}

    case I_BehaviourTest:
      break;

    default:

      // Don't call this function for undefined
      // types

      GLOBALASSERT (1 == 0);
			break;
  }
  return dptr;
}


// this is seperate so that it can be called during env changes
void RemoveBehaviourStrategy(STRATEGYBLOCK* sbptr)
{
	GLOBALASSERT(sbptr);

	/* Andy 16/8/97  - Updated to cope with partially allocated strategy blocks 
										 We need to check that each allocation has been completed before
										  trying to deallocate */
	
	/* CDF 12/11/96 - Destroys ANY strategyblock. *
	* Please maintain...                         */

	// this is to check for the sb destroy flag
	// only destroy sbs if DoNotRemoveSB = 0
	// and the destroy flag is set
	
	// first check to see if we need the SB in the next env
	// if we do we need to preserve the contents

	if(SBNeededForNextEnv(sbptr))
	{
		//we take a copy and preserve everything except our
		//ActiveStBlockList copy

		DestroyActiveStrategyBlock(sbptr);
		return;
	}			

	// this notifies the game flow system that an object has been destroyed
	

	switch(sbptr->I_SBtype) 
	{
		case I_BehaviourProximityDoor:
		{
			{
				/* patrick 7/7/97: remove sound handle, if we have one*/
				PROXDOOR_BEHAV_BLOCK *doorbhv = (PROXDOOR_BEHAV_BLOCK *)(sbptr->SBdataptr);
				if(doorbhv && doorbhv->SoundHandle!=SOUND_NOACTIVEINDEX) Sound_Stop(doorbhv->SoundHandle);
			}
			// 	Deallocater for the doors hack
			if (sbptr->SBmorphctrl && 
					sbptr->SBmorphctrl->ObMorphHeader && 
					sbptr->SBmorphctrl->ObMorphHeader->mph_frames) 
				DeallocateMem( sbptr->SBmorphctrl->ObMorphHeader->mph_frames);
	
			if (sbptr->SBmorphctrl && 
					sbptr->SBmorphctrl->ObMorphHeader) 
				DeallocateMem( sbptr->SBmorphctrl->ObMorphHeader);

			if (sbptr->SBmorphctrl)
				DeallocateMem( sbptr->SBmorphctrl);
			
			break;
		}	
		case I_BehaviourLiftDoor:
		{
			// 	Deallocater for the doors hack
			{
				/* patrick 7/7/97: remove sound handle, if we have one*/
				LIFT_DOOR_BEHAV_BLOCK *doorbhv = (LIFT_DOOR_BEHAV_BLOCK *)(sbptr->SBdataptr);
				if(doorbhv && doorbhv->SoundHandle!=SOUND_NOACTIVEINDEX) Sound_Stop(doorbhv->SoundHandle);
			}							 

			if (sbptr->SBmorphctrl && 
					sbptr->SBmorphctrl->ObMorphHeader && 
					sbptr->SBmorphctrl->ObMorphHeader->mph_frames) 
				DeallocateMem( sbptr->SBmorphctrl->ObMorphHeader->mph_frames);
	
			if (sbptr->SBmorphctrl && 
					sbptr->SBmorphctrl->ObMorphHeader) 
				DeallocateMem( sbptr->SBmorphctrl->ObMorphHeader);

			if (sbptr->SBmorphctrl)
				DeallocateMem( sbptr->SBmorphctrl);

			break;
		}	
		case I_BehaviourSwitchDoor:
		{
			{
				/* patrick 7/7/97: remove sound handle, if we have one*/
				SWITCH_DOOR_BEHAV_BLOCK *doorbhv = (SWITCH_DOOR_BEHAV_BLOCK *)(sbptr->SBdataptr);
				if(doorbhv && doorbhv->SoundHandle!=SOUND_NOACTIVEINDEX) Sound_Stop(doorbhv->SoundHandle);
			}
	
			if (sbptr->SBmorphctrl && 
					sbptr->SBmorphctrl->ObMorphHeader && 
					sbptr->SBmorphctrl->ObMorphHeader->mph_frames) 
				DeallocateMem( sbptr->SBmorphctrl->ObMorphHeader->mph_frames);
	
			if (sbptr->SBmorphctrl && 
					sbptr->SBmorphctrl->ObMorphHeader) 
				DeallocateMem( sbptr->SBmorphctrl->ObMorphHeader);

			if (sbptr->SBmorphctrl)
				DeallocateMem( sbptr->SBmorphctrl);

			break;
		}	
		case I_BehaviourSimpleAnimation:
		{

			SIMPLE_ANIM_BEHAV_BLOCK *sanimbhv;
			TXACTRLBLK *txactrl;
			TXACTRLBLK *txactrl_next;

			sanimbhv = (SIMPLE_ANIM_BEHAV_BLOCK*)(sbptr->SBdataptr);
			if (sanimbhv)
			{
				GLOBALASSERT(sanimbhv->bhvr_type == I_BehaviourSimpleAnimation);

				txactrl_next = sanimbhv->tacbSimple;
				while(txactrl_next)
				{
					txactrl = txactrl_next;
					txactrl_next = txactrl->tac_next;
					DeallocateMem((void*)txactrl);
				}
			}
			break;
		}	
		case I_BehaviourBinarySwitch:
		{
			BINARY_SWITCH_BEHAV_BLOCK *bs_bhv;
			TXACTRLBLK *txactrl;
			TXACTRLBLK *txactrl_next;

   		bs_bhv = (BINARY_SWITCH_BEHAV_BLOCK*)(sbptr->SBdataptr);
			if (bs_bhv)
			{
				GLOBALASSERT(bs_bhv->bhvr_type == I_BehaviourBinarySwitch);
				
			 	/* Andy 20/8/97: remove sound handle, if we have one*/
			 	if(bs_bhv->soundHandle!=SOUND_NOACTIVEINDEX) Sound_Stop(bs_bhv->soundHandle);

				if(bs_bhv->bs_track)
				{
					Reset_Track(bs_bhv->bs_track);
				}
						
				txactrl_next = bs_bhv->bs_tac;
				while(txactrl_next)
				{	
					txactrl = txactrl_next;
					txactrl_next = txactrl->tac_next;
					DeallocateMem((void*)txactrl);
				}
				if (bs_bhv->target_names) DeallocateMem ((void *)bs_bhv->target_names);
				if (bs_bhv->bs_targets) DeallocateMem ((void *)bs_bhv->bs_targets);
				if (bs_bhv->request_messages) DeallocateMem ((void *)bs_bhv->request_messages);
				
			}
			break;
	 	}	
		case I_BehaviourLinkSwitch:
		{
			LINK_SWITCH_BEHAV_BLOCK *bs_bhv;
			TXACTRLBLK *txactrl;
			TXACTRLBLK *txactrl_next;

   			bs_bhv = (LINK_SWITCH_BEHAV_BLOCK*)(sbptr->SBdataptr);
			if (bs_bhv)
			{
				GLOBALASSERT(bs_bhv->bhvr_type == I_BehaviourLinkSwitch);

			 	/* Andy 20/8/97: remove sound handle, if we have one*/
			 	if(bs_bhv->soundHandle!=SOUND_NOACTIVEINDEX) Sound_Stop(bs_bhv->soundHandle);

				if(bs_bhv->ls_track)
				{
					Reset_Track(bs_bhv->ls_track);
				}


				txactrl_next = bs_bhv->ls_tac;
				while(txactrl_next)
				{
					txactrl = txactrl_next;
					txactrl_next = txactrl->tac_next;
					DeallocateMem((void*)txactrl);
				}

				
				if (bs_bhv->ls_targets) DeallocateMem ((void *)bs_bhv->ls_targets);
				if (bs_bhv->lswitch_list) DeallocateMem ((void *)bs_bhv->lswitch_list);
			}
			break;
		}	
		case I_BehaviourLift:
		{
			LIFT_BEHAV_BLOCK *lift_bhv;
   		lift_bhv = (LIFT_BEHAV_BLOCK*)(sbptr->SBdataptr);
   		
			if (lift_bhv)
			{
				GLOBALASSERT(lift_bhv->bhvr_type == I_BehaviourLift);

				/* patrick 7/7//97*/
				if(lift_bhv->lift_control)
				{
					if(lift_bhv->lift_control->SoundHandle!=SOUND_NOACTIVEINDEX)
					Sound_Stop(lift_bhv->lift_control->SoundHandle);
				}
			
				if(lift_bhv->controller)
				{
					if (lift_bhv->lift_control && lift_bhv->lift_control->lift_stations)
						DeallocateMem(lift_bhv->lift_control->lift_stations);
					if (lift_bhv->lift_control)
						DeallocateMem(lift_bhv->lift_control);
				}	
			}
			break;
		}	
		case I_BehaviourAutoGun:
		{
			AUTOGUN_STATUS_BLOCK *ag_bhv;

 			ag_bhv = (AUTOGUN_STATUS_BLOCK*)(sbptr->SBdataptr);
			if (ag_bhv)
			{
				/* patrick 7/7/97*/
				if(ag_bhv->soundHandle!=SOUND_NOACTIVEINDEX)
				{
					Sound_Stop(ag_bhv->soundHandle);
				}
				Dispel_HModel(&ag_bhv->HModelController);
			
			}
			break;
		}
		case I_BehaviourAlien:
			{
				ALIEN_STATUS_BLOCK *asb;
				asb=(ALIEN_STATUS_BLOCK *)(sbptr->SBdataptr);
				Dispel_HModel(&asb->HModelController);
			}
			break;
		case I_BehaviourGenerator:
		{
			break;
		}	

		case I_BehaviourMarine:
		case I_BehaviourSeal:
		{
			MARINE_STATUS_BLOCK *marineStatusPointer;
			marineStatusPointer = (MARINE_STATUS_BLOCK *)(sbptr->SBdataptr);    
			if(marineStatusPointer)
			{
				Dispel_HModel(&marineStatusPointer->HModelController);
	
				if(marineStatusPointer->myGunFlash) DestroyActiveObject(marineStatusPointer->myGunFlash);
				if(marineStatusPointer->soundHandle!=SOUND_NOACTIVEINDEX) Sound_Stop(marineStatusPointer->soundHandle);
			}
			break;
		}
		case I_BehaviourDummy:
		{
			DUMMY_STATUS_BLOCK *dummyStatusPointer;
			dummyStatusPointer = (DUMMY_STATUS_BLOCK *)(sbptr->SBdataptr);    
			if(dummyStatusPointer)
			{
				Dispel_HModel(&dummyStatusPointer->HModelController);
			}
			break;
		}
		case I_BehaviourPredatorDisc_SeekTrack:
		{
		  	PC_PRED_DISC_BEHAV_BLOCK *bblk;
			bblk = (PC_PRED_DISC_BEHAV_BLOCK *)(sbptr->SBdataptr);    
			if(bblk)
			{
				if (bblk->soundHandle!=SOUND_NOACTIVEINDEX) {
					Sound_Stop(bblk->soundHandle);
				}
				
				Dispel_HModel(&bblk->HModelController);
	
			}
			break;
		}
		case I_BehaviourPredatorEnergyBolt:
		case I_BehaviourFrisbeeEnergyBolt:
		{
		  	CASTER_BOLT_BEHAV_BLOCK *bblk;
			bblk = (CASTER_BOLT_BEHAV_BLOCK *)(sbptr->SBdataptr);    
			if(bblk)
			{
				if (bblk->soundHandle!=SOUND_NOACTIVEINDEX) {
					Sound_Stop(bblk->soundHandle);
				}
			}
			break;
		}
		case I_BehaviourFrisbee:
		{
		  	FRISBEE_BEHAV_BLOCK *fblk;
			fblk = (FRISBEE_BEHAV_BLOCK *)(sbptr->SBdataptr);    
			if(fblk)
			{
				if (fblk->soundHandle!=SOUND_NOACTIVEINDEX) {
					Sound_Stop(fblk->soundHandle);
				}
				
				Dispel_HModel(&fblk->HModelController);
	
			}
			break;
		}

		case I_BehaviourFaceHugger:
		{
			FACEHUGGER_STATUS_BLOCK *fhugStatusPointer;
			fhugStatusPointer = (FACEHUGGER_STATUS_BLOCK *)(sbptr->SBdataptr);    
			if(fhugStatusPointer)
			{
				if(fhugStatusPointer->soundHandle!=SOUND_NOACTIVEINDEX) Sound_Stop(fhugStatusPointer->soundHandle);
			}
			Dispel_HModel(&fhugStatusPointer->HModelController);
			break;
		}

		case I_BehaviourXenoborg:
		{
			/* need to get rid of the animations...*/
			XENO_STATUS_BLOCK *xenoStatus;

			xenoStatus = (XENO_STATUS_BLOCK*)(sbptr->SBdataptr);
			if(xenoStatus)
			{
				Dispel_HModel(&xenoStatus->HModelController);
				/* Zounds! */
				if(xenoStatus->soundHandle1!=SOUND_NOACTIVEINDEX) Sound_Stop(xenoStatus->soundHandle1);
				if(xenoStatus->soundHandle2!=SOUND_NOACTIVEINDEX) Sound_Stop(xenoStatus->soundHandle2);
				if(xenoStatus->head_whirr!=SOUND_NOACTIVEINDEX) Sound_Stop(xenoStatus->head_whirr);
				if(xenoStatus->left_arm_whirr!=SOUND_NOACTIVEINDEX) Sound_Stop(xenoStatus->left_arm_whirr);
				if(xenoStatus->right_arm_whirr!=SOUND_NOACTIVEINDEX) Sound_Stop(xenoStatus->right_arm_whirr);
				if(xenoStatus->torso_whirr!=SOUND_NOACTIVEINDEX) Sound_Stop(xenoStatus->torso_whirr);

			}
			break;
		}			
		case I_BehaviourPredator:
		{
			PREDATOR_STATUS_BLOCK *predatorStatusPointer;
			predatorStatusPointer = (PREDATOR_STATUS_BLOCK *)(sbptr->SBdataptr);    
			if(predatorStatusPointer)
			{
				Dispel_HModel(&predatorStatusPointer->HModelController);
	
				if(predatorStatusPointer->soundHandle!=SOUND_NOACTIVEINDEX) Sound_Stop(predatorStatusPointer->soundHandle);
			}
			break;
		}
		case I_BehaviourQueenAlien:
		{
			QUEEN_STATUS_BLOCK *queenStatusPointer;
			queenStatusPointer = (QUEEN_STATUS_BLOCK *)(sbptr->SBdataptr);    
			if(queenStatusPointer)
			{
				Dispel_HModel(&queenStatusPointer->HModelController);
			}
			//stop any sound the queen is making
			if(queenStatusPointer->soundHandle!=SOUND_NOACTIVEINDEX) Sound_Stop(queenStatusPointer->soundHandle);
			
			break;
		}
		case I_BehaviourNetGhost:
		{
			{
				NETGHOSTDATABLOCK *ghostData;
				ghostData = (NETGHOSTDATABLOCK *)(sbptr->SBdataptr);    
				if(ghostData)
				{
					if(ghostData->myGunFlash) DestroyActiveObject(ghostData->myGunFlash);
					if(ghostData->SoundHandle!=SOUND_NOACTIVEINDEX) Sound_Stop(ghostData->SoundHandle);
					if(ghostData->SoundHandle2!=SOUND_NOACTIVEINDEX) Sound_Stop(ghostData->SoundHandle2);
					if(ghostData->SoundHandle3!=SOUND_NOACTIVEINDEX) Sound_Stop(ghostData->SoundHandle3);
					Dispel_HModel(&ghostData->HModelController);
				}
			}
			break;
		}	

		case I_BehaviourXenoborgMorphRoom:
		{
			XENO_MORPH_ROOM_DATA * xmrd = (XENO_MORPH_ROOM_DATA *)sbptr->SBdataptr;
	
			if (sbptr->SBmorphctrl && 
					sbptr->SBmorphctrl->ObMorphHeader && 
					sbptr->SBmorphctrl->ObMorphHeader->mph_frames) 
				DeallocateMem( sbptr->SBmorphctrl->ObMorphHeader->mph_frames);
	
			if (sbptr->SBmorphctrl && 
					sbptr->SBmorphctrl->ObMorphHeader) 
				DeallocateMem( sbptr->SBmorphctrl->ObMorphHeader);

			if (sbptr->SBmorphctrl)
				DeallocateMem( sbptr->SBmorphctrl);

			if (xmrd)
			{
				if (xmrd->tacb)
				{
					TXACTRLBLK *txactrl;
					TXACTRLBLK *txactrl_next;

					txactrl_next = xmrd->tacb;

					while(txactrl_next)
					{
						txactrl = txactrl_next;
						txactrl_next = txactrl->tac_next;
						DeallocateMem((void*)txactrl);
					}
				}
			}
			break;
		}
		case I_BehaviourInanimateObject :
		{
			INANIMATEOBJECT_STATUSBLOCK* objectstatusptr=(INANIMATEOBJECT_STATUSBLOCK*)sbptr->SBdataptr;
			TXACTRLBLK *txactrl;
			TXACTRLBLK *txactrl_next;

		
			if (objectstatusptr)
			{
				if(objectstatusptr->event_target)
				{
					DeallocateMem(objectstatusptr->event_target);
				}
				
				if(!objectstatusptr->inan_tac) break;
				txactrl_next = objectstatusptr->inan_tac;

				while(txactrl_next)
				{
					txactrl = txactrl_next;
					txactrl_next = txactrl->tac_next;
					DeallocateMem((void*)txactrl);
				}
			}
			break;
		}
		case I_BehaviourVideoScreen :
		{
			VIDEO_SCREEN_BEHAV_BLOCK* videoScreen=(VIDEO_SCREEN_BEHAV_BLOCK*)sbptr->SBdataptr;
			TXACTRLBLK *txactrl;
			TXACTRLBLK *txactrl_next;

		
			if (videoScreen)
			{
				
				if(!videoScreen->inan_tac) break;
				txactrl_next = videoScreen->inan_tac;

				while(txactrl_next)
				{
					txactrl = txactrl_next;
					txactrl_next = txactrl->tac_next;
					DeallocateMem((void*)txactrl);
				}
			}
			break;
		}

		case I_BehaviourPlacedLight :
		{
			PLACED_LIGHT_BEHAV_BLOCK* pl_bhv=(PLACED_LIGHT_BEHAV_BLOCK*)sbptr->SBdataptr;
			TXACTRLBLK *txactrl;
			TXACTRLBLK *txactrl_next;
					
			if (pl_bhv)
			{
				if(sbptr->SBdptr)
				{
					//set num lights to zero so it doesn't attempt to deallocate the light
					sbptr->SBdptr->ObNumLights=0;
				}
				//reset vaules in lightblock
				pl_bhv->light->LightBright=pl_bhv->light->LightBrightStore;
				pl_bhv->light->RedScale=pl_bhv->colour_red;
				pl_bhv->light->GreenScale=pl_bhv->colour_green;
				pl_bhv->light->BlueScale=pl_bhv->colour_blue;
								
				if(!pl_bhv->inan_tac) break;
				txactrl_next = pl_bhv->inan_tac;

				while(txactrl_next)
				{
					txactrl = txactrl_next;
					txactrl_next = txactrl->tac_next;
					DeallocateMem((void*)txactrl);
				}
			}
			break;
		}
		case I_BehaviourHierarchicalFragment:
			{
				HDEBRIS_BEHAV_BLOCK *hdbblk=(HDEBRIS_BEHAV_BLOCK *)sbptr->SBdataptr;

				Dispel_HModel(&hdbblk->HModelController);
			}
			break;
		
		case I_BehaviourPlacedSound:
		{
			SoundBehaveDestroy (sbptr);
			break;
		}
		
		case I_BehaviourMissionComplete:
		{
			MISSION_COMPLETE_BEHAV_BLOCK *mc_bhv;
			mc_bhv = (MISSION_COMPLETE_BEHAV_BLOCK*)sbptr->SBdataptr;
			if(mc_bhv)
			{
				GLOBALASSERT((mc_bhv->bhvr_type == I_BehaviourMissionComplete));
				ResetMission(mc_bhv->mission_objective_ptr); 

			}
			break;
		}
		
		case I_BehaviourTrackObject:
		{
			TRACK_OBJECT_BEHAV_BLOCK *to_bhv;
			TXACTRLBLK *txactrl;
			TXACTRLBLK *txactrl_next;

   			to_bhv = (TRACK_OBJECT_BEHAV_BLOCK*)(sbptr->SBdataptr);
			if (to_bhv)
			{
				GLOBALASSERT(to_bhv->bhvr_type == I_BehaviourTrackObject);
				
				if(to_bhv->to_track)
				{
					Reset_Track(to_bhv->to_track);
				}
				
				txactrl_next = to_bhv->to_tac;
				while(txactrl_next)
				{	
					txactrl = txactrl_next;
					txactrl_next = txactrl->tac_next;
					DeallocateMem((void*)txactrl);
				}
			}
			break;
		case I_BehaviourPlacedHierarchy:
		{
			PLACED_HIERARCHY_BEHAV_BLOCK *ph_bhv;

   			ph_bhv = (PLACED_HIERARCHY_BEHAV_BLOCK*)(sbptr->SBdataptr);
			if (ph_bhv)
			{
				GLOBALASSERT(ph_bhv->bhvr_type == I_BehaviourPlacedHierarchy);
				DeletePlacedHierarchy(ph_bhv);
			}
			break;
		}
		case I_BehaviourNetCorpse:
		{
			{
				NETCORPSEDATABLOCK *corpseData;
				corpseData = (NETCORPSEDATABLOCK *)(sbptr->SBdataptr);    
				if(corpseData)
				{
					if(corpseData->SoundHandle!=SOUND_NOACTIVEINDEX) Sound_Stop(corpseData->SoundHandle);
					Dispel_HModel(&corpseData->HModelController);
				}
			}
			break;
		}	

		case I_BehaviourLightFX :
		{
			LIGHT_FX_BEHAV_BLOCK * lfxbb;
			TXACTRLBLK *txactrl;
			TXACTRLBLK *txactrl_next;
			
			lfxbb = (LIGHT_FX_BEHAV_BLOCK *)sbptr->SBdataptr;
			if(lfxbb)
			{
				txactrl_next = lfxbb->anim_control;
				while(txactrl_next)
				{
					txactrl = txactrl_next;
					txactrl_next = txactrl->tac_next;
					DeallocateMem((void*)txactrl);
				}
			}
			break;
			
		}
		
		case I_BehaviourFan :
		{
			FAN_BEHAV_BLOCK* f_bhv=(FAN_BEHAV_BLOCK*)sbptr->SBdataptr;
			if(f_bhv->track)
			{
				Reset_Track(f_bhv->track);
			}
			break;

			
		}
		case I_BehaviourSpeargunBolt:
		{
			SPEAR_BEHAV_BLOCK *sbblk=(SPEAR_BEHAV_BLOCK *)sbptr->SBdataptr;

			if (sbblk->SpearThroughFragment) {
				/* There's a hierarchy to dispel. */
				Dispel_HModel(&sbblk->HierarchicalFragment);
			}
			break;
		}
		case I_BehaviourPlatform :
		{
			PLATFORMLIFT_BEHAVIOUR_BLOCK* pl_bhv=(PLATFORMLIFT_BEHAVIOUR_BLOCK*)sbptr->SBdataptr;
			if(pl_bhv->sound)
			{
				Stop_Track_Sound(pl_bhv->sound);
			}
			if(pl_bhv->start_sound)
			{
				Stop_Track_Sound(pl_bhv->start_sound);
			}
			if(pl_bhv->end_sound)
			{
				Stop_Track_Sound(pl_bhv->end_sound);
			}
			break;	
		}
		case I_BehaviourParticleGenerator :
		{
			PARTICLE_GENERATOR_BEHAV_BLOCK* pargen=(PARTICLE_GENERATOR_BEHAV_BLOCK*)sbptr->SBdataptr;
			if(pargen->sound)
			{
				Stop_Track_Sound(pargen->sound);
			}
			break;	
		}
	 	
	 	}	
		default:
		{
			break;
		}
	}

		
	/* destroy the behaviour block BUT not if I am the player	*/

	if((sbptr->I_SBtype != I_BehaviourMarinePlayer)&&(sbptr->I_SBtype != I_BehaviourAlienPlayer)&&(sbptr->I_SBtype != I_BehaviourPredatorPlayer))
	{
		/* patrick: 23/7/97 I am adding a test for SBdataptr before deallocating it- */
		if(sbptr->SBdataptr)
		{
			DeallocateMem(sbptr->SBdataptr);
			#if debug
			//I dont do a full initialisation with debug because
			//we dont want to hide switch on Behaviour type bugs
			// I just do this to trap the bastard
			sbptr->SBdataptr = NULL; 
			#endif
		}
	}

	/* remove DYNBLOCK and DisplayBlock*/
	if (sbptr->SBdptr) 
	{
		DestroyActiveObject(sbptr->SBdptr);
	}
	if (sbptr->DynPtr) 
	{
		DeallocateDynamicsBlock(sbptr->DynPtr);
	}	

	#if !debug
	// ull SB init
	if(!sbptr->SBflags.preserve_until_end_of_level)
	{
		InitialiseSBValues(sbptr);
	}
	#endif
	/* Finally remove the StrategyBlock*/
	DestroyActiveStrategyBlock(sbptr);
}




/*--------------------**
** Loading and Saving **
**--------------------*/
typedef struct prox_door_save_block
{
	SAVE_BLOCK_STRATEGY_HEADER header;

	int door_state;
	BOOL door_locked;

	//from the morph control
	int ObMorphCurrFrame;
	int ObMorphFlags;
	int ObMorphSpeed;

	BOOL triggeredByMarine;

}PROX_DOOR_SAVE_BLOCK;

void LoadStrategy_ProxDoor(SAVE_BLOCK_STRATEGY_HEADER* header)
{
	STRATEGYBLOCK* sbPtr;
	PROXDOOR_BEHAV_BLOCK *doorbhv;
	PROX_DOOR_SAVE_BLOCK* block = (PROX_DOOR_SAVE_BLOCK*) header; 

	if(header->size!=sizeof(*block)) return;

	//find the existing strategy block
	sbPtr = FindSBWithName(block->header.SBname);
	if(!sbPtr) return;

	//make sure the strategy found is of the right type
	if(sbPtr->I_SBtype != I_BehaviourProximityDoor) return;

	doorbhv = (PROXDOOR_BEHAV_BLOCK*)sbPtr->SBdataptr;

	doorbhv->door_state = block->door_state;
	doorbhv->door_locked = block->door_locked;
	doorbhv->triggeredByMarine = block->triggeredByMarine;
	doorbhv->PDmctrl->ObMorphCurrFrame = block->ObMorphCurrFrame;
	doorbhv->PDmctrl->ObMorphFlags = block->ObMorphFlags;
	doorbhv->PDmctrl->ObMorphSpeed = block->ObMorphSpeed;


	Load_SoundState(&doorbhv->SoundHandle);

}

void SaveStrategy_ProxDoor(STRATEGYBLOCK* sbPtr)
{
	PROX_DOOR_SAVE_BLOCK *block;
	PROXDOOR_BEHAV_BLOCK *doorbhv ;
	doorbhv = (PROXDOOR_BEHAV_BLOCK*)sbPtr->SBdataptr;

	GET_STRATEGY_SAVE_BLOCK(block,sbPtr);


	block->door_state = doorbhv->door_state;
	block->door_locked = doorbhv->door_locked;
	block->triggeredByMarine = doorbhv->triggeredByMarine;
	block->ObMorphCurrFrame = doorbhv->PDmctrl->ObMorphCurrFrame;
	block->ObMorphFlags = doorbhv->PDmctrl->ObMorphFlags;
	block->ObMorphSpeed = doorbhv->PDmctrl->ObMorphSpeed;

	Save_SoundState(&doorbhv->SoundHandle);
}


/*---------------------------**
** End of loading and saving **
**---------------------------*/
