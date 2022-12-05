// PCMENUS - pc specific menus for avp - eg options, video mode, etc
// also contains functions for setting video mode, etc. in conjunction
// with these menus

/*****************/
/* INCLUDE FILES */
/*****************/

#include <math.h> // for sqrt
#include <windows.h> // for SetCursor
#include <string.h> // strcat, strcpy, etc

#include "3dc.h" // for directdraw stuff
#include "chnktexi.h" // for requesting textures on powers of two
#include "d3_func.h"

//#include "menudefs.h" // general menu stuff
//#include "menugfx.h" // menu graphics stuff
#include "module.h" // required by gamedef
#include "gamedef.h" // for IDemand...
#include "pcmenus.h" // enums and functions for these menus
#include "ourasert.h"
#include "heap_tem.hpp" // so I can have sorted lists of video modes
#include "list_tem.hpp" // for linked lists as and when required

#include "dxlog.h"

#include "scstring.hpp"


extern "C" 
{
	#include "font.h" // for the font stuff
	#include "language.h" /* KJL 14:43:53 05/04/97 - language internationalization code */
	#include "gameplat.h" // for AVP_ChangeDisplayMode
	#include "hudgfx.h" // for minimizing/restoring hud gfx
	#include "frustrum.h"

	#include "stratdef.h" // needed for usr_io.h
	#include "usr_io.h" // for the PlayerInputConfig's
	#include "videomodes.h"

/**********************/
/* EXTERNAL FUNCTIONS */
/**********************/

	extern void DrawInternationalizedString(MENU_TEXT_ITEM *itemPtr, int highlighted);
	extern void DrawMenuBackdrop(void);
 	extern int IDemandSelect(void); // not in any header file!!!

/**********************/
/* EXTERNAL C GLOBALS */
/**********************/

	extern int NumAvailableVideoModes; // ditto!!!
	extern VIDEOMODEINFO AvailableVideoModes[]; // ditto - sort you globals out Chris&Neal!!!
	extern LPDIRECTDRAWSURFACE lpDDSBack;      // DirectDraw back surface
	extern unsigned char KeyboardInput[];
	extern CONTROL_METHODS ControlMethods;
	extern D3DINFO d3d;
	extern BOOL D3DHardwareAvailable;
	extern int TotalVideoMemory;
	extern int DXMemoryMode;
	extern BOOL BilinearTextureFilter;
	extern BOOL really_32_bit;
	extern SHAPEHEADER **mainshapelist;
	extern SCREENDESCRIPTORBLOCK ScreenDescriptorBlock;
	extern int VideoMode;
	extern int VideoModeType;
	extern int VideoModeTypeScreen;
	extern int ScanDrawMode;
    extern unsigned char AttemptVideoModeRestart;
	extern unsigned char TestPalette[];
	extern unsigned char LPTestPalette[]; /* to cast to lp*/
	extern int **ShadingTableArray;
	extern unsigned char **PaletteShadingTableArray;
   

	extern void SaveKeyConfiguration();
	
	extern BOOL g_bMustRedrawScreen;

	#define D3TF_FIRST D3TF_4BIT
	#define D3TF_DEFAULT D3TF_4BIT
	enum TexFmt { D3TF_4BIT, D3TF_8BIT, D3TF_16BIT, D3TF_32BIT, D3TF_MAX } d3d_desired_tex_fmt = D3TF_DEFAULT;
}
	
/***************************/
/* PRIVATE MACRO-CONSTANTS */
/***************************/

#define DISABLE_VIDEOCARD_ANALYSIS 0 // presently still unstable
#define FORCE_TEXDIMS_POWER_OF_TWO 1 // full support for determining if this is necessary not yet implemented

// some random random seeds so that we always create the same random texture
#define RND_SEED1 4479
#define RND_SEED2 22408
#define RND_SEED3 14
#define RND_SEED4 5610

/***************************/
/* PRIVATE MACRO-FUNCTIONS */
/***************************/

/*****************/
/* PRIVATE ENUMS */
/*****************/

enum ImageSizeIdx
{
	ISI_100PC = 0,
	ISI_75PC,
	ISI_50PC,
	ISI_MAX
};

enum ImageSizeRestrictionIdx
{
	ISRI_UNIT4 = 0,
	ISRI_POWER2,
	ISRI_SQUARE,
	ISRI_SQUAREANDPOWER2,
	ISRI_MAX
};

enum D3DOption
{
	VM3_SCANDRAW = 0,
	VM3_D3D = 1,

	VM3_MAX = 2
};


/**********************/
/* PRIVATE STRUCTURES */
/**********************/

class VidModeInfo : public VIDEOMODEINFO
{
	// 2/4/98 DHM: Added a lazily-constructed description string, which you
	// can get references to.

public:
	VidModeInfo(VIDEOMODEINFO const & vmi)
		: VIDEOMODEINFO(vmi),pSCString_Description(NULL) {}
	VidModeInfo()
		{ Width=0; Height=0; ColourDepth=0; pSCString_Description= NULL;}
	VidModeInfo(int w,int h,int d)
		{ Width=w; Height=h; ColourDepth=d; pSCString_Description= NULL;}
	~VidModeInfo()
	{
		if (pSCString_Description)
		{
			pSCString_Description -> R_Release();
		}
	}
	// Copy constructor potentially needs to add a reference:
	VidModeInfo(const VidModeInfo& vmi)
	{
		Width=vmi.Width; Height=vmi.Height; ColourDepth=vmi.ColourDepth;
		pSCString_Description = vmi.pSCString_Description;
		if (pSCString_Description)
		{
			pSCString_Description -> R_AddRef();
		}
	}

	// Assignment operator potentially needs to deal with references:
	VidModeInfo& operator=(const VidModeInfo& vmi)
	{
		Width=vmi.Width; Height=vmi.Height; ColourDepth=vmi.ColourDepth;
		SCString* pSCString_Description_Old = pSCString_Description;

		pSCString_Description = vmi.pSCString_Description;
		if (pSCString_Description)
		{
			pSCString_Description -> R_AddRef();
		}
		if (pSCString_Description_Old)
		{
			pSCString_Description_Old -> R_Release();
		}		
		return *this;
	}
	
	// to sort the modes for nice display
	int operator < (VidModeInfo const & vmi2) const
		{
			return ColourDepth==vmi2.ColourDepth
				? Width==vmi2.Width
					? Height<vmi2.Height
					: Width<vmi2.Width
				: ColourDepth<vmi2.ColourDepth
			;
		}
	// comparisons
	int operator == (VidModeInfo const & vmi2) const
		{
			return ColourDepth==vmi2.ColourDepth
				&& Width==vmi2.Width
				&& Height==vmi2.Height;
		}
	int operator != (VidModeInfo const & vmi2) const
		{ return ! operator == (vmi2); }
		
	// filter for D3D video modes
	int IsForD3D() const
		{
			if (ColourDepth<=8 || Width>MaxScreenWidth) // no 8-bit support - it looks crap in 3-3-2!! & look at MaxScreenWidth
				return 0;
			else
			if (ColourDepth<=16)
				if (d3d.ThisDriver.dwDeviceRenderBitDepth & DDBD_16)
					return 1;
				else
					return 0;
			else
			if (ColourDepth<=24)
				if (d3d.ThisDriver.dwDeviceRenderBitDepth & DDBD_24)
					return 1;
				else
					return 0;
			else
			if (ColourDepth<=32)
				if (d3d.ThisDriver.dwDeviceRenderBitDepth & DDBD_32)
					return 1;
				else
					return 0;
			else
			// ColourDepth>32
				return 0;
			
		}
	// filter for scan draw video modes
	int IsForScanDraw() const
		{ return ColourDepth<=8 && Width<=MaxScreenWidth; }

	// Lazily-constructed descriptive string:
	SCString* GetDescription(void)
	{
		if (!pSCString_Description)
		{
			MakeDescription();
		}
		GLOBALASSERT(pSCString_Description);

		pSCString_Description -> R_AddRef();

		return pSCString_Description;
	}
private:
	SCString* pSCString_Description;
	void MakeDescription(void)
	{
		GLOBALASSERT(NULL==pSCString_Description);
		
		SCString* pSCString_W = new SCString(Width);
		SCString* pSCString_H = new SCString(Height);
		SCString* pSCString_CD = new SCString(ColourDepth);
		SCString* pSCString_X = new SCString("X");
			// LOCALISEME

		// Construct string of the form "<width>X<height>X<bitdepth>" with a possible
		// trailing " ?" for more dubious modes:
		SCString* pSCString_Temp = new SCString
		(
			pSCString_W,
			pSCString_X,
			pSCString_H,
			pSCString_X,
			pSCString_CD
		);

		#if 1
		pSCString_Description = pSCString_Temp;
		#else
		if (notreporting_vga && *vmi==vga_mode)
		{
			SCString* pSCString_Questionable = new SCString
			(
				" ?"
			);	// LOCALISEME

			pSCString_Description = new SCString
			(
				pSCString_Temp,
				pSCString_Questionable
			);

			pSCString_Questionable -> R_Release();
			pSCString_Temp -> R_Release();
		}
		else
		{
			pSCString_Description = pSCString_Temp;
		}
		#endif
	

		pSCString_X -> R_Release();
		pSCString_CD -> R_Release();
		pSCString_H -> R_Release();
		pSCString_W -> R_Release();
	}
};

/*************************/
/* PRIVATE CONSTANT DATA */
/*************************/


static int const hw_max_num_texels [ISRI_MAX] [ITI_MAX] [ISI_MAX] =
{
	//   fullsize 3/4 size 1/2 size

	// ISRI_UNIT4
	{
		{       0,       0,       0 }, // hud
		{  327680,  184320,   81920 }, // texture
		{  900000,  506250,  225000 } // sprites
	},
	// ISRI_POWER2,
	{
		{       0,       0,       0 },
		{  327680,  327680,   81920 },
		{ 1350000,  675000,  337500 }
	},
	// ISRI_SQUARE,
	{
		{       0,       0,       0 },
		{  327680,  184320,   81920 },
		{ 1350000,  759375,  337500 }
	},
	// ISRI_SQUAREANDPOWER2,
	{
		{       0,       0,       0 },
		{  327680,  327680,   81920 },
		{ 1350000, 1350000,  337500 }
	}
};

static ImageSizeIdx const hw_try_desc_1 [] = { ISI_100PC, ISI_100PC, ISI_100PC };
static ImageSizeIdx const hw_try_desc_2 [] = { ISI_100PC, ISI_100PC,  ISI_75PC };
static ImageSizeIdx const hw_try_desc_2a[] = { ISI_100PC,  ISI_75PC, ISI_100PC };
static ImageSizeIdx const hw_try_desc_3 [] = { ISI_100PC,  ISI_75PC,  ISI_75PC };
static ImageSizeIdx const hw_try_desc_4 [] = { ISI_100PC,  ISI_75PC,  ISI_50PC };
static ImageSizeIdx const hw_try_desc_4a[] = { ISI_100PC,  ISI_50PC,  ISI_75PC };
static ImageSizeIdx const hw_try_desc_5 [] = { ISI_100PC,  ISI_50PC,  ISI_50PC };

static ImageSizeIdx const * hw_try_desc [] =
{
	hw_try_desc_1,
	hw_try_desc_2,
	hw_try_desc_2a,
	hw_try_desc_3,
	hw_try_desc_4,
	hw_try_desc_4a,
	hw_try_desc_5,
	0
};


/*******************/
/* PRIVATE GLOBALS */
/*******************/

static ImageSizeIdx const * desc_textures_to_load; // points to an array of size ITI_MAX

// HARDWARE
static int hw_avail;
static D3DOption d3d_opt;

// VIDEOMODES
static VidModeInfo vidmode[VM3_MAX];
static unsigned int sel_vidmode_index[VM3_MAX]; // a call to intialize this value will be introduced later
                                      // the last video mode should be saved in a file
                                      // and if it is still available, reselected on re-running app
static Ordered_Heap<VidModeInfo> avail_vidmodes[VM3_MAX]; // two lists, zero for not d3d, one for d3d

static int notreporting_vga = 0;
static VidModeInfo vga_mode(320,200,8);

// SHADING
static SHADING sel_shading[VM3_MAX];

// ZBUFFER
static ZBUFFERREQUESTMODES zbufopt[VM3_MAX];
static BOOL zbuf_avail[VM3_MAX];

// MIPMAP
static BOOL mipmap_opt[VM3_MAX];
static BOOL mipmap_avail[VM3_MAX];

// BILINEAR FILTER?
static BOOL bilin_filter_avail;

/* KJL 15:57:04 05/04/97 - I've added these strings here for simplicity's sake.
The	memory could be allocated/deallocated during run-time to save some space,
but it's only a few bytes. They have to be long to contain any required message. */
static char DisplayModeString[16];
static char TextureDepthString[16];

/******************/
/* PUBLIC GLOBALS */
/******************/

SHADING desired_shading = SHADE_GOURAUD;

/*********************/
/* PRIVATE FUNCTIONS */
/*********************/

static inline int BitsPerPixel(D3DTEXTUREFORMAT const * texfmt)
{
	return texfmt->Palette ? texfmt->IndexBPP : texfmt->RedBPP + texfmt->BlueBPP + texfmt->GreenBPP;
}

static inline int BitsPerPixel(TexFmt f)
{
	switch (f)
	{
		case D3TF_4BIT:
			return 4;
		case D3TF_8BIT:
			return 8;
		case D3TF_16BIT:
			return 16;
		case D3TF_32BIT:
			return 32;
		default:
			return 0;
	}
}

static void LoadVideoModeSettings(void)
{
	FILE * fp = fopen("AVPVMOPT.BIN","rb");
	if (!fp) return;


	unsigned long magic;
	fread(&magic,4,1,fp);
	if (magic == *(unsigned long *)"VMOP")
	{
		fread(&vidmode[VM3_SCANDRAW].Width,4,1,fp);
		fread(&vidmode[VM3_SCANDRAW].Height,4,1,fp);
		fread(&vidmode[VM3_SCANDRAW].ColourDepth,4,1,fp);
		fread(&vidmode[VM3_D3D].Width,4,1,fp);
		fread(&vidmode[VM3_D3D].Height,4,1,fp);
		fread(&vidmode[VM3_D3D].ColourDepth,4,1,fp);
		fread(&d3d_opt,4,1,fp);
		fread(&hw_avail,4,1,fp);
		fread(&zbufopt[VM3_SCANDRAW],4,1,fp);
		fread(&zbufopt[VM3_D3D],4,1,fp);
		fread(&d3d_desired_tex_fmt,4,1,fp);
		fread(&sel_shading[VM3_SCANDRAW],4,1,fp);
		fread(&sel_shading[VM3_D3D],4,1,fp);
		fread(&BilinearTextureFilter,4,1,fp);
		fread(&mipmap_opt[VM3_SCANDRAW],4,1,fp);
		fread(&mipmap_opt[VM3_D3D],4,1,fp);
		if (zbufopt[VM3_D3D] != RequestZBufferNever) zbufopt[VM3_D3D]=RequestZBufferAlways;
		if (zbufopt[VM3_SCANDRAW] != RequestZBufferNever) zbufopt[VM3_SCANDRAW]=RequestZBufferAlways;
		if (VM3_SCANDRAW!=d3d_opt) d3d_opt=VM3_D3D; // ensure 0 or 1
		if (d3d_desired_tex_fmt >= D3TF_MAX) d3d_desired_tex_fmt = D3TF_DEFAULT;

  		d3d_desired_tex_fmt=D3TF_8BIT;
		if (sel_shading[VM3_D3D] != SHADE_FLAT) sel_shading[VM3_D3D] = SHADE_GOURAUD;
		if (sel_shading[VM3_SCANDRAW] != SHADE_FLAT) sel_shading[VM3_SCANDRAW] = SHADE_GOURAUD;
		if (BilinearTextureFilter != FALSE) BilinearTextureFilter = TRUE;
		if (mipmap_opt[VM3_D3D] != FALSE) mipmap_opt[VM3_D3D]=TRUE;
		if (mipmap_opt[VM3_SCANDRAW] != FALSE) mipmap_opt[VM3_SCANDRAW]=TRUE;
	}
	fclose(fp);
	return;
}


static size_t IsNotEnoughVidMemForBytes(size_t required)
{
	return 0;
}

static BOOL IsNotEnoughVidMemForScreenDepth(int s_depth)
{
	// bollocks - this is all obsolete - FIXME
	return FALSE;
}
	 

/********************/
/* PUBLIC FUNCTIONS */
/********************/

BOOL PreferTextureFormat(D3DTEXTUREFORMAT const * oldfmt,D3DTEXTUREFORMAT const * newfmt)
{
	int old_bpp = BitsPerPixel(oldfmt);
	int new_bpp = BitsPerPixel(newfmt);
	int cur_bpp = BitsPerPixel(d3d_desired_tex_fmt);
	
	int old_bpp_dif = abs(old_bpp-cur_bpp);
	int new_bpp_dif = abs(new_bpp-cur_bpp);
	if (new_bpp==8 && !newfmt->Palette) return FALSE;

	if (new_bpp_dif < old_bpp_dif) return TRUE;
	if (new_bpp_dif > old_bpp_dif) return FALSE;
	
	if (new_bpp < old_bpp) return TRUE;
	if (new_bpp > old_bpp) return FALSE;
	
	if (newfmt->Palette) return TRUE;
	else return FALSE;
}
#if 1
void InitOptionsMenu(void)
{
	#if debug // Check that this function is only called once
	static int already_called = 0;
	GLOBALASSERT(!already_called);
	already_called=1;
	#endif
	
	// hardware and direct 3d, zbuffering?
	int hw_now_avail = D3DHardwareAvailable ? 1 : 0; // 4Mb cards only?
	
	// find the direct 3d driver we'll be using
	d3d.CurrentDriver = 0;
	for (int i = 0; i < d3d.NumDrivers; ++i)
	{
		if (d3d.Driver[i].Hardware && hw_now_avail)
		{
			d3d.CurrentDriver = i;
			break;
		}
		if (D3DCOLOR_RGB == d3d.Driver[i].Desc.dcmColorModel)
			d3d.CurrentDriver = i;
	}
	
	d3d.ThisDriver = d3d.Driver[d3d.CurrentDriver].Desc;
	
	zbuf_avail[VM3_D3D] = d3d.Driver[d3d.CurrentDriver].ZBuffer ? TRUE : FALSE;
	zbuf_avail[VM3_SCANDRAW] = TRUE;
	
	// not yet implemented properly and never been tested
	mipmap_avail[VM3_D3D] = d3d.ThisDriver.dpcTriCaps.dwTextureFilterCaps & (D3DPTFILTERCAPS_MIPLINEAR|D3DPTFILTERCAPS_MIPNEAREST) ? TRUE : FALSE;
	mipmap_avail[VM3_SCANDRAW] = TRUE;
	
	bilin_filter_avail = d3d.ThisDriver.dpcTriCaps.dwTextureFilterCaps & D3DPTFILTERCAPS_LINEAR ? TRUE : FALSE;

	hw_avail = hw_now_avail;
	d3d_opt = hw_avail ? VM3_D3D : VM3_SCANDRAW;
	
	sel_vidmode_index[VM3_SCANDRAW]=0; // lowest res
	sel_vidmode_index[VM3_D3D]=0;
	
	zbufopt[VM3_SCANDRAW]=RequestZBufferNever;
	zbufopt[VM3_D3D]=hw_avail ? RequestZBufferAlways : RequestZBufferNever;
	
	mipmap_opt[VM3_SCANDRAW] = mipmap_avail[VM3_SCANDRAW];
	mipmap_opt[VM3_D3D] = mipmap_avail[VM3_D3D];
	
	sel_shading[VM3_SCANDRAW]=SHADE_GOURAUD;
	sel_shading[VM3_D3D]=SHADE_GOURAUD;
	
	BilinearTextureFilter = hw_avail && bilin_filter_avail ? TRUE : FALSE;
	
	LoadVideoModeSettings();
	
	#if !ENABLE_MIPMAP_OPTION
	mipmap_opt[VM3_SCANDRAW] = mipmap_avail[VM3_SCANDRAW];
	mipmap_opt[VM3_D3D] = FALSE;
	#endif
	
	if (hw_now_avail != hw_avail) // card removed/added so set default
	{
		hw_avail = hw_now_avail;
		d3d_opt = hw_avail ? VM3_D3D : VM3_SCANDRAW;
		zbufopt[VM3_D3D]=hw_avail ? RequestZBufferAlways : RequestZBufferNever;
		BilinearTextureFilter = hw_avail && bilin_filter_avail ? TRUE : FALSE;
		mipmap_opt[VM3_D3D] = mipmap_avail[VM3_D3D];
	}
	
	// possible hardware video modes
	for (i=0; i<NumAvailableVideoModes; ++i)
	{
		VidModeInfo vmi(AvailableVideoModes[i]);
		
		if (vmi.IsForD3D() && vmi.Height>=384 && vmi.Width>=512)
			avail_vidmodes[VM3_D3D].add_entry(vmi);
	}
	
	// get the indexes of the video modes
	int idx = 0;
	for (OHIF<VidModeInfo> i_vidmode_d3d(&avail_vidmodes[VM3_D3D]); !i_vidmode_d3d.done(); i_vidmode_d3d.next(), ++idx)
	{
		if (i_vidmode_d3d()==vidmode[VM3_D3D])
		{
			sel_vidmode_index[VM3_D3D] = idx;
			break;
		}
	}
	
	RasterisationRequestMode = RequestSoftwareRasterisation;
#if 0
	ChangeDirectDrawObject();
	
	notreporting_vga = 1;
	// possible software video modes
	for (i=0; i<NumAvailableVideoModes; ++i)
	{
		VidModeInfo vmi(AvailableVideoModes[i]);
		
		if (vmi==vga_mode) notreporting_vga = 0;
		
		if (vmi.IsForScanDraw())
			avail_vidmodes[VM3_SCANDRAW].add_entry(vmi);
	}
	
	if (notreporting_vga)
	{
		if (vga_mode.IsForScanDraw())
			avail_vidmodes[VM3_SCANDRAW].add_entry(vga_mode);
	}
	
	// get the indexes of the video modes
	idx = 0;
	for (OHIF<VidModeInfo> i_vidmode_sd(&avail_vidmodes[VM3_SCANDRAW]); !i_vidmode_sd.done(); i_vidmode_sd.next(), ++idx)
	{
		if (i_vidmode_sd()==vidmode[VM3_SCANDRAW])
		{
			sel_vidmode_index[VM3_SCANDRAW] = idx;
			break;
		}
	}
	
	vidmode[VM3_SCANDRAW] = avail_vidmodes[VM3_SCANDRAW][sel_vidmode_index[VM3_SCANDRAW]];
	vidmode[VM3_D3D] = avail_vidmodes[VM3_D3D][sel_vidmode_index[VM3_D3D]];
	#if ENABLE_SHADING_OPTION
	desired_shading = sel_shading[d3d_opt];
	#endif
#endif
}
#endif

extern int SetGameVideoMode(void)
{
	desc_textures_to_load = hw_try_desc[0];
	
	LOGDXSTR("SetGameVideoMode");
	unsigned int s_width;
	unsigned int s_height;
	unsigned int s_depth;
//	do
	{
		finiObjectsExceptDD();
		WindowMode = WindowRequestMode;
		
		ScreenDescriptorBlock.SDB_Flags &= ~SDB_Flag_MIP; // mip mapping in d3d cannot be tested since no known drivers support it!
		if (VM3_SCANDRAW==d3d_opt) // don't use d3d
		{
			RasterisationRequestMode = RequestSoftwareRasterisation;
			SoftwareScanDrawRequestMode = RequestScanDrawDirectDraw;
			DXMemoryRequestMode = RequestSystemMemoryAlways;
			DXMemoryMode = SystemMemoryPreferred;
		}
		else
		{
			if (hw_avail) // use hw accel
			{
				RasterisationRequestMode = RequestDefaultRasterisation;
				SoftwareScanDrawRequestMode = RequestScanDrawDefault;
				DXMemoryRequestMode = RequestDefaultMemoryAllocation;
				DXMemoryMode = VideoMemoryPreferred;
			}
			else // use microsofts software drivers!!!
			{
				RasterisationRequestMode = RequestDefaultRasterisation;
				SoftwareScanDrawRequestMode = RequestScanDrawSoftwareRGB; // FIXME:ramp driver appears broken???
				DXMemoryRequestMode = RequestSystemMemoryAlways;
				DXMemoryMode = SystemMemoryPreferred;
			}
		}
		if (mipmap_opt[d3d_opt] && mipmap_avail[d3d_opt])
			ScreenDescriptorBlock.SDB_Flags |= SDB_Flag_MIP; // mip mapping in d3d cannot be tested since no known drivers support it!
		
		// deal with hw dd objects required?
		ChangeDirectDrawObject();
		
		#if SupportZBuffering
		if (zbuf_avail[d3d_opt])
			ZBufferRequestMode = zbufopt[d3d_opt];
		else
			ZBufferRequestMode = RequestZBufferNever;
		#else
		ZBufferRequestMode = RequestZBufferNever;
		#endif
		
		#if 0
		s_width  = vidmode[d3d_opt].Width;
		s_height = vidmode[d3d_opt].Height;
		s_depth  = vidmode[d3d_opt].ColourDepth;
		#else
		{
			DEVICEANDVIDEOMODESDESC *dPtr = &DeviceDescriptions[CurrentlySelectedDevice];
			VIDEOMODEDESC *vmPtr = &(dPtr->VideoModes[CurrentlySelectedVideoMode]);
			s_width  = vmPtr->Width;
			s_height = vmPtr->Height;
			s_depth  = vmPtr->ColourDepth;
		}
		#endif
		if (s_depth<=8)
		{
			/* temp hack to test super TLTs */
			ScreenDescriptorBlock.SDB_Flags &= ~SDB_Flag_TLTPalette;
			#if 0 && SupportZBuffering
			if (RequestZBufferAlways==ZBufferRequestMode)
				ScreenDescriptorBlock.SDB_Flags |= SDB_Flag_TLTPalette;
			#endif
			
			GLOBALASSERT(ScreenDescriptorBlock.SDB_Flags & SDB_Flag_Raw256);
			ConvertToDDPalette(TestPalette, LPTestPalette, 256, 0);

			ScreenDescriptorBlock.SDB_ScreenDepth = VideoModeType_8;
			VideoModeTypeScreen = VideoModeType_8;
		}
		else if (s_depth<=16)
		{
			ScreenDescriptorBlock.SDB_ScreenDepth = VideoModeType_15;
			VideoModeTypeScreen = VideoModeType_15;
		}
		else
		{
			ScreenDescriptorBlock.SDB_ScreenDepth = VideoModeType_24;
			VideoModeTypeScreen = VideoModeType_24;
			really_32_bit = s_depth > 24 ? TRUE : FALSE;
		}

		/* Set up the Screen Descriptor Block */

		ScreenDescriptorBlock.SDB_Width     = s_width;
		ScreenDescriptorBlock.SDB_Height    = s_height;
		ScreenDescriptorBlock.SDB_Size      = s_width*s_height;

		ScreenDescriptorBlock.SDB_DiagonalWidth = sqrt((float)s_width*s_width+s_height*s_height)+0.5;

		ScreenDescriptorBlock.SDB_CentreX   = s_width/2;
		ScreenDescriptorBlock.SDB_CentreY   = s_height/2;

		/* KJL 12:00:54 07/04/97 - new projection angles */
		#if USE_FOV_53
		ScreenDescriptorBlock.SDB_ProjX     = s_width;
		ScreenDescriptorBlock.SDB_ProjY     = s_height*4/3;
		#else
		ScreenDescriptorBlock.SDB_ProjX     = s_width/2;
		ScreenDescriptorBlock.SDB_ProjY     = s_height/2;
		#endif

		ScreenDescriptorBlock.SDB_ClipLeft  = 0;
		ScreenDescriptorBlock.SDB_ClipRight = s_width;
		ScreenDescriptorBlock.SDB_ClipUp    = 0;
		ScreenDescriptorBlock.SDB_ClipDown  = s_height;
		
		GenerateDirectDrawSurface();
		SetCursor(NULL);

		/* For dubious DirectX failures */
		if (AttemptVideoModeRestart)
			return 0;
		LOGDXSTR("Testing");
			
	}
	#if 0
	while
	(
		InitialiseDirect3DImmediateMode() || VM3_SCANDRAW==d3d_opt
			? VideoMemoryPreferred == DXMemoryMode
				? IsNotEnoughVidMemForScreenDepth(s_depth)
				: 0
			: (GLOBALASSERT(0=="CANNOT INITIALIZE DIRECT 3D FOR THIS VIDEO MODE"),1)//(EmergencyPcOptionsMenu("CANNOT INITIALIZE DIRECT 3D FOR THIS VIDEO MODE"), 1)
	);
	#endif
	if (IsNotEnoughVidMemForScreenDepth(s_depth))
	{
		// erm, something
		return 0;
	}

	if (!InitialiseDirect3DImmediateMode())
	{
		return 0;
	}
	
	if (ScanDrawMode == ScanDrawDirectDraw)
	{
		ScreenDescriptorBlock.SDB_Depth = ScreenDescriptorBlock.SDB_ScreenDepth;
		VideoModeType = VideoModeTypeScreen;

	}
	else
	{
		ScreenDescriptorBlock.SDB_Depth = VideoModeType_24;
		VideoModeType = VideoModeType_24;
	}


	#define VideoMode_Any 0x0fffffff
	VideoMode = VideoMode_Any; // to trip up any functions using this variable
	
	// but if it matches a predifined constant, set it so - may be required ??????
	switch (s_width)
	{
		case 320:
			switch (s_height)
			{
				case 200:
					switch (VideoModeTypeScreen)
					{
						case VideoModeType_8:
							VideoMode = VideoMode_DX_320x200x8;
							break;
						case VideoModeType_8T:
							VideoMode = VideoMode_DX_320x200x8T;
							break;
						case VideoModeType_15:
							VideoMode = VideoMode_DX_320x200x15;
							break;
					}
					break;
				case 240:
					if (VideoModeType_8==VideoModeTypeScreen)
						VideoMode = VideoMode_DX_320x240x8;
					break;
			}
			break;
		case 640:
			if (480==s_height) switch (VideoModeTypeScreen)
			{
				case VideoModeType_8:
					VideoMode = VideoMode_DX_640x480x8;
					break;
				case VideoModeType_8T:
					VideoMode = VideoMode_DX_640x480x8T;
					break;
				case VideoModeType_15:
					VideoMode = VideoMode_DX_640x480x15;
					break;
				case VideoModeType_24:
					VideoMode = VideoMode_DX_640x480x24;
					break;
			}
			break;
		case 800:
			if (600==s_height) switch (VideoModeTypeScreen)
			{
				case VideoModeType_8:
					VideoMode = VideoMode_DX_800x600x8;
					break;
				case VideoModeType_8T:
					VideoMode = VideoMode_DX_800x600x8T;
					break;
				case VideoModeType_15:
					VideoMode = VideoMode_DX_800x600x15;
					break;
				case VideoModeType_24:
					VideoMode = VideoMode_DX_800x600x24;
					break;
			}
			break;
		case 1024:
			if (768==s_height) switch (VideoModeTypeScreen)
			{
				case VideoModeType_8:
					VideoMode = VideoMode_DX_1024x768x8;
					break;
				case VideoModeType_8T:
					VideoMode = VideoMode_DX_1024x768x8T;
					break;
				case VideoModeType_15:
					VideoMode = VideoMode_DX_1024x768x15;
					break;
				case VideoModeType_24:
					VideoMode = VideoMode_DX_1024x768x24;
					break;
			}
			break;
		case 1280:
			if (1024==s_height) switch (VideoModeTypeScreen)
			{
				case VideoModeType_8:
					VideoMode = VideoMode_DX_1280x1024x8;
					break;
				case VideoModeType_8T:
					VideoMode = VideoMode_DX_1280x1024x8T;
					break;
				case VideoModeType_15:
					VideoMode = VideoMode_DX_1280x1024x15;
					break;
				case VideoModeType_24:
					VideoMode = VideoMode_DX_1280x1024x24;
					break;
			}
			break;
		case 1600:
			if (1200==s_height) switch (VideoModeTypeScreen)
			{
				case VideoModeType_8:
					VideoMode = VideoMode_DX_1600x1200x8;
					break;
				case VideoModeType_8T:
					VideoMode = VideoMode_DX_1600x1200x8T;
					break;
				case VideoModeType_15:
					VideoMode = VideoMode_DX_1600x1200x15;
					break;
				case VideoModeType_24:
					VideoMode = VideoMode_DX_1600x1200x24;
					break;
			}
			break;
	}
	
	LOGDXSTR("leaving SetGameVideoMode");
	return 1;
}

extern "C"
{
/* KJL 17:11:26 19/07/98 - The 3 fns below are the interface between the
video mode selection and the frontend menus. */

extern char *GetVideoModeDescription(void)
{
	const VidModeInfo* pVidModeInfo = &avail_vidmodes[d3d_opt][sel_vidmode_index[d3d_opt]];
	
	// Unfortunately, the heap template [] operator gives us a const reference;
	// we must cast away the constness:

	return ((VidModeInfo*)pVidModeInfo) -> GetDescription()->pProjCh();
}

extern void PreviousVideoMode(void)
{
	if(sel_vidmode_index[d3d_opt] > 0)
	{
		sel_vidmode_index[d3d_opt]--;
	}
	else
	{
		sel_vidmode_index[d3d_opt] = avail_vidmodes[d3d_opt].size() -1;
	}
}

extern void NextVideoMode(void)
{
	if(sel_vidmode_index[d3d_opt] < (avail_vidmodes[d3d_opt].size()-1))
	{
		sel_vidmode_index[d3d_opt]++;
	}
	else
	{
		sel_vidmode_index[d3d_opt] = 0;
	}
}
extern void SaveVideoModeSettings(void)
{
	vidmode[VM3_SCANDRAW] = avail_vidmodes[VM3_SCANDRAW][sel_vidmode_index[VM3_SCANDRAW]];
	vidmode[VM3_D3D] = avail_vidmodes[VM3_D3D][sel_vidmode_index[VM3_D3D]];

	FILE * fp = fopen("AVPVMOPT.BIN","wb");
	if (!fp) return;
	fwrite("VMOP",4,1,fp);
	fwrite(&vidmode[VM3_SCANDRAW].Width,4,1,fp);
	fwrite(&vidmode[VM3_SCANDRAW].Height,4,1,fp);
	fwrite(&vidmode[VM3_SCANDRAW].ColourDepth,4,1,fp);
	fwrite(&vidmode[VM3_D3D].Width,4,1,fp);
	fwrite(&vidmode[VM3_D3D].Height,4,1,fp);
	fwrite(&vidmode[VM3_D3D].ColourDepth,4,1,fp);
	fwrite(&d3d_opt,4,1,fp);
	fwrite(&hw_avail,4,1,fp);
	fwrite(&zbufopt[VM3_SCANDRAW],4,1,fp);
	fwrite(&zbufopt[VM3_D3D],4,1,fp);
	fwrite(&d3d_desired_tex_fmt,4,1,fp);
	fwrite(&sel_shading[VM3_SCANDRAW],4,1,fp);
	fwrite(&sel_shading[VM3_D3D],4,1,fp);
	fwrite(&BilinearTextureFilter,4,1,fp);
	fwrite(&mipmap_opt[VM3_SCANDRAW],4,1,fp);
	fwrite(&mipmap_opt[VM3_D3D],4,1,fp);
	fclose(fp);
	return;
}
}