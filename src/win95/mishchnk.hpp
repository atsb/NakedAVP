#ifndef _miscchunk_hpp
#define _miscchunk_hpp 1

#include <time.h>
#include "chunk.hpp"

#include "chnktype.hpp"

#define UseLocalAssert No
#include "ourasert.h"
#define assert(x) GLOBALASSERT(x)

#define twprintf printf

extern char * users_name;

class File_Chunk;

class Lockable_Chunk_With_Children : public Chunk_With_Children
{

public:

	Lockable_Chunk_With_Children (Chunk_With_Children * parent, const char * identifier)
		: Chunk_With_Children (parent, identifier),
		output_chunk_for_process(FALSE),
		updated(FALSE), updated_outside(FALSE), local_lock(FALSE),
		external_lock(FALSE), deleted(FALSE)
	{}


// derived classes from this class must have the 
// following functions (or it won't compile)!!

	virtual BOOL file_equals(HANDLE &) = 0;
	// the file equals function knows to look in
	// the file from the start of the current chunk
	// to see if the current chunk is the same one
	
	virtual const char * get_head_id() = 0;
	virtual void set_lock_user(char *) = 0;

// this function will lock the chunk if it can - 
// will return true on success or if the chunk has 
// already been locked locally.

	BOOL lock_chunk(File_Chunk &);

// The unlock_chunk will unlock the chunk
// if the updateyn flag is set, the updated flag will be set
// internally and the chunk may be updated on the next file update
// If there is no update, the chunk will be unlocked in the file

	BOOL unlock_chunk (File_Chunk &, BOOL updateyn);

	BOOL update_chunk_in_file(HANDLE &rif_file);

	// Selective output functions, 
	// These will output on condition of the flag, the flag will be set
	// to FALSE

	BOOL output_chunk_for_process;
	
	virtual size_t size_chunk_for_process();
	
	virtual void fill_data_block_for_process(char * data_start);	


	BOOL updated;
	BOOL updated_outside;
	BOOL local_lock;
	BOOL external_lock;

	BOOL deleted;
	
};


///////////////////////////////////////////////

class Object_Chunk;
class Shape_Chunk;
class Dummy_Object_Chunk;
class Environment_Data_Chunk;

class File_Chunk : public Chunk_With_Children
{
public:

	//constructor
	File_Chunk (const char * filename);
	File_Chunk ();

	//destructor
	~File_Chunk ();

	// file handling
	BOOL update_file ();
	BOOL write_file (const char *);

	BOOL check_file();

	BOOL update_chunks_from_file();

	// the file_chunk must link all of its shapes & objects together
	// in post_input_processing

	virtual void post_input_processing();

	// copy string when constructed
	// to this variable
	char * filename;


	// some easy access functions
	
	void list_objects(List<Object_Chunk *> * pList);
	void list_shapes(List<Shape_Chunk *> * pList);
	void list_dummy_objects(List<Dummy_Object_Chunk *> * pList);

	Environment_Data_Chunk * get_env_data();

	Object_Chunk* get_object_by_index(int index);
	void assign_index_to_object(Object_Chunk* object);
	
private:

	friend class Shape_Chunk;
	friend class Object_Chunk;
	friend class Lockable_Chunk_With_Children;

	int flags;

	int object_array_size;
	Object_Chunk** object_array;

	void build_object_array();
};

///////////////////////////////////////////////


class RIF_Version_Num_Chunk : public Chunk
{
public:
	RIF_Version_Num_Chunk	(Chunk_With_Children * parent, const char *data, size_t /*size*/)
	:	Chunk (parent,"RIFVERIN"), file_version_no (*((int *) data))
	{}

	int file_version_no;

// output_chunk updates the above	

	virtual void fill_data_block (char * data_start);

	virtual size_t size_chunk () 
	{
		chunk_size = 16;
		return 16;
	}

private:	

	friend class File_Chunk;
	friend class GodFather_Chunk;
	friend class RIF_File_Chunk;
	


	RIF_Version_Num_Chunk	(Chunk_With_Children * parent)
	:	Chunk (parent,"RIFVERIN"), file_version_no (0)
	{}
};

///////////////////////////////////////////////

class RIF_File_Chunk : public Chunk_With_Children
{
public:	

	// This is a special chunk which does not output itself

	RIF_File_Chunk (Chunk_With_Children * parent, const char * fname);
	
	size_t size_chunk()
	{
		return chunk_size = 0;
	}
	
	virtual void post_input_processing();
	
	BOOL output_chunk (HANDLE & /*h*/){return TRUE;};
	void fill_data_block (char * /*c*/){};

	void list_objects(List<Object_Chunk *> * pList);
	void list_shapes(List<Shape_Chunk *> * pList);
	Environment_Data_Chunk * get_env_data();
	
};

///////////////////////////////////////////////

class RIF_Name_Chunk : public Chunk
{
public:

	RIF_Name_Chunk (Chunk_With_Children * parent, const char * rname);
	~RIF_Name_Chunk();
	// constructor from buffer
	RIF_Name_Chunk (Chunk_With_Children * parent, const char * sdata, size_t ssize);
	char * rif_name;

	virtual size_t size_chunk ()
	{
		return (chunk_size = 12 + strlen (rif_name) + 4 - strlen (rif_name)%4);
	}

	virtual void fill_data_block (char * data_start);
	
private:

	friend class Environment_Data_Chunk;
	friend class Environment_Game_Mode_Chunk;
	friend class Shape_External_File_Chunk;
	friend class Sprite_Header_Chunk;
	


};

#endif
