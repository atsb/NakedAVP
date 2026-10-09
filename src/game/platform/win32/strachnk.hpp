#ifndef strachnk_hpp
#define strachnk_hpp 1


#include "chunk.hpp"
#include "chnktype.hpp"
#include "ltfx_exp.h"


#define StratDoor 50
#define StratLift 51
#define StratBinSwitch 52
#define StratSimpleObject 53
#define StratPlatLift 54
#define StratAirlock 57
#define StratSwitchDoor 59
#define StratLiftNoTel 60
#define StratLinkSwitch 61
#define StratNewSimpleObject 62
#define StratConsole 63
#define StratLighting 64
#define StratMultiSwitch 66
#define StratTeleport 67
#define StratAreaSwitch 68
#define StratEnemy 69
#define StratMissionObjective 70
#define StratMissionHint 71
#define StratTrack 72
#define StratMessage 73
#define StratFan 75
#define StratHierarchy 76
#define StratDeathVolume 77
#define StratSelfDestruct 78
#define StratGenerator 79
#define StratTrackDestruct 80

#define StratSwingDoor 1000
#define StratPlacedBomb 1001
#define StratR6Switch	1002
#define StratR6SimpleObject 1003

#define StratMummyInanimate 2000
#define StratMummyPickup 2001
#define StratMummySimple 2002
#define StratMummyTriggerVolume 2003
#define StratMummyPivotObject_Old 2004
#define StratMummyChest 2005
#define StratMummyAlterCameraRange 2006
#define StratMummyPivotObject 2007


class  AvpStrat;

class AVP_Strategy_Chunk :public Chunk
{
	public:
	AVP_Strategy_Chunk(Chunk_With_Children* ,const char*,size_t);
	AVP_Strategy_Chunk(Chunk_With_Children*);
	~AVP_Strategy_Chunk();
	
	virtual BOOL output_chunk (HANDLE &hand);
	virtual size_t size_chunk();
	virtual void fill_data_block(char* data_start);

	int index;// for matching strategy up with the object it belongs to, currently only used by for light strategies
	int spare2;
	
	AvpStrat* Strategy; 
};

class AVP_External_Strategy_Chunk :public Chunk
{
	public:
	AVP_External_Strategy_Chunk(Chunk_With_Children* ,const char*,size_t);
	AVP_External_Strategy_Chunk(Chunk_With_Children*);
	~AVP_External_Strategy_Chunk();
	
	virtual BOOL output_chunk (HANDLE &hand);
	virtual size_t size_chunk();
	virtual void fill_data_block(char* data_start);

	ObjectID ObjID;
	int ExtEnvNum;
	int ThisEnvNum;
	int spare;
	
	AvpStrat* Strategy; 
};

// for attaching strategies to
class Virtual_Object_Chunk :public Chunk_With_Children
{
public:

	Virtual_Object_Chunk(Chunk_With_Children * parent)
	: Chunk_With_Children (parent, "VIOBJECT")
	{}


	// constructor from buffer
	Virtual_Object_Chunk (Chunk_With_Children * const parent,const char *, size_t const);


};
class Virtual_Object_Properties_Chunk: public Chunk
{
	public :
	Virtual_Object_Properties_Chunk(Chunk_With_Children* parent,const char* _name);
	Virtual_Object_Properties_Chunk(Chunk_With_Children* parent,const char*,size_t);
	~Virtual_Object_Properties_Chunk();

	virtual size_t size_chunk();
	virtual void fill_data_block(char* data_start);

	
	ChunkVectorInt location;
	int size;
	char* name;
	ObjectID ID;
	int pad1,pad2;
	
	ObjectID CalculateID();
};


class AvpStrat
{
	public:
	AvpStrat(const char* data_start);
	AvpStrat(int _StrategyType);
	virtual ~AvpStrat(){}
	int StrategyType;
	
	int Type,ExtraData;
	virtual size_t GetStrategySize();
	virtual void fill_data_block(char* data);
};

class MiscStrategy :public AvpStrat
{
	public:
	MiscStrategy(const char* data_start,size_t size);
	virtual ~MiscStrategy();

	int blocksize;
	char* datablock;
	
	virtual size_t GetStrategySize();
	virtual void fill_data_block(char* data);
};


#define SimStratFlag_NotifyTargetOnDestruction 0x00000001
#define SimStratFlag_NotifyTargetOnPickup	   0x00000002

#define SimStratFlag_SmallExplosion	0x00000010 //explosive barrel type things
#define SimStratFlag_BigExplosion	0x00000020
#define SimStratFlag_MolotovExplosion	0x00000030

#define SimStratFlag_ExplosionMask	0x00000030
#define SimStratFlag_ExplosionShift	4

//contained within the integrity
#define SimpleStrategy_SparesDontContainJunk 0x80000000

//and for r6
#define SimpleStrategy_R6IntegrityValid 0x40000000
#define SimpleStrategy_R6IntegrityMask 0xffff
#define SimpleStrategy_R6DestTypeMask 0xff0000
#define SimpleStrategy_R6DestTypeShift 16


class SimpleStrategy : public AvpStrat
{
public:
	SimpleStrategy(const char* data_start,size_t size);

	int Type,ExtraData;

	int mass;
	int integrity;

	int flags;
	int target_request;
	union
	{
		ObjectID targetID;
		unsigned char R6_Linked_Guards[4];
	};
		
	virtual size_t GetStrategySize();
	virtual void fill_data_block(char* data);

	unsigned int get_integrity() {return integrity &~ SimpleStrategy_SparesDontContainJunk;}

	unsigned int get_r6_integrity();
	void set_r6_integrity(unsigned int);
	unsigned int get_r6_destruction_type();
	void set_r6_destruction_type(unsigned int);
};

class R6SimpleStrategy : public SimpleStrategy
{
public :
	R6SimpleStrategy(const char* data_start,size_t);

	ObjectID r6SoundID; // id of a linked sound
	int r6_spare1;
	int r6_spare2;
	int r6_spare3;
	int r6_spare4;
	int r6_spare5;
	int r6_spare6;


	virtual size_t GetStrategySize();
	virtual void fill_data_block(char* data);
};



#define LiftFlag_Here					0x00000001
#define LiftFlag_Airlock				0x00000002
#define LiftFlag_NoTel					0x00000004 //switches aren't teleported
#define LiftFlag_ExitOtherSide			0x00000008 
struct ExtLift
{
	int EnvNum;
	int junk[4];
	ObjectID LiftID;
};

class LiftStrategy :public AvpStrat
{
	public:
	LiftStrategy(const char* data_start,size_t size);
	virtual ~LiftStrategy();
	
	int NumAssocLifts;
	ObjectID* AssocLifts;
	
	ObjectID AssocDoor;
	ObjectID AssocCallSwitch;
	ObjectID AssocFloorSwitch;
	int LiftFlags;
	int Floor;
	int NumExternalLifts;
	ExtLift* ExternalLifts;

	int Facing;

	virtual size_t GetStrategySize();
	virtual void fill_data_block(char* data);
};

#define PlatformLiftFlags_Disabled	0x00000001
#define PlatformLiftFlags_OneUse  	0x00000002
class PlatLiftStrategy :public AvpStrat
{
	public:
	PlatLiftStrategy(const char* data_start,size_t size);
	
	int flags;
	int spare[5];

	virtual size_t GetStrategySize();
	virtual void fill_data_block(char* data);
};

#define DoorFlag_Locked			0x00000001	 
#define DoorFlag_Proximity		0x00000002
#define DoorFlag_Open			0x00000004
#define DoorFlag_Lift			0x00000008
#define DoorFlag_Horizontal		0x00000010 //door lying horizontal

class DoorStrategy :public AvpStrat
{
	public:
	DoorStrategy(const char* data_start,size_t);
	
	int DoorFlags;
	unsigned char TimeToOpen; //in tenths of a second
	unsigned char TimeToClose; //in tenths of a second
	short spare;

	virtual size_t GetStrategySize();
	virtual void fill_data_block(char* data);
};

#define R6SwitchDoorFlag_SwitchOperated 0x00000001


class SwitchDoorStrategy : public AvpStrat
{
	public:
	SwitchDoorStrategy(const char* data_start,size_t);
	
	ObjectID AssocDoor;
	//union
	//{
		int spare1;
	//	struct
	//	{
	//		unsigned int r6_flags:20;
	//		unsigned int r6_open_time:6; //in tenths of a second
	//		unsigned int r6_close_time:6; //in tenths of a second
	//	};
	//};
	
	int spare2;

	virtual size_t GetStrategySize();
	virtual void fill_data_block(char* data);
};		

struct SwitchTarget
{
	ObjectID ID;
	int request;
};

#define BinSwitchFlag_StartsOn 0x00000001
#define BinSwitchFlag_OffMessageSame 0x00000002
#define BinSwitchFlag_OffMessageNone 0x00000004

class BinSwitchStrategy :public AvpStrat
{
	public :
	BinSwitchStrategy(const char* data_start,size_t);
	virtual ~BinSwitchStrategy(){};

	int flags;
	int Mode;
	int Time;
	int Security;
	SwitchTarget Target;

	virtual size_t GetStrategySize();
	virtual void fill_data_block(char* data);
};

class LinkSwitchStrategy :public BinSwitchStrategy
{
	public :
	LinkSwitchStrategy(const char* data_start,size_t);
	virtual ~LinkSwitchStrategy();

	int NumLinks;
	ObjectID* LinkedSwitches;

	virtual size_t GetStrategySize();
	virtual void fill_data_block(char* data);
};

#define MultiSwitchFlag_SwitchUpdated 0x80000000
#define MultiSwitchFlag_OffMessageSame 0x00000002
#define MultiSwitchFlag_OffMessageNone 0x00000004

#define MultiSwitchRequest_LinkedSwitch 0x00000002
class MultiSwitchStrategy :public AvpStrat
{
	public :
	MultiSwitchStrategy(const char* data_start,size_t);
	virtual ~MultiSwitchStrategy();

	int RestState;
	int Mode;
	int Time;
	int Security;
	
	int NumTargets;
	SwitchTarget* Targets;

	int NumLinks;
	ObjectID* LinkedSwitches;
	
	int pad1; //used to be UseRestStateAfter , so possible conversion upon load
	int flags;
	
	virtual size_t GetStrategySize();
	virtual void fill_data_block(char* data);
};

class AreaSwitchStrategy :public MultiSwitchStrategy
{
	public :
	AreaSwitchStrategy(const char* data_start,size_t);

	ChunkVectorInt trigger_min,trigger_max;
	int area_pad1,area_pad2;
	
	virtual size_t GetStrategySize();
	virtual void fill_data_block(char* data);
};



class ConsoleStrategy :public AvpStrat
{
	public :
	ConsoleStrategy(const char* data_start,size_t);

	int ConsoleNum;
	int spare1,spare2;
	
	virtual size_t GetStrategySize();
	virtual void fill_data_block(char* data);
};

class LightingStrategy : public AvpStrat
{
	public :
	LightingStrategy(const char* data_start,size_t);

	LightFXData LightData;

	int pad1,pad2,pad3;
	
	virtual size_t GetStrategySize();
	virtual void fill_data_block(char* data);
};

#define Teleport_All 0
#define Teleport_Marine 1
#define Teleport_Alien 2
#define Teleport_Predator 3

class TeleportStrategy : public AvpStrat
{
	public:
	TeleportStrategy(const char* data_start,size_t);
	
	ObjectID TeleportTo;
	int Type;
	int spare1,spare2;

	virtual size_t GetStrategySize();
	virtual void fill_data_block(char* data);
};		


class EnemyStrategy : public AvpStrat
{
	public:
	EnemyStrategy(const char* data_start,size_t);
	
	int MissionType;
	int target_request;
	ObjectID DeathTarget;

	int ExtraMissionData;
	int spares[3]; 


	virtual size_t GetStrategySize();
	virtual void fill_data_block(char* data);
};

class GeneratorStrategy : public AvpStrat
{
	public:
	GeneratorStrategy(const char* data_start,size_t);
	
	int MissionType;
	int ExtraMissionData;
	
	int spares[6]; 


	virtual size_t GetStrategySize();
	virtual void fill_data_block(char* data);
};


#define MissionFlag_Visible 0x00000001
#define MissionFlag_CurrentlyPossible  0x00000002

#define MissionTrigger_MakeVisible 0x00000001
#define MissionTrigger_MakePossible 0x00000002
#define MissionTrigger_DontComplete 0x00000004

enum MissionCompletionEffects
{
	MCE_None,
	MCE_CompleteLevel,
	MCE_Last,
};

struct MissionMessage
{
	ObjectID target_mission;
	int effect_on_target;
};

struct RequestTarget
{
	ObjectID target;
	int request;
};

class MissionObjectiveStrategy : public AvpStrat
{
	public:
	MissionObjectiveStrategy(const char* data_start,size_t);

	//indeces in english.txt
	int mission_description_string;
	int mission_complete_string;
	int mission_number;

	int flags;

	int mission_completion_effect;

	int num_mission_targets;
	MissionMessage* mission_targets;

	int num_request_targets;
	RequestTarget* request_targets;

	int pad1,pad2;
	
	virtual size_t GetStrategySize();
	virtual void fill_data_block(char* data);
};

class MissionHintStrategy : public AvpStrat
{
	public:
	MissionHintStrategy(const char* data_start,size_t);

	int mission_hint_string;
	int mission_number;
	int flags;

	int pad1,pad2;

	virtual size_t GetStrategySize();
	virtual void fill_data_block(char* data);
};		

#define TextMessageFlag_NotActiveAtStart 0x00000001

class TextMessageStrategy : public AvpStrat
{
	public:
	TextMessageStrategy(const char* data_start,size_t);

	int message_string;

	int flags,pad2;

	virtual size_t GetStrategySize();
	virtual void fill_data_block(char* data);
};


#define TrackRequestFlag_ActiveForward 0x00000001
#define TrackRequestFlag_ActiveBackward 0x00000002
#define TrackRequestFlag_OppositeBackward 0x00000004

struct TrackRequestTarget
{
	ObjectID targetID;
	int request;
	int flags;
};

struct TrackPointEffect
{
	int point_no;
	int num_targets;
	TrackRequestTarget* targets;
	int pad1;

	~TrackPointEffect()
	{
		if(targets) delete [] targets;
	}
	

};

class TrackStrategy : public AvpStrat
{
	public:
	TrackStrategy(const char* data_start,size_t);
	~TrackStrategy();

	int num_point_effects;
	TrackPointEffect** point_effects;

	int pad1;
	int pad2;
	
	virtual size_t GetStrategySize();
	virtual void fill_data_block(char* data);
};

class TrackDestructStrategy : public TrackStrategy
{
	public:
	TrackDestructStrategy(const char* data_start,size_t);

	int integrity;
	int target_request; 
	ObjectID targetID;	//target when blown up
	
	virtual size_t GetStrategySize();
	virtual void fill_data_block(char* data);
};

class HierarchyStrategy : public AvpStrat
{
	public:
	HierarchyStrategy(const char* data_start,size_t);
	~HierarchyStrategy();

	int num_point_effects;
	TrackPointEffect** point_effects;

	int pad1;
	int pad2;
	
	virtual size_t GetStrategySize();
	virtual void fill_data_block(char* data);
};

class FanStrategy : public AvpStrat
{
	public:
	FanStrategy(const char* data_start,size_t);

	int speed_up_time;
	int slow_down_time;
	int fan_wind_strength;
	int pad2;
	
	virtual size_t GetStrategySize();
	virtual void fill_data_block(char* data);
};

#define DeathVolumeFlag_StartsOn 0x00000001
#define DeathVolumeFlag_CollisionNotRequired 0x00000002
class DeathVolumeStrategy : public AvpStrat
{
	public:
	DeathVolumeStrategy(const char* data_start,size_t);

	ChunkVectorInt volume_min;
	ChunkVectorInt volume_max;
	int flags;
	int damage; //damage per second , 0 = infinite
	int pad2;
	
	virtual size_t GetStrategySize();
	virtual void fill_data_block(char* data);
};

class SelfDestructStrategy : public AvpStrat
{
	public:
	SelfDestructStrategy(const char* data_start,size_t);

	int timer;//in seconds
	int pad[4];
		
	virtual size_t GetStrategySize();
	virtual void fill_data_block(char* data);
};






/////////////////////rainbow 6 strategy alert/////////////////////////

#define SwingDoorFlag_Open 0x00000001
#define SwingDoorFlag_Locked 0x00000002
//flag set to show time_open has been set to the new default of 0
#define SwingDoorFlag_UpdatedTime 0x80000000
class SwingDoorStrategy : public AvpStrat
{
	public:
	SwingDoorStrategy(const char* data_start,size_t);

	int time_open;//in milliseconds
	ObjectID paired_door;
	ObjectID doorway_module;
	int flags;
	int time_to_pick;//in milliseconds
	int spare3;
	int spare4;
		
	virtual size_t GetStrategySize();
	virtual void fill_data_block(char* data);
};



#define BombFlag_TerroristActivate 0x00000001
#define BombFlag_Armed 			   0x00000002

#define BombType_Bomb 0
#define BombType_SecurityConsole 1
#define BombType_VirusCapsule 2
#define BombType_Computer 3
#define BombType_NonCriticalAlarm 4


class PlacedBombStrategy : public AvpStrat
{
	public:
	PlacedBombStrategy(const char* data_start,size_t);

	int type;
	int time;//seconds
	int flags;
	int integrity;

	
	unsigned char objective_number; //a value from 0 to 4
	unsigned short time_to_pick; //seconds
	unsigned char pad;
	
	int spare2,spare3,spare4;
		
	virtual size_t GetStrategySize();
	virtual void fill_data_block(char* data);
};

class R6SwitchStrategy : public AvpStrat
{
	public:
	R6SwitchStrategy(const char* data_start,size_t);

	ObjectID TargetID;
	
	
	int spare1,spare2,spare3,spare4;
		
	virtual size_t GetStrategySize();
	virtual void fill_data_block(char* data);	
};

/////////////////////Mummy strategy alert/////////////////////////
class MummyInanimateStrategy : public AvpStrat
{
public :
	MummyInanimateStrategy(const char* data_start,size_t);
	MummyInanimateStrategy();

	int destructionType;
	int health;   //0 to 100
	char generatedPickups[4];
	ObjectID linkedSoundID;
	ObjectID activateID;
	
	int spare1,spare2,spare3,spare4;

		
	virtual size_t GetStrategySize();
	virtual void fill_data_block(char* data);	
};


class MummyPickupStrategy : public AvpStrat
{
public :
	MummyPickupStrategy(const char* data_start,size_t);
	MummyPickupStrategy();

	int pickupType;
	BOOL inactiveAtStart;	
	int spare1,spare2,spare3,spare4;
		
	virtual size_t GetStrategySize();
	virtual void fill_data_block(char* data);	
};

class MummyTriggerVolumeStrategy : public AvpStrat
{
public :
	MummyTriggerVolumeStrategy(const char* data_start,size_t);

	ChunkVectorInt trigger_min,trigger_max;
	ObjectID targetID;

	int spare1,spare2,spare3,spare4;
		
	virtual size_t GetStrategySize();
	virtual void fill_data_block(char* data);
};

#define MummyPivotObject_Pillar 0
#define MummyPivotObject_Flagstone 1

class MummyPivotObjectStrategy : public AvpStrat
{
public :
	MummyPivotObjectStrategy(const char* data_start,size_t);

	int typeID; //pillar or flagstone
	int triggerDelay; //in milliseconds (fixed point seconds in game)
	ObjectID targetID;

	int spare1,spare2;

	ChunkVectorInt trigger_min,trigger_max;
		
	virtual size_t GetStrategySize();
	virtual void fill_data_block(char* data);
};

class MummyChestStrategy : public AvpStrat
{
public :
	MummyChestStrategy(const char* data_start,size_t);

	unsigned char objectives[4];

	ChunkVectorInt camera_location;
	int spare;
		
	virtual size_t GetStrategySize();
	virtual void fill_data_block(char* data);
};


#define MUMMY_CAMERA_AXIS_MIN_X 0
#define MUMMY_CAMERA_AXIS_MAX_X 1
#define MUMMY_CAMERA_AXIS_MIN_Z 2
#define MUMMY_CAMERA_AXIS_MAX_Z 3

class MummyAlterCameraRangeStrategy : public AvpStrat
{
public :
	MummyAlterCameraRangeStrategy(const char* data_start,size_t);

	ChunkVectorInt zone_min,zone_max;
	int enter_range; // camera range in mm for one side of he zone
	int exit_range; // ditto for opposite side

	int axis; //axis that the change happens across
		
	int spare1,spare2,spare3,spare4;

		
	virtual size_t GetStrategySize();
	virtual void fill_data_block(char* data);
};

#endif
