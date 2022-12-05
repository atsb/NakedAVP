#include "npcsetup.h"
#include "ourasert.h"
#include "list_tem.hpp"
#include "chunkpal.hpp"
#include "chnkload.hpp"
#include "projload.hpp"
#include "envchunk.hpp"
#include "avpchunk.hpp"
#include "strachnk.hpp"
#include "gamedef.h"
#include "progress_bar.h"
#include "scream.h"
#include "avp_menus.h"


#if ALIEN_DEMO
#define DIRECTORY_FOR_RIFS "alienavp_huds/"
#else
#define DIRECTORY_FOR_RIFS "avp_huds/"
#endif
#define FIRST_FREE_IMAGE_GROUP 3 // 0 for char,1 for weapon rif ,2 for env

#if debug
extern "C"
{
	BOOL ForceLoad_Alien=FALSE;
	BOOL ForceLoad_Marine=FALSE;
	BOOL ForceLoad_Predator=FALSE;
	BOOL ForceLoad_Hugger=FALSE;
	BOOL ForceLoad_Queen=FALSE;
	BOOL ForceLoad_Civvie=FALSE;
	BOOL ForceLoad_PredAlien=FALSE;
	BOOL ForceLoad_Xenoborg=FALSE;
	BOOL ForceLoad_Pretorian=FALSE;
	BOOL ForceLoad_SentryGun=FALSE;

	extern BOOL KeepMainRifFile;

};
#endif

static char Marine_File[]= "hnpcmarine.rif";
static char Alien_File[]= "hnpcalien.rif";
static char Predator_File[]= "hnpcpredator.rif";
static char Hugger_File[]= "hnpchugger.rif";
static char AlienQueen_File[]= "queen.rif";
static char Civilian_File[]= "hnpc_civvie.rif";
static char PredAlien_File[]= "hnpcpred_alien.rif";
static char Xenoborg_File[]= "hnpc_xenoborg.rif";
static char Pretorian_File[]= "hnpcpretorian.rif";
static char SentryGun_File[]= "sentry.rif";

static char* HNPC_FileNames[]=
{
	&Marine_File[0],
	&Alien_File[0],
	&Predator_File[0],
	&Hugger_File[0],
	&AlienQueen_File[0],
	&Civilian_File[0],
	&PredAlien_File[0],
	&Xenoborg_File[0],
	&Pretorian_File[0],
	&SentryGun_File[0],
};

typedef enum
{
	HNPC_Marine,	
	HNPC_Alien,	
	HNPC_Predator,	
	HNPC_Hugger,	
	HNPC_AlienQueen,
	HNPC_Civilian,
	HNPC_PredAlien,
	HNPC_Xenoborg,
	HNPC_Pretorian,
	HNPC_SentryGun,
	HNPC_Last,	
}HNPC_Files;

static BOOL Load_HNPC[HNPC_Last];


class LoadedNPC
{
public:		
	LoadedNPC(RIF_Child_Chunk const *);
	LoadedNPC(char const *);
	LoadedNPC();
	LoadedNPC(LoadedNPC const &);
	~LoadedNPC();
	LoadedNPC & operator = (LoadedNPC const &);
	
	void Load(int progress_start,int progress_interval);
	void Unload(); // not called automatically
	inline int IsValid()
		{ return filename ? 1 : 0; }
	
	int operator == (LoadedNPC const &) const;
	inline int operator != (LoadedNPC const & npc2) const
		{ return ! operator == (npc2); }
	
private:
	char * rifname;
	char * filename;
	int img_group;
	
	RIFFHANDLE npc_rif;
	static List<int> image_groups;
};
	
List<LoadedNPC> loaded_npcs;
	
LoadedNPC::LoadedNPC(RIF_Child_Chunk const * rcc)
: rifname(0)
, filename(0)
, img_group(-1)
, npc_rif(INVALID_RIFFHANDLE)
{
	if (rcc->filename && *rcc->filename)
	{
		char const * fnptr = rcc->filename;
		if (strchr(fnptr,'\\')) fnptr = strrchr(fnptr,'\\')+1;
		if (strchr(fnptr,'/')) fnptr = strrchr(fnptr,'/')+1;
		if (strchr(fnptr,':')) fnptr = strrchr(fnptr,':')+1;
		
		if (!_strnicmp(fnptr,"npc",3) || 
			!_strnicmp (fnptr, "hnpc", 4) ||
			!_strnicmp (fnptr, "loof", 4) ||
			!_strnicmp (fnptr, "marwep", 6) ||
			!_strnicmp (fnptr, "pred_hud", 8) ||
			!_strnicmp (fnptr, "bmantest", 8)
			) // all thes files names start with 'npc'
		{
			filename = new char[strlen(DIRECTORY_FOR_RIFS)+strlen(fnptr)+1];
			strcpy(filename,DIRECTORY_FOR_RIFS);
			strcat(filename,fnptr);

			rifname = new char[strlen(rcc->rifname)+1];
			strcpy(rifname,rcc->rifname);
		}
	}
}
LoadedNPC::LoadedNPC(char const * name)
: rifname(0)
, filename(0)
, img_group(-1)
, npc_rif(INVALID_RIFFHANDLE)
{
	if (name && *name)
	{
		char const * fnptr = name;
		if (strchr(fnptr,'\\')) fnptr = strrchr(fnptr,'\\')+1;
		if (strchr(fnptr,'/')) fnptr = strrchr(fnptr,'/')+1;
		if (strchr(fnptr,':')) fnptr = strrchr(fnptr,':')+1;
		
		filename = new char[strlen(DIRECTORY_FOR_RIFS)+strlen(fnptr)+1];
		strcpy(filename,DIRECTORY_FOR_RIFS);
		strcat(filename,fnptr);

		rifname = new char[strlen(name)+1];
		strcpy(rifname,name);
		
	}
}

LoadedNPC::LoadedNPC()
: rifname(0)
, filename(0)
, img_group(-1)
, npc_rif(INVALID_RIFFHANDLE)
{
}

LoadedNPC::LoadedNPC(LoadedNPC const & npc2)
: rifname(npc2.rifname ? new char[strlen(npc2.rifname)+1] : 0)
, filename(npc2.filename ? new char[strlen(npc2.filename)+1] : 0)
, img_group(npc2.img_group)
, npc_rif(npc2.npc_rif)
{
	if (rifname) strcpy(rifname,npc2.rifname);
	if (filename) strcpy(filename,npc2.filename);
}

LoadedNPC::~LoadedNPC()
{
	if (rifname) delete[] rifname;
	if (filename) delete[] filename;
}

LoadedNPC & LoadedNPC::operator = (LoadedNPC const & npc2)
{
	if (&npc2 != this)
	{
		if (rifname) delete[] rifname;
		if (filename) delete[] filename;

		rifname = npc2.rifname ? new char[strlen(npc2.rifname)+1] : 0;
		filename = npc2.filename ? new char[strlen(npc2.filename)+1] : 0;
		img_group = npc2.img_group;
		npc_rif = npc2.npc_rif;

		if (rifname) strcpy(rifname,npc2.rifname);
		if (filename) strcpy(filename,npc2.filename);
	}
	return *this;
}

int LoadedNPC::operator == (LoadedNPC const & npc2) const
{
	return !_stricmp(rifname ? rifname : "",npc2.rifname ? npc2.rifname : "");
}

void LoadedNPC::Load(int progress_start,int progress_interval)
{
	Set_Progress_Bar_Position(progress_start);
	if (filename)
	{
		npc_rif = avp_load_rif_non_env(filename);
		if (INVALID_RIFFHANDLE != npc_rif)
		{
			#if MaxImageGroups>1
			for (img_group=FIRST_FREE_IMAGE_GROUP; image_groups.contains(img_group); ++img_group)
				;
			GLOBALASSERT(img_group<MaxImageGroups);
			image_groups.add_entry(img_group);
			SetCurrentImageGroup(img_group); // PROBLEM AT PRESENT, copy_rif_data changes this
			#endif
			Set_Progress_Bar_Position(progress_start+progress_interval/2);
			copy_rif_data(npc_rif,CCF_IMAGEGROUPSET + CCF_LOAD_AS_HIERARCHY_IF_EXISTS,progress_start+progress_interval/2,progress_interval/2);
			unload_rif(npc_rif); // doesnt destroy copied data
		}
	}
}

void LoadedNPC::Unload()
{
	if (INVALID_RIFFHANDLE != npc_rif)
	{
		#if MaxImageGroups>1
		image_groups.delete_entry(img_group);
		SetCurrentImageGroup(img_group);
		DeallocateCurrentImages();
		#endif
		avp_undo_rif_load(npc_rif); // destroys copied shapes
	}
	npc_rif = INVALID_RIFFHANDLE;
}


static BOOL MarineIsNomcombatant(AVP_Generator_Chunk* agc)
{
	AVP_Generator_Extra_Data_Chunk* agedc=agc->get_extra_data_chunk();
	if(agedc)
	{
		AVP_Strategy_Chunk* asc =(AVP_Strategy_Chunk*) agedc->lookup_single_child("AVPSTRAT");
		if(asc)
		{
			if(asc->Strategy && asc->Strategy->StrategyType==StratEnemy)
			{
				EnemyStrategy* es=(EnemyStrategy*)asc->Strategy;
				if(es->MissionType==3)
				{
					return TRUE;
				}
					
			}
		}
	}
	return FALSE;
}


List<int> LoadedNPC::image_groups;

extern "C"
{
	extern BOOL Current_Level_Requires_Mirror_Image();
    extern int AllowGoldWeapons;
};

void InitNPCs(RIFFHANDLE h)
{
	
	File_Chunk * old_env_chunk = Env_Chunk; // push Env_Chunk
	
	
	// build list of objects for the npcs in this level

	for(int i=0;i<HNPC_Last;i++)
	{
		Load_HNPC[i]=FALSE;
	}

	#if debug
	if(ForceLoad_Alien)Load_HNPC[HNPC_Alien]=TRUE;
	if(ForceLoad_Marine)Load_HNPC[HNPC_Marine]=TRUE;
	if(ForceLoad_Predator)Load_HNPC[HNPC_Predator]=TRUE;
	if(ForceLoad_Hugger)Load_HNPC[HNPC_Hugger]=TRUE;
	if(ForceLoad_Queen)Load_HNPC[HNPC_AlienQueen]=TRUE;
	if(ForceLoad_Civvie)Load_HNPC[HNPC_Civilian]=TRUE;
	if(ForceLoad_PredAlien)Load_HNPC[HNPC_PredAlien]=TRUE;
	if(ForceLoad_Xenoborg)Load_HNPC[HNPC_Xenoborg]=TRUE;
	if(ForceLoad_Pretorian)Load_HNPC[HNPC_Pretorian]=TRUE;
	if(ForceLoad_SentryGun)Load_HNPC[HNPC_SentryGun]=TRUE;
	#endif
	
	HNPC_Files DefaultGeneratorEnemy;
	
	Chunk * pChunk = ((Chunk_With_Children*)h->envd)->lookup_single_child("GLOGENDC");
	if(pChunk)
	{
		Global_Generator_Data_Chunk* ggdc=(Global_Generator_Data_Chunk*)pChunk;
		switch(ggdc->EnemyGenerated)
		{
			case Generate_Aliens :
				DefaultGeneratorEnemy=HNPC_Alien;
				break;
			case Generate_Marines :
				DefaultGeneratorEnemy=HNPC_Marine;
				break;					
			default :
				DefaultGeneratorEnemy=HNPC_Marine;
				GLOBALASSERT("Invalid enemy type"==0);
		}
	}
	else
	{
		DefaultGeneratorEnemy=HNPC_Alien;
	}

	if(Current_Level_Requires_Mirror_Image())
	{
		//need to load model for player's character , for mirror
		switch(AvP.PlayerType)
		{
			case I_Marine :
				Load_HNPC[HNPC_Marine]=TRUE;
				break;

			case I_Alien :
				Load_HNPC[HNPC_Alien]=TRUE;
				break;
			
			case I_Predator :
				Load_HNPC[HNPC_Predator]=TRUE;
				break;
		}
	}

	

	if(AvP.Network != I_No_Network)
	{
		/* KJL 19:35:46 01/07/98 - multiplayer only needs the 3 main characters */
		Load_HNPC[HNPC_Alien]=TRUE;
		Load_HNPC[HNPC_Predator]=TRUE;
		Load_HNPC[HNPC_Marine]=TRUE;
	}
	
	
	/* KJL 16:31:03 06/05/98 - Force all characters to be loaded 
	for testing of 'bots etc. */
	
	#if 0 //characters can now be loaded with command line options
	Load_HNPC[HNPC_Alien]=TRUE;
	Load_HNPC[HNPC_Predator]=TRUE;
	Load_HNPC[HNPC_Marine]=TRUE;
	Load_HNPC[HNPC_PredAlien] =TRUE;
	#endif

	Special_Objects_Chunk * soc = 0;
 	soc = (Special_Objects_Chunk *)((Chunk_With_Children*)h->envd)->lookup_single_child ("SPECLOBJ");
	
	if (soc)
	{
		List<Chunk *> cl;
		soc->lookup_child("AVPGENER",cl);
		for (LIF<Chunk *> cli(&cl); !cli.done(); cli.next())
		{
			AVP_Generator_Chunk * agc = (AVP_Generator_Chunk *)cli();
		
			if (agc->type)
			{
				#if 0 //forget about game mode settings for generators / badguys
				if(AvP.PlayerType==I_Alien && (agc->flags & AVPGENFLAG_AVPGAMEMODEALIEN)||
				   AvP.PlayerType==I_Marine && (agc->flags & AVPGENFLAG_AVPGAMEMODEMARINE)||
				   AvP.PlayerType==I_Predator && (agc->flags & AVPGENFLAG_AVPGAMEMODEPREDATOR))
				#endif
				{
					switch (agc->type)
					{
						case 1:
							Load_HNPC[HNPC_Alien]=TRUE;
							break;
						case 2:
							if(AvP.Network == I_No_Network)
								Load_HNPC[HNPC_Predator]=TRUE;
							break;
						
						case 3:
							if(AvP.Network == I_No_Network)
								Load_HNPC[HNPC_Hugger]=TRUE;
							break;
						case 4:
							if(AvP.Network == I_No_Network)
								Load_HNPC[HNPC_Xenoborg]=TRUE;
							break;
						case 5:
							Load_HNPC[HNPC_PredAlien] =TRUE;
							break;
						case 6:
							if(AvP.Network == I_No_Network)
								Load_HNPC[HNPC_AlienQueen]=TRUE;
							break;
						case 7:	//marine or variant of marine
							if(AvP.Network == I_No_Network)
							{
								switch (agc->sub_type)
								{
									case 0:	//pulse rifle
									case 5:	//pistol marine
									case 10: //smartgun
									case 20://grenade launcher
									case 50://flamethrower
									case 60://sadar
									case 70://Minigun
										Load_HNPC[HNPC_Marine]=TRUE;
										if(!Load_HNPC[HNPC_Civilian])
										{
											//noncombatant marines become civilians
											if(MarineIsNomcombatant(agc))
											{
												Load_HNPC[HNPC_Civilian]=TRUE;											
			
											}
										}
										break;
	
	
									case 30://shotgun 1
									case 31://shotgun 2
									case 32://shotgun 3
									case 40://pistol 1
									case 41://pistol 2
									case 80://Molotov
									case 90://Scientist1
									case 91://Scientist2
									case 100://civilian flamer
									case 110://civilian unarmed
									case 120://android
										Load_HNPC[HNPC_Civilian] =TRUE;
										break;
		
									
									default:
										Load_HNPC[HNPC_Marine]=TRUE;
										break;
								}
							}
							break;
						case 8:
							Load_HNPC[HNPC_Pretorian]=TRUE;
							break;
						case 9:
							if(AvP.Network == I_No_Network)
								Load_HNPC[HNPC_SentryGun]=TRUE;
							break;
						default:
							break;
					}
					
				
				}
			}
			else
			{
				//see if generator is a multiplayer start position
				if(agc->flags & AVPGENFLAG_MULTIPLAYERSTART)
				{
					continue;
				}
				
				//this is a generator , look to see what it generates
				AVP_Generator_Extended_Settings_Chunk* setting=agc->get_extended_settings();
				if(setting)
				{
					BOOL all_zero=TRUE;
					if(AvP.Network == I_No_Network)
					{
						if(setting->weights->PulseMarine_Wt ||
						   setting->weights->PistolMarine_Wt ||
						   setting->weights->FlameMarine_Wt ||
						   setting->weights->SmartMarine_Wt ||
						   setting->weights->SadarMarine_Wt ||
						   setting->weights->GrenadeMarine_Wt ||
						   setting->weights->MinigunMarine_Wt)
						{
							Load_HNPC[HNPC_Marine]=TRUE;
							all_zero=FALSE;
						}
						if(setting->weights->ShotgunCiv_Wt ||
						   setting->weights->PistolCiv_Wt ||
						   setting->weights->FlameCiv_Wt ||
						   setting->weights->UnarmedCiv_Wt ||
						   setting->weights->MolotovCiv_Wt)
						{
							Load_HNPC[HNPC_Civilian]=TRUE;
							all_zero=FALSE;
						}
					}
					
					if(setting->weights->Alien_Wt)
					{
						Load_HNPC[HNPC_Alien]=TRUE;
						all_zero=FALSE;
					}
					if(setting->weights->PredAlien_Wt)
					{
						Load_HNPC[HNPC_PredAlien]=TRUE;
						all_zero=FALSE;
					}
					if(setting->weights->Praetorian_Wt)
					{
						Load_HNPC[HNPC_Pretorian]=TRUE;
						all_zero=FALSE;
					}

					if(all_zero)
					{
						//all the weightings are zero , use the default badguy
						Load_HNPC[DefaultGeneratorEnemy]=TRUE;
					}


				}
				else
				{
					//no extended generator data , use the default badguy
					Load_HNPC[DefaultGeneratorEnemy]=TRUE;
				}
			}
		}
	}
	
	List<LoadedNPC> newnpcs;

	for(int i=0;i<HNPC_Last;i++)
	{
		if(Load_HNPC[i])
		{
			LoadedNPC tempnpc(HNPC_FileNames[i]);
			if (tempnpc.IsValid())
			{
				newnpcs.add_entry(tempnpc);
			}
		}
	}

/* predator disk not included in demos */
#if !(PREDATOR_DEMO||MARINE_DEMO||ALIEN_DEMO)
	if(AvP.PlayerType==I_Predator || Load_HNPC[HNPC_Predator])
	{
		//need to load the disk hierarchy
		LoadedNPC tempnpc("disk.rif");
		if (tempnpc.IsValid())
		{
			newnpcs.add_entry(tempnpc);
		}

	}
#endif

	if(AvP.PlayerType==I_Marine || Load_HNPC[HNPC_Marine])
	{
        // if the mdisk.rif file exists, add it.  Note: Only the Gold version
        // has this file, so the OpenGameFile is called just to check if it
        // is available.
        FILE *rifFile = OpenGameFile(DIRECTORY_FOR_RIFS"mdisk.rif", 
                FILEMODE_READONLY, FILETYPE_PERM);
        if (rifFile != NULL)
        {
            CloseGameFile(rifFile);

            //need to load the mdisk hierarchy
            LoadedNPC tempnpc("mdisk.rif");
            if (tempnpc.IsValid())
            {
                AllowGoldWeapons = 1;
                newnpcs.add_entry(tempnpc);
            }
        }
	}
	
	// see what we already have, unloading what we don't need, and ensuring we don't load a npc twice
	for (LIF<LoadedNPC> i_loaded_npc(&loaded_npcs); !i_loaded_npc.done(); )
	{
		if (newnpcs.contains(i_loaded_npc()))
		{
			newnpcs.delete_entry(i_loaded_npc());
			i_loaded_npc.next();
		}
		else
		{
			LoadedNPC tounload(i_loaded_npc());
			tounload.Unload();
			i_loaded_npc.delete_current();
		}
	}

#if debug	
	if(!KeepMainRifFile)
#endif	
	{
		//at this point we no longer need the main level rif file
		unload_rif(h);
	}
	
	// load the new ones, adding them to the main list
	int NumToLoad=newnpcs.size();
	int NumLoaded=0;
	
	if(Load_HNPC[HNPC_Marine] || Load_HNPC[HNPC_Civilian] || AvP.PlayerType==I_Marine)
	{
		LoadMarineScreamSounds();
	}
	if(Load_HNPC[HNPC_Predator] ||AvP.PlayerType==I_Predator)
	{
		LoadPredatorScreamSounds();
	}
	if(Load_HNPC[HNPC_Alien] || Load_HNPC[HNPC_PredAlien]|| Load_HNPC[HNPC_Pretorian] || AvP.PlayerType==I_Alien)
	{
		LoadAlienScreamSounds();
	}
	if(Load_HNPC[HNPC_AlienQueen])
	{
		LoadQueenScreamSounds();
	}
	
	for (LIF<LoadedNPC> i_newnpc(&newnpcs); !i_newnpc.done(); i_newnpc.next())
	{
		LoadedNPC toload(i_newnpc());
		toload.Load(PBAR_NPC_START+(PBAR_NPC_INTERVAL*NumLoaded)/NumToLoad,PBAR_NPC_INTERVAL/NumToLoad);
		loaded_npcs.add_entry(toload);
		NumLoaded++;
	}
	Set_Progress_Bar_Position(PBAR_NPC_START+PBAR_NPC_INTERVAL);

#if debug
	if(KeepMainRifFile)
	{
		Env_Chunk = old_env_chunk; // pop Env_Chunk
	}
	else
#endif	
	{
		Env_Chunk=0;
	}
}

void EndNPCs()
{
	while (loaded_npcs.size())
	{
		LoadedNPC tounload(loaded_npcs.first_entry());
		tounload.Unload();
		loaded_npcs.delete_first_entry();
	}
	EmptyHierarchyLibrary();
	UnloadScreamSounds();
}
