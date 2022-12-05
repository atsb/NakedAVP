#ifndef PLATFORM_INCLUDED
#define PLATFORM_INCLUDED

#include "fixer.h"

#include "shape.h" // struct imageheader

/*

 Platform Specific Header Include

*/

#ifdef __cplusplus
extern "C"  {
#endif

/* Types */

typedef struct LONGLONGCH {

	unsigned int lo32;
	int hi32;

} LONGLONGCH;

/*

 Sine and Cosine

*/

#define GetSin(a) sine[a]
#define GetCos(a) cosine[a]

/*
	Available processor types
*/

typedef enum {

	PType_OffBottomOfScale,
	PType_486,
	PType_Pentium,
	PType_P6,
	PType_PentiumMMX,
	PType_Klamath,
	PType_OffTopOfScale

} PROCESSORTYPES;

/*
	Video mode decsription (to be filled
	in by DirectDraw callback).
*/

typedef struct videomodeinfo {

	int Width; /* in pixels */
	int Height; /* in pixels */
	int ColourDepth; /* in bits per pixel */

} VIDEOMODEINFO;

/*
	Maximum number of display modes
	that could be detected on unknown
	hardware.
*/

#define MaxAvailableVideoModes 100

/*
	#defines, structures etc for
	textprint system
*/

#define MaxMsgChars 100
#define MaxMessages 20

/* Font description */

#define CharWidth   8 /* In PIXELS, not bytes */
#define CharHeight  10 /* In PIXELS, not bytes */
#define CharVertSep (CharHeight + 0) /* In PIXELS, not bytes */

#define FontStart 33 // random squiggle in standard ASCII
#define FontEnd 127 // different random squiggle in standard ASCII

#define FontInvisValue 0x00 // value to be colour keyed out of font blit

typedef struct printqueueitem {
	
	char text[MaxMsgChars];
	int text_length;
	int x;
	int y;
		
} PRINTQUEUEITEM;



/* KJL 12:30:05 9/9/97 - new keyboard, mouse etc. enum */
enum KEY_ID
{
	KEY_ESCAPE,

	KEY_0,			
	KEY_1,			
	KEY_2,			
	KEY_3,			
	KEY_4,			
	KEY_5,			
	KEY_6,			
	KEY_7,			
	KEY_8,			
	KEY_9,			

	KEY_A,			
	KEY_B,			
	KEY_C,			
	KEY_D,			
	KEY_E,			
	KEY_F,			
	KEY_G,			
	KEY_H,			
	KEY_I,			
	KEY_J,			
	KEY_K,			
	KEY_L,			
	KEY_M,			
	KEY_N,			
	KEY_O,			
	KEY_P,			
	KEY_Q,			
	KEY_R,			
	KEY_S,			
	KEY_T,			
	KEY_U,			
	KEY_V,			
	KEY_W,			
	KEY_X,			
	KEY_Y,			
	KEY_Z,			

	KEY_LEFT,
	KEY_RIGHT,
	KEY_UP,
	KEY_DOWN,
	KEY_CR,
	KEY_TAB,
	KEY_INS,
	KEY_DEL,
	KEY_END,
	KEY_HOME,
	KEY_PAGEUP,
	KEY_PAGEDOWN,
	KEY_BACKSPACE,	
	KEY_COMMA,		
	KEY_FSTOP,		
	KEY_SPACE,
			
	KEY_LEFTSHIFT,	
	KEY_RIGHTSHIFT,  
	KEY_LEFTALT,     
	KEY_RIGHTALT,    
	KEY_LEFTCTRL,	
	KEY_RIGHTCTRL,   
	
	KEY_CAPS,		
	KEY_NUMLOCK,		
	KEY_SCROLLOK,	

	KEY_NUMPAD0,     
	KEY_NUMPAD1,     
	KEY_NUMPAD2,     
	KEY_NUMPAD3,     
	KEY_NUMPAD4,     
	KEY_NUMPAD5,     
	KEY_NUMPAD6,     
	KEY_NUMPAD7,     
	KEY_NUMPAD8,     
	KEY_NUMPAD9,     
	KEY_NUMPADSUB,   
	KEY_NUMPADADD,   
	KEY_NUMPADDEL,
	KEY_NUMPADENTER,
	KEY_NUMPADDIVIDE,
	KEY_NUMPADMULTIPLY,

	KEY_LBRACKET,	
	KEY_RBRACKET,	
	KEY_SEMICOLON,	
	KEY_APOSTROPHE,	
	KEY_GRAVE,		
	KEY_BACKSLASH,	
	KEY_SLASH,
	KEY_CAPITAL,
	KEY_MINUS,
	KEY_EQUALS,
	KEY_LWIN,
	KEY_RWIN,
	KEY_APPS,


	KEY_F1,
	KEY_F2,
	KEY_F3,
	KEY_F4,
	KEY_F5,
	KEY_F6,
	KEY_F7,
	KEY_F8,								 
	KEY_F9,
	KEY_F10,
	KEY_F11,
	KEY_F12,

	KEY_JOYSTICK_BUTTON_1,
	KEY_JOYSTICK_BUTTON_2,
	KEY_JOYSTICK_BUTTON_3,
	KEY_JOYSTICK_BUTTON_4,
	KEY_JOYSTICK_BUTTON_5,
	KEY_JOYSTICK_BUTTON_6,
	KEY_JOYSTICK_BUTTON_7,
	KEY_JOYSTICK_BUTTON_8,
	KEY_JOYSTICK_BUTTON_9,
	KEY_JOYSTICK_BUTTON_10,
	KEY_JOYSTICK_BUTTON_11,
	KEY_JOYSTICK_BUTTON_12,
	KEY_JOYSTICK_BUTTON_13,
	KEY_JOYSTICK_BUTTON_14,
	KEY_JOYSTICK_BUTTON_15,
	KEY_JOYSTICK_BUTTON_16,
	
	KEY_LMOUSE,
	KEY_MMOUSE,
	KEY_RMOUSE,

	KEY_MOUSEBUTTON4,
	KEY_MOUSEWHEELUP,
	KEY_MOUSEWHEELDOWN,

	// Dutch
	KEY_ORDINAL,
	KEY_LESSTHAN,
	KEY_PLUS,

	// French
	KEY_RIGHTBRACKET,
	KEY_ASTERISK,
	KEY_DOLLAR,
	KEY_U_GRAVE,
	KEY_EXCLAMATION,
	KEY_COLON,

	// German
	KEY_BETA,
	KEY_A_UMLAUT,
	KEY_O_UMLAUT,
	KEY_U_UMLAUT,
	KEY_HASH,

	// Spanish
	KEY_UPSIDEDOWNEXCLAMATION,
	KEY_C_CEDILLA,
	KEY_N_TILDE,

	// accents & diacritics
	KEY_DIACRITIC_GRAVE,
	KEY_DIACRITIC_ACUTE,
	KEY_DIACRITIC_CARET,
	KEY_DIACRITIC_UMLAUT,


	KEY_VOID=255, // used to indicate a blank spot in the key config
	MAX_NUMBER_OF_INPUT_KEYS,

};
/* 
	Mouse handler modes (velocity mode is for
	interfacing to input functions that need
	to read the mouse velocity, e.g. WeRequest
	functions, while poistion mode is for menus
	and similar problems).

    The initial mode is defined as a compiled in
	global in game.c
*/

typedef enum {

	MouseVelocityMode,
	MousePositionMode

} MOUSEMODES;


/* Defines for the purpose of familiarity of name only */

#define		LeftButton		0x0001
#define		RightButton		0x0002
#define		MiddleButton	0x0004




/*
	Video Modes
*/

typedef enum {

	VideoMode_DX_320x200x8,
	VideoMode_DX_320x200x8T,
	VideoMode_DX_320x200x15,
	VideoMode_DX_320x240x8,

	VideoMode_DX_640x480x8,
	VideoMode_DX_640x480x8T,
	VideoMode_DX_640x480x15,
	VideoMode_DX_640x480x24,

	VideoMode_DX_800x600x8,
	VideoMode_DX_800x600x8T,
	VideoMode_DX_800x600x15,
	VideoMode_DX_800x600x24,

	VideoMode_DX_1024x768x8,
	VideoMode_DX_1024x768x8T,
	VideoMode_DX_1024x768x15,
	VideoMode_DX_1024x768x24,

	VideoMode_DX_1280x1024x8,
	VideoMode_DX_1280x1024x8T,
	VideoMode_DX_1280x1024x15,
	VideoMode_DX_1280x1024x24,

	VideoMode_DX_1600x1200x8,
	VideoMode_DX_1600x1200x8T,
	VideoMode_DX_1600x1200x15,
	VideoMode_DX_1600x1200x24,
	
	MaxVideoModes

} VIDEOMODES;


#define MaxScreenWidth 1600		/* Don't get this wrong! */


/*

 Video Mode Types

*/

typedef enum {

	VideoModeType_8,
	VideoModeType_15,
	VideoModeType_24,
	VideoModeType_8T

} VIDEOMODETYPES;


/*
	Windows modes
*/

typedef enum {

	WindowModeFullScreen,
	WindowModeSubWindow

} WINDOWMODES;

typedef struct WinScaleXY {

	float x;
	float y;

} WINSCALEXY;


/*
	Dubious hack for dubious
	aspects of DirectDraw initialisation
*/

typedef enum {

    /* Default */
	NoRestartRequired,
	/* 
	  Characteristic of driver which
	  will not support changing to a different
	  bit depth without rebooting
	*/
	RestartDisplayModeNotAvailable, 
	/*
	  Characteristic of ModeX emulation leading
	  to no video memory being available because
	  of the need to cooperate with a large existing
	  GDI surface which cannot be reduced without a
	  reboot
	*/
	RestartOutOfVidMemForPrimary

} VIDEORESTARTMODES;


/*
	Direct3D driver modes
*/

typedef enum {

	D3DSoftwareRGBDriver,
	D3DSoftwareRampDriver,
	D3DHardwareRGBDriver

} D3DMODES;

/*
	Software scan draw request modes
*/

typedef enum {

	RequestScanDrawDirectDraw,
	RequestScanDrawSoftwareRGB,
	RequestScanDrawRamp,
	RequestScanDrawDefault

} SOFTWARESCANDRAWREQUESTMODES;

/*
	Rasterisation request modes
*/

typedef enum {

	RequestSoftwareRasterisation,
	RequestDefaultRasterisation

} RASTERREQUESTMODES;


/*
	Actual scan draw mode
*/

typedef enum {

	ScanDrawDirectDraw,
	ScanDrawD3DRamp,
	ScanDrawD3DSoftwareRGB,
	ScanDrawD3DHardwareRGB

} SCANDRAWMODES;


/*
	Z buffering request modes
*/

typedef enum {

	RequestZBufferNever,
	RequestZBufferAlways,
	RequestZBufferDefault

} ZBUFFERREQUESTMODES;

/*
	Z buffering modes 
*/

typedef enum {

    ZBufferOn,
	ZBufferWriteOnly,
	ZBufferOff

} ZBUFFERMODES;

/*
	Request modes for DirectX 
	memory usage
*/

typedef enum {

	RequestSystemMemoryAlways,
	RequestDefaultMemoryAllocation

} DXMEMORYREQUESTMODES;


/*
	DirectX memory usage modes
*/

typedef enum {

	SystemMemoryPreferred,
	VideoMemoryPreferred

} DXMEMORYMODES;

/*
	Types of texture files that can be
	requested from the main D3D texture
	loader.
*/

typedef enum {

	TextureTypePGM,
	TextureTypePPM

} TEXTUREFILETYPE;

/*
	Windows initialisation modes.
	See InitialiseWindowsSystem
	in the default win_proj.cpp
	for a full description.
*/

typedef enum {

	WinInitFull,
	WinInitChange

} WININITMODES;


/*
	Function prototypes
*/

/* 
	Windows functionality.  Note current
	DirectInput functions are also here since
	they are really part of the Win32 multimedia
	library.
*/

long GetWindowsTickCount(void);
void CheckForWindowsMessages(void);
BOOL ExitWindowsSystem(void);
BOOL InitialiseWindowsSystem(HANDLE hInstance, int nCmdShow, int WinInitMode);
//void KeyboardHandlerKeyDown(WPARAM wParam);
//void KeyboardHandlerKeyUp(WPARAM wParam);
//void MouseVelocityHandler(UINT message, LPARAM lParam);
//void MousePositionHandler(UINT message, LPARAM lParam);
int  ReadJoystick(void); 
int  CheckForJoystick(void);
BOOL SpawnRasterThread();
BOOL WaitForRasterThread();


/* DirectDraw */
void finiObjects(void);
void GenerateDirectDrawSurface(void);
void LockSurfaceAndGetBufferPointer(void);		
void UnlockSurface(void);
void finiObjects(void);
void ColourFillBackBuffer(int FillColour);
void ColourFillBackBufferQuad(int FillColour, int LeftX,
     int TopY, int RightX, int BotY);
void FlipBuffers(void);
void BlitToBackBuffer(void* lpBackground, RECT* destRectPtr, RECT* srcRectPtr);
void BlitToBackBufferWithoutTearing(void* lpBackground, RECT* destRectPtr, RECT* srcRectPtr);
void BlitWin95Char(int x, int y, unsigned char toprint);
void ReleaseDDSurface(void* DDSurface);
BOOL InitialiseDirectDrawObject(void);
BOOL ChangeDirectDrawObject(void);
BOOL CheckForVideoModes(int TestVideoMode);
void finiObjectsExceptDD(void);
BOOL TestMemoryAccess(void);
int ChangePalette (unsigned char* NewPalette);
int GetAvailableVideoMemory(void);
//void HandleVideoModeRestarts(HINSTANCE hInstance, int nCmdShow);
void* MakeBackdropSurface(void);
void ReleaseBackdropSurface(void);
void LockBackdropSurface(void);
void UnlockBackdropSurface(void);
void ComposeBackdropBackBuffer(void);
int GetSingleColourForPrimary(int Colour);

/* Direct 3D Immediate Mode Rasterisation Module */
BOOL InitialiseDirect3DImmediateMode(void);
BOOL LockExecuteBuffer(void);
BOOL UnlockExecuteBufferAndPrepareForUse(void);
BOOL BeginD3DScene(void);
BOOL EndD3DScene(void);
BOOL ExecuteBuffer(void);
BOOL RenderD3DScene(void);
void ReleaseDirect3D(void);
void WritePolygonToExecuteBuffer(int* itemptr);
void WriteGouraudPolygonToExecuteBuffer(int* itemptr);
void Write2dTexturedPolygonToExecuteBuffer(int* itemptr);
void WriteGouraud2dTexturedPolygonToExecuteBuffer(int* itemptr);
void Write3dTexturedPolygonToExecuteBuffer(int* itemptr);
void WriteGouraud3dTexturedPolygonToExecuteBuffer(int* itemptr);
void WriteBackdrop2dTexturedPolygonToExecuteBuffer(int* itemptr);
void WriteBackdrop3dTexturedPolygonToExecuteBuffer(int* itemptr);
void WriteEndCodeToExecuteBuffer(void);
void ReleaseD3DTexture(void* D3DTexture);
void ReleaseDirect3DNotDD(void);
void ReleaseDirect3DNotDDOrImages(void);
BOOL SetExecuteBufferDefaults(void);
void SelectD3DDriverAndDrawMode(void);
#if SUPPORT_MMX
void SelectMMXOptions(void);
#endif
BOOL TestInitD3DObject(void);

#if SupportZBuffering
BOOL CreateD3DZBuffer(void);
void FlushD3DZBuffer(void);
void WriteZBPolygonToExecuteBuffer(int* itemptr);
void WriteZBGouraudPolygonToExecuteBuffer(int* itemptr);
void WriteZB2dTexturedPolygonToExecuteBuffer(int* itemptr);
void WriteZBGouraud2dTexturedPolygonToExecuteBuffer(int* itemptr);
void WriteZB3dTexturedPolygonToExecuteBuffer(int* itemptr);
void WriteZBGouraud3dTexturedPolygonToExecuteBuffer(int* itemptr);
#endif

/* KJL 11:28:31 9/9/97 - Direct Input prototypes */
BOOL InitialiseDirectInput(void);
void ReleaseDirectInput(void);
BOOL InitialiseDirectKeyboard();
void DirectReadKeyboard(void);
void ReleaseDirectKeyboard(void);
BOOL InitialiseDirectMouse();
void DirectReadMouse(void);
void ReleaseDirectMouse(void);

/*
	Internal
*/
//#ifdef AVP_DEBUG_VERSION
int textprint(const char* t, ...);
//#else
//#define textprint(ignore)
//#endif


void ConvertToDDPalette(unsigned char* src, unsigned char* dst, int length, int flags);
int textprintXY(int x, int y, const char* t, ...);
void LoadSystemFonts(char* fname);
void DisplayWin95String(int x, int y, unsigned char *buffer);
void WriteStringToTextBuffer(int x, int y, unsigned char *buffer);
void FlushTextprintBuffer(void);
void InitPrintQueue(void);
void InitJoysticks(void);
void ReadJoysticks(void);
//int ChangeDisplayModes(HINSTANCE hInst, int nCmd, 
//     int NewVideoMode, int NewWindowMode,
//     int NewZBufferMode, int NewRasterisationMode, 
//     int NewSoftwareScanDrawMode, int NewDXMemoryMode);
int DeallocateAllImages(void);
int MinimizeAllImages(void);
int RestoreAllImages(void);
void ConvertDDToInternalPalette(unsigned char* src, unsigned char* dst, int length);
PROCESSORTYPES ReadProcessorType(void);

//extern DDCAPS direct_draw_caps;

/*

 Jake's image functions

*/

void * CopyD3DTexture(struct imageheader *iheader);

#ifdef MaxImageGroups

	#if (MaxImageGroups > 1)

		void SetCurrentImageGroup(unsigned int group);
		int DeallocateCurrentImages(void);

	#endif /* MaxImageGroups > 1 */

#endif /* defined(MaxImageGroups) */


/*
	Project callbacks
*/

void ExitGame(void);

void ProjectSpecificBufferFlipPostProcessing();

void ProjectSpecificItemListPostProcessing();

#if optimiseflip
void ProcessProjectWhileWaitingToBeFlippable();
#endif

#ifdef __cplusplus
};
#endif


#endif
