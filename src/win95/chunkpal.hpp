#ifndef _included_chunkpal_hpp_
#define _included_chunkpal_hpp_

#include "chunk.hpp"
#include "bmpnames.hpp"

class Environment_Data_Chunk;

static const unsigned char * grab_pixel_data(int ssize, unsigned char const * sdata)
{
	if (!ssize) return 0;
	ssize *= 3;

	unsigned char * pixels = new unsigned char[ssize];

	unsigned char * ddata = pixels;

	for (int cnt = ssize; cnt; --cnt, ++ddata, ++sdata) *ddata = *sdata;

	return pixels;
}



class Environment_Palette_Chunk : public Chunk
{

public:

	// constructor from buffer
	Environment_Palette_Chunk (Chunk_With_Children * const parent, const char * sdata, size_t const /*ssize*/)
		: Chunk (parent, "ENVPALET")
		, width (*(int *)sdata)
		, height (1)
		, flags (*(int *)(sdata+4))
		, pixel_data (grab_pixel_data(*(int *)sdata,(unsigned char *)(sdata+8)))
		{}
	~Environment_Palette_Chunk ();
															
	virtual size_t size_chunk ();

	virtual void fill_data_block (char * data_start);
	
	const int width;
	const int height;
	int flags; // was width, but width was always 1
	// do not use as a flag     0x00000001
	#define EnvPalFlag_UpToDate       0x00000002 // make flag
	#define EnvPalFlag_Lit            0x00000004 // uses darken flag when generating palette
	#define EnvPalFlag_V2             0x00000008 // flagged to distinguish between big tlt palette & normal palette
	#define DEFAULT_ENV_PAL_FLAGS EnvPalFlag_Lit

	const unsigned char * const pixel_data;
	
private:

	friend class Environment_Data_Chunk;
	friend class Environment_Game_Mode_Chunk;
	

};



class Preset_Palette_Chunk;

class Preset_Palette
{
public:

	Preset_Palette ()
		: size(0)
		, flags(0)
		, reserved1(0)
		, reserved2(0)
		, startpos(0)
		, name(0)
		, pixel_data(0)
		{}
	
	// copy constructor
	Preset_Palette (Preset_Palette const & c);

	~Preset_Palette ();

	Preset_Palette & operator = (Preset_Palette const & c);

	const int size;
	int flags;
	#define PrePalFlag_Reserved 0x00000001 // preset palette defines contant entries in palette for graphics which are not reloaded
	#define PrePalFlag_UpToDate 0x00000002
	#define PrePalFlag_V2       0x00000004 // calculated fixed palette using wide palette, tlt and lit textures

	const int reserved1;
	const int reserved2;
	int startpos;

	char * name;

	const unsigned char * const pixel_data;

	// for the list template
	inline BOOL operator == (Preset_Palette const & c) const { return !_stricmp(name,c.name); }
	inline BOOL operator != (Preset_Palette const & c) const { return _stricmp(name,c.name); }
	inline BOOL operator < (Preset_Palette const & c) const { return startpos < c.startpos; }

private:

	friend class Preset_Palette_Chunk;
	friend class Preset_Palette_Store_Chunk;
	
	size_t size_chunk () const;

	void fill_data_block (char * data_start);

	// constructor from buffer
	Preset_Palette (char const * sdata)
		: size (*(int *)sdata)
		, flags (*(int *)(sdata+4))
		, reserved1 (*(int *)(sdata+8))
		, reserved2 (*(int *)(sdata+12))
		, startpos (*(int *)(sdata+16))
		, pixel_data (grab_pixel_data(*(int *)sdata,(unsigned char *)(sdata+20)))
		{
			sdata += 20+size*3;
			name = new char[strlen(sdata)+1];
			strcpy(name,sdata);
		}
};


class Preset_Palette_Chunk : public Chunk
{
public:

	// constructor from buffer
	Preset_Palette_Chunk (Chunk_With_Children * const parent, char const * sdata, size_t const ssize);
	~Preset_Palette_Chunk () {};

	virtual size_t size_chunk ();

	virtual void fill_data_block (char * data_start);

	int flags;
	int version_num;

	const int reserved1;
	const int reserved2;
	const int reserved3;

	List<Preset_Palette> pplist;


private:

	friend class Environment_Data_Chunk;
	

};




class Environment_TLT_Chunk : public Chunk
{

public:

	// constructor from buffer
	Environment_TLT_Chunk (Chunk_With_Children * parent, const char * sdata, size_t ssize);

	~Environment_TLT_Chunk ();
															
	virtual size_t size_chunk ();

	virtual void fill_data_block (char * data_start);

	int width; // should be == palette size, though may well have to be 256 due to the way 3dc lookup works
	int num_levels; // usually 256, for 256 "different" shades of each colour

	#define ChunkTLT_NumReserved 5
	int reserved[ ChunkTLT_NumReserved ];

	int flags; // either expect filename or table
	#define ChunkTLTFlag_ExternalFile   0x00000001
	#define ChunkTLTFlag_OptionsChanged 0x00000002
	#define ChunkTLTFlag_V2             0x00000004 // version 2: may have width != 256, flagged so we can distinguish between two tables in chunk
	
	char * filename; // either a filename

	unsigned char * table; // or an actual table in memory

private:

	friend class Environment_Data_Chunk;
	friend class Environment_Game_Mode_Chunk;
	

	
};


class TLTConfigBlock
{
public:
	// constructos;
	TLTConfigBlock()
		{
			spares[0]=0;
			spares[1]=0;
			spares[2]=0;
			spares[3]=0;
		}

		TLTConfigBlock(double const & iistart, double const & oistart, double const & iiend, double const & oiend)
		: input_intensity_start(iistart)
		, output_intensity_start(oistart)
		, input_intensity_end(iiend)
		, output_intensity_end(oiend)
		{
			spares[0]=0;
			spares[1]=0;
			spares[2]=0;
			spares[3]=0;
		}

	// I/O
	TLTConfigBlock(char const * datablock)
		: input_intensity_start(*(double *)datablock)
		, output_intensity_start(*(double *)(datablock+8))
		, input_intensity_end(*(double *)(datablock+16))
		, output_intensity_end(*(double *)(datablock+24))
		{
			memcpy(spares,datablock+32,16);
		}
	inline size_t Size() const { return 48; }
	inline void WriteData(char * datablock) const
		{
			*(double *)datablock = input_intensity_start;
			datablock += 8;
			*(double *)datablock = output_intensity_start;
			datablock += 8;
			*(double *)datablock = input_intensity_end;
			datablock += 8;
			*(double *)datablock = output_intensity_end;
			datablock += 8;
			memcpy(datablock,spares,16);
		}

	// operators
	inline BOOL operator == (TLTConfigBlock const & tcb2) const
		{
			return input_intensity_start == tcb2.input_intensity_start
				&& output_intensity_start == tcb2.output_intensity_start
				&& input_intensity_end == tcb2.input_intensity_end
				&& output_intensity_end == tcb2.output_intensity_end;
		}
	inline BOOL operator != (TLTConfigBlock const & tcb2) const
		{ return ! operator == (tcb2); }
	
	// members
	double input_intensity_start;
	double output_intensity_start;
	double input_intensity_end;
	double output_intensity_end;

private:

	int spares[4];
	
};

struct TLTCC_Flags
{
	TLTCC_Flags(unsigned int data = 0);

	operator unsigned int () const;

	unsigned int allow_v2 : 1; // palette size refers to bigger size table
	unsigned int nodefault : 1; // suppress normal 256 colour tlts
};

class TLT_Config_Chunk : public Chunk
{

public:

	// constructor from buffer
	TLT_Config_Chunk (Chunk_With_Children * parent, const char * sdata, size_t ssize);

	~TLT_Config_Chunk()
		{ if (srcrifname) delete[] srcrifname; }

	virtual size_t size_chunk ();

	virtual void fill_data_block (char * data_start);

	char * srcrifname;

	unsigned int num_shades_white;

	unsigned int table_size;

	List<TLTConfigBlock> blocks;

	unsigned int palette_size; // for version 2 palettes and big tlt

	TLTCC_Flags flags;

	inline BOOL operator == (TLT_Config_Chunk const & tcc2) const
		{ return blocks == tcc2.blocks && num_shades_white == tcc2.num_shades_white && table_size == tcc2.table_size && flags == tcc2.flags && (!flags.allow_v2 || palette_size == tcc2.palette_size); }
	inline BOOL operator != (TLT_Config_Chunk const & tcc2) const
		{ return ! operator == (tcc2); }
	inline BOOL NeedFullRemake(TLT_Config_Chunk const & tcc_old) const
		{ return blocks != tcc_old.blocks || num_shades_white != tcc_old.num_shades_white; }
	inline BOOL NeedNewTLTPalette(TLT_Config_Chunk const & tcc_old) const
		{ return flags != tcc_old.flags || flags.allow_v2 && palette_size != tcc_old.palette_size; }
		
private:
	int reserved[3];

	friend class Environment_Data_Chunk;
	friend class Environment_Game_Mode_Chunk;
	

	
};



// Multi-Palettes for one environment

// Coloured polygons will be set on load-up
// If 15 bit is good-enough, we can use a 32K
// lookup table for the coloured polygons,
// otherwise we'll have to do the slower
// distance comparisons

// We need a new chunk which is a child of the REBENVDT environment data chunk

// this will be a GAMEMODE chunk with children, and there may be more than one
//  in the case where there are several palettes available for the environment

// there will be an identifying string in the gamepalette chunk.
// the "REBENVDT"->lookup_child("GAMPALET") will return a list of these chunks,
// so you can look through them to find the one you want
// There should be some sort of convention/restriction/communication
// about the 

// most of the information will be in its children, which can be any of the following

// ENVPALET

// RIFCHILD(s) - the name of RIF file(s)
//	   this is basically a RIF_Name_Chunk, but named differently so as to distinguish it from its own file's name
//     which should have a REBENVDT chunk with BMPNAMES and/or RIFCHILD(s) (even more RIFs)
//     specifying what non-environment specific bitmaps are used, and probably much more data,
//         - they could be sprites, etc.
//     then all the data could be automatically included (but not loaded) by the inclusion of one file

// ENVTXLIT - texture lighting table,
//     but maybe this should a filename to load if required

// CLRLOOKP - 15-bit r;g;b lookup table to get palette entries for coloured polygons on load
//     can have a table or a reference file

// other relevant chunks to be added later
// we may need some sort of BMPNAMES chunk, for flags about which bmps are quantized, etc.


// HOW TO USE - example (supplied without warranty)
// ----------

/*
File_Chunk * environment = $current environment$;

List<Chunk *> envdatalist = environment->lookup_child("REBENVDT");

if (envdatalist.size())
{
	Environment_Data_Chunk * envdata = (Environment_Data_Chunk *)envdatalist.first_entry();

	List<Chunk *> gamemodelist = envdata->lookup_child("GAMEMODE");

	Environment_Game_Mode_Chunk * selected = 0;
	
	for(LIF<Chunk *> li(&gamemodelist); !li.done(); li.next())
	{
		Environment_Game_Mode_Chunk * try_this = (Environment_Game_Mode_Chunk *) li();
		if (try_this->id_equals((const char *)$your identifier$))
		{
			selected = try_this;
			break;
		}
	}

	if (selected)
	{
		List<Chunk *> bmpgrouplist = selected->lookup_child("RIFCHILD");

		String subdir;
		
		List<Chunk *> rifnamechunk = envdata->lookup_child("RIFFNAME");
		if (rifnamechunk.size())
			subdir = ((RIF_Name_Chunk *) rifnamechunk.first_entry())->rif_name;
		else
			subdir = "empty";

		// load graphics loop
		for (LIF<Chunk *> g(&bmpgrouplist); !g.done(); g.next())
		{
			RIF_Child_Chunk * current = (RIF_Child_Chunk *)g();

			const char * mipmap_0;
			const char * mipmap_n;
			String dir;
			
			if ($loading pgms$)
			{
				dir = (String)"Game-Textures\\" + subdir + "\\" + (String)$your identifier$ + "\\";
				if (*current->filename)
				{
					dir += (String)current->rifname + "\\";
				}
				mipmap_0 = ".pg0";
				mipmap_n = ".pg%1d";
			}
			else // loading bm0 to bm6
			{
				dir = (String)"Generic-Textures\\" + current->rifname + "\\";
				mipmap_0 = ".pg0";
				mipmap_n = ".pg%1d";
			}

			// load graphics-set loop
			for (LIF<BMP_Flags> img(&current->bmps); !img.done(); img.next())
			{
				BMP_Flags bmp = img();

				char * imname = new char[strlen(bmp.filename)+5];
				strcpy(imname,bmp.filename);
				char * dotpos = strrchr(imname,'.');
				if (!dotpos) dotpos = imname + strlen(imname);
				strcpy(dotpos,mipmap_0);
				
				String filename = dir + imname;

				$load image$(filename)

				if (bmp.flags & ChunkBMPFlag_MipMapsQuantized)
				{
					// load mipmaps loop
					for (int i = 1 ; i<=6 ; ++i)
					{
						sprintf(dotpos,mipmap_n,i);
						filename = dir + imname;
						$load image$(filename)
					}
				}
				delete [] imname;
			}
		}
	}
	else
	{
		// your identifier is not valid
	}
}
else
{
	// no data
}



*/


#define GameModeFlag_Editable 0x00000001 // game modes are not editable if they are included from another RIF
#define GameModeFlag_Deleted  0x00000002 // so you can click on cancel and invisibly no updating will be done
#define GameModeFlag_Added    0x00000004 // ditto

class Environment_Game_Mode_Chunk;
class Environment_Game_Mode_Header_Chunk;



class Environment_Game_Mode_Chunk : public Chunk_With_Children
{

public:

	// constructor from buffer
	Environment_Game_Mode_Chunk (Chunk_With_Children * const parent, const char * sdata, size_t const ssize);

	~Environment_Game_Mode_Chunk(){}
															
	Environment_Game_Mode_Header_Chunk * header;
	Environment_Data_Chunk * const envd_parent;
	
	char * ExpandedIdentifier() const; // must delete return value

	inline BOOL operator == (Environment_Game_Mode_Chunk const & m) const;
	inline BOOL operator != (Environment_Game_Mode_Chunk const & m) const;
	// public function for people to check if they have	the game mode they want
	inline BOOL id_equals(const char * s) const;

private:

	friend class Environment_Data_Chunk;
	

	
};

class Environment_Game_Mode_Chunk_Pointer
{
private:
	Environment_Game_Mode_Chunk * p;
	// poor lonely thing has no friends and never will -- aaaahh

private:	
	inline Environment_Game_Mode_Chunk_Pointer(void) : p(0) {};
	friend struct List_Member<Environment_Game_Mode_Chunk_Pointer>;

public:
	// copy constructor from another of this type
	//inline Environment_Game_Mode_Chunk_Pointer(Environment_Game_Mode_Chunk_Pointer const & cp) : p(cp.p) {};
	// cast constructor from C pointer
	inline Environment_Game_Mode_Chunk_Pointer(Environment_Game_Mode_Chunk * const cp) : p(cp) {};
	// cast to C pointer operator
	inline operator Environment_Game_Mode_Chunk * (void) const { return p; }
	// no empty constructor -- p must always be valid

	// equavalence based on contents of pointer, not addresses being the same
	inline BOOL operator == (Environment_Game_Mode_Chunk_Pointer const m) const
	{
		if (!p || !m.p) return 0;
		return *p == *m.p;
	}
	inline BOOL operator != (Environment_Game_Mode_Chunk_Pointer const m) const
	{
		if (!p || !m.p) return 1;
		return *p != *m.p;
	}

};


///////////////////////////////////////////////




class Environment_Game_Mode_Header_Chunk : public Chunk
{
public:
	// constructor from buffer
	Environment_Game_Mode_Header_Chunk (Chunk_With_Children * const parent, const char * pdata, size_t const psize);
	virtual size_t size_chunk ();

	virtual void fill_data_block (char * data_start);

	inline BOOL id_equals(const char * s)
	{
		if (_stricmp(s,mode_identifier))
			return FALSE;
		else
			return TRUE;
	}

	int flags;
	char * mode_identifier;

	inline void add_rif_entry(char const * f)
	{
		char * n = new char[strlen(f)+1];
		strcpy(n,f);
		rif_files.add_entry(n);
	}
	inline const char * get_safe_source(void)
	{
		if (flags & GameModeFlag_Editable)
			return "this.rif";
		if (!rif_files.size())
			return "unknown.rif";
		return rif_files.first_entry();
	}
	
private:

	friend class Environment_Game_Mode_Chunk;
	friend class Cwm_GAMEMODEDlg;
	friend class Cwm_GAMEMODClicked116Dlg;
	friend class GameModeDlg;
	friend class GameModeRifDlg;

	List<char *> rif_files; // where the game mode goes to or comes from
	
	#define ChunkGMod_NumReserved 3
	int reserved[ChunkGMod_NumReserved];

	int version_num;
	
	// deconstructor
	~Environment_Game_Mode_Header_Chunk();


	
};

//////////////////

inline BOOL Environment_Game_Mode_Chunk::operator == (Environment_Game_Mode_Chunk const & m) const
{
	if ((header->flags | m.header->flags) & GameModeFlag_Deleted) return FALSE;
	return !_stricmp(header->mode_identifier,m.header->mode_identifier);
}
inline BOOL Environment_Game_Mode_Chunk::operator != (Environment_Game_Mode_Chunk const & m) const
{
	if ((header->flags | m.header->flags) & GameModeFlag_Deleted) return TRUE;
	return _stricmp(header->mode_identifier,m.header->mode_identifier);
}
inline BOOL Environment_Game_Mode_Chunk::id_equals(const char * s) const
{
	return header->id_equals(s);
}

	
///////////////////////////////////////////////


class RIF_Child_Chunk;

class BMP_Flags
{
public:
	BMP_Flags(void) : filename(0), flags((BMPN_Flags)0), version_num(0), enum_id(0) {}
	BMP_Flags(const char * const fname) : filename(0), flags((BMPN_Flags)0), version_num(0), enum_id(0)
	{
		if (fname)
		{
			filename = new char[strlen(fname)+1];
			strcpy(filename,fname);
		}
	}
	BMP_Flags(BMP_Name const & bn) : filename(0), flags((BMPN_Flags)(bn.flags & COPY_BMPN_FLAGS)), version_num(bn.version_num), enum_id(bn.enum_id)
	{
		if (bn.filename)
		{
			filename = new char[strlen(bn.filename)+1];
			strcpy(filename,bn.filename);
		}
	}
	BMP_Flags(BMP_Flags const & c) : filename(0), flags(c.flags), version_num(c.version_num), enum_id(c.enum_id)
	{
		if (c.filename)
		{
			filename = new char[strlen(c.filename)+1];
			strcpy(filename,c.filename);
		}
	}
	operator BMP_Name () const
	{
		BMP_Name cast(filename);
		cast.flags = flags;
		cast.version_num = version_num;
		cast.enum_id = enum_id;
		return cast;
	}
	~BMP_Flags()
	{
		if (filename) delete[] filename;
	}
	
	BMP_Flags & operator = (BMP_Flags const & c)
	{
		if (filename) delete[] filename;
		if (c.filename)
		{
			filename = new char[strlen(c.filename)+1];
			strcpy(filename,c.filename);
		}
		else
			filename = 0;
		flags = c.flags;
		version_num = c.version_num;
		enum_id = c.enum_id;
		return *this;
	}


	char * filename;
	BMPN_Flags flags;
	#define BMPFLAGS_VERSION_NUM_MASK 0x000fffff
	#define BMPFLAGS_ENUMID_SHIFT 20
	int version_num;
	int enum_id;

	inline BOOL operator == (BMP_Flags const & c) const
	{
		return !_stricmp(filename,c.filename);
	}
	inline BOOL operator != (BMP_Flags const & c) const
	{
		return _stricmp(filename,c.filename);
	}
};

enum RCC_Flags {
	RCCF_EXTERNSHAPE         = 0x00000001, // bitmaps from shape which is in game mode solely because it was an external shape added to the environment with this game mode
	RCCF_FIXEDPALETTE        = 0x00000002, // pgms are quantized with a fixed constant palette
	RCCF_HISTOGRAMEXISTS     = 0x00000004,
	RCCF_HISTOGRAMV2EXISTS     = 0x00000008,

	RCCF_DEFAULT = 0
};

class RIF_Child_Chunk : public Chunk
{
public:

// constructor from buffer
	RIF_Child_Chunk (Chunk_With_Children * const parent, const char * sdata, size_t const ssize);
	~RIF_Child_Chunk();

	virtual size_t size_chunk ();

	virtual void fill_data_block (char * data_start);
	
	int const * GetMD5Val(BMP_Flags const & rcbmp);
	void SetMD5Val(BMP_Flags const & rcbmp, int const * md5id);
	void RemoveMD5Val(char const * bname);
	Bitmap_MD5_Chunk * GetMD5Chunk(char const * bname);
	void CreateMD5Chunk(BMP_Flags const & rcbmp, int const * md5id);
	
	Environment_Game_Mode_Chunk * const egm_parent;
	
   	int version_num;

	char * filename;
	char * rifname;

	inline BOOL operator == (RIF_Child_Chunk const & c) const
	{
		return !strcmp(rifname,c.rifname);
	}
	inline BOOL operator != (RIF_Child_Chunk const & c) const
	{
		return strcmp(rifname,c.rifname);
	}

	List<BMP_Flags> bmps;
	
	RCC_Flags flags;
	
	#define ChunkRIFChild_NumReserved 2
	int reserved[ChunkRIFChild_NumReserved];


private:

	friend class Environment_Game_Mode_Chunk;
	
	
};



class Preset_Palette_Store_Chunk : public Chunk
{
public:

	// constructor from buffer
	Preset_Palette_Store_Chunk (Chunk_With_Children * const parent, char const * sdata, size_t const ssize);
	~Preset_Palette_Store_Chunk ()
	{
		if (rifname) delete[] rifname;
	}

	virtual size_t size_chunk ();

	virtual void fill_data_block (char * data_start);

	int flags;
	int version_num;

	const int reserved1;
	const int reserved2;
	const int reserved3;

	char * rifname;

	inline BOOL operator == (Preset_Palette_Store_Chunk const & c) const
	{
		return !strcmp(rifname,c.rifname);
	}
	inline BOOL operator != (Preset_Palette_Store_Chunk const & c) const
	{
		return strcmp(rifname,c.rifname);
	}

	List<Preset_Palette> pplist;


private:

	friend class Environment_Game_Mode_Chunk;
	

};





///////////////////////////////////////////////



class Coloured_Polygons_Lookup_Chunk : public Chunk
{
public:

	// constructor from buffer
	Coloured_Polygons_Lookup_Chunk (Chunk_With_Children * parent, const char * sdata, size_t ssize);

	~Coloured_Polygons_Lookup_Chunk ();
															
	virtual size_t size_chunk ();

	virtual void fill_data_block (char * data_start);
	
	int flags;
	#define ChunkCPLUFlag_ExternalFile 0x00000001

	#define ChunkCPLU_NumReserved 7
	int reserved[ ChunkCPLU_NumReserved ];

	char * filename;
	unsigned char * table;
	
private:

	friend class Environment_Game_Mode_Chunk;
	friend class Environment_Data_Chunk;
	

};



		
	
#endif // !included


























