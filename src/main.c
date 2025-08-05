#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include <SDL3/SDL.h>
#include "oglfunc.h"

#if !defined(_MSC_VER)
#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif
#include <getopt.h>
#endif

#include "fixer.h"

#include "3dc.h"
#include "platform.h"
#include "inline.h"
#include "gamedef.h"
#include "gameplat.h"
#include "ffstdio.h"
#include "vision.h"
#include "comp_shp.h"
#include "avp_envinfo.h"
#include "stratdef.h"
#include "bh_types.h"
#include "avp_userprofile.h"
#include "pldnet.h"
#include "cdtrackselection.h"
#include "gammacontrol.h"
#include "opengl.h"
#include "avp_menus.h"
#include "avp_mp_config.h"
#include "npcsetup.h"
#include "cdplayer.h"
#include "hud.h"
#include "player.h"
#include "mempool.h"
#include "avpview.h"
#include "consbind.hpp"
#include "progress_bar.h"
#include "scrshot.hpp"
#include "version.h"
#include "fmv.h"

static inline void secure_avpzero(void* p, size_t n) {
	volatile unsigned char* vp = (volatile unsigned char*)p;
	while (n--) *vp++ = 0;
}

#if defined(__IPHONEOS__) || defined(__ANDROID__)
#define FIXED_WINDOW_SIZE 1
#endif

#if defined(__IPHONEOS__) || defined(__ANDROID__)
#define USE_OPENGL_ES 1
#endif

void RE_ENTRANT_QUEUE_WinProc_AddMessage_WM_CHAR(char Ch);
void RE_ENTRANT_QUEUE_WinProc_AddMessage_WM_KEYDOWN(int wParam);

static bool SDLCALL SDLEventFilter(void* userData, SDL_Event* event);

char LevelName[] = {"predbit6\0QuiteALongNameActually"}; /* the real way to load levels */

int DebouncedGotAnyKey;
unsigned char DebouncedKeyboardInput[MAX_NUMBER_OF_INPUT_KEYS];
int GotJoystick;
int GotMouse;
int JoystickEnabled;
int MouseVelX;
int MouseVelY;

extern int ScanDrawMode;
extern SCREENDESCRIPTORBLOCK ScreenDescriptorBlock;
extern unsigned char KeyboardInput[MAX_NUMBER_OF_INPUT_KEYS];
extern unsigned char GotAnyKey;
extern int NormalFrameTime;

SDL_Window *window;
SDL_GLContext context;
SDL_Surface *surface;

SDL_Joystick *joy;
JOYINFOEX JoystickData;
JOYCAPS JoystickCaps;

// Window configuration and state
static int WindowWidth;
static int WindowHeight;
static int ViewportWidth;
static int ViewportHeight;

enum RENDERING_MODE {
	RENDERING_MODE_SOFTWARE,
	RENDERING_MODE_OPENGL
};

enum RENDERING_MODE RenderingMode;

#if defined(FIXED_WINDOW_SIZE)
static int WantFullscreen = 1;
static int WantFullscreenToggle = 0;
static int WantResolutionChange = 0;
static int WantMouseGrab = 1;
#else
static int WantFullscreen = 1;
static int WantFullscreenToggle = 1;
static int WantResolutionChange = 1;
static int WantMouseGrab = 1;
#endif

// Additional configuration
int WantSound = 1;
static int WantCDRom = 0;
static int WantJoystick = 0;

static GLuint FullscreenTexture;
static GLsizei FullscreenTextureWidth;
static GLsizei FullscreenTextureHeight;

/* originally was "/usr/lib/libGL.so.1:/usr/lib/tls/libGL.so.1:/usr/X11R6/lib/libGL.so" */
static const char * opengl_library = NULL;

static char * gamedatapath = NULL;

/* ** */

static void IngameKeyboardInput_ClearBuffer(void)
{
	// clear the keyboard state
	memset((void*) KeyboardInput, 0, MAX_NUMBER_OF_INPUT_KEYS);
	GotAnyKey = 0;
}

void DirectReadKeyboard()
{
}

void DirectReadMouse()
{
}

void ReadJoysticks()
{
	int axes, balls, hats;
	Uint8 hat;
	
	JoystickData.dwXpos = 0;
	JoystickData.dwYpos = 0;
	JoystickData.dwRpos = 0;
	JoystickData.dwUpos = 0;
	JoystickData.dwVpos = 0;
	JoystickData.dwPOV = (DWORD) -1;	
	
	if (joy == NULL || !GotJoystick) {
		return;
	}

	SDL_UpdateJoysticks();
	
	axes = SDL_GetNumJoystickAxes(joy);
	balls = SDL_GetNumJoystickBalls(joy);
	hats = SDL_GetNumJoystickHats(joy);
	
	if (axes > 0) {
		JoystickData.dwXpos = SDL_GetJoystickAxis(joy, 0) + 32768;
	}
	if (axes > 1) {
		JoystickData.dwYpos = SDL_GetJoystickAxis(joy, 1) + 32768;
	}
	
	if (hats > 0) {
		hat = SDL_GetJoystickHat(joy, 0);
		
		switch (hat) {
			default:
			case SDL_HAT_CENTERED:
				JoystickData.dwPOV = (DWORD) -1;
				break;
			case SDL_HAT_UP:
				JoystickData.dwPOV = 0;
				break;
			case SDL_HAT_RIGHT:
				JoystickData.dwPOV = 9000;
				break;
			case SDL_HAT_DOWN:
				JoystickData.dwPOV = 18000;
				break;
			case SDL_HAT_LEFT:
				JoystickData.dwPOV = 27000;
				break;
			case SDL_HAT_RIGHTUP:
				JoystickData.dwPOV = 4500;
				break;
			case SDL_HAT_RIGHTDOWN:
				JoystickData.dwPOV = 13500;
				break;
			case SDL_HAT_LEFTUP:
				JoystickData.dwPOV = 31500;
				break;
			case SDL_HAT_LEFTDOWN:
				JoystickData.dwPOV = 22500;
				break;
		}
	}
}

/* ** */

unsigned char *GetScreenShot24(int *width, int *height)
{
	unsigned char *buf;

	if (surface == NULL) {
		return NULL;
	}

	if (RenderingMode == RENDERING_MODE_OPENGL) {
		buf = (unsigned char *)malloc(ViewportWidth * ViewportHeight * 3);

		*width = ViewportWidth;
		*height = ViewportHeight;

		pglPixelStorei(GL_PACK_ALIGNMENT, 1);
		pglPixelStorei(GL_UNPACK_ALIGNMENT, 1);
		pglReadPixels(0, 0, ViewportWidth, ViewportHeight, GL_RGB, GL_UNSIGNED_BYTE, buf);
	} else {
		buf = (unsigned char *)malloc(surface->w * surface->h * 3);

		unsigned char *ptrd;
		unsigned short int *ptrs;
		int x, y;
	
		if (SDL_MUSTLOCK(surface)) {
			if (SDL_LockSurface(surface) < 0) {
				free(buf);
				return NULL; /* ... */
			}
		}
		
		ptrd = buf;
		for (y = 0; y < surface->h; y++) {
			ptrs = (unsigned short *)(((unsigned char *)surface->pixels) + (surface->h-y-1)*surface->pitch);
			for (x = 0; x < surface->w; x++) {
				unsigned int c;
				
				c = *ptrs;
				ptrd[0] = (c & 0xF800)>>8;
				ptrd[1] = (c & 0x07E0)>>3;
				ptrd[2] = (c & 0x001F)<<3;
				
				ptrs++;
				ptrd += 3;
			}
		}
		
		*width = surface->w;
		*height = surface->h;

		if (SDL_MUSTLOCK(surface)) {
			SDL_UnlockSurface(surface);
		}
	}

#if 0
	Uint16 redtable[256], greentable[256], bluetable[256];
	
	if (SDL_GetGammaRamp(redtable, greentable, bluetable) != -1) {
		unsigned char *ptr;
		int i;
		
		ptr = buf;
		for (i = 0; i < surface->w*surface->h; i++) {
			ptr[i*3+0] = redtable[ptr[i*3+0]]>>8;
			ptr[i*3+1] = greentable[ptr[i*3+1]]>>8;
			ptr[i*3+2] = bluetable[ptr[i*3+2]]>>8;
			ptr += 3;
		}
	}
#endif

	return buf;
}

/* ** */

PROCESSORTYPES ReadProcessorType()
{
	return PType_PentiumMMX;
}

/* ** */

typedef struct VideoModeStruct
{
	int w;
	int h;
	int available;
} VideoModeStruct;
VideoModeStruct VideoModeList[] = {
	{ 	512, 	384,	0	},
	{	640,	480,	0	},
	{	800,	600,	0	},
	{	1024,	768,	0	},
	{	1152,	864,	0	},
	{	1280,   720,	0	},
	{	1280,	960,	0	},
	{	1280,	1024,	0	},
	{	1366,	768,	0	},
	{	1600,	1200,	0	},
	{	1680,	1050,	0	},
	{	1920,	1080,	0	},
	{   2048,   1080,   0   },
	{	2560,	1080,	0	},
	{	2560,	1440,	0	},
	{	2560,	1600,	0	},
	{	2560,	1664,	0	},
	{   2880,   1620,   0   },
	{   3000,   2000,   0   },
	{   3200,   1800,   0   },
	{   3440,   1440,   0   },
	{	3840,	2160,	0	},
	{   4096,   2160,   0   },
	{	4096,	2304,	0	},
	{	4480,	2520,	0	},
	{	5120,	2880,	0	},
	{	6016,	3384,	0	},
	{   7680,   4320,   0   },
	{   8192,   4320,   0   }
};

int CurrentVideoMode;
const int TotalVideoModes = sizeof(VideoModeList) / sizeof(VideoModeList[0]);

void LoadDeviceAndVideoModePreferences()
{
	FILE *fp;
	int mode;
	
	fp = OpenGameFile("AvP_TempVideo.cfg", FILEMODE_READONLY, FILETYPE_CONFIG);
	
	if (fp != NULL) {
		// fullscreen mode (0=window,1=fullscreen,2=fullscreen desktop)
		// window width
		// window height
		// fullscreen width
		// fullscreen height
		// fullscreen desktop aspect ratio n
		// fullscreen desktop aspect ratio d
		// fullscreen desktop scale n
		// fullscreen desktop scale d
		// multisample number of samples (0/2/4)
	 	if (fscanf(fp, "%d", &mode) == 1) {
			fclose(fp);
		
			if (mode >= 0 && mode < TotalVideoModes && VideoModeList[mode].available) {
				CurrentVideoMode = mode;
				return;
			}
		} else {
			fclose(fp);
		}
	}
	
	/* No, or invalid, mode found */
	
	/* Try 640x480 first */
	if (VideoModeList[1].available) {
		CurrentVideoMode = 1;
	} else {
		int i;
		
		for (i = 0; i < TotalVideoModes; i++) {
			if (VideoModeList[i].available) {
				CurrentVideoMode = i;
				break;
			}
		}
	}
}

void SaveDeviceAndVideoModePreferences()
{
	FILE *fp;
	
	fp = OpenGameFile("AvP_TempVideo.cfg", FILEMODE_WRITEONLY, FILETYPE_CONFIG);
	if (fp != NULL) {
		fprintf(fp, "%d\n", CurrentVideoMode);
		fclose(fp);
	}
}

void PreviousVideoMode2()
{
	int cur = CurrentVideoMode;

	do {
		if (cur == 0)
			cur = TotalVideoModes;	
		cur--;
		if (cur == CurrentVideoMode)
			return;
	} while(!VideoModeList[cur].available);
	
	CurrentVideoMode = cur;
}

void NextVideoMode2()
{
	int cur = CurrentVideoMode;

	do {
		cur++;
		if (cur == TotalVideoModes)
			cur = 0;

		if (cur == CurrentVideoMode)
			return;
	} while(!VideoModeList[cur].available);
	
	CurrentVideoMode = cur;
}

char *GetVideoModeDescription2()
{
	return "SDL3";
}

char *GetVideoModeDescription3()
{
	static char buf[64];
	
	_snprintf(buf, 64, "%dx%d", VideoModeList[CurrentVideoMode].w, VideoModeList[CurrentVideoMode].h);

	return buf;
}

int InitSDL()
{
	if (SDL_Init(SDL_INIT_VIDEO) < 0) {
		fprintf(stderr, "SDL Init failed: %s\n", SDL_GetError());
		exit(EXIT_FAILURE);
	}

	atexit(SDL_Quit);

	SDL_AddEventWatch(SDLEventFilter, NULL);

#if 0
	SDL_Rect **SDL_AvailableVideoModes;
	SDL_AvailableVideoModes = SDL_ListModes(NULL, SDL_WINDOW_FULLSCREEN | SDL_WINDOW_OPENGL);
	if (SDL_AvailableVideoModes == NULL)
		return -1;
	
	if (SDL_AvailableVideoModes != (SDL_Rect **)-1) {
		int i, j, foundit;
		
		foundit = 0;
		for (i = 0; i < TotalVideoModes; i++) {
			SDL_Rect **modes = SDL_AvailableVideoModes;
			
			for (j = 0; modes[j]; j++) {
				if (modes[j]->w >= VideoModeList[i].w &&
				    modes[j]->h >= VideoModeList[i].h) {
					if (SDL_VideoModeOK(VideoModeList[i].w, VideoModeList[i].h, 16, SDL_FULLSCREEN | SDL_OPENGL)) {
						/* assume SDL isn't lying to us */
						VideoModeList[i].available = 1;
						
						foundit = 1;
					}
					break;
				}
			}			
		}		
		if (foundit == 0)
			return -1;
	} else {
		int i, foundit;
		
		foundit = 0;
		for (i = 0; i < TotalVideoModes; i++) {
			if (SDL_VideoModeOK(VideoModeList[i].w, VideoModeList[i].h, 16, SDL_FULLSCREEN | SDL_OPENGL)) {
				/* assume SDL isn't lying to us */
				VideoModeList[i].available = 1;
				
				foundit = 1;
			}
		}
		
		if (foundit == 0)
			return -1;
	}
#endif

{
	int i;
	
	for (i = 0; i < TotalVideoModes; i++) {
		//if (SDL_VideoModeOK(VideoModeList[i].w, VideoModeList[i].h, 16, SDL_FULLSCREEN | SDL_OPENGL)) {
			/* assume SDL isn't lying to us */
			VideoModeList[i].available = 1;
			
			//foundit = 1;
		//}
	}
}

	LoadDeviceAndVideoModePreferences();

	if (WantJoystick) {
		SDL_InitSubSystem(SDL_INIT_JOYSTICK);
			
		joy = SDL_OpenJoystick(0);
		if (joy) {
			GotJoystick = 1;
			
			JoystickCaps.wCaps = 0; /* no rudder... ? */
			
			JoystickData.dwXpos = 0;
			JoystickData.dwYpos = 0;
			JoystickData.dwRpos = 0;
			JoystickData.dwUpos = 0;
			JoystickData.dwVpos = 0;
			JoystickData.dwPOV = (DWORD) -1;
		}
	}
	
	Uint32 rmask, gmask, bmask, amask;
	
	// pre-create the software surface in OpenGL RGBA order
	// menus.c assumes RGB565; possible to support both?
#if SDL_BYTEORDER == SDL_BIG_ENDIAN
    rmask = 0xff000000;
    gmask = 0x00ff0000;
    bmask = 0x0000ff00;
    amask = 0x000000ff;
#else
    rmask = 0x0000f800;
    gmask = 0x000007e0;
    bmask = 0x0000001f;
    amask = 0x00000000;
#endif

	surface = SDL_CreateSurface(640, 480, SDL_PIXELFORMAT_RGBA5551);
	if (surface == NULL) {
		return -1;
	}

	return 0;
}

static void SetWindowSize(int PhysicalWidth, int PhysicalHeight, int VirtualWidth, int VirtualHeight)
{
#if !defined(NDEBUG)
	fprintf(stderr, "SetWindowSize(%d,%d,%d,%d); %d\n", PhysicalWidth, PhysicalHeight, VirtualWidth, VirtualHeight, CurrentVideoMode);
#endif

	ViewportWidth = PhysicalWidth;
	ViewportHeight = PhysicalHeight;

	ScreenDescriptorBlock.SDB_Width     = VirtualWidth;
	ScreenDescriptorBlock.SDB_Height    = VirtualHeight;
	ScreenDescriptorBlock.SDB_CentreX   = VirtualWidth/2;
	ScreenDescriptorBlock.SDB_CentreY   = VirtualHeight/2;
	ScreenDescriptorBlock.SDB_ProjX     = VirtualWidth/2;
	ScreenDescriptorBlock.SDB_ProjY     = VirtualHeight/2;
	ScreenDescriptorBlock.SDB_ClipLeft  = 0;
	ScreenDescriptorBlock.SDB_ClipRight = VirtualWidth;
	ScreenDescriptorBlock.SDB_ClipUp    = 0;
	ScreenDescriptorBlock.SDB_ClipDown  = VirtualHeight;

	if (window != NULL) {
		SDL_SetWindowSize(window, PhysicalWidth, PhysicalHeight);
	}
}

static int SetSoftVideoMode(int Width, int Height, int Depth)
{
	//TODO: clear surface

	RenderingMode = RENDERING_MODE_SOFTWARE;
	ScanDrawMode = ScanDrawD3DHardwareRGB;
	GotMouse = 1;

	// reset input
	IngameKeyboardInput_ClearBuffer();

	SetWindowSize(ViewportWidth, ViewportHeight, Width, Height);

	return 0;
}

/* ** */
static bool SDLCALL SDLEventFilter(void* userData, SDL_Event* event) {
	(void) userData;

	//printf("SDLEventFilter: %d\n", event->type);
	
	switch (event->type) {
		case SDL_EVENT_TERMINATING:
			AvP.MainLoopRunning = 0; /* TODO */
			break;
	}
	
	return true;
}

static int InitSDLVideo(void) {
	return 0;
}

static int SetOGLVideoMode(int Width, int Height)
{
	int oldflags;
	int flags;
	
	RenderingMode = RENDERING_MODE_OPENGL;
	ScanDrawMode = ScanDrawD3DHardwareRGB;
	GotMouse = 1;

#if defined(FIXED_WINDOW_SIZE)
	// force the game to use the full desktop
	SDL_DisplayMode dm;
	if (SDL_GetDesktopDisplayMode(0, &dm) == 0) {
		Width = dm.w;
		Height = dm.h;
	}
#endif

	if (window == NULL) {
		load_ogl_functions(0);

		flags = SDL_WINDOW_OPENGL;

#if defined(FIXED_WINDOW_SIZE)
		flags |= SDL_WINDOW_BORDERLESS;
		flags |= SDL_WINDOW_FULLSCREEN_DESKTOP;
#else
		if (WantFullscreen) {
			flags |= (WantResolutionChange != 0 ? SDL_WINDOW_FULLSCREEN : SDL_WINDOW_FULLSCREEN);
		}

		// the game doesn't properly support window resizing
		//flags |= SDL_WINDOW_RESIZABLE;
#endif

		// reset input
		IngameKeyboardInput_ClearBuffer();
		
		// force restart the video system
		SDL_QuitSubSystem(SDL_INIT_VIDEO);
		SDL_InitSubSystem(SDL_INIT_VIDEO);

		// set OpenGL attributes first
#if defined(USE_OPENGL_ES)
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 1);
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);
#else
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);
#endif
		// These should be configurable video options.
		// If user requests 8bpp, try that, else fall back to 5.
		// Same with depth.  Try 32, 24, 16.
		SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 5);
		SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 5);
		SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 5);
		SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 16);
		SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

		// These should be configurable video options.
		SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 1);
		SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 4);

		window = SDL_CreateWindow("Aliens vs Predator",
								  WindowWidth,
								  WindowHeight,
								  flags);
		if (window == NULL) {
			fprintf(stderr, "(OpenGL) SDL SDL_CreateWindow failed: %s\n", SDL_GetError());
			exit(EXIT_FAILURE);
		}
		
		context = SDL_GL_CreateContext(window);
		if (context == NULL) {
			fprintf(stderr, "(OpenGL) SDL SDL_GL_CreateContext failed: %s\n", SDL_GetError());
			exit(EXIT_FAILURE);
		}
		SDL_GL_MakeCurrent(window, context);

		// These should be configurable video options.
		SDL_GL_SetSwapInterval(1);

		load_ogl_functions(1);

		SDL_GetWindowSize(window, &Width, &Height);
		pglViewport(0, 0, Width, Height);

		// create fullscreen window texture
		pglGenTextures(1, &FullscreenTexture);

		pglBindTexture(GL_TEXTURE_2D, FullscreenTexture);

		pglTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		pglTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		pglTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		pglTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

		FullscreenTextureWidth = 1024;
		FullscreenTextureHeight = 512;
		pglTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, FullscreenTextureWidth, FullscreenTextureHeight, 0, GL_RGB, GL_UNSIGNED_SHORT_5_6_5, NULL);
	}

	SDL_GetWindowSize(window, &Width, &Height);
	
	SetWindowSize(Width, Height, Width, Height);
	
	int NewWidth, NewHeight;
	SDL_GetWindowSize(window, &Width, &Height);
	if (Width != NewWidth || Height != NewHeight) {
		//printf("Failed to change size: %d,%d vs. %d,%d\n", Width, Height, NewWidth, NewHeight);
		//Width = NewWidth;
		//Height = NewHeight;
		//SetWindowSize(Width, Height, Width, Height);
	}

	pglEnable(GL_BLEND);
	pglBlendFunc(GL_SRC_ALPHA, GL_ONE);
		
	pglEnable(GL_DEPTH_TEST);
	pglDepthFunc(GL_LEQUAL);
	pglDepthMask(GL_TRUE);
	pglDepthRange(0.0, 1.0);
		
	pglEnable(GL_TEXTURE_2D);

	pglDisable(GL_CULL_FACE);
	
	pglClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	
	InitOpenGL();
	
	return 0;
}

int InitialiseWindowsSystem(HANDLE hInstance, int nCmdShow, int WinInitMode)
{
	return 0;
}

int ExitWindowsSystem()
{
	if (joy != NULL) {
		SDL_CloseJoystick(joy);
	}

	if (FullscreenTexture != 0) {
		pglDeleteTextures(1, &FullscreenTexture);
	}
	FullscreenTexture = 0;

	load_ogl_functions(0);
	
	if (surface != NULL) {
		SDL_DestroySurface(surface);
	}
	surface = NULL;

	if (context != NULL) {
		SDL_GL_DestroyContext(context);
	}
	context = NULL;

	if (window != NULL) {
		SDL_DestroyWindow(window);
	}
	window = NULL;

	return 0;
}

static int GotPrintScn, HavePrintScn;

static int KeySymToKey(int keysym)
{
	switch(keysym) {
		case SDLK_ESCAPE:
			return KEY_ESCAPE;
			
		case SDLK_0:
			return KEY_0;
		case SDLK_1:
			return KEY_1;
		case SDLK_2:
			return KEY_2;
		case SDLK_3:
			return KEY_3;
		case SDLK_4:
			return KEY_4;
		case SDLK_5:
			return KEY_5;
		case SDLK_6:
			return KEY_6;
		case SDLK_7:
			return KEY_7;
		case SDLK_8:
			return KEY_8;
		case SDLK_9:
			return KEY_9;
		
		case SDLK_A:
			return KEY_A;
		case SDLK_B:
			return KEY_B;
		case SDLK_C:
			return KEY_C;
		case SDLK_D:
			return KEY_D;
		case SDLK_E:
			return KEY_E;
		case SDLK_F:
			return KEY_F;
		case SDLK_G:
			return KEY_G;
		case SDLK_H:
			return KEY_H;
		case SDLK_I:
			return KEY_I;
		case SDLK_J:
			return KEY_J;
		case SDLK_K:
			return KEY_K;
		case SDLK_L:
			return KEY_L;
		case SDLK_M:
			return KEY_M;
		case SDLK_N:
			return KEY_N;
		case SDLK_O:
			return KEY_O;
		case SDLK_P:
			return KEY_P;
		case SDLK_Q:
			return KEY_Q;
		case SDLK_R:
			return KEY_R;
		case SDLK_S:
			return KEY_S;
		case SDLK_T:
			return KEY_T;
		case SDLK_U:
			return KEY_U;
		case SDLK_V:
			return KEY_V;
		case SDLK_W:
			return KEY_W;
		case SDLK_X:
			return KEY_X;
		case SDLK_Y:
			return KEY_Y;
		case SDLK_Z:
			return KEY_Z;
				
		case SDLK_LEFT:
			return KEY_LEFT;
		case SDLK_RIGHT:
			return KEY_RIGHT;
		case SDLK_UP:
			return KEY_UP;
		case SDLK_DOWN:
			return KEY_DOWN;		
		case SDLK_RETURN:
			return KEY_CR;
		case SDLK_TAB:
			return KEY_TAB;
		case SDLK_INSERT:
			return KEY_INS;
		case SDLK_DELETE:
			return KEY_DEL;
		case SDLK_END:
			return KEY_END;
		case SDLK_HOME:
			return KEY_HOME;
		case SDLK_PAGEUP:
			return KEY_PAGEUP;
		case SDLK_PAGEDOWN:
			return KEY_PAGEDOWN;
		case SDLK_BACKSPACE:
			return KEY_BACKSPACE;
		case SDLK_COMMA:
			return KEY_COMMA;
		case SDLK_PERIOD:
			return KEY_FSTOP;
		case SDLK_SPACE:
			return KEY_SPACE;
			
		case SDLK_LSHIFT:
			return KEY_LEFTSHIFT;
		case SDLK_RSHIFT:
			return KEY_RIGHTSHIFT;
		case SDLK_LALT:
			return KEY_LEFTALT;
		case SDLK_RALT:
			return KEY_RIGHTALT;
		case SDLK_LCTRL:
			return KEY_LEFTCTRL;
		case SDLK_RCTRL:
			return KEY_RIGHTCTRL;

		case SDLK_CAPSLOCK:
			return KEY_CAPS;
		case SDLK_NUMLOCKCLEAR:
			return KEY_NUMLOCK;
		case SDLK_SCROLLLOCK:
			return KEY_SCROLLOK;
			
		case SDLK_KP_0:
			return KEY_NUMPAD0;
		case SDLK_KP_1:
			return KEY_NUMPAD1;
		case SDLK_KP_2:
			return KEY_NUMPAD2;
		case SDLK_KP_3:
			return KEY_NUMPAD3;
		case SDLK_KP_4:
			return KEY_NUMPAD4;
		case SDLK_KP_5:
			return KEY_NUMPAD5;
		case SDLK_KP_6:
			return KEY_NUMPAD6;
		case SDLK_KP_7:
			return KEY_NUMPAD7;
		case SDLK_KP_8:
			return KEY_NUMPAD8;
		case SDLK_KP_9:
			return KEY_NUMPAD9;
		case SDLK_KP_MINUS:
			return KEY_NUMPADSUB;
		case SDLK_KP_PLUS:
			return KEY_NUMPADADD;
		case SDLK_KP_PERIOD:
			return KEY_NUMPADDEL;
		case SDLK_KP_ENTER:
			return KEY_NUMPADENTER;
		case SDLK_KP_DIVIDE:
			return KEY_NUMPADDIVIDE;
		case SDLK_KP_MULTIPLY:
			return KEY_NUMPADMULTIPLY;
	
		case SDLK_LEFTBRACKET:
			return KEY_LBRACKET;
		case SDLK_RIGHTBRACKET:
			return KEY_RBRACKET;
		case SDLK_SEMICOLON:
			return KEY_SEMICOLON;
		case SDLK_APOSTROPHE:
			return KEY_APOSTROPHE;
		case SDLK_GRAVE:
			return KEY_GRAVE;
		case SDLK_BACKSLASH:
			return KEY_BACKSLASH;
		case SDLK_SLASH:
			return KEY_SLASH;
/*		case SDLK_
			return KEY_CAPITAL; */
		case SDLK_MINUS:
			return KEY_MINUS;
		case SDLK_EQUALS:
			return KEY_EQUALS;
		case SDLK_LGUI:
			return KEY_LWIN;
		case SDLK_RGUI:
			return KEY_RWIN;
/*		case SDLK_
			return KEY_APPS; */
		
		case SDLK_F1:
			return KEY_F1;
		case SDLK_F2:
			return KEY_F2;
		case SDLK_F3:
			return KEY_F3;
		case SDLK_F4:
			return KEY_F4;
		case SDLK_F5:
			return KEY_F5;
		case SDLK_F6:
			return KEY_F6;
		case SDLK_F7:
			return KEY_F7;
		case SDLK_F8:
			return KEY_F8;
		case SDLK_F9:
			return KEY_F9;
		case SDLK_F10:
			return KEY_F10;
		case SDLK_F11:
			return KEY_F11;
		case SDLK_F12:
			return KEY_F12;

/* finish foreign keys */

		default:
			return -1;
	}
}

char ShiftDown = 0;
char CapsLockOn = 0;
const char ShiftAddition[2] = { 32, 0 };

static void handle_keypress(int key, int unicode, int press)
{	
	if (key == -1)
		return;

	// Nasty hack to allow temporary character entry by TCH68k on Github
	
	if ((key == KEY_LEFTSHIFT) || (key == KEY_RIGHTSHIFT))
	{
		ShiftDown = press;
	}
	else if (press) {
		if ((key >= KEY_A) && (key <= KEY_Z))
		{
			RE_ENTRANT_QUEUE_WinProc_AddMessage_WM_CHAR(65 + (key - KEY_A) + ShiftAddition[ShiftDown ^ CapsLockOn]);
		}
		else if ((key >= KEY_0) && (key <= KEY_9))
		{
			RE_ENTRANT_QUEUE_WinProc_AddMessage_WM_CHAR(48 + (key - KEY_0)); /* TODO: Shift numbers -> symbols */
		}
		else if ((key >= KEY_NUMPAD0) && (key <= KEY_NUMPAD9))
		{
			RE_ENTRANT_QUEUE_WinProc_AddMessage_WM_CHAR(48 + (key - KEY_NUMPAD0));
		}
		else if (false) /* TODO: other symbols */
		{
			// BOO!
		}
	else switch (key) {
		case KEY_CAPS:
			CapsLockOn ^= 1;
			break;
			case KEY_CR:
				SDL_StartTextInput(NULL);
				RE_ENTRANT_QUEUE_WinProc_AddMessage_WM_CHAR('\r');
				break;
			case KEY_BACKSPACE:
				RE_ENTRANT_QUEUE_WinProc_AddMessage_WM_KEYDOWN(VK_BACK);
				break;
			case KEY_END:
				RE_ENTRANT_QUEUE_WinProc_AddMessage_WM_KEYDOWN(VK_END);
				break;
			case KEY_HOME:
				RE_ENTRANT_QUEUE_WinProc_AddMessage_WM_KEYDOWN(VK_HOME);
				break;
			case KEY_LEFT:
				RE_ENTRANT_QUEUE_WinProc_AddMessage_WM_KEYDOWN(VK_LEFT);
				break;
			case KEY_UP:
				RE_ENTRANT_QUEUE_WinProc_AddMessage_WM_KEYDOWN(VK_UP);
				break;
			case KEY_RIGHT:
				RE_ENTRANT_QUEUE_WinProc_AddMessage_WM_KEYDOWN(VK_RIGHT);
				break;
			case KEY_DOWN:
				RE_ENTRANT_QUEUE_WinProc_AddMessage_WM_KEYDOWN(VK_DOWN);
				break;
			case KEY_INS:
				RE_ENTRANT_QUEUE_WinProc_AddMessage_WM_KEYDOWN(VK_INSERT);
				break;
			case KEY_DEL:
				RE_ENTRANT_QUEUE_WinProc_AddMessage_WM_KEYDOWN(VK_DELETE);
				break;
			case KEY_TAB:
				RE_ENTRANT_QUEUE_WinProc_AddMessage_WM_KEYDOWN(VK_TAB);
				break;
			default:
				SDL_StartTextInput(NULL);
				break;
		}
	}
	
	if (press && !KeyboardInput[key]) {
		DebouncedKeyboardInput[key] = 1;
		DebouncedGotAnyKey = 1;
	}
	
	if (press)
		GotAnyKey = 1;
	KeyboardInput[key] = press;
}

void CheckForWindowsMessages()
{
	SDL_Event event;
	float x, y, wantmouse;
	int buttons;
	
	GotAnyKey = 0;
	DebouncedGotAnyKey = 0;
	secure_avpzero(DebouncedKeyboardInput, sizeof DebouncedKeyboardInput);
	
	wantmouse =	(SDL_GetWindowRelativeMouseMode(window) == true);

	// "keyboard" events that don't have an up event
	KeyboardInput[KEY_MOUSEWHEELUP] = 0;
	KeyboardInput[KEY_MOUSEWHEELDOWN] = 0;
	
	while (SDL_PollEvent(&event)) {
		switch(event.type) {
			case SDL_EVENT_MOUSE_BUTTON_DOWN:
				break;
			case SDL_EVENT_MOUSE_BUTTON_UP:
				break;
			case SDL_EVENT_MOUSE_WHEEL:
				if (wantmouse) {
					if (event.wheel.y < 0) {
						handle_keypress(KEY_MOUSEWHEELDOWN, 0, 1);
					} else if (event.wheel.y > 0) {
						handle_keypress(KEY_MOUSEWHEELUP, 0, 1);
					}
				}
				break;
			case SDL_EVENT_TEXT_INPUT: {
				SDL_StartTextInput(NULL);
					int unicode = event.text.text[0]; //TODO convert to utf-32
					if (unicode && !(unicode & 0xFF80)) {
						RE_ENTRANT_QUEUE_WinProc_AddMessage_WM_CHAR(unicode);
						KeyboardEntryQueue_Add(unicode);
					}
				}
				break;
			case SDL_EVENT_KEY_DOWN:
				SDL_StartTextInput(NULL);
				if (event.key.key == SDLK_PRINTSCREEN) {
					if (HavePrintScn == 0)
						GotPrintScn = 1;
					HavePrintScn = 1;
				} else {
					handle_keypress(KeySymToKey(event.key.key), 0, 1);
				}
				break;
			case SDL_EVENT_KEY_UP:
				SDL_StartTextInput(NULL);
				if (event.key.key == SDLK_PRINTSCREEN) {
					GotPrintScn = 0;
					HavePrintScn = 0;
				} else {
					handle_keypress(KeySymToKey(event.key.key), 0, 0);
				}
				break;
			case SDL_EVENT_WINDOW_FOCUS_LOST:
					// disable mouse grab?
				break;
			case SDL_EVENT_WINDOW_RESIZED:
					//printf("test, %d,%d\n", event.window.data1, event.window.data2);
					WindowWidth = event.window.data1;
					WindowHeight = event.window.data2;
					if (RenderingMode == RENDERING_MODE_SOFTWARE) {
						SetWindowSize(WindowWidth, WindowHeight, 640, 480);
					} else {
						SetWindowSize(WindowWidth, WindowHeight, WindowWidth, WindowHeight);
					}
					if (pglViewport != NULL) {
						pglViewport(0, 0, WindowWidth, WindowHeight);
					}
				break;
			case SDL_EVENT_QUIT:
				AvP.MainLoopRunning = 0; /* TODO */
				exit(0); //TODO
				SDL_StopTextInput(NULL);
				break;
		}
	}
	
	buttons = SDL_GetRelativeMouseState(&x, &y);
	
	if (wantmouse) {
		if (buttons & SDL_BUTTON_MASK(1))
			handle_keypress(KEY_LMOUSE, 0, 1);
		else
			handle_keypress(KEY_LMOUSE, 0, 0);
		if (buttons & SDL_BUTTON_MASK(2))
			handle_keypress(KEY_MMOUSE, 0, 1);
		else
			handle_keypress(KEY_MMOUSE, 0, 0);
		if (buttons & SDL_BUTTON_MASK(3))
			handle_keypress(KEY_RMOUSE, 0, 1);
		else
			handle_keypress(KEY_RMOUSE, 0, 0);

		MouseVelX = DIV_FIXED(x, NormalFrameTime);
		MouseVelY = DIV_FIXED(y, NormalFrameTime);
	} else {
		KeyboardInput[KEY_LMOUSE] = 0;
		KeyboardInput[KEY_MMOUSE] = 0;
		KeyboardInput[KEY_RMOUSE] = 0;
		MouseVelX = 0;
		MouseVelY = 0;
	}

	if (GotJoystick) {
		float numbuttons;
		int x;
		
		SDL_UpdateJoysticks();
		
		numbuttons = SDL_GetNumJoystickButtons(joy);
		if (numbuttons > 16) numbuttons = 16;
		
		for (x = 0; x < numbuttons; x++) {
			if (SDL_GetJoystickButton(joy, x)) {
				GotAnyKey = 1;
				if (!KeyboardInput[KEY_JOYSTICK_BUTTON_1+x]) {
					KeyboardInput[KEY_JOYSTICK_BUTTON_1+x] = 1;
					DebouncedKeyboardInput[KEY_JOYSTICK_BUTTON_1+x] = 1;
				}
			} else {
				KeyboardInput[KEY_JOYSTICK_BUTTON_1+x] = 0;
			}	
		}
	}

//#warning Redo WantX, need to split it out better so fullscreen can temporary set relative without clobbering user setting
	if ((KeyboardInput[KEY_LEFTALT]||KeyboardInput[KEY_RIGHTALT]) && DebouncedKeyboardInput[KEY_CR]) {
		if (WantFullscreenToggle != 0) {
			int displayMode = SDL_GetWindowFlags(window);
			//printf("Current window mode:%08x\n", displayMode);
			if ((displayMode & (SDL_WINDOW_FULLSCREEN|SDL_WINDOW_FULLSCREEN)) != 0) {
				SDL_SetWindowFullscreen(window, 0);
			} else {
				SDL_SetWindowFullscreen(window, WantResolutionChange ? SDL_WINDOW_FULLSCREEN : SDL_WINDOW_FULLSCREEN);
			}

			displayMode = SDL_GetWindowFlags(window);
			//printf("New window mode:%08x\n", displayMode);
			if ((displayMode & (SDL_WINDOW_FULLSCREEN|SDL_WINDOW_FULLSCREEN)) != 0) {
				SDL_SetWindowRelativeMouseMode(window, true);
				WantFullscreen = 1;
			} else {
				SDL_SetWindowRelativeMouseMode(window, WantMouseGrab ? true : false);
				WantFullscreen = 0;
			}
		}
	}

	if (KeyboardInput[KEY_LEFTCTRL] && DebouncedKeyboardInput[KEY_G]) {
		int IsWindowed = (SDL_GetWindowFlags(window) & (SDL_WINDOW_FULLSCREEN|SDL_WINDOW_FULLSCREEN)) == 0;

		if (IsWindowed) {
			WantMouseGrab = WantMouseGrab != 0 ? 0 : 1;
			if (WantMouseGrab != 0) {
				SDL_SetWindowRelativeMouseMode(window, true);
			} else {
				SDL_SetWindowRelativeMouseMode(window, false);
			}
			WantMouseGrab = (SDL_GetWindowRelativeMouseMode(window) == true);
		}
	}

	// a second reset of relative mouse state because
	// enabling relative mouse mode moves the mouse
	SDL_SetWindowRelativeMouseMode(window, true);
        SDL_GetRelativeMouseState(NULL, NULL);

	if (GotPrintScn) {
		GotPrintScn = 0;
		
		ScreenShot();
	}
}
        
void InGameFlipBuffers()
{
#if !defined(NDEBUG)
	check_for_errors();
#endif

	SDL_GL_SwapWindow(window);
}

void FlipBuffers()
{
	pglClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	pglDisable(GL_ALPHA_TEST);
	pglDisable(GL_BLEND);
	pglDisable(GL_DEPTH_TEST);

	pglBindTexture(GL_TEXTURE_2D, FullscreenTexture);
	pglTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	pglTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	pglTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
	pglTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 640, 480, GL_RGB, GL_UNSIGNED_SHORT_5_6_5, surface->pixels);

	GLfloat x0;
	GLfloat x1;
	GLfloat y0;
	GLfloat y1;
	GLfloat s0;
	GLfloat s1;
	GLfloat t0;
	GLfloat t1;

	// figure out the best way to fit the 640x480 virtual window
	GLfloat a = ViewportHeight * 640.0f / 480.0f;
	GLfloat b = ViewportWidth * 480.0f / 640.0f;

	if (a <= ViewportWidth) {
		// a x ViewportHeight window
		y0 = -1.0f;
		y1 =  1.0f;

		x1 = 1.0 - (ViewportWidth - a) / ViewportWidth;
		x0 = -x1;
	} else {
		// ViewportWidth x b window
		x0 = -1.0f;
		x1 =  1.0f;

		y1 = 1.0 - (ViewportHeight - b) / ViewportHeight;
		y0 = -y1;
	}

	s0 = 0.0f;
	s1 = 640.0f / (float) FullscreenTextureWidth;
	t0 = 0.0f;
	t1 = 480.0f / (float) FullscreenTextureHeight;

	GLfloat v[4*4];
	GLshort s[6];

	v[0+0*4] = x0;
	v[1+0*4] = y0;
	v[2+0*4] = s0;
	v[3+0*4] = t1;
	v[0+1*4] = x1;
	v[1+1*4] = y0;
	v[2+1*4] = s1;
	v[3+1*4] = t1;
	v[0+2*4] = x1;
	v[1+2*4] = y1;
	v[2+2*4] = s1;
	v[3+2*4] = t0;
	v[0+3*4] = x0;
	v[1+3*4] = y1;
	v[2+3*4] = s0;
	v[3+3*4] = t0;

	s[0] = 0;
	s[1] = 1;
	s[2] = 2;
	s[3] = 0;
	s[4] = 2;
	s[5] = 3;

	pglEnableClientState(GL_VERTEX_ARRAY);
	pglVertexPointer(2, GL_FLOAT, sizeof(GLfloat) * 4, &v[0]);

	pglEnableClientState(GL_TEXTURE_COORD_ARRAY);
	pglTexCoordPointer(2, GL_FLOAT, sizeof(GLfloat) * 4, &v[2]);

	pglDisableClientState(GL_COLOR_ARRAY);

	pglColor4f(1.0f, 1.0f, 1.0f, 1.0f);
	pglDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, s);

	pglBindTexture(GL_TEXTURE_2D, 0);

#if !defined(NDEBUG)
	check_for_errors();
#endif

	SDL_GL_SwapWindow(window);
}

char *AvpCDPath = 0;

#if !defined(_MSC_VER)
static const struct option getopt_long_options[] = {
{ "help",	0,	NULL,	'h' },
{ "version",	0,	NULL,	'v' },
{ "fullscreen",	0,	NULL,	'f' },
{ "windowed",	0,	NULL,	'w' },
{ "nosound",	0,	NULL,	's' },
{ "nocdrom",	0,	NULL,	'c' },
{ "nojoy",	0,	NULL,	'j' },
{ "debug",	0,	NULL,	'd' },
{ "withgl",	1,	NULL,	'g' },
{ "datapath",	1,	NULL,	'p' },
/*
{ "loadrifs",	1,	NULL,	'l' },
{ "server",	0,	someval,	1 },
{ "client",	1,	someval,	2 },
*/
{ NULL,		0,	NULL,	0 },
};
#endif

static const char *usage_string =
"Aliens vs Predator Linux - http://www.icculus.org/avp/\n"
"Based on Rebellion Developments AvP Gold source\n"
"      [-h | --help]           Display this help message\n"
"      [-v | --version]        Display the game version\n"
"      [-f | --fullscreen]     Run the game fullscreen\n"
"      [-w | --windowed]       Run the game in a window\n"
"      [-s | --nosound]        Do not access the soundcard\n"
"      [-c | --nocdrom]        Do not access the CD-ROM\n"
"      [-j | --nojoy]          Do not access the joystick\n"
"      [-p | --datapath] [x]   Look at [x] for game files\n"
"      [-g | --withgl] [x]     Use [x] instead of /usr/lib/libGL.so.1 for OpenGL\n"
;
         
int main(int argc, char *argv[])
{			
#if !defined(_MSC_VER)
	int c;
	
	opterr = 0;
	while ((c = getopt_long(argc, argv, "hvfwscdg:p:", getopt_long_options, NULL)) != -1) {
		switch(c) {
			case 'h':
				printf("%s", usage_string);
				exit(EXIT_SUCCESS);
			case 'v':
				printf("%s", AvPVersionString);
				exit(EXIT_SUCCESS);
			case 'f':
				WantFullscreen = 1;
				break;
			case 'w':
				WantFullscreen = 0;
				break;
			case 's':
				WantSound = 0;
				break;
			case 'c':
				WantCDRom = 0;
				break;
			case 'j':
				WantJoystick = 0;
				break;			
			case 'd': {
				extern int DebuggingCommandsActive;
				DebuggingCommandsActive = 1;
				}
				break;
			case 'g':
				opengl_library = optarg;
				break;
			case 'p':
				gamedatapath = optarg;
				break;
			default:
				printf("%s", usage_string);
				exit(EXIT_FAILURE);	
		}
	}
#endif
	InitGameDirectories(argv[0], gamedatapath);
	
	if (InitSDL() == -1) {
		fprintf(stderr, "Could not find a sutable resolution!\n");
		fprintf(stderr, "At least 512x384 is needed.  Does OpenGL work?\n");
		exit(EXIT_FAILURE);
	}
		
	LoadCDTrackList();
	
	SetFastRandom();
	
#if MARINE_DEMO
	ffInit("fastfile/mffinfo.txt","fastfile/");
#elif ALIEN_DEMO
	ffInit("alienfastfile/ffinfo.txt","alienfastfile/");
#else
	ffInit("fastfile/ffinfo.txt","fastfile/");
#endif
	InitGame();

	WindowWidth = VideoModeList[CurrentVideoMode].w;
	WindowHeight = VideoModeList[CurrentVideoMode].h;
	SetOGLVideoMode(0, 0);
	SetSoftVideoMode(640, 480, 16);
	
	InitialVideoMode();

	/* Env_List can probably be removed */
	Env_List[0]->main = LevelName;
	
	InitialiseSystem();
	InitialiseRenderer();
	
	LoadKeyConfiguration();
	
	SoundSys_Start();
	if (WantCDRom) CDDA_Start();
	
	InitTextStrings();
	
	BuildMultiplayerLevelNameArray();
	
	ChangeDirectDrawObject();
	AvP.LevelCompleted = 0;
	LoadSounds("PLAYER");

	/* is this still neccessary? */
	AvP.CurrentEnv = AvP.StartingEnv = 0;

#if ALIEN_DEMO
	AvP.PlayerType = I_Alien;
	SetLevelToLoad(AVP_ENVIRONMENT_INVASION_A);
#elif PREDATOR_DEMO
	AvP.PlayerType = I_Predator;
	SetLevelToLoad(AVP_ENVIRONMENT_INVASION_P);
#elif MARINE_DEMO
	AvP.PlayerType = I_Marine;
	SetLevelToLoad(AVP_ENVIRONMENT_INVASION);
#endif

#if !(ALIEN_DEMO|PREDATOR_DEMO|MARINE_DEMO)	
while (AvP_MainMenus())
#else
if (AvP_MainMenus())
#endif
{
	int menusActive = 0;
	int thisLevelHasBeenCompleted = 0;
	
	/* turn off any special effects */
	d3d_light_ctrl.ctrl = LCCM_NORMAL;

	SetOGLVideoMode(0, 0);

	InitialiseGammaSettings(RequestedGammaSetting);
	
	start_of_loaded_shapes = load_precompiled_shapes();
	
	InitCharacter();
	
	LoadRifFile(); /* sets up a map */
	
	AssignAllSBNames();
	
	StartGame();
	
	ffcloseall();
	
	AvP.MainLoopRunning = 1;
	
	ScanImagesForFMVs();
	
	ResetFrameCounter();

	Game_Has_Loaded();
	
	ResetFrameCounter();
	
	if(AvP.Network!=I_No_Network)
	{
		/*Need to choose a starting position for the player , but first we must look
		through the network messages to find out which generator spots are currently clear*/
		netGameData.myGameState = NGS_Playing;
		MinimalNetCollectMessages();
		TeleportNetPlayerToAStartingPosition(Player->ObStrategyBlock,1);
	}

	IngameKeyboardInput_ClearBuffer();
	
	while(AvP.MainLoopRunning) {
		CheckForWindowsMessages();
		
		switch(AvP.GameMode) {
		case I_GM_Playing:
			if ((!menusActive || (AvP.Network!=I_No_Network && !netGameData.skirmishMode)) && !AvP.LevelCompleted) {
				/* TODO: print some debugging stuff */
				
				DoAllShapeAnimations();
				
				UpdateGame();
				
				AvpShowViews();
				
				MaintainHUD();
				
				CheckCDAndChooseTrackIfNeeded();
				
				if(InGameMenusAreRunning() && ( (AvP.Network!=I_No_Network && netGameData.skirmishMode) || (AvP.Network==I_No_Network)) ) {
					SoundSys_StopAll();
				}
			} else {
				ReadUserInput();
				
				SoundSys_Management();
				
				FlushD3DZBuffer();
				
				ThisFramesRenderingHasBegun();
			}

			menusActive = AvP_InGameMenus();
			if (AvP.RestartLevel) menusActive=0;
			
			if (AvP.LevelCompleted) {
				SoundSys_FadeOutFast();
				DoCompletedLevelStatisticsScreen();
				thisLevelHasBeenCompleted = 1;
			}

			ThisFramesRenderingHasFinished();

			InGameFlipBuffers();
			
			FrameCounterHandler();
			{
				PLAYER_STATUS *playerStatusPtr = (PLAYER_STATUS *) (Player->ObStrategyBlock->SBdataptr);
				
				if (!menusActive && playerStatusPtr->IsAlive && !AvP.LevelCompleted) {
					DealWithElapsedTime();
				}
			}
			break;
			
		case I_GM_Menus:
			AvP.GameMode = I_GM_Playing;
			break;
		default:
			fprintf(stderr, "AvP.MainLoopRunning: gamemode = %d\n", AvP.GameMode);
			exit(EXIT_FAILURE);
		}
		
		if (AvP.RestartLevel) {
			AvP.RestartLevel = 0;
			AvP.LevelCompleted = 0;

			FixCheatModesInUserProfile(UserProfilePtr);

			RestartLevel();
		}
	}
	
	AvP.LevelCompleted = thisLevelHasBeenCompleted;

	FixCheatModesInUserProfile(UserProfilePtr);

	ReleaseAllFMVTextures();

	CONSBIND_WriteKeyBindingsToConfigFile();
	
	DeInitialisePlayer();
	
	DeallocatePlayersMirrorImage();
	
	KillHUD();
	
	Destroy_CurrentEnvironment();
	
	DeallocateAllImages();
	
	EndNPCs();
	
	ExitGame();
	
	SoundSys_StopAll();
	
	SoundSys_ResetFadeLevel();
	
	CDDA_Stop();
	
	if (AvP.Network != I_No_Network) {
		EndAVPNetGame();
	}
	
	ClearMemoryPool();

/* go back to menu mode */
#if !(ALIEN_DEMO|PREDATOR_DEMO|MARINE_DEMO)
	SetSoftVideoMode(640, 480, 16);
#endif	
}

	SoundSys_StopAll();
	SoundSys_RemoveAll();
	
	ExitSystem();
	
	CDDA_End();
	ClearMemoryPool();
	
	return 0;
}
