#ifndef _included_enumchnk_hpp_
#define _included_enumchnk_hpp_

#include "chunk.hpp"
#include "mishchnk.hpp"

class Enum_Header_Chunk;

class Enum_Chunk : public Lockable_Chunk_With_Children
{
public:

	// empty constructor
	Enum_Chunk (Chunk_With_Children * parent);

	// constructor from buffer
	Enum_Chunk (Chunk_With_Children * const parent,const char *, size_t const);

	Enum_Header_Chunk * get_header();

	// functions for the locking functionality

	BOOL file_equals (HANDLE &);
	const char * get_head_id();
	void set_lock_user(char *);
	
	void post_input_processing();

private:

	friend class File_Chunk;
	friend class GodFather_Chunk;




};

///////////////////////////////////////////////

class Enum_Header_Chunk : public Chunk
{
public:
	// constructor from buffer
	Enum_Header_Chunk (Chunk_With_Children * parent, const char * pdata, size_t psize);

	virtual size_t size_chunk ()
	{
		chunk_size = 36;
		return chunk_size;
	}

	virtual BOOL output_chunk (HANDLE &);

	virtual void fill_data_block (char * data_start);

	void prepare_for_output();

private:

	friend class Enum_Chunk;
	friend class File_Chunk;

	int flags;

	int version_no;

	char lock_user[17];
	
	
	// constructor from parent
	Enum_Header_Chunk (Enum_Chunk * parent)
	: Chunk (parent, "ENUMHEAD"),
	flags (0), version_no (0)
	{}
	
};
	



#endif // _included_enumchnk_hpp_
