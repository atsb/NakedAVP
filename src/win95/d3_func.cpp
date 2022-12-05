
// Interface  functions (written in C++) for
// Direct3D immediate mode system

// Must link to C code in main engine system

extern "C" {

// Mysterious definition required by objbase.h 
// (included via one of the include files below)
// to start definition of obscure unique in the
// universe IDs required  by Direct3D before it
// will deign to cough up with anything useful...

#define INITGUID

#include "3dc.h"

#include "awTexLd.h"

#include "dxlog.h"
#include "module.h"
#include "inline.h"

#include "d3_func.h"
#include "d3dmacs.h"

#include "string.h"

#include "kshape.h"
#include "eax.h"
#include "vmanpset.h"

extern "C++" {
#include "chnktexi.h"
#include "chnkload.hpp" // c++ header which ignores class definitions/member functions if __cplusplus is not defined ?
#include "r2base.h"
}

#define UseLocalAssert No
#include "ourasert.h"



// FIXME!!! Structures in d3d structure
// never have any size field set!!!
// This is how it's done in Microsoft's
// demo code --- but ARE THEY LYING???

// As far as I know the execute buffer should always be in
// system memory on any configuration, but this may
// eventually have to be changed to something that reacts
// to the caps bit in the driver, once drivers have reached
// the point where we can safely assume that such bits will be valid.
#define ForceExecuteBufferIntoSystemMemory Yes

// To define TBLEND mode --- at present
// it must be on for ramp textures and
// off for evrything else...
#define ForceTBlendCopy No

// Set to Yes for debugging, to No for normal
// operations (i.e. if we need a palettised
// file for an accelerator, load it from
// pre-palettised data, using code not yet 
// written as of 27 / 8/ 96)
#define QuantiseOnLoad Yes

// Set to Yes to make default texture filter bilinear averaging rather
// than nearest
BOOL BilinearTextureFilter = 1;

extern LPDIRECTDRAW lpDD;

#if 0//
// Externs

extern int VideoMode;
extern int DXMemoryMode;
extern int ZBufferRequestMode;
extern int RasterisationRequestMode;
extern int SoftwareScanDrawRequestMode;
extern SCREENDESCRIPTORBLOCK ScreenDescriptorBlock;
extern VIEWDESCRIPTORBLOCK* Global_VDB_Ptr;
extern IMAGEHEADER ImageHeaderArray[];
extern BOOL MMXAvailable;


//Globals

int D3DDriverMode;



static unsigned char DefaultD3DTextureFilterMin;
static unsigned char DefaultD3DTextureFilterMax;


#if SuppressWarnings
static int* itemptr_tmp;
#endif


#endif //
HRESULT LastError;
int ExBufSize;

LPDIRECT3DEXECUTEBUFFER lpD3DExecCmdBuf;
LPDIRECTDRAWSURFACE lpZBuffer;
extern LPDIRECTDRAWSURFACE lpDDSBack;
extern LPDIRECTDRAWSURFACE lpDDSPrimary;
extern LPDIRECTDRAWPALETTE lpDDPal[]; // DirectDraw palette

D3DINFO d3d;
BOOL D3DHardwareAvailable;

int StartDriver;
int StartFormat;


static int devZBufDepth;


extern int WindowMode;
extern int ZBufferMode;
extern int ScanDrawMode;
extern int VideoModeColourDepth;

extern enum TexFmt { D3TF_4BIT, D3TF_8BIT, D3TF_16BIT, D3TF_32BIT, D3TF_MAX } d3d_desired_tex_fmt;

// Callback function to enumerate devices present
// on system (required so that device interface GUID
// can be retrieved if for no other reason).  Device
// information is copied into instance of structure
// defined in d3_func.h

HRESULT WINAPI DeviceEnumerator(LPGUID lpGuid,
        LPSTR lpDeviceDescription, LPSTR lpDeviceName,
        LPD3DDEVICEDESC lpHWDesc, LPD3DDEVICEDESC lpHELDesc, LPVOID lpContext)
{
    int *lpStartDriver = (int *)lpContext;

    /*
      Record the D3D driver's description
	  of itself.
    */

    memcpy(&d3d.Driver[d3d.NumDrivers].Guid, lpGuid, sizeof(GUID));
    strcpy(d3d.Driver[d3d.NumDrivers].About, lpDeviceDescription);
    strcpy(d3d.Driver[d3d.NumDrivers].Name, lpDeviceName);

    /*
      Is this a hardware device or software emulation?  Checking the color
      model for a valid model works.
    */

    if (lpHWDesc->dcmColorModel)
	  {
	   D3DHardwareAvailable = Yes;
       d3d.Driver[d3d.NumDrivers].Hardware = Yes;
       memcpy(&d3d.Driver[d3d.NumDrivers].Desc, lpHWDesc,
               sizeof(D3DDEVICEDESC));
      } 
    else 
      {
       d3d.Driver[d3d.NumDrivers].Hardware = No;
       memcpy(&d3d.Driver[d3d.NumDrivers].Desc, lpHELDesc,
               sizeof(D3DDEVICEDESC));
      }

    /*
      Does this driver do texture mapping?
    */

    d3d.Driver[d3d.NumDrivers].Textures =
        (d3d.Driver[d3d.NumDrivers].Desc.dpcTriCaps.dwTextureCaps &
         D3DPTEXTURECAPS_PERSPECTIVE) ? TRUE : FALSE;

    /*
      Can this driver use a z-buffer?
    */

    d3d.Driver[d3d.NumDrivers].ZBuffer =
        d3d.Driver[d3d.NumDrivers].Desc.dwDeviceZBufferBitDepth & (DDBD_16 | DDBD_24 | DDBD_32)
                ? TRUE : FALSE;

// The driver description is recorded here,
// and some basic things like ZBuffering 
// availability are noted separately.  Other
// things such as hardware acceleration for 
// translucency could potentially be noted here
// for use in the Write functions to decide 
// whether e.g. iflag_transparent should be 
// treated as valid.  Obviously this will
// require modification of the D3DDRIVERINFO
// structure in d3_func.h

    *lpStartDriver = d3d.NumDrivers;

    d3d.NumDrivers++;
    if (d3d.NumDrivers == MAX_D3D_DRIVERS)
      return (D3DENUMRET_CANCEL);
	else
      return (D3DENUMRET_OK);
}

// This function is called from 
// InitialiseDirect3DImmediateMode, to
// insert and execute opcodes which cannot
// be run during a "real" scene because of
// some obscure feature of Direct3D.



// Initialise Direct3D immediate mode system

BOOL InitialiseDirect3DImmediateMode(void)
{
    BOOL RetVal;
// to tell device enum function that it has not been called before
	StartDriver = -1; 
// to tell texture enum function that it has not been called before
    StartFormat = -1;

// default is no hardware
// note that we are still resetting 
// this here just in case the test from
// InitialiseSystem failed and things aren't
// what we thought...
	D3DHardwareAvailable = No;

// Zero d3d structure
    memset(&d3d, 0, sizeof(D3DINFO));

//  Set up Direct3D interface object
  									 
	LastError = lpDD->QueryInterface(IID_IDirect3D, (LPVOID*) &d3d.lpD3D);
	LOGDXERR(LastError);

    if (LastError != DD_OK)
	  return FALSE;
// Use callback function to enumerate available devices on system
// and acquire device GUIDs etc
// note that we are still resetting 
// this here just in case the test from
// InitialiseSystem failed and things aren't
// what we thought...
    LastError = d3d.lpD3D->EnumDevices(DeviceEnumerator, (LPVOID)&StartDriver);
	LOGDXERR(LastError);

    if (LastError != D3D_OK)
      return FALSE;

// Must be run as soon as possible in the 
// initialisation sequence, but after the
// device enumeration (and obviously after
// DirectDraw object and surface initialisation).
    SelectD3DDriverAndDrawMode();
	d3d.ThisDriver = d3d.Driver[d3d.CurrentDriver].Desc;

// Test!!! Release D3D object if we don't need it and do an
// early exit.  Will this fix the banding problem on some
// accelerators in palettised modes??
// Evidently not.  Still, probably a good thing to do...
// But!!! The whole banding problem appears to be a 
// modeX emulation problem on some 3D accelerator cards...
   #if 1
   if (ScanDrawMode == ScanDrawDirectDraw)
     {
	  ReleaseDirect3DNotDDOrImages();
	  return TRUE;
	 }
   #endif


//  Note that this must be done BEFORE the D3D device object is created
    #if SupportZBuffering
    if (ZBufferMode != ZBufferOff)
	{
    	RetVal = CreateD3DZBuffer();
		if (RetVal == FALSE) return FALSE;
	}
	#endif

//  Set up Direct3D device object (must be linked to back buffer)
    LastError = lpDDSBack->QueryInterface(d3d.Driver[d3d.CurrentDriver].Guid,
               (LPVOID*)&d3d.lpD3DDevice);
	LOGDXERR(LastError);

    if (LastError != DD_OK) 
      return FALSE;

	AW_TL_ERC awErr = AwSetD3DDevice(d3d.lpD3DDevice);
    GLOBALASSERT(AW_TLE_OK==awErr);
// Enumerate texture formats and pick one
// (palettised if possible).
    d3d.NumTextureFormats = 0;

	d3d_desired_tex_fmt=D3TF_8BIT;

    LastError = d3d.lpD3DDevice->EnumTextureFormats
              (TextureFormatsEnumerator,
              (LPVOID)&StartFormat);
	LOGDXERR(LastError);

    if (LastError != D3D_OK) 
    #if debug
      {
	   ReleaseDirect3D();
	   exit(LastError);
	  }
    #else
      return FALSE;
    #endif

    d3d.CurrentTextureFormat = StartFormat;

    // NEW NEW NEW
    // Note: we are NOT restricted to only one texture format
    awErr = AwSetTextureFormat(&d3d.TextureFormat[StartFormat].ddsd);
    GLOBALASSERT(AW_TLE_OK==awErr);

// Create viewport
	LastError = d3d.lpD3D->CreateViewport(&d3d.lpD3DViewport, NULL);
	LOGDXERR(LastError);

   if (LastError != D3D_OK)
     return FALSE;

// Add viewport to desired device
   LastError = d3d.lpD3DDevice->AddViewport(d3d.lpD3DViewport);
	LOGDXERR(LastError);

   if (LastError != D3D_OK)
     return FALSE;

// Set up viewport data

// Note that the viewport is always set to the
// SDB limits because internal clipping is handled
// within the main engine code using the VDB
// system.

   {
    // Configure viewport here
    D3DVIEWPORT viewPort;
	memset(&viewPort, 0, sizeof(D3DVIEWPORT));
	viewPort.dwSize = sizeof(D3DVIEWPORT);
	viewPort.dwX = 0; // origins x and y
	viewPort.dwY = 0;
	viewPort.dwWidth = ScreenDescriptorBlock.SDB_Width;
	viewPort.dwHeight = ScreenDescriptorBlock.SDB_Height;
	viewPort.dvScaleX = D3DVAL((float)viewPort.dwWidth / 2.0); // ????was 2.0
	viewPort.dvScaleY = D3DVAL((float)viewPort.dwHeight  / 2.0); // ????was 2.0
    viewPort.dvMaxX = D3DVAL(D3DDivide(D3DVAL(viewPort.dwWidth), 
                  D3DVAL(2 * viewPort.dvScaleX))); // ????
    viewPort.dvMaxY = D3DVAL(D3DDivide(D3DVAL(viewPort.dwHeight),
                  D3DVAL(2 * viewPort.dvScaleY))); // ????

    // And actually set viewport
    LastError = d3d.lpD3DViewport->SetViewport(&viewPort);
	LOGDXERR(LastError);

    if (LastError != D3D_OK)
	  return FALSE;
   }

// At present we will not set a background material for the
// viewport, staying instead with the blit code in backdrop.c

// Also, we are not currently adding default lights to the
// viewport, since the engine lighting system is completely
// disconnected from the immediate mode lighting module.

// Create execute buffer

  {
   // Locals for execute buffer
   D3DEXECUTEBUFFERDESC d3dexDesc;

   // Set up structure to initialise buffer
   // Note some instructions (e.g. lines) may be smaller than a
   // triangle, but none can be larger
   ExBufSize = ((sizeof(D3DINSTRUCTION) + sizeof(D3DTRIANGLE))
                * MaxD3DInstructions)
			    + (sizeof(D3DTLVERTEX) * MaxD3DVertices);
   memset(&d3dexDesc, 0, sizeof(D3DEXECUTEBUFFERDESC));
   d3dexDesc.dwSize = sizeof(D3DEXECUTEBUFFERDESC);
   d3dexDesc.dwFlags = D3DDEB_BUFSIZE | D3DDEB_CAPS;
   d3dexDesc.dwBufferSize = ExBufSize;
   #if ForceExecuteBufferIntoSystemMemory
   d3dexDesc.dwCaps = D3DDEBCAPS_SYSTEMMEMORY;
   #else
   d3dexDesc.dwCaps = D3DDEBCAPS_MEM; // untested!!!
   #endif

   // Create buffer
   LastError = d3d.lpD3DDevice->CreateExecuteBuffer
            (&d3dexDesc, &lpD3DExecCmdBuf, NULL);
	LOGDXERR(LastError);

   if (LastError != D3D_OK)
     return FALSE;
  }

  // Temporary patch here because the defaults function
  // buggers palette setting in ScanDrawDirectDraw
  // for some really obscure reason...

  if (ScanDrawMode != ScanDrawDirectDraw)
    SetExecuteBufferDefaults();


  return TRUE; 	   
}

#if 1

// Note that error conditions have been removed
// on the grounds that an early exit will prevent 
// EndScene being run if this function is used,
// which screws up all subsequent buffers

BOOL RenderD3DScene(void)

{
    // Begin scene
	// My theory is that the functionality of this
	// thing must invoke a DirectDraw surface lock
	// on the back buffer without telling you.  However,
	// we shall see...

	LastError = d3d.lpD3DDevice->BeginScene();
	LOGDXERR(LastError);

	// if (LastError != D3D_OK)
	//  return FALSE;

	// Execute buffer
	LastError = d3d.lpD3DDevice->Execute(lpD3DExecCmdBuf, 
	          d3d.lpD3DViewport, D3DEXECUTE_UNCLIPPED);
	LOGDXERR(LastError);

	// if (LastError != D3D_OK)
	//  return FALSE;

	// End scene
    LastError = d3d.lpD3DDevice->EndScene();
	LOGDXERR(LastError);

	// if (LastError != D3D_OK)
	//  return FALSE;

	   

	return TRUE;
}

#else

int Time1, Time2, Time3, Time4;

BOOL RenderD3DScene(void)

{

    // Begin scene
	// My theory is that the functionality of this
	// thing must invoke a DirectDraw surface lock
	// on the back buffer without telling you.  However,
	// we shall see...

    Time1 = GetWindowsTickCount();

	LastError = d3d.lpD3DDevice->BeginScene();
	LOGDXERR(LastError);

	if (LastError != D3D_OK)
	  return FALSE;

    Time2 = GetWindowsTickCount();

	// Execute buffer
	#if 1
	LastError = d3d.lpD3DDevice->Execute(lpD3DExecCmdBuf, 
	          d3d.lpD3DViewport, D3DEXECUTE_UNCLIPPED);
	LOGDXERR(LastError);
	#else
	LastError = d3d.lpD3DDevice->Execute(lpD3DExecCmdBuf, 
	          d3d.lpD3DViewport, D3DEXECUTE_CLIPPED);
	LOGDXERR(LastError);
	#endif

	if (LastError != D3D_OK)
	  return FALSE;

    Time3 = GetWindowsTickCount();

	// End scene
    LastError = d3d.lpD3DDevice->EndScene();
	LOGDXERR(LastError);

	if (LastError != D3D_OK)
	  return FALSE;

    Time4 = GetWindowsTickCount();

	   

	return TRUE;

#endif


// With a bit of luck this should automatically
// release all the Direct3D and  DirectDraw
// objects using their own functionality.
// A separate call to finiObjects 
// is not required.
// NOTE!!! This depends on Microsoft macros
// in d3dmacs.h, which is in the win95 directory
// and must be upgraded from sdk upgrades!!!

void ReleaseDirect3D(void)

{
    DeallocateAllImages();
    RELEASE(d3d.lpD3DViewport);
    RELEASE(d3d.lpD3DDevice);
	#if SupportZBuffering
    RELEASE(lpZBuffer);
	#endif
    RELEASE(lpDDPal[0]);
    RELEASE(lpDDSBack);
    RELEASE(lpDDSPrimary);
    RELEASE(d3d.lpD3D);
    RELEASE(lpDD);

	/* release Direct Input stuff */
	ReleaseDirectKeyboard();
	ReleaseDirectMouse();
	ReleaseDirectInput();

	// Reset windows palette entry allocation
	if ((VideoModeColourDepth == 8) && (WindowMode == WindowModeSubWindow))
	  {
	   HDC hdc = GetDC(NULL);
	   SetSystemPaletteUse(hdc, SYSPAL_STATIC);
       ReleaseDC(NULL, hdc);
	  }
}

// Release all Direct3D objects 
// but not DirectDraw

void ReleaseDirect3DNotDDOrImages(void)

{
    RELEASE(d3d.lpD3DViewport);
    RELEASE(d3d.lpD3DDevice);
	#if SupportZBuffering
    RELEASE(lpZBuffer);
	#endif
    RELEASE(d3d.lpD3D);
}

void ReleaseDirect3DNotDD(void)

{
    DeallocateAllImages();
    RELEASE(d3d.lpD3DViewport);
    RELEASE(d3d.lpD3DDevice);
	#if SupportZBuffering
    RELEASE(lpZBuffer);
	#endif
    RELEASE(d3d.lpD3D);
}


// NOTE!!! These functions depend on Microsoft macros
// in d3dmacs.h, which is in the win95 directory
// and must be upgraded from sdk upgrades!!!


// ALSO NOTE!!!  All this stuff involves heavy
// use of floating point assembler in software
// emulation, probably hand parallelised between
// the FPU and the processor or something bizarre,
// implying that this stuff SHOULD NOT be used
// one anything below a Pentium.

// AND AGAIN!!! Due to the nature of the item 
// format, the  rasterisation module MUST RECEIVE
// repeated vertices in the data area.  Tough shit,
// Microsoft, that's what I say...

void WritePolygonToExecuteBuffer(int* itemptr)

{
}

void WriteGouraudPolygonToExecuteBuffer(int* itemptr)

{
}

void Write2dTexturedPolygonToExecuteBuffer(int* itemptr)

{
}

void WriteGouraud2dTexturedPolygonToExecuteBuffer(int* itemptr)

{
}


void Write3dTexturedPolygonToExecuteBuffer(int* itemptr)

{
 }

void WriteGouraud3dTexturedPolygonToExecuteBuffer(int* itemptr)

{
}

#if SupportZBuffering

// To make effective use of 16 bit z buffers (common
// on hardware accelerators) we must have a z coordinate 
// which has been transformed into screen space in such a
// way that linearity and planearity are preserved, i.e. as
// if we had done a homogeneous transformation.  For the case 
// in which the far clipping plane is much further away than the
// near one (effectively true for 3dc, especially as there is
// no far clipping plane as such), the appropriate transformation
// can be reduced to zScreen = (ZWorld - ZNear) / ZWorld.  This
// calculation is therefore (unfortunately) done for each vertex
// for z buffered items, taking ZNear to be the current
// VDB_ClipZ * GlobalScale.


void WriteZBPolygonToExecuteBuffer(int* itemptr)

{
}

void WriteZBGouraudPolygonToExecuteBuffer(int* itemptr)

{
}

void WriteZB2dTexturedPolygonToExecuteBuffer(int* itemptr)

{
}


void WriteZBGouraud2dTexturedPolygonToExecuteBuffer(int* itemptr)

{
}

void WriteZB3dTexturedPolygonToExecuteBuffer(int* itemptr)

{
}

void WriteZBGouraud3dTexturedPolygonToExecuteBuffer(int* itemptr)

{
}

#endif
	


void WriteBackdrop2dTexturedPolygonToExecuteBuffer(int* itemptr)

{
}

// Same as ordinary 2d textured draw at present, but may e.g.
// require different texture wrapping behaviour or
// w values.

void WriteBackdrop3dTexturedPolygonToExecuteBuffer(int* itemptr)

{
}


// Note that this is all dead crap and deeply unoptimised
// But then... a) it's really only a test and 
// b) it's for the tools group anyway...


void DirectWriteD3DLine(VECTOR2D* LineStart, VECTOR2D* LineEnd, int LineColour)

{

}


// reload D3D image -- assumes a base pointer points to the image loaded
// from disc, in a suitable format

void ReloadImageIntoD3DImmediateSurface(IMAGEHEADER* iheader)
{

    void *reloadedTexturePtr = ReloadImageIntoD3DTexture(iheader);
	LOCALASSERT(reloadedTexturePtr != NULL);
	
	int gotTextureHandle = GetTextureHandle(iheader);
	LOCALASSERT(gotTextureHandle == TRUE);
}

void* ReloadImageIntoD3DTexture(IMAGEHEADER* iheader)
{
	// NOTE FIXME BUG HACK
	// what if the image was a DD surface ??
	
	if (iheader->hBackup)
	{
		iheader->D3DTexture = AwCreateTexture("rf",AW_TLF_PREVSRC|AW_TLF_COMPRESS);
		return iheader->D3DTexture;
	}
	else return NULL;
}

int GetTextureHandle(IMAGEHEADER *imageHeaderPtr)
{
 	LPDIRECT3DTEXTURE Texture = (LPDIRECT3DTEXTURE) imageHeaderPtr->D3DTexture;
	
	LastError = Texture->GetHandle(d3d.lpD3DDevice, (D3DTEXTUREHANDLE*)&(imageHeaderPtr->D3DHandle));

	if (LastError != D3D_OK) return FALSE;

	return TRUE;
}



void ReleaseD3DTexture(void* D3DTexture)

{
	LPDIRECT3DTEXTURE lpTexture;

    lpTexture = (LPDIRECT3DTEXTURE) D3DTexture;
	RELEASE(lpTexture);
}


#if SupportZBuffering

BOOL CreateD3DZBuffer(void)

{
    DDSURFACEDESC ddsd;

    // For safety, kill any existing z buffer
	#if SupportZBuffering
	RELEASE(lpZBuffer);
	#endif

    
	// If we do not have z buffering support
	// on this driver, give up now
	if (!(d3d.Driver[d3d.CurrentDriver].ZBuffer))
	  return FALSE;

    memset(&ddsd,0,sizeof(DDSURFACEDESC));
    ddsd.dwSize = sizeof(DDSURFACEDESC);
    ddsd.dwFlags = (DDSD_WIDTH | DDSD_HEIGHT | DDSD_CAPS | DDSD_ZBUFFERBITDEPTH);
    ddsd.dwHeight = ScreenDescriptorBlock.SDB_Height;
    ddsd.dwWidth = ScreenDescriptorBlock.SDB_Width;
    ddsd.ddsCaps.dwCaps = DDSCAPS_ZBUFFER;

	// If we are on a hardware driver, then the z buffer
	// MUST be in video memory.  Otherwise, it MUST be
	// in system memory.  I think.

    if (d3d.Driver[d3d.CurrentDriver].Hardware)
        ddsd.ddsCaps.dwCaps |= DDSCAPS_VIDEOMEMORY;
    else
        ddsd.ddsCaps.dwCaps |= DDSCAPS_SYSTEMMEMORY;

    // Get the Z buffer bit depth from this driver's 
    // D3D device description and add it to the description
	// of the surface we want to create

    devZBufDepth = d3d.Driver[d3d.CurrentDriver].Desc.dwDeviceZBufferBitDepth;

    if (devZBufDepth & DDBD_32)
        ddsd.dwZBufferBitDepth = 32;
    else if (devZBufDepth & DDBD_24)
        ddsd.dwZBufferBitDepth = 24;
    else if (devZBufDepth & DDBD_16)
        ddsd.dwZBufferBitDepth = 16;
    else if (devZBufDepth & DDBD_8)
        ddsd.dwZBufferBitDepth = 8;
    else 
	  {
	   #if debug
	   ReleaseDirect3D();
	   exit(0x511621);
	   #else
	   return FALSE;
	   #endif
	  }

    // Eight-bit z buffer? Fuck off.
	if (ddsd.dwZBufferBitDepth == 8)
	  return FALSE;

    // Now we must actually make the z buffer

    LastError = lpDD->CreateSurface(&ddsd,&lpZBuffer, NULL);

    if (LastError != DD_OK)
	{
	   RELEASE(lpZBuffer);
	   return FALSE;
	}

    LastError = lpDDSBack->AddAttachedSurface(lpZBuffer);

    if (LastError != DD_OK)
	{
	   RELEASE(lpZBuffer);
	   return FALSE;
	}

	return TRUE;
}

#define ZFlushVal 0xffffffff

// At present we are using my z flush function, with the
// (undocumented) addition of a fill colour for the 
// actual depth fill, since it seems to work and at least
// I know what it does.  If it starts failing we'll probably
// have to go back to invoking the viewport clear through 
// Direct3D.

void FlushD3DZBuffer(void)

{

	DDBLTFX ddbltfx;

    memset(&ddbltfx, 0, sizeof(ddbltfx));
	ddbltfx.dwSize = sizeof(ddbltfx);
	ddbltfx.dwFillDepth	= devZBufDepth;
	ddbltfx.dwFillColor	= ZFlushVal;
	
	/* lets blt a color to the surface*/
	LastError = lpZBuffer->Blt(NULL, NULL, NULL, DDBLT_DEPTHFILL | DDBLT_WAIT, &ddbltfx);

}
void SecondFlushD3DZBuffer(void)
{
	#if 1
	{
	   WriteEndCodeToExecuteBuffer();
  	   UnlockExecuteBufferAndPrepareForUse();
	   ExecuteBuffer();
  	   LockExecuteBuffer();
	}

	DDBLTFX ddbltfx;

    memset(&ddbltfx, 0, sizeof(ddbltfx));
	ddbltfx.dwSize = sizeof(ddbltfx);
	ddbltfx.dwFillDepth	= devZBufDepth;
	ddbltfx.dwFillColor	= ZFlushVal;
	
	/* lets blt a color to the surface*/
	LastError = lpZBuffer->Blt(NULL, NULL, NULL, DDBLT_DEPTHFILL | DDBLT_WAIT, &ddbltfx);
	#else
	extern void ClearZBufferWithPolygon(void);
	ClearZBufferWithPolygon();
	#endif
}


#endif
void FlushZB(void)
{
    HRESULT hRes; 
    D3DRECT d3dRect; 
 
 
    d3dRect.lX1 = 0; 
    d3dRect.lX2 = 640; 
    d3dRect.lY1 = 0; 
    d3dRect.lY2 = 480; 
    hRes = d3d.lpD3DViewport->Clear(1, &d3dRect, D3DCLEAR_ZBUFFER); 


}

// For extern "C"

};
 