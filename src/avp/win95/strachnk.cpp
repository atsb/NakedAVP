#include "strachnk.hpp"
#include "md5.h"

//macro for helping to force inclusion of chunks when using libraries
FORCE_CHUNK_INCLUDE_IMPLEMENT(strachnk)

RIF_IMPLEMENT_DYNCREATE("AVPSTRAT",AVP_Strategy_Chunk)

AVP_Strategy_Chunk::AVP_Strategy_Chunk(Chunk_With_Children* parent,const char* data,size_t datasize)
:Chunk(parent,"AVPSTRAT")
{
	index= *(int*)data;
	data+=4;
	spare2= *(int*)data;
	data+=4;

	int type=*((int*)data);
	switch(type)
	{
		case StratLift :
			Strategy=new LiftStrategy(data,datasize-8);
			break;
		case StratDoor :
			Strategy=new DoorStrategy(data,datasize-8);
			break;
		case StratBinSwitch :
			Strategy=new BinSwitchStrategy(data,datasize-8);
			break;
		case StratLinkSwitch :
			Strategy=new LinkSwitchStrategy(data,datasize-8);
	 		break;
	 	case StratSimpleObject :
	 	case StratMummySimple :
	 		Strategy=new AvpStrat(data);
			break;
	 	case StratNewSimpleObject :
	 		Strategy=new SimpleStrategy(data,datasize-8);
			break;
		case StratPlatLift :
			Strategy=new PlatLiftStrategy(data,datasize-8);
			break;
		case StratSwitchDoor :
			Strategy=new SwitchDoorStrategy(data,datasize-8);
			break;
		case StratConsole :
			Strategy=new ConsoleStrategy(data,datasize-8);
			break;
		case StratLighting :
			Strategy=new LightingStrategy(data,datasize-8);
			break;
		case StratMultiSwitch :
			Strategy=new MultiSwitchStrategy(data,datasize-8);
			break;
		case StratAreaSwitch :
			Strategy=new AreaSwitchStrategy(data,datasize-8);
			break;
		case StratTeleport :
			Strategy=new TeleportStrategy(data,datasize-8);
			break;
		case StratEnemy :
			Strategy=new EnemyStrategy(data,datasize-8);
			break;
		case StratMissionObjective :
			Strategy=new MissionObjectiveStrategy(data,datasize-8);
			break;
		case StratTrack :
			Strategy=new TrackStrategy(data,datasize-8);
			break;
		case StratTrackDestruct :
			Strategy=new TrackDestructStrategy(data,datasize-8);
			break;
		case StratMessage :
			Strategy=new TextMessageStrategy(data,datasize-8);
			break;
		case StratFan :
			Strategy=new FanStrategy(data,datasize-8);
			break;
		case StratHierarchy :
			Strategy=new HierarchyStrategy(data,datasize-8);
			break;
		
		case StratDeathVolume :
			Strategy=new DeathVolumeStrategy(data,datasize-8);
			break;
		
		case StratSelfDestruct :
			Strategy=new SelfDestructStrategy(data,datasize-8);
			break;

		case StratGenerator :
			Strategy=new GeneratorStrategy(data,datasize-8);
			break;

		case StratSwingDoor :
			Strategy=new SwingDoorStrategy(data,datasize-8);
			break;

		case StratPlacedBomb :
			Strategy=new PlacedBombStrategy(data,datasize-8);
			break;
		
		case StratR6Switch :
			Strategy=new R6SwitchStrategy(data,datasize-8);
			break;

		case StratR6SimpleObject :
			Strategy = new R6SimpleStrategy(data,datasize-8);
			break;

		case StratMummyInanimate :
			Strategy = new MummyInanimateStrategy(data,datasize - 8);
			break;

		case StratMummyPickup :
			Strategy = new MummyPickupStrategy(data,datasize - 8);
			break;
	
		case StratMummyTriggerVolume :
			Strategy = new MummyTriggerVolumeStrategy(data,datasize - 8);
			break;
	
		case StratMummyPivotObject_Old :
		case StratMummyPivotObject :
			Strategy = new MummyPivotObjectStrategy(data,datasize - 8);
			break;

		case StratMummyChest :
			Strategy = new MummyChestStrategy(data,datasize - 8);
			break;
		
		case StratMummyAlterCameraRange :
			Strategy = new MummyAlterCameraRangeStrategy(data,datasize - 8);
			break;
	   
		default :
			Strategy=new MiscStrategy(data,datasize-8);	
	}
}

AVP_Strategy_Chunk::AVP_Strategy_Chunk(Chunk_With_Children* parent)
:Chunk(parent,"AVPSTRAT")
{
	Strategy=0;
	index=spare2=0;
}

AVP_Strategy_Chunk::~AVP_Strategy_Chunk()
{
	delete Strategy;
}

BOOL AVP_Strategy_Chunk::output_chunk (HANDLE &hand)
{
	unsigned long junk;
	BOOL ok;
	char * data_block;

	data_block = this->make_data_block_from_chunk();

	ok = WriteFile (hand, (long *) data_block, (unsigned long) chunk_size, &junk, 0);

	delete [] data_block;

	if (!ok) return FALSE;

	return TRUE;
}

size_t AVP_Strategy_Chunk::size_chunk()
{
	chunk_size=20+Strategy->GetStrategySize();
	return chunk_size;
}

void AVP_Strategy_Chunk::fill_data_block(char* data_start)
{
	strncpy (data_start, identifier, 8);

	data_start += 8;

	*((int *) data_start) = chunk_size;

	data_start += 4;

	*((int *) data_start) = index;
	data_start += 4;
	*((int *) data_start) = spare2;
	data_start += 4;
	
	Strategy->fill_data_block(data_start);
}

////////////////////////////////////////////////////////
//Class AvP_External_Strategy_Chunk
RIF_IMPLEMENT_DYNCREATE("AVPEXSTR",AVP_External_Strategy_Chunk)

AVP_External_Strategy_Chunk::AVP_External_Strategy_Chunk(Chunk_With_Children* parent,const char* data,size_t datasize)
:Chunk(parent,"AVPEXSTR")
{
	ObjID=*(ObjectID*)data;
	data+=8;
	ExtEnvNum=*(int*)data;
	data+=4;
	ThisEnvNum= *(int*)data;
	data+=4;
	spare= *(int*)data;
	data+=4;

	int type=*((int*)data);
	switch(type)
	{
		case StratLift :
			Strategy=new LiftStrategy(data,datasize-8);
			break;
		case StratDoor :
			Strategy=new DoorStrategy(data,datasize-8);
			break;
		case StratBinSwitch :
			Strategy=new BinSwitchStrategy(data,datasize-8);
			break;
	 	case StratSimpleObject :
	 		Strategy=new AvpStrat(data);
			break;
		case StratPlatLift :
			Strategy=new PlatLiftStrategy(data,datasize-8);
			break;
		default :
			Strategy=new MiscStrategy(data,datasize-8);	
	}
}

AVP_External_Strategy_Chunk::AVP_External_Strategy_Chunk(Chunk_With_Children* parent)
:Chunk(parent,"AVPEXSTR")
{
	ObjID.id1=ObjID.id2=0;
	ExtEnvNum=0;
	Strategy=0;
	ThisEnvNum=0;
	spare=0;
}

AVP_External_Strategy_Chunk::~AVP_External_Strategy_Chunk()
{
	delete Strategy;
}

BOOL AVP_External_Strategy_Chunk::output_chunk (HANDLE &hand)
{
	unsigned long junk;
	BOOL ok;
	char * data_block;

	data_block = this->make_data_block_from_chunk();

	ok = WriteFile (hand, (long *) data_block, (unsigned long) chunk_size, &junk, 0);

	delete [] data_block;

	if (!ok) return FALSE;

	return TRUE;
}

size_t AVP_External_Strategy_Chunk::size_chunk()
{
	chunk_size=32+Strategy->GetStrategySize();
	return chunk_size;
}

void AVP_External_Strategy_Chunk::fill_data_block(char* data_start)
{
	strncpy (data_start, identifier, 8);

	data_start += 8;

	*((int *) data_start) = chunk_size;

	data_start += 4;

	*((ObjectID *) data_start) =ObjID;
	data_start += 8;
	*((int *) data_start) = ExtEnvNum;
	data_start += 4;
	*((int *) data_start) = ThisEnvNum;
	data_start += 4;
	*((int *) data_start) = spare;
	data_start += 4;
	
	Strategy->fill_data_block(data_start);
}
/////////////////////////////////////////////////////////////
//Class Virtual_Object_Chunk

RIF_IMPLEMENT_DYNCREATE("VIOBJECT",Virtual_Object_Chunk)
CHUNK_WITH_CHILDREN_LOADER("VIOBJECT",Virtual_Object_Chunk)

/*
Children for Virtual_Object_Chunk

"AVPSTRAT"		AVP_Strategy_Chunk
"VOBJPROP"		Virtual_Object_Properties_Chunk
*/

////////////////////////////////////////////////////////
//Class Virtual_Object_Properties_Chunk

RIF_IMPLEMENT_DYNCREATE("VOBJPROP",Virtual_Object_Properties_Chunk)

ObjectID Virtual_Object_Properties_Chunk::CalculateID()
{
	ID.id1=ID.id2=0;
	
	char Name[100];

	/*Add on a few extra letters so that I don't have to worry about having the same name
	as any normal objects etc*/
	strcpy(Name,"VirtOb");
	strcat(Name,name);
	
	char buffer[16];
	md5_buffer(Name,strlen(Name),&buffer[0]);
	buffer[7]=0;
	ID = *(ObjectID*)&buffer[0];
	return ID;

}

Virtual_Object_Properties_Chunk::Virtual_Object_Properties_Chunk(Chunk_With_Children* parent,const char* _name)
:Chunk(parent,"VOBJPROP")
{
	location.x=location.y=location.z=0;
	size=1000;
	pad1=pad2=0;
	name=new char[strlen(_name)+1];
	strcpy(name,_name);		
	CalculateID();
}

Virtual_Object_Properties_Chunk::Virtual_Object_Properties_Chunk(Chunk_With_Children* parent,const char* data,size_t)
:Chunk(parent,"VOBJPROP")
{
	location=*(ChunkVectorInt*)data;
	data+=sizeof(ChunkVectorInt);
	size=*(int*)data;
	data+=sizeof(int);
	
	int length=strlen(data);
	name=new char[length+1];
	strcpy(name,data);
	data+=(length+4)&~3;
	
	ID=*(ObjectID*)data;
	data+=sizeof(ObjectID);

	pad1=*(int*)data;
	data+=sizeof(int);				
	pad2=*(int*)data;
	data+=sizeof(int);				
}

Virtual_Object_Properties_Chunk::~Virtual_Object_Properties_Chunk()
{
	if(name)delete [] name;
}

size_t Virtual_Object_Properties_Chunk::size_chunk()
{
	chunk_size=12+sizeof(ChunkVectorInt)+3*sizeof(int)+sizeof(ObjectID);
	chunk_size+=(strlen(name)+4)&~3;
	return chunk_size;
}

void Virtual_Object_Properties_Chunk::fill_data_block(char* data_start)
{
	strncpy (data_start, identifier, 8);
	data_start += 8;
	*((int *) data_start) = chunk_size;
	data_start += 4;

	*(ChunkVectorInt*)data_start=location;
	data_start+=sizeof(ChunkVectorInt);
	*(int*)data_start=size;
	data_start+=sizeof(int);
	
	strcpy(data_start,name);
	data_start+=(strlen(name)+4)&~3;
	
	*(ObjectID*)data_start=ID;
	data_start+=sizeof(ObjectID);
	
	*(int*)data_start=pad1;
	data_start+=sizeof(int);
	*(int*)data_start=pad2;
	data_start+=sizeof(int);
	
}

/////////////////////////////////////////////////////////////


AvpStrat::AvpStrat(const char* data_start)
{
	StrategyType=*((int*)data_start);
	data_start+=4;
	Type=*((int*)data_start);
	data_start+=4;
	ExtraData=*((int*)data_start);

}				   

AvpStrat::AvpStrat(int _StrategyType)
{
	StrategyType = _StrategyType;
	Type=ExtraData=0;
}

size_t AvpStrat::GetStrategySize()
{
	return 12;
}

void AvpStrat::fill_data_block(char* data)
{
	*(int*)data=StrategyType;
	data+=4;
	*(int*)data=Type;
	data+=4;
	*(int*)data=ExtraData;
}					 

MiscStrategy::MiscStrategy(const char* data_start,size_t size)
:AvpStrat(data_start)
{
	data_start+=AvpStrat::GetStrategySize();
	blocksize=size-AvpStrat::GetStrategySize();
	if(blocksize)
	{
		datablock=new char[blocksize];
		memcpy(datablock,data_start,blocksize);		
	}
	else
		datablock=0;
}


MiscStrategy::~MiscStrategy()
{
	delete [] datablock;
}

size_t MiscStrategy::GetStrategySize()
{
	return blocksize+AvpStrat::GetStrategySize();
}

void MiscStrategy::fill_data_block(char* data)
{
	AvpStrat::fill_data_block(data);
	data+=AvpStrat::GetStrategySize();

	memcpy(data,datablock,blocksize);
}
//////////////////////////////////////////////////////////////////
SimpleStrategy::SimpleStrategy(const char * data_start,size_t)
:AvpStrat(data_start)
{
	data_start+=AvpStrat::GetStrategySize();

	Type = *((int *)data_start);
	data_start +=4;
	
	ExtraData = *((int *)data_start);
	data_start +=4;
	
	mass = *((int *)data_start);
	data_start +=4;
	
	integrity = *((int *)data_start);
	data_start +=4;
	
	flags = *((int *)data_start);
	data_start +=4;
	
	target_request = *((int *)data_start);
	data_start +=4;
	
	targetID = *((ObjectID *)data_start);

	if(!(integrity & SimpleStrategy_SparesDontContainJunk))
	{
		integrity|=SimpleStrategy_SparesDontContainJunk;
		flags=0;
		target_request=0;
		targetID.id1=0;
		targetID.id2=0;
	}
	
}

size_t SimpleStrategy::GetStrategySize()
{
	return 32+AvpStrat::GetStrategySize();
}

void SimpleStrategy::fill_data_block(char* data)
{
	AvpStrat::fill_data_block(data);
	data+=AvpStrat::GetStrategySize();

	*((int *)data) = Type;
	data +=4;
	
	*((int *)data) = ExtraData;
	data +=4;
	
	*((int *)data) = mass;
	data +=4;
	
	*((int *)data) = integrity;
	data +=4;
	
	*((int *)data) = flags;
	data +=4;
	
	*((int *)data) = target_request;
	data +=4;
	
	*((ObjectID *)data) = targetID;
}

unsigned int SimpleStrategy::get_r6_integrity()
{
	if(integrity & SimpleStrategy_R6IntegrityValid)
	{
		return integrity & SimpleStrategy_R6IntegrityMask; 
	}
	else
	{
		return 100;
	}
}
void SimpleStrategy::set_r6_integrity(unsigned int integ)
{
	if(integ>100) integ = 100;
	
	integrity &=~SimpleStrategy_R6IntegrityMask;
	integrity |= integ |  SimpleStrategy_R6IntegrityValid;
}

unsigned int SimpleStrategy::get_r6_destruction_type()
{
	return (integrity & SimpleStrategy_R6DestTypeMask) >> SimpleStrategy_R6DestTypeShift; 
}

void SimpleStrategy::set_r6_destruction_type(unsigned int dest_type)
{
	if(dest_type>200) dest_type = 200;

	integrity &=~ SimpleStrategy_R6DestTypeMask;
	integrity |= dest_type << SimpleStrategy_R6DestTypeShift;  	
}

//////////////////////////////////////////////

R6SimpleStrategy::R6SimpleStrategy(const char * data,size_t size)
:SimpleStrategy(data,size)
{
	data+=SimpleStrategy::GetStrategySize();

	CHUNK_EXTRACT(r6SoundID,ObjectID);
	CHUNK_EXTRACT(r6_spare1,int);
	CHUNK_EXTRACT(r6_spare2,int);
	CHUNK_EXTRACT(r6_spare3,int);
	CHUNK_EXTRACT(r6_spare4,int);
	CHUNK_EXTRACT(r6_spare5,int);
	CHUNK_EXTRACT(r6_spare6,int);

}

size_t R6SimpleStrategy::GetStrategySize()
{
	return 6*sizeof(int)+sizeof(ObjectID) + SimpleStrategy::GetStrategySize();
}

void R6SimpleStrategy::fill_data_block(char* data)
{
	SimpleStrategy::fill_data_block(data);
	data+=SimpleStrategy::GetStrategySize();

	CHUNK_FILL(r6SoundID,ObjectID);
	CHUNK_FILL(r6_spare1,int);
	CHUNK_FILL(r6_spare2,int);
	CHUNK_FILL(r6_spare3,int);
	CHUNK_FILL(r6_spare4,int);
	CHUNK_FILL(r6_spare5,int);
	CHUNK_FILL(r6_spare6,int);

}


//////////////////////////////////////////////

LiftStrategy::LiftStrategy(const char* data_start,size_t /*size*/)
:AvpStrat(data_start)
{

	data_start+=AvpStrat::GetStrategySize();
	
	NumAssocLifts=*(int*)data_start;
	data_start+=4;
	if(NumAssocLifts)
	{
		AssocLifts=new ObjectID[NumAssocLifts];
		for(int i=0;i<NumAssocLifts;i++)
		{
			AssocLifts[i]=*(ObjectID*)data_start;
			data_start+=8;
		}
	}
	else
		AssocLifts=0;
	
	
	AssocDoor=*(ObjectID*)data_start;
	data_start+=8;
	AssocCallSwitch=*(ObjectID*)data_start;
	data_start+=8;
	AssocFloorSwitch=*(ObjectID*)data_start;
	data_start+=8;

	LiftFlags=*(int*)data_start;

	data_start+=4;
	Floor=*(int*)data_start;
	data_start+=4;
	
	NumExternalLifts=*(int*)data_start;
	data_start+=4;
	if(NumExternalLifts)
	{
		ExternalLifts=new ExtLift[NumExternalLifts];
		for(int i=0;i<NumExternalLifts;i++)
		{
			ExternalLifts[i]=*(ExtLift*)data_start;
			data_start+=sizeof(ExtLift);
		}
	}
	else
	{
		ExternalLifts=0;
	}
	Facing=*(int*)data_start;
}

LiftStrategy::~LiftStrategy()
{
	if (AssocLifts) delete [] AssocLifts;
	if (ExternalLifts) delete [] ExternalLifts;	
}

size_t LiftStrategy::GetStrategySize()
{
	int size=AvpStrat::GetStrategySize();
	size+=44+8*NumAssocLifts;
	size+=NumExternalLifts*sizeof(ExtLift);
	return size;
}

void LiftStrategy::fill_data_block(char* data)
{
	AvpStrat::fill_data_block(data);
	data+=AvpStrat::GetStrategySize();
	

	(*(int*)data)=NumAssocLifts;
	data+=4;
	for(int i=0;i<NumAssocLifts;i++)
	{
		*(ObjectID*)data=AssocLifts[i];
		data+=8;
	}
	*(ObjectID*)data=AssocDoor;
	data+=8;
	*(ObjectID*)data=AssocCallSwitch;
	data+=8;
	*(ObjectID*)data=AssocFloorSwitch;
	data+=8;

	(*(int*)data)=LiftFlags;
	data+=4;
	*(int*)data=Floor;
	data+=4;
	*(int*)data=NumExternalLifts;
	data+=4;
	for(int i=0;i<NumExternalLifts;i++)
	{
		*(ExtLift*)data=ExternalLifts[i];
		data+=sizeof(ExtLift);
	}
	*(int*)data=Facing;
}
PlatLiftStrategy::PlatLiftStrategy(const char* data_start,size_t /*size*/)
:AvpStrat(data_start)
{

	data_start+=AvpStrat::GetStrategySize();
	
	flags=*(int*)data_start;
	data_start+=4;
	
	for(int i=0;i<5;i++)
	{
		spare[i]=*(int*)data_start;
		data_start+=4;
	}
}


size_t PlatLiftStrategy::GetStrategySize()
{
	int size=AvpStrat::GetStrategySize();
	size+=24;
	return size;
}

void PlatLiftStrategy::fill_data_block(char* data)
{
	AvpStrat::fill_data_block(data);
	data+=AvpStrat::GetStrategySize();
	

	*(int*)data=flags;
	data+=4;
	for(int i=0;i<5;i++)
	{
		*(int*)data=spare[i];
		data+=4;
	}

}

DoorStrategy::DoorStrategy(const char* data_start,size_t /*size*/ )
:AvpStrat(data_start)
{
	data_start+=AvpStrat::GetStrategySize();
	DoorFlags=*(int*)data_start;
	data_start+=4;
	TimeToOpen=*(unsigned char*)data_start;
	data_start++;
	TimeToClose=*(unsigned char*)data_start;
	data_start++;
	spare=*(short*)data_start;
}

size_t DoorStrategy::GetStrategySize()
{
	return 8+AvpStrat::GetStrategySize();
}

void DoorStrategy::fill_data_block(char* data)
{
	AvpStrat::fill_data_block(data);
	data+=AvpStrat::GetStrategySize();
	*(int*)data=DoorFlags;		
	data+=4;
	*(unsigned char*)data=TimeToOpen;
	data++;
	*(unsigned char*)data=TimeToClose;
	data++;
	*(short*)data=spare;
}

SwitchDoorStrategy::SwitchDoorStrategy(const char* data_start,size_t /*size*/ )
:AvpStrat(data_start)
{
	data_start+=AvpStrat::GetStrategySize();
	AssocDoor=*(ObjectID*)data_start;
	data_start+=sizeof(ObjectID);
	spare1=*(int*)data_start;
	data_start+=4;
	spare2=*(int*)data_start;
}

size_t SwitchDoorStrategy::GetStrategySize()
{
	return 16+AvpStrat::GetStrategySize();
}

void SwitchDoorStrategy::fill_data_block(char* data)
{
	AvpStrat::fill_data_block(data);
	data+=AvpStrat::GetStrategySize();
	*(ObjectID*)data=AssocDoor;		
	data+=sizeof(ObjectID);
	*(int*)data=spare1;
	data+=4;
	*(int*)data=spare2;
}

BinSwitchStrategy::BinSwitchStrategy(const char* data_start,size_t /*size*/)
:AvpStrat(data_start)
{
	data_start+=AvpStrat::GetStrategySize();
	
	flags=*(int*)data_start;
	data_start+=4;
	Mode=*(int*)data_start;
	data_start+=4;
	Time=*(int*)data_start;
	data_start+=4;
	Security=*(int*)data_start;
	data_start+=4;
	Target=*(SwitchTarget*)data_start;
}



size_t BinSwitchStrategy::GetStrategySize()
{
	return AvpStrat::GetStrategySize()+28;
	
}

void BinSwitchStrategy::fill_data_block(char* data)
{
	AvpStrat::fill_data_block(data);
	data+=AvpStrat::GetStrategySize();
	
	*(int*)data=flags;
	data+=4;
	*(int*)data=Mode;
	data+=4;
	*(int*)data=Time;
	data+=4;
	*(int*)data=Security;
	data+=4;
	*(SwitchTarget*)data=Target;
	data+=sizeof(SwitchTarget);
	
}

LinkSwitchStrategy::LinkSwitchStrategy(const char* data_start,size_t size)
:BinSwitchStrategy(data_start,size)
{
	data_start+=BinSwitchStrategy::GetStrategySize();
	
	NumLinks=*(int*)data_start;
	data_start+=4;
	if(NumLinks)
	{
		LinkedSwitches=new ObjectID[NumLinks];
		for(int i=0;i<NumLinks;i++)
		{
			LinkedSwitches[i]=*(ObjectID*)data_start;
			data_start+=sizeof(ObjectID);
		}
	}
	else
		LinkedSwitches=0;

}

LinkSwitchStrategy::~LinkSwitchStrategy()
{
	delete [] LinkedSwitches;
}

size_t LinkSwitchStrategy::GetStrategySize()
{
	return BinSwitchStrategy::GetStrategySize()+4+NumLinks*sizeof(ObjectID);
	
}

void LinkSwitchStrategy::fill_data_block(char* data)
{
	BinSwitchStrategy::fill_data_block(data);
	data+=BinSwitchStrategy::GetStrategySize();
	
	*(int*)data=NumLinks;
	data+=4;
	for(int i=0;i<NumLinks;i++)
	{
		*(ObjectID*)data=LinkedSwitches[i];
		data+=sizeof(ObjectID);
	}
}



ConsoleStrategy::ConsoleStrategy(const char* data_start,size_t)
:AvpStrat(data_start)
{
	data_start+=AvpStrat::GetStrategySize();
	
	ConsoleNum=*(int*)data_start;
	data_start+=4;
	spare1=*(int*)data_start;
	data_start+=4;
	spare2=*(int*)data_start;
}
size_t ConsoleStrategy::GetStrategySize()
{
	return AvpStrat::GetStrategySize()+12;
	
}

void ConsoleStrategy::fill_data_block(char* data)
{
	AvpStrat::fill_data_block(data);
	data+=AvpStrat::GetStrategySize();
	
	*(int*)data=ConsoleNum;
	data+=4;
	*(int*)data=spare1;
	data+=4;
	*(int*)data=spare2;
}



LightingStrategy::LightingStrategy(const char* data_start,size_t /*size */)
:AvpStrat(data_start)
{
	data_start+=AvpStrat::GetStrategySize();
	
	LightData.type=*(unsigned long*)data_start;
	data_start+=4;
	LightData.init_state=*(unsigned long*)data_start;
	data_start+=4;
	LightData.fade_up_speed=*(unsigned long*)data_start;
	data_start+=4;
	LightData.fade_down_speed=*(unsigned long*)data_start;
	data_start+=4;
	LightData.post_fade_up_delay=*(unsigned long*)data_start;
	data_start+=4;
	LightData.post_fade_down_delay=*(unsigned long*)data_start;
	data_start+=4;

	pad1=*(int*)data_start;
	data_start+=4;
	pad2=*(int*)data_start;
	data_start+=4;
	pad3=*(int*)data_start;
}
size_t LightingStrategy::GetStrategySize()
{
	return 36+AvpStrat::GetStrategySize();
}

void LightingStrategy::fill_data_block(char* data)
{
	AvpStrat::fill_data_block(data);
	data+=AvpStrat::GetStrategySize();
	
	*(unsigned long*)data=LightData.type;
	data+=4;
	*(unsigned long*)data=LightData.init_state;
	data+=4;
	*(unsigned long*)data=LightData.fade_up_speed;
	data+=4;
	*(unsigned long*)data=LightData.fade_down_speed;
	data+=4;
	*(unsigned long*)data=LightData.post_fade_up_delay;
	data+=4;
	*(unsigned long*)data=LightData.post_fade_down_delay;
	data+=4;
	*(int*)data=pad1;
	data+=4;
	*(int*)data=pad2;
	data+=4;
	*(int*)data=pad3;
	data+=4;
}



MultiSwitchStrategy::MultiSwitchStrategy(const char* data_start,size_t /*size*/)
:AvpStrat(data_start)
{
	data_start+=AvpStrat::GetStrategySize();

	RestState=*(int*) data_start;
	data_start+=4;
	Mode=*(int*) data_start;
	data_start+=4;
	Time=*(int*) data_start;
	data_start+=4;
	Security=*(int*) data_start;
	data_start+=4;

	NumTargets=*(int*) data_start;
	data_start+=4;
	
	if(NumTargets)
	{
		Targets=new SwitchTarget[NumTargets];
		for(int i=0;i<NumTargets;i++)
		{
			Targets[i]=*(SwitchTarget*)data_start;
			data_start+=sizeof(SwitchTarget);
		}
	}
	else
	{
		Targets=0;
	}

	NumLinks=*(int*)data_start;
	data_start+=4;
	if(NumLinks)
	{
		LinkedSwitches=new ObjectID[NumLinks];
		for(int i=0;i<NumLinks;i++)
		{
			LinkedSwitches[i]=*(ObjectID*)data_start;
			data_start+=8;
		}
	}
	else
	{
		LinkedSwitches=0;
	}

	pad1=*(int*)data_start;
	data_start+=4;
	flags=*(int*)data_start;
	
	if(!(flags & MultiSwitchFlag_SwitchUpdated))
	{
		//update multiswitch format
		//pad1 used to be UseRestStateAfter
		for(int i=0;i<NumTargets;i++)
		{
			if(i>=pad1)
				Targets[i].request=1;
			else	
				Targets[i].request=0;
		}
		pad1=0;
		flags|=MultiSwitchFlag_SwitchUpdated;
	}
	


}

MultiSwitchStrategy::~MultiSwitchStrategy()
{
	if(Targets)delete [] Targets;
	if(LinkedSwitches)delete [] LinkedSwitches;
}



size_t MultiSwitchStrategy::GetStrategySize()
{
	return AvpStrat::GetStrategySize()+32+NumTargets*12+NumLinks*8;
	
}

void MultiSwitchStrategy::fill_data_block(char* data)
{
	AvpStrat::fill_data_block(data);
	data+=AvpStrat::GetStrategySize();
	
	*(int*)data=RestState;
	data+=4;
	*(int*)data=Mode;
	data+=4;
	*(int*)data=Time;
	data+=4;
	*(int*)data=Security;
	data+=4;

	*(int*)data=NumTargets;
	data+=4;
	for(int i=0;i<NumTargets;i++)
	{
		*(SwitchTarget*)data=Targets[i];
		data+=sizeof(SwitchTarget);
	}

	*(int*)data=NumLinks;
	data+=4;
	for(int i=0;i<NumLinks;i++)
	{
		*(ObjectID*)data=LinkedSwitches[i];
		data+=8;
	}

	*(int*)data=pad1;
	data+=4;
	*(int*)data=flags;	
}
////////////////////////////////////////////////////////////////////////
AreaSwitchStrategy::AreaSwitchStrategy(const char* data_start,size_t size)
:MultiSwitchStrategy(data_start,size)
{
	data_start+=MultiSwitchStrategy::GetStrategySize();
	
	trigger_min=*(ChunkVectorInt*)data_start;
	data_start+=sizeof(ChunkVectorInt);	
	trigger_max=*(ChunkVectorInt*)data_start;
	data_start+=sizeof(ChunkVectorInt);	
	
	area_pad1=*(int*)data_start;
	data_start+=4;
	area_pad2=*(int*)data_start;
	data_start+=4;

}

size_t AreaSwitchStrategy::GetStrategySize()
{
	return MultiSwitchStrategy::GetStrategySize()+8+2*sizeof(ChunkVectorInt);
	
}

void AreaSwitchStrategy::fill_data_block(char* data)
{
	MultiSwitchStrategy::fill_data_block(data);
	data+=MultiSwitchStrategy::GetStrategySize();
	
	*(ChunkVectorInt*)data=trigger_min;
	data+=sizeof(ChunkVectorInt);
	*(ChunkVectorInt*)data=trigger_max;
	data+=sizeof(ChunkVectorInt);

	*(int*)data=area_pad1;
	data+=4;
	*(int*)data=area_pad2;
	data+=4;

}
/////////////////////////////////////////////////////////////

TeleportStrategy::TeleportStrategy(const char* data_start,size_t /*size*/ )
:AvpStrat(data_start)
{
	data_start+=AvpStrat::GetStrategySize();
	TeleportTo=*(ObjectID*)data_start;
	data_start+=sizeof(ObjectID);
	Type=*(int*)data_start;
	data_start+=4;
	spare1=*(int*)data_start;
	data_start+=4;
	spare2=*(int*)data_start;
}

size_t TeleportStrategy::GetStrategySize()
{
	return 20+AvpStrat::GetStrategySize();
}

void TeleportStrategy::fill_data_block(char* data)
{
	AvpStrat::fill_data_block(data);
	data+=AvpStrat::GetStrategySize();
	*(ObjectID*)data=TeleportTo;		
	data+=sizeof(ObjectID);
	*(int*)data=Type;
	data+=4;
	*(int*)data=spare1;
	data+=4;
	*(int*)data=spare2;
}





/////////////////////////////////////////////////////////////

EnemyStrategy::EnemyStrategy(const char* data_start,size_t /*size*/ )
:AvpStrat(data_start)
{
	data_start+=AvpStrat::GetStrategySize();

	MissionType=*(int*)data_start;
	data_start+=4;
	
	target_request=*(int*)data_start;
	data_start+=4;

	DeathTarget=*(ObjectID*)data_start;
	data_start+=sizeof(ObjectID);
	
	ExtraMissionData=*(int*)data_start;
	data_start+=4;
	
	for(int i=0;i<3;i++)
	{
		spares[i]=*(int*)data_start;
		data_start+=4;
	}
}

size_t EnemyStrategy::GetStrategySize()
{
	return 32+AvpStrat::GetStrategySize();
}

void EnemyStrategy::fill_data_block(char* data)
{
	AvpStrat::fill_data_block(data);
	data+=AvpStrat::GetStrategySize();

	*(int*)data=MissionType;
	data+=4;

	*(int*)data=target_request;
	data+=4;

	*(ObjectID*)data=DeathTarget;
	data+=sizeof(DeathTarget);

	*(int*)data=ExtraMissionData;
	data+=4;
	
	for(int i=0;i<3;i++)
	{
		*(int*)data=spares[i];
		data+=4;
	}
}


/////////////////////////////////////////////////////////////
GeneratorStrategy::GeneratorStrategy(const char* data_start,size_t /*size*/ )
:AvpStrat(data_start)
{
	data_start+=AvpStrat::GetStrategySize();

	MissionType=*(int*)data_start;
	data_start+=4;
		
	ExtraMissionData=*(int*)data_start;
	data_start+=4;
	
	for(int i=0;i<6;i++)
	{
		spares[i]=*(int*)data_start;
		data_start+=4;
	}
}

size_t GeneratorStrategy::GetStrategySize()
{
	return 32+AvpStrat::GetStrategySize();
}

void GeneratorStrategy::fill_data_block(char* data)
{
	AvpStrat::fill_data_block(data);
	data+=AvpStrat::GetStrategySize();

	*(int*)data=MissionType;
	data+=4;
	
	*(int*)data=ExtraMissionData;
	data+=4;
	
	for(int i=0;i<6;i++)
	{
		*(int*)data=spares[i];
		data+=4;
	}
}


/////////////////////////////////////////////////////////////

MissionObjectiveStrategy::MissionObjectiveStrategy(const char* data_start,size_t /*size*/ )
:AvpStrat(data_start)
{
	data_start+=AvpStrat::GetStrategySize();
	
	mission_description_string=*(int*)data_start;
	data_start+=4;
	
	mission_complete_string=*(int*)data_start;
	data_start+=4;
	
	mission_number=*(int*)data_start;
	data_start+=4;

	flags=*(int*)data_start;
	data_start+=4;

	mission_completion_effect=*(int*)data_start;
	data_start+=4;

	num_mission_targets=*(int*)data_start;
	data_start+=4;

	if(num_mission_targets)
	{
		mission_targets=new MissionMessage[num_mission_targets];
		for(int i=0;i<num_mission_targets;i++)
		{
			mission_targets[i]=*(MissionMessage*)data_start;
			data_start+=sizeof(MissionMessage);
		}

	}
	else
		mission_targets=0;

	num_request_targets=*(int*)data_start;
	data_start+=4;

	if(num_request_targets)
	{
		request_targets=new RequestTarget[num_request_targets];
		for(int i=0;i<num_request_targets;i++)
		{
			request_targets[i]=*(RequestTarget*)data_start;
			data_start+=sizeof(RequestTarget);
		}

	}
	else
		request_targets=0;

	pad1=*(int*)data_start;
	data_start+=4;
	pad2=*(int*)data_start;

	
			
}

size_t MissionObjectiveStrategy::GetStrategySize()
{
	return 36+num_mission_targets*sizeof(MissionMessage)+num_request_targets*sizeof(RequestTarget)+AvpStrat::GetStrategySize();
}

void MissionObjectiveStrategy::fill_data_block(char* data)
{
	AvpStrat::fill_data_block(data);
	data+=AvpStrat::GetStrategySize();

	*(int*)data=mission_description_string;
	data+=4;

	*(int*)data=mission_complete_string;
	data+=4;

	*(int*)data=mission_number;
	data+=4;

	*(int*)data=flags;
	data+=4;

	*(int*)data=mission_completion_effect;
	data+=4;

	*(int*)data=num_mission_targets;
	data+=4;
	for(int i=0;i<num_mission_targets;i++)
	{
		*(MissionMessage*)data=mission_targets[i];
		data+=sizeof(MissionMessage);
	}
	
	
	*(int*)data=num_request_targets;
	data+=4;
	for(int i=0;i<num_request_targets;i++)
	{
		*(RequestTarget*)data=request_targets[i];
		data+=sizeof(RequestTarget);
	}

	*(int*)data=pad1;
	data+=4;

	*(int*)data=pad2;
	data+=4;

}


/////////////////////////////////////////////////////////////

MissionHintStrategy::MissionHintStrategy(const char* data_start,size_t /*size*/ )
:AvpStrat(data_start)
{
	data_start+=AvpStrat::GetStrategySize();

	mission_hint_string=*(int*)data_start;
	data_start+=4;
	
	mission_number=*(int*)data_start;
	data_start+=4;
	
	flags=*(int*)data_start;
	data_start+=4;
	
	pad1=*(int*)data_start;
	data_start+=4;
	
	pad2=*(int*)data_start;


}

size_t MissionHintStrategy::GetStrategySize()
{
	return 20+AvpStrat::GetStrategySize();
}

void MissionHintStrategy::fill_data_block(char* data)
{
	AvpStrat::fill_data_block(data);
	data+=AvpStrat::GetStrategySize();

	*(int*)data=mission_hint_string;
	data+=4;
	
	*(int*)data=mission_number;
	data+=4;

	*(int*)data=flags;
	data+=4;

	*(int*)data=pad1;
	data+=4;

	*(int*)data=pad2;
	data+=4;
}
/////////////////////////////////////////////////////////////

TextMessageStrategy::TextMessageStrategy(const char* data_start,size_t /*size*/ )
:AvpStrat(data_start)
{
	data_start+=AvpStrat::GetStrategySize();

	message_string=*(int*)data_start;
	data_start+=4;
	
	flags=*(int*)data_start;
	data_start+=4;
	
	pad2=*(int*)data_start;


}

size_t TextMessageStrategy::GetStrategySize()
{
	return 12+AvpStrat::GetStrategySize();
}

void TextMessageStrategy::fill_data_block(char* data)
{
	AvpStrat::fill_data_block(data);
	data+=AvpStrat::GetStrategySize();

	*(int*)data=message_string;
	data+=4;

	*(int*)data=flags;
	data+=4;

	*(int*)data=pad2;
	data+=4;
}

/////////////////////////////////////////////////////////////

TrackStrategy::TrackStrategy(const char* data_start,size_t /*size*/ )
:AvpStrat(data_start)
{
	data_start+=AvpStrat::GetStrategySize();

	num_point_effects=*(int*)data_start;
	data_start+=4;

	if(num_point_effects)
	{
		point_effects=new TrackPointEffect*[num_point_effects];
		for(int i=0;i<num_point_effects;i++)
		{
			point_effects[i]=new TrackPointEffect;
			TrackPointEffect* tpe=point_effects[i];

			tpe->point_no=*(int*)data_start;
			data_start+=4;
			tpe->pad1=*(int*)data_start;
			data_start+=4;
			tpe->num_targets=*(int*)data_start;
			data_start+=4;

			if(tpe->num_targets)
			{
				tpe->targets=new TrackRequestTarget[tpe->num_targets];
				for(int j=0;j<tpe->num_targets;j++)
				{
					tpe->targets[j]=*(TrackRequestTarget*)data_start;
					data_start+=sizeof(TrackRequestTarget);
				}
			}
			else
				tpe->targets=0;

		}
	}
	else
		point_effects=0;

	pad1=*(int*)data_start;
	data_start+=4;
	pad2=*(int*)data_start;

}

TrackStrategy::~TrackStrategy()
{
	for(int i=0;i<num_point_effects;i++)
	{
		delete point_effects[i];
	}
	if(point_effects) 
		delete [] point_effects;
}

size_t TrackStrategy::GetStrategySize()
{
	size_t retval=AvpStrat::GetStrategySize()+12;
	retval+=num_point_effects*12;
	for(int i=0;i<num_point_effects;i++)
	{
		retval+=sizeof(TrackRequestTarget)*point_effects[i]->num_targets;
	}
	return retval;	
}

void TrackStrategy::fill_data_block(char* data)
{
	AvpStrat::fill_data_block(data);
	data+=AvpStrat::GetStrategySize();

	*(int*)data=num_point_effects;
	data+=4;

	for(int i=0;i<num_point_effects;i++)
	{
		TrackPointEffect* tpe=point_effects[i];

		*(int*)data=tpe->point_no;
		data+=4;
		*(int*)data=tpe->pad1;
		data+=4;
		*(int*)data=tpe->num_targets;
		data+=4;

		for(int j=0;j<tpe->num_targets;j++)
		{
			*(TrackRequestTarget*)data=tpe->targets[j];
			data+=sizeof(TrackRequestTarget);
		}

		
	}

	*(int*)data=pad1;
	data+=4;
	*(int*)data=pad2;
}

/////////////////////////////////////////////////////////////
TrackDestructStrategy::TrackDestructStrategy(const char* data_start,size_t size)
:TrackStrategy(data_start,size)
{
	data_start+=TrackStrategy::GetStrategySize();

	integrity=*(int*)data_start;
	data_start+=4;

	target_request=*(int*)data_start;
	data_start+=4;

	targetID=*(ObjectID*)data_start;
}

void TrackDestructStrategy::fill_data_block(char* data)
{
	TrackStrategy::fill_data_block(data);
	data+=TrackStrategy::GetStrategySize();

	*(int*) data=integrity;
	data+=4;

	*(int*) data = target_request;
	data+=4;

	*(ObjectID*) data= targetID;

}

size_t TrackDestructStrategy::GetStrategySize()
{
	size_t retval=TrackStrategy::GetStrategySize();
	retval+=8+sizeof(ObjectID);
	return retval;
}

////////////////////////////////////////////////////////////
HierarchyStrategy::HierarchyStrategy(const char* data_start,size_t /*size*/ )
:AvpStrat(data_start)
{
	data_start+=AvpStrat::GetStrategySize();

	num_point_effects=*(int*)data_start;
	data_start+=4;

	if(num_point_effects)
	{
		point_effects=new TrackPointEffect*[num_point_effects];
		for(int i=0;i<num_point_effects;i++)
		{
			point_effects[i]=new TrackPointEffect;
			TrackPointEffect* tpe=point_effects[i];

			tpe->point_no=*(int*)data_start;
			data_start+=4;
			tpe->pad1=*(int*)data_start;
			data_start+=4;
			tpe->num_targets=*(int*)data_start;
			data_start+=4;

			if(tpe->num_targets)
			{
				tpe->targets=new TrackRequestTarget[tpe->num_targets];
				for(int j=0;j<tpe->num_targets;j++)
				{
					tpe->targets[j]=*(TrackRequestTarget*)data_start;
					data_start+=sizeof(TrackRequestTarget);
				}
			}
			else
				tpe->targets=0;

		}
	}
	else
		point_effects=0;

	pad1=*(int*)data_start;
	data_start+=4;
	pad2=*(int*)data_start;

}

HierarchyStrategy::~HierarchyStrategy()
{
	for(int i=0;i<num_point_effects;i++)
	{
		delete point_effects[i];
	}
	if(point_effects) 
		delete point_effects;
}

size_t HierarchyStrategy::GetStrategySize()
{
	size_t retval=AvpStrat::GetStrategySize()+12;
	retval+=num_point_effects*12;
	for(int i=0;i<num_point_effects;i++)
	{
		retval+=sizeof(TrackRequestTarget)*point_effects[i]->num_targets;
	}
	return retval;	
}

void HierarchyStrategy::fill_data_block(char* data)
{
	AvpStrat::fill_data_block(data);
	data+=AvpStrat::GetStrategySize();

	*(int*)data=num_point_effects;
	data+=4;

	for(int i=0;i<num_point_effects;i++)
	{
		TrackPointEffect* tpe=point_effects[i];

		*(int*)data=tpe->point_no;
		data+=4;
		*(int*)data=tpe->pad1;
		data+=4;
		*(int*)data=tpe->num_targets;
		data+=4;

		for(int j=0;j<tpe->num_targets;j++)
		{
			*(TrackRequestTarget*)data=tpe->targets[j];
			data+=sizeof(TrackRequestTarget);
		}

		
	}

	*(int*)data=pad1;
	data+=4;
	*(int*)data=pad2;
}

/////////////////////////////////////////////////////////////

FanStrategy::FanStrategy(const char* data_start,size_t /*size*/ )
:AvpStrat(data_start)
{
	data_start+=AvpStrat::GetStrategySize();

	speed_up_time=*(int*)data_start;
	data_start+=4;
	slow_down_time=*(int*)data_start;
	data_start+=4;

	fan_wind_strength=*(int*)data_start;
	data_start+=4;
	pad2=*(int*)data_start;

}


size_t FanStrategy::GetStrategySize()
{
	return AvpStrat::GetStrategySize()+16;
}

void FanStrategy::fill_data_block(char* data)
{
	AvpStrat::fill_data_block(data);
	data+=AvpStrat::GetStrategySize();

	*(int*)data=speed_up_time;
	data+=4;
	*(int*)data=slow_down_time;
	data+=4;

	*(int*)data=fan_wind_strength;
	data+=4;
	*(int*)data=pad2;
}

/////////////////////////////////////////////////////////////

DeathVolumeStrategy::DeathVolumeStrategy(const char* data_start,size_t /*size*/ )
:AvpStrat(data_start)
{
	data_start+=AvpStrat::GetStrategySize();

	volume_min=*(ChunkVectorInt*)data_start;
	data_start+=sizeof(ChunkVectorInt);
	volume_max=*(ChunkVectorInt*)data_start;
	data_start+=sizeof(ChunkVectorInt);

	flags=*(int*)data_start;
	data_start+=4;
	damage=*(int*)data_start;
	data_start+=4;
	pad2=*(int*)data_start;

}


size_t DeathVolumeStrategy::GetStrategySize()
{
	return AvpStrat::GetStrategySize()+12+2*sizeof(ChunkVectorInt);
}

void DeathVolumeStrategy::fill_data_block(char* data)
{
	AvpStrat::fill_data_block(data);
	data+=AvpStrat::GetStrategySize();

	*(ChunkVectorInt*)data=volume_min;
	data+=sizeof(ChunkVectorInt);
	*(ChunkVectorInt*)data=volume_max;
	data+=sizeof(ChunkVectorInt);

	*(int*)data=flags;
	data+=4;
	*(int*)data=damage;
	data+=4;
	*(int*)data=pad2;
}

/////////////////////////////////////////////////////////////

SelfDestructStrategy::SelfDestructStrategy(const char* data_start,size_t /*size*/ )
:AvpStrat(data_start)
{
	data_start+=AvpStrat::GetStrategySize();

	timer=*(int*)data_start;
	data_start+=4;
	for(int i=0;i<4;i++)
	{
		pad[i]=*(int*)data_start;
		data_start+=4;
	}
}


size_t SelfDestructStrategy::GetStrategySize()
{
	return AvpStrat::GetStrategySize()+20;
}

void SelfDestructStrategy::fill_data_block(char* data)
{
	AvpStrat::fill_data_block(data);
	data+=AvpStrat::GetStrategySize();

	*(int*)data=timer;
	data+=4;

	for(int i=0;i<4;i++)
	{
		*(int*)data=pad[i];
		data+=4;
	}
}



/////////////////////rainbow 6 strategy alert/////////////////////////

SwingDoorStrategy::SwingDoorStrategy(const char* data,size_t)
:AvpStrat(data)
{
	data+=AvpStrat::GetStrategySize();
	
	CHUNK_EXTRACT(time_open,int);
	CHUNK_EXTRACT(paired_door,ObjectID);
	CHUNK_EXTRACT(doorway_module,ObjectID);
	CHUNK_EXTRACT(flags,int);
	CHUNK_EXTRACT(time_to_pick,int);
	CHUNK_EXTRACT(spare3,int);
	CHUNK_EXTRACT(spare4,int);

	//need to see if this door has been updated to give it the 
	//new default time_open value of 0;
	if(!(flags & SwingDoorFlag_UpdatedTime))
	{
		flags|=SwingDoorFlag_UpdatedTime;
		time_open=0;
	}
}

size_t SwingDoorStrategy::GetStrategySize()
{
	return AvpStrat::GetStrategySize()+20+2*sizeof(ObjectID);
}

void SwingDoorStrategy::fill_data_block(char* data)
{
	AvpStrat::fill_data_block(data);
	data+=AvpStrat::GetStrategySize();

	CHUNK_FILL(time_open,int);
	CHUNK_FILL(paired_door,ObjectID);
	CHUNK_FILL(doorway_module,ObjectID);
	CHUNK_FILL(flags,int);
	CHUNK_FILL(time_to_pick,int);
	CHUNK_FILL(spare3,int);
	CHUNK_FILL(spare4,int);
}


//////////////////////////////////////////////


PlacedBombStrategy::PlacedBombStrategy(const char* data,size_t)
:AvpStrat(data)
{
	data+=AvpStrat::GetStrategySize();
	
	CHUNK_EXTRACT(type,int);
	CHUNK_EXTRACT(time,int);
	CHUNK_EXTRACT(flags,int);
	CHUNK_EXTRACT(integrity,int);

	CHUNK_EXTRACT(objective_number,unsigned char);
	CHUNK_EXTRACT(time_to_pick,unsigned short);

	CHUNK_EXTRACT(pad,unsigned char);
	CHUNK_EXTRACT(spare2,int);
	CHUNK_EXTRACT(spare3,int);
	CHUNK_EXTRACT(spare4,int);
}

size_t PlacedBombStrategy::GetStrategySize()
{
	return AvpStrat::GetStrategySize()+8*4;
}

void PlacedBombStrategy::fill_data_block(char* data)
{
	AvpStrat::fill_data_block(data);
	data+=AvpStrat::GetStrategySize();

	CHUNK_FILL(type,int);
	CHUNK_FILL(time,int);
	CHUNK_FILL(flags,int);
	CHUNK_FILL(integrity,int);
	CHUNK_FILL(objective_number,unsigned char);
	CHUNK_FILL(time_to_pick,unsigned short);

	CHUNK_FILL(pad,unsigned char);
	CHUNK_FILL(spare2,int);
	CHUNK_FILL(spare3,int);
	CHUNK_FILL(spare4,int);
}


//////////////////////////////////////////////


R6SwitchStrategy::R6SwitchStrategy(const char* data,size_t)
:AvpStrat(data)
{
	data+=AvpStrat::GetStrategySize();
	
	CHUNK_EXTRACT(TargetID,ObjectID);

	CHUNK_EXTRACT(spare1,int);
	CHUNK_EXTRACT(spare2,int);
	CHUNK_EXTRACT(spare3,int);
	CHUNK_EXTRACT(spare4,int);
}

size_t R6SwitchStrategy::GetStrategySize()
{
	return AvpStrat::GetStrategySize()+8*4;
}

void R6SwitchStrategy::fill_data_block(char* data)
{
	AvpStrat::fill_data_block(data);
	data+=AvpStrat::GetStrategySize();

	CHUNK_FILL(TargetID,ObjectID);

	CHUNK_FILL(spare1,int);
	CHUNK_FILL(spare2,int);
	CHUNK_FILL(spare3,int);
	CHUNK_FILL(spare4,int);
}



//////////////////////////////////////////////

MummyInanimateStrategy::MummyInanimateStrategy()
:AvpStrat(StratMummyInanimate)
{
	destructionType = 0;
	health = 100;
	generatedPickups[0] = 0;
	generatedPickups[1] = 0;
	generatedPickups[2] = 0;
	generatedPickups[3] = 0;
	linkedSoundID.id1 = 0;
	linkedSoundID.id2 = 0;
	activateID = linkedSoundID;
	spare1 = 0;
	spare2 = 0;
	spare3 = 0;
	spare4 = 0;
}


MummyInanimateStrategy::MummyInanimateStrategy(const char* data,size_t)
:AvpStrat(data)
{
	data+=AvpStrat::GetStrategySize();
	
	CHUNK_EXTRACT(destructionType,int);
	CHUNK_EXTRACT(health,int);
	CHUNK_EXTRACT(generatedPickups[0],char);
	CHUNK_EXTRACT(generatedPickups[1],char);
	CHUNK_EXTRACT(generatedPickups[2],char);
	CHUNK_EXTRACT(generatedPickups[3],char);
	CHUNK_EXTRACT(linkedSoundID,ObjectID);
	CHUNK_EXTRACT(activateID,ObjectID);

	CHUNK_EXTRACT(spare1,int);
	CHUNK_EXTRACT(spare2,int);
	CHUNK_EXTRACT(spare3,int);
	CHUNK_EXTRACT(spare4,int);
}

size_t MummyInanimateStrategy::GetStrategySize()
{
	size_t retval = AvpStrat::GetStrategySize();
	retval += 2*sizeof(int) + 4*sizeof(char) + 2*sizeof(ObjectID);
	retval += 4*sizeof(int);
	return retval;
}

void MummyInanimateStrategy::fill_data_block(char* data)
{
	AvpStrat::fill_data_block(data);
	data+=AvpStrat::GetStrategySize();

	CHUNK_FILL(destructionType,int);
	CHUNK_FILL(health,int);
	CHUNK_FILL(generatedPickups[0],char);
	CHUNK_FILL(generatedPickups[1],char);
	CHUNK_FILL(generatedPickups[2],char);
	CHUNK_FILL(generatedPickups[3],char);
	CHUNK_FILL(linkedSoundID,ObjectID);
	CHUNK_FILL(activateID,ObjectID);

	CHUNK_FILL(spare1,int);
	CHUNK_FILL(spare2,int);
	CHUNK_FILL(spare3,int);
	CHUNK_FILL(spare4,int);
}


//////////////////////////////////////////////
MummyPickupStrategy::MummyPickupStrategy()
:AvpStrat(StratMummyPickup)
{
	pickupType = 0;
	inactiveAtStart = 0;
	spare1 = 0;
	spare2 = 0;
	spare3 = 0;
	spare4 = 0;
}

MummyPickupStrategy::MummyPickupStrategy(const char* data,size_t)
:AvpStrat(data)
{
	data+=AvpStrat::GetStrategySize();
	
	CHUNK_EXTRACT(pickupType,int);
	CHUNK_EXTRACT(inactiveAtStart,BOOL);

	CHUNK_EXTRACT(spare1,int);
	CHUNK_EXTRACT(spare2,int);
	CHUNK_EXTRACT(spare3,int);
	CHUNK_EXTRACT(spare4,int);
}

size_t MummyPickupStrategy::GetStrategySize()
{
	return AvpStrat::GetStrategySize()+6*4;
}

void MummyPickupStrategy::fill_data_block(char* data)
{
	AvpStrat::fill_data_block(data);
	data+=AvpStrat::GetStrategySize();

	CHUNK_FILL(pickupType,int);
	CHUNK_FILL(inactiveAtStart,BOOL);

	CHUNK_FILL(spare1,int);
	CHUNK_FILL(spare2,int);
	CHUNK_FILL(spare3,int);
	CHUNK_FILL(spare4,int);
}

//////////////////////////////////////////////
MummyTriggerVolumeStrategy::MummyTriggerVolumeStrategy(const char* data,size_t)
:AvpStrat(data)
{
	data+=AvpStrat::GetStrategySize();
	
	CHUNK_EXTRACT(trigger_min,ChunkVectorInt);
	CHUNK_EXTRACT(trigger_max,ChunkVectorInt);
	CHUNK_EXTRACT(targetID,ObjectID);

	CHUNK_EXTRACT(spare1,int);
	CHUNK_EXTRACT(spare2,int);
	CHUNK_EXTRACT(spare3,int);
	CHUNK_EXTRACT(spare4,int);
}

size_t MummyTriggerVolumeStrategy::GetStrategySize()
{
	return AvpStrat::GetStrategySize()+4*4 + sizeof(ChunkVectorInt)*2 + sizeof(ObjectID);
}

void MummyTriggerVolumeStrategy::fill_data_block(char* data)
{
	AvpStrat::fill_data_block(data);
	data+=AvpStrat::GetStrategySize();

	CHUNK_FILL(trigger_min,ChunkVectorInt);
	CHUNK_FILL(trigger_max,ChunkVectorInt);
	CHUNK_FILL(targetID,ObjectID);

	CHUNK_FILL(spare1,int);
	CHUNK_FILL(spare2,int);
	CHUNK_FILL(spare3,int);
	CHUNK_FILL(spare4,int);
}


//////////////////////////////////////////////
MummyPivotObjectStrategy::MummyPivotObjectStrategy(const char* data,size_t)
:AvpStrat(data)
{
	data+=AvpStrat::GetStrategySize();
	
	CHUNK_EXTRACT(typeID,int);
	CHUNK_EXTRACT(triggerDelay,int);
	CHUNK_EXTRACT(targetID,ObjectID);

	CHUNK_EXTRACT(spare1,int);
	CHUNK_EXTRACT(spare2,int);

	//now need to see whether this is the upgraded version of the strategy
	if(StrategyType == StratMummyPivotObject_Old)
	{
		//need to add the trigger volume stuff then
		trigger_min.x=trigger_min.y=trigger_min.z=0;
		trigger_max = trigger_min;

		//change the type to the new version
		StrategyType = StratMummyPivotObject;

	}
	else
	{
		CHUNK_EXTRACT(trigger_min,ChunkVectorInt);
		CHUNK_EXTRACT(trigger_max,ChunkVectorInt);
	}
}

size_t MummyPivotObjectStrategy::GetStrategySize()
{
	return AvpStrat::GetStrategySize()+4*sizeof(int) + sizeof(ObjectID) + 2*sizeof(ChunkVectorInt);
}

void MummyPivotObjectStrategy::fill_data_block(char* data)
{
	AvpStrat::fill_data_block(data);
	data+=AvpStrat::GetStrategySize();

	CHUNK_FILL(typeID,int);
	CHUNK_FILL(triggerDelay,int);
	CHUNK_FILL(targetID,ObjectID);

	CHUNK_FILL(spare1,int);
	CHUNK_FILL(spare2,int);

	CHUNK_FILL(trigger_min,ChunkVectorInt);
	CHUNK_FILL(trigger_max,ChunkVectorInt);
}

//////////////////////////////////////////////
MummyChestStrategy::MummyChestStrategy(const char* data,size_t)
:AvpStrat(data)
{
	data+=AvpStrat::GetStrategySize();
	
	CHUNK_EXTRACT(objectives[0],unsigned char);
	CHUNK_EXTRACT(objectives[1],unsigned char);
	CHUNK_EXTRACT(objectives[2],unsigned char);
	CHUNK_EXTRACT(objectives[3],unsigned char);

	CHUNK_EXTRACT(camera_location,ChunkVectorInt);
	CHUNK_EXTRACT(spare,int);
}

size_t MummyChestStrategy::GetStrategySize()
{
	return AvpStrat::GetStrategySize()+4*sizeof(int) + 4*sizeof(unsigned char);
}

void MummyChestStrategy::fill_data_block(char* data)
{
	AvpStrat::fill_data_block(data);
	data+=AvpStrat::GetStrategySize();

	CHUNK_FILL(objectives[0],unsigned char);
	CHUNK_FILL(objectives[1],unsigned char);
	CHUNK_FILL(objectives[2],unsigned char);
	CHUNK_FILL(objectives[3],unsigned char);

	CHUNK_FILL(camera_location,ChunkVectorInt);
	CHUNK_FILL(spare,int);
}

//////////////////////////////////////////////
MummyAlterCameraRangeStrategy::MummyAlterCameraRangeStrategy(const char* data,size_t)
:AvpStrat(data)
{
	data+=AvpStrat::GetStrategySize();

	CHUNK_EXTRACT(zone_min,ChunkVectorInt)	
	CHUNK_EXTRACT(zone_max,ChunkVectorInt)	
	
	CHUNK_EXTRACT(enter_range,int)	
	CHUNK_EXTRACT(exit_range,int)	
	CHUNK_EXTRACT(axis,int)	

	CHUNK_EXTRACT(spare1,int)	
	CHUNK_EXTRACT(spare2,int)	
	CHUNK_EXTRACT(spare3,int)	
	CHUNK_EXTRACT(spare4,int)	

}

size_t MummyAlterCameraRangeStrategy::GetStrategySize()
{
	return AvpStrat::GetStrategySize()+7*sizeof(int) + 2*sizeof(ChunkVectorInt);
}

void MummyAlterCameraRangeStrategy::fill_data_block(char* data)
{
	AvpStrat::fill_data_block(data);
	data+=AvpStrat::GetStrategySize();

	CHUNK_FILL(zone_min,ChunkVectorInt)	
	CHUNK_FILL(zone_max,ChunkVectorInt)	
	
	CHUNK_FILL(enter_range,int)	
	CHUNK_FILL(exit_range,int)	
	CHUNK_FILL(axis,int)	

	CHUNK_FILL(spare1,int)	
	CHUNK_FILL(spare2,int)	
	CHUNK_FILL(spare3,int)	
	CHUNK_FILL(spare4,int)	
}
