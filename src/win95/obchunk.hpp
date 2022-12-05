#ifndef _obchunk_hpp
#define _obchunk_hpp 1

#include "chunk.hpp"

#include "chnktype.hpp"
#include "mishchnk.hpp"


// object flags

#define OBJECT_FLAG_BASE_OBJECT 0x0000100
#define OBJECT_FLAG_MODULE_OBJECT 0x0000200
#define OBJECT_FLAG_PLACED_OBJECT 0x0000400

// Note these flags have to correspond with the ones used by ed_ob_type
// and AVP generator flags

#define OBJECT_FLAG_AVPGAMEMODEMARINE 	0x00000800
#define OBJECT_FLAG_AVPGAMEMODEALIEN 		0x00001000
#define OBJECT_FLAG_AVPGAMEMODEPREDATOR	0x00002000

#define OBJECT_FLAG_PLATFORMLOADSET			0x00004000
#define OBJECT_FLAG_PCLOAD 							0x00008000
#define OBJECT_FLAG_PSXLOAD 						0x00010000
#define OBJECT_FLAG_SATURNLOAD 					    0x00020000

#define OBJECT_FLAG_NOTDIFFICULTY1 					0x00100000
#define OBJECT_FLAG_NOTDIFFICULTY2 					0x00200000
#define OBJECT_FLAG_NOTDIFFICULTY3 					0x00400000


class Shape_Chunk;
class Shape_Header_Chunk;
class Object_Header_Chunk;

// flags structure

struct object_flags
{
	unsigned int locked : 1;
// add more flags here as they are needed
};


class VModule_Array_Chunk;

class Object_Chunk : public Lockable_Chunk_With_Children
{
public:

	// constructor from buffer
	Object_Chunk (Chunk_With_Children * parent,const char *, size_t);

	// constructor from data

	Object_Chunk (Chunk_With_Children * parent, ChunkObject &obj);

	// destructor
	virtual ~Object_Chunk();

	const ChunkObject object_data;

	Shape_Chunk * get_assoc_shape();

	BOOL assoc_with_shape (Shape_Chunk *);

	BOOL deassoc_with_shape (Shape_Chunk *);

	Object_Header_Chunk * get_header();

	BOOL inc_v_no();

	BOOL same_and_updated(Object_Chunk &);
	
	BOOL assoc_with_shape_no(File_Chunk *);
	
	void update_my_chunkobject (ChunkObject &);
	
	// functions for the locking functionality

	BOOL file_equals (HANDLE &);
	const char * get_head_id();
	void set_lock_user(char *);

	virtual void post_input_processing();

	VModule_Array_Chunk * get_vmod_chunk();
	
	// destroy object needs to go through all the other
	// objects and delete itself from the VMAC
	
	void destroy(File_Chunk *);
	
	ObjectID CalculateID();

	//program_object_index can be set by the program using the chunks , so you can find where
	//the object has been put.
	//I need it so that I can find out which module each object has been put into.
	int program_object_index;

private:
	
	friend class File_Chunk;
	friend class Object_Header_Chunk;

	ChunkObject * object_data_store;

};

///////////////////////////////////////////////
	
class Object_Header_Chunk : public Chunk
{
public:
	// constructor from buffer
	Object_Header_Chunk (Chunk_With_Children * parent, const char * pdata, size_t psize);	
	virtual size_t size_chunk ();

	virtual BOOL output_chunk (HANDLE &);

	virtual void fill_data_block (char * data_start);

	const ChunkObject * const object_data;

	virtual void prepare_for_output();

	const char * get_lockuser()
	{
		return lock_user;
	}

	int flags;

private:

	friend class Object_Chunk;
	friend class Shape_Header_Chunk;
	friend class RIF_File_Chunk;
	friend class File_Chunk;

	friend class Environment;
	
	char lock_user[17];
	int version_no;

	int shape_id_no;

	Shape_Chunk * associated_shape;



	// constructor from data
	Object_Header_Chunk (Object_Chunk * parent);

};


///////////////////////////////////////////////


// automatically constructed when Object_Chunks are constructed from
// data

class Object_Interface_Data_Chunk : public Chunk_With_Children
{
public:
	// constructor from buffer
	Object_Interface_Data_Chunk (Chunk_With_Children * parent,const char *, size_t);
private:

	friend class Object_Chunk;



	// constructor from Object_Chunks
	Object_Interface_Data_Chunk (Object_Chunk * parent);

};

///////////////////////////////////////////////

class Object_Notes_Chunk : public Chunk
{
public:

	Object_Notes_Chunk (Chunk_With_Children * parent,
									 const char * _data, size_t _data_size);


	virtual ~Object_Notes_Chunk ();

	virtual size_t size_chunk ()
	{
		chunk_size = (data_size + 12);
		chunk_size += (4-chunk_size%4)%4;
		return chunk_size;
	}

	virtual BOOL output_chunk (HANDLE &);

	virtual void fill_data_block (char * data_start);

	const size_t data_size;
	const char * const data;
	
private:

	char * data_store;

};

///////////////////////////////////////////////

class Object_Module_Data_Chunk : public Chunk_With_Children
{
public:

	// constructor from Object_Chunks
	Object_Module_Data_Chunk (Object_Chunk * parent)
	: Chunk_With_Children (parent, "MODULEDT")
	{}
	// constructor from buffer
	Object_Module_Data_Chunk (Chunk_With_Children * parent,const char *, size_t);

private:

	friend class Object_Chunk;
	friend class AI_Module_Master_Chunk; 
	friend class AI_Module_Slave_Chunk; 
	// constructor from buffer
	Object_Module_Data_Chunk (Object_Chunk * parent,const char *, size_t);
	
};

///////////////////////////////////////////////

class VModule_Array_Chunk : public Chunk
{
public:
	
	VModule_Array_Chunk (Object_Module_Data_Chunk * parent, VMod_Arr_Item * vma, int num_in_vma);
	VModule_Array_Chunk (Chunk_With_Children* parent, const char * vmdata, size_t vmsize);
	~VModule_Array_Chunk ();
	
	int num_array_items;
	
	VMod_Arr_Item * vmod_array;
	
	virtual void fill_data_block (char *);
	virtual size_t size_chunk ();

private:

	friend class Object_Module_Data_Chunk;


};


///////////////////////////////////////////////

#if 0
class Adjacent_Modules_Chunk : public Chunk
{
public:

	Adjacent_Modules_Chunk(Object_Module_Data_Chunk * parent, List<Adjacent_Module> aml)
	: Chunk (parent, "ADJMDLST"), adjacent_modules_list (aml)
	{}

	List<Adjacent_Module> adjacent_modules_list;

	virtual void fill_data_block (char *);
	virtual size_t size_chunk ();

	
private:

	friend class Object_Module_Data_Chunk;
	
	Adjacent_Modules_Chunk(Object_Module_Data_Chunk * parent, const char * data, size_t size);
	
};
#endif

//a replacement for adjacent_modules_chunk
#define AdjacentModuleFlag_EntryPointSetByHand 0x00000001
#define AdjacentModuleFlag_InsideAdjacentModule 0x00000002
#define AdjacentModuleFlag_AdjacentModuleInsideMe 0x00000004
#define AdjacentModuleFlag_Vertical 0x00000008

class Adjacent_Module_Entry_Points_Chunk : public Chunk
{
public:

	Adjacent_Module_Entry_Points_Chunk(Object_Module_Data_Chunk * parent, List<Adjacent_Module> aml)
	: Chunk (parent, "ADJMDLEP"), adjacent_modules_list (aml)
	{}
	Adjacent_Module_Entry_Points_Chunk(Chunk_With_Children * parent, const char * data, size_t size);
	List<Adjacent_Module> adjacent_modules_list;

	virtual void fill_data_block (char *);
	virtual size_t size_chunk ();

	
private:

	friend class Object_Module_Data_Chunk;
	

	
};


///////////////////////////////////////////////

class Module_Flag_Chunk : public Chunk
{
public:
	Module_Flag_Chunk(Object_Module_Data_Chunk * parent);
	Module_Flag_Chunk(Chunk_With_Children* parent, const char * data, size_t );
	
	virtual void fill_data_block(char*);
	virtual size_t size_chunk()
	{return chunk_size=20;}

	int Flags;
	int spare;
private:
	friend class Object_Module_Data_Chunk;
	

	
};
///////////////////////////////////////////////

class Module_Zone_Chunk : public Chunk
{
public:
	Module_Zone_Chunk(Object_Module_Data_Chunk * parent);
	Module_Zone_Chunk(Chunk_With_Children* parent, const char * data, size_t );
	
	virtual void fill_data_block(char*);
	virtual size_t size_chunk()
	{return chunk_size=20;}

	int Zone;
	int spare;
private:
	friend class Object_Module_Data_Chunk;
};


///////////////////////////////////////////////
class Module_Acoustics_Chunk : public Chunk
{
public:
	Module_Acoustics_Chunk(Object_Module_Data_Chunk * parent);
	Module_Acoustics_Chunk(Chunk_With_Children* parent, const char * data, size_t );
	
	virtual void fill_data_block(char*);
	virtual size_t size_chunk()
	{return chunk_size=24;}

	int env_index; 
	float reverb;
	int spare;
private:
	friend class Object_Module_Data_Chunk;
	


};
///////////////////////////////////////////////

class Object_Project_Data_Chunk : public Chunk_With_Children
{
public:

	// constructor from Object_Chunks
	Object_Project_Data_Chunk (Object_Chunk * parent)
	: Chunk_With_Children (parent, "OBJPRJDT")
	{}
	// constructor from buffer
	Object_Project_Data_Chunk (Chunk_With_Children * const parent,const char *,const size_t);
private:

	friend class Object_Chunk;

	

};



///////////////////////////////////////////////



struct ChunkTrackSection
{

	ChunkQuat quat_start;
	ChunkQuat quat_end;
	
	ChunkVectorInt pivot_start;
	ChunkVectorInt pivot_end;
	
	ChunkVectorInt object_offset;

	int time_for_section;

	int spare;


};

#define TrackFlag_Loop 0x00000001
#define TrackFlag_PlayingAtStart 0x00000002
#define TrackFlag_LoopBackAndForth 0x00000004
#define TrackFlag_UseTrackSmoothing 0x00000008
#define TrackFlag_QuatProblemSorted 0x80000000

class Object_Track_Chunk2 : public Chunk
{
public:

	Object_Track_Chunk2 (Object_Chunk * parent);
	Object_Track_Chunk2 (Chunk_With_Children * parent,const char *, size_t);

	~Object_Track_Chunk2();
	
	int num_sections;
	ChunkTrackSection * sections;

	int flags;
	int timer_start;
 
	virtual void fill_data_block (char *);
	virtual size_t size_chunk ();
	
private:

	friend class Object_Chunk;
	
	
};

#define TrackSoundFlag_Loop 0x00000001
class Object_Track_Sound_Chunk : public Chunk
{
	public :
	
	Object_Track_Sound_Chunk(Chunk_With_Children* parent);
	Object_Track_Sound_Chunk (Chunk_With_Children * const parent,const char *, size_t const);
	~Object_Track_Sound_Chunk();
	
	size_t size_chunk ();
	void fill_data_block (char * data_start);
	
	char* wav_name;
	unsigned long inner_range;
	unsigned long outer_range;
	int max_volume;
	int	pitch;
	int flags;
	int index;	

};

///////////////////////////////////////////////


struct AltObjectLocation 
{
	ChunkVectorInt position;
	ChunkQuat orientation;
	int spare1,spare2;
};


//chunk for storing list of possible alternate locations for an object
//use for placed objects and generators and possibly sounds
class Object_Alternate_Locations_Chunk : public Chunk
{
public :
	Object_Alternate_Locations_Chunk(Chunk_With_Children* parent,const char* data, size_t);	
	Object_Alternate_Locations_Chunk(Chunk_With_Children* parent);
	~Object_Alternate_Locations_Chunk();

	void fill_data_block(char* data);
	size_t size_chunk();
	
	int num_locations;
	AltObjectLocation* locations;

	int group; //objects with the same group (other than 0) share the same die roll

	int spare2;
};

#endif
