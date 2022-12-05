#ifndef _included_ffread_hpp_
#define _included_ffread_hpp_

#ifndef __cplusplus
#error "ffread.hpp requires C++ compilation"
#endif

#include "fixer.h"

/*

Fastfile format:

1. Identifier, num_files
2. Header - contains information about files and offsets
3. Data - concatenated files

1.1 DWORD: RFFL - identifier
1.2 DWORD: version 0
1.3 DWORD: num_files
1.4 DWORD: num bytes of header
1.5 DWORD: num bytes of data

2.1 DWORD: Offset
2.2 DWORD: Length
2.3 STRING: Filename - null term then to 4 bytes

3.1 DATA: padded to 4 bytes per file

*/

#include <stddef.h>
#include "list_tem.hpp"

enum FFError
{
	FF_OK,
	FF_COULDNOTOPENFILE,
	FF_COULDNOTREADFILE,
	FF_COULDNOTWRITEFILE,
	FF_INVALIDPARMS
};

void ReportError(char const * mesg1, char const * mesg2 = 0);

class FFDataI
{
private:
	char * filename;
	void * data;
	size_t length;
	
public:
	FFDataI(char const *_filename = 0, void *_data = 0, size_t _length = 0);
	FFDataI(FFDataI const &, ptrdiff_t offset = 0);
	FFDataI & operator = (FFDataI const &);
	~FFDataI();
	
	int operator == (FFDataI const &) const;
	int operator != (FFDataI const & ffd) const
		{ return ! operator == (ffd); }
	
	int operator == (char const * name) const;
	int operator != (char const * name) const
		{ return ! operator == (name); }
	
	int operator < (FFDataI const &) const;
	int operator < (char const * name) const;
	
	inline void * GetDataPointer() const
		{ return data; }
	inline size_t GetLength() const
		{ return length; }
};

#define FFHI_HASHTABLESIZE 256

class FFHeaderI
{
private:
	char * filename;
	void * data;
	size_t length;
	List<FFDataI> files [FFHI_HASHTABLESIZE]; // hash table
	
	static int HashFunction(char const * nam);

	BOOL should_be_kept;

public:
	FFHeaderI(char const *_filename = 0,BOOL _should_be_kept=FALSE);
	FFHeaderI(FFHeaderI const &);
	FFHeaderI & operator = (FFHeaderI const &);
	~FFHeaderI();
	
	int operator == (FFHeaderI const &) const;
	int operator != (FFHeaderI const & ffh) const
		{ return ! operator == (ffh); }
	
	FFError Read(char const *_filename = 0);
	
	void const * FindFile(char const * name, size_t * lengthP) const;
	//mark files that should kept when using ffclose_almost_all with a capital letter
	//should probably be done with an extra entry in the info text file, but I want to mantain
	//compatibility with the predator demo
	BOOL ShouldBeKept(){return should_be_kept;}
	
	void Clear();
};

#endif // ! _included_ffread_hpp_
