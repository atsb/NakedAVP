#ifndef _hierchnk_hpp
#define _hierchnk_hpp 1

#include "chunk.hpp"
#include "list_tem.hpp"
#include "chnktype.hpp"

class Object_Chunk;
class Object_Hierarchy_Data_Chunk;
class Object_Hierarchy_Name_Chunk;

class Shape_Chunk;

#ifdef MAXEXPORT
	class INode;
#endif


class Object_Hierarchy_Chunk : public Chunk_With_Children
{
public:

	Object_Hierarchy_Chunk(Chunk_With_Children * parent)
	: Chunk_With_Children (parent, "OBJCHIER")
	{}
	// constructor from buffer
	Object_Hierarchy_Chunk (Chunk_With_Children * const parent,const char *, size_t const);
	
	List <Object_Hierarchy_Chunk *> list_h_children();
	
	Object_Hierarchy_Data_Chunk * get_data ();
	Object_Hierarchy_Name_Chunk * get_name ();


#ifdef MAXEXPORT
	INode* node;
#endif


};

class Object_Hierarchy_Data_Chunk : public Chunk
{
public:

	Object_Hierarchy_Data_Chunk (Object_Hierarchy_Chunk * parent, const char * obname);
	Object_Hierarchy_Data_Chunk (Chunk_With_Children * parent, const char * sdata, size_t /*ssize*/);
	
	~Object_Hierarchy_Data_Chunk ();

	int num_extra_data;
	int* extra_data;
	
	Object_Chunk * const object;

	virtual size_t size_chunk ();

	virtual void fill_data_block (char * data_start);

	virtual void post_input_processing();

	void find_object_for_this_section(); //find the object_chunk from the name
	
	char * ob_name;

private:

	friend class Object_Hierarchy_Chunk;

	
};


// This chunk will normally go in the root as a way of identifiying the hierarchy
class Object_Hierarchy_Name_Chunk : public Chunk 
{
public:

	Object_Hierarchy_Name_Chunk (Object_Hierarchy_Chunk * parent, const char * hname);
	Object_Hierarchy_Name_Chunk (Chunk_With_Children * parent, const char * sdata, size_t /*ssize*/);
	~Object_Hierarchy_Name_Chunk();
	
	char * hierarchy_name;
	
	virtual size_t size_chunk ()
	{
		return(chunk_size = 12 + strlen (hierarchy_name) + 4 - strlen (hierarchy_name)%4);
	}

	virtual void fill_data_block (char * data_start);
	
private:

	friend class Object_Hierarchy_Chunk;

	
};


struct Replaced_Shape_Details
{
	~Replaced_Shape_Details()
	{
		if(old_object_name) delete[] old_object_name;
		if(new_object_name) delete[] new_object_name;
	}
	
	//object name of shape to be replaced
	char* old_object_name;
	//object that replaces it.	
	char* new_object_name;
};


#define Avp_ShapeSet_Flag_Female 0x00000001

#define List_Object_Hierarchy_Alternate_Shape_Set_Chunk(parent,list) (parent)->lookup_child("OBHALTSH",list)
class Object_Hierarchy_Alternate_Shape_Set_Chunk : public Chunk
{
public :
	Object_Hierarchy_Alternate_Shape_Set_Chunk(Chunk_With_Children* parent,int num,const char* name);
	Object_Hierarchy_Alternate_Shape_Set_Chunk(Chunk_With_Children* parent,const char* name);
	Object_Hierarchy_Alternate_Shape_Set_Chunk (Chunk_With_Children * parent, const char * data, size_t /*ssize*/);
	~Object_Hierarchy_Alternate_Shape_Set_Chunk();

	virtual size_t size_chunk();
	
	virtual void fill_data_block (char * data_start);

	char* Shape_Set_Name;
	int Shape_Set_Num;

	List<Replaced_Shape_Details*> Replaced_Shape_List; 

	int flags;
	int spare[3];
	
private:


};


#define AvP_HColl_Flag_NotRandom 0x00000001

#define List_Hierarchy_Shape_Set_Collection_Chunk(parent,list) (parent)->lookup_child("HSETCOLL",list)
//this chunk hold a list of indeces for shape_set_chunks that should be applied
class Hierarchy_Shape_Set_Collection_Chunk : public Chunk
{
public :
	Hierarchy_Shape_Set_Collection_Chunk(Chunk_With_Children* parent,int num,const char* name);
	Hierarchy_Shape_Set_Collection_Chunk (Chunk_With_Children * parent, const char * data, size_t /*ssize*/);
	~Hierarchy_Shape_Set_Collection_Chunk();

	virtual size_t size_chunk();
	
	virtual void fill_data_block (char * data_start);

	char* Set_Collection_Name;
	int Set_Collection_Num;

	List<int> Index_List; 

	int TypeIndex;
	int flags;
};

class Hierarchy_Degradation_Distance_Chunk : public Chunk
{
public :
	Hierarchy_Degradation_Distance_Chunk(Chunk_With_Children * parent, const char * data, size_t /*ssize*/);
	Hierarchy_Degradation_Distance_Chunk(Chunk_With_Children* parent,int _num_detail_levels);
	~Hierarchy_Degradation_Distance_Chunk();

	virtual size_t size_chunk();
	
	virtual void fill_data_block (char * data_start);

	int num_detail_levels;
	int* distance_array;

	
private:
};

class Hierarchy_Bounding_Box_Chunk :public Chunk
{
public :
	Hierarchy_Bounding_Box_Chunk(Chunk_With_Children * parent,const char*data,size_t datasize);
	
	Hierarchy_Bounding_Box_Chunk(Chunk_With_Children * parent)
	:Chunk(parent,"HIERBBOX")
	{}

	virtual void fill_data_block (char * data_start);

	virtual size_t size_chunk()
	{return chunk_size=12+6*4;}

	ChunkVectorInt min;
	ChunkVectorInt max;
};
#endif
