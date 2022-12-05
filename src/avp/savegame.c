#include "3dc.h"

#include "inline.h"
#include "module.h"
#include "stratdef.h"
#include "gamedef.h"
#include "bh_types.h"
#include "comp_shp.h"
#include "dynblock.h"

#include "bh_alien.h"
#include "pvisible.h"
#include "bh_pred.h"
#include "bh_xeno.h"
#include "bh_paq.h"
#include "bh_queen.h"
#include "bh_marin.h"
#include "bh_fhug.h"
#include "bh_debri.h"
#include "bh_plachier.h"
#include "plat_shp.h"
#include "psnd.h"
#include "lighting.h"
#include "pldnet.h"
#include "bh_dummy.h"
#include "bh_videoscreen.h"
#include "bh_plift.h"
#include "bh_ldoor.h"
#include "avp_menus.h"
#include "game_statistics.h"
#include "avp_userprofile.h"
#include "huddefs.h"
#include "fmv.h"

#include "savegame.h"
#include "huffman.hpp"

#define UseLocalAssert Yes
#include "ourasert.h"

static struct
{
	char* BufferStart;
	char* BufferPos;
	int BufferSize;
	int BufferSpaceLeft;
	int BufferSpaceUsed;

} SaveInfo = {0,0,0,0,0};

static struct
{
	char* BufferStart;
	char* BufferPos;
	int BufferSize;
	int BufferSpaceLeft;

} LoadInfo = {0,0,0,0};


int LoadGameRequest = SAVELOAD_REQUEST_NONE; //slot number of game to be loaded
int SaveGameRequest = SAVELOAD_REQUEST_NONE; //slot number of game to be saved


#define NUM_SAVES_FOR_EASY_MODE 8
#define NUM_SAVES_FOR_MEDIUM_MODE 4
#define NUM_SAVES_FOR_HARD_MODE 2

int NumberOfSavesLeft;



static void SaveStrategies();
static void LoadStrategy(SAVE_BLOCK_STRATEGY_HEADER* header);

static void SaveDeadStrategies();
static void LoadDeadStrategy(SAVE_BLOCK_HEADER* block);


static void LoadMiscGlobalStuff(SAVE_BLOCK_HEADER* header);
static void SaveMiscGlobalStuff();

/*---------------------------------------------**
** externs for all the load and save functions **
**---------------------------------------------*/
extern void LoadStrategy_LiftDoor(SAVE_BLOCK_STRATEGY_HEADER* header);
extern void SaveStrategy_LiftDoor(STRATEGYBLOCK* sbPtr);

extern void LoadStrategy_ProxDoor(SAVE_BLOCK_STRATEGY_HEADER* header);
extern void SaveStrategy_ProxDoor(STRATEGYBLOCK* sbPtr);

extern void LoadStrategy_SwitchDoor(SAVE_BLOCK_STRATEGY_HEADER* header);
extern void SaveStrategy_SwitchDoor(STRATEGYBLOCK* sbPtr);

extern void LoadStrategy_PlatformLift(SAVE_BLOCK_STRATEGY_HEADER* header);
extern void SaveStrategy_PlatformLift(STRATEGYBLOCK* sbPtr);

extern void LoadStrategy_BinarySwitch(SAVE_BLOCK_STRATEGY_HEADER* header);
extern void SaveStrategy_BinarySwitch(STRATEGYBLOCK* sbPtr);

extern void LoadStrategy_LinkSwitch(SAVE_BLOCK_STRATEGY_HEADER* header);
extern void SaveStrategy_LinkSwitch(STRATEGYBLOCK* sbPtr);

extern void LoadStrategy_Generator(SAVE_BLOCK_STRATEGY_HEADER* header);
extern void SaveStrategy_Generator(STRATEGYBLOCK* sbPtr);

extern void LoadHiveSettings(SAVE_BLOCK_HEADER* header);
extern void SaveHiveSettings();

extern void LoadStrategy_InanimateObject(SAVE_BLOCK_STRATEGY_HEADER* header);
extern void SaveStrategy_InanimateObject(STRATEGYBLOCK* sbPtr);

extern void LoadStrategy_LightFx(SAVE_BLOCK_STRATEGY_HEADER* header);
extern void SaveStrategy_LightFx(STRATEGYBLOCK* sbPtr);

extern void LoadStrategy_PlacedSound(SAVE_BLOCK_STRATEGY_HEADER* header);
extern void SaveStrategy_PlacedSound(STRATEGYBLOCK* sbPtr);

extern void LoadStrategy_Message(SAVE_BLOCK_STRATEGY_HEADER* header);
extern void SaveStrategy_Message(STRATEGYBLOCK* sbPtr);

extern void LoadStrategy_MissionComplete(SAVE_BLOCK_STRATEGY_HEADER* header);
extern void SaveStrategy_MissionComplete(STRATEGYBLOCK* sbPtr);

extern void LoadStrategy_TrackObject(SAVE_BLOCK_STRATEGY_HEADER* header);
extern void SaveStrategy_TrackObject(STRATEGYBLOCK* sbPtr);

extern void LoadStrategy_Fan(SAVE_BLOCK_STRATEGY_HEADER* header);
extern void SaveStrategy_Fan(STRATEGYBLOCK* sbPtr);

extern void LoadStrategy_PlacedLight(SAVE_BLOCK_STRATEGY_HEADER* header);
extern void SaveStrategy_PlacedLight(STRATEGYBLOCK* sbPtr);

extern void LoadStrategy_VideoScreen(SAVE_BLOCK_STRATEGY_HEADER* header);
extern void SaveStrategy_VideoScreen(STRATEGYBLOCK* sbPtr);

extern void LoadStrategy_DeathVolume(SAVE_BLOCK_STRATEGY_HEADER* header);
extern void SaveStrategy_DeathVolume(STRATEGYBLOCK* sbPtr);

extern void LoadStrategy_ParticleGenerator(SAVE_BLOCK_STRATEGY_HEADER* header);
extern void SaveStrategy_ParticleGenerator(STRATEGYBLOCK* sbPtr);

extern void LoadStrategy_SelfDestruct(SAVE_BLOCK_STRATEGY_HEADER* header);
extern void SaveStrategy_SelfDestruct(STRATEGYBLOCK* sbPtr);

extern void LoadStrategy_Alien(SAVE_BLOCK_STRATEGY_HEADER* header);
extern void SaveStrategy_Alien(STRATEGYBLOCK* sbPtr);

extern void LoadStrategy_Corpse(SAVE_BLOCK_STRATEGY_HEADER* header);
extern void SaveStrategy_Corpse(STRATEGYBLOCK* sbPtr);

extern void LoadStrategy_FaceHugger(SAVE_BLOCK_STRATEGY_HEADER* header);
extern void SaveStrategy_FaceHugger(STRATEGYBLOCK* sbPtr);

extern void LoadStrategy_Marine(SAVE_BLOCK_STRATEGY_HEADER* header);
extern void SaveStrategy_Marine(STRATEGYBLOCK* sbPtr);

extern void LoadMarineSquadState(SAVE_BLOCK_HEADER* header);
extern void SaveMarineSquadState();

extern void LoadStrategy_PlacedHierarchy(SAVE_BLOCK_STRATEGY_HEADER* header);
extern void SaveStrategy_PlacedHierarchy(STRATEGYBLOCK* sbPtr);

extern void LoadStrategy_Predator(SAVE_BLOCK_STRATEGY_HEADER* header);
extern void SaveStrategy_Predator(STRATEGYBLOCK* sbPtr);

extern void LoadStrategy_Xenoborg(SAVE_BLOCK_STRATEGY_HEADER* header);
extern void SaveStrategy_Xenoborg(STRATEGYBLOCK* sbPtr);

extern void LoadStrategy_Queen(SAVE_BLOCK_STRATEGY_HEADER* header);
extern void SaveStrategy_Queen(STRATEGYBLOCK* sbPtr);

extern void LoadStrategy_Player(SAVE_BLOCK_STRATEGY_HEADER* header);
extern void SaveStrategy_Player(STRATEGYBLOCK* sbPtr);

extern void LoadStrategy_Autogun(SAVE_BLOCK_STRATEGY_HEADER* header);
extern void SaveStrategy_Autogun(STRATEGYBLOCK* sbPtr);

extern void LoadStrategy_HierarchicalDebris(SAVE_BLOCK_STRATEGY_HEADER* header);
extern void SaveStrategy_HierarchicalDebris(STRATEGYBLOCK* sbPtr);

extern void Load_Decals(SAVE_BLOCK_HEADER* header);
extern void Save_Decals();

extern void Load_Particles(SAVE_BLOCK_HEADER* header);
extern void Save_Particles();

extern void Load_VolumetricExplosions(SAVE_BLOCK_HEADER* header);
extern void Save_VolumetricExplosions();

extern void Load_PheromoneTrails(SAVE_BLOCK_HEADER* header);
extern void Save_PheromoneTrails();

extern void Load_LightElements(SAVE_BLOCK_HEADER* header);
extern void Save_LightElements();

extern void Load_MessageHistory(SAVE_BLOCK_HEADER* header);
extern void Save_MessageHistory();

extern void LoadStrategy_Grenade(SAVE_BLOCK_STRATEGY_HEADER* header);
extern void SaveStrategy_Grenade(STRATEGYBLOCK* sbPtr);

extern void LoadStrategy_FlareGrenade(SAVE_BLOCK_STRATEGY_HEADER* header);
extern void SaveStrategy_FlareGrenade(STRATEGYBLOCK* sbPtr);

extern void LoadStrategy_ProxGrenade(SAVE_BLOCK_STRATEGY_HEADER* header);
extern void SaveStrategy_ProxGrenade(STRATEGYBLOCK* sbPtr);

extern void LoadStrategy_Rocket(SAVE_BLOCK_STRATEGY_HEADER* header);
extern void SaveStrategy_Rocket(STRATEGYBLOCK* sbPtr);

extern void LoadStrategy_PPPlasmaBolt(SAVE_BLOCK_STRATEGY_HEADER* header);
extern void SaveStrategy_PPPlasmaBolt(STRATEGYBLOCK* sbPtr);

extern void LoadStrategy_PredatorEnergyBolt(SAVE_BLOCK_STRATEGY_HEADER* header);
extern void SaveStrategy_PredatorEnergyBolt(STRATEGYBLOCK* sbPtr);

extern void LoadStrategy_PulseGrenade(SAVE_BLOCK_STRATEGY_HEADER* header);
extern void SaveStrategy_PulseGrenade(STRATEGYBLOCK* sbPtr);

extern void LoadStrategy_ClusterGrenade(SAVE_BLOCK_STRATEGY_HEADER* header);
extern void SaveStrategy_ClusterGrenade(STRATEGYBLOCK* sbPtr);

extern void LoadStrategy_Molotov(SAVE_BLOCK_STRATEGY_HEADER* header);
extern void SaveStrategy_Molotov(STRATEGYBLOCK* sbPtr);

extern void LoadStrategy_PredatorDisc(SAVE_BLOCK_STRATEGY_HEADER* header);
extern void SaveStrategy_PredatorDisc(STRATEGYBLOCK* sbPtr);

extern void LoadStrategy_SpearBolt(SAVE_BLOCK_STRATEGY_HEADER* header);
extern void SaveStrategy_SpearBolt(STRATEGYBLOCK* sbPtr);

extern void LoadStrategy_Grapple(SAVE_BLOCK_STRATEGY_HEADER* header);
extern void SaveStrategy_Grapple(STRATEGYBLOCK* sbPtr);

extern void LoadStrategy_Debris(SAVE_BLOCK_STRATEGY_HEADER* header);
extern void SaveStrategy_Debris(STRATEGYBLOCK* sbPtr);

extern void LoadLevelHeader(SAVE_BLOCK_HEADER* header);
extern void SaveLevelHeader();

extern void Load_WeaponsCGlobals(SAVE_BLOCK_HEADER* header);
extern void Save_WeaponsCGlobals();

extern void Load_SoundState_NoRef(SAVE_BLOCK_HEADER* header);
extern void Save_SoundsWithNoReference();

extern void LoadStrategy_Frisbee(SAVE_BLOCK_STRATEGY_HEADER* header);
extern void SaveStrategy_Frisbee(STRATEGYBLOCK* sbPtr);

extern void LoadStrategy_FrisbeeEnergyBolt(SAVE_BLOCK_STRATEGY_HEADER* header);
extern void SaveStrategy_FrisbeeEnergyBolt(STRATEGYBLOCK* sbPtr);

extern int DebuggingCommandsActive;

/*-----------------------------------------------------**
** end of externs for all the load and save functions  **
**-----------------------------------------------------*/

extern void DisplaySavesLeft();

/*
Functions for converting between ai modules and their indeces.
They don't really belong here , but they are only used by loading/saving at 
the moment.
*/

struct aimodule * GetPointerFromAIModuleIndex(int index)
{
	
	if(index>=0 && index<AIModuleArraySize)
	{
		return &AIModuleArray[index];
	}
	else
	{
		return NULL;
	}
}

int GetIndexFromAIModulePointer(struct aimodule* module)
{
	if(module)
		return module->m_index;
	else
		return -1;
}





void* GetPointerForSaveBlock(unsigned int size)
{
	void* retPointer;
	
	//see if we need to enlarge the buffer to allow for this block
	if(size > SaveInfo.BufferSpaceLeft)
	{
		//need more space
		int spaceNeeded = SaveInfo.BufferSpaceUsed + size;
		//buffer incremented in lots of 100000 bytes (more or less)
		int newBufferSize = ((spaceNeeded /100000)+1)*100000;

		//allocate the memory
		SaveInfo.BufferStart = (char*) realloc(SaveInfo.BufferStart,newBufferSize);

		//correct the position etc.
		SaveInfo.BufferPos = SaveInfo.BufferStart + SaveInfo.BufferSpaceUsed;
		SaveInfo.BufferSpaceLeft += (newBufferSize - SaveInfo.BufferSize);
		SaveInfo.BufferSize = newBufferSize;
	
	}

	//get the pointer to the next part of the save buffer
	retPointer = (void*)SaveInfo.BufferPos;

	SaveInfo.BufferSpaceUsed += size;
	SaveInfo.BufferSpaceLeft -= size;
	SaveInfo.BufferPos += size;

	return retPointer;
}



static void InitSaveGame()
{
	SaveInfo.BufferPos = SaveInfo.BufferStart;
	SaveInfo.BufferSpaceLeft = SaveInfo.BufferSize;
	SaveInfo.BufferSpaceUsed = 0;
	
}


static BOOL SaveGameAllowed()
{
	PLAYER_STATUS *playerStatusPtr;

	//can't save in multiplayer (or skirmish)
	if(AvP.Network != I_No_Network) return FALSE;

	//check player's state
	if(!Player) return FALSE;
	if(!Player->ObStrategyBlock) return FALSE;
	if(!Player->ObStrategyBlock->SBdataptr) return FALSE;

	playerStatusPtr = (PLAYER_STATUS*)Player->ObStrategyBlock->SBdataptr;

	//player must be alive , and not face hugged
	if(!playerStatusPtr->IsAlive) return FALSE;
	if(playerStatusPtr->MyFaceHugger) return FALSE;

	//cheating?
	if(DebuggingCommandsActive || CheatMode_Active!=CHEATMODE_NONACTIVE)
	{
		NewOnScreenMessage(GetTextString(TEXTSTRING_SAVEGAME_NOTALLOWED));
		return FALSE;
	}

	//now check the number of saves left
	//first some validation
	
	if(NumberOfSavesLeft<0) NumberOfSavesLeft =0;
	switch(AvP.Difficulty)
	{
		case I_Easy :
			if(NumberOfSavesLeft > NUM_SAVES_FOR_EASY_MODE) 
				NumberOfSavesLeft = NUM_SAVES_FOR_EASY_MODE;
			break;
		case I_Medium : 
			if(NumberOfSavesLeft > NUM_SAVES_FOR_MEDIUM_MODE) 
				NumberOfSavesLeft = NUM_SAVES_FOR_MEDIUM_MODE;
			break;
		case I_Hard :
		case I_Impossible :
			if(NumberOfSavesLeft > NUM_SAVES_FOR_HARD_MODE) 
				NumberOfSavesLeft = NUM_SAVES_FOR_HARD_MODE;
			break;
		default: ;
	}


	if(!NumberOfSavesLeft)
	{
		NewOnScreenMessage(GetTextString(TEXTSTRING_SAVEGAME_NOSAVESLEFT));
		return FALSE;
	}
	
	NumberOfSavesLeft--;


	//saving is allowed then
	return TRUE;


}

void SaveGame()
{
	char filename[100];
	FILE *file;
	int headerLength;
	HuffmanPackage *packagePtr;

	//make sure there is a save request
	if(SaveGameRequest ==  SAVELOAD_REQUEST_NONE) return;

	if(SaveGameRequest<0 || SaveGameRequest>=NUMBER_OF_SAVE_SLOTS)
	{
		SaveGameRequest = SAVELOAD_REQUEST_NONE;		
		return;
	}

	//check that we are allowed to save at this point
	if(!SaveGameAllowed())
	{
		//cancel request
		SaveGameRequest = SAVELOAD_REQUEST_NONE;
		return;	
	}

	InitSaveGame();

	SaveLevelHeader();

	headerLength = SaveInfo.BufferSpaceUsed;

	SaveDeadStrategies();

	SaveStrategies();

	SaveHiveSettings();

	SaveMarineSquadState();

	SaveMiscGlobalStuff();

	Save_SoundsWithNoReference();

	//save particles etc.
	Save_Decals();
	Save_Particles();
	Save_VolumetricExplosions();
	Save_PheromoneTrails();
	Save_LightElements();

	Save_MessageHistory();

	Save_WeaponsCGlobals();


	//get the filename
	GetFilenameForSaveSlot(SaveGameRequest,filename);
	
	//clear the save request
	SaveGameRequest = SAVELOAD_REQUEST_NONE;

	//write the file
 	file = OpenGameFile(filename, FILEMODE_WRITEONLY, FILETYPE_CONFIG);
	
	if (file == NULL)
	{
		GLOBALASSERT("Error saving file"==0);
		return;
	}

	fwrite(SaveInfo.BufferStart, headerLength, 1, file);

	packagePtr = HuffmanCompression(SaveInfo.BufferStart+headerLength,SaveInfo.BufferSpaceUsed-headerLength);

	fwrite(packagePtr, packagePtr->CompressedDataSize+sizeof(HuffmanPackage), 1, file);
	
	fclose(file);

	NewOnScreenMessage(GetTextString(TEXTSTRING_SAVEGAME_GAMESAVED));
	DisplaySavesLeft();
}



static void EndLoadGame()
{
	if(LoadInfo.BufferStart)
	{
		DeallocateMem(LoadInfo.BufferStart);
	}
	LoadInfo.BufferStart = NULL;
	LoadInfo.BufferPos = NULL;
	LoadInfo.BufferSize = 0;
	LoadInfo.BufferSpaceLeft = 0;
}

extern SAVE_SLOT_HEADER SaveGameSlot[];

extern int AlienEpisodeToPlay;
extern int MarineEpisodeToPlay;
extern int PredatorEpisodeToPlay;
BOOL ValidateLevelForLoadGameRequest(SAVE_SLOT_HEADER* save_slot)
{
	//see if we will need to change level
	if(save_slot->Species != AvP.PlayerType) return FALSE;

	//probably need to reload if in cheat mode
	if(CheatMode_Active!=CHEATMODE_NONACTIVE) return FALSE;
	
	switch(save_slot->Species)
	{
		case I_Marine :
			if(save_slot->Episode != MarineEpisodeToPlay) return FALSE;;
			break;

		case I_Alien :
			if(save_slot->Episode != AlienEpisodeToPlay) return FALSE;
			break;

		case I_Predator :
			if(save_slot->Episode != PredatorEpisodeToPlay) return FALSE;
			break;
	}
	//certainly need to change level if in multiplayer (or skirmish)
	if(AvP.Network != I_No_Network) return FALSE;

	return TRUE;
}

void LoadSavedGame()
{
	SAVE_SLOT_HEADER* save_slot;
	char filename[100];
	FILE *file;
	BOOL terminal_error = FALSE;

	if(LoadGameRequest == SAVELOAD_REQUEST_NONE) return;

	if(LoadGameRequest<0 || LoadGameRequest>=NUMBER_OF_SAVE_SLOTS)
	{
		LoadGameRequest = SAVELOAD_REQUEST_NONE;		
		return;
	}
	

	//get the save_slot
	save_slot = &SaveGameSlot[LoadGameRequest];

	//make sure the slot is being used
	ScanSaveSlots();
	if(!save_slot->SlotUsed) return;

	///make sure we're on the right level etc.
	if(!ValidateLevelForLoadGameRequest(save_slot))
	{
		AvP.MainLoopRunning = FALSE;
		return;
	}
	{
		extern int GlobalFrameCounter;
		if(!GlobalFrameCounter) return;
	}
	//set the difficulty level and restart the level
	AvP.Difficulty = save_slot->Difficulty;
	RestartLevel();

	//get the filename for the save slot
	GetFilenameForSaveSlot(LoadGameRequest,filename);

	//we can now clear the load request
	LoadGameRequest = SAVELOAD_REQUEST_NONE;

	//load the file
	file = OpenGameFile(filename, FILEMODE_READONLY, FILETYPE_CONFIG);

	if(file==NULL)
	{
		//failed to load
		EndLoadGame();
		return;
	}

	fseek(file, 0, SEEK_END);
	LoadInfo.BufferSize = ftell(file);
	rewind(file);
	
   	if(!LoadInfo.BufferSize)
	{
		fclose(file);
		EndLoadGame();
		return;
	}

	//allocate buffer , and read file into memory
	LoadInfo.BufferStart = (char*) AllocateMem(LoadInfo.BufferSize);
	fread(LoadInfo.BufferStart, LoadInfo.BufferSize, 1, file);
	fclose(file);

	
	LoadInfo.BufferPos = LoadInfo.BufferStart;
	LoadInfo.BufferSpaceLeft = LoadInfo.BufferSize;
	
	// attempt to access level header	
	{
		//read the next header
		SAVE_BLOCK_HEADER* header = (SAVE_BLOCK_HEADER*) LoadInfo.BufferPos;

		if(header->size>LoadInfo.BufferSpaceLeft)
		{
			//oh dear, dodgy file
			GLOBALASSERT("Invalid save game header size"==0);
			terminal_error = TRUE;
		}
		else
		{
			//go to the next header
			LoadInfo.BufferPos += header->size;
			LoadInfo.BufferSpaceLeft -= header->size;
	
			switch(header->type)
			{
				case SaveBlock_MainHeader :
					LoadLevelHeader(header);
					break;
				default:
					GLOBALASSERT("Unrecognized save block type"==0);
					terminal_error = TRUE;
					break;
			}
		}
	}
	
	if (!terminal_error)
	{
		if (!strncmp (LoadInfo.BufferPos, "REBCRIF1", 8))
		{
			char *uncompressedBufferPtr = (char*)HuffmanDecompress((HuffmanPackage*)(LoadInfo.BufferPos)); 		
			LoadInfo.BufferSpaceLeft = ((HuffmanPackage*)LoadInfo.BufferPos)->UncompressedDataSize;
			DeallocateMem(LoadInfo.BufferStart);
			LoadInfo.BufferStart=uncompressedBufferPtr;
			LoadInfo.BufferPos = LoadInfo.BufferStart;
		}
		else
		{
			terminal_error = TRUE;
		}
		
	}

	//go through loading things
	while(LoadInfo.BufferSpaceLeft && !terminal_error)
	{
		//read the next header
		SAVE_BLOCK_HEADER* header = (SAVE_BLOCK_HEADER*) LoadInfo.BufferPos;

		if(header->size>LoadInfo.BufferSpaceLeft)
		{
			//oh dear, dodgy file
			GLOBALASSERT("Invalid save game header size"==0);
			terminal_error = TRUE;
			break;
		}
		
		//go to the next header
		LoadInfo.BufferPos += header->size;
		LoadInfo.BufferSpaceLeft -= header->size;


		switch(header->type)
		{
			case SaveBlock_MainHeader :
				LoadLevelHeader(header);
				break;

			case SaveBlock_DeadStrategy :
				LoadDeadStrategy(header);
				break;

			case SaveBlock_Strategy :
				LoadStrategy((SAVE_BLOCK_STRATEGY_HEADER*) header);
				break;

			case SaveBlock_Track :
				//all these should be used up by the various strategy loaders
				GLOBALASSERT("Unexpected track save block"==0);
				break;

			case SaveBlock_Hierarchy :
				//all these should be used up by the various strategy loaders
				GLOBALASSERT("Unexpected hierarchy save block"==0);
				break;

			case SaveBlock_HierarchySection :
				//all these should be used up by the various strategy loaders
				GLOBALASSERT("Unexpected hierarchy section save block"==0);
				break;
			
			case SaveBlock_HierarchyTween :
				//all these should be used up by the various strategy loaders
				GLOBALASSERT("Unexpected hierarchy tween save block"==0);
				break;
			
			case SaveBlock_HierarchyDecals :
				//all these should be used up by the various strategy loaders
				GLOBALASSERT("Unexpected hierarchy decal save block"==0);
				break;

			case SaveBlock_HierarchyDelta :
				//all these should be used up by the various strategy loaders
				GLOBALASSERT("Unexpected hierarchy delta save block"==0);
				break;

			case SaveBlock_GlobalHive :
				LoadHiveSettings(header);
				break;

			case SaveBlock_MiscGlobal :
				LoadMiscGlobalStuff(header);
				break;

			case SaveBlock_MarineSquad :
				LoadMarineSquadState(header);
				break;

			case SaveBlock_Particles :
				Load_Particles(header);
				break;
			
			case SaveBlock_Decals :
				Load_Decals(header);
				break;

			case SaveBlock_PheromoneTrail :
				Load_PheromoneTrails(header);
				break;

			case SaveBlock_VolumetricExplosions :
				Load_VolumetricExplosions(header);
				break;

			case SaveBlock_LightElements :
				Load_LightElements(header);
				break;
			
			case SaveBlock_MessageHistory :
				Load_MessageHistory(header);
				break;

			case SaveBlock_WeaponsCGlobals :
				Load_WeaponsCGlobals(header);
				break;

			case SaveBlock_SoundState :
				Load_SoundState_NoRef(header);
				break;
				

			default :
				GLOBALASSERT("Unrecognized save block type"==0);
				terminal_error = TRUE;
				break;
		}
		

	}
	
	if(terminal_error)
	{
		//the save file was screwed , restart the level to be on the safe side
		RestartLevel();
		NewOnScreenMessage(GetTextString(TEXTSTRING_SAVEGAME_ERRORLOADING));
	}

	EndLoadGame();

	RemoveDestroyedStrategyBlocks();

	//make sure all the containing modules are set properly
	{
        extern int NumActiveStBlocks;
        extern STRATEGYBLOCK *ActiveStBlockList[];      

        int sbIndex = 0;
        STRATEGYBLOCK *sbPtr;

        /* loop thro' the strategy block list, looking for objects that need to have
        their visibilities managed ... */
        while(sbIndex < NumActiveStBlocks)
        {       
        	sbPtr = ActiveStBlockList[sbIndex++];
        	if(sbPtr->maintainVisibility && sbPtr->DynPtr)
			{
        		MODULE* newModule;
        		newModule = ModuleFromPosition(&(sbPtr->DynPtr->Position), (sbPtr->containingModule));                              

				if(newModule) sbPtr->containingModule = newModule;
			}
        }
	}

	AllNewModuleHandler();

	DoObjectVisibilities();

	ResetFrameCounter();

	if(!terminal_error)
	{
		NewOnScreenMessage(GetTextString(TEXTSTRING_SAVEGAME_GAMELOADED));
		DisplaySavesLeft();
	}

}

SAVE_BLOCK_HEADER* GetNextBlockIfOfType(SAVE_BLOCK_TYPE type)
{
	SAVE_BLOCK_HEADER* header;

	GLOBALASSERT(LoadInfo.BufferPos);

	if(LoadInfo.BufferSpaceLeft==0)
	{
		//no more blocks left
		return 0;
	}
	
	//look at the next header in the buffer
	header = (SAVE_BLOCK_HEADER*) LoadInfo.BufferPos;
	if(header->size>LoadInfo.BufferSpaceLeft)
	{
		//oh dear, dodgy file
		GLOBALASSERT("Invalid save game header size"==0);
		return 0;
	}

	//see if the header is of the type that we are after
	if(header->type == type)
	{
		//okay , advance the buffer position , and return this header
		LoadInfo.BufferPos += header->size;
		LoadInfo.BufferSpaceLeft -= header->size;

		return header;
	}
	else
	{
		//out of luck
		return 0;
	}

}

static void SaveStrategies()
{
	int i;

	for(i=0;i<NumActiveStBlocks;i++)
	{
		STRATEGYBLOCK* sbPtr = ActiveStBlockList[i];

		switch(sbPtr->I_SBtype)
		{
			case I_BehaviourMarinePlayer :
				SaveStrategy_Player(sbPtr);
				break;
			
			case I_BehaviourLiftDoor :
				SaveStrategy_LiftDoor(sbPtr);
				break;

			case I_BehaviourProximityDoor :
				SaveStrategy_ProxDoor(sbPtr);
				break;

			case I_BehaviourSwitchDoor :
				SaveStrategy_SwitchDoor(sbPtr);
				break;

			case I_BehaviourPlatform :
				SaveStrategy_PlatformLift(sbPtr);
				break;

			case I_BehaviourBinarySwitch :
				SaveStrategy_BinarySwitch(sbPtr);
				break;

			case I_BehaviourLinkSwitch :
				SaveStrategy_LinkSwitch(sbPtr);
				break;

			case I_BehaviourGenerator :
				SaveStrategy_Generator(sbPtr);
				break;

			case I_BehaviourInanimateObject :
				SaveStrategy_InanimateObject(sbPtr);
				break;
	
			case I_BehaviourLightFX :
				SaveStrategy_LightFx(sbPtr);
				break;

			case I_BehaviourPlacedSound :
				SaveStrategy_PlacedSound(sbPtr);
				break;

			case I_BehaviourMessage :
				SaveStrategy_Message(sbPtr);
				break;

			case I_BehaviourMissionComplete :
				SaveStrategy_MissionComplete(sbPtr);
				break;

			case I_BehaviourTrackObject :
				SaveStrategy_TrackObject(sbPtr);
				break;

			case I_BehaviourFan :
				SaveStrategy_Fan(sbPtr);
				break;

			case I_BehaviourPlacedLight :
				SaveStrategy_PlacedLight(sbPtr);
				break;

			case I_BehaviourVideoScreen :
				SaveStrategy_VideoScreen(sbPtr);
				break;

			case I_BehaviourSelfDestruct :
				SaveStrategy_SelfDestruct(sbPtr);
				break;

			case I_BehaviourParticleGenerator :
				SaveStrategy_ParticleGenerator(sbPtr);
				break;

			case I_BehaviourDeathVolume :
				SaveStrategy_DeathVolume(sbPtr);
				break;

			case I_BehaviourAlien :
			   	SaveStrategy_Alien(sbPtr);
				break;

			case I_BehaviourNetCorpse :
			   	SaveStrategy_Corpse(sbPtr);
				break;

			case I_BehaviourFaceHugger :
			   	SaveStrategy_FaceHugger(sbPtr);
				break;

			case I_BehaviourMarine :
			   	SaveStrategy_Marine(sbPtr);
				break;

			case I_BehaviourPlacedHierarchy :
			   	SaveStrategy_PlacedHierarchy(sbPtr);
				break;

			case I_BehaviourPredator :
			   	SaveStrategy_Predator(sbPtr);
				break;

			case I_BehaviourXenoborg :
			   	SaveStrategy_Xenoborg(sbPtr);
				break;

			case I_BehaviourQueenAlien :
			   	SaveStrategy_Queen(sbPtr);
				break;
		
			case I_BehaviourAutoGun :
			   	SaveStrategy_Autogun(sbPtr);
				break;
	
			case I_BehaviourHierarchicalFragment :
			   	SaveStrategy_HierarchicalDebris(sbPtr);
				break;

			case I_BehaviourGrenade :
				SaveStrategy_Grenade(sbPtr);
				break;

			case I_BehaviourFlareGrenade :
				SaveStrategy_FlareGrenade(sbPtr);
				break;

			case I_BehaviourProximityGrenade :
				SaveStrategy_ProxGrenade(sbPtr);
				break;

			case I_BehaviourRocket :
				SaveStrategy_Rocket(sbPtr);
				break;

			case I_BehaviourPPPlasmaBolt :
				SaveStrategy_PPPlasmaBolt(sbPtr);
				break;

			case I_BehaviourPredatorEnergyBolt :
				SaveStrategy_PredatorEnergyBolt(sbPtr);
				break;

			case I_BehaviourFrisbeeEnergyBolt :
				SaveStrategy_FrisbeeEnergyBolt(sbPtr);
				break;

			case I_BehaviourPulseGrenade :
				SaveStrategy_PulseGrenade(sbPtr);
				break;

			case I_BehaviourClusterGrenade :
				SaveStrategy_ClusterGrenade(sbPtr);
				break;

			case I_BehaviourMolotov :
				SaveStrategy_Molotov(sbPtr);
				break;

			case I_BehaviourPredatorDisc_SeekTrack :
				SaveStrategy_PredatorDisc(sbPtr);
				break;
			
			case I_BehaviourSpeargunBolt :
				SaveStrategy_SpearBolt(sbPtr);
				break;

			case I_BehaviourGrapplingHook :
				SaveStrategy_Grapple(sbPtr);
				break;

			case I_BehaviourFragment :
				SaveStrategy_Debris(sbPtr);
				break;
	
			case I_BehaviourFrisbee :
				SaveStrategy_Frisbee(sbPtr);
				break;
			
			default: ;
		}
	}
	
}

static void LoadStrategy(SAVE_BLOCK_STRATEGY_HEADER* header)
{
	switch(header->bhvr_type)
	{
		case I_BehaviourMarinePlayer :
			LoadStrategy_Player(header);
			break;

		case I_BehaviourLiftDoor :
			LoadStrategy_LiftDoor(header);
			break;

		case I_BehaviourProximityDoor :
			LoadStrategy_ProxDoor(header);
			break;

		case I_BehaviourSwitchDoor :
			LoadStrategy_SwitchDoor(header);
			break;

		case I_BehaviourPlatform :
			LoadStrategy_PlatformLift(header);
			break;

		case I_BehaviourBinarySwitch :
			LoadStrategy_BinarySwitch(header);
			break;

		case I_BehaviourLinkSwitch :
			LoadStrategy_LinkSwitch(header);
			break;

		case I_BehaviourGenerator :
			LoadStrategy_Generator(header);
			break;

		case I_BehaviourInanimateObject :
			LoadStrategy_InanimateObject(header);
			break;
		
		case I_BehaviourLightFX :
			LoadStrategy_LightFx(header);
			break;

		case I_BehaviourPlacedSound :
			LoadStrategy_PlacedSound(header);
			break;

		case I_BehaviourMessage :
			LoadStrategy_Message(header);
			break;

		case I_BehaviourMissionComplete :
			LoadStrategy_MissionComplete(header);
			break;

		case I_BehaviourTrackObject :
			LoadStrategy_TrackObject(header);
			break;

		case I_BehaviourFan :
			LoadStrategy_Fan(header);
			break;
	
		case I_BehaviourPlacedLight :
			LoadStrategy_PlacedLight(header);
			break;

		case I_BehaviourVideoScreen :
			LoadStrategy_VideoScreen(header);
			break;

		case I_BehaviourSelfDestruct :
			LoadStrategy_SelfDestruct(header);
			break;

		case I_BehaviourParticleGenerator :
			LoadStrategy_ParticleGenerator(header);
			break;

		case I_BehaviourDeathVolume :
			LoadStrategy_DeathVolume(header);
			break;

		case I_BehaviourAlien :
			LoadStrategy_Alien(header);
			break;

		case I_BehaviourNetCorpse :
		   	LoadStrategy_Corpse(header);
			break;

		case I_BehaviourFaceHugger :
		   	LoadStrategy_FaceHugger(header);
			break;

		case I_BehaviourMarine :
		   	LoadStrategy_Marine(header);
			break;
		
		case I_BehaviourPlacedHierarchy :
		   	LoadStrategy_PlacedHierarchy(header);
			break;

		case I_BehaviourPredator :
		   	LoadStrategy_Predator(header);
			break;
	
		case I_BehaviourXenoborg :
		   	LoadStrategy_Xenoborg(header);
			break;

		case I_BehaviourQueenAlien :
		   	LoadStrategy_Queen(header);
			break;
	
		case I_BehaviourAutoGun :
		   	LoadStrategy_Autogun(header);
			break;

		case I_BehaviourHierarchicalFragment :
		   	LoadStrategy_HierarchicalDebris(header);
			break;

		case I_BehaviourGrenade :
			LoadStrategy_Grenade(header);
			break;

		case I_BehaviourFlareGrenade :
			LoadStrategy_FlareGrenade(header);
			break;

		case I_BehaviourProximityGrenade :
			LoadStrategy_ProxGrenade(header);
			break;

		case I_BehaviourRocket :
			LoadStrategy_Rocket(header);
			break;

		case I_BehaviourPPPlasmaBolt :
			LoadStrategy_PPPlasmaBolt(header);
			break;

		case I_BehaviourPredatorEnergyBolt :
			LoadStrategy_PredatorEnergyBolt(header);
			break;

		case I_BehaviourFrisbeeEnergyBolt :
			LoadStrategy_FrisbeeEnergyBolt(header);
			break;

		case I_BehaviourPulseGrenade :
			LoadStrategy_PulseGrenade(header);
			break;

		case I_BehaviourClusterGrenade :
			LoadStrategy_ClusterGrenade(header);
			break;

		case I_BehaviourMolotov :
			LoadStrategy_Molotov(header);
			break;

		case I_BehaviourPredatorDisc_SeekTrack :
			LoadStrategy_PredatorDisc(header);
			break;

		case I_BehaviourSpeargunBolt :
			LoadStrategy_SpearBolt(header);
			break;

		case I_BehaviourGrapplingHook :
			LoadStrategy_Grapple(header);
			break;

		case I_BehaviourFragment :
			LoadStrategy_Debris(header);
			break;

		case I_BehaviourFrisbee :
			LoadStrategy_Frisbee(header);
			break;
		
		default: ;
	}
}


/*------------------------------------**
** Loading and saving dead strategies **
**------------------------------------*/

typedef struct dead_strategy_save_block
{
	SAVE_BLOCK_HEADER header;
	char SBname[SB_NAME_LENGTH];

}DEAD_STRATEGY_SAVE_BLOCK;

static void SaveDeadStrategies()
{
	extern STRATEGYBLOCK FreeStBlockData[];
	int i;
	STRATEGYBLOCK* sbPtr = &FreeStBlockData[0];

	//search for all the strategies that existed at the start , and have been destroyed
	for(i=0;i<maxstblocks;i++ , sbPtr++)
	{
		if(sbPtr->SBflags.destroyed_but_preserved)
		{
			DEAD_STRATEGY_SAVE_BLOCK* block;	
			GET_SAVE_BLOCK_POINTER(block);

			block->header.type = SaveBlock_DeadStrategy;
			block->header.size = sizeof(*block);

			COPY_NAME(block->SBname,sbPtr->SBname);
			
		}	
	}
}

static void LoadDeadStrategy(SAVE_BLOCK_HEADER* header)
{
	DEAD_STRATEGY_SAVE_BLOCK* block = (DEAD_STRATEGY_SAVE_BLOCK*) header;

	STRATEGYBLOCK* sbPtr = FindSBWithName(block->SBname);
	if(sbPtr)
	{
		DestroyAnyStrategyBlock(sbPtr);
	}
}



/*--------------------------------------------------------------------------------**
** Loading and saving miscellaneos global rubbish that  has nowhere else to live **
**--------------------------------------------------------------------------------*/

extern unsigned int IncrementalSBname;
extern int PlayersMaxHeightWhilstNotInContactWithGround;


typedef struct misc_global_save_block
{
	SAVE_BLOCK_HEADER header;

	unsigned int IncrementalSBname;
	AvP_GameStats CurrentGameStatistics;
	int PlayersMaxHeightWhilstNotInContactWithGround;
	int  NumberOfSavesLeft;

	int FMV_MessageNumber; 
	int FMV_FrameNumber;

}MISC_GLOBAL_SAVE_BLOCK;

static void LoadMiscGlobalStuff(SAVE_BLOCK_HEADER* header)
{
	MISC_GLOBAL_SAVE_BLOCK* block;
	block = (MISC_GLOBAL_SAVE_BLOCK*) header;

	if(block->header.size != sizeof(*block)) return;

	IncrementalSBname = block->IncrementalSBname;
	CurrentGameStatistics = block->CurrentGameStatistics;
	PlayersMaxHeightWhilstNotInContactWithGround = block->PlayersMaxHeightWhilstNotInContactWithGround;
	NumberOfSavesLeft = block-> NumberOfSavesLeft;

	StartFMVAtFrame(block->FMV_MessageNumber,block->FMV_FrameNumber);
}

static void SaveMiscGlobalStuff()
{
	MISC_GLOBAL_SAVE_BLOCK* block;
	block = GetPointerForSaveBlock(sizeof(*block));

	//fill in the header
	block->header.type = SaveBlock_MiscGlobal;
	block->header.size = sizeof(*block);

	block->IncrementalSBname = IncrementalSBname;
	block->CurrentGameStatistics = CurrentGameStatistics;
	block->PlayersMaxHeightWhilstNotInContactWithGround = PlayersMaxHeightWhilstNotInContactWithGround;
	block->NumberOfSavesLeft =  NumberOfSavesLeft;

	GetFMVInformation(&block->FMV_MessageNumber,&block->FMV_FrameNumber);
}

extern void DisplaySavesLeft()
{
	char text [100];

	sprintf(text, "%s: %d",GetTextString(TEXTSTRING_SAVEGAME_SAVESLEFT),NumberOfSavesLeft);

	NewOnScreenMessage(text);
}

extern void ResetNumberOfSaves()
{
	switch(AvP.Difficulty)
	{
		case I_Easy :
			NumberOfSavesLeft = NUM_SAVES_FOR_EASY_MODE;
			break;
		case I_Medium : 
			NumberOfSavesLeft = NUM_SAVES_FOR_MEDIUM_MODE;
			break;
		case I_Hard :
		case I_Impossible :
			NumberOfSavesLeft = NUM_SAVES_FOR_HARD_MODE;
			break;
		default:
			break;
	}
}
