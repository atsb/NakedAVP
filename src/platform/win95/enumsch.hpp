#ifndef _included_enumsch_hpp_
#define _included_enumsch_hpp_

#include "chunk.hpp"

class Enum_Constant
{
public:
	char * cname;
	int value;
	int reserved;

	Enum_Constant() : cname(0), value(0), reserved(0) {}

	~Enum_Constant()
	{
		if (cname) delete[] cname;
	}

	Enum_Constant(char const * const _cname, int const _value);
	Enum_Constant(Enum_Constant const & ec2);

	Enum_Constant & operator = (Enum_Constant const & ec2);

	BOOL operator == (Enum_Constant const & ec2) const;
	BOOL operator != (Enum_Constant const & ec2) const;
	BOOL operator < (Enum_Constant const & ec2) const;

private:

	friend class BMP_Enums_Chunk;

	// constructor from buffer
	Enum_Constant(char const * sdata);

	size_t size_chunk() const;

	void fill_data_block(char * data_start);
};


class Enum_Const_List
{
public:
	List<Enum_Constant> enums;

	virtual ~Enum_Const_List(){}
};


class BMP_Enums_Chunk : public Chunk, public Enum_Const_List
{
public:
	// constructor from buffer
	BMP_Enums_Chunk (Chunk_With_Children * const parent, char const * sdata, size_t const ssize);

	~BMP_Enums_Chunk ()
	{
		if (ctype) delete[] ctype;
	}

	virtual size_t size_chunk ();
	virtual void fill_data_block (char * data_start);

	char * ctype;
	int reserved1;
	int reserved2;
	// List<Enum_Constant> enums;

private:

	friend class Enum_Chunk;

};


#endif // !_included_enumsch_hpp_
