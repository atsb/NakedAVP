#include "fragchnk.hpp"

//macro for helping to force inclusion of chunks when using libraries
FORCE_CHUNK_INCLUDE_IMPLEMENT(fragchnk)

RIF_IMPLEMENT_DYNCREATE("FRAGTYPE",Fragment_Type_Chunk)

CHUNK_WITH_CHILDREN_LOADER("FRAGTYPE",Fragment_Type_Chunk)

/*
Children for Fragment_Type_Chunk :

Fragment_Type_Data_Chunk		"FRGTYPDC"
Fragment_Type_Shape_Chunk		"FRGTYPSC"
Fragment_Type_Sound_Chunk		"FRGSOUND"
*/


const char* Fragment_Type_Chunk::get_name()
{
	Fragment_Type_Data_Chunk* ftdc=(Fragment_Type_Data_Chunk*)lookup_single_child("FRGTYPDC");

	if(ftdc)
		return ftdc->frag_type_name;
	else
		return 0;
	
}

////////////////////////////////////////////////////////////////////////
RIF_IMPLEMENT_DYNCREATE("FRGTYPDC",Fragment_Type_Data_Chunk)

Fragment_Type_Data_Chunk::Fragment_Type_Data_Chunk(Chunk_With_Children* parent,const char* name)
:Chunk(parent,"FRGTYPDC")
{
	frag_type_name=new char[strlen(name)+1];
	strcpy(frag_type_name,name);
	pad1=pad2=pad3=0;
}

Fragment_Type_Data_Chunk::Fragment_Type_Data_Chunk(Chunk_With_Children* const parent,const char* data,size_t const )
:Chunk(parent,"FRGTYPDC")
{
	int length=strlen(data)+1;
	frag_type_name=new char[length];
	strcpy(frag_type_name,data);
	data+=(length+3)&~3;

	pad1=*(int*)data;
	data+=4;
	pad2=*(int*)data;
	data+=4;
	pad3=*(int*)data;
	data+=4;
}

Fragment_Type_Data_Chunk::~Fragment_Type_Data_Chunk()
{
	if(frag_type_name) delete [] frag_type_name;
}											 

void Fragment_Type_Data_Chunk::fill_data_block(char* data_start)
{
	strncpy (data_start, identifier, 8);
	data_start += 8;
	*((int *) data_start) = chunk_size;
	data_start += 4;

	strcpy(data_start,frag_type_name);
	data_start+=(strlen(frag_type_name)+4)&~3;	

	*(int*)data_start=pad1;
	data_start+=4;
	*(int*)data_start=pad2;
	data_start+=4;
	*(int*)data_start=pad3;
	data_start+=4;

}

size_t Fragment_Type_Data_Chunk::size_chunk()
{
	chunk_size=12+12;
	chunk_size+=(strlen(frag_type_name)+4)&~3;
	return chunk_size;
}


////////////////////////////////////////////////////////////////////////
RIF_IMPLEMENT_DYNCREATE("FRGTYPSC",Fragment_Type_Shape_Chunk)

Fragment_Type_Shape_Chunk::Fragment_Type_Shape_Chunk(Chunk_With_Children* parent,const char* _name,int number,ChunkVectorInt _location)
:Chunk(parent,"FRGTYPSC")
{
	name=new char[strlen(_name)+1];
	strcpy(name,_name);
	num_fragments=number;
	location=_location;
	pad1=pad2=pad3=0;
}

Fragment_Type_Shape_Chunk::Fragment_Type_Shape_Chunk(Chunk_With_Children* const parent,const char* data,size_t const )
:Chunk(parent,"FRGTYPSC")
{
	int length=strlen(data)+1;
	name=new char[length];
	strcpy(name,data);
	data+=(length+3)&~3;

	num_fragments=*(int*)data;
	data+=4;

	location=*(ChunkVectorInt*)data;
	data+=sizeof(ChunkVectorInt);

	
	pad1=*(int*)data;
	data+=4;
	pad2=*(int*)data;
	data+=4;
	pad3=*(int*)data;
	data+=4;
}

Fragment_Type_Shape_Chunk::~Fragment_Type_Shape_Chunk()
{
	if(name) delete [] name;
}											 

void Fragment_Type_Shape_Chunk::fill_data_block(char* data_start)
{
	strncpy (data_start, identifier, 8);
	data_start += 8;
	*((int *) data_start) = chunk_size;
	data_start += 4;

	strcpy(data_start,name);
	data_start+=(strlen(name)+4)&~3;	

	*(int*)data_start=num_fragments;
	data_start+=4;

	*(ChunkVectorInt*)data_start=location;
	data_start+=sizeof(ChunkVectorInt);
	
	*(int*)data_start=pad1;
	data_start+=4;
	*(int*)data_start=pad2;
	data_start+=4;
	*(int*)data_start=pad3;
	data_start+=4;

}

size_t Fragment_Type_Shape_Chunk::size_chunk()
{
	chunk_size=12+16+sizeof(ChunkVectorInt);
	chunk_size+=(strlen(name)+4)&~3;
	return chunk_size;
}

////////////////////////////////////////////////////////////////////////
RIF_IMPLEMENT_DYNCREATE("FRGSOUND",Fragment_Type_Sound_Chunk)

Fragment_Type_Sound_Chunk::Fragment_Type_Sound_Chunk(Chunk_With_Children* parent)
:Chunk(parent,"FRGSOUND")
{
	wav_name=0;
	inner_range=5000;
	outer_range=40000;
	max_volume=127;
	pitch=0;
	pad=0;
}

Fragment_Type_Sound_Chunk::Fragment_Type_Sound_Chunk(Chunk_With_Children* const parent,const char* data,size_t const )
:Chunk(parent,"FRGSOUND")
{
	inner_range=*(unsigned long*)data;
	data+=4;
	outer_range=*(unsigned long*)data;
	data+=4;

	max_volume=*(int*)data;
	data+=4;
	pitch=*(int*)data;
	data+=4;
	pad=*(int*)data;
	data+=4;

	wav_name=new char[strlen(data)+1];
	strcpy(wav_name,data);

}

Fragment_Type_Sound_Chunk::~Fragment_Type_Sound_Chunk()
{
	if(wav_name) delete [] wav_name;
}											 

void Fragment_Type_Sound_Chunk::fill_data_block(char* data_start)
{
	strncpy (data_start, identifier, 8);
	data_start += 8;
	*((int *) data_start) = chunk_size;
	data_start += 4;
	
	*(unsigned long*)data_start=inner_range;
	data_start+=4;
	*(unsigned long*)data_start=outer_range;
	data_start+=4;

	*(int*)data_start=max_volume;
	data_start+=4;
	*(int*)data_start=pitch;
	data_start+=4;
	*(int*)data_start=pad;
	data_start+=4;

	strcpy(data_start,wav_name);

}

size_t Fragment_Type_Sound_Chunk::size_chunk()
{
	chunk_size=12+20;
	chunk_size+=(strlen(wav_name)+4)&~3;
	return chunk_size;
}

