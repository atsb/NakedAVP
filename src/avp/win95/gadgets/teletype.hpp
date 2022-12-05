/*
	
	teletype.hpp

	Created 17/11/97 by David Malcolm

*/

#ifndef _teletype
#define _teletype 1

	#ifndef _gadget
	#include "gadget.h"
	#endif

/* Version settings *****************************************************/

/* Constants  ***********************************************************/
	#define I_Font_TeletypeLettering	(DATABASE_MESSAGE_FONT)

/* Macros ***************************************************************/

/* Type definitions *****************************************************/
	#if UseGadgets
		#ifndef _ourbool
		#include "ourbool.h"
		#endif

		#ifndef _scstring
		#include "scstring.hpp"
		#endif

// moved wrapper here since scstring.hpp is a C++ header and templates can't have C linkage
#ifdef __cplusplus
	extern "C" {
#endif

	class TeletypeDaemon; // fully declared in TELETYPE.CPP
	class TextReportGadget; // fully declared in TREPGADG.HPP

	class TeletypeGadget : public Gadget
	{
	public:
		void Render
		(
			const struct r2pos& R2Pos,
			const struct r2rect& R2Rect_Clip,
			int FixP_Alpha
		);
		TeletypeGadget
		(
			TextReportGadget* pTextReportGadg,
				// parent
			SCString* pSCString
		);
		~TeletypeGadget();

		OurBool HasFinishedPrinting(void);
			// so that the next line knows when to begin

		SCString* GetStringWithoutReference(void);
			// Doesn't bother adding to reference count:

		void InformParentOfTeletypeCompletion(void);

		void DirectRenderCursor
		(
			const struct r2pos& R2Pos,
			const struct r2rect& R2Rect_Clip,
			int FixP_Alpha
		);
			// called by parent so that it can render its cursor even if 
			// it's finished printing - so that the last message can have
			// a flashing cursor

	private:
		TextReportGadget* pTextReportGadg_Val;
			// so that when the daemon finishes it can inform the parent; the
			// parent can then start displaying any further lines in the queue...

		SCString* pSCString_Val;

		TeletypeDaemon* p666;

	};
	
		inline SCString* TeletypeGadget::GetStringWithoutReference(void)
		{
			// Doesn't bother adding to reference count:
			return pSCString_Val;
		}

	#endif // UseGadgets

/* Exported globals *****************************************************/

/* Function prototypes **************************************************/



/* End of the header ****************************************************/


#ifdef __cplusplus
	};
#endif

#endif
