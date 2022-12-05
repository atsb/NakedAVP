/*******************************************************************
 *
 *    DESCRIPTION: 	modcmds.cpp
 *
 *    AUTHOR: David Malcolm
 *
 *    HISTORY:  Created 27/11/97
 *
 *******************************************************************/

/* Includes ********************************************************/
#include <ctype.h>

#include "3dc.h"
#include "gadget.h"

	#if UseGadgets
		#include "module.h"
		#include "modcmds.hpp"
		#include "stratdef.h"
		#include "dynblock.h"
	#endif
	
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
		#if UseGadgets
		extern SCENEMODULE **Global_ModulePtr;
		extern DISPLAYBLOCK* Player;
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

/* Internal type definitions ***************************************/

/* Internal function prototypes ************************************/

/* Internal globals ************************************************/

/* Exported function definitions ***********************************/

#if UseGadgets
// namespace ModuleCommands
void ModuleCommands :: ListModules(void)
{

	char Msg[256];

	if
	(
		Global_ModulePtr
	)
	{
		SCENEMODULE* pSceneModule = Global_ModulePtr[0];

		if ( pSceneModule )
		{
			MODULE** ppModule_I = pSceneModule -> sm_marray;

			if ( ppModule_I )
			{
				GADGET_NewOnScreenMessage( "MODULE LISTING (CASE INSENSITIVE):" );

				int Index=0;

				// Iterate through NULL-terminated list of MODULE*s
				while ( *ppModule_I )
				{
					MODULE* pModule = *ppModule_I;

					// Diagnostic on pModule:
					{
						if
						(
							pModule -> name
						)
						{
							// Get upper-case module name:
							char TempName[256];
							char* pCh_Dst = &TempName[0];
							char* pCh_Src = pModule -> name;
							int CharCount = 0;
							while ( (*pCh_Src) && (CharCount < 255) )
							{
								*(pCh_Dst++) = toupper( *(pCh_Src++));
								CharCount++;
							}

							*pCh_Dst = 0;

							sprintf
							(
								Msg,
								"MODULE:%3i \"%s\"",
								Index,
								TempName
							);
						}
						else
						{
							sprintf
							(
								Msg,
								"MODULE:%3i NULL NAME",
								Index
							);							
						}
						

						GADGET_NewOnScreenMessage( Msg );

					}

					// Advance to next MODULE*
					(ppModule_I++);
					Index++;

				}

				sprintf
				(
					Msg,
					"END OF MODULE LISTING (%i MODULES)",
					ModuleArraySize
				);
				GADGET_NewOnScreenMessage( Msg );

			}
			else
			{
				GADGET_NewOnScreenMessage( "CANNOT LIST MODULES; SM_MARRAY IS NULL" );
			}
		}
		else
		{
			GADGET_NewOnScreenMessage( "CANNOT LIST MODULES; GLOBALMODULEPTR[0] IS NULL" );
		}
	}
	else
	{
		GADGET_NewOnScreenMessage( "CANNOT LIST MODULES; GLOBALMODULEPTR IS NULL" );
	}
}

void ModuleCommands :: TryToTeleport(char* UpperCasePotentialModuleName)
{
	// the test name is in upper case (since its been typed in)
	// we must test against the module names without actually
	// converting them to upper case

	// If we find a match we try to teleport the player...

	/* PRECONDITION */
	{
		GLOBALASSERT( UpperCasePotentialModuleName );
	}

	/* CODE */
	{
		char Msg[256];

		{
			sprintf
			(
				Msg,
				"TELEPORT REQUEST TO MODULE \"%s\"",
				UpperCasePotentialModuleName
			);
			
			GADGET_NewOnScreenMessage( Msg );
		}

		MODULE* pModule_Targ = FindModule( UpperCasePotentialModuleName );

		if ( !pModule_Targ )
		{
			GADGET_NewOnScreenMessage( "UNRECOGNISED MODULE" );
			return;
		}

		GLOBALASSERT( pModule_Targ );

		#if 1
		GADGET_NewOnScreenMessage( "(FOUND THE MODULE)" );
		#endif

		TeleportPlayerToModule
		(
			pModule_Targ
		);
	}
}

MODULE* ModuleCommands :: FindModule(char* UpperCasePotentialModuleName)
{
	// allowed to return NULL if no match
	GLOBALASSERT( UpperCasePotentialModuleName );

	MODULE* pModule_Found = NULL;

	if
	(
		Global_ModulePtr
	)
	{
		SCENEMODULE* pSceneModule = Global_ModulePtr[0];

		if ( pSceneModule )
		{
			MODULE** ppModule_I = pSceneModule -> sm_marray;

			if ( ppModule_I )
			{
				int Index=0;

				// Iterate through NULL-terminated list of MODULE*s
				while ( *ppModule_I )
				{
					MODULE* pModule = *ppModule_I;

					// Diagnostic on pModule:
					{
						if
						(
							pModule -> name
						)
						{
							//use a case insensitive comparison instead of converting
							//to upper case
							#if 0
							// Get upper-case module name:
							char TempName[256];
							char* pCh_Dst = &TempName[0];
							char* pCh_Src = pModule -> name;
							int CharCount = 0;
							while ( (*pCh_Src) && (CharCount < 255) )
							{
								*(pCh_Dst++) = toupper( *(pCh_Src++));
								CharCount++;
							}

							*pCh_Dst = 0;
							#endif

							#if 1
							if
							(
								0 == _stricmp
								(
									pModule -> name,
									UpperCasePotentialModuleName
								)
							)
							{
								// Got a match:
								pModule_Found = pModule;
							}
							#else
							sprintf
							(
								Msg,
								"MODULE:%3i \"%s\"",
								Index,
								TempName
							);
							#endif
						}
					}

					// Advance to next MODULE*
					(ppModule_I++);
					Index++;

				}
			}
			else
			{
				GADGET_NewOnScreenMessage( "CANNOT FIND MODULES; SM_MARRAY IS NULL" );
			}
		}
		else
		{
			GADGET_NewOnScreenMessage( "CANNOT FIND MODULES; GLOBALMODULEPTR[0] IS NULL" );
		}
	}
	else
	{
		GADGET_NewOnScreenMessage( "CANNOT FIND MODULES; GLOBALMODULEPTR IS NULL" );
	}	

	return pModule_Found;
}

void ModuleCommands :: TeleportPlayerToModule(MODULE* pModule_Dst)
{
	/* PRECONDITION */
	{
		GLOBALASSERT( pModule_Dst );
	}

	/* CODE */
	{
		if (Player )
		{
			if ( Player -> ObStrategyBlock )
			{
				// Code based around fn EmergencyPlaceObjectInModule() in PVISIBLE.C:

				if ( Player -> ObStrategyBlock -> DynPtr ) 
				{
					DYNAMICSBLOCK* dynPtr = Player -> ObStrategyBlock -> DynPtr;

					dynPtr -> Position.vx = pModule_Dst -> m_world.vx;
					dynPtr -> Position.vy = pModule_Dst -> m_world.vy;
					dynPtr -> Position.vz = pModule_Dst -> m_world.vz;
					dynPtr->LinImpulse.vx = 0;
					dynPtr->LinImpulse.vy = 0;
					dynPtr->LinImpulse.vz = 0;
					dynPtr -> PrevPosition = dynPtr -> Position;
				   
				}

				/* finally, update the sb's module */
				Player -> ObStrategyBlock -> containingModule = pModule_Dst;
			}
		}
		
	}
}
#endif // UseGadgets



/* Internal function definitions ***********************************/
