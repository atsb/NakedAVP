#include "chunk.hpp"
#include <math.h>
#include "chnktype.hpp"

#define UseLocalAssert No
#include "ourasert.h"
#define assert(x) GLOBALASSERT(x)

// misc data structures functions
BOOL operator==(const obinfile &o1, const obinfile &o2)
{
	return (&o1 == &o2);
}
BOOL operator!=(const obinfile &o1, const obinfile &o2)
{
	return (&o1 != &o2);
}

BOOL operator==(const shpinfile &s1, const shpinfile &s2)
{
	return (&s1 == &s2);
}
BOOL operator!=(const shpinfile &s1, const shpinfile &s2)
{
	return (&s1 != &s2);
}

BOOL operator== (const ChunkUV_List &c1, const ChunkUV_List &c2)
{
	return(&c1 == &c2);
}

BOOL operator!= (const ChunkUV_List &c1, const ChunkUV_List &c2)
{
	return(&c1 != &c2);
}

BOOL operator== (const ObjectID &o1, const ObjectID &o2)
{
	return((o1.id1==o2.id1) && (o1.id2==o2.id2));
}

BOOL operator!= (const ObjectID &o1, const ObjectID &o2)
{
	return((o1.id1!=o2.id1) || (o1.id2!=o2.id2));
}

ObjectID  Minimum(const ObjectID &o1,const  ObjectID &o2)
{
	if(o1.id1<o2.id1) return o1;
	if(o1.id1>o2.id1) return o2;
	if(o1.id2<o2.id2) return o1;
	return o2;
}
//////////////////////////////////////////////
#define CHUNK_MAX_UVINDEX ((1<<20)-1)
#define CHUNK_MAX_TEXTUREINDEX	((1<<12)-1)

#define CHUNK_UVINDEX_MASK 		 0xfffff000
#define CHUNK_TEXTUREINDEX_MASK  0xfff
#define CHUNK_NEWUVINDEX_BITS 0x0000f000

unsigned int ChunkPoly::GetUVIndex()
{
	if(colour & CHUNK_NEWUVINDEX_BITS)
	{
		unsigned int uv_index;
		uv_index=(colour & CHUNK_NEWUVINDEX_BITS) << 4;
		uv_index|= (colour >> 16);

		return uv_index;
	}
	else
	{
		//uvindex is just in the top 16 bits
		return (colour >> 16);
	}	
	
}

unsigned int ChunkPoly::GetTextureIndex()
{
	return (colour & CHUNK_TEXTUREINDEX_MASK);
}

void ChunkPoly::SetUVIndex(unsigned int uv_index)
{
	assert(uv_index<=CHUNK_MAX_UVINDEX);
	//clear the old uvindex
	colour &=~CHUNK_UVINDEX_MASK;
	
	if(uv_index<65536)
	{
		//fit uv index into the top 16 bits if it will fit , to maintain compatibility with
		//old chunk loaders
		colour |= (uv_index<<16);
	}
	else
	{
		//put the bottom 16 bits of the uv_index in the top 16 bits of the colour
		colour |= (uv_index & 0xffff) << 16;
		//put the next 4 bits of the uv_index in the lower middle 4 bits of the colour
		uv_index>>=16;
		colour |= (uv_index <<12);
	}

}

void ChunkPoly::SetTextureIndex(unsigned int texture_index)
{
	assert(texture_index<=CHUNK_MAX_TEXTUREINDEX);

	colour &=~ CHUNK_TEXTUREINDEX_MASK;
	colour |= texture_index;
}



//////////////////////////////////////////////

ChunkVector operator+(const ChunkVector& a, const ChunkVector& b)
{
	ChunkVector v;
	v.x=a.x+b.x;
	v.y=a.y+b.y;
	v.z=a.z+b.z;
	return v;
}


ChunkVector operator-(const ChunkVector& a, const ChunkVector& b)
{
	ChunkVector v;
	v.x=a.x-b.x;
	v.y=a.y-b.y;
	v.z=a.z-b.z;
	return v;
}

ChunkVector& ChunkVector::operator+=(const ChunkVector& a)
{
  x += a.x;
  y += a.y;
  z += a.z;

  return *this;
}



ChunkVector& ChunkVector::operator-=(const ChunkVector& a)
{
  x -= a.x;
  y -= a.y;
  z -= a.z;

  return *this;
}



ChunkVector::operator VECTORCH () const
{
	VECTORCH v;
	v.vx = (int)x;
	v.vy = (int)y;
	v.vz = (int)z;

	return(v);
}

ChunkVector::operator ChunkVectorInt () const
{
	ChunkVectorInt v;
	v.x = (int)x;
	v.y = (int)y;
	v.z = (int)z;

	return(v);
}
ChunkVector::operator ChunkVectorFloat () const
{
	ChunkVectorFloat v;
	v.x = (float)x;
	v.y = (float)y;
	v.z = (float)z;

	return(v);
}

ChunkVector operator*(const ChunkVector & a, const double s)
{
	ChunkVector v;
	v.x = a.x * s;
	v.y = a.y * s;
	v.z = a.z * s;
	return(v);
	
}

ChunkVector operator/(const ChunkVector & a, const double s)
{
	ChunkVector v;
	v.x = a.x / s;
	v.y = a.y / s;
	v.z = a.z / s;
	return(v);
}

ChunkVector operator*(const ChunkVector& a, const ChunkVector& b)
{
	ChunkVector v;
  	v.x=a.y*b.z - a.z*b.y;
    v.y=a.z*b.x - a.x*b.z;
    v.z=a.x*b.y - a.y*b.x;
	return v; 
}


double dot(const ChunkVector& a, const ChunkVector& b)
{
  return(a.x*b.x + a.y*b.y + a.z*b.z);
}

double  mod(const ChunkVector& a)
{
  return(sqrt(dot(a,a)));
}

int ChunkVector::norm()
{
  double modulos = mod(*this);
  
  if(modulos == 0)return(0);

  x /=modulos;
  y /= modulos;
  z /= modulos;

  return(1);

}
//////////////////////////////////////////////
ChunkVectorInt operator+(const ChunkVectorInt& a, const ChunkVectorInt& b)
{
	ChunkVectorInt v;
	v.x=a.x+b.x;
	v.y=a.y+b.y;
	v.z=a.z+b.z;
	return v;
}

ChunkVectorInt operator-(const ChunkVectorInt& a, const ChunkVectorInt& b)
{
	ChunkVectorInt v;
	v.x=a.x-b.x;
	v.y=a.y-b.y;
	v.z=a.z-b.z;
	return v;
}

ChunkVectorInt& ChunkVectorInt::operator+=(const ChunkVectorInt& a)
{
  x += a.x;
  y += a.y;
  z += a.z;

  return *this;
}

ChunkVectorInt& ChunkVectorInt::operator-=(const ChunkVectorInt& a)
{
  x -= a.x;
  y -= a.y;
  z -= a.z;

  return *this;
}

ChunkVectorInt::operator VECTORCH () const
{
	VECTORCH v;
	v.vx = x;
	v.vy = y;
	v.vz = z;

	return(v);
}

ChunkVectorInt operator*(const ChunkVectorInt & a, const double s)
{
	ChunkVectorInt v;
	v.x =(int) (a.x * s);
	v.y =(int) (a.y * s);
	v.z =(int) (a.z * s);
	return(v);
	
}

ChunkVectorInt operator/(const ChunkVectorInt & a, const double s)
{
	ChunkVectorInt v;
	v.x =(int) (a.x / s);
	v.y =(int) (a.y / s);
	v.z =(int) (a.z / s);
	return(v);
}

double  mod(const ChunkVectorInt& a)
{
  return(sqrt((double)a.x*(double)a.x+(double)a.y*(double)a.y+(double)a.z*(double)a.z));
}

int ChunkVectorInt::norm()
{
  double modulos = mod(*this) /65536.0;
  
  if(modulos == 0)return(0);

  x =(int) (x/modulos);
  y =(int) (y/modulos);
  z =(int) (z/modulos);

  return(1);

}
////////////////////////////////////////////////////////
ChunkVectorFloat operator+(const ChunkVectorFloat& a, const ChunkVectorFloat& b)
{
	ChunkVectorFloat v;
	v.x=a.x+b.x;
	v.y=a.y+b.y;
	v.z=a.z+b.z;
	return v;
}


ChunkVectorFloat operator-(const ChunkVectorFloat& a, const ChunkVectorFloat& b)
{
	ChunkVectorFloat v;
	v.x=a.x-b.x;
	v.y=a.y-b.y;
	v.z=a.z-b.z;
	return v;
}

ChunkVectorFloat& ChunkVectorFloat::operator+=(const ChunkVectorFloat& a)
{
  x += a.x;
  y += a.y;
  z += a.z;

  return *this;
}



ChunkVectorFloat& ChunkVectorFloat::operator-=(const ChunkVectorFloat& a)
{
  x -= a.x;
  y -= a.y;
  z -= a.z;

  return *this;
}

ChunkVectorFloat operator*(const ChunkVectorFloat & a, const double s)
{
	ChunkVectorFloat v;
	v.x =(float) (a.x * s);
	v.y =(float) (a.y * s);
	v.z =(float) (a.z * s);
	return(v);
	
}

ChunkVectorFloat operator/(const ChunkVectorFloat & a, const double s)
{
	ChunkVectorFloat v;
	v.x =(float) (a.x / s);
	v.y =(float) (a.y / s);
	v.z =(float) (a.z / s);
	return(v);
}

ChunkVectorFloat::operator VECTORCH () const
{
	VECTORCH v;
	v.vx =(int) (x*65536);
	v.vy =(int) (y*65536);
	v.vz =(int) (z*65536);

	return(v);
}

int ChunkVectorFloat::norm()
{
  float modulos =(float) mod(*this);
  
  if(modulos == 0)return(0);

  x /= modulos;
  y /= modulos;
  z /= modulos;

  return(1);
}

double  mod(const ChunkVectorFloat& a)
{
  return(sqrt((double)a.x*(double)a.x+(double)a.y*(double)a.y+(double)a.z*(double)a.z));
}
////////////////////////////////////////////////////////
ChunkShape::~ChunkShape()
{
	
	if (v_list) delete [] v_list;
	if (v_normal_list) delete [] v_normal_list;
	if (p_normal_list) delete [] p_normal_list;
	if (poly_list) delete [] poly_list;
	if (uv_list) delete [] uv_list;
	if (texture_fns)
		for (int i = 0; i<num_texfiles; i++)
			if (texture_fns[i]) delete texture_fns[i];
}

ChunkShape::ChunkShape()
{
	num_polys = 0;
	num_verts = 0;
	num_uvs = 0;
	num_texfiles = 0;

	v_list = 0;
	v_normal_list = 0;
	p_normal_list = 0;
	poly_list = 0;
	uv_list = 0;
	texture_fns = 0;		

	radius_about_centre=0;
}


ChunkShape::ChunkShape(const ChunkShape &shp)
{
	int i;

	radius = shp.radius;
	max = shp.max;
	min = shp.min;
	num_polys = shp.num_polys;
	num_verts = shp.num_verts;
	num_uvs = shp.num_uvs;
	num_texfiles = shp.num_texfiles;

	if (shp.v_list) {
		v_list = new ChunkVectorInt [num_verts];
		for (i=0;	i<num_verts; i++)
			v_list[i] = shp.v_list[i];
	}
	else v_list = 0;

	if (shp.v_normal_list) {
		v_normal_list = new ChunkVectorFloat [num_verts];
		for (i=0;	i<num_verts; i++)
			v_normal_list[i] = shp.v_normal_list[i];
	}
	else v_normal_list = 0;

	if (shp.p_normal_list) {
		p_normal_list = new ChunkVectorFloat [num_polys];
		for (i=0;	i<num_polys; i++)
			p_normal_list[i] = shp.p_normal_list[i];
	}
	else p_normal_list = 0;

	if (shp.poly_list) {
		poly_list = new ChunkPoly [num_polys];
		for (i=0;	i<num_polys; i++)
			poly_list[i] = shp.poly_list[i];
	}
	else poly_list = 0;

	if (shp.uv_list) {
		uv_list = new ChunkUV_List [num_uvs];
		for (i=0; i<num_uvs; i++)
			uv_list[i] = shp.uv_list[i];
	}
	else uv_list = 0;

	if (shp.texture_fns) {
		texture_fns = new char * [num_texfiles];
		for (i=0; i<num_texfiles; i++) {
			texture_fns[i] = new char [strlen (shp.texture_fns[i]) + 1];
			strcpy (texture_fns[i], shp.texture_fns[i]);
		}
	}
	else texture_fns = 0;

	centre=shp.centre;
	radius_about_centre=shp.radius_about_centre;
}

ChunkShape& ChunkShape::operator=(const ChunkShape &shp)
{
	int i;	

	if (v_list) delete [] v_list;
	if (v_normal_list) delete [] v_normal_list;
	if (p_normal_list) delete [] p_normal_list;
	if (poly_list) delete [] poly_list;
	if (uv_list) delete [] uv_list;
	if (texture_fns)
		for (int i = 0; i<num_texfiles; i++)
			if (texture_fns[i]) delete texture_fns[i];

	radius = shp.radius;
	max = shp.max;
	min = shp.min;
	num_polys = shp.num_polys;
	num_verts = shp.num_verts;
	num_uvs = shp.num_uvs;
	num_texfiles = shp.num_texfiles;


		
	if (shp.v_list) {
		v_list = new ChunkVectorInt [num_verts];
		for (i=0;	i<num_verts; i++)
			v_list[i] = shp.v_list[i];
	}
	else v_list = 0;

	if (shp.v_normal_list) {
		v_normal_list = new ChunkVectorFloat [num_verts];
		for (i=0;	i<num_verts; i++)
			v_normal_list[i] = shp.v_normal_list[i];
	}
	else v_normal_list = 0;

	if (shp.p_normal_list) {
		p_normal_list = new ChunkVectorFloat [num_polys];
		for (i=0;	i<num_polys; i++)
			p_normal_list[i] = shp.p_normal_list[i];
	}
	else p_normal_list = 0;

	if (shp.poly_list) {
		poly_list = new ChunkPoly [num_polys];
		for (i=0;	i<num_polys; i++)
			poly_list[i] = shp.poly_list[i];
	}
	else poly_list = 0;

	if (shp.uv_list) {
		uv_list = new ChunkUV_List [num_uvs];
		for (i=0; i<num_uvs; i++)
			uv_list[i] = shp.uv_list[i];
	}
	else uv_list = 0;

	if (shp.texture_fns) {
		texture_fns = new char * [num_texfiles];
		for (i=0; i<num_texfiles; i++) {
			texture_fns[i] = new char [strlen (shp.texture_fns[i]) + 1];
			strcpy (texture_fns[i], shp.texture_fns[i]);
		}
	}
	else texture_fns = 0;
	
	centre=shp.centre;
	radius_about_centre=shp.radius_about_centre;

	return *this;
}

void ChunkShape::rescale (double scale)
{
	int i;

	if (v_list) 
	{
		for (i=0; i<num_verts; i++)
		{
			v_list[i].x =(int) (v_list[i].x*scale);
			v_list[i].y =(int) (v_list[i].y*scale);
			v_list[i].z =(int) (v_list[i].z*scale);
		}
	}

	radius =(float) (radius*scale);
	max.x =(int) (max.x*scale);
	max.y =(int) (max.y*scale);
	max.z =(int) (max.z*scale);
	
	min.x =(int) (min.x*scale);
	min.y =(int) (min.y*scale);
	min.z =(int) (min.z*scale);

	centre=centre*scale;
	radius_about_centre=(float) (radius_about_centre*scale);

}	


VMod_Arr_Item::VMod_Arr_Item()
{
}

VMod_Arr_Item::~VMod_Arr_Item()
{
}

VMod_Arr_Item::VMod_Arr_Item(const VMod_Arr_Item & vma)
{
	branch_no = vma.branch_no;
	flags = vma.flags;
	spare = vma.spare;
	object_index=vma.object_index;
}

VMod_Arr_Item& VMod_Arr_Item::operator=(const VMod_Arr_Item & vma)
{
	if (&vma == this) return(*this);

	branch_no = vma.branch_no;
	flags = vma.flags;
	spare = vma.spare;
	object_index=vma.object_index;

	return(*this);
	
}

BOOL operator==(const VMod_Arr_Item & vm1, const VMod_Arr_Item & vm2)
{
	return(&vm1 == &vm2);
	
}

BOOL operator!=(const VMod_Arr_Item & vm1, const VMod_Arr_Item & vm2)
{
	return(&vm1 != &vm2);
	
}
	
///////////////////////////////////////

Adjacent_Module::Adjacent_Module()
{
	flags = 0;
	entry_point.x=0;
	entry_point.y=0;
	entry_point.z=0;
}

Adjacent_Module::~Adjacent_Module()
{
}

Adjacent_Module::Adjacent_Module(const Adjacent_Module & am)
{
	object_index=am.object_index;
	flags = am.flags;
	entry_point = am.entry_point;
}

Adjacent_Module& Adjacent_Module::operator=(const Adjacent_Module & am)
{
	if (&am == this) return(*this);

	object_index=am.object_index;
	flags = am.flags;
	entry_point = am.entry_point;

	return(*this);
	
}

BOOL operator==(const Adjacent_Module & am1, const Adjacent_Module & am2)
{
	return(&am1 == &am2);
}

BOOL operator!=(const Adjacent_Module & am1, const Adjacent_Module & am2)
{
	return(&am1 != &am2);
}


///////////////////////////////////////

BOOL operator==(poly_change_info const &f1, poly_change_info const &f2)
{
	return(&f1 == &f2);
}

BOOL operator!=(poly_change_info const &f1, poly_change_info const &f2)
{
	return(&f1 != &f2);
}


//////////////////////////////////////////////
ChunkAnimFrame::~ChunkAnimFrame()
{
	if(name) delete [] name;
	if(v_list) delete [] v_list;
	if(p_normal_list) delete [] p_normal_list;

}

ChunkAnimFrame::ChunkAnimFrame()
{
	name=0;
	num_polys=0;
	num_verts=0;
	v_list=0;
	p_normal_list=0;
	flags=num_interp_frames=pad3=pad4=0;
}

ChunkAnimFrame::ChunkAnimFrame(const ChunkAnimFrame & frm)
{
	
	if(frm.name)
	{
		name=new char[strlen(frm.name)+1];
		strcpy(name,frm.name);
	}
	else
		name=0;
	
	num_polys=frm.num_polys;
	num_verts=frm.num_verts;

	if(num_polys)
	{
		p_normal_list=new ChunkVectorFloat[num_polys];
		for(int i=0;i<num_polys;i++)
		{
			p_normal_list[i]=frm.p_normal_list[i];
		}
	}
	else
		p_normal_list=0;
	
	if(num_verts)
	{
		v_list=new ChunkVectorInt[num_verts];
		for(int i=0;i<num_verts;i++)
		{
			v_list[i]=frm.v_list[i];
		}
	}
	else
		v_list=0;
	flags=frm.flags;
	num_interp_frames=frm.num_interp_frames;
	pad3=frm.pad3;
	pad4=frm.pad4;
}		 
/*
ChunkAnimFrame::ChunkAnimFrame(ChunkAnimFrame* startframe,ChunkAnimFrame* endframe,int startwt,int endwt,ChunkShape const *cs)
{
	name=0;
	num_polys=startframe->num_polys;
	num_verts=startframe->num_verts;
	flags=startframe->flags|animframeflag_interpolated_frame;
	num_interp_frames=0;
	pad3=0;
	pad4=0;

	v_list=new ChunkVector[num_verts];
	p_normal_list=new ChunkVector[num_polys];

	double start_mult=startwt/(double)(startwt+endwt);
	double end_mult=endwt/(double)(startwt+endwt);

	for(int i=0;i<num_verts;i++)
	{
		v_list[i].x=startframe->v_list[i].x*start_mult+endframe->v_list[i].x*end_mult;
		v_list[i].y=startframe->v_list[i].y*start_mult+endframe->v_list[i].y*end_mult;
		v_list[i].z=startframe->v_list[i].z*start_mult+endframe->v_list[i].z*end_mult;
	}

	for(i=0;i<num_polys;i++)
	{
		ChunkVector v1=cs->v_list[cs->poly_list[i].vert_ind[1]]-cs->v_list[cs->poly_list[i].vert_ind[0]];	
		ChunkVector v2=cs->v_list[cs->poly_list[i].vert_ind[2]]-cs->v_list[cs->poly_list[i].vert_ind[0]];
		ChunkVector norm;
		norm.x=v1.y*v2.z-v1.z*v2.y;	
		norm.y=v1.z*v2.x-v1.x*v2.z;	
		norm.z=v1.x*v2.y-v1.y*v2.x;
		double length=sqrt(norm.x*norm.x+norm.y*norm.y+norm.z*norm.z);	
		cs->p_normal_list[i]=norm*(1/length);
	}

}
*/
ChunkAnimFrame& ChunkAnimFrame::operator=(const ChunkAnimFrame &frm)
{
	
	if(name) delete [] name;
	if(v_list) delete [] v_list;
	if(p_normal_list) delete [] p_normal_list;
	
	
	if(frm.name)
	{
		name=new char[strlen(frm.name)+1];
		strcpy(name,frm.name);
	}
	else
		name=0;

	num_polys=frm.num_polys;
	num_verts=frm.num_verts;

	if(num_polys)
	{
		p_normal_list=new ChunkVectorFloat[num_polys];
		for(int i=0;i<num_polys;i++)
		{
			p_normal_list[i]=frm.p_normal_list[i];
		}
	}
	else
		p_normal_list=0;
	
	if(num_verts)
	{
		v_list=new ChunkVectorInt[num_verts];
		for(int i=0;i<num_verts;i++)
		{
			v_list[i]=frm.v_list[i];
		}
	}
	else
		v_list=0;
	flags=frm.flags;
	num_interp_frames=frm.num_interp_frames;
	pad3=frm.pad3;
	pad4=frm.pad4;
	return *this;
}


ChunkAnimSequence::~ChunkAnimSequence()
{
	if(name) delete [] name;
	for(int i=0;i<NumFrames;i++)
	{
		if(Frames[i])delete Frames[i];
	}
	if(Frames) delete [] Frames;
	if(v_normal_list) delete [] v_normal_list;
}

ChunkAnimSequence::ChunkAnimSequence()
{
	SequenceNum=-1;
	name=0;
	NumFrames=0;
	Frames=0;
	flags=pad2=pad3=pad4=0;
	
	num_verts=0;
	v_normal_list=0;
	min.x=min.y=min.z=0;
	max.x=max.y=max.z=0;
	radius=0;

}
ChunkAnimSequence::ChunkAnimSequence(const ChunkAnimSequence & seq)
{
	SequenceNum=seq.SequenceNum;

	if(seq.name)
	{
		name=new char[strlen(seq.name)+1];
		strcpy(name,seq.name);
		
	}
	else
		name=0;

	NumFrames=seq.NumFrames;
	if(NumFrames)
	{
		Frames=new ChunkAnimFrame*[NumFrames];
		for(int i=0;i<NumFrames;i++)
		{
			Frames[i]=new ChunkAnimFrame(*seq.Frames[i]);
		}
	}
	else
		Frames=0;
	
	flags=seq.flags;
	pad2=seq.pad2;
	pad3=seq.pad3;
	pad4=seq.pad4;
	
	num_verts=seq.num_verts;
	if(num_verts)
	{
		v_normal_list=new ChunkVectorFloat[num_verts];
		for(int i=0;i<num_verts;i++)
		{
			v_normal_list[i]=seq.v_normal_list[i];
		}
	}
	else v_normal_list=0;
	min=seq.min;
	max=seq.max;
	radius=seq.radius;		
}

ChunkAnimSequence& ChunkAnimSequence::operator=(const ChunkAnimSequence &seq)
{
	if(name) delete [] name;
	if(Frames) delete [] Frames;
	if(v_normal_list) delete [] v_normal_list;
	SequenceNum=seq.SequenceNum;

	if(seq.name)
	{
		name=new char[strlen(seq.name)+1];
		strcpy(name,seq.name);
		
	}
	else
		name=0;

	NumFrames=seq.NumFrames;
	if(NumFrames)
	{
		Frames=new ChunkAnimFrame*[NumFrames];
		for(int i=0;i<NumFrames;i++)
		{
			Frames[i]=new ChunkAnimFrame(*seq.Frames[i]);
		}
	}
	else
		Frames=0;
	
	flags=seq.flags;
	pad2=seq.pad2;
	pad3=seq.pad3;
	pad4=seq.pad4;
	
	num_verts=seq.num_verts;
	if(num_verts)
	{
		v_normal_list=new ChunkVectorFloat[num_verts];
		for(int i=0;i<num_verts;i++)
		{
			v_normal_list[i]=seq.v_normal_list[i];
		}
	}
	else v_normal_list=0;
	min=seq.min;
	max=seq.max;
	radius=seq.radius;		
	return *this;
}


void ChunkAnimSequence::UpdateNormalsAndExtents(ChunkShape const * cs,List<int>* poly_not_in_bb)
{
	int i;
	
	if(!cs) return;
	num_verts=cs->num_verts;
	if(!v_normal_list)v_normal_list=new ChunkVectorFloat[cs->num_verts];
	for(i=0;i<num_verts;i++)
	{
		v_normal_list[i].x=0;
		v_normal_list[i].y=0;
		v_normal_list[i].z=0;
	}
	for(i=0;i<cs->num_polys;i++)
	{
		const ChunkPoly* cp=&cs->poly_list[i];
		for(int j=0;j<cp->num_verts;j++)
		{
			int vi=cp->vert_ind[j];
			for(int k=0;k<NumFrames;k++)
			{
				v_normal_list[vi]+=Frames[k]->p_normal_list[i];
			}
		}
	}
	for(i=0;i<num_verts;i++)
	{
		double length=mod(v_normal_list[i]);
		if(length)
		{
			v_normal_list[i]=v_normal_list[i]/length;
		}
		else
		{
			v_normal_list[i].x=1;
			v_normal_list[i].y=0;
			v_normal_list[i].z=0;
		}	
	}

	
	max.x = -2000000000;
	max.y = -2000000000;
	max.z = -2000000000;
	
	min.x = 2000000000;
	min.y = 2000000000;
	min.z = 2000000000;
	
	radius = 0;
	
	int* vert_in_bb=0;
	if(poly_not_in_bb)
	{
		vert_in_bb=new int[cs->num_verts];
		for(i=0;i<cs->num_verts;i++)
		{
			vert_in_bb[i]=0;
		}
		for(i=0;i<cs->num_polys;i++)
		{
			if(poly_not_in_bb->contains(i))continue;
			const ChunkPoly* cp=&cs->poly_list[i];
			for(int j=0;j<cp->num_verts;j++)
			{
				vert_in_bb[cp->vert_ind[j]]=1;
			}
		
		}
	}
	for(i=0;i<NumFrames;i++)
	{
		ChunkAnimFrame* caf=Frames[i];
		for (int j=0; j<caf->num_verts; j++)
		{
			if(vert_in_bb && !vert_in_bb[j]) continue;
			max.x = max(max.x, caf->v_list[j].x);
			max.y = max(max.y, caf->v_list[j].y);
			max.z = max(max.z, caf->v_list[j].z);

			min.x = min(min.x, caf->v_list[j].x);
			min.y = min(min.y, caf->v_list[j].y);
			min.z = min(min.z, caf->v_list[j].z);
			
			double temp_rad = mod(caf->v_list[j]);
			
			radius = max (radius, (float)temp_rad);
		}
	}
	if(vert_in_bb) delete [] vert_in_bb;
}

void ChunkAnimSequence::DeleteInterpolatedFrames()
{
	int i;
	int NewNumFrames=NumFrames;
	
	for(i=0;i<NumFrames;i++)
	{
		if(Frames[i]->flags & animframeflag_interpolated_frame)NewNumFrames--;
	}
	if(NewNumFrames==NumFrames)return;

	int framepos=0;
	for(i=0;i<NumFrames;i++)
	{
		if(Frames[i]->flags & animframeflag_interpolated_frame) continue;
		Frames[framepos++]=Frames[i];
	}
	NumFrames=NewNumFrames;
	Frames=(ChunkAnimFrame**)realloc(Frames,sizeof(ChunkAnimFrame*)*NumFrames);

}

void ChunkAnimSequence::GenerateInterpolatedFrames(ChunkShape const *cs)
{
	DeleteInterpolatedFrames();
	/*
	int NewNumFrames=NumFrames;
	for(int i=0;i<NumFrames;i++)
	{
		NewNumFrames+=Frames[i]->num_interp_frames;	
	}
	if(NewNumFrames==NumFrames) return;

	ChunkAnimFrame** NewFrames=new ChunkAnimFrame*[NewNumFrames];
	
	int framepos=0;
	for( i=0;i<NumFrames;i++ )
	{
		NewFrames[framepos++]=Frames[i];	
		if(Frames[i]->num_interp_frames==0)continue;

		ChunkAnimFrame* startframe=Frames[i];
		ChunkAnimFrame*	endframe=Frames[(i+1)%NumFrames];
		
		for(int j=0;j<startframe->num_interp_frames;j++)
		{
			NewFrames[framepos++]=new ChunkAnimFrame(startframe,endframe,startframe->num_interp_frames-j,j+1,cs);
		}
	}
	delete [] Frames;
	Frames=NewFrames;
	NumFrames=NewNumFrames;
	*/
}
