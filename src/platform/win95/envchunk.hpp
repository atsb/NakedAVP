#ifndef _envchunk_hpp
#define _envchunk_hpp 1

#include "chunk.hpp"
#include "chnktype.hpp"
#include "mishchnk.hpp"


class Environment_Data_Header_Chunk;

class Environment_Data_Chunk : public Lockable_Chunk_With_Children
{
public:

	// empty constructor
	Environment_Data_Chunk (Chunk_With_Children * parent);
	// constructor from buffer
	Environment_Data_Chunk (Chunk_With_Children * const parent,const char *, size_t const);

	Environment_Data_Header_Chunk * get_header();

	// functions for the locking functionality

	BOOL file_equals (HANDLE &);
	const char * get_head_id();
	void set_lock_user(char *);
	
	void post_input_processing();

private:

	friend class File_Chunk;
	friend class GodFather_Chunk;
	friend class RIF_File_Chunk;




};

///////////////////////////////////////////////

class Environment_Data_Header_Chunk : public Chunk
{
public:
	// constructor from buffer
	Environment_Data_Header_Chunk (Chunk_With_Children * parent, const char * pdata, size_t psize);

	virtual size_t size_chunk ()
	{
		chunk_size = 36;
		return chunk_size;
	}

	virtual BOOL output_chunk (HANDLE &);

	virtual void fill_data_block (char * data_start);

	void prepare_for_output();

private:

	friend class Environment_Data_Chunk;
	friend class File_Chunk;

	int flags;

	int version_no;

	char lock_user[17];
	
	
	// constructor from parent
	Environment_Data_Header_Chunk (Environment_Data_Chunk * parent)
	: Chunk (parent, "ENDTHEAD"),
	flags (0), version_no (0)
	{}
	
};
	
///////////////////////////////////////////////

class Environment_Scale_Chunk : public Chunk
{

public:

	// constructor from data
	Environment_Scale_Chunk (Environment_Data_Chunk * parent, double _scale)
	: Chunk (parent, "ENVSDSCL"),
	scale (_scale)
	{}
	
	// constructor from buffer
	Environment_Scale_Chunk (Chunk_With_Children * parent, const char * sdata, size_t /*ssize*/)
	: Chunk (parent, "ENVSDSCL"),
	scale ( *((double *) sdata) )
	{}
	
	virtual size_t size_chunk ()
	{
		chunk_size = 20;
		return chunk_size;
	}

	virtual void fill_data_block (char * data_start);
	
	const double scale;

private:

	friend class Environment_Data_Chunk;

	
	
	
};

///////////////////////////////////////////////

class Special_Objects_Chunk : public Chunk_With_Children
{

public:

	Special_Objects_Chunk(Chunk_With_Children * parent)
	: Chunk_With_Children (parent, "SPECLOBJ")
	{}
	// constructor from buffer
	Special_Objects_Chunk (Chunk_With_Children * const parent,const char *, size_t const);
private:

	friend class Environment_Data_Chunk;
	friend class AVP_Generator_Chunk;



};

///////////////////////////////////////////////
class Environment_Acoustics_Chunk : public Chunk
{
public:
	Environment_Acoustics_Chunk(Environment_Data_Chunk * parent);
	Environment_Acoustics_Chunk(Chunk_With_Children* parent, const char * data, size_t );	
	virtual void fill_data_block(char*);
	virtual size_t size_chunk()
	{return chunk_size=24;}

	int env_index; 
	float reverb;
	int spare;
private:
	friend class Environment_Data_Chunk;
	


};
///////////////////////////////////////////////
class Sound_Directory_Chunk : public Chunk
{
public :
	Sound_Directory_Chunk(Environment_Data_Chunk* parent,const char* dir);
	~Sound_Directory_Chunk();
	// constructor from buffer
	Sound_Directory_Chunk (Chunk_With_Children * const parent,const char *, size_t const);
	char* directory;

	size_t size_chunk()
	{
		return (chunk_size =12 + ((strlen (directory)+4)&~3));
	}

	void fill_data_block(char* data_start);

private:

	friend class Environment_Data_Chunk;



};

/////////////////////Available shape set collections////////////////////////////////////////
class Random_Texture_ID_Chunk : public Chunk
{
public :
	Random_Texture_ID_Chunk(Chunk_With_Children* parent,const char* _name);
	Random_Texture_ID_Chunk(Chunk_With_Children* parent,const char* data, size_t);
	~Random_Texture_ID_Chunk();

	void fill_data_block(char* data);
	size_t size_chunk();

	char* name;
	List<int> random_types;
	
	int spare1,spare2;

};
#endif
