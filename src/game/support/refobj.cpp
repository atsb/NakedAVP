/*******************************************************************
 *
 *    DESCRIPTION: 	refobj.cpp
 *
 *    AUTHOR: David Malcolm
 *
 *    HISTORY:  Created 15/9/97
 *
 *******************************************************************/

/* Includes ********************************************************/
#include "3dc.h"
#include "refobj.hpp"

	#if TrackReferenceCounted
		#include "dcontext.hpp"

		#ifndef list_template_hpp
		#include "list_tem.hpp"
		#endif
	#endif

	#define UseLocalAssert Yes
	#include "ourasert.h"

/* Version settings ************************************************/
	#if TrackReferenceCounted
		#define OutputRefCountLogOnExit	Yes
	
		
		#if OutputRefCountLogOnExit
			#include "debuglog.hpp"
		#endif


	#endif

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
	char const* refobj_fail_addref = "Failure in R_AddRef()\n";
	char const* refobj_fail_release = "Failure in R_Release()\n";
	char const* refobj_fail_destructor = "Failure in Destructor()\n";

/* Internal type definitions ***************************************/
#if TrackReferenceCounted
class RefCountObject_TrackData
{
public:
	enum transtype
	{
		tt_addref,
		tt_release
	};

private:
	class ReferenceTransaction
	{
	public:
		void Dump( R_DumpContext& theContext ) const;

	private:
		char* Filename;
		int LineNum;
		enum transtype Type;

	protected:
		ReferenceTransaction(char* theFilename, int theLineNum, enum transtype theType) :
			Filename(theFilename),
			LineNum(theLineNum),
			Type(theType)
		{}
	};

	class Transaction_R_AddRef : public ReferenceTransaction
	{
	public:
		Transaction_R_AddRef(char* theFilename, int theLineNum) :
			ReferenceTransaction(theFilename, theLineNum, tt_addref)
		{}
	};

	class Transaction_R_Release : public ReferenceTransaction
	{
	public:
		Transaction_R_Release(char* theFilename, int theLineNum) :
			ReferenceTransaction(theFilename, theLineNum, tt_release)
		{}
	};

public:
	static void DumpAll(R_DumpContext& theContext);

	RefCountObject_TrackData(RefCountObject *const pRCObj, char* theFilename, int theLineNum);
	~RefCountObject_TrackData();

	void Track_R_AddRef(char* theFilename, int theLineNum);
	void Track_R_Release(char* theFilename, int theLineNum);

	void DumpTransactions( R_DumpContext& theContext ) const;

private:
	RefCountObject *const pRCObj_Val;
	char* constructionFilename;
	int constructionLineNum;
	List<ReferenceTransaction*> List_pTransaction;

private:
	/*
		Maintain various global stuff.
		This is all held together as a BSS object so that we can be sure that we call
		the log-on-exit in the destructor before destroying the records of what hasn't
		been fully released.

		We maintain a list of all reference counted objects, which the class RefCountObject
		has indirect access to.
		
	*/
	class Globals
	{
		friend class RefCountObject_TrackData;

	private:
		List<RefCountObject*> List_pRCObj;
		#if OutputRefCountLogOnExit
		char* filename;
		#endif
	public:
		#if OutputRefCountLogOnExit
		Globals(char* theFilename) : 
			filename(theFilename),
		#else
		Globals() : 
		#endif
			List_pRCObj()				
		{
		}
		~Globals();

	};

	static Globals theGlobals;
};


	static RefCountObject_TrackData :: Globals RefCountObject_TrackData :: theGlobals
	(
		#if OutputRefCountLogOnExit		
		"REFDUMP.TXT"
		#endif
	);

	#if 0
	static List<RefCountObject*> RefCountObject_TrackData :: TheList :: List_pRCObj;
	#endif



#endif
/* Internal function prototypes ************************************/

/* Internal globals ************************************************/

/* Exported function definitions ***********************************/

// class RefCountObject
#if TrackReferenceCounted
void RefCountObject :: Track_Construct(void)
{
	GLOBALASSERT( pTrackData == NULL );

	pTrackData = new RefCountObject_TrackData(this,"unknown file",666);
}

void RefCountObject :: Track_R_AddRef(char* theFilename, int theLineNum)
{
	GLOBALASSERT( pTrackData );

	pTrackData -> Track_R_AddRef(theFilename,theLineNum);
}

void RefCountObject :: Track_R_Release(char* theFilename, int theLineNum)
{
	GLOBALASSERT( pTrackData );

	pTrackData -> Track_R_Release(theFilename,theLineNum);
}

void RefCountObject :: Track_Destroy(void)
{
	GLOBALASSERT( pTrackData );

	delete pTrackData;
}

void RefCountObject :: ReferenceDump(R_DumpContext& theContext) const
{
	GLOBALASSERT( pTrackData );

	DumpIDForReferenceDump(theContext);

	pTrackData -> DumpTransactions(theContext);
}

static void RefCountObject :: DumpAll(R_DumpContext& theContext)
{
	RefCountObject_TrackData :: DumpAll(theContext);
}
#endif // TrackReferenceCounted
/* Internal function definitions ***********************************/
#if TrackReferenceCounted
// class RefCountObject_TrackData
// class RefCountObject_TrackData :: ReferenceTransaction
// public:
void RefCountObject_TrackData :: ReferenceTransaction :: Dump( R_DumpContext& theContext ) const
{
	char* Action =
	(
		( Type == tt_addref)
		?
		"R_AddRef()"
		:
		"R_Release()"
	);
	
	theContext.dprintf("-- %s in file \"%s\" at line %i\n",Action,Filename,LineNum);
}

// public:
static void RefCountObject_TrackData :: DumpAll(R_DumpContext& theContext)
{
	theContext . dprintf
	(
		"RefCountObject::DumpAll(); num objects=%i\n",
		theGlobals . List_pRCObj . size()
	);

	for
	(
		CLIF<RefCountObject*> oi(&theGlobals . List_pRCObj);
		!oi . done();
		oi . next()
	)
	{
		GLOBALASSERT(oi());
		oi() -> ReferenceDump(theContext);
	}
}
RefCountObject_TrackData :: RefCountObject_TrackData(RefCountObject *const pRCObj,char* theFilename, int theLineNum) :
	pRCObj_Val(pRCObj),
	List_pTransaction(),
	constructionFilename(theFilename),
	constructionLineNum(theLineNum)
{
	theGlobals . List_pRCObj . add_entry(pRCObj_Val);
}

RefCountObject_TrackData :: ~RefCountObject_TrackData()
{
	while ( List_pTransaction . size() > 0)
	{
		List_pTransaction . delete_first_entry();
	}

	theGlobals . List_pRCObj . delete_entry(pRCObj_Val);

}

void RefCountObject_TrackData :: Track_R_AddRef(char* theFilename, int theLineNum)
{
	List_pTransaction . add_entry_end
	(
		new Transaction_R_AddRef(theFilename,theLineNum)
	);
}

void RefCountObject_TrackData :: Track_R_Release(char* theFilename, int theLineNum)
{
	List_pTransaction . add_entry_end
	(
		new Transaction_R_Release(theFilename,theLineNum)
	);
}

void RefCountObject_TrackData :: DumpTransactions( R_DumpContext& theContext) const
{
	theContext . dprintf("-- Constructed in \"%s\" at line %i\n",constructionFilename,constructionLineNum);
	for
	(
		CLIF<ReferenceTransaction*> oi(&List_pTransaction);
		!oi . done();
		oi . next()
	)
	{
		oi() -> Dump(theContext);
	}
}

// private:

#if OutputRefCountLogOnExit
// We put one of these in the BSS segment so that its destructor
// gets called at exit.  The destructor writes all the info
// to a log file
// class Globals
RefCountObject_TrackData :: Globals :: ~Globals()
{
	LogFile theLogFile(filename);
	RefCountObject :: DumpAll(theLogFile);
}
#endif

#endif // TrackReferenceCounted
