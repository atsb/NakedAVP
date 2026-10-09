/*-----------------Patrick 15/1/97---------------------
  This header file supports	both the object visibility 
  management source, and pickup/inanimate objects....
  -----------------------------------------------------*/

#ifndef _pvisible_h_
	#define _pvisible_h_ 1

	#ifdef __cplusplus
		extern "C" {
	#endif

  	/* this enum defines the different inanimate object types ... */
	typedef enum inanimateobject_type
	{
		IOT_Non=-1,
		IOT_Static=0,
		IOT_Furniture,	
		IOT_Weapon,
		IOT_Ammo,
		IOT_Health,
		IOT_Armour,
		IOT_Key,
		IOT_BoxedSentryGun,
		IOT_IRGoggles,
		IOT_ReservedForJohn,
		IOT_DataTape,
	    IOT_MTrackerUpgrade,
		IOT_PheromonePod,
		IOT_SpecialPickupObject,
		IOT_HitMeAndIllDestroyBase,
		IOT_FieldCharge,
	} INANIMATEOBJECT_TYPE;


	/* this structure defines the datablock for pickup objects:
	weapons, armour, etc, and is also used for inanimate objects
	such as chairs, which are functionally quite similar
	(except that they can't be picked up, obviously) */
	
	typedef struct inan_frag
	{
		int ShapeIndex;
		int NumFrags;
	} INAN_FRAG;

	#define ObjectEventFlag_Destroyed 0x00000001
	#define ObjectEventFlag_PickedUp  0x00000002

	typedef struct object_event_target
	{
		int triggering_event;
		int request;
		char event_target_ID[SB_NAME_LENGTH]; 
		STRATEGYBLOCK* event_target_sbptr;
 	}OBJECT_EVENT_TARGET;
	
	typedef struct inanimateobjectstatusblock
	{
		INANIMATEOBJECT_TYPE typeId;
		int subType; /* weapon id, security level or other relevant enumerated type... */
		BOOL Indestructable;
		int respawnTimer;
		int lifespanTimer;//for dropped weapons in multiplayer that time out
		int startingHealth;
		int startingArmour;

		TXACTRLBLK *inan_tac;//for objects with anims on them

		OBJECT_EVENT_TARGET* event_target;//another strategy can be notified when this object is destroyed or picked up 

		int explosionType;//non zero for explosive objects
		int explosionTimer; //slight time delay after destruction for explosion
		int explosionStartFrame; //frame that explosion started

		INAN_FRAG * fragments;
		int num_frags;
		unsigned int ghosted_object:1;

	} INANIMATEOBJECT_STATUSBLOCK;

	/* tools interface */
	typedef struct tools_data_inanimateobject
	{
		struct vectorch position;
		struct euler orientation;
		INANIMATEOBJECT_TYPE typeId;
		int subType; /* weapon id, security level or other relevant enumerated type... */
		int shapeIndex;	/* for john */
		char nameID[SB_NAME_LENGTH];

		int mass; // Kilos??
		int integrity; // 0-20 (>20 = indestructable)
		
		int triggering_event;
		int event_request;
		char event_target_ID[SB_NAME_LENGTH]; 

		int explosionType;

		INAN_FRAG * fragments;
		int num_frags;
	} TOOLS_DATA_INANIMATEOBJECT;

	#define DEFAULT_OBJECT_INTEGRITY (100)
	#define NO_OF_FRAGMENTS_FROM_OBJECT (6)
	#define OBJECT_RESPAWN_TIME (ONE_FIXED*40)

	#define OBJECT_RESPAWN_NO_RESPAWN 0x7fffffff
	 

	/*--------------------------------------------
	  Prototypes
	  --------------------------------------------*/
	void InitObjectVisibilities(void);
	void DoObjectVisibilities(void);
	MODULE* ModuleFromPosition(VECTORCH *position, MODULE* startingModule);
	void DoObjectVisibility(STRATEGYBLOCK *sbPtr);
 	void InitInanimateObject(void* bhdata, STRATEGYBLOCK *sbPtr);
	void InanimateObjectBehaviour(STRATEGYBLOCK *sbPtr);
	void InanimateObjectIsDamaged(STRATEGYBLOCK *sbPtr, DAMAGE_PROFILE *damage, int multiple);
	void KillInanimateObjectForRespawn(STRATEGYBLOCK *sbPtr);
	void SendRequestToInanimateObject(STRATEGYBLOCK* sbptr,BOOL state,int extended_data);
	
	void MakeObjectNear(STRATEGYBLOCK *sbPtr);
	void MakeObjectFar(STRATEGYBLOCK *sbPtr);
	/* KJL 14:34:25 24/05/98 - does what it says */
	void RespawnAllObjects(void);

	void RespawnAllPickups(void);
	/*--------------------------------------------------
	  An external global that I need to make gun flashes
	  --------------------------------------------------*/
	extern MODULEMAPBLOCK VisibilityDefaultObjectMap;
	
	STRATEGYBLOCK* CreateMultiplayerWeaponPickup(VECTORCH* location,int type,char* name);
	void MakePlayersWeaponPickupVisible();
	#ifdef __cplusplus
		}
	#endif
#endif
