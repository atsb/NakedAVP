#include <string.h>

#include "enumsch.hpp"

//macro for helping to force inclusion of chunks when using libraries
FORCE_CHUNK_INCLUDE_IMPLEMENT(enumsch)

Enum_Constant::Enum_Constant(char const * const _cname, int const _value) : cname(0), value(_value), reserved(0)
{
	if (_cname)
	{
		cname = new char[strlen(_cname)+1];
		strcpy(cname,_cname);
	}
}

Enum_Constant::Enum_Constant(Enum_Constant const & ec2) : cname(0), value(ec2.value), reserved(ec2.reserved)
{
	if (ec2.cname)
	{
		cname = new char[strlen(ec2.cname)+1];
		strcpy(cname,ec2.cname);
	}
}

Enum_Constant & Enum_Constant::operator = (Enum_Constant const & ec2)
{
	if (cname) delete[] cname;
	cname = 0;
	value = ec2.value;
	reserved = ec2.reserved;
	if (ec2.cname)
	{
		cname = new char[strlen(ec2.cname)+1];
		strcpy(cname,ec2.cname);
	}

	return *this;
}

BOOL Enum_Constant::operator == (Enum_Constant const & ec2) const
{
	if (cname && ec2.cname)
		if (!strcmp(cname,ec2.cname)) return TRUE;
	return FALSE;
}

BOOL Enum_Constant::operator != (Enum_Constant const & ec2) const
{
	if (cname && ec2.cname)
		if (!strcmp(cname,ec2.cname)) return FALSE;
	return TRUE;
}

BOOL Enum_Constant::operator < (Enum_Constant const & ec2) const
{
	if (cname && ec2.cname)
		if (strcmp(cname,ec2.cname)<0) return TRUE;
	return FALSE;
}


Enum_Constant::Enum_Constant(char const * sdata)
: cname(0), value(*(int *)sdata), reserved(*(int *)(sdata+4))
{
	sdata+=8;
	if (*sdata)
	{
		cname = new char[strlen(sdata)+1];
		strcpy(cname,sdata);
	}
}

size_t Enum_Constant::size_chunk() const
{
	return 8 + ((cname ? strlen(cname)+1 : 1) +3 &~3);
}

void Enum_Constant::fill_data_block (char * data_start)
{
	*(int*)data_start = value;
	*(int*)(data_start+4) = reserved;
	
	data_start += 8;

	strcpy(data_start,cname ? cname : "");
}


///////
RIF_IMPLEMENT_DYNCREATE("BMPENUMS",BMP_Enums_Chunk)

BMP_Enums_Chunk::BMP_Enums_Chunk(Chunk_With_Children * const parent, char const * sdata, size_t const /*ssize*/)
: Chunk(parent,"BMPENUMS"), ctype(0), reserved1(*(int *)sdata), reserved2(*(int *)(sdata+4))
{
	sdata+=8;
	unsigned int const len = strlen(sdata)+1;

	if (len>1)
	{
		ctype = new char[len];
		strcpy(ctype,sdata);
	}

	sdata += len + 3 &~3;

	unsigned int const enlistsize = *(int *)sdata;
	sdata += 4;

	for (unsigned int i = enlistsize; i; --i)
	{
		Enum_Constant current(sdata);
		sdata += current.size_chunk();
		enums.add_entry(current);
	}
}

size_t BMP_Enums_Chunk::size_chunk ()
{
	chunk_size = 12 + 8 + (ctype ? strlen(ctype)+1 : 1) + 4 +3 &~3;

	for (LIF<Enum_Constant> li(&enums); !li.done(); li.next())
	{
		chunk_size += li().size_chunk();
	}
	return chunk_size;
}

void BMP_Enums_Chunk::fill_data_block (char * data_start)
{
	strncpy (data_start, identifier, 8);
	data_start += 8;

	*(int *) data_start = chunk_size;
	data_start += 4;

	*(int*)data_start = reserved1;
	*(int*)(data_start+4) = reserved2;
	data_start += 8;

	strcpy(data_start,ctype ? ctype : "");
	data_start += strlen(data_start)+1 +3 &~3;

	*(int *)data_start = enums.size();
	data_start += 4;

	for (LIF<Enum_Constant> li(&enums); !li.done(); li.next())
	{
		Enum_Constant current(li());

		current.fill_data_block(data_start);
		data_start += current.size_chunk();
	}

}

