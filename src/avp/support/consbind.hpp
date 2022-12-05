/*
	
	consbind.hpp

	Created 6/4/98 by DHM:
	
	Ability to bind strings to a key.  When the key is pressed, the string
	is passed to the console as if it had been typed.
*/

#ifndef _consbind_hpp
#define _consbind_hpp 1

	#if defined( _MSC_VER )
		#pragma once
	#endif

	#ifdef __cplusplus
		#ifndef _scstring
			#include "scstring.hpp"
		#endif

		#ifndef _reflist_hpp
			#include "reflist.hpp"
		#endif
	#endif /* __cplusplus */

/* Version settings *****************************************************/
#ifdef __cplusplus
	#define KeyBindingUses_WM_KEYDOWN	No
		// if this is set to yes, the system works off the WM_KEYDOWN
		// messages, which provide a system for getting at the keyboard
		// which is guaranteed to work for the codes in pp247-9 of
		// Petzold 4th Edition, across all locales.
		// It also:
		// 	- debounces for free, then gives a typematic action
		//	- won't lose keystrokes, even at low framerates
		//	- provides ordering between all the keystrokes

	#define KeyBindingUses_KEY_ID	Yes
		// if this is set, the system accesses the KeyboardInput[] array
		// in IO.C  Currently (7/4/98) this comes from DirectInput, via
		// DI_FUNC.CPP
		// I have been told that this has the advantage of allowing us to treat
		// mouse/joystick buttons in an analagous way.  However it has
		// the following disadvantages:
		//	- it will lose keystrokes at low framerates (below 60Hz at the
		// rate I type)
		//	- it loses the ordering of keystrokes that come in the same frame
		//	- I believe it is locale-dependent; I believe there's no guarantee
		// it will remap according to settings in the Control Panel.
		// 
		// There's no debouncing at present.
#endif /* __cplusplus */


/* Constants  ***********************************************************/

/* Macros ***************************************************************/

/* Type definitions *****************************************************/
#ifdef __cplusplus
	#if KeyBindingUses_KEY_ID
	typedef enum KEY_ID BindableKey;
	#endif

	#if KeyBindingUses_WM_KEYDOWN
	typedef WPARAM BindableKey;
	#endif

	class KeyBinding
	{
	public:
		static void ParseBindCommand
		(
			ProjChar* pProjCh_ToParse
		);
		static void ParseUnbindCommand
		(
			ProjChar* pProjCh_ToParse
		);

		#if 0
		static void AttemptToBind
		(
			SCString* pSCString_Key, // description of key
			SCString* pSCString_ToBind // string to be bound
		);
		static void AttemptToUnbind
		(			
			SCString* pSCString_Key // description of key
		);
		#endif
		static void UnbindAll(void);

		static void ListAllBindings(void);

		static void WriteToConfigFile(char* Filename);
			// overwrites the file with a batch file that'll
			// restore current bindings
			// Also destroys all current bindings, so that
			// next time into the game you don't get a second
			// lot when the config file is processed

		#if KeyBindingUses_WM_KEYDOWN
		static void Process_WM_KEYDOWN
		(
			WPARAM wParam
		);
		#endif

		#if KeyBindingUses_KEY_ID
		static void Maintain(void);
		#endif

	public:
		static int bEcho;
		
	private:
		// Private ctor/dtor; to be called only by static fns of the class:
		KeyBinding
		(
			BindableKey theKey_ToUse,
			SCString* pSCString_ToBind
		);
		~KeyBinding();

		#if 0
		static OurBool bGetKeyForString
		(
			BindableKey& theKey_Out,
			const ProjChar* const pProjCh_In
		);
			// takes an input string and tries to figure out
			// what the corresponding WPARAM would be...
			// Returns truth if it manages to get a sensible value
			// into the output area.
		#endif
			

		static void ErrorDontRecogniseKey( SCString* pSCString_Key );

		void ListThis(void) const;
			// used by ListAllBindings()

		static SCString* MakeStringForKey
		(
			BindableKey theKey
		);

		static OurBool ParseBindCommand
		(
			BindableKey& theKey_Out,
			ProjChar** ppProjCh_Out,
				// returns where in the input string to continue processing

			ProjChar* pProjCh_In
		);	// returns Yes if it understands the binding and fills out the output


	private:
		BindableKey theKey;
		SCString* pSCString_ToOutput;

		// Maintain a static list of all of objects of the class:
		static List<KeyBinding*> List_pKeyBindings;

		// A list that ought to be local to Process_WM_KEYDOWN()
		// and the Maintain() functions
		// but has been made static to the class as an optimisation:
		static RefList<SCString> PendingList;
		

	};
		// Should the binding be debounced/undebounced?
		// Should the string appear to be typed?

		// Algorithm for processing?

		// Perhaps should just look at Windows messages for key down,
		// like we should have been doing all along...
#endif /* __cplusplus */

/* Exported globals *****************************************************/

/* Function prototypes **************************************************/

#ifdef __cplusplus
	extern "C" {
#endif
		void CONSBIND_WriteKeyBindingsToConfigFile(void);
#ifdef __cplusplus
	};
#endif


/* End of the header ****************************************************/



#endif
