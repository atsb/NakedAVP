/*
	
	trig666.hpp

*/

#ifndef _trig666
#define _trig666 1

	#ifndef _daemon
	#include "daemon.h"
	#endif

/* Version settings *****************************************************/

/* Macros ***************************************************************/

/* Type definitions *****************************************************/
	class TriggerDaemon : public Daemon
	{
	public:
		TriggerDaemon
		(
			OurBool fActive
		);
		~TriggerDaemon();
		virtual void Triggered(void) = 0;
			// called by the daemon's activity whenever the daemon decides to
			// trigger it.
	private:
				
	};

	// A daemon which fires at regular intervals (potentially more than once per frame)
	class PulsingTriggerDaemon : public TriggerDaemon
	{
	public:
		PulsingTriggerDaemon
		(
			OurBool fActive,
			int FixP_Period // interval between triggers in seconds			
		);

		~PulsingTriggerDaemon();

		ACTIVITY_RETURN_TYPE Activity(ACTIVITY_INPUT);		
			// never causes callback hooks to fire

		void SetFuse_FixP
		(
			int FixP_Fuse // time until it next triggers; doesn't change the period
		);

		// void Triggered(void) remains pure virtual
	private:
		int FixP_Period_Val;
		int FixP_TimeToNextPulse;
	};

	// A countdown daemon which DESTROYS ITSELF after it triggers
	class CountdownDaemon : public TriggerDaemon
	{
	public:
		CountdownDaemon
		(
			OurBool fActive,
			int FixP_Fuse // time until it triggers
		);

		~CountdownDaemon();

		ACTIVITY_RETURN_TYPE Activity(ACTIVITY_INPUT);
			// never causes callback hooks to fire
		
		void SetFuse_FixP
		(
			int FixP_Fuse // time until it triggers
		);

		// void Triggered(void) remains pure virtual
	private:
		int FixP_TimeRemaining;
	};
		

/* Exported globals *****************************************************/

/* Function prototypes **************************************************/



/* End of the header ****************************************************/

#endif
