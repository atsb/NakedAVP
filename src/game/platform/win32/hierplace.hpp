#ifndef _hierplace_hpp
#define _hierplace_hpp 1

#include "chunk.hpp"
#include "chnktype.hpp"

class Placed_Hierarchy_Data_Chunk;

class Placed_Hierarchy_Chunk : public Chunk_With_Children
{
public :

	Placed_Hierarchy_Chunk(Chunk_With_Children* parent,const char* _name,int _hierarchy_index,ChunkVectorInt& _location,ChunkQuat& _orientation);
	Placed_Hierarchy_Chunk(Chunk_With_Children * const parent,const char* ,const size_t);


	Placed_Hierarchy_Data_Chunk* get_data_chunk()
	{
		return (Placed_Hierarchy_Data_Chunk*)lookup_single_child("PLACHIDT");
	}
};

class  Placed_Hierarchy_Data_Chunk : public Chunk
{
public :	
	Placed_Hierarchy_Data_Chunk(Chunk_With_Children* parent,const char* , const size_t);
	Placed_Hierarchy_Data_Chunk(Chunk_With_Children* parent,const char* _name,int _hierarchy_index,ChunkVectorInt& _location,ChunkQuat& _orientation);
	~Placed_Hierarchy_Data_Chunk();

	void fill_data_block(char*);
	size_t size_chunk();



	char* name;
	
	int hierarchy_index;

	ChunkVectorInt location;
	ChunkQuat	orientation;

	ObjectID id;

	int num_extra_data;
	int* extra_data;


};

struct Timed_Sound_Description
{
	int sound_index;
	int start_time; //in ms
	int end_time; //in ms
};


#define  HierarchySequenceFlag_Loop 0x00000001
#define  HierarchySequenceFlag_InitialSequence 0x00000002
#define  HierarchySequenceFlag_Playing 0x00000004
class Placed_Hierarchy_Sequence_Chunk : public Chunk
{
public:	
	Placed_Hierarchy_Sequence_Chunk(Chunk_With_Children* parent,const char* , const size_t);
	Placed_Hierarchy_Sequence_Chunk(Chunk_With_Children* parent,int _index);
	~Placed_Hierarchy_Sequence_Chunk();
	
	void fill_data_block(char*);
	size_t size_chunk();
	
	
	int index;
	int sequence;
	int sub_sequence;
	int time; //time to play sequence in ms
	int flags;
	
	int sound_list_size;
	Timed_Sound_Description* sound_list;
	

	int num_extra_data;
	int* extra_data;
};


#endif
