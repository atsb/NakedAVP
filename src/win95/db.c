#ifndef NDEBUG

/* ********************************************************************	*
 *																		*
 *	DB.C - Debugging functions.											*
 *																		*
 *	By: Garry Lancaster									Version: 2.0	*
 *																		*
 * ******************************************************************** */

/* N O T E S ********************************************************** */

/* A lot of these functions should be called via macros defined in db.h */

/* If you don't want to link this file with Windows OS files set the 
 * define DB_NOWINDOWS. This will also stop linking with the Direct Draw
 * stuff, which is, after all, a part of Windows. If you want Windows 
 * stuff, but NOT Direct Draw, define DB_NODIRECTDRAW.
 */

#define DB_NOWINDOWS
#define DB_NODIRECTDRAW
 
/* ******************************************************************** *
 * 																		*
 *	I N T E R F A C E - both internal and external.						*
 * 																		*
 * ******************************************************************** */

/* I N C L U D E S **************************************************** */

#include "fixer.h"

/* Windows includes. Actually internal, but here to allow pre-compilation. */
#ifndef DB_NOWINDOWS
	#include <windows.h>
	#include "advwin32.h"
#endif	
#ifndef DB_NODIRECTDRAW
	#include <ddraw.h>
#endif
#include "db.h"	 /* Contains most off the interface. */

/* This variable dictates whether macros ending _opt get executed. */
int db_option = 0; /* Default is off. */

/* ******************************************************************** *
 * 																		*
 *	I N T E R N A L 													*
 * 																		*
 * ******************************************************************** */

/* I N C L U D E S **************************************************** */

/* Defining DB_NOWINDOWS implies DB_NODIRECTDRAW should	also be defined. */
#ifdef DB_NOWINDOWS
	#ifndef DB_NODIRECTDRAW
		#define DB_NODIRECTDRAW
	#endif
#endif
		
/* ANSI includes. */
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <stdarg.h>	/* For variable arguments. */

/* C O N S T A N T S ************************************************** */

/* Possible return value for MessageBox() */
#define NO_MEMORY	0

/* Possible value for the width field of a font_struct. */
#define PROP_WIDTH 0

/* Logfile name */
#define LOGFILE_NAME "logfile.txt"

/* Set this to 1 if the logfile name is an absolute path. Otherwise the
 * logfile will go in the directory that is current when db_log_init() 
 * is called.
 */
#define ABSOLUTE_PATH	1

/* M A C R O S ******************************************************** */

/* Causes a division by zero exception. */
#define DB_FORCE_EXCEPTION()	( db_vol_zero = 1 / db_vol_zero )

/* Cause a brakepoint. */
//#define DB_FORCE_BRAKEPOINT()	do {__asm int 3} while(0)
#define DB_FORCE_BRAKEPOINT() { }

/* T Y P E S ********************************************************** */


typedef struct font_struct *fontPtr;

struct font_struct {
	void *dummy1;
	unsigned short dummy2, dummy3;
	void *dummy4;
	char filename[16];
	unsigned short width, height;
	unsigned short ascent, avgwidth;
	unsigned char byte_width;
	unsigned char filler1;
	short filler2;
	char facename[28];
	unsigned short *prop_width_dataP;
	unsigned char *bitmapP;
};

union PtrPackTag
{
	unsigned char  *cP;
	unsigned short *wP;
	unsigned long  *lP;
};

/* G L O B A L ******************************************************** */
/* Should all be static. */
static BOOL	db_use_brakepoints = FALSE;

/* Name of file to output log messages to. */
static char LogFileNameP[255];

/* Have we initialized the log file? */
static int InitialisedLog = 0;

/* Type of display mode we are in. */
static int db_display_type = DB_DOS;

/* For DirectDraw mode. */
#ifndef DB_NODIRECTDRAW			
static struct db_dd_mode_tag dd_mode = {NULL, NULL, 0, 0, 0, 0, 0, 0, 0, 0};
static fontPtr FontP = NULL;
#endif

/* Volatile zero. */
static volatile int db_vol_zero = 0;

/* Text strings for output. */
static const char* db_assert_textA[ 3 ] =
{
	"ASSERTION FAILED!",
	"Expression: %s",
	"File: %s Line: %d"
};

#ifndef DB_NOWINDOWS
static const char* db_prompt_std =
	"Quit program/force e(x)ception? [y/n/x]";
static const char* db_prompt_windows =
	"Quit program? [Yes/No]/force exception? [Cancel]";
#endif
	
static const char* db_assert_log_begin_text =
	"DB: FAILED ASSERTION BEGINS";
static const char* db_assert_log_end_text =
	"DB: FAILED ASSERTION ENDS";
static const char* db_msg_log_begin_text =
	"DB: MSG BEGINS";
static const char* db_msg_log_end_text =
	"DB: MSG ENDS";
	
#ifdef DB_NOWINDOWS
static char db_log_file_name[261] = LOGFILE_NAME;
#else
static char db_log_file_name[MAX_PATH+1] = LOGFILE_NAME;
#endif


/* P R O T O S ******************************************************** */
/* Should all be static. */

static void db_do_std_prompt(unsigned yOffset);

#ifndef DB_NOWINDOWS
static void db_do_win_prompt(const char* titleStrP, const char* bodyStrP);
#endif /* ifndef DB_NOWINDOWS */

#ifndef DB_NODIRECTDRAW

/* Load debugging font, return NULL if we fail. */
static fontPtr guiload_font(char *new_fname);

/* Cleanup function for the above. */
static fontPtr CleanupFontLoadFail(HANDLE fH, fontPtr fontP);

/* Outputs debugging text. */
static void out_text(LPDIRECTDRAWSURFACE surfP, int xc, int yc, 
	const char *text, short x_limit, fontPtr fP);

/* Debounce all the keys the Direct Draw stuff uses. */	
static void Debounce(void);

/* Wait for any outstanding flip operations to be completed, provided
 * that the user specified DB_FLIP in the bltOrFlip field of the 
 * db_dd_mode_tag function. If they specified DB_BLIT this function returns
 * immediately.
 */
static void DbWaitForHw(void);
	
/* Flips between the draw and visible surfaces. */	
static void DbFlip(void);

/* Blits the contents of the draw surface to the visible surface. */
static void DbBlt(void);
	
#endif /* ifndef DB_NODIRECTDRAW */

/* F U N C T I O N S **************************************************	*/

/* ******************************************************************** *
 * 																		*
 *	I N T E R F A C E - both internal and external.						*
 * 																		*
 * ******************************************************************** */

/* NEW FNS for formatted debug strings. */
void __cdecl db_logf_fired(const char *fmtStrP, ...)
{
	char msg[ 1024 ];
	va_list varArgList;
	
	va_start( varArgList, fmtStrP );
	vsprintf( msg, fmtStrP, varArgList );
	va_end( varArgList );

	db_log_fired( msg );
}

void __cdecl db_printf_fired(int x, int y, const char *fmtStrP, ...)
{
	char msg[ 256 ];
	va_list varArgList;
	
	va_start( varArgList, fmtStrP );
	vsprintf( msg, fmtStrP, varArgList );
	va_end( varArgList );

	db_print_fired( x, y, msg );
}

void __cdecl db_msgf_fired(const char *fmtStrP, ...)
{
	char msg[ 256 ];
	va_list varArgList;
	
	va_start( varArgList, fmtStrP );
	vsprintf( msg, fmtStrP, varArgList );
	va_end( varArgList );

	db_msg_fired( (const char *) msg );
}

/* Called whenever an assertion fails. */
void db_assert_fail(const char *exprP, const char *fileP, int line)
{
	db_log_fired( db_assert_log_begin_text );
	db_log_fired( db_assert_textA[ 0 ] );
	db_logf_fired( db_assert_textA[ 1 ], exprP );
	db_logf_fired( db_assert_textA[ 2 ], fileP, line );
	db_log_fired( db_assert_log_end_text );

	switch(db_display_type)
	{
		case DB_DOS:
			printf( db_assert_textA[ 0 ] );
			printf("\n");
			printf( db_assert_textA[ 1 ], exprP	);
			printf("\n");
			printf( db_assert_textA[ 2 ], fileP, line );
			printf("\n");
			db_do_std_prompt( 0 );
			break;
#ifndef DB_NODIRECTDRAW			
		case DB_DIRECTDRAW:
			{
				char msg[256];
				unsigned short xLimit = (unsigned short) dd_mode.width;
				
				/* Wait for any hardware to finish flipping. */
				DbWaitForHw();
				
				out_text((LPDIRECTDRAWSURFACE) dd_mode.drawSurfaceP,
					0, 0, db_assert_textA[ 0 ], xLimit, FontP);
				wsprintf(msg, db_assert_textA[ 1 ], exprP);
				out_text((LPDIRECTDRAWSURFACE) dd_mode.drawSurfaceP,
					0, 16, msg, xLimit, FontP);
				wsprintf(msg, db_assert_textA[ 2 ], fileP, line);
				out_text((LPDIRECTDRAWSURFACE) dd_mode.drawSurfaceP,
					0, 32, msg, xLimit, FontP);
				db_do_std_prompt( 48 );
			}	
			break;
#endif	
#ifndef DB_NOWINDOWS		
		case DB_WINDOWS:
			{
				char fmtMsg[ 256 ];
				char msg[256];
				
				strcpy( fmtMsg, db_assert_textA[ 1 ] );
				strcat( fmtMsg, db_assert_textA[ 2 ] );
				strcat( fmtMsg, db_prompt_windows );
				sprintf(msg, fmtMsg, exprP, fileP, line);
				
				db_do_win_prompt( db_assert_textA[ 0 ], msg );
			}
			break;
#endif			
		default:
			break;
	}
}

/* Displays a message and has the program pause until the user responds
 * to it.
 */
void db_msg_fired(const char *strP)
{
	db_log_fired( db_msg_log_begin_text );
	db_log_fired( strP );
	db_log_fired( db_msg_log_end_text );

	switch(db_display_type)
	{
		case DB_DOS:
			printf("%s\n", strP);
			db_do_std_prompt( 0 );
			break;
#ifndef DB_NOWINDOWS			
		case DB_WINDOWS:
			db_do_win_prompt( "Debugging Message", strP );
			break;
#endif
#ifndef DB_NODIRECTDRAW			
		case DB_DIRECTDRAW:
			{
				unsigned short xLimit = (unsigned short) dd_mode.width;
				
				/* Wait for any flip hardware to be ready. */
				DbWaitForHw();
				
				out_text((LPDIRECTDRAWSURFACE) dd_mode.drawSurfaceP,
					0, 0, strP, xLimit, FontP);
				db_do_std_prompt( 16 );
			}	
			break;	
#endif
		default:
			break;				
	}
}

/* Displays a message and continues program execution immediately. */
void db_print_fired(int x, int y, const char *strP)
{
	switch(db_display_type)
	{
		case DB_DOS:
			printf("%s\n", strP);
			break;
#ifndef DB_NOWINDOWS			
		case DB_WINDOWS:
			break;
#endif
#ifndef DB_NODIRECTDRAW		
		case DB_DIRECTDRAW:
		{
			unsigned short xLimit = (unsigned short) (dd_mode.width - x);
			out_text((LPDIRECTDRAWSURFACE) dd_mode.drawSurfaceP, x, y, 
				strP, xLimit, FontP);
			break;
		}	
#endif
		default:
			break;			
	}		
}

/* Writes a message to a log file. */
/* At least files can be output in the same way under DOS, Windows and 
 * Direct Draw!
 */
void db_log_fired(const char *strP)
{
	/* Have we intialised the file?	*/
	if(!InitialisedLog) db_log_init();
	{
		/* Open a file for appending, creating one if it doesn't yet exist. */
		FILE *fP = OpenGameFile(LogFileNameP, FILEMODE_APPEND, FILETYPE_CONFIG);

		if(!fP) return;

		fprintf(fP, "%s\n", strP);
		fclose(fP);
	}
}

void db_log_init(void)
{
	#if ABSOLUTE_PATH
	sprintf( LogFileNameP, "%s", db_log_file_name ); 
	#else
	/* Append the log file name to the current working directory. */
	sprintf( LogFileNameP, "%s/%s", getcwd( LogFileNameP, 240 ),
		db_log_file_name );
	#endif
	
	/* Delete old log file. */
	DeleteGameFile(LogFileNameP);
	
	/* Flag that we have initialised the log file. */
	InitialisedLog = 1;
}

extern void db_set_log_file_ex(const char *strP)
{
	InitialisedLog = 0;
	
	strcpy(db_log_file_name, strP);
}

void db_set_mode_ex(int mode, void *modeInfoP, void *newFontP)
{
	db_display_type = mode;	
#ifndef DB_NODIRECTDRAW	
	if(dd_mode.visibleSurfaceP)
	{
		IDirectDrawSurface_Release((LPDIRECTDRAWSURFACE) dd_mode.visibleSurfaceP);
		dd_mode.visibleSurfaceP = NULL;
	}
	
	if(dd_mode.drawSurfaceP)
	{
		IDirectDrawSurface_Release((LPDIRECTDRAWSURFACE) dd_mode.drawSurfaceP);
		dd_mode.drawSurfaceP = NULL;
	}

	if(mode == DB_DIRECTDRAW) 
	{
		dd_mode = *((struct db_dd_mode_tag *) modeInfoP);

		if(dd_mode.visibleSurfaceP)
		{
			IDirectDrawSurface_AddRef((LPDIRECTDRAWSURFACE) dd_mode.visibleSurfaceP);
		}

		if(dd_mode.drawSurfaceP)
		{
			IDirectDrawSurface_AddRef((LPDIRECTDRAWSURFACE) dd_mode.drawSurfaceP);
		}

		if(!FontP)
		{
			if(!newFontP)
				FontP = guiload_font("DIALOG.FNT");	
			else
				FontP = (fontPtr) newFontP;
		}
		if(!FontP) 
		{
			db_log_fired("DB ERROR: Font load failed. Exiting...");
			exit(0);
		}	
	}	
#endif	
}

/* Called to set whether exceptions or brakepoints are called. */
void DbUseBrakepoints(BOOL use_brakepoints)
{
	db_use_brakepoints = use_brakepoints;
}

int db_get_mode(void** modeInfoPP, void **FontPP)
{
	// blank return areas
	*FontPP = NULL;
	*modeInfoPP = NULL;

#ifndef DB_NODIRECTDRAW
	if(db_display_type == DB_DIRECTDRAW)
	{
		// copy font data
		*FontPP = (void *) FontP;

		// copy surface data
		*modeInfoPP = (void *) &dd_mode;
	}
#endif
	return db_display_type;
}

void db_uninit(void)
{
	#ifndef DB_NODIRECTDRAW
	if(FontP)
	{
		if(FontP->bitmapP)
		{
			GlobalFree(FontP->bitmapP);
		}
		if(FontP->prop_width_dataP)
		{
			GlobalFree(FontP->prop_width_dataP);
		}
		
		GlobalFree(FontP);
		FontP = NULL;
	}

	if(dd_mode.visibleSurfaceP)
	{
		IDirectDrawSurface_Release((LPDIRECTDRAWSURFACE) dd_mode.visibleSurfaceP);
	}

	if(dd_mode.drawSurfaceP)
	{
		IDirectDrawSurface_Release((LPDIRECTDRAWSURFACE) dd_mode.drawSurfaceP);
	}

	#endif
}

/* ******************************************************************** *
 * 																		*
 *	I N T E R N A L 													*
 * 																		*
 * ******************************************************************** */

static void db_do_std_prompt(unsigned yOffset)
{
	int ch = 0;

	switch(db_display_type)
	{
		case DB_DOS:
#if 0		
			printf( db_prompt_std );
			printf("\n");
			do
			{
				ch = toupper(getch());
			}
			while((ch != 'N') && (ch != 'Y') && (ch != 'X'));
#endif
			ch = 'N';
						
			break;
#ifndef DB_NODIRECTDRAW
		case DB_DIRECTDRAW:
		{
			SHORT response;
			BOOL done = FALSE;
			unsigned short xLimit = (unsigned short) dd_mode.width;
			
			out_text((LPDIRECTDRAWSURFACE) dd_mode.drawSurfaceP,
				0, yOffset, db_prompt_std, xLimit, FontP);
			
			/* Show the message. */
			if(dd_mode.bltOrFlip == DB_FLIP) 
			{
				DbFlip();
			}
			else 
			{
				DbBlt();
			}
			
			/* Wait for a valid key press. */
			do
			{
				response = GetAsyncKeyState('Y');
				if(response & 0x8000) 
				{
					ch = 'Y';
					done = TRUE;
				}	
				response = GetAsyncKeyState('N');
				if(response & 0x8000) 
				{
					ch = 'N';
					done = TRUE;
				}	
				response = GetAsyncKeyState('X');
				if(response & 0x8000)
				{
					ch = 'X';
					done = TRUE;
				}
			}
			while(!done);
			
			Debounce();		
			
			/* Return the flip surfaces to their pre-message state. */
			if(dd_mode.bltOrFlip == DB_FLIP) 
			{
				DbFlip();
			}			
			break;
		}
#endif /* ifndef DB_NODIRECTDRAW */
	}/* switch(db_display_type) */

	if(ch == 'Y')
	{
		exit(-10);
	}
	else if (ch == 'X')
	{
		if(db_use_brakepoints)
		{
			DB_FORCE_BRAKEPOINT();
		}
		else
		{
			DB_FORCE_EXCEPTION();
		}
	}

}/* db_do_std_prompt() */

#ifndef DB_NOWINDOWS
static void db_do_win_prompt(const char* titleStrP, const char* bodyStrP)
{
	int response;

	response = MessageBox
	(
		NULL, 				/* Dialog has no KNOWN parent window. 			*/
		bodyStrP,  			/* Message to go in box. 						*/
		titleStrP,			/* Box title. 									*/
		MB_YESNOCANCEL| 	/* Put up a 'Yes' and a 'No' button. 			*/
		MB_SETFOREGROUND| 	/* Shove message box to front of display. 		*/
		MB_ICONEXCLAMATION| /* Use an exclamation mark to decorate box. 	*/
		MB_TASKMODAL 		/* Suspend current task until box is closed.	*/
	);
	
	if((response == IDYES) || (response == NO_MEMORY)) 
	{
		exit( -10 );
	}
	else if(response == IDCANCEL) 
	{
		if(db_use_brakepoints)
		{
			DB_FORCE_BRAKEPOINT();
		}
		else
		{
			DB_FORCE_EXCEPTION();
		}
	}
}/* db_do_win_prompt() */
#endif /* ifndef DB_NOWINDOWS */

#ifndef DB_NODIRECTDRAW

static fontPtr guiload_font(char *new_fname)
{
 	HANDLE *fH;
	BOOL status;
	unsigned short c, byte, y;
	union PtrPackTag dest; 
	unsigned char inByte;
	fontPtr fntSP;
	DWORD bytesRead;

	/* Open file for reading */
	fH = CreateFile(new_fname, GENERIC_READ, 0, NULL, OPEN_EXISTING,
		FILE_ATTRIBUTE_NORMAL|FILE_FLAG_SEQUENTIAL_SCAN, NULL);
	if(!fH) 
	{
		db_log_fired("DB ERROR: Couldn't open font file.");
		return CleanupFontLoadFail(NULL, NULL);
	}
	

	/* Allocate memory for our font structure */
	fntSP = (fontPtr) 
		GlobalAlloc(GMEM_FIXED, sizeof(struct font_struct));
	if(!fntSP) 
	{
		db_log_fired("DB ERROR: Not enough memory for font structure.");
		return CleanupFontLoadFail(fH, NULL);
	}	
	
	/* Nullify pointers inside the font structure. */
	fntSP->bitmapP = NULL;
	fntSP->prop_width_dataP = NULL;
	
	/* Load up font structure */
	status = ReadFile(fH, fntSP, sizeof(struct font_struct), &bytesRead, 
		NULL);
	if((!status) || (bytesRead != sizeof(struct font_struct)))
	{
		db_log_fired("DB ERROR: Error reading structure from font file.");
		return CleanupFontLoadFail(fH, fntSP);
	}
		
	/* Allocate memory for font bitmap */
	fntSP->bitmapP = (unsigned char *) GlobalAlloc(GMEM_FIXED, 
		fntSP->byte_width * dd_mode.bitsPerPixel * fntSP->height * 128);
	if(!fntSP->bitmapP)	
	{
		db_log_fired("DB ERROR: Not enough memory for font bitmap.");
		return CleanupFontLoadFail(fH, fntSP);
	}	
	
	/* Work out proportional text widths for this font
 	 * (if the font is proportional).
	 */
	if(fntSP->width == PROP_WIDTH)
	{
		/* Allocate memory for proportional width data */
		fntSP->prop_width_dataP = (unsigned short *) 
			GlobalAlloc(GMEM_FIXED, 256);
		if(!fntSP->prop_width_dataP) 
		{
			db_log_fired("DB ERROR: Not enough memory for proportional "
				"font data");
			return CleanupFontLoadFail(fH, fntSP);
		}	

		/* Read proportional width data */
		status = ReadFile(fH, fntSP->prop_width_dataP, 256, &bytesRead,
			NULL);
		if((!status) || (bytesRead != 256))
		{
			db_log_fired("DB ERROR: Error reading proportional font data "
				"from file.");
			return CleanupFontLoadFail(fH, fntSP);
		}
			
		/* Round width of font to nearest long word. */
		{
			int i;
			unsigned short *propP = fntSP->prop_width_dataP;
			
			if(dd_mode.bitsPerPixel == 8)
			{
				for(i = 0; i < 128; i ++)
				{
					/* Round up to 4 pixels. */
					*propP = (unsigned short) ((*propP + 3U) & (~3U));
				}
				propP++;
			}
			else
			{
				for(i = 0; i < 128; i ++)
				{
					/* Round up to 2 pixels. */
					*propP = (unsigned short) ((*propP + 1U) & (~1U));
				}
				propP++;
			}
		}	
	}
	else
	{
		/* Round width to nearest long word. */
		if(dd_mode.bitsPerPixel == 8)
		{
			/* Round up to 4 pixels. */
			fntSP->width = (unsigned char) ((fntSP->width + 2) & (~3));
		}
		else
		{
			/* Round up to 2 pixels. */
			fntSP->width = (unsigned char) ((fntSP->width + 1) & (~1));
		}
	}
	
	/* Load up bitmap data */
	dest.cP = fntSP->bitmapP;
	for(c = 0; c < 128; c ++)
	{
		for(y = 0; y < fntSP->height; y ++)
		{
			for(byte = 0; byte < fntSP->byte_width; byte ++)
			{
				unsigned char bitMask = 0x80;
				
				/* Read a byte of data */
				status = ReadFile(fH, &inByte, 1, &bytesRead, NULL);
				if((!status) || (bytesRead != 1))
				{
					db_log_fired("DB ERROR: Error reading font bitmap from "
						"file.");
					return CleanupFontLoadFail(fH, fntSP);
				}
				
				/* Translate 1 bit per pixel data into current Direct Draw
				 * screen mode bit depth.
				 */
				if(dd_mode.bitsPerPixel == 8)
				{ 
					do 
					{
						if(inByte & bitMask) *(dest.cP) = 
							(unsigned char) dd_mode.foreCol;
						else *(dest.cP) = (unsigned char) dd_mode.backCol;
						dest.cP++;
						
						/* Shift bitMask 1 bit to the right */
						bitMask >>= 1;
					}
					while(bitMask);
				}
				else
				{
					do 
					{
						if(inByte & bitMask) *(dest.wP) = dd_mode.foreCol;
						else *(dest.wP) = dd_mode.backCol;
						dest.wP++;
						
						/* Shift bitMask 1 bit to the right */
						bitMask >>= 1;
					}
					while(bitMask);
				}
			}
		}
	}

	/* Close the font file */
	CloseHandle(fH);

	return fntSP;
}

static fontPtr CleanupFontLoadFail(HANDLE fH, fontPtr fontP)
{
	/* Close file if necessary */
	if(fH) CloseHandle(fH);
	
	/* Is the font struct allocated? */
	if(fontP)
	{
		/* Yes. Is the bitmap allocated? If so, free it. */
		if(fontP->bitmapP) GlobalFree(fontP->bitmapP);
		
		/* Is the proportional width data allocated. If so, free it. */
		if(fontP->prop_width_dataP) GlobalFree(fontP->prop_width_dataP);
		
		/* Free the font structure. */
		GlobalFree(fontP);
	}
	
	return NULL;
}

static void out_text(LPDIRECTDRAWSURFACE surfP, int xc, int yc, 
	const char *text, short x_limit, fontPtr fP)
{
	register unsigned long *srcP, *destP;
	register unsigned int x, y; 
	unsigned long heightTimesPitch, charOffset;
	unsigned int prop_width;
	int srcIncr, longsPerLine;
	unsigned int bitShift;
	DDSURFACEDESC surfaceDesc;
	
	/* Lock the surface. */
	{
		HRESULT res;
		
		surfaceDesc.dwSize = sizeof surfaceDesc;
		res = IDirectDrawSurface_Lock(surfP, NULL, &surfaceDesc, 
			DDLOCK_WAIT, NULL);
		if(res != DD_OK) 
		{
			db_log3("Couldn't lock surface.");
			return;
		}
	}
	
	/* Round xc to nearest long word. */
	if(dd_mode.bitsPerPixel == 8)
	{
		xc = (xc + 2) & (~3);
		bitShift = 2;
	}
	else
	{
		xc = (xc + 1) & (~1);
		bitShift = 1;
	}
	
	/* Point to DRAM buffer co-ordinate where the top left of the
	 * first character should be written.
	 */
	destP = (unsigned long *) surfaceDesc.lpSurface + 
		((yc * surfaceDesc.lPitch) >> 2) + (xc >> bitShift);
	heightTimesPitch = (fP->height * surfaceDesc.lPitch) >> 2;
	longsPerLine = (fP->byte_width * dd_mode.bitsPerPixel) >> 2;
	charOffset = longsPerLine * fP->height;

	/* Write our text string */
	while(*text != '\0')
	{
		/* Blit a single character */
		/* Point srcP to first byte of the bitmap for the current
		 * character.
		 */
		srcP = ((unsigned long *) fP->bitmapP) + (*text) * charOffset;

		/* Get width of this character (in pixels). */
		if(fP->width == PROP_WIDTH)
			prop_width = *(fP->prop_width_dataP + (*text));
		else prop_width = fP->width;
		
		/* Check we will not exceed our original x_limit if we blit
		 * this character. If we will, we should stop writing.
		 */
		x_limit = (short) (x_limit - prop_width);
		if(x_limit < 0) break;
		
		/* Convert prop_width from pixels to longs. */
		prop_width >>= bitShift;
		
		srcIncr = longsPerLine - prop_width;
		
		y = fP->height;
		do
		{
			x = prop_width;
			do
			{
				/* Move 1 long word. */
				*destP++ = *srcP++;
			}
			while(--x != 0);
			
			/* Point to start of next horizontal line of character square
			 * in DRAM buffer.
			 */
			destP += (surfaceDesc.lPitch >> 2) - prop_width;
			srcP += srcIncr;
		}
		while(--y != 0);
		
		/* Point to start of next character position in DRAM buffer */
		destP -= heightTimesPitch - prop_width;

		/* Advance one character in text string */
		text++;
	}
	
	/* Unlock surface. */
	{
		HRESULT res;
		
		res = IDirectDrawSurface_Unlock(surfP, 
			(LPVOID) surfaceDesc.lpSurface);
		if(res != DD_OK) db_log_fired("Couldn't unlock surface.");
	}
	return;
}

static void Debounce(void)
{
	BOOL bouncing;
	
	/* Debounce all the keys we use - that is Y, N, and RETURN. */
	do
	{
		bouncing = FALSE;
		if(GetAsyncKeyState('Y') & 0x8000) bouncing = TRUE;
		if(GetAsyncKeyState('N') & 0x8000) bouncing = TRUE;
		if(GetAsyncKeyState('X') & 0x8000) bouncing = TRUE;
		if(GetAsyncKeyState(VK_RETURN) & 0x8000) bouncing = TRUE;
	}
	while(bouncing);
}

static void DbWaitForHw(void)
{
	/* Wait until the last flip is finished and the last blt done */
	BOOL finished;

	/* Stay in this loop until the hardware is free. */
	do
	{
		finished = TRUE;   

		/* Make sure all flips are done. */
		if(dd_mode.bltOrFlip == DB_FLIP)
		{
			if(IDirectDrawSurface_GetFlipStatus(
				(LPDIRECTDRAWSURFACE) dd_mode.visibleSurfaceP, 
				DDGFS_ISFLIPDONE) != DD_OK)
				finished = FALSE;
		}
	}
	while(!finished);
}

static void DbFlip(void)
{
	LPDIRECTDRAWSURFACE	fromSurfP = 
		(LPDIRECTDRAWSURFACE) dd_mode.drawSurfaceP;
	LPDIRECTDRAWSURFACE	toSurfP = 
		(LPDIRECTDRAWSURFACE) dd_mode.visibleSurfaceP;
	HRESULT res;
	
	/* Try to flip the screen. */
	res = IDirectDrawSurface_Flip(toSurfP, fromSurfP, DDFLIP_WAIT);
	
	if(res != DD_OK)
		db_log_fired("Internal debug flip failed - message lost!");
}

static void DbBlt(void)
{
	LPDIRECTDRAWSURFACE	fromSurfP = 
		(LPDIRECTDRAWSURFACE) dd_mode.drawSurfaceP;
	LPDIRECTDRAWSURFACE	toSurfP = 
		(LPDIRECTDRAWSURFACE) dd_mode.visibleSurfaceP;
	HRESULT res;
	RECT toRect;
	
	/* Initialise to Rect. */
	toRect.left = dd_mode.bltXOffset;
	toRect.top = dd_mode.bltYOffset;
	toRect.right = toRect.left + dd_mode.width;
	toRect.bottom = toRect.top + dd_mode.height;
	
	/* Try to blit from the draw to the visible surface. */
	res = IDirectDrawSurface_Blt(toSurfP, &toRect, fromSurfP, NULL,
		DDBLT_WAIT, NULL);	
		
	if(res != DD_OK) 
		db_log_fired("Internal debug blit failed - message lost.");	
}

#endif

#else
	;
#endif /* ! NDEBUG */
