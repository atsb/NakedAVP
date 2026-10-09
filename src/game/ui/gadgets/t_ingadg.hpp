/*
	
	t_ingadg.hpp

*/

#ifndef _t_ingadg_hpp
#define _t_ingadg_hpp 1

	#ifndef _gadget
	#include "gadget.h"
	#endif

	#if UseGadgets
		#ifndef _textin_hpp
		#include "textin.hpp"
		#endif

		#ifndef _scstring
		#include "scstring.hpp"
		#endif
	#endif

/* Version settings *****************************************************/

/* Constants  ***********************************************************/

/* Macros ***************************************************************/

/* Type definitions *****************************************************/
	#if UseGadgets
	class AcyclicFixedSpeedHoming; // fully declared in COORDSTR.HPP

	class AVPTextInputState : public TextInputState
	{
	public:
		AVPTextInputState() :
			TextInputState
			(
				Yes, // OurBool bForceUpperCase,
				"" // const char* pProjCh_Init
			)
		{
		}

		void Key_Up(void);
		void Key_Down(void);
		void Key_Tab(void);

	// Virtual function implementations:
	public:
		void ProcessCarriageReturn(void);
	protected:
		void TextEntryError(void);
	};


	class TextEntryGadget : public Gadget
	{
	private:
		AcyclicFixedSpeedHoming* p666_FadeIn;
			// Add an alpha channeled-box; with a fade-up
			// The daemon has a fixed point "relative" alpha value
		
		AVPTextInputState theState;

	public:
		void Render
		(
			const struct r2pos& R2Pos,
			const struct r2rect& R2Rect_Clip,
			int FixP_Alpha
		);
		void DontRender(void);
			// inform it that it's not being rendered (so it can fade out)

		TextEntryGadget();

		~TextEntryGadget();

		SCString& GetCurrentState(void);
			// returns a ref to a copy of the "string under construction" in its current state
	
		void CharTyped
		(
			char Ch
				// note that this _is _ a char
		)
		{
			theState . CharTyped( Ch );
		}		
		void Key_Backspace(void)
		{
			theState . Key_Backspace();
		}
		void Key_End(void)
		{
			theState . Key_End();
		}
		void Key_Home(void)
		{
			theState . Key_Home();
		}
		void Key_Left(void)
		{
			theState . Key_Left();
		}
		void Key_Up(void)
		{
			theState . Key_Up();
		}
		void Key_Right(void)
		{
			theState . Key_Right();
		}
		void Key_Down(void)
		{
			theState . Key_Down();
		}
		void Key_Delete(void)
		{
			theState . Key_Delete();
		}
		void Key_Tab(void)
		{
			theState . Key_Tab();
		}

		void SetString(SCString& SCString_ToUse)
		{
			theState . SetString(SCString_ToUse);
		}
	};
	#endif // UseGadgets

/* Exported globals *****************************************************/

/* Function prototypes **************************************************/



/* End of the header ****************************************************/


#endif
