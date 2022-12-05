/*******************************************************************
 *
 *    DESCRIPTION: 	trepgadg.cpp - text report gadget
 *
 *    AUTHOR: David Malcolm
 *
 *    HISTORY:  Created 14/11/97
 *
 *******************************************************************/

/* Includes ********************************************************/
#include "3dc.h"
#include "daemon.h"
#include "trepgadg.hpp"
#include "teletype.hpp"
#include "coordstr.hpp"
#include "trig666.hpp"

#include "indexfnt.hpp"

#include "wrapstr.hpp"

#include "inline.h"

#include "iofocus.h"

#include "consolelog.hpp"

	#include "rootgadg.hpp"
	#include "hudgadg.hpp"
		// for ClearTheQueue()

	#define UseLocalAssert Yes
	#include "ourasert.h"

/* Version settings ************************************************/

/* Constants *******************************************************/
	#define MAX_MESSAGES_TO_DISPLAY		(20)
	#define PARTIAL_MESSAGES_TO_DISPLAY	(5)

	#define FIXP_SECONDS_UNTIL_TEXT_REPORTS_DISAPPEAR (ONE_FIXED * 5)
	#define FIXP_CHEESY_FLASH_RATE		(ONE_FIXED * 6)
	#define INT_CHEESY_FLASH_DURATION	(1)

/* Macros **********************************************************/

/* Imported function prototypes ************************************/

/* Imported data ***************************************************/
#ifdef __cplusplus
	extern "C"
	{
#endif
		#if 0
		extern OurBool			DaveDebugOn;
		extern FDIEXTENSIONTAG	FDIET_Dummy;
		extern IFEXTENSIONTAG	IFET_Dummy;
		extern FDIQUAD			FDIQuad_WholeScreen;
		extern FDIPOS			FDIPos_Origin;
		extern FDIPOS			FDIPos_ScreenCentre;
		extern IFOBJECTLOCATION IFObjLoc_Origin;
		extern UncompressedGlobalPlotAtomID UGPAID_StandardNull;
		extern IFCOLOUR			IFColour_Dummy;
 		extern IFVECTOR			IFVec_Zero;
		#endif
		extern int RealFrameTime;
#ifdef __cplusplus
	};
#endif


static int NumberOfLinesToDisplay=0;

/* Exported globals ************************************************/

/* Internal type definitions ***************************************/
	#if UseGadgets
	class TextReportDaemon_Scroll : public AcyclicHomingCoordinate
	{
	public:
		TextReportDaemon_Scroll
		(
			TextReportGadget* pTextReportGadg
		);
		~TextReportDaemon_Scroll();
	private:
	};
	class TextReportDaemon_Disappear : public PulsingTriggerDaemon
	{
	public:
		TextReportDaemon_Disappear
		(
			TextReportGadget* pTextReportGadg
		);

		~TextReportDaemon_Disappear();

		void Triggered(void);

	private:
		TextReportGadget* pTextReportGadg_Val;
	};
	class CheesyDaemon_Flash : public CyclicPulsingCoordinate
	{
	public:
		CheesyDaemon_Flash();
	private:
	};

	class CheesyDaemon_Lifetime : public AcyclicFixedSpeedHoming
	{
	public:
		CheesyDaemon_Lifetime();
		OurBool bStillAlive(void);
		void Reset(void);
	};

	#endif // UseGadgets

/* Internal function prototypes ************************************/
#if UseGadgets
void TestStringRender_Unclipped
(
	r2pos& R2Pos_Cursor,
		// start position for string;
		// gets written back to with final position

		// Renders as a single line; it is asserted that the result is fully within
		// the physical screen (i.e. already clipped)
	const SCString& SCStr
);
#endif

/* Internal globals ************************************************/

/* Exported function definitions ***********************************/
#if UseGadgets
// class TextReportGadget : public Gadget
// public:
void TextReportGadget :: Render
(
	const struct r2pos& R2Pos,
	const struct r2rect& R2Rect_Clip,
	int FixP_Alpha
)
{
	/* PRECONDITION */
	{
	}

	/* CODE */
	{
		IndexedFont* pLetterFont = IndexedFont :: GetFont( I_Font_TeletypeLettering );
		GLOBALASSERT( pLetterFont );

		int FontHeight = pLetterFont -> GetHeight();

		struct r2rect R2Rect_ClipForText = r2rect
		(
			R2Pos,
			TEXT_REPORT_MAX_W,
			( MAX_MESSAGES_TO_DISPLAY * FontHeight )
		);

		// Clip this rectangle with the passed clip rectangle:
		R2Rect_Clip . Clip( R2Rect_ClipForText );

		// Alpha mounting poly:
		{
			R2Rect_ClipForText . AlphaFill
			(
				0,
				64,
				0,
				(FixP_Alpha * 2 / (256 * 3)) 
					// unsigned char translucency
			);
		}

		#if 0
		{
			SCString* pSCStr_Temp = new SCString("PLACEHOLDER STRING");
			r2pos R2Pos_Cursor = R2Rect_ClipForText . GetPos();

			TestStringRender_Unclipped
			(
				R2Pos_Cursor, // r2pos& R2Pos_Cursor,
					// start position for string;
					// gets written back to with final position

					// Renders as a single line; it is asserted that the result is fully within
					// the physical screen (i.e. already clipped)
				*pSCStr_Temp // const SCString& SCStr
			);

			pSCStr_Temp -> R_Release();
		}
		#endif

		// Calculate y-displacement based upon number of lines:
		int HeightOfLines = (List_pTeletypeGadg_Displaying . size() * FontHeight);

		struct r2pos R2Pos_Teletype = R2Pos;

		// displace up so that last line is just at bottom of view:
		R2Pos_Teletype . y -= (HeightOfLines - ( MAX_MESSAGES_TO_DISPLAY * FontHeight ) );
		
		// Iterate through the teletype gadgets:
		{
			for
			(
				List_Iterator_Forward<TeletypeGadget*> oi(&(List_pTeletypeGadg_Displaying));
				!oi.done();
				oi.next()
			)
			{
				#if 1
				oi() -> Render
				(
					R2Pos_Teletype, // const struct r2pos& R2Pos,
					R2Rect_ClipForText, // const struct r2rect& R2Rect_Clip,
					FixP_Alpha // int FixP_Alpha
				);
				R2Pos_Teletype . y += FontHeight;
				#else
				textprint
				(
					"Textline:\"%s\"\n",
					oi() -> pProjCh()
				);
				#endif
			}
		}

		// If the last one has finished; add a cheesy flashing cursor:
		if ( List_pTeletypeGadg_Displaying . size() > 0)
		{
			TeletypeGadget* pLast = List_pTeletypeGadg_Displaying . last_entry();

			GLOBALASSERT( pLast );

			if ( pLast -> HasFinishedPrinting() )
			{

				if ( p666_CheeseLifetime -> bStillAlive() )
				{
					pLast -> DirectRenderCursor
					(
						r2pos(R2Pos_Teletype.x,R2Pos_Teletype.y-FontHeight),
						R2Rect_ClipForText,
						MUL_FIXED( FixP_Alpha, p666_CheeseFlash -> GetCoord_FixP() )
					);
				}
			}
		}


		#if 0
		// Diagnostic on queuing messages:
		{
			textprint("Text report queue:\n");
			for
			(
				List_Iterator_Forward<SCString*> oi(&(List_pSCString_ToAppear));
					// a _pointer_ 
					// to the list.
				!oi.done();
				oi.next()
			)
			{
				textprint
				(
					"Queuing:\"%s\"\n",
					oi() -> pProjCh()
				);
			}
		}
		#endif

	}
}

struct r2pos TextReportGadget :: GetPos_Rel
(
	const struct r2rect& R2Rect_Parent
) const
{
	return r2pos
	(
		(R2Rect_Parent . Width() - TEXT_REPORT_MAX_W)/2,
		p666_Scroll -> GetCoord_Int()
	);
}

r2size TextReportGadget :: GetSize
(
	const struct r2rect& // R2Rect_Parent
) const
{
	IndexedFont* pLetterFont = IndexedFont :: GetFont( I_Font_TeletypeLettering );
	GLOBALASSERT( pLetterFont );

	return r2size
	(
		TEXT_REPORT_MAX_W,
		( MAX_MESSAGES_TO_DISPLAY * pLetterFont -> GetHeight() )
	);
}


TextReportGadget :: TextReportGadget
(
) : Gadget
	(
		#if debug
		"TextReportGadget"
		#endif
	),
	RefList_SCString_ToAppear(),
	List_pTeletypeGadg_Displaying()
{
	/* PRECONDITION */
	{
	}

	/* CODE */
	{
		#if 0
		pSCString_Current = NULL;
		#endif

		p666_Scroll = new TextReportDaemon_Scroll
		(
			this
		);
		p666_Disappear = new TextReportDaemon_Disappear
		(
			this
		);

		p666_CheeseFlash = new CheesyDaemon_Flash();
		p666_CheeseLifetime = new CheesyDaemon_Lifetime();
	}
}

TextReportGadget :: ~TextReportGadget()
{
	/* PRECONDITION */
	{
	}

	/* CODE */
	{
		#if 1
		{
			for
			(
				List_Iterator_Forward<TeletypeGadget*> oi(&(List_pTeletypeGadg_Displaying));
				!oi.done();
				oi.next()
			)
			{
				// Delete the teletype objects:
				delete( oi() );
			}
		}
		#else
		if ( pSCString_Current )
		{
			pSCString_Current -> R_Release();
		}
		#endif

		delete p666_Scroll;
		delete p666_Disappear;
		delete p666_CheeseFlash;
		delete p666_CheeseLifetime;

	}
	NumberOfLinesToDisplay=0;
}

void TextReportGadget :: AddTextReport
(
	SCString* pSCString_ToAdd
)
{
	/* PRECONDITION */
	{
		GLOBALASSERT( pSCString_ToAdd );
	}

	/* CODE */
	{
		#if 1
		IndexedFont* pLetterFont = IndexedFont :: GetFont( I_Font_TeletypeLettering );
		GLOBALASSERT( pLetterFont );

		List<SCString*>* pList_pSCString_Wrapped = WordWrap :: DeprecatedMake
		(
			*pSCString_ToAdd, // const SCString& SCString_In,

			*pLetterFont, // const IndexedFont& IndexedFnt_In,

			TEXT_REPORT_MAX_W, // int W_FirstLine_In,
			TEXT_REPORT_MAX_W // int W_Subsequently_In
		);

		// Iterate through list of strings, adding each as teletype gadgets to the report,
		// then releasing the strings:
//		NumberOfLinesToDisplay=0;
		for
		(
			LIF<SCString*> oi( pList_pSCString_Wrapped );
			!oi.done();
			oi.next()
		)
		{
			GLOBALASSERT( oi() );
		    AddTeletypeLine( oi() );
			oi() -> R_Release();
			NumberOfLinesToDisplay++;
		}
		{
			p666_Scroll -> SetTarget_Int
			(
				( IOFOCUS_AcceptTyping() )
				?
				( GetFullyOnScreenScrollCoord() )
				:
				( GetPartiallyOnScreenScrollCoord() )
			);
			p666_Disappear -> Stop();
		}


		delete pList_pSCString_Wrapped;
		#else
		AddTeletypeLine
		(
			pSCString_ToAdd
		);
		#endif
	}
}

void TextReportGadget :: ClearQueue(void)
{
	// clears the queue of buffered messages; could be handy if you've
	// started a listing of 300 module names
	
	RefList_SCString_ToAppear . EmptyYourself();

	#if 0
	{
		int NumKilled = RefList_SCString_ToAppear . NumEntries();
		
		SCString* pSCString_Temp1 = new SCString("CLEARED MESSAGE DISPLAY QUEUE; NUM LINES=");
			// LOCALISEME()
		SCString* pSCString_Temp2 = new SCString(NumKilled);

		SCString* pSCString_Feedback = new SCString
		(
			pSCString_Temp1,
			pSCString_Temp2
		);

		pSCString_Temp2	-> R_Release();
		pSCString_Temp1	-> R_Release();

		pSCString_Feedback -> SendToScreen();

		pSCString_Feedback -> R_Release();
	}	
	#endif
}

/*static*/ void TextReportGadget :: ClearTheQueue(void)
{
	// tries to find the (singleton) queue and clears it

	GLOBALASSERT( RootGadget :: GetRoot() );

	if ( RootGadget :: GetRoot() -> GetHUD() )
	{
		RootGadget :: GetRoot() -> GetHUD() -> ClearTheTextReportQueue();
	}
}


void TextReportGadget :: TeletypeCompletionHook(void)
{
	// If the queue is non-empty, add first string as new teletype object:

	// Destructive read the first string in the queue:
	SCString* pSCString_FirstInQueue = RefList_SCString_ToAppear . GetYourFirst();

	if
	(
		pSCString_FirstInQueue
	)
	{
		DirectAddTeletypeLine
		(
			pSCString_FirstInQueue
		);

		// GetYourFirst() leaves us holding the reference...
		pSCString_FirstInQueue -> R_Release();
	}
	else
	{
		// No more messages;
		// Reset disappearance time, and "arm" the disappearance:
		p666_Disappear -> SetFuse_FixP
		(
			FIXP_SECONDS_UNTIL_TEXT_REPORTS_DISAPPEAR	// int FixP_Fuse // time until it next triggers; doesn't change the period
		);

		p666_Disappear -> Start();

		p666_CheeseLifetime -> Reset();
	}

	//add a timer for this line of text
	LineTimes.add_entry(FIXP_SECONDS_UNTIL_TEXT_REPORTS_DISAPPEAR);
}

int TextReportGadget :: GetFullyOnScreenScrollCoord(void)
{
	return 0;
}

int TextReportGadget :: GetPartiallyOnScreenScrollCoord(void)
{
	IndexedFont* pLetterFont = IndexedFont :: GetFont( I_Font_TeletypeLettering );
	GLOBALASSERT( pLetterFont );

	
	return -((MAX_MESSAGES_TO_DISPLAY - NumberOfLinesToDisplay) * pLetterFont -> GetHeight() );
}

int TextReportGadget :: GetOffScreenScrollCoord(void)
{
	IndexedFont* pLetterFont = IndexedFont :: GetFont( I_Font_TeletypeLettering );
	GLOBALASSERT( pLetterFont );
	
	return -(MAX_MESSAGES_TO_DISPLAY * pLetterFont -> GetHeight() );
}

void TextReportGadget :: Disappear(void)
{
	// to be called only by TextReportDaemon_Disappear
	//clear the list of timers for when individual lines should disappear
	NumberOfLinesToDisplay = 0;
	while(LineTimes.size()) LineTimes.delete_first_entry();

	p666_Scroll -> SetTarget_Int
	(
		GetOffScreenScrollCoord()
			// int Int_TargetCoord
	);
	p666_Disappear -> Stop();
}

void TextReportGadget :: ForceOnScreen(void)
{
	// called by the marine HUD gadget if input focus set to typing
	// to stop the object scrolling away
	p666_Scroll -> SetTarget_Int
	(
		GetFullyOnScreenScrollCoord()
			// int Int_TargetCoord
	);
	p666_Disappear -> Stop();	
}



//private:
void TextReportGadget :: AddTeletypeLine
(
	SCString* pSCString_ToAdd
)
{
	// Either to queue, or direct

	/* PRECONDITION */
	{
		GLOBALASSERT( pSCString_ToAdd );
	}

	/* CODE */
	{
		/* KJL 11:33:13 30/03/98 - duplicate output to log file */
		OutputToConsoleLogfile( pSCString_ToAdd -> pProjCh() );

		// If finished displaying last message; immediately display this one, otherwise
		// add to list.  The message display daemon should process the queue:
		if ( List_pTeletypeGadg_Displaying . size() > 0 )
		{
			if
			(
				!List_pTeletypeGadg_Displaying . last_entry() -> HasFinishedPrinting()
			)
			{
				// Can't add a new teletype object yet; there's an existing one which
				// hasn't finished yet; add to queue:

				RefList_SCString_ToAppear . AddToEnd
				(
					*pSCString_ToAdd
				);

				return;
			}
		}

		// Otherwise you can add the teletype object:
		{
			DirectAddTeletypeLine
			(
				pSCString_ToAdd
			);
		}
	}
}

void TextReportGadget :: DirectAddTeletypeLine
(
	SCString* pSCString_ToAdd
)
{
	/* PRECONDITION */
	{
		GLOBALASSERT( pSCString_ToAdd );
	}

	/* CODE */
	{
		// You no longer own a reference to this string
		#if 0
		{
			IndexedFont* pLetterFont = IndexedFont :: GetFont( I_Font_TeletypeLettering );
			GLOBALASSERT( pLetterFont );

			List<SCString*>* pList_pSCString_Wrapped = WordWrap :: Make
			(
				*pSCString_ToAdd, // const SCString& SCString_In,

				*pLetterFont, // const IndexedFont& IndexedFnt_In,

				TEXT_REPORT_MAX_W, // int W_FirstLine_In,
				TEXT_REPORT_MAX_W // int W_Subsequently_In
			);

			pSCString_ToAdd -> R_Release();

			// Iterate through list of strings, adding each as teletype gadgets to the report,
			// then releasing the strings:
			for
			(
				LIF<SCString*> oi( pList_pSCString_Wrapped );
				!oi.done();
				oi.next()
			)
			{
			    List_pTeletypeGadg_Displaying . add_entry_end
			    (
					new TeletypeGadget
					(
						this, // TextReportGadget* pTextReportGadg,
						oi() // SCString* pSCString
					)
				);
				oi() -> R_Release();
			}

			delete pList_pSCString_Wrapped;
		}
		#else
		{
		    List_pTeletypeGadg_Displaying . add_entry_end
		    (
				new TeletypeGadget
				(
					this, // TextReportGadget* pTextReportGadg,
					pSCString_ToAdd // SCString* pSCString
				)
			);
		}
		#endif

		PostprocessForAddingTeletypeLine();
	}
}

void TextReportGadget :: PostprocessForAddingTeletypeLine(void)
{
	if ( List_pTeletypeGadg_Displaying . size() > MAX_MESSAGES_TO_DISPLAY )
	{
		TeletypeGadget* pTeletypeGadg = List_pTeletypeGadg_Displaying . first_entry();
		
		GLOBALASSERT( pTeletypeGadg );

		delete pTeletypeGadg;

		List_pTeletypeGadg_Displaying . delete_first_entry();
	}

	// Make it visible, either fully or partially, depending on input focus:
	{
		p666_Scroll -> SetTarget_Int
		(
			( IOFOCUS_AcceptTyping() )
			?
			( GetFullyOnScreenScrollCoord() )
			:
			( GetPartiallyOnScreenScrollCoord() )
		);
	}

	p666_Disappear -> Stop();
}


// need a daemon for handling scrolling, disposal of old messages, etc.

int TextReportGadget :: MinYDisplacement(void)
{
	return 0;
		// for now
}

int TextReportGadget :: MaxYDisplacement(void)
{
	return 0;
		// for now
}

void TextReportGadget :: UpdateLineTimes()
{
	int time;
	
	//if we are currently displaying to many lines ,g et rifd of the earliest ones
	while(NumberOfLinesToDisplay > MAX_MESSAGES_TO_DISPLAY)
	{
		if(LineTimes.size()) LineTimes.delete_first_entry();
		NumberOfLinesToDisplay--; 
	}
	
	//go through our list of timers for each line , updating them
	for(LIF<int> timelif(&LineTimes);!timelif.done();)
	{
		time=timelif()-RealFrameTime;
		if(time<=0)
		{
			//this timer has expired , so reduce the number of lines displayed by one
			//also delete this timer from the list
			timelif.delete_current();
			NumberOfLinesToDisplay--;
			if(NumberOfLinesToDisplay<0)
			{
				NumberOfLinesToDisplay=0;
			}
			
			//update the sceen coordinates (assuming the console isn't off screen)
			{
				p666_Scroll -> SetTarget_Int
				(
					( IOFOCUS_AcceptTyping() )
					?
					( GetFullyOnScreenScrollCoord() )
					:
					( GetPartiallyOnScreenScrollCoord() )
				);
			}
			
		}
		else
		{
			//update this timer
			timelif.change_current(time);
			timelif.next();
		}
	}
}

#endif //UseGadgets

/* Internal function definitions ***********************************/
#if UseGadgets
// class TextReportDaemon_Scroll : public AcyclicHomingCoordinate
// public:
TextReportDaemon_Scroll :: TextReportDaemon_Scroll
(
	TextReportGadget* pTextReportGadg
) : AcyclicHomingCoordinate
	(
		pTextReportGadg -> GetOffScreenScrollCoord(), //int Int_InitialCoord,
		pTextReportGadg -> GetOffScreenScrollCoord() // int Int_TargetCoord
	)
{
}

TextReportDaemon_Scroll :: ~TextReportDaemon_Scroll()
{
}
// private:
// class TextReportDaemon_Disappear : public PulsingTriggerDaemon
// public:
TextReportDaemon_Disappear :: TextReportDaemon_Disappear
(
	TextReportGadget* pTextReportGadg
) : PulsingTriggerDaemon
	(
		Yes, // OurBool fActive,
		FIXP_SECONDS_UNTIL_TEXT_REPORTS_DISAPPEAR // int FixP_Period // interval between triggers in seconds			
	)
{
	GLOBALASSERT( pTextReportGadg );
	pTextReportGadg_Val = pTextReportGadg;
}

TextReportDaemon_Disappear :: ~TextReportDaemon_Disappear()
{
}

void TextReportDaemon_Disappear :: Triggered(void)
{
	// Set the homing coordinate for the text report gadget so it disappears off the screen
	// again

	
	pTextReportGadg_Val -> Disappear();
}


// class CheesyFlashDaemon : public CyclicPulsingCoordinate
// public:
CheesyDaemon_Flash :: CheesyDaemon_Flash
(
) : CyclicPulsingCoordinate
	(
		0, // int Int_InitialCoord,
		1, // int Int_SecondCoord,
		(ONE_FIXED * 4), // int FixP_Velocity,
		Yes // OurBool fActive
	)
{
	// empty
}

// class CheesyDaemon_Lifetime : public AcyclicFixedSpeedHoming
// public:
CheesyDaemon_Lifetime :: CheesyDaemon_Lifetime
(
) : AcyclicFixedSpeedHoming
	(
		INT_CHEESY_FLASH_DURATION, // int Int_InitialCoord,
		0, // int Int_TargetCoord,
		ONE_FIXED // int FixP_Speed
	)
{
	// empty
}

OurBool CheesyDaemon_Lifetime :: bStillAlive(void)
{
	return ( GetCoord_FixP() > 0);
}

void CheesyDaemon_Lifetime :: Reset(void)
{
	SetCoord_Int( INT_CHEESY_FLASH_DURATION );
}



#endif // UseGadgets
