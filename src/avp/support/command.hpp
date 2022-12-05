/*
	
	command.hpp

	An object for encapsulating requests; see pp233-242 of
 	"Design Patterns"

*/

#ifndef _command
#define _command 1

	#ifndef _refobj
	#include "refobj.hpp"
	#endif
	
#ifdef __cplusplus
	extern "C" {
#endif

/* Version settings *****************************************************/

/* Constants  ***********************************************************/

/* Macros ***************************************************************/

/* Type definitions *****************************************************/
	class Command : public RefCountObject
	{
	public:
		virtual OurBool Execute(void) = 0;
			// return value is "was command completed successfully?"

	protected:
		// Empty constructor:
		Command() : RefCountObject() {}
		
	protected:
		// Protected destructor; Release() is the only method allowed to 
		// delete it...
		virtual ~Command()
		{
		 	// empty
		}
	};
	
/* Exported globals *****************************************************/

/* Function prototypes **************************************************/



/* End of the header ****************************************************/


#ifdef __cplusplus
	};
#endif

#endif
