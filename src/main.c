#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "SDL.h"
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

SDL_Surface *surface;

SDL_Joystick *joy;
JOYINFOEX JoystickData;
JOYCAPS JoystickCaps;

/* defaults */
static int WantFullscreen = 0;
int WantSound = 1;
static int WantCDRom = 0;
static int WantJoystick = 0;

/* originally was "/usr/lib/libGL.so.1:/usr/lib/tls/libGL.so.1:/usr/X11R6/lib/libGL.so" */
static const char * opengl_library = NULL;

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

	SDL_JoystickUpdate();
	
	axes = SDL_JoystickNumAxes(joy);
	balls = SDL_JoystickNumBalls(joy);
	hats = SDL_JoystickNumHats(joy);
	
	if (axes > 0) {
		JoystickData.dwXpos = SDL_JoystickGetAxis(joy, 0) + 32768;
	}
	if (axes > 1) {
		JoystickData.dwYpos = SDL_JoystickGetAxis(joy, 1) + 32768;
	}
	
	if (hats > 0) {
		hat = SDL_JoystickGetHat(joy, 0);
		
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
//	Uint16 redtable[256], greentable[256], bluetable[256];
	
	if (surface == NULL) {
		return NULL;
	}
	
	buf = (unsigned char *)malloc(surface->w * surface->h * 3);
	
	if (surface->flags & SDL_OPENGL) {
		pglPixelStorei(GL_PACK_ALIGNMENT, 1);
		pglPixelStorei(GL_UNPACK_ALIGNMENT, 1);
		pglReadPixels(0, 0, surface->w, surface->h, GL_RGB, GL_UNSIGNED_BYTE, buf);
	} else {
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
		
		if (SDL_MUSTLOCK(surface)) {
			SDL_UnlockSurface(surface);
		}
	}
	
	*width = surface->w;
	*height = surface->h;

#if 0	
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
{   720,    480,    0   },
{	800,	600,	0	},
{	1024,	768,	0	},
{	1152,	864,	0	},
{   1280,   720,    0   },
{   1280,   768,    0   },
{	1280,	960,	0	},
{	1280,	1024,	0	},
{	1366,	768,	0	},
{	1600,	1200,	0	},
{   1920,   1080,   0   },
{   1920,   1200,   0   }
};

int CurrentVideoMode;
const int TotalVideoModes = sizeof(VideoModeList) / sizeof(VideoModeList[0]);

void LoadDeviceAndVideoModePreferences()
{
	FILE *fp;
	int mode;
	
	fp = OpenGameFile("AvP_TempVideo.cfg", FILEMODE_READONLY, FILETYPE_CONFIG);
	
	if (fp != NULL) {
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
	return "SDL";
}

char *GetVideoModeDescription3()
{
	static char buf[64];
	
	_snprintf(buf, 64, "%dx%d", VideoModeList[CurrentVideoMode].w, VideoModeList[CurrentVideoMode].h);

	return buf;
}

int InitSDL()
{
	SDL_Rect **SDL_AvailableVideoModes;

	if (SDL_Init(SDL_INIT_VIDEO) < 0) {
		fprintf(stderr, "SDL Init failed: %s\n", SDL_GetError());
		exit(EXIT_FAILURE);
	}

	atexit(SDL_Quit);
	
	// needs to be cleaned up; SDL_VideoModeOK and SDL_ListModes aren't compatible
	SDL_AvailableVideoModes = (SDL_Rect **)-1; //SDL_ListModes(NULL, SDL_FULLSCREEN | SDL_OPENGL);
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
	
	LoadDeviceAndVideoModePreferences();

	if (WantJoystick) {
		SDL_InitSubSystem(SDL_INIT_JOYSTICK);
		
		if (SDL_NumJoysticks() > 0) {
			/* TODO: make joystick number a configuration parameter */
			
			joy = SDL_JoystickOpen(0);
			if (joy) {
				GotJoystick = 1;
			}
			
			JoystickCaps.wCaps = 0; /* no rudder... ? */
			
			JoystickData.dwXpos = 0;
			JoystickData.dwYpos = 0;
			JoystickData.dwRpos = 0;
			JoystickData.dwUpos = 0;
			JoystickData.dwVpos = 0;
			JoystickData.dwPOV = (DWORD) -1;
		}
	}
	
	surface = NULL;
	
	return 0;
}

/* ** */
static void load_opengl_library(const char *lib)
{
	char tmppath[PATH_MAX];
	size_t len, copylen;
	
	if (lib == NULL) {
		if (SDL_GL_LoadLibrary(NULL) == 0) {
			/* success */
			return;
		}
		
		fprintf( stderr, "ERROR: no opengl libraries given\n" );
		exit( EXIT_FAILURE );
	}
	
	while (lib != NULL && *lib) {
		len = strcspn(lib, ":");
				
		copylen = min(len, PATH_MAX-1);
				
		strncpy(tmppath, lib, copylen);
		tmppath[copylen] = 0;
		
		if (SDL_GL_LoadLibrary(tmppath) == 0) {
			/* success */
			return;
		}
		
		lib += len;
		lib += strspn(lib, ":");
	}
	
	fprintf(stderr, "ERROR: unable to initialize opengl library: %s\n", SDL_GetError());
	exit(EXIT_FAILURE);
}

int SetSoftVideoMode(int Width, int Height, int Depth)
{
	SDL_GrabMode isgrab;
	int flags;
	
	load_ogl_functions(0);
	
	/* 
	  let sdl try loading the opengl library, to see if it is even available 
	  this is definitely not enough, but it's a start...
	*/
	if (!surface || !(surface->flags & SDL_OPENGL)) {
		load_opengl_library(opengl_library);
	}
	
	ScanDrawMode = ScanDrawD3DHardwareRGB;
	GotMouse = 1;
	
	flags = SDL_SWSURFACE|SDL_DOUBLEBUF;
	if (surface != NULL) {
	 	if (surface->flags & SDL_FULLSCREEN)
	 		flags |= SDL_FULLSCREEN;
		isgrab = SDL_WM_GrabInput(SDL_GRAB_QUERY);

		SDL_FreeSurface(surface);
	} else {
		if (WantFullscreen)
			flags |= SDL_FULLSCREEN;
		isgrab = SDL_GRAB_OFF;
	}
	
	// reset input
	IngameKeyboardInput_ClearBuffer();
	
	// force restart the video system
	SDL_QuitSubSystem(SDL_INIT_VIDEO);
	SDL_InitSubSystem(SDL_INIT_VIDEO);
	
	if ((surface = SDL_SetVideoMode(Width, Height, Depth, flags)) == NULL) {
		fprintf(stderr, "(Software) SDL SetVideoMode failed: %s\n", SDL_GetError());
		exit(EXIT_FAILURE);
	}
	
	SDL_WM_SetCaption("Aliens vs Predator", "Aliens vs Predator");

	/* this is for supporting keyboard input processing with little hassle */
	SDL_EnableKeyRepeat(SDL_DEFAULT_REPEAT_DELAY, SDL_DEFAULT_REPEAT_INTERVAL);
	SDL_EnableUNICODE(1); /* toggle it to ON */
      
	if (isgrab == SDL_GRAB_ON)
		SDL_WM_GrabInput(SDL_GRAB_ON);

	if ((surface->flags & SDL_FULLSCREEN) || 
	    (SDL_WM_GrabInput(SDL_GRAB_QUERY) == SDL_GRAB_ON))
		SDL_ShowCursor(0);

	ScreenDescriptorBlock.SDB_Width     = Width;
	ScreenDescriptorBlock.SDB_Height    = Height;
	ScreenDescriptorBlock.SDB_CentreX   = Width/2;
	ScreenDescriptorBlock.SDB_CentreY   = Height/2;
	ScreenDescriptorBlock.SDB_ProjX     = Width/2;
	ScreenDescriptorBlock.SDB_ProjY     = Height/2;
	ScreenDescriptorBlock.SDB_ClipLeft  = 0;
	ScreenDescriptorBlock.SDB_ClipRight = Width;
	ScreenDescriptorBlock.SDB_ClipUp    = 0;
	ScreenDescriptorBlock.SDB_ClipDown  = Height;
	
	return 0;	
}

int SetOGLVideoMode(int Width, int Height)
{
	SDL_GrabMode isgrab;
	int flags;
	
	ScanDrawMode = ScanDrawD3DHardwareRGB;
	GotMouse = 1;
	
	load_ogl_functions(0);
	
	flags = SDL_OPENGL;
	if (surface != NULL) {
		if (surface->flags & SDL_FULLSCREEN)
			flags |= SDL_FULLSCREEN;
		isgrab = SDL_WM_GrabInput(SDL_GRAB_QUERY);

		SDL_FreeSurface(surface);
	} else {
		if (WantFullscreen) {
			flags |= SDL_FULLSCREEN;
		}
		
		isgrab = SDL_GRAB_OFF;
	}

	// reset input
	IngameKeyboardInput_ClearBuffer();
	
	// force restart the video system
	SDL_QuitSubSystem(SDL_INIT_VIDEO);
	SDL_InitSubSystem(SDL_INIT_VIDEO);

	load_opengl_library(opengl_library);
			
	SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 5);
	SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 5);
	SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 5);
	SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 16);
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

	// These should be configurable video options.
	//SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 1);
	//SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 4);
	SDL_GL_SetAttribute(SDL_GL_SWAP_CONTROL, 1);

	if ((surface = SDL_SetVideoMode(Width, Height, 0, flags)) == NULL) {
		fprintf(stderr, "(OpenGL) SDL SetVideoMode failed: %s\n", SDL_GetError());
		exit(EXIT_FAILURE);
	}
	
	load_ogl_functions(1);
	
	SDL_WM_SetCaption("Aliens vs Predator", "Aliens vs Predator");

	/* this is for supporting keyboard input processing with little hassle */
	SDL_EnableKeyRepeat(SDL_DEFAULT_REPEAT_DELAY, SDL_DEFAULT_REPEAT_INTERVAL);
	SDL_EnableUNICODE(1); /* toggle it to ON */
      
	if (isgrab == SDL_GRAB_ON)
		SDL_WM_GrabInput(SDL_GRAB_ON);

	if ((surface->flags & SDL_FULLSCREEN) || 
	    (SDL_WM_GrabInput(SDL_GRAB_QUERY) == SDL_GRAB_ON))
		SDL_ShowCursor(0);	
	
	pglViewport(0, 0, Width, Height);

	pglEnable(GL_BLEND);
	pglBlendFunc(GL_SRC_ALPHA, GL_ONE);
	
	pglEnable(GL_DEPTH_TEST);
	pglDepthFunc(GL_LEQUAL);
	pglDepthMask(GL_TRUE);
	pglDepthRange(0.0, 1.0);
	
	pglEnable(GL_TEXTURE_2D);

	pglDisable(GL_CULL_FACE);
	
	pglClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	
	ScreenDescriptorBlock.SDB_Width     = Width;
	ScreenDescriptorBlock.SDB_Height    = Height;
	ScreenDescriptorBlock.SDB_CentreX   = Width/2;
	ScreenDescriptorBlock.SDB_CentreY   = Height/2;
	ScreenDescriptorBlock.SDB_ProjX     = Width/2;
	ScreenDescriptorBlock.SDB_ProjY     = Height/2;
	ScreenDescriptorBlock.SDB_ClipLeft  = 0;
	ScreenDescriptorBlock.SDB_ClipRight = Width;
	ScreenDescriptorBlock.SDB_ClipUp    = 0;
	ScreenDescriptorBlock.SDB_ClipDown  = Height;
	
	load_ogl_functions(1);

	InitOpenGL();
	
	return 0;
}

int InitialiseWindowsSystem(HANDLE hInstance, int nCmdShow, int WinInitMode)
{
	return 0;
}

int ExitWindowsSystem()
{
	if (joy) {
		SDL_JoystickClose(joy);
	}

	load_ogl_functions(0);
	
	if (surface) {
		SDL_FreeSurface(surface);
	}
	
	surface = NULL;

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
		
		case SDLK_a:
			return KEY_A;
		case SDLK_b:
			return KEY_B;
		case SDLK_c:
			return KEY_C;
		case SDLK_d:
			return KEY_D;
		case SDLK_e:
			return KEY_E;
		case SDLK_f:
			return KEY_F;
		case SDLK_g:
			return KEY_G;
		case SDLK_h:
			return KEY_H;
		case SDLK_i:
			return KEY_I;
		case SDLK_j:
			return KEY_J;
		case SDLK_k:
			return KEY_K;
		case SDLK_l:
			return KEY_L;
		case SDLK_m:
			return KEY_M;
		case SDLK_n:
			return KEY_N;
		case SDLK_o:
			return KEY_O;
		case SDLK_p:
			return KEY_P;
		case SDLK_q:
			return KEY_Q;
		case SDLK_r:
			return KEY_R;
		case SDLK_s:
			return KEY_S;
		case SDLK_t:
			return KEY_T;
		case SDLK_u:
			return KEY_U;
		case SDLK_v:
			return KEY_V;
		case SDLK_w:
			return KEY_W;
		case SDLK_x:
			return KEY_X;
		case SDLK_y:
			return KEY_Y;
		case SDLK_z:
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
		case SDLK_NUMLOCK:
			return KEY_NUMLOCK;
		case SDLK_SCROLLOCK:
			return KEY_SCROLLOK;
			
		case SDLK_KP0:
			return KEY_NUMPAD0;
		case SDLK_KP1:
			return KEY_NUMPAD1;
		case SDLK_KP2:
			return KEY_NUMPAD2;
		case SDLK_KP3:
			return KEY_NUMPAD3;
		case SDLK_KP4:
			return KEY_NUMPAD4;
		case SDLK_KP5:
			return KEY_NUMPAD5;
		case SDLK_KP6:
			return KEY_NUMPAD6;
		case SDLK_KP7:
			return KEY_NUMPAD7;
		case SDLK_KP8:
			return KEY_NUMPAD8;
		case SDLK_KP9:
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
		case SDLK_QUOTE:
			return KEY_APOSTROPHE;
		case SDLK_BACKQUOTE:
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
		case SDLK_LSUPER:
			return KEY_LWIN;
		case SDLK_RSUPER:
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

static void handle_keypress(int key, int unicode, int press)
{	
	void RE_ENTRANT_QUEUE_WinProc_AddMessage_WM_CHAR(char Ch);
	void RE_ENTRANT_QUEUE_WinProc_AddMessage_WM_KEYDOWN(int wParam);

	if (key == -1)
		return;

	if (press) {
		switch(key) {
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
				if (unicode && !(unicode & 0xFF80)) {
					RE_ENTRANT_QUEUE_WinProc_AddMessage_WM_CHAR(unicode);
					KeyboardEntryQueue_Add(unicode);
				}
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

static void handle_buttonpress(int button, int press)
{
	int key;

	switch(button) {
		case 4: /* mouse wheel up */
			key = KEY_MOUSEWHEELUP;
			break;
		case 5: /* mouse wheel down */
			key = KEY_MOUSEWHEELDOWN;
			break;
		default: /* other buttons are handled elsewhere */
			return;
	}
	
	/* since this currently only handles wheel up/down */
	if (press == 0)
		return;
		
	if (press && !KeyboardInput[key]) {
		DebouncedKeyboardInput[key] = 1;
	}
	
	GotAnyKey = 1;
	KeyboardInput[key] = press;
}

void CheckForWindowsMessages()
{
	SDL_Event event;
	int x, y, buttons, wantmouse;
	
	GotAnyKey = 0;
	DebouncedGotAnyKey = 0;
	memset(DebouncedKeyboardInput, 0, sizeof(DebouncedKeyboardInput));
	
	wantmouse =	(surface->flags & SDL_FULLSCREEN) ||
			(SDL_WM_GrabInput(SDL_GRAB_QUERY) == SDL_GRAB_ON);

	KeyboardInput[KEY_MOUSEWHEELUP] = 0;
	KeyboardInput[KEY_MOUSEWHEELDOWN] = 0;
	
	if (SDL_PollEvent(&event)) {
		do {
			switch(event.type) {
				case SDL_MOUSEBUTTONDOWN:
					if (wantmouse)
						handle_buttonpress(event.button.button, 1);
					break;
				case SDL_MOUSEBUTTONUP:
					break;
				case SDL_KEYDOWN:
					if (event.key.keysym.sym == SDLK_PRINT) {
						if (HavePrintScn == 0)
							GotPrintScn = 1;
						HavePrintScn = 1;
					} else {
						handle_keypress(KeySymToKey(event.key.keysym.sym), event.key.keysym.unicode, 1);
					}
					break;
				case SDL_KEYUP:
					if (event.key.keysym.sym == SDLK_PRINT) {
						GotPrintScn = 0;
						HavePrintScn = 0;
					} else {
						handle_keypress(KeySymToKey(event.key.keysym.sym), 0, 0);
					}
					break;
				case SDL_QUIT:
					AvP.MainLoopRunning = 0; /* TODO */
					break;
			}
		} while (SDL_PollEvent(&event));
	}
	
	buttons = SDL_GetRelativeMouseState(&x, &y);
	
	if (wantmouse) {
		if (buttons & SDL_BUTTON(1))
			handle_keypress(KEY_LMOUSE, 0, 1);
		else
			handle_keypress(KEY_LMOUSE, 0, 0);
		if (buttons & SDL_BUTTON(2))
			handle_keypress(KEY_MMOUSE, 0, 1);
		else
			handle_keypress(KEY_MMOUSE, 0, 0);
		if (buttons & SDL_BUTTON(3))
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
		int numbuttons;
		
		SDL_JoystickUpdate();
		
		numbuttons = SDL_JoystickNumButtons(joy);
		if (numbuttons > 16) numbuttons = 16;
		
		for (x = 0; x < numbuttons; x++) {
			if (SDL_JoystickGetButton(joy, x)) {
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

	if ((KeyboardInput[KEY_LEFTALT]||KeyboardInput[KEY_RIGHTALT]) && DebouncedKeyboardInput[KEY_CR]) {
		SDL_GrabMode gm;

		SDL_WM_ToggleFullScreen(surface);
		
		gm = SDL_WM_GrabInput(SDL_GRAB_QUERY);
		if (gm == SDL_GRAB_OFF && !(surface->flags & SDL_FULLSCREEN))
			SDL_ShowCursor(1);
		else
			SDL_ShowCursor(0);
	}

	if (KeyboardInput[KEY_LEFTCTRL] && DebouncedKeyboardInput[KEY_G]) {
		SDL_GrabMode gm;
		
		gm = SDL_WM_GrabInput(SDL_GRAB_QUERY);
		SDL_WM_GrabInput((gm == SDL_GRAB_ON) ? SDL_GRAB_OFF : SDL_GRAB_ON);
		
		gm = SDL_WM_GrabInput(SDL_GRAB_QUERY);
		if (gm == SDL_GRAB_OFF && !(surface->flags & SDL_FULLSCREEN))
			SDL_ShowCursor(1);
		else
			SDL_ShowCursor(0);
	}
	
	if (GotPrintScn) {
		GotPrintScn = 0;
		
		ScreenShot();
	}
}
        
void InGameFlipBuffers()
{
	SDL_GL_SwapBuffers();
}

void FlipBuffers()
{
	SDL_Flip(surface);
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
"      [-g | --withgl] [x]     Use [x] instead of /usr/lib/libGL.so.1 for OpenGL\n"
;
         
int main(int argc, char *argv[])
{			
#if !defined(_MSC_VER)
	int c;
	
	opterr = 0;
	while ((c = getopt_long(argc, argv, "hvfwscdg:", getopt_long_options, NULL)) != -1) {
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
			default:
				printf("%s", usage_string);
				exit(EXIT_FAILURE);	
		}
	}
#endif

	InitGameDirectories(argv[0]);
	
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
	
	SetOGLVideoMode(VideoModeList[CurrentVideoMode].w, VideoModeList[CurrentVideoMode].h);
	
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
