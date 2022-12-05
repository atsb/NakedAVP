#include "3dc.h"

#include <sys/stat.h>
#include <string.h>

#include "inline.h"
#include "module.h"

#include "chnktexi.h"
#include "d3d_hud.h"
#define UseLocalAssert Yes
#include "ourasert.h"
#include "hud_layout.h"

#undef textprint

#define textprintOn Yes

#define DHMtextprint Yes
	/* define to use Dave Malcolm's replacement textprint routines */


/* As specified by Roxby */
#define ClearScreenColour 1000

/*
  Experiment to try and fix mystery driver problems
  Don't set with ForceWindowsPalette on!!!
  Leave this on!!!
  On some video cards it seems necessary not only to
  set palette on vblanking interval, but also AFTER
  D3D initialisation is complete...
*/

#define ChangePaletteOnVBAlways Yes

/*
	Turn on or off checking for the validity
	of a video mode before switching to it.
	Actually causes problems on some systems
	because the DirectDraw enumerator fails to
	report valid modes (although on other systems
	it can report ones that can't be reached, ho
	hum), so at least for Chris H. it needs to be
	turned off.
	NOTE THAT THIS SHOULD HAVE THE SAME SETTING AS
	CheckVideoModes at the start of dd_func.cpp, at
	least until we make it a system.h value or
	something else sensible...
*/

#define CheckVideoModes No


/*

 externs for commonly used global variables and arrays

*/

extern SHAPEHEADER **mainshapelist;
extern SHAPEHEADER *testpaletteshapelist[];
extern SCREENDESCRIPTORBLOCK ScreenDescriptorBlock;
extern int *Global_ShapeNormals;
extern int *Global_ShapePoints;
extern int *ItemPointers[];
extern int ItemData[];
extern char projectsubdirectory[];

extern int WinLeftX;
extern int WinRightX;
extern int WinTopY;
extern int WinBotY;

extern int WindowRequestMode;
extern int VideoRequestMode;
extern int ZBufferRequestMode;
extern int RasterisationRequestMode;
extern int SoftwareScanDrawRequestMode;
extern int DXMemoryRequestMode;

extern int TotalVideoMemory;
extern int NumAvailableVideoModes;
extern VIDEOMODEINFO AvailableVideoModes[];

extern int memoryInitialisationFailure;

extern IMAGEHEADER ImageHeaderArray[]; /* Array of Image Headers */

/*

 Global Variables for PC Watcom Functions
 and Windows 95!

*/

/* Timer */
   long lastTickCount;

	unsigned char *ScreenBuffer    = 0;		/* Ensure initialised to Null */

	unsigned char LPTestPalette[1024]; /* to cast to lp*/
	
	int InputMode;

	int VideoMode;
	int VideoModeType;
	int VideoModeTypeScreen;
	int WindowMode;
	int ScanDrawMode;
	int ZBufferMode;
	int DXMemoryMode;
    unsigned char AttemptVideoModeRestart;
	VIDEORESTARTMODES VideoRestartMode;

    PROCESSORTYPES ProcessorType;
	BOOL MMXAvailable;

	unsigned char *TextureLightingTable = 0;

	unsigned char *PaletteRemapTable = 0;

	int NumShadingTables    = 0;

	int NumPaletteShadingTables              = 0;

	int FrameRate;
	int NormalFrameTime;
	int PrevNormalFrameTime;
	extern int CloakingPhase;

	/* These two are dummy values to get the DOS platform to compile */

	unsigned char KeyCode;
	unsigned char KeyASCII;


	#if SuppressWarnings
	unsigned char *palette_tmp;
	static VIEWDESCRIPTORBLOCK* vdb_tmp;
	static SCREENDESCRIPTORBLOCK* sdb_tmp;
	#endif

    /* Keyboard */
	unsigned char KeyboardInput[MAX_NUMBER_OF_INPUT_KEYS];
    unsigned char GotAnyKey;

    /* Input communication with Windows Procedure */
    /* Print system */

#if !DHMtextprint
    PRINTQUEUEITEM PrintQueue[MaxMessages];
	int MessagesStoredThisFrame;
#endif

	int textprintPosX;
	int textprintPosY;
	IMAGEHEADER* fontHeader;

	/* Added 28/11/97 by DHM: boolean for run-time switching on/off of textprint */
	int bEnableTextprint = No;

	/* Added 28/1/98 by DHM: as above, but applies specifically to textprintXY */
	int bEnableTextprintXY = Yes;

	/* Palette */

	unsigned char PaletteBuffer[768 + 1];

/* Test Palette */

unsigned char TestPalette[768];
unsigned char TestPalette2[768];




/* KJL 11:48:45 28/01/98 - used to scale NormalFrameTime, so the game can be slowed down */
int TimeScale=65536;

/* KJL 16:00:11 28/01/98 - unscaled frame time */
int RealFrameTime;
int GlobalFrameCounter;
int RouteFinder_CallsThisFrame;

/* KJL 15:08:43 29/03/98 - added to give extra flexibility to debugging text */
int PrintDebuggingText(const char* t, ...);
int ReleasePrintDebuggingText(const char* t, ...);


/*

 IO and Other Functions for the PC

*/


/*

 Get Shape Data

 Function returns a pointer to the Shape Header Block

*/

SHAPEHEADER* GetShapeData(int shapenum)

{

	if(shapenum>=0 && shapenum< maxshapes)
	{
		SHAPEHEADER *sptr = mainshapelist[shapenum];
		return sptr;
	}
	
	return NULL;
}


/*

 Platform specific VDB functions for Initialisation and ShowView()

*/

void PlatformSpecificVDBInit(VIEWDESCRIPTORBLOCK *vdb)

{
	#if SuppressWarnings
	vdb_tmp = vdb;
	#endif
}


void PlatformSpecificShowViewEntry(VIEWDESCRIPTORBLOCK *vdb, SCREENDESCRIPTORBLOCK *sdb)

{
	#if SuppressWarnings
	vdb_tmp = vdb;
	sdb_tmp = sdb;
	#endif
}


void PlatformSpecificShowViewExit(VIEWDESCRIPTORBLOCK *vdb, SCREENDESCRIPTORBLOCK *sdb)

{
	#if SuppressWarnings
	vdb_tmp = vdb;
	sdb_tmp = sdb;
	#endif
}


/*

 Convert UNIX to MS-DOS

*/

void GetDOSFilename(char *fnameptr)

{

	while(*fnameptr) {

		if(*fnameptr == 0x2f) *fnameptr = 0x5c;
		fnameptr++;

	}

}

/*

 Compare two filenames.

 The first filename is assumed to be raw i.e. has no project subdirectory appended.
 The second is assumed to be ready for use.

 Make a copy of both strings, prefix the copy of the first with the project subdirectory
 and convert them to DOS format before the comparison.

*/

int CompareFilenameCH(char *string1, char *string2)

{

	char *srtmp1;
	char *srtmp2;
	int slen1 = 0;
	int slen2 = 0;
	int i;
	char fname1[ImageNameSize];
	char fname2[ImageNameSize];


	#if 0
	textprint(" Compare "); textprint(string1); textprint("\n");
	textprint(" with    "); textprint(string2); textprint("\n");
	/*WaitForReturn();*/
	#endif


	/* Make a copy of string 1, adding the project subdirectory */

	srtmp1 = projectsubdirectory;
	srtmp2 = fname1;
	while(*srtmp1) *srtmp2++ = *srtmp1++;
	srtmp1 = string1;
	while(*srtmp1) *srtmp2++ = *srtmp1++;
	*srtmp2 = 0;

	/* Make a copy of string 2 */

	srtmp1 = string2;
	srtmp2 = fname2;
	while(*srtmp1) *srtmp2++ = *srtmp1++;
	*srtmp2 = 0;

	/* How long are they? */

	srtmp1 = fname1;
	while(*srtmp1++ != 0)
		slen1++;

	srtmp2 = fname2;
	while(*srtmp2++ != 0)
		slen2++;

	fname1[slen1] = 0;	/* Term */
	fname2[slen2] = 0;

	#if 0
	textprint("slen1 = %d, ", slen1);
	textprint("slen2 = %d\n", slen2);
	#endif

	#if 0
	textprint(" Compare "); textprint(fname1); textprint("\n");
	textprint(" with    "); textprint(fname2); textprint("\n");
	/*WaitForReturn();*/
	#endif


	GetDOSFilename(fname1);
	GetDOSFilename(fname2);


	if(slen1 != slen2) {
		/*textprint("not same\n");*/
		return No;
	}

	srtmp1 = fname1;
	srtmp2 = fname2;

	#if 0
	textprint(" Compare "); textprint(srtmp1); textprint("\n");
	textprint(" with    "); textprint(srtmp2); textprint("\n");
	WaitForReturn();
	#endif

	for(i = slen1; i!=0; i--) {
		if(*srtmp1++ != *srtmp2++) {
			/*textprint("not same\n");*/
			return No;
		}
	}

	/*textprint("same\n");*/
	return Yes;

}







/*

 Create an RGB table for "palette"

 "GetRemappedPaletteColour()" is an access function for this table

*/

int NearestColour(int rs, int gs, int bs, unsigned char *palette)

{

	int i;
	VECTORCH p0;
	VECTORCH p1;
	int nearest_index;
	int nearest_delta;
	int d;


	p0.vx = rs;
	p0.vy = gs;
	p0.vz = bs;

	nearest_index = 0;
	nearest_delta = bigint;

	for(i = 0; i < 256; i++) {

		p1.vx = palette[0];
		p1.vy = palette[1];
		p1.vz = palette[2];

		d = FandVD_Distance_3d(&p0, &p1);

		if(d < nearest_delta) {

			nearest_delta = d;
			nearest_index = i;

		}

		palette += 3;

	}

	return nearest_index;

}


/*************************************************************************/
/*************************************************************************/


/*

 Initialise System and System Variables

*/

void InitialiseSystem()
{
	BOOL 		rc;
	HINSTANCE hInstance = 0;
	int nCmdShow = 1;
	
    /*
		Pick up processor type
	*/

    ProcessorType = ReadProcessorType();

    if ((ProcessorType == PType_PentiumMMX) ||
	   (ProcessorType == PType_Klamath) ||
	   (ProcessorType == PType_OffTopOfScale))
	  MMXAvailable = TRUE;
	else
	  MMXAvailable = FALSE;

    /*
		Copy initial requests to current variables,
		subject to later modification.
	*/

    VideoMode = VideoRequestMode;
	WindowMode = WindowRequestMode;

    /*
		Initialise dubious restart
		system for ModeX emulation
		and other problems
	*/

    AttemptVideoModeRestart = No;

    VideoRestartMode = NoRestartRequired;

    /*
		Potentially a whole suite of caps
		functions could be sensibly called
		from here, to determine available
		sound hardware, network links, 3D
		hardware acceleration etc
	*/

    /* 
	  Initialise the basic Direct Draw object,
	  find a hardware 3D capable driver if 
	  possible and appropriate, and 
      determine what display modes, available
	  video memory etc exist.
	*/

#if 0 /* LINUX */
	if (InitialiseDirectDrawObject()
	    == FALSE)
	   /* 
	     If we cannot get a video mode, 
	     fail.  No point in a non debugging option
	     for this.
	   */
	   {
	    ReleaseDirect3D();
	    exit(0x997799);
	   }

    /*
		Initialise global to say whether
		we think there is an onboard 3D 
		acceleration card / motherboard 
		built-in
	*/

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

    /* Initialise main window, windows procedure etc */
	rc = InitialiseWindowsSystem(hInstance, nCmdShow, WinInitFull);

    /* Initialise input interface */
    memset((void*)KeyboardInput, No, MAX_NUMBER_OF_INPUT_KEYS);
	GotAnyKey = No;

#if 0 /* LINUX */
	/* launch Direct Input */
	InitialiseDirectInput();
	InitialiseDirectKeyboard();
	InitialiseDirectMouse();
	InitJoysticks();
#endif

    /* Initialise textprint system */
    textprintPosX = 0;
	textprintPosY = 0;
	#if debug
	InitPrintQueue();
	#endif

	#if SUPPORT_MMX
	SelectMMXOptions();
	#endif

	{
		/* CDF 4/2/97 */
		extern void ConstructOneOverSinTable(void);

		ConstructOneOverSinTable();
	}

}


/*

 Exit the system

*/

void ExitSystem(void)
{
	/* Game specific exit functions */
	ExitGame();


	// Added by Mark so that Direct Sound exits cleanly
	#if SOUND_ON
	ExitSoundSystem();	// In ds_func.cpp
	#endif

    /* 
      Shaft DirectDraw and hit Direct3D 
      with a blunt Bill.
	  Note that ReleaseDirect3D is currently
	  responsible for whacking DirectDraw
	  and DirectInput as well; I should probably
	  rename it ReleaseDirectX sometime...
    */

	ReleaseDirect3D();

	/* Kill windows procedures */
	ExitWindowsSystem();
}

/*
	Timer functions are based on Windows timer
	giving number of millisecond ticks since Windows
	was last booted.  Note this will wrap round after
   Windows has been up continuously for  approximately
	49.7 days.  This is not considered to be too
	significant a limitation...
*/



void ResetFrameCounter(void)
{
	lastTickCount = timeGetTime();
	
	/* KJL 15:03:33 12/16/96 - I'm setting NormalFrameTime too, rather than checking that it's
	non-zero everytime I have to divide by it, since it usually is zero on the first frame. */
	NormalFrameTime = 65536 >> 4;
	PrevNormalFrameTime = NormalFrameTime;

	RealFrameTime = NormalFrameTime;
	FrameRate = 16;
	GlobalFrameCounter=0;
	CloakingPhase = 0;
	
	
	RouteFinder_CallsThisFrame=0;
}
void FrameCounterHandler(void)
{
	int newTickCount = timeGetTime();
	int fcnt;

	fcnt = newTickCount - lastTickCount;
	lastTickCount = newTickCount;

    if (fcnt == 0)
	  fcnt = 1; /* for safety */

	FrameRate = TimerFrame / fcnt;

	PrevNormalFrameTime = NormalFrameTime;
	NormalFrameTime = DIV_FIXED(fcnt,TimerFrame);

	RealFrameTime = NormalFrameTime;

	{
		if (TimeScale!=ONE_FIXED)
		{
			NormalFrameTime = MUL_FIXED(NormalFrameTime,TimeScale);
		}

	}
	/* cap NormalFrameTime if frame rate is really low */
	if (NormalFrameTime>16384) NormalFrameTime=16384;
	GlobalFrameCounter++;
	CloakingPhase += NormalFrameTime>>5;

	RouteFinder_CallsThisFrame=0;
}

/*

 Wait for Return Key

 Such a function may not be defined on some platforms

 On Windows 95 the description of this function has 
 been changed, so that it calls FlushTextprintBuffer
 and FlipBuffers before going into the actual
 WaitForReturn code. This is necessary if it is to
 behave in the same way after a textprint call as it
 does on the DOS platform.

*/

void WaitForReturn(void)

{
	/* Crude but probably serviceable for now */
	long SavedTickCount;
	SavedTickCount  = lastTickCount;

/* Display any lingering text */
    FlushTextprintBuffer();
	FlipBuffers();

	while (!(KeyboardInput[KEY_CR]))
	   DirectReadKeyboard();

	lastTickCount = SavedTickCount;
}




/*
	By copying the globals here we guarantee
	that game functions will receive a set of
	input values updated at a defined time
*/


void ReadUserInput(void)
{
	DirectReadMouse();
    ReadJoysticks();
	DirectReadKeyboard();
}

/*
	At present all keyboard and mouse input is handled
	through project specific functionality in win_func,
	and all these functions are therefore empty.  Later
	we may port to DirectInput, at which point we 
	may reactivate these.
*/



void ReadKeyboard(void)

{
}


void ReadMouse(void)

{


}



/*
	Not NECESSARILY the standard functionality,
	but it seems good enough to me...
*/

void CursorHome(void)

{
/* Reset positions for textprint system */
	textprintPosX = 0;
	textprintPosY = 0;
}


void GetProjectFilename(char *fname, char *image)
{

	char *src;
	char *dst;


	src = projectsubdirectory;
	dst = fname;

	while(*src)
		*dst++ = *src++;

	src = image;

	while(*src)
		*dst++ = *src++;

	*dst = 0;

}


/*

 Attempts to load the image file.

 Returns a pointer to the image if successful, else zero.

 Image Header is filled out if successful, else ignore it.

 NOTE

 The pointer to the image data is also stored in the image
 header.

*/

TEXTURE* LoadImageCH(char *fname, IMAGEHEADER *iheader)
{
	return 0;
}	


void ConvertToDDPalette(unsigned char* src, unsigned char* dst, int length, int flags)
{
	int i;

/*
	Copy palette, introducing flags and shifting up
	to 8 bit triple
*/

	for (i=0; i<length; i++)
		{
		 *dst++ = (*src++) << 2;
		 *dst++ = (*src++) << 2;
		 *dst++ = (*src++) << 2;
		 *dst++ = flags; 
		}
}	
		
/*

 Platform specific version of "printf()"

 Not all platforms support, or indeed are ABLE to support printf() in its
 general form. For this reasons calls to textprint() are made through this
 function.

*/

/*
	If debug or textprintOn are not defined, these 
	function defintions are collapsed to a simple
	return value, which should collapse to no object
	code under optimisation.  
	The whole issue of turning on or off textprint
	beyond this point is hereby left to Kevin and 
	Chris H to fight to the death about...
*/

#if DHMtextprint


/*
	Dave Malcolm 21/11/96:

	I have rewritten the Win95 textprint routines below.  
	
	It should now support:
		- carriage returns are no longer automatic at the end of lines; there is a #define if you want this behaviour back
		- carriage return characters cause a carriage return
		- wraparound at the right-hand edge of the screen, with textprint() wrapping to the left-hand edge,
		and textprintXY() wrapping back to the X coordinate
		- clipping at the bottom edge of the screen
		- a warning message if text has been lost due to clipping or buffer overflows etc.
		- a y-offset that can be used to scroll up and down the text overlay output from textprint
*/

	/* VERSION SETTINGS: */	
		#define AutomaticNewLines	No
			/* set this to Yes and you will get a \n inserted automatically at the end of each line */
			#if AutomaticNewLines
				#error Not yet written...
			#endif

	/* LOW LEVEL ASSERTION SUPPORT */
		/*
			We cannot use standard assertions in this routine because this routine is called by the standard
			assertion routine, and so would run the risk of infinite loops and excitingly obscure bugs.

			For this reason we define a special assert macro.
		*/

#if 1
	#define LOWLEVELASSERT(ignore)
#else
		#if debug

			#define LOWLEVELASSERT(x) \
			     (void)									\
			     (										\
			     	(x) 								\
			     	? 1 : (ReleaseDirect3D(),exit(GlobalAssertCode),0)	\
			     )										

		#else
			/* Assertions are disabled at compile-time: */
			#define LOWLEVELASSERT(ignore)
		
		#endif
#endif


	/* 
		We extract arguments into a buffer, with a dodgy hack to increase it in size to give more defence
		against buffer overflows; there seems to be no easy & robust way to give vsprintf() a buffer size...

		This buffer is reset once per string per frame
	*/
	#define PARANOIA_BYTES	(1024)
	#define TEXTPRINT_BUFFER_SIZE	(MaxMsgChars+PARANOIA_BYTES+1)
	static char TextprintBuffer[TEXTPRINT_BUFFER_SIZE]="";

	/*

	The PRINTQUEUEITEM structure from PLATFORM.H is not used by my system; instead of queueing strings to be
	displayed we do it on a character by character basis, with a limit on the total number of chars per frame.

	This limit is set to be (MaxMsgChars*MaxMessages), which gives the same power and more flexibility than the 
	old system.

	When the queue is full, additional characters get ignored.

	This is queue is reset once per frame.

	*/

	typedef struct daveprintchar {
		char CharToPrint;
		int x,y;
	} DAVEPRINTCHAR;

	#define DHM_PRINT_QUEUE_SIZE (MaxMsgChars*MaxMessages)

	static DAVEPRINTCHAR DHM_PrintQueue[DHM_PRINT_QUEUE_SIZE];
	static int DHM_NumCharsInQueue=0;

	static int fTextLost=No;
	static char TextLostMessage[]="textprint warning:TEXT LOST";
	#define TEXT_LOST_X	(50)
	#define TEXT_LOST_Y	(20)

	volatile int textprint_Y_offset=0;


/* Dave's version of initialising the print queue */
void InitPrintQueue(void)
{
	DHM_NumCharsInQueue=0;
	fTextLost=No;
}

/*

	Old systems comment:
		Write all messages in buffer to screen
		(to be called at end of frame, after surface
		/ execute buffer unlock in DrawItemListContents,
		so that text appears at the front of the back 
		buffer immediately before the flip).

	This is Dave's version of the same:
*/

void FlushTextprintBuffer(void)

{
	/* PRECONDITION: */
	{
		LOWLEVELASSERT(DHM_NumCharsInQueue<DHM_PRINT_QUEUE_SIZE);
	}

	/* CODE: */
	{
		{
			int i;
			DAVEPRINTCHAR* pDPR=&DHM_PrintQueue[0];

			for (i=0; i<DHM_NumCharsInQueue; i++)
			{
			#if 0
				BlitWin95Char
				(
					pDPR->x, 
					pDPR->y,
					pDPR->CharToPrint
				);
			#else 
				D3D_BlitWhiteChar
				(
					pDPR->x, 
					pDPR->y,
					pDPR->CharToPrint
				);
			#endif
				pDPR++;
			}

			if (fTextLost)
			{
				/* Display error message in case test has been lost due to clipping of Y edge, or buffer overflow */
				int i;
				int NumChars=strlen(TextLostMessage);

				for (i=0;i<NumChars;i++)
				{
	   //			   	BlitWin95Char(TEXT_LOST_X+(i*CharWidth),TEXT_LOST_Y,TextLostMessage[i]);
				}

				fTextLost=No;
			}
		}
		DHM_NumCharsInQueue=0;

	}
}

static int LastDisplayableXForChars(void)
{
	return ScreenDescriptorBlock.SDB_Width-CharWidth;
}

static int LastDisplayableYForChars(void)
{
	return ScreenDescriptorBlock.SDB_Height-CharHeight;
}


static void DHM_AddToQueue(int x,int y, char Ch)
{

	if
	(
		(y>=0)
		&&
		(y<=LastDisplayableYForChars())
	)
	{
		if (DHM_NumCharsInQueue<DHM_PRINT_QUEUE_SIZE)
		{
			DAVEPRINTCHAR* pDPR=&DHM_PrintQueue[DHM_NumCharsInQueue++];
			/* We insert into the queue at this position, updating the length of the queue */

			pDPR->x=x;
			pDPR->y=y;
			pDPR->CharToPrint=Ch;
		}
		else
		{
			/* Otherwise the queue if full, we will have to ignore this char; set an error flag so we get a message*/
			fTextLost=Yes;
		}
	}
	else
	{
		/* Otherwise the text is off the top or bottom of the screen; set an error flag to get a message up*/
		fTextLost=Yes;
	}
}

static int DHM_MoveBufferToQueue(int* pPosX,int* pPosY,int fZeroLeftMargin)
{
	/* 
	Function takes two integers by reference (using pointers), and outputs whatever is in
	the string buffer into the character queue, so that code can be shared by textprint() and textprintXY()

	Returns "number of lines": any carriage returns or word wraps
	*/

	/* PRECONDITION */
	{
		LOWLEVELASSERT(pPosX);
		LOWLEVELASSERT(pPosY);
	}

	/* CODE */
	{
		int NumLines=0;

		int LeftMarginX;

		if (fZeroLeftMargin)
		{
			LeftMarginX=0;
		}
		else
		{
			LeftMarginX=*pPosX;
		}



		/* Iterate through the string in the buffer, adding the individual characters to the queue */
		{
			char* pCh=&TextprintBuffer[0];
			int SafetyCount=0;

			while
			(
				((*pCh)!='\0')
				&&
				((SafetyCount++)<MaxMsgChars)
			)
			{
				switch (*pCh)
				{
					case '\n':
						{
							/* Wrap around to next line.,. */
							(*pPosY)+=HUD_FONT_HEIGHT;
							(*pPosX)=LeftMarginX;
							NumLines++;
						
						}
						break;
					default:
						{
							/* It is a standard character or a space */
							DHM_AddToQueue(*pPosX,(*pPosY)+textprint_Y_offset, *pCh);

							(*pPosX)+=AAFontWidths[(unsigned char)*pCh];//CharWidthInPixels(*pCh);

							if ((*pPosX)>LastDisplayableXForChars())
							{
								/* Wrap around to next line.,. */
								(*pPosY)+=HUD_FONT_HEIGHT;
								(*pPosX)=LeftMarginX;
								NumLines++;
							}
						}
				}

				/* ...and on to the next character*/
				pCh++;
			}
		}
		
		/* Clear the string buffer */
		{
			TextprintBuffer[0]='\0';
		}

		return NumLines;
	}

}


int textprint(const char* t, ...)
{
	#if (debug && textprintOn)
	if
	(
		bEnableTextprint
	)	
	{
		/*
		Get message string from arguments into buffer...
		*/
		{
			va_list ap;
		
			va_start(ap, t);
			vsprintf(&TextprintBuffer[0], t, ap);
			va_end(ap);
		}

		/* 
		Attempt to trap buffer overflows...
		*/
		{
			LOWLEVELASSERT(strlen(TextprintBuffer)<TextprintBuffer);
		}

		return DHM_MoveBufferToQueue(&textprintPosX,&textprintPosY,Yes);

		
	}
	else
	{
		// Run-time disabling of textprint()
		return 0;
	}
	#else
		/* Do nothing; hope the function call gets optimised away */
		return 0;
	#endif
}
int PrintDebuggingText(const char* t, ...)
{
	/*
	Get message string from arguments into buffer...
	*/
	{
		va_list ap;
	
		va_start(ap, t);
		vsprintf(&TextprintBuffer[0], t, ap);
		va_end(ap);
	}

	/* 
	Attempt to trap buffer overflows...
	*/
	{
		LOWLEVELASSERT(strlen(TextprintBuffer)<TextprintBuffer);
	}

	return DHM_MoveBufferToQueue(&textprintPosX,&textprintPosY,Yes);
}
int ReleasePrintDebuggingText(const char* t, ...)
{
	/*
	Get message string from arguments into buffer...
	*/
	{
		va_list ap;
	
		va_start(ap, t);
		vsprintf(&TextprintBuffer[0], t, ap);
		va_end(ap);
	}

	/* 
	Attempt to trap buffer overflows...
	*/
	{
		LOWLEVELASSERT(strlen(TextprintBuffer)<TextprintBuffer);
	}

	return DHM_MoveBufferToQueue(&textprintPosX,&textprintPosY,Yes);
}


int textprintXY(int x, int y, const char* t, ...)

{
	#if (debug && textprintOn)
	if
	(
		bEnableTextprintXY
	)	
	{
		/*
		Get message string from arguments into buffer...
		*/
		{
			va_list ap;
		
			va_start(ap, t);
			vsprintf(&TextprintBuffer[0], t, ap);
			va_end(ap);
		}

		/* 
		Attempt to trap buffer overflows...
		*/
		{
			LOWLEVELASSERT(strlen(TextprintBuffer)<TextprintBuffer);
		}

		{
			int localX=x;
			int localY=y;

			return DHM_MoveBufferToQueue(&localX,&localY,No);
		}

		
	}
	else
	{
		// Run-time disabling of textprint()
		return 0;
	}
	#else
	{
		/* Do nothing; hope the function call gets optimised away */

		return 0;
	}
	#endif
}



	/* 
	 *
	 * 
	
	End of Dave Malcolm's text routines; old version is below 
	
	 *
	 *
	 */
#else

/*
	NOTE!!!! All this software is intended for debugging
	only.  It emulates the print interface in any video
	mode supported by the engine, but there are limits -
	messages will pile up at the bottom of the screen
	and overwrite each other, all messages will appear at the
	fron in the main screen (NOT clipped to the VDB), messages
	will not wrap round if they are longer than a screen line
	unless \n is inserted in the print string, and the text colour
	is not guaranteed to be white in paletted modes.
	So there.
*/


/*
	IMPORTANT!!!!
	Messages longer than MaxMsgChars are liable
	to CRASH this routine.  I haven't bothered 
	to do anything about this on the grounds that
	we can't tell how long the message is until after
	the vsprintf call, and the crash is likely to
	occur in vsprintf itself as it overflows the
	buffer.
*/

/*
	!!!!! FIXME??
	textprints don't seem to appear 
	in SubWindow mode --- possibly
	because the colours in the font
	are going to system font colours which
	are invisible???
*/

#if (debug && textprintOn)

int textprint(const char* t, ...)

{
	int i,j;
	va_list ap;
	char message[MaxMsgChars];
	char outmsg[MaxMsgChars];
	int numlines;
	int CharCount;
	int XPos=0;

	va_start(ap, t);

	vsprintf(&message[0], t, ap);

	va_end(ap);

    i = 0;
	j = 0;
	numlines = 0;
	CharCount = strlen(&message[0]);

    /* Read through message buffer until we reach the terminator */
    while ((i < CharCount) && (message[i] != '\0'))
  	{
    	outmsg[j++] = message[i];
		XPos+=CharWidth;
        /* newline within string */
        if ((message[i] == '\n')||(XPos>ScreenDescriptorBlock.SDB_Width))
	      {
	       /* Display string and reset to start of next line */
	       WriteStringToTextBuffer(textprintPosX, textprintPosY, 
	              &outmsg[0]);
		   textprintPosX = 0;
		   textprintPosY += HUD_FONT_HEIGHT;
		   XPos=0;
		   /* Messages can pile up at bottom of screen */
		   if (textprintPosY > ScreenDescriptorBlock.SDB_Height)
		     textprintPosY = ScreenDescriptorBlock.SDB_Height;
		   /* Clear output string and reset variables */
		   {
			int k;
		    for (k=0; k<(j+1); k++)
			  outmsg[k] = 0;
		   }
		   j = 0;
		   /* Record number of lines output */
		   numlines++;
		  }
		 i++;
       }

	/* Flush any remaining characters */
	WriteStringToTextBuffer(textprintPosX, textprintPosY, 
	     &outmsg[0]);
    textprintPosX = 0;
	textprintPosY += HUD_FONT_HEIGHT;
	/* Messages can pile up at bottom of screen */
	if (textprintPosY > ScreenDescriptorBlock.SDB_Height)
	  textprintPosY = ScreenDescriptorBlock.SDB_Height;
	numlines++;

	return numlines;
}

/*
	Textprint to defined location on screen
	(in screen coordinates for current video
	mode).
	NOTE!!! Newlines within strings sent to this
	function will be IGNORED.
*/

int textprintXY(int x, int y, const char* t, ...)

{
	va_list ap;
	char message[MaxMsgChars];

	va_start(ap, t);

	vsprintf(&message[0], t, ap);

	va_end(ap);

	WriteStringToTextBuffer(x, y, &message[0]);

	return 1; /* for one line */
}

#else

int textprint(const char* t, ...)

{
	return 0;
}

int textprintXY(int x, int y, const char* t, ...)

{
	return 0;
}

#endif


/*
	Add string to text buffer 
*/

void WriteStringToTextBuffer(int x, int y, unsigned char *buffer)

{
	if (MessagesStoredThisFrame < MaxMessages)
	  {
	   strcpy(PrintQueue[MessagesStoredThisFrame].text, buffer);

       PrintQueue[MessagesStoredThisFrame].text_length = strlen(buffer);
	   PrintQueue[MessagesStoredThisFrame].x = x;
	   PrintQueue[MessagesStoredThisFrame].y = y;

	   MessagesStoredThisFrame++;
	  }
}


/*
	Display string of chracters, starting at passed pointer,
	at location on screen starting with x and y.

	Patched by Dave Malcolm 20/11/96 so that text wraps around when it reaches the right hand edge of the screen, in
	this routine, at least...
*/


void DisplayWin95String(int x, int y, unsigned char *buffer)

{
	int InitialX=x;
	int stlen;
	unsigned char ch;

	stlen = strlen(buffer);

    do
	  {
       ch = (unsigned char) *buffer;
	   BlitWin95Char(x, y, ch);
	   x += CharWidth;
	   if (x > (ScreenDescriptorBlock.SDB_Width 
	       - CharWidth))
		{
			#if 1
				/* Wrap to new line, based on coordinates for display...*/
				x=InitialX;
				y+=HUD_FONT_HEIGHT;
			#else
			   	/* Characters will pile up at screen edge */
				x = (ScreenDescriptorBlock.SDB_Width - CharWidth);
			#endif
		}
	   
	   buffer++;
	   stlen--;
	  }
	while ((ch != '\n') && (ch != '\0') &&
	      (stlen > 0));
}

/*
	Write all messages in buffer to screen
	(to be called at end of frame, after surface
	/ execute buffer unlock in DrawItemListContents,
	so that text appears at the front of the back 
	buffer immediately before the flip).
*/

void FlushTextprintBuffer(void)

{
	int i;

    for (i=0; i<MessagesStoredThisFrame; i++)
	  {
	   if (PrintQueue[i].text_length)
	     DisplayWin95String(PrintQueue[i].x, 
	        PrintQueue[i].y, PrintQueue[i].text);

       /* 
         More mystery code from Roxby --- an extra safety
		 check for printing?? Or a hangover from a linked
		 list version of the data structure???
	   */
       PrintQueue[i].text_length = 0;
	  }

    MessagesStoredThisFrame = 0;
}

/* Initialise print queue */

void InitPrintQueue(void)

{
	int i;

    /* Mystery code from Roxby here... */
    for (i=0; i < MaxMessages; i++)
	   PrintQueue[i].text_length = 0;

    MessagesStoredThisFrame = 0;
}

#endif
	/*end of old version of text routines */

/*
	Load main, 8 bit paletted, font 
	(assumed to be on hard drive at present)
	and create hi and true colour mode fonts
	from it. Note that for this system to work 
	properly all bits on must be white or similar 
	in 8 bit mode 222 and Raw256 palettes as well
	as mode 8T.
*/

/*
	MUST be called after GenerateDirectDrawSurface,
	i.e. AFTER SetVideoMode.
	AND ONLY ONCE!!!!
*/



/*
	This function is intended to allow YOU,
	the user, to obtain your heart's fondest desires
	by one simple call.  Money? Love? A better job?
	It's all here, you have only to ask...
	No, I was lying actually.
	In fact, this should allow you to change
	display modes cleanly.  Pass your request modes
	(as normally set up in system.c).  For all entries
	which you do not want to change, simply pass
	the current global value (e.g. ZBufferRequestMode
	in the NewZBufferMode entry).

    Note that the function must always be passed the
	HINSTANCE and nCmdShow from winmain.
*/

/*
	Note that this function will NOT
	reinitialise the DirectDraw object
	or switch to or from a hardware DD
	device, but it will release and rebuild
	all the Direct3D objects.
*/

/*
	Note that you MUST be in the right
	directory for a texture reload before you
	call this, and normal operations CAN change
	the directory...
*/

/*
	NOTE!!! If you start in DirectDraw mode
	and go to Direct3D mode, this function
	CANNOT POSSIBLY WORK WITHOUT A FULL SHAPE 
	RELOAD, since the shape data is overwritten
	during DirectDraw initialisation!!!!

    NOTE ALSO: TEXTURE RELOAD MAY BE DODGY 
	WITHOUT A SHAPE RELOAD!!!
*/

int ChangeDisplayModes(HINSTANCE hInst, int nCmd, 
     int NewVideoMode, int NewWindowMode,
     int NewZBufferMode, int NewRasterisationMode, 
     int NewSoftwareScanDrawMode, int NewDXMemoryMode)
{
    BOOL rc;
	BOOL ChangeWindow = No;

/*
	Shut down DirectX objects and destroy
	the current window, if necessary.
*/

    if (NewWindowMode != WindowMode)
	  ChangeWindow = Yes;

    DeallocateAllImages();

    ReleaseDirect3DNotDD();

    finiObjectsExceptDD();


    if (ChangeWindow)
      ExitWindowsSystem(); 

/* Test!! */

/*
	Set the request modes and actual modes
	according to the passed values.
*/

    VideoRequestMode = NewVideoMode;
    WindowRequestMode = NewWindowMode;
	ZBufferRequestMode = NewZBufferMode;
	RasterisationRequestMode = NewRasterisationMode;
	SoftwareScanDrawRequestMode = NewSoftwareScanDrawMode;
	DXMemoryRequestMode = NewDXMemoryMode;

    VideoMode = VideoRequestMode;
	WindowMode = WindowRequestMode;

	/* this may reconstruct the dd object depending
	   on the rasterisation request mode and whether
	   a hardware dd driver is selected or could be
	   available */
	ChangeDirectDrawObject();

    /*
	  Check that our new video mode exists,
	  and pick a valid option if it doesn't and
	  we can find one.
	*/

    #if CheckVideoModes
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
	#endif

/*
	Recreate the window, allowing
	for possible change in WindowMode.
*/

    if (ChangeWindow)
	  {
	   rc = InitialiseWindowsSystem(hInst, nCmd, 
	      WinInitChange);

       if (rc == FALSE)
	     return rc;
	  }

/*
	Set the video mode again.  This
	will handle all changes to DirectDraw
	objects, all Direct3D initialisation,
	and other request modes such as
	zbuffering.
*/

    /*
		Err... shutting down and restarting
		on a hardware driver appears to
		screw up file handling somehow...
		umm... but not for Microsoft demos,
		obviously...
		FIXME!!!
	*/

/*
    SetVideoMode[VideoMode]();
*/

/*
	Lose all the textures and reload the 
	debugging font
*/

    InitialiseTextures();


/*
	Well, we HOPE it's okay...
*/

    return TRUE;
}


/*
	Reverse of ConvertToDDPalette, introduced
	to maintain internal interfaces only...
*/

void ConvertDDToInternalPalette(unsigned char* src, unsigned char* dst, int length)
{
	int i;

/*
	Copy palette, shifting down
	to 5 bit triple
*/

	for (i=0; i<length; i++)
		{
		 *dst++ = (*src++) >> 2;
		 *dst++ = (*src++) >> 2;
		 *dst++ = (*src++) >> 2;
		}
}	
