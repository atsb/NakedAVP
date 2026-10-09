#ifndef wpchunk_hpp
#define wpchunk_hpp 1

#include "chunk.hpp"
#include "chnktype.hpp"
#include "obchunk.hpp"

struct WaypointLink
{
	int index;
	int flags;
};

struct ModuleLink
{
	~ModuleLink();

	char*  module_name;
	int flags;
};

#define WaypointFlag_CentreDefinedByUser 0x80000000
#define WaypointFlag_FirstGroundWaypoint 0x40000000 
struct ChunkWaypoint
{
	ChunkWaypoint();
	~ChunkWaypoint();
	
	int index;
	ChunkVectorInt min,max; //relative to centre
	ChunkVectorInt centre; //relative to world

	int NumWPLinks;
	WaypointLink* WayLinks;

	int NumModLinks;
	ModuleLink* ModLinks;

	int flags,spare2;

};

class Module_Waypoint_Chunk : public Chunk
{
	public :
	Module_Waypoint_Chunk(Chunk_With_Children*,const char *,size_t);
	Module_Waypoint_Chunk(Chunk_With_Children*);
	~Module_Waypoint_Chunk();

	virtual size_t size_chunk();
	virtual void fill_data_block(char* data_start);
	
	//Copies waypoint data and deletes the old waypoint_chunk
	void TransferWaypointData(Module_Waypoint_Chunk*);

	int NumWaypoints;
	ChunkWaypoint* Waypoints;

	ChunkWaypoint* AlienWaypoints;
	ChunkWaypoint* GroundWaypoints;
	
	short NumAlienWaypoints;
	short NumGroundWaypoints;

	int spare1;
	int spare2;
};

class AI_Module_Master_Chunk : public Chunk
{
	public :
	AI_Module_Master_Chunk(Chunk_With_Children*,const char*,size_t);
	AI_Module_Master_Chunk(Object_Module_Data_Chunk*);

	virtual size_t size_chunk();
	virtual void fill_data_block(char* data_start);

	void AddModule(Object_Chunk*);
	Object_Chunk* get_my_object_chunk();

	List<Object_Chunk*> ModuleList;
};

class AI_Module_Slave_Chunk : public Chunk
{
	public :
	AI_Module_Slave_Chunk(Chunk_With_Children*,const char*,size_t);
	AI_Module_Slave_Chunk(Object_Module_Data_Chunk*,Object_Chunk*);
	~AI_Module_Slave_Chunk();

	virtual size_t size_chunk();
	virtual void fill_data_block(char* data_start);
	virtual void post_input_processing();

	Object_Chunk* get_my_object_chunk();

	Object_Chunk* MasterModule;
	int MasterModuleIndex;
};


#endif


