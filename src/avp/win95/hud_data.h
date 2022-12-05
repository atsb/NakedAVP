/*KJL****************************************************************************************
*  							       D I S P L A Y   D A T A 	            				    *
****************************************************************************************KJL*/

/* KJL 16:58:11 05/08/97 - the display data was getting quite long, so I moved it to 
a header file to make things clearer. */
static struct DDGraphicTag HUDDDInfo[NO_OF_MARINE_HUD_GFX];

/* MARINE LO RES (width = 320) */
static struct DigitPropertiesTag LoresMarineHUDDigitProperties[] =
{
	/* motion tracker digits, units first, thousands last */
	/* these are relative to motion tracker centre! */
	{9, -2, MARINE_HUD_FONT_MT_SMALL},	  
	{3, -2, MARINE_HUD_FONT_MT_SMALL},	  
	{-8, -2, MARINE_HUD_FONT_MT_BIG},
    {-17, -2,MARINE_HUD_FONT_MT_BIG},	
  
   	/* -ve values are relative to right side of the screen */
  	/* marine health, units first */
  	{28,10, MARINE_HUD_FONT_BLUE},
	{19,10, MARINE_HUD_FONT_BLUE},
	{10,10, MARINE_HUD_FONT_BLUE},
  		
  	/* marine energy, units first */
  	{28,25, MARINE_HUD_FONT_BLUE},
	{19,25, MARINE_HUD_FONT_BLUE},
	{10,25, MARINE_HUD_FONT_BLUE},
   	
  	/* marine armour, units first */
  	{28,40, MARINE_HUD_FONT_BLUE},
	{19,40, MARINE_HUD_FONT_BLUE},
	{10,40, MARINE_HUD_FONT_BLUE},
 
   	/* marine ammo/rounds, units first */
  	{-22,10, MARINE_HUD_FONT_RED},
	{-31,10, MARINE_HUD_FONT_RED},
	{-40,10, MARINE_HUD_FONT_RED},
 
   	/* marine ammo/magazines, units first */
  	{-61,10, MARINE_HUD_FONT_RED},
	{-70,10, MARINE_HUD_FONT_RED},
      
   	/* marine secondary ammo/rounds, units first */
  	{-22,25, MARINE_HUD_FONT_RED},
	{-31,25, MARINE_HUD_FONT_RED},
	{-40,25, MARINE_HUD_FONT_RED},
 
   	/* marine secondary ammo/magazines, units first */
  	{-61,25, MARINE_HUD_FONT_RED},
	{-70,25, MARINE_HUD_FONT_RED},
};

#if 0 /* SBF - unused */
static char *LoresMarineHUDGfxFilenamePtr[]=
{
    {"blips.pg0"}, 	//MARINE_HUD_GFX_MOTIONTRACKERBLIP,
    {"num.pg0"},	//MARINE_HUD_GFX_NUMERALS,
	{"gunsight.pg0"},	//MARINE_HUD_GFX_GUNSIGHTS,
	{"trakfont.pg0"},
	{"bluebar.pg0"},
};
#endif /* SBF */

#if 0
static struct HUDFontDescTag LoresHUDFontDesc[] =
{
	//MARINE_HUD_FONT_BLUE,
	{
		0,//XOffset
		12,//Height
		8,//Width
	},
	//MARINE_HUD_FONT_RED,
	{
		8,//XOffset
		12,//Height
		8,//Width
	},
	//MARINE_HUD_FONT_MT_SMALL,
	{
		8,//XOffset
		8,//Height
		5,//Width
	},
	//MARINE_HUD_FONT_MT_BIG,
	{
		0,//XOffset
		12,//Height
		8,//Width
	},						 
	//ALIEN_HUD_FONT,
	{
		0,
		8,
		6,
	},

};
#endif

#if 0 /* SBF - unused */
static struct LittleMDescTag LoresHUDLittleM =
{
	80,8,  // source top,left

	5,5,   // width, height

	7,7,   // screen x,y
};
#endif /* SBF */

/* MARINE MED RES (width = 640)  */
static struct DigitPropertiesTag MedresMarineHUDDigitProperties[] =
{
	/* motion tracker digits, units first, thousands last */
	/* these are relative to motion tracker centre! */
	{17, -4, MARINE_HUD_FONT_MT_SMALL},	  
	{9, -4, MARINE_HUD_FONT_MT_SMALL},	  
	{-9, -4, MARINE_HUD_FONT_MT_BIG},
    {-25, -4,MARINE_HUD_FONT_MT_BIG},	
  
   	/* -ve values are relative to right side of the screen */
  	/* marine health, units first */
  	{56,20, MARINE_HUD_FONT_BLUE},
	{38,20, MARINE_HUD_FONT_BLUE},
	{20,20, MARINE_HUD_FONT_BLUE},
  		
  	/* marine energy, units first */
  	{56,50, MARINE_HUD_FONT_BLUE},
	{38,50, MARINE_HUD_FONT_BLUE},
	{20,50, MARINE_HUD_FONT_BLUE},
   	
  	/* marine armour, units first */
  	{56,80, MARINE_HUD_FONT_BLUE},
	{38,80, MARINE_HUD_FONT_BLUE},
	{20,80, MARINE_HUD_FONT_BLUE},
 
   	/* marine ammo/rounds, units first */
  	{-44,20, MARINE_HUD_FONT_RED},
	{-62,20, MARINE_HUD_FONT_RED},
	{-80,20, MARINE_HUD_FONT_RED},
 
   	/* marine ammo/magazines, units first */
  	{-122,20, MARINE_HUD_FONT_RED},
	{-140,20, MARINE_HUD_FONT_RED},
      
   	/* marine secondary ammo/rounds, units first */
  	{-44,50, MARINE_HUD_FONT_RED},
	{-62,50, MARINE_HUD_FONT_RED},
	{-80,50, MARINE_HUD_FONT_RED},
 
   	/* marine secondary ammo/magazines, units first */
  	{-122,50, MARINE_HUD_FONT_RED},
	{-140,50, MARINE_HUD_FONT_RED},
};

#if 0 /* SBF - unused */
static char *MedresMarineHUDGfxFilenamePtr[]=
{
    {"blipsHRz.pg0"}, 	//MARINE_HUD_GFX_MOTIONTRACKERBLIP,
    {"numMR.pg0"},	//MARINE_HUD_GFX_NUMERALS,
	{"sightsmr.pg0"},	//MARINE_HUD_GFX_GUNSIGHTS,
	{"trkfntmr.pg0"},
	{"blubarmr.pg0"},
};
#endif /* SBF */

#if 0
static struct HUDFontDescTag MedresHUDFontDesc[] =
{
	//MARINE_HUD_FONT_BLUE,
	{
		0,//XOffset
		24,//Height
		16,//Width
	},
	//MARINE_HUD_FONT_RED,
	{
		16,//XOffset
		24,//Height
		16,//Width
	},
	//MARINE_HUD_FONT_MT_SMALL,
	{
		14,//XOffset
		12,//Height
		8,//Width
	},
	//MARINE_HUD_FONT_MT_BIG,
	{
		0,//XOffset
		24,//Height
		14,//Width
	},
	//ALIEN_HUD_FONT,
	{
		0,
		16,
		12,
	},

};
#endif

#if 0 /* SBF - unused */
static struct LittleMDescTag MedresHUDLittleM =
{
	120,14,  // source top,left

	8,11,   // width, height

	10,10,   // screen x,y
};
#endif /* SBF */





/* MARINE HI RES (width = 800)  */
static struct DigitPropertiesTag HiresMarineHUDDigitProperties[] =
{
	/* motion tracker digits, units first, thousands last */
	/* these are relative to motion tracker centre! */
	{17, -4, MARINE_HUD_FONT_MT_SMALL},	  
	{5, -4, MARINE_HUD_FONT_MT_SMALL},	  
	{-16, -4, MARINE_HUD_FONT_MT_BIG},
    {-32, -4,MARINE_HUD_FONT_MT_BIG},	
  
   	/* -ve values are relative to right side of the screen */
  	/* marine health, units first */
  	{60,20, MARINE_HUD_FONT_BLUE},
	{40,20, MARINE_HUD_FONT_BLUE},
	{20,20, MARINE_HUD_FONT_BLUE},
  		
  	/* marine energy, units first */
  	{60,55, MARINE_HUD_FONT_BLUE},
	{40,55, MARINE_HUD_FONT_BLUE},
	{20,55, MARINE_HUD_FONT_BLUE},
   	
  	/* marine armour, units first */
  	{60,90, MARINE_HUD_FONT_BLUE},
	{40,90, MARINE_HUD_FONT_BLUE},
	{20,90, MARINE_HUD_FONT_BLUE},
 
   	/* marine ammo/rounds, units first */
  	{-44,20, MARINE_HUD_FONT_RED},
	{-64,20, MARINE_HUD_FONT_RED},
	{-84,20, MARINE_HUD_FONT_RED},
 
   	/* marine ammo/magazines, units first */
  	{-122,20, MARINE_HUD_FONT_RED},
	{-142,20, MARINE_HUD_FONT_RED},
      
   	/* marine secondary ammo/rounds, units first */
  	{-44,55, MARINE_HUD_FONT_RED},
	{-64,55, MARINE_HUD_FONT_RED},
	{-84,55, MARINE_HUD_FONT_RED},
 
   	/* marine secondary ammo/magazines, units first */
  	{-122,55, MARINE_HUD_FONT_RED},
	{-142,55, MARINE_HUD_FONT_RED},
};

#if 0 /* SBF - unused */
static char *HiresMarineHUDGfxFilenamePtr[]=
{
    {"blipsHRz.pg0"}, 	//MARINE_HUD_GFX_MOTIONTRACKERBLIP,
    {"numhR.pg0"},	//MARINE_HUD_GFX_NUMERALS,
	{"sightsmr.pg0"},	//MARINE_HUD_GFX_GUNSIGHTS,
	{"trkfnthr.pg0"},
	{"blubarhr.pg0"},
};
#endif /* SBF */

#if 0
static struct HUDFontDescTag HiresHUDFontDesc[] =
{
	//MARINE_HUD_FONT_BLUE,
	{
		0,//XOffset
		27,//Height
		19,//Width
	},
	//MARINE_HUD_FONT_RED,
	{
		20,//XOffset
		27,//Height
		19,//Width
	},
	//MARINE_HUD_FONT_MT_SMALL,
	{
		18,//XOffset
		15,//Height
		8,//Width
	},
	//MARINE_HUD_FONT_MT_BIG,
	{
		0,//XOffset
		29,//Height
		17,//Width
	},
};
#endif

#if 0 /* SBF - unused */
static struct LittleMDescTag HiresHUDLittleM =
{
	150,17,  // source top,left

	9,13,   // width, height

	14,14,   // screen x,y
};
#endif

#if 0 /* SBF - unused */

/* PREDATOR */


static char *LoresPredatorHUDGfxFilenamePtr[]=
{
	{"topmask.pg0"},	//PREDATOR_HUD_GFX_TOP,
	{"botmmask.pg0"},	//PREDATOR_HUD_GFX_BOTTOM,
   	{"prednum.pg0"},	//PREDATOR_HUD_GFX_NUMBERS,
    {"predsymb.pg0"},   //PREDATOR_HUD_GFX_SYMBOLS,
};
static char *MedresPredatorHUDGfxFilenamePtr[]=
{
	{"prhdtpMR.pg0"},	//PREDATOR_HUD_GFX_TOP,
	{"prhdbmMR.pg0"},	//PREDATOR_HUD_GFX_BOTTOM,
   	{"prednum.pg0"},	//PREDATOR_HUD_GFX_NUMBERS,
    {"predsymb.pg0"},   //PREDATOR_HUD_GFX_SYMBOLS,
};
#if 0
static struct DigitPropertiesTag LoresPredatorHUDDigitProperties[] =
{
	/* armour, units first */
	{63, 158, PREDATOR_HUD_GFX_NUMBERS},	  
	{72, 153, PREDATOR_HUD_GFX_NUMBERS},	  
	{81, 149, PREDATOR_HUD_GFX_NUMBERS},
    {90, 146, PREDATOR_HUD_GFX_NUMBERS},
    {99, 144, PREDATOR_HUD_GFX_NUMBERS},	
			  
	/* health, units first */
	{249, 158, PREDATOR_HUD_GFX_NUMBERS},	  
	{240, 153, PREDATOR_HUD_GFX_NUMBERS},	  
	{231, 149, PREDATOR_HUD_GFX_NUMBERS},
    {222, 146, PREDATOR_HUD_GFX_NUMBERS},
    {213, 144, PREDATOR_HUD_GFX_NUMBERS},	
	
	/* threat display, units first */
	{90-6, 180, PREDATOR_HUD_GFX_NUMBERS},	  
	{110-6, 180, PREDATOR_HUD_GFX_NUMBERS},	  
	{130-6, 180, PREDATOR_HUD_GFX_NUMBERS},	  
	{150-6, 180, PREDATOR_HUD_GFX_NUMBERS},	  
	{170-6, 180, PREDATOR_HUD_GFX_NUMBERS},	  
	{190-6, 180, PREDATOR_HUD_GFX_NUMBERS},	  
	{210-6, 180, PREDATOR_HUD_GFX_NUMBERS},	  
	{230-6, 180, PREDATOR_HUD_GFX_NUMBERS},	  
    	
};
static struct DigitPropertiesTag MedresPredatorHUDDigitProperties[] =
{
	/* armour, units first */
	{63*2+8, 158*2+80, PREDATOR_HUD_GFX_NUMBERS},	  
	{72*2+8, 153*2+80, PREDATOR_HUD_GFX_NUMBERS},	  
	{81*2+8, 149*2+80, PREDATOR_HUD_GFX_NUMBERS},
    {90*2+8, 146*2+80, PREDATOR_HUD_GFX_NUMBERS},
    {99*2+8, 144*2+80, PREDATOR_HUD_GFX_NUMBERS},	
			  
	/* health, units first */
	{249*2, 158*2+80, PREDATOR_HUD_GFX_NUMBERS},	  
	{240*2, 153*2+80, PREDATOR_HUD_GFX_NUMBERS},	  
	{231*2, 149*2+80, PREDATOR_HUD_GFX_NUMBERS},
    {222*2, 146*2+80, PREDATOR_HUD_GFX_NUMBERS},
    {213*2, 144*2+80, PREDATOR_HUD_GFX_NUMBERS},	
	
	/* threat display, units first */
	{90-6, 180, PREDATOR_HUD_GFX_NUMBERS},	  
	{110-6, 180, PREDATOR_HUD_GFX_NUMBERS},	  
	{130-6, 180, PREDATOR_HUD_GFX_NUMBERS},	  
	{150-6, 180, PREDATOR_HUD_GFX_NUMBERS},	  
	{170-6, 180, PREDATOR_HUD_GFX_NUMBERS},	  
	{190-6, 180, PREDATOR_HUD_GFX_NUMBERS},	  
	{210-6, 180, PREDATOR_HUD_GFX_NUMBERS},	  
	{230-6, 180, PREDATOR_HUD_GFX_NUMBERS},	  
    	
};
#endif


/* ALIEN */
static char *LoresAlienHUDGfxFilenamePtr[]=
{
	{"AlHudBot.pg0"}, // ALIEN_HUD_GFX_BOTTOM
	{"AlHudLft.pg0"}, // ALIEN_HUD_GFX_LEFT
	{"AlHudRgt.pg0"}, // ALIEN_HUD_GFX_RIGHT
   	{"AlHudTop.pg0"}, // ALIEN_HUD_GFX_TOP
    {"AlienNum.pg0"}, // ALIEN_HUD_GFX_NUMBERS
};
static char *MedresAlienHUDGfxFilenamePtr[]=
{
	{"ahMRBtm.pg0"}, // ALIEN_HUD_GFX_BOTTOM
	{"ahMRLft.pg0"}, // ALIEN_HUD_GFX_LEFT
	{"ahMRRgt.pg0"}, // ALIEN_HUD_GFX_RIGHT
   	{"ahMRTop.pg0"}, // ALIEN_HUD_GFX_TOP
    {"ahMRNum.pg0"}, // ALIEN_HUD_GFX_NUMBERS
};

#if 0
static struct DigitPropertiesTag LoresAlienHUDDigitProperties[] =
{
	/* health, units first */
	{288, 157, ALIEN_HUD_GFX_NUMBERS},	  
	{281, 157, ALIEN_HUD_GFX_NUMBERS},	  
	{274, 157, ALIEN_HUD_GFX_NUMBERS},
};

static struct DigitPropertiesTag MedresAlienHUDDigitProperties[] =
{
	/* health, units first */
	{288*2, 157*2+80, ALIEN_HUD_GFX_NUMBERS},	  
	{281*2, 157*2+80, ALIEN_HUD_GFX_NUMBERS},	  
	{274*2, 157*2+80, ALIEN_HUD_GFX_NUMBERS},
};
#endif

#endif /* SBF */

