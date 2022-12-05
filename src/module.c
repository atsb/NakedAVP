#include "3dc.h"

#include "inline.h"
#include "module.h"
#include "stratdef.h"
#include "bh_types.h"
#include "pvisible.h"
#include "pfarlocs.h"
#include "avpview.h"
#include "ourasert.h"
#include "pldghost.h"

#if SupportModules

/* imported externs */

extern SCENE Global_Scene;
extern int NumActiveBlocks;
extern DISPLAYBLOCK *ActiveBlockList[];
extern DISPLAYBLOCK *dptr_last;
extern unsigned char KeyASCII;

/**** Protos ****/

void FindVisibleModules(VMODULE *vptr,int flag);

/**** Statics ****/

static MODULE **Global_ModuleArrayPtr;

void AllNewModuleHandler(void)
{
	{
		int i;
		SCENEMODULE *smptr;
		smptr = Global_ModulePtr[Global_Scene];
		Global_ModuleArrayPtr = smptr->sm_marray;
	
		for(i = 0; i < ModuleArraySize; i++)
		{
			ModuleCurrVisArray[i] = 0;
		}
	}
	
	/* handle dynamic module objects */
	{
		int numberOfObjects = NumActiveBlocks;
		
	   	while (numberOfObjects--)
		{
			DISPLAYBLOCK* objectPtr = ActiveBlockList[numberOfObjects];
			
			if(objectPtr->ObFlags3 & ObFlag3_DynamicModuleObject)
			{
				STRATEGYBLOCK *sbPtr = objectPtr->ObStrategyBlock;

				sbPtr->containingModule = (ModuleFromPosition(&(objectPtr->ObWorld), sbPtr->containingModule));
				if (sbPtr->containingModule)
				if (ModuleIsPhysical(sbPtr->containingModule))
				{
					ModuleCurrVisArray[sbPtr->containingModule->m_index] = 1;
					if(sbPtr->containingModule->m_vmptr) FindVisibleModules(sbPtr->containingModule->m_vmptr,1);
				}
			}
		}
	}
	
	/*If this is an network game , and this machine is the ai server , then need to check if
	  there are any aliens near to other players*/
	if(AvP.Network!=I_No_Network && AvP.NetworkAIServer)
	{
		/* go through the strategy blocks looking for players*/
		int sbIndex;
		for(sbIndex=0;sbIndex<NumActiveStBlocks;sbIndex++)
		{
			STRATEGYBLOCK *playerSbPtr = ActiveStBlockList[sbIndex];
			NETGHOSTDATABLOCK *ghostData;
			if(playerSbPtr->I_SBtype!=I_BehaviourNetGhost) continue;
			ghostData = (NETGHOSTDATABLOCK *)playerSbPtr->SBdataptr;

			if(ghostData->type==I_BehaviourAlienPlayer ||
			   ghostData->type==I_BehaviourMarinePlayer ||
			   ghostData->type==I_BehaviourPredatorPlayer)
			{
				int sbIndex2;
				//found one of the players
				if(!playerSbPtr->containingModule) continue;

				/*now search through the strategy blocks , to see if any aliens are
				visible from the player's location*/
				for(sbIndex2=0;sbIndex2<NumActiveStBlocks;sbIndex2++)
				{
					STRATEGYBLOCK *alienSbPtr = ActiveStBlockList[sbIndex2];
					/*Is it an alien?*/
					if(alienSbPtr->I_SBtype!=I_BehaviourAlien) continue;
					if(!alienSbPtr->containingModule) continue;

					if(IsModuleVisibleFromModule(playerSbPtr->containingModule,alienSbPtr->containingModule))
					{
						/*The player can see the alien , so link in all modules that the
						player can see*/
						if (ModuleIsPhysical(playerSbPtr->containingModule))
						{
							ModuleCurrVisArray[playerSbPtr->containingModule->m_index] = 1;
							if(playerSbPtr->containingModule->m_vmptr) FindVisibleModules(playerSbPtr->containingModule->m_vmptr,1);
						}

						/*Since all modules visible by this player have now been linked , don't need
						to check for any more aliens for this player*/
						break;

					}

				}

			}	
		}
	}
	
	/* handle player visibilities */
	{
		extern MODULE * playerPherModule;
		playerPherModule = (ModuleFromPosition(&(Global_VDB_Ptr->VDB_World), playerPherModule));
	
		if(!playerPherModule)
		{
			playerPherModule = (ModuleFromPosition(&(Player->ObWorld), playerPherModule));
		}

		if (playerPherModule)
		{
			ModuleCurrVisArray[playerPherModule->m_index] = 2;
			if(playerPherModule->m_vmptr) FindVisibleModules(playerPherModule->m_vmptr,2);
		}
	}



	/* handle AIMODULE visibility stuff */
	{
		int i;
		for(i = 0; i < ModuleArraySize; i++)
		{
			if (ModuleCurrVisArray[i] == 2)
			{
				AIMODULE *aiModulePtr = Global_ModuleArrayPtr[i]->m_aimodule;
				if (aiModulePtr)
				{
					MODULE **modulelistPtr = aiModulePtr->m_module_ptrs;
					while(*modulelistPtr)
					{
						int index = (*modulelistPtr)->m_index;
						if (!ModuleCurrVisArray[index]) ModuleCurrVisArray[index]=1;
						modulelistPtr++;
					}
				}
			}
		}
	}
	/* update active block list */
	{
		int i;
	
		for(i = 0; i < ModuleArraySize; i++)
		{
			MODULE *mptr = Global_ModuleArrayPtr[i];

			if(ModuleCurrVisArray[i])
			{
				if(mptr->m_dptr == 0 && ((mptr->m_flags & m_flag_dormant) == 0))
				{
					AllocateModuleObject(mptr);
				}

			}
			else
			{
				if(mptr->m_dptr) DeallocateModuleObject(mptr);
			}

		}

	}

	/* call Patrick's code */
	DoObjectVisibilities();
}


void FindVisibleModules(VMODULE *vptr,int flag)
{
	while(vptr->vmod_type != vmtype_term)
	{
		MODULE *mptr = NULL;

		/* Add this module to the visible array */
		if(vptr->vmod_mref.mref_ptr != NULL)
		{
			mptr = vptr->vmod_mref.mref_ptr;
			ModuleCurrVisArray[mptr->m_index] = flag;
		}

		/* VMODULE instructions */
		switch(vptr->vmod_instr)
		{
			case vmodi_null:
				vptr++;
				break;

			case vmodi_bra_vc:
				/* If the door/viewport is closed... */
				/* Branch to this vptr */
				/* else vptr++; */
				if(mptr != NULL)
				{
					if(mptr->m_flags & m_flag_open) vptr++;
					else vptr = vptr->vmod_data.vmodidata_ptr;
				}
				break;
		}
	}
}


int ThisObjectIsInAModuleVisibleFromCurrentlyVisibleModules(STRATEGYBLOCK *sbPtr)
{
	VMODULE *vPtr;
	
	GLOBALASSERT(sbPtr);
	GLOBALASSERT(sbPtr->containingModule);

	vPtr = sbPtr->containingModule->m_vmptr;
	GLOBALASSERT(vPtr);
	
	if(ModuleCurrVisArray[sbPtr->containingModule->m_index] == 2)
	{
		return 1;
	}
	
	while(vPtr->vmod_type != vmtype_term)
	{
		MODULE *mptr = NULL;

		/* consider this module */
		if(vPtr->vmod_mref.mref_ptr != NULL)
		{
			mptr = vPtr->vmod_mref.mref_ptr;
			if(ModuleCurrVisArray[mptr->m_index] == 2)
			{
				if(vPtr->vmod_instr==vmodi_bra_vc)
				{
					if(mptr->m_flags & m_flag_open) return 1;
				}
				else
				{
					return 1;
				}
			}
		}

		/* VMODULE instructions */
		switch(vPtr->vmod_instr)
		{
			case vmodi_null:
				vPtr++;
				break;

			case vmodi_bra_vc:
				/* If the door/viewport is closed... */
				/* Branch to this vPtr */
				/* else vPtr++; */
				if(mptr != NULL)
				{
					if(mptr->m_flags & m_flag_open) vPtr++;
					else vPtr = vPtr->vmod_data.vmodidata_ptr;
				}
				break;
		}
	}

	return 0;
}





void ModuleFunctions(MODULE *mptr, MFUNCTION mf)
{
	switch(mf) 
	{
		case mfun_null:
			break;
	}
}


void AllocateModuleObject(MODULE *mptr)
{

	DISPLAYBLOCK *dptr;
	MODULEMAPBLOCK *mapblockptr;
	STRATEGYBLOCK *sb = 0;

	#if SupportMorphing
	MORPHCTRL *mc;
	#endif

	dptr_last = 0;

	mptr->m_dptr = 0;

	if(mptr == 0) return;					/* Whoops! */

	if(mptr->m_mapptr == 0) return;		/* Not all modules have maps */


	dptr = CreateActiveObject();


	if(dptr) {


		/* Tell the module we exist */

		mptr->m_dptr = dptr;

		/* Tell the object who its module is */

		dptr->ObMyModule = mptr;

		/* Get the strategy block, if it exists */

		if(mptr->m_sbptr) sb = mptr->m_sbptr;

		/* If there is a STRATEGYBLOCK, tell it we exist */

		if(sb) {

			dptr->ObStrategyBlock = sb;
			sb->SBdptr = dptr;

		}


		/* Read the map */

		mapblockptr = mptr->m_mapptr;

		dptr->ObShape = mapblockptr->MapShape;



		CopyLocation(&mapblockptr->MapWorld, &dptr->ObWorld);
		CopyEuler(&mapblockptr->MapEuler, &dptr->ObEuler);

		dptr->ObFlags  = mapblockptr->MapFlags;
		dptr->ObFlags2 = mapblockptr->MapFlags2;
		dptr->ObFlags3 = mapblockptr->MapFlags3;




		#if SupportMorphing

		/* If there is a strategy block, see if it has a MORPHCTRL structure */

		if(sb) {

			if(sb->SBmorphctrl) {

				/* Pass MORPHCTRL to dptr */

				dptr->ObMorphCtrl = sb->SBmorphctrl;

				/* Copy the morph pointer from the map to the dptr */

				mc = dptr->ObMorphCtrl;

				mc->ObMorphHeader = mapblockptr->MapMorphHeader;

				/* OLD TEST - These values are now set elsewhere */
				#if 0
				if(mc->ObMorphHeader) {
					mc->ObMorphCurrFrame = 0;
					mc->ObMorphFlags = mph_flag_play/* | mph_flag_noloop | mph_flag_reverse*/;
					mc->ObMorphSpeed = ONE_FIXED;
				}
				#endif

			}

		}

		#endif	/* Support Morphing */


		dptr->ObLightType = LightType_PerVertex;
		dptr->ObFlags |= ObFlag_MultLSrc;


		if(mapblockptr->MapVDBData)
			MapSetVDB(dptr, mapblockptr->MapVDBData);

		dptr->ObLightType = mapblockptr->MapLightType;


		MapBlockInit(dptr);

		/* KJL 14:15:34 04/19/97 - their used to be lots of maths here
		to calculate orientation, but in AvP all modules are aligned to
		the world space axes... */
		{
			extern MATRIXCH IdentityMatrix;
			dptr->ObMat = IdentityMatrix;
		}
		

		/*

		Module lights

		There is an option for a pointer to an array of lights in a module
		structure. These lights are transferred to the display block and
		flagged as "LFlag_WasNotAllocated" so that "DeallocateLightBlock()"
		knows to ignore them.

		The number of lights in the array is "m_numlights" and the pointer
		is called "m_lightarray".

		The addition of non-allocated does not need to be a module specific
		option.

		Non-allocated lights can co-exist peacefully with the other lights.

		*/

		if(mptr->m_numlights && mptr->m_lightarray) {

			LIGHTBLOCK *lptr_array = mptr->m_lightarray;
			int i;

			for(i = mptr->m_numlights; i!=0; i--) {

				/* Make sure the light is flagged correctly */

				lptr_array->LightFlags |= LFlag_WasNotAllocated;

				/* Add the light */

				AddLightBlock(dptr, lptr_array);

				/* Next light from the array */

				lptr_array++;

			}

		}




		/*

		As with shared points, extra item data for prelighting is also
		copied from the module to the display block.

		WARNING:

		Allocation and deallocation of this pointer is the responsibility
		of the user!

		*/

		dptr->ObEIDPtr = mptr->m_extraitemdata;

		/* Added Name to DISPLAYBLOCK */

		dptr->name = mptr->name;

		MapPostProcessing(dptr);


		ModuleObjectJustAllocated(mptr);				/* Project Function */

		/* Bug Fix */

		if (dptr->ObStrategyBlock) {

			STRATEGYBLOCK *sbptr=dptr->ObStrategyBlock;

			if (sbptr->I_SBtype==I_BehaviourSimpleAnimation) {

				SIMPLE_ANIM_BEHAV_BLOCK *sanimbhv;

				sanimbhv = (SIMPLE_ANIM_BEHAV_BLOCK*)(sbptr->SBdataptr);

				GLOBALASSERT(sanimbhv->bhvr_type == I_BehaviourSimpleAnimation);
				GLOBALASSERT (dptr == sbptr->SBdptr);

				if(!dptr->ObTxAnimCtrlBlks)	{ 
					dptr->ObTxAnimCtrlBlks = sanimbhv->tacbSimple;
				}

			}

		}

	}

	dptr_last = dptr;

}


void DeallocateModuleObject(MODULE *mptr)

{

	DISPLAYBLOCK *dptr;
	STRATEGYBLOCK *sb;


	if(mptr->m_dptr) {

		ModuleObjectAboutToBeDeallocated(mptr);	/* Project Function */

		dptr = mptr->m_dptr;

		DestroyActiveObject(dptr);

		/* Clear module reference to dptr */

		mptr->m_dptr = 0;

		/* If there is a STRATEGYBLOCK, clear that reference too */

		if(mptr->m_sbptr) {

			sb = mptr->m_sbptr;
			sb->SBdptr = 0;

		}

	}

}


/*

 The Module Preprocessor

 Pass the array of pointers to modules.
 This function creates module indices and converts names into pointers.

*/

/*
  temp patch to match current chunk
  loader configuration
*/

void PreprocessAllModules(void)
{

	SCENEMODULE **sm_array_ptr;
	SCENEMODULE *sm_ptr;


	if(Global_ModulePtr == 0) return;

	sm_array_ptr = Global_ModulePtr;

	while(*sm_array_ptr)
	{
		sm_ptr = *sm_array_ptr;

		PreprocessModuleArray(sm_ptr->sm_marray);

		sm_array_ptr++;
	}
}



/*

 A special function to deallocate the module visibility arrays

*/

void DeallocateModuleVisArrays(void)

{

	if(ModuleCurrVisArray) {

		DeallocateMem(ModuleCurrVisArray);
		ModuleCurrVisArray = 0;

	}

}


/*

 Allocate the two arrays used to keep track of module visibility from
 frame to frame. The function uses the global scene variable to access the
 appropriate SCENEMODULE and find out how many modules are present. It also
 deallocates the previous arrays if they exist.

*/

int GetModuleVisArrays(void)

{

	SCENEMODULE *sm_ptr;
	MODULE **m_array_ptr;
	MODULE *m_ptr;
	int index, i;


	if(Global_ModulePtr == 0) return No;

	DeallocateModuleVisArrays();

	sm_ptr = Global_ModulePtr[Global_Scene];

	m_array_ptr = sm_ptr->sm_marray;
	index       = smallint;

	while(*m_array_ptr) {

		m_ptr = *m_array_ptr++;
		if(m_ptr->m_index > index) index = m_ptr->m_index;

	}

	ModuleArraySize = index + 1;


	ModuleCurrVisArray  = (char*) AllocateMem(ModuleArraySize);

	if(ModuleCurrVisArray)
	{

		for(i = 0; i < ModuleArraySize; i++)
		{

			ModuleCurrVisArray[i]  = 0;
		}

		#if 0
		textprint("visibility arrays ok, size %d\n", ModuleArraySize);
		#endif

		return Yes;

	}

	else return No;

}




#define ppma_print No


#if 1


void PreprocessModuleArray(MODULE **m_array_ptr)
{
	MODULE **m_array = m_array_ptr;
	MODULE *m_ptr;
	int index;


	#if ppma_print
	textprint("PreprocessModuleArray %u\n", m_array_ptr);
	#endif


	index = 0;

	while(*m_array) {

		/* Get the module pointer */

		m_ptr = *m_array;


		/* Assign the module an index */

		m_ptr->m_index = index++;


		#if ppma_print
		textprint("\nModule %u, ", m_ptr);
		PrintName(&m_ptr->m_name);
		textprint(", index %d\n", m_ptr->m_index);
		textprint("  (vptr = ");
		PrintName(&m_ptr->m_vptr.mref_name);
		textprint(")\n");
		#endif


		/* Convert module references from names to pointers */

		if(!(m_ptr->m_flags & m_flag_gotptrs)) {

			#if 0
			/* Vertical Pointer */

			ConvertModuleNameToPointer(&m_ptr->m_vptr, m_array_ptr);


			/* Extent Pointer */

			ConvertModuleNameToPointer(&m_ptr->m_ext, m_array_ptr);


			/* Function Pointer */

			ConvertModuleNameToPointer(&m_ptr->m_funref, m_array_ptr);


			// Hack by John to make the m_link pointers work

			if (m_ptr->m_link_ptrs)
			{
				MREF * m_link_ptr = m_ptr->m_link_ptrs;
				
				while (m_link_ptr->mref_ptr)
				{
					ConvertModuleNameToPointer(m_link_ptr++, m_array_ptr);
				}

			}
			 #endif
			/* VMODULE Array */

			if(m_ptr->m_vmptr) {

				/* Convert VMODIDATA names to pointers */

				PreprocessVMODIDATA(m_ptr->m_vmptr);

				/* Convert MREF names to pointers */
				/*
				v_ptr = m_ptr->m_vmptr;

				while(v_ptr->vmod_type != vmtype_term) {

					ConvertModuleNameToPointer(&v_ptr->vmod_mref, m_array_ptr);

					v_ptr++;

				}
				*/
			}


			/* Tell the module that its names are now pointers */

			m_ptr->m_flags |= m_flag_gotptrs;

		}


		/* Calculate module extents */
	
		//I'll set the extents and world position in the loaders -Richard.
		//GetModuleMapData(m_ptr);


		/* Next module array entry */

		m_array++;

	}

	/*WaitForReturn();*/

}


#else




#endif


/*

 VMODIDATA is the data associated with VMODI, the VMODULE instruction. Some
 of the data items are names which need to be converted to pointers.

*/

void PreprocessVMODIDATA(VMODULE *v_ptr)

{

	VMODULE *v_array_ptr = v_ptr;


	while(v_ptr->vmod_type != vmtype_term) {

		if(!(v_ptr->vmod_flags & vm_flag_gotptrs)) {

			switch(v_ptr->vmod_instr) {

				case vmodi_null:
					break;

				case vmodi_bra_vc:
					ConvertVModuleNameToPointer(&v_ptr->vmod_data, v_array_ptr);
					break;

			}

			v_ptr->vmod_flags |= vm_flag_gotptrs;

		}

		v_ptr++;

	}

}


/*

 Convert MREF name to MREF pointer

*/


#define cmntp_print No


void ConvertModuleNameToPointer(MREF *mref_ptr, MODULE **m_array_ptr)

{

	MODULE *m_ptr;
	int StillSearching;


	#if cmntp_print
	textprint("ConvertModuleNameToPointer\n");
	#endif


	/* Set "null" names to null pointers */

	if(CompareName((char *)&mref_ptr->mref_name, "null")) {

		#if cmntp_print
		textprint("making ptr null\n");
		#endif

		mref_ptr->mref_ptr = 0;
		return;

	}


	/* Search for the module with the same name */

	#if cmntp_print
	textprint("Searching for name...\n");
	#endif

	StillSearching = Yes;

	while(*m_array_ptr && StillSearching) {

		m_ptr = *m_array_ptr;

		if(CompareName((char *)&mref_ptr->mref_name, (char *)&m_ptr->m_name)) {

			#if cmntp_print
			textprint("  found name ");
			PrintName(&m_ptr->m_name);
			textprint(", ptr %u\n", m_ptr);
			#endif

			mref_ptr->mref_ptr = m_ptr;
			StillSearching = No;

		}

		m_array_ptr++;

	}


	/* If the name was not found, make this a null pointer */

	if(StillSearching) mref_ptr->mref_ptr = 0;

}


/*

 Convert VMODIDATA.vmodidata_label names to VMODIDATA.vmodidata_ptr

*/


#define cvmntp_print No


void ConvertVModuleNameToPointer(VMODIDATA *vmodidata_ptr, VMODULE *v_array_ptr)

{

	int StillSearching;


	#if cvmntp_print
	textprint("ConvertVModuleNameToPointer\n");
	#endif


	/* Set "null" names to null pointers */

	if(CompareName((char *)&vmodidata_ptr->vmodidata_label, "null")) {

		#if cvmntp_print
		textprint("  making vmodidata_ptr null\n");
		#endif

		vmodidata_ptr->vmodidata_ptr = 0;
		return;

	}


	/* Search for the VMODULE with the same name */

	#if cvmntp_print
	textprint("  Searching for name...\n");
	#endif

	StillSearching = Yes;

	while((v_array_ptr->vmod_type != vmtype_term) && StillSearching) {

		if(CompareName((char *)&vmodidata_ptr->vmodidata_label, (char *)&v_array_ptr->vmod_name)) {

			#if cmntp_print
			textprint("  found name ");
			PrintName(&v_array_ptr->vmod_name);
			textprint(", ptr %u\n", v_array_ptr);
			#endif

			vmodidata_ptr->vmodidata_ptr = v_array_ptr;
			StillSearching = No;

		}

		v_array_ptr++;

	}


	/* If the name was not found, make this a null pointer */

	if(StillSearching)
		{
			if(v_array_ptr->vmod_type == vmtype_term)
				{
					vmodidata_ptr->vmodidata_ptr = v_array_ptr;
				}
			else
				{
					vmodidata_ptr->vmodidata_ptr = 0;
				}
		}


}




int CompareName(char *name1, char *name2)

{

	int i;


	for(i = 4; i!=0; i--) {

		if(*name1++ != *name2++) return No;

	}

	return Yes;

}


void PrintName(char *name)

{

	char m_name[5];


	m_name[0] = name[0];
	m_name[1] = name[1];
	m_name[2] = name[2];
	m_name[3] = name[3];
	m_name[4] = 0;
	textprint(m_name);

}


int IsModuleVisibleFromModule(MODULE *source, MODULE *target) {

	VMODULE *vptr;	
	MODULE *mptr;
	int gotit;

	vptr=source->m_vmptr;
	mptr=NULL;
	gotit=0;

	if ((source==NULL)||(target==NULL)) return(0);
	if (source==target) return(1);

	while(! ((vptr->vmod_type == vmtype_term)||(gotit)) ) {

		/* Add this module to the visible array */

		if(vptr->vmod_mref.mref_ptr) {

			mptr = vptr->vmod_mref.mref_ptr;

			if (mptr==target) gotit=1;

		}

		/* VMODULE instructions */

		switch(vptr->vmod_instr) {

			case vmodi_null:

				vptr++;

				break;

			case vmodi_bra_vc:

				/* NYD */

				/* If the door/viewport is closed... */

				/* Branch to this vptr */

				if(mptr)
					{
						if(mptr->m_flags & m_flag_open)
							vptr++;
						else
							vptr = vptr->vmod_data.vmodidata_ptr;
					}



				/* else vptr++; */

				break;

		}

	}

	return(gotit);

}

int IsAIModuleVisibleFromAIModule(AIMODULE *source,AIMODULE *target) {

	if ((source==NULL)||(target==NULL)) return(0);
	if (source==target) return(1);

	{
		MODULE **targetModulelistPtr;
		MODULE **sourceModulelistPtr = source->m_module_ptrs;
		while(*sourceModulelistPtr) {

			targetModulelistPtr=target->m_module_ptrs;
			while(*targetModulelistPtr) {
				if (IsModuleVisibleFromModule(*sourceModulelistPtr,*targetModulelistPtr)) {
					return(1);
				}
				targetModulelistPtr++;
			}
			sourceModulelistPtr++;
		}
	}

	return(0);
}

#endif
