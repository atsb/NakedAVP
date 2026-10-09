#ifndef _fragchnk_hpp_
#define _fragchnk_hpp_

#include "chunk.hpp"
#include "chnktype.hpp"


class Fragment_Type_Data_Chunk : public Chunk
{
public :
	
	Fragment_Type_Data_Chunk(Chunk_With_Children* parent,const char* name);
	Fragment_Type_Data_Chunk (Chunk_With_Children * const parent,const char *, size_t const);
	~Fragment_Type_Data_Chunk();
	
	size_t size_chunk ();
	void fill_data_block (char * data_start);
	

	char* frag_type_name;
	int pad1,pad2,pad3;

};

class Fragment_Type_Shape_Chunk : public Chunk
{
public:	
	
	Fragment_Type_Shape_Chunk(Chunk_With_Children* parent,const char* _name,int number,ChunkVectorInt _location);
	Fragment_Type_Shape_Chunk (Chunk_With_Children * const parent,const char *, size_t const);
	~Fragment_Type_Shape_Chunk();
	
	size_t size_chunk ();
	void fill_data_block (char * data_start);
	
	
	int num_fragments;
	ChunkVectorInt location;
	char* name;
	
	int pad1,pad2,pad3;	

};
class Fragment_Type_Sound_Chunk : public Chunk
{
public:	
	
	Fragment_Type_Sound_Chunk(Chunk_With_Children* parent);
	Fragment_Type_Sound_Chunk (Chunk_With_Children * const parent,const char *, size_t const);
	~Fragment_Type_Sound_Chunk();
	
	size_t size_chunk ();
	void fill_data_block (char * data_start);
	
	char* wav_name;
	unsigned long inner_range;
	unsigned long outer_range;
	int max_volume;
	int	pitch;
	int pad;	

};

class Fragment_Type_Chunk : public Chunk_With_Children
{
public :
	Fragment_Type_Chunk(Chunk_With_Children * parent,const char* name)
	: Chunk_With_Children (parent, "FRAGTYPE")
	{new Fragment_Type_Data_Chunk(this,name);}
	
	Fragment_Type_Chunk (Chunk_With_Children * const parent,const char *, size_t const);

	const char* get_name();

};






#endif
