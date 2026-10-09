#include "hierplace.hpp"
#include "md5.h"

//macro for helping to force inclusion of chunks when using libraries
FORCE_CHUNK_INCLUDE_IMPLEMENT(hierplace)

////////////////////////////////////////////////
//Class Placed_Hiearchy_Chunk


RIF_IMPLEMENT_DYNCREATE("PLACHIER",Placed_Hierarchy_Chunk)

CHUNK_WITH_CHILDREN_LOADER("PLACHIER",Placed_Hierarchy_Chunk)
/*
Children for Placed_Hiearchy_Chunk :

"PLACHIDT"		Placed_Hierarchy_Data_Chunk
"PLHISEQU"		Placed_Hierarchy_Sequence_Chunk
"INDSOUND"		Indexed_Sound_Chunk
"AVPSTRAT"		AVP_Strategy_Chunk
*/



Placed_Hierarchy_Chunk::Placed_Hierarchy_Chunk(Chunk_With_Children* parent,const char* _name,int _hierarchy_index,ChunkVectorInt& _location,ChunkQuat& _orientation)
:Chunk_With_Children(parent,"PLACHIER")
{
	new Placed_Hierarchy_Data_Chunk(this,_name,_hierarchy_index,_location,_orientation);
}

///////////////////////////////////////////////////////////////////////
RIF_IMPLEMENT_DYNCREATE("PLACHIDT",Placed_Hierarchy_Data_Chunk)

Placed_Hierarchy_Data_Chunk::Placed_Hierarchy_Data_Chunk(Chunk_With_Children* parent,const char* data, const size_t)
:Chunk(parent,"PLACHIDT")
{
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
	data+=(length+4)&~3;

	hierarchy_index=*(int*)data;
	data+=4;

	location=*(ChunkVectorInt*)data;
	data+=sizeof(ChunkVectorInt);

	orientation=*(ChunkQuat*)data;
	data+=sizeof(ChunkQuat);

	id=*(ObjectID*)data;
	data+=sizeof(ObjectID);

	num_extra_data=*(int*) data;
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

Placed_Hierarchy_Data_Chunk::Placed_Hierarchy_Data_Chunk(Chunk_With_Children* parent,const char* _name,int _hierarchy_index,ChunkVectorInt& _location,ChunkQuat& _orientation)
:Chunk(parent,"PLACHIDT")
{
	if(_name)
	{
		name=new char[strlen(_name)+1];
		strcpy(name,_name);
	}
	else
	{
		name=0;
	}
	location=_location;
	orientation=_orientation;
	hierarchy_index=_hierarchy_index;

	num_extra_data=0;
	extra_data=0;

	char buffer[16];
	md5_buffer(name,strlen(name),&buffer[0]);
	buffer[7]=0;
	id = *(ObjectID*)&buffer[0];
}

Placed_Hierarchy_Data_Chunk::~Placed_Hierarchy_Data_Chunk()
{
	if(extra_data)	delete [] extra_data;
	if(name) delete [] name;
}

void Placed_Hierarchy_Data_Chunk::fill_data_block(char* data)
{
	strncpy (data, identifier, 8);
	data += 8;
	*((int *) data) = chunk_size;
	data += 4;

	if(name)
	{
		strcpy(data,name);
	}
	else
	{
		*(int*)data=0;
	}
	data+=(strlen(data)+4)&~3;

	*(int*)data=hierarchy_index;
	data+=4;

	*(ChunkVectorInt*)data=location;
	data+=sizeof(ChunkVectorInt);

	*(ChunkQuat*) data=orientation;
	data+=sizeof(ChunkQuat);

	*(ObjectID*) data=id;
	data+=sizeof(ObjectID);

	*(int*)data=num_extra_data;
	data+=4;

	for(int i=0;i<num_extra_data;i++)
	{
		*(int*) data=extra_data[i];
		data+=4;
	}

}

size_t Placed_Hierarchy_Data_Chunk::size_chunk()
{
	chunk_size=12+4+sizeof(ChunkVectorInt)+sizeof(ChunkQuat)+sizeof(ObjectID);
	chunk_size+=4+4*num_extra_data;
	if(name)
		chunk_size+=(strlen(name)+4)&~3;
	else
		chunk_size+=4;

	return chunk_size;
}

/////////////////////////////////////////////////////////////////////////
RIF_IMPLEMENT_DYNCREATE("PLHISEQU",Placed_Hierarchy_Sequence_Chunk)

Placed_Hierarchy_Sequence_Chunk::Placed_Hierarchy_Sequence_Chunk(Chunk_With_Children* parent,const char* data, const size_t)
:Chunk(parent,"PLHISEQU")
{
	index=*(int*) data;
	data+=4;
	
	sequence=*(int*) data;
	data+=4;
	
	sub_sequence=*(int*)data;
	data+=4;
	
	time=*(int*)data ;
	data+=4;
	
	flags=*(int*)data ;
	data+=4;

	sound_list_size=*(int*)data;
	data+=4;

	if(sound_list_size)
	{
		sound_list=new Timed_Sound_Description[sound_list_size];
		for(int i=0;i<sound_list_size;i++)
		{
			sound_list[i]=*(Timed_Sound_Description*)data;
			data+=sizeof(Timed_Sound_Description);
		}
	}
	else
	{
		sound_list=0;
	}
		
	num_extra_data=*(int*) data;
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

Placed_Hierarchy_Sequence_Chunk::Placed_Hierarchy_Sequence_Chunk(Chunk_With_Children* parent,int _index)
:Chunk(parent,"PLHISEQU")
{
	index=_index;
	sequence=0;
	sub_sequence=0;
	time=1000;
	sound_list_size=0;
	sound_list=0;
	extra_data=0;
	num_extra_data=0;
	flags=0;


}

Placed_Hierarchy_Sequence_Chunk::~Placed_Hierarchy_Sequence_Chunk()
{
	if(extra_data)
	{
		delete extra_data;
	}
	if(sound_list)
	{
		delete sound_list;
	}		
}

void Placed_Hierarchy_Sequence_Chunk::fill_data_block(char* data)
{
	strncpy (data, identifier, 8);
	data += 8;
	*((int *) data) = chunk_size;
	data += 4;

	*(int*)data=index;
	data+=4;

	*(int*)data=sequence;
	data+=4;

	*(int*)data=sub_sequence;
	data+=4;

	*(int*)data=time;
	data+=4;

	*(int*)data=flags;
	data+=4;

	*(int*)data=sound_list_size;
	data+=4;

	for(int i=0;i<sound_list_size;i++)
	{
		*(Timed_Sound_Description*) data=sound_list[i];
		data+=sizeof(Timed_Sound_Description);
	}

	*(int*)data=num_extra_data;
	data+=4;

	for(int i=0;i<num_extra_data;i++)
	{
		*(int*) data=extra_data[i];
		data+=4;
	}
}

size_t Placed_Hierarchy_Sequence_Chunk::size_chunk()
{
	chunk_size=12+24;
	chunk_size+=sound_list_size*sizeof(Timed_Sound_Description);
	chunk_size+=4+4*num_extra_data;
	return chunk_size;
}
