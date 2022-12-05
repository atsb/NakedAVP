#include "chunkpal.hpp"
#include "mishchnk.hpp"

#ifndef UseLocalAssert
#define UseLocalAssert 1
#endif
#include "ourasert.h"
#define assert(x) GLOBALASSERT(x)

//macro for helping to force inclusion of chunks when using libraries
FORCE_CHUNK_INCLUDE_IMPLEMENT(chunkpal)

///////////////////////////////////////

// Class Environment_Palette_Chunk functions


void Palette_Outdated(Chunk_With_Children * parent)
{
	if (parent)
	{
		List<Chunk *> plist;
		parent->lookup_child("ENVPALET",plist);
		for (LIF<Chunk *> plit(&plist); !plit.done(); plit.next())
		{
			((Environment_Palette_Chunk *)plit())->flags &= ~EnvPalFlag_UpToDate;
		}
	}
}

void FixedPalette_Outdated(Chunk_With_Children * parent)
{
	if (parent)
	{
		List<Chunk *> plist;
		parent->lookup_child("PRSETPAL",plist);
		for (LIF<Chunk *> plit(&plist); !plit.done(); plit.next())
		{
			for (LIF<Preset_Palette> findconst(&((Preset_Palette_Chunk *)plit())->pplist); !findconst.done(); findconst.next())
			{
				Preset_Palette temp = findconst();
				if (temp.flags & PrePalFlag_Reserved)
				{
					temp.flags &= ~PrePalFlag_UpToDate;
					findconst.change_current(temp);
				}
			}
		}
	}
}

BOOL IsFixedPalette(Chunk_With_Children * parent)
{
	if (parent)
	{
		List<Chunk *> plist;
		parent->lookup_child("PRSETPAL",plist);
		
		LIF<Chunk *> plit(&plist);
		for (; !plit.done(); plit.next())
		{
			for (LIF<Preset_Palette> findconst(&((Preset_Palette_Chunk *)plit())->pplist); !findconst.done(); findconst.next())
			{
				if (findconst().flags & PrePalFlag_Reserved)
				{
					return TRUE;
				}
			}
		}
		parent->lookup_child("SETPALST",plist);
		for (plit = LIF<Chunk *> (&plist); !plit.done(); plit.next())
		{
			for (LIF<Preset_Palette> findconst(&((Preset_Palette_Store_Chunk *)plit())->pplist); !findconst.done(); findconst.next())
			{
				if (findconst().flags & PrePalFlag_Reserved)
				{
					return TRUE;
				}
			}
		}
	}
	return FALSE;
}

RIF_IMPLEMENT_DYNCREATE("ENVPALET",Environment_Palette_Chunk)

Environment_Palette_Chunk::~Environment_Palette_Chunk ()
{
	if (pixel_data)
	{
		unsigned char * temp_pd = (unsigned char *)pixel_data;
		delete [] temp_pd;
	}
}

size_t Environment_Palette_Chunk::size_chunk()
{
	chunk_size = 12 + 8 + (width * height * 3 + 3 & ~3);
	return chunk_size;
}

void Environment_Palette_Chunk::fill_data_block (char * data_start)
{
	strncpy (data_start, identifier, 8);

	data_start += 8;

	*((int *) data_start) = chunk_size;

	data_start += 4;

	*((int*)data_start) = width * height;
	*((int*)(data_start+4)) = flags;
	
	data_start += 8;

	for (int i=0; i<(width*height*3); i++)
	{
		data_start[i] = pixel_data[i];
	}
	
}


///////////////////////////////////////

// Class Preset_Palette_Chunk functions


Preset_Palette::~Preset_Palette ()
{
	if (pixel_data)
	{
		unsigned char * temp_pd = (unsigned char *)pixel_data;
		delete [] temp_pd;
	}
	if (name) delete[] name;
}

Preset_Palette::Preset_Palette (Preset_Palette const & c)
: size(c.size)
, flags(c.flags)
, reserved1(c.reserved1)
, reserved2(c.reserved2)
, startpos(c.startpos)
, name(0)
, pixel_data(grab_pixel_data(c.size, c.pixel_data))
{
	if (c.name)
	{
		name = new char[strlen(c.name)+1];
		strcpy(name,c.name);
	}
}

Preset_Palette & Preset_Palette::operator = (Preset_Palette const & c)
{
	if (pixel_data)
	{
		unsigned char * temp_pd = (unsigned char *)pixel_data;
		delete [] temp_pd;
	}
	if (name)
	{
		delete[] name;
		name = 0;
	}
	if (c.name)
	{
		name = new char[strlen(c.name)+1];
		strcpy(name,c.name);
	}

	*(int *)&size = c.size;
	*(int *)&flags = c.flags;
	*(int *)&reserved1 = c.reserved1;
	*(int *)&reserved2 = c.reserved2;
	*(int *)&startpos = c.startpos;

	*(const unsigned char * *)&pixel_data = grab_pixel_data(c.size,c.pixel_data);

	return *this;
}



size_t Preset_Palette::size_chunk() const
{
	return 20 + (size * 3 + (name ? strlen(name)+1 : 8) +3 &~3);
}

void Preset_Palette::fill_data_block (char * data_start)
{
	*(int*)data_start = size;
	*(int*)(data_start+4) = flags;
	*(int*)(data_start+8) = reserved1;
	*(int*)(data_start+12) = reserved2;
	*(int*)(data_start+16) = startpos;
	
	data_start += 20;

	unsigned char const * sptr = pixel_data;

	for (int i=size*3; i; --i, ++sptr, ++data_start)
	{
		*data_start = *sptr;
	}
	strcpy(data_start,name ? name : "unnamed");
}

RIF_IMPLEMENT_DYNCREATE("PRSETPAL",Preset_Palette_Chunk)

Preset_Palette_Chunk::Preset_Palette_Chunk(Chunk_With_Children * const parent, char const * sdata, size_t const /*ssize*/)
: Chunk(parent,"PRSETPAL")
, flags(*(int *)(sdata+4))
, version_num(*(int *)sdata)
, reserved1(*(int *)(sdata+8))
, reserved2(*(int *)(sdata+12))
, reserved3(*(int *)(sdata+16))
{
	int const pplistsize = *(int *)(sdata+20);
	sdata += 24;

	for (int i = pplistsize; i; --i)
	{
		Preset_Palette current(sdata);
		sdata += current.size_chunk();
		pplist.add_entry(current);
	}
}


size_t Preset_Palette_Chunk::size_chunk ()
{
	chunk_size = 12 + 24;

	for (LIF<Preset_Palette> li(&pplist); !li.done(); li.next())
	{
		chunk_size += li().size_chunk();
	}

	return chunk_size;
}

void Preset_Palette_Chunk::fill_data_block (char * data_start)
{
	strncpy (data_start, identifier, 8);

	data_start += 8;

	*(int *) data_start = chunk_size;

	data_start += 4;

	*(int*)data_start = version_num;
	*(int*)(data_start+4) = flags;
	*(int*)(data_start+8) = reserved1;
	*(int*)(data_start+12) = reserved2;
	*(int*)(data_start+16) = reserved3;
	*(int*)(data_start+20) = pplist.size();

	data_start += 24;

	for (LIF<Preset_Palette> li(&pplist); !li.done(); li.next())
	{
		Preset_Palette current(li());

		current.fill_data_block(data_start);
		data_start += current.size_chunk();
	}
}


///////////////////////////////////////

// Class Environment_TLT_Chunk functions


RIF_IMPLEMENT_DYNCREATE("ENVTXLIT",Environment_TLT_Chunk)

Environment_TLT_Chunk::Environment_TLT_Chunk (Chunk_With_Children * parent, const char * sdata, size_t ssize)
: Chunk (parent, "ENVTXLIT"), width (*((int*)(sdata))),
	num_levels (*((int*)(sdata+4))), flags(*(int *)(sdata+28)), filename(0), table (0)
{
	int i;
	
	for (i=0; i<ChunkTLT_NumReserved; ++i) reserved[i] = *(int *)(sdata+8+(i<<2));

	if (flags & ChunkTLTFlag_ExternalFile)
	{
		filename = new char[strlen(sdata+32)+1];
		strcpy(filename,sdata+32);
	}
	else if ((signed) ssize >= 32 + width*num_levels)
	{
		table = new unsigned char [width*num_levels];
		unsigned char * tableptr = table;
		unsigned char const * sdataptr = (unsigned char *)(sdata+32);
		for (i=width*num_levels; i; --i)
		{
			*tableptr++ = *sdataptr++;
		}
	}
	
}

Environment_TLT_Chunk::~Environment_TLT_Chunk ()
{
	if (table) delete[] table;
	if (filename) delete[] filename;
}

size_t Environment_TLT_Chunk::size_chunk()
{
	if (flags & ChunkTLTFlag_ExternalFile)
	{
		if (filename)
		{
			chunk_size = 12 + 32 + (strlen(filename)+4 & ~3);
		}
		else
		{
			chunk_size = 12 + 32 + 4; 
		}
	}
	else
	{
		chunk_size = 12 + 32 + width * num_levels;
	}
	return(chunk_size);
	
}

void Environment_TLT_Chunk::fill_data_block (char * data_start)
{
	int i;
	
	strncpy (data_start, identifier, 8);

	data_start += 8;

	*((int *) data_start) = chunk_size;

	data_start += 4;

	*((int*)data_start) = width;
	*((int*)(data_start+4)) = num_levels;
	
	data_start += 8;

	for (i=0; i < ChunkTLT_NumReserved ; ++i, data_start+=4)
		*((int *)data_start) = reserved[i];

	*(int *)data_start = flags;
	data_start+=4;

	if (flags & ChunkTLTFlag_ExternalFile)
	{
		if (filename)
			strcpy(data_start,filename);
		else
			*data_start = 0;
	}
	else
	{
		if (table)
		{
			unsigned char * tableptr = table;

			for (i=width*num_levels; i; --i)
			{
				*data_start++ = *tableptr++;
			}
		}
		else
		{
			for (i=width*num_levels; i; --i)
			{
				*data_start++ = 0;
			}
		}
	}
}


///////////////////////////////////////

// Class TLT_Config_Chunk functions

TLTCC_Flags::TLTCC_Flags(unsigned int data)
: allow_v2  (data & 0x00000001 ? 1 : 0)
, nodefault (data & 0x00000002 ? 1 : 0)
{
}

TLTCC_Flags::operator unsigned int () const
{
	return
		allow_v2  * 0x00000001 +
		nodefault * 0x00000002 ;
}

RIF_IMPLEMENT_DYNCREATE("TLTCONFG",TLT_Config_Chunk)

TLT_Config_Chunk::TLT_Config_Chunk (Chunk_With_Children * parent, const char * sdata, size_t ssize)
: Chunk (parent, "TLTCONFG")
, num_shades_white(*(unsigned int const *)sdata)
, table_size(*(unsigned int const *)(sdata+4))
, palette_size(*(unsigned int const *)(sdata+12))
, flags(*(unsigned int const *)(sdata+8))
{
	if (!table_size) table_size = 256;

	sdata+=16;

	for (int i=0; i<3; ++i, sdata+=4) reserved[i] = *(int *)sdata;
	
	unsigned int const len = strlen(sdata)+4&~3;
	srcrifname = new char[len];
	memcpy(srcrifname,sdata,len);
	sdata += len;
	
	unsigned int listsize = *(int *)sdata;
	sdata += 4;

	while (listsize)
	{
		TLTConfigBlock block(sdata);
		sdata += block.Size();
		blocks.add_entry_end(block);
		listsize--;
	}
	
	// hmm, size_chunk was wrong so allow sizes which were wrong in that way to pass
	assert (ssize + 12 == size_chunk() || ssize + 12 + (44+strlen(srcrifname)+4&~3) - (44+strlen(srcrifname)+4&~4) == size_chunk());
}

size_t TLT_Config_Chunk::size_chunk()
{
	chunk_size = 44 + (srcrifname ? strlen(srcrifname) : 0) + 4 &~3;
	for (LIF<TLTConfigBlock> tbli(&blocks); !tbli.done(); tbli.next())
	{
		chunk_size += tbli().Size();
	}
	return chunk_size;
}

void TLT_Config_Chunk::fill_data_block (char * data_start)
{
	strncpy (data_start, identifier, 8);
	data_start += 8;

	*((int *) data_start) = chunk_size;
	data_start += 4;

	*(unsigned int *)data_start = num_shades_white;
	data_start += 4;

	*(unsigned int *)data_start = table_size;
	data_start += 4;

	*(unsigned int *)data_start = flags;
	data_start += 4;

	*(unsigned int *)data_start = palette_size;
	data_start += 4;

	for (int i=0; i<3 ; ++i, data_start+=4)
		*(int *)data_start = reserved[i];

	strcpy(data_start,srcrifname ? srcrifname : "");
	
	data_start += strlen(data_start) + 4 &~3;

	*(int *)data_start = blocks.size();
	data_start+=4;

	for (LIF<TLTConfigBlock> tbli(&blocks); !tbli.done(); tbli.next())
	{
		tbli().WriteData(data_start);
		data_start += tbli().Size();
	}
}


///////////////////////////////////////

// Class Environment_Data_Chunk functions

// constructor from buffer

RIF_IMPLEMENT_DYNCREATE("GAMEMODE",Environment_Game_Mode_Chunk)

/*
Children For  Environment_Game_Mode_Chunk :


"GMODHEAD"		Environment_Game_Mode_Header_Chunk
"ENVPALET"		Environment_Palette_Chunk
"ENVTXLIT"		Environment_TLT_Chunk
"TLTCONFG"		TLT_Config_Chunk
"CLRLOOKP"		Coloured_Polygons_Lookup_Chunk
"MATCHIMG"		Matching_Images_Chunk
"SHBMPNAM"		External_Shape_BMPs_Store_Chunk
"RIFCHILD"		RIF_Child_Chunk
"SETPALST"		Preset_Palette_Store_Chunk
"BMPMD5ID"		Bitmap_MD5_Chunk
*/


Environment_Game_Mode_Chunk::Environment_Game_Mode_Chunk(Chunk_With_Children* const parent,const char* data,size_t const size)
:Chunk_With_Children(parent,"GAMEMODE") , envd_parent((Environment_Data_Chunk *)parent)
{
	const char * buffer_ptr = data;
	while ((data-buffer_ptr)< (signed) size) {
		if ((*(int *)(data + 8)) + (data-buffer_ptr) > (signed) size) {
			Parent_File->error_code = CHUNK_FAILED_ON_LOAD_NOT_RECOGNISED;
			break;
		}
		
		DynCreate(data);
		data += *(int *)(data + 8);
	}
}

char * Environment_Game_Mode_Chunk::ExpandedIdentifier() const
{
	char const * full_fname = header->flags & GameModeFlag_Editable ? ((File_Chunk *)GetRootChunk())->filename : header->get_safe_source();
	char const * skip = strrchr(full_fname,'\\');
	if (skip) full_fname = skip+1;
	skip = strrchr(full_fname,'/');
	if (skip) full_fname = skip+1;
	skip = strrchr(full_fname,':');
	if (skip) full_fname = skip+1;
	char * retp = new char[strlen(full_fname)+strlen(header->mode_identifier)+3];
	strcpy(retp,full_fname);
	char * dotpos = strrchr(retp,'.');
	if (dotpos) *dotpos = 0;
	strcat(retp,"::");
	strcat(retp,header->mode_identifier);
	return retp;
}

// header chunk
RIF_IMPLEMENT_DYNCREATE("GMODHEAD",Environment_Game_Mode_Header_Chunk)

Environment_Game_Mode_Header_Chunk::Environment_Game_Mode_Header_Chunk(Chunk_With_Children * const parent, const char * pdata, size_t const /*psize*/)
: Chunk (parent, "GMODHEAD"), flags(0), rif_files()
{
	flags = *((int *) pdata);

	pdata+=4;

	for (int i=0; i<ChunkGMod_NumReserved; i++, pdata+=4)
	{
		reserved[i] = *((int *) pdata);
	}

	version_num = *(int *)pdata;
	pdata+=4;
	
	mode_identifier = new char[strlen(pdata)+1];

	strcpy (mode_identifier, (pdata));

	pdata += strlen(mode_identifier) + 1;

	while (*pdata)
	{
		unsigned int len = strlen(pdata)+1;

		char * riffn = new char[len];
		strcpy(riffn,pdata);
		rif_files.add_entry(riffn);

		pdata += len;
	}

	((Environment_Game_Mode_Chunk*)parent)->header = this;
}


Environment_Game_Mode_Header_Chunk::~Environment_Game_Mode_Header_Chunk()
{
	if (mode_identifier) delete[] mode_identifier;

	while (rif_files.size())
	{
		delete [] rif_files.first_entry();
		rif_files.delete_first_entry();
	}
}
	


void Environment_Game_Mode_Header_Chunk::fill_data_block ( char * data_start)
{
	strncpy (data_start, identifier, 8);

	data_start += 8;

	*((int *) data_start) = chunk_size;

	data_start += 4;

	*((int *) data_start) = flags;
	data_start+=4;

	for (int i=0; i<ChunkGMod_NumReserved; i++, data_start+=4)
	{
		*((int *) data_start) = reserved[i];
	}
	*(int *)data_start = version_num;
	data_start+=4;
	
	strcpy ((data_start), mode_identifier);

	data_start += strlen(mode_identifier) + 1;

	for (LIF<char *> li(&rif_files); !li.done(); li.next())
	{
		strcpy(data_start,li());

		data_start += strlen(li())+1;
	}

	*data_start = 0; // double 0-byte terminator
}

	
size_t Environment_Game_Mode_Header_Chunk::size_chunk ()
{
	chunk_size = 12+20+(strlen(mode_identifier)+1);

	for (LIF<char *> li(&rif_files); !li.done(); li.next())
	{
		chunk_size += strlen(li())+1;
	}

	chunk_size += 4; // 1 byte terminator, 3(max) to pad
	chunk_size &= ~3;
	return chunk_size;
}



////////////////////////////////////////

// Class RIF_Child_Chunk - for RIFs in game modes containing graphics
RIF_IMPLEMENT_DYNCREATE("RIFCHILD",RIF_Child_Chunk)

RIF_Child_Chunk::~RIF_Child_Chunk()
{
	if (filename) delete[] filename;
	if (rifname) delete[] rifname;
}

size_t RIF_Child_Chunk::size_chunk()
{
	chunk_size = 12 + 12 + (strlen(rifname)+1 +3 &~3) + 4 + (strlen(filename)+1 +3 &~3) + 4;

	for (LIF<BMP_Flags> li(&bmps); !li.done(); li.next())
	{
		chunk_size += 8 + strlen(li().filename)+1 +3 & ~3;
	}

	return chunk_size;
}

void RIF_Child_Chunk::fill_data_block(char * data_start)
{
	strncpy (data_start, identifier, 8);

	data_start += 8;

	*((int *) data_start) = chunk_size;

	data_start += 4;

	for (int i=0; i<ChunkRIFChild_NumReserved; i++, data_start+=4)
	{
		*((int *) data_start) = reserved[i];
	}

	*(RCC_Flags *)data_start = flags;
	data_start += 4;

	strcpy(data_start,rifname);
	data_start += strlen(rifname)+1 +3 &~3;

	*((int *) data_start) = version_num;

	data_start += 4;

	strcpy(data_start,filename);
	data_start += strlen(filename)+1 +3 &~3;

	*((int *) data_start) = bmps.size();
	data_start += 4;

	for (LIF<BMP_Flags> li(&bmps); !li.done(); li.next())
	{
		*((int *) data_start) = li().flags;
		data_start += 4;
		*((int *) data_start) = li().version_num & BMPFLAGS_VERSION_NUM_MASK | li().enum_id << BMPFLAGS_ENUMID_SHIFT;
		data_start += 4;

		strcpy(data_start, li().filename);
		data_start += strlen(li().filename)+1 +3 &~3;
	}

}

int const * RIF_Child_Chunk::GetMD5Val(BMP_Flags const & rcbmp)
{
	Bitmap_MD5_Chunk * md5c = GetMD5Chunk(rcbmp.filename);
	if (md5c)
		if (rcbmp.version_num == md5c->version_num)
			return md5c->md5_val;
	return 0;
}

void RIF_Child_Chunk::RemoveMD5Val(char const * bname)
{
	Bitmap_MD5_Chunk * md5c = GetMD5Chunk(bname);
	if (md5c)
		delete md5c;
}

void RIF_Child_Chunk::SetMD5Val(BMP_Flags const & rcbmp, int const * md5id)
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

Bitmap_MD5_Chunk * RIF_Child_Chunk::GetMD5Chunk(char const * bname)
{
	List<Chunk *> chlst;
	parent->lookup_child("BMPMD5ID",chlst);

	for (LIF<Chunk *> i_chlst(&chlst); !i_chlst.done(); i_chlst.next())
	{
		Bitmap_MD5_Chunk * md5c = (Bitmap_MD5_Chunk *)i_chlst();

		if (!strcmp(md5c->bmpname,bname))
			if (!(md5c->rifname ? rifname ? strcmp(rifname,md5c->rifname) : *md5c->rifname : rifname ? *rifname : 0) &&
				!(md5c->shapename ? *md5c->shapename : 1))
				return md5c;
	}

	return 0;
}

void RIF_Child_Chunk::CreateMD5Chunk(BMP_Flags const & rcbmp, int const * md5id)
{
	new Bitmap_MD5_Chunk(parent,md5id,rcbmp,rifname);
}

RIF_Child_Chunk::RIF_Child_Chunk (Chunk_With_Children * const parent, const char * sdata, size_t const /*ssize*/)
: Chunk(parent,"RIFCHILD"), egm_parent((Environment_Game_Mode_Chunk * const)parent)
{
	int i;

	for (i=0; i<ChunkRIFChild_NumReserved; i++, sdata+=4)
	{
		reserved[i] = *((int *) sdata);
	}
	flags = *(RCC_Flags *)sdata;
	sdata += 4;

	unsigned int len = strlen(sdata)+1;
	rifname = new char[len];
	strcpy(rifname,sdata);
	sdata += len+3 &~3;

	version_num = *(int *)sdata;
	sdata += 4;

	len = strlen(sdata)+1;
	filename = new char[len];
	strcpy(filename,sdata);
	sdata += len+3 &~3;

	int listsize = *(int *)sdata;
	sdata+=4;

	for (i = listsize; i; --i)
	{
		BMP_Flags temp;
		temp.flags = *(BMPN_Flags *)sdata;
		sdata += 4;
		temp.enum_id = (int)(*(unsigned int *)sdata >> BMPFLAGS_ENUMID_SHIFT);
		temp.version_num = *(int *)sdata & BMPFLAGS_VERSION_NUM_MASK;
		sdata += 4;

		len = strlen(sdata)+1;
		temp.filename = new char[len];
		strcpy(temp.filename,sdata);
		sdata += len+3 &~3;

		bmps.add_entry(temp);
	}
	
}

/////////////////////////////////
// Preset_Palette_Store_Chunk
// ties in with RIF_Child_Chunk
// the filenames (-dirname) should match

RIF_IMPLEMENT_DYNCREATE("SETPALST",Preset_Palette_Store_Chunk)

Preset_Palette_Store_Chunk::Preset_Palette_Store_Chunk(Chunk_With_Children * const parent, char const * sdata, size_t const /*ssize*/)
: Chunk(parent,"SETPALST")
, flags(*(int *)(sdata+4))
, version_num(*(int *)sdata)
, reserved1(*(int *)(sdata+8))
, reserved2(*(int *)(sdata+12))
, reserved3(*(int *)(sdata+16))
{
	sdata += 20;

	unsigned int const len = strlen(sdata)+1;
	rifname = new char[len];
	strcpy(rifname,sdata);
	sdata += len + 3 &~3;
	
	int const pplistsize = *(int *)sdata;
	sdata += 4;

	for (int i = pplistsize; i; --i)
	{
		Preset_Palette current(sdata);
		sdata += current.size_chunk();
		pplist.add_entry(current);
	}
}


size_t Preset_Palette_Store_Chunk::size_chunk ()
{
	chunk_size = 12 + 24 + strlen(rifname)+1+3 &~3;

	for (LIF<Preset_Palette> li(&pplist); !li.done(); li.next())
	{
		chunk_size += li().size_chunk();
	}

	return chunk_size;
}

void Preset_Palette_Store_Chunk::fill_data_block (char * data_start)
{
	strncpy (data_start, identifier, 8);

	data_start += 8;

	*(int *) data_start = chunk_size;

	data_start += 4;

	*(int*)data_start = version_num;
	*(int*)(data_start+4) = flags;
	*(int*)(data_start+8) = reserved1;
	*(int*)(data_start+12) = reserved2;
	*(int*)(data_start+16) = reserved3;
	data_start += 20;

	strcpy(data_start,rifname);
	data_start+=strlen(rifname)+1+3 &~3;

	*(int*)(data_start) = pplist.size();
	data_start += 4;

	for (LIF<Preset_Palette> li(&pplist); !li.done(); li.next())
	{
		Preset_Palette current(li());

		current.fill_data_block(data_start);
		data_start += current.size_chunk();
	}
}





///////////////////////////////////////

// Class Coloured_Polygons_Lookup_Chunk functions

// simple 32K tables for palettes to quickly map coloured
// polygons to the right palette colour on loading, with 15-bit definition


RIF_IMPLEMENT_DYNCREATE("CLRLOOKP",Coloured_Polygons_Lookup_Chunk)

Coloured_Polygons_Lookup_Chunk::Coloured_Polygons_Lookup_Chunk (Chunk_With_Children * parent, const char * sdata, size_t /*ssize*/)
: Chunk (parent, "CLRLOOKP"), flags (*((int*)(sdata))),
	filename(0), table (0)
{
	int i;
	
	for (i=0; i<ChunkCPLU_NumReserved; ++i) reserved[i] = *(int *)(sdata+4+(i<<2));

	if (flags & ChunkCPLUFlag_ExternalFile)
	{
		filename = new char[strlen(sdata+32)+1];
		strcpy(filename,sdata+32);
	}
	else
	{
		table = new unsigned char [1<<15];
		unsigned char * tableptr = table;
		unsigned char const * sdataptr = (unsigned char *)(sdata+32);
		for (i=1<<15; i; --i)
		{
			*tableptr++ = *sdataptr++;
		}
	}

}

Coloured_Polygons_Lookup_Chunk::~Coloured_Polygons_Lookup_Chunk ()
{
	if (table) delete [] table;
	if (filename) delete [] filename;
}

size_t Coloured_Polygons_Lookup_Chunk::size_chunk()
{
	if (flags & ChunkCPLUFlag_ExternalFile)
	{
		if (filename)
		{
			chunk_size = 12 + 32 + (strlen(filename)+4 & ~3);
		}
		else
		{
			chunk_size = 12 + 32 + 4; 
		}
	}
	else
	{
		chunk_size = 12 + 32 + (1<<15);
	}
	return(chunk_size);
	
}

void Coloured_Polygons_Lookup_Chunk::fill_data_block (char * data_start)
{
	int i;
	
	strncpy (data_start, identifier, 8);

	data_start += 8;

	*((int *) data_start) = chunk_size;

	data_start += 4;

	*((int*)data_start) = flags;
	
	data_start += 4;

	for (i=0; i < ChunkCPLU_NumReserved ; ++i, data_start+=4)
		*((int *)data_start) = reserved[i];

	if (flags & ChunkCPLUFlag_ExternalFile)
	{
		if (filename)
			strcpy(data_start,filename);
		else // escape
			*data_start = 0;
	}
	else
	{
		if (table)
		{
			unsigned char * tableptr = table;

			for (i=1<<15; i; --i)
			{
				*data_start++ = *tableptr++;
			}
		}
		else // escape
		{
			for (i=1<<15; i; --i)
			{
				*data_start++ = 0;
			}
		}
	}
}
