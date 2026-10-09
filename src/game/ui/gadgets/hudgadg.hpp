/*
	
	hudgadg.hpp

*/

#ifndef _hudgadg
#define _hudgadg 1

	#ifndef _gadget
	#include "gadget.h"
	#endif

#ifdef __cplusplus

/* Version settings *****************************************************/

/* Constants  ***********************************************************/

/* Macros ***************************************************************/

/* Type definitions *****************************************************/
	#if UseGadgets

		#ifndef GAMEDEF_INCLUDED

			#ifndef MODULE_INCLUDED
			#include "module.h"
			#endif
				// irritatingly, GAMEDEF.H assumes MODULE.H has already been included...

		#include "gamedef.h"
		#endif

		#ifndef _scstring
			#ifdef __cplusplus
				extern "C++" {
					// JH 140298 - C++ header can only be included in C++ source and must have C++ linkage
					#include "scstring.hpp"
				}
			#endif
		#endif

	class TextReportGadget; // fully declared in TREPGADG.HPP

	// HUD Gadget is an abstract base class for 3 types of HUD; one for each species
	// It's abstract because the Render() method remains pure virtual
	class HUDGadget : public Gadget
	{
	public:
		static HUDGadget* GetHUD(void);

		// Factory method:
		static HUDGadget* MakeHUD
		(
			I_PLAYER_TYPE IPlayerType_ToMake
		);

		virtual void AddTextReport
		(
			SCString* pSCString_ToAdd
				// ultimately turn into an MCString
		) = 0;

		virtual void ClearTheTextReportQueue(void) = 0;

		
		#if EnableStatusPanels
		virtual void RequestStatusPanel
		(
			enum StatusPanelIndex I_StatusPanel
		) = 0;
		virtual void NoRequestedPanel(void) = 0;
		#endif

		virtual void CharTyped
		(
			char Ch
				// note that this _is _ a char
		) = 0;

		virtual void Key_Backspace(void) = 0;
		virtual void Key_End(void) = 0;
		virtual void Key_Home(void) = 0;
		virtual void Key_Left(void) = 0;
		virtual void Key_Up(void) = 0;
		virtual void Key_Right(void) = 0;
		virtual void Key_Down(void) = 0;
		virtual void Key_Delete(void) = 0;
		virtual void Key_Tab(void) = 0;

		virtual void Jitter(int FixP_Magnitude) = 0;


		// Destructor:
		

	protected:
		// Constructor is protected since an abstract class
		HUDGadget
		(
			#if debug
			char* DebugName
			#endif
		);

	private:
		static HUDGadget* pSingleton;

	#if 0
	// Temporary text feedback implementation:
	protected:
		SCString* pSCString_Current;
	#endif
	
	public:
		virtual ~HUDGadget();
	};

	// Inline methods:
		inline /*static*/ HUDGadget* HUDGadget::GetHUD(void)
		{
			return pSingleton;
		}
	#endif // UseGadgets

/* Exported globals *****************************************************/

/* Function prototypes **************************************************/



/* End of the header ****************************************************/


#endif

#endif
