/* CDF 22/6/98 - Single file for the death lists. */

#include "3dc.h"
#include "inline.h"
#include "module.h"
#include "stratdef.h"
#include "gamedef.h"
#include "comp_shp.h"
#include "dynblock.h"
#include "dynamics.h"
#include "pfarlocs.h"
#include "pheromon.h"
#include "bh_types.h"
#include "pvisible.h"
#include "bh_far.h"
#include "bh_debri.h"
#include "bh_pred.h"
#include "bh_paq.h"
#include "bh_queen.h"
#include "bh_marin.h"
#include "bh_alien.h"
#include "lighting.h"
#include "bh_weap.h"
#include "weapons.h"
#include "psnd.h"
#include "equipmnt.h"
#include "los.h"
#include "ai_sight.h"
#include "targeting.h"
#include "dxlog.h"
#include "showcmds.h"

#define UseLocalAssert Yes
#include "ourasert.h"

/* General Death Structures! */
DEATH_DATA Alien_Deaths[] = {
	{
		HMSQT_AlienCrouch,		/* Sequence_Type	 */
		ACrSS_Dies,				/* Sub_Sequence		 */
		(ONE_FIXED>>2),			/* TweeningTime		 */
		-1,						/* Sequence_Length	 */
		0,						/* Multiplayer_Code	 */
		(0<<16)|0,				/* Unique Code */
		0,						/* wound_flags		 */
		0,						/* priority_wounds	 */
		0,						/* Template			 */
		{
			0,						/* Front			 */
			0,						/* Back				 */
			0,						/* Left				 */
			0,						/* Right			 */
		},
		0,						/* Burning			 */
		0,						/* Electrical		 */
		1,						/* Crouching		 */
		0,						/* Minor Boom		 */
		0,						/* Major Boom		 */
	},
	{
		HMSQT_AlienCrawl,		/* Sequence_Type	 */
		ACSS_Pain_Fall_Right,	/* Sub_Sequence		 */
		(ONE_FIXED>>2),			/* TweeningTime		 */
		-1,						/* Sequence_Length	 */
		1,						/* Multiplayer_Code	 */
		(0<<16)|1,				/* Unique Code */
		0,						/* wound_flags		 */
		0,						/* priority_wounds	 */
		0,						/* Template			 */
		{
			0,						/* Front			 */
			0,						/* Back				 */
			0,						/* Left				 */
			0,						/* Right			 */
		},
		0,						/* Burning			 */
		0,						/* Electrical		 */
		1,						/* Crouching		 */
		0,						/* Minor Boom		 */
		0,						/* Major Boom		 */
	},
	{
		HMSQT_AlienCrawl,		/* Sequence_Type	 */
		ACSS_Boom_Fall_Back,	/* Sub_Sequence		 */
		(ONE_FIXED>>4),			/* TweeningTime		 */
		-1,						/* Sequence_Length	 */
		2,						/* Multiplayer_Code	 */
		(0<<16)|2,				/* Unique Code */
		0,						/* wound_flags		 */
		0,						/* priority_wounds	 */
		0,						/* Template			 */
		{
			1,						/* Front			 */
			0,						/* Back				 */
			0,						/* Left				 */
			0,						/* Right			 */
		},
		0,						/* Burning			 */
		0,						/* Electrical		 */
		1,						/* Crouching		 */
		1,						/* Minor Boom		 */
		1,						/* Major Boom		 */
	},
	{
		HMSQT_AlienStand,		/* Sequence_Type	 */
		ASSS_Pain_Fall_Back,	/* Sub_Sequence		 */
		(ONE_FIXED>>2),			/* TweeningTime		 */
		-1,						/* Sequence_Length	 */
		3,						/* Multiplayer_Code	 */
		(0<<16)|3,				/* Unique Code */
		0,						/* wound_flags		 */
		0,						/* priority_wounds	 */
		0,						/* Template			 */
		{
			0,						/* Front			 */
			0,						/* Back				 */
			0,						/* Left				 */
			0,						/* Right			 */
		},
		0,						/* Burning			 */
		0,						/* Electrical		 */
		0,						/* Crouching		 */
		0,						/* Minor Boom		 */
		0,						/* Major Boom		 */
	},
	{
		HMSQT_AlienStand,		/* Sequence_Type	 */
		ASSS_Dies,				/* Sub_Sequence		 */
		(ONE_FIXED>>2),			/* TweeningTime		 */
		-1,						/* Sequence_Length	 */
		4,						/* Multiplayer_Code	 */
		(0<<16)|4,				/* Unique Code */
		0,						/* wound_flags		 */
		0,						/* priority_wounds	 */
		0,						/* Template			 */
		{
			0,						/* Front			 */
			0,						/* Back				 */
			0,						/* Left				 */
			0,						/* Right			 */
		},
		0,						/* Burning			 */
		0,						/* Electrical		 */
		0,						/* Crouching		 */
		0,						/* Minor Boom		 */
		0,						/* Major Boom		 */
	},
	{
		HMSQT_AlienStand,		/* Sequence_Type	 */
		ASSS_Pain_Fall_Fwd,		/* Sub_Sequence		 */
		(ONE_FIXED>>2),			/* TweeningTime		 */
		-1,						/* Sequence_Length	 */
		5,						/* Multiplayer_Code	 */
		(0<<16)|5,				/* Unique Code */
		0,						/* wound_flags		 */
		0,						/* priority_wounds	 */
		0,						/* Template			 */
		{
			0,						/* Front			 */
			0,						/* Back				 */
			0,						/* Left				 */
			0,						/* Right			 */
		},
		0,						/* Burning			 */
		0,						/* Electrical		 */
		0,						/* Crouching		 */
		0,						/* Minor Boom		 */
		0,						/* Major Boom		 */
	},
	{
		HMSQT_AlienStand,		/* Sequence_Type	 */
		ASSS_Boom_Fall_Fwd,		/* Sub_Sequence		 */
		(ONE_FIXED>>4),			/* TweeningTime		 */
		-1,						/* Sequence_Length	 */
		6,						/* Multiplayer_Code	 */
		(0<<16)|6,				/* Unique Code */
		0,						/* wound_flags		 */
		0,						/* priority_wounds	 */
		0,						/* Template			 */
		{
			0,						/* Front			 */
			1,						/* Back				 */
			0,						/* Left				 */
			0,						/* Right			 */
		},
		0,						/* Burning			 */
		0,						/* Electrical		 */
		0,						/* Crouching		 */
		1,						/* Minor Boom		 */
		1,						/* Major Boom		 */
	},
	{
		HMSQT_AlienStand,		/* Sequence_Type	 */
		ASSS_Boom_Fall_Back,	/* Sub_Sequence		 */
		(ONE_FIXED>>5),			/* TweeningTime		 */
		-1,						/* Sequence_Length	 */
		7,						/* Multiplayer_Code	 */
		(0<<16)|7,				/* Unique Code */
		0,						/* wound_flags		 */
		0,						/* priority_wounds	 */
		0,						/* Template			 */
		{
			0,						/* Front			 */
			0,						/* Back				 */
			0,						/* Left				 */
			0,						/* Right			 */
		},
		0,						/* Burning			 */
		0,						/* Electrical		 */
		0,						/* Crouching		 */
		1,						/* Minor Boom		 */
		1,						/* Major Boom		 */
	},
	{
		HMSQT_AlienStand,		/* Sequence_Type	 */
		ASSS_Spin_Clockwise,	/* Sub_Sequence		 */
		(ONE_FIXED>>5),			/* TweeningTime		 */
		-1,						/* Sequence_Length	 */
		8,						/* Multiplayer_Code	 */
		(0<<16)|8,				/* Unique Code */
		0,						/* wound_flags		 */
		0,						/* priority_wounds	 */
		0,						/* Template			 */
		{
			0,						/* Front			 */
			0,						/* Back				 */
			0,						/* Left				 */
			0,						/* Right			 */
		},
		0,						/* Burning			 */
		0,						/* Electrical		 */
		0,						/* Crouching		 */
		1,						/* Minor Boom		 */
		0,						/* Major Boom		 */
	},
	{
		HMSQT_AlienCrouch,		/* Sequence_Type	 */
		ACrSS_Dies_Thrash,		/* Sub_Sequence		 */
		(ONE_FIXED>>2),			/* TweeningTime		 */
		-1,						/* Sequence_Length	 */
		9,						/* Multiplayer_Code	 */
		(0<<16)|9,				/* Unique Code */
		0,						/* wound_flags		 */
		0,						/* priority_wounds	 */
		0,						/* Template			 */
		{
			0,						/* Front			 */
			0,						/* Back				 */
			0,						/* Left				 */
			0,						/* Right			 */
		},
		0,						/* Burning			 */
		0,						/* Electrical		 */
		1,						/* Crouching		 */
		0,						/* Minor Boom		 */
		0,						/* Major Boom		 */
	},
	/* Electric deaths! */
	{
		HMSQT_AlienStand,		/* Sequence_Type	 */
		ASSS_Boom_Fall_Back,	/* Sub_Sequence		 */
		(ONE_FIXED>>5),			/* TweeningTime		 */
		-1,						/* Sequence_Length	 */
		10,						/* Multiplayer_Code	 */
		(0<<16)|10,				/* Unique Code */
		0,						/* wound_flags		 */
		0,						/* priority_wounds	 */
		0,						/* Template			 */
		{
			0,						/* Front			 */
			0,						/* Back				 */
			0,						/* Left				 */
			0,						/* Right			 */
		},
		0,						/* Burning			 */
		1,						/* Electrical		 */
		0,						/* Crouching		 */
		0,						/* Minor Boom		 */
		0,						/* Major Boom		 */
	},
	{
		HMSQT_AlienStand,		/* Sequence_Type	 */
		ASSS_Pain_Fall_Back,	/* Sub_Sequence		 */
		(ONE_FIXED>>2),			/* TweeningTime		 */
		-1,						/* Sequence_Length	 */
		11,						/* Multiplayer_Code	 */
		(0<<16)|11,				/* Unique Code */
		0,						/* wound_flags		 */
		0,						/* priority_wounds	 */
		0,						/* Template			 */
		{
			0,						/* Front			 */
			0,						/* Back				 */
			0,						/* Left				 */
			0,						/* Right			 */
		},
		0,						/* Burning			 */
		1,						/* Electrical		 */
		0,						/* Crouching		 */
		0,						/* Minor Boom		 */
		0,						/* Major Boom		 */
	},
	{
		HMSQT_AlienStand,		/* Sequence_Type	 */
		ASSS_Pain_Fall_Fwd,		/* Sub_Sequence		 */
		(ONE_FIXED>>2),			/* TweeningTime		 */
		-1,						/* Sequence_Length	 */
		12,						/* Multiplayer_Code	 */
		(0<<16)|12,				/* Unique Code */
		0,						/* wound_flags		 */
		0,						/* priority_wounds	 */
		0,						/* Template			 */
		{
			0,						/* Front			 */
			0,						/* Back				 */
			0,						/* Left				 */
			0,						/* Right			 */
		},
		0,						/* Burning			 */
		0,						/* Electrical		 */
		0,						/* Crouching		 */
		0,						/* Minor Boom		 */
		0,						/* Major Boom		 */
	},
	{
		HMSQT_AlienStand,		/* Sequence_Type	 */
		ASSS_Spin_Clockwise,	/* Sub_Sequence		 */
		(ONE_FIXED>>5),			/* TweeningTime		 */
		-1,						/* Sequence_Length	 */
		13,						/* Multiplayer_Code	 */
		(0<<16)|13,				/* Unique Code */
		0,						/* wound_flags		 */
		0,						/* priority_wounds	 */
		0,						/* Template			 */
		{
			0,						/* Front			 */
			0,						/* Back				 */
			0,						/* Left				 */
			0,						/* Right			 */
		},
		0,						/* Burning			 */
		1,						/* Electrical		 */
		0,						/* Crouching		 */
		0,						/* Minor Boom		 */
		0,						/* Major Boom		 */
	},
	{
		HMSQT_AlienCrawl,		/* Sequence_Type	 */
		ACSS_Pain_Fall_Right,	/* Sub_Sequence		 */
		(ONE_FIXED>>2),			/* TweeningTime		 */
		-1,						/* Sequence_Length	 */
		14,						/* Multiplayer_Code	 */
		(0<<16)|14,				/* Unique Code */
		0,						/* wound_flags		 */
		0,						/* priority_wounds	 */
		0,						/* Template			 */
		{
			0,						/* Front			 */
			0,						/* Back				 */
			0,						/* Left				 */
			0,						/* Right			 */
		},
		0,						/* Burning			 */
		1,						/* Electrical		 */
		1,						/* Crouching		 */
		0,						/* Minor Boom		 */
		0,						/* Major Boom		 */
	},
	{
		HMSQT_AlienCrawl,		/* Sequence_Type	 */
		ACSS_Boom_Fall_Back,	/* Sub_Sequence		 */
		(ONE_FIXED>>4),			/* TweeningTime		 */
		-1,						/* Sequence_Length	 */
		15,						/* Multiplayer_Code	 */
		(0<<16)|15,				/* Unique Code */
		0,						/* wound_flags		 */
		0,						/* priority_wounds	 */
		0,						/* Template			 */
		{
			1,						/* Front			 */
			0,						/* Back				 */
			0,						/* Left				 */
			0,						/* Right			 */
		},
		0,						/* Burning			 */
		1,						/* Electrical		 */
		1,						/* Crouching		 */
		0,						/* Minor Boom		 */
		0,						/* Major Boom		 */
	},
	/* Terminator */
	{
		-1,						/* Sequence_Type	 */
		-1,						/* Sub_Sequence		 */
		0,						/* TweeningTime		 */
		0,						/* Sequence_Length	 */
		0,						/* Multiplayer_Code	 */
		(0<<16)|0,				/* Unique Code */
		0,						/* wound_flags		 */
		0,						/* priority_wounds	 */
		0,						/* Template			 */
		{
			0,						/* Front			 */
			0,						/* Back				 */
			0,						/* Left				 */
			0,						/* Right			 */
		},
		0,						/* Burning			 */
		0,						/* Electrical		 */
		0,						/* Crouching		 */
		0,						/* Minor Boom		 */
		0,						/* Major Boom		 */
	},
};

DEATH_DATA Marine_Deaths[] = {
	{
		HMSQT_MarineStand,		/* Sequence_Type	 */
		MSSS_Dies_Standard,		/* Sub_Sequence		 */
		(ONE_FIXED>>3),			/* TweeningTime		 */
		-1,						/* Sequence_Length	 */
		0,						/* Multiplayer_Code	 */
		(1<<16)|0,				/* Unique Code */
		0,						/* wound_flags		 */
		0,						/* priority_wounds	 */
		0,						/* Template			 */
		{
			0,						/* Front			 */
			0,						/* Back				 */
			0,						/* Left				 */
			0,						/* Right			 */
		},
		0,						/* Burning			 */
		0,						/* Electrical		 */
		0,						/* Crouching		 */
		0,						/* Minor Boom		 */
		0,						/* Major Boom		 */
	},
	{
		HMSQT_MarineCrouch,		/* Sequence_Type	 */
		MCrSS_Dies_Standard,	/* Sub_Sequence		 */
		(ONE_FIXED>>3),			/* TweeningTime		 */
		-1,						/* Sequence_Length	 */
		1,						/* Multiplayer_Code	 */
		(1<<16)|1,				/* Unique Code */
		0,						/* wound_flags		 */
		0,						/* priority_wounds	 */
		0,						/* Template			 */
		{
			0,						/* Front			 */
			0,						/* Back				 */
			0,						/* Left				 */
			0,						/* Right			 */
		},
		0,						/* Burning			 */
		0,						/* Electrical		 */
		1,						/* Crouching		 */
		0,						/* Minor Boom		 */
		0,						/* Major Boom		 */
	},
	{
		HMSQT_MarineStand,		/* Sequence_Type	 */
		MSSS_Tem_Back_Death,	/* Sub_Sequence		 */
		(ONE_FIXED>>2),			/* TweeningTime		 */
		-1,						/* Sequence_Length	 */
		2,						/* Multiplayer_Code	 */
		(1<<16)|2,				/* Unique Code */
		0,						/* wound_flags		 */
		0,						/* priority_wounds	 */
		1,						/* Template			 */
		{
			1,						/* Front			 */
			0,						/* Back				 */
			0,						/* Left				 */
			0,						/* Right			 */
		},
		0,						/* Burning			 */
		0,						/* Electrical		 */
		0,						/* Crouching		 */
		0,						/* Minor Boom		 */
		0,						/* Major Boom		 */
	},
	{
		HMSQT_MarineStand,		/* Sequence_Type	 */
		MSSS_Tem_Front_Death,	/* Sub_Sequence		 */
		(ONE_FIXED>>2),			/* TweeningTime		 */
		-1,						/* Sequence_Length	 */
		3,						/* Multiplayer_Code	 */
		(1<<16)|3,				/* Unique Code */
		0,						/* wound_flags		 */
		0,						/* priority_wounds	 */
		1,						/* Template			 */
		{
			0,						/* Front			 */
			1,						/* Back				 */
			0,						/* Left				 */
			0,						/* Right			 */
		},
		0,						/* Burning			 */
		0,						/* Electrical		 */
		0,						/* Crouching		 */
		0,						/* Minor Boom		 */
		0,						/* Major Boom		 */
	},
	{
		HMSQT_MarineStand,		/* Sequence_Type	 */
		MSSS_Tem_Sum_Death,		/* Sub_Sequence		 */
		(ONE_FIXED>>5),			/* TweeningTime		 */
		-1,						/* Sequence_Length	 */
		4,						/* Multiplayer_Code	 */
		(1<<16)|4,				/* Unique Code */
		0,						/* wound_flags		 */
		0,						/* priority_wounds	 */
		1,						/* Template			 */
		{
			1,						/* Front			 */
			0,						/* Back				 */
			0,						/* Left				 */
			0,						/* Right			 */
		},
		0,						/* Burning			 */
		0,						/* Electrical		 */
		0,						/* Crouching		 */
		1,						/* Minor Boom		 */
		1,						/* Major Boom		 */
	},
	{
		HMSQT_MarineStand,		/* Sequence_Type	 */
		MSSS_Tem_Burning,		/* Sub_Sequence		 */
		(ONE_FIXED>>2),			/* TweeningTime		 */
		-1,						/* Sequence_Length	 */
		5,						/* Multiplayer_Code	 */
		(1<<16)|5,				/* Unique Code */
		0,						/* wound_flags		 */
		0,						/* priority_wounds	 */
		1,						/* Template			 */
		{
			0,						/* Front			 */
			0,						/* Back				 */
			0,						/* Left				 */
			0,						/* Right			 */
		},
		1,						/* Burning			 */
		0,						/* Electrical		 */
		0,						/* Crouching		 */
		0,						/* Minor Boom		 */
		0,						/* Major Boom		 */
	},
	{
		HMSQT_MarineStand,		/* Sequence_Type	 */
		MSSS_Tem_LeftSholdr,	/* Sub_Sequence		 */
		(ONE_FIXED>>2),			/* TweeningTime		 */
		-1,						/* Sequence_Length	 */
		6,						/* Multiplayer_Code	 */
		(1<<16)|6,				/* Unique Code */
		section_flag_left_arm, 	/* wound_flags		 */
		0,						/* priority_wounds	 */
		1,						/* Template			 */
		{
			0,						/* Front			 */
			0,						/* Back				 */
			0,						/* Left				 */
			0,						/* Right			 */
		},
		0,						/* Burning			 */
		0,						/* Electrical		 */
		0,						/* Crouching		 */
		0,						/* Minor Boom		 */
		0,						/* Major Boom		 */
	},
	{
		HMSQT_MarineStand,		/* Sequence_Type	 */
		MSSS_Tem_RightSholdr,	/* Sub_Sequence		 */
		(ONE_FIXED>>2),			/* TweeningTime		 */
		-1,						/* Sequence_Length	 */
		7,						/* Multiplayer_Code	 */
		(1<<16)|7,				/* Unique Code */
		section_flag_right_arm,	/* wound_flags		 */
		0,						/* priority_wounds	 */
		1,						/* Template			 */
		{
			0,						/* Front			 */
			0,						/* Back				 */
			0,						/* Left				 */
			0,						/* Right			 */
		},
		0,						/* Burning			 */
		0,						/* Electrical		 */
		0,						/* Crouching		 */
		0,						/* Minor Boom		 */
		0,						/* Major Boom		 */
	},
	{
		HMSQT_MarineStand,		/* Sequence_Type	 */
		MSSS_Tem_LeftForarm,	/* Sub_Sequence		 */
		(ONE_FIXED>>2),			/* TweeningTime		 */
		-1,						/* Sequence_Length	 */
		8,						/* Multiplayer_Code	 */
		(1<<16)|8,				/* Unique Code */
		section_flag_left_hand,	/* wound_flags		 */
		0,						/* priority_wounds	 */
		1,						/* Template			 */
		{
			0,						/* Front			 */
			0,						/* Back				 */
			0,						/* Left				 */
			0,						/* Right			 */
		},
		0,						/* Burning			 */
		0,						/* Electrical		 */
		0,						/* Crouching		 */
		0,						/* Minor Boom		 */
		0,						/* Major Boom		 */
	},
	{
		HMSQT_MarineStand,		/* Sequence_Type	 */
		MSSS_Tem_RightForarm,	/* Sub_Sequence		 */
		(ONE_FIXED>>2),			/* TweeningTime		 */
		-1,						/* Sequence_Length	 */
		9,						/* Multiplayer_Code	 */
		(1<<16)|9,				/* Unique Code */
		section_flag_right_hand,/* wound_flags		 */
		0,						/* priority_wounds	 */
		1,						/* Template			 */
		{
			0,						/* Front			 */
			0,						/* Back				 */
			0,						/* Left				 */
			0,						/* Right			 */
		},
		0,						/* Burning			 */
		0,						/* Electrical		 */
		0,						/* Crouching		 */
		0,						/* Minor Boom		 */
		0,						/* Major Boom		 */
	},
	{
		HMSQT_MarineStand,		/* Sequence_Type	 */
		MSSS_Tem_LeftThigh,		/* Sub_Sequence		 */
		(ONE_FIXED>>2),			/* TweeningTime		 */
		-1,						/* Sequence_Length	 */
		10,						/* Multiplayer_Code	 */
		(1<<16)|10,				/* Unique Code */
		section_flag_left_leg,	/* wound_flags		 */
		section_flag_left_leg|section_flag_left_foot,/* priority_wounds	 */
		1,						/* Template			 */
		{
			0,						/* Front			 */
			0,						/* Back				 */
			0,						/* Left				 */
			0,						/* Right			 */
		},
		0,						/* Burning			 */
		0,						/* Electrical		 */
		0,						/* Crouching		 */
		0,						/* Minor Boom		 */
		0,						/* Major Boom		 */
	},
	{
		HMSQT_MarineStand,		/* Sequence_Type	 */
		MSSS_Tem_RightThigh,	/* Sub_Sequence		 */
		(ONE_FIXED>>2),			/* TweeningTime		 */
		-1,						/* Sequence_Length	 */
		11,						/* Multiplayer_Code	 */
		(1<<16)|11,				/* Unique Code */
		section_flag_right_leg,	/* wound_flags		 */
		section_flag_right_leg|section_flag_right_foot,/* priority_wounds	 */
		1,						/* Template			 */
		{
			0,						/* Front			 */
			0,						/* Back				 */
			0,						/* Left				 */
			0,						/* Right			 */
		},
		0,						/* Burning			 */
		0,						/* Electrical		 */
		0,						/* Crouching		 */
		0,						/* Minor Boom		 */
		0,						/* Major Boom		 */
	},
	{
		HMSQT_MarineStand,		/* Sequence_Type	 */
		MSSS_Tem_LeftShin,		/* Sub_Sequence		 */
		(ONE_FIXED>>2),			/* TweeningTime		 */
		-1,						/* Sequence_Length	 */
		12,						/* Multiplayer_Code	 */
		(1<<16)|12,				/* Unique Code */
		section_flag_left_foot,	/* wound_flags		 */
		section_flag_left_foot,	/* priority_wounds	 */
		1,						/* Template			 */
		{
			0,						/* Front			 */
			0,						/* Back				 */
			0,						/* Left				 */
			0,						/* Right			 */
		},
		0,						/* Burning			 */
		0,						/* Electrical		 */
		0,						/* Crouching		 */
		0,						/* Minor Boom		 */
		0,						/* Major Boom		 */
	},
	{
		HMSQT_MarineStand,		/* Sequence_Type	 */
		MSSS_Tem_RightShin,		/* Sub_Sequence		 */
		(ONE_FIXED>>2),			/* TweeningTime		 */
		-1,						/* Sequence_Length	 */
		13,						/* Multiplayer_Code	 */
		(1<<16)|13,				/* Unique Code */
		section_flag_right_foot,/* wound_flags		 */
		section_flag_right_foot,/* priority_wounds	 */
		1,						/* Template			 */
		{
			0,						/* Front			 */
			0,						/* Back				 */
			0,						/* Left				 */
			0,						/* Right			 */
		},
		0,						/* Burning			 */
		0,						/* Electrical		 */
		0,						/* Crouching		 */
		0,						/* Minor Boom		 */
		0,						/* Major Boom		 */
	},
	/* Listed twice for pain and boom. */
	{
		HMSQT_MarineStand,		/* Sequence_Type	 */
		MSSS_Tem_LeftThigh,		/* Sub_Sequence		 */
		(ONE_FIXED>>2),			/* TweeningTime		 */
		-1,						/* Sequence_Length	 */
		10,						/* Multiplayer_Code	 */
		(1<<16)|14,				/* Unique Code */
		section_flag_left_leg,	/* wound_flags		 */
		section_flag_left_leg|section_flag_left_foot,/* priority_wounds	 */
		1,						/* Template			 */
		{
			0,						/* Front			 */
			0,						/* Back				 */
			0,						/* Left				 */
			0,						/* Right			 */
		},
		0,						/* Burning			 */
		0,						/* Electrical		 */
		0,						/* Crouching		 */
		1,						/* Minor Boom		 */
		1,						/* Major Boom		 */
	},
	{
		HMSQT_MarineStand,		/* Sequence_Type	 */
		MSSS_Tem_RightThigh,	/* Sub_Sequence		 */
		(ONE_FIXED>>2),			/* TweeningTime		 */
		-1,						/* Sequence_Length	 */
		11,						/* Multiplayer_Code	 */
		(1<<16)|15,				/* Unique Code */
		section_flag_right_leg,	/* wound_flags		 */
		section_flag_right_leg|section_flag_right_foot,/* priority_wounds	 */
		1,						/* Template			 */
		{
			0,						/* Front			 */
			0,						/* Back				 */
			0,						/* Left				 */
			0,						/* Right			 */
		},
		0,						/* Burning			 */
		0,						/* Electrical		 */
		0,						/* Crouching		 */
		1,						/* Minor Boom		 */
		1,						/* Major Boom		 */
	},
	{
		HMSQT_MarineStand,		/* Sequence_Type	 */
		MSSS_Tem_LeftShin,		/* Sub_Sequence		 */
		(ONE_FIXED>>2),			/* TweeningTime		 */
		-1,						/* Sequence_Length	 */
		12,						/* Multiplayer_Code	 */
		(1<<16)|16,				/* Unique Code */
		section_flag_left_foot,	/* wound_flags		 */
		section_flag_left_foot,	/* priority_wounds	 */
		1,						/* Template			 */
		{
			0,						/* Front			 */
			0,						/* Back				 */
			0,						/* Left				 */
			0,						/* Right			 */
		},
		0,						/* Burning			 */
		0,						/* Electrical		 */
		0,						/* Crouching		 */
		1,						/* Minor Boom		 */
		1,						/* Major Boom		 */
	},
	{
		HMSQT_MarineStand,		/* Sequence_Type	 */
		MSSS_Tem_RightShin,		/* Sub_Sequence		 */
		(ONE_FIXED>>2),			/* TweeningTime		 */
		-1,						/* Sequence_Length	 */
		13,						/* Multiplayer_Code	 */
		(1<<16)|17,				/* Unique Code */
		section_flag_right_foot,/* wound_flags		 */
		section_flag_right_foot,/* priority_wounds	 */
		1,						/* Template			 */
		{
			0,						/* Front			 */
			0,						/* Back				 */
			0,						/* Left				 */
			0,						/* Right			 */
		},
		0,						/* Burning			 */
		0,						/* Electrical		 */
		0,						/* Crouching		 */
		1,						/* Minor Boom		 */
		1,						/* Major Boom		 */
	},
	{
		HMSQT_MarineStand,		/* Sequence_Type	 */
		MSSS_Tem_Back_Death,	/* Sub_Sequence		 */
		(ONE_FIXED>>2),			/* TweeningTime		 */
		-1,						/* Sequence_Length	 */
		14,						/* Multiplayer_Code	 */
		(1<<16)|18,				/* Unique Code */
		0,						/* wound_flags		 */
		0,						/* priority_wounds	 */
		1,						/* Template			 */
		{
			1,						/* Front			 */
			0,						/* Back				 */
			0,						/* Left				 */
			0,						/* Right			 */
		},
		0,						/* Burning			 */
		0,						/* Electrical		 */
		0,						/* Crouching		 */
		0,						/* Minor Boom		 */
		0,						/* Major Boom		 */
	},
	/* And now the crouch template. */
	{
		HMSQT_MarineCrouch,		/* Sequence_Type	 */
		MCrSS_Dies_Standard,	/* Sub_Sequence		 */
		(ONE_FIXED>>3),			/* TweeningTime		 */
		-1,						/* Sequence_Length	 */
		1,						/* Multiplayer_Code	 */
		15,						/* wound_flags		 */
		(1<<16)|19,				/* Unique Code */
		0,						/* priority_wounds	 */
		1,						/* Template			 */
		{
			0,						/* Front			 */
			0,						/* Back				 */
			0,						/* Left				 */
			0,						/* Right			 */
		},
		0,						/* Burning			 */
		0,						/* Electrical		 */
		1,						/* Crouching		 */
		0,						/* Minor Boom		 */
		0,						/* Major Boom		 */
	},
	/* And now this one, for directionlessness. */
	{
		HMSQT_MarineStand,		/* Sequence_Type	 */
		MSSS_Tem_Back_Death,	/* Sub_Sequence		 */
		(ONE_FIXED>>2),			/* TweeningTime		 */
		-1,						/* Sequence_Length	 */
		16,						/* Multiplayer_Code	 */
		(1<<16)|20,				/* Unique Code */
		0,						/* wound_flags		 */
		0,						/* priority_wounds	 */
		1,						/* Template			 */
		{
			0,						/* Front			 */
			0,						/* Back				 */
			0,						/* Left				 */
			0,						/* Right			 */
		},
		0,						/* Burning			 */
		0,						/* Electrical		 */
		0,						/* Crouching		 */
		0,						/* Minor Boom		 */
		0,						/* Major Boom		 */
	},
	{
		HMSQT_MarineStand,		/* Sequence_Type	 */
		MSSS_Tem_Electric_Death_One,	/* Sub_Sequence		 */
		(ONE_FIXED>>2),			/* TweeningTime		 */
		-1,						/* Sequence_Length	 */
		17,						/* Multiplayer_Code	 */
		(1<<16)|21,				/* Unique Code */
		0,						/* wound_flags		 */
		0,						/* priority_wounds	 */
		1,						/* Template			 */
		{
			0,						/* Front			 */
			0,						/* Back				 */
			0,						/* Left				 */
			0,						/* Right			 */
		},
		0,						/* Burning			 */
		1,						/* Electrical		 */
		0,						/* Crouching		 */
		0,						/* Minor Boom		 */
		0,						/* Major Boom		 */
	},
	{
		HMSQT_MarineStand,		/* Sequence_Type	 */
		MSSS_Tem_Electric_Death_Two,	/* Sub_Sequence		 */
		(ONE_FIXED>>2),			/* TweeningTime		 */
		-1,						/* Sequence_Length	 */
		18,						/* Multiplayer_Code	 */
		(1<<16)|22,				/* Unique Code */
		0,						/* wound_flags		 */
		0,						/* priority_wounds	 */
		1,						/* Template			 */
		{
			0,						/* Front			 */
			0,						/* Back				 */
			0,						/* Left				 */
			0,						/* Right			 */
		},
		0,						/* Burning			 */
		1,						/* Electrical		 */
		0,						/* Crouching		 */
		0,						/* Minor Boom		 */
		0,						/* Major Boom		 */
	},
	{
		HMSQT_MarineCrouch,		/* Sequence_Type	 */
		MCrSS_Tem_Electric_Death_One,	/* Sub_Sequence		 */
		(ONE_FIXED>>2),			/* TweeningTime		 */
		-1,						/* Sequence_Length	 */
		19,						/* Multiplayer_Code	 */
		(1<<16)|23,				/* Unique Code */
		0,						/* wound_flags		 */
		0,						/* priority_wounds	 */
		1,						/* Template			 */
		{
			0,						/* Front			 */
			0,						/* Back				 */
			0,						/* Left				 */
			0,						/* Right			 */
		},
		0,						/* Burning			 */
		1,						/* Electrical		 */
		1,						/* Crouching		 */
		0,						/* Minor Boom		 */
		0,						/* Major Boom		 */
	},
	{
		-1,						/* Sequence_Type	 */
		-1,						/* Sub_Sequence		 */
		0,						/* TweeningTime		 */
		0,						/* Sequence_Length	 */
		0,						/* Multiplayer_Code	 */
		(1<<16)|0,				/* Unique Code */
		0,						/* wound_flags		 */
		0,						/* priority_wounds	 */
		0,						/* Template			 */
		{
			0,						/* Front			 */
			0,						/* Back				 */
			0,						/* Left				 */
			0,						/* Right			 */
		},
		0,						/* Burning			 */
		0,						/* Electrical		 */
		0,						/* Crouching		 */
		0,						/* Minor Boom		 */
		0,						/* Major Boom		 */
	},
};

DEATH_DATA Predator_Special_SelfDestruct_Death = {

	HMSQT_PredatorStand,	/* Sequence_Type	 */
	PSSS_Dies_Standard,		/* Sub_Sequence		 */
	(ONE_FIXED>>3),			/* TweeningTime		 */
	-1,						/* Sequence_Length	 */
	10,						/* Multiplayer_Code	 */
	(2<<16)|0,				/* Unique Code */
	0,						/* wound_flags		 */
	0,						/* priority_wounds	 */
	0,						/* Template			 */
	{
		0,						/* Front			 */
		0,						/* Back				 */
		0,						/* Left				 */
		0,						/* Right			 */
	},
	0,						/* Burning			 */
	0,						/* Electrical		 */
	0,						/* Crouching		 */
	0,						/* Minor Boom		 */
	0,						/* Major Boom		 */
};

DEATH_DATA Predator_Deaths[] = {
	{
		HMSQT_PredatorStand,	/* Sequence_Type	 */
		PSSS_Dies_Standard,		/* Sub_Sequence		 */
		(ONE_FIXED>>3),			/* TweeningTime		 */
		-1,						/* Sequence_Length	 */
		0,						/* Multiplayer_Code	 */
		(2<<16)|0,				/* Unique Code */
		0,						/* wound_flags		 */
		0,						/* priority_wounds	 */
		0,						/* Template			 */
		{
			0,						/* Front			 */
			0,						/* Back				 */
			0,						/* Left				 */
			0,						/* Right			 */
		},
		0,						/* Burning			 */
		0,						/* Electrical		 */
		0,						/* Crouching		 */
		0,						/* Minor Boom		 */
		0,						/* Major Boom		 */
	},
	{
		HMSQT_PredatorCrouch,	/* Sequence_Type	 */
		PCrSS_Dies_Standard,	/* Sub_Sequence		 */
		(ONE_FIXED>>3),			/* TweeningTime		 */
		-1,						/* Sequence_Length	 */
		1,						/* Multiplayer_Code	 */
		(2<<16)|1,				/* Unique Code */
		0,						/* wound_flags		 */
		0,						/* priority_wounds	 */
		0,						/* Template			 */
		{
			0,						/* Front			 */
			0,						/* Back				 */
			0,						/* Left				 */
			0,						/* Right			 */
		},
		0,						/* Burning			 */
		0,						/* Electrical		 */
		1,						/* Crouching		 */
		0,						/* Minor Boom		 */
		0,						/* Major Boom		 */
	},
	{
		HMSQT_PredatorStand,	/* Sequence_Type	 */
		PSSS_TemDeath_Fwrd,		/* Sub_Sequence		 */
		(ONE_FIXED>>3),			/* TweeningTime		 */
		-1,						/* Sequence_Length	 */
		2,						/* Multiplayer_Code	 */
		(2<<16)|2,				/* Unique Code */
		0,						/* wound_flags		 */
		0,						/* priority_wounds	 */
		1,						/* Template			 */
		{
			0,						/* Front			 */
			1,						/* Back				 */
			0,						/* Left				 */
			0,						/* Right			 */
		},
		0,						/* Burning			 */
		0,						/* Electrical		 */
		0,						/* Crouching		 */
		0,						/* Minor Boom		 */
		0,						/* Major Boom		 */
	},
	{
		HMSQT_PredatorStand,	/* Sequence_Type	 */
		PSSS_TemDeath_Bwrd,		/* Sub_Sequence		 */
		(ONE_FIXED>>3),			/* TweeningTime		 */
		-1,						/* Sequence_Length	 */
		3,						/* Multiplayer_Code	 */
		(2<<16)|3,				/* Unique Code */
		0,						/* wound_flags		 */
		0,						/* priority_wounds	 */
		1,						/* Template			 */
		{
			1,						/* Front			 */
			0,						/* Back				 */
			0,						/* Left				 */
			0,						/* Right			 */
		},
		0,						/* Burning			 */
		0,						/* Electrical		 */
		0,						/* Crouching		 */
		0,						/* Minor Boom		 */
		0,						/* Major Boom		 */
	},
	{
		HMSQT_PredatorStand,	/* Sequence_Type	 */
		PSSS_Tem_LeftArm,		/* Sub_Sequence		 */
		(ONE_FIXED>>3),			/* TweeningTime		 */
		-1,						/* Sequence_Length	 */
		4,						/* Multiplayer_Code	 */
		(2<<16)|4,				/* Unique Code */
		section_flag_left_arm,	/* wound_flags		 */
		0,						/* priority_wounds	 */
		1,						/* Template			 */
		{
			0,						/* Front			 */
			0,						/* Back				 */
			0,						/* Left				 */
			0,						/* Right			 */
		},
		0,						/* Burning			 */
		0,						/* Electrical		 */
		0,						/* Crouching		 */
		0,						/* Minor Boom		 */
		0,						/* Major Boom		 */
	},
	{
		HMSQT_PredatorStand,	/* Sequence_Type	 */
		PSSS_Tem_LeftLeg,		/* Sub_Sequence		 */
		(ONE_FIXED>>3),			/* TweeningTime		 */
		-1,						/* Sequence_Length	 */
		5,						/* Multiplayer_Code	 */
		(2<<16)|5,				/* Unique Code */
		section_flag_left_leg,	/* wound_flags		 */
		0,						/* priority_wounds	 */
		1,						/* Template			 */
		{
			0,						/* Front			 */
			0,						/* Back				 */
			0,						/* Left				 */
			0,						/* Right			 */
		},
		0,						/* Burning			 */
		0,						/* Electrical		 */
		0,						/* Crouching		 */
		0,						/* Minor Boom		 */
		0,						/* Major Boom		 */
	},
	{
		HMSQT_PredatorStand,	/* Sequence_Type	 */
		PSSS_Tem_RightArm,		/* Sub_Sequence		 */
		(ONE_FIXED>>3),			/* TweeningTime		 */
		-1,						/* Sequence_Length	 */
		6,						/* Multiplayer_Code	 */
		(2<<16)|6,				/* Unique Code */
		section_flag_right_arm,	/* wound_flags		 */
		0,						/* priority_wounds	 */
		1,						/* Template			 */
		{
			0,						/* Front			 */
			0,						/* Back				 */
			0,						/* Left				 */
			0,						/* Right			 */
		},
		0,						/* Burning			 */
		0,						/* Electrical		 */
		0,						/* Crouching		 */
		0,						/* Minor Boom		 */
		0,						/* Major Boom		 */
	},
	{
		HMSQT_PredatorStand,	/* Sequence_Type	 */
		PSSS_Tem_RightLeg,		/* Sub_Sequence		 */
		(ONE_FIXED>>3),			/* TweeningTime		 */
		-1,						/* Sequence_Length	 */
		7,						/* Multiplayer_Code	 */
		(2<<16)|7,				/* Unique Code */
		section_flag_right_leg,	/* wound_flags		 */
		0,						/* priority_wounds	 */
		1,						/* Template			 */
		{
			0,						/* Front			 */
			0,						/* Back				 */
			0,						/* Left				 */
			0,						/* Right			 */
		},
		0,						/* Burning			 */
		0,						/* Electrical		 */
		0,						/* Crouching		 */
		0,						/* Minor Boom		 */
		0,						/* Major Boom		 */
	},
	{
		HMSQT_PredatorStand,	/* Sequence_Type	 */
		PSSS_Tem_Riddled,		/* Sub_Sequence		 */
		(ONE_FIXED>>3),			/* TweeningTime		 */
		-1,						/* Sequence_Length	 */
		8,						/* Multiplayer_Code	 */
		(2<<16)|8,				/* Unique Code */
		0,						/* wound_flags		 */
		0,						/* priority_wounds	 */
		1,						/* Template			 */
		{
			0,						/* Front			 */
			0,						/* Back				 */
			0,						/* Left				 */
			0,						/* Right			 */
		},
		0,						/* Burning			 */
		0,						/* Electrical		 */
		0,						/* Crouching		 */
		1,						/* Minor Boom		 */
		1,						/* Major Boom		 */
	},
	{
		HMSQT_PredatorStand,	/* Sequence_Type	 */
		PSSS_Tem_Burning,		/* Sub_Sequence		 */
		(ONE_FIXED>>3),			/* TweeningTime		 */
		-1,						/* Sequence_Length	 */
		9,						/* Multiplayer_Code	 */
		(2<<16)|9,				/* Unique Code */
		0,						/* wound_flags		 */
		0,						/* priority_wounds	 */
		1,						/* Template			 */
		{
			0,						/* Front			 */
			0,						/* Back				 */
			0,						/* Left				 */
			0,						/* Right			 */
		},
		1,						/* Burning			 */
		0,						/* Electrical		 */
		0,						/* Crouching		 */
		0,						/* Minor Boom		 */
		0,						/* Major Boom		 */
	},
	{
		HMSQT_PredatorCrouch,	/* Sequence_Type	 */
		PCrSS_Dies_Standard,	/* Sub_Sequence		 */
		(ONE_FIXED>>3),			/* TweeningTime		 */
		-1,						/* Sequence_Length	 */
		10,						/* Multiplayer_Code	 */
		(2<<16)|10,				/* Unique Code */
		0,						/* wound_flags		 */
		0,						/* priority_wounds	 */
		1,						/* Template			 */
		{
			0,						/* Front			 */
			0,						/* Back				 */
			0,						/* Left				 */
			0,						/* Right			 */
		},
		0,						/* Burning			 */
		0,						/* Electrical		 */
		1,						/* Crouching		 */
		0,						/* Minor Boom		 */
		0,						/* Major Boom		 */
	},
	{
		-1,						/* Sequence_Type	 */
		-1,						/* Sub_Sequence		 */
		0,						/* TweeningTime		 */
		0,						/* Sequence_Length	 */
		0,						/* Multiplayer_Code	 */
		(2<<16)|0,				/* Unique Code */
		0,						/* wound_flags		 */
		0,						/* priority_wounds	 */
		0,						/* Template			 */
		{
			0,						/* Front			 */
			0,						/* Back				 */
			0,						/* Left				 */
			0,						/* Right			 */
		},
		0,						/* Burning			 */
		0,						/* Electrical		 */
		0,						/* Crouching		 */
		0,						/* Minor Boom		 */
		0,						/* Major Boom		 */
	},
};

DEATH_DATA Xenoborg_Deaths[] = {
	{
		HMSQT_Xenoborg,			/* Sequence_Type	 */
		XBSS_Die_Backwards,		/* Sub_Sequence		 */
		(ONE_FIXED>>2),			/* TweeningTime		 */
		-1,						/* Sequence_Length	 */
		0,						/* Multiplayer_Code	 */
		(3<<16)|0,				/* Unique Code */
		0,						/* wound_flags		 */
		0,						/* priority_wounds	 */
		0,						/* Template			 */
		{
			1,						/* Front			 */
			0,						/* Back				 */
			0,						/* Left				 */
			0,						/* Right			 */
		},
		0,						/* Burning			 */
		0,						/* Electrical		 */
		0,						/* Crouching		 */
		0,						/* Minor Boom		 */
		0,						/* Major Boom		 */
	},
	{
		HMSQT_Xenoborg,			/* Sequence_Type	 */
		XBSS_Die_Forwards,		/* Sub_Sequence		 */
		(ONE_FIXED>>2),			/* TweeningTime		 */
		-1,						/* Sequence_Length	 */
		1,						/* Multiplayer_Code	 */
		(3<<16)|1,				/* Unique Code */
		0,						/* wound_flags		 */
		0,						/* priority_wounds	 */
		0,						/* Template			 */
		{
			0,						/* Front			 */
			1,						/* Back				 */
			0,						/* Left				 */
			0,						/* Right			 */
		},
		0,						/* Burning			 */
		0,						/* Electrical		 */
		0,						/* Crouching		 */
		0,						/* Minor Boom		 */
		0,						/* Major Boom		 */
	},
	{
		HMSQT_Xenoborg,			/* Sequence_Type	 */
		XBSS_Die_Backwards,		/* Sub_Sequence		 */
		(ONE_FIXED>>2),			/* TweeningTime		 */
		-1,						/* Sequence_Length	 */
		2,						/* Multiplayer_Code	 */
		(3<<16)|2,				/* Unique Code */
		0,						/* wound_flags		 */
		0,						/* priority_wounds	 */
		0,						/* Template			 */
		{
			1,						/* Front			 */
			0,						/* Back				 */
			0,						/* Left				 */
			0,						/* Right			 */
		},
		0,						/* Burning			 */
		0,						/* Electrical		 */
		0,						/* Crouching		 */
		1,						/* Minor Boom		 */
		1,						/* Major Boom		 */
	},
	{
		HMSQT_Xenoborg,			/* Sequence_Type	 */
		XBSS_Die_Forwards,		/* Sub_Sequence		 */
		(ONE_FIXED>>2),			/* TweeningTime		 */
		-1,						/* Sequence_Length	 */
		3,						/* Multiplayer_Code	 */
		(3<<16)|3,				/* Unique Code */
		0,						/* wound_flags		 */
		0,						/* priority_wounds	 */
		0,						/* Template			 */
		{
			0,						/* Front			 */
			1,						/* Back				 */
			0,						/* Left				 */
			0,						/* Right			 */
		},
		0,						/* Burning			 */
		0,						/* Electrical		 */
		0,						/* Crouching		 */
		1,						/* Minor Boom		 */
		1,						/* Major Boom		 */
	},
	{
		HMSQT_Xenoborg,			/* Sequence_Type	 */
		XBSS_Standing_Death,	/* Sub_Sequence		 */
		(ONE_FIXED>>2),			/* TweeningTime		 */
		-1,						/* Sequence_Length	 */
		4,						/* Multiplayer_Code	 */
		(3<<16)|4,				/* Unique Code */
		0,						/* wound_flags		 */
		0,						/* priority_wounds	 */
		0,						/* Template			 */
		{
			0,						/* Front			 */
			0,						/* Back				 */
			0,						/* Left				 */
			0,						/* Right			 */
		},
		0,						/* Burning			 */
		0,						/* Electrical		 */
		0,						/* Crouching		 */
		0,						/* Minor Boom		 */
		0,						/* Major Boom		 */
	},
	{
		HMSQT_Xenoborg,			/* Sequence_Type	 */
		XBSS_Standing_Death,	/* Sub_Sequence		 */
		(ONE_FIXED>>2),			/* TweeningTime		 */
		-1,						/* Sequence_Length	 */
		5,						/* Multiplayer_Code	 */
		(3<<16)|5,				/* Unique Code */
		0,						/* wound_flags		 */
		0,						/* priority_wounds	 */
		0,						/* Template			 */
		{
			0,						/* Front			 */
			0,						/* Back				 */
			0,						/* Left				 */
			0,						/* Right			 */
		},
		1,						/* Burning			 */
		0,						/* Electrical		 */
		0,						/* Crouching		 */
		0,						/* Minor Boom		 */
		0,						/* Major Boom		 */
	},
	{
		HMSQT_Xenoborg,			/* Sequence_Type	 */
		XBSS_LeftLegMissingDeath,	/* Sub_Sequence		 */
		(ONE_FIXED>>4),			/* TweeningTime		 */
		-1,						/* Sequence_Length	 */
		6,						/* Multiplayer_Code	 */
		(3<<16)|6,				/* Unique Code */
		section_flag_left_leg,	/* wound_flags		 */
		section_flag_left_leg|section_flag_left_foot,	/* priority_wounds	 */
		0,						/* Template			 */
		{
			0,						/* Front			 */
			0,						/* Back				 */
			0,						/* Left				 */
			0,						/* Right			 */
		},
		0,						/* Burning			 */
		0,						/* Electrical		 */
		0,						/* Crouching		 */
		0,						/* Minor Boom		 */
		0,						/* Major Boom		 */
	},
	{
		HMSQT_Xenoborg,			/* Sequence_Type	 */
		XBSS_RightLegMissingDeath,	/* Sub_Sequence		 */
		(ONE_FIXED>>4),			/* TweeningTime		 */
		-1,						/* Sequence_Length	 */
		7,						/* Multiplayer_Code	 */
		(3<<16)|7,				/* Unique Code */
		section_flag_right_leg,	/* wound_flags		 */
		section_flag_right_leg|section_flag_right_foot,	/* priority_wounds	 */
		0,						/* Template			 */
		{
			0,						/* Front			 */
			0,						/* Back				 */
			0,						/* Left				 */
			0,						/* Right			 */
		},
		0,						/* Burning			 */
		0,						/* Electrical		 */
		0,						/* Crouching		 */
		0,						/* Minor Boom		 */
		0,						/* Major Boom		 */
	},
	{
		HMSQT_Xenoborg,			/* Sequence_Type	 */
		XBSS_LeftLegMissingDeath,	/* Sub_Sequence		 */
		(ONE_FIXED>>4),			/* TweeningTime		 */
		-1,						/* Sequence_Length	 */
		8,						/* Multiplayer_Code	 */
		(3<<16)|8,				/* Unique Code */
		section_flag_left_leg,	/* wound_flags		 */
		section_flag_left_leg|section_flag_left_foot,	/* priority_wounds	 */
		0,						/* Template			 */
		{
			0,						/* Front			 */
			0,						/* Back				 */
			0,						/* Left				 */
			0,						/* Right			 */
		},
		0,						/* Burning			 */
		0,						/* Electrical		 */
		0,						/* Crouching		 */
		1,						/* Minor Boom		 */
		1,						/* Major Boom		 */
	},
	{
		HMSQT_Xenoborg,			/* Sequence_Type	 */
		XBSS_RightLegMissingDeath,	/* Sub_Sequence		 */
		(ONE_FIXED>>4),			/* TweeningTime		 */
		-1,						/* Sequence_Length	 */
		9,						/* Multiplayer_Code	 */
		(3<<16)|9,				/* Unique Code */
		section_flag_right_leg,	/* wound_flags		 */
		section_flag_right_leg|section_flag_right_foot,	/* priority_wounds	 */
		0,						/* Template			 */
		{
			0,						/* Front			 */
			0,						/* Back				 */
			0,						/* Left				 */
			0,						/* Right			 */
		},
		0,						/* Burning			 */
		0,						/* Electrical		 */
		0,						/* Crouching		 */
		1,						/* Minor Boom		 */
		1,						/* Major Boom		 */
	},
	{
		-1,						/* Sequence_Type	 */
		-1,						/* Sub_Sequence		 */
		0,						/* TweeningTime		 */
		0,						/* Sequence_Length	 */
		0,						/* Multiplayer_Code	 */
		(3<<16)|0,				/* Unique Code */
		0,						/* wound_flags		 */
		0,						/* priority_wounds	 */
		0,						/* Template			 */
		{
			0,						/* Front			 */
			0,						/* Back				 */
			0,						/* Left				 */
			0,						/* Right			 */
		},
		0,						/* Burning			 */
		0,						/* Electrical		 */
		0,						/* Crouching		 */
		0,						/* Minor Boom		 */
		0,						/* Major Boom		 */
	},
};

/* And now, in a change from our advertised programme, Alien Attacks. */

ATTACK_DATA Alien_Special_Gripping_Attack = {
	HMSQT_AlienStand,		/* Sequence_Type	 */
	ASSS_Feed,				/* Sub_Sequence		 */
	(ONE_FIXED>>2),			/* TweeningTime		 */
	-1,				 		/* Sequence_Length	 */
	0,						/* Multiplayer_Code	 */
	(0<<16)|0,						/* Unique_Code	 */
	0,						/* wound_flags		 */
	{						/* flag_damage		 */
		AMMO_NPC_ALIEN_BITE,
		AMMO_NPC_ALIEN_BITE,
		AMMO_NPC_ALIEN_BITE,
	},
	1,						/* Crouching		 */
	0,						/* Pouncing			 */
};

ATTACK_DATA Alien_Attacks[] = {

	{
		HMSQT_AlienCrouch,		/* Sequence_Type	 */
		ACrSS_Attack_Bite,		/* Sub_Sequence		 */
		(ONE_FIXED>>2),			/* TweeningTime		 */
		-1,				 		/* Sequence_Length	 */
		0,						/* Multiplayer_Code	 */
		(1<<16)|0,	 				/* Unique_Code	 */
		0,						/* wound_flags		 */
		{						/* flag_damage		 */
			AMMO_NPC_ALIEN_BITE,
			AMMO_NPC_ALIEN_BITE,
			AMMO_NPC_ALIEN_BITE,
		},
		1,						/* Crouching		 */
		0,						/* Pouncing			 */
	},
	{
		HMSQT_AlienCrouch,		/* Sequence_Type	 */
		ACrSS_Attack_Tail,		/* Sub_Sequence		 */
		(ONE_FIXED>>2),			/* TweeningTime		 */
		-1,				 		/* Sequence_Length	 */
		1,						/* Multiplayer_Code	 */
		(1<<16)|1,	 				/* Unique_Code	 */
		section_flag_tail,		/* wound_flags		 */
		{						/* flag_damage		 */
			AMMO_NPC_ALIEN_TAIL,
			AMMO_NPC_ALIEN_TAIL,
			AMMO_NPC_ALIEN_TAIL,
		},
		1,						/* Crouching		 */
		0,						/* Pouncing			 */
	},
	{
		HMSQT_AlienCrouch,		/* Sequence_Type	 */
		ACrSS_Attack_Swipe,		/* Sub_Sequence		 */
		(ONE_FIXED>>2),			/* TweeningTime		 */
		-1,				 		/* Sequence_Length	 */
		2,						/* Multiplayer_Code	 */
		(1<<16)|2,	 				/* Unique_Code	 */
		section_flag_left_hand,	/* wound_flags		 */
		{						/* flag_damage		 */
			AMMO_NPC_ALIEN_CLAW,
			AMMO_NPC_ALIEN_CLAW,
			AMMO_NPC_ALIEN_CLAW,
		},
		1,						/* Crouching		 */
		0,						/* Pouncing			 */
	},
	{
		HMSQT_AlienStand,		/* Sequence_Type	 */
		ASSS_Attack_Bite,		/* Sub_Sequence		 */
		(ONE_FIXED>>2),			/* TweeningTime		 */
		-1,						/* Sequence_Length	 */
		3,						/* Multiplayer_Code	 */
		(1<<16)|3,	 				/* Unique_Code	 */
		0,						/* wound_flags		 */
		{						/* flag_damage		 */
			AMMO_NPC_ALIEN_BITE,
			AMMO_NPC_ALIEN_BITE,
			AMMO_NPC_ALIEN_BITE,
		},
		0,						/* Crouching		 */
		0,						/* Pouncing			 */
	},
	{
		HMSQT_AlienStand,		/* Sequence_Type	 */
		ASSS_Attack_Left_Swipe_In,/* Sub_Sequence	 */
		(ONE_FIXED>>2),			/* TweeningTime		 */
		-1,						/* Sequence_Length	 */
		4,						/* Multiplayer_Code	 */
		(1<<16)|4,	 				/* Unique_Code	 */
		section_flag_left_hand,	/* wound_flags		 */
		{						/* flag_damage		 */
			AMMO_NPC_ALIEN_CLAW,
			AMMO_NPC_ALIEN_CLAW,
			AMMO_NPC_ALIEN_CLAW,
		},
		0,						/* Crouching		 */
		0,						/* Pouncing			 */
	},
	{
		HMSQT_AlienStand,		/* Sequence_Type	 */
		ASSS_Attack_Right_Swipe_In,	/* Sub_Sequence	 */
		(ONE_FIXED>>2),			/* TweeningTime		 */
		-1,						/* Sequence_Length	 */
		5,						/* Multiplayer_Code	 */
		(1<<16)|5,	 				/* Unique_Code	 */
		section_flag_right_hand,/* wound_flags		 */
		{						/* flag_damage		 */
			AMMO_NPC_ALIEN_CLAW,
			AMMO_NPC_ALIEN_CLAW,
			AMMO_NPC_ALIEN_CLAW,
		},
		0,						/* Crouching		 */
		0,						/* Pouncing			 */
	},
	{
		HMSQT_AlienStand,		/* Sequence_Type	 */
		ASSS_Attack_Tail,		/* Sub_Sequence	 */
		(ONE_FIXED>>2),			/* TweeningTime		 */
		-1,						/* Sequence_Length	 */
		5,						/* Multiplayer_Code	 */
		(1<<16)|6,	 				/* Unique_Code	 */
		section_flag_tail,		/* wound_flags		 */
		{						/* flag_damage		 */
			AMMO_NPC_ALIEN_TAIL,
			AMMO_NPC_ALIEN_TAIL,
			AMMO_NPC_ALIEN_TAIL,
		},
		0,						/* Crouching		 */
		0,						/* Pouncing			 */
	},
	{
		HMSQT_AlienCrouch,		/* Sequence_Type	 */
		ACrSS_Pounce,			/* Sub_Sequence		 */
		(ONE_FIXED>>2),			/* TweeningTime		 */
		ALIEN_ATTACKTIME, 		/* Sequence_Length	 */
		6,						/* Multiplayer_Code	 */
		(1<<16)|7,	 				/* Unique_Code	 */
		section_flag_tail,		/* wound_flags		 */
		{						/* flag_damage		 */
			AMMO_NPC_ALIEN_TAIL,
			AMMO_NPC_ALIEN_CLAW,
			AMMO_NPC_ALIEN_CLAW,
		},
		1,						/* Crouching		 */
		1,						/* Pouncing			 */
	},
	{
		HMSQT_AlienStand,		/* Sequence_Type	 */
		ASSS_Attack_Both_In,	/* Sub_Sequence	 */
		(ONE_FIXED>>2),			/* TweeningTime		 */
		-1,						/* Sequence_Length	 */
		7,						/* Multiplayer_Code	 */
		(1<<16)|8,	 				/* Unique_Code	 */
		section_flag_right_hand|section_flag_left_hand,/* wound_flags		 */
		{						/* flag_damage		 */
			AMMO_NPC_ALIEN_CLAW,
			AMMO_NPC_ALIEN_CLAW,
			AMMO_NPC_ALIEN_CLAW,
		},
		0,						/* Crouching		 */
		0,						/* Pouncing			 */
	},
	{
		HMSQT_AlienStand,		/* Sequence_Type	 */
		ASSS_Attack_Both_Down,	/* Sub_Sequence	 */
		(ONE_FIXED>>2),			/* TweeningTime		 */
		-1,						/* Sequence_Length	 */
		8,						/* Multiplayer_Code	 */
		(1<<16)|9,	 				/* Unique_Code	 */
		section_flag_right_hand|section_flag_left_hand,/* wound_flags		 */
		{						/* flag_damage		 */
			AMMO_NPC_ALIEN_CLAW,
			AMMO_NPC_ALIEN_CLAW,
			AMMO_NPC_ALIEN_CLAW,
		},
		0,						/* Crouching		 */
		0,						/* Pouncing			 */
	},
	{
		HMSQT_AlienStand,		/* Sequence_Type	 */
		ASSS_Attack_Low_Left_Swipe,/* Sub_Sequence	 */
		(ONE_FIXED>>2),			/* TweeningTime		 */
		-1,						/* Sequence_Length	 */
		9,						/* Multiplayer_Code	 */
		(1<<16)|10,	 				/* Unique_Code	 */
		section_flag_left_hand,	/* wound_flags		 */
		{						/* flag_damage		 */
			AMMO_NPC_ALIEN_CLAW,
			AMMO_NPC_ALIEN_CLAW,
			AMMO_NPC_ALIEN_CLAW,
		},
		0,						/* Crouching		 */
		0,						/* Pouncing			 */
	},
	{
		HMSQT_AlienStand,		/* Sequence_Type	 */
		ASSS_Attack_Low_Right_Swipe, /* Sub_Sequence	 */
		(ONE_FIXED>>2),			/* TweeningTime		 */
		-1,						/* Sequence_Length	 */
		10,						/* Multiplayer_Code	 */
		(1<<16)|11,	 				/* Unique_Code	 */
		section_flag_right_hand,/* wound_flags		 */
		{						/* flag_damage		 */
			AMMO_NPC_ALIEN_CLAW,
			AMMO_NPC_ALIEN_CLAW,
			AMMO_NPC_ALIEN_CLAW,
		},
		0,						/* Crouching		 */
		0,						/* Pouncing			 */
	},
	{
		-1,						/* Sequence_Type	 */
		-1,						/* Sub_Sequence		 */
		0,						/* TweeningTime		 */
		0,						/* Sequence_Length	 */
		0,						/* Multiplayer_Code	 */
		0,	 				/* Unique_Code	 */
		0,						/* wound_flags		 */
        {
            AMMO_NONE				/* damage_type		 */
        },
		0,						/* Crouching		 */
		0,						/* Pouncing			 */
	},

};

ATTACK_DATA Wristblade_Attacks[] = {

	{
		HMSQT_PredatorCrouch,	/* Sequence_Type	 */
		PCrSS_Attack_Primary,	/* Sub_Sequence		 */
		(ONE_FIXED>>2),			/* TweeningTime		 */
		-1,				 		/* Sequence_Length	 */
		0,						/* Multiplayer_Code	 */
		(2<<16)|0,	 				/* Unique_Code	 */
		0,						/* wound_flags		 */
		{						/* flag_damage		 */
			AMMO_PRED_WRISTBLADE,
			AMMO_PRED_WRISTBLADE,
			AMMO_PRED_WRISTBLADE,
		},
		1,						/* Crouching		 */
		0,						/* Pouncing			 */
	},
	{
		HMSQT_PredatorStand,	/* Sequence_Type	 */
		PSSS_Attack_Primary,	/* Sub_Sequence		 */
		(ONE_FIXED>>2),			/* TweeningTime		 */
		-1,				 		/* Sequence_Length	 */
		1,						/* Multiplayer_Code	 */
		(2<<16)|1,	 				/* Unique_Code	 */
		0,						/* wound_flags		 */
		{						/* flag_damage		 */
			AMMO_HEAVY_PRED_WRISTBLADE,
			AMMO_HEAVY_PRED_WRISTBLADE,
			AMMO_HEAVY_PRED_WRISTBLADE,
		},
		0,						/* Crouching		 */
		0,						/* Pouncing			 */
	},
	{
		HMSQT_PredatorStand,	/* Sequence_Type	 */
		PSSS_Attack_Quick_Jab,	/* Sub_Sequence		 */
		(ONE_FIXED>>2),			/* TweeningTime		 */
		-1,				 		/* Sequence_Length	 */
		2,						/* Multiplayer_Code	 */
		(2<<16)|2, 				/* Unique_Code	 */
		0,						/* wound_flags		 */
		{						/* flag_damage		 */
			AMMO_PRED_WRISTBLADE,
			AMMO_PRED_WRISTBLADE,
			AMMO_PRED_WRISTBLADE,
		},
		0,						/* Crouching		 */
		0,						/* Pouncing			 */
	},
	{
		HMSQT_PredatorStand,	/* Sequence_Type	 */
		PSSS_Attack_Uppercut,	/* Sub_Sequence		 */
		(ONE_FIXED>>2),			/* TweeningTime		 */
		-1,				 		/* Sequence_Length	 */
		3,						/* Multiplayer_Code	 */
		(2<<16)|3, 				/* Unique_Code	 */
		0,						/* wound_flags		 */
		{						/* flag_damage		 */
			AMMO_HEAVY_PRED_WRISTBLADE,
			AMMO_HEAVY_PRED_WRISTBLADE,
			AMMO_HEAVY_PRED_WRISTBLADE,
		},
		0,						/* Crouching		 */
		0,						/* Pouncing			 */
	},
	{
		-1,						/* Sequence_Type	 */
		-1,						/* Sub_Sequence		 */
		0,						/* TweeningTime		 */
		0,						/* Sequence_Length	 */
		0,						/* Multiplayer_Code	 */
		0,		 				/* Unique_Code	 */
		0,						/* wound_flags		 */
        {
            AMMO_NONE				/* damage_type		 */
        },
		0,						/* Crouching		 */
		0,						/* Pouncing			 */
	},

};

ATTACK_DATA PredStaff_Attacks[] = {

	{
		HMSQT_PredatorCrouch,	/* Sequence_Type	 */
		PCrSS_Attack_Primary,	/* Sub_Sequence		 */
		(ONE_FIXED>>2),			/* TweeningTime		 */
		-1,				 		/* Sequence_Length	 */
		0,						/* Multiplayer_Code	 */
		(3<<16)|0, 				/* Unique_Code	 */
		0,						/* wound_flags		 */
		{						/* flag_damage		 */
			AMMO_NPC_PRED_STAFF,
			AMMO_NPC_PRED_STAFF,
			AMMO_NPC_PRED_STAFF,
		},
		1,						/* Crouching		 */
		0,						/* Pouncing			 */
	},
	{
		HMSQT_PredatorStand,	/* Sequence_Type	 */
		PSSS_Attack_Primary,	/* Sub_Sequence		 */
		(ONE_FIXED>>2),			/* TweeningTime		 */
		-1,				 		/* Sequence_Length	 */
		1,						/* Multiplayer_Code	 */
		(3<<16)|1, 				/* Unique_Code	 */
		0,						/* wound_flags		 */
		{						/* flag_damage		 */
			AMMO_NPC_PRED_STAFF,
			AMMO_NPC_PRED_STAFF,
			AMMO_NPC_PRED_STAFF,
		},
		0,						/* Crouching		 */
		0,						/* Pouncing			 */
	},
	{
		HMSQT_PredatorStand,	/* Sequence_Type	 */
		PSSS_Attack_Offense_Sweep,	/* Sub_Sequence		 */
		(ONE_FIXED>>2),			/* TweeningTime		 */
		-1,				 		/* Sequence_Length	 */
		2,						/* Multiplayer_Code	 */
		(3<<16)|2, 				/* Unique_Code	 */
		0,						/* wound_flags		 */
		{						/* flag_damage		 */
			AMMO_NPC_PRED_STAFF,
			AMMO_NPC_PRED_STAFF,
			AMMO_NPC_PRED_STAFF,
		},
		0,						/* Crouching		 */
		0,						/* Pouncing			 */
	},
	{
		HMSQT_PredatorStand,	/* Sequence_Type	 */
		PSSS_Attack_Defence_Stab,	/* Sub_Sequence		 */
		(ONE_FIXED>>2),			/* TweeningTime		 */
		-1,				 		/* Sequence_Length	 */
		3,						/* Multiplayer_Code	 */
		(3<<16)|3, 				/* Unique_Code	 */
		0,						/* wound_flags		 */
		{						/* flag_damage		 */
			AMMO_NPC_PRED_STAFF,
			AMMO_NPC_PRED_STAFF,
			AMMO_NPC_PRED_STAFF,
		},
		0,						/* Crouching		 */
		0,						/* Pouncing			 */
	},
	{
		HMSQT_PredatorStand,	/* Sequence_Type	 */
		PSSS_Attack_Defence_Sweep,	/* Sub_Sequence		 */
		(ONE_FIXED>>2),			/* TweeningTime		 */
		-1,				 		/* Sequence_Length	 */
		4,						/* Multiplayer_Code	 */
		(3<<16)|4, 				/* Unique_Code	 */
		0,						/* wound_flags		 */
		{						/* flag_damage		 */
			AMMO_NPC_PRED_STAFF,
			AMMO_NPC_PRED_STAFF,
			AMMO_NPC_PRED_STAFF,
		},
		0,						/* Crouching		 */
		0,						/* Pouncing			 */
	},
	{
		-1,						/* Sequence_Type	 */
		-1,						/* Sub_Sequence		 */
		0,						/* TweeningTime		 */
		0,						/* Sequence_Length	 */
		0,						/* Multiplayer_Code	 */
		0,		 				/* Unique_Code	 */
		0,						/* wound_flags		 */
        {
            AMMO_NONE    		/* damage_type		 */
        },
		0,						/* Crouching		 */
		0,						/* Pouncing			 */
	},

};
