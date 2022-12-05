extern "C" {

#include "3dc.h"
#include "inline.h"
#include "module.h"
#include "gamedef.h"
#include "stratdef.h"
#include "vramtime.h"

#include "dxlog.h"

#include "d3_func.h"
#include "d3dmacs.h"

#include "string.h"

#include "kshape.h"
#include "frustrum.h"

#include "d3d_hud.h"
#include "gamedef.h"

#include "particle.h"

#define UseLocalAssert No
#include "ourasert.h"
extern "C++"{
#include "r2base.h"
#include <math.h> // for sqrt
};
#include "HUD_layout.h"
#define HAVE_VISION_H 1
#include "vision.h"
#include "lighting.h"
#include "showcmds.h"
#include "frustrum.h"
#include "smacker.h"
#include "d3d_render.h"
#include "avp_userprofile.h"
#include "bh_types.h"

#include <string.h>


#define RGBLIGHT_MAKE(r,g,b) RGB_MAKE(r,g,b)
#define RGBALIGHT_MAKE(r,g,b,a) RGBA_MAKE(r,g,b,a)


// Set to Yes to make default texture filter bilinear averaging rather
// than nearest
extern BOOL BilinearTextureFilter;



#define FMV_ON 0
#define FMV_EVERYWHERE 0
#define WIBBLY_FMV_ON 0
#define FMV_SIZE 128
VECTORCH FmvPosition = {42985-3500,2765-5000,-35457};

#define FOG_ON 0
#if 1

#define FOG_COLOUR 0x7f7f7f //0x404040
#define FOG_SCALE 512
#else
#define FOG_COLOUR 0x7f7f7f//0xd282828 //0x404040
#define FOG_SCALE 256
#endif

#define TriangleVertices   3

#define LineVertices 2

#define DefaultVertexIntensity 200
#define TransparentAlphaValue 128

#define DefaultColour (RGBLIGHT_MAKE(DefaultVertexIntensity,DefaultVertexIntensity,DefaultVertexIntensity))
#define DefaultAlphaColour (RGBALIGHT_MAKE(DefaultVertexIntensity,DefaultVertexIntensity,DefaultVertexIntensity,TransparentAlphaValue))


#if 0
#define TxIntensityLShift 4
#else
#define TxIntensityLShift 0
#endif

#define TxIntensityLimit 255

#define MaxVerticesInExecuteBuffer (MaxD3DVertices)
// Colour is a 24-bit RGB colour, standard engine
// internal format.
// a is an 8 bit alpha value.
#define MAKE_ALPHACOLOUR(colour, a)   ((D3DCOLOR) (((a) << 24) | (colour)))

extern int SpecialFXImageNumber;
extern int SmokyImageNumber;
extern int ChromeImageNumber;
extern int HUDFontsImageNumber;
extern int BurningImageNumber;
extern int PredatorVisionChangeImageNumber;
extern int PredatorNumbersImageNumber;
extern int StaticImageNumber;
extern int AAFontImageNumber;
extern int WaterShaftImageNumber;

D3DTEXTUREHANDLE FMVTextureHandle[4];
D3DTEXTUREHANDLE NoiseTextureHandle;

int LightIntensityAtPoint(VECTORCH *pointPtr);

// Externs

extern int VideoMode;
extern int WindowMode;
extern int ScanDrawMode;
extern int DXMemoryMode;
extern int ZBufferRequestMode;
extern int RasterisationRequestMode;
extern int SoftwareScanDrawRequestMode;
extern SCREENDESCRIPTORBLOCK ScreenDescriptorBlock;
extern VIEWDESCRIPTORBLOCK* Global_VDB_Ptr;


extern IMAGEHEADER ImageHeaderArray[];
extern int NumImagesArray[];

extern LPDIRECTDRAW lpDD;
extern LPDIRECTDRAWSURFACE lpDDSPrimary;
extern BOOL MMXAvailable;
extern int NormalFrameTime;
int WireFrameMode;

extern int HUDScaleFactor;

//Globals

int D3DDriverMode;
int NumVertices; 

static LPVOID ExecuteBufferDataArea;
static LPVOID ExecuteBufferInstArea;
static LPVOID ExecBufDataPtr;
static LPVOID ExecBufInstPtr;
static LPVOID ExecBufStartInstPtr;
#if SupportZBuffering
#endif


static unsigned char D3DShadingMode;
static unsigned char D3DTexturePerspective;
static unsigned char D3DTBlendMode;
static unsigned char D3DAlphaMode;
static D3DTEXTUREHANDLE CurrTextureHandle;
static unsigned char DefaultD3DTextureFilterMin;
static unsigned char DefaultD3DTextureFilterMax;
// Globals for frame by frame definition of
// coloured materials for D3D rendering interface




static int NumberOfRenderedTriangles=0;
int NumberOfLandscapePolygons;
RENDERSTATES CurrentRenderStates;
extern HRESULT LastError;


void ChangeTranslucencyMode(enum TRANSLUCENCY_TYPE translucencyRequired);
void ChangeFilteringMode(enum FILTERING_MODE_ID filteringRequired);

#define CheckTranslucencyModeIsCorrect(x) \
if (CurrentRenderStates.TranslucencyMode!=(x)) \
	ChangeTranslucencyMode((x));

#define CheckFilteringModeIsCorrect(x) \
if (CurrentRenderStates.FilteringMode!=(x)) \
	ChangeFilteringMode((x));



extern D3DINFO d3d;

extern int ExBufSize;
extern LPDIRECT3DEXECUTEBUFFER lpD3DExecCmdBuf;





/* KJL 15:12:02 10/05/98 - FMV stuff */
static PALETTEENTRY FMVPalette[4][256];

void D3D_DrawFMVOnWater(int xOrigin, int yOrigin, int zOrigin);
static void UpdateFMVTextures(int maxTextureNumberToUpdate);
extern int NextFMVFrame(void*bufferPtr, int x, int y, int w, int h, int fmvNumber);
extern void UpdateFMVPalette(PALETTEENTRY *FMVPalette, int fmvNumber);

void D3D_DrawFMV(int xOrigin, int yOrigin, int zOrigin);

VECTORCH FMVParticleAbsPosition[450];
VECTORCH FMVParticlePosition[450];
int FMVParticleColour;

void D3D_DrawSwirlyFMV(int xOrigin, int yOrigin, int zOrigin);
void D3D_FMVParticle_Output(RENDERVERTEX *renderVerticesPtr);


void DrawFBM(void);



void D3D_DrawCable(VECTORCH *centrePtr, MATRIXCH *orientationPtr);


int WaterXOrigin;
int WaterZOrigin;
float WaterUScale;
float WaterVScale;

BOOL SetExecuteBufferDefaults(void)

{
    D3DEXECUTEBUFFERDESC d3dexDesc;
    D3DEXECUTEDATA d3dExecData;

    memset(&d3dexDesc, 0, sizeof(D3DEXECUTEBUFFERDESC));
    d3dexDesc.dwSize = sizeof(D3DEXECUTEBUFFERDESC);

	// Acquire lock on execute buffer
	LastError = lpD3DExecCmdBuf->Lock(&d3dexDesc);
	LOGDXERR(LastError);

	if (LastError != D3D_OK)
	  return FALSE;

    // Zero buffer and acquire pointers to start of areas
    memset(d3dexDesc.lpData, 0, ExBufSize);

    // Initialise pointers to data areas within buffer
    ExecuteBufferDataArea = d3dexDesc.lpData;
	ExecuteBufferInstArea = (LPVOID) 
	   ((int) ExecuteBufferDataArea
	   + (int) (sizeof(D3DTLVERTEX) * MaxVerticesInExecuteBuffer));

	ExecBufDataPtr = ExecuteBufferDataArea;
	ExecBufInstPtr = ExecuteBufferInstArea;

    NumVertices = 0;

// If we can, we want to turn off culling at the level of the rasterisation
// module.  Note that Microsoft's software renderers do not support this,
// unfortunately.
   #if 1
   if (ScanDrawMode == ScanDrawD3DHardwareRGB)
     {
      OP_STATE_RENDER(1, ExecBufInstPtr);
        STATE_DATA(D3DRENDERSTATE_CULLMODE, D3DCULL_NONE, ExecBufInstPtr);
	 }
	#endif

    #if SupportZBuffering
    OP_STATE_RENDER(1, ExecBufInstPtr);
    STATE_DATA(D3DRENDERSTATE_ZENABLE, TRUE, ExecBufInstPtr);


    // Certainly necess for new mixed HSR model!!!
    OP_STATE_RENDER(1, ExecBufInstPtr);
    STATE_DATA(D3DRENDERSTATE_ZWRITEENABLE, TRUE, ExecBufInstPtr);

    OP_STATE_RENDER(1, ExecBufInstPtr);
    STATE_DATA(D3DRENDERSTATE_ZFUNC, D3DCMP_LESSEQUAL, ExecBufInstPtr);
  
    #endif

    // Set default alpha modes with blending off
    OP_STATE_RENDER(1, ExecBufInstPtr);
       STATE_DATA(D3DRENDERSTATE_SRCBLEND,
       D3DBLEND_SRCALPHA, ExecBufInstPtr);

    OP_STATE_RENDER(1, ExecBufInstPtr);
       STATE_DATA(D3DRENDERSTATE_DESTBLEND,
       D3DBLEND_INVSRCALPHA, ExecBufInstPtr);

    OP_STATE_RENDER(1, ExecBufInstPtr);
      STATE_DATA(D3DRENDERSTATE_ALPHABLENDENABLE,
             FALSE, ExecBufInstPtr);

	#if FOG_ON
	OP_STATE_RENDER(1, ExecBufInstPtr);
	STATE_DATA(D3DRENDERSTATE_FOGENABLE, TRUE, ExecBufInstPtr);
	
	OP_STATE_RENDER(1, ExecBufInstPtr);
	STATE_DATA(D3DRENDERSTATE_FOGCOLOR, FOG_COLOUR, ExecBufInstPtr);
	#endif
	#if 0
	{
		float fogStart = 2000.0f; // these values work on a Voodoo2, but not a G200. Hmm.
		float fogEnd = 32000.0f;
		OP_STATE_RENDER(2, ExecBufInstPtr);
		STATE_DATA(D3DRENDERSTATE_FOGENABLE, TRUE, ExecBufInstPtr);
		STATE_DATA(D3DRENDERSTATE_FOGCOLOR, 0x7f7f7f, ExecBufInstPtr);

		#if 1
		OP_STATE_RENDER(3, ExecBufInstPtr);
		STATE_DATA(D3DRENDERSTATE_FOGTABLEMODE, D3DFOG_LINEAR, ExecBufInstPtr);
		STATE_DATA(D3DRENDERSTATE_FOGTABLESTART, *(DWORD*)(&fogStart), ExecBufInstPtr);
		STATE_DATA(D3DRENDERSTATE_FOGTABLEEND, *(DWORD*)(&fogEnd), ExecBufInstPtr);
		#endif
//		STATE_DATA(D3DRENDERSTATE_SPECULARENABLE, FALSE, ExecBufInstPtr);
	}
	#endif

	OP_STATE_RENDER(1, ExecBufInstPtr);
	STATE_DATA(D3DRENDERSTATE_TEXTUREMAPBLEND, D3DTBLEND_MODULATE, ExecBufInstPtr);

    OP_EXIT(ExecBufInstPtr);

    // Unlock buffer
    LastError = lpD3DExecCmdBuf->Unlock();
	LOGDXERR(LastError);

	if (LastError != D3D_OK)
	  return FALSE;

    // Setup for execution
    memset(&d3dExecData, 0, sizeof(D3DEXECUTEDATA));
    d3dExecData.dwSize = sizeof(D3DEXECUTEDATA);
	// Vertices start at beginning of buffer, i.e.
	// at ExecuteBufferDataArea, so there is no
	// Vertex Offset.
    d3dExecData.dwVertexCount = NumVertices;

    d3dExecData.dwInstructionOffset = (ULONG) ((char*) ExecuteBufferInstArea
                                    - (char*) ExecuteBufferDataArea);
    d3dExecData.dwInstructionLength = (ULONG) ((char*)ExecBufInstPtr 
                                    - (char*) ExecuteBufferInstArea);

    // Do the actual set up
    LastError = lpD3DExecCmdBuf->SetExecuteData(&d3dExecData);
	LOGDXERR(LastError);

	if (LastError != D3D_OK)
	  return FALSE;

	LastError = d3d.lpD3DDevice->BeginScene();
	LOGDXERR(LastError);

	if (LastError != D3D_OK)
	  return FALSE;

	// Execute buffer
	// We will specify no clipping since the engine
	// already clips to the VDB.  Eventually we might
	// try for no culling as well...if we're feeling
	// really daring...
	LastError = d3d.lpD3DDevice->Execute(lpD3DExecCmdBuf, 
	          d3d.lpD3DViewport, D3DEXECUTE_UNCLIPPED);
	LOGDXERR(LastError);

	if (LastError != D3D_OK)
	  return FALSE;

	// End scene
    LastError = d3d.lpD3DDevice->EndScene();
	LOGDXERR(LastError);

	if (LastError != D3D_OK)
 	  return FALSE;

    return TRUE;
}
static int NeedToClearExecuteBuffer=1;
BOOL LockExecuteBuffer(void)

{
    D3DEXECUTEBUFFERDESC d3dexDesc;

    memset(&d3dexDesc, 0, sizeof(D3DEXECUTEBUFFERDESC));
    d3dexDesc.dwSize = sizeof(D3DEXECUTEBUFFERDESC);

	// Acquire lock on execute buffer
	LastError = lpD3DExecCmdBuf->Lock(&d3dexDesc);
	LOGDXERR(LastError);

	if (LastError != D3D_OK)
	  return FALSE;

    // Zero buffer and acquire pointers to start of areas
//    memset(d3dexDesc.lpData, 0, ExBufSize);
  //	if(NeedToClearExecuteBuffer)
	{
	    memset(d3dexDesc.lpData, 0, ExBufSize);
  //		NeedToClearExecuteBuffer=0;
	}
	//textprint("ExBufSize %d %d %d %d\n",ExBufSize,sizeof(D3DINSTRUCTION),sizeof(D3DTRIANGLE),sizeof(D3DTLVERTEX) );
    // Two lines below should really be in init routine,
	// but are here for the moment in case locking has
	// weird side effects.
	// FIXME!!!!
    // Initialise pointers to data areas within buffer
    ExecuteBufferDataArea = d3dexDesc.lpData;
	// Hmmm....
	ExecuteBufferInstArea = (LPVOID) 
	   ((int) ExecuteBufferDataArea
	   + (int) (sizeof(D3DTLVERTEX) * MaxVerticesInExecuteBuffer));

	ExecBufDataPtr = ExecuteBufferDataArea;
	ExecBufInstPtr = ExecuteBufferInstArea;
	NumVertices = 0;

// Force initialisation of render states during
// first call to Write functions
	D3DShadingMode = 0xff;
	D3DTexturePerspective = 0xff;
	D3DAlphaMode = 0xff;

//  Other initialisations
	CurrTextureHandle = (D3DTEXTUREHANDLE) 0xff;


// Save pointer to execute buffer and
// wind that pointer on by the
// amount of a process vertices data opcode and
// associated data
    ExecBufStartInstPtr = ExecBufInstPtr;
    ExecBufInstPtr = (void*)(((LPD3DINSTRUCTION) ExecBufInstPtr) + 1);
    ExecBufInstPtr = (void*)(((LPD3DPROCESSVERTICES) ExecBufInstPtr) + 1);

	#if 0
    // Set default texture filter for this frame
	// NOTE THAT BILINEAR BEHAVIOUR MIGHT EVENTUALLY BE BEST
	// MOVED TO AN ITEM FLAG, NOT A COMPILE OPTION
	// Note at present no support for averaging between
	// mip-maps!!! May well not exist on all (or most) accelerators
	// anyway!!!
	if (BilinearTextureFilter)
	{
		if (ScreenDescriptorBlock.SDB_Flags & SDB_Flag_MIP)
		  {
		   DefaultD3DTextureFilterMin = D3DFILTER_MIPLINEAR;
		   DefaultD3DTextureFilterMax = D3DFILTER_MIPLINEAR;
		  }
		else
		  {
		   DefaultD3DTextureFilterMin = D3DFILTER_LINEAR;
		   DefaultD3DTextureFilterMax = D3DFILTER_LINEAR;
		  }
	}
	else
	{
		if (ScreenDescriptorBlock.SDB_Flags & SDB_Flag_MIP)
		  {
		   DefaultD3DTextureFilterMin = D3DFILTER_MIPNEAREST;
		   DefaultD3DTextureFilterMax = D3DFILTER_MIPNEAREST;
		  }
		else
		  {
		   DefaultD3DTextureFilterMin = D3DFILTER_NEAREST;
		   DefaultD3DTextureFilterMax = D3DFILTER_NEAREST;
		  }
	}
    // Actually set default texture filters

    OP_STATE_RENDER(2, ExecBufInstPtr);
    STATE_DATA(D3DRENDERSTATE_TEXTUREMAG, DefaultD3DTextureFilterMax, ExecBufInstPtr);
    STATE_DATA(D3DRENDERSTATE_TEXTUREMIN, DefaultD3DTextureFilterMin, ExecBufInstPtr);
	#endif
    return TRUE;
}


BOOL UnlockExecuteBufferAndPrepareForUse(void)

{
    D3DEXECUTEDATA d3dExecData;

    // Unlock buffer
    LastError = lpD3DExecCmdBuf->Unlock();
	LOGDXERR(LastError);

	if (LastError != D3D_OK)
	  return FALSE;

    // Setup for execution
    memset(&d3dExecData, 0, sizeof(D3DEXECUTEDATA));
    d3dExecData.dwSize = sizeof(D3DEXECUTEDATA);
	// Vertices start at beginning of buffer, i.e.
	// at ExecuteBufferDataArea, so there is no
	// Vertex Offset.
    d3dExecData.dwVertexCount = NumVertices;

    d3dExecData.dwInstructionOffset = (ULONG) ((char*) ExecuteBufferInstArea
                                    - (char*) ExecuteBufferDataArea);
    d3dExecData.dwInstructionLength = (ULONG) ((char*)ExecBufInstPtr 
                                    - (char*) ExecuteBufferInstArea);

    // Do the actual set up
    LastError = lpD3DExecCmdBuf->SetExecuteData(&d3dExecData);
	LOGDXERR(LastError);

	if (LastError != D3D_OK)
	  return FALSE;

    return TRUE;
}

// Test for multiple execute buffers!!!!
BOOL BeginD3DScene(void)

{
#if SOFTWARE_RENDERER
    LockSurfaceAndGetBufferPointer();

	return TRUE;
#else
	NumberOfRenderedTriangles=0;
	LastError = d3d.lpD3DDevice->BeginScene();
	LOGDXERR(LastError);

	if (LastError != D3D_OK)
	  return FALSE;
	else
	  return TRUE;
#endif
}
void D3D_SetupSceneDefaults(void)
{
	/* force translucency state to be reset */
	CurrentRenderStates.TranslucencyMode = TRANSLUCENCY_NOT_SET;
	CurrentRenderStates.FilteringMode = FILTERING_NOT_SET;
	CheckFilteringModeIsCorrect(FILTERING_BILINEAR_ON);
	CheckWireFrameMode(0);
	OP_STATE_RENDER(1, ExecBufInstPtr);
	STATE_DATA(D3DRENDERSTATE_DITHERENABLE, TRUE, ExecBufInstPtr);
}
BOOL EndD3DScene(void)

{
#if SOFTWARE_RENDERER
	UnlockSurface();	
	return TRUE;
#else
	extern int NormalFrameTime;

    LastError = d3d.lpD3DDevice->EndScene();
	LOGDXERR(LastError);
	if (ShowDebuggingText.PolyCount)
	{
		ReleasePrintDebuggingText("NumberOfLandscapePolygons: %d\n",NumberOfLandscapePolygons);
		ReleasePrintDebuggingText("NumberOfRenderedTriangles: %d\n",NumberOfRenderedTriangles);
	}
//	textprint ("NumberOfRenderedTrianglesPerSecond: %d\n",DIV_FIXED(NumberOfRenderedTriangles,NormalFrameTime));
	NumberOfLandscapePolygons=0;

	if (LastError != D3D_OK)
	  return FALSE;
	else
	  return TRUE;
#endif
	
}
void WriteEndCodeToExecuteBuffer(void)
{
	// Initialise execute buffer by specifying
	// that all operations will be rasterisation
	// only (this optimises the immediate mode interface
	// behaviour, since it knows that it will not
	// be asked to perform geometry transformations
	// or lighting operations).
	// Write process vertices instruction
	// to the START of the execute buffer,
	// now that we know how many vertices
	// there are.

	OP_PROCESS_VERTICES(1, ExecBufStartInstPtr);
	PROCESSVERTICES_DATA(D3DPROCESSVERTICES_COPY, 0, NumVertices, ExecBufStartInstPtr);

	// Write single termination opcode to buffer
	OP_EXIT(ExecBufInstPtr);
}

#include "pentime.h"
BOOL ExecuteBuffer(void)

{
#if SOFTWARE_RENDERER
	return TRUE;
#else

 //	textprint("%d\n",sizeof(D3DINSTRUCTION) + sizeof(D3DTRIANGLE));
//	textprint("NumVertices %d\n Instructions %d",NumVertices,(char*)ExecBufInstPtr-(char*)ExecBufStartInstPtr);
//ProfileStart();
	LastError = d3d.lpD3DDevice->Execute(lpD3DExecCmdBuf, 
	          d3d.lpD3DViewport, D3DEXECUTE_UNCLIPPED);
	LOGDXERR(LastError);
//ProfileStop("execute buffer");
	if (LastError != D3D_OK)
	  return FALSE;
    else
	  return TRUE;
#endif
}

void CheckRenderStatesForModule(MODULE *modulePtr)
{
//  	textprint("fog %d\n",CurrentRenderStates.FogIsOn);

	if (modulePtr->m_flags & MODULEFLAG_FOG)
	{
//		CurrentRenderStates.FogIsOn = 1;
	}
	else 
	{
//		CurrentRenderStates.FogIsOn = 0;
	}
}

void SetFogDistance(int fogDistance)
{
	if (fogDistance>10000) fogDistance = 10000;
	fogDistance+=5000;
	fogDistance=2000;
	CurrentRenderStates.FogDistance = fogDistance;
//	textprint("fog distance %d\n",fogDistance);

}

void ChangeTranslucencyMode(enum TRANSLUCENCY_TYPE translucencyRequired)
{
	
//	OP_STATE_RENDER(1, ExecBufInstPtr);
//	STATE_DATA(D3DRENDERSTATE_STIPPLEDALPHA,FALSE, ExecBufInstPtr);
//	OP_STATE_RENDER(1, ExecBufInstPtr);
//	STATE_DATA(D3DRENDERSTATE_STIPPLEENABLE,FALSE, ExecBufInstPtr);
//	if (CurrentRenderStates.TranslucencyMode!=(translucencyRequired))
	{
		CurrentRenderStates.TranslucencyMode=translucencyRequired;
		switch(CurrentRenderStates.TranslucencyMode)
		{
		 	case TRANSLUCENCY_OFF:
			{
				if (TRIPTASTIC_CHEATMODE||MOTIONBLUR_CHEATMODE)
				{
					if (D3DAlphaMode != Yes)
					{
						D3DAlphaMode = Yes;
						OP_STATE_RENDER(1, ExecBufInstPtr);
						STATE_DATA(D3DRENDERSTATE_ALPHABLENDENABLE,	TRUE, ExecBufInstPtr);
					}
					OP_STATE_RENDER(2, ExecBufInstPtr);
					STATE_DATA(D3DRENDERSTATE_SRCBLEND,D3DBLEND_INVSRCALPHA, ExecBufInstPtr);
					STATE_DATA(D3DRENDERSTATE_DESTBLEND,D3DBLEND_SRCALPHA, ExecBufInstPtr);
				}
				else
				{
					if (D3DAlphaMode != No)
					{
						D3DAlphaMode = No;
						OP_STATE_RENDER(1, ExecBufInstPtr);
						STATE_DATA(D3DRENDERSTATE_ALPHABLENDENABLE,	FALSE, ExecBufInstPtr);
					}
					OP_STATE_RENDER(2, ExecBufInstPtr);
					STATE_DATA(D3DRENDERSTATE_SRCBLEND,D3DBLEND_ONE, ExecBufInstPtr);
					STATE_DATA(D3DRENDERSTATE_DESTBLEND,D3DBLEND_ZERO, ExecBufInstPtr);
				}
 				
				break;
			}
		 	case TRANSLUCENCY_NORMAL:
			{
				if (D3DAlphaMode != Yes)
				{
					D3DAlphaMode = Yes;
					OP_STATE_RENDER(1, ExecBufInstPtr);
					STATE_DATA(D3DRENDERSTATE_ALPHABLENDENABLE,	TRUE, ExecBufInstPtr);
				}
				OP_STATE_RENDER(2, ExecBufInstPtr);
				STATE_DATA(D3DRENDERSTATE_SRCBLEND,D3DBLEND_SRCALPHA, ExecBufInstPtr);
				STATE_DATA(D3DRENDERSTATE_DESTBLEND,D3DBLEND_INVSRCALPHA, ExecBufInstPtr);
				
				
				
				break;
			}
		 	case TRANSLUCENCY_COLOUR:
			{
				if (D3DAlphaMode != Yes)
				{
					D3DAlphaMode = Yes;
					OP_STATE_RENDER(1, ExecBufInstPtr);
					STATE_DATA(D3DRENDERSTATE_ALPHABLENDENABLE,	TRUE, ExecBufInstPtr);
				}
				OP_STATE_RENDER(2, ExecBufInstPtr);
				STATE_DATA(D3DRENDERSTATE_SRCBLEND,D3DBLEND_ZERO, ExecBufInstPtr);
				STATE_DATA(D3DRENDERSTATE_DESTBLEND,D3DBLEND_SRCCOLOR, ExecBufInstPtr);
				break;
			}
		 	case TRANSLUCENCY_INVCOLOUR:
			{
				if (D3DAlphaMode != Yes)
				{
					D3DAlphaMode = Yes;
					OP_STATE_RENDER(1, ExecBufInstPtr);
					STATE_DATA(D3DRENDERSTATE_ALPHABLENDENABLE,	TRUE, ExecBufInstPtr);
				}							   
				OP_STATE_RENDER(2, ExecBufInstPtr);
				STATE_DATA(D3DRENDERSTATE_SRCBLEND,D3DBLEND_ZERO, ExecBufInstPtr);
				STATE_DATA(D3DRENDERSTATE_DESTBLEND,D3DBLEND_INVSRCCOLOR, ExecBufInstPtr);
				break;
			}
	  		case TRANSLUCENCY_GLOWING:
			{
				if (D3DAlphaMode != Yes)
				{
					D3DAlphaMode = Yes;
					OP_STATE_RENDER(1, ExecBufInstPtr);
					STATE_DATA(D3DRENDERSTATE_ALPHABLENDENABLE,	TRUE, ExecBufInstPtr);
				}
				OP_STATE_RENDER(2, ExecBufInstPtr);
				STATE_DATA(D3DRENDERSTATE_SRCBLEND,D3DBLEND_SRCALPHA, ExecBufInstPtr);
				STATE_DATA(D3DRENDERSTATE_DESTBLEND,D3DBLEND_ONE, ExecBufInstPtr);
				break;
			}
	  		case TRANSLUCENCY_DARKENINGCOLOUR:
			{
				if (D3DAlphaMode != Yes)
				{
					D3DAlphaMode = Yes;
					OP_STATE_RENDER(1, ExecBufInstPtr);
					STATE_DATA(D3DRENDERSTATE_ALPHABLENDENABLE,	TRUE, ExecBufInstPtr);
				}
				OP_STATE_RENDER(2, ExecBufInstPtr);
				STATE_DATA(D3DRENDERSTATE_SRCBLEND,D3DBLEND_INVDESTCOLOR, ExecBufInstPtr);
				STATE_DATA(D3DRENDERSTATE_DESTBLEND,D3DBLEND_ZERO, ExecBufInstPtr);
				break;
			}
			case TRANSLUCENCY_JUSTSETZ:
			{
				if (D3DAlphaMode != Yes)
				{
					D3DAlphaMode = Yes;
					OP_STATE_RENDER(1, ExecBufInstPtr);
					STATE_DATA(D3DRENDERSTATE_ALPHABLENDENABLE,	TRUE, ExecBufInstPtr);
				}
				OP_STATE_RENDER(2, ExecBufInstPtr);
				STATE_DATA(D3DRENDERSTATE_SRCBLEND,D3DBLEND_ZERO, ExecBufInstPtr);
				STATE_DATA(D3DRENDERSTATE_DESTBLEND,D3DBLEND_ONE, ExecBufInstPtr);
			}

			default:
				break;
		}
	}
}

void ChangeFilteringMode(enum FILTERING_MODE_ID filteringRequired)
{
	CurrentRenderStates.FilteringMode = filteringRequired;

	switch(CurrentRenderStates.FilteringMode)
	{
		case FILTERING_BILINEAR_OFF:
		{
		    OP_STATE_RENDER(2, ExecBufInstPtr);
		    STATE_DATA(D3DRENDERSTATE_TEXTUREMAG, D3DFILTER_NEAREST, ExecBufInstPtr);
		    STATE_DATA(D3DRENDERSTATE_TEXTUREMIN, D3DFILTER_NEAREST, ExecBufInstPtr);
			break;
		}
		case FILTERING_BILINEAR_ON:
		{
		    OP_STATE_RENDER(2, ExecBufInstPtr);
		    STATE_DATA(D3DRENDERSTATE_TEXTUREMAG, D3DFILTER_LINEAR, ExecBufInstPtr);
		    STATE_DATA(D3DRENDERSTATE_TEXTUREMIN, D3DFILTER_LINEAR, ExecBufInstPtr);
			break;
		}
		default:
		{
			LOCALASSERT("Unrecognized filtering mode"==0);
			break;
		}
	}	
}

extern void CheckWireFrameMode(int shouldBeOn)
{
	if (shouldBeOn) shouldBeOn=1;
	if(CurrentRenderStates.WireFrameModeIsOn!=shouldBeOn)
	{
		CurrentRenderStates.WireFrameModeIsOn=shouldBeOn;
	   if (shouldBeOn)
	   {
			OP_STATE_RENDER(1, ExecBufInstPtr);
			STATE_DATA(D3DRENDERSTATE_FILLMODE,	D3DFILL_WIREFRAME, ExecBufInstPtr);
	   }
	   else
	   {
			OP_STATE_RENDER(1, ExecBufInstPtr);
			STATE_DATA(D3DRENDERSTATE_FILLMODE,	D3DFILL_SOLID, ExecBufInstPtr);
	   }

	}
	
}
void D3D_Line(VECTOR2D* LineStart, VECTOR2D* LineEnd, int LineColour)
{
	if (d3d.ThisDriver.dpcLineCaps.dwShadeCaps & D3DPSHADECAPS_COLORFLATRGB)
	{
		int x1, x2, y1, y2;

		D3DTLVERTEX *vertexPtr = &((LPD3DTLVERTEX)ExecuteBufferDataArea)[NumVertices];

	    // Test!!!
		// LineColour = 0xffffffff;
		vertexPtr->sx = LineStart->vx;
		vertexPtr->sy = LineStart->vy;
		vertexPtr[1].sx = LineEnd->vx;
		vertexPtr[1].sy = LineEnd->vy;

		NumVertices+=2;

    	vertexPtr->color = LineColour;
		vertexPtr[1].color = LineColour;

	    // Always turn off translucency for the moment
		CheckTranslucencyModeIsCorrect(TRANSLUCENCY_OFF);

	    // Turn OFF texturing if it is on...
	    if (CurrTextureHandle != NULL)
		  {
	       OP_STATE_RENDER(1, ExecBufInstPtr);
	          STATE_DATA(D3DRENDERSTATE_TEXTUREHANDLE, NULL, ExecBufInstPtr);
		   CurrTextureHandle = NULL;
		  }

		OP_LINE_LIST(1, ExecBufInstPtr);
	        ((LPD3DLINE)ExecBufInstPtr)->v1 = (NumVertices-2); // vertex index
	        ((LPD3DLINE)ExecBufInstPtr)->v2 = (NumVertices-1); // vertex index
	        ExecBufInstPtr = ((char*)ExecBufInstPtr) + sizeof(D3DLINE);
	}

	if (NumVertices > (MaxVerticesInExecuteBuffer-12)) 
	{
	   WriteEndCodeToExecuteBuffer();
  	   UnlockExecuteBufferAndPrepareForUse();
	   ExecuteBuffer();
  	   LockExecuteBuffer();
	}
}

static void D3D_OutputTriangles(void); 
void D3D_BackdropPolygon_Output(POLYHEADER *inputPolyPtr,RENDERVERTEX *renderVerticesPtr)
{
	int flags;
	int texoffset;

	D3DTEXTUREHANDLE TextureHandle;

	float ZNear;
	float RecipW, RecipH;

    // Get ZNear
	ZNear = (float) (Global_VDB_Ptr->VDB_ClipZ * GlobalScale);


	// Take header information
	flags = inputPolyPtr->PolyFlags;

	// We assume bit 15 (TxLocal) HAS been
	// properly cleared this time...
	texoffset = (inputPolyPtr->PolyColour & ClrTxDefn);

	TextureHandle = (D3DTEXTUREHANDLE)
	          ImageHeaderArray[texoffset].D3DHandle;

    // Check for textures that have not loaded
	// properly

    if (TextureHandle == (D3DTEXTUREHANDLE) 0)
	  return;
	
	if(ImageHeaderArray[texoffset].ImageWidth==128)
	{
		RecipW = 1.0 /128.0;
	}
	else
	{
		float width = (float) ImageHeaderArray[texoffset].ImageWidth;
		RecipW = (1.0 / width);
	}
	if(ImageHeaderArray[texoffset].ImageHeight==128)
	{
		RecipH = 1.0 / 128.0;
	}
	else
	{
		float height = (float) ImageHeaderArray[texoffset].ImageHeight;
		RecipH = (1.0 / height);
	}


	/* OUTPUT VERTICES TO EXECUTE BUFFER */
	{
		int i = RenderPolygon.NumberOfVertices;
		RENDERVERTEX *vertices = renderVerticesPtr;

		do
		{
			D3DTLVERTEX *vertexPtr = &((LPD3DTLVERTEX)ExecuteBufferDataArea)[NumVertices];
		  	float oneOverZ;
		  	oneOverZ = (1.0)/vertices->Z;
			float zvalue;

			{
				int x = (vertices->X*(Global_VDB_Ptr->VDB_ProjX+1))/vertices->Z+Global_VDB_Ptr->VDB_CentreX;

				if (x<Global_VDB_Ptr->VDB_ClipLeft)
				{
					x=Global_VDB_Ptr->VDB_ClipLeft;
				}	
				else if (x>Global_VDB_Ptr->VDB_ClipRight)
				{
					x=Global_VDB_Ptr->VDB_ClipRight;	
				}
				
				vertexPtr->sx=x;
			}
			{
				int y = (vertices->Y*(Global_VDB_Ptr->VDB_ProjY+1))/vertices->Z+Global_VDB_Ptr->VDB_CentreY;
				
				if (y<Global_VDB_Ptr->VDB_ClipUp)
				{
					y=Global_VDB_Ptr->VDB_ClipUp;
				}
				else if (y>Global_VDB_Ptr->VDB_ClipDown)
				{
					y=Global_VDB_Ptr->VDB_ClipDown;	
				}
				vertexPtr->sy=y;
				
			}
			vertexPtr->tu = ((float)(vertices->U>>16)+.5) * RecipW;
			vertexPtr->tv = ((float)(vertices->V>>16)+.5) * RecipH;
			vertexPtr->rhw = oneOverZ;

			vertexPtr->color = RGBLIGHT_MAKE(vertices->R,vertices->G,vertices->B);
			vertexPtr->sz = 1.0;

			vertexPtr->specular=RGBALIGHT_MAKE(0,0,0,255);
		
			vertices++;
			NumVertices++;
		}
	  	while(--i);
	}

 	CheckTranslucencyModeIsCorrect(TRANSLUCENCY_OFF);

	// Insert state change for shading model if required
    if (D3DShadingMode != D3DSHADE_GOURAUD)
	  {
   	   D3DShadingMode = D3DSHADE_GOURAUD;
	   OP_STATE_RENDER(1, ExecBufInstPtr);
	     STATE_DATA(D3DRENDERSTATE_SHADEMODE,
		       D3DSHADE_GOURAUD, ExecBufInstPtr);
	  }

// Insert state change for texturing perspective value
// Note that drawtx3das2d options have ONLY been allowed for here,
// not when the rhw values are generated.  This is a deliberate choice,
// based on the assumption that drawtx3das2d will not be used very often
// and the extra branching at the top of this function will impose a 
// greater cost than the (rare) savings in floating pt divisions are worth.
// Or so I claim...

    if (D3DTexturePerspective != Yes)
    {
		D3DTexturePerspective = Yes;
		OP_STATE_RENDER(1, ExecBufInstPtr);
		STATE_DATA(D3DRENDERSTATE_TEXTUREPERSPECTIVE, TRUE, ExecBufInstPtr);
	}

    if (TextureHandle != CurrTextureHandle)
	{
    	OP_STATE_RENDER(1, ExecBufInstPtr);
        STATE_DATA(D3DRENDERSTATE_TEXTUREHANDLE, TextureHandle, ExecBufInstPtr);
	   	CurrTextureHandle = TextureHandle;
	}


	D3D_OutputTriangles();
}

void D3D_ZBufferedGouraudTexturedPolygon_Output(POLYHEADER *inputPolyPtr,RENDERVERTEX *renderVerticesPtr)
{
	int flags;
	int texoffset;

	D3DTEXTUREHANDLE TextureHandle;

	float ZNear;
	float RecipW, RecipH;

    // Get ZNear
	ZNear = (float) (Global_VDB_Ptr->VDB_ClipZ * GlobalScale);


	// Take header information
	flags = inputPolyPtr->PolyFlags;

	// We assume bit 15 (TxLocal) HAS been
	// properly cleared this time...
	texoffset = (inputPolyPtr->PolyColour & ClrTxDefn);
	if (texoffset)
	{
		TextureHandle = (D3DTEXTUREHANDLE)ImageHeaderArray[texoffset].D3DHandle;
	}
	else
	{	
		TextureHandle = CurrTextureHandle;
	}

    // Check for textures that have not loaded
	// properly

//    if (TextureHandle == (D3DTEXTUREHANDLE) 0)
 //	  return;

	#ifdef AVP_DEBUG_VERSION	
	if(ImageHeaderArray[texoffset].ImageWidth==128)
	{
		RecipW = (1.0f /128.0f)/65536.0f;
	}
	else
	{
		float width = (float) ImageHeaderArray[texoffset].ImageWidth;
		RecipW = (1.0f / width)/65536.0f;
	}
	if(ImageHeaderArray[texoffset].ImageHeight==128)
	{
		RecipH = (1.0f / 128.0f)/65536.0f;
	}
	else
	{
		float height = (float) ImageHeaderArray[texoffset].ImageHeight;
		RecipH = (1.0f / height)/65536.0f;
	}
	#else
	RecipW = (1.0f/65536.0f)/128.0f;
	RecipH = (1.0f/65536.0f)/128.0f;
	#endif
//	RecipW = (1.0/65536.0)/(float) ImageHeaderArray[texoffset].ImageWidth;
//	RecipH = (1.0/65536.0)/(float) ImageHeaderArray[texoffset].ImageHeight;

	/* OUTPUT VERTICES TO EXECUTE BUFFER */
	{
		int i = RenderPolygon.NumberOfVertices;
		RENDERVERTEX *vertices = renderVerticesPtr;

		do
		{
			D3DTLVERTEX *vertexPtr = &((LPD3DTLVERTEX)ExecuteBufferDataArea)[NumVertices];
		  	float oneOverZ;
		  	oneOverZ = (1.0f)/(vertices->Z);
			float zvalue;

			vertexPtr->tu = ((float)vertices->U) * RecipW + (1.0f/256.0f);
			vertexPtr->tv = ((float)vertices->V) * RecipH + (1.0f/256.0f);
			vertexPtr->rhw = oneOverZ;

			{
				zvalue = vertices->Z+HeadUpDisplayZOffset;
	   //			zvalue /= 65536.0;
	   		   	zvalue = 1.0f - ZNear/zvalue;
			}	
			
			{
				int x = (vertices->X*(Global_VDB_Ptr->VDB_ProjX+1))/vertices->Z+Global_VDB_Ptr->VDB_CentreX;

				#if 1
				if (x<Global_VDB_Ptr->VDB_ClipLeft)
				{
					x=Global_VDB_Ptr->VDB_ClipLeft;
				}	
				else if (x>Global_VDB_Ptr->VDB_ClipRight)
				{
					x=Global_VDB_Ptr->VDB_ClipRight;	
				}
				#endif
				vertexPtr->sx=x;
			}
			{
				int y = (vertices->Y*(Global_VDB_Ptr->VDB_ProjY+1))/vertices->Z+Global_VDB_Ptr->VDB_CentreY;
				
				#if 1
				if (y<Global_VDB_Ptr->VDB_ClipUp)
				{
					y=Global_VDB_Ptr->VDB_ClipUp;
				}
				else if (y>Global_VDB_Ptr->VDB_ClipDown)
				{
					y=Global_VDB_Ptr->VDB_ClipDown;	
				}
				#endif
				vertexPtr->sy=y;
				
			}
//			vertexPtr->tu = ((float)(vertices->U>>16)+0.5) * RecipW;
//			vertexPtr->tv = ((float)(vertices->V>>16)+0.5) * RecipH;
//			vertexPtr->rhw = oneOverZ;

			{
				extern unsigned char GammaValues[256];
		  		vertexPtr->color = RGBALIGHT_MAKE(GammaValues[vertices->R],GammaValues[vertices->G],GammaValues[vertices->B],vertices->A);
			}

			vertexPtr->sz = zvalue;
			#if 0//FOG_ON
			{
				extern unsigned char GammaValues[256];
				int r = GammaValues[vertices->SpecularR];
				int g = GammaValues[vertices->SpecularG];
				int b = GammaValues[vertices->SpecularB];

				if(CurrentRenderStates.FogIsOn && vertices->Z>5000)//CurrentRenderStates.FogDistance)
				{
//					extern int CloakingPhase;
					int brightness = (vertices->R+vertices->G+vertices->B)*86*3;

					int fog = ((vertices->Z-5000/*-CurrentRenderStates.FogDistance*/)*4);
					if (fog<0) fog=0;
				 	if (fog>brightness) fog = brightness;
					if (fog>ONE_FIXED) fog=ONE_FIXED;
					
					{
						extern int SkyColour_R;
						int rfog = MUL_FIXED(fog,SkyColour_R);
				 		if (r<rfog) r=rfog;					
					}
					{
						extern int SkyColour_G;
						int gfog = MUL_FIXED(fog,SkyColour_G);
				 		if (g<gfog) g=gfog;					
					}
					{
						extern int SkyColour_B;
						int bfog = MUL_FIXED(fog,SkyColour_B);
				 		if (b<bfog) b=bfog;					
					}
				}
			   	vertexPtr->specular=RGBALIGHT_MAKE(r,g,b,255);
			}
			#else
			#if 0	
		   	if (RenderPolygon.TranslucencyMode != TRANSLUCENCY_OFF)
		    { 
				vertexPtr->specular=0;
			}
			else
			#endif
			{
				extern unsigned char GammaValues[256];
				vertexPtr->specular=RGBALIGHT_MAKE(GammaValues[vertices->SpecularR],GammaValues[vertices->SpecularG],GammaValues[vertices->SpecularB],255);
			}
			#endif
		
			vertices++;
			NumVertices++;
		}
	  	while(--i);
	}
	CheckTranslucencyModeIsCorrect(RenderPolygon.TranslucencyMode);

	// Insert state change for shading model if required
    if (D3DShadingMode != D3DSHADE_GOURAUD)
	  {
   	   D3DShadingMode = D3DSHADE_GOURAUD;
	   OP_STATE_RENDER(1, ExecBufInstPtr);
	     STATE_DATA(D3DRENDERSTATE_SHADEMODE,
		       D3DSHADE_GOURAUD, ExecBufInstPtr);
	  }

// Insert state change for texturing perspective value
// Note that drawtx3das2d options have ONLY been allowed for here,
// not when the rhw values are generated.  This is a deliberate choice,
// based on the assumption that drawtx3das2d will not be used very often
// and the extra branching at the top of this function will impose a 
// greater cost than the (rare) savings in floating pt divisions are worth.
// Or so I claim...

    if (D3DTexturePerspective != Yes)
    {
		D3DTexturePerspective = Yes;
		OP_STATE_RENDER(1, ExecBufInstPtr);
		STATE_DATA(D3DRENDERSTATE_TEXTUREPERSPECTIVE, TRUE, ExecBufInstPtr);
	}

	#if FMV_EVERYWHERE
	TextureHandle = FMVTextureHandle[0];
	#endif
    if (TextureHandle != CurrTextureHandle)
	{
    	OP_STATE_RENDER(1, ExecBufInstPtr);
        STATE_DATA(D3DRENDERSTATE_TEXTUREHANDLE, TextureHandle, ExecBufInstPtr);
	   	CurrTextureHandle = TextureHandle;
	}


	D3D_OutputTriangles();
}
void D3D_ZBufferedGouraudPolygon_Output(POLYHEADER *inputPolyPtr,RENDERVERTEX *renderVerticesPtr)
{													
	int flags;

	float ZNear;

    // Get ZNear
	ZNear = (float) (Global_VDB_Ptr->VDB_ClipZ * GlobalScale);

	// Take header information
	flags = inputPolyPtr->PolyFlags;


	/* OUTPUT VERTICES TO EXECUTE BUFFER */
	{
		int i = RenderPolygon.NumberOfVertices;
		RENDERVERTEX *vertices = renderVerticesPtr;

		do
		{
			D3DTLVERTEX *vertexPtr = &((LPD3DTLVERTEX)ExecuteBufferDataArea)[NumVertices];
		  	float oneOverZ;
		  	oneOverZ = (1.0)/vertices->Z;
			float zvalue;

			{
				int x = (vertices->X*(Global_VDB_Ptr->VDB_ProjX+1))/vertices->Z+Global_VDB_Ptr->VDB_CentreX;
				if (x<Global_VDB_Ptr->VDB_ClipLeft)
				{
					x=Global_VDB_Ptr->VDB_ClipLeft;
				}	
				else if (x>Global_VDB_Ptr->VDB_ClipRight)
				{
					x=Global_VDB_Ptr->VDB_ClipRight;	
				}
				
				vertexPtr->sx=x;
			}
			{
				int y = (vertices->Y*(Global_VDB_Ptr->VDB_ProjY+1))/vertices->Z+Global_VDB_Ptr->VDB_CentreY;
				if (y<Global_VDB_Ptr->VDB_ClipUp)
				{
					y=Global_VDB_Ptr->VDB_ClipUp;
				}
				else if (y>Global_VDB_Ptr->VDB_ClipDown)
				{
					y=Global_VDB_Ptr->VDB_ClipDown;	
				}
				vertexPtr->sy=y;
			}
			{
				zvalue = vertices->Z+HeadUpDisplayZOffset;
				zvalue = ((zvalue-ZNear)/zvalue);
			}	
	
			{
				if (flags & iflag_transparent)
				{
			  		vertexPtr->color = RGBALIGHT_MAKE(vertices->R,vertices->G,vertices->B, vertices->A);
				}
				else
				{
					vertexPtr->color = RGBLIGHT_MAKE(vertices->R,vertices->G,vertices->B);
				}
			}
			vertexPtr->sz = zvalue;
			vertices++;
			NumVertices++;
		}
	  	while(--i);
	}
	CheckTranslucencyModeIsCorrect(RenderPolygon.TranslucencyMode);

	// Insert state change for shading model if required
    if (D3DShadingMode != D3DSHADE_GOURAUD)
	  {
   	   D3DShadingMode = D3DSHADE_GOURAUD;
	   OP_STATE_RENDER(1, ExecBufInstPtr);
	     STATE_DATA(D3DRENDERSTATE_SHADEMODE,
		       D3DSHADE_GOURAUD, ExecBufInstPtr);
	  }

    // Turn OFF texturing if it is on...
    if (CurrTextureHandle != NULL)
	  {
       OP_STATE_RENDER(1, ExecBufInstPtr);
          STATE_DATA(D3DRENDERSTATE_TEXTUREHANDLE, NULL, ExecBufInstPtr);
	   CurrTextureHandle = NULL;
	  }

	D3D_OutputTriangles();
}
void D3D_PredatorThermalVisionPolygon_Output(POLYHEADER *inputPolyPtr,RENDERVERTEX *renderVerticesPtr)
{													
	float ZNear;

    // Get ZNear
	ZNear = (float) (Global_VDB_Ptr->VDB_ClipZ * GlobalScale);

	/* OUTPUT VERTICES TO EXECUTE BUFFER */
	{
		int i = RenderPolygon.NumberOfVertices;
		RENDERVERTEX *vertices = renderVerticesPtr;

		do
		{
			D3DTLVERTEX *vertexPtr = &((LPD3DTLVERTEX)ExecuteBufferDataArea)[NumVertices];
		  	float oneOverZ;
		  	oneOverZ = (1.0)/vertices->Z;
			float zvalue;

			{
				int x = (vertices->X*(Global_VDB_Ptr->VDB_ProjX+1))/vertices->Z+Global_VDB_Ptr->VDB_CentreX;
				if (x<Global_VDB_Ptr->VDB_ClipLeft)
				{
					x=Global_VDB_Ptr->VDB_ClipLeft;
				}	
				else if (x>Global_VDB_Ptr->VDB_ClipRight)
				{
					x=Global_VDB_Ptr->VDB_ClipRight;	
				}
				
				vertexPtr->sx=x;
			}
			{
				int y = (vertices->Y*(Global_VDB_Ptr->VDB_ProjY+1))/vertices->Z+Global_VDB_Ptr->VDB_CentreY;
				if (y<Global_VDB_Ptr->VDB_ClipUp)
				{
					y=Global_VDB_Ptr->VDB_ClipUp;
				}
				else if (y>Global_VDB_Ptr->VDB_ClipDown)
				{
					y=Global_VDB_Ptr->VDB_ClipDown;	
				}
				vertexPtr->sy=y;
			}
			{
				zvalue = vertices->Z+HeadUpDisplayZOffset;
				zvalue = ((zvalue-ZNear)/zvalue);
			}	
	
			vertexPtr->color = RGBALIGHT_MAKE(vertices->R,vertices->G,vertices->B,vertices->A);//RGBALIGHT_MAKE(255,255,255,255);
			vertexPtr->sz = zvalue;
			vertexPtr->rhw = zvalue;
			vertices++;
			NumVertices++;
		}
	  	while(--i);
	}
	CheckTranslucencyModeIsCorrect(TRANSLUCENCY_OFF);

	// Insert state change for shading model if required
    if (D3DShadingMode != D3DSHADE_GOURAUD)
	  {
   	   D3DShadingMode = D3DSHADE_GOURAUD;
	   OP_STATE_RENDER(1, ExecBufInstPtr);
	     STATE_DATA(D3DRENDERSTATE_SHADEMODE,
		       D3DSHADE_GOURAUD, ExecBufInstPtr);
	  }

    // Turn OFF texturing if it is on...
    if (CurrTextureHandle != NULL)
	{
       OP_STATE_RENDER(1, ExecBufInstPtr);
       STATE_DATA(D3DRENDERSTATE_TEXTUREHANDLE, NULL, ExecBufInstPtr);
	   CurrTextureHandle = NULL;
	}
	D3D_OutputTriangles();
}

void D3D_PredatorSeeAliensVisionPolygon_Output(POLYHEADER *inputPolyPtr,RENDERVERTEX *renderVerticesPtr)
{													
	float ZNear;

    // Get ZNear
	ZNear = (float) (Global_VDB_Ptr->VDB_ClipZ * GlobalScale);

	/* OUTPUT VERTICES TO EXECUTE BUFFER */
	{
		int i = RenderPolygon.NumberOfVertices;
		RENDERVERTEX *vertices = renderVerticesPtr;

		do
		{
			D3DTLVERTEX *vertexPtr = &((LPD3DTLVERTEX)ExecuteBufferDataArea)[NumVertices];
		  	float oneOverZ;
		  	oneOverZ = (1.0)/vertices->Z;
			float zvalue;

			{
				int x = (vertices->X*(Global_VDB_Ptr->VDB_ProjX+1))/vertices->Z+Global_VDB_Ptr->VDB_CentreX;
				if (x<Global_VDB_Ptr->VDB_ClipLeft)
				{
					x=Global_VDB_Ptr->VDB_ClipLeft;
				}	
				else if (x>Global_VDB_Ptr->VDB_ClipRight)
				{
					x=Global_VDB_Ptr->VDB_ClipRight;	
				}
				
				vertexPtr->sx=x;
			}
			{
				int y = (vertices->Y*(Global_VDB_Ptr->VDB_ProjY+1))/vertices->Z+Global_VDB_Ptr->VDB_CentreY;
				if (y<Global_VDB_Ptr->VDB_ClipUp)
				{
					y=Global_VDB_Ptr->VDB_ClipUp;
				}
				else if (y>Global_VDB_Ptr->VDB_ClipDown)
				{
					y=Global_VDB_Ptr->VDB_ClipDown;	
				}
				vertexPtr->sy=y;
			}
			{
				zvalue = vertices->Z+HeadUpDisplayZOffset;
				zvalue = ((zvalue-ZNear)/zvalue);
			}	
	
			vertexPtr->color = RGBALIGHT_MAKE(255,0,0,255);//255,255,255,255);
			vertexPtr->sz = zvalue;
			vertexPtr->rhw = zvalue;
			vertices++;
			NumVertices++;
		}
	  	while(--i);
	}
	CheckTranslucencyModeIsCorrect(TRANSLUCENCY_GLOWING)//JUSTSETZ);


    // Turn OFF texturing if it is on...
    if (CurrTextureHandle != NULL)
	{
       OP_STATE_RENDER(1, ExecBufInstPtr);
       STATE_DATA(D3DRENDERSTATE_TEXTUREHANDLE, NULL, ExecBufInstPtr);
	   CurrTextureHandle = NULL;
	}

	D3D_OutputTriangles();
}
void D3D_ZBufferedCloakedPolygon_Output(POLYHEADER *inputPolyPtr,RENDERVERTEX *renderVerticesPtr)
{
	extern int CloakingMode;
	extern char CloakedPredatorIsMoving;
	int uOffset = FastRandom()&255;
	int vOffset = FastRandom()&255;

	int flags;
	int texoffset;

	D3DTEXTUREHANDLE TextureHandle;

	float ZNear;
	float RecipW, RecipH;

    // Get ZNear
	ZNear = (float) (Global_VDB_Ptr->VDB_ClipZ * GlobalScale);


	// Take header information
	flags = inputPolyPtr->PolyFlags;

	// We assume bit 15 (TxLocal) HAS been
	// properly cleared this time...
	texoffset = (inputPolyPtr->PolyColour & ClrTxDefn);

	TextureHandle = (D3DTEXTUREHANDLE)
	          ImageHeaderArray[texoffset].D3DHandle;
    // Check for textures that have not loaded
	// properly

    if (TextureHandle == (D3DTEXTUREHANDLE) 0)
	  return;


	if(ImageHeaderArray[texoffset].ImageWidth==128)
	{
		RecipW = 1.0 /128.0;
	}
	else
	{
		float width = (float) ImageHeaderArray[texoffset].ImageWidth;
		RecipW = (1.0 / width);
	}
	if(ImageHeaderArray[texoffset].ImageHeight==128)
	{
		RecipH = 1.0 / 128.0;
	}
	else
	{
		float height = (float) ImageHeaderArray[texoffset].ImageHeight;
		RecipH = (1.0 / height);
	}


	/* OUTPUT VERTICES TO EXECUTE BUFFER */
	{
		int i = RenderPolygon.NumberOfVertices;
		RENDERVERTEX *vertices = renderVerticesPtr;

		do
		{
			D3DTLVERTEX *vertexPtr = &((LPD3DTLVERTEX)ExecuteBufferDataArea)[NumVertices];
		  	float oneOverZ;
		  	oneOverZ = (1.0)/vertices->Z;
			float zvalue;

			{
				int x = (vertices->X*(Global_VDB_Ptr->VDB_ProjX+1))/vertices->Z+Global_VDB_Ptr->VDB_CentreX;
				if (x<Global_VDB_Ptr->VDB_ClipLeft)
				{
					x=Global_VDB_Ptr->VDB_ClipLeft;
				}	
				else if (x>Global_VDB_Ptr->VDB_ClipRight)
				{
					x=Global_VDB_Ptr->VDB_ClipRight;	
				}
				
				vertexPtr->sx=x;
			}
			{
				int y = (vertices->Y*(Global_VDB_Ptr->VDB_ProjY+1))/vertices->Z+Global_VDB_Ptr->VDB_CentreY;
				if (y<Global_VDB_Ptr->VDB_ClipUp)
				{
					y=Global_VDB_Ptr->VDB_ClipUp;
				}
				else if (y>Global_VDB_Ptr->VDB_ClipDown)
				{
					y=Global_VDB_Ptr->VDB_ClipDown;	
				}
				vertexPtr->sy=y;
			}

			vertexPtr->tu = ((float)(vertices->U>>16)+0.5) * RecipW;
			vertexPtr->tv = ((float)(vertices->V>>16)+0.5) * RecipH;
	   		
	   		if (CloakedPredatorIsMoving)
			{
		   		vertexPtr->color = RGBALIGHT_MAKE(vertices->R,vertices->G,vertices->B,vertices->A);
			}
			else
			{
		   		vertexPtr->color = RGBALIGHT_MAKE(vertices->R,vertices->G,vertices->B,vertices->A);
			}
			CheckTranslucencyModeIsCorrect(TRANSLUCENCY_NORMAL);

			vertexPtr->rhw = oneOverZ;

			{
				zvalue = vertices->Z+HeadUpDisplayZOffset;
				zvalue = ((zvalue-ZNear)/zvalue);
			}	
	
			vertexPtr->sz = zvalue;

//		   	vertexPtr->specular=RGBALIGHT_MAKE(vertices->SpecularR,vertices->SpecularG,vertices->SpecularB,255);
			vertexPtr->specular=RGBALIGHT_MAKE(0,0,0,255);

			vertices++;
			NumVertices++;
		}
	  	while(--i);
	}

	// Insert state change for shading model if required
    if (D3DShadingMode != D3DSHADE_GOURAUD)
	  {
   	   D3DShadingMode = D3DSHADE_GOURAUD;
	   OP_STATE_RENDER(1, ExecBufInstPtr);
	     STATE_DATA(D3DRENDERSTATE_SHADEMODE,
		       D3DSHADE_GOURAUD, ExecBufInstPtr);
	  }

// Insert state change for texturing perspective value
// Note that drawtx3das2d options have ONLY been allowed for here,
// not when the rhw values are generated.  This is a deliberate choice,
// based on the assumption that drawtx3das2d will not be used very often
// and the extra branching at the top of this function will impose a 
// greater cost than the (rare) savings in floating pt divisions are worth.
// Or so I claim...

    if (D3DTexturePerspective != Yes)
    {
		D3DTexturePerspective = Yes;
		OP_STATE_RENDER(1, ExecBufInstPtr);
		STATE_DATA(D3DRENDERSTATE_TEXTUREPERSPECTIVE, TRUE, ExecBufInstPtr);
	}
    if (TextureHandle != CurrTextureHandle)
	{
       OP_STATE_RENDER(1, ExecBufInstPtr);
       STATE_DATA(D3DRENDERSTATE_TEXTUREHANDLE, TextureHandle, ExecBufInstPtr);
	   CurrTextureHandle = TextureHandle;
	}
    /*
    if (TextureHandle != NULL)
	{
    	OP_STATE_RENDER(1, ExecBufInstPtr);
    	STATE_DATA(D3DRENDERSTATE_TEXTUREHANDLE, NULL, ExecBufInstPtr);
		CurrTextureHandle = NULL;
	}
	*/

	D3D_OutputTriangles();
}
void D3D_CloakedPredatorPolygon_Output(POLYHEADER *inputPolyPtr,RENDERVERTEX *renderVerticesPtr)
{
	float ZNear = (float) (Global_VDB_Ptr->VDB_ClipZ * GlobalScale);

	int texoffset = SpecialFXImageNumber;
	D3DTEXTUREHANDLE TextureHandle = (D3DTEXTUREHANDLE)ImageHeaderArray[texoffset].D3DHandle;
	float RecipW, RecipH;
	
	if(ImageHeaderArray[texoffset].ImageWidth==256)
	{
		RecipW = 1.0 /256.0;
	}
	else
	{
		float width = (float) ImageHeaderArray[texoffset].ImageWidth;
		RecipW = (1.0 / width);
	}
	if(ImageHeaderArray[texoffset].ImageHeight==256)
	{
		RecipH = 1.0 / 256.0;
	}
	else
	{
		float height = (float) ImageHeaderArray[texoffset].ImageHeight;
		RecipH = (1.0 / height);
	}

	/* OUTPUT VERTICES TO EXECUTE BUFFER */
	{
		int i = RenderPolygon.NumberOfVertices;
		RENDERVERTEX *vertices = renderVerticesPtr;

		do
		{
			D3DTLVERTEX *vertexPtr = &((LPD3DTLVERTEX)ExecuteBufferDataArea)[NumVertices];
		  	float oneOverZ;
		  	oneOverZ = (1.0)/(vertices->Z);
			float zvalue;

			vertexPtr->tu = ((float)vertices->U) * RecipW;
			vertexPtr->tv = ((float)vertices->V) * RecipH;
			vertexPtr->rhw = oneOverZ;

			{
				zvalue = vertices->Z+HeadUpDisplayZOffset;
	   //			zvalue /= 65536.0;
	   		   	zvalue = 1.0 - ZNear/zvalue;
			}	
			
			{
				int x = (vertices->X*(Global_VDB_Ptr->VDB_ProjX+1))/vertices->Z+Global_VDB_Ptr->VDB_CentreX;

				#if 1
				if (x<Global_VDB_Ptr->VDB_ClipLeft)
				{
					x=Global_VDB_Ptr->VDB_ClipLeft;
				}	
				else if (x>Global_VDB_Ptr->VDB_ClipRight)
				{
					x=Global_VDB_Ptr->VDB_ClipRight;	
				}
				#endif
				vertexPtr->sx=x;
			}
			{
				int y = (vertices->Y*(Global_VDB_Ptr->VDB_ProjY+1))/vertices->Z+Global_VDB_Ptr->VDB_CentreY;
				
				#if 1
				if (y<Global_VDB_Ptr->VDB_ClipUp)
				{
					y=Global_VDB_Ptr->VDB_ClipUp;
				}
				else if (y>Global_VDB_Ptr->VDB_ClipDown)
				{
					y=Global_VDB_Ptr->VDB_ClipDown;	
				}
				#endif
				vertexPtr->sy=y;
				
			}
			vertexPtr->color = RGBALIGHT_MAKE(255,255,255,32);
			vertexPtr->sz = zvalue;
		
			vertices++;
			NumVertices++;
		}
	  	while(--i);
	}

	CheckTranslucencyModeIsCorrect(TRANSLUCENCY_GLOWING);

	// Insert state change for shading model if required
    if (D3DShadingMode != D3DSHADE_GOURAUD)
	  {
   	   D3DShadingMode = D3DSHADE_GOURAUD;
	   OP_STATE_RENDER(1, ExecBufInstPtr);
	     STATE_DATA(D3DRENDERSTATE_SHADEMODE,
		       D3DSHADE_GOURAUD, ExecBufInstPtr);
	  }

// Insert state change for texturing perspective value
// Note that drawtx3das2d options have ONLY been allowed for here,
// not when the rhw values are generated.  This is a deliberate choice,
// based on the assumption that drawtx3das2d will not be used very often
// and the extra branching at the top of this function will impose a 
// greater cost than the (rare) savings in floating pt divisions are worth.
// Or so I claim...

    if (D3DTexturePerspective != Yes)
    {
		D3DTexturePerspective = Yes;
		OP_STATE_RENDER(1, ExecBufInstPtr);
		STATE_DATA(D3DRENDERSTATE_TEXTUREPERSPECTIVE, TRUE, ExecBufInstPtr);
	}

	#if 1
    if (TextureHandle != CurrTextureHandle)
	{
    	OP_STATE_RENDER(1, ExecBufInstPtr);
        STATE_DATA(D3DRENDERSTATE_TEXTUREHANDLE, TextureHandle, ExecBufInstPtr);
	   	CurrTextureHandle = TextureHandle;
	}
	#else
    if (CurrTextureHandle != NoiseTextureHandle)
	{
    	OP_STATE_RENDER(1, ExecBufInstPtr);
        STATE_DATA(D3DRENDERSTATE_TEXTUREHANDLE, NoiseTextureHandle, ExecBufInstPtr);
        CurrTextureHandle = NoiseTextureHandle;
	}
	#endif

	D3D_OutputTriangles();
}

void D3D_ZBufferedTexturedPolygon_Output(POLYHEADER *inputPolyPtr,RENDERVERTEX *renderVerticesPtr)
{
	int flags;
	int texoffset;
	int intensity;

	D3DTEXTUREHANDLE TextureHandle;

	float ZNear;
	float RecipW, RecipH;

    // Get ZNear
	ZNear = (float) (Global_VDB_Ptr->VDB_ClipZ * GlobalScale);


	// Take header information
	flags = inputPolyPtr->PolyFlags;

	intensity = (inputPolyPtr->PolyColour >> TxDefn);
	intensity >>= TxDefn;

	// We assume bit 15 (TxLocal) HAS been
	// properly cleared this time...
	texoffset = (inputPolyPtr->PolyColour & ClrTxDefn);

	TextureHandle = (D3DTEXTUREHANDLE)
	          ImageHeaderArray[texoffset].D3DHandle;

    // Check for textures that have not loaded
	// properly

    if (TextureHandle == (D3DTEXTUREHANDLE) 0)
	  return;
	
	if(ImageHeaderArray[texoffset].ImageWidth==128)
	{
		RecipW = 1.0 /128.0;
	}
	else
	{
		float width = (float) ImageHeaderArray[texoffset].ImageWidth;
		RecipW = (1.0 / width);
	}
	if(ImageHeaderArray[texoffset].ImageHeight==128)
	{
		RecipH = 1.0 / 128.0;
	}
	else
	{
		float height = (float) ImageHeaderArray[texoffset].ImageHeight;
		RecipH = (1.0 / height);
	}


	/* OUTPUT VERTICES TO EXECUTE BUFFER */
	{
		int i = RenderPolygon.NumberOfVertices;
		RENDERVERTEX *vertices = renderVerticesPtr;

		do
		{
			D3DTLVERTEX *vertexPtr = &((LPD3DTLVERTEX)ExecuteBufferDataArea)[NumVertices];
		  	float oneOverZ;
		  	oneOverZ = (1.0)/vertices->Z;
			float zvalue;

			{
				int x = (vertices->X*(Global_VDB_Ptr->VDB_ProjX+1))/vertices->Z+Global_VDB_Ptr->VDB_CentreX;
				if (x<Global_VDB_Ptr->VDB_ClipLeft)
				{
					x=Global_VDB_Ptr->VDB_ClipLeft;
				}	
				else if (x>Global_VDB_Ptr->VDB_ClipRight)
				{
					x=Global_VDB_Ptr->VDB_ClipRight;	
				}
				
				vertexPtr->sx=x;
			}
			{
				int y = (vertices->Y*(Global_VDB_Ptr->VDB_ProjY+1))/vertices->Z+Global_VDB_Ptr->VDB_CentreY;
				if (y<Global_VDB_Ptr->VDB_ClipUp)
				{
					y=Global_VDB_Ptr->VDB_ClipUp;
				}
				else if (y>Global_VDB_Ptr->VDB_ClipDown)
				{
					y=Global_VDB_Ptr->VDB_ClipDown;	
				}
				vertexPtr->sy=y;
			}
			vertexPtr->tu = ((float)(vertices->U>>16)+0.5) * RecipW;
			vertexPtr->tv = ((float)(vertices->V>>16)+0.5) * RecipH;
			vertexPtr->rhw = oneOverZ;

			{
				zvalue = vertices->Z+HeadUpDisplayZOffset;
				zvalue = ((zvalue-ZNear)/zvalue);
			}	
	
		  	 // Different behaviour for different driver modes
			switch (D3DDriverMode)
			{
				case D3DSoftwareRGBDriver:
				{
					if (flags & iflag_nolight)
					{
						vertexPtr->color = DefaultColour;
					}
					else
					{
						int i1shift = intensity;
					  	vertexPtr->color = RGBLIGHT_MAKE(i1shift, i1shift,i1shift);
					}
					break;
				}

				case D3DSoftwareRampDriver:
				{
					// Blue component only valid for ramp driver
					if (flags & iflag_nolight)
					{
						vertexPtr->color = RGB_MAKE(0,0,DefaultVertexIntensity);
					}
					else
					{
						vertexPtr->color = RGB_MAKE(0,0,intensity);
					}

					break;
				}
				
				case D3DHardwareRGBDriver:
				{
					if (flags & iflag_transparent)
					{
						if (flags & iflag_nolight)
						{
							vertexPtr->color = DefaultAlphaColour;
						}		
						else
						{
					  		vertexPtr->color = RGBALIGHT_MAKE(intensity,intensity,intensity, TransparentAlphaValue);
						}											   
					}
					else
					{
						if (flags & iflag_nolight)
						{
							vertexPtr->color = DefaultColour;
						}
						else
						{
							vertexPtr->color = RGBLIGHT_MAKE(intensity,intensity,intensity);
						}
					}
					break;
				}


				default:
				{
					vertexPtr->color = 0;
					break;
				}
			}
			vertexPtr->sz = zvalue;
			vertexPtr->specular=RGBALIGHT_MAKE(0,0,0,255);
			vertices++;
			NumVertices++;
		}
	  	while(--i);
	}

	if (flags & iflag_transparent)
	{ 
		CheckTranslucencyModeIsCorrect(TRANSLUCENCY_NORMAL);
	}
	else
	{ 
		CheckTranslucencyModeIsCorrect(TRANSLUCENCY_OFF);
	}

	// Insert state change for shading model if required
    if (D3DShadingMode != D3DSHADE_GOURAUD)
	  {
   	   D3DShadingMode = D3DSHADE_GOURAUD;
	   OP_STATE_RENDER(1, ExecBufInstPtr);
	     STATE_DATA(D3DRENDERSTATE_SHADEMODE,
		       D3DSHADE_GOURAUD, ExecBufInstPtr);
	  }

// Insert state change for texturing perspective value
// Note that drawtx3das2d options have ONLY been allowed for here,
// not when the rhw values are generated.  This is a deliberate choice,
// based on the assumption that drawtx3das2d will not be used very often
// and the extra branching at the top of this function will impose a 
// greater cost than the (rare) savings in floating pt divisions are worth.
// Or so I claim...

    if ((flags & iflag_drawtx3das2d) ||
	   (Global_VDB_Ptr->VDB_Flags & ViewDB_Flag_drawtx3das2d))
	  {
       if (D3DTexturePerspective != No)
	     {
	      D3DTexturePerspective = No;
          OP_STATE_RENDER(1, ExecBufInstPtr);
	        STATE_DATA(D3DRENDERSTATE_TEXTUREPERSPECTIVE, 
	           FALSE, ExecBufInstPtr);
	     }
	  }
	else
	  {
       if (D3DTexturePerspective != Yes)
	     {
	      D3DTexturePerspective = Yes;
          OP_STATE_RENDER(1, ExecBufInstPtr);
	        STATE_DATA(D3DRENDERSTATE_TEXTUREPERSPECTIVE, 
	           TRUE, ExecBufInstPtr);
	     }
	  }

    if (TextureHandle != CurrTextureHandle)
	  {
       OP_STATE_RENDER(1, ExecBufInstPtr);
          STATE_DATA(D3DRENDERSTATE_TEXTUREHANDLE, TextureHandle, ExecBufInstPtr);
	   CurrTextureHandle = TextureHandle;
	  }
	D3D_OutputTriangles();
}
#define OUTPUT_TRIANGLE(a,b,c,n) \
((LPD3DTRIANGLE)ExecBufInstPtr)->v1 = (NumVertices+(a)-(n)); \
((LPD3DTRIANGLE)ExecBufInstPtr)->v2 = (NumVertices+(b)-(n)); \
((LPD3DTRIANGLE)ExecBufInstPtr)->v3 = (NumVertices+(c)-(n)); \
((LPD3DTRIANGLE)ExecBufInstPtr)->wFlags = D3DTRIFLAG_EDGEENABLETRIANGLE; \
ExecBufInstPtr = ((char*)ExecBufInstPtr) + sizeof(D3DTRIANGLE); \
NumberOfRenderedTriangles++;

static void D3D_OutputTriangles(void)
{
	#if 0
    OP_STATE_RENDER(1, ExecBufInstPtr);
    STATE_DATA(D3DRENDERSTATE_TEXTUREHANDLE, NULL, ExecBufInstPtr);
	#endif

	#if 0
	/* KJL 18:17:54 24/02/98 - doesn't seem to make any difference! */
	if (QWORD_ALIGNED(ExecBufInstPtr))
    {
        OP_NOP(ExecBufInstPtr);
	}
	#endif
	
	switch(RenderPolygon.NumberOfVertices)
	{
		default:
		case 3:
		{
			OP_TRIANGLE_LIST(1, ExecBufInstPtr);
			OUTPUT_TRIANGLE(0,2,1, 3);
			break;
		}
		case 4:
		{
			OP_TRIANGLE_LIST(2, ExecBufInstPtr);
			OUTPUT_TRIANGLE(0,1,2, 4);
			OUTPUT_TRIANGLE(0,2,3, 4);
			break;
		}
		case 5:
		{
			OP_TRIANGLE_LIST(3, ExecBufInstPtr);
		    OUTPUT_TRIANGLE(0,1,4, 5);
		    OUTPUT_TRIANGLE(1,3,4, 5);
		    OUTPUT_TRIANGLE(1,2,3, 5);
			break;
		}
		case 6:
		{
			OP_TRIANGLE_LIST(4, ExecBufInstPtr);
		    OUTPUT_TRIANGLE(0,4,5, 6);
		    OUTPUT_TRIANGLE(0,3,4, 6);
		    OUTPUT_TRIANGLE(0,2,3, 6);
		    OUTPUT_TRIANGLE(0,1,2, 6);
			break;
		}
		case 7:
		{
			OP_TRIANGLE_LIST(5, ExecBufInstPtr);
		    OUTPUT_TRIANGLE(0,5,6, 7);
		    OUTPUT_TRIANGLE(0,4,5, 7);
		    OUTPUT_TRIANGLE(0,3,4, 7);
		    OUTPUT_TRIANGLE(0,2,3, 7);
		    OUTPUT_TRIANGLE(0,1,2, 7);
			break;
		}		
		case 8:
		{
			OP_TRIANGLE_LIST(6, ExecBufInstPtr);
		    OUTPUT_TRIANGLE(0,6,7, 8);
		    OUTPUT_TRIANGLE(0,5,6, 8);
		    OUTPUT_TRIANGLE(0,4,5, 8);
		    OUTPUT_TRIANGLE(0,3,4, 8);
		    OUTPUT_TRIANGLE(0,2,3, 8);
		    OUTPUT_TRIANGLE(0,1,2, 8);
			break;
		}		
				

	}
	if (NumVertices > (MaxVerticesInExecuteBuffer-12)) 
	{
	   WriteEndCodeToExecuteBuffer();
  	   UnlockExecuteBufferAndPrepareForUse();
	   ExecuteBuffer();
  	   LockExecuteBuffer();
	}
}

void D3D_HUD_Setup(void)
{
	CheckTranslucencyModeIsCorrect(TRANSLUCENCY_GLOWING);

	// turn off perspective drawing
    if (D3DTexturePerspective != No)
	{
		D3DTexturePerspective = No;
		OP_STATE_RENDER(1, ExecBufInstPtr);
		STATE_DATA(D3DRENDERSTATE_TEXTUREPERSPECTIVE, FALSE, ExecBufInstPtr);
	}

/*	
    OP_STATE_RENDER(2, ExecBufInstPtr);
	STATE_DATA(D3DRENDERSTATE_TEXTUREMAG, D3DFILTER_NEAREST, ExecBufInstPtr);
	STATE_DATA(D3DRENDERSTATE_TEXTUREMIN, D3DFILTER_NEAREST, ExecBufInstPtr);
    OP_STATE_RENDER(2, ExecBufInstPtr);
    STATE_DATA(D3DRENDERSTATE_TEXTUREMAG, DefaultD3DTextureFilterMax, ExecBufInstPtr);
    STATE_DATA(D3DRENDERSTATE_TEXTUREMIN, DefaultD3DTextureFilterMin, ExecBufInstPtr);
*/
	OP_STATE_RENDER(1, ExecBufInstPtr);
    STATE_DATA(D3DRENDERSTATE_ZFUNC, D3DCMP_LESSEQUAL, ExecBufInstPtr);

}


void D3D_HUDQuad_Output(int imageNumber,struct VertexTag *quadVerticesPtr, unsigned int colour)
{
	float RecipW, RecipH;

	D3DTEXTUREHANDLE TextureHandle = (D3DTEXTUREHANDLE)ImageHeaderArray[imageNumber].D3DHandle;

    // Check for textures that have not loaded properly
    LOCALASSERT(TextureHandle != (D3DTEXTUREHANDLE) 0);
	
	if(ImageHeaderArray[imageNumber].ImageWidth==128)
	{
		RecipW = 1.0f / 128.0f;
	}
	else
	{
		float width = (float) ImageHeaderArray[imageNumber].ImageWidth - 0.0f;
		RecipW = (1.0f / width);
	}
	if(ImageHeaderArray[imageNumber].ImageHeight==128)
	{
		RecipH = 1.0f / 128.0f;
	}
	else
	{
		float height = (float) ImageHeaderArray[imageNumber].ImageHeight - 0.0f;
		RecipH = (1.0f / height);
	}


	/* OUTPUT quadVerticesPtr TO EXECUTE BUFFER */
	{
		int i = 4;
		do
		{
			D3DTLVERTEX *vertexPtr = &((LPD3DTLVERTEX)ExecuteBufferDataArea)[NumVertices];
			
//			textprint("x %d, y %d, u %d, v %d\n",quadVerticesPtr->X,quadVerticesPtr->Y,quadVerticesPtr->U,quadVerticesPtr->V);
			vertexPtr->sx=quadVerticesPtr->X;
			vertexPtr->sy=quadVerticesPtr->Y;
			vertexPtr->tu = ((float)(quadVerticesPtr->U)) * RecipW;
			vertexPtr->tv = ((float)(quadVerticesPtr->V)) * RecipH;
			vertexPtr->rhw = 1.0f;
	  		vertexPtr->color = colour;
			vertexPtr->specular = RGBALIGHT_MAKE(0,0,0,255);

			quadVerticesPtr++;
			NumVertices++;
		}
	  	while(--i);
	}

	// set correct texture handle
    if (TextureHandle != CurrTextureHandle)
	{
       OP_STATE_RENDER(1, ExecBufInstPtr);
       STATE_DATA(D3DRENDERSTATE_TEXTUREHANDLE, TextureHandle, ExecBufInstPtr);
	   CurrTextureHandle = TextureHandle;
	}
	
	CheckTranslucencyModeIsCorrect(TRANSLUCENCY_GLOWING);


	/* output triangles to execute buffer */
	OP_TRIANGLE_LIST(2, ExecBufInstPtr);
	OUTPUT_TRIANGLE(0,1,3, 4);
	OUTPUT_TRIANGLE(1,2,3, 4);
	
	/* check to see if buffer is getting full */
	if (NumVertices > (MaxVerticesInExecuteBuffer-12)) 
	{
	   WriteEndCodeToExecuteBuffer();
  	   UnlockExecuteBufferAndPrepareForUse();
	   ExecuteBuffer();
  	   LockExecuteBuffer();
	}
}


void D3D_DrawRebellionLogo(unsigned int alpha)
{
	extern int RebellionLogoImageNumber;
	extern int FoxLogoImageNumber;
	float scale = ScreenDescriptorBlock.SDB_Width/640.0f;
	D3DTEXTUREHANDLE TextureHandle = (D3DTEXTUREHANDLE)ImageHeaderArray[FoxLogoImageNumber].D3DHandle;
	int colour = 0xffffff+((alpha>>8)<<24);
    // Check for textures that have not loaded properly
    LOCALASSERT(TextureHandle != (D3DTEXTUREHANDLE) 0);
	
	/* OUTPUT quadVerticesPtr TO EXECUTE BUFFER */
	{
		float x[] = {0.0f,1.0f,1.0f,0.0f};
		float y[] = {0.0f,0.0f,1.0f,1.0f};
		int i;
		for (i=0; i<4; i++)
		{
			D3DTLVERTEX *vertexPtr = &((LPD3DTLVERTEX)ExecuteBufferDataArea)[NumVertices];
			vertexPtr->sx = (x[i]*63.0f+64.0f)*scale;
			vertexPtr->sy = (y[i]*63.0f+4.0f)*scale;
			vertexPtr->sz = 0.0f;
			vertexPtr->tu = x[i];
			vertexPtr->tv = y[i];
			vertexPtr->rhw = 1.0;
	  		vertexPtr->color = colour;
			vertexPtr->specular = RGBALIGHT_MAKE(0,0,0,255);
			
			NumVertices++;
		}
	}
	// set correct texture handle
    if (TextureHandle != CurrTextureHandle)
	{
       OP_STATE_RENDER(1, ExecBufInstPtr);
       STATE_DATA(D3DRENDERSTATE_TEXTUREHANDLE, TextureHandle, ExecBufInstPtr);
	   CurrTextureHandle = TextureHandle;
	}
	CheckTranslucencyModeIsCorrect(TRANSLUCENCY_NORMAL);


	/* output triangles to execute buffer */
	OP_TRIANGLE_LIST(2, ExecBufInstPtr);
	OUTPUT_TRIANGLE(0,1,3, 4);
	OUTPUT_TRIANGLE(1,2,3, 4);
	
	/* check to see if buffer is getting full */
	if (NumVertices > (MaxVerticesInExecuteBuffer-12)) 
	{
	   WriteEndCodeToExecuteBuffer();
  	   UnlockExecuteBufferAndPrepareForUse();
	   ExecuteBuffer();
  	   LockExecuteBuffer();
	}
	
	TextureHandle = (D3DTEXTUREHANDLE)ImageHeaderArray[RebellionLogoImageNumber].D3DHandle;

	{
		float x[] = {0.0f,1.0f,1.0f,0.0f};
		float y[] = {0.0f,0.0f,1.0f,1.0f};
		int i;
		for (i=0; i<4; i++)
		{
			D3DTLVERTEX *vertexPtr = &((LPD3DTLVERTEX)ExecuteBufferDataArea)[NumVertices];
			vertexPtr->sx = (x[i]*127.0f+4.0f+130.0f)*scale;
			vertexPtr->sy = (y[i]*63.0f+4.0f)*scale;
			vertexPtr->sz = 0.0f;
			vertexPtr->tu = x[i];
			vertexPtr->tv = y[i];
			vertexPtr->rhw = 1.0;
	  		vertexPtr->color = colour;
			vertexPtr->specular = RGBALIGHT_MAKE(0,0,0,255);
			
			NumVertices++;
		}
	}

	// set correct texture handle
    if (TextureHandle != CurrTextureHandle)
	{
       OP_STATE_RENDER(1, ExecBufInstPtr);
       STATE_DATA(D3DRENDERSTATE_TEXTUREHANDLE, TextureHandle, ExecBufInstPtr);
	   CurrTextureHandle = TextureHandle;
	}
	CheckTranslucencyModeIsCorrect(TRANSLUCENCY_NORMAL);


	/* output triangles to execute buffer */
	OP_TRIANGLE_LIST(2, ExecBufInstPtr);
	OUTPUT_TRIANGLE(0,1,3, 4);
	OUTPUT_TRIANGLE(1,2,3, 4);
	
	/* check to see if buffer is getting full */
	if (NumVertices > (MaxVerticesInExecuteBuffer-12)) 
	{
	   WriteEndCodeToExecuteBuffer();
  	   UnlockExecuteBufferAndPrepareForUse();
	   ExecuteBuffer();
  	   LockExecuteBuffer();
	}
}


void D3D_DrawParticle_Rain(PARTICLE *particlePtr,VECTORCH *prevPositionPtr)
{
	VECTORCH vertices[3];
	vertices[0] = *prevPositionPtr;

	/* translate second vertex into view space */
	TranslatePointIntoViewspace(&vertices[0]);

	/* is particle within normal view frustrum ? */
	if((-vertices[0].vx <= vertices[0].vz)
	&&(vertices[0].vx <= vertices[0].vz)
	&&(-vertices[0].vy <= vertices[0].vz)
	&&(vertices[0].vy <= vertices[0].vz))
	{													

		vertices[1] = particlePtr->Position;
		vertices[2] = particlePtr->Position;
		vertices[1].vx += particlePtr->Offset.vx;
		vertices[2].vx -= particlePtr->Offset.vx;
		vertices[1].vz += particlePtr->Offset.vz;
		vertices[2].vz -= particlePtr->Offset.vz;

		/* translate particle into view space */
		TranslatePointIntoViewspace(&vertices[1]);
		TranslatePointIntoViewspace(&vertices[2]);

		float ZNear = (float) (Global_VDB_Ptr->VDB_ClipZ * GlobalScale);



		/* OUTPUT VERTICES TO EXECUTE BUFFER */
		{
			int i = 3;
			VECTORCH *verticesPtr = vertices;
			do
			{
				D3DTLVERTEX *vertexPtr = &((LPD3DTLVERTEX)ExecuteBufferDataArea)[NumVertices];
				int x = (verticesPtr->vx*(Global_VDB_Ptr->VDB_ProjX))/verticesPtr->vz+Global_VDB_Ptr->VDB_CentreX;
				int y = (verticesPtr->vy*(Global_VDB_Ptr->VDB_ProjY))/verticesPtr->vz+Global_VDB_Ptr->VDB_CentreY;
				{
					if (x<Global_VDB_Ptr->VDB_ClipLeft)
					{
						x=Global_VDB_Ptr->VDB_ClipLeft;
					}	
					else if (x>Global_VDB_Ptr->VDB_ClipRight)
					{
						x=Global_VDB_Ptr->VDB_ClipRight;	
					}
					
					vertexPtr->sx=x;
				}
				{
					if (y<Global_VDB_Ptr->VDB_ClipUp)
					{
						y=Global_VDB_Ptr->VDB_ClipUp;
					}
					else if (y>Global_VDB_Ptr->VDB_ClipDown)
					{
						y=Global_VDB_Ptr->VDB_ClipDown;	
					}
					vertexPtr->sy=y;
				}
		
			  	float oneOverZ = ((float)verticesPtr->vz-ZNear)/(float)verticesPtr->vz;
		
				if (i==3) vertexPtr->color = RGBALIGHT_MAKE(0,255,255,32);
				else vertexPtr->color = RGBALIGHT_MAKE(255,255,255,32);

				vertexPtr->specular = RGBALIGHT_MAKE(0,0,0,255);
				vertexPtr->sz = oneOverZ;
				NumVertices++;
				verticesPtr++;
			}
		  	while(--i);
		}
		if (CurrTextureHandle != NULL)
		{
			OP_STATE_RENDER(1, ExecBufInstPtr);
			STATE_DATA(D3DRENDERSTATE_TEXTUREHANDLE, NULL, ExecBufInstPtr);
			CurrTextureHandle = NULL;
	  	}
		CheckTranslucencyModeIsCorrect(TRANSLUCENCY_NORMAL);

		
		OP_TRIANGLE_LIST(1, ExecBufInstPtr);
		OUTPUT_TRIANGLE(0,2,1, 3);
		if (NumVertices > (MaxVerticesInExecuteBuffer-12)) 
		{
		   WriteEndCodeToExecuteBuffer();
	  	   UnlockExecuteBufferAndPrepareForUse();
		   ExecuteBuffer();
	  	   LockExecuteBuffer();
		}
	}

}

void D3D_DrawParticle_Smoke(PARTICLE *particlePtr)
{
	VECTORCH vertices[3];
	vertices[0] = particlePtr->Position;

	/* translate second vertex into view space */
	TranslatePointIntoViewspace(&vertices[0]);

	/* is particle within normal view frustrum ? */
	int inView = 0;

	if(AvP.PlayerType == I_Alien)
	{
		if((-vertices[0].vx <= vertices[0].vz*2)
		&&(vertices[0].vx <= vertices[0].vz*2)
		&&(-vertices[0].vy <= vertices[0].vz*2)
		&&(vertices[0].vy <= vertices[0].vz*2))
		{
			inView = 1;
		}
	}
	else
	{
		if((-vertices[0].vx <= vertices[0].vz)
		&&(vertices[0].vx <= vertices[0].vz)
		&&(-vertices[0].vy <= vertices[0].vz)
		&&(vertices[0].vy <= vertices[0].vz))
		{
			inView = 1;
		}
	}

	if (inView)
	{													

		vertices[1] = particlePtr->Position;
		vertices[2] = particlePtr->Position;
		vertices[1].vx += ((FastRandom()&15)-8)*2;
		vertices[1].vy += ((FastRandom()&15)-8)*2;
		vertices[1].vz += ((FastRandom()&15)-8)*2;
		vertices[2].vx += ((FastRandom()&15)-8)*2;
		vertices[2].vy += ((FastRandom()&15)-8)*2;
		vertices[2].vz += ((FastRandom()&15)-8)*2;

		/* translate particle into view space */
		TranslatePointIntoViewspace(&vertices[1]);
		TranslatePointIntoViewspace(&vertices[2]);

		float ZNear = (float) (Global_VDB_Ptr->VDB_ClipZ * GlobalScale);



		/* OUTPUT VERTICES TO EXECUTE BUFFER */
		{
			int i = 3;
			VECTORCH *verticesPtr = vertices;
			do
			{
				D3DTLVERTEX *vertexPtr = &((LPD3DTLVERTEX)ExecuteBufferDataArea)[NumVertices];
				int x = (verticesPtr->vx*(Global_VDB_Ptr->VDB_ProjX))/verticesPtr->vz+Global_VDB_Ptr->VDB_CentreX;
				int y = (verticesPtr->vy*(Global_VDB_Ptr->VDB_ProjY))/verticesPtr->vz+Global_VDB_Ptr->VDB_CentreY;
				{
					if (x<Global_VDB_Ptr->VDB_ClipLeft)
					{
						x=Global_VDB_Ptr->VDB_ClipLeft;
					}	
					else if (x>Global_VDB_Ptr->VDB_ClipRight)
					{
						x=Global_VDB_Ptr->VDB_ClipRight;	
					}
					
					vertexPtr->sx=x;
				}
				{
					if (y<Global_VDB_Ptr->VDB_ClipUp)
					{
						y=Global_VDB_Ptr->VDB_ClipUp;
					}
					else if (y>Global_VDB_Ptr->VDB_ClipDown)
					{
						y=Global_VDB_Ptr->VDB_ClipDown;	
					}
					vertexPtr->sy=y;
				}
		
				float zvalue = (vertices->vz)+HeadUpDisplayZOffset;
				zvalue = ((zvalue-ZNear)/zvalue);
		
			  	 // Different behaviour for different driver modes
				switch (D3DDriverMode)
				{
 					default:
					case D3DSoftwareRGBDriver:
					case D3DSoftwareRampDriver:
					break;
					case D3DHardwareRGBDriver:
					{
						vertexPtr->color = RGBALIGHT_MAKE((particlePtr->LifeTime>>8),(particlePtr->LifeTime>>8),0,(particlePtr->LifeTime>>7)+64);
						break;
					}
				}
				vertexPtr->sz = zvalue;
				NumVertices++;
				verticesPtr++;
			}
		  	while(--i);
		}
		OP_TRIANGLE_LIST(1, ExecBufInstPtr);
		OUTPUT_TRIANGLE(0,2,1, 3);
		if (NumVertices > (MaxVerticesInExecuteBuffer-12)) 
		{
		   WriteEndCodeToExecuteBuffer();
	  	   UnlockExecuteBufferAndPrepareForUse();
		   ExecuteBuffer();
	  	   LockExecuteBuffer();
		}
	}

}


void D3D_DecalSystem_Setup(void)
{
#if 1
	OP_STATE_RENDER(1, ExecBufInstPtr);
//	STATE_DATA(D3DRENDERSTATE_DITHERENABLE, FALSE, ExecBufInstPtr);
    STATE_DATA(D3DRENDERSTATE_ZWRITEENABLE, FALSE, ExecBufInstPtr);
#endif
}
void D3D_DecalSystem_End(void)
{
#if 1
	OP_STATE_RENDER(1, ExecBufInstPtr);
//	STATE_DATA(D3DRENDERSTATE_DITHERENABLE, TRUE, ExecBufInstPtr);
    STATE_DATA(D3DRENDERSTATE_ZWRITEENABLE, TRUE, ExecBufInstPtr);
#endif
}

void D3D_Decal_Output(DECAL *decalPtr,RENDERVERTEX *renderVerticesPtr)
{
	DECAL_DESC *decalDescPtr = &DecalDescription[decalPtr->DecalID];
	
	int texoffset;
	D3DTEXTUREHANDLE TextureHandle;

	float ZNear;
	float RecipW, RecipH;
	int colour;
	int specular=RGBALIGHT_MAKE(0,0,0,0);//255);

    // Get ZNear
	ZNear = (float) (Global_VDB_Ptr->VDB_ClipZ * GlobalScale);

	if (decalPtr->DecalID == DECAL_FMV)
	{
		#if !FMV_ON
		return;
		#endif
		TextureHandle=FMVTextureHandle[decalPtr->Centre.vx];
		RecipW = 1.0 /128.0;
		RecipH = 1.0 /128.0;
	    if (TextureHandle != CurrTextureHandle)
		{
	    	OP_STATE_RENDER(1, ExecBufInstPtr);
	        STATE_DATA(D3DRENDERSTATE_TEXTUREHANDLE, TextureHandle, ExecBufInstPtr);
		   	CurrTextureHandle = TextureHandle;
		}
	}
	else if (decalPtr->DecalID == DECAL_SHAFTOFLIGHT||decalPtr->DecalID == DECAL_SHAFTOFLIGHT_OUTER)
	{
	    if (NULL != CurrTextureHandle)
		{
		   	CurrTextureHandle = NULL;
	    	OP_STATE_RENDER(1, ExecBufInstPtr);
	        STATE_DATA(D3DRENDERSTATE_TEXTUREHANDLE, NULL, ExecBufInstPtr);
		}
	}
	else
	{
		texoffset = SpecialFXImageNumber;
		TextureHandle = (D3DTEXTUREHANDLE)ImageHeaderArray[texoffset].D3DHandle;

	    // Check for textures that have not loaded
		// properly

	    if (TextureHandle == (D3DTEXTUREHANDLE) 0)
		  return;
		
		if(ImageHeaderArray[texoffset].ImageWidth==256)
		{
			RecipW = 1.0 /256.0;
		}
		else
		{
			float width = (float) ImageHeaderArray[texoffset].ImageWidth;
			RecipW = (1.0 / width);
		}
		if(ImageHeaderArray[texoffset].ImageHeight==256)
		{
			RecipH = 1.0 / 256.0;
		}
		else
		{
			float height = (float) ImageHeaderArray[texoffset].ImageHeight;
			RecipH = (1.0 / height);
		}
	    if (TextureHandle != CurrTextureHandle)
		{
	    	OP_STATE_RENDER(1, ExecBufInstPtr);
	        STATE_DATA(D3DRENDERSTATE_TEXTUREHANDLE, TextureHandle, ExecBufInstPtr);
		   	CurrTextureHandle = TextureHandle;
		}
	}

	if (decalDescPtr->IsLit) 
	{
		int intensity = LightIntensityAtPoint(decalPtr->Vertices);
		colour = RGBALIGHT_MAKE
	  		  	(
	  		   		MUL_FIXED(intensity,decalDescPtr->RedScale[CurrentVisionMode]),
	  		   		MUL_FIXED(intensity,decalDescPtr->GreenScale[CurrentVisionMode]),
	  		   		MUL_FIXED(intensity,decalDescPtr->BlueScale[CurrentVisionMode]),
	  		   		decalDescPtr->Alpha
	  		   	);
	}
	else
	{
		colour = RGBALIGHT_MAKE
			  	(
			   		decalDescPtr->RedScale[CurrentVisionMode],
			   		decalDescPtr->GreenScale[CurrentVisionMode],
			   		decalDescPtr->BlueScale[CurrentVisionMode],
			   		decalDescPtr->Alpha
			   	);
	}
	
	if (RAINBOWBLOOD_CHEATMODE)
	{
		colour = RGBALIGHT_MAKE
							  (
							  	FastRandom()&255,
							  	FastRandom()&255,
							  	FastRandom()&255,
							  	decalDescPtr->Alpha
							  );
	}
	/* OUTPUT VERTICES TO EXECUTE BUFFER */
	{
		int i = RenderPolygon.NumberOfVertices;
		RENDERVERTEX *vertices = renderVerticesPtr;

		do
		{
			D3DTLVERTEX *vertexPtr = &((LPD3DTLVERTEX)ExecuteBufferDataArea)[NumVertices];
		  	float oneOverZ;
		  	oneOverZ = (1.0)/vertices->Z;
			float zvalue;

			{
				int x = (vertices->X*(Global_VDB_Ptr->VDB_ProjX+1))/vertices->Z+Global_VDB_Ptr->VDB_CentreX;

				if (x<Global_VDB_Ptr->VDB_ClipLeft)
				{
					x=Global_VDB_Ptr->VDB_ClipLeft;
				}	
				else if (x>Global_VDB_Ptr->VDB_ClipRight)
				{
					x=Global_VDB_Ptr->VDB_ClipRight;	
				}
				
				vertexPtr->sx=x;
			}
			{
				int y = (vertices->Y*(Global_VDB_Ptr->VDB_ProjY+1))/vertices->Z+Global_VDB_Ptr->VDB_CentreY;
				
				if (y<Global_VDB_Ptr->VDB_ClipUp)
				{
					y=Global_VDB_Ptr->VDB_ClipUp;
				}
				else if (y>Global_VDB_Ptr->VDB_ClipDown)
				{
					y=Global_VDB_Ptr->VDB_ClipDown;	
				}
				vertexPtr->sy=y;
				
			}
			vertexPtr->tu = ((float)(vertices->U>>16)+.5) * RecipW;
			vertexPtr->tv = ((float)(vertices->V>>16)+.5) * RecipH;
			vertexPtr->rhw = oneOverZ;

			{				  
				zvalue = (vertices->Z)+HeadUpDisplayZOffset-50;
			   	zvalue = ((zvalue-ZNear)/zvalue);
			}	
	
			vertexPtr->color = colour;

			vertexPtr->sz = zvalue;

		   	vertexPtr->specular= specular;//RGBALIGHT_MAKE(vertices->SpecularR,vertices->SpecularG,vertices->SpecularB,fog);


			vertices++;
			NumVertices++;
		}
	  	while(--i);
	}

	/* Check translucency mode */
	CheckTranslucencyModeIsCorrect(decalDescPtr->TranslucencyType);


    if (D3DTexturePerspective != Yes)
    {
		D3DTexturePerspective = Yes;
		OP_STATE_RENDER(1, ExecBufInstPtr);
		STATE_DATA(D3DRENDERSTATE_TEXTUREPERSPECTIVE, TRUE, ExecBufInstPtr);
	}



	D3D_OutputTriangles();
}
void D3D_Particle_Output(PARTICLE *particlePtr,RENDERVERTEX *renderVerticesPtr)
{
	#if 1
	PARTICLE_DESC *particleDescPtr = &ParticleDescription[particlePtr->ParticleID];

	int texoffset = SpecialFXImageNumber;
	D3DTEXTUREHANDLE TextureHandle = (D3DTEXTUREHANDLE)ImageHeaderArray[texoffset].D3DHandle;
	float RecipW, RecipH;
	

    // Check for textures that have not loaded
	// properly

    if (TextureHandle == (D3DTEXTUREHANDLE) 0)
	  return;
	
	if(ImageHeaderArray[texoffset].ImageWidth==256)
	{
		RecipW = 1.0 /256.0;
	}
	else
	{
		float width = (float) ImageHeaderArray[texoffset].ImageWidth;
		RecipW = (1.0 / width);
	}
	if(ImageHeaderArray[texoffset].ImageHeight==256)
	{
		RecipH = 1.0 / 256.0;
	}
	else
	{
		float height = (float) ImageHeaderArray[texoffset].ImageHeight;
		RecipH = (1.0 / height);
	}

	int colour;

	if (particleDescPtr->IsLit && !(particlePtr->ParticleID==PARTICLE_ALIEN_BLOOD && CurrentVisionMode==VISION_MODE_PRED_SEEALIENS) ) 
	{
		int intensity = LightIntensityAtPoint(&particlePtr->Position);
		if (particlePtr->ParticleID==PARTICLE_SMOKECLOUD || particlePtr->ParticleID==PARTICLE_ANDROID_BLOOD)
		{
			colour = RGBALIGHT_MAKE
				  	(
				   		MUL_FIXED(intensity,particlePtr->ColourComponents.Red),
				   		MUL_FIXED(intensity,particlePtr->ColourComponents.Green),
				   		MUL_FIXED(intensity,particlePtr->ColourComponents.Blue),
				   		particlePtr->ColourComponents.Alpha
				   	);
			
		}
		else
		{
			colour = RGBALIGHT_MAKE
				  	(
				   		MUL_FIXED(intensity,particleDescPtr->RedScale[CurrentVisionMode]),
				   		MUL_FIXED(intensity,particleDescPtr->GreenScale[CurrentVisionMode]),
				   		MUL_FIXED(intensity,particleDescPtr->BlueScale[CurrentVisionMode]),
				   		particleDescPtr->Alpha
				   	);
		}
	}
	else
	{
		colour = particlePtr->Colour;
	}
	if (RAINBOWBLOOD_CHEATMODE)
	{
		colour = RGBALIGHT_MAKE
							  (
							  	FastRandom()&255,
							  	FastRandom()&255,
							  	FastRandom()&255,
							  	particleDescPtr->Alpha
							  );
	}
	

	{
		float ZNear = (float) (Global_VDB_Ptr->VDB_ClipZ * GlobalScale);


		{
			int i = RenderPolygon.NumberOfVertices;
			RENDERVERTEX *vertices = renderVerticesPtr;

			do
			{
				D3DTLVERTEX *vertexPtr = &((LPD3DTLVERTEX)ExecuteBufferDataArea)[NumVertices];
			  	float oneOverZ = (1.0)/vertices->Z;
				float zvalue;

				{
					int x = (vertices->X*(Global_VDB_Ptr->VDB_ProjX+1))/vertices->Z+Global_VDB_Ptr->VDB_CentreX;

					if (x<Global_VDB_Ptr->VDB_ClipLeft)
					{
						x=Global_VDB_Ptr->VDB_ClipLeft;
					}	
					else if (x>Global_VDB_Ptr->VDB_ClipRight)
					{
						x=Global_VDB_Ptr->VDB_ClipRight;	
					}
					
					vertexPtr->sx=x;
				}
				{
					int y = (vertices->Y*(Global_VDB_Ptr->VDB_ProjY+1))/vertices->Z+Global_VDB_Ptr->VDB_CentreY;
					
					if (y<Global_VDB_Ptr->VDB_ClipUp)
					{
						y=Global_VDB_Ptr->VDB_ClipUp;
					}
					else if (y>Global_VDB_Ptr->VDB_ClipDown)
					{
						y=Global_VDB_Ptr->VDB_ClipDown;	
					}
					vertexPtr->sy=y;
					
				}
				vertexPtr->tu = ((float)(vertices->U>>16)+.5) * RecipW;
				vertexPtr->tv = ((float)(vertices->V>>16)+.5) * RecipH;
				vertexPtr->rhw = oneOverZ;
				if (particleDescPtr->IsDrawnInFront)
				{
					zvalue = 0.0f;
				}
				else if (particleDescPtr->IsDrawnAtBack)
				{
					zvalue = 1.0f;
				}
				else
				{
					zvalue = 1.0f - ZNear*oneOverZ;
				}
 
				vertexPtr->color = colour;
				vertexPtr->sz = zvalue;
	 		   	vertexPtr->specular=RGBALIGHT_MAKE(0,0,0,255);//RGBALIGHT_MAKE(vertices->SpecularR,vertices->SpecularG,vertices->SpecularB,fog);

	 			NumVertices++;
				vertices++;
			}
		  	while(--i);
		}

		// set correct texture handle
	    if (TextureHandle != CurrTextureHandle)
		{
	       OP_STATE_RENDER(1, ExecBufInstPtr);
	       STATE_DATA(D3DRENDERSTATE_TEXTUREHANDLE, TextureHandle, ExecBufInstPtr);
		   CurrTextureHandle = TextureHandle;
		}

		CheckTranslucencyModeIsCorrect(particleDescPtr->TranslucencyType);

	    if (D3DTexturePerspective != Yes)
	    {
			D3DTexturePerspective = Yes;
			OP_STATE_RENDER(1, ExecBufInstPtr);
			STATE_DATA(D3DRENDERSTATE_TEXTUREPERSPECTIVE, TRUE, ExecBufInstPtr);
		}



		D3D_OutputTriangles();

	}
	#endif
}
void D3D_FMVParticle_Output(RENDERVERTEX *renderVerticesPtr)
{
	D3DTEXTUREHANDLE TextureHandle = FMVTextureHandle[0];
	float RecipW, RecipH;
	
	RecipW = 1.0 /128.0;
	RecipH = 1.0 /128.0;

	int colour = FMVParticleColour&0xffffff;

	{
		float ZNear = (float) (Global_VDB_Ptr->VDB_ClipZ * GlobalScale);


		{
			int i = RenderPolygon.NumberOfVertices;
			RENDERVERTEX *vertices = renderVerticesPtr;

			do
			{
				D3DTLVERTEX *vertexPtr = &((LPD3DTLVERTEX)ExecuteBufferDataArea)[NumVertices];
			  	float oneOverZ = (1.0)/vertices->Z;
				float zvalue;

				{
					int x = (vertices->X*(Global_VDB_Ptr->VDB_ProjX+1))/vertices->Z+Global_VDB_Ptr->VDB_CentreX;

					if (x<Global_VDB_Ptr->VDB_ClipLeft)
					{
						x=Global_VDB_Ptr->VDB_ClipLeft;
					}	
					else if (x>Global_VDB_Ptr->VDB_ClipRight)
					{
						x=Global_VDB_Ptr->VDB_ClipRight;	
					}
					
					vertexPtr->sx=x;
				}
				{
					int y = (vertices->Y*(Global_VDB_Ptr->VDB_ProjY+1))/vertices->Z+Global_VDB_Ptr->VDB_CentreY;
					
					if (y<Global_VDB_Ptr->VDB_ClipUp)
					{
						y=Global_VDB_Ptr->VDB_ClipUp;
					}
					else if (y>Global_VDB_Ptr->VDB_ClipDown)
					{
						y=Global_VDB_Ptr->VDB_ClipDown;	
					}
					vertexPtr->sy=y;
					
				}
				vertexPtr->tu = ((float)(vertices->U>>16)) * RecipW;
				vertexPtr->tv = ((float)(vertices->V>>16)) * RecipH;
				vertexPtr->rhw = oneOverZ;
				zvalue = 1.0 - ZNear*oneOverZ;
 
//				vertexPtr->color = colour;
				vertexPtr->color = (colour)+(vertices->A<<24);
				vertexPtr->sz = zvalue;
	 		   	vertexPtr->specular=RGBALIGHT_MAKE(0,0,0,255);//RGBALIGHT_MAKE(vertices->SpecularR,vertices->SpecularG,vertices->SpecularB,fog);

	 			NumVertices++;
				vertices++;
			}
		  	while(--i);
		}

		// set correct texture handle
	    if (TextureHandle != CurrTextureHandle)
		{
	       OP_STATE_RENDER(1, ExecBufInstPtr);
	       STATE_DATA(D3DRENDERSTATE_TEXTUREHANDLE, TextureHandle, ExecBufInstPtr);
		   CurrTextureHandle = TextureHandle;
		}

		CheckTranslucencyModeIsCorrect(TRANSLUCENCY_NORMAL);

	    if (D3DTexturePerspective != Yes)
	    {
			D3DTexturePerspective = Yes;
			OP_STATE_RENDER(1, ExecBufInstPtr);
			STATE_DATA(D3DRENDERSTATE_TEXTUREPERSPECTIVE, TRUE, ExecBufInstPtr);
		}



		D3D_OutputTriangles();

	}
}


extern int CloakingPhase;
extern int NumActiveBlocks;
extern DISPLAYBLOCK *ActiveBlockList[];
extern int GlobalAmbience;
extern unsigned char *ScreenBuffer;
extern long BackBufferPitch;

unsigned short FlameFunction(int x, int y);
void InitRandomArrays(void);
int Turbulence(int x, int y, int t);

void UpdateForceField(void);
void D3D_DrawForceField(int xOrigin, int yOrigin, int zOrigin, int fieldType);

void UpdateWaterFall(void);
void D3D_DrawWaterFall(int xOrigin, int yOrigin, int zOrigin);
void D3D_DrawPowerFence(int xOrigin, int yOrigin, int zOrigin, int xScale, int yScale, int zScale);
void D3D_DrawExplosion(int xOrigin, int yOrigin, int zOrigin, int size);

void D3D_DrawWaterPatch(int xOrigin, int yOrigin, int zOrigin);

void D3D_DrawWaterOctagonPatch(int xOrigin, int yOrigin, int zOrigin, int xOffset, int zOffset);

int LightSourceWaterPoint(VECTORCH *pointPtr,int offset);
void D3D_DrawWaterMesh_Unclipped(void);
void D3D_DrawWaterMesh_Clipped(void);


void D3D_DrawMoltenMetal(int xOrigin, int yOrigin, int zOrigin);
void D3D_DrawMoltenMetalMesh_Unclipped(void);
void D3D_DrawMoltenMetalMesh_Clipped(void);


//#define WATER_POLY_SCALE 256
int MeshXScale;
int MeshZScale;
int WaterFallBase;
void PostLandscapeRendering(void)
{
	extern int NumOnScreenBlocks;
	extern DISPLAYBLOCK *OnScreenBlockList[];
	int numOfObjects = NumOnScreenBlocks;

	extern char LevelName[];

  	CurrentRenderStates.FogIsOn = 1;

	if (!strcmp(LevelName,"fall")||!strcmp(LevelName,"fall_m"))
	{
		char drawWaterFall = 0;
		char drawStream = 0;

		while(numOfObjects)
		{
			DISPLAYBLOCK *objectPtr = OnScreenBlockList[--numOfObjects];
			MODULE *modulePtr = objectPtr->ObMyModule;

			/* if it's a module, which isn't inside another module */
			if (modulePtr && modulePtr->name)
			{
				if( (!strcmp(modulePtr->name,"fall01"))
				  ||(!strcmp(modulePtr->name,"well01"))
				  ||(!strcmp(modulePtr->name,"well02"))
				  ||(!strcmp(modulePtr->name,"well03"))
				  ||(!strcmp(modulePtr->name,"well04"))
				  ||(!strcmp(modulePtr->name,"well05"))
				  ||(!strcmp(modulePtr->name,"well06"))
				  ||(!strcmp(modulePtr->name,"well07"))
				  ||(!strcmp(modulePtr->name,"well08"))
				  ||(!strcmp(modulePtr->name,"well")))
				{
					drawWaterFall = 1;
				}
				else if( (!strcmp(modulePtr->name,"stream02"))
				       ||(!strcmp(modulePtr->name,"stream03"))
				       ||(!strcmp(modulePtr->name,"watergate")))
				{
		   			drawStream = 1;
				}
			}
		}	

		if (drawWaterFall)
		{
			// Turn OFF texturing if it is on...
			if (CurrTextureHandle != NULL)
			{
				OP_STATE_RENDER(1, ExecBufInstPtr);
				STATE_DATA(D3DRENDERSTATE_TEXTUREHANDLE, NULL, ExecBufInstPtr);
				CurrTextureHandle = NULL;
			}
			CheckTranslucencyModeIsCorrect(TRANSLUCENCY_NORMAL);
			if (NumVertices)
			{
			   WriteEndCodeToExecuteBuffer();
		  	   UnlockExecuteBufferAndPrepareForUse();
			   ExecuteBuffer();
		  	   LockExecuteBuffer();
			}
			OP_STATE_RENDER(1, ExecBufInstPtr);
		    //STATE_DATA(D3DRENDERSTATE_ZFUNC, D3DCMP_ALWAYS, ExecBufInstPtr);
			STATE_DATA(D3DRENDERSTATE_ZWRITEENABLE, FALSE, ExecBufInstPtr);

	   		//UpdateWaterFall();
			WaterFallBase = 109952;
			
			MeshZScale = (66572-51026)/15;
			MeshXScale = (109952+3039)/45;

	   		D3D_DrawWaterFall(175545,-3039,51026);
//			MeshZScale = -(538490-392169);
//			MeshXScale = 55000;
	//		D3D_DrawWaterPatch(-100000, WaterFallBase, 538490);
										
			OP_STATE_RENDER(1, ExecBufInstPtr);
		    //STATE_DATA(D3DRENDERSTATE_ZFUNC, D3DCMP_LESSEQUAL, ExecBufInstPtr);
		    STATE_DATA(D3DRENDERSTATE_ZWRITEENABLE, TRUE, ExecBufInstPtr);
		}
		if (drawStream)
		{
			int x = 68581;
			int y = 12925;
			int z = 93696;
			MeshXScale = (87869-68581);
			MeshZScale = (105385-93696);
			{
				extern void CheckForObjectsInWater(int minX, int maxX, int minZ, int maxZ, int averageY);
				CheckForObjectsInWater(x, x+MeshXScale, z, z+MeshZScale, y);
			}

			WaterXOrigin=x;
			WaterZOrigin=z;
			WaterUScale = 4.0f/(float)MeshXScale;
			WaterVScale = 4.0f/(float)MeshZScale;
		 	MeshXScale/=4;
		 	MeshZScale/=2;
			
			// Turn OFF texturing if it is on...
			D3DTEXTUREHANDLE TextureHandle = (D3DTEXTUREHANDLE)ImageHeaderArray[ChromeImageNumber].D3DHandle;
			if (CurrTextureHandle != TextureHandle)
			{
				OP_STATE_RENDER(1, ExecBufInstPtr);
				STATE_DATA(D3DRENDERSTATE_TEXTUREHANDLE, TextureHandle, ExecBufInstPtr);
				CurrTextureHandle = TextureHandle;
			}	 
			CheckTranslucencyModeIsCorrect(TRANSLUCENCY_NORMAL);
			if (NumVertices)
			{
			   WriteEndCodeToExecuteBuffer();
		  	   UnlockExecuteBufferAndPrepareForUse();
			   ExecuteBuffer();
		  	   LockExecuteBuffer();
			}
		 	D3D_DrawWaterPatch(x, y, z);
		 	D3D_DrawWaterPatch(x+MeshXScale, y, z);
		 	D3D_DrawWaterPatch(x+MeshXScale*2, y, z);
		 	D3D_DrawWaterPatch(x+MeshXScale*3, y, z);
		 	D3D_DrawWaterPatch(x, y, z+MeshZScale);
		 	D3D_DrawWaterPatch(x+MeshXScale, y, z+MeshZScale);
		 	D3D_DrawWaterPatch(x+MeshXScale*2, y, z+MeshZScale);
		 	D3D_DrawWaterPatch(x+MeshXScale*3, y, z+MeshZScale);
		}
	}
	#if 0
	else if ( (!__stricmp(LevelName,"e3demo")) || (!__stricmp(LevelName,"e3demosp")) )
	{
		int drawOctagonPool = -1;
		int drawFMV = -1;
		int drawPredatorFMV = -1;
		int drawSwirlyFMV = -1;
		int drawSwirlyFMV2 = -1;
		int drawSwirlyFMV3 = -1;
		while(numOfObjects)
		{
			DISPLAYBLOCK *objectPtr = OnScreenBlockList[--numOfObjects];
			MODULE *modulePtr = objectPtr->ObMyModule;

			/* if it's a module, which isn't inside another module */
			if (modulePtr && modulePtr->name)
			{
				if(!__stricmp(modulePtr->name,"water1"))
				{
					drawOctagonPool = modulePtr->m_index;
				}
				else if(!__stricmp(modulePtr->name,"marine01b"))
				{
					drawFMV = modulePtr->m_index;
				}
				else if(!_stricmp(modulePtr->name,"predator01"))
				{
					drawPredatorFMV = modulePtr->m_index;
				}
				else if(!_stricmp(modulePtr->name,"toptopgr01"))
				{
					drawSwirlyFMV = modulePtr->m_index;
				}
				else if(!_stricmp(modulePtr->name,"grille04"))
				{
					drawSwirlyFMV2 = modulePtr->m_index;
				}
				#if 0
				else if(!_stricmp(modulePtr->name,"marine05"))
				{
					drawSwirlyFMV3 = modulePtr->m_index;
				}
				#endif
			}
		}	
		#if FMV_ON
//		UpdateFMVTextures(3);
		

		if (drawFMV!=-1)
		{
			DECAL fmvDecal =
			{
				DECAL_FMV,
			};
			fmvDecal.ModuleIndex = drawFMV;
			fmvDecal.UOffset = 0;

			UpdateFMVTextures(4);
			
			for (int z=0; z<6; z++)
			{
				for (int y=0; y<3; y++)
				{	
					fmvDecal.Vertices[0].vx = -149;
					fmvDecal.Vertices[1].vx = -149;
					fmvDecal.Vertices[2].vx = -149;
					fmvDecal.Vertices[3].vx = -149;

					fmvDecal.Vertices[0].vy = -3254+y*744;
					fmvDecal.Vertices[1].vy = -3254+y*744;
					fmvDecal.Vertices[2].vy = -3254+y*744+744;
					fmvDecal.Vertices[3].vy = -3254+y*744+744;

					fmvDecal.Vertices[0].vz = 49440+z*993;
					fmvDecal.Vertices[1].vz = 49440+z*993+993;
					fmvDecal.Vertices[2].vz = 49440+z*993+993;
					fmvDecal.Vertices[3].vz = 49440+z*993;
					fmvDecal.Centre.vx = ((z+y)%3)+1;
					RenderDecal(&fmvDecal);
				}
			}
		}
		if (drawPredatorFMV!=-1)
		{
			DECAL fmvDecal =
			{
				DECAL_FMV,
			};
			fmvDecal.ModuleIndex = drawPredatorFMV;
			fmvDecal.UOffset = 0;

			UpdateFMVTextures(4);
			
			for (int z=0; z<12; z++)
			{
				for (int y=0; y<7; y++)
				{	
					fmvDecal.Vertices[0].vx = -7164;
					fmvDecal.Vertices[1].vx = -7164;
					fmvDecal.Vertices[2].vx = -7164;
					fmvDecal.Vertices[3].vx = -7164;

					fmvDecal.Vertices[0].vy = -20360+y*362;
					fmvDecal.Vertices[1].vy = -20360+y*362;
					fmvDecal.Vertices[2].vy = -20360+y*362+362;
					fmvDecal.Vertices[3].vy = -20360+y*362+362;

					fmvDecal.Vertices[0].vz = 1271+z*483+483;
					fmvDecal.Vertices[1].vz = 1271+z*483;
					fmvDecal.Vertices[2].vz = 1271+z*483;
					fmvDecal.Vertices[3].vz = 1271+z*483+483;
					fmvDecal.Centre.vx = (z+y)%3;
					RenderDecal(&fmvDecal);
				}
			}
		}
		
		#endif
		
		if (drawSwirlyFMV!=-1)
		{
			UpdateFMVTextures(1);
			D3D_DrawSwirlyFMV(30000,-12500,0);
		}
		if (drawSwirlyFMV2!=-1)
		{
			UpdateFMVTextures(1);
			D3D_DrawSwirlyFMV(2605,-6267-2000,17394-3200);
		}
		
		if (drawSwirlyFMV3!=-1)
		{
//			UpdateFMVTextures(1);
			D3D_DrawSwirlyFMV(5117,3456-3000,52710-2000);
		}
		if (drawOctagonPool!=-1)
		{
			#if FMV_ON
			UpdateFMVTextures(1);
			
			MeshXScale = (3000);
			MeshZScale = (4000);
			D3D_DrawFMVOnWater(-1000,3400,22000);
			{
				DECAL fmvDecal =
				{
					DECAL_FMV,
					{
					{0,-2500,29000},
					{2000,-2500,29000},
					{2000,-2500+750*2,29000},
					{0,-2500+750*2,29000}
					},
					0
				};
				fmvDecal.ModuleIndex = drawOctagonPool;
				fmvDecal.Centre.vx = 0;
				fmvDecal.UOffset = 0;

				RenderDecal(&fmvDecal);
			}
			#endif

			int highDetailRequired = 1;
			int x = 1023;
			int y = 3400;
			int z = 27536;
			
			{
				int dx = Player->ObWorld.vx - x;
				if (dx< -8000 || dx > 8000)
				{
					highDetailRequired = 0;
				}
				else
				{
					int dz = Player->ObWorld.vz - z;
					if (dz< -8000 || dz > 8000)
					{
						highDetailRequired = 0;
					}
				}
			}			
			MeshXScale = 7700;
			MeshZScale = 7700;
			{
				extern void CheckForObjectsInWater(int minX, int maxX, int minZ, int maxZ, int averageY);
				CheckForObjectsInWater(x-MeshXScale, x+MeshXScale, z-MeshZScale, z+MeshZScale, y);
			}
			
			MeshXScale /=15;
			MeshZScale /=15;
			
			// Turn OFF texturing if it is on...
			D3DTEXTUREHANDLE TextureHandle = NULL;
			if (CurrTextureHandle != TextureHandle)
			{
				OP_STATE_RENDER(1, ExecBufInstPtr);
				STATE_DATA(D3DRENDERSTATE_TEXTUREHANDLE, TextureHandle, ExecBufInstPtr);
				CurrTextureHandle = TextureHandle;
			}	 
			CheckTranslucencyModeIsCorrect(TRANSLUCENCY_NORMAL);
			if (NumVertices)
			{
			   WriteEndCodeToExecuteBuffer();
		  	   UnlockExecuteBufferAndPrepareForUse();
			   ExecuteBuffer();
		  	   LockExecuteBuffer();
			}
			if (highDetailRequired)
			{
				MeshXScale /= 2;
				MeshZScale /= 2;
				D3D_DrawWaterOctagonPatch(x,y,z,0,0);
				D3D_DrawWaterOctagonPatch(x,y,z,15,0);
				D3D_DrawWaterOctagonPatch(x,y,z,0,15);
				D3D_DrawWaterOctagonPatch(x,y,z,15,15);
				MeshXScale = -MeshXScale;
				D3D_DrawWaterOctagonPatch(x,y,z,0,0);
				D3D_DrawWaterOctagonPatch(x,y,z,15,0);
				D3D_DrawWaterOctagonPatch(x,y,z,0,15);
				D3D_DrawWaterOctagonPatch(x,y,z,15,15);
				MeshZScale = -MeshZScale;
				D3D_DrawWaterOctagonPatch(x,y,z,0,0);
				D3D_DrawWaterOctagonPatch(x,y,z,15,0);
				D3D_DrawWaterOctagonPatch(x,y,z,0,15);
				D3D_DrawWaterOctagonPatch(x,y,z,15,15);
				MeshXScale = -MeshXScale;
				D3D_DrawWaterOctagonPatch(x,y,z,0,0);
				D3D_DrawWaterOctagonPatch(x,y,z,15,0);
				D3D_DrawWaterOctagonPatch(x,y,z,0,15);
				D3D_DrawWaterOctagonPatch(x,y,z,15,15);
			}
			else
			{
				D3D_DrawWaterOctagonPatch(x,y,z,0,0);
				MeshXScale = -MeshXScale;
				D3D_DrawWaterOctagonPatch(x,y,z,0,0);
				MeshZScale = -MeshZScale;
				D3D_DrawWaterOctagonPatch(x,y,z,0,0);
				MeshXScale = -MeshXScale;
				D3D_DrawWaterOctagonPatch(x,y,z,0,0);
			}

		}
	}
	#endif
	else if (!_stricmp(LevelName,"hangar"))
	{
	   	#if FMV_ON
		#if WIBBLY_FMV_ON
		UpdateFMVTextures(1);
	   	D3D_DrawFMV(FmvPosition.vx,FmvPosition.vy,FmvPosition.vz);
		#endif
		#endif
		#if 0
		{
			VECTORCH v = {49937,-4000,-37709};		// hangar
			D3D_DrawCable(&v);
		}
		#endif
	}
	else if (!_stricmp(LevelName,"invasion_a"))
	{
		char drawWater = 0;
		char drawEndWater = 0;

		while(numOfObjects)
		{
			DISPLAYBLOCK *objectPtr = OnScreenBlockList[--numOfObjects];
			MODULE *modulePtr = objectPtr->ObMyModule;

			/* if it's a module, which isn't inside another module */
			if (modulePtr && modulePtr->name)
			{
				if( (!strcmp(modulePtr->name,"hivepool"))
				  ||(!strcmp(modulePtr->name,"hivepool04")))
				{
					drawWater = 1;
					break;
				}
				else
				{
					if(!strcmp(modulePtr->name,"shaftbot"))
					{
						drawEndWater = 1;
					}
					if((!_stricmp(modulePtr->name,"shaft01"))
					 ||(!_stricmp(modulePtr->name,"shaft02"))
					 ||(!_stricmp(modulePtr->name,"shaft03"))
					 ||(!_stricmp(modulePtr->name,"shaft04"))
					 ||(!_stricmp(modulePtr->name,"shaft05"))
					 ||(!_stricmp(modulePtr->name,"shaft06")))
					{
						extern void HandleRainShaft(MODULE *modulePtr, int bottomY, int topY, int numberOfRaindrops);
						HandleRainShaft(modulePtr, -11726,-107080,10);
						drawEndWater = 1;
						break;
					}
				}
			}

		}	

		if (drawWater)
		{
			int x = 20767;
			int y = -36000+200;
			int z = 30238;
			MeshXScale = (36353-20767);
			MeshZScale = (41927-30238);
			{
				extern void CheckForObjectsInWater(int minX, int maxX, int minZ, int maxZ, int averageY);
				CheckForObjectsInWater(x, x+MeshXScale, z, z+MeshZScale, y);
			}

			WaterXOrigin=x;
			WaterZOrigin=z;
			WaterUScale = 4.0f/(float)MeshXScale;
			WaterVScale = 4.0f/(float)MeshZScale;
		 	MeshXScale/=4;
		 	MeshZScale/=2;
			
			// Turn OFF texturing if it is on...
			D3DTEXTUREHANDLE TextureHandle = (D3DTEXTUREHANDLE)ImageHeaderArray[ChromeImageNumber].D3DHandle;
			if (CurrTextureHandle != TextureHandle)
			{
				OP_STATE_RENDER(1, ExecBufInstPtr);
				STATE_DATA(D3DRENDERSTATE_TEXTUREHANDLE, TextureHandle, ExecBufInstPtr);
				CurrTextureHandle = TextureHandle;
			}	 
			CheckTranslucencyModeIsCorrect(TRANSLUCENCY_NORMAL);
			if (NumVertices)
			{
			   WriteEndCodeToExecuteBuffer();
		  	   UnlockExecuteBufferAndPrepareForUse();
			   ExecuteBuffer();
		  	   LockExecuteBuffer();
			}
		 	D3D_DrawWaterPatch(x, y, z);
		 	D3D_DrawWaterPatch(x+MeshXScale, y, z);
		 	D3D_DrawWaterPatch(x+MeshXScale*2, y, z);
		 	D3D_DrawWaterPatch(x+MeshXScale*3, y, z);
		 	D3D_DrawWaterPatch(x, y, z+MeshZScale);
		 	D3D_DrawWaterPatch(x+MeshXScale, y, z+MeshZScale);
		 	D3D_DrawWaterPatch(x+MeshXScale*2, y, z+MeshZScale);
		 	D3D_DrawWaterPatch(x+MeshXScale*3, y, z+MeshZScale);
		}
		else if (drawEndWater)
		{
			int x = -15471;
			int y = -11720-500;
			int z = -55875;
			MeshXScale = (15471-1800);
			MeshZScale = (55875-36392);
			{
				extern void CheckForObjectsInWater(int minX, int maxX, int minZ, int maxZ, int averageY);
				CheckForObjectsInWater(x, x+MeshXScale, z, z+MeshZScale, y);
			}
			WaterXOrigin=x;
			WaterZOrigin=z;
			WaterUScale = 4.0f/(float)(MeshXScale+1800-3782);
			WaterVScale = 4.0f/(float)MeshZScale;
		 	MeshXScale/=4;
		 	MeshZScale/=2;
			
			// Turn OFF texturing if it is on...
			D3DTEXTUREHANDLE TextureHandle = (D3DTEXTUREHANDLE)ImageHeaderArray[WaterShaftImageNumber].D3DHandle;
			if (CurrTextureHandle != TextureHandle)
			{
				OP_STATE_RENDER(1, ExecBufInstPtr);
				STATE_DATA(D3DRENDERSTATE_TEXTUREHANDLE, TextureHandle, ExecBufInstPtr);
				CurrTextureHandle = TextureHandle;
			}	 
			CheckTranslucencyModeIsCorrect(TRANSLUCENCY_NORMAL);
			if (NumVertices)
			{
			   WriteEndCodeToExecuteBuffer();
		  	   UnlockExecuteBufferAndPrepareForUse();
			   ExecuteBuffer();
		  	   LockExecuteBuffer();
			}
		 	D3D_DrawWaterPatch(x, y, z);
		 	D3D_DrawWaterPatch(x+MeshXScale, y, z);
		 	D3D_DrawWaterPatch(x+MeshXScale*2, y, z);
		 	D3D_DrawWaterPatch(x+MeshXScale*3, y, z);
		 	D3D_DrawWaterPatch(x, y, z+MeshZScale);
		 	D3D_DrawWaterPatch(x+MeshXScale, y, z+MeshZScale);
		 	D3D_DrawWaterPatch(x+MeshXScale*2, y, z+MeshZScale);
		 	D3D_DrawWaterPatch(x+MeshXScale*3, y, z+MeshZScale);
		}
	}
	#if 1
	else if (!_stricmp(LevelName,"derelict"))
	{
		char drawMirrorSurfaces = 0;
		char drawWater = 0;

		while(numOfObjects)
		{
			DISPLAYBLOCK *objectPtr = OnScreenBlockList[--numOfObjects];
			MODULE *modulePtr = objectPtr->ObMyModule;

			/* if it's a module, which isn't inside another module */
			if (modulePtr && modulePtr->name)
			{
			  	if( (!_stricmp(modulePtr->name,"start-en01"))
			  	  ||(!_stricmp(modulePtr->name,"start")))
				{
					drawMirrorSurfaces = 1;
				}
				else if (!_stricmp(modulePtr->name,"water-01"))
				{
					extern void HandleRainShaft(MODULE *modulePtr, int bottomY, int topY, int numberOfRaindrops);
					drawWater = 1;
					HandleRainShaft(modulePtr, 32000, 0, 16);
				}
			}
		}	

		if (drawMirrorSurfaces)
		{
			extern void RenderMirrorSurface(void);
			extern void RenderMirrorSurface2(void);
			extern void RenderParticlesInMirror(void);
			RenderParticlesInMirror();
			RenderMirrorSurface();
			RenderMirrorSurface2();
		}
		if (drawWater)
		{
			int x = -102799;
			int y = 32000;
			int z = -200964;
			MeshXScale = (102799-87216);
			MeshZScale = (200964-180986);
			{
				extern void CheckForObjectsInWater(int minX, int maxX, int minZ, int maxZ, int averageY);
				CheckForObjectsInWater(x, x+MeshXScale, z, z+MeshZScale, y);
			}

			WaterXOrigin=x;
			WaterZOrigin=z;
			WaterUScale = 4.0f/(float)MeshXScale;
			WaterVScale = 4.0f/(float)MeshZScale;
		 	MeshXScale/=2;
		 	MeshZScale/=2;
			
			// Turn OFF texturing if it is on...
			D3DTEXTUREHANDLE TextureHandle = (D3DTEXTUREHANDLE)ImageHeaderArray[ChromeImageNumber].D3DHandle;
			if (CurrTextureHandle != TextureHandle)
			{
				OP_STATE_RENDER(1, ExecBufInstPtr);
				STATE_DATA(D3DRENDERSTATE_TEXTUREHANDLE, TextureHandle, ExecBufInstPtr);
				CurrTextureHandle = TextureHandle;
			}	 
			CheckTranslucencyModeIsCorrect(TRANSLUCENCY_NORMAL);
			if (NumVertices)
			{
			   WriteEndCodeToExecuteBuffer();
		  	   UnlockExecuteBufferAndPrepareForUse();
			   ExecuteBuffer();
		  	   LockExecuteBuffer();
			}
		 	D3D_DrawWaterPatch(x, y, z);
		 	D3D_DrawWaterPatch(x+MeshXScale, y, z);
		 	D3D_DrawWaterPatch(x, y, z+MeshZScale);
		 	D3D_DrawWaterPatch(x+MeshXScale, y, z+MeshZScale);
		}

	}
	#endif
	else if (!_stricmp(LevelName,"genshd1"))
	{
		char drawWater = 0;

		while(numOfObjects)
		{
			DISPLAYBLOCK *objectPtr = OnScreenBlockList[--numOfObjects];
			MODULE *modulePtr = objectPtr->ObMyModule;

			/* if it's a module, which isn't inside another module */
			if (modulePtr && modulePtr->name)
			{
				if( (!_stricmp(modulePtr->name,"largespace"))
				  ||(!_stricmp(modulePtr->name,"proc13"))
				  ||(!_stricmp(modulePtr->name,"trench01"))
				  ||(!_stricmp(modulePtr->name,"trench02"))
				  ||(!_stricmp(modulePtr->name,"trench03"))
				  ||(!_stricmp(modulePtr->name,"trench04"))
				  ||(!_stricmp(modulePtr->name,"trench05"))
				  ||(!_stricmp(modulePtr->name,"trench06"))
				  ||(!_stricmp(modulePtr->name,"trench07"))
				  ||(!_stricmp(modulePtr->name,"trench08"))
				  ||(!_stricmp(modulePtr->name,"trench09")))
				{
					extern void HandleRain(int numberOfRaindrops);
					HandleRain(999);
					break;
				}
			}

		}	
	}
}

void D3D_DrawWaterTest(MODULE *testModulePtr)
{
	extern char LevelName[];
	if (!strcmp(LevelName,"genshd1"))
	{
		extern DISPLAYBLOCK *Player;

//		DISPLAYBLOCK *objectPtr = OnScreenBlockList[numOfObjects];
		MODULE *modulePtr = testModulePtr;//objectPtr->ObMyModule;
#if 0
		if (testModulePtr && testModulePtr->name)
		if(!strcmp(testModulePtr->name,"LargeSpace"))
		{
			extern void HandleRain(int numberOfRaindrops);
			HandleRain(999);
		}
#endif
		if (modulePtr && modulePtr->name)
		{
			if (!strcmp(modulePtr->name,"05"))
			{
				int y = modulePtr->m_maxy+modulePtr->m_world.vy-500;
		   		int x = modulePtr->m_minx+modulePtr->m_world.vx;
		   		int z = modulePtr->m_minz+modulePtr->m_world.vz;
				MeshXScale = (7791 - -7794);
				MeshZScale = (23378 - 7793);
				{
					extern void CheckForObjectsInWater(int minX, int maxX, int minZ, int maxZ, int averageY);
					CheckForObjectsInWater(x, x+MeshXScale, z, z+MeshZScale, y);
				}
				D3DTEXTUREHANDLE TextureHandle = (D3DTEXTUREHANDLE)ImageHeaderArray[WaterShaftImageNumber].D3DHandle;
				if (CurrTextureHandle != TextureHandle)
				{
					OP_STATE_RENDER(1, ExecBufInstPtr);
					STATE_DATA(D3DRENDERSTATE_TEXTUREHANDLE, TextureHandle, ExecBufInstPtr);
					CurrTextureHandle = TextureHandle;
				}	 
				CheckTranslucencyModeIsCorrect(TRANSLUCENCY_NORMAL);
				if (NumVertices)
				{
				   WriteEndCodeToExecuteBuffer();
			  	   UnlockExecuteBufferAndPrepareForUse();
				   ExecuteBuffer();
			  	   LockExecuteBuffer();
				}
				WaterXOrigin=x;
				WaterZOrigin=z;
				WaterUScale = 4.0f/(float)(MeshXScale);
				WaterVScale = 4.0f/(float)MeshZScale;
				#if 1
				MeshXScale/=2;
				MeshZScale/=2;
				D3D_DrawWaterPatch(x, y, z);
				D3D_DrawWaterPatch(x+MeshXScale, y, z);
				D3D_DrawWaterPatch(x, y, z+MeshZScale);
				D3D_DrawWaterPatch(x+MeshXScale, y, z+MeshZScale);

				extern void HandleRainShaft(MODULE *modulePtr, int bottomY, int topY, int numberOfRaindrops);
				HandleRainShaft(modulePtr, y,-21000,1);
				#else
				MeshXScale/=4;
				MeshZScale/=4;
				D3D_DrawWaterPatch(x, y, z);
				D3D_DrawWaterPatch(x, y, z+MeshZScale);
				D3D_DrawWaterPatch(x, y, z+MeshZScale*2);
				D3D_DrawWaterPatch(x, y, z+MeshZScale*3);
				D3D_DrawWaterPatch(x+MeshXScale, y, z);
				D3D_DrawWaterPatch(x+MeshXScale, y, z+MeshZScale);
				D3D_DrawWaterPatch(x+MeshXScale, y, z+MeshZScale*2);
				D3D_DrawWaterPatch(x+MeshXScale, y, z+MeshZScale*3);
				D3D_DrawWaterPatch(x+MeshXScale*2, y, z);
				D3D_DrawWaterPatch(x+MeshXScale*2, y, z+MeshZScale);
				D3D_DrawWaterPatch(x+MeshXScale*2, y, z+MeshZScale*2);
				D3D_DrawWaterPatch(x+MeshXScale*2, y, z+MeshZScale*3);
				D3D_DrawWaterPatch(x+MeshXScale*3, y, z);
				D3D_DrawWaterPatch(x+MeshXScale*3, y, z+MeshZScale);
				D3D_DrawWaterPatch(x+MeshXScale*3, y, z+MeshZScale*2);
				D3D_DrawWaterPatch(x+MeshXScale*3, y, z+MeshZScale*3);
				HandleRainDrops(modulePtr,2);
				#endif
			}
		}
	}
	#if 0
	else if ( (!_stricmp(LevelName,"e3demo")) || (!_stricmp(LevelName,"e3demosp")) )
	{
		if (testModulePtr && testModulePtr->name)
		{
			#if 0
			if(!_stricmp(testModulePtr->name,"watermid"))
			{
				DECAL fmvDecal =
				{
					DECAL_FMV,
					{
					{0,-2500,29000},
					{2000,-2500,29000},
					{2000,-2500+750*2,29000},
					{0,-2500+750*2,29000}
					},
					0
				};
				fmvDecal.ModuleIndex = testModulePtr->m_index;
				fmvDecal.Centre.vx = 0;
				fmvDecal.UOffset = 0;

				RenderDecal(&fmvDecal);
			}
			#endif
			if(!_stricmp(testModulePtr->name,"lowlowlo03"))
			{
				VECTORCH position = {6894,469,-13203};
				VECTORCH disp = position;
				int i,d;

				disp.vx -= Player->ObWorld.vx;
				disp.vy -= Player->ObWorld.vy;
				disp.vz -= Player->ObWorld.vz;
				d = ONE_FIXED - Approximate3dMagnitude(&disp)*2;
				if (d<0) d = 0;

				i = MUL_FIXED(10,d);
				while(i--)
				{
					VECTORCH velocity;
					velocity.vx = ((FastRandom()&1023) - 512);
					velocity.vy = ((FastRandom()&1023) - 512)+2000;
					velocity.vz = (1000+(FastRandom()&255))*2;
					MakeParticle(&(position),&(velocity),PARTICLE_STEAM);
				}
			}
		}
	}
	#endif

}

VECTORCH MeshVertex[256];
#define TEXTURE_WATER 0

VECTORCH MeshWorldVertex[256];
unsigned int MeshVertexColour[256];
unsigned int MeshVertexSpecular[256];
char MeshVertexOutcode[256];
void D3D_DrawWaterPatch(int xOrigin, int yOrigin, int zOrigin)
{
	int i=0;
	int x;
	for (x=0; x<16; x++)
	{
		int z;
		for(z=0; z<16; z++)
		{
			VECTORCH *point = &MeshVertex[i];
			
			point->vx = xOrigin+(x*MeshXScale)/15;
			point->vz = zOrigin+(z*MeshZScale)/15;


			int offset=0;

		 #if 1
			/* basic noise ripples */
//		 	offset = MUL_FIXED(32,GetSin(  (point->vx+point->vz+CloakingPhase)&4095 ) );
//		 	offset += MUL_FIXED(16,GetSin(  (point->vx-point->vz*2+CloakingPhase/2)&4095 ) );

			{
 				offset += EffectOfRipples(point);
			}
		#endif
	//		if (offset>450) offset = 450;
	//		if (offset<-450) offset = -450;
			point->vy = yOrigin+offset;

			#if 0
			MeshVertexColour[i] = LightSourceWaterPoint(point,offset);
			#else
			{
				int alpha = 128-offset/4;
		//		if (alpha>255) alpha = 255;
		//		if (alpha<128) alpha = 128;
				switch (CurrentVisionMode)
				{
					default:
					case VISION_MODE_NORMAL:
					{
//						MeshVertexColour[i] = RGBALIGHT_MAKE(10,51,28,alpha);
						MeshVertexColour[i] = RGBALIGHT_MAKE(255,255,255,alpha);
						#if 0
						#if 1
						VECTORCH pos = {24087,yOrigin,39165};
						int c = (8191-VectorDistance(&pos,point));
						if (c<0) c=0;
						else
						{
							int s = GetSin((CloakingPhase/2)&4095);
							s = MUL_FIXED(s,s)/64;
							c = MUL_FIXED(s,c);
						}
						MeshVertexSpecular[i] = (c<<16)+(((c/4)<<8)&0xff00) + (c/4);
						#else 
						if (!(FastRandom()&1023))
						{
							MeshVertexSpecular[i] = 0xc04040;
						}
						else
						{
							MeshVertexSpecular[i] = 0;
						}
						#endif
						#endif
						break;
					}
					case VISION_MODE_IMAGEINTENSIFIER:
					{
						MeshVertexColour[i] = RGBALIGHT_MAKE(0,51,0,alpha);
						break;
					}
					case VISION_MODE_PRED_THERMAL:
					case VISION_MODE_PRED_SEEALIENS:
					case VISION_MODE_PRED_SEEPREDTECH:
					{
						MeshVertexColour[i] = RGBALIGHT_MAKE(0,0,28,alpha);
					  	break;
					}
				}

			}
			#endif

			#if 1
			MeshWorldVertex[i].vx = ((point->vx-WaterXOrigin)/4+MUL_FIXED(GetSin((point->vy*16)&4095),128));			
			MeshWorldVertex[i].vy = ((point->vz-WaterZOrigin)/4+MUL_FIXED(GetSin((point->vy*16+200)&4095),128));			
			#endif
			
			#if 1
			TranslatePointIntoViewspace(point);
			#else
			point->vx -= Global_VDB_Ptr->VDB_World.vx;
			point->vy -= Global_VDB_Ptr->VDB_World.vy;
			point->vz -= Global_VDB_Ptr->VDB_World.vz;
			RotateVector(point,&(Global_VDB_Ptr->VDB_Mat));
			point->vy = MUL_FIXED(point->vy,87381);

			#endif
			/* is particle within normal view frustrum ? */
			if(AvP.PlayerType==I_Alien)	/* wide frustrum */
			{
				if(( (-point->vx <= point->vz*2)
		   			&&(point->vx <= point->vz*2)
					&&(-point->vy <= point->vz*2)
					&&(point->vy <= point->vz*2) ))
				{
					MeshVertexOutcode[i]=1;
				}
				else
				{
					MeshVertexOutcode[i]=0;
				}
			}
			else
			{
				if(( (-point->vx <= point->vz)
		   			&&(point->vx <= point->vz)
					&&(-point->vy <= point->vz)
					&&(point->vy <= point->vz) ))
				{
					MeshVertexOutcode[i]=1;
				}
				else
				{
					MeshVertexOutcode[i]=0;
				}
			}

			i++;
		}
	}

	if ((MeshVertexOutcode[0]&&MeshVertexOutcode[15]&&MeshVertexOutcode[240]&&MeshVertexOutcode[255]))
	{
		D3D_DrawMoltenMetalMesh_Unclipped();
//		D3D_DrawWaterMesh_Unclipped();
	}	
	else
//	else if (MeshVertexOutcode[0]||MeshVertexOutcode[15]||MeshVertexOutcode[240]||MeshVertexOutcode[255])
	{
		D3D_DrawMoltenMetalMesh_Clipped();
//		D3D_DrawWaterMesh_Clipped();
	}
		
	
}

void D3D_DrawWaterMesh_Unclipped(void)
{
	float ZNear = (float) (Global_VDB_Ptr->VDB_ClipZ * GlobalScale);

	/* OUTPUT VERTICES TO EXECUTE BUFFER */
	{
		D3DTLVERTEX *vertexPtr = &((LPD3DTLVERTEX)ExecuteBufferDataArea)[NumVertices];
		VECTORCH *point = MeshVertex;
		#if TEXTURE_WATER
		VECTORCH *pointWS = MeshWorldVertex;
		#endif
		int i;
		for (i=0; i<256; i++)
		{

			if (point->vz<=1) point->vz = 1;
			int x = (point->vx*(Global_VDB_Ptr->VDB_ProjX))/point->vz+Global_VDB_Ptr->VDB_CentreX;
			int y = (point->vy*(Global_VDB_Ptr->VDB_ProjY))/point->vz+Global_VDB_Ptr->VDB_CentreY;
  //			textprint("%d, %d\n",x,y);
			#if 1
			{
				if (x<Global_VDB_Ptr->VDB_ClipLeft)
				{
					x=Global_VDB_Ptr->VDB_ClipLeft;
				}	
				else if (x>Global_VDB_Ptr->VDB_ClipRight)
				{
					x=Global_VDB_Ptr->VDB_ClipRight;	
				}
				
				vertexPtr->sx=x;
			}
			{
				if (y<Global_VDB_Ptr->VDB_ClipUp)
				{
					y=Global_VDB_Ptr->VDB_ClipUp;
				}
				else if (y>Global_VDB_Ptr->VDB_ClipDown)
				{
					y=Global_VDB_Ptr->VDB_ClipDown;	
				}
				vertexPtr->sy=y;
			}
			#else
			vertexPtr->sx=x;
			vertexPtr->sy=y;
			#endif
			#if FOG_ON
			{
				int fog = (point->vz)/FOG_SCALE;
				if (fog<0) fog=0;
			 	if (fog>254) fog=254;
				fog=255-fog;
			   	vertexPtr->specular=RGBALIGHT_MAKE(0,0,0,fog);
			}
			#endif
			point->vz+=HeadUpDisplayZOffset;
		  	float oneOverZ = ((float)(point->vz)-ZNear)/(float)(point->vz);
		  //vertexPtr->color = RGBALIGHT_MAKE(66,70,0,127+(FastRandom()&63));
			vertexPtr->color = MeshVertexColour[i];
			vertexPtr->sz = oneOverZ;
			#if TEXTURE_WATER
			vertexPtr->tu = pointWS->vx/128.0;
			vertexPtr->tv =	pointWS->vz/128.0;
			#endif


			NumVertices++;
			vertexPtr++;
			point++;
			#if TEXTURE_WATER
			pointWS++;
			#endif
		}
	}
 //	textprint("numvertices %d\n",NumVertices);
    
    
    /*
     * Make sure that the triangle data (not OP) will be QWORD aligned
     */
	if (QWORD_ALIGNED(ExecBufInstPtr))
    {
        OP_NOP(ExecBufInstPtr);
    }

  	OP_TRIANGLE_LIST(450, ExecBufInstPtr);
	/* CONSTRUCT POLYS */
	{
		int x;
		for (x=0; x<15; x++)
		{
			int y;
			for(y=0; y<15; y++)
			{
				OUTPUT_TRIANGLE(0+x+(16*y),1+x+(16*y),16+x+(16*y), 256);
				OUTPUT_TRIANGLE(1+x+(16*y),17+x+(16*y),16+x+(16*y), 256);
			}
		}
	}
	#if 1
	{
	   WriteEndCodeToExecuteBuffer();
  	   UnlockExecuteBufferAndPrepareForUse();
	   ExecuteBuffer();
  	   LockExecuteBuffer();
	}
	#endif
}
void D3D_DrawWaterMesh_Clipped(void)
{
	float ZNear = (float) (Global_VDB_Ptr->VDB_ClipZ * GlobalScale);

	/* OUTPUT VERTICES TO EXECUTE BUFFER */
	{
		D3DTLVERTEX *vertexPtr = &((LPD3DTLVERTEX)ExecuteBufferDataArea)[NumVertices];
		VECTORCH *point = MeshVertex;
		#if TEXTURE_WATER
		VECTORCH *pointWS = MeshWorldVertex;
		#endif
		int i;
		for (i=0; i<256; i++)
		{
			{
				if (point->vz<=1) point->vz = 1;
				int x = (point->vx*(Global_VDB_Ptr->VDB_ProjX))/point->vz+Global_VDB_Ptr->VDB_CentreX;
				int y = (point->vy*(Global_VDB_Ptr->VDB_ProjY))/point->vz+Global_VDB_Ptr->VDB_CentreY;
				#if 1
				{
					if (x<Global_VDB_Ptr->VDB_ClipLeft)
					{
						x=Global_VDB_Ptr->VDB_ClipLeft;
					}	
					else if (x>Global_VDB_Ptr->VDB_ClipRight)
					{
						x=Global_VDB_Ptr->VDB_ClipRight;	
					}
					
					vertexPtr->sx=x;
				}
				{
					if (y<Global_VDB_Ptr->VDB_ClipUp)
					{
						y=Global_VDB_Ptr->VDB_ClipUp;
					}
					else if (y>Global_VDB_Ptr->VDB_ClipDown)
					{
						y=Global_VDB_Ptr->VDB_ClipDown;	
					}
					vertexPtr->sy=y;
				}
				#else
				vertexPtr->sx=x;
				vertexPtr->sy=y;
				#endif
				#if FOG_ON
				{
					int fog = ((point->vz)/FOG_SCALE);
					if (fog<0) fog=0;
				 	if (fog>254) fog=254;
					fog=255-fog;
				   	vertexPtr->specular=RGBALIGHT_MAKE(0,0,0,fog);
				}
				#endif
				#if TEXTURE_WATER
				vertexPtr->tu = pointWS->vx/128.0;
				vertexPtr->tv =	pointWS->vz/128.0;
				#endif
				point->vz+=HeadUpDisplayZOffset;
			  	float oneOverZ = ((float)(point->vz)-ZNear)/(float)(point->vz);
			  //	vertexPtr->color = RGBALIGHT_MAKE(66,70,0,127+(FastRandom()&63));
				vertexPtr->color = MeshVertexColour[i];
				vertexPtr->sz = oneOverZ;
			}
			NumVertices++;
			vertexPtr++;
			point++;
			#if TEXTURE_WATER
			pointWS++;
			#endif
		}
	}
//	textprint("numvertices %d\n",NumVertices);
	/* CONSTRUCT POLYS */
	{
		int x;
		for (x=0; x<15; x++)
		{
			int y;
			for(y=0; y<15; y++)
			{
				#if 1
				int p1 = 0+x+(16*y);
				int p2 = 1+x+(16*y);
				int p3 = 16+x+(16*y);
				int p4 = 17+x+(16*y);

				if (MeshVertexOutcode[p1]||MeshVertexOutcode[p2]||MeshVertexOutcode[p3])
				{
					OP_TRIANGLE_LIST(1, ExecBufInstPtr);
					OUTPUT_TRIANGLE(p1,p2,p3, 256);
				}
				if (MeshVertexOutcode[p2]||MeshVertexOutcode[p3]||MeshVertexOutcode[p4])
				{
					OP_TRIANGLE_LIST(1, ExecBufInstPtr);
					OUTPUT_TRIANGLE(p2,p4,p3, 256);
				}	
				#else
				int p2 = 1+x+(16*y);
				int p3 = 16+x+(16*y);

				if (MeshVertexOutcode[p2]&&MeshVertexOutcode[p3])
				{
					int p1 = 0+x+(16*y);
					int p4 = 17+x+(16*y);
					if (MeshVertexOutcode[p1])
					{
						OP_TRIANGLE_LIST(1, ExecBufInstPtr);
						OUTPUT_TRIANGLE(p1,p2,p3, 256);
					}
					if (MeshVertexOutcode[p4])
					{
						OP_TRIANGLE_LIST(1, ExecBufInstPtr);
						OUTPUT_TRIANGLE(p2,p4,p3, 256);
					}
				}	
				#endif				
			}
		}
	}
	#if 1
	{
	   WriteEndCodeToExecuteBuffer();
  	   UnlockExecuteBufferAndPrepareForUse();
	   ExecuteBuffer();
  	   LockExecuteBuffer();
	}
	#endif
}


int LightSourceWaterPoint(VECTORCH *pointPtr,int offset)
{
	// this needs a rewrite...
	// make a list of lights which will affect some part of the mesh
	// and go throught that for each point.

//	int intensity=0;
   	int redI=0,greenI=0,blueI=0;
	
	#if 0
	int alpha = 207-offset;
	if (alpha>255) alpha = 255;
	if (alpha<160) alpha = 160;

	return RGBALIGHT_MAKE(128,128,255,alpha);
	#else

	DISPLAYBLOCK **activeBlockListPtr = ActiveBlockList;
	for(int i = NumActiveBlocks; i!=0; i--)
	{
		DISPLAYBLOCK *dispPtr = *activeBlockListPtr++;

		if(dispPtr->ObNumLights)
		{
			for(int j = 0; j < dispPtr->ObNumLights; j++)
			{
				LIGHTBLOCK *lptr = dispPtr->ObLights[j];

				VECTORCH disp = lptr->LightWorld;
				disp.vx -= pointPtr->vx;
				disp.vy -= pointPtr->vy;
				disp.vz -= pointPtr->vz;
				
				int dist = Approximate3dMagnitude(&disp);
				
				if (dist<lptr->LightRange)
				{
					int brightness = MUL_FIXED(lptr->BrightnessOverRange,lptr->LightRange-dist);
					redI += MUL_FIXED(brightness,lptr->RedScale);			
					greenI += MUL_FIXED(brightness,lptr->GreenScale);			
					blueI += MUL_FIXED(brightness,lptr->BlueScale);
				}
			}
		}
	}
	if (redI>ONE_FIXED) redI=ONE_FIXED;
	else if (redI<GlobalAmbience) redI=GlobalAmbience;
	if (greenI>ONE_FIXED) greenI=ONE_FIXED;
	else if (greenI<GlobalAmbience) greenI=GlobalAmbience;
 	if (blueI>ONE_FIXED) blueI=ONE_FIXED;
	else if (blueI<GlobalAmbience) blueI=GlobalAmbience;
	
	int alpha = 192-offset/4;
	if (alpha>255) alpha = 255;
	if (alpha<128) alpha = 128;

//	return RGBALIGHT_MAKE(MUL_FIXED(64+(offset&128),redI),MUL_FIXED(64+(offset&128),greenI),MUL_FIXED(64+(offset&128),blueI),alpha);
//	return RGBALIGHT_MAKE(MUL_FIXED(50,redI),MUL_FIXED(255,greenI),MUL_FIXED(140,blueI),alpha);
	return RGBALIGHT_MAKE(MUL_FIXED(10,redI),MUL_FIXED(51,greenI),MUL_FIXED(28,blueI),alpha);
//	return RGBALIGHT_MAKE(MUL_FIXED(128,redI),MUL_FIXED(128,greenI),MUL_FIXED(255,blueI),alpha);
	#endif
}
int LightIntensityAtPoint(VECTORCH *pointPtr)
{
	int intensity=0;

	DISPLAYBLOCK **activeBlockListPtr = ActiveBlockList;
	for(int i = NumActiveBlocks; i!=0; i--)
	{
		DISPLAYBLOCK *dispPtr = *activeBlockListPtr++;

		if(dispPtr->ObNumLights)
		{
			for(int j = 0; j < dispPtr->ObNumLights; j++)
			{
				LIGHTBLOCK *lptr = dispPtr->ObLights[j];

				VECTORCH disp = lptr->LightWorld;
				disp.vx -= pointPtr->vx;
				disp.vy -= pointPtr->vy;
				disp.vz -= pointPtr->vz;
				
				int dist = Approximate3dMagnitude(&disp);
				
				if (dist<lptr->LightRange)
				{
					intensity += WideMulNarrowDiv(lptr->LightBright,lptr->LightRange-dist,lptr->LightRange);
				}
			}
		}
	}
	if (intensity>ONE_FIXED) intensity=ONE_FIXED;
	else if (intensity<GlobalAmbience) intensity=GlobalAmbience;
	
	/* KJL 20:31:39 12/1/97 - limit how dark things can be so blood doesn't go green */
	if (intensity<10*256) intensity = 10*256;

	return intensity;
}
signed int ForceFieldPointDisplacement[15*3+1][16];
signed int ForceFieldPointDisplacement2[15*3+1][16];
signed int ForceFieldPointVelocity[15*3+1][16];
unsigned char ForceFieldPointColour1[15*3+1][16];
unsigned char ForceFieldPointColour2[15*3+1][16];

int Phase=0;
int ForceFieldPhase=0;
void InitForceField(void)
{
	for (int x=0; x<15*3+1; x++)
		for (int y=0; y<16; y++)
		{
			ForceFieldPointDisplacement[x][y]=0;
			ForceFieldPointDisplacement2[x][y]=0;
			ForceFieldPointVelocity[x][y]=0;
		}
	ForceFieldPhase=0;
}
#if 1

void UpdateForceField(void)
{
	#if 1
	Phase+=NormalFrameTime>>6;
	ForceFieldPhase+=NormalFrameTime>>5;
	int x;
	for (x=1; x<15*3; x++)
	{
		int y;
		for (y=1; y<15; y++)
		{
			
			int acceleration =32*(-8*ForceFieldPointDisplacement[x][y]
								+ForceFieldPointDisplacement[x-1][y-1]
								+ForceFieldPointDisplacement[x-1][y]
								+ForceFieldPointDisplacement[x-1][y+1]
								+ForceFieldPointDisplacement[x][y-1]
								+ForceFieldPointDisplacement[x][y+1]
#if 0
								)
#else								

								+ForceFieldPointDisplacement[x+1][y-1]
								+ForceFieldPointDisplacement[x+1][y]
								+ForceFieldPointDisplacement[x+1][y+1])			
#endif
								-(ForceFieldPointVelocity[x][y]*5);

			ForceFieldPointVelocity[x][y] += MUL_FIXED(acceleration,NormalFrameTime);
			ForceFieldPointDisplacement2[x][y] += MUL_FIXED(ForceFieldPointVelocity[x][y],NormalFrameTime);
#if 1
			if(ForceFieldPointDisplacement2[x][y]>200) ForceFieldPointDisplacement2[x][y]=200;
			if(ForceFieldPointDisplacement2[x][y]<-200) ForceFieldPointDisplacement2[x][y]=-200;
#else
			if(ForceFieldPointDisplacement2[x][y]>512) ForceFieldPointDisplacement2[x][y]=512;
			if(ForceFieldPointDisplacement2[x][y]<-512) ForceFieldPointDisplacement2[x][y]=-512;

#endif
			{
				int offset = ForceFieldPointDisplacement2[x][y];
				int colour = ForceFieldPointVelocity[x][y]/4;

				if (offset<0) offset =-offset;
				if (colour<0) colour =-colour;
				colour=(colour+offset)/2;

				if(colour>255) colour=255;
				colour++;
				
				ForceFieldPointColour1[x][y]=FastRandom()%colour;
				ForceFieldPointColour2[x][y]=FastRandom()%colour;
			}
		}

	}
	for (x=1; x<15*3; x++)
	{
		int y;
		for (y=1; y<15; y++)
		{
			ForceFieldPointDisplacement[x][y] = ForceFieldPointDisplacement2[x][y];
		}
	}
	{
		#if 1
	  	if(ForceFieldPhase>1000)
		{
			ForceFieldPhase=0;
			int x = 1+(FastRandom()%(15*3-2));
			int y = 1+(FastRandom()%13);
			ForceFieldPointVelocity[x][y] = 10000;
			ForceFieldPointVelocity[x][y+1] = 10000;
			ForceFieldPointVelocity[x+1][y] = 10000;
			ForceFieldPointVelocity[x+1][y+1] = 10000;
		}	
		#else
	   //	if(ForceFieldPhase>1000)
		{
			ForceFieldPhase=0;
			int x = 1+(FastRandom()%(15*3-2));
			int y = 1+(FastRandom()%13);
			ForceFieldPointVelocity[x][y] = (FastRandom()&16383)+8192;
		}
		#endif				   
	}
	#else
	int x;
	int y;
	for (y=0; y<=15; y++)
	{
		ForceFieldPointDisplacement[0][y] += (FastRandom()&127)-64;
		if(ForceFieldPointDisplacement[0][y]>512) ForceFieldPointDisplacement[0][y]=512;
		if(ForceFieldPointDisplacement[0][y]<-512) ForceFieldPointDisplacement[0][y]=-512;
		ForceFieldPointVelocity[0][y] = (FastRandom()&16383)-8192;
	}
	for (x=15*3-1; x>0; x--)
	{
		for (y=0; y<=15; y++)
		{
			ForceFieldPointDisplacement[x][y] = ForceFieldPointDisplacement[x-1][y];
			ForceFieldPointVelocity[x][y] = ForceFieldPointVelocity[x-1][y];
		}

	}
	for (x=15*3-1; x>1; x--)
	{
		y = FastRandom()&15;
	 	ForceFieldPointDisplacement[x][y] = ForceFieldPointDisplacement[x-1][y];
		y = (FastRandom()&15)-1;
	 	ForceFieldPointDisplacement[x][y] = ForceFieldPointDisplacement[x-1][y];
	}
	#endif
}
void UpdateWaterFall(void)
{
	int x;
	int y;
	for (y=0; y<=15; y++)
	{
		ForceFieldPointDisplacement[0][y] += (FastRandom()&127)-64;
		if(ForceFieldPointDisplacement[0][y]>512) ForceFieldPointDisplacement[0][y]=512;
		if(ForceFieldPointDisplacement[0][y]<-512) ForceFieldPointDisplacement[0][y]=-512;
		ForceFieldPointVelocity[0][y] = (FastRandom()&16383)-8192;
	}
	for (x=15*3-1; x>0; x--)
	{
		for (y=0; y<=15; y++)
		{
			ForceFieldPointDisplacement[x][y] = ForceFieldPointDisplacement[x-1][y];
			ForceFieldPointVelocity[x][y] = ForceFieldPointVelocity[x-1][y];
		}

	}
	for (x=15*3-1; x>1; x--)
	{
		y = FastRandom()&15;
	 	ForceFieldPointDisplacement[x][y] = ForceFieldPointDisplacement[x-1][y];
		y = (FastRandom()&15)-1;
	 	ForceFieldPointDisplacement[x][y] = ForceFieldPointDisplacement[x-1][y];
	}
}

#endif
void D3D_DrawForceField(int xOrigin, int yOrigin, int zOrigin, int fieldType)
{
	MeshXScale = 4096/16;
	MeshZScale = 4096/16;
	
	for (int field=0; field<3; field++)
	{
	int i=0;			   
	int x;
	for (x=(0+field*15); x<(16+field*15); x++)
	{
		int z;
		for(z=0; z<16; z++)
		{
			VECTORCH *point = &MeshVertex[i];
			int offset = ForceFieldPointDisplacement[x][z];
			
			switch(fieldType)
			{
				case 0:
				{
				 	point->vx = xOrigin+(x*MeshXScale);
				 	point->vy = yOrigin+(z*MeshZScale);
				 	point->vz = zOrigin+offset;
					break;
				}
				case 1:
				{	

					int theta = (z*4095)/15;
					int u = (x*65536)/45;

					int b = MUL_FIXED(2*u,(65536-u));
					int c = MUL_FIXED(u,u);
					int phi = (Phase&4095);
					int x3 = (GetSin(phi))/64;
					int y3 = 5000-(GetCos((phi*3+1000)&4095)/128);
					int z3 = (GetSin((3*phi+1324)&4095))/32;
					int x2 = -x3/2;
					int y2 = 3000;
					int z2 = -z3/4;
					int innerRadius = 100;//GetSin(u/32)/16+offset;

					point->vx = xOrigin+(b*x2+c*x3)/65536+MUL_FIXED(innerRadius,GetSin(theta));
					point->vy = yOrigin-5000+(b*y2+c*y3)/65536;
					point->vz = zOrigin+(b*z2+c*z3)/65536+MUL_FIXED(innerRadius,GetCos(theta));
					break;
				}
				case 2:
				{
					int theta = (z*4095)/15;
					int phi = (x*4095)/45;
					int innerRadius = 1000+offset;
					int outerRadius = 4000;
					

					point->vx = xOrigin+MUL_FIXED(outerRadius-MUL_FIXED(innerRadius,GetSin(theta)),GetCos(phi));
					point->vy = yOrigin+MUL_FIXED(innerRadius,GetCos(theta));
					point->vz = zOrigin+MUL_FIXED(outerRadius-MUL_FIXED(innerRadius,GetSin(theta)),GetSin(phi));
					break;
				}
				case 3:
				{	

					int theta = (x*4095)/45;
					int radius = offset+2000;
					point->vx = xOrigin+MUL_FIXED(radius,GetCos(theta));
					point->vy = yOrigin+(z*MeshZScale);
					point->vz = zOrigin+MUL_FIXED(radius,GetSin(theta));
					break;
				}
			}			

			if (offset<0) offset =-offset;
			offset+=16;

//			offset-=32;
//			if (offset<0) offset = 0;

			if(offset>255) offset=255;
	  
			MeshVertexColour[i] = RGBALIGHT_MAKE(ForceFieldPointColour1[x][z],ForceFieldPointColour2[x][z],255,offset);
			#if TEXTURE_WATER
			MeshWorldVertex[i].vx = point->vx;			
			MeshWorldVertex[i].vz = point->vz;			
			#endif
			
			TranslatePointIntoViewspace(point);
			
			/* is particle within normal view frustrum ? */
			if(AvP.PlayerType==I_Alien)	/* wide frustrum */
			{
				if(( (-point->vx <= point->vz*2)
		   			&&(point->vx <= point->vz*2)
					&&(-point->vy <= point->vz*2)
					&&(point->vy <= point->vz*2) ))
				{
					MeshVertexOutcode[i]=1;
				}
				else
				{
					MeshVertexOutcode[i]=0;
				}
			}
			else
			{
				if(( (-point->vx <= point->vz)
		   			&&(point->vx <= point->vz)
					&&(-point->vy <= point->vz)
					&&(point->vy <= point->vz) ))
				{
					MeshVertexOutcode[i]=1;
				}
				else
				{
					MeshVertexOutcode[i]=0;
				}
			}

			i++;
		}
	}
	//textprint("\n");
	if ((MeshVertexOutcode[0]&&MeshVertexOutcode[15]&&MeshVertexOutcode[240]&&MeshVertexOutcode[255]))
	{
		D3D_DrawWaterMesh_Unclipped();
	}	
	else
//	else if (MeshVertexOutcode[0]||MeshVertexOutcode[15]||MeshVertexOutcode[240]||MeshVertexOutcode[255])
	{
		D3D_DrawWaterMesh_Clipped();
	}	
	}
}


void D3D_DrawPowerFence(int xOrigin, int yOrigin, int zOrigin, int xScale, int yScale, int zScale)
{
	for (int field=0; field<3; field++)
	{
	int i=0;			   
	int x;
	for (x=(0+field*15); x<(16+field*15); x++)
	{
		int z;
		for(z=0; z<16; z++)
		{
			VECTORCH *point = &MeshVertex[i];
			int offset = ForceFieldPointDisplacement[x][z];
			
		 	point->vx = xOrigin+(x*xScale);
		 	point->vy = yOrigin+(z*yScale);
		 	point->vz = zOrigin+(x*zScale);

			if (offset<0) offset =-offset;
			offset+=16;

			if(offset>255) offset=255;
	  
			MeshVertexColour[i] = RGBALIGHT_MAKE(ForceFieldPointColour1[x][z],ForceFieldPointColour2[x][z],255,offset);
			
			/* translate particle into view space */
			TranslatePointIntoViewspace(point);
			
			/* is particle within normal view frustrum ? */
			if(AvP.PlayerType==I_Alien)	/* wide frustrum */
			{
				if(( (-point->vx <= point->vz*2)
		   			&&(point->vx <= point->vz*2)
					&&(-point->vy <= point->vz*2)
					&&(point->vy <= point->vz*2) ))
				{
					MeshVertexOutcode[i]=1;
				}
				else
				{
					MeshVertexOutcode[i]=0;
				}
			}
			else
			{
				if(( (-point->vx <= point->vz)
		   			&&(point->vx <= point->vz)
					&&(-point->vy <= point->vz)
					&&(point->vy <= point->vz) ))
				{
					MeshVertexOutcode[i]=1;
				}
				else
				{
					MeshVertexOutcode[i]=0;
				}
			}

			i++;
		}
	}
	//textprint("\n");
	if ((MeshVertexOutcode[0]&&MeshVertexOutcode[15]&&MeshVertexOutcode[240]&&MeshVertexOutcode[255]))
	{
		D3D_DrawWaterMesh_Unclipped();
	}	
	else
//	else if (MeshVertexOutcode[0]||MeshVertexOutcode[15]||MeshVertexOutcode[240]||MeshVertexOutcode[255])
	{
		D3D_DrawWaterMesh_Clipped();
	}	
	}
}

void D3D_DrawWaterFall(int xOrigin, int yOrigin, int zOrigin)
{
	{
		int noRequired = MUL_FIXED(250,NormalFrameTime);
		for (int i=0; i<noRequired; i++)
		{
			VECTORCH velocity;
			VECTORCH position;
			position.vx = xOrigin;
			position.vy = yOrigin-(FastRandom()&511);//+45*MeshXScale;
			position.vz = zOrigin+(FastRandom()%(15*MeshZScale));

			velocity.vy = (FastRandom()&511)+512;//-((FastRandom()&1023)+2048)*8;
			velocity.vx = ((FastRandom()&511)+256)*2;
			velocity.vz = 0;//-((FastRandom()&511))*8;
			MakeParticle(&(position), &velocity, PARTICLE_WATERFALLSPRAY);
		}
		#if 0
		noRequired = MUL_FIXED(200,NormalFrameTime);
		for (i=0; i<noRequired; i++)
		{
			VECTORCH velocity;
			VECTORCH position;
			position.vx = xOrigin+(FastRandom()%(15*MeshZScale));
			position.vy = yOrigin+45*MeshXScale;
			position.vz = zOrigin;

			velocity.vy = -((FastRandom()&16383)+4096);
			velocity.vx = ((FastRandom()&4095)-2048);
			velocity.vz = -((FastRandom()&2047)+1048);
			MakeParticle(&(position), &velocity, PARTICLE_WATERFALLSPRAY);
		}
		#endif
	}
	{
		extern void RenderWaterFall(int xOrigin, int yOrigin, int zOrigin);
		//RenderWaterFall(xOrigin, yOrigin-500, zOrigin+50);
	}
   	return;
	for (int field=0; field<3; field++)
	{
	int i=0;			   
	int x;
	for (x=(0+field*15); x<(16+field*15); x++)
	{
		int z;
		for(z=0; z<16; z++)
		{
			VECTORCH *point = &MeshVertex[i];
			int offset = ForceFieldPointDisplacement[x][z];

		#if 1
			int u = (x*65536)/45;

			int b = MUL_FIXED(2*u,(65536-u));
			int c = MUL_FIXED(u,u);
			int y3 = 45*MeshXScale;
			int x3 = 5000;
			int y2 = 1*MeshXScale;
			int x2 = GetSin(CloakingPhase&4095)+GetCos((CloakingPhase*3+399)&4095);
			x2 = MUL_FIXED(x2,x2)/128;

			if (offset<0) offset =-offset;
			point->vx = xOrigin+MUL_FIXED(b,x2)+MUL_FIXED(c,x3)+offset;
			point->vy = yOrigin+MUL_FIXED(b,y2)+MUL_FIXED(c,y3);
			point->vz = zOrigin+(z*MeshZScale);
			
			if (point->vy>4742)
			{
				if (z<=4)
				{
					point->vy-=MeshXScale; 
					if (point->vy<4742) point->vy=4742;
					if (point->vx<179427) point->vx=179427;
				}
				else if (z<=8)
				{
					point->vx+=(8-z)*1000;
				}
			}

			#else
			if (offset<0) offset =-offset;
		 	point->vx = xOrigin-offset;
		 	point->vy = yOrigin+(x*MeshXScale);
		 	point->vz = zOrigin+(z*MeshZScale);
			#endif


			   	

			offset= (offset/4)+127;

//			offset-=32;
//			if (offset<0) offset = 0;

			if(offset>255) offset=255;
	  
			MeshVertexColour[i] = RGBALIGHT_MAKE(offset,offset,255,offset/2);
			#if TEXTURE_WATER
			MeshWorldVertex[i].vx = point->vx;			
			MeshWorldVertex[i].vz = point->vz;			
			#endif
			
			/* translate particle into view space */
			TranslatePointIntoViewspace(point);
			
			/* is particle within normal view frustrum ? */
			if(AvP.PlayerType==I_Alien)	/* wide frustrum */
			{
				if(( (-point->vx <= point->vz*2)
		   			&&(point->vx <= point->vz*2)
					&&(-point->vy <= point->vz*2)
					&&(point->vy <= point->vz*2) ))
				{
					MeshVertexOutcode[i]=1;
				}
				else
				{
					MeshVertexOutcode[i]=0;
				}
			}
			else
			{
				if(( (-point->vx <= point->vz)
		   			&&(point->vx <= point->vz)
					&&(-point->vy <= point->vz)
					&&(point->vy <= point->vz) ))
				{
					MeshVertexOutcode[i]=1;
				}
				else
				{
					MeshVertexOutcode[i]=0;
				}
			}

			i++;
		}
	}
	//textprint("\n");
	if ((MeshVertexOutcode[0]&&MeshVertexOutcode[15]&&MeshVertexOutcode[240]&&MeshVertexOutcode[255]))
	{
		D3D_DrawWaterMesh_Unclipped();
	}	
	else
//	else if (MeshVertexOutcode[0]||MeshVertexOutcode[15]||MeshVertexOutcode[240]||MeshVertexOutcode[255])
	{
		D3D_DrawWaterMesh_Clipped();
	}	
	}
}

void D3D_DrawParticleBeam(DISPLAYBLOCK *muzzlePtr, VECTORCH *targetPositionPtr)
{
	VECTORCH vertices[6];
	int beamRadius = 45+(FastRandom()&15);
	vertices[0] = *targetPositionPtr;
	vertices[1] = *targetPositionPtr;
	vertices[2] = *targetPositionPtr;

	vertices[3] = muzzlePtr->ObWorld;
	vertices[4] = muzzlePtr->ObWorld;
	vertices[5] = muzzlePtr->ObWorld;

	vertices[1].vy += beamRadius;
	vertices[2].vy -= beamRadius;
	vertices[4].vy += beamRadius;
	vertices[5].vy -= beamRadius;

	/* translate vertices into vxew space */
	TranslatePointIntoViewspace(&vertices[0]);
	TranslatePointIntoViewspace(&vertices[1]);
	TranslatePointIntoViewspace(&vertices[2]);
	TranslatePointIntoViewspace(&vertices[3]);
	TranslatePointIntoViewspace(&vertices[4]);
	TranslatePointIntoViewspace(&vertices[5]);
	
	float ZNear = (float) (Global_VDB_Ptr->VDB_ClipZ * GlobalScale);

	/* OUTPUT VERTICES TO EXECUTE BUFFER */
	{
		int i = 6;
		VECTORCH *verticesPtr = vertices;
		do
		{
			D3DTLVERTEX *vertexPtr = &((LPD3DTLVERTEX)ExecuteBufferDataArea)[NumVertices];
			int x = (verticesPtr->vx*(Global_VDB_Ptr->VDB_ProjX))/verticesPtr->vz+Global_VDB_Ptr->VDB_CentreX;
			int y = (verticesPtr->vy*(Global_VDB_Ptr->VDB_ProjY))/verticesPtr->vz+Global_VDB_Ptr->VDB_CentreY;
			{
				if (x<Global_VDB_Ptr->VDB_ClipLeft)
				{
					x=Global_VDB_Ptr->VDB_ClipLeft;
				}	
				else if (x>Global_VDB_Ptr->VDB_ClipRight)
				{
					x=Global_VDB_Ptr->VDB_ClipRight;	
				}
				
				vertexPtr->sx=x;
			}
			{
				if (y<Global_VDB_Ptr->VDB_ClipUp)
				{
					y=Global_VDB_Ptr->VDB_ClipUp;
				}
				else if (y>Global_VDB_Ptr->VDB_ClipDown)
				{
					y=Global_VDB_Ptr->VDB_ClipDown;	
				}
				vertexPtr->sy=y;
			}
	
		  	float oneOverZ = ((float)verticesPtr->vz-ZNear)/(float)verticesPtr->vz;
	
		  	 // Different behavxour for different driver modes
			switch (D3DDriverMode)
			{
				default:
				case D3DSoftwareRGBDriver:
				case D3DSoftwareRampDriver:
				break;
				case D3DHardwareRGBDriver:
				{
					if (i==6 || i==3) vertexPtr->color = RGBALIGHT_MAKE(255,255,255,255);
					else vertexPtr->color = RGBALIGHT_MAKE(0,0,255,32);

					break;
				}
			}
			vertexPtr->sz = oneOverZ;
			NumVertices++;
			verticesPtr++;
		}
	  	while(--i);
	}
	CheckTranslucencyModeIsCorrect(TRANSLUCENCY_GLOWING);

	OP_TRIANGLE_LIST(4, ExecBufInstPtr);
	OUTPUT_TRIANGLE(0,3,5, 6);
	OUTPUT_TRIANGLE(0,3,4, 6);
	OUTPUT_TRIANGLE(0,2,5, 6);
	OUTPUT_TRIANGLE(0,1,4, 6);
	if (NumVertices > (MaxVerticesInExecuteBuffer-12)) 
	{
	   WriteEndCodeToExecuteBuffer();
  	   UnlockExecuteBufferAndPrepareForUse();
	   ExecuteBuffer();
  	   LockExecuteBuffer();
	}
}
void D3D_DrawExplosion(int xOrigin, int yOrigin, int zOrigin, int size)
{
			if (NumVertices)
			{
			   WriteEndCodeToExecuteBuffer();
		  	   UnlockExecuteBufferAndPrepareForUse();
			   ExecuteBuffer();
		  	   LockExecuteBuffer();
			}
		
			// Turn OFF texturing if it is on...
			if (CurrTextureHandle != NULL)
			{
				OP_STATE_RENDER(1, ExecBufInstPtr);
				STATE_DATA(D3DRENDERSTATE_TEXTUREHANDLE, NULL, ExecBufInstPtr);
				CurrTextureHandle = NULL;
			}
	
			CheckTranslucencyModeIsCorrect(TRANSLUCENCY_GLOWING);
	
	
		 	OP_STATE_RENDER(1, ExecBufInstPtr);
		 	STATE_DATA(D3DRENDERSTATE_ZWRITEENABLE, FALSE, ExecBufInstPtr);
	MeshXScale = 4096/16;
	MeshZScale = 4096/16;
	
	int i=0;
	int x;
	for (x=0; x<16; x++)
	{
		int z;
		for(z=0; z<16; z++)
		{
			VECTORCH *point = &MeshVertex[i];
			int offset;// = MUL_FIXED((FastRandom()&1023)-512,CloakingPhase&65535);//MUL_FIXED(255,GetSin((point->vx+point->vy+point->vz+CloakingPhase)&4095));
			int sd;
			{
				int theta = ((z*2048)/15)&4095;
				int phi = ((x*4096)/15)&4095;
				if (theta==0) sd = 0;
				else sd = phi+theta;
				if (phi==0 || phi==2048) sd = 0;
				else sd = phi+theta;

				int outerRadius = MUL_FIXED(4000,size);

				if (sd)
				offset = MUL_FIXED((FastRandom()&511)-255,size);	
				
				offset=0;
				
				point->vx = xOrigin+MUL_FIXED(outerRadius+offset,MUL_FIXED(GetSin(theta),GetCos(phi)));
				point->vy = yOrigin+MUL_FIXED(outerRadius+offset,MUL_FIXED(GetSin(theta),GetSin(phi)));
				point->vz = zOrigin+MUL_FIXED(outerRadius+offset,GetCos(theta));
			}

			if(offset<0) offset = -offset;
			MeshVertexColour[i] = RGBALIGHT_MAKE(255,GetSin((sd+CloakingPhase)&4095)&255,0,32);

			TranslatePointIntoViewspace(point);
			
			/* is particle within normal view frustrum ? */
			if(AvP.PlayerType==I_Alien)	/* wide frustrum */
			{
				if(( (-point->vx <= point->vz*2)
		   			&&(point->vx <= point->vz*2)
					&&(-point->vy <= point->vz*2)
					&&(point->vy <= point->vz*2) ))
				{
					MeshVertexOutcode[i]=1;
				}
				else
				{
					MeshVertexOutcode[i]=0;
				}
			}
			else
			{
				if(( (-point->vx <= point->vz)
		   			&&(point->vx <= point->vz)
					&&(-point->vy <= point->vz)
					&&(point->vy <= point->vz) ))
				{
					MeshVertexOutcode[i]=1;
				}
				else
				{
					MeshVertexOutcode[i]=0;
				}
			}

			i++;
		}
	}
	textprint("drawing explosion\n");
	if ((MeshVertexOutcode[0]&&MeshVertexOutcode[15]&&MeshVertexOutcode[240]&&MeshVertexOutcode[255]))
	{
		D3D_DrawWaterMesh_Unclipped();
	}	
	else
//	else if (MeshVertexOutcode[0]||MeshVertexOutcode[15]||MeshVertexOutcode[240]||MeshVertexOutcode[255])
	{
		D3D_DrawWaterMesh_Clipped();
	}	
			OP_STATE_RENDER(1, ExecBufInstPtr);
			STATE_DATA(D3DRENDERSTATE_ZWRITEENABLE, TRUE, ExecBufInstPtr);

}


void DrawFrameRateBar(void)
{
	extern int NormalFrameTime;
	#if 1
	{
		int width = DIV_FIXED(ScreenDescriptorBlock.SDB_Width/120,NormalFrameTime);
		if (width>ScreenDescriptorBlock.SDB_Width) width=ScreenDescriptorBlock.SDB_Width;

		r2rect rectangle
		(
			0,0,
			width,
			24
		);
		textprint("width %d\n",width);															 

		rectangle . AlphaFill
		(
			0xff, // unsigned char R,
			0x00,// unsigned char G,
			0x00,// unsigned char B,
		   	128 // unsigned char translucency
		);
	}
	#endif
}
void D3D_DrawAlienRedBlipIndicatingJawAttack(void)
{
	r2rect rectangle
	(
		16,16,
		32,
		32
	);

	rectangle . AlphaFill
	(
		0xff, // unsigned char R,
		0x00,// unsigned char G,
		0x00,// unsigned char B,
	   	128 // unsigned char translucency
	);
}
void D3D_DrawBackdrop(void)
{
	extern char LevelName[];

	if (TRIPTASTIC_CHEATMODE||MOTIONBLUR_CHEATMODE) return;

	if(WireFrameMode)
	{
   		ColourFillBackBuffer(0);
		return;
	}
	else if(ShowDebuggingText.Tears)
	{
		ColourFillBackBuffer((63<<5));
		return;
	}

	{
		int needToDrawBackdrop=0;
		extern int NumActiveBlocks;
		extern DISPLAYBLOCK *ActiveBlockList[];
		
		int numOfObjects = NumActiveBlocks;
		while(numOfObjects--)
		{
			DISPLAYBLOCK *objectPtr = ActiveBlockList[numOfObjects];
			MODULE *modulePtr = objectPtr->ObMyModule;

			
			if (modulePtr && (ModuleCurrVisArray[modulePtr->m_index] == 2) &&modulePtr->m_flags&MODULEFLAG_SKY)
			{
				needToDrawBackdrop=1;
				break;
			}
		}
		if(needToDrawBackdrop)
		{
			extern BOOL LevelHasStars;
			extern void RenderSky(void);

			ColourFillBackBuffer(0);
	  		
			if (LevelHasStars)
			{
				extern void RenderStarfield(void);
				RenderStarfield();
			}
			else
			{
		  		RenderSky();
			}
			return;
		}
	}


	/* if the player is outside the environment, clear the screen! */	
	{
		extern MODULE *playerPherModule;
 		if (!playerPherModule)
 		{
 			ColourFillBackBuffer(0);
			return;
		}
	}
	{
		PLAYER_STATUS *playerStatusPtr= (PLAYER_STATUS *) (Player->ObStrategyBlock->SBdataptr);

		if (!playerStatusPtr->IsAlive || FREEFALL_CHEATMODE)
		{
			// minimise effects of camera glitches
			ColourFillBackBuffer(0);
			return;
		}
	}
}

void MakeNoiseTexture(void)
{
// 	return;
	DDSURFACEDESC ddsd;
	LPDIRECTDRAWSURFACE tempSurface;
	LPDIRECT3DTEXTURE tempTexture;

	LPDIRECTDRAWSURFACE destSurface;
	LPDIRECT3DTEXTURE destTexture;


	memcpy(&ddsd, &(d3d.TextureFormat[d3d.CurrentTextureFormat].ddsd), sizeof(ddsd));

	ddsd.dwSize = sizeof(ddsd);

	ddsd.dwFlags = (DDSD_CAPS | DDSD_HEIGHT | DDSD_WIDTH | DDSD_PIXELFORMAT);
	ddsd.ddsCaps.dwCaps = (DDSCAPS_SYSTEMMEMORY|DDSCAPS_TEXTURE);

	ddsd.dwHeight = 256;
	ddsd.dwWidth = 256;

	LastError = lpDD->CreateSurface(&ddsd, &tempSurface, NULL);
	LOGDXERR(LastError);

	

	LastError = tempSurface->QueryInterface(IID_IDirect3DTexture, (LPVOID*) &tempTexture);
	LOGDXERR(LastError);

	// Query destination surface for a texture interface.
	memset(&ddsd, 0, sizeof(DDSURFACEDESC));
	ddsd.dwSize = sizeof(DDSURFACEDESC);

	LastError = tempSurface->GetSurfaceDesc(&ddsd);
	LOGDXERR(LastError);

	ddsd.dwFlags = (DDSD_CAPS | DDSD_HEIGHT | DDSD_WIDTH | DDSD_PIXELFORMAT);
	ddsd.ddsCaps.dwCaps = (DDSCAPS_TEXTURE | DDSCAPS_ALLOCONLOAD );

	LastError = lpDD->CreateSurface(&ddsd, &destSurface, NULL);
	LOGDXERR(LastError);

	/* KJL 11:59:21 09/02/98 - check for palettised modes */
	{
		int PalCaps;

		if (ddsd.ddpfPixelFormat.dwFlags & DDPF_PALETTEINDEXED8)
		{
			PalCaps = (DDPCAPS_8BIT | DDPCAPS_ALLOW256);
		}
		#if 0
		else if (ddsd.ddpfPixelFormat.dwFlags &	DDPF_PALETTEINDEXED4)
		{
			PalCaps = DDPCAPS_4BIT;
		}
		else
		#endif
		{
			PalCaps = 0;
		}

		if (PalCaps)
		{
			LPDIRECTDRAWPALETTE destPalette = NULL;
			PALETTEENTRY palette[256];
			memset(palette, 0, sizeof(PALETTEENTRY) * 256);
			for(int i=0;i<256;i++)
			{
				palette[i].peRed = i;
				palette[i].peGreen = i;
				palette[i].peBlue = i;
			}

			LastError = lpDD->CreatePalette(PalCaps, palette, &destPalette, NULL);
			LOGDXERR(LastError);

			LastError = destSurface->SetPalette(destPalette);
			LastError = tempSurface->SetPalette(destPalette);
			LOGDXERR(LastError);
			{
		   		memset(&ddsd, 0, sizeof(DDSURFACEDESC));
				ddsd.dwSize = sizeof(DDSURFACEDESC);
				LastError = tempSurface->Lock(NULL, &ddsd, 0, NULL);
				LOGDXERR(LastError);

				
				unsigned char *dst = (unsigned char *)ddsd.lpSurface;
				LOCALASSERT(dst);
			 	for(int i = 0; i < 256; i ++)
			 	{
		 			for(int j = 0; j < 256; j ++)
			 		{
						int c = FastRandom()&255;
						*dst++ = (c);
					}
			 		/* move the pitch to width difference */
			 		//dst += ddsd.lPitch - 256;
			 	}

				LastError = tempSurface->Unlock(NULL);
				LOGDXERR(LastError);
			}
		}
		else
		{
			{
		   		memset(&ddsd, 0, sizeof(DDSURFACEDESC));
				ddsd.dwSize = sizeof(DDSURFACEDESC);
				LastError = tempSurface->Lock(NULL, &ddsd, 0, NULL);
				LOGDXERR(LastError);

				
				unsigned short *dst = (unsigned short *)ddsd.lpSurface;
				LOCALASSERT(dst);
			 	for(int i = 0; i < 256; i ++)
			 	{
					int density = FastRandom()&31;
					{
			 			for(int j = 0; j < 256; j ++)
				 		{
							int c = FastRandom()&31;
							*dst++ = (c)+(c<<6)+(c<<11);
				 		}
					}
			 		/* move the pitch to width difference */
			 		//dst += ddsd.lPitch - 256;
			 	}

				LastError = tempSurface->Unlock(NULL);
				LOGDXERR(LastError);
			}
		}
	}

	LastError = destSurface->QueryInterface(IID_IDirect3DTexture,(LPVOID*) &destTexture);
	LOGDXERR(LastError);

	LastError = destTexture->Load(tempTexture);
 	LOGDXERR(LastError);

	// Clean up surfaces etc 
//	RELEASE(SrcTexture);
	LastError = destTexture->GetHandle(d3d.lpD3DDevice, &NoiseTextureHandle);
	RELEASE(tempSurface);
	RELEASE(tempTexture);

}
void DrawNoiseOverlay(int t)
{
//	return;
	{
		D3DTLVERTEX *vertexPtr = &((LPD3DTLVERTEX)ExecuteBufferDataArea)[NumVertices];
		  
		extern float CameraZoomScale;
		float u = FastRandom()&255;
		float v = FastRandom()&255;
		int c = 255;
		int size = 256;//*CameraZoomScale;
	  	vertexPtr->sx =	Global_VDB_Ptr->VDB_ClipLeft;
	  	vertexPtr->sy =	Global_VDB_Ptr->VDB_ClipUp;
		vertexPtr->sz = 1.0f;
		vertexPtr->rhw = 1.0f;
		vertexPtr->tu = u/256.0;
		vertexPtr->tv = v/256.0;
		vertexPtr->color = RGBALIGHT_MAKE(c,c,c,t);
		vertexPtr++;
	  	vertexPtr->sx =	Global_VDB_Ptr->VDB_ClipRight;
	  	vertexPtr->sy =	Global_VDB_Ptr->VDB_ClipUp;
		vertexPtr->sz = 1.0f;
		vertexPtr->rhw = 1.0f;
		vertexPtr->tu = (u+size)/256.0;
		vertexPtr->tv = v/256.0;
		vertexPtr->color = RGBALIGHT_MAKE(c,c,c,t);
		vertexPtr++;
	  	vertexPtr->sx =	Global_VDB_Ptr->VDB_ClipRight;
	  	vertexPtr->sy =	Global_VDB_Ptr->VDB_ClipDown;
		vertexPtr->sz = 1.0f;
		vertexPtr->rhw = 1.0f;
		vertexPtr->tu = (u+size)/256.0;
		vertexPtr->tv = (v+size)/256.0;
		vertexPtr->color = RGBALIGHT_MAKE(c,c,c,t);
		vertexPtr++;
	  	vertexPtr->sx =	Global_VDB_Ptr->VDB_ClipLeft;
	  	vertexPtr->sy =	Global_VDB_Ptr->VDB_ClipDown;
		vertexPtr->sz = 1.0f;
		vertexPtr->rhw = 1.0f;
		vertexPtr->tu = u/256.0;
		vertexPtr->tv = (v+size)/256.0;
		vertexPtr->color = RGBALIGHT_MAKE(c,c,c,t);

		NumVertices+=4;
	}
	CheckTranslucencyModeIsCorrect(TRANSLUCENCY_GLOWING);
	CheckFilteringModeIsCorrect(FILTERING_BILINEAR_ON);
    OP_STATE_RENDER(1, ExecBufInstPtr);
    STATE_DATA(D3DRENDERSTATE_ZFUNC, D3DCMP_ALWAYS, ExecBufInstPtr);

 	NoiseTextureHandle = (D3DTEXTUREHANDLE)ImageHeaderArray[StaticImageNumber].D3DHandle;
    if (CurrTextureHandle != NoiseTextureHandle)
	{
    	OP_STATE_RENDER(1, ExecBufInstPtr);
        STATE_DATA(D3DRENDERSTATE_TEXTUREHANDLE, NoiseTextureHandle, ExecBufInstPtr);
        CurrTextureHandle = NoiseTextureHandle;
	}
    if (D3DTexturePerspective != No)
	{
		D3DTexturePerspective = No;
		OP_STATE_RENDER(1, ExecBufInstPtr);
		STATE_DATA(D3DRENDERSTATE_TEXTUREPERSPECTIVE, FALSE, ExecBufInstPtr);
	}


	OP_TRIANGLE_LIST(2, ExecBufInstPtr);
	OUTPUT_TRIANGLE(0,1,3, 4);
	OUTPUT_TRIANGLE(1,2,3, 4);

    OP_STATE_RENDER(1, ExecBufInstPtr);
    STATE_DATA(D3DRENDERSTATE_ZFUNC, D3DCMP_LESSEQUAL, ExecBufInstPtr);

	if (NumVertices > (MaxVerticesInExecuteBuffer-12)) 
	{
	   WriteEndCodeToExecuteBuffer();
  	   UnlockExecuteBufferAndPrepareForUse();
	   ExecuteBuffer();
  	   LockExecuteBuffer();
	}

}
void DrawScanlinesOverlay(float level)
{
//	return;
	{
		D3DTLVERTEX *vertexPtr = &((LPD3DTLVERTEX)ExecuteBufferDataArea)[NumVertices];
		
		float u = 0.0f;//FastRandom()&255;
		float v = 128.0f;//FastRandom()&255;
		int c = 255;
		extern float CameraZoomScale;
		int t;
	   	f2i(t,64.0f+level*64.0f);
		
		float size = 128.0f*(1.0f-level*0.8f);//*CameraZoomScale;

	  	vertexPtr->sx =	Global_VDB_Ptr->VDB_ClipRight;
	  	vertexPtr->sy =	Global_VDB_Ptr->VDB_ClipUp;
		vertexPtr->sz = 1.0f;
		vertexPtr->rhw = 1.0f;
		vertexPtr->tu = (v-size)/256.0f;
		vertexPtr->tv = 1.0f;
		vertexPtr->color = RGBALIGHT_MAKE(c,c,c,t);
		vertexPtr++;
	  	vertexPtr->sx =	Global_VDB_Ptr->VDB_ClipLeft;
	  	vertexPtr->sy =	Global_VDB_Ptr->VDB_ClipUp;
		vertexPtr->sz = 1.0f;
		vertexPtr->rhw = 1.0f;
		vertexPtr->tu = (v-size)/256.0f;
		vertexPtr->tv = 1.0f;
		vertexPtr->color = RGBALIGHT_MAKE(c,c,c,t);
		vertexPtr++;
	  	vertexPtr->sx =	Global_VDB_Ptr->VDB_ClipLeft;
	  	vertexPtr->sy =	Global_VDB_Ptr->VDB_ClipDown;
		vertexPtr->sz = 1.0f;
		vertexPtr->rhw = 1.0f;
		vertexPtr->tu = (v+size)/256.0f;
		vertexPtr->tv = 1.0f;
		vertexPtr->color = RGBALIGHT_MAKE(c,c,c,t);
		vertexPtr++;
	  	vertexPtr->sx =	Global_VDB_Ptr->VDB_ClipRight;
	  	vertexPtr->sy =	Global_VDB_Ptr->VDB_ClipDown;
		vertexPtr->sz = 1.0f;
		vertexPtr->rhw = 1.0f;
		vertexPtr->tu = (v+size)/256.0f;
		vertexPtr->tv = 1.0f;
		vertexPtr->color = RGBALIGHT_MAKE(c,c,c,t);

		NumVertices+=4;
	}
	CheckTranslucencyModeIsCorrect(TRANSLUCENCY_NORMAL);
	CheckFilteringModeIsCorrect(FILTERING_BILINEAR_ON);
    OP_STATE_RENDER(1, ExecBufInstPtr);
    STATE_DATA(D3DRENDERSTATE_ZFUNC, D3DCMP_ALWAYS, ExecBufInstPtr);

 	NoiseTextureHandle = (D3DTEXTUREHANDLE)ImageHeaderArray[PredatorNumbersImageNumber].D3DHandle;
    if (CurrTextureHandle != NoiseTextureHandle)
	{
    	OP_STATE_RENDER(1, ExecBufInstPtr);
        STATE_DATA(D3DRENDERSTATE_TEXTUREHANDLE, NoiseTextureHandle, ExecBufInstPtr);
        CurrTextureHandle = NoiseTextureHandle;
	}
    if (D3DTexturePerspective != No)
	{
		D3DTexturePerspective = No;
		OP_STATE_RENDER(1, ExecBufInstPtr);
		STATE_DATA(D3DRENDERSTATE_TEXTUREPERSPECTIVE, FALSE, ExecBufInstPtr);
	}


	OP_TRIANGLE_LIST(2, ExecBufInstPtr);
	OUTPUT_TRIANGLE(0,1,3, 4);
	OUTPUT_TRIANGLE(1,2,3, 4);

    OP_STATE_RENDER(1, ExecBufInstPtr);
    STATE_DATA(D3DRENDERSTATE_ZFUNC, D3DCMP_LESSEQUAL, ExecBufInstPtr);

	if (NumVertices > (MaxVerticesInExecuteBuffer-12)) 
	{
	   WriteEndCodeToExecuteBuffer();
  	   UnlockExecuteBufferAndPrepareForUse();
	   ExecuteBuffer();
  	   LockExecuteBuffer();
	}

	if (level==1.0f) DrawNoiseOverlay(128);
}
void DrawPredatorVisionChangeOverlay(int time)
{
//	return;
	{
	if (time>ONE_FIXED)
	for(int i=0; i<8; i++)
	{
		D3DTLVERTEX *vertexPtr = &((LPD3DTLVERTEX)ExecuteBufferDataArea)[NumVertices];
		int t = (time-ONE_FIXED-((i+3)&7)*2048)*2;		  
		if (t<0) t = 0;
		
		float rightU;
		float leftU;
		if (t>ONE_FIXED)
		{
			leftU = 0.0f;
			rightU = 0.5f;
		}	
		else if (t>ONE_FIXED/2)
		{
			leftU = 1.0f - ((float)(t))/65536.0f;
			rightU = 1.0f - ((float)(t-ONE_FIXED/2))/65536.0f;
		}
		else
		{
			leftU = 1.0f - ((float)(t))/65536.0f;
			rightU = 1.0f;
		}

		float topV = 44.0f/128.0f;
		float bottomV = 119.0f/128.0f;
		if (leftU<0) leftU=0;
		PrintDebuggingText("%f %f\n",leftU,rightU);
		int rightX = MUL_FIXED((Global_VDB_Ptr->VDB_ClipRight-Global_VDB_Ptr->VDB_ClipLeft),t)+Global_VDB_Ptr->VDB_ClipLeft;
		if (rightX>Global_VDB_Ptr->VDB_ClipRight) rightX = Global_VDB_Ptr->VDB_ClipRight;
		int y = i*60;
		int h = 60;//+MUL_FIXED(10,time-ONE_FIXED);
		int c = 255;

	  	vertexPtr->sx =	Global_VDB_Ptr->VDB_ClipLeft;
	  	vertexPtr->sy =	y;
		vertexPtr->sz = 0;
		vertexPtr->rhw = 1.0f;
		vertexPtr->tu = leftU;
		vertexPtr->tv = topV;
		vertexPtr->color = RGBALIGHT_MAKE(c,c,c,255);
		vertexPtr->specular = RGBALIGHT_MAKE(0,0,0,255);
		vertexPtr++;
	  	vertexPtr->sx =	rightX;
	  	vertexPtr->sy =	y;
		vertexPtr->sz = 0;
		vertexPtr->rhw = 1.0f;
		vertexPtr->tu = rightU;
		vertexPtr->tv = topV;
		vertexPtr->color = RGBALIGHT_MAKE(c,c,c,255);
		vertexPtr->specular = RGBALIGHT_MAKE(0,0,0,255);
		vertexPtr++;
	  	vertexPtr->sx =	rightX;
	  	vertexPtr->sy = y+h;
		vertexPtr->sz = 0;
		vertexPtr->rhw = 1.0f;
		vertexPtr->tu = rightU;
		vertexPtr->tv = bottomV;
		vertexPtr->color = RGBALIGHT_MAKE(c,c,c,255);
		vertexPtr->specular = RGBALIGHT_MAKE(0,0,0,255);
		vertexPtr++;
	  	vertexPtr->sx =	Global_VDB_Ptr->VDB_ClipLeft;
	  	vertexPtr->sy =	y+h;
		vertexPtr->sz = 0;
		vertexPtr->rhw = 1.0f;
		vertexPtr->tu = leftU;
		vertexPtr->tv = bottomV;
		vertexPtr->color = RGBALIGHT_MAKE(255,255,255,255);
		vertexPtr->specular = RGBALIGHT_MAKE(0,0,0,255);

		NumVertices+=4;
		CheckTranslucencyModeIsCorrect(TRANSLUCENCY_COLOUR);
		CheckFilteringModeIsCorrect(FILTERING_BILINEAR_OFF);

	 //	NoiseTextureHandle = 0;
		D3DTEXTUREHANDLE TextureHandle = (D3DTEXTUREHANDLE)ImageHeaderArray[PredatorVisionChangeImageNumber].D3DHandle;
	    if (CurrTextureHandle != TextureHandle)
		{
	    	OP_STATE_RENDER(1, ExecBufInstPtr);
	        STATE_DATA(D3DRENDERSTATE_TEXTUREHANDLE, TextureHandle, ExecBufInstPtr);
	        CurrTextureHandle = TextureHandle;
		}

		OP_TRIANGLE_LIST(2, ExecBufInstPtr);
		OUTPUT_TRIANGLE(0,1,3, 4);
		OUTPUT_TRIANGLE(1,2,3, 4);
		if (NumVertices > (MaxVerticesInExecuteBuffer-12)) 
		{
		   WriteEndCodeToExecuteBuffer();
	  	   UnlockExecuteBufferAndPrepareForUse();
		   ExecuteBuffer();
	  	   LockExecuteBuffer();
		}
	}
	}
	
	{
	if (time>ONE_FIXED+8192) time = ONE_FIXED+8192;
	{
		D3DTLVERTEX *vertexPtr = &((LPD3DTLVERTEX)ExecuteBufferDataArea)[NumVertices];
		  
		
		float rightU = 1.0f;
		float leftU = 1.0f - ((float)time)/65536.0f;
		float v = 0;
		if (leftU<0) leftU=0;
		PrintDebuggingText("%f %f\n",leftU,rightU);
		int rightX = MUL_FIXED((Global_VDB_Ptr->VDB_ClipRight-Global_VDB_Ptr->VDB_ClipLeft),time)+Global_VDB_Ptr->VDB_ClipLeft;
		int c=128;

		int size = 256/4;
	  	vertexPtr->sx =	Global_VDB_Ptr->VDB_ClipLeft;
	  	vertexPtr->sy =	Global_VDB_Ptr->VDB_ClipUp;
		vertexPtr->sz = 0.001;
		vertexPtr->rhw = 1.0f;
		vertexPtr->tu = leftU;
		vertexPtr->tv = v/256.0;
		vertexPtr->color = RGBALIGHT_MAKE(c,c,c,255);
		vertexPtr->specular = 0;
		vertexPtr++;
	  	vertexPtr->sx =	rightX;
	  	vertexPtr->sy =	Global_VDB_Ptr->VDB_ClipUp;
		vertexPtr->sz = 0.001;
		vertexPtr->rhw = 1.0f;
		vertexPtr->tu = rightU;
		vertexPtr->tv = v/256.0;
		vertexPtr->color = RGBALIGHT_MAKE(c,c,c,255);
		vertexPtr->specular = 0;
		vertexPtr++;
	  	vertexPtr->sx =	rightX;
	  	vertexPtr->sy =	Global_VDB_Ptr->VDB_ClipDown;
		vertexPtr->sz = 0.001;
		vertexPtr->rhw = 1.0f;
		vertexPtr->tu = rightU;
		vertexPtr->tv = (v+size)/256.0;
		vertexPtr->color = RGBALIGHT_MAKE(c,c,c,255);
		vertexPtr->specular = 0;
		vertexPtr++;
	  	vertexPtr->sx =	Global_VDB_Ptr->VDB_ClipLeft;
	  	vertexPtr->sy =	Global_VDB_Ptr->VDB_ClipDown;
		vertexPtr->sz = 0.001;
		vertexPtr->rhw = 1.0f;
		vertexPtr->tu = leftU;
		vertexPtr->tv = (v+size)/256.0;
		vertexPtr->color = RGBALIGHT_MAKE(c,c,c,255);
		vertexPtr->specular = 0;

		NumVertices+=4;
	}
	CheckTranslucencyModeIsCorrect(TRANSLUCENCY_OFF);
	CheckFilteringModeIsCorrect(FILTERING_BILINEAR_ON);

 //	NoiseTextureHandle = 0;
	D3DTEXTUREHANDLE TextureHandle = (D3DTEXTUREHANDLE)ImageHeaderArray[PredatorVisionChangeImageNumber].D3DHandle;
    if (CurrTextureHandle != TextureHandle)
	{
    	OP_STATE_RENDER(1, ExecBufInstPtr);
        STATE_DATA(D3DRENDERSTATE_TEXTUREHANDLE, TextureHandle, ExecBufInstPtr);
        CurrTextureHandle = TextureHandle;
	}

	OP_TRIANGLE_LIST(2, ExecBufInstPtr);
	OUTPUT_TRIANGLE(0,1,3, 4);
	OUTPUT_TRIANGLE(1,2,3, 4);
	if (NumVertices > (MaxVerticesInExecuteBuffer-12)) 
	{
	   WriteEndCodeToExecuteBuffer();
  	   UnlockExecuteBufferAndPrepareForUse();
	   ExecuteBuffer();
  	   LockExecuteBuffer();
	}
	}

}
#if 0
#define OCTAVES 1
float u[OCTAVES];
float v[OCTAVES];
float du[OCTAVES];
float dv[OCTAVES];
int setup=0;
void DrawFBM(void)
{
	extern int CloudyImageNumber;
	D3DTEXTUREHANDLE TextureHandle = (D3DTEXTUREHANDLE)ImageHeaderArray[CloudyImageNumber].D3DHandle;
//	return;		   1
	int i;
	int size = 256;
//	float u = FastRandom()&255;
//	float v = FastRandom()&255;
	int t =255;
	if(!setup)
	{
		setup=1;
		for(i=OCTAVES-1;i>=0;i--)
		{
			du[i] = ( (float)((FastRandom()&65535)-32768)*(i+1) )/16384.0;
			dv[i] = ( (float)((FastRandom()&65535)-32768)*(i+1) )/16384.0;
		}											   
	}
	
	float timeScale = ((float)NormalFrameTime)/65536.0;
	for(i=0; i<OCTAVES; i++)
	{
		u[i]+=du[i]*timeScale;
		v[i]+=dv[i]*timeScale;
		#if 1
		{
			MATRIXCH mat = (Global_VDB_Ptr->VDB_Mat);
			VECTORCH v[4]=
			{
				{-ONE_FIXED,-ONE_FIXED,ONE_FIXED},
				{ ONE_FIXED,-ONE_FIXED,ONE_FIXED},
				{ ONE_FIXED, 0,	ONE_FIXED},
				{-ONE_FIXED, 0,	ONE_FIXED},
			};				 
			TransposeMatrixCH(&mat);
			RotateVector(&v[0],&mat);
			RotateVector(&v[1],&mat);
			RotateVector(&v[2],&mat);
			RotateVector(&v[3],&mat);
			if (v[0].vy=0) v[0].vy=-1;
			if (v[1].vy=0) v[1].vy=-1;
			if (v[2].vy=0) v[2].vy=-1;
			if (v[3].vy=0) v[3].vy=-1;
			if (v[0].vy>0) v[0].vy=-v[0].vy;
			if (v[1].vy>0) v[1].vy=-v[1].vy;
			if (v[2].vy>0) v[2].vy=-v[2].vy;
			if (v[3].vy>0) v[3].vy=-v[3].vy;

			D3DTLVERTEX *vertexPtr = &((LPD3DTLVERTEX)ExecuteBufferDataArea)[NumVertices];
			  
			vertexPtr->sx =	Global_VDB_Ptr->VDB_ClipLeft;
		  	vertexPtr->sy =	Global_VDB_Ptr->VDB_ClipUp;
			vertexPtr->sz = 1;
			vertexPtr->tu = -((float)v[0].vx)/((float)v[0].vy)/1.0;
			vertexPtr->tv = -((float)v[0].vz)/((float)v[0].vy)/1.0;
			vertexPtr->color = RGBALIGHT_MAKE(200,255,255,t);
			vertexPtr->specular = RGBALIGHT_MAKE(0,0,0,255);
			vertexPtr++;
		  	vertexPtr->sx =	Global_VDB_Ptr->VDB_ClipRight;
		  	vertexPtr->sy =	Global_VDB_Ptr->VDB_ClipUp;
			vertexPtr->sz = 1;
			vertexPtr->tu = -((float)v[1].vx)/((float)v[1].vy)/1.0;
			vertexPtr->tv = -((float)v[1].vz)/((float)v[1].vy)/1.0;
			vertexPtr->color = RGBALIGHT_MAKE(200,255,255,t);
			vertexPtr->specular = RGBALIGHT_MAKE(0,0,0,255);
			vertexPtr++;
		  	vertexPtr->sx =	Global_VDB_Ptr->VDB_ClipRight;
		  	vertexPtr->sy =	(Global_VDB_Ptr->VDB_ClipUp+Global_VDB_Ptr->VDB_ClipDown)/2;
			vertexPtr->sz = 1;
			vertexPtr->tu = -((float)v[2].vx)/((float)v[2].vy)/1.0;
			vertexPtr->tv = -((float)v[2].vz)/((float)v[2].vy)/1.0;
			vertexPtr->color = RGBALIGHT_MAKE(200,255,255,t);
			vertexPtr->specular = RGBALIGHT_MAKE(0,0,0,255);
			vertexPtr++;
		  	vertexPtr->sx =	Global_VDB_Ptr->VDB_ClipLeft;
		  	vertexPtr->sy =	(Global_VDB_Ptr->VDB_ClipUp+Global_VDB_Ptr->VDB_ClipDown)/2;
			vertexPtr->sz = 1;
			vertexPtr->tu = -((float)v[3].vx)/((float)v[3].vy)/1.0;
			vertexPtr->tv = -((float)v[3].vz)/((float)v[3].vy)/1.0;
			vertexPtr->color = RGBALIGHT_MAKE(200,255,255,t);
			vertexPtr->specular = RGBALIGHT_MAKE(0,0,0,255);

			NumVertices+=4;
		}
		
		#else
		{
			D3DTLVERTEX *vertexPtr = &((LPD3DTLVERTEX)ExecuteBufferDataArea)[NumVertices];
			  
			vertexPtr->sx =	Global_VDB_Ptr->VDB_ClipLeft;
		  	vertexPtr->sy =	Global_VDB_Ptr->VDB_ClipUp;
			vertexPtr->sz = 1;
			vertexPtr->tu = u[i]/256.0;
			vertexPtr->tv = v[i]/256.0;
			vertexPtr->color = RGBALIGHT_MAKE(200,255,255,t);
			vertexPtr->specular = RGBALIGHT_MAKE(0,0,0,255);
			vertexPtr++;
		  	vertexPtr->sx =	Global_VDB_Ptr->VDB_ClipRight;
		  	vertexPtr->sy =	Global_VDB_Ptr->VDB_ClipUp;
			vertexPtr->sz = 1;
			vertexPtr->tu = (u[i]+size)/256.0;
			vertexPtr->tv = v[i]/256.0;
			vertexPtr->color = RGBALIGHT_MAKE(200,255,255,t);
			vertexPtr->specular = RGBALIGHT_MAKE(0,0,0,255);
			vertexPtr++;
		  	vertexPtr->sx =	Global_VDB_Ptr->VDB_ClipRight;
		  	vertexPtr->sy =	Global_VDB_Ptr->VDB_ClipDown;
			vertexPtr->sz = 1;
			vertexPtr->tu = (u[i]+size)/256.0;
			vertexPtr->tv = (v[i]+size)/256.0;
			vertexPtr->color = RGBALIGHT_MAKE(200,255,255,t);
			vertexPtr->specular = RGBALIGHT_MAKE(0,0,0,255);
			vertexPtr++;
		  	vertexPtr->sx =	Global_VDB_Ptr->VDB_ClipLeft;
		  	vertexPtr->sy =	Global_VDB_Ptr->VDB_ClipDown;
			vertexPtr->sz = 1;
			vertexPtr->tu = u[i]/256.0;
			vertexPtr->tv = (v[i]+size)/256.0;
			vertexPtr->color = RGBALIGHT_MAKE(200,255,255,t);
			vertexPtr->specular = RGBALIGHT_MAKE(0,0,0,255);

			NumVertices+=4;
		}
		#endif
		size*=2;
		t/=2;
		CheckTranslucencyModeIsCorrect(TRANSLUCENCY_GLOWING);
		CheckFilteringModeIsCorrect(FILTERING_BILINEAR_ON);

	 //	NoiseTextureHandle = 0;
	    if (CurrTextureHandle != TextureHandle)
		{
	    	OP_STATE_RENDER(1, ExecBufInstPtr);
	        STATE_DATA(D3DRENDERSTATE_TEXTUREHANDLE, TextureHandle, ExecBufInstPtr);
	        CurrTextureHandle = NoiseTextureHandle;
		}
	    if (D3DTexturePerspective != No)
		{
			D3DTexturePerspective = No;
			OP_STATE_RENDER(1, ExecBufInstPtr);
			STATE_DATA(D3DRENDERSTATE_TEXTUREPERSPECTIVE, FALSE, ExecBufInstPtr);
		}


		OP_TRIANGLE_LIST(2, ExecBufInstPtr);
		OUTPUT_TRIANGLE(0,1,3, 4);
		OUTPUT_TRIANGLE(1,2,3, 4);
		if (NumVertices > (MaxVerticesInExecuteBuffer-12)) 
		{
		   WriteEndCodeToExecuteBuffer();
	  	   UnlockExecuteBufferAndPrepareForUse();
		   ExecuteBuffer();
	  	   LockExecuteBuffer();
		}
	}

}
#endif
void D3D_SkyPolygon_Output(POLYHEADER *inputPolyPtr,RENDERVERTEX *renderVerticesPtr)
{
	int flags;
	int texoffset;

	D3DTEXTUREHANDLE TextureHandle;

	float ZNear;
	float RecipW, RecipH;

    // Get ZNear
	ZNear = (float) (Global_VDB_Ptr->VDB_ClipZ * GlobalScale);


	// Take header information
	flags = inputPolyPtr->PolyFlags;

	// We assume bit 15 (TxLocal) HAS been
	// properly cleared this time...
	texoffset = (inputPolyPtr->PolyColour & ClrTxDefn);

	TextureHandle = (D3DTEXTUREHANDLE)ImageHeaderArray[texoffset].D3DHandle;

    // Check for textures that have not loaded
	// properly

//    if (TextureHandle == (D3DTEXTUREHANDLE) 0)
 //	  return;

	#ifdef AVP_DEBUG_VERSION	
	if(ImageHeaderArray[texoffset].ImageWidth==128)
	{
		RecipW = (1.0 /128.0)/65536.0f;
	}
	else
	{
		float width = (float) ImageHeaderArray[texoffset].ImageWidth;
		RecipW = (1.0 / width)/65536.0f;
	}
	if(ImageHeaderArray[texoffset].ImageHeight==128)
	{
		RecipH = (1.0 / 128.0)/65536.0f;
	}
	else
	{
		float height = (float) ImageHeaderArray[texoffset].ImageHeight;
		RecipH = (1.0 / height)/65536.0f;
	}
	#else
	RecipW = (1.0/65536.0)/128.0;
	RecipH = (1.0/65536.0)/128.0;
	#endif
//	RecipW = (1.0/65536.0)/(float) ImageHeaderArray[texoffset].ImageWidth;
//	RecipH = (1.0/65536.0)/(float) ImageHeaderArray[texoffset].ImageHeight;

	/* OUTPUT VERTICES TO EXECUTE BUFFER */
	{
		int i = RenderPolygon.NumberOfVertices;
		RENDERVERTEX *vertices = renderVerticesPtr;

		do
		{
			D3DTLVERTEX *vertexPtr = &((LPD3DTLVERTEX)ExecuteBufferDataArea)[NumVertices];
		  	float oneOverZ;
		  	oneOverZ = (1.0)/(vertices->Z);
			float zvalue;

			vertexPtr->tu = ((float)vertices->U) * RecipW + (1.0/256.0);
			vertexPtr->tv = ((float)vertices->V) * RecipH + (1.0/256.0);
			vertexPtr->rhw = oneOverZ;

			{
				zvalue = vertices->Z+HeadUpDisplayZOffset;
	   //			zvalue /= 65536.0;
	   		   	zvalue = 1.0 - ZNear/zvalue;
			}	
			
			{
				int x = (vertices->X*(Global_VDB_Ptr->VDB_ProjX+1))/vertices->Z+Global_VDB_Ptr->VDB_CentreX;

				#if 1
				if (x<Global_VDB_Ptr->VDB_ClipLeft)
				{
					x=Global_VDB_Ptr->VDB_ClipLeft;
				}	
				else if (x>Global_VDB_Ptr->VDB_ClipRight)
				{
					x=Global_VDB_Ptr->VDB_ClipRight;	
				}
				#endif
				vertexPtr->sx=x;
			}
			{
				int y = (vertices->Y*(Global_VDB_Ptr->VDB_ProjY+1))/vertices->Z+Global_VDB_Ptr->VDB_CentreY;
				
				#if 1
				if (y<Global_VDB_Ptr->VDB_ClipUp)
				{
					y=Global_VDB_Ptr->VDB_ClipUp;
				}
				else if (y>Global_VDB_Ptr->VDB_ClipDown)
				{
					y=Global_VDB_Ptr->VDB_ClipDown;	
				}
				#endif
				vertexPtr->sy=y;
				
			}
//			vertexPtr->tu = ((float)(vertices->U>>16)+0.5) * RecipW;
//			vertexPtr->tv = ((float)(vertices->V>>16)+0.5) * RecipH;
//			vertexPtr->rhw = oneOverZ;

	
	  		vertexPtr->color = RGBALIGHT_MAKE(vertices->R,vertices->G,vertices->B,vertices->A);

			vertexPtr->sz = 1.0;
			vertexPtr->specular=RGBALIGHT_MAKE(0,0,0,255);
			vertices++;
			NumVertices++;
		}
	  	while(--i);
	}

	CheckTranslucencyModeIsCorrect(RenderPolygon.TranslucencyMode);
	// Insert state change for shading model if required
    if (D3DShadingMode != D3DSHADE_GOURAUD)
	  {
   	   D3DShadingMode = D3DSHADE_GOURAUD;
	   OP_STATE_RENDER(1, ExecBufInstPtr);
	     STATE_DATA(D3DRENDERSTATE_SHADEMODE,
		       D3DSHADE_GOURAUD, ExecBufInstPtr);
	  }

// Insert state change for texturing perspective value
// Note that drawtx3das2d options have ONLY been allowed for here,
// not when the rhw values are generated.  This is a deliberate choice,
// based on the assumption that drawtx3das2d will not be used very often
// and the extra branching at the top of this function will impose a 
// greater cost than the (rare) savings in floating pt divisions are worth.
// Or so I claim...

    if (D3DTexturePerspective != Yes)
    {
		D3DTexturePerspective = Yes;
		OP_STATE_RENDER(1, ExecBufInstPtr);
		STATE_DATA(D3DRENDERSTATE_TEXTUREPERSPECTIVE, TRUE, ExecBufInstPtr);
	}

	#if FMV_EVERYWHERE
	TextureHandle = FMVTextureHandle[0];
	#endif

    if (TextureHandle != CurrTextureHandle)
	{
    	OP_STATE_RENDER(1, ExecBufInstPtr);
        STATE_DATA(D3DRENDERSTATE_TEXTUREHANDLE, TextureHandle, ExecBufInstPtr);
	   	CurrTextureHandle = TextureHandle;
	}


	D3D_OutputTriangles();
}

void D3D_DrawFMVOnWater(int xOrigin, int yOrigin, int zOrigin)
{
	int colour;
	switch (CurrentVisionMode)
	{
		default:
		case VISION_MODE_NORMAL:
		{
			colour = RGBLIGHT_MAKE(128,128,128);
			break;
		}
		case VISION_MODE_IMAGEINTENSIFIER:
		{
			colour = RGBLIGHT_MAKE(0,255,0);
			break;
		}
		case VISION_MODE_PRED_THERMAL:
		case VISION_MODE_PRED_SEEALIENS:
		case VISION_MODE_PRED_SEEPREDTECH:
		{
			colour = RGBLIGHT_MAKE(128,0,128);
		  	break;
		}
	}
	int i=0;
	int x;
	for (x=0; x<16; x++)
	{
		int z;
		for(z=0; z<16; z++)
		{
			VECTORCH *point = &MeshVertex[i];
					
			point->vz = zOrigin + (z*MeshZScale)/15;
			point->vz += (FastRandom()&31);
		
			point->vx = (x*MeshXScale)/15;
			point->vx -= MeshXScale/2;
			point->vx = (point->vx * (32-z))/16 + (FastRandom()&31);
			point->vx += xOrigin + MeshXScale/2;

			int offset = EffectOfRipples(point);
	 		point->vy = yOrigin+offset;


			{
				int alpha = 192-offset*4;
				if (alpha>255) alpha = 255;
				if (alpha<128) alpha = 128;
				MeshVertexColour[i] = colour + (alpha<<24);
			}
			
			#if 1
			TranslatePointIntoViewspace(point);
			#else
			point->vx -= Global_VDB_Ptr->VDB_World.vx;
			point->vy -= Global_VDB_Ptr->VDB_World.vy;
			point->vz -= Global_VDB_Ptr->VDB_World.vz;
			MeshWorldVertex[i] = *point;
			RotateVector(point,&(Global_VDB_Ptr->VDB_Mat));
			point->vy = MUL_FIXED(point->vy,87381);

			#endif
			/* is particle within normal view frustrum ? */
			if(AvP.PlayerType==I_Alien)	/* wide frustrum */
			{
				if(( (-point->vx <= point->vz*2)
		   			&&(point->vx <= point->vz*2)
					&&(-point->vy <= point->vz*2)
					&&(point->vy <= point->vz*2) ))
				{
					MeshVertexOutcode[i]=1;
				}
				else
				{
					MeshVertexOutcode[i]=0;
				}
			}
			else
			{
				if(( (-point->vx <= point->vz)
		   			&&(point->vx <= point->vz)
					&&(-point->vy <= point->vz)
					&&(point->vy <= point->vz) ))
				{
					MeshVertexOutcode[i]=1;
				}
				else
				{
					MeshVertexOutcode[i]=0;
				}
			}

			{
				// u 
				MeshWorldVertex[i].vx = (x*4096)/15;
				// v
				MeshWorldVertex[i].vy = (z*2048)/20;
				
			}
		   	i++;
		}
	}

	D3DTEXTUREHANDLE TextureHandle = FMVTextureHandle[0];
	if (CurrTextureHandle != TextureHandle)
	{
		OP_STATE_RENDER(1, ExecBufInstPtr);
		STATE_DATA(D3DRENDERSTATE_TEXTUREHANDLE, TextureHandle, ExecBufInstPtr);
		CurrTextureHandle = TextureHandle;
	}
	CheckTranslucencyModeIsCorrect(TRANSLUCENCY_GLOWING);
	if (NumVertices)
	{
	   WriteEndCodeToExecuteBuffer();
  	   UnlockExecuteBufferAndPrepareForUse();
	   ExecuteBuffer();
  	   LockExecuteBuffer();
	}
	OP_STATE_RENDER(1, ExecBufInstPtr);
    STATE_DATA(D3DRENDERSTATE_ZWRITEENABLE, FALSE, ExecBufInstPtr);

	if ((MeshVertexOutcode[0]&&MeshVertexOutcode[15]&&MeshVertexOutcode[240]&&MeshVertexOutcode[255]))
	{
		D3D_DrawMoltenMetalMesh_Unclipped();
	}	
	else
//	else if (MeshVertexOutcode[0]||MeshVertexOutcode[15]||MeshVertexOutcode[240]||MeshVertexOutcode[255])
	{
		D3D_DrawMoltenMetalMesh_Clipped();
	}
		
	OP_STATE_RENDER(1, ExecBufInstPtr);
    STATE_DATA(D3DRENDERSTATE_ZWRITEENABLE, TRUE, ExecBufInstPtr);
	
}

void D3D_DrawMoltenMetal(int xOrigin, int yOrigin, int zOrigin)
{
	int i=0;
	int x;
	for (x=0; x<16; x++)
	{
		int z;
		for(z=0; z<16; z++)
		{
			VECTORCH *point = &MeshVertex[i];
			
			point->vx = xOrigin+(x*MeshXScale)/15;
			point->vz = zOrigin+(z*MeshZScale)/15;
		 #if 0
			
			int offset=0;

		 	offset = MUL_FIXED(32,GetSin(  (point->vx+point->vz+CloakingPhase)&4095 ) );
		 	offset += MUL_FIXED(16,GetSin(  (point->vx-point->vz*2+CloakingPhase/2)&4095 ) );
			{
				float dx=point->vx-22704;
				float dz=point->vz+20652;
				float a = dx*dx+dz*dz;
				a=sqrt(a);

				offset+= MUL_FIXED(200,GetSin( (((int)a-CloakingPhase)&4095)  ));
			}
		#endif
		 #if 1
			int offset=0;

			/* basic noise ripples */
		 	offset = MUL_FIXED(128,GetSin(  ((point->vx+point->vz)/16+CloakingPhase)&4095 ) );
		 	offset += MUL_FIXED(64,GetSin(  ((point->vx-point->vz*2)/4+CloakingPhase/2)&4095 ) );
		 	offset += MUL_FIXED(64,GetSin(  ((point->vx*5-point->vz)/32+CloakingPhase/5)&4095 ) );

		#endif
			if (offset>450) offset = 450;
			if (offset<-1000) offset = -1000;
			point->vy = yOrigin+offset;

			{
				int shade = 191+(offset+256)/8;
				MeshVertexColour[i] = RGBLIGHT_MAKE(shade,shade,shade);
			}
			
			#if 1
			TranslatePointIntoViewspace(point);
			#else
			point->vx -= Global_VDB_Ptr->VDB_World.vx;
			point->vy -= Global_VDB_Ptr->VDB_World.vy;
			point->vz -= Global_VDB_Ptr->VDB_World.vz;
			MeshWorldVertex[i] = *point;
			RotateVector(point,&(Global_VDB_Ptr->VDB_Mat));
			point->vy = MUL_FIXED(point->vy,87381);

			#endif
			/* is particle within normal view frustrum ? */
			if(AvP.PlayerType==I_Alien)	/* wide frustrum */
			{
				if(( (-point->vx <= point->vz*2)
		   			&&(point->vx <= point->vz*2)
					&&(-point->vy <= point->vz*2)
					&&(point->vy <= point->vz*2) ))
				{
					MeshVertexOutcode[i]=1;
				}
				else
				{
					MeshVertexOutcode[i]=0;
				}
			}
			else
			{
				if(( (-point->vx <= point->vz)
		   			&&(point->vx <= point->vz)
					&&(-point->vy <= point->vz)
					&&(point->vy <= point->vz) ))
				{
					MeshVertexOutcode[i]=1;
				}
				else
				{
					MeshVertexOutcode[i]=0;
				}
			}

			#if 0
			{
				// v
				MeshWorldVertex[i].vy = (offset+256)*4;
				// u 
				MeshWorldVertex[i].vx = ((MeshWorldVertex[i].vx)&4095);
				
			}
			#else
			{
				Normalise(&MeshWorldVertex[i]);
				// v
				int theta = (MeshWorldVertex[i].vy+offset);
				if (theta<0) theta=0;
				if (theta>ONE_FIXED) theta=ONE_FIXED;

				// u 
				int arctan = ((atan2((double)MeshWorldVertex[i].vx,(double)MeshWorldVertex[i].vz)/ 6.28318530718))*4095;
				MeshWorldVertex[i].vx = (arctan+offset)&4095;

				MeshWorldVertex[i].vy = ArcCos(theta);
				
			}
			#endif


			i++;
		}
	}

	D3DTEXTUREHANDLE TextureHandle = (D3DTEXTUREHANDLE)ImageHeaderArray[StaticImageNumber].D3DHandle;
	if (CurrTextureHandle != TextureHandle)
	{
		OP_STATE_RENDER(1, ExecBufInstPtr);
		STATE_DATA(D3DRENDERSTATE_TEXTUREHANDLE, TextureHandle, ExecBufInstPtr);
		CurrTextureHandle = TextureHandle;
	}
	CheckTranslucencyModeIsCorrect(TRANSLUCENCY_OFF);
	if (NumVertices)
	{
	   WriteEndCodeToExecuteBuffer();
  	   UnlockExecuteBufferAndPrepareForUse();
	   ExecuteBuffer();
  	   LockExecuteBuffer();
	}
	if ((MeshVertexOutcode[0]&&MeshVertexOutcode[15]&&MeshVertexOutcode[240]&&MeshVertexOutcode[255]))
	{
		D3D_DrawMoltenMetalMesh_Unclipped();
	}	
	else
//	else if (MeshVertexOutcode[0]||MeshVertexOutcode[15]||MeshVertexOutcode[240]||MeshVertexOutcode[255])
	{
		D3D_DrawMoltenMetalMesh_Clipped();
	}
		
	
}

void D3D_DrawMoltenMetalMesh_Unclipped(void)
{
	float ZNear = (float) (Global_VDB_Ptr->VDB_ClipZ * GlobalScale);

	/* OUTPUT VERTICES TO EXECUTE BUFFER */
	{
		D3DTLVERTEX *vertexPtr = &((LPD3DTLVERTEX)ExecuteBufferDataArea)[NumVertices];
		VECTORCH *point = MeshVertex;

		VECTORCH *pointWS = MeshWorldVertex;

		int i;
		for (i=0; i<256; i++)
		{

			if (point->vz<=1) point->vz = 1;
			int x = (point->vx*(Global_VDB_Ptr->VDB_ProjX+1))/point->vz+Global_VDB_Ptr->VDB_CentreX;
			int y = (point->vy*(Global_VDB_Ptr->VDB_ProjY+1))/point->vz+Global_VDB_Ptr->VDB_CentreY;
  //			textprint("%d, %d\n",x,y);
			#if 1
			{
				if (x<Global_VDB_Ptr->VDB_ClipLeft)
				{
					x=Global_VDB_Ptr->VDB_ClipLeft;
				}	
				else if (x>Global_VDB_Ptr->VDB_ClipRight)
				{
					x=Global_VDB_Ptr->VDB_ClipRight;	
				}
				
				vertexPtr->sx=x;
			}
			{
				if (y<Global_VDB_Ptr->VDB_ClipUp)
				{
					y=Global_VDB_Ptr->VDB_ClipUp;
				}
				else if (y>Global_VDB_Ptr->VDB_ClipDown)
				{
					y=Global_VDB_Ptr->VDB_ClipDown;	
				}
				vertexPtr->sy=y;
			}
			#else
			vertexPtr->sx=x;
			vertexPtr->sy=y;
			#endif


			point->vz+=HeadUpDisplayZOffset;
		  	float oneOverZ = ((float)(point->vz)-ZNear)/(float)(point->vz);
		   	vertexPtr->color = MeshVertexColour[i];
			vertexPtr->specular = 0;
			vertexPtr->sz = oneOverZ;

			vertexPtr->tu = pointWS->vx*WaterUScale+(1.0f/256.0f);
			vertexPtr->tv =	pointWS->vy*WaterVScale+(1.0f/256.0f);
			vertexPtr->rhw = 1.0/point->vz;


			NumVertices++;
			vertexPtr++;
			point++;

			pointWS++;
		}
	}
 //	textprint("numvertices %d\n",NumVertices);
    
    
    /*
     * Make sure that the triangle data (not OP) will be QWORD aligned
     */
	if (QWORD_ALIGNED(ExecBufInstPtr))
    {
        OP_NOP(ExecBufInstPtr);
    }

  	OP_TRIANGLE_LIST(450, ExecBufInstPtr);
	/* CONSTRUCT POLYS */
	{
		int x;
		for (x=0; x<15; x++)
		{
			int y;
			for(y=0; y<15; y++)
			{
				OUTPUT_TRIANGLE(0+x+(16*y),1+x+(16*y),16+x+(16*y), 256);
				OUTPUT_TRIANGLE(1+x+(16*y),17+x+(16*y),16+x+(16*y), 256);
			}
		}
	}
	#if 1
	{
	   WriteEndCodeToExecuteBuffer();
  	   UnlockExecuteBufferAndPrepareForUse();
	   ExecuteBuffer();
  	   LockExecuteBuffer();
	}
	#endif
}
void D3D_DrawMoltenMetalMesh_Clipped(void)
{
	float ZNear = (float) (Global_VDB_Ptr->VDB_ClipZ * GlobalScale);

	/* OUTPUT VERTICES TO EXECUTE BUFFER */
	{
		D3DTLVERTEX *vertexPtr = &((LPD3DTLVERTEX)ExecuteBufferDataArea)[NumVertices];
		VECTORCH *point = MeshVertex;

		VECTORCH *pointWS = MeshWorldVertex;

		int i;
		for (i=0; i<256; i++)
		{
			{
				int z = point->vz;
				if (z<=0) z = 1;
				int x = (point->vx*(Global_VDB_Ptr->VDB_ProjX+1))/z+Global_VDB_Ptr->VDB_CentreX;
				int y = (point->vy*(Global_VDB_Ptr->VDB_ProjY+1))/z+Global_VDB_Ptr->VDB_CentreY;
				#if 1
				{
					if (x<Global_VDB_Ptr->VDB_ClipLeft)
					{
						x=Global_VDB_Ptr->VDB_ClipLeft;
					}	
					else if (x>Global_VDB_Ptr->VDB_ClipRight)
					{
						x=Global_VDB_Ptr->VDB_ClipRight;	
					}
					
					vertexPtr->sx=x;
				}
				{
					if (y<Global_VDB_Ptr->VDB_ClipUp)
					{
						y=Global_VDB_Ptr->VDB_ClipUp;
					}
					else if (y>Global_VDB_Ptr->VDB_ClipDown)
					{
						y=Global_VDB_Ptr->VDB_ClipDown;	
					}
					vertexPtr->sy=y;
				}
				#else
				vertexPtr->sx=x;
				vertexPtr->sy=y;
				#endif

				vertexPtr->tu = pointWS->vx*WaterUScale+(1.0f/256.0f);
				vertexPtr->tv =	pointWS->vy*WaterVScale+(1.0f/256.0f);
		
				point->vz+=HeadUpDisplayZOffset;
			  	float oneOverZ = ((float)(z)-ZNear)/(float)(z);
				vertexPtr->color = MeshVertexColour[i];
				vertexPtr->specular = 0;
				vertexPtr->sz = oneOverZ;
				vertexPtr->rhw = 1.0f/(float)z;

			}
			NumVertices++;
			vertexPtr++;
			point++;

			pointWS++;
		}
	}
//	textprint("numvertices %d\n",NumVertices);
	/* CONSTRUCT POLYS */
	{
//		OP_TRIANGLE_LIST(450, ExecBufInstPtr);
		int x;
		for (x=0; x<15; x++)
		{
			int y;
			for(y=0; y<15; y++)
			{
				int p1 = 0+x+(16*y);
				int p2 = 1+x+(16*y);
				int p3 = 16+x+(16*y);
				int p4 = 17+x+(16*y);

				#if 0
				if (MeshVertexOutcode[p1]&&MeshVertexOutcode[p2]&&MeshVertexOutcode[p3])
				{
					OP_TRIANGLE_LIST(1, ExecBufInstPtr);
					OUTPUT_TRIANGLE(p1,p2,p3, 256);
				}
				if (MeshVertexOutcode[p2]&&MeshVertexOutcode[p3]&&MeshVertexOutcode[p4])
				{
					OP_TRIANGLE_LIST(1, ExecBufInstPtr);
					OUTPUT_TRIANGLE(p2,p4,p3, 256);
				}	
				#else
				if (MeshVertexOutcode[p1]&&MeshVertexOutcode[p2]&&MeshVertexOutcode[p3]&&MeshVertexOutcode[p4])
				{
					OP_TRIANGLE_LIST(2, ExecBufInstPtr);
					OUTPUT_TRIANGLE(p1,p2,p3, 256);
					OUTPUT_TRIANGLE(p2,p4,p3, 256);
				}	

				#endif
			}
		}
	}
	#if 1
	{
	   WriteEndCodeToExecuteBuffer();
  	   UnlockExecuteBufferAndPrepareForUse();
	   ExecuteBuffer();
  	   LockExecuteBuffer();
	}
	#endif
	#if 1
	{
//		OP_TRIANGLE_LIST(450, ExecBufInstPtr);
		POLYHEADER fakeHeader;

		fakeHeader.PolyFlags = 0;
		fakeHeader.PolyColour = 0;
		RenderPolygon.TranslucencyMode = TRANSLUCENCY_NORMAL;
		int x;
		for (x=0; x<15; x++)
		{
			int y;
			for(y=0; y<15; y++)
			{
				int p[4];
				p[0] = 0+x+(16*y);
				p[1] = 1+x+(16*y);
				p[2] = 17+x+(16*y);
				p[3] = 16+x+(16*y);

				if (!(MeshVertexOutcode[p[0]]&&MeshVertexOutcode[p[1]]&&MeshVertexOutcode[p[2]]&&MeshVertexOutcode[p[3]]))
				{
				   {
						int i;
						for (i=0; i<4; i++) 
						{
							VerticesBuffer[i].X	= MeshVertex[p[i]].vx;
							VerticesBuffer[i].Y	= MeshVertex[p[i]].vy;
							VerticesBuffer[i].Z	= MeshVertex[p[i]].vz;
							VerticesBuffer[i].U = MeshWorldVertex[p[i]].vx*(WaterUScale*128.0f*65536.0f);
							VerticesBuffer[i].V = MeshWorldVertex[p[i]].vy*(WaterVScale*128.0f*65536.0f);
																   
							VerticesBuffer[i].A = (MeshVertexColour[p[i]]&0xff000000)>>24;
							VerticesBuffer[i].R = (MeshVertexColour[p[i]]&0x00ff0000)>>16;
							VerticesBuffer[i].G	= (MeshVertexColour[p[i]]&0x0000ff00)>>8;
							VerticesBuffer[i].B = MeshVertexColour[p[i]]&0x000000ff;
							VerticesBuffer[i].SpecularR = 0;
							VerticesBuffer[i].SpecularG = 0;
							VerticesBuffer[i].SpecularB = 0;
						}
						RenderPolygon.NumberOfVertices=4;
					}
					{
						int outcode = QuadWithinFrustrum();
														  
						if (outcode)
						{		 
							GouraudTexturedPolygon_ClipWithZ();
							if(RenderPolygon.NumberOfVertices<3) continue;
							GouraudTexturedPolygon_ClipWithNegativeX();
							if(RenderPolygon.NumberOfVertices<3) continue;
							GouraudTexturedPolygon_ClipWithPositiveY();
							if(RenderPolygon.NumberOfVertices<3) continue;
							GouraudTexturedPolygon_ClipWithNegativeY();
							if(RenderPolygon.NumberOfVertices<3) continue;
							GouraudTexturedPolygon_ClipWithPositiveX();
							if(RenderPolygon.NumberOfVertices<3) continue;
					   //	D3D_ZBufferedGouraudPolygon_Output(&fakeHeader,RenderPolygon.Vertices);
					   		D3D_ZBufferedGouraudTexturedPolygon_Output(&fakeHeader,RenderPolygon.Vertices);
						}
					}	
				}
			}
		}
	}
	#endif
	#if 1
	{
	   WriteEndCodeToExecuteBuffer();
  	   UnlockExecuteBufferAndPrepareForUse();
	   ExecuteBuffer();
  	   LockExecuteBuffer();
	}
	#endif
}

void *DynamicImagePtr;

#define NUMBER_OF_SMACK_SURFACES 4
LPDIRECTDRAWSURFACE SrcDDSurface[NUMBER_OF_SMACK_SURFACES];
LPDIRECT3DTEXTURE SrcTexture[NUMBER_OF_SMACK_SURFACES];
void *SrcSurfacePtr[NUMBER_OF_SMACK_SURFACES];

LPDIRECTDRAWSURFACE DstDDSurface[NUMBER_OF_SMACK_SURFACES]={0,0,0};
LPDIRECT3DTEXTURE DstTexture[NUMBER_OF_SMACK_SURFACES]={0,0,0};
int CurrentSurface;

void InitDrawTest(void)
{
#if FMV_ON
	DDSURFACEDESC ddsd;
	int surface = NUMBER_OF_SMACK_SURFACES;

	while(surface--)
	{
		memcpy(&ddsd, &(d3d.TextureFormat[d3d.CurrentTextureFormat].ddsd), sizeof(ddsd));

		ddsd.dwSize = sizeof(ddsd);

		ddsd.dwFlags = (DDSD_CAPS | DDSD_HEIGHT | DDSD_WIDTH | DDSD_PIXELFORMAT);
			ddsd.ddsCaps.dwCaps = (DDSCAPS_SYSTEMMEMORY|DDSCAPS_TEXTURE);

		ddsd.dwHeight = FMV_SIZE;
		ddsd.dwWidth = FMV_SIZE;

		LastError = lpDD->CreateSurface(&ddsd, &SrcDDSurface[surface], NULL);
		LOGDXERR(LastError);

		DDBLTFX ddbltfx;
		memset(&ddbltfx, 0, sizeof(ddbltfx));
		ddbltfx.dwSize = sizeof(ddbltfx);
		ddbltfx.dwFillColor	= 0;// (2<<11)+(26<<5)+8;
		LastError=SrcDDSurface[surface]->Blt(NULL, NULL, NULL, DDBLT_COLORFILL | DDBLT_WAIT, &ddbltfx);
		LOGDXERR(LastError);   

		LastError = SrcDDSurface[surface]->QueryInterface(IID_IDirect3DTexture, (LPVOID*) &SrcTexture[surface]);
		LOGDXERR(LastError);
	

		{
			int PalCaps;

			if (ddsd.ddpfPixelFormat.dwFlags & DDPF_PALETTEINDEXED8)
			{
				PalCaps = (DDPCAPS_8BIT | DDPCAPS_ALLOW256);
			}
			else if (ddsd.ddpfPixelFormat.dwFlags &	DDPF_PALETTEINDEXED4)
			{
				PalCaps = DDPCAPS_4BIT;
			}
			else
			{
				PalCaps = 0;
			}

			if (PalCaps)
			{
				LPDIRECTDRAWPALETTE destPalette = NULL;
				LastError = lpDD->CreatePalette(PalCaps, FMVPalette[surface], &destPalette, NULL);
				LOGDXERR(LastError);

				LastError = SrcDDSurface[surface]->SetPalette(destPalette);
//				LastError = tempSurface->SetPalette(destPalette);
				LOGDXERR(LastError);
			}
		}



		memset(&ddsd, 0, sizeof(DDSURFACEDESC));
		ddsd.dwSize = sizeof(DDSURFACEDESC);
		LastError = SrcDDSurface[surface]->Lock(NULL,&ddsd,0,NULL);
		LOGDXERR(LastError);
	
		SrcSurfacePtr[surface] = (void*)ddsd.lpSurface;
	
	   	LastError = SrcDDSurface[surface]->Unlock(NULL);
		LOGDXERR(LastError);
	}
	CurrentSurface=0;

	{
		extern void InitFMV(void);
		InitFMV();
	}
	
	FMVTextureHandle[0]=0;
	FMVTextureHandle[1]=0;
	FMVTextureHandle[2]=0;
#endif

}

void D3D_DrawFMV(int xOrigin, int yOrigin, int zOrigin)
{
	MeshXScale = 4096/8/3;
	MeshZScale = 4096/8/3;
	OP_STATE_RENDER(1, ExecBufInstPtr);
    STATE_DATA(D3DRENDERSTATE_ZWRITEENABLE, FALSE, ExecBufInstPtr);
	
	for (int field=0; field<3; field++)
	{
	for (int zfield=0; zfield<3; zfield++)
	{
	int i=0;			   
	int x;

	for (x=(0+field*15); x<(16+field*15); x++)
	{
		int z;
		for(z=0+zfield*15; z<16+zfield*15; z++)
		{
			VECTORCH *point = &MeshVertex[i];
			
		 	point->vx = xOrigin+(x*MeshXScale);
		 	point->vy = yOrigin+(z*MeshZScale);

			int offset;
		 	offset = MUL_FIXED(128,GetSin(  ((point->vx+point->vy)+CloakingPhase)&4095 ) );
		 	offset += MUL_FIXED(64,GetSin(  ((point->vx-point->vy*2)+CloakingPhase/2)&4095 ) );
		 	offset += MUL_FIXED(64,GetSin(  ((point->vx*5-point->vy)/2+CloakingPhase/5)&4095 ) );

		 	point->vz = zOrigin+offset;

			{
				int shade = 127+(offset+256)/4;
				MeshVertexColour[i] = RGBALIGHT_MAKE(shade,shade,shade,192);
			}
			#if 0
			{
				int sd;
				int size=65536;
				int theta = ((z*4096)/45)&4095;
				int phi = ((x*2048)/45)&4095;
				if (theta==0) sd = 0;
				else sd = phi+theta;
				if (phi==0 || phi==2048) sd = 0;
				else sd = phi+theta;

				int outerRadius = MUL_FIXED(4000+offset,size);

				if (sd)
				offset = MUL_FIXED((FastRandom()&511)-255,size);	
				
				offset=0;
				
				point->vx = xOrigin+2000+MUL_FIXED(outerRadius+offset,MUL_FIXED(GetSin(theta),GetCos(phi)));
				point->vy = yOrigin+MUL_FIXED(outerRadius+offset,MUL_FIXED(GetSin(theta),GetSin(phi)));
				point->vz = zOrigin+MUL_FIXED(outerRadius+offset,GetCos(theta));
			}
	  		#endif
			MeshWorldVertex[i].vx = (x*4096/45);			
			MeshWorldVertex[i].vy = (z*2048/45);			
			
			TranslatePointIntoViewspace(point);
			
			/* is particle within normal view frustrum ? */
			if(AvP.PlayerType==I_Alien)	/* wide frustrum */
			{
				if(( (-point->vx <= point->vz*2)
		   			&&(point->vx <= point->vz*2)
					&&(-point->vy <= point->vz*2)
					&&(point->vy <= point->vz*2) ))
				{
					MeshVertexOutcode[i]=1;
				}
				else
				{
					MeshVertexOutcode[i]=0;
				}
			}
			else
			{
				if(( (-point->vx <= point->vz)
		   			&&(point->vx <= point->vz)
					&&(-point->vy <= point->vz)
					&&(point->vy <= point->vz) ))
				{
					MeshVertexOutcode[i]=1;
				}
				else
				{
					MeshVertexOutcode[i]=0;
				}
			}

			i++;
		}
	}
	//textprint("\n");
	D3DTEXTUREHANDLE TextureHandle = FMVTextureHandle[0];
	if (CurrTextureHandle != TextureHandle)
	{
		OP_STATE_RENDER(1, ExecBufInstPtr);
		STATE_DATA(D3DRENDERSTATE_TEXTUREHANDLE, TextureHandle, ExecBufInstPtr);
		CurrTextureHandle = TextureHandle;
	}
	CheckTranslucencyModeIsCorrect(TRANSLUCENCY_NORMAL);//GLOWING);
	if (NumVertices)
	{
	   WriteEndCodeToExecuteBuffer();
  	   UnlockExecuteBufferAndPrepareForUse();
	   ExecuteBuffer();
  	   LockExecuteBuffer();
	}
	if ((MeshVertexOutcode[0]&&MeshVertexOutcode[15]&&MeshVertexOutcode[240]&&MeshVertexOutcode[255]))
	{
		D3D_DrawMoltenMetalMesh_Unclipped();
	}	
	else
//	else if (MeshVertexOutcode[0]||MeshVertexOutcode[15]||MeshVertexOutcode[240]||MeshVertexOutcode[255])
	{
		D3D_DrawMoltenMetalMesh_Clipped();
	}	
}	}

	OP_STATE_RENDER(1, ExecBufInstPtr);
    STATE_DATA(D3DRENDERSTATE_ZWRITEENABLE, TRUE, ExecBufInstPtr);
}


void RenderFMVParticle(int t, int o, VECTORCH *offsetPtr)
{
	{
		VECTORCH translatedPosition = MeshVertex[o+t];
		translatedPosition.vx += offsetPtr->vx;
		translatedPosition.vy += offsetPtr->vy;
		translatedPosition.vz += offsetPtr->vz;
		TranslatePointIntoViewspace(&translatedPosition);
		VerticesBuffer[0].X = translatedPosition.vx;
		VerticesBuffer[0].Y = translatedPosition.vy;
		VerticesBuffer[0].Z = translatedPosition.vz;
	}
	{
		VECTORCH translatedPosition = MeshVertex[1+t];
		translatedPosition.vx += offsetPtr->vx;
		translatedPosition.vy += offsetPtr->vy;
		translatedPosition.vz += offsetPtr->vz;
		TranslatePointIntoViewspace(&translatedPosition);
		VerticesBuffer[1].X = translatedPosition.vx;
		VerticesBuffer[1].Y = translatedPosition.vy;
		VerticesBuffer[1].Z = translatedPosition.vz;
	}
	{
		VECTORCH translatedPosition = MeshVertex[16+t];
		translatedPosition.vx += offsetPtr->vx;
		translatedPosition.vy += offsetPtr->vy;
		translatedPosition.vz += offsetPtr->vz;
		TranslatePointIntoViewspace(&translatedPosition);
		VerticesBuffer[2].X = translatedPosition.vx;
		VerticesBuffer[2].Y = translatedPosition.vy;
		VerticesBuffer[2].Z = translatedPosition.vz;
	}
	{
		int outcode = TriangleWithinFrustrum();
										  
		if (outcode)
		{		 
			/* setup */	
			RenderPolygon.NumberOfVertices=3;
			
			VerticesBuffer[0].U = MeshWorldVertex[o+t].vx;
			VerticesBuffer[0].V = MeshWorldVertex[o+t].vy;
			VerticesBuffer[0].A = 192;

			VerticesBuffer[1].U = MeshWorldVertex[1+t].vx;
			VerticesBuffer[1].V = MeshWorldVertex[1+t].vy;
			VerticesBuffer[1].A = 192;

			VerticesBuffer[2].U = MeshWorldVertex[16+t].vx;
			VerticesBuffer[2].V = MeshWorldVertex[16+t].vy;
			VerticesBuffer[2].A = 192;


			if (outcode!=2)
			{
				TexturedPolygon_ClipWithZ();
				if(RenderPolygon.NumberOfVertices<3) return;
				TexturedPolygon_ClipWithNegativeX();
				if(RenderPolygon.NumberOfVertices<3) return;
				TexturedPolygon_ClipWithPositiveY();
				if(RenderPolygon.NumberOfVertices<3) return;
				TexturedPolygon_ClipWithNegativeY();
				if(RenderPolygon.NumberOfVertices<3) return;
				TexturedPolygon_ClipWithPositiveX();
				if(RenderPolygon.NumberOfVertices<3) return;
				D3D_FMVParticle_Output(RenderPolygon.Vertices);
  			}
			else D3D_FMVParticle_Output(VerticesBuffer);
		}
	}	
}


void D3D_DrawSwirlyFMV(int xOrigin, int yOrigin, int zOrigin)
{
	MeshXScale = 2048/8;
	MeshZScale = (2048*3)/8/4;
	OP_STATE_RENDER(1, ExecBufInstPtr);
    STATE_DATA(D3DRENDERSTATE_ZWRITEENABLE, FALSE, ExecBufInstPtr);
	
	int i=0;			   
	int x;

	for (x=0; x<16; x++)
	{
		int z;
		for(z=0; z<16; z++)
		{
			VECTORCH *point = &MeshVertex[i];
			
		 	point->vz = zOrigin+(x*MeshXScale);
		 	point->vy = yOrigin+(z*MeshZScale);

			int offset;
		 	offset = MUL_FIXED(128,GetSin(  ((point->vz+point->vy)+CloakingPhase)&4095 ) );
		 	offset += MUL_FIXED(64,GetSin(  ((point->vz-point->vy*2)+CloakingPhase/2)&4095 ) );
		 	offset += MUL_FIXED(64,GetSin(  ((point->vz*5-point->vy)/2+CloakingPhase/5)&4095 ) );

		 	point->vx = xOrigin+offset;

	   		MeshWorldVertex[i].vx = ((15-x)*65536*128)/15;			
			MeshWorldVertex[i].vy = (z*49152*128)/15;			
			
			
   			i++;
		}
	}

	for (x=0; x<15; x++)
	{
		int y;
		for(y=0; y<15; y++)
		{
			int a = (x+y*15)*2;
			int b = 1;
			int o = x+y*16;
			int t = 0;
			do
			{
				int p = a+b;

		   		#if 1
		   		RenderFMVParticle(o,t,&FMVParticlePosition[p]);
				#else
				{
					PARTICLE particle;
					particle.ParticleID = PARTICLE_NONCOLLIDINGFLAME;
					particle.Position = FMVParticleAbsPosition[p];
					particle.Colour = RGBA_MAKE(255,255,255,16);
					particle.Size = 3000;
					RenderParticle(&particle);
				}
				#endif

				FMVParticleAbsPosition[p] = MeshVertex[o+t];
				FMVParticleAbsPosition[p].vx += FMVParticlePosition[p].vx;
				FMVParticleAbsPosition[p].vy += FMVParticlePosition[p].vy;
				FMVParticleAbsPosition[p].vz += FMVParticlePosition[p].vz;

   				t=17;
			}
			while(b--);
		}
	}
	for(int p=0; p<450; p++)
	{
		#if 0
		if ((CloakingPhase&32767) > 16383)
		{
			int n=p+52;
			int o=p+76;
			if (n>449) n-=449;
			if (o>449) o-=449;

			FMVParticlePosition[p].vx -= MUL_FIXED(FMVParticleAbsPosition[p].vx - FMVParticleAbsPosition[n].vx,NormalFrameTime*8);
			FMVParticlePosition[p].vy -= MUL_FIXED(FMVParticleAbsPosition[p].vy - FMVParticleAbsPosition[n].vy,NormalFrameTime*8);
			FMVParticlePosition[p].vz -= MUL_FIXED(FMVParticleAbsPosition[p].vz - FMVParticleAbsPosition[n].vz,NormalFrameTime*8);
			FMVParticlePosition[p].vx += MUL_FIXED(FMVParticleAbsPosition[p].vx - FMVParticleAbsPosition[o].vx,NormalFrameTime/4);
			FMVParticlePosition[p].vy += MUL_FIXED(FMVParticleAbsPosition[p].vy - FMVParticleAbsPosition[o].vy,NormalFrameTime/4);
			FMVParticlePosition[p].vz += MUL_FIXED(FMVParticleAbsPosition[p].vz - FMVParticleAbsPosition[o].vz,NormalFrameTime/4);

		}
		else
		{

			FMVParticlePosition[p].vx -= MUL_FIXED(FMVParticlePosition[p].vx,NormalFrameTime*4);
			FMVParticlePosition[p].vy -= MUL_FIXED(FMVParticlePosition[p].vy,NormalFrameTime*4);
			FMVParticlePosition[p].vz -= MUL_FIXED(FMVParticlePosition[p].vz,NormalFrameTime*4);

			int m = Approximate3dMagnitude(&FMVParticlePosition[p]);

			if (m<=20)
			{
				FMVParticlePosition[p].vx = 0;
				FMVParticlePosition[p].vy = 0;
				FMVParticlePosition[p].vz = 0;
			}

		}
		#else
				FMVParticlePosition[p].vx = 0;
				FMVParticlePosition[p].vy = 0;
				FMVParticlePosition[p].vz = 0;
		#endif
	}

	OP_STATE_RENDER(1, ExecBufInstPtr);
    STATE_DATA(D3DRENDERSTATE_ZWRITEENABLE, TRUE, ExecBufInstPtr);
}

static void UpdateFMVTextures(int maxTextureNumberToUpdate)
{
#if FMV_ON
	int surfaceNumber=maxTextureNumberToUpdate;
	while(surfaceNumber--)
	{
		DDSURFACEDESC ddsd;


		memset(&ddsd, 0, sizeof(DDSURFACEDESC));
		ddsd.dwSize = sizeof(DDSURFACEDESC);
		LastError = SrcDDSurface[surfaceNumber]->Lock(NULL,&ddsd,DDLOCK_WAIT,NULL);
		LOGDXERR(LastError);
		
		{
			if (!NextFMVFrame(SrcSurfacePtr[surfaceNumber],0,0,128,96, surfaceNumber))
			{
		    	LastError = SrcDDSurface[surfaceNumber]->Unlock(NULL);
				LOGDXERR(LastError);
			 	return;
			}
	  	}

	    LastError = SrcDDSurface[surfaceNumber]->Unlock(NULL);
		LOGDXERR(LastError);

		if (DstDDSurface[surfaceNumber]) ReleaseDDSurface(DstDDSurface[surfaceNumber]);
		if (DstTexture[surfaceNumber]) ReleaseD3DTexture(DstTexture[surfaceNumber]);

		// Query destination surface for a texture interface.
		memset(&ddsd, 0, sizeof(DDSURFACEDESC));
		ddsd.dwSize = sizeof(DDSURFACEDESC);

		LastError = SrcDDSurface[surfaceNumber]->GetSurfaceDesc(&ddsd);
		LOGDXERR(LastError);

		ddsd.dwFlags = (DDSD_CAPS | DDSD_HEIGHT | DDSD_WIDTH | DDSD_PIXELFORMAT);
		ddsd.ddsCaps.dwCaps = (DDSCAPS_TEXTURE | DDSCAPS_ALLOCONLOAD );

		LastError = lpDD->CreateSurface(&ddsd, &DstDDSurface[surfaceNumber], NULL);
		LOGDXERR(LastError);
		{
			int PalCaps;

			if (ddsd.ddpfPixelFormat.dwFlags & DDPF_PALETTEINDEXED8)
			{
				PalCaps = (DDPCAPS_8BIT | DDPCAPS_ALLOW256);
			}
			else if (ddsd.ddpfPixelFormat.dwFlags &	DDPF_PALETTEINDEXED4)
			{
				PalCaps = DDPCAPS_4BIT;
			}
			else
			{
				PalCaps = 0;
			}

			if (PalCaps)
			{
				LPDIRECTDRAWPALETTE destPalette = NULL;

				LastError = lpDD->CreatePalette(PalCaps, FMVPalette[surfaceNumber], &destPalette, NULL);
				LOGDXERR(LastError);

				UpdateFMVPalette(FMVPalette[surfaceNumber],surfaceNumber);
				LastError = DstDDSurface[surfaceNumber]->SetPalette(destPalette);
				LOGDXERR(LastError);
				LastError = SrcDDSurface[surfaceNumber]->SetPalette(destPalette);
				LOGDXERR(LastError);

			}
		}
		LastError = DstDDSurface[surfaceNumber]->QueryInterface(IID_IDirect3DTexture,(LPVOID*) &DstTexture[surfaceNumber]);
		LOGDXERR(LastError);

		LastError = DstTexture[surfaceNumber]->Load(SrcTexture[surfaceNumber]);
	 	LOGDXERR(LastError);

		LastError = DstTexture[surfaceNumber]->GetHandle(d3d.lpD3DDevice, &FMVTextureHandle[surfaceNumber]);
		LOGDXERR(LastError);
	}
#endif
}


void KillFMVTexture(void)
{
	int surface = NUMBER_OF_SMACK_SURFACES;

	while(surface--)
	{
		RELEASE(SrcDDSurface[surface]);
		RELEASE(SrcTexture[surface]);
		RELEASE(DstDDSurface[surface]);
		RELEASE(DstTexture[surface]);
	}

}


void ThisFramesRenderingHasBegun(void)
{
		if (ScanDrawMode != ScanDrawDirectDraw)
		{
			BeginD3DScene();
			LockExecuteBuffer();
			D3D_SetupSceneDefaults();
		}
		else
		{
			LockSurfaceAndGetBufferPointer();
		}
}																  

void ThisFramesRenderingHasFinished(void)
{
	if (ScanDrawMode != ScanDrawDirectDraw)
	{
		WriteEndCodeToExecuteBuffer();
		UnlockExecuteBufferAndPrepareForUse();
		ExecuteBuffer();
		EndD3DScene();
	}

 	/* KJL 11:46:56 01/16/97 - kill off any lights which are fated to be removed */
	LightBlockDeallocation();

}
void D3D_DrawWaterOctagonPatch(int xOrigin, int yOrigin, int zOrigin, int xOffset, int zOffset)
{
	float grad = 2.414213562373;
	int i=0;
	int x;
	for (x=xOffset; x<16+xOffset; x++)
	{
		int z;
		for(z=zOffset; z<16+zOffset; z++)
		{
			VECTORCH *point = &MeshVertex[i];

		  	if (x>z)
			{
				float m,xs;
				if (x!=0)
				{
					m = (float)(z)/(float)(x);
					xs = grad/(grad+m);
				}
				else
				{
					xs = 0;
				}
				#if 1
				f2i(point->vx , xs*x*MeshXScale);
				f2i(point->vz , (grad-grad*xs)*x*MeshZScale);
				#else
				point->vx = xs*x*MeshXScale;
				point->vz = (grad-grad*xs)*x*MeshZScale;
				#endif
			}
			else
			{
				float m,xs;
				if (z!=0)
				{
					m = (float)(x)/(float)(z);
					xs = grad/(grad+m);
				}
				else
				{
					xs = 0;
				}
				#if 1
				f2i(point->vz ,	xs*z*MeshZScale);
				f2i(point->vx ,	(grad-grad*xs)*z*MeshXScale);
				#else
				point->vz =	xs*z*MeshZScale;
				point->vx =	(grad-grad*xs)*z*MeshXScale;
				#endif
			}

			point->vx += xOrigin;
			point->vz += zOrigin;

			int offset = EffectOfRipples(point);
			
			point->vy = yOrigin+offset;

			#if 0
			MeshVertexColour[i] = LightSourceWaterPoint(point,offset);
			#else
			{
				int alpha = 128-offset/4;
		//		if (alpha>255) alpha = 255;
		//		if (alpha<128) alpha = 128;
				switch (CurrentVisionMode)
				{
					default:
					case VISION_MODE_NORMAL:
					{
						MeshVertexColour[i] = RGBALIGHT_MAKE(10,51,28,alpha);
						break;
					}
					case VISION_MODE_IMAGEINTENSIFIER:
					{
						MeshVertexColour[i] = RGBALIGHT_MAKE(0,51,0,alpha);
						break;
					}
					case VISION_MODE_PRED_THERMAL:
					case VISION_MODE_PRED_SEEALIENS:
					case VISION_MODE_PRED_SEEPREDTECH:
					{
						MeshVertexColour[i] = RGBALIGHT_MAKE(0,0,28,alpha);
					  	break;
					}
				}

			}
			#endif
			TranslatePointIntoViewspace(point);
			/* is particle within normal view frustrum ? */
			if(AvP.PlayerType==I_Alien)	/* wide frustrum */
			{
				if(( (-point->vx <= point->vz*2)
		   			&&(point->vx <= point->vz*2)
					&&(-point->vy <= point->vz*2)
					&&(point->vy <= point->vz*2) ))
				{
					MeshVertexOutcode[i]=1;
				}
				else
				{
					MeshVertexOutcode[i]=0;
				}
			}
			else
			{
				if(( (-point->vx <= point->vz)
		   			&&(point->vx <= point->vz)
					&&(-point->vy <= point->vz)
					&&(point->vy <= point->vz) ))
				{
					MeshVertexOutcode[i]=1;
				}
				else
				{
					MeshVertexOutcode[i]=0;
				}
			}

			i++;
		}
	}

	if ((MeshVertexOutcode[0]&&MeshVertexOutcode[15]&&MeshVertexOutcode[240]&&MeshVertexOutcode[255]))
	{
		D3D_DrawWaterMesh_Unclipped();
	}	
	else
//	else if (MeshVertexOutcode[0]||MeshVertexOutcode[15]||MeshVertexOutcode[240]||MeshVertexOutcode[255])
	{
		D3D_DrawWaterMesh_Clipped();
	}
		
	
}


extern void D3D_DrawSliderBar(int x, int y, int alpha)
{
	struct VertexTag quadVertices[4];
	int sliderHeight = 11;
	unsigned int colour = alpha>>8;

	if (colour>255) colour = 255;
	colour = (colour<<24)+0xffffff;

	quadVertices[0].Y = y;
	quadVertices[1].Y = y;
	quadVertices[2].Y = y + sliderHeight;
	quadVertices[3].Y = y + sliderHeight;
	
	CheckFilteringModeIsCorrect(FILTERING_BILINEAR_OFF);
	{
		int topLeftU = 1;
		int topLeftV = 68;

		quadVertices[0].U = topLeftU;
		quadVertices[0].V = topLeftV;
		quadVertices[1].U = topLeftU + 2;
		quadVertices[1].V = topLeftV;
		quadVertices[2].U = topLeftU + 2;
		quadVertices[2].V = topLeftV + sliderHeight;
		quadVertices[3].U = topLeftU;
		quadVertices[3].V = topLeftV + sliderHeight;
		
		quadVertices[0].X = x;
		quadVertices[3].X = x;
		quadVertices[1].X = x + 2;
		quadVertices[2].X = x + 2;
			
		D3D_HUDQuad_Output
		(
			HUDFontsImageNumber,
			quadVertices,
			colour
		);
	}
	{
		int topLeftU = 7;
		int topLeftV = 68;

		quadVertices[0].U = topLeftU;
		quadVertices[0].V = topLeftV;
		quadVertices[1].U = topLeftU + 2;
		quadVertices[1].V = topLeftV;
		quadVertices[2].U = topLeftU + 2;
		quadVertices[2].V = topLeftV + sliderHeight;
		quadVertices[3].U = topLeftU;
		quadVertices[3].V = topLeftV + sliderHeight;
		
		quadVertices[0].X = x+213+2;
		quadVertices[3].X = x+213+2;
		quadVertices[1].X = x+2 +213+2;
		quadVertices[2].X = x+2 +213+2;
			
		D3D_HUDQuad_Output
		(
			HUDFontsImageNumber,
			quadVertices,
			colour
		);
	}
	quadVertices[2].Y = y + 2;
	quadVertices[3].Y = y + 2;

	{
		int topLeftU = 5;
		int topLeftV = 77;

		quadVertices[0].U = topLeftU;
		quadVertices[0].V = topLeftV;
		quadVertices[1].U = topLeftU;
		quadVertices[1].V = topLeftV;
		quadVertices[2].U = topLeftU;
		quadVertices[2].V = topLeftV + 2;
		quadVertices[3].U = topLeftU;
		quadVertices[3].V = topLeftV + 2;
		
		quadVertices[0].X = x + 2;
		quadVertices[3].X = x + 2;
		quadVertices[1].X = x + 215;
		quadVertices[2].X = x + 215;
			
		D3D_HUDQuad_Output
		(
			HUDFontsImageNumber,
			quadVertices,
			colour
		);
	}
	quadVertices[0].Y = y + 9;
	quadVertices[1].Y = y + 9;
	quadVertices[2].Y = y + 11;
	quadVertices[3].Y = y + 11;

	{
		int topLeftU = 5;
		int topLeftV = 77;

		quadVertices[0].U = topLeftU;
		quadVertices[0].V = topLeftV;
		quadVertices[1].U = topLeftU;
		quadVertices[1].V = topLeftV;
		quadVertices[2].U = topLeftU;
		quadVertices[2].V = topLeftV + 2;
		quadVertices[3].U = topLeftU;
		quadVertices[3].V = topLeftV + 2;
		
		quadVertices[0].X = x + 2;
		quadVertices[3].X = x + 2;
		quadVertices[1].X = x + 215;
		quadVertices[2].X = x + 215;
			
		D3D_HUDQuad_Output
		(
			HUDFontsImageNumber,
			quadVertices,
			colour
		);
	}



}

extern void D3D_DrawSlider(int x, int y, int alpha)
{
	struct VertexTag quadVertices[4];
	int sliderHeight = 5;
	unsigned int colour = alpha>>8;

	if (colour>255) colour = 255;
	colour = (colour<<24)+0xffffff;

	quadVertices[0].Y = y;
	quadVertices[1].Y = y;
	quadVertices[2].Y = y + sliderHeight;
	quadVertices[3].Y = y + sliderHeight;
	
	CheckFilteringModeIsCorrect(FILTERING_BILINEAR_OFF);
	{
		int topLeftU = 11;
		int topLeftV = 74;

		quadVertices[0].U = topLeftU;
		quadVertices[0].V = topLeftV;
		quadVertices[1].U = topLeftU + 9;
		quadVertices[1].V = topLeftV;
		quadVertices[2].U = topLeftU + 9;
		quadVertices[2].V = topLeftV + sliderHeight;
		quadVertices[3].U = topLeftU;
		quadVertices[3].V = topLeftV + sliderHeight;
		
		quadVertices[0].X = x;
		quadVertices[3].X = x;
		quadVertices[1].X = x + 9;
		quadVertices[2].X = x + 9;
			
		D3D_HUDQuad_Output
		(
			HUDFontsImageNumber,
			quadVertices,
			colour
		);
	}
}

void DrawFontTest(void)
{
	CheckFilteringModeIsCorrect(FILTERING_BILINEAR_OFF);

	struct VertexTag quadVertices[4];

	quadVertices[0].Y = 0;
	quadVertices[1].Y = 0;
	quadVertices[2].Y = 256;
	quadVertices[3].Y = 256;
	
	quadVertices[0].U = 0;
	quadVertices[0].V = 0;
	quadVertices[1].U = 256;
	quadVertices[1].V = 0;
	quadVertices[2].U = 256;
	quadVertices[2].V = 256;
	quadVertices[3].U = 0;
	quadVertices[3].V = 256;
	
	quadVertices[0].X = 0;
	quadVertices[3].X = 0;
	quadVertices[1].X = 256;
	quadVertices[2].X = 256;
			
	D3D_HUDQuad_Output
	(
		AAFontImageNumber,
		quadVertices,
		0xffffffff
	);
}
extern void D3D_DrawRectangle(int x, int y, int w, int h, int alpha)
{
	struct VertexTag quadVertices[4];
	int sliderHeight = 11;
	unsigned int colour = alpha>>8;

	if (colour>255) colour = 255;
	colour = (colour<<24)+0xffffff;

	quadVertices[0].Y = y;
	quadVertices[1].Y = y;
	quadVertices[2].Y = y + 6;
	quadVertices[3].Y = y + 6;
	
	CheckFilteringModeIsCorrect(FILTERING_BILINEAR_OFF);
	/* top left corner */
	{
		int topLeftU = 1;
		int topLeftV = 238;

		quadVertices[0].U = topLeftU;
		quadVertices[0].V = topLeftV;
		quadVertices[1].U = topLeftU + 6;
		quadVertices[1].V = topLeftV;
		quadVertices[2].U = topLeftU + 6;
		quadVertices[2].V = topLeftV + 6;
		quadVertices[3].U = topLeftU;
		quadVertices[3].V = topLeftV + 6;
		
		quadVertices[0].X = x;
		quadVertices[3].X = x;
		quadVertices[1].X = x + 6;
		quadVertices[2].X = x + 6;
			
		D3D_HUDQuad_Output
		(
			AAFontImageNumber,
			quadVertices,
			colour
		);
	}
	/* top */
	{
		int topLeftU = 9;
		int topLeftV = 238;

		quadVertices[0].U = topLeftU;
		quadVertices[0].V = topLeftV;
		quadVertices[1].U = topLeftU;
		quadVertices[1].V = topLeftV;
		quadVertices[2].U = topLeftU;
		quadVertices[2].V = topLeftV + 6;
		quadVertices[3].U = topLeftU;
		quadVertices[3].V = topLeftV + 6;
		
		quadVertices[0].X = x+6;
		quadVertices[3].X = x+6;
		quadVertices[1].X = x+6 + w-12;
		quadVertices[2].X = x+6 + w-12;
			
		D3D_HUDQuad_Output
		(
			AAFontImageNumber,
			quadVertices,
			colour
		);
	}
	/* top right corner */
	{
		int topLeftU = 11;
		int topLeftV = 238;

		quadVertices[0].U = topLeftU;
		quadVertices[0].V = topLeftV;
		quadVertices[1].U = topLeftU + 6;
		quadVertices[1].V = topLeftV;
		quadVertices[2].U = topLeftU + 6;
		quadVertices[2].V = topLeftV + 6;
		quadVertices[3].U = topLeftU;
		quadVertices[3].V = topLeftV + 6;
		
		quadVertices[0].X = x + w - 6;
		quadVertices[3].X = x + w - 6;
		quadVertices[1].X = x + w;
		quadVertices[2].X = x + w;
			
		D3D_HUDQuad_Output
		(
			AAFontImageNumber,
			quadVertices,
			colour
		);
	}
	quadVertices[0].Y = y + 6;
	quadVertices[1].Y = y + 6;
	quadVertices[2].Y = y + h - 6;
	quadVertices[3].Y = y + h - 6;
	/* right */
	{
		int topLeftU = 1;
		int topLeftV = 246;

		quadVertices[0].U = topLeftU;
		quadVertices[0].V = topLeftV;
		quadVertices[1].U = topLeftU + 6;
		quadVertices[1].V = topLeftV;
		quadVertices[2].U = topLeftU + 6;
		quadVertices[2].V = topLeftV;
		quadVertices[3].U = topLeftU;
		quadVertices[3].V = topLeftV;
		
		D3D_HUDQuad_Output
		(
			AAFontImageNumber,
			quadVertices,
			colour
		);
	}
	/* left */
	{
		int topLeftU = 1;
		int topLeftV = 246;

		quadVertices[0].U = topLeftU;
		quadVertices[0].V = topLeftV;
		quadVertices[1].U = topLeftU + 6;
		quadVertices[1].V = topLeftV;
		quadVertices[2].U = topLeftU + 6;
		quadVertices[2].V = topLeftV;
		quadVertices[3].U = topLeftU;
		quadVertices[3].V = topLeftV;
		
		quadVertices[0].X = x;
		quadVertices[3].X = x;
		quadVertices[1].X = x + 6;
		quadVertices[2].X = x + 6;

		D3D_HUDQuad_Output
		(
			AAFontImageNumber,
			quadVertices,
			colour
		);
	}
	quadVertices[0].Y = y + h - 6;
	quadVertices[1].Y = y + h - 6;
	quadVertices[2].Y = y + h;
	quadVertices[3].Y = y + h;
	/* bottom left corner */
	{
		int topLeftU = 1;
		int topLeftV = 248;

		quadVertices[0].U = topLeftU;
		quadVertices[0].V = topLeftV;
		quadVertices[1].U = topLeftU + 6;
		quadVertices[1].V = topLeftV;
		quadVertices[2].U = topLeftU + 6;
		quadVertices[2].V = topLeftV + 6;
		quadVertices[3].U = topLeftU;
		quadVertices[3].V = topLeftV + 6;
		
		quadVertices[0].X = x;
		quadVertices[3].X = x;
		quadVertices[1].X = x + 6;
		quadVertices[2].X = x + 6;
			
		D3D_HUDQuad_Output
		(
			AAFontImageNumber,
			quadVertices,
			colour
		);
	}
	/* bottom */
	{
		int topLeftU = 9;
		int topLeftV = 238;

		quadVertices[0].U = topLeftU;
		quadVertices[0].V = topLeftV;
		quadVertices[1].U = topLeftU;
		quadVertices[1].V = topLeftV;
		quadVertices[2].U = topLeftU;
		quadVertices[2].V = topLeftV + 6;
		quadVertices[3].U = topLeftU;
		quadVertices[3].V = topLeftV + 6;
		
		quadVertices[0].X = x+6;
		quadVertices[3].X = x+6;
		quadVertices[1].X = x+6 + w-12;
		quadVertices[2].X = x+6 + w-12;
			
		D3D_HUDQuad_Output
		(
			AAFontImageNumber,
			quadVertices,
			colour
		);
	}
	/* bottom right corner */
	{
		int topLeftU = 11;
		int topLeftV = 248;

		quadVertices[0].U = topLeftU;
		quadVertices[0].V = topLeftV;
		quadVertices[1].U = topLeftU + 6;
		quadVertices[1].V = topLeftV;
		quadVertices[2].U = topLeftU + 6;
		quadVertices[2].V = topLeftV + 6;
		quadVertices[3].U = topLeftU;
		quadVertices[3].V = topLeftV + 6;
		
		quadVertices[0].X = x + w - 6;
		quadVertices[3].X = x + w - 6;
		quadVertices[1].X = x + w;
		quadVertices[2].X = x + w;
			
		D3D_HUDQuad_Output
		(
			AAFontImageNumber,
			quadVertices,
			colour
		);
	}


}
extern void D3D_DrawColourBar(int yTop, int yBottom, int rScale, int gScale, int bScale)
{
	extern unsigned char GammaValues[256];

	CheckTranslucencyModeIsCorrect(TRANSLUCENCY_OFF);
    if (CurrTextureHandle)
	{
    	OP_STATE_RENDER(1, ExecBufInstPtr);
        STATE_DATA(D3DRENDERSTATE_TEXTUREHANDLE, NULL, ExecBufInstPtr);
        CurrTextureHandle = NULL;
	}

	for (int i=0; i<255; )
	{
		{
			D3DTLVERTEX *vertexPtr = &((LPD3DTLVERTEX)ExecuteBufferDataArea)[NumVertices];
			unsigned int colour;
			unsigned int c;
			
			c = GammaValues[i];
			colour = RGBA_MAKE(MUL_FIXED(c,rScale),MUL_FIXED(c,gScale),MUL_FIXED(c,bScale),0);
		  	vertexPtr->sx =	(Global_VDB_Ptr->VDB_ClipRight*i)/255;
		  	vertexPtr->sy =	yTop;
			vertexPtr->sz = 0;
			vertexPtr->color = colour;
			vertexPtr++;
		  	vertexPtr->sx =	(Global_VDB_Ptr->VDB_ClipRight*i)/255;
		  	vertexPtr->sy =	yBottom;
			vertexPtr->sz = 0;
			vertexPtr->color = colour;
			vertexPtr++;
			
			i++;
			c = GammaValues[i];
			colour = RGBA_MAKE(MUL_FIXED(c,rScale),MUL_FIXED(c,gScale),MUL_FIXED(c,bScale),0);
		  	vertexPtr->sx =	(Global_VDB_Ptr->VDB_ClipRight*i)/255;
		  	vertexPtr->sy =	yBottom;
			vertexPtr->sz = 0;
			vertexPtr->color = colour;
			vertexPtr++;
		  	vertexPtr->sx =	(Global_VDB_Ptr->VDB_ClipRight*i)/255;
		  	vertexPtr->sy =	yTop;
			vertexPtr->sz = 0;
			vertexPtr->color = colour;
			vertexPtr++;

			NumVertices+=4;
		}


		OP_TRIANGLE_LIST(2, ExecBufInstPtr);
		OUTPUT_TRIANGLE(0,1,3, 4);
		OUTPUT_TRIANGLE(1,2,3, 4);
		if (NumVertices > (MaxVerticesInExecuteBuffer-12)) 
		{
		   WriteEndCodeToExecuteBuffer();
	  	   UnlockExecuteBufferAndPrepareForUse();
		   ExecuteBuffer();
	  	   LockExecuteBuffer();
		}
	}
}


extern void D3D_FadeDownScreen(int brightness, int colour)
{
	{
		D3DTLVERTEX *vertexPtr = &((LPD3DTLVERTEX)ExecuteBufferDataArea)[NumVertices];
		int t = 255 - (brightness>>8);
		if (t<0) t = 0;

 	  	vertexPtr->sx =	Global_VDB_Ptr->VDB_ClipLeft;
	  	vertexPtr->sy =	Global_VDB_Ptr->VDB_ClipUp;
		vertexPtr->sz = 0;
		vertexPtr->color = (t<<24)+colour;
		vertexPtr++;
	  	vertexPtr->sx =	Global_VDB_Ptr->VDB_ClipRight;
	  	vertexPtr->sy =	Global_VDB_Ptr->VDB_ClipUp;
		vertexPtr->sz = 0;
		vertexPtr->color = (t<<24)+colour;
		vertexPtr++;
	  	vertexPtr->sx =	Global_VDB_Ptr->VDB_ClipRight;
	  	vertexPtr->sy =	Global_VDB_Ptr->VDB_ClipDown;
		vertexPtr->sz = 0;
		vertexPtr->color = (t<<24)+colour;
		vertexPtr++;
	  	vertexPtr->sx =	Global_VDB_Ptr->VDB_ClipLeft;
	  	vertexPtr->sy =	Global_VDB_Ptr->VDB_ClipDown;
		vertexPtr->sz = 0;
		vertexPtr->color = (t<<24)+colour;

		NumVertices+=4;
	}
	CheckTranslucencyModeIsCorrect(TRANSLUCENCY_NORMAL);
//	CheckFilteringModeIsCorrect(FILTERING_BILINEAR_ON);

 //	NoiseTextureHandle = 0;
    if (CurrTextureHandle)
	{
    	OP_STATE_RENDER(1, ExecBufInstPtr);
        STATE_DATA(D3DRENDERSTATE_TEXTUREHANDLE, NULL, ExecBufInstPtr);
        CurrTextureHandle = NULL;
	}

	OP_TRIANGLE_LIST(2, ExecBufInstPtr);
	OUTPUT_TRIANGLE(0,1,3, 4);
	OUTPUT_TRIANGLE(1,2,3, 4);
	if (NumVertices > (MaxVerticesInExecuteBuffer-12)) 
	{
	   WriteEndCodeToExecuteBuffer();
  	   UnlockExecuteBufferAndPrepareForUse();
	   ExecuteBuffer();
  	   LockExecuteBuffer();
	}
}
extern void ClearZBufferWithPolygon(void)
{
	{
		D3DTLVERTEX *vertexPtr = &((LPD3DTLVERTEX)ExecuteBufferDataArea)[NumVertices];

 	  	vertexPtr->sx =	Global_VDB_Ptr->VDB_ClipLeft;
	  	vertexPtr->sy =	Global_VDB_Ptr->VDB_ClipUp;
		vertexPtr->sz = 1.0f;
		vertexPtr->rhw = 1;
		vertexPtr->color = 0;
		vertexPtr++;
	  	vertexPtr->sx =	Global_VDB_Ptr->VDB_ClipRight;
	  	vertexPtr->sy =	Global_VDB_Ptr->VDB_ClipUp;
		vertexPtr->sz = 1.0f;
		vertexPtr->rhw = 1;
		vertexPtr->color = 0;
		vertexPtr++;
	  	vertexPtr->sx =	Global_VDB_Ptr->VDB_ClipRight;
	  	vertexPtr->sy =	Global_VDB_Ptr->VDB_ClipDown;
		vertexPtr->sz = 1.0f;
		vertexPtr->rhw = 1;
		vertexPtr->color = 0;
		vertexPtr++;
	  	vertexPtr->sx =	Global_VDB_Ptr->VDB_ClipLeft;
	  	vertexPtr->sy =	Global_VDB_Ptr->VDB_ClipDown;
		vertexPtr->sz = 1.0f;
		vertexPtr->rhw = 1;
		vertexPtr->color = 0;

		NumVertices+=4;
	}
	CheckTranslucencyModeIsCorrect(TRANSLUCENCY_JUSTSETZ);
    OP_STATE_RENDER(1, ExecBufInstPtr);
    STATE_DATA(D3DRENDERSTATE_ZFUNC, D3DCMP_ALWAYS, ExecBufInstPtr);

	D3DTEXTUREHANDLE TextureHandle = 0;
	if (CurrTextureHandle != TextureHandle)
	{
		OP_STATE_RENDER(1, ExecBufInstPtr);
		STATE_DATA(D3DRENDERSTATE_TEXTUREHANDLE, TextureHandle, ExecBufInstPtr);
		CurrTextureHandle = TextureHandle;
	}

	OP_TRIANGLE_LIST(2, ExecBufInstPtr);
	OUTPUT_TRIANGLE(0,1,3, 4);
	OUTPUT_TRIANGLE(1,2,3, 4);
    
    OP_STATE_RENDER(1, ExecBufInstPtr);
    STATE_DATA(D3DRENDERSTATE_ZFUNC, D3DCMP_LESSEQUAL, ExecBufInstPtr);
	
	if (NumVertices > (MaxVerticesInExecuteBuffer-12)) 
	{
	   WriteEndCodeToExecuteBuffer();
  	   UnlockExecuteBufferAndPrepareForUse();
	   ExecuteBuffer();
  	   LockExecuteBuffer();
	}
}

extern void D3D_PlayerOnFireOverlay(void)
{
	{
		D3DTLVERTEX *vertexPtr = &((LPD3DTLVERTEX)ExecuteBufferDataArea)[NumVertices];
		int t = 128;
		int colour = (FMVParticleColour&0xffffff)+(t<<24);

		float u = (FastRandom()&255)/256.0f;
		float v = (FastRandom()&255)/256.0f;

 	  	vertexPtr->sx =	Global_VDB_Ptr->VDB_ClipLeft;
	  	vertexPtr->sy =	Global_VDB_Ptr->VDB_ClipUp;
		vertexPtr->sz = 0;
		vertexPtr->rhw = 1;
		vertexPtr->tu = u;
		vertexPtr->tv = v;
		vertexPtr->color = colour;
		vertexPtr++;
	  	vertexPtr->sx =	Global_VDB_Ptr->VDB_ClipRight;
	  	vertexPtr->sy =	Global_VDB_Ptr->VDB_ClipUp;
		vertexPtr->sz = 0;
		vertexPtr->rhw = 1;
		vertexPtr->tu = u+1.0f;
		vertexPtr->tv = v;
		vertexPtr->color = colour;
		vertexPtr++;
	  	vertexPtr->sx =	Global_VDB_Ptr->VDB_ClipRight;
	  	vertexPtr->sy =	Global_VDB_Ptr->VDB_ClipDown;
		vertexPtr->sz = 0;
		vertexPtr->rhw = 1;
		vertexPtr->tu = u+1.0f;
		vertexPtr->tv = v+1.0f;
		vertexPtr->color = colour;
		vertexPtr++;
	  	vertexPtr->sx =	Global_VDB_Ptr->VDB_ClipLeft;
	  	vertexPtr->sy =	Global_VDB_Ptr->VDB_ClipDown;
		vertexPtr->sz = 0;
		vertexPtr->rhw = 1;
		vertexPtr->tu = u;
		vertexPtr->tv = v+1.0f;
		vertexPtr->color = colour;

		NumVertices+=4;
	}
	CheckTranslucencyModeIsCorrect(TRANSLUCENCY_GLOWING);
	CheckFilteringModeIsCorrect(FILTERING_BILINEAR_ON);

	D3DTEXTUREHANDLE TextureHandle = (D3DTEXTUREHANDLE)ImageHeaderArray[BurningImageNumber].D3DHandle;
	if (CurrTextureHandle != TextureHandle)
	{
		OP_STATE_RENDER(1, ExecBufInstPtr);
		STATE_DATA(D3DRENDERSTATE_TEXTUREHANDLE, TextureHandle, ExecBufInstPtr);
		CurrTextureHandle = TextureHandle;
	}

	OP_TRIANGLE_LIST(2, ExecBufInstPtr);
	OUTPUT_TRIANGLE(0,1,3, 4);
	OUTPUT_TRIANGLE(1,2,3, 4);
	if (NumVertices > (MaxVerticesInExecuteBuffer-12)) 
	{
	   WriteEndCodeToExecuteBuffer();
  	   UnlockExecuteBufferAndPrepareForUse();
	   ExecuteBuffer();
  	   LockExecuteBuffer();
	}
}
extern void D3D_ScreenInversionOverlay(void)
{
	int theta[2];
	int colour = 0xffffffff;
	int i;

	theta[0] = (CloakingPhase/8)&4095;
	theta[1] = (800-CloakingPhase/8)&4095;
	
	CheckTranslucencyModeIsCorrect(TRANSLUCENCY_DARKENINGCOLOUR);
	CheckFilteringModeIsCorrect(FILTERING_BILINEAR_ON);
	
	for (i=0; i<2; i++)
	{
		float sin = (GetSin(theta[i]))/65536.0f/16.0f;
		float cos = (GetCos(theta[i]))/65536.0f/16.0f;
		{
			D3DTLVERTEX *vertexPtr = &((LPD3DTLVERTEX)ExecuteBufferDataArea)[NumVertices];
	 	  	vertexPtr->sx =	Global_VDB_Ptr->VDB_ClipLeft;
		  	vertexPtr->sy =	Global_VDB_Ptr->VDB_ClipUp;
			vertexPtr->sz = 0;
			vertexPtr->rhw = 1;
			vertexPtr->tu = 0.375 + (cos*(-1) - sin*(-1));
			vertexPtr->tv = 0.375 + (sin*(-1) + cos*(-1));
			vertexPtr->color = colour;
			vertexPtr++;
		  	vertexPtr->sx =	Global_VDB_Ptr->VDB_ClipRight;
		  	vertexPtr->sy =	Global_VDB_Ptr->VDB_ClipUp;
			vertexPtr->sz = 0;
			vertexPtr->rhw = 1;
			vertexPtr->tu = .375 + (cos*(+1) - sin*(-1));
			vertexPtr->tv = .375 + (sin*(+1) + cos*(-1));
			vertexPtr->color = colour;
			vertexPtr++;
		  	vertexPtr->sx =	Global_VDB_Ptr->VDB_ClipRight;
		  	vertexPtr->sy =	Global_VDB_Ptr->VDB_ClipDown;
			vertexPtr->sz = 0;
			vertexPtr->rhw = 1;
			vertexPtr->tu = .375 + (cos*(+1) - sin*(+1));
			vertexPtr->tv = .375 + (sin*(+1) + cos*(+1));
			vertexPtr->color = colour;
			vertexPtr++;
		  	vertexPtr->sx =	Global_VDB_Ptr->VDB_ClipLeft;
		  	vertexPtr->sy =	Global_VDB_Ptr->VDB_ClipDown;
			vertexPtr->sz = 0;
			vertexPtr->rhw = 1;
			vertexPtr->tu = .375 + (cos*(-1) - sin*(+1));
			vertexPtr->tv = .375 + (sin*(-1) + cos*(+1));
			vertexPtr->color = colour;

			NumVertices+=4;
		}

	//	D3DTEXTUREHANDLE TextureHandle = NULL;
		D3DTEXTUREHANDLE TextureHandle = (D3DTEXTUREHANDLE)ImageHeaderArray[SpecialFXImageNumber].D3DHandle;
		if (CurrTextureHandle != TextureHandle)
		{
			OP_STATE_RENDER(1, ExecBufInstPtr);
			STATE_DATA(D3DRENDERSTATE_TEXTUREHANDLE, TextureHandle, ExecBufInstPtr);
			CurrTextureHandle = TextureHandle;
		}

		OP_TRIANGLE_LIST(2, ExecBufInstPtr);
		OUTPUT_TRIANGLE(0,1,3, 4);
		OUTPUT_TRIANGLE(1,2,3, 4);
		if (NumVertices > (MaxVerticesInExecuteBuffer-12)) 
		{
		   WriteEndCodeToExecuteBuffer();
	  	   UnlockExecuteBufferAndPrepareForUse();
		   ExecuteBuffer();
	  	   LockExecuteBuffer();
		}
		CheckTranslucencyModeIsCorrect(TRANSLUCENCY_COLOUR);
	}
}	
extern void D3D_PredatorScreenInversionOverlay(void)
{
	int colour = 0xffffffff;
	int i;
	{
		D3DTLVERTEX *vertexPtr = &((LPD3DTLVERTEX)ExecuteBufferDataArea)[NumVertices];
		  
	  	vertexPtr->sx =	Global_VDB_Ptr->VDB_ClipLeft;
	  	vertexPtr->sy =	Global_VDB_Ptr->VDB_ClipUp;
		vertexPtr->sz = 1.0f;
		vertexPtr->rhw = 1.0f;
		vertexPtr->color = colour;
		vertexPtr++;
	  	vertexPtr->sx =	Global_VDB_Ptr->VDB_ClipRight;
	  	vertexPtr->sy =	Global_VDB_Ptr->VDB_ClipUp;
		vertexPtr->sz = 1.0f;
		vertexPtr->rhw = 1.0f;
		vertexPtr->color = colour;
		vertexPtr++;
	  	vertexPtr->sx =	Global_VDB_Ptr->VDB_ClipRight;
	  	vertexPtr->sy =	Global_VDB_Ptr->VDB_ClipDown;
		vertexPtr->sz = 1.0f;
		vertexPtr->rhw = 1.0f;
		vertexPtr->color = colour;
		vertexPtr++;
	  	vertexPtr->sx =	Global_VDB_Ptr->VDB_ClipLeft;
	  	vertexPtr->sy =	Global_VDB_Ptr->VDB_ClipDown;
		vertexPtr->sz = 1.0f;
		vertexPtr->rhw = 1.0f;
		vertexPtr->color = colour;

		NumVertices+=4;
	}
	CheckTranslucencyModeIsCorrect(TRANSLUCENCY_DARKENINGCOLOUR);
	CheckFilteringModeIsCorrect(FILTERING_BILINEAR_ON);
    OP_STATE_RENDER(1, ExecBufInstPtr);
    STATE_DATA(D3DRENDERSTATE_ZFUNC, D3DCMP_ALWAYS, ExecBufInstPtr);

// 	D3DTEXTUREHANDLE TextureHandle = (D3DTEXTUREHANDLE)ImageHeaderArray[StaticImageNumber].D3DHandle;
	D3DTEXTUREHANDLE TextureHandle = NULL;
//		D3DTEXTUREHANDLE TextureHandle = (D3DTEXTUREHANDLE)ImageHeaderArray[SpecialFXImageNumber].D3DHandle;
	if (CurrTextureHandle != TextureHandle)
	{
		OP_STATE_RENDER(1, ExecBufInstPtr);
		STATE_DATA(D3DRENDERSTATE_TEXTUREHANDLE, TextureHandle, ExecBufInstPtr);
		CurrTextureHandle = TextureHandle;
	}

	OP_TRIANGLE_LIST(2, ExecBufInstPtr);
	OUTPUT_TRIANGLE(0,1,3, 4);
	OUTPUT_TRIANGLE(1,2,3, 4);
    
    OP_STATE_RENDER(1, ExecBufInstPtr);
    STATE_DATA(D3DRENDERSTATE_ZFUNC, D3DCMP_LESSEQUAL, ExecBufInstPtr);
	
	if (NumVertices > (MaxVerticesInExecuteBuffer-12)) 
	{
	   WriteEndCodeToExecuteBuffer();
  	   UnlockExecuteBufferAndPrepareForUse();
	   ExecuteBuffer();
  	   LockExecuteBuffer();
	}
}	
extern void D3D_PlayerDamagedOverlay(int intensity)
{
	int theta[2];
//	int colour = 0x00ffff + (intensity<<24);
	int colour,baseColour;
	int i;

	theta[0] = (CloakingPhase/8)&4095;
	theta[1] = (800-CloakingPhase/8)&4095;
	
	switch(AvP.PlayerType)
	{
		default:
			LOCALASSERT(0);
			/* if no debug then fall through to marine */
		case I_Marine:
			baseColour = 0xff0000;
			break;
			
		case I_Alien:
			baseColour = 0xffff00;
			break;
		
		case I_Predator:
			baseColour = 0x00ff00;
			break;
	}
	CheckTranslucencyModeIsCorrect(TRANSLUCENCY_INVCOLOUR);
	CheckFilteringModeIsCorrect(FILTERING_BILINEAR_ON);
	colour = 0xffffff - baseColour + (intensity<<24);
	for(i=0; i<=1; i++)
	{
		float sin = (GetSin(theta[i]))/65536.0f/16.0f;
		float cos = (GetCos(theta[i]))/65536.0f/16.0f;
		{
			D3DTLVERTEX *vertexPtr = &((LPD3DTLVERTEX)ExecuteBufferDataArea)[NumVertices];
	 	  	vertexPtr->sx =	Global_VDB_Ptr->VDB_ClipLeft;
		  	vertexPtr->sy =	Global_VDB_Ptr->VDB_ClipUp;
			vertexPtr->sz = 0;
			vertexPtr->rhw = 1;
			vertexPtr->tu = 0.875 + (cos*(-1) - sin*(-1));
			vertexPtr->tv = 0.375 + (sin*(-1) + cos*(-1));
			vertexPtr->color = colour;
			vertexPtr++;
		  	vertexPtr->sx =	Global_VDB_Ptr->VDB_ClipRight;
		  	vertexPtr->sy =	Global_VDB_Ptr->VDB_ClipUp;
			vertexPtr->sz = 0;
			vertexPtr->rhw = 1;
			vertexPtr->tu = .875 + (cos*(+1) - sin*(-1));
			vertexPtr->tv = .375 + (sin*(+1) + cos*(-1));
			vertexPtr->color = colour;
			vertexPtr++;
		  	vertexPtr->sx =	Global_VDB_Ptr->VDB_ClipRight;
		  	vertexPtr->sy =	Global_VDB_Ptr->VDB_ClipDown;
			vertexPtr->sz = 0;
			vertexPtr->rhw = 1;
			vertexPtr->tu = .875 + (cos*(+1) - sin*(+1));
			vertexPtr->tv = .375 + (sin*(+1) + cos*(+1));
			vertexPtr->color = colour;
			vertexPtr++;
		  	vertexPtr->sx =	Global_VDB_Ptr->VDB_ClipLeft;
		  	vertexPtr->sy =	Global_VDB_Ptr->VDB_ClipDown;
			vertexPtr->sz = 0;
			vertexPtr->rhw = 1;
			vertexPtr->tu = .875 + (cos*(-1) - sin*(+1));
			vertexPtr->tv = .375 + (sin*(-1) + cos*(+1));
			vertexPtr->color = colour;

			NumVertices+=4;
		}

	//	D3DTEXTUREHANDLE TextureHandle = NULL;
		D3DTEXTUREHANDLE TextureHandle = (D3DTEXTUREHANDLE)ImageHeaderArray[SpecialFXImageNumber].D3DHandle;
		if (CurrTextureHandle != TextureHandle)
		{
			OP_STATE_RENDER(1, ExecBufInstPtr);
			STATE_DATA(D3DRENDERSTATE_TEXTUREHANDLE, TextureHandle, ExecBufInstPtr);
			CurrTextureHandle = TextureHandle;
		}

		OP_TRIANGLE_LIST(2, ExecBufInstPtr);
		OUTPUT_TRIANGLE(0,1,3, 4);
		OUTPUT_TRIANGLE(1,2,3, 4);
		if (NumVertices > (MaxVerticesInExecuteBuffer-12)) 
		{
		   WriteEndCodeToExecuteBuffer();
	  	   UnlockExecuteBufferAndPrepareForUse();
		   ExecuteBuffer();
	  	   LockExecuteBuffer();
		}
		
//		colour = 0xff0000+(intensity<<24);
		colour = baseColour +(intensity<<24);
		CheckTranslucencyModeIsCorrect(TRANSLUCENCY_GLOWING);
	}
}	

#define NUMBER_OF_CABLE_SEGMENTS 25
int CableTheta[NUMBER_OF_CABLE_SEGMENTS];
int CablePhi[NUMBER_OF_CABLE_SEGMENTS];

int CableThetaVelocity[NUMBER_OF_CABLE_SEGMENTS];
int CablePhiVelocity[NUMBER_OF_CABLE_SEGMENTS];

VECTORCH CableForce[NUMBER_OF_CABLE_SEGMENTS];
VECTORCH CableDirection[NUMBER_OF_CABLE_SEGMENTS];
VECTORCH CableThetaDirection[NUMBER_OF_CABLE_SEGMENTS];
VECTORCH CablePhiDirection[NUMBER_OF_CABLE_SEGMENTS];

extern void MakeMatrixFromDirection(VECTORCH *directionPtr, MATRIXCH *matrixPtr);

void D3D_DrawCable(VECTORCH *centrePtr, MATRIXCH *orientationPtr)
{
	{
			// Turn OFF texturing if it is on...
			if (CurrTextureHandle != NULL)
			{
				OP_STATE_RENDER(1, ExecBufInstPtr);
				STATE_DATA(D3DRENDERSTATE_TEXTUREHANDLE, NULL, ExecBufInstPtr);
				CurrTextureHandle = NULL;
			}
	
			CheckTranslucencyModeIsCorrect(TRANSLUCENCY_GLOWING);
	
			if (NumVertices)
			{
			   WriteEndCodeToExecuteBuffer();
		  	   UnlockExecuteBufferAndPrepareForUse();
			   ExecuteBuffer();
		  	   LockExecuteBuffer();
			}
	
			OP_STATE_RENDER(1, ExecBufInstPtr);
			STATE_DATA(D3DRENDERSTATE_ZWRITEENABLE, FALSE, ExecBufInstPtr);
	
	}
	MeshXScale = 4096/16;
	MeshZScale = 4096/16;
	
	for (int field=0; field<3; field++)
	{
	int i=0;			   
	int x;
	for (x=(0+field*15); x<(16+field*15); x++)
	{
		int z;
		for(z=0; z<16; z++)
		{
			VECTORCH *point = &MeshVertex[i];
			{	
				int innerRadius = 20;
				VECTORCH radius;
				int theta = ((4096*z)/15)&4095;
				int rOffset = GetSin((x*64+theta/32-CloakingPhase)&4095);
				rOffset = MUL_FIXED(rOffset,rOffset)/512;


				radius.vx = MUL_FIXED(innerRadius+rOffset/8,GetSin(theta));
				radius.vy = MUL_FIXED(innerRadius+rOffset/8,GetCos(theta));
				radius.vz = 0;
				
				RotateVector(&radius,orientationPtr);

				point->vx = centrePtr[x].vx+radius.vx;
				point->vy = centrePtr[x].vy+radius.vy;
				point->vz = centrePtr[x].vz+radius.vz;

				MeshVertexColour[i] = RGBALIGHT_MAKE(0,rOffset,255,128);

			}
			
			TranslatePointIntoViewspace(point);
			
			/* is particle within normal view frustrum ? */
			if(AvP.PlayerType==I_Alien)	/* wide frustrum */
			{
				if(( (-point->vx <= point->vz*2)
		   			&&(point->vx <= point->vz*2)
					&&(-point->vy <= point->vz*2)
					&&(point->vy <= point->vz*2) ))
				{
					MeshVertexOutcode[i]=1;
				}
				else
				{
					MeshVertexOutcode[i]=0;
				}
			}
			else
			{
				if(( (-point->vx <= point->vz)
		   			&&(point->vx <= point->vz)
					&&(-point->vy <= point->vz)
					&&(point->vy <= point->vz) ))
				{
					MeshVertexOutcode[i]=1;
				}
				else
				{
					MeshVertexOutcode[i]=0;
				}
			}

			i++;
		}
	}
	//textprint("\n");
   	if ((MeshVertexOutcode[0]&&MeshVertexOutcode[15]&&MeshVertexOutcode[240]&&MeshVertexOutcode[255]))
	{
		D3D_DrawMoltenMetalMesh_Unclipped();
	   //	D3D_DrawWaterMesh_Unclipped();
	}	
	else
//	else if (MeshVertexOutcode[0]||MeshVertexOutcode[15]||MeshVertexOutcode[240]||MeshVertexOutcode[255])
	{
		D3D_DrawMoltenMetalMesh_Clipped();
  	   //	D3D_DrawWaterMesh_Clipped();
	}	
	}
			OP_STATE_RENDER(1, ExecBufInstPtr);
			STATE_DATA(D3DRENDERSTATE_ZWRITEENABLE, TRUE, ExecBufInstPtr);
}



void SetupFMVTexture(FMVTEXTURE *ftPtr)
{
	DDSURFACEDESC ddsd;
	memcpy(&ddsd, &(d3d.TextureFormat[d3d.CurrentTextureFormat].ddsd), sizeof(ddsd));

	ddsd.dwSize = sizeof(ddsd);

	ddsd.dwFlags = (DDSD_CAPS | DDSD_HEIGHT | DDSD_WIDTH | DDSD_PIXELFORMAT);
		ddsd.ddsCaps.dwCaps = (DDSCAPS_SYSTEMMEMORY|DDSCAPS_TEXTURE);

	ddsd.dwHeight = FMV_SIZE;
	ddsd.dwWidth = FMV_SIZE;

	LastError = lpDD->CreateSurface(&ddsd, &(ftPtr->SrcSurface), NULL);
	LOGDXERR(LastError);

	DDBLTFX ddbltfx;
	memset(&ddbltfx, 0, sizeof(ddbltfx));
	ddbltfx.dwSize = sizeof(ddbltfx);
	ddbltfx.dwFillColor	= 0;// (2<<11)+(26<<5)+8;
	LastError=(ftPtr->SrcSurface)->Blt(NULL, NULL, NULL, DDBLT_COLORFILL | DDBLT_WAIT, &ddbltfx);
	LOGDXERR(LastError);   

	LastError = (ftPtr->SrcSurface)->QueryInterface(IID_IDirect3DTexture, (LPVOID*) &(ftPtr->SrcTexture));
	LOGDXERR(LastError);


	{
		int PalCaps;

		if (ddsd.ddpfPixelFormat.dwFlags & DDPF_PALETTEINDEXED8)
		{
			PalCaps = (DDPCAPS_8BIT | DDPCAPS_ALLOW256);
		}
		else if (ddsd.ddpfPixelFormat.dwFlags &	DDPF_PALETTEINDEXED4)
		{
			PalCaps = DDPCAPS_4BIT;
		}
		else
		{
			PalCaps = 0;
		}

		if (PalCaps)
		{
			LPDIRECTDRAWPALETTE destPalette = NULL;
			LastError = lpDD->CreatePalette(PalCaps, ftPtr->SrcPalette, &destPalette, NULL);
			LOGDXERR(LastError);

			LastError = (ftPtr->SrcSurface)->SetPalette(destPalette);
//				LastError = tempSurface->SetPalette(destPalette);
			LOGDXERR(LastError);
		}
	}
	ftPtr->DestTexture = 0;
	ftPtr->SoundVolume = 0;
}



void UpdateFMVTexture(FMVTEXTURE *ftPtr)
{
	LPDIRECTDRAWSURFACE destSurface = NULL;
	LOCALASSERT(ftPtr);
	LOCALASSERT(ftPtr->ImagePtr);
	LPDIRECTDRAWSURFACE srcSurface = ftPtr->SrcSurface;
	LPDIRECT3DTEXTURE srcTexture = ftPtr->SrcTexture;

	LOCALASSERT(srcSurface);

	DDSURFACEDESC ddsd;
	memset(&ddsd, 0, sizeof(DDSURFACEDESC));
	ddsd.dwSize = sizeof(DDSURFACEDESC);
	
	LastError = srcSurface->Lock(NULL,&ddsd,DDLOCK_WAIT,NULL);
	
	// check for success
	{
		if (!NextFMVTextureFrame(ftPtr,(void*)ddsd.lpSurface))
		{
	    	LastError = srcSurface->Unlock(NULL);
			LOGDXERR(LastError);
		 	return;
		}
  	}

    LastError = srcSurface->Unlock(NULL);
	LOGDXERR(LastError);
	
	if (ftPtr->DestTexture)
	{
		ReleaseD3DTexture(ftPtr->DestTexture);
		ftPtr->DestTexture = 0;
	}

	// Query destination surface for a texture interface.
	memset(&ddsd, 0, sizeof(DDSURFACEDESC));
	ddsd.dwSize = sizeof(DDSURFACEDESC);

	LastError = srcSurface->GetSurfaceDesc(&ddsd);
	LOGDXERR(LastError);

	ddsd.dwFlags = (DDSD_CAPS | DDSD_HEIGHT | DDSD_WIDTH | DDSD_PIXELFORMAT);
	ddsd.ddsCaps.dwCaps = (DDSCAPS_TEXTURE | DDSCAPS_ALLOCONLOAD );

	LastError = lpDD->CreateSurface(&ddsd, &destSurface, NULL);
	LOGDXERR(LastError);
	{
		int PalCaps;

		if (ddsd.ddpfPixelFormat.dwFlags & DDPF_PALETTEINDEXED8)
		{
			PalCaps = (DDPCAPS_8BIT | DDPCAPS_ALLOW256);
		}
		else if (ddsd.ddpfPixelFormat.dwFlags &	DDPF_PALETTEINDEXED4)
		{
			PalCaps = DDPCAPS_4BIT;
		}
		else
		{
			PalCaps = 0;
		}

		#if 1
		if (PalCaps)
		{
			LPDIRECTDRAWPALETTE destPalette = NULL;

			LastError = lpDD->CreatePalette(PalCaps, ftPtr->SrcPalette, &destPalette, NULL);
			LOGDXERR(LastError);
			UpdateFMVTexturePalette(ftPtr);
			LastError = destSurface->SetPalette(destPalette);
			LOGDXERR(LastError);
			LastError = srcSurface->SetPalette(destPalette);
			LOGDXERR(LastError);
			
			destPalette->Release();
		}
		#endif
	}
	LastError = destSurface->QueryInterface(IID_IDirect3DTexture,(LPVOID*) &(ftPtr->DestTexture));
	LOGDXERR(LastError);

	LastError = (ftPtr->DestTexture)->Load(srcTexture);
 	LOGDXERR(LastError);

	LastError = (ftPtr->DestTexture)->GetHandle(d3d.lpD3DDevice, &(ftPtr->ImagePtr->D3DHandle));
	LOGDXERR(LastError);

  //	ftPtr->ImagePtr->DDSurface = destSurface;
//	ftPtr->ImagePtr->D3DTexture = (ftPtr->DestTexture);	
	if (destSurface) ReleaseDDSurface(destSurface);
}
#if 0
static int GammaSetting;
void UpdateGammaSettings(int g, int forceUpdate)
{
	LPDIRECTDRAWGAMMACONTROL handle;
	DDGAMMARAMP gammaValues;

	if (g==GammaSetting && !forceUpdate) return;

	lpDDSPrimary->QueryInterface(IID_IDirectDrawGammaControl,(LPVOID*)&handle);

//	handle->GetGammaRamp(0,&gammaValues);
	for (int i=0; i<=255; i++)
	{
		int u = ((i*65536)/255);
		int m = MUL_FIXED(u,u);
		int l = MUL_FIXED(2*u,ONE_FIXED-u);

		int a;
		
		a = m/256+MUL_FIXED(g,l);
		if (a<0) a=0;
		if (a>255) a=255;

		gammaValues.red[i]=a*256;
		gammaValues.green[i]=a*256;
		gammaValues.blue[i]=a*256;
	}
//	handle->SetGammaRamp(0,&gammaValues);
	handle->SetGammaRamp(DDSGR_CALIBRATE,&gammaValues);
	GammaSetting=g;
//	handle->SetGammaRamp(DDSGR_CALIBRATE,&gammaValues);
	//handle->GetGammaRamp(0,&gammaValues);
	RELEASE(handle);
}
#endif



// For extern "C"

};


















											







#if 0
void D3D_Polygon_Output(RENDERVERTEX *renderVerticesPtr)
{
	D3DTEXTUREHANDLE TextureHandle;

	float ZNear;
	float RecipW, RecipH;


	if(RenderPolygon.IsTextured)
	{
		int texoffset = RenderPolygon.ImageIndex;
		TextureHandle = (D3DTEXTUREHANDLE) ImageHeaderArray[texoffset].D3DHandle;
		if(ImageHeaderArray[texoffset].ImageWidth==128)
		{
			RecipW = 1.0 /128.0;
		}
		else
		{
			float width = (float) ImageHeaderArray[texoffset].ImageWidth;
			RecipW = (1.0 / width);
		}
		if(ImageHeaderArray[texoffset].ImageHeight==128)
		{
			RecipH = 1.0 / 128.0;
		}
		else
		{
			float height = (float) ImageHeaderArray[texoffset].ImageHeight;
			RecipH = (1.0 / height);
		}
	}


	/* OUTPUT VERTICES TO EXECUTE BUFFER */
	{
		int i = RenderPolygon.NumberOfVertices;
		RENDERVERTEX *vertices = renderVerticesPtr;

		do
		{
			D3DTLVERTEX *vertexPtr = &((LPD3DTLVERTEX)ExecuteBufferDataArea)[NumVertices];
		  	float oneOverZ;
		  	oneOverZ = (1.0)/vertices->Z;
			float zvalue;

			{
				int x = (vertices->X*(Global_VDB_Ptr->VDB_ProjX+1))/vertices->Z+Global_VDB_Ptr->VDB_CentreX;

				if (x<Global_VDB_Ptr->VDB_ClipLeft)
				{
					x=Global_VDB_Ptr->VDB_ClipLeft;
				}	
				else if (x>Global_VDB_Ptr->VDB_ClipRight)
				{
					x=Global_VDB_Ptr->VDB_ClipRight;	
				}
				
				vertexPtr->sx=x;
			}
			{
				int y = (vertices->Y*(Global_VDB_Ptr->VDB_ProjY+1))/vertices->Z+Global_VDB_Ptr->VDB_CentreY;
				
				if (y<Global_VDB_Ptr->VDB_ClipUp)
				{
					y=Global_VDB_Ptr->VDB_ClipUp;
				}
				else if (y>Global_VDB_Ptr->VDB_ClipDown)
				{
					y=Global_VDB_Ptr->VDB_ClipDown;	
				}
				vertexPtr->sy=y;
				
			}
			
			if (RenderPolygon.IsTextured)
			{
				vertexPtr->tu = ((float)(vertices->U>>16)+0.5) * RecipW;
				vertexPtr->tv = ((float)(vertices->V>>16)+0.5) * RecipH;
				vertexPtr->rhw = oneOverZ;
			}

			{
				zvalue = vertices->Z+HeadUpDisplayZOffset;
	   //			zvalue /= 65536.0;
	   		   	zvalue = ((zvalue-ZNear)/zvalue);
			}	
	
 			if (RenderPolygon.TranslucencyMode!=TRANSLUCENCY_OFF)
			{
		  		vertexPtr->color = RGBALIGHT_MAKE(vertices->R,vertices->G,vertices->B, vertices->A);
			}
			else
			{
				vertexPtr->color = RGBLIGHT_MAKE(vertices->R,vertices->G,vertices->B);
			}

 			if (RenderPolygon.IsSpecularLit)
			{
		  		vertexPtr->specular = RGBALIGHT_MAKE(vertices->R/2,vertices->G/2,vertices->B/2, 0);
			}
			else
			{
				vertexPtr->specular=0;
			}

			vertexPtr->sz = zvalue;

			#if FOG_ON
			if(CurrentRenderStates.FogIsOn)
			{
				if (vertices->Z>CurrentRenderStates.FogDistance)
				{
					int fog = ((vertices->Z-CurrentRenderStates.FogDistance)/FOG_SCALE);
					if (fog<0) fog=0;
				 	if (fog>254) fog=254;
				  	fog=255-fog;
				   	vertexPtr->specular|=RGBALIGHT_MAKE(0,0,0,fog);
				}
				else
				{
				   	vertexPtr->specular|=RGBALIGHT_MAKE(0,0,0,255);
				}
			}
			#endif
		
			vertices++;
			NumVertices++;
		}
	  	while(--i);
	}

	CheckTranslucencyModeIsCorrect(RenderPolygon.TranslucencyMode);

    if (D3DTexturePerspective != Yes)
    {
		D3DTexturePerspective = Yes;
		OP_STATE_RENDER(1, ExecBufInstPtr);
		STATE_DATA(D3DRENDERSTATE_TEXTUREPERSPECTIVE, TRUE, ExecBufInstPtr);
	}

    if (TextureHandle != CurrTextureHandle)
	{
    	OP_STATE_RENDER(1, ExecBufInstPtr);
        STATE_DATA(D3DRENDERSTATE_TEXTUREHANDLE, TextureHandle, ExecBufInstPtr);
	   	CurrTextureHandle = TextureHandle;
	}


	D3D_OutputTriangles();
}

#endif






void r2rect :: AlphaFill
(
	unsigned char R,
	unsigned char G,
	unsigned char B,
	unsigned char translucency
) const
{
	GLOBALASSERT
	(
		bValidPhys()
	);
	if (y1<=y0) return;
	#if 1
	/* OUTPUT quadVerticesPtr TO EXECUTE BUFFER */
	{
		D3DCOLOR Colour;

  		Colour = RGBALIGHT_MAKE(R,G,B,translucency);

		{
			D3DTLVERTEX *vertexPtr = &((LPD3DTLVERTEX)ExecuteBufferDataArea)[NumVertices];

			/* Vertex 0 = Top left */
			vertexPtr->sx= x0;
			vertexPtr->sy= y0;
			vertexPtr->color = Colour;
			
			NumVertices++;
			vertexPtr++;

			/* Vertex 1 = Top right */
			vertexPtr->sx=( x1 - 1);
			vertexPtr->sy=( y0 );
			vertexPtr->color = Colour;
			
			NumVertices++;
			vertexPtr++;

			/* Vertex 2 = Bottom right */
			vertexPtr->sx=( x1 - 1);
			vertexPtr->sy=( y1 - 1);
			vertexPtr->color = Colour;
			
			NumVertices++;
			vertexPtr++;

			/* Vertex 3 = Bottom left */
			vertexPtr->sx=x0;
			vertexPtr->sy=( y1 - 1);
			vertexPtr->color = Colour;
			
			NumVertices++;
		}
	}
	// set correct texture handle
    if (0 != CurrTextureHandle)
	{
       OP_STATE_RENDER(1, ExecBufInstPtr);
       STATE_DATA(D3DRENDERSTATE_TEXTUREHANDLE, 0, ExecBufInstPtr);
	   CurrTextureHandle = 0;
	}

	CheckTranslucencyModeIsCorrect(TRANSLUCENCY_GLOWING);
	/* output triangles to execute buffer */
	OP_TRIANGLE_LIST(2, ExecBufInstPtr);
	OUTPUT_TRIANGLE(0,1,3, 4);
	OUTPUT_TRIANGLE(1,2,3, 4);
	
	/* check to see if buffer is getting full */
	if (NumVertices > (MaxVerticesInExecuteBuffer-12)) 
	{
	   WriteEndCodeToExecuteBuffer();
  	   UnlockExecuteBufferAndPrepareForUse();
	   ExecuteBuffer();
  	   LockExecuteBuffer();
	}
	#endif
}
extern void D3D_RenderHUDNumber_Centred(unsigned int number,int x,int y,int colour);
extern void D3D_RenderHUDString(char *stringPtr,int x,int y,int colour);
extern void D3D_RenderHUDString_Clipped(char *stringPtr,int x,int y,int colour);
extern void D3D_RenderHUDString_Centred(char *stringPtr, int centreX, int y, int colour);

extern void D3D_RenderHUDNumber_Centred(unsigned int number,int x,int y,int colour)
{
	struct VertexTag quadVertices[4];
	int noOfDigits=3;
	int h = MUL_FIXED(HUDScaleFactor,HUD_DIGITAL_NUMBERS_HEIGHT);
	int w = MUL_FIXED(HUDScaleFactor,HUD_DIGITAL_NUMBERS_WIDTH);

	quadVertices[0].Y = y;
	quadVertices[1].Y = y;
	quadVertices[2].Y = y + h;
	quadVertices[3].Y = y + h;
	
	x += (3*w)/2;
	
	CheckFilteringModeIsCorrect(FILTERING_BILINEAR_OFF);

	do
	{
		int digit = number%10;
		number/=10;
		{
			int topLeftU; 
			int topLeftV;
			if (digit<8)
			{
				topLeftU = 1+(digit)*16;
				topLeftV = 1;
			}
			else
			{
				topLeftU = 1+(digit-8)*16;
				topLeftV = 1+24;
			}
			if (AvP.PlayerType == I_Marine) topLeftV+=80;

			quadVertices[0].U = topLeftU;
			quadVertices[0].V = topLeftV;
			quadVertices[1].U = topLeftU + HUD_DIGITAL_NUMBERS_WIDTH;
			quadVertices[1].V = topLeftV;
			quadVertices[2].U = topLeftU + HUD_DIGITAL_NUMBERS_WIDTH;
			quadVertices[2].V = topLeftV + HUD_DIGITAL_NUMBERS_HEIGHT;
			quadVertices[3].U = topLeftU;
			quadVertices[3].V = topLeftV + HUD_DIGITAL_NUMBERS_HEIGHT;
			
			x -= 1+w;
			quadVertices[0].X = x;
			quadVertices[3].X = x;
			quadVertices[1].X = x + w;
			quadVertices[2].X = x + w;
				
			D3D_HUDQuad_Output
			(
				HUDFontsImageNumber,
				quadVertices,
				colour
			);
		}
	}
	while(--noOfDigits);

}


extern void D3D_RenderHUDString(char *stringPtr,int x,int y,int colour)
{
	struct VertexTag quadVertices[4];

	quadVertices[0].Y = y-1;
	quadVertices[1].Y = y-1;
	quadVertices[2].Y = y + HUD_FONT_HEIGHT + 1;
	quadVertices[3].Y = y + HUD_FONT_HEIGHT + 1;
	
	CheckFilteringModeIsCorrect(FILTERING_BILINEAR_OFF);
	while( *stringPtr )
	{
		char c = *stringPtr++;

		{
			int topLeftU = 1+((c-32)&15)*16;
			int topLeftV = 1+((c-32)>>4)*16;

			quadVertices[0].U = topLeftU - 1;
			quadVertices[0].V = topLeftV - 1;
			quadVertices[1].U = topLeftU + HUD_FONT_WIDTH + 1;
			quadVertices[1].V = topLeftV - 1;
			quadVertices[2].U = topLeftU + HUD_FONT_WIDTH + 1;
			quadVertices[2].V = topLeftV + HUD_FONT_HEIGHT + 1;
			quadVertices[3].U = topLeftU - 1;
			quadVertices[3].V = topLeftV + HUD_FONT_HEIGHT + 1;
			
			quadVertices[0].X = x - 1;
			quadVertices[3].X = x - 1;
			quadVertices[1].X = x + HUD_FONT_WIDTH + 1;
			quadVertices[2].X = x + HUD_FONT_WIDTH + 1;
				
			D3D_HUDQuad_Output
			(
				AAFontImageNumber,
				quadVertices,
				colour
			);
		}
		x += AAFontWidths[c];
	}
}
extern void D3D_RenderHUDString_Clipped(char *stringPtr,int x,int y,int colour)
{
	struct VertexTag quadVertices[4];

 	LOCALASSERT(y<=0);

	CheckFilteringModeIsCorrect(FILTERING_BILINEAR_OFF);

	quadVertices[2].Y = y + HUD_FONT_HEIGHT + 1;
	quadVertices[3].Y = y + HUD_FONT_HEIGHT + 1;
	
	quadVertices[0].Y = 0;
	quadVertices[1].Y = 0;

	while ( *stringPtr )
	{
		char c = *stringPtr++;

		{
			int topLeftU = 1+((c-32)&15)*16;
			int topLeftV = 1+((c-32)>>4)*16;

			quadVertices[0].U = topLeftU - 1;
			quadVertices[0].V = topLeftV - y;
			quadVertices[1].U = topLeftU + HUD_FONT_WIDTH+1;
			quadVertices[1].V = topLeftV - y;
			quadVertices[2].U = topLeftU + HUD_FONT_WIDTH+1;
			quadVertices[2].V = topLeftV + HUD_FONT_HEIGHT+1;
			quadVertices[3].U = topLeftU - 1;
			quadVertices[3].V = topLeftV + HUD_FONT_HEIGHT+1;
			
			quadVertices[0].X = x - 1;
			quadVertices[3].X = x - 1;
			quadVertices[1].X = x + HUD_FONT_WIDTH + 1;
			quadVertices[2].X = x + HUD_FONT_WIDTH + 1;
				
			D3D_HUDQuad_Output
			(
				AAFontImageNumber,
				quadVertices,
				colour
			);
		}
		x += AAFontWidths[c];
	}
}

void D3D_RenderHUDString_Centred(char *stringPtr, int centreX, int y, int colour)
{
	int length = 0;
	char *ptr = stringPtr;

	while(*ptr)
	{
		length+=AAFontWidths[*ptr++];
	}
	length = MUL_FIXED(HUDScaleFactor,length);
// 	D3D_RenderHUDString(stringPtr,centreX-length/2,y,colour);

	int x = centreX-length/2;
{
	struct VertexTag quadVertices[4];

	quadVertices[0].Y = y-MUL_FIXED(HUDScaleFactor,1);
	quadVertices[1].Y = y-MUL_FIXED(HUDScaleFactor,1);
	quadVertices[2].Y = y + MUL_FIXED(HUDScaleFactor,HUD_FONT_HEIGHT + 1);
	quadVertices[3].Y = y + MUL_FIXED(HUDScaleFactor,HUD_FONT_HEIGHT + 1);
	
	CheckFilteringModeIsCorrect(FILTERING_BILINEAR_OFF);
	while( *stringPtr )
	{
		char c = *stringPtr++;

		{
			int topLeftU = 1+((c-32)&15)*16;
			int topLeftV = 1+((c-32)>>4)*16;
			#if 1
			quadVertices[0].U = topLeftU - 1;
			quadVertices[0].V = topLeftV - 1;
			quadVertices[1].U = topLeftU + HUD_FONT_WIDTH + 1;
			quadVertices[1].V = topLeftV - 1;
			quadVertices[2].U = topLeftU + HUD_FONT_WIDTH + 1;
			quadVertices[2].V = topLeftV + HUD_FONT_HEIGHT + 1;
			quadVertices[3].U = topLeftU - 1;
			quadVertices[3].V = topLeftV + HUD_FONT_HEIGHT + 1;
			#else
			quadVertices[0].U = topLeftU ;
			quadVertices[0].V = topLeftV ;
			quadVertices[1].U = topLeftU + HUD_FONT_WIDTH ;
			quadVertices[1].V = topLeftV ;
			quadVertices[2].U = topLeftU + HUD_FONT_WIDTH ;
			quadVertices[2].V = topLeftV + HUD_FONT_HEIGHT ;
			quadVertices[3].U = topLeftU ;
			quadVertices[3].V = topLeftV + HUD_FONT_HEIGHT ;
			#endif
			quadVertices[0].X = x - MUL_FIXED(HUDScaleFactor,1);
			quadVertices[3].X = x - MUL_FIXED(HUDScaleFactor,1);
			quadVertices[1].X = x + MUL_FIXED(HUDScaleFactor,HUD_FONT_WIDTH + 1);
			quadVertices[2].X = x + MUL_FIXED(HUDScaleFactor,HUD_FONT_WIDTH + 1);
				
			D3D_HUDQuad_Output
			(
				AAFontImageNumber,
				quadVertices,
				colour
			);
		}
		x += MUL_FIXED(HUDScaleFactor,AAFontWidths[c]);
	}
}
}

extern "C"
{

extern void RenderString(char *stringPtr, int x, int y, int colour)
{
	D3D_RenderHUDString(stringPtr,x,y,colour);
}

extern void RenderStringCentred(char *stringPtr, int centreX, int y, int colour)
{
	int length = 0;
	char *ptr = stringPtr;

	while(*ptr)
	{
		length+=AAFontWidths[*ptr++];
	}
	D3D_RenderHUDString(stringPtr,centreX-length/2,y,colour);
}

extern void RenderStringVertically(char *stringPtr, int centreX, int bottomY, int colour)
{
	struct VertexTag quadVertices[4];
	int y = bottomY;

	quadVertices[0].X = centreX - (HUD_FONT_HEIGHT/2) - 1;
	quadVertices[1].X = quadVertices[0].X;
	quadVertices[2].X = quadVertices[0].X+2+HUD_FONT_HEIGHT*1;
	quadVertices[3].X = quadVertices[2].X;
	
	CheckFilteringModeIsCorrect(FILTERING_BILINEAR_OFF);
	while( *stringPtr )
	{
		char c = *stringPtr++;

		{
			int topLeftU = 1+((c-32)&15)*16;
			int topLeftV = 1+((c-32)>>4)*16;

			quadVertices[0].U = topLeftU - 1;
			quadVertices[0].V = topLeftV - 1;
			quadVertices[1].U = topLeftU + HUD_FONT_WIDTH;
			quadVertices[1].V = topLeftV - 1;
			quadVertices[2].U = topLeftU + HUD_FONT_WIDTH;
			quadVertices[2].V = topLeftV + HUD_FONT_HEIGHT + 1;
			quadVertices[3].U = topLeftU - 1;
			quadVertices[3].V = topLeftV + HUD_FONT_HEIGHT + 1;

			quadVertices[0].Y = y ;
			quadVertices[1].Y = y - HUD_FONT_WIDTH*1 -1;
			quadVertices[2].Y = y - HUD_FONT_WIDTH*1 -1;
			quadVertices[3].Y = y ;
				
			D3D_HUDQuad_Output
			(								  
				AAFontImageNumber,
				quadVertices,
				colour
			);
		}
	   	y -= AAFontWidths[c];
	}
}

};


