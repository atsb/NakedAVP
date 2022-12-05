#include "3dc.h"		 
#include "inline.h"
#include "module.h"
#include "stratdef.h"
#include "gamedef.h"
#include "avp_envinfo.h"
#include "avp_userprofile.h"
#include "avp_mp_config.h"
#include "pldnet.h"

static enum AVP_ENVIRONMENT_ID MarineEpisodes[] =
{
	// main single player game
	AVP_ENVIRONMENT_DERELICT,
	AVP_ENVIRONMENT_COLONY,
	AVP_ENVIRONMENT_INVASION,
	AVP_ENVIRONMENT_ORBITAL,
	AVP_ENVIRONMENT_TYRARGO,
	AVP_ENVIRONMENT_TYRARGOHANGAR,

	AVP_ENVIRONMENT_TEMPLE_M,
	AVP_ENVIRONMENT_VAULTS_M,
	AVP_ENVIRONMENT_FERARCO_M,
	AVP_ENVIRONMENT_GATEWAY_M,
	AVP_ENVIRONMENT_WATERFALL_M,
	// that's all folks
	AVP_ENVIRONMENT_END_OF_LIST
};

static enum AVP_ENVIRONMENT_ID PredatorEpisodes[] =
{
	// main single player game
	AVP_ENVIRONMENT_WATERFALL,
	AVP_ENVIRONMENT_AREA52,
	AVP_ENVIRONMENT_VAULTS,
	AVP_ENVIRONMENT_FURY161,
	AVP_ENVIRONMENT_CAVERNS,
	AVP_ENVIRONMENT_CAVERNSEND,
	
	AVP_ENVIRONMENT_INVASION_P,
	AVP_ENVIRONMENT_ESCAPE_P,
	AVP_ENVIRONMENT_TEMPLE_P,
	AVP_ENVIRONMENT_EARTHBOUND_P,
	AVP_ENVIRONMENT_TYRARGO_P,

	// that's all folks
	AVP_ENVIRONMENT_END_OF_LIST
};

static enum AVP_ENVIRONMENT_ID AlienEpisodes[] =
{
	// main single player game
	AVP_ENVIRONMENT_TEMPLE,
	AVP_ENVIRONMENT_ESCAPE,
	AVP_ENVIRONMENT_FERARCO,
	AVP_ENVIRONMENT_GATEWAY,
	AVP_ENVIRONMENT_EARTHBOUND,

	AVP_ENVIRONMENT_INVASION_A,
	AVP_ENVIRONMENT_DERELICT_A,
	AVP_ENVIRONMENT_TYRARGO_A,
	AVP_ENVIRONMENT_CAVERNS_A,
	AVP_ENVIRONMENT_FURY161_A,
	// that's all folks
	AVP_ENVIRONMENT_END_OF_LIST
};

static enum AVP_ENVIRONMENT_ID MultiplayerEpisodes[] =
{
 #ifndef MPLAYER_DEMO
	AVP_ENVIRONMENT_SEWER,
//	AVP_ENVIRONMENT_SCREAM,
	AVP_ENVIRONMENT_MASSACRE,
//	AVP_ENVIRONMENT_STATION,
//	AVP_ENVIRONMENT_DESTRUCTION,
	AVP_ENVIRONMENT_STATUE,
	AVP_ENVIRONMENT_JOCKEY,
 #endif

	AVP_ENVIRONMENT_HIVE,

//and now the multipack levels
	AVP_ENVIRONMENT_LEADWORKS_MP,
	AVP_ENVIRONMENT_HADLEYSHOPE_MP,
	AVP_ENVIRONMENT_MEATFACTORY_MP,
	AVP_ENVIRONMENT_NOSTROMO_MP,
	AVP_ENVIRONMENT_SUBWAY_MP,
	AVP_ENVIRONMENT_ELEVATOR_MP,
	AVP_ENVIRONMENT_LAB14_MP,
	AVP_ENVIRONMENT_COMPOUND_MP,
	AVP_ENVIRONMENT_OFFICE_MP,

	// that's all folks
	AVP_ENVIRONMENT_END_OF_LIST
};

static enum AVP_ENVIRONMENT_ID CooperativeEpisodes[] =
{
	AVP_ENVIRONMENT_KENS_COOP,
	AVP_ENVIRONMENT_HIVE_COOP,
	AVP_ENVIRONMENT_TRAPPED_COOP,
	AVP_ENVIRONMENT_ALS_DM_COOP,
	AVP_ENVIRONMENT_JOCKEY_COOP,

//and now the multipack levels
	AVP_ENVIRONMENT_LEADWORKS_COOP,
	AVP_ENVIRONMENT_HADLEYSHOPE_COOP,
	AVP_ENVIRONMENT_MEATFACTORY_COOP,
	AVP_ENVIRONMENT_NOSTROMO_COOP,
	AVP_ENVIRONMENT_SUBWAY_COOP,
	AVP_ENVIRONMENT_ELEVATOR_COOP,
	AVP_ENVIRONMENT_LAB14_COOP,
	AVP_ENVIRONMENT_COMPOUND_COOP,

	AVP_ENVIRONMENT_END_OF_LIST
};

static char *RifNamesForEnvironments[] =
{
	// primarily Marine
	"derelict",//AVP_ENVIRONMENT_DERELICT,
	"genshd1",//AVP_ENVIRONMENT_COLONY,
	"invasion",//AVP_ENVIRONMENT_INVASION,
	"odobenus",//AVP_ENVIRONMENT_ORBITAL,
	"sulaco",//AVP_ENVIRONMENT_TYRARGO,
	"hangar",//AVP_ENVIRONMENT_TYRARGOHANGAR,

	// primarily Predator
	"fall",//AVP_ENVIRONMENT_WATERFALL,
	"area52",//AVP_ENVIRONMENT_AREA52,
	"vaults",//AVP_ENVIRONMENT_VAULTS,
	"furyall",//AVP_ENVIRONMENT_FURY161,
	"caverns",//AVP_ENVIRONMENT_CAVERNS,
	"battle",//AVP_ENVIRONMENT_CAVERNSEND,

	// primarily Alien
	"nost03",//AVP_ENVIRONMENT_FERARCO,
	"temple",//AVP_ENVIRONMENT_TEMPLE,
	"stat101",//AVP_ENVIRONMENT_GATEWAY,
	"escape",//AVP_ENVIRONMENT_ESCAPE,
	"breakout",//AVP_ENVIRONMENT_EARTHBOUND,

	// primarily multiplayer
	"als-dm",//AVP_ENVIRONMENT_SEWER,
	"e3demo",//AVP_ENVIRONMENT_MASSACRE,
	"statue",//AVP_ENVIRONMENT_STATUE,
	"jockey",//AVP_ENVIRONMENT_JOCKEY
	"hive",//AVP_ENVIRONEMENT_HIVE

	// Alien bonus levels 
	"invasion_a",//AVP_ENVIRONMENT_INVASION_A,
	"derelict_a",//AVP_ENVIRONMENT_DERELICT_A,
	"sulaco_a",//AVP_ENVIRONMENT_TYRARGO_A,
	"furyall_a",//AVP_ENVIRONMENT_FURY161_A,
	"caverns_a",//AVP_ENVIRONMENT_CAVERNS_A,

	// Predator	bonus levels
	"invasion_p",//AVP_ENVIRONMENT_INVASION_P,
	"sulaco_p",//AVP_ENVIRONMENT_TYRARGO_P,
	"temple_p",//AVP_ENVIRONMENT_TEMPLE_P,
	"escape_p",//AVP_ENVIRONMENT_ESCAPE_P,
	"breakout_p",//AVP_ENVIRONMENT_EARTHBOUND_P,
	
	// Marine bonus levels
	"fall_m",//AVP_ENVIRONMENT_WATERFALL_M,
	"vaults_m",//AVP_ENVIRONMENT_VAULTS_M,
	"nost03_m",//AVP_ENVIRONMENT_FERARCO_M,
	"temple_m",//AVP_ENVIRONMENT_TEMPLE_M,
	"stat101_m",//AVP_ENVIRONMENT_GATEWAY_M,


	//cooperative levels
	"kens-co-op",//AVP_ENVIRONMENT_KENS_COOP,
	"hive_c",//AVP_ENVIRONMENT_HIVE_COOP,
	"trapped",//AVP_ENVIRONMENT_TRAPPED_COOP,
	"als-dm-coop",//AVP_ENVIRONMENT_ALS_DM_COOP,
	"jockeycoop",//AVP_ENVIRONMENT_JOCKEY_COOP,

	// demo levels
	"e3demosp",//AVP_ENVIRONMENT_E3DEMOSP,

	"Not a Level",//AVP_ENVIRONMENT_END_OF_LIST

	//multipack multiplayer levels
	"leadworks",//AVP_ENVIRONMENT_LEADWORKS_MP,
	"hadleyshope",//AVP_ENVIRONMENT_HADLEYSHOPE_MP,
	"meat_factory",//AVP_ENVIRONMENT_MEATFACTORY_MP,
	"nostromo",//AVP_ENVIRONMENT_NOSTROMO_MP,
	"subway",//AVP_ENVIRONMENT_SUBWAY_MP,
	"elevator",//AVP_ENVIRONMENT_ELEVATOR_MP,
	"lab14",//AVP_ENVIRONMENT_LAB14_MP,
	"compound",//AVP_ENVIRONMENT_COMPOUND_MP,
	"office",//AVP_ENVIRONMENT_OFFICE_MP,

	//multipack multiplayer cooperative levels
	"leadworks_coop",//AVP_ENVIRONMENT_LEADWORKS_COOP,
	"hadleyshope_coop",//AVP_ENVIRONMENT_HADLEYSHOPE_COOP,
	"co-op_meat_factory",//AVP_ENVIRONMENT_MEATFACTORY_COOP,
	"nostromo_coop",//AVP_ENVIRONMENT_NOSTROMO_COOP,
	"subwaycoop",//AVP_ENVIRONMENT_SUBWAY_COOP,
	"elevator_co-op",//AVP_ENVIRONMENT_ELEVATOR_COOP,
	"lab14coop",//AVP_ENVIRONMENT_LAB14_COOP,
	"compoundcoop",//AVP_ENVIRONMENT_COMPOUND_COOP,
};

extern char LevelName[];

AvP_Level_Target_Desc LevelStatsTargets[I_MaxDifficulties][AVP_ENVIRONMENT_END_OF_LIST] = {
{
	{
		{	/* Derelict / Easy */
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},	/* Creatures Killed */
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},	/* Creatures decapitated*/
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},	/* Trophies / Live Head Bites */
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},	/* Dead Head Bites */
			-1,		/* Shots Fired */
			-1,		/* Accuracy */
			-1,		/* Spotted */
			0,		/* Total Seconds (unsigned!) */
			0,		/* Total Minutes (unsigned!) */
			-1,		/* Total Hours */
			0,		/* Cloaked Seconds */
			0,		/* Cloaked Minutes */
			-1,		/* Cloaked Hours */
			-1,		/* Health Damage */
			-1,		/* Armour Damage */
			-1,		/* Average Speed */
			-1,		/* Field Charge Used */
			-1,		/* Head Shot Percentage */
			{		/* Padding */
				0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0
			}
		},
		CHEATMODE_NONACTIVE	/* Cheat to activate */
	},
	{
		{	/* Colony / Easy */
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},	/* Creatures Killed */
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},	/* Creatures decapitated*/
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},	/* Trophies / Live Head Bites */
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},	/* Dead Head Bites */
			-1,		/* Shots Fired */
			-1,		/* Accuracy */
			-1,		/* Spotted */
			0,		/* Total Seconds (unsigned!) */
			0,		/* Total Minutes (unsigned!) */
			-1,		/* Total Hours */
			0,		/* Cloaked Seconds */
			0,		/* Cloaked Minutes */
			-1,		/* Cloaked Hours */
			-1,		/* Health Damage */
			-1,		/* Armour Damage */
			-1,		/* Average Speed */
			-1,		/* Field Charge Used */
			-1,		/* Head Shot Percentage */
			{		/* Padding */
				0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0
			}
		},
		CHEATMODE_NONACTIVE	/* Cheat to activate */
	},
	{
		{	/* Invasion / Easy */
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},	/* Creatures Killed */
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},	/* Creatures decapitated*/
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},	/* Trophies / Live Head Bites */
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},	/* Dead Head Bites */
			-1,		/* Shots Fired */
			-1,		/* Accuracy */
			-1,		/* Spotted */
			0,		/* Total Seconds (unsigned!) */
			0,		/* Total Minutes (unsigned!) */
			-1,		/* Total Hours */
			0,		/* Cloaked Seconds */
			0,		/* Cloaked Minutes */
			-1,		/* Cloaked Hours */
			-1,		/* Health Damage */
			-1,		/* Armour Damage */
			-1,		/* Average Speed */
			-1,		/* Field Charge Used */
			-1,		/* Head Shot Percentage */
			{		/* Padding */
				0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0
			}
		},
		CHEATMODE_NONACTIVE	/* Cheat to activate */
	},
	{
		{	/* Orbital / Easy */
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},	/* Creatures Killed */
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},	/* Creatures decapitated*/
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},	/* Trophies / Live Head Bites */
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},	/* Dead Head Bites */
			-1,		/* Shots Fired */
			-1,		/* Accuracy */
			-1,		/* Spotted */
			0,		/* Total Seconds (unsigned!) */
			0,		/* Total Minutes (unsigned!) */
			-1,		/* Total Hours */
			0,		/* Cloaked Seconds */
			0,		/* Cloaked Minutes */
			-1,		/* Cloaked Hours */
			-1,		/* Health Damage */
			-1,		/* Armour Damage */
			-1,		/* Average Speed */
			-1,		/* Field Charge Used */
			-1,		/* Head Shot Percentage */
			{		/* Padding */
				0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0
			}
		},
		CHEATMODE_NONACTIVE	/* Cheat to activate */
	},
	{
		{	/* Tyrago / Easy */
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},	/* Creatures Killed */
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},	/* Creatures decapitated*/
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},	/* Trophies / Live Head Bites */
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},	/* Dead Head Bites */
			-1,		/* Shots Fired */
			-1,		/* Accuracy */
			-1,		/* Spotted */
			0,		/* Total Seconds (unsigned!) */
			0,		/* Total Minutes (unsigned!) */
			-1,		/* Total Hours */
			0,		/* Cloaked Seconds */
			0,		/* Cloaked Minutes */
			-1,		/* Cloaked Hours */
			-1,		/* Health Damage */
			-1,		/* Armour Damage */
			-1,		/* Average Speed */
			-1,		/* Field Charge Used */
			-1,		/* Head Shot Percentage */
			{		/* Padding */
				0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0
			}
		},
		CHEATMODE_NONACTIVE	/* Cheat to activate */
	},
	{
		{	/* Hangar / Easy */
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},	/* Creatures Killed */
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},	/* Creatures decapitated*/
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},	/* Trophies / Live Head Bites */
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},	/* Dead Head Bites */
			-1,		/* Shots Fired */
			-1,		/* Accuracy */
			-1,		/* Spotted */
			0,		/* Total Seconds (unsigned!) */
			0,		/* Total Minutes (unsigned!) */
			-1,		/* Total Hours */
			0,		/* Cloaked Seconds */
			0,		/* Cloaked Minutes */
			-1,		/* Cloaked Hours */
			-1,		/* Health Damage */
			-1,		/* Armour Damage */
			-1,		/* Average Speed */
			-1,		/* Field Charge Used */
			-1,		/* Head Shot Percentage */
			{		/* Padding */
				0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0
			}
		},
		CHEATMODE_NONACTIVE	/* Cheat to activate */
	},
	{
		{	/* Waterfall / Easy */
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},	/* Creatures Killed */
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},	/* Creatures decapitated*/
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},	/* Trophies / Live Head Bites */
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},	/* Dead Head Bites */
			-1,		/* Shots Fired */
			-1,		/* Accuracy */
			-1,		/* Spotted */
			0,		/* Total Seconds (unsigned!) */
			0,		/* Total Minutes (unsigned!) */
			-1,		/* Total Hours */
			0,		/* Cloaked Seconds */
			0,		/* Cloaked Minutes */
			-1,		/* Cloaked Hours */
			-1,		/* Health Damage */
			-1,		/* Armour Damage */
			-1,		/* Average Speed */
			-1,		/* Field Charge Used */
			-1,		/* Head Shot Percentage */
			{		/* Padding */
				0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0
			}
		},
		CHEATMODE_NONACTIVE	/* Cheat to activate */
	},
	{
		{	/* Area52 / Easy */
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},	/* Creatures Killed */
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},	/* Creatures decapitated*/
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},	/* Trophies / Live Head Bites */
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},	/* Dead Head Bites */
			-1,		/* Shots Fired */
			-1,		/* Accuracy */
			-1,		/* Spotted */
			0,		/* Total Seconds (unsigned!) */
			0,		/* Total Minutes (unsigned!) */
			-1,		/* Total Hours */
			0,		/* Cloaked Seconds */
			0,		/* Cloaked Minutes */
			-1,		/* Cloaked Hours */
			-1,		/* Health Damage */
			-1,		/* Armour Damage */
			-1,		/* Average Speed */
			-1,		/* Field Charge Used */
			-1,		/* Head Shot Percentage */
			{		/* Padding */
				0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0
			}
		},
		CHEATMODE_NONACTIVE	/* Cheat to activate */
	},
	{
		{	/* Vaults / Easy */
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},	/* Creatures Killed */
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},	/* Creatures decapitated*/
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},	/* Trophies / Live Head Bites */
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},	/* Dead Head Bites */
			-1,		/* Shots Fired */
			-1,		/* Accuracy */
			-1,		/* Spotted */
			0,		/* Total Seconds (unsigned!) */
			0,		/* Total Minutes (unsigned!) */
			-1,		/* Total Hours */
			0,		/* Cloaked Seconds */
			0,		/* Cloaked Minutes */
			-1,		/* Cloaked Hours */
			-1,		/* Health Damage */
			-1,		/* Armour Damage */
			-1,		/* Average Speed */
			-1,		/* Field Charge Used */
			-1,		/* Head Shot Percentage */
			{		/* Padding */
				0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0
			}
		},
		CHEATMODE_NONACTIVE	/* Cheat to activate */
	},
	{
		{	/* Fury161 / Easy */
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Creatures Killed */
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Creatures decapitated*/
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Trophies / Live Head Bites */
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Dead Head Bites */
			-1,		/* Shots Fired */
			-1,		/* Accuracy */
			-1,		/* Spotted */
			0,		/* Total Seconds (unsigned!) */
			0,		/* Total Minutes (unsigned!) */
			-1,		/* Total Hours */
			0,		/* Cloaked Seconds */
			0,		/* Cloaked Minutes */
			-1,		/* Cloaked Hours */
			-1,		/* Health Damage */
			-1,		/* Armour Damage */
			-1,		/* Average Speed */
			-1,		/* Field Charge Used */
			-1,		/* Head Shot Percentage */
			{		/* Padding */
				0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,
			},
		},
		CHEATMODE_NONACTIVE,	/* Cheat to activate */
	},
	{
		{	/* Caverns / Easy */
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Creatures Killed */
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Creatures decapitated*/
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Trophies / Live Head Bites */
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Dead Head Bites */
			-1,		/* Shots Fired */
			-1,		/* Accuracy */
			-1,		/* Spotted */
			0,		/* Total Seconds (unsigned!) */
			0,		/* Total Minutes (unsigned!) */
			-1,		/* Total Hours */
			0,		/* Cloaked Seconds */
			0,		/* Cloaked Minutes */
			-1,		/* Cloaked Hours */
			-1,		/* Health Damage */
			-1,		/* Armour Damage */
			-1,		/* Average Speed */
			-1,		/* Field Charge Used */
			-1,		/* Head Shot Percentage */
			{		/* Padding */
				0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,
			},
		},
		CHEATMODE_NONACTIVE,	/* Cheat to activate */
	},
	{
		{	/* Battle / Easy */
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Creatures Killed */
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Creatures decapitated*/
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Trophies / Live Head Bites */
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Dead Head Bites */
			-1,		/* Shots Fired */
			-1,		/* Accuracy */
			-1,		/* Spotted */
			0,		/* Total Seconds (unsigned!) */
			0,		/* Total Minutes (unsigned!) */
			-1,		/* Total Hours */
			0,		/* Cloaked Seconds */
			0,		/* Cloaked Minutes */
			-1,		/* Cloaked Hours */
			-1,		/* Health Damage */
			-1,		/* Armour Damage */
			-1,		/* Average Speed */
			-1,		/* Field Charge Used */
			-1,		/* Head Shot Percentage */
			{		/* Padding */
				0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,
			},
		},
		CHEATMODE_NONACTIVE,	/* Cheat to activate */
	},
	{
		{	/* Feraco / Easy */
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Creatures Killed */
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Creatures decapitated*/
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Trophies / Live Head Bites */
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Dead Head Bites */
			-1,		/* Shots Fired */
			-1,		/* Accuracy */
			-1,		/* Spotted */
			0,		/* Total Seconds (unsigned!) */
			0,		/* Total Minutes (unsigned!) */
			-1,		/* Total Hours */
			0,		/* Cloaked Seconds */
			0,		/* Cloaked Minutes */
			-1,		/* Cloaked Hours */
			-1,		/* Health Damage */
			-1,		/* Armour Damage */
			-1,		/* Average Speed */
			-1,		/* Field Charge Used */
			-1,		/* Head Shot Percentage */
			{		/* Padding */
				0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,
			},
		},
		CHEATMODE_NONACTIVE,	/* Cheat to activate */
	},
	{
		{	/* Temple / Easy */
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Creatures Killed */
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Creatures decapitated*/
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Trophies / Live Head Bites */
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Dead Head Bites */
			-1,		/* Shots Fired */
			-1,		/* Accuracy */
			-1,		/* Spotted */
			0,		/* Total Seconds (unsigned!) */
			0,		/* Total Minutes (unsigned!) */
			-1,		/* Total Hours */
			0,		/* Cloaked Seconds */
			0,		/* Cloaked Minutes */
			-1,		/* Cloaked Hours */
			-1,		/* Health Damage */
			-1,		/* Armour Damage */
			-1,		/* Average Speed */
			-1,		/* Field Charge Used */
			-1,		/* Head Shot Percentage */
			{		/* Padding */
				0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,
			},
		},
		CHEATMODE_NONACTIVE,	/* Cheat to activate */
	},
	{
		{	/* Gateway / Easy */
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Creatures Killed */
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Creatures decapitated*/
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Trophies / Live Head Bites */
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Dead Head Bites */
			-1,		/* Shots Fired */
			-1,		/* Accuracy */
			-1,		/* Spotted */
			0,		/* Total Seconds (unsigned!) */
			0,		/* Total Minutes (unsigned!) */
			-1,		/* Total Hours */
			0,		/* Cloaked Seconds */
			0,		/* Cloaked Minutes */
			-1,		/* Cloaked Hours */
			-1,		/* Health Damage */
			-1,		/* Armour Damage */
			-1,		/* Average Speed */
			-1,		/* Field Charge Used */
			-1,		/* Head Shot Percentage */
			{		/* Padding */
				0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,
			},
		},
		CHEATMODE_NONACTIVE,	/* Cheat to activate */
	},
	{
		{	/* Escape / Easy */
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Creatures Killed */
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Creatures decapitated*/
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Trophies / Live Head Bites */
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Dead Head Bites */
			-1,		/* Shots Fired */
			-1,		/* Accuracy */
			-1,		/* Spotted */
			0,		/* Total Seconds (unsigned!) */
			0,		/* Total Minutes (unsigned!) */
			-1,		/* Total Hours */
			0,		/* Cloaked Seconds */
			0,		/* Cloaked Minutes */
			-1,		/* Cloaked Hours */
			-1,		/* Health Damage */
			-1,		/* Armour Damage */
			-1,		/* Average Speed */
			-1,		/* Field Charge Used */
			-1,		/* Head Shot Percentage */
			{		/* Padding */
				0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,
			},
		},
		CHEATMODE_NONACTIVE,	/* Cheat to activate */
	},
	{
		{	/* Earthbound / Easy */
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Creatures Killed */
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Creatures decapitated*/
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Trophies / Live Head Bites */
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Dead Head Bites */
			-1,		/* Shots Fired */
			-1,		/* Accuracy */
			-1,		/* Spotted */
			0,		/* Total Seconds (unsigned!) */
			0,		/* Total Minutes (unsigned!) */
			-1,		/* Total Hours */
			0,		/* Cloaked Seconds */
			0,		/* Cloaked Minutes */
			-1,		/* Cloaked Hours */
			-1,		/* Health Damage */
			-1,		/* Armour Damage */
			-1,		/* Average Speed */
			-1,		/* Field Charge Used */
			-1,		/* Head Shot Percentage */
			{		/* Padding */
				0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,
			},
		},
		CHEATMODE_NONACTIVE,	/* Cheat to activate */
	},
	{
		{	/* Sewer / Easy */
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Creatures Killed */
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Creatures decapitated*/
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Trophies / Live Head Bites */
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Dead Head Bites */
			-1,		/* Shots Fired */
			-1,		/* Accuracy */
			-1,		/* Spotted */
			0,		/* Total Seconds (unsigned!) */
			0,		/* Total Minutes (unsigned!) */
			-1,		/* Total Hours */
			0,		/* Cloaked Seconds */
			0,		/* Cloaked Minutes */
			-1,		/* Cloaked Hours */
			-1,		/* Health Damage */
			-1,		/* Armour Damage */
			-1,		/* Average Speed */
			-1,		/* Field Charge Used */
			-1,		/* Head Shot Percentage */
			{		/* Padding */
				0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,
			},
		},
		CHEATMODE_NONACTIVE,	/* Cheat to activate */
	},
	{
		{	/* Massacre / Easy */
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Creatures Killed */
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Creatures decapitated*/
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Trophies / Live Head Bites */
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Dead Head Bites */
			-1,		/* Shots Fired */
			-1,		/* Accuracy */
			-1,		/* Spotted */
			0,		/* Total Seconds (unsigned!) */
			0,		/* Total Minutes (unsigned!) */
			-1,		/* Total Hours */
			0,		/* Cloaked Seconds */
			0,		/* Cloaked Minutes */
			-1,		/* Cloaked Hours */
			-1,		/* Health Damage */
			-1,		/* Armour Damage */
			-1,		/* Average Speed */
			-1,		/* Field Charge Used */
			-1,		/* Head Shot Percentage */
			{		/* Padding */
				0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,
			},
		},
		CHEATMODE_NONACTIVE,	/* Cheat to activate */
	},
	{
		{	/* Statue / Easy */
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Creatures Killed */
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Creatures decapitated*/
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Trophies / Live Head Bites */
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Dead Head Bites */
			-1,		/* Shots Fired */
			-1,		/* Accuracy */
			-1,		/* Spotted */
			0,		/* Total Seconds (unsigned!) */
			0,		/* Total Minutes (unsigned!) */
			-1,		/* Total Hours */
			0,		/* Cloaked Seconds */
			0,		/* Cloaked Minutes */
			-1,		/* Cloaked Hours */
			-1,		/* Health Damage */
			-1,		/* Armour Damage */
			-1,		/* Average Speed */
			-1,		/* Field Charge Used */
			-1,		/* Head Shot Percentage */
			{		/* Padding */
				0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,
			},
		},
		CHEATMODE_NONACTIVE,	/* Cheat to activate */
	},
	{
		{	/* Jockey / Easy */
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Creatures Killed */
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Creatures decapitated*/
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Trophies / Live Head Bites */
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Dead Head Bites */
			-1,		/* Shots Fired */
			-1,		/* Accuracy */
			-1,		/* Spotted */
			0,		/* Total Seconds (unsigned!) */
			0,		/* Total Minutes (unsigned!) */
			-1,		/* Total Hours */
			0,		/* Cloaked Seconds */
			0,		/* Cloaked Minutes */
			-1,		/* Cloaked Hours */
			-1,		/* Health Damage */
			-1,		/* Armour Damage */
			-1,		/* Average Speed */
			-1,		/* Field Charge Used */
			-1,		/* Head Shot Percentage */
			{		/* Padding */
				0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,
			},
		},
		CHEATMODE_NONACTIVE,	/* Cheat to activate */
	},
	{
		{	/* Hive / Easy */
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Creatures Killed */
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Creatures decapitated*/
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Trophies / Live Head Bites */
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Dead Head Bites */
			-1,		/* Shots Fired */
			-1,		/* Accuracy */
			-1,		/* Spotted */
			0,		/* Total Seconds (unsigned!) */
			0,		/* Total Minutes (unsigned!) */
			-1,		/* Total Hours */
			0,		/* Cloaked Seconds */
			0,		/* Cloaked Minutes */
			-1,		/* Cloaked Hours */
			-1,		/* Health Damage */
			-1,		/* Armour Damage */
			-1,		/* Average Speed */
			-1,		/* Field Charge Used */
			-1,		/* Head Shot Percentage */
			{		/* Padding */
				0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,
			},
		},
		CHEATMODE_NONACTIVE,	/* Cheat to activate */
	},
	{
		{	/* Invasion_A / Easy */
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Creatures Killed */
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Creatures decapitated*/
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Trophies / Live Head Bites */
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Dead Head Bites */
			-1,		/* Shots Fired */
			-1,		/* Accuracy */
			-1,		/* Spotted */
			0,		/* Total Seconds (unsigned!) */
			0,		/* Total Minutes (unsigned!) */
			-1,		/* Total Hours */
			0,		/* Cloaked Seconds */
			0,		/* Cloaked Minutes */
			-1,		/* Cloaked Hours */
			-1,		/* Health Damage */
			-1,		/* Armour Damage */
			-1,		/* Average Speed */
			-1,		/* Field Charge Used */
			-1,		/* Head Shot Percentage */
			{		/* Padding */
				0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,
			},
		},
		CHEATMODE_NONACTIVE,	/* Cheat to activate */
	},
	{
		{	/* Derelict_A / Easy */
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Creatures Killed */
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Creatures decapitated*/
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Trophies / Live Head Bites */
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Dead Head Bites */
			-1,		/* Shots Fired */
			-1,		/* Accuracy */
			-1,		/* Spotted */
			0,		/* Total Seconds (unsigned!) */
			0,		/* Total Minutes (unsigned!) */
			-1,		/* Total Hours */
			0,		/* Cloaked Seconds */
			0,		/* Cloaked Minutes */
			-1,		/* Cloaked Hours */
			-1,		/* Health Damage */
			-1,		/* Armour Damage */
			-1,		/* Average Speed */
			-1,		/* Field Charge Used */
			-1,		/* Head Shot Percentage */
			{		/* Padding */
				0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,
			},
		},
		CHEATMODE_NONACTIVE,	/* Cheat to activate */
	},
	{
		{	/* Tyrago_A / Easy */
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Creatures Killed */
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Creatures decapitated*/
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Trophies / Live Head Bites */
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Dead Head Bites */
			-1,		/* Shots Fired */
			-1,		/* Accuracy */
			-1,		/* Spotted */
			0,		/* Total Seconds (unsigned!) */
			0,		/* Total Minutes (unsigned!) */
			-1,		/* Total Hours */
			0,		/* Cloaked Seconds */
			0,		/* Cloaked Minutes */
			-1,		/* Cloaked Hours */
			-1,		/* Health Damage */
			-1,		/* Armour Damage */
			-1,		/* Average Speed */
			-1,		/* Field Charge Used */
			-1,		/* Head Shot Percentage */
			{		/* Padding */
				0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,
			},
		},
		CHEATMODE_NONACTIVE,	/* Cheat to activate */
	},
	{
		{	/* Fury161_A / Easy */
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Creatures Killed */
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Creatures decapitated*/
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Trophies / Live Head Bites */
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Dead Head Bites */
			-1,		/* Shots Fired */
			-1,		/* Accuracy */
			-1,		/* Spotted */
			0,		/* Total Seconds (unsigned!) */
			0,		/* Total Minutes (unsigned!) */
			-1,		/* Total Hours */
			0,		/* Cloaked Seconds */
			0,		/* Cloaked Minutes */
			-1,		/* Cloaked Hours */
			-1,		/* Health Damage */
			-1,		/* Armour Damage */
			-1,		/* Average Speed */
			-1,		/* Field Charge Used */
			-1,		/* Head Shot Percentage */
			{		/* Padding */
				0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,
			},
		},
		CHEATMODE_NONACTIVE,	/* Cheat to activate */
	},
	{
		{	/* Caverns_A / Easy */
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Creatures Killed */
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Creatures decapitated*/
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Trophies / Live Head Bites */
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Dead Head Bites */
			-1,		/* Shots Fired */
			-1,		/* Accuracy */
			-1,		/* Spotted */
			0,		/* Total Seconds (unsigned!) */
			0,		/* Total Minutes (unsigned!) */
			-1,		/* Total Hours */
			0,		/* Cloaked Seconds */
			0,		/* Cloaked Minutes */
			-1,		/* Cloaked Hours */
			-1,		/* Health Damage */
			-1,		/* Armour Damage */
			-1,		/* Average Speed */
			-1,		/* Field Charge Used */
			-1,		/* Head Shot Percentage */
			{		/* Padding */
				0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,
			},
		},
		CHEATMODE_NONACTIVE,	/* Cheat to activate */
	},
	{
		{	/* Invasion_P / Easy */
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Creatures Killed */
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Creatures decapitated*/
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Trophies / Live Head Bites */
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Dead Head Bites */
			-1,		/* Shots Fired */
			-1,		/* Accuracy */
			-1,		/* Spotted */
			0,		/* Total Seconds (unsigned!) */
			0,		/* Total Minutes (unsigned!) */
			-1,		/* Total Hours */
			0,		/* Cloaked Seconds */
			0,		/* Cloaked Minutes */
			-1,		/* Cloaked Hours */
			-1,		/* Health Damage */
			-1,		/* Armour Damage */
			-1,		/* Average Speed */
			-1,		/* Field Charge Used */
			-1,		/* Head Shot Percentage */
			{		/* Padding */
				0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,
			},
		},
		CHEATMODE_NONACTIVE,	/* Cheat to activate */
	},
	{
		{	/* Tyrago_P / Easy */
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Creatures Killed */
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Creatures decapitated*/
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Trophies / Live Head Bites */
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Dead Head Bites */
			-1,		/* Shots Fired */
			-1,		/* Accuracy */
			-1,		/* Spotted */
			0,		/* Total Seconds (unsigned!) */
			0,		/* Total Minutes (unsigned!) */
			-1,		/* Total Hours */
			0,		/* Cloaked Seconds */
			0,		/* Cloaked Minutes */
			-1,		/* Cloaked Hours */
			-1,		/* Health Damage */
			-1,		/* Armour Damage */
			-1,		/* Average Speed */
			-1,		/* Field Charge Used */
			-1,		/* Head Shot Percentage */
			{		/* Padding */
				0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,
			},
		},
		CHEATMODE_NONACTIVE,	/* Cheat to activate */
	},
	{
		{	/* Temple_P / Easy */
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Creatures Killed */
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Creatures decapitated*/
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Trophies / Live Head Bites */
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Dead Head Bites */
			-1,		/* Shots Fired */
			-1,		/* Accuracy */
			-1,		/* Spotted */
			0,		/* Total Seconds (unsigned!) */
			0,		/* Total Minutes (unsigned!) */
			-1,		/* Total Hours */
			0,		/* Cloaked Seconds */
			0,		/* Cloaked Minutes */
			-1,		/* Cloaked Hours */
			-1,		/* Health Damage */
			-1,		/* Armour Damage */
			-1,		/* Average Speed */
			-1,		/* Field Charge Used */
			-1,		/* Head Shot Percentage */
			{		/* Padding */
				0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,
			},
		},
		CHEATMODE_NONACTIVE,	/* Cheat to activate */
	},
	{
		{	/* Escape_P / Easy */
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Creatures Killed */
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Creatures decapitated*/
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Trophies / Live Head Bites */
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Dead Head Bites */
			-1,		/* Shots Fired */
			-1,		/* Accuracy */
			-1,		/* Spotted */
			0,		/* Total Seconds (unsigned!) */
			0,		/* Total Minutes (unsigned!) */
			-1,		/* Total Hours */
			0,		/* Cloaked Seconds */
			0,		/* Cloaked Minutes */
			-1,		/* Cloaked Hours */
			-1,		/* Health Damage */
			-1,		/* Armour Damage */
			-1,		/* Average Speed */
			-1,		/* Field Charge Used */
			-1,		/* Head Shot Percentage */
			{		/* Padding */
				0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,
			},
		},
		CHEATMODE_NONACTIVE,	/* Cheat to activate */
	},
	{
		{	/* Earthbound_P / Easy */
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Creatures Killed */
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Creatures decapitated*/
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Trophies / Live Head Bites */
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Dead Head Bites */
			-1,		/* Shots Fired */
			-1,		/* Accuracy */
			-1,		/* Spotted */
			0,		/* Total Seconds (unsigned!) */
			0,		/* Total Minutes (unsigned!) */
			-1,		/* Total Hours */
			0,		/* Cloaked Seconds */
			0,		/* Cloaked Minutes */
			-1,		/* Cloaked Hours */
			-1,		/* Health Damage */
			-1,		/* Armour Damage */
			-1,		/* Average Speed */
			-1,		/* Field Charge Used */
			-1,		/* Head Shot Percentage */
			{		/* Padding */
				0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,
			},
		},
		CHEATMODE_NONACTIVE,	/* Cheat to activate */
	},
	{
		{	/* Waterfall_M / Easy */
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Creatures Killed */
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Creatures decapitated*/
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Trophies / Live Head Bites */
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Dead Head Bites */
			-1,		/* Shots Fired */
			-1,		/* Accuracy */
			-1,		/* Spotted */
			0,		/* Total Seconds (unsigned!) */
			0,		/* Total Minutes (unsigned!) */
			-1,		/* Total Hours */
			0,		/* Cloaked Seconds */
			0,		/* Cloaked Minutes */
			-1,		/* Cloaked Hours */
			-1,		/* Health Damage */
			-1,		/* Armour Damage */
			-1,		/* Average Speed */
			-1,		/* Field Charge Used */
			-1,		/* Head Shot Percentage */
			{		/* Padding */
				0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,
			},
		},
		CHEATMODE_NONACTIVE,	/* Cheat to activate */
	},
	{
		{	/* Vaults_M / Easy */
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Creatures Killed */
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Creatures decapitated*/
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Trophies / Live Head Bites */
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Dead Head Bites */
			-1,		/* Shots Fired */
			-1,		/* Accuracy */
			-1,		/* Spotted */
			0,		/* Total Seconds (unsigned!) */
			0,		/* Total Minutes (unsigned!) */
			-1,		/* Total Hours */
			0,		/* Cloaked Seconds */
			0,		/* Cloaked Minutes */
			-1,		/* Cloaked Hours */
			-1,		/* Health Damage */
			-1,		/* Armour Damage */
			-1,		/* Average Speed */
			-1,		/* Field Charge Used */
			-1,		/* Head Shot Percentage */
			{		/* Padding */
				0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,
			},
		},
		CHEATMODE_NONACTIVE,	/* Cheat to activate */
	},
	{
		{	/* Feraco_M / Easy */
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Creatures Killed */
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Creatures decapitated*/
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Trophies / Live Head Bites */
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Dead Head Bites */
			-1,		/* Shots Fired */
			-1,		/* Accuracy */
			-1,		/* Spotted */
			0,		/* Total Seconds (unsigned!) */
			0,		/* Total Minutes (unsigned!) */
			-1,		/* Total Hours */
			0,		/* Cloaked Seconds */
			0,		/* Cloaked Minutes */
			-1,		/* Cloaked Hours */
			-1,		/* Health Damage */
			-1,		/* Armour Damage */
			-1,		/* Average Speed */
			-1,		/* Field Charge Used */
			-1,		/* Head Shot Percentage */
			{		/* Padding */
				0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,
			},
		},
		CHEATMODE_NONACTIVE,	/* Cheat to activate */
	},
	{
		{	/* Temple_M / Easy */
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Creatures Killed */
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Creatures decapitated*/
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Trophies / Live Head Bites */
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Dead Head Bites */
			-1,		/* Shots Fired */
			-1,		/* Accuracy */
			-1,		/* Spotted */
			0,		/* Total Seconds (unsigned!) */
			0,		/* Total Minutes (unsigned!) */
			-1,		/* Total Hours */
			0,		/* Cloaked Seconds */
			0,		/* Cloaked Minutes */
			-1,		/* Cloaked Hours */
			-1,		/* Health Damage */
			-1,		/* Armour Damage */
			-1,		/* Average Speed */
			-1,		/* Field Charge Used */
			-1,		/* Head Shot Percentage */
			{		/* Padding */
				0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,
			},
		},
		CHEATMODE_NONACTIVE,	/* Cheat to activate */
	},
	{
		{	/* Gateway_M / Easy */
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Creatures Killed */
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Creatures decapitated*/
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Trophies / Live Head Bites */
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Dead Head Bites */
			-1,		/* Shots Fired */
			-1,		/* Accuracy */
			-1,		/* Spotted */
			0,		/* Total Seconds (unsigned!) */
			0,		/* Total Minutes (unsigned!) */
			-1,		/* Total Hours */
			0,		/* Cloaked Seconds */
			0,		/* Cloaked Minutes */
			-1,		/* Cloaked Hours */
			-1,		/* Health Damage */
			-1,		/* Armour Damage */
			-1,		/* Average Speed */
			-1,		/* Field Charge Used */
			-1,		/* Head Shot Percentage */
			{		/* Padding */
				0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,
			},
		},
		CHEATMODE_NONACTIVE,	/* Cheat to activate */
	},
	{
		{	/* Kens_Coop / Easy */
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Creatures Killed */
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Creatures decapitated*/
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Trophies / Live Head Bites */
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Dead Head Bites */
			-1,		/* Shots Fired */
			-1,		/* Accuracy */
			-1,		/* Spotted */
			0,		/* Total Seconds (unsigned!) */
			0,		/* Total Minutes (unsigned!) */
			-1,		/* Total Hours */
			0,		/* Cloaked Seconds */
			0,		/* Cloaked Minutes */
			-1,		/* Cloaked Hours */
			-1,		/* Health Damage */
			-1,		/* Armour Damage */
			-1,		/* Average Speed */
			-1,		/* Field Charge Used */
			-1,		/* Head Shot Percentage */
			{		/* Padding */
				0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,
			},
		},
		CHEATMODE_NONACTIVE,	/* Cheat to activate */
	},
	{
		{	/* Hive_Coop / Easy */
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Creatures Killed */
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Creatures decapitated*/
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Trophies / Live Head Bites */
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Dead Head Bites */
			-1,		/* Shots Fired */
			-1,		/* Accuracy */
			-1,		/* Spotted */
			0,		/* Total Seconds (unsigned!) */
			0,		/* Total Minutes (unsigned!) */
			-1,		/* Total Hours */
			0,		/* Cloaked Seconds */
			0,		/* Cloaked Minutes */
			-1,		/* Cloaked Hours */
			-1,		/* Health Damage */
			-1,		/* Armour Damage */
			-1,		/* Average Speed */
			-1,		/* Field Charge Used */
			-1,		/* Head Shot Percentage */
			{		/* Padding */
				0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,
			},
		},
		CHEATMODE_NONACTIVE,	/* Cheat to activate */
	},
	{
		{	/* Trapped_Coop / Easy */
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Creatures Killed */
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Creatures decapitated*/
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Trophies / Live Head Bites */
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Dead Head Bites */
			-1,		/* Shots Fired */
			-1,		/* Accuracy */
			-1,		/* Spotted */
			0,		/* Total Seconds (unsigned!) */
			0,		/* Total Minutes (unsigned!) */
			-1,		/* Total Hours */
			0,		/* Cloaked Seconds */
			0,		/* Cloaked Minutes */
			-1,		/* Cloaked Hours */
			-1,		/* Health Damage */
			-1,		/* Armour Damage */
			-1,		/* Average Speed */
			-1,		/* Field Charge Used */
			-1,		/* Head Shot Percentage */
			{		/* Padding */
				0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,
			},
		},
		CHEATMODE_NONACTIVE,	/* Cheat to activate */
	},
	{
		{	/* Als_DM_Coop / Easy */
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Creatures Killed */
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Creatures decapitated*/
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Trophies / Live Head Bites */
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Dead Head Bites */
			-1,		/* Shots Fired */
			-1,		/* Accuracy */
			-1,		/* Spotted */
			0,		/* Total Seconds (unsigned!) */
			0,		/* Total Minutes (unsigned!) */
			-1,		/* Total Hours */
			0,		/* Cloaked Seconds */
			0,		/* Cloaked Minutes */
			-1,		/* Cloaked Hours */
			-1,		/* Health Damage */
			-1,		/* Armour Damage */
			-1,		/* Average Speed */
			-1,		/* Field Charge Used */
			-1,		/* Head Shot Percentage */
			{		/* Padding */
				0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,
			},
		},
		CHEATMODE_NONACTIVE,	/* Cheat to activate */
	},
	{
		{	/* E3DemoSP / Easy */
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Creatures Killed */
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Creatures decapitated*/
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Trophies / Live Head Bites */
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Dead Head Bites */
			-1,		/* Shots Fired */
			-1,		/* Accuracy */
			-1,		/* Spotted */
			0,		/* Total Seconds (unsigned!) */
			0,		/* Total Minutes (unsigned!) */
			-1,		/* Total Hours */
			0,		/* Cloaked Seconds */
			0,		/* Cloaked Minutes */
			-1,		/* Cloaked Hours */
			-1,		/* Health Damage */
			-1,		/* Armour Damage */
			-1,		/* Average Speed */
			-1,		/* Field Charge Used */
			-1,		/* Head Shot Percentage */
			{		/* Padding */
				0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,
			},
		},
		CHEATMODE_NONACTIVE,	/* Cheat to activate */
	},
},

{
	{
		{	/* Derelict / Medium */
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Creatures Killed */
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Creatures decapitated*/
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Trophies / Live Head Bites */
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Dead Head Bites */
			-1,		/* Shots Fired */
			-1,		/* Accuracy */
			-1,		/* Spotted */
			0,		/* Total Seconds (unsigned!) */
			0,		/* Total Minutes (unsigned!) */
			-1,		/* Total Hours */
			0,		/* Cloaked Seconds */
			0,		/* Cloaked Minutes */
			-1,		/* Cloaked Hours */
			-1,		/* Health Damage */
			-1,		/* Armour Damage */
			-1,		/* Average Speed */
			-1,		/* Field Charge Used */
			80,		/* Head Shot Percentage */
			{		/* Padding */
				0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,
			},
		},
		CHEATMODE_JOHNWOO,	/* Cheat to activate */
	},
	{
		{	/* Colony / Medium */
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Creatures Killed */
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Creatures decapitated*/
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Trophies / Live Head Bites */
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Dead Head Bites */
			-1,		/* Shots Fired */
			40,		/* Accuracy */
			-1,		/* Spotted */
			0,		/* Total Seconds (unsigned!) */
			0,		/* Total Minutes (unsigned!) */
			-1,		/* Total Hours */
			0,		/* Cloaked Seconds */
			0,		/* Cloaked Minutes */
			-1,		/* Cloaked Hours */
			-1,		/* Health Damage */
			-1,		/* Armour Damage */
			-1,		/* Average Speed */
			-1,		/* Field Charge Used */
			-1,		/* Head Shot Percentage */
			{		/* Padding */
				0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,
			},
		},
		CHEATMODE_GRENADE,	/* Cheat to activate */
	},
	{
		{	/* Invasion / Medium */
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Creatures Killed */
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Creatures decapitated*/
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Trophies / Live Head Bites */
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Dead Head Bites */
			-1,		/* Shots Fired */
			-1,		/* Accuracy */
			-1,		/* Spotted */
			0,		/* Total Seconds (unsigned!) */
			4,		/* Total Minutes (unsigned!) */
			0,		/* Total Hours */
			0,		/* Cloaked Seconds */
			0,		/* Cloaked Minutes */
			-1,		/* Cloaked Hours */
			-1,		/* Health Damage */
			-1,		/* Armour Damage */
			-1,		/* Average Speed */
			-1,		/* Field Charge Used */
			-1,		/* Head Shot Percentage */
			{		/* Padding */
				0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,
			},
		},
		CHEATMODE_WARPSPEED,	/* Cheat to activate */
	},
	{
		{	/* Orbital / Medium */
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Creatures Killed */
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Creatures decapitated*/
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Trophies / Live Head Bites */
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Dead Head Bites */
			20,		/* Shots Fired */
			-1,		/* Accuracy */
			-1,		/* Spotted */
			0,		/* Total Seconds (unsigned!) */
			0,		/* Total Minutes (unsigned!) */
			-1,		/* Total Hours */
			0,		/* Cloaked Seconds */
			0,		/* Cloaked Minutes */
			-1,		/* Cloaked Hours */
			-1,		/* Health Damage */
			-1,		/* Armour Damage */
			-1,		/* Average Speed */
			-1,		/* Field Charge Used */
			-1,		/* Head Shot Percentage */
			{		/* Padding */
				0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,
			},
		},
		CHEATMODE_LANDOFTHEGIANTS,	/* Cheat to activate */
	},
	{
		{	/* Tyrago / Medium */
			{-1,32,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Creatures Killed */
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Creatures decapitated*/
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Trophies / Live Head Bites */
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Dead Head Bites */
			-1,		/* Shots Fired */
			-1,		/* Accuracy */
			-1,		/* Spotted */
			0,		/* Total Seconds (unsigned!) */
			0,		/* Total Minutes (unsigned!) */
			-1,		/* Total Hours */
			0,		/* Cloaked Seconds */
			0,		/* Cloaked Minutes */
			-1,		/* Cloaked Hours */
			-1,		/* Health Damage */
			-1,		/* Armour Damage */
			-1,		/* Average Speed */
			-1,		/* Field Charge Used */
			-1,		/* Head Shot Percentage */
			{		/* Padding */
				0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,
			},
		},
		CHEATMODE_SLUGTRAIL,	/* Cheat to activate */
	},
	{
		{	/* Hangar / Medium */
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Creatures Killed */
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Creatures decapitated*/
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Trophies / Live Head Bites */
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Dead Head Bites */
			-1,		/* Shots Fired */
			-1,		/* Accuracy */
			-1,		/* Spotted */
			0,		/* Total Seconds (unsigned!) */
			0,		/* Total Minutes (unsigned!) */
			-1,		/* Total Hours */
			0,		/* Cloaked Seconds */
			0,		/* Cloaked Minutes */
			-1,		/* Cloaked Hours */
			-1,		/* Health Damage */
			-1,		/* Armour Damage */
			-1,		/* Average Speed */
			-1,		/* Field Charge Used */
			-1,		/* Head Shot Percentage */
			{		/* Padding */
				0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,
			},
		},
		CHEATMODE_NONACTIVE,	/* Cheat to activate */
	},
	{
		{	/* Waterfall / Medium */
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Creatures Killed */
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Creatures decapitated*/
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Trophies / Live Head Bites */
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Dead Head Bites */
			-1,		/* Shots Fired */
			80,		/* Accuracy */
			-1,		/* Spotted */
			0,		/* Total Seconds (unsigned!) */
			0,		/* Total Minutes (unsigned!) */
			-1,		/* Total Hours */
			0,		/* Cloaked Seconds */
			0,		/* Cloaked Minutes */
			-1,		/* Cloaked Hours */
			-1,		/* Health Damage */
			-1,		/* Armour Damage */
			-1,		/* Average Speed */
			-1,		/* Field Charge Used */
			-1,		/* Head Shot Percentage */
			{		/* Padding */
				0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,
			},
		},
		CHEATMODE_PIGSTICKING,	/* Cheat to activate */
	},
	{
		{	/* Area52 / Medium */
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Creatures Killed */
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Creatures decapitated*/
			{-1,-1,-1,-1,-1,-1,-1,25,-1,-1,},	/* Trophies / Live Head Bites */
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Dead Head Bites */
			-1,		/* Shots Fired */
			-1,		/* Accuracy */
			-1,		/* Spotted */
			0,		/* Total Seconds (unsigned!) */
			0,		/* Total Minutes (unsigned!) */
			-1,		/* Total Hours */
			0,		/* Cloaked Seconds */
			0,		/* Cloaked Minutes */
			-1,		/* Cloaked Hours */
			-1,		/* Health Damage */
			-1,		/* Armour Damage */
			-1,		/* Average Speed */
			-1,		/* Field Charge Used */
			-1,		/* Head Shot Percentage */
			{		/* Padding */
				0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,
			},
		},
		CHEATMODE_SUPERGORE,	/* Cheat to activate */
	},
	{
		{	/* Vaults / Medium */
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Creatures Killed */
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Creatures decapitated*/
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Trophies / Live Head Bites */
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Dead Head Bites */
			-1,		/* Shots Fired */
			-1,		/* Accuracy */
			-1,		/* Spotted */
			0,		/* Total Seconds (unsigned!) */
			0,		/* Total Minutes (unsigned!) */
			-1,		/* Total Hours */
			0,		/* Cloaked Seconds */
			0,		/* Cloaked Minutes */
			-1,		/* Cloaked Hours */
			100,	/* Health Damage */
			-1,		/* Armour Damage */
			-1,		/* Average Speed */
			-1,		/* Field Charge Used */
			-1,		/* Head Shot Percentage */
			{		/* Padding */
				0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,
			},
		},
		CHEATMODE_DISCOINFERNO,	/* Cheat to activate */
	},
	{
		{	/* Fury161 / Medium */
			{-1,40,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Creatures Killed */
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Creatures decapitated*/
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Trophies / Live Head Bites */
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Dead Head Bites */
			-1,		/* Shots Fired */
			-1,		/* Accuracy */
			-1,		/* Spotted */
			0,		/* Total Seconds (unsigned!) */
			0,		/* Total Minutes (unsigned!) */
			-1,		/* Total Hours */
			0,		/* Cloaked Seconds */
			0,		/* Cloaked Minutes */
			-1,		/* Cloaked Hours */
			-1,		/* Health Damage */
			-1,		/* Armour Damage */
			-1,		/* Average Speed */
			-1,		/* Field Charge Used */
			-1,		/* Head Shot Percentage */
			{		/* Padding */
				0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,
			},
		},
		CHEATMODE_BALLSOFFIRE,	/* Cheat to activate */
	},
	{
		{	/* Caverns / Medium */
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Creatures Killed */
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Creatures decapitated*/
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Trophies / Live Head Bites */
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Dead Head Bites */
			-1,		/* Shots Fired */
			-1,		/* Accuracy */
			15,		/* Spotted */
			0,		/* Total Seconds (unsigned!) */
			0,		/* Total Minutes (unsigned!) */
			-1,		/* Total Hours */
			0,		/* Cloaked Seconds */
			0,		/* Cloaked Minutes */
			-1,		/* Cloaked Hours */
			-1,		/* Health Damage */
			-1,		/* Armour Damage */
			-1,		/* Average Speed */
			-1,		/* Field Charge Used */
			-1,		/* Head Shot Percentage */
			{		/* Padding */
				0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,
			},
		},
		CHEATMODE_RAINBOWBLOOD,	/* Cheat to activate */
	},
	{
		{	/* Battle / Medium */
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Creatures Killed */
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Creatures decapitated*/
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Trophies / Live Head Bites */
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Dead Head Bites */
			-1,		/* Shots Fired */
			-1,		/* Accuracy */
			-1,		/* Spotted */
			0,		/* Total Seconds (unsigned!) */
			0,		/* Total Minutes (unsigned!) */
			-1,		/* Total Hours */
			0,		/* Cloaked Seconds */
			0,		/* Cloaked Minutes */
			-1,		/* Cloaked Hours */
			-1,		/* Health Damage */
			-1,		/* Armour Damage */
			-1,		/* Average Speed */
			-1,		/* Field Charge Used */
			-1,		/* Head Shot Percentage */
			{		/* Padding */
				0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,
			},
		},
		CHEATMODE_NONACTIVE,	/* Cheat to activate */
	},
	{
		{	/* Feraco / Medium */
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Creatures Killed */
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Creatures decapitated*/
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Trophies / Live Head Bites */
			{-1,-1,-1,-1,-1,-1,-1,-1,15,-1,},	/* Dead Head Bites */
			-1,		/* Shots Fired */
			-1,		/* Accuracy */
			-1,		/* Spotted */
			0,		/* Total Seconds (unsigned!) */
			0,		/* Total Minutes (unsigned!) */
			-1,		/* Total Hours */
			0,		/* Cloaked Seconds */
			0,		/* Cloaked Minutes */
			-1,		/* Cloaked Hours */
			-1,		/* Health Damage */
			-1,		/* Armour Damage */
			-1,		/* Average Speed */
			-1,		/* Field Charge Used */
			-1,		/* Head Shot Percentage */
			{		/* Padding */
				0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,
			},
		},
		CHEATMODE_PIPECLEANER,	/* Cheat to activate */
	},
	{
		{	/* Temple / Medium */
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Creatures Killed */
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Creatures decapitated*/
			{-1,-1,-1,-1,-1,-1,-1,-1,10,-1,},	/* Trophies / Live Head Bites */
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Dead Head Bites */
			-1,		/* Shots Fired */
			-1,		/* Accuracy */
			-1,		/* Spotted */
			0,		/* Total Seconds (unsigned!) */
			0,		/* Total Minutes (unsigned!) */
			-1,		/* Total Hours */
			0,		/* Cloaked Seconds */
			0,		/* Cloaked Minutes */
			-1,		/* Cloaked Hours */
			-1,		/* Health Damage */
			-1,		/* Armour Damage */
			-1,		/* Average Speed */
			-1,		/* Field Charge Used */
			-1,		/* Head Shot Percentage */
			{		/* Padding */
				0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,
			},
		},
		CHEATMODE_SNIPERMUNCH,	/* Cheat to activate */
	},
	{
		{	/* Gateway / Medium */
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Creatures Killed */
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Creatures decapitated*/
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Trophies / Live Head Bites */
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Dead Head Bites */
			-1,		/* Shots Fired */
			-1,		/* Accuracy */
			-1,		/* Spotted */
			(30*ONE_FIXED),	/* Total Seconds (unsigned!) */
			4,		/* Total Minutes (unsigned!) */
			0,		/* Total Hours */
			0,		/* Cloaked Seconds */
			0,		/* Cloaked Minutes */
			-1,		/* Cloaked Hours */
			-1,		/* Health Damage */
			-1,		/* Armour Damage */
			9000,	/* Average Speed */
			-1,		/* Field Charge Used */
			-1,		/* Head Shot Percentage */
			{		/* Padding */
				0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,
			},
		},
		CHEATMODE_MOTIONBLUR,	/* Cheat to activate */
	},
	{
		{	/* Escape / Medium */
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Creatures Killed */
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Creatures decapitated*/
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Trophies / Live Head Bites */
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Dead Head Bites */
			-1,		/* Shots Fired */
			-1,		/* Accuracy */
			-1,		/* Spotted */
			0,		/* Total Seconds (unsigned!) */
			2,		/* Total Minutes (unsigned!) */
			0,		/* Total Hours */
			0,		/* Cloaked Seconds */
			0,		/* Cloaked Minutes */
			-1,		/* Cloaked Hours */
			-1,		/* Health Damage */
			-1,		/* Armour Damage */
			-1,		/* Average Speed */
			-1,		/* Field Charge Used */
			-1,		/* Head Shot Percentage */
			{		/* Padding */
				0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,
			},
		},
		CHEATMODE_NAUSEA,	/* Cheat to activate */
	},
	{
		{	/* Earthbound / Medium */
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Creatures Killed */
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Creatures decapitated*/
			{-1,-1,-1,-1,-1,-1, 1,-1,-1,-1,},	/* Trophies / Live Head Bites */
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Dead Head Bites */
			-1,		/* Shots Fired */
			-1,		/* Accuracy */
			-1,		/* Spotted */
			0,		/* Total Seconds (unsigned!) */
			0,		/* Total Minutes (unsigned!) */
			-1,		/* Total Hours */
			0,		/* Cloaked Seconds */
			0,		/* Cloaked Minutes */
			-1,		/* Cloaked Hours */
			-1,		/* Health Damage */
			-1,		/* Armour Damage */
			-1,		/* Average Speed */
			-1,		/* Field Charge Used */
			-1,		/* Head Shot Percentage */
			{		/* Padding */
				0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,
			},
		},
		CHEATMODE_MIRROR,	/* Cheat to activate */
	},
	{
		{	/* Sewer / Medium */
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Creatures Killed */
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Creatures decapitated*/
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Trophies / Live Head Bites */
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Dead Head Bites */
			-1,		/* Shots Fired */
			-1,		/* Accuracy */
			-1,		/* Spotted */
			0,		/* Total Seconds (unsigned!) */
			0,		/* Total Minutes (unsigned!) */
			-1,		/* Total Hours */
			0,		/* Cloaked Seconds */
			0,		/* Cloaked Minutes */
			-1,		/* Cloaked Hours */
			-1,		/* Health Damage */
			-1,		/* Armour Damage */
			-1,		/* Average Speed */
			-1,		/* Field Charge Used */
			-1,		/* Head Shot Percentage */
			{		/* Padding */
				0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,
			},
		},
		CHEATMODE_NONACTIVE,	/* Cheat to activate */
	},
	{
		{	/* Massacre / Medium */
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Creatures Killed */
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Creatures decapitated*/
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Trophies / Live Head Bites */
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Dead Head Bites */
			-1,		/* Shots Fired */
			-1,		/* Accuracy */
			-1,		/* Spotted */
			0,		/* Total Seconds (unsigned!) */
			0,		/* Total Minutes (unsigned!) */
			-1,		/* Total Hours */
			0,		/* Cloaked Seconds */
			0,		/* Cloaked Minutes */
			-1,		/* Cloaked Hours */
			-1,		/* Health Damage */
			-1,		/* Armour Damage */
			-1,		/* Average Speed */
			-1,		/* Field Charge Used */
			-1,		/* Head Shot Percentage */
			{		/* Padding */
				0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,
			},
		},
		CHEATMODE_NONACTIVE,	/* Cheat to activate */
	},
	{
		{	/* Statue / Medium */
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Creatures Killed */
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Creatures decapitated*/
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Trophies / Live Head Bites */
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Dead Head Bites */
			-1,		/* Shots Fired */
			-1,		/* Accuracy */
			-1,		/* Spotted */
			0,		/* Total Seconds (unsigned!) */
			0,		/* Total Minutes (unsigned!) */
			-1,		/* Total Hours */
			0,		/* Cloaked Seconds */
			0,		/* Cloaked Minutes */
			-1,		/* Cloaked Hours */
			-1,		/* Health Damage */
			-1,		/* Armour Damage */
			-1,		/* Average Speed */
			-1,		/* Field Charge Used */
			-1,		/* Head Shot Percentage */
			{		/* Padding */
				0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,
			},
		},
		CHEATMODE_NONACTIVE,	/* Cheat to activate */
	},
	{
		{	/* Jockey / Medium */
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Creatures Killed */
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Creatures decapitated*/
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Trophies / Live Head Bites */
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Dead Head Bites */
			-1,		/* Shots Fired */
			-1,		/* Accuracy */
			-1,		/* Spotted */
			0,		/* Total Seconds (unsigned!) */
			0,		/* Total Minutes (unsigned!) */
			-1,		/* Total Hours */
			0,		/* Cloaked Seconds */
			0,		/* Cloaked Minutes */
			-1,		/* Cloaked Hours */
			-1,		/* Health Damage */
			-1,		/* Armour Damage */
			-1,		/* Average Speed */
			-1,		/* Field Charge Used */
			-1,		/* Head Shot Percentage */
			{		/* Padding */
				0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,
			},
		},
		CHEATMODE_NONACTIVE,	/* Cheat to activate */
	},
	{
		{	/* Hive / Medium */
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Creatures Killed */
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Creatures decapitated*/
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Trophies / Live Head Bites */
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Dead Head Bites */
			-1,		/* Shots Fired */
			-1,		/* Accuracy */
			-1,		/* Spotted */
			0,		/* Total Seconds (unsigned!) */
			0,		/* Total Minutes (unsigned!) */
			-1,		/* Total Hours */
			0,		/* Cloaked Seconds */
			0,		/* Cloaked Minutes */
			-1,		/* Cloaked Hours */
			-1,		/* Health Damage */
			-1,		/* Armour Damage */
			-1,		/* Average Speed */
			-1,		/* Field Charge Used */
			-1,		/* Head Shot Percentage */
			{		/* Padding */
				0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,
			},
		},
		CHEATMODE_NONACTIVE,	/* Cheat to activate */
	},
	{
		{	/* Invasion_A / Medium */
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Creatures Killed */
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Creatures decapitated*/
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Trophies / Live Head Bites */
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Dead Head Bites */
			-1,		/* Shots Fired */
			-1,		/* Accuracy */
			-1,		/* Spotted */
			0,		/* Total Seconds (unsigned!) */
			0,		/* Total Minutes (unsigned!) */
			-1,		/* Total Hours */
			0,		/* Cloaked Seconds */
			0,		/* Cloaked Minutes */
			-1,		/* Cloaked Hours */
			-1,		/* Health Damage */
			-1,		/* Armour Damage */
			-1,		/* Average Speed */
			-1,		/* Field Charge Used */
			-1,		/* Head Shot Percentage */
			{		/* Padding */
				0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,
			},
		},
		CHEATMODE_NONACTIVE,	/* Cheat to activate */
	},
	{
		{	/* Derelict_A / Medium */
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Creatures Killed */
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Creatures decapitated*/
			{-1,-1,-1,-1,-1,-1,-1,20,-1,-1,},	/* Trophies / Live Head Bites */
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Dead Head Bites */
			-1,		/* Shots Fired */
			-1,		/* Accuracy */
			-1,		/* Spotted */
			0,		/* Total Seconds (unsigned!) */
			0,		/* Total Minutes (unsigned!) */
			-1,		/* Total Hours */
			0,		/* Cloaked Seconds */
			0,		/* Cloaked Minutes */
			-1,		/* Cloaked Hours */
			-1,		/* Health Damage */
			-1,		/* Armour Damage */
			-1,		/* Average Speed */
			-1,		/* Field Charge Used */
			-1,		/* Head Shot Percentage */
			{		/* Padding */
				0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,
			},
		},
		CHEATMODE_IMPOSSIBLEMISSION,	/* Cheat to activate */
	},
	{
		{	/* Tyrago_A / Medium */
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Creatures Killed */
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Creatures decapitated*/
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Trophies / Live Head Bites */
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Dead Head Bites */
			-1,		/* Shots Fired */
			-1,		/* Accuracy */
			-1,		/* Spotted */
			0,		/* Total Seconds (unsigned!) */
			0,		/* Total Minutes (unsigned!) */
			-1,		/* Total Hours */
			0,		/* Cloaked Seconds */
			0,		/* Cloaked Minutes */
			-1,		/* Cloaked Hours */
			-1,		/* Health Damage */
			-1,		/* Armour Damage */
			-1,		/* Average Speed */
			-1,		/* Field Charge Used */
			-1,		/* Head Shot Percentage */
			{		/* Padding */
				0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,
			},
		},
		CHEATMODE_NONACTIVE,	/* Cheat to activate */
	},
	{
		{	/* Fury161_A / Medium */
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Creatures Killed */
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Creatures decapitated*/
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Trophies / Live Head Bites */
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Dead Head Bites */
			-1,		/* Shots Fired */
			-1,		/* Accuracy */
			-1,		/* Spotted */
			0,		/* Total Seconds (unsigned!) */
			0,		/* Total Minutes (unsigned!) */
			-1,		/* Total Hours */
			0,		/* Cloaked Seconds */
			0,		/* Cloaked Minutes */
			-1,		/* Cloaked Hours */
			-1,		/* Health Damage */
			-1,		/* Armour Damage */
			-1,		/* Average Speed */
			-1,		/* Field Charge Used */
			-1,		/* Head Shot Percentage */
			{		/* Padding */
				0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,
			},
		},
		CHEATMODE_NONACTIVE,	/* Cheat to activate */
	},
	{
		{	/* Caverns_A / Medium */
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Creatures Killed */
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Creatures decapitated*/
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Trophies / Live Head Bites */
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Dead Head Bites */
			-1,		/* Shots Fired */
			-1,		/* Accuracy */
			-1,		/* Spotted */
			0,		/* Total Seconds (unsigned!) */
			0,		/* Total Minutes (unsigned!) */
			-1,		/* Total Hours */
			0,		/* Cloaked Seconds */
			0,		/* Cloaked Minutes */
			-1,		/* Cloaked Hours */
			-1,		/* Health Damage */
			-1,		/* Armour Damage */
			-1,		/* Average Speed */
			-1,		/* Field Charge Used */
			-1,		/* Head Shot Percentage */
			{		/* Padding */
				0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,
			},
		},
		CHEATMODE_NONACTIVE,	/* Cheat to activate */
	},
	{
		{	/* Invasion_P / Medium */
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Creatures Killed */
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Creatures decapitated*/
			{-1,15,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Trophies / Live Head Bites */
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Dead Head Bites */
			-1,		/* Shots Fired */
			-1,		/* Accuracy */
			-1,		/* Spotted */
			0,		/* Total Seconds (unsigned!) */
			0,		/* Total Minutes (unsigned!) */
			-1,		/* Total Hours */
			0,		/* Cloaked Seconds */
			0,		/* Cloaked Minutes */
			-1,		/* Cloaked Hours */
			-1,		/* Health Damage */
			-1,		/* Armour Damage */
			-1,		/* Average Speed */
			-1,		/* Field Charge Used */
			-1,		/* Head Shot Percentage */
			{		/* Padding */
				0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,
			},
		},
		CHEATMODE_TICKERTAPE,	/* Cheat to activate */
	},
	{
		{	/* Tyrago_P / Medium */
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Creatures Killed */
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Creatures decapitated*/
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Trophies / Live Head Bites */
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Dead Head Bites */
			-1,		/* Shots Fired */
			-1,		/* Accuracy */
			-1,		/* Spotted */
			0,		/* Total Seconds (unsigned!) */
			0,		/* Total Minutes (unsigned!) */
			-1,		/* Total Hours */
			0,		/* Cloaked Seconds */
			0,		/* Cloaked Minutes */
			-1,		/* Cloaked Hours */
			-1,		/* Health Damage */
			-1,		/* Armour Damage */
			-1,		/* Average Speed */
			-1,		/* Field Charge Used */
			-1,		/* Head Shot Percentage */
			{		/* Padding */
				0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,
			},
		},
		CHEATMODE_NONACTIVE,	/* Cheat to activate */
	},
	{
		{	/* Temple_P / Medium */
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Creatures Killed */
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Creatures decapitated*/
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Trophies / Live Head Bites */
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Dead Head Bites */
			-1,		/* Shots Fired */
			-1,		/* Accuracy */
			-1,		/* Spotted */
			0,		/* Total Seconds (unsigned!) */
			0,		/* Total Minutes (unsigned!) */
			-1,		/* Total Hours */
			0,		/* Cloaked Seconds */
			0,		/* Cloaked Minutes */
			-1,		/* Cloaked Hours */
			-1,		/* Health Damage */
			-1,		/* Armour Damage */
			-1,		/* Average Speed */
			-1,		/* Field Charge Used */
			-1,		/* Head Shot Percentage */
			{		/* Padding */
				0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,
			},
		},
		CHEATMODE_NONACTIVE,	/* Cheat to activate */
	},
	{
		{	/* Escape_P / Medium */
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Creatures Killed */
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Creatures decapitated*/
			{-1,-1,-1,-1,-1,-1,-1,-1,10,-1,},	/* Trophies / Live Head Bites */
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Dead Head Bites */
			-1,		/* Shots Fired */
			-1,		/* Accuracy */
			-1,		/* Spotted */
			0,		/* Total Seconds (unsigned!) */
			0,		/* Total Minutes (unsigned!) */
			-1,		/* Total Hours */
			0,		/* Cloaked Seconds */
			0,		/* Cloaked Minutes */
			-1,		/* Cloaked Hours */
			-1,		/* Health Damage */
			-1,		/* Armour Damage */
			-1,		/* Average Speed */
			-1,		/* Field Charge Used */
			-1,		/* Head Shot Percentage */
			{		/* Padding */
				0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,
			},
		},
		CHEATMODE_TRIPTASTIC,	/* Cheat to activate */
	},
	{
		{	/* Earthbound_P / Medium */
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Creatures Killed */
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Creatures decapitated*/
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Trophies / Live Head Bites */
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Dead Head Bites */
			-1,		/* Shots Fired */
			-1,		/* Accuracy */
			-1,		/* Spotted */
			0,		/* Total Seconds (unsigned!) */
			0,		/* Total Minutes (unsigned!) */
			-1,		/* Total Hours */
			0,		/* Cloaked Seconds */
			0,		/* Cloaked Minutes */
			-1,		/* Cloaked Hours */
			-1,		/* Health Damage */
			-1,		/* Armour Damage */
			-1,		/* Average Speed */
			-1,		/* Field Charge Used */
			-1,		/* Head Shot Percentage */
			{		/* Padding */
				0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,
			},
		},
		CHEATMODE_NONACTIVE,	/* Cheat to activate */
	},
	{
		{	/* Waterfall_M / Medium */
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Creatures Killed */
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Creatures decapitated*/
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Trophies / Live Head Bites */
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Dead Head Bites */
			-1,		/* Shots Fired */
			-1,		/* Accuracy */
			-1,		/* Spotted */
			0,		/* Total Seconds (unsigned!) */
			0,		/* Total Minutes (unsigned!) */
			-1,		/* Total Hours */
			0,		/* Cloaked Seconds */
			0,		/* Cloaked Minutes */
			-1,		/* Cloaked Hours */
			-1,		/* Health Damage */
			-1,		/* Armour Damage */
			-1,		/* Average Speed */
			-1,		/* Field Charge Used */
			-1,		/* Head Shot Percentage */
			{		/* Padding */
				0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,
			},
		},
		CHEATMODE_NONACTIVE,	/* Cheat to activate */
	},
	{
		{	/* Vaults_M / Medium */
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Creatures Killed */
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Creatures decapitated*/
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Trophies / Live Head Bites */
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Dead Head Bites */
			-1,		/* Shots Fired */
			-1,		/* Accuracy */
			-1,		/* Spotted */
			0,		/* Total Seconds (unsigned!) */
			0,		/* Total Minutes (unsigned!) */
			-1,		/* Total Hours */
			0,		/* Cloaked Seconds */
			0,		/* Cloaked Minutes */
			-1,		/* Cloaked Hours */
			60,		/* Health Damage */
			-1,		/* Armour Damage */
			-1,		/* Average Speed */
			-1,		/* Field Charge Used */
			-1,		/* Head Shot Percentage */
			{		/* Padding */
				0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,
			},
		},
		CHEATMODE_UNDERWATER,	/* Cheat to activate */
	},
	{
		{	/* Feraco_M / Medium */
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Creatures Killed */
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Creatures decapitated*/
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Trophies / Live Head Bites */
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Dead Head Bites */
			-1,		/* Shots Fired */
			-1,		/* Accuracy */
			-1,		/* Spotted */
			0,		/* Total Seconds (unsigned!) */
			0,		/* Total Minutes (unsigned!) */
			-1,		/* Total Hours */
			0,		/* Cloaked Seconds */
			0,		/* Cloaked Minutes */
			-1,		/* Cloaked Hours */
			-1,		/* Health Damage */
			-1,		/* Armour Damage */
			-1,		/* Average Speed */
			-1,		/* Field Charge Used */
			-1,		/* Head Shot Percentage */
			{		/* Padding */
				0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,
			},
		},
		CHEATMODE_NONACTIVE,	/* Cheat to activate */
	},
	{
		{	/* Temple_M / Medium */
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Creatures Killed */
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Creatures decapitated*/
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Trophies / Live Head Bites */
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Dead Head Bites */
			-1,		/* Shots Fired */
			-1,		/* Accuracy */
			-1,		/* Spotted */
			0,		/* Total Seconds (unsigned!) */
			0,		/* Total Minutes (unsigned!) */
			-1,		/* Total Hours */
			0,		/* Cloaked Seconds */
			0,		/* Cloaked Minutes */
			-1,		/* Cloaked Hours */
			100,	/* Health Damage */
			-1,		/* Armour Damage */
			-1,		/* Average Speed */
			-1,		/* Field Charge Used */
			-1,		/* Head Shot Percentage */
			{		/* Padding */
				0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,
			},
		},
		CHEATMODE_FREEFALL,	/* Cheat to activate */
	},
	{
		{	/* Gateway_M / Medium */
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Creatures Killed */
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Creatures decapitated*/
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Trophies / Live Head Bites */
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Dead Head Bites */
			-1,		/* Shots Fired */
			-1,		/* Accuracy */
			-1,		/* Spotted */
			0,		/* Total Seconds (unsigned!) */
			0,		/* Total Minutes (unsigned!) */
			-1,		/* Total Hours */
			0,		/* Cloaked Seconds */
			0,		/* Cloaked Minutes */
			-1,		/* Cloaked Hours */
			-1,		/* Health Damage */
			-1,		/* Armour Damage */
			-1,		/* Average Speed */
			-1,		/* Field Charge Used */
			-1,		/* Head Shot Percentage */
			{		/* Padding */
				0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,
			},
		},
		CHEATMODE_NONACTIVE,	/* Cheat to activate */
	},
	{
		{	/* Kens_Coop / Medium */
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Creatures Killed */
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Creatures decapitated*/
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Trophies / Live Head Bites */
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Dead Head Bites */
			-1,		/* Shots Fired */
			-1,		/* Accuracy */
			-1,		/* Spotted */
			0,		/* Total Seconds (unsigned!) */
			0,		/* Total Minutes (unsigned!) */
			-1,		/* Total Hours */
			0,		/* Cloaked Seconds */
			0,		/* Cloaked Minutes */
			-1,		/* Cloaked Hours */
			-1,		/* Health Damage */
			-1,		/* Armour Damage */
			-1,		/* Average Speed */
			-1,		/* Field Charge Used */
			-1,		/* Head Shot Percentage */
			{		/* Padding */
				0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,
			},
		},
		CHEATMODE_NONACTIVE,	/* Cheat to activate */
	},
	{
		{	/* Hive_Coop / Medium */
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Creatures Killed */
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Creatures decapitated*/
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Trophies / Live Head Bites */
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Dead Head Bites */
			-1,		/* Shots Fired */
			-1,		/* Accuracy */
			-1,		/* Spotted */
			0,		/* Total Seconds (unsigned!) */
			0,		/* Total Minutes (unsigned!) */
			-1,		/* Total Hours */
			0,		/* Cloaked Seconds */
			0,		/* Cloaked Minutes */
			-1,		/* Cloaked Hours */
			-1,		/* Health Damage */
			-1,		/* Armour Damage */
			-1,		/* Average Speed */
			-1,		/* Field Charge Used */
			-1,		/* Head Shot Percentage */
			{		/* Padding */
				0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,
			},
		},
		CHEATMODE_NONACTIVE,	/* Cheat to activate */
	},
	{
		{	/* Trapped_Coop / Medium */
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Creatures Killed */
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Creatures decapitated*/
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Trophies / Live Head Bites */
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Dead Head Bites */
			-1,		/* Shots Fired */
			-1,		/* Accuracy */
			-1,		/* Spotted */
			0,		/* Total Seconds (unsigned!) */
			0,		/* Total Minutes (unsigned!) */
			-1,		/* Total Hours */
			0,		/* Cloaked Seconds */
			0,		/* Cloaked Minutes */
			-1,		/* Cloaked Hours */
			-1,		/* Health Damage */
			-1,		/* Armour Damage */
			-1,		/* Average Speed */
			-1,		/* Field Charge Used */
			-1,		/* Head Shot Percentage */
			{		/* Padding */
				0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,
			},
		},
		CHEATMODE_NONACTIVE,	/* Cheat to activate */
	},
	{
		{	/* Als_DM_Coop / Medium */
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Creatures Killed */
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Creatures decapitated*/
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Trophies / Live Head Bites */
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Dead Head Bites */
			-1,		/* Shots Fired */
			-1,		/* Accuracy */
			-1,		/* Spotted */
			0,		/* Total Seconds (unsigned!) */
			0,		/* Total Minutes (unsigned!) */
			-1,		/* Total Hours */
			0,		/* Cloaked Seconds */
			0,		/* Cloaked Minutes */
			-1,		/* Cloaked Hours */
			-1,		/* Health Damage */
			-1,		/* Armour Damage */
			-1,		/* Average Speed */
			-1,		/* Field Charge Used */
			-1,		/* Head Shot Percentage */
			{		/* Padding */
				0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,
			},
		},
		CHEATMODE_NONACTIVE,	/* Cheat to activate */
	},
	{
		{	/* E3DemoSP / Medium */
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Creatures Killed */
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Creatures decapitated*/
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Trophies / Live Head Bites */
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Dead Head Bites */
			-1,		/* Shots Fired */
			-1,		/* Accuracy */
			-1,		/* Spotted */
			0,		/* Total Seconds (unsigned!) */
			0,		/* Total Minutes (unsigned!) */
			-1,		/* Total Hours */
			0,		/* Cloaked Seconds */
			0,		/* Cloaked Minutes */
			-1,		/* Cloaked Hours */
			-1,		/* Health Damage */
			-1,		/* Armour Damage */
			-1,		/* Average Speed */
			-1,		/* Field Charge Used */
			-1,		/* Head Shot Percentage */
			{		/* Padding */
				0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,
			},
		},
		CHEATMODE_NONACTIVE,	/* Cheat to activate */
	},
},

{
	{
		{	/* Derelict / Hard */
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Creatures Killed */
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Creatures decapitated*/
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Trophies / Live Head Bites */
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Dead Head Bites */
			-1,		/* Shots Fired */
			-1,		/* Accuracy */
			-1,		/* Spotted */
			0,		/* Total Seconds (unsigned!) */
			0,		/* Total Minutes (unsigned!) */
			-1,		/* Total Hours */
			0,		/* Cloaked Seconds */
			0,		/* Cloaked Minutes */
			-1,		/* Cloaked Hours */
			-1,		/* Health Damage */
			-1,		/* Armour Damage */
			-1,		/* Average Speed */
			-1,		/* Field Charge Used */
			80,		/* Head Shot Percentage */
			{		/* Padding */
				0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,
			},
		},
		CHEATMODE_JOHNWOO,	/* Cheat to activate */
	},
	{
		{	/* Colony / Hard */
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Creatures Killed */
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Creatures decapitated*/
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Trophies / Live Head Bites */
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Dead Head Bites */
			-1,		/* Shots Fired */
			40,		/* Accuracy */
			-1,		/* Spotted */
			0,		/* Total Seconds (unsigned!) */
			0,		/* Total Minutes (unsigned!) */
			-1,		/* Total Hours */
			0,		/* Cloaked Seconds */
			0,		/* Cloaked Minutes */
			-1,		/* Cloaked Hours */
			-1,		/* Health Damage */
			-1,		/* Armour Damage */
			-1,		/* Average Speed */
			-1,		/* Field Charge Used */
			-1,		/* Head Shot Percentage */
			{		/* Padding */
				0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,
			},
		},
		CHEATMODE_GRENADE,	/* Cheat to activate */
	},
	{
		{	/* Invasion / Hard */
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Creatures Killed */
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Creatures decapitated*/
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Trophies / Live Head Bites */
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Dead Head Bites */
			-1,		/* Shots Fired */
			-1,		/* Accuracy */
			-1,		/* Spotted */
			0,		/* Total Seconds (unsigned!) */
			4,		/* Total Minutes (unsigned!) */
			0,		/* Total Hours */
			0,		/* Cloaked Seconds */
			0,		/* Cloaked Minutes */
			-1,		/* Cloaked Hours */
			-1,		/* Health Damage */
			-1,		/* Armour Damage */
			-1,		/* Average Speed */
			-1,		/* Field Charge Used */
			-1,		/* Head Shot Percentage */
			{		/* Padding */
				0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,
			},
		},
		CHEATMODE_WARPSPEED,	/* Cheat to activate */
	},
	{
		{	/* Orbital / Hard */
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Creatures Killed */
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Creatures decapitated*/
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Trophies / Live Head Bites */
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Dead Head Bites */
			20,		/* Shots Fired */
			-1,		/* Accuracy */
			-1,		/* Spotted */
			0,		/* Total Seconds (unsigned!) */
			0,		/* Total Minutes (unsigned!) */
			-1,		/* Total Hours */
			0,		/* Cloaked Seconds */
			0,		/* Cloaked Minutes */
			-1,		/* Cloaked Hours */
			-1,		/* Health Damage */
			-1,		/* Armour Damage */
			-1,		/* Average Speed */
			-1,		/* Field Charge Used */
			-1,		/* Head Shot Percentage */
			{		/* Padding */
				0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,
			},
		},
		CHEATMODE_LANDOFTHEGIANTS,	/* Cheat to activate */
	},
	{
		{	/* Tyrago / Hard */
			{-1,32,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Creatures Killed */
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Creatures decapitated*/
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Trophies / Live Head Bites */
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Dead Head Bites */
			-1,		/* Shots Fired */
			-1,		/* Accuracy */
			-1,		/* Spotted */
			0,		/* Total Seconds (unsigned!) */
			0,		/* Total Minutes (unsigned!) */
			-1,		/* Total Hours */
			0,		/* Cloaked Seconds */
			0,		/* Cloaked Minutes */
			-1,		/* Cloaked Hours */
			-1,		/* Health Damage */
			-1,		/* Armour Damage */
			-1,		/* Average Speed */
			-1,		/* Field Charge Used */
			-1,		/* Head Shot Percentage */
			{		/* Padding */
				0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,
			},
		},
		CHEATMODE_SLUGTRAIL,	/* Cheat to activate */
	},
	{
		{	/* Hangar / Hard */
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Creatures Killed */
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Creatures decapitated*/
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Trophies / Live Head Bites */
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Dead Head Bites */
			-1,		/* Shots Fired */
			-1,		/* Accuracy */
			-1,		/* Spotted */
			0,		/* Total Seconds (unsigned!) */
			0,		/* Total Minutes (unsigned!) */
			-1,		/* Total Hours */
			0,		/* Cloaked Seconds */
			0,		/* Cloaked Minutes */
			-1,		/* Cloaked Hours */
			-1,		/* Health Damage */
			-1,		/* Armour Damage */
			-1,		/* Average Speed */
			-1,		/* Field Charge Used */
			-1,		/* Head Shot Percentage */
			{		/* Padding */
				0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,
			},
		},
		CHEATMODE_NONACTIVE,	/* Cheat to activate */
	},
	{
		{	/* Waterfall / Hard */
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Creatures Killed */
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Creatures decapitated*/
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Trophies / Live Head Bites */
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Dead Head Bites */
			-1,		/* Shots Fired */
			80,		/* Accuracy */
			-1,		/* Spotted */
			0,		/* Total Seconds (unsigned!) */
			0,		/* Total Minutes (unsigned!) */
			-1,		/* Total Hours */
			0,		/* Cloaked Seconds */
			0,		/* Cloaked Minutes */
			-1,		/* Cloaked Hours */
			-1,		/* Health Damage */
			-1,		/* Armour Damage */
			-1,		/* Average Speed */
			-1,		/* Field Charge Used */
			-1,		/* Head Shot Percentage */
			{		/* Padding */
				0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,
			},
		},
		CHEATMODE_PIGSTICKING,	/* Cheat to activate */
	},
	{
		{	/* Area52 / Hard */
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Creatures Killed */
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Creatures decapitated*/
			{-1,-1,-1,-1,-1,-1,-1,25,-1,-1,},	/* Trophies / Live Head Bites */
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Dead Head Bites */
			-1,		/* Shots Fired */
			-1,		/* Accuracy */
			-1,		/* Spotted */
			0,		/* Total Seconds (unsigned!) */
			0,		/* Total Minutes (unsigned!) */
			-1,		/* Total Hours */
			0,		/* Cloaked Seconds */
			0,		/* Cloaked Minutes */
			-1,		/* Cloaked Hours */
			-1,		/* Health Damage */
			-1,		/* Armour Damage */
			-1,		/* Average Speed */
			-1,		/* Field Charge Used */
			-1,		/* Head Shot Percentage */
			{		/* Padding */
				0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,
			},
		},
		CHEATMODE_SUPERGORE,	/* Cheat to activate */
	},
	{
		{	/* Vaults / Hard */
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Creatures Killed */
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Creatures decapitated*/
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Trophies / Live Head Bites */
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Dead Head Bites */
			-1,		/* Shots Fired */
			-1,		/* Accuracy */
			-1,		/* Spotted */
			0,		/* Total Seconds (unsigned!) */
			0,		/* Total Minutes (unsigned!) */
			-1,		/* Total Hours */
			0,		/* Cloaked Seconds */
			0,		/* Cloaked Minutes */
			-1,		/* Cloaked Hours */
			100,	/* Health Damage */
			-1,		/* Armour Damage */
			-1,		/* Average Speed */
			-1,		/* Field Charge Used */
			-1,		/* Head Shot Percentage */
			{		/* Padding */
				0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,
			},
		},
		CHEATMODE_DISCOINFERNO,	/* Cheat to activate */
	},
	{
		{	/* Fury161 / Hard */
			{-1,40,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Creatures Killed */
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Creatures decapitated*/
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Trophies / Live Head Bites */
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Dead Head Bites */
			-1,		/* Shots Fired */
			-1,		/* Accuracy */
			-1,		/* Spotted */
			0,		/* Total Seconds (unsigned!) */
			0,		/* Total Minutes (unsigned!) */
			-1,		/* Total Hours */
			0,		/* Cloaked Seconds */
			0,		/* Cloaked Minutes */
			-1,		/* Cloaked Hours */
			-1,		/* Health Damage */
			-1,		/* Armour Damage */
			-1,		/* Average Speed */
			-1,		/* Field Charge Used */
			-1,		/* Head Shot Percentage */
			{		/* Padding */
				0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,
			},
		},
		CHEATMODE_BALLSOFFIRE,	/* Cheat to activate */
	},
	{
		{	/* Caverns / Hard */
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Creatures Killed */
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Creatures decapitated*/
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Trophies / Live Head Bites */
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Dead Head Bites */
			-1,		/* Shots Fired */
			-1,		/* Accuracy */
			15,		/* Spotted */
			0,		/* Total Seconds (unsigned!) */
			0,		/* Total Minutes (unsigned!) */
			-1,		/* Total Hours */
			0,		/* Cloaked Seconds */
			0,		/* Cloaked Minutes */
			-1,		/* Cloaked Hours */
			-1,		/* Health Damage */
			-1,		/* Armour Damage */
			-1,		/* Average Speed */
			-1,		/* Field Charge Used */
			-1,		/* Head Shot Percentage */
			{		/* Padding */
				0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,
			},
		},
		CHEATMODE_RAINBOWBLOOD,	/* Cheat to activate */
	},
	{
		{	/* Battle / Hard */
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Creatures Killed */
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Creatures decapitated*/
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Trophies / Live Head Bites */
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Dead Head Bites */
			-1,		/* Shots Fired */
			-1,		/* Accuracy */
			-1,		/* Spotted */
			0,		/* Total Seconds (unsigned!) */
			0,		/* Total Minutes (unsigned!) */
			-1,		/* Total Hours */
			0,		/* Cloaked Seconds */
			0,		/* Cloaked Minutes */
			-1,		/* Cloaked Hours */
			-1,		/* Health Damage */
			-1,		/* Armour Damage */
			-1,		/* Average Speed */
			-1,		/* Field Charge Used */
			-1,		/* Head Shot Percentage */
			{		/* Padding */
				0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,
			},
		},
		CHEATMODE_NONACTIVE,	/* Cheat to activate */
	},
	{
		{	/* Feraco / Hard */
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Creatures Killed */
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Creatures decapitated*/
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Trophies / Live Head Bites */
			{-1,-1,-1,-1,-1,-1,-1,-1,15,-1,},	/* Dead Head Bites */
			-1,		/* Shots Fired */
			-1,		/* Accuracy */
			-1,		/* Spotted */
			0,		/* Total Seconds (unsigned!) */
			0,		/* Total Minutes (unsigned!) */
			-1,		/* Total Hours */
			0,		/* Cloaked Seconds */
			0,		/* Cloaked Minutes */
			-1,		/* Cloaked Hours */
			-1,		/* Health Damage */
			-1,		/* Armour Damage */
			-1,		/* Average Speed */
			-1,		/* Field Charge Used */
			-1,		/* Head Shot Percentage */
			{		/* Padding */
				0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,
			},
		},
		CHEATMODE_PIPECLEANER,	/* Cheat to activate */
	},
	{
		{	/* Temple / Hard */
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Creatures Killed */
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Creatures decapitated*/
			{-1,-1,-1,-1,-1,-1,-1,-1,10,-1,},	/* Trophies / Live Head Bites */
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Dead Head Bites */
			-1,		/* Shots Fired */
			-1,		/* Accuracy */
			-1,		/* Spotted */
			0,		/* Total Seconds (unsigned!) */
			0,		/* Total Minutes (unsigned!) */
			-1,		/* Total Hours */
			0,		/* Cloaked Seconds */
			0,		/* Cloaked Minutes */
			-1,		/* Cloaked Hours */
			-1,		/* Health Damage */
			-1,		/* Armour Damage */
			-1,		/* Average Speed */
			-1,		/* Field Charge Used */
			-1,		/* Head Shot Percentage */
			{		/* Padding */
				0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,
			},
		},
		CHEATMODE_SNIPERMUNCH,	/* Cheat to activate */
	},
	{
		{	/* Gateway / Hard */
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Creatures Killed */
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Creatures decapitated*/
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Trophies / Live Head Bites */
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Dead Head Bites */
			-1,		/* Shots Fired */
			-1,		/* Accuracy */
			-1,		/* Spotted */
			(30*ONE_FIXED),	/* Total Seconds (unsigned!) */
			4,		/* Total Minutes (unsigned!) */
			0,		/* Total Hours */
			0,		/* Cloaked Seconds */
			0,		/* Cloaked Minutes */
			-1,		/* Cloaked Hours */
			-1,		/* Health Damage */
			-1,		/* Armour Damage */
			9000,	/* Average Speed */
			-1,		/* Field Charge Used */
			-1,		/* Head Shot Percentage */
			{		/* Padding */
				0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,
			},
		},
		CHEATMODE_MOTIONBLUR,	/* Cheat to activate */
	},
	{
		{	/* Escape / Hard */
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Creatures Killed */
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Creatures decapitated*/
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Trophies / Live Head Bites */
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Dead Head Bites */
			-1,		/* Shots Fired */
			-1,		/* Accuracy */
			-1,		/* Spotted */
			0,		/* Total Seconds (unsigned!) */
			2,		/* Total Minutes (unsigned!) */
			0,		/* Total Hours */
			0,		/* Cloaked Seconds */
			0,		/* Cloaked Minutes */
			-1,		/* Cloaked Hours */
			-1,		/* Health Damage */
			-1,		/* Armour Damage */
			-1,		/* Average Speed */
			-1,		/* Field Charge Used */
			-1,		/* Head Shot Percentage */
			{		/* Padding */
				0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,
			},
		},
		CHEATMODE_NAUSEA,	/* Cheat to activate */
	},
	{
		{	/* Earthbound / Hard */
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Creatures Killed */
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Creatures decapitated*/
			{-1,-1,-1,-1,-1,-1, 1,-1,-1,-1,},	/* Trophies / Live Head Bites */
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Dead Head Bites */
			-1,		/* Shots Fired */
			-1,		/* Accuracy */
			-1,		/* Spotted */
			0,		/* Total Seconds (unsigned!) */
			0,		/* Total Minutes (unsigned!) */
			-1,		/* Total Hours */
			0,		/* Cloaked Seconds */
			0,		/* Cloaked Minutes */
			-1,		/* Cloaked Hours */
			-1,		/* Health Damage */
			-1,		/* Armour Damage */
			-1,		/* Average Speed */
			-1,		/* Field Charge Used */
			-1,		/* Head Shot Percentage */
			{		/* Padding */
				0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,
			},
		},
		CHEATMODE_MIRROR,	/* Cheat to activate */
	},
	{
		{	/* Sewer / Hard */
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Creatures Killed */
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Creatures decapitated*/
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Trophies / Live Head Bites */
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Dead Head Bites */
			-1,		/* Shots Fired */
			-1,		/* Accuracy */
			-1,		/* Spotted */
			0,		/* Total Seconds (unsigned!) */
			0,		/* Total Minutes (unsigned!) */
			-1,		/* Total Hours */
			0,		/* Cloaked Seconds */
			0,		/* Cloaked Minutes */
			-1,		/* Cloaked Hours */
			-1,		/* Health Damage */
			-1,		/* Armour Damage */
			-1,		/* Average Speed */
			-1,		/* Field Charge Used */
			-1,		/* Head Shot Percentage */
			{		/* Padding */
				0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,
			},
		},
		CHEATMODE_NONACTIVE,	/* Cheat to activate */
	},
	{
		{	/* Massacre / Hard */
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Creatures Killed */
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Creatures decapitated*/
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Trophies / Live Head Bites */
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Dead Head Bites */
			-1,		/* Shots Fired */
			-1,		/* Accuracy */
			-1,		/* Spotted */
			0,		/* Total Seconds (unsigned!) */
			0,		/* Total Minutes (unsigned!) */
			-1,		/* Total Hours */
			0,		/* Cloaked Seconds */
			0,		/* Cloaked Minutes */
			-1,		/* Cloaked Hours */
			-1,		/* Health Damage */
			-1,		/* Armour Damage */
			-1,		/* Average Speed */
			-1,		/* Field Charge Used */
			-1,		/* Head Shot Percentage */
			{		/* Padding */
				0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,
			},
		},
		CHEATMODE_NONACTIVE,	/* Cheat to activate */
	},
	{
		{	/* Statue / Hard */
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Creatures Killed */
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Creatures decapitated*/
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Trophies / Live Head Bites */
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Dead Head Bites */
			-1,		/* Shots Fired */
			-1,		/* Accuracy */
			-1,		/* Spotted */
			0,		/* Total Seconds (unsigned!) */
			0,		/* Total Minutes (unsigned!) */
			-1,		/* Total Hours */
			0,		/* Cloaked Seconds */
			0,		/* Cloaked Minutes */
			-1,		/* Cloaked Hours */
			-1,		/* Health Damage */
			-1,		/* Armour Damage */
			-1,		/* Average Speed */
			-1,		/* Field Charge Used */
			-1,		/* Head Shot Percentage */
			{		/* Padding */
				0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,
			},
		},
		CHEATMODE_NONACTIVE,	/* Cheat to activate */
	},
	{
		{	/* Jockey / Hard */
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Creatures Killed */
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Creatures decapitated*/
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Trophies / Live Head Bites */
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Dead Head Bites */
			-1,		/* Shots Fired */
			-1,		/* Accuracy */
			-1,		/* Spotted */
			0,		/* Total Seconds (unsigned!) */
			0,		/* Total Minutes (unsigned!) */
			-1,		/* Total Hours */
			0,		/* Cloaked Seconds */
			0,		/* Cloaked Minutes */
			-1,		/* Cloaked Hours */
			-1,		/* Health Damage */
			-1,		/* Armour Damage */
			-1,		/* Average Speed */
			-1,		/* Field Charge Used */
			-1,		/* Head Shot Percentage */
			{		/* Padding */
				0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,
			},
		},
		CHEATMODE_NONACTIVE,	/* Cheat to activate */
	},
	{
		{	/* Hive / Hard */
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Creatures Killed */
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Creatures decapitated*/
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Trophies / Live Head Bites */
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Dead Head Bites */
			-1,		/* Shots Fired */
			-1,		/* Accuracy */
			-1,		/* Spotted */
			0,		/* Total Seconds (unsigned!) */
			0,		/* Total Minutes (unsigned!) */
			-1,		/* Total Hours */
			0,		/* Cloaked Seconds */
			0,		/* Cloaked Minutes */
			-1,		/* Cloaked Hours */
			-1,		/* Health Damage */
			-1,		/* Armour Damage */
			-1,		/* Average Speed */
			-1,		/* Field Charge Used */
			-1,		/* Head Shot Percentage */
			{		/* Padding */
				0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,
			},
		},
		CHEATMODE_NONACTIVE,	/* Cheat to activate */
	},
	{
		{	/* Invasion_A / Hard */
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Creatures Killed */
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Creatures decapitated*/
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Trophies / Live Head Bites */
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Dead Head Bites */
			-1,		/* Shots Fired */
			-1,		/* Accuracy */
			-1,		/* Spotted */
			0,		/* Total Seconds (unsigned!) */
			0,		/* Total Minutes (unsigned!) */
			-1,		/* Total Hours */
			0,		/* Cloaked Seconds */
			0,		/* Cloaked Minutes */
			-1,		/* Cloaked Hours */
			-1,		/* Health Damage */
			-1,		/* Armour Damage */
			-1,		/* Average Speed */
			-1,		/* Field Charge Used */
			-1,		/* Head Shot Percentage */
			{		/* Padding */
				0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,
			},
		},
		CHEATMODE_NONACTIVE,	/* Cheat to activate */
	},
	{
		{	/* Derelict_A / Hard */
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Creatures Killed */
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Creatures decapitated*/
			{-1,-1,-1,-1,-1,-1,-1,20,-1,-1,},	/* Trophies / Live Head Bites */
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Dead Head Bites */
			-1,		/* Shots Fired */
			-1,		/* Accuracy */
			-1,		/* Spotted */
			0,		/* Total Seconds (unsigned!) */
			0,		/* Total Minutes (unsigned!) */
			-1,		/* Total Hours */
			0,		/* Cloaked Seconds */
			0,		/* Cloaked Minutes */
			-1,		/* Cloaked Hours */
			-1,		/* Health Damage */
			-1,		/* Armour Damage */
			-1,		/* Average Speed */
			-1,		/* Field Charge Used */
			-1,		/* Head Shot Percentage */
			{		/* Padding */
				0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,
			},
		},
		CHEATMODE_IMPOSSIBLEMISSION,	/* Cheat to activate */
	},
	{
		{	/* Tyrago_A / Hard */
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Creatures Killed */
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Creatures decapitated*/
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Trophies / Live Head Bites */
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Dead Head Bites */
			-1,		/* Shots Fired */
			-1,		/* Accuracy */
			-1,		/* Spotted */
			0,		/* Total Seconds (unsigned!) */
			0,		/* Total Minutes (unsigned!) */
			-1,		/* Total Hours */
			0,		/* Cloaked Seconds */
			0,		/* Cloaked Minutes */
			-1,		/* Cloaked Hours */
			-1,		/* Health Damage */
			-1,		/* Armour Damage */
			-1,		/* Average Speed */
			-1,		/* Field Charge Used */
			-1,		/* Head Shot Percentage */
			{		/* Padding */
				0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,
			},
		},
		CHEATMODE_NONACTIVE,	/* Cheat to activate */
	},
	{
		{	/* Fury161_A / Hard */
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Creatures Killed */
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Creatures decapitated*/
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Trophies / Live Head Bites */
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Dead Head Bites */
			-1,		/* Shots Fired */
			-1,		/* Accuracy */
			-1,		/* Spotted */
			0,		/* Total Seconds (unsigned!) */
			0,		/* Total Minutes (unsigned!) */
			-1,		/* Total Hours */
			0,		/* Cloaked Seconds */
			0,		/* Cloaked Minutes */
			-1,		/* Cloaked Hours */
			-1,		/* Health Damage */
			-1,		/* Armour Damage */
			-1,		/* Average Speed */
			-1,		/* Field Charge Used */
			-1,		/* Head Shot Percentage */
			{		/* Padding */
				0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,
			},
		},
		CHEATMODE_NONACTIVE,	/* Cheat to activate */
	},
	{
		{	/* Caverns_A / Hard */
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Creatures Killed */
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Creatures decapitated*/
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Trophies / Live Head Bites */
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Dead Head Bites */
			-1,		/* Shots Fired */
			-1,		/* Accuracy */
			-1,		/* Spotted */
			0,		/* Total Seconds (unsigned!) */
			0,		/* Total Minutes (unsigned!) */
			-1,		/* Total Hours */
			0,		/* Cloaked Seconds */
			0,		/* Cloaked Minutes */
			-1,		/* Cloaked Hours */
			-1,		/* Health Damage */
			-1,		/* Armour Damage */
			-1,		/* Average Speed */
			-1,		/* Field Charge Used */
			-1,		/* Head Shot Percentage */
			{		/* Padding */
				0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,
			},
		},
		CHEATMODE_NONACTIVE,	/* Cheat to activate */
	},
	{
		{	/* Invasion_P / Hard */
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Creatures Killed */
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Creatures decapitated*/
			{-1,15,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Trophies / Live Head Bites */
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Dead Head Bites */
			-1,		/* Shots Fired */
			-1,		/* Accuracy */
			-1,		/* Spotted */
			0,		/* Total Seconds (unsigned!) */
			0,		/* Total Minutes (unsigned!) */
			-1,		/* Total Hours */
			0,		/* Cloaked Seconds */
			0,		/* Cloaked Minutes */
			-1,		/* Cloaked Hours */
			-1,		/* Health Damage */
			-1,		/* Armour Damage */
			-1,		/* Average Speed */
			-1,		/* Field Charge Used */
			-1,		/* Head Shot Percentage */
			{		/* Padding */
				0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,
			},
		},
		CHEATMODE_TICKERTAPE,	/* Cheat to activate */
	},
	{
		{	/* Tyrago_P / Hard */
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Creatures Killed */
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Creatures decapitated*/
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Trophies / Live Head Bites */
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Dead Head Bites */
			-1,		/* Shots Fired */
			-1,		/* Accuracy */
			-1,		/* Spotted */
			0,		/* Total Seconds (unsigned!) */
			0,		/* Total Minutes (unsigned!) */
			-1,		/* Total Hours */
			0,		/* Cloaked Seconds */
			0,		/* Cloaked Minutes */
			-1,		/* Cloaked Hours */
			-1,		/* Health Damage */
			-1,		/* Armour Damage */
			-1,		/* Average Speed */
			-1,		/* Field Charge Used */
			-1,		/* Head Shot Percentage */
			{		/* Padding */
				0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,
			},
		},
		CHEATMODE_NONACTIVE,	/* Cheat to activate */
	},
	{
		{	/* Temple_P / Hard */
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Creatures Killed */
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Creatures decapitated*/
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Trophies / Live Head Bites */
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Dead Head Bites */
			-1,		/* Shots Fired */
			-1,		/* Accuracy */
			-1,		/* Spotted */
			0,		/* Total Seconds (unsigned!) */
			0,		/* Total Minutes (unsigned!) */
			-1,		/* Total Hours */
			0,		/* Cloaked Seconds */
			0,		/* Cloaked Minutes */
			-1,		/* Cloaked Hours */
			-1,		/* Health Damage */
			-1,		/* Armour Damage */
			-1,		/* Average Speed */
			-1,		/* Field Charge Used */
			-1,		/* Head Shot Percentage */
			{		/* Padding */
				0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,
			},
		},
		CHEATMODE_NONACTIVE,	/* Cheat to activate */
	},
	{
		{	/* Escape_P / Hard */
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Creatures Killed */
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Creatures decapitated*/
			{-1,-1,-1,-1,-1,-1,-1,-1,10,-1,},	/* Trophies / Live Head Bites */
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Dead Head Bites */
			-1,		/* Shots Fired */
			-1,		/* Accuracy */
			-1,		/* Spotted */
			0,		/* Total Seconds (unsigned!) */
			0,		/* Total Minutes (unsigned!) */
			-1,		/* Total Hours */
			0,		/* Cloaked Seconds */
			0,		/* Cloaked Minutes */
			-1,		/* Cloaked Hours */
			-1,		/* Health Damage */
			-1,		/* Armour Damage */
			-1,		/* Average Speed */
			-1,		/* Field Charge Used */
			-1,		/* Head Shot Percentage */
			{		/* Padding */
				0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,
			},
		},
		CHEATMODE_TRIPTASTIC,	/* Cheat to activate */
	},
	{
		{	/* Earthbound_P / Hard */
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Creatures Killed */
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Creatures decapitated*/
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Trophies / Live Head Bites */
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Dead Head Bites */
			-1,		/* Shots Fired */
			-1,		/* Accuracy */
			-1,		/* Spotted */
			0,		/* Total Seconds (unsigned!) */
			0,		/* Total Minutes (unsigned!) */
			-1,		/* Total Hours */
			0,		/* Cloaked Seconds */
			0,		/* Cloaked Minutes */
			-1,		/* Cloaked Hours */
			-1,		/* Health Damage */
			-1,		/* Armour Damage */
			-1,		/* Average Speed */
			-1,		/* Field Charge Used */
			-1,		/* Head Shot Percentage */
			{		/* Padding */
				0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,
			},
		},
		CHEATMODE_NONACTIVE,	/* Cheat to activate */
	},
	{
		{	/* Waterfall_M / Hard */
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Creatures Killed */
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Creatures decapitated*/
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Trophies / Live Head Bites */
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Dead Head Bites */
			-1,		/* Shots Fired */
			-1,		/* Accuracy */
			-1,		/* Spotted */
			0,		/* Total Seconds (unsigned!) */
			0,		/* Total Minutes (unsigned!) */
			-1,		/* Total Hours */
			0,		/* Cloaked Seconds */
			0,		/* Cloaked Minutes */
			-1,		/* Cloaked Hours */
			-1,		/* Health Damage */
			-1,		/* Armour Damage */
			-1,		/* Average Speed */
			-1,		/* Field Charge Used */
			-1,		/* Head Shot Percentage */
			{		/* Padding */
				0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,
			},
		},
		CHEATMODE_NONACTIVE,	/* Cheat to activate */
	},
	{
		{	/* Vaults_M / Hard */
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Creatures Killed */
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Creatures decapitated*/
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Trophies / Live Head Bites */
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Dead Head Bites */
			-1,		/* Shots Fired */
			-1,		/* Accuracy */
			-1,		/* Spotted */
			0,		/* Total Seconds (unsigned!) */
			0,		/* Total Minutes (unsigned!) */
			-1,		/* Total Hours */
			0,		/* Cloaked Seconds */
			0,		/* Cloaked Minutes */
			-1,		/* Cloaked Hours */
			60,		/* Health Damage */
			-1,		/* Armour Damage */
			-1,		/* Average Speed */
			-1,		/* Field Charge Used */
			-1,		/* Head Shot Percentage */
			{		/* Padding */
				0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,
			},
		},
		CHEATMODE_UNDERWATER,	/* Cheat to activate */
	},
	{
		{	/* Feraco_M / Hard */
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Creatures Killed */
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Creatures decapitated*/
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Trophies / Live Head Bites */
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Dead Head Bites */
			-1,		/* Shots Fired */
			-1,		/* Accuracy */
			-1,		/* Spotted */
			0,		/* Total Seconds (unsigned!) */
			0,		/* Total Minutes (unsigned!) */
			-1,		/* Total Hours */
			0,		/* Cloaked Seconds */
			0,		/* Cloaked Minutes */
			-1,		/* Cloaked Hours */
			-1,		/* Health Damage */
			-1,		/* Armour Damage */
			-1,		/* Average Speed */
			-1,		/* Field Charge Used */
			-1,		/* Head Shot Percentage */
			{		/* Padding */
				0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,
			},
		},
		CHEATMODE_NONACTIVE,	/* Cheat to activate */
	},
	{
		{	/* Temple_M / Hard */
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Creatures Killed */
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Creatures decapitated*/
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Trophies / Live Head Bites */
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Dead Head Bites */
			-1,		/* Shots Fired */
			-1,		/* Accuracy */
			-1,		/* Spotted */
			0,		/* Total Seconds (unsigned!) */
			0,		/* Total Minutes (unsigned!) */
			-1,		/* Total Hours */
			0,		/* Cloaked Seconds */
			0,		/* Cloaked Minutes */
			-1,		/* Cloaked Hours */
			100,	/* Health Damage */
			-1,		/* Armour Damage */
			-1,		/* Average Speed */
			-1,		/* Field Charge Used */
			-1,		/* Head Shot Percentage */
			{		/* Padding */
				0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,
			},
		},
		CHEATMODE_FREEFALL,	/* Cheat to activate */
	},
	{
		{	/* Gateway_M / Hard */
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Creatures Killed */
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Creatures decapitated*/
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Trophies / Live Head Bites */
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Dead Head Bites */
			-1,		/* Shots Fired */
			-1,		/* Accuracy */
			-1,		/* Spotted */
			0,		/* Total Seconds (unsigned!) */
			0,		/* Total Minutes (unsigned!) */
			-1,		/* Total Hours */
			0,		/* Cloaked Seconds */
			0,		/* Cloaked Minutes */
			-1,		/* Cloaked Hours */
			-1,		/* Health Damage */
			-1,		/* Armour Damage */
			-1,		/* Average Speed */
			-1,		/* Field Charge Used */
			-1,		/* Head Shot Percentage */
			{		/* Padding */
				0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,
			},
		},
		CHEATMODE_NONACTIVE,	/* Cheat to activate */
	},
	{
		{	/* Kens_Coop / Hard */
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Creatures Killed */
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Creatures decapitated*/
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Trophies / Live Head Bites */
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Dead Head Bites */
			-1,		/* Shots Fired */
			-1,		/* Accuracy */
			-1,		/* Spotted */
			0,		/* Total Seconds (unsigned!) */
			0,		/* Total Minutes (unsigned!) */
			-1,		/* Total Hours */
			0,		/* Cloaked Seconds */
			0,		/* Cloaked Minutes */
			-1,		/* Cloaked Hours */
			-1,		/* Health Damage */
			-1,		/* Armour Damage */
			-1,		/* Average Speed */
			-1,		/* Field Charge Used */
			-1,		/* Head Shot Percentage */
			{		/* Padding */
				0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,
			},
		},
		CHEATMODE_NONACTIVE,	/* Cheat to activate */
	},
	{
		{	/* Hive_Coop / Hard */
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Creatures Killed */
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Creatures decapitated*/
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Trophies / Live Head Bites */
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Dead Head Bites */
			-1,		/* Shots Fired */
			-1,		/* Accuracy */
			-1,		/* Spotted */
			0,		/* Total Seconds (unsigned!) */
			0,		/* Total Minutes (unsigned!) */
			-1,		/* Total Hours */
			0,		/* Cloaked Seconds */
			0,		/* Cloaked Minutes */
			-1,		/* Cloaked Hours */
			-1,		/* Health Damage */
			-1,		/* Armour Damage */
			-1,		/* Average Speed */
			-1,		/* Field Charge Used */
			-1,		/* Head Shot Percentage */
			{		/* Padding */
				0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,
			},
		},
		CHEATMODE_NONACTIVE,	/* Cheat to activate */
	},
	{
		{	/* Trapped_Coop / Hard */
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Creatures Killed */
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Creatures decapitated*/
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Trophies / Live Head Bites */
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Dead Head Bites */
			-1,		/* Shots Fired */
			-1,		/* Accuracy */
			-1,		/* Spotted */
			0,		/* Total Seconds (unsigned!) */
			0,		/* Total Minutes (unsigned!) */
			-1,		/* Total Hours */
			0,		/* Cloaked Seconds */
			0,		/* Cloaked Minutes */
			-1,		/* Cloaked Hours */
			-1,		/* Health Damage */
			-1,		/* Armour Damage */
			-1,		/* Average Speed */
			-1,		/* Field Charge Used */
			-1,		/* Head Shot Percentage */
			{		/* Padding */
				0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,
			},
		},
		CHEATMODE_NONACTIVE,	/* Cheat to activate */
	},
	{
		{	/* Als_DM_Coop / Hard */
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Creatures Killed */
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Creatures decapitated*/
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Trophies / Live Head Bites */
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Dead Head Bites */
			-1,		/* Shots Fired */
			-1,		/* Accuracy */
			-1,		/* Spotted */
			0,		/* Total Seconds (unsigned!) */
			0,		/* Total Minutes (unsigned!) */
			-1,		/* Total Hours */
			0,		/* Cloaked Seconds */
			0,		/* Cloaked Minutes */
			-1,		/* Cloaked Hours */
			-1,		/* Health Damage */
			-1,		/* Armour Damage */
			-1,		/* Average Speed */
			-1,		/* Field Charge Used */
			-1,		/* Head Shot Percentage */
			{		/* Padding */
				0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,
			},
		},
		CHEATMODE_NONACTIVE,	/* Cheat to activate */
	},
	{
		{	/* E3DemoSP / Hard */
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Creatures Killed */
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Creatures decapitated*/
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Trophies / Live Head Bites */
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Dead Head Bites */
			-1,		/* Shots Fired */
			-1,		/* Accuracy */
			-1,		/* Spotted */
			0,		/* Total Seconds (unsigned!) */
			0,		/* Total Minutes (unsigned!) */
			-1,		/* Total Hours */
			0,		/* Cloaked Seconds */
			0,		/* Cloaked Minutes */
			-1,		/* Cloaked Hours */
			-1,		/* Health Damage */
			-1,		/* Armour Damage */
			-1,		/* Average Speed */
			-1,		/* Field Charge Used */
			-1,		/* Head Shot Percentage */
			{		/* Padding */
				0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,
			},
		},
		CHEATMODE_NONACTIVE,	/* Cheat to activate */
	},
},

{
	{
		{	/* Derelict / Impossible */
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Creatures Killed */
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Creatures decapitated*/
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Trophies / Live Head Bites */
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Dead Head Bites */
			-1,		/* Shots Fired */
			-1,		/* Accuracy */
			-1,		/* Spotted */
			0,		/* Total Seconds (unsigned!) */
			0,		/* Total Minutes (unsigned!) */
			-1,		/* Total Hours */
			0,		/* Cloaked Seconds */
			0,		/* Cloaked Minutes */
			-1,		/* Cloaked Hours */
			-1,		/* Health Damage */
			-1,		/* Armour Damage */
			-1,		/* Average Speed */
			-1,		/* Field Charge Used */
			-1,		/* Head Shot Percentage */
			{		/* Padding */
				0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,
			},
		},
		CHEATMODE_NONACTIVE,	/* Cheat to activate */
	},
	{
		{	/* Colony / Impossible */
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Creatures Killed */
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Creatures decapitated*/
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Trophies / Live Head Bites */
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Dead Head Bites */
			-1,		/* Shots Fired */
			-1,		/* Accuracy */
			-1,		/* Spotted */
			0,		/* Total Seconds (unsigned!) */
			0,		/* Total Minutes (unsigned!) */
			-1,		/* Total Hours */
			0,		/* Cloaked Seconds */
			0,		/* Cloaked Minutes */
			-1,		/* Cloaked Hours */
			-1,		/* Health Damage */
			-1,		/* Armour Damage */
			-1,		/* Average Speed */
			-1,		/* Field Charge Used */
			-1,		/* Head Shot Percentage */
			{		/* Padding */
				0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,
			},
		},
		CHEATMODE_NONACTIVE,	/* Cheat to activate */
	},
	{
		{	/* Invasion / Impossible */
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Creatures Killed */
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Creatures decapitated*/
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Trophies / Live Head Bites */
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Dead Head Bites */
			-1,		/* Shots Fired */
			-1,		/* Accuracy */
			-1,		/* Spotted */
			0,		/* Total Seconds (unsigned!) */
			0,		/* Total Minutes (unsigned!) */
			-1,		/* Total Hours */
			0,		/* Cloaked Seconds */
			0,		/* Cloaked Minutes */
			-1,		/* Cloaked Hours */
			-1,		/* Health Damage */
			-1,		/* Armour Damage */
			-1,		/* Average Speed */
			-1,		/* Field Charge Used */
			-1,		/* Head Shot Percentage */
			{		/* Padding */
				0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,
			},
		},
		CHEATMODE_NONACTIVE,	/* Cheat to activate */
	},
	{
		{	/* Orbital / Impossible */
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Creatures Killed */
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Creatures decapitated*/
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Trophies / Live Head Bites */
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Dead Head Bites */
			-1,		/* Shots Fired */
			-1,		/* Accuracy */
			-1,		/* Spotted */
			0,		/* Total Seconds (unsigned!) */
			0,		/* Total Minutes (unsigned!) */
			-1,		/* Total Hours */
			0,		/* Cloaked Seconds */
			0,		/* Cloaked Minutes */
			-1,		/* Cloaked Hours */
			-1,		/* Health Damage */
			-1,		/* Armour Damage */
			-1,		/* Average Speed */
			-1,		/* Field Charge Used */
			-1,		/* Head Shot Percentage */
			{		/* Padding */
				0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,
			},
		},
		CHEATMODE_NONACTIVE,	/* Cheat to activate */
	},
	{
		{	/* Tyrago / Impossible */
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Creatures Killed */
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Creatures decapitated*/
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Trophies / Live Head Bites */
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Dead Head Bites */
			-1,		/* Shots Fired */
			-1,		/* Accuracy */
			-1,		/* Spotted */
			0,		/* Total Seconds (unsigned!) */
			0,		/* Total Minutes (unsigned!) */
			-1,		/* Total Hours */
			0,		/* Cloaked Seconds */
			0,		/* Cloaked Minutes */
			-1,		/* Cloaked Hours */
			-1,		/* Health Damage */
			-1,		/* Armour Damage */
			-1,		/* Average Speed */
			-1,		/* Field Charge Used */
			-1,		/* Head Shot Percentage */
			{		/* Padding */
				0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,
			},
		},
		CHEATMODE_NONACTIVE,	/* Cheat to activate */
	},
	{
		{	/* Hangar / Impossible */
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Creatures Killed */
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Creatures decapitated*/
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Trophies / Live Head Bites */
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Dead Head Bites */
			-1,		/* Shots Fired */
			-1,		/* Accuracy */
			-1,		/* Spotted */
			0,		/* Total Seconds (unsigned!) */
			0,		/* Total Minutes (unsigned!) */
			-1,		/* Total Hours */
			0,		/* Cloaked Seconds */
			0,		/* Cloaked Minutes */
			-1,		/* Cloaked Hours */
			-1,		/* Health Damage */
			-1,		/* Armour Damage */
			-1,		/* Average Speed */
			-1,		/* Field Charge Used */
			-1,		/* Head Shot Percentage */
			{		/* Padding */
				0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,
			},
		},
		CHEATMODE_NONACTIVE,	/* Cheat to activate */
	},
	{
		{	/* Waterfall / Impossible */
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Creatures Killed */
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Creatures decapitated*/
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Trophies / Live Head Bites */
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Dead Head Bites */
			-1,		/* Shots Fired */
			-1,		/* Accuracy */
			-1,		/* Spotted */
			0,		/* Total Seconds (unsigned!) */
			0,		/* Total Minutes (unsigned!) */
			-1,		/* Total Hours */
			0,		/* Cloaked Seconds */
			0,		/* Cloaked Minutes */
			-1,		/* Cloaked Hours */
			-1,		/* Health Damage */
			-1,		/* Armour Damage */
			-1,		/* Average Speed */
			-1,		/* Field Charge Used */
			-1,		/* Head Shot Percentage */
			{		/* Padding */
				0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,
			},
		},
		CHEATMODE_NONACTIVE,	/* Cheat to activate */
	},
	{
		{	/* Area52 / Impossible */
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Creatures Killed */
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Creatures decapitated*/
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Trophies / Live Head Bites */
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Dead Head Bites */
			-1,		/* Shots Fired */
			-1,		/* Accuracy */
			-1,		/* Spotted */
			0,		/* Total Seconds (unsigned!) */
			0,		/* Total Minutes (unsigned!) */
			-1,		/* Total Hours */
			0,		/* Cloaked Seconds */
			0,		/* Cloaked Minutes */
			-1,		/* Cloaked Hours */
			-1,		/* Health Damage */
			-1,		/* Armour Damage */
			-1,		/* Average Speed */
			-1,		/* Field Charge Used */
			-1,		/* Head Shot Percentage */
			{		/* Padding */
				0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,
			},
		},
		CHEATMODE_NONACTIVE,	/* Cheat to activate */
	},
	{
		{	/* Vaults / Impossible */
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Creatures Killed */
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Creatures decapitated*/
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Trophies / Live Head Bites */
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Dead Head Bites */
			-1,		/* Shots Fired */
			-1,		/* Accuracy */
			-1,		/* Spotted */
			0,		/* Total Seconds (unsigned!) */
			0,		/* Total Minutes (unsigned!) */
			-1,		/* Total Hours */
			0,		/* Cloaked Seconds */
			0,		/* Cloaked Minutes */
			-1,		/* Cloaked Hours */
			-1,		/* Health Damage */
			-1,		/* Armour Damage */
			-1,		/* Average Speed */
			-1,		/* Field Charge Used */
			-1,		/* Head Shot Percentage */
			{		/* Padding */
				0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,
			},
		},
		CHEATMODE_NONACTIVE,	/* Cheat to activate */
	},
	{
		{	/* Fury161 / Impossible */
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Creatures Killed */
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Creatures decapitated*/
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Trophies / Live Head Bites */
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Dead Head Bites */
			-1,		/* Shots Fired */
			-1,		/* Accuracy */
			-1,		/* Spotted */
			0,		/* Total Seconds (unsigned!) */
			0,		/* Total Minutes (unsigned!) */
			-1,		/* Total Hours */
			0,		/* Cloaked Seconds */
			0,		/* Cloaked Minutes */
			-1,		/* Cloaked Hours */
			-1,		/* Health Damage */
			-1,		/* Armour Damage */
			-1,		/* Average Speed */
			-1,		/* Field Charge Used */
			-1,		/* Head Shot Percentage */
			{		/* Padding */
				0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,
			},
		},
		CHEATMODE_NONACTIVE,	/* Cheat to activate */
	},
	{
		{	/* Caverns / Impossible */
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Creatures Killed */
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Creatures decapitated*/
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Trophies / Live Head Bites */
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Dead Head Bites */
			-1,		/* Shots Fired */
			-1,		/* Accuracy */
			-1,		/* Spotted */
			0,		/* Total Seconds (unsigned!) */
			0,		/* Total Minutes (unsigned!) */
			-1,		/* Total Hours */
			0,		/* Cloaked Seconds */
			0,		/* Cloaked Minutes */
			-1,		/* Cloaked Hours */
			-1,		/* Health Damage */
			-1,		/* Armour Damage */
			-1,		/* Average Speed */
			-1,		/* Field Charge Used */
			-1,		/* Head Shot Percentage */
			{		/* Padding */
				0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,
			},
		},
		CHEATMODE_NONACTIVE,	/* Cheat to activate */
	},
	{
		{	/* Battle / Impossible */
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Creatures Killed */
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Creatures decapitated*/
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Trophies / Live Head Bites */
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Dead Head Bites */
			-1,		/* Shots Fired */
			-1,		/* Accuracy */
			-1,		/* Spotted */
			0,		/* Total Seconds (unsigned!) */
			0,		/* Total Minutes (unsigned!) */
			-1,		/* Total Hours */
			0,		/* Cloaked Seconds */
			0,		/* Cloaked Minutes */
			-1,		/* Cloaked Hours */
			-1,		/* Health Damage */
			-1,		/* Armour Damage */
			-1,		/* Average Speed */
			-1,		/* Field Charge Used */
			-1,		/* Head Shot Percentage */
			{		/* Padding */
				0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,
			},
		},
		CHEATMODE_NONACTIVE,	/* Cheat to activate */
	},
	{
		{	/* Feraco / Impossible */
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Creatures Killed */
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Creatures decapitated*/
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Trophies / Live Head Bites */
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Dead Head Bites */
			-1,		/* Shots Fired */
			-1,		/* Accuracy */
			-1,		/* Spotted */
			0,		/* Total Seconds (unsigned!) */
			0,		/* Total Minutes (unsigned!) */
			-1,		/* Total Hours */
			0,		/* Cloaked Seconds */
			0,		/* Cloaked Minutes */
			-1,		/* Cloaked Hours */
			-1,		/* Health Damage */
			-1,		/* Armour Damage */
			-1,		/* Average Speed */
			-1,		/* Field Charge Used */
			-1,		/* Head Shot Percentage */
			{		/* Padding */
				0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,
			},
		},
		CHEATMODE_NONACTIVE,	/* Cheat to activate */
	},
	{
		{	/* Temple / Impossible */
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Creatures Killed */
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Creatures decapitated*/
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Trophies / Live Head Bites */
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Dead Head Bites */
			-1,		/* Shots Fired */
			-1,		/* Accuracy */
			-1,		/* Spotted */
			0,		/* Total Seconds (unsigned!) */
			0,		/* Total Minutes (unsigned!) */
			-1,		/* Total Hours */
			0,		/* Cloaked Seconds */
			0,		/* Cloaked Minutes */
			-1,		/* Cloaked Hours */
			-1,		/* Health Damage */
			-1,		/* Armour Damage */
			-1,		/* Average Speed */
			-1,		/* Field Charge Used */
			-1,		/* Head Shot Percentage */
			{		/* Padding */
				0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,
			},
		},
		CHEATMODE_NONACTIVE,	/* Cheat to activate */
	},
	{
		{	/* Gateway / Impossible */
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Creatures Killed */
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Creatures decapitated*/
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Trophies / Live Head Bites */
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Dead Head Bites */
			-1,		/* Shots Fired */
			-1,		/* Accuracy */
			-1,		/* Spotted */
			0,		/* Total Seconds (unsigned!) */
			0,		/* Total Minutes (unsigned!) */
			-1,		/* Total Hours */
			0,		/* Cloaked Seconds */
			0,		/* Cloaked Minutes */
			-1,		/* Cloaked Hours */
			-1,		/* Health Damage */
			-1,		/* Armour Damage */
			-1,		/* Average Speed */
			-1,		/* Field Charge Used */
			-1,		/* Head Shot Percentage */
			{		/* Padding */
				0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,
			},
		},
		CHEATMODE_NONACTIVE,	/* Cheat to activate */
	},
	{
		{	/* Escape / Impossible */
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Creatures Killed */
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Creatures decapitated*/
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Trophies / Live Head Bites */
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Dead Head Bites */
			-1,		/* Shots Fired */
			-1,		/* Accuracy */
			-1,		/* Spotted */
			0,		/* Total Seconds (unsigned!) */
			0,		/* Total Minutes (unsigned!) */
			-1,		/* Total Hours */
			0,		/* Cloaked Seconds */
			0,		/* Cloaked Minutes */
			-1,		/* Cloaked Hours */
			-1,		/* Health Damage */
			-1,		/* Armour Damage */
			-1,		/* Average Speed */
			-1,		/* Field Charge Used */
			-1,		/* Head Shot Percentage */
			{		/* Padding */
				0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,
			},
		},
		CHEATMODE_NONACTIVE,	/* Cheat to activate */
	},
	{
		{	/* Earthbound / Impossible */
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Creatures Killed */
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Creatures decapitated*/
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Trophies / Live Head Bites */
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Dead Head Bites */
			-1,		/* Shots Fired */
			-1,		/* Accuracy */
			-1,		/* Spotted */
			0,		/* Total Seconds (unsigned!) */
			0,		/* Total Minutes (unsigned!) */
			-1,		/* Total Hours */
			0,		/* Cloaked Seconds */
			0,		/* Cloaked Minutes */
			-1,		/* Cloaked Hours */
			-1,		/* Health Damage */
			-1,		/* Armour Damage */
			-1,		/* Average Speed */
			-1,		/* Field Charge Used */
			-1,		/* Head Shot Percentage */
			{		/* Padding */
				0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,
			},
		},
		CHEATMODE_NONACTIVE,	/* Cheat to activate */
	},
	{
		{	/* Sewer / Impossible */
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Creatures Killed */
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Creatures decapitated*/
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Trophies / Live Head Bites */
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Dead Head Bites */
			-1,		/* Shots Fired */
			-1,		/* Accuracy */
			-1,		/* Spotted */
			0,		/* Total Seconds (unsigned!) */
			0,		/* Total Minutes (unsigned!) */
			-1,		/* Total Hours */
			0,		/* Cloaked Seconds */
			0,		/* Cloaked Minutes */
			-1,		/* Cloaked Hours */
			-1,		/* Health Damage */
			-1,		/* Armour Damage */
			-1,		/* Average Speed */
			-1,		/* Field Charge Used */
			-1,		/* Head Shot Percentage */
			{		/* Padding */
				0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,
			},
		},
		CHEATMODE_NONACTIVE,	/* Cheat to activate */
	},
	{
		{	/* Massacre / Impossible */
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Creatures Killed */
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Creatures decapitated*/
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Trophies / Live Head Bites */
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Dead Head Bites */
			-1,		/* Shots Fired */
			-1,		/* Accuracy */
			-1,		/* Spotted */
			0,		/* Total Seconds (unsigned!) */
			0,		/* Total Minutes (unsigned!) */
			-1,		/* Total Hours */
			0,		/* Cloaked Seconds */
			0,		/* Cloaked Minutes */
			-1,		/* Cloaked Hours */
			-1,		/* Health Damage */
			-1,		/* Armour Damage */
			-1,		/* Average Speed */
			-1,		/* Field Charge Used */
			-1,		/* Head Shot Percentage */
			{		/* Padding */
				0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,
			},
		},
		CHEATMODE_NONACTIVE,	/* Cheat to activate */
	},
	{
		{	/* Statue / Impossible */
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Creatures Killed */
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Creatures decapitated*/
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Trophies / Live Head Bites */
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Dead Head Bites */
			-1,		/* Shots Fired */
			-1,		/* Accuracy */
			-1,		/* Spotted */
			0,		/* Total Seconds (unsigned!) */
			0,		/* Total Minutes (unsigned!) */
			-1,		/* Total Hours */
			0,		/* Cloaked Seconds */
			0,		/* Cloaked Minutes */
			-1,		/* Cloaked Hours */
			-1,		/* Health Damage */
			-1,		/* Armour Damage */
			-1,		/* Average Speed */
			-1,		/* Field Charge Used */
			-1,		/* Head Shot Percentage */
			{		/* Padding */
				0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,
			},
		},
		CHEATMODE_NONACTIVE,	/* Cheat to activate */
	},
	{
		{	/* Jockey / Impossible */
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Creatures Killed */
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Creatures decapitated*/
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Trophies / Live Head Bites */
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Dead Head Bites */
			-1,		/* Shots Fired */
			-1,		/* Accuracy */
			-1,		/* Spotted */
			0,		/* Total Seconds (unsigned!) */
			0,		/* Total Minutes (unsigned!) */
			-1,		/* Total Hours */
			0,		/* Cloaked Seconds */
			0,		/* Cloaked Minutes */
			-1,		/* Cloaked Hours */
			-1,		/* Health Damage */
			-1,		/* Armour Damage */
			-1,		/* Average Speed */
			-1,		/* Field Charge Used */
			-1,		/* Head Shot Percentage */
			{		/* Padding */
				0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,
			},
		},
		CHEATMODE_NONACTIVE,	/* Cheat to activate */
	},
	{
		{	/* Hive / Impossible */
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Creatures Killed */
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Creatures decapitated*/
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Trophies / Live Head Bites */
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Dead Head Bites */
			-1,		/* Shots Fired */
			-1,		/* Accuracy */
			-1,		/* Spotted */
			0,		/* Total Seconds (unsigned!) */
			0,		/* Total Minutes (unsigned!) */
			-1,		/* Total Hours */
			0,		/* Cloaked Seconds */
			0,		/* Cloaked Minutes */
			-1,		/* Cloaked Hours */
			-1,		/* Health Damage */
			-1,		/* Armour Damage */
			-1,		/* Average Speed */
			-1,		/* Field Charge Used */
			-1,		/* Head Shot Percentage */
			{		/* Padding */
				0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,
			},
		},
		CHEATMODE_NONACTIVE,	/* Cheat to activate */
	},
	{
		{	/* Invasion_A / Impossible */
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Creatures Killed */
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Creatures decapitated*/
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Trophies / Live Head Bites */
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Dead Head Bites */
			-1,		/* Shots Fired */
			-1,		/* Accuracy */
			-1,		/* Spotted */
			0,		/* Total Seconds (unsigned!) */
			0,		/* Total Minutes (unsigned!) */
			-1,		/* Total Hours */
			0,		/* Cloaked Seconds */
			0,		/* Cloaked Minutes */
			-1,		/* Cloaked Hours */
			-1,		/* Health Damage */
			-1,		/* Armour Damage */
			-1,		/* Average Speed */
			-1,		/* Field Charge Used */
			-1,		/* Head Shot Percentage */
			{		/* Padding */
				0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,
			},
		},
		CHEATMODE_NONACTIVE,	/* Cheat to activate */
	},
	{
		{	/* Derelict_A / Impossible */
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Creatures Killed */
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Creatures decapitated*/
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Trophies / Live Head Bites */
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Dead Head Bites */
			-1,		/* Shots Fired */
			-1,		/* Accuracy */
			-1,		/* Spotted */
			0,		/* Total Seconds (unsigned!) */
			0,		/* Total Minutes (unsigned!) */
			-1,		/* Total Hours */
			0,		/* Cloaked Seconds */
			0,		/* Cloaked Minutes */
			-1,		/* Cloaked Hours */
			-1,		/* Health Damage */
			-1,		/* Armour Damage */
			-1,		/* Average Speed */
			-1,		/* Field Charge Used */
			-1,		/* Head Shot Percentage */
			{		/* Padding */
				0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,
			},
		},
		CHEATMODE_NONACTIVE,	/* Cheat to activate */
	},
	{
		{	/* Tyrago_A / Impossible */
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Creatures Killed */
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Creatures decapitated*/
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Trophies / Live Head Bites */
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Dead Head Bites */
			-1,		/* Shots Fired */
			-1,		/* Accuracy */
			-1,		/* Spotted */
			0,		/* Total Seconds (unsigned!) */
			0,		/* Total Minutes (unsigned!) */
			-1,		/* Total Hours */
			0,		/* Cloaked Seconds */
			0,		/* Cloaked Minutes */
			-1,		/* Cloaked Hours */
			-1,		/* Health Damage */
			-1,		/* Armour Damage */
			-1,		/* Average Speed */
			-1,		/* Field Charge Used */
			-1,		/* Head Shot Percentage */
			{		/* Padding */
				0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,
			},
		},
		CHEATMODE_NONACTIVE,	/* Cheat to activate */
	},
	{
		{	/* Fury161_A / Impossible */
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Creatures Killed */
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Creatures decapitated*/
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Trophies / Live Head Bites */
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Dead Head Bites */
			-1,		/* Shots Fired */
			-1,		/* Accuracy */
			-1,		/* Spotted */
			0,		/* Total Seconds (unsigned!) */
			0,		/* Total Minutes (unsigned!) */
			-1,		/* Total Hours */
			0,		/* Cloaked Seconds */
			0,		/* Cloaked Minutes */
			-1,		/* Cloaked Hours */
			-1,		/* Health Damage */
			-1,		/* Armour Damage */
			-1,		/* Average Speed */
			-1,		/* Field Charge Used */
			-1,		/* Head Shot Percentage */
			{		/* Padding */
				0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,
			},
		},
		CHEATMODE_NONACTIVE,	/* Cheat to activate */
	},
	{
		{	/* Caverns_A / Impossible */
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Creatures Killed */
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Creatures decapitated*/
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Trophies / Live Head Bites */
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Dead Head Bites */
			-1,		/* Shots Fired */
			-1,		/* Accuracy */
			-1,		/* Spotted */
			0,		/* Total Seconds (unsigned!) */
			0,		/* Total Minutes (unsigned!) */
			-1,		/* Total Hours */
			0,		/* Cloaked Seconds */
			0,		/* Cloaked Minutes */
			-1,		/* Cloaked Hours */
			-1,		/* Health Damage */
			-1,		/* Armour Damage */
			-1,		/* Average Speed */
			-1,		/* Field Charge Used */
			-1,		/* Head Shot Percentage */
			{		/* Padding */
				0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,
			},
		},
		CHEATMODE_NONACTIVE,	/* Cheat to activate */
	},
	{
		{	/* Invasion_P / Impossible */
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Creatures Killed */
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Creatures decapitated*/
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Trophies / Live Head Bites */
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Dead Head Bites */
			-1,		/* Shots Fired */
			-1,		/* Accuracy */
			-1,		/* Spotted */
			0,		/* Total Seconds (unsigned!) */
			0,		/* Total Minutes (unsigned!) */
			-1,		/* Total Hours */
			0,		/* Cloaked Seconds */
			0,		/* Cloaked Minutes */
			-1,		/* Cloaked Hours */
			-1,		/* Health Damage */
			-1,		/* Armour Damage */
			-1,		/* Average Speed */
			-1,		/* Field Charge Used */
			-1,		/* Head Shot Percentage */
			{		/* Padding */
				0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,
			},
		},
		CHEATMODE_NONACTIVE,	/* Cheat to activate */
	},
	{
		{	/* Tyrago_P / Impossible */
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Creatures Killed */
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Creatures decapitated*/
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Trophies / Live Head Bites */
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Dead Head Bites */
			-1,		/* Shots Fired */
			-1,		/* Accuracy */
			-1,		/* Spotted */
			0,		/* Total Seconds (unsigned!) */
			0,		/* Total Minutes (unsigned!) */
			-1,		/* Total Hours */
			0,		/* Cloaked Seconds */
			0,		/* Cloaked Minutes */
			-1,		/* Cloaked Hours */
			-1,		/* Health Damage */
			-1,		/* Armour Damage */
			-1,		/* Average Speed */
			-1,		/* Field Charge Used */
			-1,		/* Head Shot Percentage */
			{		/* Padding */
				0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,
			},
		},
		CHEATMODE_NONACTIVE,	/* Cheat to activate */
	},
	{
		{	/* Temple_P / Impossible */
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Creatures Killed */
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Creatures decapitated*/
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Trophies / Live Head Bites */
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Dead Head Bites */
			-1,		/* Shots Fired */
			-1,		/* Accuracy */
			-1,		/* Spotted */
			0,		/* Total Seconds (unsigned!) */
			0,		/* Total Minutes (unsigned!) */
			-1,		/* Total Hours */
			0,		/* Cloaked Seconds */
			0,		/* Cloaked Minutes */
			-1,		/* Cloaked Hours */
			-1,		/* Health Damage */
			-1,		/* Armour Damage */
			-1,		/* Average Speed */
			-1,		/* Field Charge Used */
			-1,		/* Head Shot Percentage */
			{		/* Padding */
				0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,
			},
		},
		CHEATMODE_NONACTIVE,	/* Cheat to activate */
	},
	{
		{	/* Escape_P / Impossible */
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Creatures Killed */
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Creatures decapitated*/
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Trophies / Live Head Bites */
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Dead Head Bites */
			-1,		/* Shots Fired */
			-1,		/* Accuracy */
			-1,		/* Spotted */
			0,		/* Total Seconds (unsigned!) */
			0,		/* Total Minutes (unsigned!) */
			-1,		/* Total Hours */
			0,		/* Cloaked Seconds */
			0,		/* Cloaked Minutes */
			-1,		/* Cloaked Hours */
			-1,		/* Health Damage */
			-1,		/* Armour Damage */
			-1,		/* Average Speed */
			-1,		/* Field Charge Used */
			-1,		/* Head Shot Percentage */
			{		/* Padding */
				0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,
			},
		},
		CHEATMODE_NONACTIVE,	/* Cheat to activate */
	},
	{
		{	/* Earthbound_P / Impossible */
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Creatures Killed */
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Creatures decapitated*/
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Trophies / Live Head Bites */
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Dead Head Bites */
			-1,		/* Shots Fired */
			-1,		/* Accuracy */
			-1,		/* Spotted */
			0,		/* Total Seconds (unsigned!) */
			0,		/* Total Minutes (unsigned!) */
			-1,		/* Total Hours */
			0,		/* Cloaked Seconds */
			0,		/* Cloaked Minutes */
			-1,		/* Cloaked Hours */
			-1,		/* Health Damage */
			-1,		/* Armour Damage */
			-1,		/* Average Speed */
			-1,		/* Field Charge Used */
			-1,		/* Head Shot Percentage */
			{		/* Padding */
				0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,
			},
		},
		CHEATMODE_NONACTIVE,	/* Cheat to activate */
	},
	{
		{	/* Waterfall_M / Impossible */
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Creatures Killed */
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Creatures decapitated*/
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Trophies / Live Head Bites */
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Dead Head Bites */
			-1,		/* Shots Fired */
			-1,		/* Accuracy */
			-1,		/* Spotted */
			0,		/* Total Seconds (unsigned!) */
			0,		/* Total Minutes (unsigned!) */
			-1,		/* Total Hours */
			0,		/* Cloaked Seconds */
			0,		/* Cloaked Minutes */
			-1,		/* Cloaked Hours */
			-1,		/* Health Damage */
			-1,		/* Armour Damage */
			-1,		/* Average Speed */
			-1,		/* Field Charge Used */
			-1,		/* Head Shot Percentage */
			{		/* Padding */
				0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,
			},
		},
		CHEATMODE_NONACTIVE,	/* Cheat to activate */
	},
	{
		{	/* Vaults_M / Impossible */
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Creatures Killed */
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Creatures decapitated*/
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Trophies / Live Head Bites */
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Dead Head Bites */
			-1,		/* Shots Fired */
			-1,		/* Accuracy */
			-1,		/* Spotted */
			0,		/* Total Seconds (unsigned!) */
			0,		/* Total Minutes (unsigned!) */
			-1,		/* Total Hours */
			0,		/* Cloaked Seconds */
			0,		/* Cloaked Minutes */
			-1,		/* Cloaked Hours */
			-1,		/* Health Damage */
			-1,		/* Armour Damage */
			-1,		/* Average Speed */
			-1,		/* Field Charge Used */
			-1,		/* Head Shot Percentage */
			{		/* Padding */
				0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,
			},
		},
		CHEATMODE_NONACTIVE,	/* Cheat to activate */
	},
	{
		{	/* Feraco_M / Impossible */
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Creatures Killed */
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Creatures decapitated*/
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Trophies / Live Head Bites */
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Dead Head Bites */
			-1,		/* Shots Fired */
			-1,		/* Accuracy */
			-1,		/* Spotted */
			0,		/* Total Seconds (unsigned!) */
			0,		/* Total Minutes (unsigned!) */
			-1,		/* Total Hours */
			0,		/* Cloaked Seconds */
			0,		/* Cloaked Minutes */
			-1,		/* Cloaked Hours */
			-1,		/* Health Damage */
			-1,		/* Armour Damage */
			-1,		/* Average Speed */
			-1,		/* Field Charge Used */
			-1,		/* Head Shot Percentage */
			{		/* Padding */
				0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,
			},
		},
		CHEATMODE_NONACTIVE,	/* Cheat to activate */
	},
	{
		{	/* Temple_M / Impossible */
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Creatures Killed */
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Creatures decapitated*/
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Trophies / Live Head Bites */
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Dead Head Bites */
			-1,		/* Shots Fired */
			-1,		/* Accuracy */
			-1,		/* Spotted */
			0,		/* Total Seconds (unsigned!) */
			0,		/* Total Minutes (unsigned!) */
			-1,		/* Total Hours */
			0,		/* Cloaked Seconds */
			0,		/* Cloaked Minutes */
			-1,		/* Cloaked Hours */
			-1,		/* Health Damage */
			-1,		/* Armour Damage */
			-1,		/* Average Speed */
			-1,		/* Field Charge Used */
			-1,		/* Head Shot Percentage */
			{		/* Padding */
				0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,
			},
		},
		CHEATMODE_NONACTIVE,	/* Cheat to activate */
	},
	{
		{	/* Gateway_M / Impossible */
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Creatures Killed */
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Creatures decapitated*/
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Trophies / Live Head Bites */
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Dead Head Bites */
			-1,		/* Shots Fired */
			-1,		/* Accuracy */
			-1,		/* Spotted */
			0,		/* Total Seconds (unsigned!) */
			0,		/* Total Minutes (unsigned!) */
			-1,		/* Total Hours */
			0,		/* Cloaked Seconds */
			0,		/* Cloaked Minutes */
			-1,		/* Cloaked Hours */
			-1,		/* Health Damage */
			-1,		/* Armour Damage */
			-1,		/* Average Speed */
			-1,		/* Field Charge Used */
			-1,		/* Head Shot Percentage */
			{		/* Padding */
				0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,
			},
		},
		CHEATMODE_NONACTIVE,	/* Cheat to activate */
	},
	{
		{	/* Kens_Coop / Impossible */
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Creatures Killed */
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Creatures decapitated*/
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Trophies / Live Head Bites */
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Dead Head Bites */
			-1,		/* Shots Fired */
			-1,		/* Accuracy */
			-1,		/* Spotted */
			0,		/* Total Seconds (unsigned!) */
			0,		/* Total Minutes (unsigned!) */
			-1,		/* Total Hours */
			0,		/* Cloaked Seconds */
			0,		/* Cloaked Minutes */
			-1,		/* Cloaked Hours */
			-1,		/* Health Damage */
			-1,		/* Armour Damage */
			-1,		/* Average Speed */
			-1,		/* Field Charge Used */
			-1,		/* Head Shot Percentage */
			{		/* Padding */
				0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,
			},
		},
		CHEATMODE_NONACTIVE,	/* Cheat to activate */
	},
	{
		{	/* Hive_Coop / Impossible */
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Creatures Killed */
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Creatures decapitated*/
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Trophies / Live Head Bites */
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Dead Head Bites */
			-1,		/* Shots Fired */
			-1,		/* Accuracy */
			-1,		/* Spotted */
			0,		/* Total Seconds (unsigned!) */
			0,		/* Total Minutes (unsigned!) */
			-1,		/* Total Hours */
			0,		/* Cloaked Seconds */
			0,		/* Cloaked Minutes */
			-1,		/* Cloaked Hours */
			-1,		/* Health Damage */
			-1,		/* Armour Damage */
			-1,		/* Average Speed */
			-1,		/* Field Charge Used */
			-1,		/* Head Shot Percentage */
			{		/* Padding */
				0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,
			},
		},
		CHEATMODE_NONACTIVE,	/* Cheat to activate */
	},
	{
		{	/* Trapped_Coop / Impossible */
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Creatures Killed */
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Creatures decapitated*/
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Trophies / Live Head Bites */
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Dead Head Bites */
			-1,		/* Shots Fired */
			-1,		/* Accuracy */
			-1,		/* Spotted */
			0,		/* Total Seconds (unsigned!) */
			0,		/* Total Minutes (unsigned!) */
			-1,		/* Total Hours */
			0,		/* Cloaked Seconds */
			0,		/* Cloaked Minutes */
			-1,		/* Cloaked Hours */
			-1,		/* Health Damage */
			-1,		/* Armour Damage */
			-1,		/* Average Speed */
			-1,		/* Field Charge Used */
			-1,		/* Head Shot Percentage */
			{		/* Padding */
				0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,
			},
		},
		CHEATMODE_NONACTIVE,	/* Cheat to activate */
	},
	{
		{	/* Als_DM_Coop / Impossible */
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Creatures Killed */
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Creatures decapitated*/
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Trophies / Live Head Bites */
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Dead Head Bites */
			-1,		/* Shots Fired */
			-1,		/* Accuracy */
			-1,		/* Spotted */
			0,		/* Total Seconds (unsigned!) */
			0,		/* Total Minutes (unsigned!) */
			-1,		/* Total Hours */
			0,		/* Cloaked Seconds */
			0,		/* Cloaked Minutes */
			-1,		/* Cloaked Hours */
			-1,		/* Health Damage */
			-1,		/* Armour Damage */
			-1,		/* Average Speed */
			-1,		/* Field Charge Used */
			-1,		/* Head Shot Percentage */
			{		/* Padding */
				0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,
			},
		},
		CHEATMODE_NONACTIVE,	/* Cheat to activate */
	},
	{
		{	/* Jockey_Coop / Impossible */
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Creatures Killed */
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Creatures decapitated*/
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Trophies / Live Head Bites */
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,},	/* Dead Head Bites */
			-1,		/* Shots Fired */
			-1,		/* Accuracy */
			-1,		/* Spotted */
			0,		/* Total Seconds (unsigned!) */
			0,		/* Total Minutes (unsigned!) */
			-1,		/* Total Hours */
			0,		/* Cloaked Seconds */
			0,		/* Cloaked Minutes */
			-1,		/* Cloaked Hours */
			-1,		/* Health Damage */
			-1,		/* Armour Damage */
			-1,		/* Average Speed */
			-1,		/* Field Charge Used */
			-1,		/* Head Shot Percentage */
			{		/* Padding */
				0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,
			},
		},
		CHEATMODE_NONACTIVE,	/* Cheat to activate */
	},
	{
		{	/* E3DemoSP / Impossible */
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},	/* Creatures Killed */
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},	/* Creatures decapitated*/
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},	/* Trophies / Live Head Bites */
			{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},	/* Dead Head Bites */
			-1,		/* Shots Fired */
			-1,		/* Accuracy */
			-1,		/* Spotted */
			0,		/* Total Seconds (unsigned!) */
			0,		/* Total Minutes (unsigned!) */
			-1,		/* Total Hours */
			0,		/* Cloaked Seconds */
			0,		/* Cloaked Minutes */
			-1,		/* Cloaked Hours */
			-1,		/* Health Damage */
			-1,		/* Armour Damage */
			-1,		/* Average Speed */
			-1,		/* Field Charge Used */
			-1,		/* Head Shot Percentage */
			{		/* Padding */
				0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0
			}
		},
		CHEATMODE_NONACTIVE	/* Cheat to activate */
	}
}

};

void SetLevelToLoadForAlien(int episode)
{
	strcpy(LevelName,RifNamesForEnvironments[AlienEpisodes[episode]]);
}
void SetLevelToLoadForPredator(int episode)
{
	strcpy(LevelName,RifNamesForEnvironments[PredatorEpisodes[episode]]);
}
void SetLevelToLoadForMarine(int episode)
{
	strcpy(LevelName,RifNamesForEnvironments[MarineEpisodes[episode]]);
}

void SetLevelToLoadForMultiplayer(int episode)
{
	//is this a custom level?
	if(episode>=MAX_NO_OF_MULTIPLAYER_EPISODES)
	{
		//it certainly is
		//(the game type sent passed to the function doesn't really matter , as long as it isn't NGT_COOP)
		sprintf(LevelName,"Custom/%s",GetCustomMultiplayerLevelName(episode,NGT_Individual));
	}
	else
	{
		strcpy(LevelName,RifNamesForEnvironments[MultiplayerEpisodes[episode]]);
	}
}
void SetLevelToLoadForCooperative(int episode)
{
	//is this a custom level?
	if(episode>=MAX_NO_OF_COOPERATIVE_EPISODES)
	{
		//it certainly is
		sprintf(LevelName,"Custom/%s",GetCustomMultiplayerLevelName(episode,NGT_Coop));
	}
	else
	{
		strcpy(LevelName,RifNamesForEnvironments[CooperativeEpisodes[episode]]);
	}
}

void SetLevelToLoad(enum AVP_ENVIRONMENT_ID env)
{
	strcpy(LevelName,RifNamesForEnvironments[env]);
}
void SetLevelToLoadForCheatMode(int environment)
{
	if (environment<=10)
	{
		SetLevelToLoadForMarine(environment);
	}
	else if (environment>=22)
	{
		SetLevelToLoadForAlien(environment-22);
	}
	else
	{
		SetLevelToLoadForPredator(environment-11);
	}
}

int NumberForCurrentLevel(void) {

	int a;

	for (a=0; a<AVP_ENVIRONMENT_END_OF_MULTIPACK_LIST; a++) {
		if (strcmp(LevelName,RifNamesForEnvironments[a])==0) {
			return(a);
		}
	}

	return(a);
}



static BOOL DoesNamedLevelExist(const char* level_name)
{
	FILE *file_handle;
	char filename[200];
	
	sprintf(filename, "avp_rifs/%s.rif", level_name);

	file_handle = OpenGameFile(filename, FILEMODE_READONLY, FILETYPE_PERM);
	if(file_handle == NULL)	return FALSE;
	fclose(file_handle);

	return TRUE;
}

BOOL DoesMultiplayerLevelExist(int level)
{
	/*
	Check that the level number is valid , and the level actually exists on 
	the players hard drive.
	*/
	if(level<0 || level>=MAX_NO_OF_MULTIPLAYER_EPISODES) return FALSE;
	return DoesNamedLevelExist(RifNamesForEnvironments[MultiplayerEpisodes[level]]);

}

BOOL DoesCooperativeLevelExist(int level)
{
	/*
	Check that the level number is valid , and the level actually exists on 
	the players hard drive.
	*/
	if(level<0 || level>=MAX_NO_OF_COOPERATIVE_EPISODES) return FALSE;
	return DoesNamedLevelExist(RifNamesForEnvironments[CooperativeEpisodes[level]]);
}
