#ifndef _toolchnk_hpp
#define _toolchnk_hpp

#include "chunk.hpp"
#include "chnktype.hpp"

class Camera_Origin_Chunk :public Chunk
{
public :
	Camera_Origin_Chunk(Chunk_With_Children* parent);
	Camera_Origin_Chunk(Chunk_With_Children * parent,const char* data,size_t size);

	ChunkVector location;
	ChunkMatrix orientation;//not used yet

	virtual void fill_data_block (char * data_start);
	virtual size_t size_chunk (){return chunk_size=12+sizeof(ChunkVector)+sizeof(ChunkMatrix); }


};


#endif
