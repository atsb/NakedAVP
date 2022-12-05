/****

Windows functionality that is definitely
not project specific.

****/

// To link code to main C functions 

extern "C" {

#include "3dc.h"
#include "inline.h"

// For modifications necessary to make Alt-Tabbing
// behaviour (WM_ACTIVATEAPP) work full screen.
// This is necessary to support full screen
// ActiveMovie play.

#define SupportAltTab Yes

// Globals

static HANDLE RasterThread;

// Externs

extern BOOL bActive;

// These function are here solely to provide a clean
// interface layer, since Win32 include files are fully
// available in both C and C++.
// All functions linking to standard windows code are
// in win_func.cpp or win_proj.cpp, and all DirectX 
// interface functions
// should be in dd_func.cpp (in the Win95 directory)
// or d3_func.cpp, dp_func.cpp, ds_func.cpp etc.
// Project specific platfrom functionality for Win95
// should be in project/win95, in files called 
// dd_proj.cpp etc.


// GetTickCount is the standard windows return
// millisecond time function, which isn't actually
// accurate to a millisecond.  In order to get FRI
// to work properly with GetTickCount at high frame 
// rates, you will have to switch KalmanTimer to Yes
// at the start of io.c to turn on a filtering algorithm
// in the frame counter handler.  
// Alternately, we can use the mm function 
// timeGetTime to get the time accurate to a millisecond.
// There is still enough variation in this to make
// the kalman filter probably worthwhile, however.

long GetWindowsTickCount(void)

{
    #if 0
	return GetTickCount();
	#else
	return timeGetTime();
	#endif
}

// This function is set up using a PeekMessage check,
// with a return on a failure of GetMessage, on the
// grounds that it might be more stable than just
// GetMessage.  But then again, maybe not.  
// PM_NOREMOVE means do not take this message out of
// the queue.  The while loop is designed to ensure
// that all messages are sent through to the Windows
// Procedure are associated with a maximum of one frame's
// delay in the main engine cycle, ensuring that e.g.
// keydown messages do not build up in the queue.

// if necessary, one could extern this flag
// to determine if a task-switch has occurred which might
// have trashed a static display, to decide whether to
// redraw the screen. After doing so, one should reset
// the flag

BOOL g_bMustRedrawScreen = FALSE;

void CheckForWindowsMessages(void)
{
	MSG         msg;
	extern signed int MouseWheelStatus;
	
	MouseWheelStatus = 0;

	// Initialisation for the current embarassingly primitive mouse 
	// handler...

	do
	{
		while (PeekMessage(&msg, NULL, 0, 0, PM_NOREMOVE))
		{
			if (!GetMessage(&msg, NULL, 0, 0))
				return;

			TranslateMessage(&msg);
			DispatchMessage(&msg);

			#if (!SupportAltTab)
			// Panic
			if (!bActive)
			{
				// Dubious hack...
				#if 0
				ExitSystem();
				#else
				ReleaseDirect3D();
				exit(0x00);
				#endif
			}
			#endif
		}
		
		// JH 13/2/98 - if the app is not active we should not return from the message lopp
		// until the app is re-activated
		
		if (!bActive)
		{
			ResetFrameCounter();
			Sleep(0);
			g_bMustRedrawScreen = TRUE;
		}
	}
		while (!bActive);
}




// Experimental functions to handle a separate
// thread to run rasterisation on hardware at low
// priority.

// Note that the RenderD3DScene function does not need
// to call ExitThread explictly - the return at the
// end of the function will do this for this, giving
// thread exit code equal to the return value from
// the function.

/*
  Note that this assumes DrawPerFrame mode!!!
  necessary for some hardware accelerators anyway
  (deferred texturing problem!!!)
*/

BOOL SpawnRasterThread()

{
	DWORD RasterThreadId;
	// Stack size of new thread in bytes.
	// For the moment, we will set it to
	// 128K, the normal size for the engine
	// process.
	// Note that this is in bytes.
	// Note that stack size should grow as 
	// necessary.  We hope.
	DWORD StackSize = 128 * 1024;


    // Create the thread
    RasterThread = CreateThread(
	   NULL,     // no security
	   StackSize,        // default stack size
	   (LPTHREAD_START_ROUTINE) RenderD3DScene,
	   0,        // no argument for function
	   0,        // default creation flags
	   &RasterThreadId); // get thread ID

    if (RasterThread == NULL)
	  {
       #if debug
	   ReleaseDirect3D();
	   exit(0xabab);
	   #else
	   return FALSE;
       #endif
	  }

    #if 1
	// Set the priority on the thread to
	// below normal, since we want this thread
	// to be unimportant --- it is only monitoring
	// the hardware rasteriser.  Hopefully.
	// Note that this priority value maybe should
	// be THREAD_PRIORITY_LOWEST or THREAD_PRIORITY_IDLE,
	// or maybe we shouldn't call this function at all.
	// Also, we must have a THREAD_SET_INFORMATION
	// access right associated with the thread for this
	// to work.  Hopefully, this should be the default
	// when using CreateThread.
	SetThreadPriority(RasterThread, 
	   THREAD_PRIORITY_NORMAL);
	#endif

	return TRUE;
}

BOOL WaitForRasterThread()

{
    BOOL RetVal;
	DWORD ThreadStatus;
	int i;

    // Note that if this is to work the 
    // rasterisation thread must have a
	// THREAD_QUERY_INFORMATION access right,
	// but we believe CreateThread should supply
	// this as a default.

    // Note!!! At some stage we may want to put a
	// delay loop in the statement below, in the
	// time honoured Saturn fashion, depending on how
	// much impact calling GetExitCodeThread has on the
	// rest of the system - hopefully not much...

    do
	  {
       RetVal = GetExitCodeThread(RasterThread,
	                    &ThreadStatus);
	  }
	while ((RetVal == TRUE) && 
	      (ThreadStatus == STILL_ACTIVE));

    // Failed to get a status report on the thread
	if (RetVal == FALSE)
	  {
	   #if debug
	   ReleaseDirect3D();
	   exit(0xabbb);
	   #else
	   return FALSE;
	   #endif
	  }

	return TRUE;
}


/*
  Pick up processor types,
  either from assembler test (note
  I have asm to do this, but it must 
  be converted from as / Motorola format
  to masm / Intel), or (more likely) from
  a text file left by the launcher, which
  can use GetProcessorType from the 
  mssetup api
*/

#if defined(_MSC_VER)

static unsigned int GetCPUId(void)
{
	unsigned int retval;
	_asm
	{
		mov eax,1
		_emit 0x0f   ; CPUID (00001111 10100010) - This is a Pentium
		             ; specific instruction which gets information on the
		_emit 0xa2   ; processor. A Pentium family processor should set
		             ; bits 11-8 of eax to 5.
		mov retval,edx
	}
	return retval;
}

#else

#error "Unknown compiler"

#endif


PROCESSORTYPES ReadProcessorType(void)
{
	SYSTEM_INFO SystemInfo;
	int ProcessorType;
	PROCESSORTYPES RetVal;

    GetSystemInfo(&SystemInfo);

    ProcessorType = SystemInfo.dwProcessorType;

    switch (ProcessorType)
	  {
	   case PROCESSOR_INTEL_386:
		 RetVal = PType_OffBottomOfScale;
		 break;

	   case PROCESSOR_INTEL_486:
		 RetVal = PType_486;
		 break;

	   case PROCESSOR_INTEL_PENTIUM:
		 if (GetCPUId() & 0x00800000)
		 	RetVal = PType_PentiumMMX;
		 else
		 	RetVal = PType_Pentium;
		 break;

       #if 0
	   case PROCESSOR_INTEL_SOMETHING:
		 RetVal = PType_Klamath;
		 break;
	   #endif

	   default:
	     RetVal = PType_OffTopOfScale;
		 break;
	  }

	return RetVal;
}



// End of extern C declaration 

};




