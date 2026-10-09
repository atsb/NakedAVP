#include "toolchnk.hpp"

//macro for helping to force inclusion of chunks when using libraries
FORCE_CHUNK_INCLUDE_IMPLEMENT(toolchnk)

RIF_IMPLEMENT_DYNCREATE("CAMORIGN",Camera_Origin_Chunk)

Camera_Origin_Chunk::Camera_Origin_Chunk(Chunk_With_Children* parent)
:Chunk(parent,"CAMORIGN")
{
	location.x=0;
	location.y=0;
	location.z=0;
	ChunkMatrix identity={1,0,0,0,1,0,0,0,1};
	orientation=identity;
}

Camera_Origin_Chunk::Camera_Origin_Chunk(Chunk_With_Children* parent,const char* data,size_t )
:Chunk(parent,"CAMORIGN")
{
	location=*(ChunkVector*)data;
	data+=sizeof(ChunkVector);
	orientation=*(ChunkMatrix*)data;
}

void Camera_Origin_Chunk::fill_data_block(char * data_start)
{
	strncpy (data_start, identifier, 8);
	data_start += 8;
	*((int *) data_start) = chunk_size;
	data_start += 4;

	*(ChunkVector*)data_start=location;
	data_start+=sizeof(ChunkVector);
	*(ChunkMatrix*)data_start=orientation;
}

