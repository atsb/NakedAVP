#include "avpchunk.hpp"
#include "md5.h"
//#include "strachnk.hpp"
//#include "obchunk.hpp"

//macro for helping to force inclusion of chunks when using libraries
FORCE_CHUNK_INCLUDE_IMPLEMENT(avpchunk)

RIF_IMPLEMENT_DYNCREATE("AVPGENER",AVP_Generator_Chunk)

void AVP_Generator_Chunk::fill_data_block (char * data_start)
{
	strncpy (data_start, identifier, 8);

	data_start += 8;

	*((int *) data_start) = chunk_size;

	data_start += 4;
	
	*((ChunkVectorInt *) data_start) = location;
	data_start += sizeof(ChunkVectorInt);
	
	
	*((int *) data_start) = orientation;
	data_start += 4;
	
	*((int *) data_start) = type;
	data_start += 4;

	*((int *) data_start) = flags;
	data_start += 4;
	
	*(data_start) = textureID;
	data_start ++;

	*(data_start) = sub_type;
	data_start ++;
	
	*(data_start) = extra1;
	data_start ++;
	
	*(data_start) = extra2;
	data_start ++;
	
	strcpy (data_start, name);
	
}

size_t AVP_Generator_Chunk::size_chunk ()
{
	chunk_size = 12 + 3 * 4 + 4 * 4 + strlen(name) + 4 - strlen(name)%4;

	return(chunk_size);
	
}

AVP_Generator_Chunk::AVP_Generator_Chunk (Chunk_With_Children * parent, const char * data, size_t /*size*/)
: Chunk (parent, "AVPGENER")
{
	location = *((ChunkVectorInt*) data);
	data += sizeof(ChunkVectorInt);
		
	orientation = *((int *) data);
	data += 4;
	
	type = *((int *) data);
	data += 4;

	flags = *((int *) data);
	data += 4;
	
	textureID = *(data);
	data ++;
	
	sub_type = *(data);
	data ++;
	
	extra1 = *(unsigned char*)(data);
	data ++;
	
	extra2 = *(unsigned char*)(data);
	data ++;
	
	name = new char [strlen(data) + 1];
	strcpy (name, data);
}


AVP_Generator_Chunk::~AVP_Generator_Chunk ()
{
	if (name)
		delete [] name;
}


ObjectID AVP_Generator_Chunk::CalculateID()
{
	ObjectID retval={0,0};
	
	//get the environment_data_chunk
	Chunk_With_Children* cwc=((Special_Objects_Chunk *)parent)->parent;
	
	List<Chunk*> chlist;
	cwc->lookup_child("RIFFNAME",chlist);
	if(!chlist.size()) return retval;
	char Name[100];

	strcpy(Name,((RIF_Name_Chunk*)chlist.first_entry())->rif_name);

	strcat(Name,name);
	char buffer[16];
	md5_buffer(Name,strlen(Name),&buffer[0]);
	buffer[7]=0;
	retval = *(ObjectID*)&buffer[0];
	return retval;

}

AVP_Generator_Extra_Data_Chunk* AVP_Generator_Chunk::get_extra_data_chunk()
{
	List<Chunk*> chlist;
	parent->lookup_child("AVPGENEX",chlist);
	if(!chlist.size())return 0;

	for(LIF<Chunk*> chlif(&chlist);!chlif.done();chlif.next())
	{
		AVP_Generator_Extra_Data_Chunk* agedc=(AVP_Generator_Extra_Data_Chunk*)chlif();
		List<Chunk*> chlist2;
		agedc->lookup_child("AVPGENNM",chlist2);
		if(chlist2.size())
		{
			const char* ename=((AVP_Generator_Extra_Name_Chunk*)chlist2.first_entry())->name;
			if(!strcmp(name,ename))
			{
				return agedc;
			}
		}
	}
	//no extra data chunk found
	return 0;

}
AVP_Generator_Extra_Data_Chunk* AVP_Generator_Chunk::create_extra_data_chunk()
{
	AVP_Generator_Extra_Data_Chunk* agedc=get_extra_data_chunk();
	if(agedc)return agedc;
	
	return new AVP_Generator_Extra_Data_Chunk(parent,this);
}

AVP_Generator_Extended_Settings_Chunk* AVP_Generator_Chunk::get_extended_settings()
{
	if(flags & AVPGENFLAG_ADVANCEDGENERATOR)
	{
		AVP_Generator_Extra_Data_Chunk* agedc=get_extra_data_chunk();
		if(!agedc) return 0;
		return (AVP_Generator_Extended_Settings_Chunk*)agedc->lookup_single_child("GENEXSET");
	}
	return 0;
}

AVP_Generator_Extended_Settings_Chunk* AVP_Generator_Chunk::create_extended_settings()
{
	flags|=AVPGENFLAG_ADVANCEDGENERATOR;
	AVP_Generator_Extra_Data_Chunk* agedc=create_extra_data_chunk();

	AVP_Generator_Extended_Settings_Chunk* setting=(AVP_Generator_Extended_Settings_Chunk*)agedc->lookup_single_child("GENEXSET");
	if(setting) return setting;

	return new AVP_Generator_Extended_Settings_Chunk(agedc);
}

Object_Alternate_Locations_Chunk* AVP_Generator_Chunk::get_alternate_locations_chunk()
{
	AVP_Generator_Extra_Data_Chunk* agedc=get_extra_data_chunk();
	if(!agedc) return 0;
	return (Object_Alternate_Locations_Chunk*)agedc->lookup_single_child("ALTLOCAT");
}


//////////////////////////////////////////////////////////////////////////////
RIF_IMPLEMENT_DYNCREATE("AVPGENEX",AVP_Generator_Extra_Data_Chunk)

//loader for AVP_Generator_Extra_Data_Chunk
CHUNK_WITH_CHILDREN_LOADER("AVPGENEX",AVP_Generator_Extra_Data_Chunk)

/*
children for AVP_Generator_Extra_Data_Chunk :

"AVPSTRAT"		AVP_Strategy_Chunk)
"AVPGENNM"		AVP_Generator_Extra_Name_Chunk)
"R6GENDAT"		Rainbow6_Generator_Extra_Data_Chunk)
"GENEXSET"		AVP_Generator_Extended_Settings_Chunk)
"ALTLOCAT"		Object_Alternate_Locations_Chunk)
*/


AVP_Generator_Extra_Data_Chunk::AVP_Generator_Extra_Data_Chunk(Chunk_With_Children* parent,AVP_Generator_Chunk* agc)
:Chunk_With_Children(parent,"AVPGENEX")
{
	new AVP_Generator_Extra_Name_Chunk(this,agc->name);	
}

AVP_Generator_Chunk* AVP_Generator_Extra_Data_Chunk::get_generator_chunk()
{
	List<Chunk*> chlist;
	lookup_child("AVPGENNM",chlist);
	if(!chlist.size()) return 0;
	const char* name=((AVP_Generator_Extra_Name_Chunk*)chlist.first_entry())->name;

	parent->lookup_child("AVPGENER",chlist);
	for(LIF<Chunk*> chlif(&chlist);!chlif.done();chlif.next())
	{
		AVP_Generator_Chunk* agc=(AVP_Generator_Chunk*)chlif();
		if(!strcmp(agc->name,name))
		{
			return agc;
		}
	}
	//no generator chunk found
	return 0;
}
//////////////////////////////////////////////////////////////////////////////

RIF_IMPLEMENT_DYNCREATE("AVPGENNM",AVP_Generator_Extra_Name_Chunk)

AVP_Generator_Extra_Name_Chunk::AVP_Generator_Extra_Name_Chunk(Chunk_With_Children * parent, const char * data, size_t /*size*/)
:Chunk(parent,"AVPGENNM")
{
	int length=strlen(data);
	name=new char[length+1];
	strcpy(name,data);
}

AVP_Generator_Extra_Name_Chunk::AVP_Generator_Extra_Name_Chunk(Chunk_With_Children * parent, const char * _name)
:Chunk(parent,"AVPGENNM")
{
	name=new char[strlen(_name)+1];
	strcpy(name,_name);
}

AVP_Generator_Extra_Name_Chunk::~AVP_Generator_Extra_Name_Chunk()
{
	delete [] name;
}

void AVP_Generator_Extra_Name_Chunk::fill_data_block(char* data_start)
{
	strncpy (data_start, identifier, 8);
	data_start += 8;
	*((int *) data_start) = chunk_size;
	data_start += 4;

	strcpy(data_start,name);
}


size_t AVP_Generator_Extra_Name_Chunk::size_chunk()
{
	chunk_size=12;
	chunk_size+=(strlen(name)+4)&~3;
	return chunk_size;
}

//////////////////////////////////////////////////////////////////////////////

RIF_IMPLEMENT_DYNCREATE("GENEXSET",AVP_Generator_Extended_Settings_Chunk)

AVP_Generator_Extended_Settings_Chunk::AVP_Generator_Extended_Settings_Chunk(Chunk_With_Children* parent,const char* data,size_t)
:Chunk(parent,"GENEXSET")
{
	CHUNK_EXTRACT(GenerationRate,int)
	CHUNK_EXTRACT(GenRateIncrease,int)
	CHUNK_EXTRACT(GenLimit,unsigned char)
	CHUNK_EXTRACT(pad1,unsigned char)
	CHUNK_EXTRACT(pad2,unsigned char)
	CHUNK_EXTRACT(pad3,unsigned char)
	CHUNK_EXTRACT(spare1,int)
	CHUNK_EXTRACT(spare2,int)
    	
	
	size_t size=max(*(int*) data,(int)sizeof(AVP_Generator_Weighting));

	weights=(AVP_Generator_Weighting*)new unsigned char[size];
	memset(weights,0,sizeof(AVP_Generator_Weighting));
	memcpy(weights,data,*(int*) data);

	weights->data_size=size;
}

AVP_Generator_Extended_Settings_Chunk::AVP_Generator_Extended_Settings_Chunk(Chunk_With_Children* parent)
:Chunk(parent,"GENEXSET")
{
	weights=(AVP_Generator_Weighting *)new unsigned char[sizeof(AVP_Generator_Weighting)];
	memset(weights,0,sizeof(AVP_Generator_Weighting));
	weights->data_size=sizeof(AVP_Generator_Weighting);
	GenLimit=pad1=pad2=pad3=0;
	spare1=spare2=0;
	
}


AVP_Generator_Extended_Settings_Chunk::~AVP_Generator_Extended_Settings_Chunk()
{
	delete [] weights;
}

void AVP_Generator_Extended_Settings_Chunk::fill_data_block (char * data)
{
	CHUNK_FILL_START
	
	CHUNK_FILL(GenerationRate,int)
	CHUNK_FILL(GenRateIncrease,int)
	CHUNK_FILL(GenLimit,unsigned char)
	CHUNK_FILL(pad1,unsigned char)
	CHUNK_FILL(pad2,unsigned char)
	CHUNK_FILL(pad3,unsigned char)
	CHUNK_FILL(spare1,int)
	CHUNK_FILL(spare2,int)
	
	
	memcpy(data,weights,weights->data_size);
}

size_t AVP_Generator_Extended_Settings_Chunk::size_chunk ()
{
	chunk_size = 12+20+weights->data_size;
	return(chunk_size);
	
}


//////////////////////////////////////////////////////////////////////////////
RIF_IMPLEMENT_DYNCREATE("GLOGENDC",Global_Generator_Data_Chunk)

Global_Generator_Data_Chunk::Global_Generator_Data_Chunk(Chunk_With_Children* parent)
:Chunk(parent,"GLOGENDC")
{
	EnemyGenerated=Generate_Aliens;
	MaxNPCSOnLevel=25;
	NPCSPerMinute=4;
	NPCAcceleration=2;
	HiveStateChangeTime=60;
	spare1=spare2=0;

}
Global_Generator_Data_Chunk::Global_Generator_Data_Chunk(Chunk_With_Children* parent,const char* data,size_t)
:Chunk(parent,"GLOGENDC")
{
	EnemyGenerated=*(int*)data;
	data+=sizeof(int);
	MaxNPCSOnLevel=*(int*)data;
	data+=sizeof(int);
	NPCSPerMinute=*(int*)data;
	data+=sizeof(int);
	NPCAcceleration=*(int*)data;
	data+=sizeof(int);
	HiveStateChangeTime=*(int*)data;
	data+=sizeof(int);

	spare1=*(int*)data;
	data+=sizeof(int);
	spare2=*(int*)data;
	data+=sizeof(int);

}

void Global_Generator_Data_Chunk::fill_data_block (char * data_start)
{
	strncpy (data_start, identifier, 8);
	data_start += 8;
	*((int *) data_start) = chunk_size;
	data_start += 4;
	

	*(int*)data_start=EnemyGenerated;
	data_start+=4;
	*(int*)data_start=MaxNPCSOnLevel;
	data_start+=4;
	*(int*)data_start=NPCSPerMinute;
	data_start+=4;
	*(int*)data_start=NPCAcceleration;
	data_start+=4;
	*(int*)data_start=HiveStateChangeTime;
	data_start+=4;

	*(int*)data_start=spare1;
	data_start+=4;
	*(int*)data_start=spare2;
	data_start+=4;
}

size_t Global_Generator_Data_Chunk::size_chunk ()
{
	chunk_size = 12+28;

	return(chunk_size);
	
}
//////////////////////////////////////////////////////////////////////////////
RIF_IMPLEMENT_DYNCREATE("AVPSTART",AVP_Player_Start_Chunk)

AVP_Player_Start_Chunk::AVP_Player_Start_Chunk(Chunk_With_Children* parent)
:Chunk(parent,"AVPSTART")
{
	location.x=location.y=location.z=0;
	orientation.mat11=orientation.mat22=orientation.mat33=65536;
	orientation.mat12=orientation.mat23=orientation.mat31=0;
	orientation.mat21=orientation.mat32=orientation.mat13=0;
	moduleID.id1=0;
	moduleID.id2=0;

}

AVP_Player_Start_Chunk::AVP_Player_Start_Chunk(Chunk_With_Children* parent,const char* data,size_t)
:Chunk(parent,"AVPSTART")
{
	CHUNK_EXTRACT(location,ChunkVectorInt)
	CHUNK_EXTRACT(orientation,ChunkMatrix)
	CHUNK_EXTRACT(moduleID,ObjectID)
}

void AVP_Player_Start_Chunk::fill_data_block (char * data)
{
	CHUNK_FILL_START

	CHUNK_FILL(location,ChunkVectorInt)
	CHUNK_FILL(orientation,ChunkMatrix)
	CHUNK_FILL(moduleID,ObjectID)
	
}

size_t AVP_Player_Start_Chunk::size_chunk ()
{
	chunk_size = 12+sizeof(ChunkVectorInt)+sizeof(ChunkMatrix)+sizeof(ObjectID);

	return(chunk_size);
	
}
//////////////////////////////////////////////////////////////////////////////
RIF_IMPLEMENT_DYNCREATE("AVPCABLE",AVP_Power_Cable_Chunk)

AVP_Power_Cable_Chunk::AVP_Power_Cable_Chunk(Chunk_With_Children* parent,const char* data,size_t)
:Chunk (parent,"AVPCABLE")
{
	location=*(ChunkVectorInt*)data;
	data+=sizeof(ChunkVectorInt);

	id=*(ObjectID*)data;
	data+=sizeof(ObjectID);

	max_charge=*(int*)data;
	data+=4;

	initial_charge=*(int*)data;
	data+=4;

	recharge_rate=*(int*)data;
	data+=4;

	flags=*(int*)data;
	data+=4;
		
	for(int i=0;i<3;i++)
	{
		spare[i]=*(int*)data;
		data+=4;
	}

	int length=strlen(data);
	if(length)
	{
		name=new char[length+1];
		strcpy(name,data);	
	}
	else
	{
		name=0;
	}
}

AVP_Power_Cable_Chunk::AVP_Power_Cable_Chunk(Chunk_With_Children* parent,const char* _name,ObjectID _id)
:Chunk (parent,"AVPCABLE")
{
	location.x=location.y=location.z=0;
	name=new char[strlen(_name)+1];
	strcpy(name,_name);
	id=_id;
	for(int i=0;i<3;i++) spare[i]=0;
}

AVP_Power_Cable_Chunk::~AVP_Power_Cable_Chunk()
{
	if(name) delete name;
}

void AVP_Power_Cable_Chunk::fill_data_block(char* data)
{
	strncpy (data, identifier, 8);
	data += 8;
	*((int *) data) = chunk_size;
	data += 4;
	
	*(ChunkVectorInt*)data=location;
	data+=sizeof(ChunkVectorInt);

	*(ObjectID*)data=id;
	data+=sizeof(ObjectID);

	*(int*)data=max_charge;
	data+=4;

	*(int*)data=initial_charge;
	data+=4;

	*(int*)data=recharge_rate;
	data+=4;

	*(int*)data=flags;
	data+=4;

	
	for(int i=0;i<3;i++)
	{
		*(int*)data=spare[i];
		data+=4;
	}

	if(name)
	{
		strcpy(data,name);
	}
	else
	{
		*data=0;
	}

}
size_t AVP_Power_Cable_Chunk::size_chunk()
{
	chunk_size=(strlen(name)+4)&~3;
	chunk_size+=12+sizeof(ChunkVectorInt)+sizeof(ObjectID)+28;
	return chunk_size;
}

//////////////////////////////////////////////////////////////////////////////
RIF_IMPLEMENT_DYNCREATE("AVPENVIR",AVP_Environment_Settings_Chunk)


AVP_Environment_Settings_Chunk::AVP_Environment_Settings_Chunk(Chunk_With_Children* parent,const char* data,size_t data_size)
:Chunk(parent,"AVPENVIR")
{
	size_t size=max(data_size,sizeof(AVP_Environment_Settings));

	settings=(AVP_Environment_Settings*)new unsigned char[size];
	memcpy(settings,data,data_size);

	if(settings->data_size<16)
	{
		settings->sky_colour_red=200;
		settings->sky_colour_green=200;
		settings->sky_colour_blue=200;
	}
	if(settings->data_size<24)
	{
		settings->predator_pistol=1;
		settings->predator_plasmacaster=1;
		settings->predator_disc=1;
		settings->predator_medicomp=1;
		settings->predator_grappling_hook=0;
		settings->predator_num_spears=30;
		settings->marine_jetpack=0;
		settings->stars_in_sky=0;
		settings->spare_bits=0;
	}

	settings->data_size=size;
}
AVP_Environment_Settings_Chunk::AVP_Environment_Settings_Chunk(Chunk_With_Children* parent)
:Chunk(parent,"AVPENVIR")
{
	settings=(AVP_Environment_Settings*)new unsigned char[sizeof(AVP_Environment_Settings)];
	settings->data_size=sizeof(AVP_Environment_Settings);
	settings->sky_colour_red=200;
	settings->sky_colour_green=200;
	settings->sky_colour_blue=200;

	settings->predator_pistol=1;
	settings->predator_plasmacaster=1;
	settings->predator_disc=1;
	settings->predator_medicomp=1;
	settings->predator_grappling_hook=0;
	settings->predator_num_spears=30;
	settings->marine_jetpack=0;
	settings->stars_in_sky=0;
	settings->spare_bits=0;
			
}


AVP_Environment_Settings_Chunk::~AVP_Environment_Settings_Chunk()
{
	delete [] settings;
}

void AVP_Environment_Settings_Chunk::fill_data_block (char * data_start)
{
	strncpy (data_start, identifier, 8);
	data_start += 8;
	*((int *) data_start) = chunk_size;
	data_start += 4;
	
	memcpy(data_start,settings,settings->data_size);
}

size_t AVP_Environment_Settings_Chunk::size_chunk ()
{
	chunk_size = 12+settings->data_size;
	return(chunk_size);
	
}

AVP_Environment_Settings_Chunk* GetAVPEnvironmentSettings(Environment_Data_Chunk* env_chunk)
{
	assert(env_chunk);
	
	AVP_Environment_Settings_Chunk* env_set=(AVP_Environment_Settings_Chunk*)env_chunk->lookup_single_child("AVPENVIR");
	if(!env_set) env_set=new AVP_Environment_Settings_Chunk(env_chunk);

	return env_set;
}
//////////////////////////////////////////////////////////////////////////////
RIF_IMPLEMENT_DYNCREATE("AVPDECAL",AVP_Decal_Chunk)

AVP_Decal_Chunk::AVP_Decal_Chunk(Chunk_With_Children* parent,const char* data,size_t)
:Chunk(parent,"AVPDECAL")
{
	num_decals=*(int*)data;
	data+=4;

	int loaded_decal_size=*(int*)data;
	data+=4;

	decal_size=max(loaded_decal_size,(int)sizeof(AVP_Decal));

	//allocate buffer for decals , and initialise to zero
	decal_buffer=new char[num_decals*decal_size];
	memset(decal_buffer,0,num_decals*decal_size);

	char* buffer_ptr=decal_buffer;
	for(int i=0;i<num_decals;i++)
	{
		memcpy(buffer_ptr,data,loaded_decal_size);
		buffer_ptr+=decal_size;
		data+=loaded_decal_size;
	}

	//only allow access to the decals if the loaded structure size is less than or equal
	//to the current stucture size
	if(loaded_decal_size<=(int)sizeof(AVP_Decal))
	{
		decals=(AVP_Decal*)decal_buffer;
	}
	else
	{
		decals=0;
	}
	
}

AVP_Decal_Chunk::AVP_Decal_Chunk(Chunk_With_Children* parent,int num_dec)
:Chunk(parent,"AVPDECAL")
{
	num_decals=num_dec;
	decal_size=sizeof(AVP_Decal);

	//allocate buffer for decals , and initialise to zero
	decal_buffer=new char[num_decals*decal_size];
	memset(decal_buffer,0,num_decals*decal_size);

	decals=(AVP_Decal*)decal_buffer;
	
}

AVP_Decal_Chunk::~AVP_Decal_Chunk()
{
	if(decal_buffer) delete [] decal_buffer;
}

void AVP_Decal_Chunk::fill_data_block(char* data)
{
	strncpy (data, identifier, 8);
	data += 8;
	*((int *) data) = chunk_size;
	data += 4;

	*(int*) data = num_decals;
	data+=4;

	*(int*) data = decal_size;
	data+=4;

	memcpy(data,decal_buffer,decal_size*num_decals);

}

size_t AVP_Decal_Chunk::size_chunk()
{
	chunk_size=12+8+num_decals*decal_size;
	return chunk_size;
}
//////////////////////////////////////////////////////////////////////////////
//Class AVP_Particle_Generator_Chunk
RIF_IMPLEMENT_DYNCREATE("PARGENER",AVP_Particle_Generator_Chunk)

CHUNK_WITH_CHILDREN_LOADER("PARGENER",AVP_Particle_Generator_Chunk)

/*
Children for AVP_Particle_Generator_Chunk :

AVP_Particle_Generator_Data_Chunk 	"PARGENDA"
Object_Alternate_Locations_Chunk	"ALTLOCAT"
Indexed_Sound_Chunk					"INDSOUND"
*/


AVP_Particle_Generator_Chunk::AVP_Particle_Generator_Chunk(Chunk_With_Children* parent , const char* name,ObjectID & id)
:Chunk_With_Children(parent,"PARGENER")
{
	new AVP_Particle_Generator_Data_Chunk(this,name,id); 
}

AVP_Particle_Generator_Data_Chunk* AVP_Particle_Generator_Chunk::get_data_chunk()
{
	return (AVP_Particle_Generator_Data_Chunk*) lookup_single_child("PARGENDA");
}

Object_Alternate_Locations_Chunk* AVP_Particle_Generator_Chunk::get_alternate_locations_chunk()
{
	return (Object_Alternate_Locations_Chunk*) lookup_single_child("ALTLOCAT");
}

Indexed_Sound_Chunk* AVP_Particle_Generator_Chunk::get_sound_chunk()
{
	return (Indexed_Sound_Chunk*) lookup_single_child("INDSOUND");
}

//////////////////////////////////////////////////////////////////////////////
RIF_IMPLEMENT_DYNCREATE("PARGENDA",AVP_Particle_Generator_Data_Chunk)

AVP_Particle_Generator_Data_Chunk::AVP_Particle_Generator_Data_Chunk(Chunk_With_Children* parent, const char* _name,ObjectID & _id)
:Chunk(parent,"PARGENDA")
{
	name = new char[strlen(_name)+1];
	strcpy(name,_name);
	id=_id;
	parent_id.id1=0;
	parent_id.id2=0;
	time=probability=speed=quantity=0;
	spare1=spare2=0;
}


AVP_Particle_Generator_Data_Chunk::AVP_Particle_Generator_Data_Chunk(Chunk_With_Children* parent, const char* data,size_t)
:Chunk(parent,"PARGENDA")
{
	
	CHUNK_EXTRACT(type,int)
	CHUNK_EXTRACT(flags,int)
	CHUNK_EXTRACT(time,unsigned short)
	CHUNK_EXTRACT(probability,unsigned short)
	CHUNK_EXTRACT(speed,unsigned short)
	CHUNK_EXTRACT(quantity,unsigned short)
	CHUNK_EXTRACT(spare1,int)
	CHUNK_EXTRACT(spare2,int)
	CHUNK_EXTRACT(id,ObjectID)
	CHUNK_EXTRACT(parent_id,ObjectID)
	CHUNK_EXTRACT(location,ChunkVectorInt)
	CHUNK_EXTRACT(orientation,ChunkQuat)
	CHUNK_EXTRACT_STRING(name);
}

AVP_Particle_Generator_Data_Chunk::~AVP_Particle_Generator_Data_Chunk()
{
	if(name) delete [] name;
}

void AVP_Particle_Generator_Data_Chunk::fill_data_block(char* data)
{
	CHUNK_FILL_START
	
	CHUNK_FILL(type,int)
	CHUNK_FILL(flags,int)
	CHUNK_FILL(time,unsigned short)
	CHUNK_FILL(probability,unsigned short)
	CHUNK_FILL(speed,unsigned short)
	CHUNK_FILL(quantity,unsigned short)
	CHUNK_FILL(spare1,int)
	CHUNK_FILL(spare2,int)
	CHUNK_FILL(id,ObjectID)
	CHUNK_FILL(parent_id,ObjectID)
	CHUNK_FILL(location,ChunkVectorInt)
	CHUNK_FILL(orientation,ChunkQuat)
	CHUNK_FILL_STRING(name)
}

size_t AVP_Particle_Generator_Data_Chunk::size_chunk()
{
	chunk_size=12+6*sizeof(int)+sizeof(ObjectID)*2;
	chunk_size+=sizeof(ChunkVectorInt)+sizeof(ChunkQuat);
	chunk_size+=(strlen(name)+4)&~3;
	return chunk_size;

}

//////////////////////////////////////////////////////////////////////////////

RIF_IMPLEMENT_DYNCREATE("R6GENDAT",Rainbow6_Generator_Extra_Data_Chunk)

Rainbow6_Generator_Extra_Data_Chunk::Rainbow6_Generator_Extra_Data_Chunk(Chunk_With_Children* parent,const char* data,size_t)
:Chunk(parent,"R6GENDAT")
{
	distance=*(int*)data;
	data+=4;
	flags=*(int*)data;
	data+=4;
	
	num_extra_data=*(int*)data;
	data+=4;
	if(num_extra_data)
	{
		extra_data=new int[num_extra_data];
		for(int i=0;i<num_extra_data;i++)
		{
			extra_data[i]=*(int*)data;
			data+=4;
		}
	}
	else
	{
		extra_data=0;
	}
}

Rainbow6_Generator_Extra_Data_Chunk::Rainbow6_Generator_Extra_Data_Chunk(Chunk_With_Children* parent)
:Chunk(parent,"R6GENDAT")
{
	distance=0;
	flags=0;
	num_extra_data=0;
	extra_data=0;
}

Rainbow6_Generator_Extra_Data_Chunk::~Rainbow6_Generator_Extra_Data_Chunk()
{
	if(extra_data) delete extra_data;
}


void Rainbow6_Generator_Extra_Data_Chunk::fill_data_block(char* data)
{
	strncpy (data, identifier, 8);
	data += 8;
	*(int *) data = chunk_size;
	data += 4;

	*(int*)data = distance;
	data+=4;
	*(int*)data = flags;
	data+=4;
	*(int*)data = num_extra_data;
	data+=4;

	for(int i=0;i<num_extra_data;i++)
	{
		*(int*)data = extra_data[i];
		data+=4;
	}
}

size_t Rainbow6_Generator_Extra_Data_Chunk::size_chunk()
{
	chunk_size=12+12+4*num_extra_data;
	return chunk_size;
}



