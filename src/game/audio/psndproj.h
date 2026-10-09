/* Patrick 5/6/97 -------------------------------------------------------------
  AvP Project sound header 
  ----------------------------------------------------------------------------*/
#ifndef PSNDPROJ_H
#define PSNDPROJ_H

#ifdef __cplusplus
extern "C" {
#endif

#include "equipmnt.h"

/* Andy 12/6/97 --------------------------------------------------------------
  Some background sound defines
----------------------------------------------------------------------------*/
#define BACKGROUND_VOLUME 80
#define BACKGROUND_ATTENUATION 5
#define ZONE_WIDTH_SHIFT 15    // Size of the sound zones (log 2)

#define ZONE_WIDTH 1<<ZONE_WIDTH_SHIFT

/* Patrick 5/6/97 -------------------------------------------------------------
  Enumeration of all the sounds that may be loaded/used in the game. Each
  of these corresponds to a data slot in the GameSounds[] array, and to the Id
  number specified for each sound in the sound data file 
  ----------------------------------------------------------------------------*/
typedef enum soundindex
{
	SID_PRED_LAUNCHER, // 0  //Used, plasmacaster fire
	SID_PRED_FRISBEE,	 // Yes!
	SID_PRED_PISTOL,  // Yes, pistol (Duh!)
	SID_PRED_SNARL,	//No
	SID_PRED_SCREAM1, //No
	SID_PRED_LASER, //Yes, speargun (!)
	SID_PULSE_START,  //Yes
	SID_PULSE_LOOP, //Yes
	SID_PULSE_END, //Yes
	SID_LIFT_START, //Remove?

	SID_LIFT_LOOP,		//10 //Remove?
	SID_LIFT_END, //Remove?
	SID_SWITCH1,		//Remove?
	SID_SWITCH2,		// not loaded
	SID_ALIEN_SCREAM,  // 14  //No
	SID_SWIPE,//No
	SID_SWISH,//No
	SID_TAIL, //No
	SID_VISION_ON, //Yes
	SID_VISION_LOOP,   // not loaded (Continuous pred background)
	
	SID_SWIPE2,			// 20 //No
	SID_SWIPE3,		//No
	SID_SWIPE4,		//No
	SID_PRED_HISS,	//No	
	SID_HIT_FLESH,	//No	
	SID_ALIEN_HIT,			//No
	SID_ALIEN_KILL, // Only used for queen death!
	SID_ALIEN_HISS, //No
	SID_ALIEN_HISS1,//No
	SID_FIRE,			// 29 //Yes

	SID_BUGDIE1,		// 30 //No
	SID_BUGDIE2,			  //No
	SID_BUGDIE3,			  //No
	SID_MARINE_DEATH1,		  //No
	SID_MARINE_DEATH2,		  //No
	SID_PRED_LOUDROAR,	 // 35 not loaded //No
	SID_MARINE_HIT, 	//No
	SID_ALIEN_HIT2,		//No
	SID_PICKUP, // Yes, misc pickups, e.g. security pass
	SID_RICOCH1, //Yes

	SID_RICOCH2,		// 40 //Yes
	SID_RICOCH3,//Yes
	SID_RICOCH4,//Yes
	SID_ARMSTART,		 // 43 //Xenoborg
	SID_ARMMID, //Xenoborg, sentrygun
	SID_ARMEND, //Xenoborg
	SID_PRED_SHORTROAR,	  // 46 not loaded
	SID_PRED_SLASH,		  // 47 not loaded
	SID_RIP,			  // 48 not loaded
	SID_PRED_NEWROAR, 	  // Medicomp stab!

	SID_SPLASH1,		  // 50 //Yes
	SID_SPLASH2, //Yes
	SID_SPLASH3, //yes
	SID_SPLASH4, //Yes
	SID_POWERUP, //Xenoborg
	SID_POWERDN, //Xenoborg
	SID_TELETEXT,//Yes
	SID_TRACKER_CLICK,//Yes
	SID_TRACKER_WHEEP,//Yes
	SID_ACID_SPRAY, //Facehugger damaged

	SID_DOORSTART,       // 60 //Yes
	SID_DOORMID, //Yes
	SID_DOOREND,	//Yes
	SID_BORGON,			// 63
	SID_SPARKS, //Yes
	SID_STOMP, //Xenoborg
	SID_LOADMOVE, //Xenoborg
	SID_FHUG_ATTACKLOOP, //Yes (FHug!)
	SID_FHUG_MOVE, //Yes (FHug!)
	SID_NOAMMO, //Yes (Smartgun click, sentrygun slick)

	SID_LONGLOAD,		  //70 //No
	SID_NADELOAD,//No
	SID_NADEFIRE,//Yes
	SID_NADEEXPLODE,//Yes
	SID_SHRTLOAD,	//No
	SID_INCIN_START, //Yes
	SID_INCIN_LOOP, //Yes
	SID_INCIN_END, //Yes
	SID_ROCKFIRE, //Yes (Grenade launcher)
  	SID_SHOTGUN,		//Yes (pistols)

	SID_SMART1,			  //80 //Yes
	SID_SMART2, //Yes
	SID_SMART3, //Yes
	SID_SENTRY_GUN, //Yes
	SID_SENTRY_END, //Yes
	SID_NICE_EXPLOSION, //Yes (SADARS etc)
	SID_EXPLOSION, //Yes (inanimates)
	SID_MINIGUN_END, //Yes
	SID_MINIGUN_LOOP, //Yes
	SID_SPEARGUN_HITTING_WALL, //Yes

	SID_FRAG_RICOCHETS,      //90  //Yes
	SID_PLASMABOLT_DISSIPATE, //Yes
	SID_PLASMABOLT_HIT, //Yes
	SID_BLOOD_SPLASH, //No
	SID_ALIEN_JAW_ATTACK,//Yes
	SID_TRACKER_WHEEP_HIGH, //Yes
	SID_TRACKER_WHEEP_LOW, //Yes
	SID_PULSE_RIFLE_FIRING_EMPTY, //Yes
	SID_THROW_FLARE, //Yes
	SID_BODY_BEING_HACKED_UP_0, //Yes

	SID_BODY_BEING_HACKED_UP_1, //100 //Yes
	SID_BODY_BEING_HACKED_UP_2,//Yes
	SID_BODY_BEING_HACKED_UP_3,//Yes
	SID_BODY_BEING_HACKED_UP_4,//Yes
	SID_CONSOLE_ACTIVATES,//Yes
	SID_CONSOLE_DEACTIVATES,//Yes
	SID_CONSOLE_MARINEMESSAGE,//Yes
	SID_CONSOLE_ALIENMESSAGE,//Yes
	SID_CONSOLE_PREDATORMESSAGE,//Yes
	SID_MINIGUN_READY, //No! WIL_MINIGUN_READY used instead.
	
	SID_MINIGUN_EMPTY, //110 //Yes
	SID_SMART_MODESWITCH,//Yes
	SID_GRENADE_BOUNCE,//Yes
	SID_BURNING_FLARE,//Yes
	SID_FLAMETHROWER_PILOT_LIGHT,//Not yet...
	SID_MARINE_JUMP_START,//No
	SID_MARINE_JUMP_END,//Not yet!
	SID_MARINE_PICKUP_WEAPON,//Yes
	SID_MARINE_PICKUP_AMMO,	 //Yes
	SID_MARINE_PICKUP_ARMOUR,//Yes
	
	SID_PREDATOR_PICKUP_FIELDCHARGE, //120 //Yes
	SID_PREDATOR_PICKUP_WEAPON, // In code, but never happens
	SID_PREDATOR_CLOAKING_ACTIVE,//Yes (vision modes)
	SID_PREDATOR_CLOAKING_DAMAGED,//Not yet!
	SID_PREDATOR_SPEARGUN_EMPTY,//In code, but never happens
	SID_PREDATOR_PLASMACASTER_TARGET_FOUND,//Yes
	SID_PREDATOR_PLASMACASTER_TARGET_LOCKED,//Yes
	SID_PREDATOR_PLASMACASTER_TARGET_LOST,//Yes
	SID_PREDATOR_PLASMACASTER_CHARGING,//Yes
	SID_PREDATOR_PLASMACASTER_EMPTY,// In code but never happens

	SID_PREDATOR_DISK_TARGET_LOCKED, //130 //Yes
	SID_PREDATOR_DISK_FLYING,//Yes
	SID_PREDATOR_DISK_HITTING_TARGET,//Yes
	SID_PREDATOR_DISK_HITTING_WALL,//Yes
	SID_PREDATOR_DISK_BEING_CAUGHT,//Yes
	SID_PREDATOR_DISK_RECOVERED,//Yes
	SID_PREDATOR_VOCAL_SNARL_1,//No
	SID_PREDATOR_VOCAL_SNARL_2,//No
	SID_ALIEN_TAILUNFURL, //No
	SID_ALIEN_TAUNT_1, //No

	SID_ALIEN_TAUNT_2,	// 140 //No
	SID_SENTRYGUN_LOCK, //Not at the moment
	SID_SENTRYGUN_SHUTDOWN, //Yes
	SID_WIL_MINIGUN_READY,	//Yes
	SID_SADAR_FIRE,//Yes
	SID_DISC_STICKSINWALL,//Yes
	SID_PREDATOR_PLASMACASTER_REDTRIANGLES,//Yes
	SID_WIL_PRED_PISTOL_EXPLOSION,//Yes
	SID_PROX_GRENADE_READYTOBLOW,//Yes
	SID_PROX_GRENADE_ACTIVE,	 //Yes

	SID_MARINE_JUMP_START_2, //150 //No
	SID_MARINE_JUMP_START_3,//No
	SID_MARINE_JUMP_START_4,//No 
	SID_ED_GRENADE_EXPLOSION,//Yes
	SID_ED_GRENADE_PROXEXPLOSION,//Yes
	SID_ED_MOLOTOV_EXPLOSION,//Yes
	SID_ED_LARGEWEAPONDROP,//Yes
	SID_MENUS_SELECT_ITEM,//Yes
	SID_MENUS_CHANGE_ITEM,//Yes
	SID_PRED_JUMP_START_1,//No
	SID_PRED_JUMP_START_2,//No
	SID_PRED_JUMP_START_3,//No
	SID_PRED_CLOAKON,//Yes
	SID_PRED_CLOAKOFF,//Yes
	SID_PRED_ZOOM_IN,
	SID_PRED_ZOOM_OUT,

	SID_MARINE_SMALLLANDING,
	SID_PRED_SMALLLANDING,
	SID_ED_FACEHUGGERSLAP,
	SID_LIGHT_FLICKER_ON, //Yes
	SID_ED_SENTRYTURN01,
	SID_PULSE_SWIPE01,
	SID_PULSE_SWIPE02,
	SID_PULSE_SWIPE03,
	SID_PULSE_SWIPE04,
	SID_ED_JETPACK_START,
	SID_ED_JETPACK_MID,
	SID_ED_JETPACK_END,
	SID_GRAPPLE_HIT_WALL,
	SID_GRAPPLE_THROW,
	SID_SENTRYGUNDEST,
	SID_ED_ELEC_DEATH,
	SID_IMAGE,
	SID_IMAGE_OFF,
	SID_PRED_CLOAK_DAMAGE,
	SID_ED_SKEETERLAUNCH,
	SID_ED_SKEETERPLASMAFIRE,
	SID_ED_SKEETERDISC_SPIN,
	SID_ED_SKEETERDISC_HITWALL,
	SID_ED_SKEETERCHARGE,
	SID_INTROWOOSH,

	SID_STARTOF_LOADSLOTS,
	SID_UNUSED_125,
  SID_UNUSED_126,
  SID_UNUSED_127,
  SID_UNUSED_128,
  SID_UNUSED_129,
  SID_UNUSED_130,
  SID_UNUSED_131,
  SID_UNUSED_132,
  SID_UNUSED_133,
  SID_UNUSED_134,
  SID_UNUSED_135,
  SID_UNUSED_136,
  SID_UNUSED_137,
  SID_UNUSED_138,
  SID_UNUSED_139,
  SID_UNUSED_140,
  SID_UNUSED_141,
  SID_UNUSED_142,
  SID_UNUSED_143,
  SID_UNUSED_144,
  SID_UNUSED_145,
  SID_UNUSED_146,
  SID_UNUSED_147,
  SID_UNUSED_148,
  SID_UNUSED_149,
  SID_UNUSED_150,
  SID_UNUSED_151,
  SID_UNUSED_152,
  SID_UNUSED_153,
  SID_UNUSED_154,
  SID_UNUSED_155,
  SID_UNUSED_156,
  SID_UNUSED_157,
  SID_UNUSED_158,
  SID_UNUSED_159,
  SID_UNUSED_160,
  SID_UNUSED_161,
  SID_UNUSED_162,
  SID_UNUSED_163,
  SID_UNUSED_164,
  SID_UNUSED_165,
  SID_UNUSED_166,
  SID_UNUSED_167,
  SID_UNUSED_168,
  SID_UNUSED_169,
  SID_UNUSED_170,
  SID_UNUSED_171,
  SID_UNUSED_172,
  SID_UNUSED_173,
  SID_UNUSED_174,
  SID_UNUSED_175,
  SID_UNUSED_176,
  SID_UNUSED_177,
  SID_UNUSED_178,
  SID_UNUSED_179,
  SID_UNUSED_180,
  SID_UNUSED_181,
  SID_UNUSED_182,
  SID_UNUSED_183,
  SID_UNUSED_184,
  SID_UNUSED_185,
  SID_UNUSED_186,
  SID_UNUSED_187,
  SID_UNUSED_188,
  SID_UNUSED_189,
  SID_UNUSED_190,
  SID_UNUSED_191,
  SID_UNUSED_192,
  SID_UNUSED_193,
  SID_UNUSED_194,
  SID_UNUSED_195,
  SID_UNUSED_196,
  SID_UNUSED_197,
  SID_UNUSED_198,
  SID_UNUSED_199,
  SID_UNUSED_200,
  SID_UNUSED_201,
  SID_UNUSED_202,
  SID_UNUSED_203,
  SID_UNUSED_204,
  SID_UNUSED_205,
  SID_UNUSED_206,
  SID_UNUSED_207,
  SID_UNUSED_208,
  SID_UNUSED_209,
  SID_UNUSED_210,
  SID_UNUSED_211,
  SID_UNUSED_212,
  SID_UNUSED_213,
  SID_UNUSED_214,
  SID_UNUSED_215,
  SID_UNUSED_216,
  SID_UNUSED_217,
  SID_UNUSED_218,
  SID_UNUSED_219,
  SID_UNUSED_220,
  SID_UNUSED_221,
  SID_UNUSED_222,
  SID_UNUSED_223,
  SID_UNUSED_224,
  SID_UNUSED_225,
  SID_UNUSED_226,
  SID_UNUSED_227,
  SID_UNUSED_228,
  SID_UNUSED_229,
  SID_UNUSED_230,
  SID_UNUSED_231,
  SID_UNUSED_232,
  SID_UNUSED_233,
  SID_UNUSED_234,
  SID_UNUSED_235,
  SID_UNUSED_236,
  SID_UNUSED_237,
  SID_UNUSED_238,
  SID_UNUSED_239,

	SID_ENDOF_LOADSLOTS=800,
    
	SID_MAXIMUM, /* SPECIAL: used to trap out of bounds values */ 		
	SID_NOSOUND	 /* SPECIAL: used to specify a null sound */ 		
}SOUNDINDEX;

/* Patrick 5/6/97 -------------------------------------------------------------
  Project level sound function prototypes
  ----------------------------------------------------------------------------*/
extern void DoPlayerSounds(void);
extern void MakeRicochetSound(VECTORCH *position);
extern void DoBackgroundSound(void);
extern void StopBackgroundSound(void);
extern void PlayAlienSwipeSound(void);
extern void PlayAlienTailSound(void);
extern void PlayPredSlashSound(void);
extern void PlayCudgelSound(void);

extern void MenuChangeSound(void);
extern void MenuSelectSound(void);
extern void MenuNotAvailableSound(int *handlePtr);
extern void MenuSliderBarSound(int *handlePtr);
void PlayWeaponClickingNoise(enum WEAPON_ID weaponIDNumber);
              

/* Patrick 5/6/97 -------------------------------------------------------------
  Sound data loader
  ----------------------------------------------------------------------------*/
/*Moved to psndproj since it now uses a project specific file to find the files -Richard */
extern void LoadSounds(char *soundDirectory);

//loads wav file locally or from network or from fast file as appropriate
extern int FindAndLoadWavFile(int soundNum,char* wavFileName);

/* Patrick 10/6/97 -------------------------------------------------------------
  Enumeration of CDDA tracks 
  ----------------------------------------------------------------------------*/
typedef enum cdtrackid
{
	CDTrack1 = 1,
  CDTrack2,
  CDTrack3,
  CDTrack4,
  CDTrack5,
  CDTrack6,
  CDTrack7,
  CDTrack8,
  CDTrack9,
  CDTrack10,
  CDTrack11,
  CDTrack12,
  CDTrack13,
  CDTrack14,
  CDTrack15,
  CDTrack16,
  CDTrack17,
  CDTrack18,
  CDTrack19,
  CDTrack20,
  CDTrack21,
  CDTrack22,
  CDTrack23,
  CDTrack24,
  CDTrack25,
  CDTrack26,
//  CDTrackMax, /* SPECIAL: used to trap out of bounds values */ 		
} CDTRACKID;

extern int CDTrackMax; //bas maximum cd track on the actual number of tracks

#ifdef __cplusplus
}
#endif

#endif

