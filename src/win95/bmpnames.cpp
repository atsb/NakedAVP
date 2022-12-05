#include <string.h>
#include "bmpnames.hpp"
#include "mishchnk.hpp"

#define UseLocalAssert No
#include "ourasert.h"
#define assert(x) GLOBALASSERT(x)

//macro for helping to force inclusion of chunks when using libraries
FORCE_CHUNK_INCLUDE_IMPLEMENT(bmpnames)


BMP_Name::BMP_Name (const char * fname, int const gbnc_version)
: flags((BMPN_Flags)DEFAULT_BMPN_FLAGS), index(0), version_num (gbnc_version << BMPNAME_PARENT_VER_SHIFT), enum_id(0), priority (DEFAULT_BMPN_PRIORITY), transparency_colour_union(0)
{
	filename = new char [strlen(fname)+1];
	strcpy (filename, fname);
}

BMP_Name::BMP_Name (const char * fname)
: flags((BMPN_Flags)DEFAULT_BMPN_FLAGS), index(0), version_num (0), enum_id(0), priority (DEFAULT_BMPN_PRIORITY), transparency_colour_union(0)
{
	filename = new char [strlen(fname)+1];
	strcpy (filename, fname);
}

BMP_Name::~BMP_Name ()
{
	if (filename)
		delete [] filename;
}

BMP_Name::BMP_Name (const BMP_Name & bn)
{
	if (&bn == this) return;

	filename = new char [strlen(bn.filename)+1];
	strcpy (filename, bn.filename);
	flags = bn.flags;
	index = bn.index;
	version_num = bn.version_num;
	enum_id = bn.enum_id;
	priority = bn.priority;
	transparency_colour_union = bn.transparency_colour_union;
}

void BMP_Name::Validate(void)
{
	if (flags & ChunkBMPFlag_PriorityAndTransparencyAreValid) return;

	priority = DEFAULT_BMPN_PRIORITY;
	flags = (BMPN_Flags)(flags | DEFAULT_BMPN_FLAGS);
}

const BMP_Name & BMP_Name::operator=(const BMP_Name & bn)
{
	if (&bn == this) return(*this);

	if (filename)
		delete [] filename;

	filename = new char [strlen(bn.filename)+1];
	strcpy (filename, bn.filename);
	flags = bn.flags;
	index = bn.index;
	version_num = bn.version_num;
	enum_id = bn.enum_id;
	priority = bn.priority;
	transparency_colour_union = bn.transparency_colour_union;
	
	return(*this);
}



BOOL operator==(const BMP_Name &o1, const BMP_Name &o2)
{
	if (o1.filename && o2.filename) return _stricmp(o1.filename,o2.filename) ? FALSE : TRUE;
	else return &o1 == &o2;
}

BOOL operator!=(const BMP_Name &o1, const BMP_Name &o2)
{
	if (o1.filename && o2.filename) return _stricmp(o1.filename,o2.filename) ? TRUE : FALSE;
	else return &o1 != &o2;
}


///////////////////////////////////////

// Class Chunk_With_BMPs functions



Chunk_With_BMPs::Chunk_With_BMPs (Chunk_With_Children * parent, const char * const ident, const char * bdata, size_t /*bsize*/)
: Chunk (parent, ident), max_index (0)
{
	int temp = *(int *)bdata;

	int num = temp & 0xffff;

	int ver_num = temp >> 16 & 0xffff; // the remains of a previous mistake - not really necessary anymore

	bdata += 4;
	
	for (int j=0; j<num; j++)
	{
		int f,i,d1,d2,d3;
	
		f = *((int *)bdata);
		bdata += 4;
		i = *((int *)bdata);
		bdata += 4;
		d1 = *((int *)bdata);
		bdata += 4;
		d2 = *((int *)bdata);
		bdata += 4;
		d3 = *((int *)bdata);
		bdata += 4;
	
		BMP_Name bn (bdata);
		bdata += (4-strlen(bn.filename)%4) + strlen(bn.filename);

		bn.flags = (BMPN_Flags)f;
		bn.index = i;
		bn.version_num = d1 & BMPNAME_VERSION_NUM_MASK;
		bn.enum_id = (int)((unsigned int)d1 >> BMPNAME_ENUMID_SHIFT);
		bn.priority = d2;
		bn.transparency_colour_union = d3;
						
		max_index = max (bn.index, max_index);
	
		bmps.add_entry (bn);
	}

	if (ver_num)
	{
		set_version_num(ver_num);
	}
}


size_t Chunk_With_BMPs::size_chunk ()
{
	int sz = 12 + 4;
	
	for (LIF<BMP_Name> bl(&bmps); !bl.done(); bl.next())
	{
		sz += (4-strlen(bl().filename)%4) + strlen(bl().filename) + 20;
	}

	chunk_size = sz;
	
	return (chunk_size);
	
}

void Chunk_With_BMPs::fill_data_block (char * data_start)
{
	strncpy (data_start, identifier, 8);

	data_start += 8;

	*((int *) data_start) = chunk_size;

	data_start += 4;

	*((int *) data_start) = bmps.size();

	data_start += 4;
	
	for (LIF<BMP_Name> bl(&bmps); !bl.done(); bl.next())
	{
		*((int *) data_start) = bl().flags;
		data_start += 4;
		*((int *) data_start) = bl().index;
		data_start += 4;
		*((int *) data_start) = bl().version_num & BMPNAME_VERSION_NUM_MASK | bl().enum_id << BMPNAME_ENUMID_SHIFT;
		data_start += 4;
		*((int *) data_start) = bl().priority;
		data_start += 4;
		*((int *) data_start) = bl().transparency_colour_union;
		data_start += 4;
		
		strcpy (data_start, bl().filename);
		data_start += (4-strlen(bl().filename)%4) + strlen(bl().filename);
	}
	
}

int Chunk_With_BMPs::get_version_num(void)
{
	if (parent)
	{
		List<Chunk *> verlist;
		parent->lookup_child("BMNAMVER",verlist);
		while (verlist.size()>1)
		{
			Chunk * v1 = verlist.first_entry();
			Chunk * v2 = verlist[1];
			if (((BMP_Names_Version_Chunk *)v1)->version_num > ((BMP_Names_Version_Chunk *)v2)->version_num)
			{
				delete verlist.last_entry();
				verlist.delete_last_entry();
			}
			else
			{
				delete v1;
				verlist.delete_first_entry();
			}
		}
		if (verlist.size())
		{
			int rv = ((BMP_Names_Version_Chunk *)verlist.first_entry())->version_num;
			return rv;
		}
	}
	return 0;
}

void Chunk_With_BMPs::set_version_num(int v)
{
	if (parent)
	{
		List<Chunk *> verlist;
		parent->lookup_child("BMNAMVER",verlist);
		while (verlist.size()>1)
		{
			Chunk * v1 = verlist.first_entry();
			Chunk * v2 = verlist[1];
			if (((BMP_Names_Version_Chunk *)v1)->version_num > ((BMP_Names_Version_Chunk *)v2)->version_num)
			{
				delete verlist.last_entry();
				verlist.delete_last_entry();
			}
			else
			{
				delete v1;
				verlist.delete_first_entry();
			}
		}
		if (verlist.size())
		{
			((BMP_Names_Version_Chunk *)verlist.first_entry())->version_num = v;
			return;
		}

		(new BMP_Names_Version_Chunk(parent))->version_num = v;
	}
}

void Chunk_With_BMPs::inc_version_num(void)
{
	if (parent)
	{
		List<Chunk *> verlist;
		parent->lookup_child("BMNAMVER",verlist);
		while (verlist.size()>1)
		{
			Chunk * v1 = verlist.first_entry();
			Chunk * v2 = verlist[1];
			if (((BMP_Names_Version_Chunk *)v1)->version_num > ((BMP_Names_Version_Chunk *)v2)->version_num)
			{
				delete verlist.last_entry();
				verlist.delete_last_entry();
			}
			else
			{
				delete v1;
				verlist.delete_first_entry();
			}
		}
		if (verlist.size())
		{
			((BMP_Names_Version_Chunk *)verlist.first_entry())->version_num ++;
			return;
		}

		(new BMP_Names_Version_Chunk(parent))->version_num ++;
	}
}

BMP_Names_ExtraData * Chunk_With_BMPs::GetExtendedData(void)
{
	if (parent)
	{
		List<Chunk *> verlist;
		parent->lookup_child("BMNAMEXT",verlist);
		if (verlist.size())
		{
			return (BMP_Names_ExtraData_Chunk *) verlist.first_entry();
		}
		return new BMP_Names_ExtraData_Chunk(parent);
	}
	return 0;
}

int const * Chunk_With_BMPs::GetMD5Val(BMP_Name const & rcbmp)
{
	Bitmap_MD5_Chunk * md5c = GetMD5Chunk(rcbmp.filename);
	if (md5c)
		if (rcbmp.version_num == md5c->version_num)
			return md5c->md5_val;
	return 0;
}

void Chunk_With_BMPs::RemoveMD5Val(char const * bname)
{
	Bitmap_MD5_Chunk * md5c = GetMD5Chunk(bname);
	if (md5c)
		delete md5c;
}

void Chunk_With_BMPs::SetMD5Val(BMP_Name const & rcbmp, int const * md5id)
{
	Bitmap_MD5_Chunk * md5c = GetMD5Chunk(rcbmp.filename);
	if (md5c)
	{
		if (rcbmp.version_num == md5c->version_num)
		{
			memcpy(md5c->md5_val,md5id,16);
			return;
		}
		else
			delete md5c;
	}
	CreateMD5Chunk(rcbmp,md5id);
}


/////////////////////////////////////
// Global_BMP_Name_Chunk
/////////////////////////////////////
RIF_IMPLEMENT_DYNCREATE("BMPNAMES",Global_BMP_Name_Chunk)

Bitmap_MD5_Chunk * Global_BMP_Name_Chunk::GetMD5Chunk(char const * bname)
{
	List<Chunk *> chlst;
	parent->lookup_child("BMPMD5ID",chlst);

	for (LIF<Chunk *> i_chlst(&chlst); !i_chlst.done(); i_chlst.next())
	{
		Bitmap_MD5_Chunk * md5c = (Bitmap_MD5_Chunk *)i_chlst();

		if (!strcmp(md5c->bmpname,bname))
			if (!(md5c->rifname ? *md5c->rifname : 1) && !(md5c->shapename ? *md5c->shapename : 1))
				return md5c;
	}

	return 0;
}

void Global_BMP_Name_Chunk::CreateMD5Chunk(BMP_Name const & rcbmp, int const * md5id)
{
	new Bitmap_MD5_Chunk(parent,md5id,rcbmp);
}

/////////////////////////////////////
// Bitmap_List_Store_Chunk
/////////////////////////////////////
RIF_IMPLEMENT_DYNCREATE("BMPLSTST",Bitmap_List_Store_Chunk)

Bitmap_MD5_Chunk * Bitmap_List_Store_Chunk::GetMD5Chunk(char const * bname)
{
	List<Chunk *> chlst;
	parent->lookup_child("BMPMD5ID",chlst);

	List<Chunk *> rnlst;
	parent->lookup_child("RIFFNAME",rnlst);
	char const * rname = 0;
	if (rnlst.size())
	{
		rname = ((RIF_Name_Chunk *)rnlst.first_entry())->rif_name;
	}

	for (LIF<Chunk *> i_chlst(&chlst); !i_chlst.done(); i_chlst.next())
	{
		Bitmap_MD5_Chunk * md5c = (Bitmap_MD5_Chunk *)i_chlst();

		if (!strcmp(md5c->bmpname,bname))
			if (!(md5c->rifname ? rname ? strcmp(rname,md5c->rifname) : *md5c->rifname : rname ? *rname : 0) &&
				!(md5c->shapename ? *md5c->shapename : 1))
				return md5c;
	}

	return 0;
}

void Bitmap_List_Store_Chunk::CreateMD5Chunk(BMP_Name const & rcbmp, int const * md5id)
{
	List<Chunk *> rnlst;
	parent->lookup_child("RIFFNAME",rnlst);
	char const * rname = 0;
	if (rnlst.size())
	{
		rname = ((RIF_Name_Chunk *)rnlst.first_entry())->rif_name;
	}
	new Bitmap_MD5_Chunk(parent,md5id,rcbmp,rname);
}



///////////////////////////////////////

// Class BMP_Names_Version_Chunk functions

RIF_IMPLEMENT_DYNCREATE("BMNAMVER",BMP_Names_Version_Chunk)


size_t BMP_Names_Version_Chunk::size_chunk ()
{
	chunk_size = 12+4;
	
	return (chunk_size);
	
}

void BMP_Names_Version_Chunk::fill_data_block (char * data_start)
{
	strncpy (data_start, identifier, 8);

	data_start += 8;

	*((int *) data_start) = chunk_size;

	data_start += 4;

	*((int *) data_start) = version_num;

}



///////////////////////////////////////

// Class BMP_Names_ExtraData_Chunk functions

RIF_IMPLEMENT_DYNCREATE("BMNAMEXT",BMP_Names_ExtraData_Chunk)


size_t BMP_Names_ExtraData_Chunk::size_chunk ()
{
	chunk_size = 12+52;
	
	return (chunk_size);
	
}

void BMP_Names_ExtraData_Chunk::fill_data_block (char * data_start)
{
	strncpy (data_start, identifier, 8);
	data_start += 8;

	*((int *) data_start) = chunk_size;
	data_start += 4;

	*((int *) data_start) = flags;
	data_start += 4;

	for (int i=0; i<12; ++i, data_start+=4)
	{	
		*(int *)data_start = reserved[i];
	}

}

///////////////////////////////////////

// Class External_Shape_BMPs_Store_Chunk functions
RIF_IMPLEMENT_DYNCREATE("SHBMPNAM",External_Shape_BMPs_Store_Chunk)

External_Shape_BMPs_Store_Chunk::External_Shape_BMPs_Store_Chunk (Chunk_With_Children * parent, char const * rifn, char const * shapen)
: Chunk_With_BMPs (parent, "SHBMPNAM"), rifname(0), shapename(0), version_num(0)
{
	for (int i=0; i<12; ++i)
		reserved[i] = 0;

	flags = GBF_NONE;

	if (rifn)
	{
		rifname = new char [strlen(rifn)+1];
		strcpy(rifname,rifn);
	}
	if (shapen)
	{
		shapename = new char [strlen(shapen)+1];
		strcpy(shapename,shapen);
	}
}

External_Shape_BMPs_Store_Chunk::~External_Shape_BMPs_Store_Chunk()
{
	if (rifname) delete[] rifname;
	if (shapename) delete[] shapename;
}

void External_Shape_BMPs_Store_Chunk::fill_data_block(char * data_start)
{
	Chunk_With_BMPs::fill_data_block(data_start);
	data_start += Chunk_With_BMPs::size_chunk();
	size_chunk(); // resize it just in case

	strcpy(data_start,rifname ? rifname : "");
	unsigned int const l1 = rifname ? strlen(rifname)+1 : 1;

	strcpy(data_start+l1,shapename ? shapename : "");
	unsigned int const l2 = shapename ? strlen(shapename)+1 : 1;

	data_start += l1+l2 +3&~3;

	*(int *)data_start = flags;
	data_start+=4;

	*(int *)data_start = version_num;
	data_start+=4;
	
	for (int i=0; i<12; ++i, data_start+=4)
		*(int *)data_start = reserved[i];
}

External_Shape_BMPs_Store_Chunk::External_Shape_BMPs_Store_Chunk (Chunk_With_Children * parent, const char * sdata, size_t ssize)
: Chunk_With_BMPs (parent, "SHBMPNAM", sdata, ssize)
{
	sdata += Chunk_With_BMPs::size_chunk() -12;
	unsigned int const l1 = strlen(sdata)+1;
	rifname = new char [l1];
	strcpy(rifname,sdata);
	unsigned int const l2 = strlen(sdata+l1)+1;
	shapename = new char [l2];
	strcpy(shapename,sdata+l1);
	
	sdata += l1+l2 +3&~3;

	flags = (GlobalBMPFlags)(*(int *)sdata & GBF_MASK);
	sdata+=4;
	
	version_num = *(int *)sdata;
	sdata+=4;
	
	for (int i=0; i<12; ++i, sdata+=4)
		reserved[i] = *(int *)sdata;
}


Bitmap_MD5_Chunk * External_Shape_BMPs_Store_Chunk::GetMD5Chunk(char const * bname)
{
	List<Chunk *> chlst;
	parent->lookup_child("BMPMD5ID",chlst);

	for (LIF<Chunk *> i_chlst(&chlst); !i_chlst.done(); i_chlst.next())
	{
		Bitmap_MD5_Chunk * md5c = (Bitmap_MD5_Chunk *)i_chlst();

		if (!strcmp(md5c->bmpname,bname))
			if (!(md5c->rifname ? rifname ? strcmp(rifname,md5c->rifname) : *md5c->rifname : rifname ? *rifname : 0) &&
				!(md5c->shapename ? shapename ? strcmp(shapename,md5c->shapename) : *md5c->shapename : shapename ? *shapename : 0) &&
				(flags & GBF_SPRITE && md5c->flags & BMD5F_SPRITE || !(flags & GBF_SPRITE) && !(md5c->flags & BMD5F_SPRITE)))

				return md5c;
	}

	return 0;
}

void External_Shape_BMPs_Store_Chunk::CreateMD5Chunk(BMP_Name const & rcbmp, int const * md5id)
{
	Bitmap_MD5_Chunk * md5c = new Bitmap_MD5_Chunk(parent,md5id,rcbmp,rifname,shapename);
	md5c->flags = flags & GBF_SPRITE ? BMD5F_SPRITE : BMD5F_0;
}


/*************************/
/* matching images stuff */
/*************************/

// class ImageDescriptor
// ---------------------
ImageDescriptor::ImageDescriptor()
: filename(0)
, rifname(0)
, fixrifname(0)
{
}

ImageDescriptor::ImageDescriptor(ImageDescriptor const & id2)
: flags(id2.flags)
, filename(0)
, rifname(0)
, fixrifname(0)
{
	spares[0] = id2.spares[0];
	spares[1] = id2.spares[1];
	spares[2] = id2.spares[2];

	if (id2.filename)
	{
		filename = new char [strlen(id2.filename)+1];
		strcpy(filename,id2.filename);
	}
	if (id2.rifname)
	{
		rifname = new char [strlen(id2.rifname)+1];
		strcpy(rifname,id2.rifname);
	}
	if (id2.fixrifname)
	{
		fixrifname = new char [strlen(id2.fixrifname)+1];
		strcpy(fixrifname,id2.fixrifname);
	}
}

ImageDescriptor::ImageDescriptor(IDscFlags idscf, char const * fname, char const * rname, char const * xname)
: flags(idscf)
, filename(0)
, rifname(0)
, fixrifname(0)
{
	spares[0] = 0;
	spares[1] = 0;
	spares[2] = 0;

	if (fname)
	{
		filename = new char [strlen(fname)+1];
		strcpy(filename,fname);
	}
	if (rname)
	{
		rifname = new char [strlen(rname)+1];
		strcpy(rifname,rname);
	}
	if (xname)
	{
		fixrifname = new char [strlen(xname)+1];
		strcpy(fixrifname,xname);
	}
}

ImageDescriptor::~ImageDescriptor()
{
	if (filename) delete[] filename;
	if (rifname) delete[] rifname;
	if (fixrifname) delete[] fixrifname;
}

ImageDescriptor & ImageDescriptor::operator = (ImageDescriptor const & id2)
{
	if (&id2 != this)
	{
		flags = id2.flags;
		spares[0] = id2.spares[0];
		spares[1] = id2.spares[1];
		spares[2] = id2.spares[2];

		if (filename)
		{
			delete[] filename;
			filename = 0;
		}
		if (rifname)
		{
			delete[] rifname;
			rifname = 0;
		}
		if (fixrifname)
		{
			delete[] fixrifname;
			fixrifname = 0;
		}
		
		if (id2.filename)
		{
			filename = new char [strlen(id2.filename)+1];
			strcpy(filename,id2.filename);
		}
		if (id2.rifname)
		{
			rifname = new char [strlen(id2.rifname)+1];
			strcpy(rifname,id2.rifname);
		}
		if (id2.fixrifname)
		{
			fixrifname = new char [strlen(id2.fixrifname)+1];
			strcpy(fixrifname,id2.fixrifname);
		}
	}
	return *this;
}


// I/O
ImageDescriptor::ImageDescriptor(char const * datablock)
: flags((IDscFlags)(*(int const *)datablock & IDSCF_MASK))
{
	spares[0] = *(int const *)(datablock+4);
	spares[1] = *(int const *)(datablock+8);
	spares[2] = *(int const *)(datablock+12);
	datablock += 16;

	size_t len = strlen(datablock)+1;
	filename = new char[len];
	strcpy(filename,datablock);
	datablock += len;

	len = strlen(datablock)+1;
	rifname = new char[len];
	strcpy(rifname,datablock);
	datablock += len;

	len = strlen(datablock)+1;
	fixrifname = new char[len];
	strcpy(fixrifname,datablock);
}

size_t ImageDescriptor::Size() const
{
	return 16
		+ (filename ? strlen(filename) : 0)
		+ (rifname ? strlen(rifname) : 0)
		+ (fixrifname ? strlen(fixrifname) : 0)
		+ 3
	+3&~3;
}

void ImageDescriptor::WriteData(char * datablock) const
{
	*(int *)datablock = flags;
	*(int *)(datablock+4) = spares[0];
	*(int *)(datablock+8) = spares[1];
	*(int *)(datablock+12) = spares[2];
	datablock+=16;

	strcpy(datablock,filename ? filename : "");
	datablock += strlen(datablock)+1;

	strcpy(datablock,rifname ? rifname : "");
	datablock += strlen(datablock)+1;

	strcpy(datablock,fixrifname ? fixrifname : "");
}

// operators
BOOL ImageDescriptor::operator == (ImageDescriptor const & id2) const
{
	if (flags!=id2.flags) return FALSE;
	if (_stricmp(filename ? filename : "",id2.filename ? id2.filename : "")) return FALSE;
	if (_stricmp(rifname ? rifname : "",id2.rifname ? id2.rifname : "")) return FALSE;
	if (_stricmp(fixrifname ? fixrifname : "",id2.fixrifname ? id2.fixrifname : "")) return FALSE;
	return TRUE;
}

// class MatchingImages
// --------------------

// constructos;
MatchingImages::MatchingImages(ImageDescriptor const & _load, ImageDescriptor const & _insteadof)
: load(_load)
, insteadof(_insteadof)
{
	spares[0] = 0;
	spares[1] = 0;
	spares[2] = 0;
}

// I/O
MatchingImages::MatchingImages(char const * datablock)
: load(datablock+12)
, insteadof(datablock+12+load.Size())
{
	spares[0] = *(int const *)datablock;
	spares[1] = *(int const *)(datablock+4);
	spares[2] = *(int const *)(datablock+8);
}

size_t MatchingImages::Size() const
{
	return 12 + load.Size() + insteadof.Size();
}

void MatchingImages::WriteData(char * datablock) const
{
	*(int *)datablock = spares[0];
	*(int *)(datablock+4) = spares[1];
	*(int *)(datablock+8) = spares[2];
	
	load.WriteData(datablock+12);
	insteadof.WriteData(datablock+12+load.Size());
}


// class Matching_Images_Chunk : public Chunk
// ------------------------------------------
RIF_IMPLEMENT_DYNCREATE("MATCHIMG",Matching_Images_Chunk)

// I/O
Matching_Images_Chunk::Matching_Images_Chunk(Chunk_With_Children * parent, char const * datablock, size_t size)
: Chunk(parent,"MATCHIMG")
, flags ((MICFlags)(*(int *)(datablock+8) & MICF_MASK))
{
	spares[0] = *(int *)datablock;
	spares[1] = *(int *)(datablock+4);

	int listsize = *(int *)(datablock+12);
	datablock += 16;

	for (;listsize; --listsize)
	{
		mlist.add_entry_end(datablock);
		datablock += mlist.last_entry().Size();
	}

	assert(datastart + size == datablock);
}

size_t Matching_Images_Chunk::size_chunk()
{
	chunk_size = 28; // 2dw spares, 1dw flags, 1dw list size, 12b header

	for (LIF<MatchingImages> mlit(&mlist); !mlit.done(); mlit.next())
	{
		chunk_size += mlit().Size();
	}
	
	return chunk_size;
}

void Matching_Images_Chunk::fill_data_block(char * data_start)
{
	strncpy (data_start, identifier, 8);
	data_start += 8;
	*(int *) data_start = chunk_size;
	data_start += 4;

	*(int *)data_start = spares[0];
	*(int *)(data_start+4) = spares[1];
	*(int *)(data_start+8) = flags;
	
	*(int *)(data_start+12) = mlist.size();

	data_start += 16;
	
	for (LIF<MatchingImages> mlit(&mlist); !mlit.done(); mlit.next())
	{
		mlit().WriteData(data_start);
		data_start += mlit().Size();
	}
}

ImageDescriptor const & Matching_Images_Chunk::GetLoadImage(ImageDescriptor const & _insteadof)
{
	for (LIF<MatchingImages> mlit(&mlist); !mlit.done(); mlit.next())
	{
		if (_insteadof == mlit().insteadof) return mlit().load;
	}
	return _insteadof;
}

// Bitmap MD5 Chunk
RIF_IMPLEMENT_DYNCREATE("BMPMD5ID",Bitmap_MD5_Chunk)

Bitmap_MD5_Chunk::Bitmap_MD5_Chunk(Chunk_With_Children * parent, int const * md5id, BMP_Name const & rcbmp, char const * rname, char const * sname)
: Chunk(parent,"BMPMD5ID"), flags(BMD5F_0), version_num(rcbmp.version_num), spare(0)
{
	memcpy(md5_val,md5id,16);
	
	char const * bname = rcbmp.filename;
	if (!bname) bname = "";
	bmpname = new char[strlen(bname)+1];
	strcpy(bmpname,bname);
	
	if (!rname) rname = "";
	rifname = new char[strlen(rname)+1];
	strcpy(rifname,rname);
	
	if (!sname) sname = "";
	shapename = new char[strlen(sname)+1];
	strcpy(shapename,sname);
}

Bitmap_MD5_Chunk::Bitmap_MD5_Chunk(Chunk_With_Children * parent, char const * datablock, size_t)
: Chunk(parent,"BMPMD5ID"), flags((BMPMD5_Flags)(*(int *)(datablock+4) & BMD5F_MASK)), version_num(*(int *)(datablock+8)), spare(*(int *)datablock) 
{
	memcpy(md5_val,datablock+12,16);
	datablock += 28;
	unsigned int const blen = strlen(datablock)+1;
	bmpname = new char [blen];
	strcpy(bmpname,datablock);
	datablock += blen;
	unsigned int const rlen = strlen(datablock)+1;
	rifname = new char [rlen];
	strcpy(rifname,datablock);
	datablock += rlen;
	unsigned int const slen = strlen(datablock)+1;
	shapename = new char [slen];
	strcpy(shapename,datablock);
}

Bitmap_MD5_Chunk::~Bitmap_MD5_Chunk()
{
	delete [] bmpname;
	delete [] rifname;
	delete [] shapename;
}

void Bitmap_MD5_Chunk::fill_data_block(char * datastart)
{
	strncpy (datastart, identifier, 8);
	datastart += 8;
	*((int *) datastart) = chunk_size;
	datastart += 4;

	*(int *)datastart = spare;
	*(int *)(datastart+4) = flags;
	*(int *)(datastart+8) = version_num;
	memcpy(datastart+12,md5_val,16);
	datastart += 28;
	strcpy(datastart,bmpname ? bmpname : "");
	datastart += strlen(datastart)+1;
	strcpy(datastart,rifname ? rifname : "");
	datastart += strlen(datastart)+1;
	strcpy(datastart,shapename ? shapename : "");
}

size_t Bitmap_MD5_Chunk::size_chunk()
{
	return chunk_size =	12+28
		+(bmpname ? strlen(bmpname) : 0)
		+(rifname ? strlen(rifname) : 0)
		+(shapename ? strlen(shapename) : 0)
		+3 +3&~3;
}
