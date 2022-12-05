/*******************************************************************
 *
 *    DESCRIPTION: 	missions.cpp
 *
 *    AUTHOR: David Malcolm
 *
 *    HISTORY:  Created 5/1/98
 *
 *******************************************************************/

/* Includes ********************************************************/
#include "3dc.h"
#include "module.h"
#include "inline.h"
#include "stratdef.h"
#include "gamedef.h"
#include "missions.hpp"
#include "gadget.h"

	#define UseLocalAssert Yes
	#include "ourasert.h"

extern "C"
{
	#include "paintball.h"
	extern PAINTBALLMODE PaintBallMode;
	extern void MessageHistory_Add(enum TEXTSTRING_ID stringID);
};

/* Version settings ************************************************/

/* Constants *******************************************************/

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
#ifdef __cplusplus
	};
#endif



/* Exported globals ************************************************/
	/*static*/ List<MissionHint*> MissionHint :: List_pMissionHint;
	/*static*/ List<MissionObjective*> MissionObjective :: List_pMissionObjective;

/* Internal type definitions ***************************************/

/* Internal function prototypes ************************************/

/* Internal globals ************************************************/

/* Exported function definitions ***********************************/

// class MissionHint
// Friends
// Public methods:
MissionHint :: MissionHint
(
	enum TEXTSTRING_ID I_TextString_Description,
	OurBool bVisible_New
) : I_TextString_Description_Val
	(
		I_TextString_Description
	),
	bVisible_Val( bVisible_New )
{
	List_pMissionHint . add_entry(this);
}

MissionHint :: ~MissionHint()
{
	List_pMissionHint . delete_entry(this);
}


// Protected methods:

// Private methods:

#if 0
// class MissionEvent
// public:
// protected:
MissionEvent :: MissionEvent
(
	enum TEXTSTRING_ID I_TextString_TriggeringFeedback,
	enum MissionEffects MissionFX
) : I_TextString_TriggeringFeedback_Val
	(
		I_TextString_TriggeringFeedback
	),
	MissionFX_Val( MissionFX )
{
	List_pMissionEvent . add_entry(this);
}

MissionEvent :: ~MissionEvent()
{
	List_pMissionEvent . delete_entry(this);	
}

// private:
#endif


// class MissionObjective
// public:
extern "C"
{
//function for triggering mission objective that can be called from a c file
void MissionObjectiveTriggered(void* mission_objective)
{
	GLOBALASSERT(mission_objective);
	((MissionObjective*)mission_objective)->OnTriggering();
}
void MakeMissionPossible(void* mission_objective)
{
	GLOBALASSERT(mission_objective);
	((MissionObjective*)mission_objective)->MakePossible();
}
void MakeMissionVisible(void* mission_objective)
{
	GLOBALASSERT(mission_objective);
	((MissionObjective*)mission_objective)->MakeVisible();
}

void ResetMission(void* mission_objective)
{
	GLOBALASSERT(mission_objective);
	((MissionObjective*)mission_objective)->ResetMission();
}

int GetMissionStateForSave(void* mission_objective)
{
	GLOBALASSERT(mission_objective);

	return  (int) ((MissionObjective*)mission_objective)->GetMOS();
}

void SetMissionStateFromLoad(void* mission_objective,int state)
{
	GLOBALASSERT(mission_objective);
	((MissionObjective*)mission_objective)->SetMOS_Public((MissionObjectiveState)state);
}

void PrintStringTableEntryInConsole(enum TEXTSTRING_ID string_id)
{
	#if UseGadgets
	SCString* pSCString = &StringTable :: GetSCString
	(
		string_id
	);
	pSCString -> SendToScreen();
	pSCString -> R_Release();
	#endif // UseGadgets
	/* KJL 99/2/5 - play 'incoming message' sound */
	switch(AvP.PlayerType)
	{
		case I_Marine:
		{
			Sound_Play(SID_CONSOLE_MARINEMESSAGE,NULL);
			break;
		}
       	case I_Predator:
		{
			Sound_Play(SID_CONSOLE_PREDATORMESSAGE,NULL);
			break;
		}
		case I_Alien:
		{
			Sound_Play(SID_CONSOLE_ALIENMESSAGE,NULL);
			break;
		}
		default:
			break;
	}
	// add to message history
	MessageHistory_Add(string_id);
}
};

void MissionObjective :: OnTriggering(void)
{
	textprint
	(
		"MissionObjective :: OnTriggering() called\n"
	);

	if(!bAchievable()) return;

	if ( !bAchieved() )
	{
		// Update hint visibilities:
		{
			SetMOS( MOS_VisibleAndAchieved );
		}

		// Send triggering message to the screen:
		if (I_TextString_TriggeringFeedback_Val)
		{
			#if UseGadgets
			SCString* pSCString_Feedback = &StringTable :: GetSCString
			(
				I_TextString_TriggeringFeedback_Val
			);
			pSCString_Feedback -> SendToScreen();
			pSCString_Feedback -> R_Release();
			#endif // UseGadgets
		}

		// Any further effects? e.g. next objective appears?
		{
			switch ( MissionFX_Val )
			{
				case MissionFX_None:
					// Do nothing
					break;

				case MissionFX_UncoversNext:
					// Reveal next objective in list...
					{
						MissionObjective* pNext =
						(
							List_pMissionObjective . next_entry(this)
						);

						if ( pNext )
						{
							pNext->MakeVisible();
						}

					}
					break;

				case MissionFX_CompletesLevel:
				{
					//complete level unless we are in paintball mode
					if(!PaintBallMode.IsOn)
					{
						AvP.LevelCompleted = 1;
			  //		AvP.MainLoopRunning = 0;
					}
					break;
				}
				case MissionFX_FailsLevel:
					{
						// unwritten
					}
					break;

				default:
					GLOBALASSERT(0);
					break;
			}
		}
		//go through the list of mission alterations
		for(LIF<MissionAlteration*> ma_lif(&List_pMissionAlteration);!ma_lif.done();ma_lif.next())
		{
			if(ma_lif()->alteration_to_mission & MissionAlteration_MakeVisible)
			{
				ma_lif()->mission_objective->MakeVisible();
				
			}
			if(ma_lif()->alteration_to_mission & MissionAlteration_MakePossible)
			{
				ma_lif()->mission_objective->MakePossible();
			}
		}

	}
	// otherwise already achieved; ignore
}

void MissionObjective :: MakeVisible()
{
	BOOL has_become_visible=FALSE;
	switch(MOS_Val)
	{
		default:
			GLOBALASSERT(0);
			break;
		case MOS_JustAHint:
			break;
		case MOS_HiddenUnachieved:
			SetMOS( MOS_VisibleUnachieved  );
			has_become_visible=TRUE;
			break;
		case MOS_HiddenUnachievedNotPossible:
			SetMOS( MOS_VisibleUnachievedNotPossible );
			has_become_visible=TRUE;
			break;
		case MOS_VisibleUnachieved:
			break;
		case MOS_VisibleUnachievedNotPossible:
			break;
		case MOS_VisibleAndAchieved:
			break;
	
	}

	if(has_become_visible)//send objective text to console
	{
		{
			#if UseGadgets
			SCString* pSCString_Description = &StringTable :: GetSCString
			(
				 I_TextString_Description_Val
			);

			pSCString_Description -> SendToScreen();

			pSCString_Description -> R_Release();
			#endif // UseGadgets
		}
	}
}

void MissionObjective :: MakePossible()
{
	switch(MOS_Val)
	{
		default:
			GLOBALASSERT(0);
			break;
		case MOS_JustAHint:
			break;
		case MOS_HiddenUnachieved:
			break;
		case MOS_HiddenUnachievedNotPossible:
			SetMOS( MOS_HiddenUnachieved);
			break;
		case MOS_VisibleUnachieved:
			break;
		case MOS_VisibleUnachievedNotPossible:
			SetMOS( MOS_VisibleUnachieved);
			break;
		case MOS_VisibleAndAchieved:
			break;
	
	}
}

MissionObjective :: MissionObjective
(
	enum TEXTSTRING_ID I_TextString_Description,
	enum MissionObjectiveState MOS_New,

	enum TEXTSTRING_ID I_TextString_TriggeringFeedback,
	enum MissionEffects MissionFX

) : aMissionHint_Incomplete
	(
		I_TextString_Description, // enum TEXTSTRING_ID I_TextString_Description,
		(
			bVisible( MOS_New )
			&&
			!bComplete( MOS_New )
		) // OurBool bVisible_New
	),
	aMissionHint_Complete
	(
		I_TextString_TriggeringFeedback, // enum TEXTSTRING_ID I_TextString_Description,
		(
			bVisible( MOS_New )
			&&
			bComplete( MOS_New )
		) // OurBool bVisible_New
	),
	I_TextString_Description_Val( I_TextString_Description ),
	I_TextString_TriggeringFeedback_Val( I_TextString_TriggeringFeedback ),
	MissionFX_Val( MissionFX ),
	MOS_Val( MOS_New ),
	initial_MOS_Val( MOS_New )
{
	List_pMissionObjective . add_entry(this);
}

MissionObjective :: ~MissionObjective()
{
	List_pMissionObjective . delete_entry(this);

	while(List_pMissionAlteration.size())
	{
		delete List_pMissionAlteration.first_entry();
		List_pMissionAlteration.delete_first_entry();
	}
}

// private:
void MissionObjective :: SetMOS( MissionObjectiveState MOS_New  )
{
	if ( MOS_Val != MOS_New )
	{
		MOS_Val = MOS_New;

		aMissionHint_Incomplete . SetVisibility
		(
			bVisible( MOS_New )
			&&
			!bComplete( MOS_New )
		);

		aMissionHint_Complete . SetVisibility
		(
			bVisible( MOS_New )
			&&
			bComplete( MOS_New )
		);
	}
}

void MissionObjective::AddMissionAlteration(MissionObjective* mission_objective,int alteration_to_mission)
{
	GLOBALASSERT(mission_objective);
	MissionAlteration* ma=new MissionAlteration;
	ma->mission_objective=mission_objective;
	ma->alteration_to_mission=alteration_to_mission;
	List_pMissionAlteration.add_entry(ma);
}

void MissionObjective::ResetMission()
{
	MOS_Val=initial_MOS_Val;
}

// Suggested mission objectives for GENSHD1:
void MissionHacks :: TestInit(void)
{
	#if 0
	#if 1
	new MissionHint
	(
		TEXTSTRING_LEVELMSG_001, // ProjChar* pProjCh_Description,
		Yes // OurBool bVisible
	);
	#endif

	new MissionObjective
	(
		TEXTSTRING_LEVELMSG_002,  // ProjChar* pProjCh_Description,
		MOS_VisibleUnachieved, // enum MissionObjectiveState MOS_New,
	
		TEXTSTRING_LEVELMSG_003, // ProjChar* pProjCh_TriggeringFeedback,
		MissionFX_UncoversNext // enum MissionEffects MissionFX,
	);

	new MissionObjective
	(
		TEXTSTRING_LEVELMSG_004, // ProjChar* pProjCh_Description,
		MOS_HiddenUnachieved, // enum MissionObjectiveState MOS_New,

		TEXTSTRING_LEVELMSG_005, // ProjChar* pProjCh_TriggeringFeedback,
		MissionFX_UncoversNext // enum MissionEffects MissionFX,
	);

	new MissionObjective
	(
		TEXTSTRING_LEVELMSG_006,  // ProjChar* pProjCh_Description,
		MOS_HiddenUnachieved, // enum MissionObjectiveState MOS_New,

		TEXTSTRING_LEVELMSG_007, // ProjChar* pProjCh_TriggeringFeedback,
		MissionFX_UncoversNext // enum MissionEffects MissionFX,
	);

	new MissionObjective
	(
		TEXTSTRING_LEVELMSG_008,  // ProjChar* pProjCh_Description,
		MOS_HiddenUnachieved, // enum MissionObjectiveState MOS_New,

		TEXTSTRING_LEVELMSG_009, // ProjChar* pProjCh_TriggeringFeedback,
		MissionFX_UncoversNext // enum MissionEffects MissionFX,
	);

	new MissionObjective
	(
		TEXTSTRING_LEVELMSG_010,  // ProjChar* pProjCh_Description,
		MOS_HiddenUnachieved, // enum MissionObjectiveState MOS_New,

		TEXTSTRING_LEVELMSG_011, // ProjChar* pProjCh_TriggeringFeedback,
		MissionFX_CompletesLevel // enum MissionEffects MissionFX,
	);
	#endif
}

void MissionObjective :: TestCompleteNext(void)
{
	// Do it:
	{
		MissionObjective* pMissionObjective = NULL;

		for
		(
			LIF<MissionObjective*> oi(&List_pMissionObjective);
			!oi.done();
			oi.next()
		)
		{
			if
			(
				!oi() -> bAchieved()
			)
			{
				pMissionObjective = oi();
				break;
			}
		}

		if ( pMissionObjective )
		{
			GLOBALASSERT( pMissionObjective );

			// Feedback:
			{
				#if UseGadgets

				SCString* pSCString_Temp = new SCString("TESTING MISSION COMPLETION HOOK:");

				pSCString_Temp -> SendToScreen();

				pSCString_Temp -> R_Release();
				#endif // UseGadgets

			}

			pMissionObjective -> OnTriggering();			
		}
		else
		{
			// Feedback:
			{
				#if UseGadgets

				SCString* pSCString_Temp = new SCString("NO INCOMPLETE OBJECTIVES");

				pSCString_Temp -> SendToScreen();

				pSCString_Temp -> R_Release();
				#endif // UseGadgets
			}
		}
	}
	
}



/* Internal function definitions ***********************************/







#if 0
		"BACKUP POWER SYSTEM ACTIVATED", // ProjChar* pProjCh_TriggeringFeedback,
		MissionFX_None // enum MissionEffects MissionFX,
	);

	new MissionObjective
	(
		"SECURITY DOORS RESTRICT ACCESS WITHIN THE COLONY.  FIND THE OPERATIONS ROOM IN THE MAIN BUILDING.  "
		"INSIDE ARE FIVE SWITCHES.  TRIGGER ALL OF THEM TO OPEN THE SECURITY DOORS REMOTELY.  "
			, // ProjChar* pProjCh_Description,
		Yes, // OurBool bHidden

		"SECURITY DOOR EMERGENCY OVERRIDE TRIGGERED:  DOORS HAVE BEEN OPENED.  ", // ProjChar* pProjCh_TriggeringFeedback,
		MissionFX_None // enum MissionEffects MissionFX,
	);

	new MissionObjective
	(
		"MAKE YOUR WAY TO MEDLAB.  COLLECT COMPUTER ARCHIVES "
		"DOCUMENTING THE COLONISTS WORK ON THE FACEHUGGERS.  "
			,  // ProjChar* pProjCh_Description,
		Yes, // OurBool bHidden

		"PARTIAL MEDLAB ARCHIVE COLLECTED", // ProjChar* pProjCh_TriggeringFeedback,
		MissionFX_None // enum MissionEffects MissionFX,
	);

	new MissionObjective
	(
		"THE MEDLAB ARCHIVE YOU HAVE COLLECTED IS ONLY ONE OF THREE FILES.  "
		"WE'VE GOT A PDT READING ON THE MEDLAB OFFICER WHO WAS RESEARCHING THE FACEHUGGERS.  "
		"IT'S COMING FROM THE BASEMENT OF THE PROCESSOR PLANT.  "
		"SHE MIGHT BE ALIVE, BUT IT'S PROBABLY JUST HER REMAINS.  "
		"MAKE YOUR WAY THERE AND TRY TO FIND MORE ARCHIVES.  "
			,  // ProjChar* pProjCh_Description,
		Yes, // OurBool bHidden

		"MEDLAB ARCHIVE COLLECTED", // ProjChar* pProjCh_TriggeringFeedback,
		MissionFX_None // enum MissionEffects MissionFX,
	);

	new MissionObjective
	(
		"THAT LOOKS LIKE ALL THE FILES.  "
		"GET BACK TO THE YARD.  THE LANDING BEACON CONTROLS ARE IN A RECESS TO THE LEFT OF THE "
		"ATMOSPHERE PROCESSOR.  ACTIVATE THE BEACON AND PREPARE FOR EVAC."
			,  // ProjChar* pProjCh_Description,
		Yes, // OurBool bHidden

		"LANDING BEACON ACTIVATED.  STAND CLEAR AND PREPARE FOR EVAC.  ", // ProjChar* pProjCh_TriggeringFeedback,
#endif
