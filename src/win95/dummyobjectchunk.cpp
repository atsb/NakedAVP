#include "dummyobjectchunk.hpp"

//macro for helping to force inclusion of chunks when using libraries
FORCE_CHUNK_INCLUDE_IMPLEMENT(dummyobjectchunk)


RIF_IMPLEMENT_DYNCREATE("DUMMYOBJ",Dummy_Object_Chunk)

CHUNK_WITH_CHILDREN_LOADER("DUMMYOBJ",Dummy_Object_Chunk)
/*
Children for Dummy_Object_Chunk :

"DUMOBJDT"	Dummy_Object_Data_Chunk
"DUMOBJTX"	Dummy_Object_Text_Chunk
*/



Dummy_Object_Chunk::Dummy_Object_Chunk(Chunk_With_Children* parent,const char* _name,ChunkVectorInt& _location,ChunkVectorInt& min ,ChunkVectorInt& max ,ChunkQuat& orient)
:Chunk_With_Children(parent,"DUMMYOBJ")
{
	new Dummy_Object_Data_Chunk(this,_name,_location,min,max,orient);
}

Dummy_Object_Data_Chunk* Dummy_Object_Chunk::get_data_chunk()
{
	return (Dummy_Object_Data_Chunk*) lookup_single_child("DUMOBJDT");
}


const char* Dummy_Object_Chunk::get_text()
{
	//find the text
	Dummy_Object_Text_Chunk* text_chunk = (Dummy_Object_Text_Chunk*) lookup_single_child("DUMOBJTX");
	if(!text_chunk)
	{
		//no text
		return NULL;
	}
	
	return text_chunk->get_text();
}

void Dummy_Object_Chunk::set_text(const char* text)
{
	//find the text chunk
	Dummy_Object_Text_Chunk* text_chunk = (Dummy_Object_Text_Chunk*) lookup_single_child("DUMOBJTX");

	if(!text || strlen(text)==0)
	{
		//delete the text chunk if it exists
		delete text_chunk;
	}
	else
	{
		if(text_chunk)
		{
			//alter the existing text
			text_chunk->set_text(text);
		}
		else
		{
			//create a new text chunk
			new Dummy_Object_Text_Chunk(this,text);
		}
	}
}


//////////////////////////////Dummy_Object_Data_Chunk////////////////////////////////

RIF_IMPLEMENT_DYNCREATE("DUMOBJDT",Dummy_Object_Data_Chunk)

Dummy_Object_Data_Chunk::Dummy_Object_Data_Chunk(Dummy_Object_Chunk* parent,const char* _name ,ChunkVectorInt& _location,ChunkVectorInt& min ,ChunkVectorInt& max ,ChunkQuat& orient)
:Chunk(parent,"DUMOBJDT")
{
	location=_location;
	min_extents=min;
	max_extents=max;
	orientation=orient;

	name=new char[strlen(_name)+1];
	strcpy(name,_name);
}


Dummy_Object_Data_Chunk::Dummy_Object_Data_Chunk(Chunk_With_Children* parent,const char* data,size_t)
:Chunk(parent,"DUMOBJDT")
{
	CHUNK_EXTRACT(location,ChunkVectorInt)
	CHUNK_EXTRACT(min_extents,ChunkVectorInt)
	CHUNK_EXTRACT(max_extents,ChunkVectorInt)
	CHUNK_EXTRACT(orientation,ChunkQuat)
	CHUNK_EXTRACT_STRING(name)
}


Dummy_Object_Data_Chunk::~Dummy_Object_Data_Chunk()
{
	if(name) delete [] name;
}


void Dummy_Object_Data_Chunk::fill_data_block(char* data)
{
	CHUNK_FILL_START
	
	CHUNK_FILL(location,ChunkVectorInt)
	CHUNK_FILL(min_extents,ChunkVectorInt)
	CHUNK_FILL(max_extents,ChunkVectorInt)
	CHUNK_FILL(orientation,ChunkQuat)
	CHUNK_FILL_STRING(name)

}


size_t Dummy_Object_Data_Chunk::size_chunk()
{
	chunk_size=12+3*sizeof(ChunkVectorInt)+sizeof(ChunkQuat);
	chunk_size+=(strlen(name)+4)&~3;
	return chunk_size;
}

/////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////Dummy_Object_Text_Chunk////////////////////////////////

RIF_IMPLEMENT_DYNCREATE("DUMOBJTX",Dummy_Object_Text_Chunk)

Dummy_Object_Text_Chunk::Dummy_Object_Text_Chunk(Dummy_Object_Chunk* parent,const char* _text)
:Chunk(parent,"DUMOBJTX"),text(NULL)
{
	set_text(_text);	
}

Dummy_Object_Text_Chunk::Dummy_Object_Text_Chunk(Chunk_With_Children* parent,const char* data,size_t)
:Chunk(parent,"DUMOBJTX")
{
	CHUNK_EXTRACT_STRING(text)
}


Dummy_Object_Text_Chunk::~Dummy_Object_Text_Chunk()
{
	delete [] text;
}

void Dummy_Object_Text_Chunk::fill_data_block(char* data)
{
	CHUNK_FILL_START
	
	CHUNK_FILL_STRING(text)
}

void Dummy_Object_Text_Chunk::set_text(const char* _text)
{
	char* new_text = NULL;
	if(_text)
	{
		new_text = new char[strlen(_text)+1];
		strcpy(new_text,_text);
	}

	delete [] text;
	text = new_text;
	
}


size_t Dummy_Object_Text_Chunk::size_chunk()
{
	chunk_size=12;
	if(text)
		chunk_size+=(strlen(text)+4)&~3;
	else
		chunk_size+=4;
	return chunk_size;
}
