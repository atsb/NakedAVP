/*******************************************************************
 *
 *    DESCRIPTION: Daemon code - things that need to be updated on a
 *		per-frame basis provided they are "active"
 *    AUTHOR: David Malcolm
 *
 *    HISTORY:    
 *
 *******************************************************************/

/* Includes ********************************************************/
#include "3dc.h"
#include "daemon.h"

#include "inline.h"

#if SupportCallbackHooks
#include "scrobj.hpp"
#endif
	
	#define UseLocalAssert Yes
	#include "ourasert.h"

/* Constants *******************************************************/
	#define UseRealFrameTime	Yes

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
	#endif
#ifdef __cplusplus
	};
#endif


/* Internal function prototypes ************************************/

/* Exported globals ************************************************/
	/*static*/ int Daemon :: DaemonTimeScale = ONE_FIXED;

	#if !IndividualTiming
		/*static*/ int Daemon :: FixP_Time = 0;
	#endif

/* Internal globals ************************************************/
// static
Daemon* Daemon :: p666_FirstActive = NULL;
// static
Daemon* Daemon :: p666_Iteration_Current = NULL;
// static
Daemon* Daemon :: p666_Iteration_Next = NULL;

/* Exported function definitions ***********************************/
#if SupportCallbackHooks
// class CallbackHook
CallbackHook :: CallbackHook
(
	Daemon* p666_New,
	void* pUser_New
)
{
	if ( p666_New -> pFirstHook )
	{
		GLOBALASSERT( p666_New -> pFirstHook -> pPrvHook == NULL);
		p666_New -> pFirstHook -> pPrvHook = this;
	}

	pNxtHook = p666_New -> pFirstHook;

	p666_New -> pFirstHook = this;	

	pPrvHook = NULL;

	p666_Val = p666_New;
	pUser_Val = pUser_New;
}

CallbackHook :: ~CallbackHook()
{
	// Remove from list:
	if ( pPrvHook )
	{
		#if debug
		if ( p666_Val )
		{
			GLOBALASSERT( this != p666_Val -> pFirstHook );
		}
		#endif
		pPrvHook -> pNxtHook = pNxtHook;
	}
	else
	{
		if ( p666_Val )
		{
			// this was it's daemon's first	callback:
			GLOBALASSERT( this == p666_Val -> pFirstHook );
			p666_Val -> pFirstHook = pNxtHook;		
		}
	}

	if ( pNxtHook )
	{
		pNxtHook -> pPrvHook = pPrvHook;
	}

}
#endif // SupportCallbackHooks

// class Daemon
Daemon :: Daemon
(
	OurBool fActive
)
{
	#if SupportCallbackHooks
	pFirstHook= NULL;
	#endif
	
	fIsActive_Val = No;

	if (fActive)
	{
		Start();
	}
	else
	{
		p666_NextActive = NULL;
		p666_PrevActive = NULL;
	}
}

Daemon :: ~Daemon()
{
	if (fIsActive_Val)	
	{
		Stop();
	}

    if ( p666_Iteration_Current == this )
    {
        // Then this daemon is being processed for Activity();
        // Set the static iteration ptr to NULL to signify it has been deleted
        // so that callback hooks don't get called
        p666_Iteration_Current = NULL;
    }

	#if SupportCallbackHooks
	// remove from screen objects lists of attached daemons
	while ( pFirstHook )
	{
		pFirstHook -> p666_Val = NULL;
		
		pFirstHook = pFirstHook -> pNxtHook;
	}
	#endif
}


void Daemon :: Start(void)
{
	if (!fIsActive_Val)
	{
		// Insert at front of active 666 list
		p666_PrevActive = NULL;
		p666_NextActive = p666_FirstActive;

		if (p666_FirstActive)
		{
			p666_FirstActive -> p666_PrevActive = this;
		}

		p666_FirstActive = this;

		fIsActive_Val = Yes;
	}
}

void Daemon :: Stop(void)
{
	if (fIsActive_Val)
	{
		// Remove from active 666 list
        {
            // Check against the iteration in the Maintain() static function:
            {
                if ( p666_Iteration_Next == this )
                {
                    // then this is due the next daemon to have its Activity() called;
                    // advance the static iteration ptr to this daemon's next
                    p666_Iteration_Next = p666_NextActive;
                }
            }

    		if ( p666_PrevActive )		
    		{
    			GLOBALASSERT( p666_PrevActive -> fIsActive_Val );

    			p666_PrevActive -> p666_NextActive = p666_NextActive;	
    		} 

    		if ( p666_NextActive )		
    		{
    			GLOBALASSERT( p666_NextActive -> fIsActive_Val );

    			p666_NextActive -> p666_PrevActive = p666_PrevActive;
    		}

    		if (p666_FirstActive == this)
    		{
    			p666_FirstActive = p666_NextActive;
    		}
        }

		fIsActive_Val = No;
	}
}

void Daemon :: SetActive(OurBool fActive)
{
	if (fActive)
	{
		Start();
	}
	else
	{
		Stop();
	}
}

OurBool Daemon :: bActive(void) const
{
	return fIsActive_Val;
}

#if SupportCallbackHooks
void Daemon :: ForceHookActivity(void)
{
	// a way to call the OnActivity() method for all attached hooks

	CallbackHook* pCallbackHook = pFirstHook;
	while ( pCallbackHook )
	{
		CallbackHook* pCallbackHook_Nxt = pCallbackHook -> pNxtHook;
		
		pCallbackHook -> OnActivity();
		
		pCallbackHook = pCallbackHook_Nxt;
	}

}
#endif // SupportCallbackHooks

#if 0
void Daemon :: LinkScreenObject
(
	ScreenObject& ScrObj
)
{
	UNWRITTEN();
}

void Daemon :: UnlinkScreenObject
(
	ScreenObject& ScrObj
)
{
	UNWRITTEN();
}
#endif

// static
void Daemon :: Maintain(int FixP_Time_ToUse)
{
	GLOBALASSERT( NULL == p666_Iteration_Current );
	GLOBALASSERT( NULL == p666_Iteration_Next );

	#if DaemonDiagnostics
	ProfileStart();
	#endif

	p666_Iteration_Current = p666_FirstActive;

	FixP_Time = FixP_Time_ToUse;

	while ( p666_Iteration_Current )
	{
		p666_Iteration_Next = p666_Iteration_Current -> p666_NextActive;

		{
			#if DaemonNaming && DaemonDiagnostics
            char* tempDebugName = p666_Iteration_Current -> GetDebugName();
                // in case it gets deleted during the loop

			ProfileStart();
			#endif

			#if SupportCallbackHooks
			if
			(
				p666_Iteration_Current -> Activity(FixP_Time)
			)
			{
				if ( p666_Iteration_Current )
                {
    				// run the OnActivity() method for all the callback hooks attached to this daemon
    				CallbackHook* pCallbackHook = p666_Iteration_Current -> pFirstHook;
    				while ( pCallbackHook )
    				{
    					CallbackHook* pCallbackHook_Nxt = pCallbackHook -> pNxtHook;
    					
    					pCallbackHook -> OnActivity();
    					
    					pCallbackHook = pCallbackHook_Nxt;
    				}
                }
                // else the iterating daemon got deleted during the call to Activity()
			}
			#else
			{
				#if IndividualTiming
				{
					p666_Iteration_Current -> Activity(FixP_Time);
				}
				#else
				{
					p666_Iteration_Current -> Activity();
				}
				#endif
			}
			#endif

			#if DaemonNaming && DaemonDiagnostics
			ProfileStop
			(
				tempDebugName
			);
			#endif

		}
        /* 
            Advance to the next in the iteration.
            This will be either the next ptr of the current as stored above,
            or one further along the list (since the pNext one itself might
            have got deleted during the call to Activity)
        */
		p666_Iteration_Current = p666_Iteration_Next;
	}

	#if DaemonDiagnostics

	ProfileStop("Daemon :: Maintain()");

	#if CountActiveDaemons
	textprint("Num active daemons:%i\n",GetNumActive());
	#endif

	#endif // #if DaemonDiagnostics

}

void DAEMON_Init(void)
{
}

extern "C" {
#if UseRealFrameTime
extern int RealFrameTime;
#else
extern int NormalFrameTime;
#endif
}

void DAEMON_Maintain(void)
{
	#if 0
	textprint("DAEMON_Maintain() called\n");
	#endif
	
	#if 0
	textprint("RealFrameTime=%i\n",RealFrameTime);
	#endif

	int DaemonFrameTime =
	(
		#if UseRealFrameTime
		RealFrameTime
		#else
		NormalFrameTime
		#endif
	);

	{
		if (Daemon :: DaemonTimeScale!=ONE_FIXED)
		{
			DaemonFrameTime = MUL_FIXED(DaemonFrameTime,Daemon :: DaemonTimeScale);
		}

	}

	/* cap DaemonFrameTime if frame rate is really low */
	if (DaemonFrameTime>32768) DaemonFrameTime=32768;

	Daemon :: Maintain
	(
		DaemonFrameTime
	);
}


/* Internal function definitions ***********************************/
