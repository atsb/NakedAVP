#ifndef _bmpnames_hpp_
#define _bmpnames_hpp_

#include "chunk.hpp"

// for assert
#define UseLocalAssert No
#include "ourasert.h"
#define assert(x) GLOBALASSERT(x)

enum BMPN_Flags
{
	ChunkBMPFlag_Null                 = 0x00000000, // all flags reset
	ChunkBMPFlag_NotInShape           = 0x00000001, // not a texture map, maybe a sprite or hud graphic
	ChunkBMPFlag_UsesTransparency     = 0x00000002, // transparency_colour defines invisible pixels
	ChunkBMPFlag_RequireGameMipMaps   = 0x00000004, // mip maps are required for the game
	ChunkBMPFlag_RequireToolsMipMaps  = 0x00000008, // mip maps are required for the interface engine
	ChunkBMPFlag_MipMapsExist         = 0x00000010, // internal mip maps are up to date
	ChunkBMPFlag_NotLit               = 0x00000020, // not light sourced (eg. hud, iflag_nolight), so do not put darker colours into palette
	ChunkBMPFlag_FixedPalette         = 0x00000040, // will be quantized once only to a fixed sub-palette of each main palette
	ChunkBMPFlag_Quantized            = 0x00000080, // .PG0 exists which corresponds to the palette
	// See below                        0x00000100
	ChunkBMPFlag_MipMapsQuantized     = 0x00000200, // .PG1-.PG6 exist which correspond to the palette and are mip maps
	ChunkBMPFlag_PP0Exists            = 0x00000400, // internal .PP0 exists
	ChunkBMPFlag_NotInPC              = 0x00000800, // for reduced memory, reduced features on some platforms
	ChunkBMPFlag_NotInSaturn          = 0x00001000, // for reduced memory, reduced features on some platforms
	ChunkBMPFlag_NotInPlaystation     = 0x00002000, // for reduced memory, reduced features on some platforms
	ChunkBMPFlag_BM0Exists            = 0x00004000, // 256 colour palettized texture for hw accelerators exists
	ChunkBMPFlag_BMnsExist            = 0x00008000, // mip mapped versions of 256 colour palettized texture exist
	ChunkBMP_Dither                   = 0x00070000, //
	ChunkBMP_DitherFloyd              = 0x00010000, //
	ChunkBMP_DitherFloydDamp1         = 0x00020000, //
	ChunkBMP_DitherFloydDamp2         = 0x00030000, // 3 bits to control the type of error diffusion (if any)
	ChunkBMP_DitherJarvis             = 0x00040000, //
	ChunkBMP_DitherJarvisDamp1        = 0x00050000, //
	ChunkBMP_DitherJarvisDamp2        = 0x00060000, //
	ChunkBMPFlag_RqQuantLUV           = 0x00080000, // Remap in LUV colour space
	ChunkBMPFlag_HistogramExists      = 0x00100000, // used by cencon in palette generation - help by outputting .HST files
	ChunkBMPFlag_HistogramV2Exists    = 0x00200000, // used by cencon in palette generation - help by outputting .HS2 files (non-lit histograms for lit bitmaps with conceptual tlt palette)
	ChunkBMPFlag_CopiedGenMipMaps     = 0x00400000, // mip map HW generic textures have been copied to final dest
	ChunkBMPFlag_CopiedGenBaseTex     = 0x00800000, // base (non-mip) HW generic textures have been copied to final dest
	
	ChunkBMPFlag_IFF                  = 0x01000000, // a very important flag indeed:
		// when this flag is set, the file is an IFF file and the filename stores a 
		// full relative path from the 'textures-root' directory for the project
		// all other flags are complete bollocks when this flag is set
		// (except for the NotIn.... flags)
		// all this data will be in the file itself (transparency data anyway,,,)
		// the file can be updated without the use of cencon, so for this reason
		// I'll store the widths and heights where the transparent colour used
		// to be. I'll also provide member-access functions to access this data
		// which will check the flag is correct

// This flag will be set on newer RIF files, because older ones will have priorities of 0 which is not ideal!
// When a chunk with these flag not set is detected, default values are filled in and the old values are not used.
	ChunkBMPFlag_PriorityAndTransparencyAreValid = 0x00000100

};

//I have removed transparency from the default flags at the request of the artists

// default flags for new bitmaps
#define DEFAULT_BMPN_FLAGS ((BMPN_Flags) ( \
        ChunkBMPFlag_PriorityAndTransparencyAreValid | \
        ChunkBMPFlag_RequireToolsMipMaps | /* test */ \
        ChunkBMPFlag_RequireGameMipMaps ))

// user flags that should correspond for corresponding bitmaps
#define COPY_BMPN_FLAGS ((BMPN_Flags) ( \
        ChunkBMPFlag_NotInShape | \
        ChunkBMPFlag_UsesTransparency | \
        ChunkBMPFlag_RequireToolsMipMaps | \
        ChunkBMPFlag_RequireGameMipMaps | \
        ChunkBMPFlag_MipMapsExist | \
        ChunkBMPFlag_BMnsExist | \
        ChunkBMPFlag_BM0Exists | \
        ChunkBMPFlag_PP0Exists | \
        ChunkBMPFlag_NotLit | \
        ChunkBMPFlag_FixedPalette | \
        ChunkBMPFlag_NotInPC | \
        ChunkBMPFlag_NotInSaturn | \
        ChunkBMPFlag_NotInPlaystation | \
        ChunkBMP_Dither | \
        ChunkBMPFlag_RqQuantLUV | \
        ChunkBMPFlag_IFF))

// flags that when changed require requantizing
#define CHECKMODIFY_BMPN_FLAGS ((BMPN_Flags) ( \
        ChunkBMPFlag_UsesTransparency | \
        ChunkBMPFlag_FixedPalette /* not sure */ | \
        ChunkBMP_Dither | \
        ChunkBMPFlag_RqQuantLUV ))

// flags to reset if a bitmap needs requantizing
#define QUANTIZED_BMPN_FLAGS ((BMPN_Flags) ( \
        ChunkBMPFlag_Quantized | \
        ChunkBMPFlag_MipMapsQuantized ))

#define COMPLETED_BMPN_FLAGS ((BMPN_Flags) ( \
		ChunkBMPFlag_CopiedGenBaseTex | \
		ChunkBMPFlag_CopiedGenMipMaps | \
		QUANTIZED_BMPN_FLAGS ))

#define DEFAULT_BMPN_PRIORITY 6


extern void Palette_Outdated(Chunk_With_Children * parent); // decalred here, defined in chunkpal to avoid extra compiler dependencies
extern void FixedPalette_Outdated(Chunk_With_Children * parent); // decalred here, defined in chunkpal to avoid extra compiler dependencies
extern BOOL IsFixedPalette(Chunk_With_Children * parent);


class BMP_Name
{
public:

	BMP_Name(const char * fname, int const gbnc_version);
	~BMP_Name();

	BMP_Name(const BMP_Name &);
	const BMP_Name & operator=(const BMP_Name &);
	
	char * filename;
	
	BMPN_Flags flags;
	int index;
	int version_num;
	int enum_id;
	#define BMPNAME_PARENT_VER_SHIFT 8
	// version num contains bmp version num (incremental on update)
	// and Global_BMP_Name_Chunk version (at the time of creation) num shifted up
	// This is so that if a bitmap is removed and then added, its
	// version num will still be greater than that of the removed version
	#define BMPNAME_VERSION_NUM_MASK 0x000fffff
	#define BMPNAME_ENUMID_SHIFT 20
	// the top 12 bits of the previously spare data item (data1)
	// contain an enumeration constant (max 4095)
	// and the bottom 20 bits are available for version numbers
	// a bit cramped and not ideal, but we are running out of storage space
	// there are still two bytes free(0) in the priority data

	#define MAX_PC_PRIORITY 0xff
	#define MAX_PSX_PRIORITY 0xff
	inline int get_pc_priority(void) const { return priority & 0xff; }
	inline int get_psx_priority(void) const { return priority >> 8 & 0xff; }
	inline void set_pc_priority(int const p) { priority &= ~0xff; priority |= p & 0xff; }
	inline void set_psx_priority(int const p) { priority &= ~0xff00; priority |= (p & 0xff) << 8; }
	
	friend BOOL operator==(const BMP_Name &o1, const BMP_Name &o2);
	friend BOOL operator!=(const BMP_Name &o1, const BMP_Name &o2);
	
	BMP_Name()
	: filename(0), flags((BMPN_Flags)DEFAULT_BMPN_FLAGS), index(0), version_num (0), priority (DEFAULT_BMPN_PRIORITY), transparency_colour_union(0) {}
	
	void Validate(void);
	
	unsigned GetTranspRedVal() const;
	unsigned GetTranspGreenVal() const;
	unsigned GetTranspBlueVal() const;
	unsigned GetWidth() const;
	unsigned GetHeight() const;
	
	void SetTranspRedVal(unsigned);
	void SetTranspGreenVal(unsigned);
	void SetTranspBlueVal(unsigned);
	void SetWidth(unsigned);
	void SetHeight(unsigned);
	
	// copy all data
	void CopyUnionDataFrom(BMP_Name const & rBmp);
	bool DifferentTransparencyColour(BMP_Name const & rBmp) const;
	
	enum
	{	
		MAXVAL_RGB = 0xff,
		MAXVAL_WH = 0xffff
	};

private:

	int priority; // contains pc palettized mode palette generating priority as well as 16/256 colour priority for attahced palettes
	
	enum
	{
		SHIFT_R = 22,
		SHIFT_G = 12,
		SHIFT_B = 2,
		
		SHIFT_W = 0,
		SHIFT_H = 16
	};
	
	unsigned transparency_colour_union; // == r<<22 + g<<12 + b << 2 ; r,g,b <- [0..255], but don't assume this'll always be the case
		// or H<<16 + W

	BMP_Name(const char * fname);
	// initial part of constructor from buffer.
	// GBNC and BLSC loaders find the rest of the data
	// and put it into the BMP_Name object constructed with this constructor

	friend class Chunk_With_BMPs;
	friend class BMP_Flags;
};

// functions to access the union transparency_colour_union which isn't a real C/C++ union
// they will check that you're performing valid accesses (ie. using the right part of the union)
// if you're a friend class, please ensure you use these access functions or know what you're doing
inline unsigned BMP_Name::GetTranspRedVal() const
{
	assert(!(flags & ChunkBMPFlag_IFF)); // not available for IFF files - it's in the file
	return transparency_colour_union >> SHIFT_R & MAXVAL_RGB;
}

inline unsigned BMP_Name::GetTranspGreenVal() const
{
	assert(!(flags & ChunkBMPFlag_IFF)); // not available for IFF files - it's in the file
	return transparency_colour_union >> SHIFT_G & MAXVAL_RGB;
}

inline unsigned BMP_Name::GetTranspBlueVal() const
{
	assert(!(flags & ChunkBMPFlag_IFF)); // not available for IFF files - it's in the file
	return transparency_colour_union >> SHIFT_B & MAXVAL_RGB;
}

inline unsigned BMP_Name::GetWidth() const
{
	assert(flags & ChunkBMPFlag_IFF); // only available for IFF files - required since they can be modified externally
	return transparency_colour_union >> SHIFT_W & MAXVAL_WH;
}

inline unsigned BMP_Name::GetHeight() const
{
	assert(flags & ChunkBMPFlag_IFF); // only available for IFF files - required since they can be modified externally
	return transparency_colour_union >> SHIFT_H & MAXVAL_WH;
}

inline void BMP_Name::SetTranspRedVal(unsigned v)
{
	assert(!(flags & ChunkBMPFlag_IFF)); // not available for IFF files - it's in the file
	assert(v<=MAXVAL_RGB); // sensible value
	
	transparency_colour_union &= ~(MAXVAL_RGB << SHIFT_R);
	transparency_colour_union |= v << SHIFT_R;
}

inline void BMP_Name::SetTranspGreenVal(unsigned v)
{
	assert(!(flags & ChunkBMPFlag_IFF)); // not available for IFF files - it's in the file
	assert(v<=MAXVAL_RGB); // sensible value
	
	transparency_colour_union &= ~(MAXVAL_RGB << SHIFT_G);
	transparency_colour_union |= v << SHIFT_G;
}

inline void BMP_Name::SetTranspBlueVal(unsigned v)
{
	assert(!(flags & ChunkBMPFlag_IFF)); // not available for IFF files - it's in the file
	assert(v<=MAXVAL_RGB); // sensible value
	
	transparency_colour_union &= ~(MAXVAL_RGB << SHIFT_B);
	transparency_colour_union |= v << SHIFT_B;
}

inline void BMP_Name::SetWidth(unsigned v)
{
	assert(flags & ChunkBMPFlag_IFF); // only available for IFF files - required since they can be modified externally
	assert(v<=MAXVAL_WH); // sensible value
	
	transparency_colour_union &= ~(MAXVAL_WH << SHIFT_W);
	transparency_colour_union |= v << SHIFT_W;
}

inline void BMP_Name::SetHeight(unsigned v)
{
	assert(flags & ChunkBMPFlag_IFF); // only available for IFF files - required since they can be modified externally
	assert(v<=MAXVAL_WH); // sensible value
	
	transparency_colour_union &= ~(MAXVAL_WH << SHIFT_H);
	transparency_colour_union |= v << SHIFT_H;
}

inline void BMP_Name::CopyUnionDataFrom(BMP_Name const & rBmp)
{
	assert((flags & ChunkBMPFlag_IFF)==(rBmp.flags & ChunkBMPFlag_IFF));
	
	transparency_colour_union = rBmp.transparency_colour_union;
}

inline bool BMP_Name::DifferentTransparencyColour(BMP_Name const & rBmp) const
{
	assert(!(flags & ChunkBMPFlag_IFF) && !(rBmp.flags & ChunkBMPFlag_IFF));
	
	return transparency_colour_union != rBmp.transparency_colour_union;
}


///////////////////////////////////////////////

class BMP_Names_ExtraData;
class Bitmap_MD5_Chunk;

class Chunk_With_BMPs : public Chunk
{
public:

	int max_index;
	
	List<BMP_Name> bmps;

	virtual int get_version_num(void);
	virtual void set_version_num(int);
	virtual void inc_version_num(void);
	virtual BMP_Names_ExtraData * GetExtendedData(void);

	virtual int const * GetMD5Val(BMP_Name const & rcbmp);
	virtual void SetMD5Val(BMP_Name const & rcbmp, int const * md5id);
	virtual void RemoveMD5Val(char const * bname);

	//friend class BMP_Group; // for cencon
	//friend class BMP_Info; // for cencon
	
protected:
	virtual Bitmap_MD5_Chunk * GetMD5Chunk(char const * bname) = 0;
	virtual void CreateMD5Chunk(BMP_Name const & rcbmp, int const * md5id) = 0;

	virtual size_t size_chunk ();
	virtual void fill_data_block (char * data_start);

	Chunk_With_BMPs (Chunk_With_Children * parent, const char * const ident) : Chunk(parent,ident), max_index(0) {}
	Chunk_With_BMPs (Chunk_With_Children * parent, const char * const ident, const char * sdata, size_t ssize);

};


class Global_BMP_Name_Chunk : public Chunk_With_BMPs
{
public:

	Global_BMP_Name_Chunk (Chunk_With_Children * parent)
	: Chunk_With_BMPs (parent, "BMPNAMES")
	{}
	// constructor from buffer
	Global_BMP_Name_Chunk (Chunk_With_Children * parent, const char * sdata, size_t ssize)
	: Chunk_With_BMPs (parent, "BMPNAMES", sdata, ssize) {}
private:
	virtual Bitmap_MD5_Chunk * GetMD5Chunk(char const * bname);
	virtual void CreateMD5Chunk(BMP_Name const & rcbmp, int const * md5id);

	friend class Environment_Data_Chunk;
	

	

};


class Bitmap_List_Store_Chunk : public Chunk_With_BMPs
{
public:

	Bitmap_List_Store_Chunk (Chunk_With_Children * parent)
	: Chunk_With_BMPs (parent, "BMPLSTST")
	{}

	// constructor from buffer
	// not private, so that it is easy to get to
	Bitmap_List_Store_Chunk (Chunk_With_Children * parent, const char * sdata, size_t ssize)
	: Chunk_With_BMPs (parent, "BMPLSTST", sdata, ssize) {}

private:
	virtual Bitmap_MD5_Chunk * GetMD5Chunk(char const * bname);
	virtual void CreateMD5Chunk(BMP_Name const & rcbmp, int const * md5id);

	friend class Shape_External_File_Chunk;
	
};
	


class BMP_Names_Version_Chunk : public Chunk
{
public:

	BMP_Names_Version_Chunk (Chunk_With_Children * parent)
	: Chunk (parent, "BMNAMVER"), version_num (0)
	{}
	// constructor from buffer
	BMP_Names_Version_Chunk (Chunk_With_Children * parent, const char * sdata, size_t /*ssize*/)
	: Chunk (parent, "BMNAMVER"), version_num(*(int *)sdata)
	{}

	virtual size_t size_chunk ();

	virtual void fill_data_block (char * data_start);

private:

	int version_num;

	friend class Environment_Data_Chunk;
	friend class Chunk_With_BMPs;
	friend class Shape_External_File_Chunk;
	friend class Sprite_Header_Chunk;


};	

enum GlobalBMPFlags
{
	GBF_FIXEDPALETTE      = 0x00000001,
	GBF_SPRITE            = 0x00000002,
	GBF_HISTOGRAMEXISTS   = 0x00000004,
	GBF_HISTOGRAMV2EXISTS = 0x00000008,

	GBF_NONE = 0,
	
	// IMPORTANT
	// since enums are not guaranteed to assume any particular
	// storage class, code compiled on different compilers or
	// with different settings may result in enums to be written
	// to the data block as a char and read back in as an int,
	// with the three most significant bytes containing junk.
	// THIS MASK MUST BE KEPT UP TO DATE AS THE ENUM IS EXTENDED;
	// ALSO ENSURE THAT NEW FILES LOADED INTO OLD SOFTWARE WILL
	// NOT HAVE THEIR ENUM VALUE OVER-MASKED; THE MASK IS ONLY
	// HERE TO ATTEMPT TO REMOVE PROBLEMS FROM FILES MADE
	// PRIOR TO ITS INTRODUCTION
	GBF_MASK = 0x000000ff
};

class BMP_Names_ExtraData
{
public:
	GlobalBMPFlags flags;
protected:
	int reserved[12];
};


class BMP_Names_ExtraData_Chunk : public Chunk, public BMP_Names_ExtraData
{
public:

	BMP_Names_ExtraData_Chunk (Chunk_With_Children * parent)
	: Chunk (parent, "BMNAMEXT")
	{
		for (int i=0; i<12; ++i) reserved[i] = 0;
		flags = GBF_NONE;
	}

	// constructor from buffer
	BMP_Names_ExtraData_Chunk (Chunk_With_Children * parent, const char * sdata, size_t /*ssize*/)
	: Chunk (parent, "BMNAMEXT")
	{
		flags = (GlobalBMPFlags)(*(int *)sdata & GBF_MASK);
		sdata += 4;
		for (int i=0; i<12; ++i, sdata+=4) reserved[i] = *(int *)sdata;
	}
	
private:

	virtual size_t size_chunk ();

	virtual void fill_data_block (char * data_start);

	friend class Environment_Data_Chunk;
	friend class Shape_External_File_Chunk;
	friend class Sprite_Header_Chunk;


};	


class External_Shape_BMPs_Store_Chunk : public Chunk_With_BMPs, protected BMP_Names_ExtraData
{
public:
	char * rifname; // to match one in RIF_Child_Chunk
	char * shapename; // matches rif name of original shape

	External_Shape_BMPs_Store_Chunk (Chunk_With_Children * parent, char const * rifn, char const * shapen);
	External_Shape_BMPs_Store_Chunk (Chunk_With_Children * parent, const char * sdata, size_t ssize);
	~External_Shape_BMPs_Store_Chunk();

	virtual int get_version_num(void) { return version_num; }
	virtual void set_version_num(int v) { version_num = v; }
	virtual void inc_version_num(void) { ++version_num; }
	virtual BMP_Names_ExtraData * GetExtendedData(void) { return this; }
	
private:
	virtual Bitmap_MD5_Chunk * GetMD5Chunk(char const * bname);
	virtual void CreateMD5Chunk(BMP_Name const & rcbmp, int const * md5id);

	int version_num;
	
	virtual size_t size_chunk()
	{
		return chunk_size = Chunk_With_BMPs::size_chunk() + ((rifname ? strlen(rifname) : 0) + (shapename ? strlen(shapename) : 0)+2 +3&~3) + 56;
	}

	virtual void fill_data_block(char * data_start);


	
	friend class Environment_Game_Mode_Chunk;
};


// for dealing with matching images

enum IDscFlags
{
	IDSCF_SPRITE       = 0x00000001, // image is in a sprite
	IDSCF_INCLUDED     = 0x00000002, // image is from another rif file
	IDSCF_FIXEDPALETTE = 0x00000004, // image has pgms for fixed palette
	IDSCF_SUBSHAPE     = 0x00000008, // image for shape included from another file

	IDSCF_0 = 0,
	
	// IMPORTANT
	// since enums are not guaranteed to assume any particular
	// storage class, code compiled on different compilers or
	// with different settings may result in enums to be written
	// to the data block as a char and read back in as an int,
	// with the three most significant bytes containing junk.
	// THIS MASK MUST BE KEPT UP TO DATE AS THE ENUM IS EXTENDED;
	// ALSO ENSURE THAT NEW FILES LOADED INTO OLD SOFTWARE WILL
	// NOT HAVE THEIR ENUM VALUE OVER-MASKED; THE MASK IS ONLY
	// HERE TO ATTEMPT TO REMOVE PROBLEMS FROM FILES MADE
	// PRIOR TO ITS INTRODUCTION
	IDSCF_MASK = 0x000000ff
};

class ImageDescriptor
{
public:
	// constructos;
	ImageDescriptor();
	ImageDescriptor(ImageDescriptor const &);
	ImageDescriptor(IDscFlags, char const * fname, char const * rname = 0, char const * xname = 0);
	~ImageDescriptor();
	ImageDescriptor & operator = (ImageDescriptor const &);

	// operators
	BOOL operator == (ImageDescriptor const &) const;
	inline BOOL operator != (ImageDescriptor const & id2) const
		{ return ! operator == (id2); }
	
	// members
	IDscFlags flags;

	char * filename; // name.bmp
	char * rifname; // only if IDSCF_INCLUDED is set
	char * fixrifname; // only if IDSCF_FIXEDPALETTE is set

private:
	// I/O
	ImageDescriptor(char const * datablock);
	size_t Size() const;
	void WriteData(char * datablock) const;

	friend class MatchingImages;
	
	int spares[3];
};

class MatchingImages
{
public:
	// constructos;
	MatchingImages() {}
	MatchingImages(ImageDescriptor const & _load, ImageDescriptor const & _insteadof);

	// operators
	inline BOOL operator == (MatchingImages const & m2)
		{ return load == m2.load && insteadof == m2.insteadof; }
	inline BOOL operator != (MatchingImages const & m2)
		{ return load != m2.load || insteadof != m2.insteadof; }

	// members
	ImageDescriptor load;
	ImageDescriptor insteadof;

private:

	// I/O
	MatchingImages(char const * datablock);
	size_t Size() const;
	void WriteData(char * datablock) const;
	
	friend class Matching_Images_Chunk;
	
	int spares[3];
	
};

enum MICFlags
{
	MICF_0 = 0,

	MICF_FIXEDPALETTE = 0x00000001,
	
	// IMPORTANT
	// since enums are not guaranteed to assume any particular
	// storage class, code compiled on different compilers or
	// with different settings may result in enums to be written
	// to the data block as a char and read back in as an int,
	// with the three most significant bytes containing junk.
	// THIS MASK MUST BE KEPT UP TO DATE AS THE ENUM IS EXTENDED;
	// ALSO ENSURE THAT NEW FILES LOADED INTO OLD SOFTWARE WILL
	// NOT HAVE THEIR ENUM VALUE OVER-MASKED; THE MASK IS ONLY
	// HERE TO ATTEMPT TO REMOVE PROBLEMS FROM FILES MADE
	// PRIOR TO ITS INTRODUCTION
	MICF_MASK = 0x000000ff
};

class Matching_Images_Chunk : public Chunk
{
public:
	// constructors
	Matching_Images_Chunk(Chunk_With_Children * parent)	: Chunk(parent,"MATCHIMG"), flags(MICF_0)
		{ spares[0]=0; spares[1]=0; } // empty list
	// I/O
	Matching_Images_Chunk(Chunk_With_Children * parent, char const * datablock, size_t);
	// members
	List<MatchingImages> mlist;

	MICFlags flags;

	// methods
	ImageDescriptor const & GetLoadImage(ImageDescriptor const &);

private:
	int spares[2];


	virtual size_t size_chunk();
	virtual void fill_data_block(char * data_start);

	friend class Environment_Data_Chunk;
	friend class Environment_Game_Mode_Chunk;
};

enum BMPMD5_Flags
{
	BMD5F_0 = 0,

	BMD5F_SPRITE = 0x00000001,
	
	// IMPORTANT
	// since enums are not guaranteed to assume any particular
	// storage class, code compiled on different compilers or
	// with different settings may result in enums to be written
	// to the data block as a char and read back in as an int,
	// with the three most significant bytes containing junk.
	// THIS MASK MUST BE KEPT UP TO DATE AS THE ENUM IS EXTENDED;
	// ALSO ENSURE THAT NEW FILES LOADED INTO OLD SOFTWARE WILL
	// NOT HAVE THEIR ENUM VALUE OVER-MASKED; THE MASK IS ONLY
	// HERE TO ATTEMPT TO REMOVE PROBLEMS FROM FILES MADE
	// PRIOR TO ITS INTRODUCTION
	BMD5F_MASK = 0x000000ff
};

class Bitmap_MD5_Chunk : public Chunk
{
public :
	Bitmap_MD5_Chunk(Chunk_With_Children * parent, char const * datablock, size_t);
private:
	Bitmap_MD5_Chunk(Chunk_With_Children * parent, int const * md5id, BMP_Name const & rcbmp, char const * rname = 0, char const * sname = 0);
	~Bitmap_MD5_Chunk();
	int md5_val[4];
	char * bmpname;
	char * rifname;
	char * shapename;
	BMPMD5_Flags flags;
	int version_num;

	int spare;
	
	virtual size_t size_chunk();
	virtual void fill_data_block(char * data_start);

	// your parents are your friends
	friend class Environment_Data_Chunk;
	friend class Environment_Game_Mode_Chunk;
	friend class Shape_External_File_Chunk;
	friend class Sprite_Header_Chunk;
	// some of your siblings are friends as well
	friend class Chunk_With_BMPs;
	friend class Global_BMP_Name_Chunk;
	friend class Bitmap_List_Store_Chunk;
	friend class External_Shape_BMPs_Store_Chunk;
	friend class RIF_Child_Chunk;
};



#endif
