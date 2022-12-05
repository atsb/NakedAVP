extern "C" {

#include "3dc.h"
#include "vramtime.h"
#include "dxlog.h"
#include "inline.h"
#include "scrshot.hpp"
#include "awTexLd.h" // to set the surface format for Aw gfx dd surface loads

#define UseLocalAssert No
#include "ourasert.h"


// for 640x480x8 experiment
#define InterlaceExperiment No


// In as separate define to debug because we
// might want to leave it even in a published 
// game.
#define AllowReboot Yes

// In as #define since there is no
// obvious good behaviour on failure
#define CheckForModeXInSubWindow No

// Temporary hack!
#define NoPalette No
extern void TimeStampedMessage(char *s);

// Nasty hack to try and fix non-appearance of font
// on some machines, probably due to there not being
// enough video memory for the font and BltFast
// not working outside display memory.
// According to Roxby, source colour keying won't work
// on Blt in the Beta 3, so expect a grotty font with this on.
// PS Source colour keying works, but the font
// still doesn't appear on my machine in SubWindow
// mode... Ho hum...
#define NoBltFastOnFont No


// Check to see if video mode is valid
// and rewrite it if it isn't reported

#define CheckVideoMode No

/*
	Globals
*/
	
LPDIRECTDRAW            lpDD;           // DirectDraw object
LPDIRECTDRAWSURFACE     lpDDSPrimary;   // DirectDraw primary surface
LPDIRECTDRAWSURFACE     lpDDSBack;      // DirectDraw back surface
LPDIRECTDRAWSURFACE     lpDDSHiddenBack; // for system memory rendering target, stable configuration
LPDIRECTDRAWPALETTE     lpDDPal[1];        // DirectDraw palette
#if debug || PreBeta
LPDIRECTDRAWSURFACE     lpDDDbgFont; // Debugging font, specific to current video mode
#endif
// For SubWindow mode
LPDIRECTDRAWCLIPPER		lpDDClipper;
// DirectDraw gdi surface
LPDIRECTDRAWSURFACE     lpDDGDI;      
int 					VideoModeColourDepth;
long				    BackBufferPitch;
// To describe available video hardware
int						TotalVideoMemory;
int						NumAvailableVideoModes;
VIDEOMODEINFO			AvailableVideoModes[MaxAvailableVideoModes];
// Must be kept up to date with jump table!!!!
VIDEOMODEINFO			EngineVideoModes[] = {
            // Mode 320x200x8
			320, // width
			200, // height
			8,   // colour depth (bits per pixel) 
            // Mode 320x200x8T
			320, // width
			200, // height
			8,   // colour depth (bits per pixel) 
			// Mode 320x200x15
			320, // width
			200, // height
			16, // colour depth (bits per pixel)
            // Mode 320x240x8
			320, // width
			240, // height
			8,   // colour depth (bits per pixel) 
            // Mode 640x480x8
			640, // width
			480, // height
			8,   // colour depth (bits per pixel) 
            // Mode 640x480x8T
			640, // width
			480, // height
			8,   // colour depth (bits per pixel) 
            // Mode 640x480x15
			640, // width
			480, // height
			16,   // colour depth (bits per pixel) 
            // Mode 640x480x24
			640, // width
			480, // height
			24   // colour depth (bits per pixel) 
           };

// Flag for backdrop composition

// for 640x480x8 experiment
#if InterlaceExperiment
int oddDraw;
#endif


// Surface for Backdrop composition
LPDIRECTDRAWSURFACE lpDDBackdrop;
// Pointer into Backdrop DD surface 
unsigned char* BackScreenBuffer;
// Pitch on auxilliary backdrop surface
static long BackScreenPitch;
DDPIXELFORMAT DisplayPixelFormat;

// For locking against other processes, e.g.
// mouse pointer display
unsigned char GlobalFlipLock = No;

/* Externs */

extern int VideoMode;
extern int VideoModeTypeScreen;
extern int VideoModeType;
extern SCREENDESCRIPTORBLOCK ScreenDescriptorBlock;
extern unsigned char *ScreenBuffer;
extern HWND				 hWndMain;
extern unsigned char LPTestPalette[];
extern unsigned char TestPalette[];
extern int WindowMode;
extern WINSCALEXY TopLeftSubWindow;
extern WINSCALEXY ExtentXYSubWindow;
extern int WinWidth;
extern int WinHeight;
extern int WinLeftX;
extern int WinTopY;
extern int WinRightX;
extern int WinBotY;
extern int DXMemoryMode;
extern int RasterisationRequestMode;
extern int DXMemoryRequestMode;
extern int WindowRequestMode;
extern int VideoRequestMode;
extern int ZBufferRequestMode;
extern int SoftwareScanDrawRequestMode;
extern unsigned char AttemptVideoModeRestart;
extern VIDEORESTARTMODES VideoRestartMode;
extern BOOL MMXAvailable;
extern BOOL D3DHardwareAvailable;

BOOL really_32_bit = 0;

void GenerateDirectDrawSurface()

{

    DDCAPS          ddcaps;
    HRESULT         ddrval;
    DDSURFACEDESC   ddsd;
    DDSCAPS         ddscaps;
	unsigned char   Mode8T;

    // Check for combination of a MODEX type mode
	// and SubWindowing

    #if AllowReboot
	// THIS MAY NOT WORK!!!
	  if (WindowMode == WindowModeSubWindow)
	    ddrval = lpDD->SetCooperativeLevel(hWndMain,
		  DDSCL_NORMAL);
	  else
  	    ddrval = lpDD->SetCooperativeLevel(hWndMain,
          DDSCL_EXCLUSIVE | DDSCL_FULLSCREEN | DDSCL_ALLOWREBOOT );

	LOGDXERR(ddrval);
	#else
	// More stable even if crashes cannot be booted 
	// out of?
	if (WindowMode == WindowModeSubWindow)
	    ddrval = lpDD->SetCooperativeLevel(hWndMain,
		  DDSCL_NORMAL);
	else
  	    ddrval = lpDD->SetCooperativeLevel(hWndMain,
          DDSCL_EXCLUSIVE | DDSCL_FULLSCREEN | DDSCL_ALLOWMODEX);
	LOGDXERR(ddrval);
	#endif

	TimeStampedMessage("Allow reboot thingy");

    if (ddrval != DD_OK)
	#if debug
		{
		 ReleaseDirect3D();
		 exit(0x2);
	    }
	#else
	return;
	#endif

   switch (ScreenDescriptorBlock.SDB_ScreenDepth)
      {
	   case VideoModeType_8:
	     VideoModeColourDepth = 8;
		 Mode8T = No;
		 break;
	   case VideoModeType_15:
	     VideoModeColourDepth = 16;
		 Mode8T = No;
		 break;
	   case VideoModeType_24:
		 if (really_32_bit)
	     	VideoModeColourDepth = 32;
		 else
	     	VideoModeColourDepth = 24;
		 Mode8T = No;
		 break;
	   case VideoModeType_8T: 
	     VideoModeColourDepth = 8;
		 Mode8T = Yes;
		 break;
	   default:
	     VideoModeColourDepth = 16; // default is 16 bit colour
		 break;
	  }

   // Note that SetDisplayMode is now technically part
   // of the DirectDraw2 COM interface, not DirectDraw.
   // However, as long as we do not wish to change
   // monitor refresh rates we do not need to use the
   // DirectDraw2 version or set up a separate secondary
   // DD interface object, which be just plain fiddly.

   // MAY NOT WORK LIKE THIS IN SUBWINDOW MODE!!!!
	if (WindowMode == WindowModeFullScreen)
	{
		ddrval = lpDD->SetDisplayMode(ScreenDescriptorBlock.SDB_Width, 
		ScreenDescriptorBlock.SDB_Height, VideoModeColourDepth);
		LOGDXERR(ddrval);

		if (ddrval != DD_OK)
		#if debug
		{
			ReleaseDirect3D();
			exit(ddrval);
		}
		#else
		{
			if ((ddrval == DDERR_INVALIDMODE) ||
			(ddrval == DDERR_GENERIC) ||
			(ddrval == DDERR_INVALIDPIXELFORMAT))
			{
				AttemptVideoModeRestart = Yes;
				VideoRestartMode = RestartDisplayModeNotAvailable;
			}
			return;
		}
		#endif
	}
	TimeStampedMessage("after SetDisplayMode");

	// Create primary surface and back buffer
	// IMPORTANT!!! Currently no support for triple
	// buffering in SubWindow mode!!!

	if (WindowMode == WindowModeSubWindow)
	{
		// Create primary
		memset(&ddsd,0,sizeof(DDSURFACEDESC));
		ddsd.dwSize = sizeof(DDSURFACEDESC);
		ddsd.dwFlags = DDSD_CAPS;

		// Request a 3D capable device so that
		// Direct3D accesses will work
		// note primary must be in video memory
		ddsd.ddsCaps.dwCaps = (DDSCAPS_PRIMARYSURFACE | DDSCAPS_3DDEVICE);

		ddrval = lpDD->CreateSurface(&ddsd, &lpDDSPrimary, NULL);
		LOGDXERR(ddrval);
		if (ddrval != DD_OK)
		#if debug
		{
		ReleaseDirect3D();
		exit(0x41);
		}
		#else
		return;
		#endif

		// Create back buffer
		memset(&ddsd,0,sizeof(DDSURFACEDESC));
		ddsd.dwSize = sizeof(DDSURFACEDESC);
		ddsd.dwFlags = DDSD_WIDTH | DDSD_HEIGHT | DDSD_CAPS;
		ddsd.dwHeight = ScreenDescriptorBlock.SDB_Height;
		ddsd.dwWidth = ScreenDescriptorBlock.SDB_Width;

		// Request a 3D capable device so that
		// Direct3D accesses will work
		if (DXMemoryMode == SystemMemoryPreferred)
		ddsd.ddsCaps.dwCaps = (DDSCAPS_OFFSCREENPLAIN | DDSCAPS_SYSTEMMEMORY | DDSCAPS_3DDEVICE);
		else // video memory if possible
		ddsd.ddsCaps.dwCaps= (DDSCAPS_OFFSCREENPLAIN | DDSCAPS_3DDEVICE);

		ddrval = lpDD->CreateSurface(&ddsd, &lpDDSBack, NULL);
		LOGDXERR(ddrval);
		if (ddrval != DD_OK)
		#if debug
		{
		ReleaseDirect3D();
		exit(0x5);
		}
		#else
		return;
		#endif

		// Create clipper objects
		ddrval = lpDD->CreateClipper(0, &lpDDClipper, NULL);
		LOGDXERR(ddrval);
		if (ddrval != DD_OK)
		#if debug
		{
		ReleaseDirect3D();
		exit(0x55);
		}
		#else
		return;
		#endif

		ddrval = lpDDClipper->SetHWnd(0, hWndMain);
		LOGDXERR(ddrval);
		if (ddrval != DD_OK)
		#if debug
		{
		ReleaseDirect3D();
		exit(0x51);
		}
		#else
		return;
		#endif

		ddrval = lpDDSPrimary->SetClipper(lpDDClipper);
		LOGDXERR(ddrval);
		if (ddrval != DD_OK)
		#if debug
		{
		ReleaseDirect3D();
		exit(0x52);
		}
		#else
		return;
		#endif
	}
	else // default to FullScreen... 
	{
		TimeStampedMessage("after 'default to FullScreen'");
		if (DXMemoryMode == VideoMemoryPreferred)
		{
			ddcaps.dwSize = sizeof (ddcaps);
			memset (&ddsd, 0, sizeof (ddsd));
			ddsd.dwSize = sizeof (ddsd);
			ddsd.dwFlags = DDSD_CAPS | DDSD_BACKBUFFERCOUNT;

			// Request a 3D capable device so that
			// Direct3D accesses will work
			ddsd.ddsCaps.dwCaps = DDSCAPS_PRIMARYSURFACE |
			DDSCAPS_FLIP |
			DDSCAPS_COMPLEX |
			DDSCAPS_3DDEVICE;

			ddsd.dwBackBufferCount = 1;

			ddrval = lpDD->CreateSurface(&ddsd, &lpDDSPrimary, NULL);
			LOGDXERR(ddrval);
			TimeStampedMessage("after vm CreateSurface");

			if (ddrval != DD_OK)
			#if debug
			{
				ReleaseDirect3D();
				exit(0x41);
			}
			#else
			{
				// For dubious modex emulation 
				// problem fix and dubious driver
				// cannot change to different bit depths
				// fix.
				// Note that this must be kept up to date!!!!
				if ((ddrval == DDERR_OUTOFVIDEOMEMORY) &&
				((VideoMode == VideoMode_DX_320x200x8) ||
				(VideoMode == VideoMode_DX_320x200x8T) ||
				(VideoMode == VideoMode_DX_320x240x8) ||
				(VideoMode == VideoMode_DX_320x200x15)))
				{
					AttemptVideoModeRestart = Yes;
					VideoRestartMode = RestartOutOfVidMemForPrimary;
				}
				else if ((ddrval == DDERR_INVALIDMODE) ||
				(ddrval == DDERR_GENERIC) ||
				(ddrval == DDERR_INVALIDPIXELFORMAT))
				{
					AttemptVideoModeRestart = Yes;
					VideoRestartMode = RestartDisplayModeNotAvailable;
				}

				return;
			}
			#endif

			// get a pointer to the back buffer
			// DO I NEED TO SET A SIZE FIELD HERE??
			// SEEMS I _CAN'T_, ANYWAY...
			ddscaps.dwCaps = DDSCAPS_BACKBUFFER;

			ddrval = lpDDSPrimary->GetAttachedSurface(&ddscaps,&lpDDSBack);
			LOGDXERR(ddrval);
			TimeStampedMessage("after vm GetAttachedSurface");

			if (ddrval != DD_OK)
			#if debug
			{
			ReleaseDirect3D();
			exit(0x5);
			}
			#else
			return;
			#endif
			}
			// assume we want a system memory
			// rendering target, e.g. for MMX
			else 
			{
			DDPIXELFORMAT TempPixelFormat;

			// Make primary, with back buffer in video memory
			ddcaps.dwSize = sizeof (ddcaps);
			memset (&ddsd, 0, sizeof (ddsd));
			ddsd.dwSize = sizeof (ddsd);
			ddsd.dwFlags = DDSD_CAPS | DDSD_BACKBUFFERCOUNT;

			// Request a 3D capable device so that
			// Direct3D accesses will work
			ddsd.ddsCaps.dwCaps = DDSCAPS_PRIMARYSURFACE |
			DDSCAPS_FLIP |
			DDSCAPS_COMPLEX |
			DDSCAPS_3DDEVICE;

			ddsd.dwBackBufferCount = 1;

			ddrval = lpDD->CreateSurface(&ddsd, &lpDDSPrimary, NULL);
			LOGDXERR(ddrval);
			TimeStampedMessage("after vm CreateSurface");

			if (ddrval != DD_OK)
			#if debug
			{
				ReleaseDirect3D();
				exit(ddrval);
			}
			#else
			{
				return;
			}
			#endif

			// get a pointer to the back buffer
			// Note that in this configuration the back
			// buffer is hidden from the rendering system,
			// since other configurations do not seem to be
			// stable in ModeX emulation modes
			ddscaps.dwCaps = DDSCAPS_BACKBUFFER;

			ddrval = lpDDSPrimary->GetAttachedSurface(
			&ddscaps,
			&lpDDSHiddenBack);
			LOGDXERR(ddrval);
			TimeStampedMessage("after vm GetAttachedSurface");
	
			if (ddrval != DD_OK)
			#if debug
			{
				ReleaseDirect3D();
				exit(ddrval);
			}
			#else
			return;
			#endif

			// Save pixel format of primary
			memcpy(&TempPixelFormat, &ddsd.ddpfPixelFormat, 
			sizeof(DDPIXELFORMAT));
			TimeStampedMessage("after memcpy 1");

			// Create rendering target
			memset(&ddsd,0,sizeof(DDSURFACEDESC));
			ddsd.dwSize = sizeof(DDSURFACEDESC);
			ddsd.dwFlags = DDSD_WIDTH | DDSD_HEIGHT | DDSD_CAPS;
			ddsd.dwHeight = ScreenDescriptorBlock.SDB_Height;
			ddsd.dwWidth = ScreenDescriptorBlock.SDB_Width;

			// Ensure rendering target has same format as primary
			memcpy(&ddsd.ddpfPixelFormat, &TempPixelFormat,
			sizeof(DDPIXELFORMAT));
			TimeStampedMessage("after memcpy 2");

			// Request a 3D capable device so that
			// Direct3D accesses will work
			ddsd.ddsCaps.dwCaps = (DDSCAPS_OFFSCREENPLAIN | DDSCAPS_SYSTEMMEMORY | DDSCAPS_3DDEVICE);

			ddrval = lpDD->CreateSurface(&ddsd, &lpDDSBack, NULL);
			TimeStampedMessage("after CreateSurface");
			LOGDXERR(ddrval);
			if (ddrval != DD_OK)
			#if debug
			{
				ReleaseDirect3D();
				exit(ddrval);
			}
			#else
			return;
			#endif
		}
	}

	// Set the Colour Palette (paletted modes only)
	#if NoPalette
	#else
    if (VideoModeColourDepth == 8)
	  {

		TimeStampedMessage("SHOULD NEVER GET HERE");

	   if (WindowMode == WindowModeSubWindow)
	   // Use system palette in a SubWindow case
	     {
		  // Standard windows RGB + flags palette
		  // structure prototyped in various versions
		  // (Though not, apparently, others) of objbase.h
		  PALETTEENTRY SystemPalette[256];
		  // Get system palette via GDI device context
		  HDC hdc = GetDC(NULL);
          GetSystemPaletteEntries(hdc, 0, (1 << 8), SystemPalette);
		  // Restrict system to two palette entries only.
		  SetSystemPaletteUse(hdc, SYSPAL_NOSTATIC);
          ReleaseDC(NULL, hdc);

          // Flag system palette entries
		  // Will this work?
		  // I sure hope so...
		  {
		   int i;
		   // Take all but 20 basic entries for our
		   // greedy little sod of a game window
           for (i=0; i<1; i++) 
             SystemPalette[i].peFlags = PC_EXPLICIT;
           for (i=1; i<(256-1); i++) 
		     {
			  SystemPalette[i].peRed = LPTestPalette[i*4];
			  SystemPalette[i].peGreen = LPTestPalette[(i*4)+1];
			  SystemPalette[i].peBlue = LPTestPalette[(i*4)+2];
              SystemPalette[i].peFlags = PC_RESERVED;
			 }
           for (i=(256-1); i<256; i++) 
             SystemPalette[i].peFlags = PC_EXPLICIT;
		  }

          // Create the palette within DirectDraw, using
		  // DD palette initialisation rather than ours
          ddrval = lpDD->CreatePalette(DDPCAPS_8BIT | DDPCAPS_ALLOW256, 
                 SystemPalette, &lpDDPal[0], NULL);
	LOGDXERR(ddrval);

          if (ddrval != DD_OK)
	      #if debug
		    {
		     ReleaseDirect3D();
	         exit(0x7);
	        }
	      #else
	      return;
	      #endif

          // Set palette on both buffers
          ddrval = lpDDSBack->SetPalette(lpDDPal[0]);
	LOGDXERR(ddrval);
          ddrval = lpDDSPrimary->SetPalette(lpDDPal[0]);
	LOGDXERR(ddrval);

          if (ddrval != DD_OK)
	      #if debug
		    {
		     ReleaseDirect3D();
	         exit(ddrval);
	        }
	      #else
	      return;
	      #endif
		  // Save palette in internal format for
		  // use later
		  // Out because it causes problems with
		  // window mode swapping.
		  // ConvertDDToInternalPalette((unsigned char*) SystemPalette, TestPalette, 256);
		 }
	   else // default to FullScreen
	     {
	      if ((ScreenDescriptorBlock.SDB_Flags 
	         & SDB_Flag_Raw256) || (Mode8T))// full 256 colours
	        ddrval = lpDD->CreatePalette((DDPCAPS_8BIT | DDPCAPS_ALLOW256),
								(LPPALETTEENTRY)(LPTestPalette),
								&lpDDPal[0],
								NULL);
	      else // default is 222
	        ddrval = lpDD->CreatePalette((DDPCAPS_8BIT),
								(LPPALETTEENTRY)(LPTestPalette),
								&lpDDPal[0],
								NULL);
	LOGDXERR(ddrval);
          if (ddrval != DD_OK)
	      #if debug
		    {
		     ReleaseDirect3D();
	         exit(0x7);
	        }
	      #else
	      return;
	      #endif

	      ddrval = lpDDSPrimary->SetPalette(lpDDPal[0]);
	LOGDXERR(ddrval);

          if (ddrval != DD_OK)
	      #if debug
		    {
		     ReleaseDirect3D();
	         exit(0x8);
	        }
	      #else
	      return;
	      #endif

// Set palette on BOTH buffers to work around
// bug in Direct3D initialisation!!!
	      ddrval = lpDDSBack->SetPalette(lpDDPal[0]);
	LOGDXERR(ddrval);

          if (ddrval != DD_OK)
	      #if debug
		    {
		     ReleaseDirect3D();
	         exit(0x8);
	        }
	      #else
	      return;
	      #endif
	     }
      }
    #endif // for NoPalette

    // Get the surface desc for AW DDSurface loads
    
    memset(&ddsd,0,sizeof ddsd);
    ddsd.dwSize = sizeof ddsd;
    ddrval = lpDDSPrimary->GetSurfaceDesc(&ddsd);
    LOGDXERR(ddrval);
    GLOBALASSERT(DD_OK==ddrval);
	TimeStampedMessage("after memset");
    AwSetSurfaceFormat(&ddsd);
	TimeStampedMessage("after AwSetSurfaceFormat");
    
    // Do an initial lock and unlock on the back buffer
	// to pull out vital information such as the 
	// surface description
	LockSurfaceAndGetBufferPointer();
	UnlockSurface();
	TimeStampedMessage("after Lock & Unlock");

} 

// Lock back buffer and get screen pointer

void		LockSurfaceAndGetBufferPointer()
{
   DDSURFACEDESC    ddsdback;
   HRESULT          ddrval;
   int count = 0;

   #if optimiseblit
   while (lpDDSBack->GetBltStatus(DDGBS_ISBLTDONE) != DD_OK);
   #endif

   memset(&ddsdback, 0, sizeof(ddsdback));
   ddsdback.dwSize = sizeof(ddsdback);

   // DDLOCK_WAIT is another safety option that
   // should in theory be removable by using
   // the GetBltStatus call above.
   // Or of course the while loop...
   while ((ddrval = lpDDSBack->Lock(NULL, &ddsdback, DDLOCK_WAIT, NULL)) == DDERR_WASSTILLDRAWING)
		{
		 count++;
		 if (count>1) // was 10000, for test purposes ONLY!!!
		   {
	LOGDXERR(ddrval);
			ReleaseDirect3D();
			exit(0x2001);
		   }
		}
	
	LOGDXERR(ddrval);
	if (ddrval != DD_OK)
	  {
	   #if debug
	   ReleaseDirect3D();
	   exit(ddrval);
	   #else
	   return;
	   #endif
	  }

	/* ddsdback now contains my lpSurface)*/

	ScreenBuffer = (unsigned char *)ddsdback.lpSurface;
	BackBufferPitch = ddsdback.lPitch;

    // Get pixel format description of
	// rendering target
    memcpy(&DisplayPixelFormat, &ddsdback.ddpfPixelFormat, 
	      sizeof(DDPIXELFORMAT));
}

void UnlockSurface(void)
{
    HRESULT         ddrval;

	ddrval = lpDDSBack->Unlock((LPVOID)ScreenBuffer);
	LOGDXERR(ddrval);

    #if debug
	if (ddrval != DD_OK)
	  {
	   ReleaseDirect3D();
	   exit(ddrval);
	  }
	#endif
}

void FlipBuffers(void)
{
	HRESULT ddrval;

    // for locking against other draw processes,
	// e.g. mouse pointer
	GlobalFlipLock = Yes;

    // IMPORTANT!!! OptimiseFlip, Blit are
	// not supported in SubWindow mode!!!

	if (WindowMode == WindowModeSubWindow)
	  {
	   RECT dest;

	   // For SubWindow mode, we cannot flip, but
	   // must instead do a (stretched, in principle)
	   // blit to the front buffer to combine with GDI
	   // surfaces.
	   // The whole back buffer should be taken.
	   dest.left = WinLeftX;
	   dest.top = WinTopY;
	   dest.right = WinRightX;
	   dest.bottom = WinBotY;
	   ddrval = lpDDSPrimary->Blt(&dest, lpDDSBack,
	            NULL, DDBLT_WAIT, NULL);
	  }
	else // default to FullScreen
	  {
	   // we hope to flip, and flip to hope
	   // we are the tumbling jongleurs...
	   if (DXMemoryMode == VideoMemoryPreferred)
	     {
          #if optimiseflip
	      while (lpDDSBack->GetFlipStatus(DDGFS_ISFLIPDONE) == DDERR_WASSTILLDRAWING)
		      ProcessProjectWhileWaitingToBeFlippable();
	      #endif

          // we are going to flip with DDFLIP_WAIT on
	      // even if optimiseflip is set. 
	      // IT'S THE ONLY WAY TO BE SURE.
          ddrval = lpDDSPrimary->Flip(NULL, DDFLIP_WAIT);

	      while(1)
	        {
   		     if (ddrval == DD_OK)
     	       break;
		     if (ddrval == DDERR_SURFACELOST)
 			   {
  			    ddrval = lpDDSPrimary->Restore();
 			    if (ddrval != DD_OK)
    		      break;
  			   }
 		     if (ddrval != DDERR_WASSTILLDRAWING)
 		       break;
	        }
		 }
	   else // assume system memory rendering target
	     {
		  // Take rendering target to back buffer
		  // Rendering target should map fully onto back buffer
		  ddrval = lpDDSHiddenBack->Blt(NULL, lpDDSBack,
	            NULL, DDBLT_WAIT, NULL);

          // And now do a standard flip
          #if optimiseflip
	      while (lpDDSBack->GetFlipStatus(DDGFS_ISFLIPDONE) == DDERR_WASSTILLDRAWING)
		      ProcessProjectWhileWaitingToBeFlippable();
	      #endif

          // we are going to flip with DDFLIP_WAIT on
	      // even if optimiseflip is set. 
	      // IT'S THE ONLY WAY TO BE SURE.
          ddrval = lpDDSPrimary->Flip(NULL, DDFLIP_WAIT);

	      while(1)
	        {
   		     if (ddrval == DD_OK)
     	       break;
		     if (ddrval == DDERR_SURFACELOST)
 			   {
  			    ddrval = lpDDSPrimary->Restore();
 			    if (ddrval != DD_OK)
    		      break;
  			   }
 		     if (ddrval != DDERR_WASSTILLDRAWING)
 		       break;
	        }
		 }
      }

	GlobalFlipLock = No;

    ProjectSpecificBufferFlipPostProcessing();
	
	HandleScreenShot();

    #if InterlaceExperiment
	oddDraw ^= 1;
	#endif
}

void InGameFlipBuffers(void)
{
	FlipBuffers();
	return;
	HRESULT ddrval;

    // for locking against other draw processes,
	// e.g. mouse pointer
	GlobalFlipLock = Yes;

    // IMPORTANT!!! OptimiseFlip, Blit are
	// not supported in SubWindow mode!!!

	if (WindowMode == WindowModeSubWindow)
	  {
	   RECT dest;

	   // For SubWindow mode, we cannot flip, but
	   // must instead do a (stretched, in principle)
	   // blit to the front buffer to combine with GDI
	   // surfaces.
	   // The whole back buffer should be taken.
	   dest.left = WinLeftX;
	   dest.top = WinTopY;
	   dest.right = WinRightX;
	   dest.bottom = WinBotY;
	   ddrval = lpDDSPrimary->Blt(&dest, lpDDSBack,
	            NULL, DDBLT_WAIT, NULL);
	  }
	else // default to FullScreen
	  {
	   // we hope to flip, and flip to hope
	   // we are the tumbling jongleurs...
	   if (DXMemoryMode == VideoMemoryPreferred)
	     {
          #if optimiseflip
	      while (lpDDSBack->GetFlipStatus(DDGFS_ISFLIPDONE) == DDERR_WASSTILLDRAWING)
		      ProcessProjectWhileWaitingToBeFlippable();
	      #endif

          // we are going to flip with DDFLIP_WAIT on
	      // even if optimiseflip is set. 
	      // IT'S THE ONLY WAY TO BE SURE.
          ddrval = lpDDSPrimary->Flip(NULL, 0);

	      while(0)
	        {
   		     if (ddrval == DD_OK)
     	       break;
		     if (ddrval == DDERR_SURFACELOST)
 			   {
  			    ddrval = lpDDSPrimary->Restore();
 			    if (ddrval != DD_OK)
    		      break;
  			   }
 		     if (ddrval != DDERR_WASSTILLDRAWING)
 		       break;
	        }
		 }
	   else // assume system memory rendering target
	     {
		 LOCALASSERT(0);
		  // Take rendering target to back buffer
		  // Rendering target should map fully onto back buffer
		  ddrval = lpDDSHiddenBack->Blt(NULL, lpDDSBack,
	            NULL, DDBLT_WAIT, NULL);

          // And now do a standard flip
          #if optimiseflip
	      while (lpDDSBack->GetFlipStatus(DDGFS_ISFLIPDONE) == DDERR_WASSTILLDRAWING)
		      ProcessProjectWhileWaitingToBeFlippable();
	      #endif

          // we are going to flip with DDFLIP_WAIT on
	      // even if optimiseflip is set. 
	      // IT'S THE ONLY WAY TO BE SURE.
          ddrval = lpDDSPrimary->Flip(NULL, DDFLIP_WAIT);

	      while(1)
	        {
   		     if (ddrval == DD_OK)
     	       break;
		     if (ddrval == DDERR_SURFACELOST)
 			   {
  			    ddrval = lpDDSPrimary->Restore();
 			    if (ddrval != DD_OK)
    		      break;
  			   }
 		     if (ddrval != DDERR_WASSTILLDRAWING)
 		       break;
	        }
		 }
      }

	GlobalFlipLock = No;

    ProjectSpecificBufferFlipPostProcessing();
	
	HandleScreenShot();

    #if InterlaceExperiment
	oddDraw ^= 1;
	#endif
}

// As of 29/4/96, slightly later in the day,
// it now deallocates NOTHING explcitly, but
// just calls Release on the entire DD object,
// which seems a better approach. Ho hum.

// This function is now OBSOLETE (25 / 7 / 96)
// CALL RELEASEDIRECT3D INSTEAD!!!

void finiObjects(void)
{
    if (lpDD != NULL)
    {
     lpDD->Release();
     lpDD = NULL;
    }
}

// Release ONE direct draw surface, to be used
// when replacing images via imageheader array,
// e.g. in LoadBackdrop.

void ReleaseDDSurface(void* DDSurface)

{
	LPDIRECTDRAWSURFACE lpSurface;

	WaitForVRamReady(VWS_DDRELEASE);
    
    lpSurface = (LPDIRECTDRAWSURFACE) DDSurface;

	if (lpSurface != NULL)
	  {
	   lpSurface->Release();
	   lpSurface = NULL;
	  }
}


// Blt member is used rather BltFast because BltFast does not 
// allow a colour fill

void ColourFillBackBuffer(int FillColour)

{
	HRESULT ddrval;
	DDBLTFX ddbltfx;

    memset(&ddbltfx, 0, sizeof(ddbltfx));
	ddbltfx.dwSize = sizeof(ddbltfx);
	ddbltfx.dwFillColor	= FillColour;

	/* lets blt a color to the surface*/
    #if optimiseblit 
	while (lpDDSBack->GetBltStatus(DDGBS_CANBLT) != DD_OK);
	#endif

	#if optimiseblit
	ddrval = lpDDSBack->Blt(NULL, NULL, NULL, DDBLT_COLORFILL | DDBLT_ASYNC, &ddbltfx);

    if (ddrval == DDERR_WASSTILLDRAWING)
	  ddrval = lpDDSBack->Blt(NULL, NULL, NULL, DDBLT_COLORFILL | DDBLT_WAIT, 
	                      &ddbltfx);
	#else
	ddrval = lpDDSBack->Blt(NULL, NULL, NULL, DDBLT_COLORFILL | DDBLT_WAIT, &ddbltfx);
	#endif

	LOGDXERR(ddrval);
	if (ddrval != DD_OK)
	  {
	   #if debug
	   ReleaseDirect3D();
	   exit(ddrval);
	   #else
	   return;
	   #endif
	  }
}

// Blt member is used rather BltFast because BltFast does not 
// allow a colour fill

void ColourFillBackBufferQuad(int FillColour, int LeftX,
     int TopY, int RightX, int BotY)

{
	HRESULT ddrval;
	DDBLTFX ddbltfx;
	RECT destRect;

    destRect.left = LeftX;
	destRect.top = TopY;
	destRect.right = RightX;
	destRect.bottom = BotY;
 
    memset(&ddbltfx, 0, sizeof(ddbltfx));
	ddbltfx.dwSize = sizeof(ddbltfx);
	ddbltfx.dwFillColor	= FillColour;
	
	/* lets blt a color to the surface*/
    #if optimiseblit 
	while (lpDDSBack->GetBltStatus(DDGBS_CANBLT) != DD_OK);
	#endif

	#if optimiseblit
	ddrval = lpDDSBack->Blt(&destRect, NULL, NULL, DDBLT_COLORFILL | DDBLT_ASYNC, &ddbltfx);

    if (ddrval == DDERR_WASSTILLDRAWING)
	  ddrval = lpDDSBack->Blt(&destRect, NULL, NULL, DDBLT_COLORFILL | DDBLT_WAIT, &ddbltfx);
	#else
	ddrval = lpDDSBack->Blt(&destRect, NULL, NULL, DDBLT_COLORFILL | DDBLT_WAIT, &ddbltfx);
	#endif

	LOGDXERR(ddrval);
	if (ddrval != DD_OK)
	  {
	   #if debug
	   ReleaseDirect3D();
	   exit(ddrval);
	   #else
	   return;
	   #endif
	  }
}

// Note Blt is used rather than BltFast because BltFast will
// always attempt to invoke an asynchronous blit and this appears
// to be unstable at present (cf. problems with optimiseblit)
// CHECK THIS -- WITH DDBLTFASTWAIT???
// COMMENT ABOVE NOW OBSOLETE...

void BlitToBackBuffer(void* lpBackground, RECT* destRectPtr, RECT* srcRectPtr)

{
	HRESULT ddrval;

    #if optimiseblit 
	while (lpDDSBack->GetBltStatus(DDGBS_CANBLT) != DD_OK);
	#endif

	#if optimiseblit
	ddrval = lpDDSBack->Blt(destRectPtr, (LPDIRECTDRAWSURFACE) lpBackground,
	  srcRectPtr, DDBLT_ASYNC, NULL);

    if (ddrval == DDERR_WASSTILLDRAWING)
	  ddrval = lpDDSBack->Blt(destRectPtr, (LPDIRECTDRAWSURFACE) lpBackground,
	             srcRectPtr, DDBLT_WAIT, NULL);
	#else
	ddrval = lpDDSBack->Blt(destRectPtr, (LPDIRECTDRAWSURFACE) lpBackground,
	  srcRectPtr, DDBLT_WAIT, NULL);
	#endif

	LOGDXERR(ddrval);
	if (ddrval != DD_OK)
	  {
	   #if debug
	   ReleaseDirect3D();
	   exit(ddrval);
	   #else
	   return;
	   #endif
	  }
}

// Note Blt is used rather than BltFast because BltFast does not
// support DDBLTFX and therefore cannot be used to attempt to
// prevent tearing

void BlitToBackBufferWithoutTearing(void* lpBackground, RECT* destRectPtr, RECT* srcRectPtr)

{
	HRESULT ddrval;
	DDBLTFX ddbltfx;

    memset(&ddbltfx, 0, sizeof(ddbltfx));
	ddbltfx.dwSize = sizeof(ddbltfx);
	ddbltfx.dwDDFX = DDBLTFX_NOTEARING;

    #if optimiseblit 
	while (lpDDSBack->GetBltStatus(DDGBS_CANBLT) != DD_OK);
	#endif

	#if optimiseblit
	ddrval = lpDDSBack->Blt(destRectPtr, (LPDIRECTDRAWSURFACE) lpBackground,
	  srcRectPtr, DDBLT_ASYNC | DDBLT_DDFX, &ddbltfx);

    if (ddrval == DDERR_WASSTILLDRAWING)
	  ddrval = lpDDSBack->Blt(destRectPtr, (LPDIRECTDRAWSURFACE) lpBackground,
	             srcRectPtr, DDBLT_WAIT | DDBLT_DDFX, &ddbltfx);
	#else
	ddrval = lpDDSBack->Blt(destRectPtr, (LPDIRECTDRAWSURFACE) lpBackground,
	  srcRectPtr, DDBLT_WAIT | DDBLT_DDFX, &ddbltfx);
	#endif

	LOGDXERR(ddrval);
	if (ddrval != DD_OK)
	  {
	   #if debug
	   ReleaseDirect3D();
	   exit(ddrval);
	   #else
	   return;
	   #endif
	  }
}

#if 0

// Note Blt is used rather than BltFast because BltFast does
// not support the DDBLTFX structure and therefore cannot accept
// a rotated blit

void RotatedBlitToBackBuffer(void* lpBackground, RECT* destRectPtr, RECT* srcRectPtr, int RollZ)

{
	HRESULT ddrval;
	DDBLTFX ddbltfx;

    memset(&ddbltfx, 0, sizeof(ddbltfx));
	ddbltfx.dwSize = sizeof(ddbltfx);
	ddbltfx.dwRotationAngle = RollZ;

    #if optimiseblit 
	while (lpDDSBack->GetBltStatus(DDGBS_CANBLT) != DD_OK);
	#endif

	#if optimiseblit
	ddrval = lpDDSBack->Blt(destRectPtr, (LPDIRECTDRAWSURFACE) lpBackground,
	  srcRectPtr, DDBLT_ASYNC | DDBLT_ROTATIONANGLE, &ddbltfx);

    if (ddrval == DDERR_WASSTILLDRAWING)
	  ddrval = lpDDSBack->Blt(destRectPtr, (LPDIRECTDRAWSURFACE) lpBackground,
	             srcRectPtr, DDBLT_WAIT | DDBLT_ROTATIONANGLE, &ddbltfx);
	#else
	ddrval = lpDDSBack->Blt(destRectPtr, (LPDIRECTDRAWSURFACE) lpBackground,
	  srcRectPtr, DDBLT_WAIT | DDBLT_ROTATIONANGLE, &ddbltfx);
	#endif

	LOGDXERR(ddrval);
	if (ddrval != DD_OK)
	  {
	   #if debug
	   ReleaseDirect3D();
	   exit(ddrval);
	   #else
	   return;
	   #endif
	  }
}

// Note Blt is used rather than BltFast because BltFast does not
// support DDBLTFX and therefore cannot be used to attempt to
// prevent tearing

void RotatedBlitToBackBufferWithoutTearing(void* lpBackground, RECT* destRectPtr, RECT* srcRectPtr, int RollZ)

{
	HRESULT ddrval;
	DDBLTFX ddbltfx;

    memset(&ddbltfx, 0, sizeof(ddbltfx));
	ddbltfx.dwSize = sizeof(ddbltfx);
	ddbltfx.dwDDFX = DDBLTFX_NOTEARING;
	ddbltfx.dwRotationAngle = RollZ;

    #if optimiseblit 
	while (lpDDSBack->GetBltStatus(DDGBS_CANBLT) != DD_OK);
	#endif

	#if optimiseblit
	ddrval = lpDDSBack->Blt(destRectPtr, (LPDIRECTDRAWSURFACE) lpBackground,
	  srcRectPtr, DDBLT_ASYNC | DDBLT_DDFX | DDBLT_ROTATIONANGLE, &ddbltfx);

    if (ddrval == DDERR_WASSTILLDRAWING)
	  ddrval = lpDDSBack->Blt(destRectPtr, (LPDIRECTDRAWSURFACE) lpBackground,
	             srcRectPtr, DDBLT_WAIT | DDBLT_DDFX | DDBLT_ROTATIONANGLE, &ddbltfx);
	#else
	ddrval = lpDDSBack->Blt(destRectPtr, (LPDIRECTDRAWSURFACE) lpBackground,
	  srcRectPtr, DDBLT_WAIT | DDBLT_DDFX | DDBLT_ROTATIONANGLE, &ddbltfx);
	#endif

	LOGDXERR(ddrval);
	if (ddrval != DD_OK)
	  {
	   #if debug
	   ReleaseDirect3D();
	   exit(ddrval);
	   #else
	   return;
	   #endif
	  }
}

#endif


// Note x, y are assumed to be TOP LEFT of character
// BltFast is used here, partially as an experiment (ahem...)

// THIS FUNCTION ASSUMES A VERTICAL FONT BRUSH!!!

// FIXME!!! This function does not always seem to
// produce visible text in SubWindow mode; i.e.
// as of 29/4/96 it works on Roxby's and fails on
// mine.  I suspect this is because we are using 
// BltFast, which 'works only on display memory',
// and there isn't enough video RAM on my card
// to put the debug font in video memory in SubWindow
// mode.  Or something.
// At present I have done nothing about this, since Roxby
// claims that as of Beta 3 he can't get source colour 
// keying working on the blitter without using BltFast, and
// we need source colour keying to get the font to 
// look right.  
// Will have to be looked at at some stage, 'tho.

#if debug || PreBeta
void BlitWin95Char(int x, int y, unsigned char toprint)

{
	static int bltwin95char_ok=1;
	if (!bltwin95char_ok) return;

	int FontIndex;
	RECT source;
    HRESULT ddrval;
	#if NoBltFastOnFont
	RECT destination;
	#endif

	// Check for data out of range for font
    if ((toprint < FontStart) || (toprint > FontEnd))
	  return; 

    // Check for character being on screen
	if ((x < ScreenDescriptorBlock.SDB_ClipLeft) ||
	   ((x + CharWidth) > ScreenDescriptorBlock.SDB_ClipRight) ||
	   (y < ScreenDescriptorBlock.SDB_ClipUp) ||
       ((y + CharHeight) > ScreenDescriptorBlock.SDB_ClipDown))
	  return;
     
	// Generate font index and source rectangle
	// Assumes vertical brush
	FontIndex = (toprint - FontStart);

	source.left = 0;
	source.top = (FontIndex * CharHeight);
	source.right = CharWidth;
	source.bottom = ((FontIndex + 1) * CharHeight);

    // Do blit
	#if NoBltFastOnFont
	destination.left = x;
	destination.top = y;
	destination.right = (x+CharWidth);
	destination.bottom = (y+CharHeight);
	ddrval = lpDDSBack->Blt(&destination, lpDDDbgFont,
	       &source, DDBLT_WAIT | DDBLT_KEYSRC, NULL);
	#else
	ddrval = lpDDSBack->BltFast(x, y,
			 lpDDDbgFont, &source, 
			 DDBLTFAST_WAIT | DDBLTFAST_SRCCOLORKEY);
	#endif

	LOGDXERR(ddrval);
    if (ddrval != DD_OK)
	#if debug && 0
	  {
	   ReleaseDirect3D();
	   exit(0x11);
	  }
	#else
	 bltwin95char_ok = 0;
	 return;
	#endif
}

#else
void BlitWin95Char(int x, int y, unsigned char toprint)
{
}
#endif

// IMPORTANT!!! FIXME!!!!

// This function has been hacked to FORCE images into 
// system memory due to what appears to be a bug in 
// DirectDraw's defaulting to system memory...

// Has HOPEFULLY now been fixed to deal with non 8 bit
// images... but I wouldn't count on it...

// If SysMem is TRUE, the image should go into system memory.  If it is
// FALSE, we will try for video memory.  And may God walk at our side in
// the valley of the shadow.



// This callback enumerates all the DirectDraw
// devices present on a system searching for one
// with hardware 3D support available.  If such
// a device is present, it will always be selected.
// Otherwise, no valid device will be returned.

static BOOL bHardwareDDObj = FALSE;
static BOOL bNoHardwareDD = FALSE;


BOOL FAR PASCAL EnumDDObjectsCallback(GUID FAR* lpGUID, LPSTR lpDriverDesc,
                                      LPSTR lpDriverName, LPVOID lpContext)
{
    LPDIRECTDRAW lpDDTest;
    DDCAPS DriverCaps, HELCaps;
	HRESULT ddrval;

    /*
      A NULL GUID* indicates the DirectDraw HEL which we are not interested
      in at the moment, because we are cruel and heartless and really
	  really nasty people, after all.
    */

    if (lpGUID)
	  {
       // Create the DirectDraw device using this driver.  If it fails,
       // just move on to the next driver.

       ddrval = DirectDrawCreate(lpGUID, &lpDDTest, NULL);
	LOGDXERR(ddrval);
       if (ddrval != DD_OK)
         return DDENUMRET_OK;

       // Get the capabilities of this DirectDraw driver.  If it fails,
       // just move on to the next driver.

       memset(&DriverCaps, 0, sizeof(DDCAPS));
       DriverCaps.dwSize = sizeof(DDCAPS);
       memset(&HELCaps, 0, sizeof(DDCAPS));
       HELCaps.dwSize = sizeof(DDCAPS);

       ddrval = lpDDTest->GetCaps(&DriverCaps, &HELCaps);
	LOGDXERR(ddrval);
	   if (ddrval != DD_OK)
	     {
          lpDDTest->Release();
          return DDENUMRET_OK;
         }

       // JH - changed so that debugging in a window works if you've got two cards or something
       if (DriverCaps.dwCaps & DDCAPS_3D && WindowRequestMode != WindowModeSubWindow)
	     {
          // We have found a 3d hardware device.  Return the DD object
          // and stop enumeration.
          // This assumes that 3d capable DD hardware
		  // drivers cannot operate in a window.
		  // True? CHECKME!!!
          WindowMode = WindowModeFullScreen;
          *(LPDIRECTDRAW*)lpContext = lpDDTest;
          return DDENUMRET_CANCEL;
         }       
        lpDDTest->Release();
    }
    return DDENUMRET_OK;
}


// Set up the DirectDraw object for the
// system. Using this we can determine the
// system capabilities. The DirectDraw object
// in question should never be released until
// ExitSystem, even if window or video modes
// are changed.  Note that as well as checking
// for valid video modes etc, this function will
// also search for hardware DirectDraw objects
// and use one if appropriate.

// Note that GenerateDirectDrawSurface and
// SetVideoMode now just set a video mode, 
// rather than (as previously) initialising the
// DirectDraw model.

// This function should be called
// during InitialiseSystem, so that we can
// be sure that SetVideoMode (called later)
// will be setting a valid mode.  The function
// must be called after InitialVideoMode (which
// sets the mode it validates) and before 
// InitialiseWindowsSystem, since in the
// current version that needs to know the
// WindowMode, not the VideoMode.  Note that 
// InitialiseWindowsSystem itself must be called
// after InitialVideoMode (it is currently
// called at the start of InitialiseSystem).

DDCAPS direct_draw_caps;

BOOL InitialiseDirectDrawObject(void)

{
    HRESULT         ddrval;
	LPDIRECTDRAW    lpDDHardware = NULL;

    // Set up DirectDraw object.

    // If we are not in emulation only mode,
	// search for a hardware 3D capable DirectDraw
	// driver and use it.  If this fails, or if we
	// are in emulation, go straight for the HEL.
	if (RasterisationRequestMode != RequestSoftwareRasterisation)
	  {
       ddrval = DirectDrawEnumerate(EnumDDObjectsCallback, &lpDDHardware);
	LOGDXERR(ddrval);
	   if (ddrval != DD_OK)
	     {
	      #if debug
	      ReleaseDirect3D();
	      exit(ddrval);
	      #else
	      return FALSE;
	      #endif
	     }
      }

    // If we don't have a hardware driver, either
	// because there wasn't one or because we are in 
	// HEL... then make one.

    if (!lpDDHardware)
	  {
	   if (RasterisationRequestMode != RequestSoftwareRasterisation)
	      bNoHardwareDD = TRUE;
       bHardwareDDObj = FALSE;
       ddrval = DirectDrawCreate(NULL, &lpDD, NULL);
	   LOGDXERR(ddrval);

       if (ddrval != DD_OK)
	   #if debug
	      {
	       ReleaseDirect3D();
	       exit(ddrval);
	      }
	   #else
	      {
	       ReleaseDirect3D(); // for safety
	       return FALSE;
	      }
	   #endif
	  }
	else
	{
	   bHardwareDDObj = TRUE;
	   lpDD = lpDDHardware;
	}
	
	AwSetDDObject(lpDD);

    // Get caps for hardware DD driver/HEL layer
    memset(&direct_draw_caps, 0, sizeof(direct_draw_caps));
    direct_draw_caps.dwSize = sizeof(direct_draw_caps);
    ddrval = lpDD->GetCaps(&direct_draw_caps, NULL);
	LOGDXERR(ddrval);

    if (ddrval != DD_OK)
	#if debug
	   {
	    ReleaseDirect3D();
	    exit(ddrval);
	   }
	#else
	   {
	    ReleaseDirect3D(); // for safety
	    return FALSE;
	   }
	#endif

    // Put statistics into globals
	TotalVideoMemory = (int) (direct_draw_caps.dwVidMemTotal);
    // At some stage, we should set up a proper
	// structure here with the general system
	// capabilities in it.
	// Notably, we may need to check DD surface
	// alignment values and the stride.

    // NOTE THAT AS GARRY HAS POINTED OUT, TOTAL
	// FREE VIDMEM CAN CHANGE AFTER SETTING THE
	// VIDEO MODE OR COOPERATIVE LEVEL, SO WE
	// SHOULD REALLY BE RUNNING THIS GETCAPS MEMBER
	// ON THE DDOBJECT AGAIN AFTER GENERATESURFACE, OR
	// POSSIBLY WHENEVER THE USER WANTS.
	// ***FIXME!!!***

    // Run the display modes enumerator
	// Note that NULL in the second entry
	// means enumerate all available display
	// modes on the driver.
	NumAvailableVideoModes = 0;
    ddrval = lpDD->EnumDisplayModes(0, NULL, 0, EnumDisplayModesCallback);
	LOGDXERR(ddrval);

    if (ddrval != DD_OK)
	#if debug
	   {
	    ReleaseDirect3D();
	    exit(ddrval);
	   }
	#else
	   {
	    ReleaseDirect3D(); // for safety
	    return FALSE;
	   }
	#endif

    // Check that the video mode asked
	// for (initially at least in 
	// InitialVideoMode) is actually
	// available.  If it is not, attempt to
	// default to 640x480x8 and 640x480x15
	// (liable to be the most common modes)
	// and if that fails, exit altogether
	// with a failure code.

    // Note!!! This only matters in FullScreen
	// mode.  In SubWindow mode we must use the
	// Windows display mode anyway.  Obviously
	// initialisation of the video mode will fail 
	// in SubWindow mode if the colour depth of
	// the Windows display is different to the
	// VideoMode colour depth, but I have not bothered
	// to check for this since SubWindow mode is
	// only intended for debugging and the failure
	// case exits with an appropriate DirectDraw
	// error anyway.
	#if CheckVideoMode
	if (WindowMode == WindowModeFullScreen)
	  {
	   if (!(CheckForVideoModes(VideoMode)))
	     {
	      VideoMode = VideoMode_DX_640x480x8;
	      if (!(CheckForVideoModes(VideoMode)))
	        {
		     VideoMode = VideoMode_DX_640x480x15;
		     if (!(CheckForVideoModes(VideoMode)))
		       {
			    ReleaseDirect3D(); // for safety
			    return FALSE;
			   }
		    } 
	     }
	  }
	#endif // for CheckVideoMode

	return TRUE; // Successful completion
}


// return TRUE on success

static BOOL ReallyChangeDDObj(void)
{
	finiObjects();

	if (InitialiseDirectDrawObject()
	    == FALSE)
	   /* 
	     If we cannot get a video mode, 
	     fail.  No point in a non debugging option
	     for this.
	   */
	   {
		#if debug
	    ReleaseDirect3D();
	    exit(0x997798);
		#else
		return FALSE;
		#endif
	   }

    /*
		Initialise global to say whether
		we think there is an onboard 3D 
		acceleration card / motherboard 
		built-in
	*/
	#if 0
    TestInitD3DObject();

/*
	This is (HOPEFULLY!!) now the right
	place to put this call.  Note that it is
	not absolutely certain that we can do test
	blits from DirectDraw without setting
	a cooperative level, however... And note also
	that MMX works better with the back buffer in
	system memory...
*/
    TestMemoryAccess();
	#endif
	return TRUE;
}

int SelectDirectDrawObject(LPGUID pGUID)
{
    HRESULT         ddrval;
	LPDIRECTDRAW    lpDDHardware = NULL;

    // Set up DirectDraw object.
	bNoHardwareDD = TRUE;
    bHardwareDDObj = FALSE;
    ddrval = DirectDrawCreate(pGUID, &lpDD, NULL);
	
	AwSetDDObject(lpDD);

    // Get caps for hardware DD driver/HEL layer
    memset(&direct_draw_caps, 0, sizeof(direct_draw_caps));
    direct_draw_caps.dwSize = sizeof(direct_draw_caps);
    ddrval = lpDD->GetCaps(&direct_draw_caps, NULL);
	LOGDXERR(ddrval);

    if (ddrval != DD_OK)
	#if debug
	   {
	    ReleaseDirect3D();
	    exit(ddrval);
	   }
	#else
	   {
	    ReleaseDirect3D(); // for safety
	    return FALSE;
	   }
	#endif

    // Put statistics into globals
	TotalVideoMemory = (int) (direct_draw_caps.dwVidMemTotal);
    // At some stage, we should set up a proper
	// structure here with the general system
	// capabilities in it.
	// Notably, we may need to check DD surface
	// alignment values and the stride.

    // NOTE THAT AS GARRY HAS POINTED OUT, TOTAL
	// FREE VIDMEM CAN CHANGE AFTER SETTING THE
	// VIDEO MODE OR COOPERATIVE LEVEL, SO WE
	// SHOULD REALLY BE RUNNING THIS GETCAPS MEMBER
	// ON THE DDOBJECT AGAIN AFTER GENERATESURFACE, OR
	// POSSIBLY WHENEVER THE USER WANTS.
	// ***FIXME!!!***

    // Run the display modes enumerator
	// Note that NULL in the second entry
	// means enumerate all available display
	// modes on the driver.
	NumAvailableVideoModes = 0;
    ddrval = lpDD->EnumDisplayModes(0, NULL, 0, EnumDisplayModesCallback);
	LOGDXERR(ddrval);

    if (ddrval != DD_OK)
	#if debug
	   {
	    ReleaseDirect3D();
	    exit(ddrval);
	   }
	#else
	   {
	    ReleaseDirect3D(); // for safety
	    return FALSE;
	   }
	#endif

    // Check that the video mode asked
	// for (initially at least in 
	// InitialVideoMode) is actually
	// available.  If it is not, attempt to
	// default to 640x480x8 and 640x480x15
	// (liable to be the most common modes)
	// and if that fails, exit altogether
	// with a failure code.

    // Note!!! This only matters in FullScreen
	// mode.  In SubWindow mode we must use the
	// Windows display mode anyway.  Obviously
	// initialisation of the video mode will fail 
	// in SubWindow mode if the colour depth of
	// the Windows display is different to the
	// VideoMode colour depth, but I have not bothered
	// to check for this since SubWindow mode is
	// only intended for debugging and the failure
	// case exits with an appropriate DirectDraw
	// error anyway.
	#if CheckVideoMode
	if (WindowMode == WindowModeFullScreen)
	  {
	   if (!(CheckForVideoModes(VideoMode)))
	     {
	      VideoMode = VideoMode_DX_640x480x8;
	      if (!(CheckForVideoModes(VideoMode)))
	        {
		     VideoMode = VideoMode_DX_640x480x15;
		     if (!(CheckForVideoModes(VideoMode)))
		       {
			    ReleaseDirect3D(); // for safety
			    return FALSE;
			   }
		    } 
	     }
	  }
	#endif // for CheckVideoMode

	return TRUE; // Successful completion
}



BOOL ChangeDirectDrawObject(void)
{
	if (RequestSoftwareRasterisation==RasterisationRequestMode)
	{
		// software modes required
		if (bHardwareDDObj)
		{
			// but was previously hardware DD
			return ReallyChangeDDObj();
		}
	}
	else
	{
		// hardware modes requested if available
		if (!bHardwareDDObj && !bNoHardwareDD)
		{
			// but was previously software DD and
			// we haven't established that hardware isn't available
			return ReallyChangeDDObj();
		}
	}

	return TRUE;
}



BOOL CheckForVideoModes(int TestVideoMode)

{
	int i=0;
	BOOL EarlyExit = FALSE;

    do
	  {
	   if ((EngineVideoModes[TestVideoMode].Width 
		   == AvailableVideoModes[i].Width) && 
		   (EngineVideoModes[TestVideoMode].Height
		   == AvailableVideoModes[i].Height) && 
		   (EngineVideoModes[TestVideoMode].ColourDepth
		   == AvailableVideoModes[i].ColourDepth))
		  EarlyExit = TRUE;
	   else
	      i++;
	  }	
	while ((i < NumAvailableVideoModes) &&
	       (!EarlyExit));

    return(EarlyExit);
}

HRESULT CALLBACK EnumDisplayModesCallback(LPDDSURFACEDESC pddsd, LPVOID Context)
{
    AvailableVideoModes[NumAvailableVideoModes].Width = pddsd->dwWidth;
    AvailableVideoModes[NumAvailableVideoModes].Height = pddsd->dwHeight;
    AvailableVideoModes[NumAvailableVideoModes].ColourDepth = pddsd->ddpfPixelFormat.dwRGBBitCount;
	NumAvailableVideoModes++;
    
	if (NumAvailableVideoModes < MaxAvailableVideoModes)
       return DDENUMRET_OK;
	else
	   return DDENUMRET_CANCEL;
}

// Deallocate all objects except the basic
// DirectDraw objects, which we want to do
// before shifting display modes or window 
// mode.  Note that ExitWindowsSystem must be
// called separately.  Note also that ExitSystem
// should be called for a full system shutdown.

// FIXME!!! BACKDROPS?!?!? -- done in 
// DeallocateAllImages (called from ReleaseDirect3D
// and ReleaseDirect3DNotDD)

// Note that getting the right sequence for releases
// here is CRUCIAL.  Note also that the font surface
// is NOT explicitly deallocated; hopefully it will just
// go when we kill the primary (as an offscreen surface).
// Could this cause problems for DeallocateAllImages??
// Um, err, I sure hope not...


void finiObjectsExceptDD(void)
{
	if (WindowMode == WindowModeSubWindow)
	  {
	   if (lpDDClipper != NULL)
	     {
	      lpDDClipper->Release();
		  lpDDClipper = NULL;
		 }
	  }

	if (lpDDPal[0] != NULL) // should be killed neway???
	  {
	   lpDDPal[0]->Release();
	   lpDDPal[0] = NULL;
	  }

	if (lpDDSBack != NULL)
	  {
	   lpDDSBack->Release();
	   lpDDSBack = NULL;
	  }

    if (lpDDSPrimary != NULL)
      {
       lpDDSPrimary->Release();
       lpDDSPrimary = NULL;
      }
}


// This will hopefully allow us to change
// the palette at runtime, assuming that
// we are in a palettised mode.  It will
// wait for vertical blank to make it work 
// on all video cards (I hope).  Note that
// the palette should be passed in engine
// internal format, i.e. 6 bits, and will be
// converted to the DirectDraw format internally.

// Note we assume entry 0 in the lpDDPal array!!!

// Note for 222 mode we set the FIRST 64 entries!!!
// Not certain that's right, actually...

int ChangePalette (unsigned char* NewPalette)

{
	unsigned char DDPalette[1024]; // 256 colours, 4 bytes each
	int NumEntries;

    // Check for palettised mode
	// Note we must use VideoModeTypeScreen
	// due to nature of Direct3D ramp driver
	// interface.

    if ((VideoModeTypeScreen != VideoModeType_8) &&
	   (VideoModeTypeScreen != VideoModeType_8T))
	  return No;

    // Check for FullScreen mode
	// if (WindowMode != WindowModeFullScreen)
	//  return No;

    // Set up number of entries to change
	if (ScreenDescriptorBlock.SDB_Flags & SDB_Flag_222)
	  NumEntries = 256; // actually, this seems to be right...
	else if ((ScreenDescriptorBlock.SDB_Flags & SDB_Flag_Raw256)
	        || (VideoModeTypeScreen == VideoModeType_8T))
	  NumEntries = 256;
	else
	  return No; // undefined behaviour

    // Convert to DirectDraw 4 bytes, 8 bit format
    // with all flag entries set to zero

    ConvertToDDPalette(NewPalette, DDPalette, NumEntries, 0);

    // Wait for beginning of next vertical
	// blanking interval
    lpDD->WaitForVerticalBlank(DDWAITVB_BLOCKBEGIN, NULL);

    // Set all entries in palette to new values
    lpDDPal[0]->SetEntries(0,0,NumEntries,(LPPALETTEENTRY)
          DDPalette);

    return Yes;
}

// At some stage it may be worth expanding this function
// to get more than video memory, or replacing it
// with one that uses GetCaps to fill out a general
// structure.

int GetAvailableVideoMemory(void)
{
  DDCAPS          ddcaps;
  HRESULT         ddrval;

  // Get caps for DirectDraw object
  memset(&ddcaps, 0, sizeof(ddcaps));
  ddcaps.dwSize = sizeof(ddcaps);
  ddrval = lpDD->GetCaps(&ddcaps, NULL);
	LOGDXERR(ddrval);

  if (ddrval != DD_OK)
  #if debug
     {
      ReleaseDirect3D();
      exit(ddrval);
     }
  #else
     return -1;
  #endif

  return (int) (ddcaps.dwVidMemFree);
}

/*
  This is designed to allow for ModeX
  emulation failures where the system
  boots up in a low memory cost mode
  which actually takes up much more than 
  a high cost mode because GDI cannot be
  evicted.  Note that the VideoMode reset
  done here is in any case a last ditch 
  measure, since the higher res version will
  be much slower as well as (normally) more
  costly in terms of memory.

  It will also now do start errors due to the 
  driver not being able to change to a mode of
  a different colour depth, and this not being reported
  by the DirectDraw object's enum display modes
  callback.  Hopefully.

  NOTE THAT THIS MUST BE CALLED FROM YOUR WINMAIN
  AFTER SETVIDEOMODE IF IT IS TO WORK.

  NOTE ALSO THAT THE ACTIVATION CODE FOR THIS IS
  IN GENERATEDIRECTDRAWSURFACE AND WILL NOT BE TURNED
  ON UNLESS -->DEBUG<-- IS -->OFF<--.
*/

void HandleVideoModeRestarts(HINSTANCE hInstance, int nCmdShow)
{
    if (AttemptVideoModeRestart)
	  {
	   switch (VideoRestartMode)
	     {
		  case RestartDisplayModeNotAvailable:
		     {
			  // Rewrite mode as equivalent resolution,
			  // different bit depth, before trying again.
			  // Note this must be kept up to date!!!
			  switch (VideoMode)
			    {
			     case VideoMode_DX_320x200x8:
				   {
				    ReleaseDirect3D();
					exit(0xfedd);
				   }
				   VideoMode = VideoMode_DX_320x200x15;
				   break;

			     case VideoMode_DX_320x200x8T:
				   VideoMode = VideoMode_DX_320x200x15;
				   break;

			     case VideoMode_DX_320x200x15:
				   VideoMode = VideoMode_DX_320x200x8T;
				   break;

			     case VideoMode_DX_320x240x8:
				   VideoMode = VideoMode_DX_320x200x15;
				   break;

			     case VideoMode_DX_640x480x8:
				   VideoMode = VideoMode_DX_640x480x15;
				   break;

			     case VideoMode_DX_640x480x8T:
				   VideoMode = VideoMode_DX_640x480x15;
				   break;

			     case VideoMode_DX_640x480x15:
				   VideoMode = VideoMode_DX_640x480x8T;
                   break;

			     case VideoMode_DX_640x480x24:
				   VideoMode = VideoMode_DX_640x480x15;
				   break;

				 default: // hmm...
				   break;
				}

              // Clear variables
			  AttemptVideoModeRestart = No;
			  VideoRestartMode = NoRestartRequired;
              
              ChangeDisplayModes(hInstance, nCmdShow,
	             VideoMode, WindowRequestMode, 
	             ZBufferRequestMode, RasterisationRequestMode,
		         SoftwareScanDrawRequestMode, DXMemoryRequestMode);
			 }
		    break;

          case RestartOutOfVidMemForPrimary:
		     {
	          // Guess mode reset
	          // Note this must be kept up to date!!!
	          if ((VideoMode == VideoMode_DX_320x200x8) ||
	            (VideoMode == VideoMode_DX_320x200x8T) ||
		        (VideoMode == VideoMode_DX_320x240x8))
		        VideoMode = VideoMode_DX_640x480x8;
	          else if (VideoMode == VideoMode_DX_320x200x15)
	            VideoMode = VideoMode_DX_640x480x15;

              // Clear variables
			  AttemptVideoModeRestart = No;
			  VideoRestartMode = NoRestartRequired;
              
              ChangeDisplayModes(hInstance, nCmdShow,
	             VideoMode, WindowRequestMode, 
	             ZBufferRequestMode, RasterisationRequestMode,
		         SoftwareScanDrawRequestMode, DXMemoryRequestMode);
			 }
		    break;

          default: // err...
		    break;
		 }
	  }
}


// Take a 24 bit RGB colour and return it in the 
// correct format for the current primary surface
// We will assume that the primary is NOT in a 
// palettised mode, since the mapping if we have 
// already set a palette is undefined.

int GetSingleColourForPrimary(int Colour)

{
    unsigned long m;
    int s;
    int red_shift, red_scale;
    int green_shift, green_scale;
    int blue_shift, blue_scale;
	int RetVal = 0;
	int r, g ,b;

    // Fail for palettised modes
	if ((VideoModeTypeScreen == VideoModeType_8)
	   || (VideoModeTypeScreen == VideoModeType_8T))
	  return -1;

    // Extra! check for palettised modes
	if ((DisplayPixelFormat.dwFlags & DDPF_PALETTEINDEXED8)
	   || (DisplayPixelFormat.dwFlags & DDPF_PALETTEINDEXED4))
	  return -1;

    // Determine r, g, b masks and scale
    for (s = 0, m = DisplayPixelFormat.dwRBitMask; 
       !(m & 1); s++, m >>= 1);

    red_shift = s;
    red_scale = 255 / (DisplayPixelFormat.dwRBitMask >> s);

    for (s = 0, m = DisplayPixelFormat.dwGBitMask; 
       !(m & 1); s++, m >>= 1);

    green_shift = s;
    green_scale = 255 / (DisplayPixelFormat.dwGBitMask >> s);

    for (s = 0, m = DisplayPixelFormat.dwBBitMask; 
       !(m & 1); s++, m >>= 1);

    blue_shift = s;
    blue_scale = 255 / (DisplayPixelFormat.dwBBitMask >> s);

    // Extract r,g,b components from input colour
	r = (Colour >> 16) & 0xff;
	g = (Colour >> 8) & 0xff;
	b = Colour & 0xff;

	// Scale and shift each value
    r /= red_scale;
    g /= green_scale;
    b /= blue_scale;
    RetVal = (r << red_shift) | (g << green_shift) 
           | (b << blue_shift);

    return(RetVal);
}

#if triplebuffer
// NOTE TRIPLE BUFFERING SUPPORT HAS BEEN -->REMOVED<--
// ON THE GROUNDS THAT THE SECOND BACK BUFFER TAKES UP
// EXTRA MEMORY AND IT'S NOT GOING TO GO ANY FASTER
// ANYWAY UNLESS YOUR VIDEO CARD FLIPS IN HARDWARE AND
// TAKES MORE THAN ABOUT 10 MILLISECONDS TO DO IT.
// WHICH I CERTAINLY HOPE ISN'T TRUE...

// NOTE ALSO: THIS FUNCTION DOESN'T WORK! (DESPITE
// THE DOCUMENTATION).  YOU NEED TO USE GETATTACHEDSURFACE
// TWICE INSTEAD.  SEE ARSEWIPE CODE FOR A WORKING
// EXAMPLE (NOTE THAT TRIPLE BUFFERING CAN BE OPTIMAL
// IN ARSEWIPE BECAUSE IT'S 2D, AND THE TIME TO THE NEXT
// SURFACE LOCK IN A RENDERING CYCLE CAN THUS BE MUCH LESS
// THAN IN 3DC).

// defines this function as FAR PASCAL, i.e.
// using Windows standard parameter passing convention,
// which will be neecessary for all callbacks

HRESULT WINAPI InitTripleBuffers(LPDIRECTDRAWSURFACE lpdd, 
	 LPDDSURFACEDESC lpsd, LPVOID lpc)
{
	if ((lpsd->ddsCaps.dwCaps & DDSCAPS_FLIP) &&
	   (!(lpsd->ddsCaps.dwCaps & DDSCAPS_FRONTBUFFER))
	   && (BackBufferEnum < MaxBackBuffers))
	  lpDDSBack[BackBufferEnum++] = lpdd;

    if (BackBufferEnum == MaxBackBuffers)
	  return(DDENUMRET_CANCEL);
	else
	  return(DDENUMRET_OK);
}
#endif



/* new versions of these functions, provided by neil */
static GDIObjectReferenceCount = 0;
BOOL GetGDISurface(void)
{
	HRESULT hr = DD_OK;

	if(WindowMode	== WindowModeSubWindow) return TRUE;
		
	if(!lpDDGDI)
	{
		hr = lpDD->GetGDISurface(&lpDDGDI);	
		if(hr != DD_OK)
		{
			ReleaseDirect3D();
			exit(hr);
		}
   	}

	// flip the buffer to the GDI surface 
	hr = lpDD->FlipToGDISurface();
	if(hr != DD_OK)
	{
		ReleaseDirect3D();
		exit(hr);
	}
    
    GDIObjectReferenceCount++;

    // release windows palette entries -
	// whenever we flip to the GDI surface,
	// we will want the full palette.

	if (VideoModeColourDepth == 8)
	{
		HDC hdc = GetDC(NULL);
	   	SetSystemPaletteUse(hdc, SYSPAL_STATIC);
       	ReleaseDC(NULL, hdc);
	}
   
  	return TRUE;
}


BOOL LeaveGDISurface(void)
{
	HRESULT hr = DD_OK;

	if(WindowMode == WindowModeSubWindow) return TRUE;
		
    if (GDIObjectReferenceCount <= 1)
	{
 	   ReleaseDDSurface(lpDDGDI);
	}

    // recapture system static palette
	// entries (see note above in
	// GetGDISurface... )

	if (VideoModeColourDepth == 8)
	{
	   HDC hdc = GetDC(NULL);
	   SetSystemPaletteUse(hdc, SYSPAL_NOSTATIC);
       ReleaseDC(NULL, hdc);
	}

	return TRUE;
}

/************ for extern "C"*****************/

};

