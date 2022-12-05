/*
	
	missions.hpp

*/

#ifndef _missions
#define _missions 1

	#if defined( _MSC_VER )
		#pragma once
	#endif

	#ifndef _scstring
	#include "scstring.hpp"
	#endif

	#ifndef _langenum_h_
	#include "langenum.h"
	#endif

	#ifndef _strtab
	#include "strtab.hpp"
	#endif

#ifdef __cplusplus
	extern "C" {
#endif

/* Version settings *****************************************************/
	#define WithinTheGame	Yes
		// as opposed to within the editor

/* Constants  ***********************************************************/

/* Macros ***************************************************************/

/* Type definitions *****************************************************/
	// Enum for what can happen when a mission event is triggered; these
	// effects are "meta-events" e.g. the levels is completed.  These are
	// post-processing after the rest of the mission event processing occurs.
	enum MissionEffects
	{
		MissionFX_None = 0,

		MissionFX_UncoversNext,

		MissionFX_CompletesLevel,

		// Possibility of:
		//	MissionFX_CompletesLevelSpecial
		// which takes you to a hidden level for power-ups?

		MissionFX_FailsLevel,

		NUM_MISSION_EFFECTS
	}; // suggested naming: MissionFX

	enum MissionObjectiveState
	{
		MOS_JustAHint = 0,
		MOS_HiddenUnachieved,
		MOS_HiddenUnachievedNotPossible,
		MOS_VisibleUnachieved,
		MOS_VisibleUnachievedNotPossible,
		MOS_VisibleAndAchieved,

		NUM_MOS
	};

	class MissionHint
	{
	// Friends

	// Public methods:
	public:
		MissionHint
		(
			enum TEXTSTRING_ID I_TextString_Description,
			OurBool bVisible_New
		);

		SCString* GetDesc(void) const;

		OurBool bVisible(void) const;

		void SetVisibility
		(
			OurBool bVisible_New
		);

		static const List<MissionHint*>& GetAll(void);
		
		
	// Protected methods:
	protected:

	// Private methods:
	private:

	// Private data:
	private:
		enum TEXTSTRING_ID I_TextString_Description_Val;
		OurBool bVisible_Val;

		static List<MissionHint*> List_pMissionHint;
	public:
		virtual ~MissionHint();
	};
	// Inline methods:
		inline SCString* MissionHint::GetDesc(void) const
		{
			return &StringTable :: GetSCString
			(
				I_TextString_Description_Val
			);
		}
		inline OurBool MissionHint::bVisible(void) const
		{
			return bVisible_Val;
		}
		inline void MissionHint::SetVisibility
		(
			OurBool bVisible_New
		)
		{
			bVisible_Val = bVisible_New;
		}
		inline /*static*/ const List<MissionHint*>& MissionHint::GetAll(void)
		{
			return List_pMissionHint;
		}


	#if 0
	class MissionEvent
	{
	public:
		virtual void OnTriggering(void) = 0;

		virtual ~MissionEvent();

	protected:
		MissionEvent
		(
			enum TEXTSTRING_ID I_TextString_TriggeringFeedback,
			enum MissionEffects MissionFX
		);

	protected:
		enum MissionEffects MissionFX_Val;

	private:
		enum TEXTSTRING_ID I_TextString_TriggeringFeedback_Val;

		static List<MissionEvent*> List_pMissionEvent;
	};
	#endif
	
	class MissionObjective;
	#define MissionAlteration_MakeVisible  0x00000001
	#define MissionAlteration_MakePossible  0x00000002
	//when a mission is achieved it can alter the visibility and doability of other mission objectives
	struct MissionAlteration 
	{
		MissionObjective* mission_objective;
		int alteration_to_mission;
	};

	class MissionObjective
	{
	public:
		void OnTriggering(void);

		MissionObjective
		(
			enum TEXTSTRING_ID I_TextString_Description,
			enum MissionObjectiveState MOS,

			enum TEXTSTRING_ID I_TextString_TriggeringFeedback,
			enum MissionEffects MissionFX
		);

		

		static void TestCompleteNext(void);

		int bAchieved(void) const;

		void AddMissionAlteration(MissionObjective* mission_objective,int alteration_to_mission);

		void MakeVisible();
		void MakePossible();
		void ResetMission(); //restores objective to start of level state

		MissionObjectiveState GetMOS() {return MOS_Val;};
		void SetMOS_Public(MissionObjectiveState MOS_New) {SetMOS(MOS_New);} ;

	private:
		void SetMOS( MissionObjectiveState MOS_New  );

	private:
		#if WithinTheGame
		MissionHint aMissionHint_Incomplete;
		MissionHint aMissionHint_Complete;
		#endif

		enum TEXTSTRING_ID I_TextString_Description_Val;

		enum TEXTSTRING_ID I_TextString_TriggeringFeedback_Val;
		enum MissionEffects MissionFX_Val;

		enum MissionObjectiveState MOS_Val;
		enum MissionObjectiveState initial_MOS_Val;


		static List<MissionObjective*> List_pMissionObjective;

		List<MissionAlteration*> List_pMissionAlteration;

	public:
		int bAchievable(void) const
		{
			return ( MOS_Val != MOS_HiddenUnachievedNotPossible  &&
					 MOS_Val != MOS_VisibleUnachievedNotPossible);
		}

	private:
		static int bVisible
		(
			enum MissionObjectiveState MOS_In
		)
		{
			return ( MOS_In != MOS_HiddenUnachieved  &&
					 MOS_In != MOS_HiddenUnachievedNotPossible);
		}

		static int bComplete
		(
			enum MissionObjectiveState MOS_In
		)
		{
			return ( MOS_In == MOS_VisibleAndAchieved );
		}
	public:
		~MissionObjective();
	};

		inline int MissionObjective::bAchieved(void) const
		{
			return bComplete( MOS_Val );
		}
		


/* Exported globals *****************************************************/

/* Function prototypes **************************************************/
	namespace MissionHacks
	{
		void TestInit(void);
	};



/* End of the header ****************************************************/


#ifdef __cplusplus
	};
#endif

#endif

