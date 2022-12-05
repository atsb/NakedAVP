#include "enumchnk.hpp"
//#include "enumsch.hpp"

//macro for helping to force inclusion of chunks when using libraries
FORCE_CHUNK_INCLUDE_IMPLEMENT(enumchnk)

// Class Enum_Chunk functions

RIF_IMPLEMENT_DYNCREATE("REBENUMS",Enum_Chunk)
// constructor from buffer
LOCKABLE_CHUNK_WITH_CHILDREN_LOADER("REBENUMS",Enum_Chunk)
/*
Children for Enum_Chunk :

"ENUMHEAD"	Enum_Header_Chunk
"BMPENUMS"	BMP_Enums_Chunk
*/



// empty constructor
Enum_Chunk::Enum_Chunk (Chunk_With_Children  * parent)
:Lockable_Chunk_With_Children (parent, "REBENUMS")
{
	// as necessary, generated automatically
	new Enum_Header_Chunk (this);
}


BOOL Enum_Chunk::file_equals (HANDLE & /*rif_file*/)
{
	return(TRUE);
}	

Enum_Header_Chunk * Enum_Chunk::get_header()
{
	
	return (Enum_Header_Chunk *) this->lookup_single_child ("ENUMHEAD");
}	

const char * Enum_Chunk::get_head_id()
{
	Enum_Header_Chunk * hdptr = get_header();

	if (!hdptr) return (0);

	return(hdptr->identifier);
	
}	

void Enum_Chunk::set_lock_user (char * user)
{
	Enum_Header_Chunk * hdptr = get_header();

	if (!hdptr) return;

	strncpy (hdptr->lock_user, user,16);

	hdptr->lock_user[16] = 0;
}
	
void Enum_Chunk::post_input_processing()
{
	if (get_header())
		if (get_header()->flags & GENERAL_FLAG_LOCKED)
			external_lock = TRUE;

	Chunk_With_Children::post_input_processing();

}

///////////////////////////////////////

// Class Enum_Header_Chunk functions
RIF_IMPLEMENT_DYNCREATE("ENUMHEAD",Enum_Header_Chunk)

// from buffer
Enum_Header_Chunk::Enum_Header_Chunk (Chunk_With_Children * parent, const char * hdata, size_t /*hsize*/)
	: Chunk (parent, "ENUMHEAD"),
	flags (0), version_no (0)
{
	flags = *((int *) hdata);

	strncpy (lock_user, (hdata + 4), 16);
	lock_user[16] = '\0';

	version_no = *((int *) (hdata + 20));
}	

BOOL Enum_Header_Chunk::output_chunk (HANDLE & hand)
{
	unsigned long junk;
	BOOL ok;
	char * data_block;

	data_block = make_data_block_from_chunk();

	ok = WriteFile (hand, (long *) data_block, (unsigned long) chunk_size, &junk, 0);

	delete [] data_block;

	if (!ok) return FALSE;

	return TRUE;
}

void Enum_Header_Chunk::fill_data_block ( char * data_start)
{
	strncpy (data_start, identifier, 8);

	data_start += 8;

	*((int *) data_start) = chunk_size;

	data_start += 4;

	*((int *) data_start) = flags;
	strncpy ((data_start + 4), lock_user, 16);

	*((int *) (data_start+20)) = version_no;
}

void Enum_Header_Chunk::prepare_for_output()
{
	version_no ++;
}

