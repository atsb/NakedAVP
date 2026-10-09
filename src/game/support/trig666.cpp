/*******************************************************************
 *
 *    DESCRIPTION: 	trig666.cpp
 *
 *    AUTHOR: David Malcolm
 *
 *    HISTORY:  Created 18/11/97 from Headhunter's TRIGGERS.CPP; had to
 * 				rename to avoid conflict with AVP file TRIGGERS.H
 *
 *******************************************************************/

/* Includes ********************************************************/
#include "3dc.h"
#include "trig666.hpp"
	
	#define UseLocalAssert Yes
	#include "ourasert.h"

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
		extern IFOBJECTLOCATION IFObjLoc_Origin;
		extern UncompressedGlobalPlotAtomID UGPAID_StandardNull;
		extern IFCOLOUR			IFColour_Dummy;
 		extern IFVECTOR			IFVec_Zero;
		#endif
#ifdef __cplusplus
	};
#endif



/* Exported globals ************************************************/

/* Internal type definitions ***************************************/

/* Internal function prototypes ************************************/

/* Internal globals ************************************************/

/* Exported function definitions ***********************************/
// class TriggerDaemon : public Daemon
TriggerDaemon :: TriggerDaemon
(
	OurBool fActive
) : Daemon
	(
		fActive
	)
{
}

TriggerDaemon :: ~TriggerDaemon()
{
}

// A daemon which fires at regular intervals (potentially more than once per frame)
// class PulsingTriggerDaemon : public TriggerDaemon
PulsingTriggerDaemon :: PulsingTriggerDaemon
(
	OurBool fActive,
	int FixP_Period // interval between triggers in seconds			
) : TriggerDaemon
	(
		fActive
	)
{
	GLOBALASSERT( FixP_Period > 0);

	FixP_Period_Val = FixP_Period;
	FixP_TimeToNextPulse = FixP_Period;	
}

PulsingTriggerDaemon :: ~PulsingTriggerDaemon()
{
}

ACTIVITY_RETURN_TYPE PulsingTriggerDaemon :: Activity(ACTIVITY_INPUT)
{
	while ( FixP_Time > 0 )
	{
		if ( FixP_Time >= FixP_TimeToNextPulse )
		{
			// then elapse some of the available time to take you to the pulse
			FixP_Time -= FixP_TimeToNextPulse;
			FixP_TimeToNextPulse = FixP_Period_Val;

			// and trigger:
			Triggered();
		}
		else
		{
			// Not enough time to warrant triggering; reduce time
			// to next pulse and stop.
			FixP_TimeToNextPulse -= FixP_Time;
			ACTIVITY_RVAL_NOCHANGE
		}
	}

	ACTIVITY_RVAL_NOCHANGE
}

void PulsingTriggerDaemon :: SetFuse_FixP
(
	int FixP_Fuse // time until it next triggers; doesn't change the period
)
{
	GLOBALASSERT( FixP_Fuse > 0 );
	FixP_TimeToNextPulse = FixP_Fuse;
}

// A countdown daemon which DESTROYS ITSELF after it triggers
// class CountdownDaemon : public TriggerDaemon
CountdownDaemon :: CountdownDaemon
(
	OurBool fActive,
	int FixP_Fuse // time until it triggers
) : TriggerDaemon
	(
		fActive
	)
{
	GLOBALASSERT( FixP_Fuse > 0 );

	FixP_TimeRemaining = FixP_Fuse;
}

CountdownDaemon :: ~CountdownDaemon()
{
}

ACTIVITY_RETURN_TYPE CountdownDaemon :: Activity(ACTIVITY_INPUT)
{
	#if 0
	textprint
	(
		"CountdownDaemon :: Activity(%i) with fuse %i\n",
		 FixP_Time,
		 FixP_TimeRemaining
	);
	#endif

	if ( FixP_TimeRemaining > FixP_Time )
	{
		// Keep counting down:
		FixP_TimeRemaining -= FixP_Time;
	}	
	else
	{
		// Countdown has elapsed:
		Triggered();
		delete this;
	}

	ACTIVITY_RVAL_NOCHANGE
}

void CountdownDaemon :: SetFuse_FixP
(
	int FixP_Fuse // time until it triggers
)
{
	FixP_TimeRemaining = FixP_Fuse;
}

/* Internal function definitions ***********************************/
