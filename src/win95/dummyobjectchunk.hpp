
#ifndef _DummyObjectChunk_hpp
#define _DummyObjectChunk_hpp 1

#include "chunk.hpp"
#include "list_tem.hpp"
#include "chnktype.hpp"

class Dummy_Object_Data_Chunk;

class Dummy_Object_Chunk : public Chunk_With_Children
{
public:

	Dummy_Object_Chunk(Chunk_With_Children* parent,const char* _name ,ChunkVectorInt& _location,ChunkVectorInt& min ,ChunkVectorInt& max ,ChunkQuat& orient);

	// constructor from buffer
	Dummy_Object_Chunk (Chunk_With_Children * const parent,const char *, size_t const);

	Dummy_Object_Data_Chunk * get_data_chunk();//gets data chunk (name and location)

	const char* get_text(); //get text attached to a dummy object
	void set_text(const char* text); //change the text attached to a dummy object
	
};

//chunk containing  name and location of dummy object
class  Dummy_Object_Data_Chunk : public Chunk
{
public :
	Dummy_Object_Data_Chunk(Dummy_Object_Chunk* parent,const char* _name ,ChunkVectorInt& _location,ChunkVectorInt& min ,ChunkVectorInt& max ,ChunkQuat& orient);
	Dummy_Object_Data_Chunk (Chunk_With_Children * parent, const char * data, size_t );
	~Dummy_Object_Data_Chunk();


/*------------------------**
** Main dummy object data **
**------------------------*/
	char* name;
	
	ChunkVectorInt location;
	ChunkQuat orientation;
	
	ChunkVectorInt min_extents;
	ChunkVectorInt max_extents;
/*------------------------**
** Main dummy object data **
**------------------------*/

	size_t size_chunk();
	void fill_data_block (char * data);

private :

	friend class Dummy_Object_Chunk;

};


//contains the 'user text' from 3dsmax
class Dummy_Object_Text_Chunk : public Chunk
{
public :
	Dummy_Object_Text_Chunk(Dummy_Object_Chunk* parent,const char* _text);
	Dummy_Object_Text_Chunk(Chunk_With_Children * parent, const char * data, size_t );
	~Dummy_Object_Text_Chunk();

	size_t size_chunk();
	void fill_data_block (char * data);


	const char* get_text() {return text;}
	void set_text(const char* _text);

private :
	char* text;

};


#endif
