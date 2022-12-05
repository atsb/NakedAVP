/* Patrick 5/6/97 -------------------------------------------------------------
  AvP Project sound source
  ----------------------------------------------------------------------------*/
#include "3dc.h"
#include "module.h"
#include "inline.h"
#include "stratdef.h"
#include "gamedef.h"
#include "gameplat.h"
#include "bh_types.h"
#include "inventry.h"
#include "weapons.h"
#include "psnd.h"
#include "psndplat.h"
#include "avp_menus.h"
#include "scream.h"

#define UseLocalAssert Yes
#include "ourasert.h"
#include "ffstdio.h"
#include "db.h"
#include "dxlog.h"

#define PRED_PISTOL_PITCH_CHANGE 300

#define CDDA_TEST 			No
#define CD_VOLUME_TEST 	No
#define SOUND_TEST_3D 	No

#define LOAD_SOUND_FROM_FAST_FILE 	   Yes
#if 1
//allow loading from outside of fastfiles to help with custom levels
#define LOAD_SOUND_FROM_FAST_FILE_ONLY No
#else
#define LOAD_SOUND_FROM_FAST_FILE_ONLY (LOAD_USING_FASTFILES)
#endif

#define USE_REBSND_LOADERS  (LOAD_USING_FASTFILES||PREDATOR_DEMO||MARINE_DEMO||ALIEN_DEMO||DEATHMATCH_DEMO)
#define USE_COMMON_FLL_FILE  (LOAD_USING_FASTFILES||PREDATOR_DEMO||MARINE_DEMO||ALIEN_DEMO||DEATHMATCH_DEMO)


/* Andy 9/6/97 ----------------------------------------------------------------
  Internal globals  
-----------------------------------------------------------------------------*/
int weaponHandle = SOUND_NOACTIVEINDEX;

#if 0
static int weaponReloading = 0;
static int backgroundHandle = SOUND_NOACTIVEINDEX;
#endif
static int sadarReloadTimer = 0;
static int weaponPitchTimer = 0;
static int playOneShotWS = 1;
static int oldRandomValue = -1;

#if SOUND_TEST_3D
static int testLoop = SOUND_NOACTIVEINDEX;
#endif

#if CDDA_TEST
static int doneCDDA = 0;
#endif
			
/* Has the player made a noise? */
int playerNoise;

/* Patrick 5/6/97 -------------------------------------------------------------
  External refernces
  ----------------------------------------------------------------------------*/
extern int NormalFrameTime;
extern ACTIVESOUNDSAMPLE ActiveSounds[];

/* Patrick 5/6/97 -------------------------------------------------------------
  Function definitions 
  ----------------------------------------------------------------------------*/

/* Patrick 16/6/97 ----------------------------------------------------------------
  A.N.Other background sound management function  
------------------------------------------------------------------------------*/

#if 0
static void DoPredatorBackgroundLoop(void)
{
	if (backgroundHandle == SOUND_NOACTIVEINDEX)
  {
    Sound_Play(SID_VISION_LOOP,"evl",&backgroundHandle,75);
  }
}
#endif


void DoPlayerSounds(void)
{
	PLAYER_STATUS *playerStatusPtr;
	PLAYER_WEAPON_DATA *weaponPtr;

	#if CDDA_TEST
	if (doneCDDA == 0)
	{
		CDDA_SwitchOn();

		doneCDDA = 1;

		if (AvP.PlayerType == I_Marine) 				CDDA_Play(CDTrack1);
		else if (AvP.PlayerType == I_Predator)	CDDA_Play(CDTrack3);
		else if (AvP.PlayerType == I_Alien) 		CDDA_Play(CDTrack2);

	}
	#endif
 
 	#if CD_VOLUME_TEST
	{
		extern unsigned char KeyboardInput[];
		static int CDVolume = CDDA_VOLUME_DEFAULT;

		if(!CDDA_IsPlaying()) CDDA_Play(CDTrack1);
		if(KeyboardInput[KEY_L])
		{
			CDVolume++;
			CDDA_ChangeVolume(CDVolume);
		}
		else if(KeyboardInput[KEY_K])
		{
			CDVolume--;
			CDDA_ChangeVolume(CDVolume);
		}

		{
			int currentSetting = CDDA_GetCurrentVolumeSetting();
			textprint("CD VOL: %d \n",currentSetting);
		}
	}
	#endif

	#if SOUND_TEST_3D
	if(testLoop == SOUND_NOACTIVEINDEX)
	{
		VECTORCH zeroLoc = {0,0,0};
		Sound_Play(SID_VISION_LOOP,"del",&zeroLoc,&testLoop);
	}
	#endif

	#if 0
	/* sort out background sounds */	
	if(AvP.PlayerType == I_Predator) DoPredatorBackgroundLoop();              	
	else DoBackgroundSound();
	#endif
		
	/* do weapon sound */
	    
 	/* access the extra data hanging off the strategy block */
	playerStatusPtr= (PLAYER_STATUS *) (Player->ObStrategyBlock->SBdataptr);
 	GLOBALASSERT(playerStatusPtr);
    	
 	/* init a pointer to the weapon's data */
 	weaponPtr = &(playerStatusPtr->WeaponSlot[playerStatusPtr->SelectedWeaponSlot]);
    
 	if (sadarReloadTimer)
 	{
 		sadarReloadTimer -= NormalFrameTime;
 		if (sadarReloadTimer <= 0)
		{
			sadarReloadTimer = 0;
			playerNoise=1;
		}
 	}
                           
 	/* Handle weapon reloading */
 	#if 0
 	if (weaponPtr->CurrentState == WEAPONSTATE_RELOAD_PRIMARY)
 	{
 		if (weaponReloading == 0)
 		{
   		weaponReloading = 1;
   	
   		switch (weaponPtr->WeaponIDNumber)
   		{
   			case WEAPON_SADAR:
   				sadarReloadTimer = ONE_FIXED;
   				break;
				case WEAPON_GRENADELAUNCHER:
   				Sound_Play(SID_NADELOAD,"h");
				playerNoise=1;
   				break;
   			case WEAPON_FLAMETHROWER:
   				/* Flame thrower reload? */
   				break;
   			case WEAPON_SMARTGUN:
   				Sound_Play(SID_LONGLOAD,"h");
				playerNoise=1;
   				break;
   			case WEAPON_FRISBEE_LAUNCHER:
   				sadarReloadTimer = ONE_FIXED;
   				break;
				case WEAPON_GRENADELAUNCHER:
   				Sound_Play(SID_NADELOAD,"h");
				playerNoise=1;
   				break;
   			default:
   				Sound_Play(SID_SHRTLOAD,"h");
				playerNoise=1;
   				break;
		 	}
 		}
	}
	else
	{
  	weaponReloading = 0;
	}          
	#endif

	switch(weaponPtr->WeaponIDNumber)
	{			
		case(WEAPON_PRED_PISTOL):
		{
			if(weaponPtr->CurrentState == WEAPONSTATE_FIRING_PRIMARY)			
			{
		 		if(weaponHandle == SOUND_NOACTIVEINDEX) 
				{
		 		  //Sound_Play(SID_PULSE_START,"hp",-PRED_PISTOL_PITCH_CHANGE);	  
				  //Sound_Play(SID_PULSE_LOOP,"elhp",&weaponHandle,-PRED_PISTOL_PITCH_CHANGE);					
			   	  //weaponPitchTimer=ONE_FIXED>>3;
				  Sound_Play(SID_PRED_PISTOL,"h");					
					playerNoise=1;
			  	}
				else
				{
					//weaponPitchTimer-=NormalFrameTime;
					//if (weaponPitchTimer<=0)
					//{
					//	weaponPitchTimer=ONE_FIXED>>3;
					//	Sound_ChangePitch(weaponHandle,(FastRandom()&63)-(32+PRED_PISTOL_PITCH_CHANGE));
					//}
				}
			}
	   		else
			{
				//if(weaponHandle != SOUND_NOACTIVEINDEX)
				//{
		          //if (ActiveSounds[weaponHandle].soundIndex == SID_PULSE_LOOP)
		          //{
			      //    Sound_Play(SID_PULSE_END,"hp",-PRED_PISTOL_PITCH_CHANGE);
				  //	  Sound_Stop(weaponHandle);
			 	  //}
			 	//}
			}
  		break;
   	}

		case(WEAPON_PULSERIFLE):
		{
			if(weaponPtr->CurrentState == WEAPONSTATE_FIRING_PRIMARY)			
			{
		 		if(weaponHandle == SOUND_NOACTIVEINDEX) 
				{
		 		  Sound_Play(SID_PULSE_START,"h");	  
			   	Sound_Play(SID_PULSE_LOOP,"elh",&weaponHandle);					
					playerNoise=1;
			   	weaponPitchTimer=ONE_FIXED>>3;
			  }
				else
				{
					weaponPitchTimer-=NormalFrameTime;
					if (weaponPitchTimer<=0)
					{
						weaponPitchTimer=ONE_FIXED>>3;
						Sound_ChangePitch(weaponHandle,(FastRandom()&63)-32);
						playerNoise=1;
					}
				}
			}
   		else if (weaponPtr->CurrentState == WEAPONSTATE_FIRING_SECONDARY)
   		{
   			if (weaponHandle == SOUND_NOACTIVEINDEX)
   			{
     			Sound_Play(SID_NADEFIRE,"h");
				playerNoise=1;
   			}
	 		}
   		else
			{
				if(weaponHandle != SOUND_NOACTIVEINDEX)
				{
         	Sound_Play(SID_PULSE_END,"h");
				  Sound_Stop(weaponHandle);
			 	}
			}
   		break;
   	}

   	case(WEAPON_FLAMETHROWER):
		{
			if(weaponPtr->CurrentState == WEAPONSTATE_FIRING_PRIMARY)			
			{
	 			if(weaponHandle == SOUND_NOACTIVEINDEX) 
				{
	    		Sound_Play(SID_INCIN_START,"h");	  
	  			Sound_Play(SID_INCIN_LOOP,"elh",&weaponHandle);					
				playerNoise=1;
			}
	  	}
			else
			{
				if(weaponHandle != SOUND_NOACTIVEINDEX)
				{
					Sound_Play(SID_INCIN_END,"h");
					Sound_Stop(weaponHandle);
		 		}
			}
     	break;
		}      

   	case (WEAPON_MINIGUN):
   	{
		#if 0
     	if(weaponPtr->CurrentState == WEAPONSTATE_FIRING_PRIMARY)		
     	{
     		if(weaponHandle == SOUND_NOACTIVEINDEX) {
     			Sound_Play(SID_MINIGUN_LOOP,"elh",&weaponHandle);
				playerNoise=1;
			}
	   	}
     	else
     	{
     		if(weaponHandle != SOUND_NOACTIVEINDEX)
     		{
       		Sound_Play(SID_MINIGUN_END,"h");
       		Sound_Stop(weaponHandle);
     		}
     	}
		#else
        if (PlayerStatusPtr->IsAlive==0) {
     		if(weaponHandle != SOUND_NOACTIVEINDEX)
     		{
       		Sound_Play(SID_MINIGUN_END,"h");
       		Sound_Stop(weaponHandle);
     		}
		}
		#endif
     	break;
   	}  

   	case (WEAPON_AUTOSHOTGUN):
   	{
     	if (weaponPtr->CurrentState == WEAPONSTATE_FIRING_PRIMARY)
     	{
	   		Sound_Play(SID_SHOTGUN,"h");
			playerNoise=1;
	 	}
     	break;
   	}           

   	case (WEAPON_MARINE_PISTOL):
   	case (WEAPON_TWO_PISTOLS):
   	{
     	if ((weaponPtr->CurrentState == WEAPONSTATE_FIRING_PRIMARY)
			||(weaponPtr->CurrentState == WEAPONSTATE_FIRING_SECONDARY))
     	{
	   		Sound_Play(SID_SHOTGUN,"h");
			playerNoise=1;
	 	}

     	break;
   	}           

 	case (WEAPON_SADAR):
   	{
     	if (weaponPtr->CurrentState == WEAPONSTATE_FIRING_PRIMARY)
     	{
  	 		Sound_Play(SID_SADAR_FIRE,"h");
			playerNoise=1;
	 	}
     	break;
   	}      

 	case (WEAPON_FRISBEE_LAUNCHER):
   	{
     	if (weaponPtr->CurrentState == WEAPONSTATE_FIRING_PRIMARY)
     	{
			if (weaponPtr->StateTimeOutCounter == WEAPONSTATE_INITIALTIMEOUTCOUNT) {
				playerNoise=1;
	 			if (weaponHandle == SOUND_NOACTIVEINDEX) {
		  	 		Sound_Play(SID_ED_SKEETERCHARGE,"eh",&weaponHandle);
				}
			} else {
	 			if (weaponHandle == SOUND_NOACTIVEINDEX) {
					playerNoise=0;
				}
			}
	 	} else {
 			if (weaponHandle != SOUND_NOACTIVEINDEX) {
				Sound_Stop(weaponHandle);
			}
		}
     	break;
   	}      


   	case(WEAPON_SMARTGUN):
		{
			if(weaponPtr->CurrentState == WEAPONSTATE_FIRING_PRIMARY)			
			{
				if(weaponHandle == SOUND_NOACTIVEINDEX) 
				{
				 	unsigned int rand=FastRandom() % 3;
         	if (rand == oldRandomValue) rand=(rand + 1) % 3;
         	oldRandomValue = rand;
			playerNoise=1;
         	switch (rand)
         	{
         		case 0:
         		{
          		Sound_Play(SID_SMART1,"ehp",&weaponHandle,(FastRandom()&255)-128);					
          		break;
         		}
        		case 1:
         		{
          		Sound_Play(SID_SMART2,"ehp",&weaponHandle,(FastRandom()&255)-128);					
          		break;
         		}
        		case 2:
         		{
          		Sound_Play(SID_SMART3,"ehp",&weaponHandle,(FastRandom()&255)-128);					
          		break;
         		}
			 		default:
				 		{
				 			break;
				 		}
				 	}
			 	}
     	}
			break;
		}

		case(WEAPON_GRENADELAUNCHER):
		{
			if(weaponPtr->CurrentState == WEAPONSTATE_FIRING_PRIMARY)			
			{
				if(playOneShotWS) 
				{
					Sound_Play(SID_ROCKFIRE,"h");
					playerNoise=1;
					playOneShotWS = 0;
				}
			}
			else playOneShotWS = 1;
			break;
		}

		case(WEAPON_PRED_WRISTBLADE):
		{
			#if 0
			if(weaponPtr->CurrentState == WEAPONSTATE_FIRING_PRIMARY)			
			{	
				if(playOneShotWS) 
				{
		        	unsigned int rand=FastRandom() % 6;
		        	if (rand == oldRandomValue) rand = (rand + 1) % 6;
	     			oldRandomValue = rand;
				  	switch (rand)
		        	{
		         		case 0:
		        		{
			          		Sound_Play(SID_SWIPE,"ehp",&weaponHandle,(FastRandom()&255)-128);					
			          		break;
			          	}
			          	case 1:
			          	{
			          		Sound_Play(SID_SWIPE2,"ehp",&weaponHandle,(FastRandom()&255)-128);					
			          		break;
			          	}
			          	case 2:
			          	{
			  	       		Sound_Play(SID_SWIPE3,"ehp",&weaponHandle,(FastRandom()&255)-128);					
				         		break;
			           	}
						case 3:
			          	{
			          		Sound_Play(SID_SWIPE4,"ehp",&weaponHandle,(FastRandom()&255)-128);					
			         		break;
			         	}
						case 4:
			         	{
			         		Sound_Play(SID_PRED_SLASH,"ehp",&weaponHandle,(FastRandom()&255)-128);					
			        		break;
			         	}
						case 5:
			         	{
			        		Sound_Play(SID_RIP,"ehp",&weaponHandle,(FastRandom()&255)-128);					
				       		break;
			         	}
					 	default:
					 	{
					 		break;
					 	}
				 	}
					playOneShotWS = 0;
				}
			}
			else playOneShotWS = 1;				
			#endif
			break;
		}
	
		case(WEAPON_ALIEN_CLAW):
		{
			#if 0
			if(weaponPtr->CurrentState == WEAPONSTATE_FIRING_PRIMARY)			
			{	
				if(playOneShotWS) 
				{
	       			unsigned int rand=FastRandom() & 3;
        			if (rand == oldRandomValue) rand=(rand + 1) & 3;
				   
				   	oldRandomValue = rand;
        			switch (rand)
        			{
         				case 0:
         				{
          					Sound_Play(SID_SWIPE,"ehp",&weaponHandle,(FastRandom()&255)-128);					
          					break;
          				}
          				case 1:
          				{
          					Sound_Play(SID_SWIPE2,"ehp",&weaponHandle,(FastRandom()&255)-128);					
          					break;
          				}
          				case 2:
          				{
          					Sound_Play(SID_SWIPE3,"ehp",&weaponHandle,(FastRandom()&255)-128);					
          					break;
           				}
						case 3:
          				{
          					Sound_Play(SID_SWIPE4,"ehp",&weaponHandle,(FastRandom()&255)-128);					
          					break;
           				}
						default:
						{
							break;
						}
					}
			 		playOneShotWS = 0;
				}
			} else {
				playOneShotWS = 1;
			}
			#endif
			break;
		}

		case(WEAPON_ALIEN_GRAB):
		{
			#if 0
			if(weaponPtr->CurrentState == WEAPONSTATE_FIRING_PRIMARY)			
			{
				if(playOneShotWS) 
				{
			  	unsigned int rand=FastRandom() & 1;
       		if (rand == oldRandomValue) rand=(rand + 1) & 1;
					oldRandomValue = rand;
			   	switch (rand)
        	{
         		case 0:
         		{
          		Sound_Play(SID_SWISH,"ehp",&weaponHandle,(FastRandom()&255)-128);					
          		break;
          	}
          	case 1:
          	{
          		Sound_Play(SID_TAIL,"ehp",&weaponHandle,(FastRandom()&255)-128);					
          		break;
          	}
          
					 	default:
					 	{
					 		break;
					 	}
				 	}
			 		playOneShotWS = 0;
				}
			}
			else playOneShotWS = 1;
			#endif
			break;
		}

		case(WEAPON_PRED_RIFLE):
		{
			if(weaponPtr->CurrentState == WEAPONSTATE_FIRING_PRIMARY)			
			{
				if(playOneShotWS) 
				{
					Sound_Play(SID_PRED_LASER,"hp",(FastRandom()&255)-128);
					playerNoise=1;
					playOneShotWS = 0;
				}
			}
			else playOneShotWS = 1;
			break;
		}

		case(WEAPON_PRED_SHOULDERCANNON):
		{
			if(weaponPtr->CurrentState == WEAPONSTATE_FIRING_PRIMARY)			
			{
				if(playOneShotWS) 
				{
					//Sound_Play(SID_PRED_LAUNCHER,"hp",(FastRandom()&255)-128);
					playOneShotWS = 0;
				}
			}
			else playOneShotWS = 1;
			break;
		}	 

		case(WEAPON_PRED_DISC):
		{
			if(weaponPtr->CurrentState == WEAPONSTATE_FIRING_PRIMARY)			
			{
				if(playOneShotWS) 
				{
					Sound_Play(SID_PRED_FRISBEE,"hp",(FastRandom()&255)-128);
					playerNoise=1;
					playOneShotWS = 0;
				}
			}
			else playOneShotWS = 1;
			break;
		}	 
	   
		case(WEAPON_ALIEN_SPIT):
		{
			if(weaponPtr->CurrentState == WEAPONSTATE_FIRING_PRIMARY)			
			{
				if(playOneShotWS) 
				{
				 	Sound_Play(SID_ACID_SPRAY,"hp",(FastRandom()&255)-128);
					playerNoise=1;
			 		playOneShotWS = 0;
				}
			}
			else playOneShotWS = 1;
			break;
		}	
		default:
		{
			break;
		}
	}
}

static int SpotEffectWeaponHandle = SOUND_NOACTIVEINDEX;
void PlayWeaponClickingNoise(enum WEAPON_ID weaponIDNumber)
{
 	if(SpotEffectWeaponHandle != SOUND_NOACTIVEINDEX)
		return;
	
	switch(weaponIDNumber)
	{
		// Marine weapons
		case WEAPON_PULSERIFLE:
		{
 			Sound_Play(SID_PULSE_RIFLE_FIRING_EMPTY,"eh",&SpotEffectWeaponHandle);
			break;
		}
		case WEAPON_SMARTGUN:
		{
 			Sound_Play(SID_NOAMMO,"eh",&SpotEffectWeaponHandle);
			break;
		}
		case WEAPON_MINIGUN:
		{
			#if 0
			Sound_Play(SID_MINIGUN_EMPTY,"eh",&SpotEffectWeaponHandle);
			#endif
			break;
		}
		// Predator weapons
		case WEAPON_PRED_RIFLE:
		{
			Sound_Play(SID_PREDATOR_SPEARGUN_EMPTY,"eh",&SpotEffectWeaponHandle);
			break;
		}

		default:
			break;
	}
}


void MakeRicochetSound(VECTORCH *position)
{
	switch(NormalFrameTime&0x3)
	{
		case(0):
			Sound_Play(SID_RICOCH1,"pd",((FastRandom()&255)-128),position);	 
			break;
		case(1):
			Sound_Play(SID_RICOCH2,"pd",((FastRandom()&255)-128),position);	 
			break;
		case(2):
			Sound_Play(SID_RICOCH3,"pd",((FastRandom()&255)-128),position);	 
			break;
		case(3):
			Sound_Play(SID_RICOCH4,"pd",((FastRandom()&255)-128),position);	 
			break;
		default:
			break;
	}
}



void PlayAlienSwipeSound(void) {

	#if 0
	 unsigned int rand=FastRandom() & 3;
     if (rand == oldRandomValue) rand=(rand + 1) & 3;
	 
	 oldRandomValue = rand;
     switch (rand)
     {
     	case 0:
     	{
     		Sound_Play(SID_SWIPE,"ehp",&weaponHandle,(FastRandom()&255)-128);					
     		break;
     	}
     	case 1:
     	{
     		Sound_Play(SID_SWIPE2,"ehp",&weaponHandle,(FastRandom()&255)-128);					
     		break;
     	}
     	case 2:
     	{
     		Sound_Play(SID_SWIPE3,"ehp",&weaponHandle,(FastRandom()&255)-128);					
     		break;
     	}
	 	case 3:
     	{
     		Sound_Play(SID_SWIPE4,"ehp",&weaponHandle,(FastRandom()&255)-128);
     		break;
     	}
	 	default:
	 	{
	 		break;
	 	}
	 }
	#else
	PlayAlienSound(0,ASC_Swipe,((FastRandom()&255)-128),
		&weaponHandle,NULL);
	#endif
}

void PlayAlienTailSound(void) {

	PlayAlienSound(0,ASC_TailSound,((FastRandom()&255)-128),
		&weaponHandle,NULL);

}
	
void PlayPredSlashSound(void) {

	#if 0
	unsigned int rand=FastRandom() % 6;
	if (rand == oldRandomValue) rand = (rand + 1) % 6;
	oldRandomValue = rand;
	switch (rand)
	{
		case 0:
		{
	  		Sound_Play(SID_SWIPE,"ehp",&weaponHandle,(FastRandom()&255)-128);					
	  		break;
	  	}
	  	case 1:
	  	{
	  		Sound_Play(SID_SWIPE2,"ehp",&weaponHandle,(FastRandom()&255)-128);					
	  		break;
	  	}
	  	case 2:
	  	{
	   		Sound_Play(SID_SWIPE3,"ehp",&weaponHandle,(FastRandom()&255)-128);					
	     		break;
	   	}
		case 3:
	  	{
	  		Sound_Play(SID_SWIPE4,"ehp",&weaponHandle,(FastRandom()&255)-128);					
	 		break;
	 	}
		case 4:
	 	{
	   		Sound_Play(SID_SWIPE3,"ehp",&weaponHandle,(FastRandom()&255)-128);					
			break;
	 	}
		case 5:
	 	{
	  		Sound_Play(SID_SWIPE2,"ehp",&weaponHandle,(FastRandom()&255)-128);					
	   		break;
	 	}
	 	default:
	 	{
	 		break;
	 	}
	}
	#else
	PlayPredatorSound(0,PSC_Swipe,((FastRandom()&255)-128),
		&weaponHandle,NULL);
	#endif
}

void PlayCudgelSound(void) {

	unsigned int rand=FastRandom() % 4;
	if (rand == oldRandomValue) rand = (rand + 1) % 4;
	oldRandomValue = rand;
	switch (rand)
	{
		case 0:
		{
	  		Sound_Play(SID_PULSE_SWIPE01,"ehp",&weaponHandle,(FastRandom()&255)-128);					
	  		break;
	  	}
	  	case 1:
	  	{
	  		Sound_Play(SID_PULSE_SWIPE02,"ehp",&weaponHandle,(FastRandom()&255)-128);					
	  		break;
	  	}
	  	case 2:
	  	{
	   		Sound_Play(SID_PULSE_SWIPE03,"ehp",&weaponHandle,(FastRandom()&255)-128);					
	     		break;
	   	}
		case 3:
	  	{
	  		Sound_Play(SID_PULSE_SWIPE04,"ehp",&weaponHandle,(FastRandom()&255)-128);					
	 		break;
	 	}
	 	default:
	 	{
	 		break;
	 	}
	}

}


char * SecondSoundDir = 0;
static const char * FirstSoundDir = "SOUND\\";

int FindAndLoadWavFile(int soundNum,char* wavFileName)
{
	static char sound_name[200];
	sprintf (sound_name, "%s%s", FirstSoundDir,wavFileName);

#if LOAD_SOUND_FROM_FAST_FILE
	//first look in fast file
	{
		size_t nLen;
		if(ffreadbuf(sound_name,&nLen))
		{
			return LoadWavFromFastFile(soundNum,sound_name);
		}
	}
#endif

#if !LOAD_SOUND_FROM_FAST_FILE_ONLY
	//look for sound locally
	{
	
		{
			//check to see if file exists locally first
			FILE* wavFile = OpenGameFile(sound_name, FILEMODE_READONLY, FILETYPE_PERM);
			if(!wavFile && SecondSoundDir)
			{
				//look for sound over network
				sprintf (sound_name, "%s%s", SecondSoundDir,wavFileName);

				wavFile = OpenGameFile(sound_name, FILEMODE_READONLY, FILETYPE_PERM);
				if(!wavFile) {
					LOGDXFMT(("Failed to find %s\n",wavFileName));	
					return 0;
				}

			}
			fclose(wavFile);
		}

		return LoadWavFile(soundNum,sound_name) ;
	}
#else
	LOGDXFMT(("Failed to find %s\n",wavFileName));	
	return 0;
#endif
}



/* Patrick 5/6/97 -------------------------------------------------------------
  Sound data loaders 
  ----------------------------------------------------------------------------*/

extern unsigned char *ExtractWavFile(int soundIndex, unsigned char *bufferPtr);
void *LoadRebSndFile(char *filename)
{
	void *bufferPtr;
	long int save_pos, size_of_file;
	FILE *fp;
	fp = OpenGameFile(filename, FILEMODE_READONLY, FILETYPE_PERM);
	
	if (!fp) goto error;

	save_pos=ftell(fp);
	fseek(fp,0L,SEEK_END);
	size_of_file=ftell(fp);
	fseek(fp,save_pos,SEEK_SET);
	
	bufferPtr = AllocateMem(size_of_file);
	LOCALASSERT(bufferPtr);	

	
	if (!fread(bufferPtr, size_of_file,1,fp))
	{
		fclose(fp);
		DeallocateMem(bufferPtr);
		goto error;
	}

	fclose(fp);
	return bufferPtr;
	
error:
	{
		return 0;
	}
}	

void ReleaseRebSndFile(void *bufferPtr)
{
	LOCALASSERT(bufferPtr);
	DeallocateMem(bufferPtr);
}

void LoadSounds(char *soundDirectory)
{
	void *rebSndBuffer;
	unsigned char *bufferPtr;
	int soundIndex;
	int pitch;

	LOCALASSERT(soundDirectory);

	/* first check that sound has initialised and is turned on */
	if(!SoundSys_IsOn()) return;	

	/* load RebSnd file into a (big) buffer	*/
	{
		char filename[64];
#if ALIEN_DEMO
		strcpy(filename, "./alienfastfile");
#else
		strcpy(filename, "./fastfile");
#endif
		strcat(filename, "/common.ffl");

		rebSndBuffer = LoadRebSndFile(filename);

		if (!rebSndBuffer)
		{
			LOCALASSERT(0);
			return;
		}
	}
		
	/* Process the file */
	bufferPtr = (unsigned char*) rebSndBuffer;
	soundIndex = (int)(*bufferPtr++);
	pitch = (int)((signed char)(*bufferPtr++));
	while((soundIndex!=0xff)||(pitch!=-1))
	{
		if((soundIndex<0)||(soundIndex>=SID_MAXIMUM))
		{
			/* invalid sound number */
			LOCALASSERT("Invalid Sound Index"==0);
		}
		if(GameSounds[soundIndex].loaded)
		{
			/* Duplicate game sound loaded */
			LOCALASSERT("Duplicate game sound loaded"==0);
		}
		
	  	bufferPtr = ExtractWavFile(soundIndex, bufferPtr);
		
	  	GameSounds[soundIndex].loaded = 1;
		GameSounds[soundIndex].activeInstances = 0;	 
		GameSounds[soundIndex].volume = VOLUME_DEFAULT;		

		/* pitch offset is in semitones: need to convert to 1/128ths */
		GameSounds[soundIndex].pitch = pitch;		
				
		InitialiseBaseFrequency(soundIndex);
		soundIndex = (int)(*bufferPtr++);
		pitch = (int)((signed char)(*bufferPtr++));
	}

	ReleaseRebSndFile(rebSndBuffer);
}
