#ifndef EQUATES_INCLUDED
#define EQUATES_INCLUDED

/*

 Equates & Enums for AVP

*/

#ifdef __cplusplus
extern "C" {
#endif

#define MaxObjectLights 50			/* Sources attached to the object */

#define maxlightblocks 100			/* This ought to be MORE than enough */

#define MaxLightsPerObject 100	/* Sources lighting the object */


/*

 3d Texture Scan Subdivision limits

*/

#if 0 /* only used by krender.c */
#define lin_s_max 5

#if 0

	#define lin_s_zthr 320		/* 1.25 */

#else

	#if 1

		#define lin_s_zthr 281		/* 1.1 */

	#else

		#define lin_s_zthr 260		/* 1.01 */

	#endif

#endif
#endif


#define GlobalScale 1

/*
 Scenes and View Types
*/

/* not really used */
typedef enum {
	AVP_Scene0
} SCENE;


/*

 View Handler Function Array Indices

*/

/* VIEWSTATES isn't really used either */
typedef enum {

	VState_Inside,
	VState_RelativeRemote,
	VState_RelativeYRemote,
	VState_FixedRemote,
	VState_FlyBy,
	VState_LagRelRemote,
	VState_TrackingRemote,
	VState_LagRelYRemote,

	VState_Last

} VIEWSTATES;


/*

 View Interior Types

*/

/* ITYPES isn't really used either */
typedef enum {

	IType_Default,
	IType_Body,
	IType_Car,
	IType_Aircraft,

	IType_Last

} ITYPES;


/* Map Types */

typedef enum {

	MapType_Default,
	MapType_Player,
	MapType_PlayerShipCamera,
 	MapType_Sprite,
	MapType_Term

} AVP_MAP_TYPES;


/*  Strategies */

typedef enum {

	StrategyI_Null,
	StrategyI_Camera,
	StrategyI_Player,
	StrategyI_Test,
	StrategyI_NewtonTest,
	StrategyI_HomingTest,
	StrategyI_MissileTest,
	StrategyI_GravityOnly,
	StrategyI_Database,
	StrategyI_DoorPROX,
	StrategyI_Terminal,
	StrategyI_Last		/* Always the last */

} AVP_STRATEGIES;

/***********end for C++************/

#ifdef __cplusplus
};
#endif

#endif
