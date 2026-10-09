#ifndef _chnktype_hpp
#define _chnktype_hpp 1

#include "3dc.h"
#include "list_tem.hpp"

struct ChunkVectorInt;
struct ChunkVectorFloat;

struct ChunkVector
{

	double x;
	double y;
	double z;
  	
  	
  	ChunkVector friend operator+(const ChunkVector&, const ChunkVector&);
  	ChunkVector friend operator-(const ChunkVector&, const ChunkVector&);
  	ChunkVector& operator+=(const ChunkVector&);
  	ChunkVector& operator-=(const ChunkVector&);
	
	ChunkVector friend operator*(const ChunkVector&, const double);
 	ChunkVector friend operator/(const ChunkVector&, const double); 
  	
  	ChunkVector friend operator*(const ChunkVector&, const ChunkVector&); //cross prod


	operator VECTORCH () const;
	operator ChunkVectorInt () const;
	operator ChunkVectorFloat () const;

  	friend double dot(const ChunkVector&, const ChunkVector&);//dot product
  	friend double mod(const ChunkVector&);//magnitude of vector
  	int norm(); //normalize
};

struct ChunkVectorInt
{

	int x;
	int y;
	int z;
  	
  	
  	ChunkVectorInt friend operator+(const ChunkVectorInt&, const ChunkVectorInt&);
  	ChunkVectorInt friend operator-(const ChunkVectorInt&, const ChunkVectorInt&);
  	ChunkVectorInt& operator+=(const ChunkVectorInt&);
  	ChunkVectorInt& operator-=(const ChunkVectorInt&);
	
	ChunkVectorInt friend operator*(const ChunkVectorInt&, const double);
 	ChunkVectorInt friend operator/(const ChunkVectorInt&, const double); 
  	
  	//ChunkVectorInt friend operator*(const ChunkVectorInt&, const ChunkVectorInt&); //cross prod
	
	operator VECTORCH () const;

  	//friend double dot(const ChunkVector&, const ChunkVector&);//dot product
  	friend double mod(const ChunkVectorInt&);//magnitude of vector
  	int norm(); //normalize to 65536
};
struct ChunkVectorFloat
{

	float x;
	float y;
	float z;
  	
  	ChunkVectorFloat friend operator+(const ChunkVectorFloat&, const ChunkVectorFloat&);
  	ChunkVectorFloat friend operator-(const ChunkVectorFloat&, const ChunkVectorFloat&);
  	ChunkVectorFloat& operator+=(const ChunkVectorFloat&);
  	ChunkVectorFloat& operator-=(const ChunkVectorFloat&);
	
	ChunkVectorFloat friend operator*(const ChunkVectorFloat&, const double);
 	ChunkVectorFloat friend operator/(const ChunkVectorFloat&, const double); 
	
	//ChunkVectorInt friend operator*(const ChunkVectorInt&, const ChunkVectorInt&); //cross prod

	operator VECTORCH () const;

  	//friend double dot(const ChunkVector&, const ChunkVector&);//dot product
  	friend double mod(const ChunkVectorFloat&);//magnitude of vector
  	int norm(); //normalize to 1
};

struct ChunkUV
{

	float u;
	float v;

};

// in integers I suppose

struct ChunkMatrix
{

	int mat11;
	int mat12;
	int mat13;

	int mat21;
	int mat22;
	int mat23;

	int mat31;
	int mat32;
	int mat33;

};


struct ChunkUV_List
{
	int num_verts;
	ChunkUV vert[4];

	// for list iterator
	friend BOOL operator== (const ChunkUV_List &, const ChunkUV_List &);
	friend BOOL operator!= (const ChunkUV_List &, const ChunkUV_List &);
	
};

class ChunkPoly
{
public:

	int engine_type;
	int normal_index;
	int flags;
	unsigned int colour;

	int num_verts;

	int vert_ind[4];


	//functions for gettings and setting texture and uv indeces in the colour
	unsigned int GetUVIndex();
	unsigned int GetTextureIndex();

	void SetUVIndex(unsigned int uv_index);
	void SetTextureIndex(unsigned int texture_index);

		
};


struct ChunkShape
{
	ChunkShape();
	~ChunkShape();

	ChunkShape (const ChunkShape &);
	ChunkShape& operator=(const ChunkShape &);


	float radius;	 //radius of points about 0,0,0

	ChunkVectorInt max;
	ChunkVectorInt min;

	ChunkVectorInt centre;	//average of min and max
	float radius_about_centre;	

	int num_verts;
	ChunkVectorInt * v_list;

	//int num_vert_normals; //I don't think num_vert_normals is ever used
	ChunkVectorFloat * v_normal_list;

	int num_polys;
	ChunkPoly * poly_list;
	ChunkVectorFloat * p_normal_list;

	int num_uvs;
	ChunkUV_List * uv_list;

	int num_texfiles;
	char ** texture_fns;

	void rescale (double);
};

struct ChunkQuat
{
	float x,y,z,w;
};


struct ObjectID
{
	int id1;
	int id2;
	
	friend BOOL operator== (const ObjectID  &, const ObjectID &);
	friend BOOL operator!= (const ObjectID  &, const ObjectID &);
	friend ObjectID  Minimum(const ObjectID &,const  ObjectID &);
	
};

struct ChunkObject
{
	ChunkVectorInt location;

	ChunkQuat orientation;

	BOOL is_base_object;

	char o_name[50];

	int index_num; //this won't get changed by update_my_chunkobject

	ObjectID ID;
};


struct VMod_Arr_Item
{
	VMod_Arr_Item();
	~VMod_Arr_Item();

	VMod_Arr_Item(const VMod_Arr_Item & vma);
	VMod_Arr_Item & operator=(const VMod_Arr_Item & vma);
	
	int branch_no;
	int flags;
	int spare;
	int object_index;
	
	friend BOOL operator==(const VMod_Arr_Item &, const VMod_Arr_Item &);
	friend BOOL operator!=(const VMod_Arr_Item &, const VMod_Arr_Item &);	
};

struct Adjacent_Module
{
	Adjacent_Module();
	~Adjacent_Module();

	Adjacent_Module(const Adjacent_Module & vma);
	Adjacent_Module & operator=(const Adjacent_Module & vma);
	
	int flags;
	ChunkVectorInt entry_point;
	int object_index;

	friend BOOL operator==(const Adjacent_Module & am1, const Adjacent_Module & am2);
	friend BOOL operator!=(const Adjacent_Module & am1, const Adjacent_Module & am2);	
};

class Shape_Chunk;
class Shape_Sub_Shape_Chunk;

struct a_frame
{
	a_frame()
	: shape1a (0), shape1b(0), shape2a(0), shape2b(0)
	{}

	Shape_Sub_Shape_Chunk * shape1a;
	Shape_Chunk * shape1b;
	Shape_Sub_Shape_Chunk * shape2a;
	Shape_Chunk * shape2b;
	int spare;
};

	
// Data structures for bits and pieces

struct obinfile
{
	int filepos;
	char name[50];
	size_t length;

	friend BOOL operator==(const obinfile &o1, const obinfile &o2);
	friend BOOL operator!=(const obinfile &o1, const obinfile &o2);
};

struct shpinfile
{
	int filepos;
	int id;
	size_t length;

	friend BOOL operator==(const shpinfile &s1, const shpinfile &s2);
	friend BOOL operator!=(const shpinfile &s1, const shpinfile &s2);
};


struct poly_change_info
{
	int poly_num;
	int vert_num_before;
	int vert_num_after;
	
	friend BOOL operator==(poly_change_info const &f1, poly_change_info const &f2);
	friend BOOL operator!=(poly_change_info const &f1, poly_change_info const &f2);
};



#define animframeflag_not_in_psx			0x00000001
#define animframeflag_not_in_saturn			0x00000002
#define animframeflag_interpolated_frame	0x00000004
struct ChunkAnimFrame
{
	ChunkAnimFrame();
	~ChunkAnimFrame();

	ChunkAnimFrame(const ChunkAnimFrame &);
	//constructor for interpolated frame
	//ChunkAnimFrame(ChunkAnimFrame* startframe,ChunkAnimFrame* endframe,int startwt,int endwt,ChunkShape const *cs);
	ChunkAnimFrame& operator=(const ChunkAnimFrame &);
	

	char* name;
	
	int num_polys;
	int num_verts;
	
	ChunkVectorInt * v_list;
	ChunkVectorFloat * p_normal_list;
	
	int flags;	
	int num_interp_frames;
	int pad3,pad4;
};


#define animseqflag_not_in_psx			0x00000001
#define animseqflag_not_in_saturn		0x00000002


struct ChunkAnimSequence
{
	ChunkAnimSequence();
	~ChunkAnimSequence();

	ChunkAnimSequence (const ChunkAnimSequence &);
	ChunkAnimSequence& operator=(const ChunkAnimSequence &);
	
	void DeleteInterpolatedFrames();
	void GenerateInterpolatedFrames(ChunkShape const *cs);
	
	void UpdateNormalsAndExtents(ChunkShape const *cs,List<int>* poly_not_in_bb=0);

	int SequenceNum;
	char* name;

	int NumFrames;
	ChunkAnimFrame** Frames;
	
	int flags;
	int pad2,pad3,pad4;

	ChunkVectorInt min;
	ChunkVectorInt max;
	float radius;
	
	int num_verts;
	ChunkVectorFloat* v_normal_list;
};
#endif
