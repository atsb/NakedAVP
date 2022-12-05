#ifndef _oechunk_h_
#define _oechunk_h_ 1

#include "chunk.hpp"
#include "obchunk.hpp"

struct	ChunkMapBlock
{
	char TemplateName[20];
	char TemplateNotes[100];
	int MapType;
	int MapShape;
	int MapFlags;
	int MapFlags2;
	int MapFlags3;
	int MapCType;
	int MapCGameType;
	int MapCStrategyS;
	int MapCStrategyL;
	int MapInteriorType;
	int MapLightType;
	int MapMass;
	VECTORCH MapNewtonV;
	VECTORCH MapOrigin;
	int MapViewType;

	int MapVDBData;
	int SimShapeList;
	


};

class Map_Block_Chunk : public Chunk
{
public:
	virtual size_t size_chunk()
	{
		return (chunk_size=216);
	}
	virtual BOOL output_chunk (HANDLE &);

	virtual void fill_data_block (char * data_start);

	ChunkMapBlock map_data;
	friend class Object_Project_Data_Chunk;
	
	Map_Block_Chunk (Object_Project_Data_Chunk * parent)
		:Chunk(parent,"MAPBLOCK")
	{}

	//constructor from buffer
	Map_Block_Chunk (Chunk_With_Children * parent,const char* data,size_t);
};

struct ChunkStrategy
{
	char StrategyName[20];
	char StrategyNotes[100];
	int Strategy;
};

class Strategy_Chunk : public Chunk
{
public :
	virtual size_t size_chunk()
	{
		return (chunk_size=136);
	}
	virtual BOOL output_chunk (HANDLE &);

	virtual void fill_data_block (char * data_start);

	ChunkStrategy strategy_data;
	friend class Object_Project_Data_Chunk;

	Strategy_Chunk(Object_Project_Data_Chunk *parent)
		:Chunk(parent,"STRATEGY")
	{}

	//constructor from buffer
	Strategy_Chunk (Chunk_With_Children * parent,const char* data,size_t);
};

#endif
