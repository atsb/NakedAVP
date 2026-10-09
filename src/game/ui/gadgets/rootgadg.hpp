/*
	
	rootgadg.hpp

*/

#ifndef _rootgadg
#define _rootgadg 1

	#ifndef _gadget
	#include "gadget.h"
	#endif

#ifdef __cplusplus
	extern "C" {
#endif

/* Version settings *****************************************************/

/* Constants  ***********************************************************/

/* Macros ***************************************************************/

/* Type definitions *****************************************************/

	#if UseGadgets
	class HUDGadget; // fully declared in HUDGADG.HPP

	class RootGadget : public Gadget
	{
		friend void GADGET_Init(void);
		friend void GADGET_UnInit(void);
			// friend functions: these get permission in order to allow
			// construction/destruction

	public:
		void Render
		(
			const struct r2pos& R2Pos,
			const struct r2rect& R2Rect_Clip,
			int FixP_Alpha
		);

		static RootGadget* GetRoot(void);

		HUDGadget* GetHUD(void);

		void RefreshHUD(void);

	private:
		RootGadget();
		

	private:
		static RootGadget* pSingleton;

		HUDGadget* pHUDGadg;
			// allowed to be NULL if no head-up-display e.g. when not in a game
	private:
		~RootGadget();
	};
	
	// Inline methods:
		inline /*static*/ RootGadget* RootGadget::GetRoot(void)
		{
			return pSingleton;
		}
		inline HUDGadget* RootGadget::GetHUD(void)
		{
			return pHUDGadg;
		}
	#endif // UseGadgets

/* Exported globals *****************************************************/

/* Function prototypes **************************************************/



/* End of the header ****************************************************/


#ifdef __cplusplus
	};
#endif

#endif
