/*
	
	REFLIST.HPP

	Created 27/1/98 by DHM:
	-----------------------

	Template for managing lists of pointers to reference-counted objects.

	The functions R_AddRef() and R_Release() must be defined for the type parameter.

	Uses the list template LIST_TEM.HPP

	The access functions return pointers to objects, or NULL for failure.  The rule
	is that:

		Read()
		------
	just returns a pointer; the caller should R_AddRef() only if they want to store the pointer
	somewhere (like the COM out-parameter rule; see p81 of "Inside COM")

		GetYour()
		---------
	destructively reads the pointer from the list; ownership of the reference is transferred
	to the caller who _must_ call R_Release() at some subsequent point.	

*/

#ifndef _reflist_hpp
#define _reflist_hpp 1

	#if defined( _MSC_VER )
		#pragma once
	#endif

	#ifndef list_template_hpp
	#include "list_tem.hpp"
	#endif

	#ifndef _refobj
	#include "refobj.hpp"
	#endif

extern char const* reflist_fail_destructor;


/* Type definitions *****************************************************/

// nb templates cannot have C linkage
template <class RC> class RefList
{
private:
	List<RC*> List_pRC;
	
public:
	// {{{ Constructors:
	RefList() : List_pRC()
	{
	}
	// }}}

	// {{{ Destructor
	~RefList()
	{
		EmptyYourself();
	}
	// }}}

	// {{{
	int NumEntries(void) const
	{
		return List_pRC . size();
	}
	// }}}

	// {{{ Inserting new members
	void AddToFront(RC& theRC)
	{
		theRC . R_AddRef();

		List_pRC . add_entry_start( &theRC );
	}
	void AddToEnd(RC& theRC)
	{
		theRC . R_AddRef();

		List_pRC . add_entry_end( &theRC );
	}
	// }}}

	// {{{ Accessing & removing existing members; see notes at top of header
	RC* ReadFirst(void) const
	{
		if (List_pRC . size() >0 )
		{
			return List_pRC . first_entry();
		}
		else
		{
			return NULL;
		}
	}
	RC* ReadFinal(void) const
	{
		if (List_pRC . size() >0 )
		{
			return List_pRC . last_entry();
		}
		else
		{
			return NULL;
		}
	}
	RC* GetYourFirst(void)
	{
		if (List_pRC . size() >0 )
		{
			RC* pReturn = List_pRC . first_entry();

			List_pRC . delete_first_entry();
				// note that a reference is still owned; ownership is transferred to the caller

			return pReturn;
		}
		else
		{
			return NULL;
		}
	}
	RC* GetYourFinal(void)
	{
		if (List_pRC . size() >0 )
		{
			RC* pReturn = List_pRC . last_entry();

			List_pRC . delete_last_entry();
				// note that a reference is still owned; ownership is transferred to the caller

			return pReturn;
		}
		else
		{
			return NULL;
		}
	}
	void EmptyYourself(void)
	{
		// Destroys the list, releasing all refs
		while ( List_pRC . size() > 0 )
		{
			RC* pRC = List_pRC . first_entry();

			List_pRC . delete_first_entry();

			#ifndef NDEBUG
			if ( !pRC )
			{
				fail( reflist_fail_destructor );
			}
			#endif

			pRC -> R_Release();			
		}
	}
	// }}}


}; // end of template <class RC> class RefList


/* End of the header ****************************************************/


#endif
