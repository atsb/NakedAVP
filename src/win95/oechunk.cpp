#include "oechunk.h"
#include "chunk.hpp"

//macro for helping to force inclusion of chunks when using libraries
FORCE_CHUNK_INCLUDE_IMPLEMENT(oechunk)

extern Chunk *Parent_File;

RIF_IMPLEMENT_DYNCREATE("MAPBLOCK",Map_Block_Chunk)

Map_Block_Chunk::Map_Block_Chunk(Chunk_With_Children * parent,const char* data,size_t)
	:Chunk(parent,"MAPBLOCK")
{
	strncpy(map_data.TemplateName,data,20);
	strncpy(map_data.TemplateNotes,data+20,100);
	map_data.MapType=*((int*)(data+120));
	map_data.MapShape=*((int*)(data+124));
	map_data.MapFlags=*((int*)(data+128));
	map_data.MapFlags2=*((int*)(data+132));
	map_data.MapFlags3=*((int*)(data+136));
	map_data.MapCType=*((int*)(data+140));
	map_data.MapCGameType=*((int*)(data+144));
	map_data.MapCStrategyS=*((int*)(data+148));
	map_data.MapCStrategyL=*((int*)(data+152));
	map_data.MapInteriorType=*((int*)(data+126));
	map_data.MapLightType=*((int*)(data+160));
	map_data.MapMass=*((int*)(data+164));
	map_data.MapNewtonV.vx=*((int*)(data+168));
	map_data.MapNewtonV.vy=*((int*)(data+172));
	map_data.MapNewtonV.vz=*((int*)(data+176));
	map_data.MapOrigin.vx=*((int*)(data+180));
	map_data.MapOrigin.vy=*((int*)(data+184));
	map_data.MapOrigin.vz=*((int*)(data+188));
	map_data.MapViewType=*((int*)(data+192));
	map_data.MapVDBData=*((int*)(data+196));
	map_data.SimShapeList=*((int*)(data+200));
}

RIF_IMPLEMENT_DYNCREATE("STRATEGY",Strategy_Chunk)

Strategy_Chunk::Strategy_Chunk(Chunk_With_Children * parent,const char * data,size_t)
	:Chunk(parent,"STRATEGY")
{
	strncpy(strategy_data.StrategyName,data,20);
	strncpy(strategy_data.StrategyNotes,data+20,100);
	strategy_data.Strategy=*((int*)(data+120));
}

void Map_Block_Chunk::fill_data_block (char* data_start)
{
	strncpy (data_start, identifier, 8);

	data_start += 8;

	*((int *) data_start) = chunk_size;

	data_start += 4;

	strncpy (data_start,map_data.TemplateName,20);
	data_start+=20;
	strncpy (data_start,map_data.TemplateNotes,100);
	data_start+=100;
	
	*((int*)data_start)=map_data.MapType;
	data_start += 4;
	
	*((int*)data_start)=map_data.MapShape;
	data_start += 4;
	*((int*)data_start)=map_data.MapFlags;
	data_start += 4;
	*((int*)data_start)=map_data.MapFlags2;
	data_start += 4;
	*((int*)data_start)=map_data.MapFlags3;
	data_start += 4;
	*((int*)data_start)=map_data.MapCType;
	data_start += 4;
	*((int*)data_start)=map_data.MapCGameType;
	data_start += 4;
	*((int*)data_start)=map_data.MapCStrategyS;
	data_start+=4;
	*((int*)data_start)=map_data.MapCStrategyL;;
	data_start+=4;
	*((int*)data_start)=map_data.MapInteriorType;
	data_start+=4;
	*((int*)data_start)=map_data.MapLightType;
	data_start+=4;
	*((int*)data_start)=map_data.MapMass;
	data_start+=4;
	*((int*)data_start)=map_data.MapNewtonV.vx;
	data_start+=4;
	*((int*)data_start)=map_data.MapNewtonV.vy;
	data_start+=4;
	*((int*)data_start)=map_data.MapNewtonV.vz;
	data_start+=4;
	*((int*)data_start)=map_data.MapOrigin.vx;
	data_start+=4;
	*((int*)data_start)=map_data.MapOrigin.vy;
	data_start+=4;
	*((int*)data_start)=map_data.MapOrigin.vz;
	data_start+=4;
	*((int*)data_start)=map_data.MapViewType;
	data_start+=4;
	*((int*)data_start)=map_data.MapVDBData;
	data_start+=4;
	*((int*)data_start)=map_data.SimShapeList;
}

void Strategy_Chunk::fill_data_block(char* data_start)
{
	strncpy (data_start, identifier, 8);

	data_start += 8;

	*((int *) data_start) = chunk_size;

	data_start += 4;

	strncpy (data_start,strategy_data.StrategyName,20);
	data_start+=20;
	strncpy (data_start,strategy_data.StrategyNotes,100);
	data_start+=100;
	
	*((int*)data_start)=strategy_data.Strategy;
}

BOOL Map_Block_Chunk::output_chunk(HANDLE &hand)
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

BOOL Strategy_Chunk::output_chunk(HANDLE &hand)
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
