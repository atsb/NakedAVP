#include <string.h>

#include "chunk.hpp"

#define UseLocalAssert No
#include "ourasert.h"
#define assert(x) GLOBALASSERT(x)

#define twprintf printf

char * users_name = "Player";

#include "hash_tem.hpp"
Chunk * Parent_File;

// Non class functions ( only one as yet )

void list_chunks_in_file(List<int> * pList, HANDLE hand, char const * chunk_id)
{
	unsigned long bytes_read;

	char buffer[8];
	BOOL ok = TRUE;
	
	while (pList->size())
		pList->delete_first_entry();

	// assuming we start at the front of a parent chunk, 
	// containing the child chunk specified

	int init_file_pos = SetFilePointer (hand,0,0,FILE_CURRENT);
	int file_pos;
	int file_length = GetFileSize(hand, 0);

	SetFilePointer (hand,8,0,FILE_CURRENT);

	int chunk_length;
	int sub_chunk_ln;

	ReadFile (hand, (long *) &chunk_length, 4, &bytes_read, 0);

	if ((init_file_pos + chunk_length) > file_length) return;

	while ((file_pos = SetFilePointer (hand,0,0,FILE_CURRENT))
					< (init_file_pos + chunk_length)  && ok) {

		ok = ReadFile (hand, (long *) buffer, 8, &bytes_read, 0);
		if (strncmp(buffer, chunk_id, 8) == 0)
			pList->add_entry(file_pos);

		ok = ReadFile (hand, (long *) &sub_chunk_ln, 4, &bytes_read, 0);

		SetFilePointer (hand,sub_chunk_ln-12,0,FILE_CURRENT);
	}
}

#ifndef RIFF_OPTIMIZE
List<int> list_chunks_in_file (HANDLE & hand, const char * chunk_id)
{

	List<int> chunk_list;

	list_chunks_in_file(&chunk_list, hand, chunk_id);

	return chunk_list;
}
#endif

//////////////////////////////////////////////


// Class Chunk functions

Chunk::~Chunk()
{
	if (parent) {
		if (parent->children == this) {
			parent->children = next;
			if (next)
				next->previous = previous;
		}
		else {
			if (previous)
				previous->next = next;
			if (next)
				next->previous = previous;
		}
	}
}

Chunk::Chunk(Chunk_With_Children * _parent, const char * _identifier)
: error_code(0)
{
	strncpy (identifier_store, _identifier, 8);
	identifier_store[8] = 0;
	identifier = identifier_store;
	parent = _parent;
	next = NULL;
	previous = NULL;

	if (parent){
		if (parent->children) {
			Chunk * pTail = parent->children;
			while (pTail->next)
				pTail = pTail->next;
			pTail->next = this;
			previous = pTail;
		}
		else
			parent->children = this;
	}
}

BOOL Chunk::output_chunk (HANDLE & hand)
{
	unsigned long junk;
	BOOL ok;
	char * data_block;

	data_block = make_data_block_from_chunk();

	if (data_block)
	{
		ok = WriteFile (hand, (long *) data_block, (unsigned long) chunk_size, &junk, 0);
		delete [] data_block;

		if (!ok) return FALSE;
	}
	else if (chunk_size)
	{
		return(FALSE);
	}
	return TRUE;
}



char * Chunk::make_data_block_from_chunk ()
{
	char * data_block;
	size_t block_size;

	block_size = size_chunk();

	if (!chunk_size)
	{
		return(0);
	}
	
	data_block = new char [block_size];

	this->fill_data_block(data_block);

	return data_block;
}

char * Chunk::make_data_block_for_process ()
{
	char * data_block;
	size_t block_size;

	block_size = size_chunk_for_process();

	if (!chunk_size)
	{
		return(0);
	}

	data_block = new char [block_size];

	fill_data_block_for_process(data_block);

	return data_block;
}


size_t Chunk::size_chunk_for_process()
{
	return size_chunk();
}


void Chunk::fill_data_block_for_process(char * data_start)
{
	fill_data_block(data_start);
}

Chunk_With_Children * Chunk::GetRootChunk(void)
{
	Chunk * retp = this;

	while (retp->parent) retp = retp->parent;

	return (Chunk_With_Children *) retp;
}

Chunk_With_Children const * Chunk::GetRootChunk(void) const
{
	Chunk const * retp = this;

	while (retp->parent) retp = retp->parent;

	return (Chunk_With_Children const *) retp;
}


///////////////////////////////////////

// Class Miscellaneous_Chunk functions

Miscellaneous_Chunk::Miscellaneous_Chunk (Chunk_With_Children * parent, const char * identifier,
 const char * _data, size_t _data_size) 
: Chunk (parent, identifier), data_size (_data_size), data(NULL) 
{
	if (data_size)
	{
		data_store = new char [data_size];

		*((char **) &data) = data_store;

		for (int i = 0; i < (signed)data_size; i++)
			data_store[i] = _data[i];
	}
	else
	{
		data_store = NULL;
	}

}

void Miscellaneous_Chunk::fill_data_block (char * data_start)
{
	strncpy (data_start, identifier, 8);

	data_start += 8;

	*((int *) data_start) = chunk_size;

	data_start += 4;

	for (int i = 0; i<(signed)(chunk_size-12); i++)
		data_start[i] = data[i];
}

Miscellaneous_Chunk::~Miscellaneous_Chunk ()
{
	delete [] data_store;
}

///////////////////////////////////////

// Class Chunk_With_Children functions

Chunk_With_Children::~Chunk_With_Children()
{
	while (children)
		delete children;
}

size_t Chunk_With_Children::size_chunk ()
{
	Chunk * child_ptr = children;
	
	chunk_size = 12; // identifier + length

	if (children) 
		while (child_ptr != NULL) {
			chunk_size += child_ptr->size_chunk();
			child_ptr = child_ptr->next;
		}

	return chunk_size;

}

BOOL Chunk_With_Children::output_chunk (HANDLE &hand)
{
	unsigned long junk;
	Chunk * child_ptr = children;
	BOOL ok;

	ok = WriteFile (hand, (long *) identifier, 8, &junk, 0);

	if (!ok) return FALSE;

	ok = WriteFile (hand, (long *) &chunk_size, 4, &junk, 0);

	if (!ok) return FALSE;
	
	if (children) 
		while (child_ptr != NULL && ok){
			ok = child_ptr->output_chunk(hand);
			child_ptr = child_ptr->next;
		}

	if (!ok) return FALSE;

	return TRUE;


}


size_t Chunk_With_Children::size_chunk_for_process()
{
	Chunk * child_ptr = children;
	
	chunk_size = 12; // identifier + length

	if (children) 
		while (child_ptr != NULL) {
			chunk_size += child_ptr->size_chunk_for_process();
			child_ptr = child_ptr->next;
		}

	return chunk_size;

}


void Chunk_With_Children::fill_data_block_for_process(char * data_start)
{
	strncpy (data_start, identifier, 8);

	data_start += 8;

	*((int *) data_start) = chunk_size;

	data_start += 4;

	Chunk * child_ptr = children;

	if (children)
		while (child_ptr != NULL) {
			child_ptr->fill_data_block_for_process (data_start);
			data_start += child_ptr->chunk_size;
			child_ptr = child_ptr->next;
		}

}



void Chunk_With_Children::fill_data_block(char * data_start)
{
	strncpy (data_start, identifier, 8);

	data_start += 8;

	*((int *) data_start) = chunk_size;

	data_start += 4;

	Chunk * child_ptr = children;

	if (children)
		while (child_ptr != NULL) {
			child_ptr->fill_data_block (data_start);
			data_start += child_ptr->chunk_size;
			child_ptr = child_ptr->next;
		}

}

#ifndef RIFF_OPTIMIZE
List<Chunk *> Chunk_With_Children::lookup_child (const char * class_ident) const
{
	List<Chunk *> child_list;
	
	lookup_child(class_ident,child_list);
	
	return child_list;
}
#endif

void Chunk_With_Children::lookup_child (const char * class_ident,List<Chunk*>& child_list) const
{
	//make sure the list is empty first
	while(child_list.size())
	{
		child_list.delete_first_entry();
	}

	Chunk * child_ptr = children;

	if (children)	
		while	(child_ptr != NULL) {
			if (strncmp (class_ident, child_ptr->identifier, 8) == 0)
			{
				assert (!child_ptr->r_u_miscellaneous());
				child_list.add_entry(child_ptr);
			}
			child_ptr = child_ptr->next;
		}

	
}

unsigned Chunk_With_Children::count_children (char const * class_ident) const
{
	unsigned nChildren = 0;
	
	Chunk * child_ptr = children;

	if (children)	
		while	(child_ptr != NULL) {
			if (strncmp (class_ident, child_ptr->identifier, 8) == 0)
			{
				assert (!child_ptr->r_u_miscellaneous());
				++ nChildren;
			}
			child_ptr = child_ptr->next;
		}

	return nChildren;
}

Chunk* Chunk_With_Children::lookup_single_child (const char * class_ident) const
{
	#if debug
	//if debug make sure there is at most one of the required chunk type
	Chunk * child_ptr = children;
	Chunk * chunk_found=0;
	if (children)	
		while	(child_ptr != NULL) {
			if (strncmp (class_ident, child_ptr->identifier, 8) == 0)
			{
				assert (!child_ptr->r_u_miscellaneous());
				assert(!chunk_found); 
				chunk_found=child_ptr;
			}
			child_ptr = child_ptr->next;
		}
	return chunk_found;
	#else
	Chunk * child_ptr = children;

	if (children)	
		while	(child_ptr != NULL) {
			if (strncmp (class_ident, child_ptr->identifier, 8) == NULL)
			{
				assert (!child_ptr->r_u_miscellaneous());
				return child_ptr;
			}
			child_ptr = child_ptr->next;
		}
	return 0;
	#endif
	

	
}

void Chunk_With_Children::prepare_for_output()
{
	Chunk * child_ptr = children;
	if (children) 
		while (child_ptr != NULL){
			child_ptr->prepare_for_output ();
			child_ptr = child_ptr->next;
		}
}	

void Chunk_With_Children::post_input_processing()
{
	Chunk * child_ptr = children;
	if (children) 
		while (child_ptr != NULL){
			child_ptr->post_input_processing ();
			child_ptr = child_ptr->next;
		}
}	

///////////////////////////////////////////////////
//Chunk registering stuff


class RifRegEntry
{
	public:
		int chunk_id_1;
		int chunk_id_2;
		int parent_id_1;
		int parent_id_2;
		Chunk * (* m_pfnCreate) (Chunk_With_Children* parent,const char* data);
		
		
		/*
			For two members of this class to be considered similar there chunk_is members must match.
			If both parent_id members are zero for one of the objects being compared , then they are considered
			equal regardless of the parent_id members of the other object.
			Otherwise the parent_id members of the two objects must match.
			(This is done because not all of the constructors insist on a given parent)
		*/
		inline bool operator == (RifRegEntry const & rEntry) const
		{
			if(chunk_id_1 != rEntry.chunk_id_1 || chunk_id_2 != rEntry.chunk_id_2) return FALSE;
			if(parent_id_1 == 0 && parent_id_2 == 0) return TRUE;
			if(rEntry.parent_id_1 == 0 && rEntry.parent_id_2 == 0) return TRUE;
			return  parent_id_1 == rEntry.parent_id_1 && parent_id_2 == rEntry.parent_id_2;	
		}
		inline bool operator != (RifRegEntry const & rEntry) const
		{
			return ! operator == (rEntry);
		}
};

inline unsigned HashFunction(RifRegEntry const & rEntry)
{
	return HashFunction(rEntry.chunk_id_1 + rEntry.chunk_id_2);
}

static HashTable<RifRegEntry> * g_pRifRegister = NULL;
	
void Chunk::Register(const char* idChunk,const char* idParent, Chunk * (* pfnCreate) (Chunk_With_Children* parent,const char* data) )
{
	static HashTable<RifRegEntry> reg;
	char temp_id[8];
	
	g_pRifRegister = &reg;
	
	RifRegEntry entry;

	strncpy(temp_id,idChunk,8);
	entry.chunk_id_1 = *(int*) &temp_id[0];
	entry.chunk_id_2 = *(int*) &temp_id[4];
	entry.m_pfnCreate = pfnCreate;

	if(idParent)
	{
		strncpy(temp_id,idParent,8);
		entry.parent_id_1 = *(int*) &temp_id[0];
		entry.parent_id_2 = *(int*) &temp_id[4];
	}
	else
	{
		entry.parent_id_1 = 0;
		entry.parent_id_2 = 0;
	}
	
	reg.AddAsserted(entry);
}

Chunk* Chunk_With_Children::DynCreate(const char* data)
{
	/*
	Look in hash tables for a constructor for this block
	If none exists , create a Miscellaneous_Chunk
	*/
	if (g_pRifRegister)
	{
		RifRegEntry test;
		test.chunk_id_1 = *(int*) data;
		test.chunk_id_2 = *(int*) &data[4];
		test.parent_id_1 = *(int*) identifier;
		test.parent_id_2 = *(int*) &identifier[4];
		test.m_pfnCreate = NULL;
		
		RifRegEntry const * pEntry = g_pRifRegister->Contains(test);
		if (pEntry)
		{
			return pEntry->m_pfnCreate(this,data);
		}
	}
	return new Miscellaneous_Chunk(this,data,(data + 12), (*(int *) (data + 8))-12);
}
