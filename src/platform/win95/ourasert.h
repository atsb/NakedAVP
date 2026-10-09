
/*
	This is our assert file for the Win95
	platform, with Dave's global/local assert
	distinctions.
*/

/* 
	Note that WaitForReturn now calls FlushTextprintBuffer
	and FlipBuffers implicitly.
*/

/*
	Modified 10th December 1996 by Dave Malcolm.  Now can be set so that
	functions are supplied by the project/platform to fire when an assertion
	fires.

	Also is set so that the compiler will generate an error message if you manage to
	include the file more than once (with confusing definitons of UseLocalAssert);
	this can be disabled.
*/


#ifdef _OURASERT
	#if StopCompilationOnMultipleInclusions
		#error OURASERT.H included more than once
	#endif
#else
	#define _OURASERT	1
#endif


#ifdef AVP_DEBUG_VERSION
	#define ASSERT_SYSTEM_ON 1
#else
	#define ASSERT_SYSTEM_ON 0
#endif


#if UseProjPlatAssert
/* New assertions system */

	#ifdef __cplusplus
	extern "C" {
	#endif
		int GlobalAssertFired(char* Filename, int LineNum, char* Condition);
		int LocalAssertFired(char* Filename, int LineNum, char* Condition);
		void ExitFired(char* Filename, int LineNum, int ExitCode);
	#ifdef __cplusplus
	};
	#endif


	#if ASSERT_SYSTEM_ON

		#define GLOBALASSERT(x) 		\
		 	(void)( (x) ? 1 : 			\
			    (						\
					GlobalAssertFired	\
					(					\
						__FILE__,		\
						__LINE__,		\
						#x	 			\
					)					\
				)						\
			)

	    #if UseLocalAssert

		   #define LOCALASSERT(x) \
		 	(void)( (x) ? 1 : 			\
			    (						\
					LocalAssertFired	\
					(					\
						__FILE__,		\
						__LINE__,		\
						#x				\
					)					\
				)						\
			)

	    #else

	       #define LOCALASSERT(ignore)

	    #endif


		#define exit(x) ExitFired(__FILE__,__LINE__,x)

	#else
	   
	    #define GLOBALASSERT(ignore) ((void)0)

	    #define LOCALASSERT(ignore) ((void)0)

	#endif
	

#else
/* Old assertions system */

	#define GlobalAssertCode 0xffff
	#define LocalAssertCode  0xfffe


	#if 0//debug

		#define GLOBALASSERT(x) \
		     (void)((x) ? 1 : \
			    (textprint("\nGAF " #x "\nLINE %d\nFILE'%s'\n", \
				__LINE__, __FILE__), WaitForReturn(), \
				ExitSystem(), exit(GlobalAssertCode), \
				0))

	    #if UseLocalAssert

		   #define LOCALASSERT(x) \
		        (void)((x) ? 1 : \
			       (textprint("\nLAF " #x "LINE %d\nFILE'%s'\n", \
				   __LINE__, __FILE__), WaitForReturn(), \
				   ExitSystem(), exit(LocalAssertCode), \
				   0))

	    #else

	       #define LOCALASSERT(ignore)

	    #endif

	#else
	   
	    #define GLOBALASSERT(ignore)

	    #define LOCALASSERT(ignore)

	#endif
#endif
