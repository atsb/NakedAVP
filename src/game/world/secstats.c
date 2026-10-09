/***** Secstats.c *****/
/***** CDF 21/11/97 *****/

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
#include "lighting.h"
#include "bh_weap.h"
#include "weapons.h"
#include "psnd.h"
#include "load_shp.h"

#define UseLocalAssert Yes
#include "ourasert.h"
#include "particle.h"


/* Marine With Pulse Rifle Centre Locations */
HITLOCATIONTABLEENTRY MWPRCL[] = {
	/* Centre locs */
	{"pelvis",	centre_aspect,	ONE_FIXED/28},
	{"Lthigh",	centre_aspect,	ONE_FIXED/28},
	{"Lshin",	centre_aspect,	ONE_FIXED/28},
	{"Lfoot",	centre_aspect,	ONE_FIXED/28},
	{"Lpouch",	centre_aspect,	ONE_FIXED/28},
	{"Rthigh",	centre_aspect,	ONE_FIXED/28},
	{"Rshin",	centre_aspect,	ONE_FIXED/28},
	{"Rfoot",	centre_aspect,	ONE_FIXED/28},
	{"Rpouch",	centre_aspect,	ONE_FIXED/28},
	{"chest",	centre_aspect,	ONE_FIXED/28},
	{"Rbicep",	centre_aspect,	ONE_FIXED/28},
	{"Rforearm",centre_aspect,	ONE_FIXED/28},
	{"Rpalm",	centre_aspect,	ONE_FIXED/28},
	{"R1fingers",centre_aspect,	ONE_FIXED/28},
	{"R2fingers",centre_aspect,	ONE_FIXED/28},
	{"R1thumb",	centre_aspect,	ONE_FIXED/28},
	{"R2thumb",	centre_aspect,	ONE_FIXED/28},
	{"Rpad",	centre_aspect,	ONE_FIXED/28},
	{"Lbicep",	centre_aspect,	ONE_FIXED/28},
	{"Lforearm",centre_aspect,	ONE_FIXED/28},
	{"Lpalm",	centre_aspect,	ONE_FIXED/28},
	{"L1fingers",centre_aspect,	ONE_FIXED/28},
	{"L2fingers",centre_aspect,	ONE_FIXED/28},
	{"L1thumb",	centre_aspect,	ONE_FIXED/28},
	{"L2thumb",	centre_aspect,	ONE_FIXED/28},
	{"Lpad",	centre_aspect,	ONE_FIXED/28},
	{"neck",	centre_aspect,	ONE_FIXED/28},
	{"head",	centre_aspect,	ONE_FIXED/28},
	{NULL,		centre_aspect,	0},
};

HITLOCATIONTABLEENTRY MWPRTL[] = {
	/* Top locs */
	{"chest",	centre_aspect,	ONE_FIXED/11},
	{"Rbicep",	centre_aspect,	ONE_FIXED/11},
	{"Rforearm",centre_aspect,	ONE_FIXED/11},
	{"Rpalm",	centre_aspect,	ONE_FIXED/11},
	{"Rpad",	centre_aspect,	ONE_FIXED/11},
	{"Lbicep",	centre_aspect,	ONE_FIXED/11},
	{"Lforearm",centre_aspect,	ONE_FIXED/11},
	{"Lpalm",	centre_aspect,	ONE_FIXED/11},
	{"Lpad",	centre_aspect,	ONE_FIXED/11},
	{"neck",	centre_aspect,	ONE_FIXED/11},
	{"head",	centre_aspect,	ONE_FIXED/11},
	{NULL,		centre_aspect,	0},
};

HITLOCATIONTABLEENTRY MWPRBL[] = {
	/* Base locs */
	{"pelvis",	centre_aspect,	ONE_FIXED/9},
	{"Lthigh",	centre_aspect,	ONE_FIXED/9},
	{"Lshin",	centre_aspect,	ONE_FIXED/9},
	{"Lfoot",	centre_aspect,	ONE_FIXED/9},
	{"Lpouch",	centre_aspect,	ONE_FIXED/9},
	{"Rthigh",	centre_aspect,	ONE_FIXED/9},
	{"Rshin",	centre_aspect,	ONE_FIXED/9},
	{"Rfoot",	centre_aspect,	ONE_FIXED/9},
	{"Rpouch",	centre_aspect,	ONE_FIXED/9},
	{NULL,		centre_aspect,	0},
};

HITLOCATIONTABLEENTRY MWPRLL[] = {
	/* Left locs */
	{"pelvis",	centre_aspect,	ONE_FIXED/16},
	{"Lthigh",	centre_aspect,	ONE_FIXED/16},
	{"Lshin",	centre_aspect,	ONE_FIXED/16},
	{"Lfoot",	centre_aspect,	ONE_FIXED/16},
	{"Lpouch",	centre_aspect,	ONE_FIXED/16},
	{"chest",	centre_aspect,	ONE_FIXED/16},
	{"Lbicep",	centre_aspect,	ONE_FIXED/16},
	{"Lforearm",centre_aspect,	ONE_FIXED/16},
	{"Lpalm",	centre_aspect,	ONE_FIXED/16},
	{"L1fingers",centre_aspect,	ONE_FIXED/16},
	{"L2fingers",centre_aspect,	ONE_FIXED/16},
	{"L1thumb",	centre_aspect,	ONE_FIXED/16},
	{"L2thumb",	centre_aspect,	ONE_FIXED/16},
	{"Lpad",	centre_aspect,	ONE_FIXED/16},
	{"neck",	centre_aspect,	ONE_FIXED/16},
	{"head",	centre_aspect,	ONE_FIXED/16},
	{NULL,		centre_aspect,	0},
};

HITLOCATIONTABLEENTRY MWPRRL[] = {
	/* Right locs */
	{"pelvis",	centre_aspect,	ONE_FIXED/16},
	{"Rthigh",	centre_aspect,	ONE_FIXED/16},
	{"Rshin",	centre_aspect,	ONE_FIXED/16},
	{"Rfoot",	centre_aspect,	ONE_FIXED/16},
	{"Rpouch",	centre_aspect,	ONE_FIXED/16},
	{"chest",	centre_aspect,	ONE_FIXED/16},
	{"Rbicep",	centre_aspect,	ONE_FIXED/16},
	{"Rforearm",centre_aspect,	ONE_FIXED/16},
	{"Rpalm",	centre_aspect,	ONE_FIXED/16},
	{"R1fingers",centre_aspect,	ONE_FIXED/16},
	{"R2fingers",centre_aspect,	ONE_FIXED/16},
	{"R1thumb",	centre_aspect,	ONE_FIXED/16},
	{"R2thumb",	centre_aspect,	ONE_FIXED/16},
	{"Rpad",	centre_aspect,	ONE_FIXED/16},
	{"neck",	centre_aspect,	ONE_FIXED/16},
	{"head",	centre_aspect,	ONE_FIXED/16},
	{NULL,		centre_aspect,	0},
};

HITLOCATIONTABLEENTRY MWPRTLL[] = {
	/* TopLeft locs */
	{"chest",	centre_aspect,	ONE_FIXED/7},
	{"Lbicep",	centre_aspect,	ONE_FIXED/7},
	{"Lforearm",centre_aspect,	ONE_FIXED/7},
	{"Lpalm",	centre_aspect,	ONE_FIXED/7},
	{"Lpad",	centre_aspect,	ONE_FIXED/7},
	{"neck",	centre_aspect,	ONE_FIXED/7},
	{"head",	centre_aspect,	ONE_FIXED/7},
	{NULL,		centre_aspect,	0},
};

HITLOCATIONTABLEENTRY MWPRTRL[] = {
	/* TopRight locs */
	{"chest",	centre_aspect,	ONE_FIXED/7},
	{"Rbicep",	centre_aspect,	ONE_FIXED/7},
	{"Rforearm",centre_aspect,	ONE_FIXED/7},
	{"Rpalm",	centre_aspect,	ONE_FIXED/7},
	{"Rpad",	centre_aspect,	ONE_FIXED/7},
	{"neck",	centre_aspect,	ONE_FIXED/7},
	{"head",	centre_aspect,	ONE_FIXED/7},
	{NULL,		centre_aspect,	0},
};

HITLOCATIONTABLEENTRY MWPRBLL[] = {
	/* BaseLeft locs */
	{"pelvis",	centre_aspect,	ONE_FIXED/5},
	{"Lthigh",	centre_aspect,	ONE_FIXED/5},
	{"Lshin",	centre_aspect,	ONE_FIXED/5},
	{"Lfoot",	centre_aspect,	ONE_FIXED/5},
	{"Lpouch",	centre_aspect,	ONE_FIXED/5},
	{NULL,		centre_aspect,	0},
};

HITLOCATIONTABLEENTRY MWPRBRL[] = {
	/* BaseRight locs */
	{"pelvis",	centre_aspect,	ONE_FIXED/5},
	{"Rthigh",	centre_aspect,	ONE_FIXED/5},
	{"Rshin",	centre_aspect,	ONE_FIXED/5},
	{"Rfoot",	centre_aspect,	ONE_FIXED/5},
	{"Rpouch",	centre_aspect,	ONE_FIXED/5},
	{NULL,		centre_aspect,	0},
};

/* Civvies */

HITLOCATIONTABLEENTRY MCCL[] = {
	/* Centre locs */
	{"pelvis presley",		centre_aspect,	ONE_FIXED/23},
	{"male left thigh",		centre_aspect,	ONE_FIXED/23},
	{"male left shin",		centre_aspect,	ONE_FIXED/23},
	{"male left foot",		centre_aspect,	ONE_FIXED/23},
	{"male right thigh",	centre_aspect,	ONE_FIXED/23},
	{"male right shin",		centre_aspect,	ONE_FIXED/23},
	{"male right foot",		centre_aspect,	ONE_FIXED/23},
	{"stomach",				centre_aspect,	ONE_FIXED/23},
	{"chest",				centre_aspect,	ONE_FIXED/23},
	{"male right bicep",	centre_aspect,	ONE_FIXED/23},
	{"male right forearm",	centre_aspect,	ONE_FIXED/23},
	{"right palm",			centre_aspect,	ONE_FIXED/23},
	{"right fings top",		centre_aspect,	ONE_FIXED/23},
	{"right fings end",		centre_aspect,	ONE_FIXED/23},
	{"male right thumb",	centre_aspect,	ONE_FIXED/23},
	{"male left bicep",		centre_aspect,	ONE_FIXED/23},
	{"male left forearm",	centre_aspect,	ONE_FIXED/23},
	{"left palm",			centre_aspect,	ONE_FIXED/23},
	{"left fings top",		centre_aspect,	ONE_FIXED/23},
	{"left fings end",		centre_aspect,	ONE_FIXED/23},
	{"male left thumb",		centre_aspect,	ONE_FIXED/23},
	{"neck",				centre_aspect,	ONE_FIXED/23},
	{"head",				centre_aspect,	ONE_FIXED/23},
	{NULL,					centre_aspect,	0},
};

HITLOCATIONTABLEENTRY MCTL[] = {
	/* Top locs */
	{"chest",				centre_aspect,	ONE_FIXED/9},
	{"male right bicep",	centre_aspect,	ONE_FIXED/9},
	{"male right forearm",	centre_aspect,	ONE_FIXED/9},
	{"right palm",			centre_aspect,	ONE_FIXED/9},
	{"male left bicep",		centre_aspect,	ONE_FIXED/9},
	{"male left forearm",	centre_aspect,	ONE_FIXED/9},
	{"left palm",			centre_aspect,	ONE_FIXED/9},
	{"neck",				centre_aspect,	ONE_FIXED/9},
	{"head",				centre_aspect,	ONE_FIXED/9},
	{NULL,					centre_aspect,	0},
};

HITLOCATIONTABLEENTRY MCBL[] = {
	/* Base locs */
	{"pelvis presley",		centre_aspect,	ONE_FIXED/8},
	{"male left thigh",		centre_aspect,	ONE_FIXED/8},
	{"male left shin",		centre_aspect,	ONE_FIXED/8},
	{"male left foot",		centre_aspect,	ONE_FIXED/8},
	{"male right thigh",	centre_aspect,	ONE_FIXED/8},
	{"male right shin",		centre_aspect,	ONE_FIXED/8},
	{"male right foot",		centre_aspect,	ONE_FIXED/8},
	{"stomach",				centre_aspect,	ONE_FIXED/8},
	{NULL,					centre_aspect,	0},
};

HITLOCATIONTABLEENTRY MCLL[] = {
	/* Left locs */
	{"pelvis presley",		centre_aspect,	ONE_FIXED/11},
	{"male left thigh",		centre_aspect,	ONE_FIXED/11},
	{"male left shin",		centre_aspect,	ONE_FIXED/11},
	{"male left foot",		centre_aspect,	ONE_FIXED/11},
	{"stomach",				centre_aspect,	ONE_FIXED/11},
	{"chest",				centre_aspect,	ONE_FIXED/11},
	{"male left bicep",		centre_aspect,	ONE_FIXED/11},
	{"male left forearm",	centre_aspect,	ONE_FIXED/11},
	{"left palm",			centre_aspect,	ONE_FIXED/11},
	{"neck",				centre_aspect,	ONE_FIXED/11},
	{"head",				centre_aspect,	ONE_FIXED/11},
	{NULL,					centre_aspect,	0},
};

HITLOCATIONTABLEENTRY MCRL[] = {
	/* Right locs */
	{"pelvis presley",		centre_aspect,	ONE_FIXED/11},
	{"male right thigh",	centre_aspect,	ONE_FIXED/11},
	{"male right shin",		centre_aspect,	ONE_FIXED/11},
	{"male right foot",		centre_aspect,	ONE_FIXED/11},
	{"stomach",				centre_aspect,	ONE_FIXED/11},
	{"chest",				centre_aspect,	ONE_FIXED/11},
	{"male right bicep",	centre_aspect,	ONE_FIXED/11},
	{"male right forearm",	centre_aspect,	ONE_FIXED/11},
	{"right palm",			centre_aspect,	ONE_FIXED/11},
	{"neck",				centre_aspect,	ONE_FIXED/11},
	{"head",				centre_aspect,	ONE_FIXED/11},
	{NULL,					centre_aspect,	0},
};

HITLOCATIONTABLEENTRY MCTLL[] = {
	/* Top Left locs */
	{"chest",				centre_aspect,	ONE_FIXED/6},
	{"male left bicep",		centre_aspect,	ONE_FIXED/6},
	{"male left forearm",	centre_aspect,	ONE_FIXED/6},
	{"left palm",			centre_aspect,	ONE_FIXED/6},
	{"neck",				centre_aspect,	ONE_FIXED/6},
	{"head",				centre_aspect,	ONE_FIXED/6},
	{NULL,					centre_aspect,	0},
};

HITLOCATIONTABLEENTRY MCTRL[] = {
	/* Top Right locs */
	{"chest",				centre_aspect,	ONE_FIXED/6},
	{"male right bicep",	centre_aspect,	ONE_FIXED/6},
	{"male right forearm",	centre_aspect,	ONE_FIXED/6},
	{"right palm",			centre_aspect,	ONE_FIXED/6},
	{"neck",				centre_aspect,	ONE_FIXED/6},
	{"head",				centre_aspect,	ONE_FIXED/6},
	{NULL,					centre_aspect,	0},
};

HITLOCATIONTABLEENTRY MCBLL[] = {
	/* Base Left locs */
	{"pelvis presley",		centre_aspect,	ONE_FIXED/4},
	{"male left thigh",		centre_aspect,	ONE_FIXED/4},
	{"male left shin",		centre_aspect,	ONE_FIXED/4},
	{"male left foot",		centre_aspect,	ONE_FIXED/4},
	{NULL,					centre_aspect,	0},
};

HITLOCATIONTABLEENTRY MCBRL[] = {
	/* Base Right locs */
	{"pelvis presley",		centre_aspect,	ONE_FIXED/4},
	{"male right thigh",	centre_aspect,	ONE_FIXED/4},
	{"male right shin",		centre_aspect,	ONE_FIXED/4},
	{"male right foot",		centre_aspect,	ONE_FIXED/4},
	{NULL,					centre_aspect,	0},
};

/* ALIENS */

HITLOCATIONTABLEENTRY AlCL[] = {
	/* Centre Locs */
	{"abdom",		centre_aspect,	ONE_FIXED/21},
	{"left bicep",	centre_aspect,	ONE_FIXED/21},
	{"left forearm",centre_aspect,	ONE_FIXED/21},
	{"left palm",	centre_aspect,	ONE_FIXED/21},
	{"right bicep",	centre_aspect,	ONE_FIXED/21},
	{"right forearm",centre_aspect,	ONE_FIXED/21},
	{"right palm",	centre_aspect,	ONE_FIXED/21},
	{"neck",		centre_aspect,	ONE_FIXED/21},
	{"head",		centre_aspect,	ONE_FIXED/21},
	{"left thigh",	centre_aspect,	ONE_FIXED/21},
	{"left shin",	centre_aspect,	ONE_FIXED/21},
	{"left foot",	centre_aspect,	ONE_FIXED/21},
	{"right thigh",	centre_aspect,	ONE_FIXED/21},
	{"right shin",	centre_aspect,	ONE_FIXED/21},
	{"right foot",	centre_aspect,	ONE_FIXED/21},
	{"pipe_lt",		back_aspect,	ONE_FIXED/21},
	{"pipe_lb",		back_aspect,	ONE_FIXED/21},
	{"pipe_rt",		back_aspect,	ONE_FIXED/21},
	{"pipe_rb",		back_aspect,	ONE_FIXED/21},
	{"100 tail",	back_aspect,	ONE_FIXED/21},
	{"chest",		centre_aspect,	ONE_FIXED/21},
	{NULL,			centre_aspect,	0},
};

HITLOCATIONTABLEENTRY AlTL[] = {
	/* Top Locs */
	{"left bicep",	centre_aspect,	ONE_FIXED/7},
	{"right bicep",	centre_aspect,	ONE_FIXED/7},
	{"neck",		centre_aspect,	ONE_FIXED/7},
	{"head",		centre_aspect,	ONE_FIXED/7},
	{"pipe_lt",		back_aspect,	ONE_FIXED/7},
	{"pipe_rt",		back_aspect,	ONE_FIXED/7},
	{"chest",		centre_aspect,	ONE_FIXED/7},
	{NULL,			centre_aspect,	0},
};

HITLOCATIONTABLEENTRY AlBL[] = {
	/* Base Locs */
	{"left thigh",	centre_aspect,	ONE_FIXED/8},
	{"left shin",	centre_aspect,	ONE_FIXED/8},
	{"left foot",	centre_aspect,	ONE_FIXED/8},
	{"right thigh",	centre_aspect,	ONE_FIXED/8},
	{"right shin",	centre_aspect,	ONE_FIXED/8},
	{"right foot",	centre_aspect,	ONE_FIXED/8},
	{"100 tail",	back_aspect,	ONE_FIXED/8},
	{"abdom",		centre_aspect,	ONE_FIXED/8},
	{NULL,			centre_aspect,	0},
};

HITLOCATIONTABLEENTRY AlLL[] = {
	/* Left Locs */
	{"abdom",		centre_aspect,	ONE_FIXED/13},
	{"left bicep",	centre_aspect,	ONE_FIXED/13},
	{"left forearm",centre_aspect,	ONE_FIXED/13},
	{"left palm",	centre_aspect,	ONE_FIXED/13},
	{"neck",		centre_aspect,	ONE_FIXED/13},
	{"head",		centre_aspect,	ONE_FIXED/13},
	{"left thigh",	centre_aspect,	ONE_FIXED/13},
	{"left shin",	centre_aspect,	ONE_FIXED/13},
	{"left foot",	centre_aspect,	ONE_FIXED/13},
	{"pipe_lt",		back_aspect,	ONE_FIXED/13},
	{"pipe_lb",		back_aspect,	ONE_FIXED/13},
	{"100 tail",	back_aspect,	ONE_FIXED/13},
	{"chest",		centre_aspect,	ONE_FIXED/13},
	{NULL,			centre_aspect,	0},
};

HITLOCATIONTABLEENTRY AlRL[] = {
	/* Right Locs */
	{"abdom",		centre_aspect,	ONE_FIXED/13},
	{"right bicep",	centre_aspect,	ONE_FIXED/13},
	{"right forearm",centre_aspect,	ONE_FIXED/13},
	{"right palm",	centre_aspect,	ONE_FIXED/13},
	{"neck",		centre_aspect,	ONE_FIXED/13},
	{"head",		centre_aspect,	ONE_FIXED/13},
	{"right thigh",	centre_aspect,	ONE_FIXED/13},
	{"right shin",	centre_aspect,	ONE_FIXED/13},
	{"right foot",	centre_aspect,	ONE_FIXED/13},
	{"pipe_rt",		back_aspect,	ONE_FIXED/13},
	{"pipe_rb",		back_aspect,	ONE_FIXED/13},
	{"100 tail",	back_aspect,	ONE_FIXED/13},
	{"chest",		centre_aspect,	ONE_FIXED/13},
	{NULL,			centre_aspect,	0},
};

HITLOCATIONTABLEENTRY AlTRL[] = {
	/* Top Right Locs */
	{"right bicep",	centre_aspect,	ONE_FIXED/5},
	{"neck",		centre_aspect,	ONE_FIXED/5},
	{"head",		centre_aspect,	ONE_FIXED/5},
	{"pipe_rt",		back_aspect,	ONE_FIXED/5},
	{"chest",		centre_aspect,	ONE_FIXED/5},
	{NULL,			centre_aspect,	0},
};

HITLOCATIONTABLEENTRY AlTLL[] = {
	/* Top Left Locs */
	{"left bicep",	centre_aspect,	ONE_FIXED/5},
	{"neck",		centre_aspect,	ONE_FIXED/5},
	{"head",		centre_aspect,	ONE_FIXED/5},
	{"pipe_lt",		back_aspect,	ONE_FIXED/5},
	{"chest",		centre_aspect,	ONE_FIXED/5},
	{NULL,			centre_aspect,	0},
};

HITLOCATIONTABLEENTRY AlBLL[] = {
	/* Base Left Locs */
	{"left thigh",	centre_aspect,	ONE_FIXED/5},
	{"left shin",	centre_aspect,	ONE_FIXED/5},
	{"left foot",	centre_aspect,	ONE_FIXED/5},
	{"100 tail",	back_aspect,	ONE_FIXED/5},
	{"abdom",		centre_aspect,	ONE_FIXED/5},
	{NULL,			centre_aspect,	0},
};

HITLOCATIONTABLEENTRY AlBRL[] = {
	/* Base Right Locs */
	{"right thigh",	centre_aspect,	ONE_FIXED/5},
	{"right shin",	centre_aspect,	ONE_FIXED/5},
	{"right foot",	centre_aspect,	ONE_FIXED/5},
	{"100 tail",	back_aspect,	ONE_FIXED/5},
	{"abdom",		centre_aspect,	ONE_FIXED/5},
	{NULL,			centre_aspect,	0},
};

/* PRED-ALIENS */

HITLOCATIONTABLEENTRY PrdAlCL[] = {
	/* Centre Locs */
	{"arse",		centre_aspect,	ONE_FIXED/18},
	{"left bicep",	centre_aspect,	ONE_FIXED/18},
	{"left forearm",centre_aspect,	ONE_FIXED/18},
	{"left hand",	centre_aspect,	ONE_FIXED/18},
	{"right bicep",	centre_aspect,	ONE_FIXED/18},
	{"right forearm",centre_aspect,	ONE_FIXED/18},
	{"right hand",	centre_aspect,	ONE_FIXED/18},
	{"neck",		centre_aspect,	ONE_FIXED/18},
	{"head",		centre_aspect,	ONE_FIXED/18},
	{"left thigh",	centre_aspect,	ONE_FIXED/18},
	{"left shin",	centre_aspect,	ONE_FIXED/18},
	{"left foot",	centre_aspect,	ONE_FIXED/18},
	{"right thigh",	centre_aspect,	ONE_FIXED/18},
	{"right shin",	centre_aspect,	ONE_FIXED/18},
	{"right foot",	centre_aspect,	ONE_FIXED/18},
	{"tail base",	back_aspect,	ONE_FIXED/18},
	{"tummy",		centre_aspect,	ONE_FIXED/18},
	{"chest",		centre_aspect,	ONE_FIXED/18},
	{NULL,			centre_aspect,	0},		   
};

HITLOCATIONTABLEENTRY PrdAlTL[] = {
	/* Top Locs */
	{"left bicep",	centre_aspect,	ONE_FIXED/5},
	{"right bicep",	centre_aspect,	ONE_FIXED/5},
	{"neck",		centre_aspect,	ONE_FIXED/5},
	{"head",		centre_aspect,	ONE_FIXED/5},
	{"chest",		centre_aspect,	ONE_FIXED/5},
	{NULL,			centre_aspect,	0},
};

HITLOCATIONTABLEENTRY PrdAlBL[] = {
	/* Base Locs */
	{"left thigh",	centre_aspect,	ONE_FIXED/8},
	{"left shin",	centre_aspect,	ONE_FIXED/8},
	{"left foot",	centre_aspect,	ONE_FIXED/8},
	{"right thigh",	centre_aspect,	ONE_FIXED/8},
	{"right shin",	centre_aspect,	ONE_FIXED/8},
	{"right foot",	centre_aspect,	ONE_FIXED/8},
	{"tail base",	back_aspect,	ONE_FIXED/8},
	{"arse",		centre_aspect,	ONE_FIXED/8},
	{NULL,			centre_aspect,	0},
};

HITLOCATIONTABLEENTRY PrdAlLL[] = {
	/* Left Locs */
	{"arse",		centre_aspect,	ONE_FIXED/11},
	{"left bicep",	centre_aspect,	ONE_FIXED/11},
	{"left forearm",centre_aspect,	ONE_FIXED/11},
	{"left hand",	centre_aspect,	ONE_FIXED/11},
	{"neck",		centre_aspect,	ONE_FIXED/11},
	{"head",		centre_aspect,	ONE_FIXED/11},
	{"left thigh",	centre_aspect,	ONE_FIXED/11},
	{"left shin",	centre_aspect,	ONE_FIXED/11},
	{"left foot",	centre_aspect,	ONE_FIXED/11},
	{"tail base",	back_aspect,	ONE_FIXED/11},
	{"chest",		centre_aspect,	ONE_FIXED/11},
	{NULL,			centre_aspect,	0},
};

HITLOCATIONTABLEENTRY PrdAlRL[] = {
	/* Right Locs */
	{"arse",		centre_aspect,	ONE_FIXED/11},
	{"right bicep",	centre_aspect,	ONE_FIXED/11},
	{"right forearm",centre_aspect,	ONE_FIXED/11},
	{"right hand",	centre_aspect,	ONE_FIXED/11},
	{"neck",		centre_aspect,	ONE_FIXED/11},
	{"head",		centre_aspect,	ONE_FIXED/11},
	{"right thigh",	centre_aspect,	ONE_FIXED/11},
	{"right shin",	centre_aspect,	ONE_FIXED/11},
	{"right foot",	centre_aspect,	ONE_FIXED/11},
	{"tail base",	back_aspect,	ONE_FIXED/11},
	{"chest",		centre_aspect,	ONE_FIXED/11},
	{NULL,			centre_aspect,	0},
};

HITLOCATIONTABLEENTRY PrdAlTRL[] = {
	/* Top Right Locs */
	{"right bicep",	centre_aspect,	ONE_FIXED/4},
	{"neck",		centre_aspect,	ONE_FIXED/4},
	{"head",		centre_aspect,	ONE_FIXED/4},
	{"chest",		centre_aspect,	ONE_FIXED/4},
	{NULL,			centre_aspect,	0},
};

HITLOCATIONTABLEENTRY PrdAlTLL[] = {
	/* Top Left Locs */
	{"left bicep",	centre_aspect,	ONE_FIXED/4},
	{"neck",		centre_aspect,	ONE_FIXED/4},
	{"head",		centre_aspect,	ONE_FIXED/4},
	{"chest",		centre_aspect,	ONE_FIXED/4},
	{NULL,			centre_aspect,	0},
};

HITLOCATIONTABLEENTRY PrdAlBLL[] = {
	/* Base Left Locs */
	{"left thigh",	centre_aspect,	ONE_FIXED/5},
	{"left shin",	centre_aspect,	ONE_FIXED/5},
	{"left foot",	centre_aspect,	ONE_FIXED/5},
	{"tail base",	back_aspect,	ONE_FIXED/5},
	{"arse",		centre_aspect,	ONE_FIXED/5},
	{NULL,			centre_aspect,	0},
};

HITLOCATIONTABLEENTRY PrdAlBRL[] = {
	/* Base Right Locs */
	{"right thigh",	centre_aspect,	ONE_FIXED/5},
	{"right shin",	centre_aspect,	ONE_FIXED/5},
	{"right foot",	centre_aspect,	ONE_FIXED/5},
	{"tail base",	back_aspect,	ONE_FIXED/5},
	{"arse",		centre_aspect,	ONE_FIXED/5},
	{NULL,			centre_aspect,	0},
};

/* PRAETORIAN GUARD */

HITLOCATIONTABLEENTRY PraetCL[] = {
	/* Centre Locs */
	{"abdom",		centre_aspect,	ONE_FIXED/19},
	{"left bicep",	centre_aspect,	ONE_FIXED/19},
	{"left forearm",centre_aspect,	ONE_FIXED/19},
	{"left palm",	centre_aspect,	ONE_FIXED/19},
	{"right bicep",	centre_aspect,	ONE_FIXED/19},
	{"right forearm",centre_aspect,	ONE_FIXED/19},
	{"right palm",	centre_aspect,	ONE_FIXED/19},
	{"neck",		centre_aspect,	ONE_FIXED/19},
	{"head",		centre_aspect,	ONE_FIXED/19},
	{"left thigh",	centre_aspect,	ONE_FIXED/19},
	{"left shin",	centre_aspect,	ONE_FIXED/19},
	{"left ankle",	centre_aspect,	ONE_FIXED/19},
	{"left foot",	centre_aspect,	ONE_FIXED/19},
	{"right thigh",	centre_aspect,	ONE_FIXED/19},
	{"right shin",	centre_aspect,	ONE_FIXED/19},
	{"right ankle",	centre_aspect,	ONE_FIXED/19},
	{"right foot",	centre_aspect,	ONE_FIXED/19},
	{"100 tail",	back_aspect,	ONE_FIXED/19},
	{"chest",		centre_aspect,	ONE_FIXED/19},
	{NULL,			centre_aspect,	0},
};

HITLOCATIONTABLEENTRY PraetTL[] = {
	/* Top Locs */
	{"left bicep",	centre_aspect,	ONE_FIXED/5},
	{"right bicep",	centre_aspect,	ONE_FIXED/5},
	{"neck",		centre_aspect,	ONE_FIXED/5},
	{"head",		centre_aspect,	ONE_FIXED/5},
	{"chest",		centre_aspect,	ONE_FIXED/5},
	{NULL,			centre_aspect,	0},
};

HITLOCATIONTABLEENTRY PraetBL[] = {
	/* Base Locs */
	{"left thigh",	centre_aspect,	ONE_FIXED/10},
	{"left shin",	centre_aspect,	ONE_FIXED/10},
	{"left ankle",	centre_aspect,	ONE_FIXED/10},
	{"left foot",	centre_aspect,	ONE_FIXED/10},
	{"right thigh",	centre_aspect,	ONE_FIXED/10},
	{"right shin",	centre_aspect,	ONE_FIXED/10},
	{"right ankle",	centre_aspect,	ONE_FIXED/10},
	{"right foot",	centre_aspect,	ONE_FIXED/10},
	{"100 tail",	back_aspect,	ONE_FIXED/10},
	{"abdom",		centre_aspect,	ONE_FIXED/10},
	{NULL,			centre_aspect,	0},
};

HITLOCATIONTABLEENTRY PraetLL[] = {
	/* Left Locs */
	{"abdom",		centre_aspect,	ONE_FIXED/12},
	{"left bicep",	centre_aspect,	ONE_FIXED/12},
	{"left forearm",centre_aspect,	ONE_FIXED/12},
	{"left palm",	centre_aspect,	ONE_FIXED/12},
	{"neck",		centre_aspect,	ONE_FIXED/12},
	{"head",		centre_aspect,	ONE_FIXED/12},
	{"left thigh",	centre_aspect,	ONE_FIXED/12},
	{"left shin",	centre_aspect,	ONE_FIXED/12},
	{"left ankle",	centre_aspect,	ONE_FIXED/12},
	{"left foot",	centre_aspect,	ONE_FIXED/12},
	{"100 tail",	back_aspect,	ONE_FIXED/12},
	{"chest",		centre_aspect,	ONE_FIXED/12},
	{NULL,			centre_aspect,	0},
};

HITLOCATIONTABLEENTRY PraetRL[] = {
	/* Right Locs */
	{"abdom",		centre_aspect,	ONE_FIXED/12},
	{"right bicep",	centre_aspect,	ONE_FIXED/12},
	{"right forearm",centre_aspect,	ONE_FIXED/12},
	{"right palm",	centre_aspect,	ONE_FIXED/12},
	{"neck",		centre_aspect,	ONE_FIXED/12},
	{"head",		centre_aspect,	ONE_FIXED/12},
	{"right thigh",	centre_aspect,	ONE_FIXED/12},
	{"right shin",	centre_aspect,	ONE_FIXED/12},
	{"right ankle",	centre_aspect,	ONE_FIXED/12},
	{"right foot",	centre_aspect,	ONE_FIXED/12},
	{"100 tail",	back_aspect,	ONE_FIXED/12},
	{"chest",		centre_aspect,	ONE_FIXED/12},
	{NULL,			centre_aspect,	0},
};

HITLOCATIONTABLEENTRY PraetTRL[] = {
	/* Top Right Locs */
	{"right bicep",	centre_aspect,	ONE_FIXED/4},
	{"neck",		centre_aspect,	ONE_FIXED/4},
	{"head",		centre_aspect,	ONE_FIXED/4},
	{"chest",		centre_aspect,	ONE_FIXED/4},
	{NULL,			centre_aspect,	0},
};

HITLOCATIONTABLEENTRY PraetTLL[] = {
	/* Top Left Locs */
	{"left bicep",	centre_aspect,	ONE_FIXED/4},
	{"neck",		centre_aspect,	ONE_FIXED/4},
	{"head",		centre_aspect,	ONE_FIXED/4},
	{"chest",		centre_aspect,	ONE_FIXED/4},
	{NULL,			centre_aspect,	0},
};

HITLOCATIONTABLEENTRY PraetBLL[] = {
	/* Base Left Locs */
	{"left thigh",	centre_aspect,	ONE_FIXED/6},
	{"left shin",	centre_aspect,	ONE_FIXED/6},
	{"left ankle",	centre_aspect,	ONE_FIXED/6},
	{"left foot",	centre_aspect,	ONE_FIXED/6},
	{"100 tail",	back_aspect,	ONE_FIXED/6},
	{"abdom",		centre_aspect,	ONE_FIXED/6},
	{NULL,			centre_aspect,	0},
};

HITLOCATIONTABLEENTRY PraetBRL[] = {
	/* Base Right Locs */
	{"right thigh",	centre_aspect,	ONE_FIXED/6},
	{"right shin",	centre_aspect,	ONE_FIXED/6},
	{"right ankle",	centre_aspect,	ONE_FIXED/6},
	{"right foot",	centre_aspect,	ONE_FIXED/6},
	{"100 tail",	back_aspect,	ONE_FIXED/6},
	{"abdom",		centre_aspect,	ONE_FIXED/6},
	{NULL,			centre_aspect,	0},
};

/* PREDATORS */

/* Predator Centre Locations */
HITLOCATIONTABLEENTRY PCL[] = {
	/* Centre locs */
	{"Pelvis",		centre_aspect,	ONE_FIXED/20},
	{"L thi",		centre_aspect,	ONE_FIXED/20},
	{"L shin",		centre_aspect,	ONE_FIXED/20},
	{"L foot",		centre_aspect,	ONE_FIXED/20},
	{"R thi",		centre_aspect,	ONE_FIXED/20},
	{"R shin",		centre_aspect,	ONE_FIXED/20},
	{"R foot",		centre_aspect,	ONE_FIXED/20},
	{"chest",		centre_aspect,	ONE_FIXED/20},
	{"R shoulder",	centre_aspect,	ONE_FIXED/20},
	{"R bicep",		centre_aspect,	ONE_FIXED/20},
	{"R elbow",		centre_aspect,	ONE_FIXED/20},
	{"R forearm",	centre_aspect,	ONE_FIXED/20},
	{"R hand palm",	centre_aspect,	ONE_FIXED/20},
	{"L shoulder",	centre_aspect,	ONE_FIXED/20},
	{"L bicep",		centre_aspect,	ONE_FIXED/20},
	{"L elbow",		centre_aspect,	ONE_FIXED/20},
	{"L forearm",	centre_aspect,	ONE_FIXED/20},
	{"L hand palm",	centre_aspect,	ONE_FIXED/20},
	{"neck",		centre_aspect,	ONE_FIXED/20},
	{"head",		centre_aspect,	ONE_FIXED/20},
	{NULL,			centre_aspect,	0},
};

HITLOCATIONTABLEENTRY PTL[] = {
	/* Top locs */
	{"chest",		centre_aspect,	ONE_FIXED/13},
	{"R shoulder",	centre_aspect,	ONE_FIXED/13},
	{"R bicep",		centre_aspect,	ONE_FIXED/13},
	{"R elbow",		centre_aspect,	ONE_FIXED/13},
	{"R forearm",	centre_aspect,	ONE_FIXED/13},
	{"R hand palm",	centre_aspect,	ONE_FIXED/13},
	{"L shoulder",	centre_aspect,	ONE_FIXED/13},
	{"L bicep",		centre_aspect,	ONE_FIXED/13},
	{"L elbow",		centre_aspect,	ONE_FIXED/13},
	{"L forearm",	centre_aspect,	ONE_FIXED/13},
	{"L hand palm",	centre_aspect,	ONE_FIXED/13},
	{"neck",		centre_aspect,	ONE_FIXED/13},
	{"head",		centre_aspect,	ONE_FIXED/13},
	{NULL,			centre_aspect,	0},
};

HITLOCATIONTABLEENTRY PBL[] = {
	/* Base locs */
	{"Pelvis",		centre_aspect,	ONE_FIXED/7},
	{"L thi",		centre_aspect,	ONE_FIXED/7},
	{"L shin",		centre_aspect,	ONE_FIXED/7},
	{"L foot",		centre_aspect,	ONE_FIXED/7},
	{"R thi",		centre_aspect,	ONE_FIXED/7},
	{"R shin",		centre_aspect,	ONE_FIXED/7},
	{"R foot",		centre_aspect,	ONE_FIXED/7},
	{NULL,			centre_aspect,	0},
};

HITLOCATIONTABLEENTRY PLL[] = {
	/* Left locs */
	{"Pelvis",		centre_aspect,	ONE_FIXED/12},
	{"L thi",		centre_aspect,	ONE_FIXED/12},
	{"L shin",		centre_aspect,	ONE_FIXED/12},
	{"L foot",		centre_aspect,	ONE_FIXED/12},
	{"chest",		centre_aspect,	ONE_FIXED/12},
	{"L shoulder",	centre_aspect,	ONE_FIXED/12},
	{"L bicep",		centre_aspect,	ONE_FIXED/12},
	{"L elbow",		centre_aspect,	ONE_FIXED/12},
	{"L forearm",	centre_aspect,	ONE_FIXED/12},
	{"L hand palm",	centre_aspect,	ONE_FIXED/12},
	{"neck",		centre_aspect,	ONE_FIXED/12},
	{"head",		centre_aspect,	ONE_FIXED/12},
	{NULL,			centre_aspect,	0},
};

HITLOCATIONTABLEENTRY PRL[] = {
	/* Right locs */
	{"Pelvis",		centre_aspect,	ONE_FIXED/12},
	{"R thi",		centre_aspect,	ONE_FIXED/12},
	{"R shin",		centre_aspect,	ONE_FIXED/12},
	{"R foot",		centre_aspect,	ONE_FIXED/12},
	{"chest",		centre_aspect,	ONE_FIXED/12},
	{"R shoulder",	centre_aspect,	ONE_FIXED/12},
	{"R bicep",		centre_aspect,	ONE_FIXED/12},
	{"R elbow",		centre_aspect,	ONE_FIXED/12},
	{"R forearm",	centre_aspect,	ONE_FIXED/12},
	{"R hand palm",	centre_aspect,	ONE_FIXED/12},
	{"neck",		centre_aspect,	ONE_FIXED/12},
	{"head",		centre_aspect,	ONE_FIXED/12},
	{NULL,			centre_aspect,	0},
};

HITLOCATIONTABLEENTRY PTLL[] = {
	/* TopLeft locs */
	{"chest",		centre_aspect,	ONE_FIXED/8},
	{"L shoulder",	centre_aspect,	ONE_FIXED/8},
	{"L bicep",		centre_aspect,	ONE_FIXED/8},
	{"L elbow",		centre_aspect,	ONE_FIXED/8},
	{"L forearm",	centre_aspect,	ONE_FIXED/8},
	{"L hand palm",	centre_aspect,	ONE_FIXED/8},
	{"neck",		centre_aspect,	ONE_FIXED/8},
	{"head",		centre_aspect,	ONE_FIXED/8},
	{NULL,			centre_aspect,	0},
};

HITLOCATIONTABLEENTRY PTRL[] = {
	/* TopRight locs */
	{"chest",		centre_aspect,	ONE_FIXED/8},
	{"R shoulder",	centre_aspect,	ONE_FIXED/8},
	{"R bicep",		centre_aspect,	ONE_FIXED/8},
	{"R elbow",		centre_aspect,	ONE_FIXED/8},
	{"R forearm",	centre_aspect,	ONE_FIXED/8},
	{"R hand palm",	centre_aspect,	ONE_FIXED/8},
	{"neck",		centre_aspect,	ONE_FIXED/8},
	{"head",		centre_aspect,	ONE_FIXED/8},
	{NULL,			centre_aspect,	0},
};

HITLOCATIONTABLEENTRY PBLL[] = {
	/* BaseLeft locs */
	{"Pelvis",		centre_aspect,	ONE_FIXED/4},
	{"L thi",		centre_aspect,	ONE_FIXED/4},
	{"L shin",		centre_aspect,	ONE_FIXED/4},
	{"L foot",		centre_aspect,	ONE_FIXED/4},
	{NULL,			centre_aspect,	0},
};

HITLOCATIONTABLEENTRY PBRL[] = {
	/* BaseRight locs */
	{"Pelvis",		centre_aspect,	ONE_FIXED/4},
	{"R thi",		centre_aspect,	ONE_FIXED/4},
	{"R shin",		centre_aspect,	ONE_FIXED/4},
	{"R foot",		centre_aspect,	ONE_FIXED/4},
	{NULL,			centre_aspect,	0},
};

/* XENOBORGS */
HITLOCATIONTABLEENTRY XBCL[] = {
	/* Centre Locs */
	{"pelvis presley",	centre_aspect,	ONE_FIXED/26},
	{"left bicep",		centre_aspect,	ONE_FIXED/26},
	{"left forearm",	centre_aspect,	ONE_FIXED/26},
	{"left attacher",	front_aspect,	ONE_FIXED/26},
	{"lazer left",		front_aspect,	ONE_FIXED/26},
	{"barney",			front_aspect,	ONE_FIXED/26},
	{"right bicep",		centre_aspect,	ONE_FIXED/26},
	{"right forearm",	centre_aspect,	ONE_FIXED/26},
	{"right attacher",	front_aspect,	ONE_FIXED/26},
	{"pump",			front_aspect,	ONE_FIXED/26},
	{"ming",			front_aspect,	ONE_FIXED/26},
	{"neck",			centre_aspect,	ONE_FIXED/26},
	{"head",			centre_aspect,	ONE_FIXED/26},
	{"sights",			centre_aspect,	ONE_FIXED/26},
	{"left thigh",		centre_aspect,	ONE_FIXED/26},
	{"left shin",		centre_aspect,	ONE_FIXED/26},
	{"left foot",		centre_aspect,	ONE_FIXED/26},
	{"right thigh",		centre_aspect,	ONE_FIXED/26},
	{"right shin",		centre_aspect,	ONE_FIXED/26},
	{"right foot",		centre_aspect,	ONE_FIXED/26},
	{"back pac",		back_aspect,	ONE_FIXED/26},
	{"exhaust",			back_aspect,	ONE_FIXED/26},
	{"pipe",			back_aspect,	ONE_FIXED/26},
	{"tail a",			back_aspect,	ONE_FIXED/26},
	{"tummy",			centre_aspect,	ONE_FIXED/26},
	{"chest",			centre_aspect,	ONE_FIXED/26},
	{NULL,				centre_aspect,	0},
};

HITLOCATIONTABLEENTRY XBTL[] = {
	/* Top locs */
	{"left bicep",		centre_aspect,	ONE_FIXED/16},
	{"left forearm",	centre_aspect,	ONE_FIXED/16},
	{"left attacher",	front_aspect,	ONE_FIXED/16},
	{"lazer left",		front_aspect,	ONE_FIXED/16},
	{"barney",			front_aspect,	ONE_FIXED/16},
	{"right bicep",		centre_aspect,	ONE_FIXED/16},
	{"right forearm",	centre_aspect,	ONE_FIXED/16},
	{"right attacher",	front_aspect,	ONE_FIXED/16},
	{"pump",			front_aspect,	ONE_FIXED/16},
	{"ming",			front_aspect,	ONE_FIXED/16},
	{"neck",			centre_aspect,	ONE_FIXED/16},
	{"head",			centre_aspect,	ONE_FIXED/16},
	{"sights",			centre_aspect,	ONE_FIXED/16},
	{"exhaust",			back_aspect,	ONE_FIXED/16},
	{"pipe",			back_aspect,	ONE_FIXED/16},
	{"chest",			centre_aspect,	ONE_FIXED/16},
	{NULL,				centre_aspect,	0},
};

HITLOCATIONTABLEENTRY XBBL[] = {
	/* Base locs */
	{"pelvis presley",	centre_aspect,	ONE_FIXED/9},
	{"left thigh",		centre_aspect,	ONE_FIXED/9},
	{"left shin",		centre_aspect,	ONE_FIXED/9},
	{"left foot",		centre_aspect,	ONE_FIXED/9},
	{"right thigh",		centre_aspect,	ONE_FIXED/9},
	{"right shin",		centre_aspect,	ONE_FIXED/9},
	{"right foot",		centre_aspect,	ONE_FIXED/9},
	{"back pac",		back_aspect,	ONE_FIXED/9},
	{"tail a",			back_aspect,	ONE_FIXED/9},
	{NULL,				centre_aspect,	0},
};

HITLOCATIONTABLEENTRY XBLL[] = {
	/* Left locs */
	{"pelvis presley",	centre_aspect,	ONE_FIXED/18},
	{"left bicep",		centre_aspect,	ONE_FIXED/18},
	{"left forearm",	centre_aspect,	ONE_FIXED/18},
	{"left attacher",	front_aspect,	ONE_FIXED/18},
	{"lazer left",		front_aspect,	ONE_FIXED/18},
	{"barney",			front_aspect,	ONE_FIXED/18},
	{"neck",			centre_aspect,	ONE_FIXED/18},
	{"head",			centre_aspect,	ONE_FIXED/18},
	{"sights",			centre_aspect,	ONE_FIXED/18},
	{"left thigh",		centre_aspect,	ONE_FIXED/18},
	{"left shin",		centre_aspect,	ONE_FIXED/18},
	{"left foot",		centre_aspect,	ONE_FIXED/18},
	{"back pac",		back_aspect,	ONE_FIXED/18},
	{"exhaust",			back_aspect,	ONE_FIXED/18},
	{"pipe",			back_aspect,	ONE_FIXED/18},
	{"tail a",			back_aspect,	ONE_FIXED/18},
	{"tummy",			centre_aspect,	ONE_FIXED/18},
	{"chest",			centre_aspect,	ONE_FIXED/18},
	{NULL,				centre_aspect,	0},
};

HITLOCATIONTABLEENTRY XBRL[] = {
	/* Right locs */
	{"pelvis presley",	centre_aspect,	ONE_FIXED/17},
	{"right bicep",		centre_aspect,	ONE_FIXED/17},
	{"right forearm",	centre_aspect,	ONE_FIXED/17},
	{"right attacher",	front_aspect,	ONE_FIXED/17},
	{"pump",			front_aspect,	ONE_FIXED/17},
	{"ming",			front_aspect,	ONE_FIXED/17},
	{"neck",			centre_aspect,	ONE_FIXED/17},
	{"head",			centre_aspect,	ONE_FIXED/17},
	{"right thigh",		centre_aspect,	ONE_FIXED/17},
	{"right shin",		centre_aspect,	ONE_FIXED/17},
	{"right foot",		centre_aspect,	ONE_FIXED/17},
	{"back pac",		back_aspect,	ONE_FIXED/17},
	{"exhaust",			back_aspect,	ONE_FIXED/17},
	{"pipe",			back_aspect,	ONE_FIXED/17},
	{"tail a",			back_aspect,	ONE_FIXED/17},
	{"tummy",			centre_aspect,	ONE_FIXED/17},
	{"chest",			centre_aspect,	ONE_FIXED/17},
	{NULL,				centre_aspect,	0},
};

HITLOCATIONTABLEENTRY XBTLL[] = {
	/* TopLeft locs */
	{"left bicep",		centre_aspect,	ONE_FIXED/11},
	{"left forearm",	centre_aspect,	ONE_FIXED/11},
	{"left attacher",	front_aspect,	ONE_FIXED/11},
	{"lazer left",		front_aspect,	ONE_FIXED/11},
	{"barney",			front_aspect,	ONE_FIXED/11},
	{"neck",			centre_aspect,	ONE_FIXED/11},
	{"head",			centre_aspect,	ONE_FIXED/11},
	{"sights",			centre_aspect,	ONE_FIXED/11},
	{"exhaust",			back_aspect,	ONE_FIXED/11},
	{"pipe",			back_aspect,	ONE_FIXED/11},
	{"chest",			centre_aspect,	ONE_FIXED/11},
	{NULL,				centre_aspect,	0},
};

HITLOCATIONTABLEENTRY XBTRL[] = {
	/* TopRight locs */
	{"right bicep",		centre_aspect,	ONE_FIXED/10},
	{"right forearm",	centre_aspect,	ONE_FIXED/10},
	{"right attacher",	front_aspect,	ONE_FIXED/10},
	{"pump",			front_aspect,	ONE_FIXED/10},
	{"ming",			front_aspect,	ONE_FIXED/10},
	{"neck",			centre_aspect,	ONE_FIXED/10},
	{"head",			centre_aspect,	ONE_FIXED/10},
	{"exhaust",			back_aspect,	ONE_FIXED/10},
	{"pipe",			back_aspect,	ONE_FIXED/10},
	{"chest",			centre_aspect,	ONE_FIXED/10},
	{NULL,				centre_aspect,	0},
};

HITLOCATIONTABLEENTRY XBBLL[] = {
	/* BaseLeft locs */
	{"pelvis presley",	centre_aspect,	ONE_FIXED/6},
	{"left thigh",		centre_aspect,	ONE_FIXED/6},
	{"left shin",		centre_aspect,	ONE_FIXED/6},
	{"left foot",		centre_aspect,	ONE_FIXED/6},
	{"back pac",		back_aspect,	ONE_FIXED/6},
	{"tail a",			back_aspect,	ONE_FIXED/6},
	{NULL,				centre_aspect,	0},
};

HITLOCATIONTABLEENTRY XBBRL[] = {
	/* BaseRight locs */
	{"pelvis presley",	centre_aspect,	ONE_FIXED/6},
	{"right thigh",		centre_aspect,	ONE_FIXED/6},
	{"right shin",		centre_aspect,	ONE_FIXED/6},
	{"right foot",		centre_aspect,	ONE_FIXED/6},
	{"back pac",		back_aspect,	ONE_FIXED/6},
	{"tail a",			back_aspect,	ONE_FIXED/6},
	{NULL,				centre_aspect,	0},
};

/* Sentry Guns */
HITLOCATIONTABLEENTRY SGGL[] = {
	/* General locs */
	{"legs",	centre_aspect,	ONE_FIXED/3},
	{"pivot",	centre_aspect,	ONE_FIXED/3},
	{"gun",		centre_aspect,	ONE_FIXED/3},
	{NULL,		centre_aspect,	0},
};

HITLOCATIONTABLE Global_Hitlocation_Tables[] = {
	{
		/* MARINE */
		"marine with pulse rifle",
		0,   /*index*/
		&MWPRCL[0],
		&MWPRTL[0],
		&MWPRBL[0],
		&MWPRLL[0],
		&MWPRRL[0],
		&MWPRTLL[0],
		&MWPRTRL[0],
		&MWPRBLL[0],
		&MWPRBRL[0],
	},
	{
		/* MARINE */
		"marine with smart gun",
		1,   /*index*/
		&MWPRCL[0],
		&MWPRTL[0],
		&MWPRBL[0],
		&MWPRLL[0],
		&MWPRRL[0],
		&MWPRTLL[0],
		&MWPRTRL[0],
		&MWPRBLL[0],
		&MWPRBRL[0],
	},
	{
		/* MARINE */
		"marine with flame thrower",
		2,   /*index*/
		&MWPRCL[0],
		&MWPRTL[0],
		&MWPRBL[0],
		&MWPRLL[0],
		&MWPRRL[0],
		&MWPRTLL[0],
		&MWPRTRL[0],
		&MWPRBLL[0],
		&MWPRBRL[0],
	},
	{
		/* CIVILIAN */
		"male civvie",
		3,   /*index*/
		&MCCL[0],
		&MCTL[0],
		&MCBL[0],
		&MCLL[0],
		&MCRL[0],
		&MCTLL[0],
		&MCTRL[0],
		&MCBLL[0],
		&MCBRL[0],
	},
	{
		/* ALIEN */
		"alien",
		4,   /*index*/
		&AlCL[0],
		&AlTL[0],
		&AlBL[0],
		&AlLL[0],
		&AlRL[0],
		&AlTLL[0],
		&AlTRL[0],
		&AlBLL[0],
		&AlBRL[0],
	},
	{
		/* PRED-ALIEN */
		"predalien",
		5,   /*index*/
		&PrdAlCL[0],
		&PrdAlTL[0],
		&PrdAlBL[0],
		&PrdAlLL[0],
		&PrdAlRL[0],
		&PrdAlTLL[0],
		&PrdAlTRL[0],
		&PrdAlBLL[0],
		&PrdAlBRL[0],
	},
	{
		/* PRAETORIAN */
		"praetorian",
		6,   /*index*/
		&PraetCL[0],
		&PraetTL[0],
		&PraetBL[0],
		&PraetLL[0],
		&PraetRL[0],
		&PraetTLL[0],
		&PraetTRL[0],
		&PraetBLL[0],
		&PraetBRL[0],
	},
	{
		/* PREDATOR */
		"predator",
		7,   /*index*/
		&PCL[0],
		&PTL[0],
		&PBL[0],
		&PLL[0],
		&PRL[0],
		&PTLL[0],
		&PTRL[0],
		&PBLL[0],
		&PBRL[0],
	},
	{
		/* XENOBORG */
		"xenoborg",
		8,   /*index*/
		&XBCL[0],
		&XBTL[0],
		&XBBL[0],
		&XBLL[0],
		&XBRL[0],
		&XBTLL[0],
		&XBTRL[0],
		&XBBLL[0],
		&XBBRL[0],
	},
	{
		/* SENTRY GUN */
		"sentrygun",
		9,   /*index*/
		&SGGL[0],
		&SGGL[0],
		&SGGL[0],
		&SGGL[0],
		&SGGL[0],
		&SGGL[0],
		&SGGL[0],
		&SGGL[0],
		&SGGL[0],
	},
	{
		/* Terminator */
		NULL,
		-1,   /*index*/
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
	},
};

SECTION_ATTACHMENT Default_Stats = {
	NULL,
	NULL,
	NULL,
	{
		10000,	/* Health */
		5000,		/* Armour */
		0, /* IsOnFire */
		{
			1,	/* Acid Resistant */
			1,	/* Fire Resistant */
			1,	/* Electric Resistant */
			1,	/* Perfect Armour */
			0,	/* Electric Sensitive */
			0,	/* Combustability */
			0,	/* Indestructable */
		},
	},
	section_flag_doesnthurtsb|section_flag_passdamagetoparent,
};

char *Android_Hierarchy_Names[] = {
	"Android shotgun",
	"Android Shotgun Special",
	"Android Pistol Special",
	"Android template",
	NULL,
};

SECTION_ATTACHMENT Global_Section_Attachments[] = {
	/* FACEHUGGERS! */
	{
		"hnpchugger",
		"body",
		NULL,
		{
			5,	/* Health */
			0,		/* Armour */
			0, /* IsOnFire */
			{
				1,	/* Acid Resistant */
				0,	/* Fire Resistant */
				0,	/* Electric Resistant */
				0,	/* Perfect Armour */
				0,	/* Electric Sensitive */
				0,	/* Combustability */
				0,	/* Indestructable */
			},
		},
		section_has_sparkoflife|section_sprays_acid,
	},
	{
		"hnpchugger",
		"right air sack",
		NULL,
		{
			5,	/* Health */
			0,		/* Armour */
			0, /* IsOnFire */
			{
				1,	/* Acid Resistant */
				0,	/* Fire Resistant */
				0,	/* Electric Resistant */
				0,	/* Perfect Armour */
				0,	/* Electric Sensitive */
				0,	/* Combustability */
				0,	/* Indestructable */
			},
		},
		section_has_sparkoflife|section_sprays_acid,
	},
	{
		"hnpchugger",
		"left air sack",
		NULL,
		{
			5,	/* Health */
			0,		/* Armour */
			0, /* IsOnFire */
			{
				1,	/* Acid Resistant */
				0,	/* Fire Resistant */
				0,	/* Electric Resistant */
				0,	/* Perfect Armour */
				0,	/* Electric Sensitive */
				0,	/* Combustability */
				0,	/* Indestructable */
			},
		},
		section_has_sparkoflife|section_sprays_acid,
	},
	{
		"hnpchugger",
		"l00 tail",
		NULL,
		{
			4,	/* Health */
			0,		/* Armour */
			0, /* IsOnFire */
			{
				1,	/* Acid Resistant */
				0,	/* Fire Resistant */
				0,	/* Electric Resistant */
				0,	/* Perfect Armour */
				0,	/* Electric Sensitive */
				0,	/* Combustability */
				0,	/* Indestructable */
			},
		},
		section_has_sparkoflife|section_sprays_acid,
	},
	{
		"hnpchugger",
		"l01 tail",
		NULL,
		{
			4,	/* Health */
			0,		/* Armour */
			0, /* IsOnFire */
			{
				1,	/* Acid Resistant */
				0,	/* Fire Resistant */
				0,	/* Electric Resistant */
				0,	/* Perfect Armour */
				0,	/* Electric Sensitive */
				0,	/* Combustability */
				0,	/* Indestructable */
			},
		},
		section_has_sparkoflife|section_sprays_acid,
	},
	{
		"hnpchugger",
		"l02 tail",
		NULL,
		{
			4,	/* Health */
			0,		/* Armour */
			0, /* IsOnFire */
			{
				1,	/* Acid Resistant */
				0,	/* Fire Resistant */
				0,	/* Electric Resistant */
				0,	/* Perfect Armour */
				0,	/* Electric Sensitive */
				0,	/* Combustability */
				0,	/* Indestructable */
			},
		},
		section_has_sparkoflife|section_sprays_acid,
	},
	{
		"hnpchugger",
		"l03 tail",
		NULL,
		{
			4,	/* Health */
			0,		/* Armour */
			0, /* IsOnFire */
			{
				1,	/* Acid Resistant */
				0,	/* Fire Resistant */
				0,	/* Electric Resistant */
				0,	/* Perfect Armour */
				0,	/* Electric Sensitive */
				0,	/* Combustability */
				0,	/* Indestructable */
			},
		},
		section_has_sparkoflife|section_sprays_acid,
	},
	{
		"hnpchugger",
		"l04 tail",
		NULL,
		{
			4,	/* Health */
			0,		/* Armour */
			0, /* IsOnFire */
			{
				1,	/* Acid Resistant */
				0,	/* Fire Resistant */
				0,	/* Electric Resistant */
				0,	/* Perfect Armour */
				0,	/* Electric Sensitive */
				0,	/* Combustability */
				0,	/* Indestructable */
			},
		},
		section_has_sparkoflife|section_sprays_acid,
	},
	{
		"hnpchugger",
		"l05 tail",
		NULL,
		{
			4,	/* Health */
			0,		/* Armour */
			0, /* IsOnFire */
			{
				1,	/* Acid Resistant */
				0,	/* Fire Resistant */
				0,	/* Electric Resistant */
				0,	/* Perfect Armour */
				0,	/* Electric Sensitive */
				0,	/* Combustability */
				0,	/* Indestructable */
			},
		},
		section_has_sparkoflife|section_sprays_acid,
	},
	{
		"hnpchugger",
		"l06 tail",
		NULL,
		{
			4,	/* Health */
			0,		/* Armour */
			0, /* IsOnFire */
			{
				1,	/* Acid Resistant */
				0,	/* Fire Resistant */
				0,	/* Electric Resistant */
				0,	/* Perfect Armour */
				0,	/* Electric Sensitive */
				0,	/* Combustability */
				0,	/* Indestructable */
			},
		},
		section_has_sparkoflife|section_sprays_acid,
	},
	{
		"hnpchugger",
		"l07 tail",
		NULL,
		{
			4,	/* Health */
			0,		/* Armour */
			0, /* IsOnFire */
			{
				1,	/* Acid Resistant */
				0,	/* Fire Resistant */
				0,	/* Electric Resistant */
				0,	/* Perfect Armour */
				0,	/* Electric Sensitive */
				0,	/* Combustability */
				0,	/* Indestructable */
			},
		},
		section_has_sparkoflife|section_sprays_acid,
	},
	{
		"hnpchugger",
		"l08 tail",
		NULL,
		{
			4,	/* Health */
			0,		/* Armour */
			0, /* IsOnFire */
			{
				1,	/* Acid Resistant */
				0,	/* Fire Resistant */
				0,	/* Electric Resistant */
				0,	/* Perfect Armour */
				0,	/* Electric Sensitive */
				0,	/* Combustability */
				0,	/* Indestructable */
			},
		},
		section_has_sparkoflife|section_sprays_acid,
	},
	{
		"hnpchugger",
		"l09 tail",
		NULL,
		{
			4,	/* Health */
			0,		/* Armour */
			0, /* IsOnFire */
			{
				1,	/* Acid Resistant */
				0,	/* Fire Resistant */
				0,	/* Electric Resistant */
				0,	/* Perfect Armour */
				0,	/* Electric Sensitive */
				0,	/* Combustability */
				0,	/* Indestructable */
			},
		},
		section_has_sparkoflife|section_sprays_acid,
	},
	{
		"hnpchugger",
		"l10 tail",
		NULL,
		{
			4,	/* Health */
			0,		/* Armour */
			0, /* IsOnFire */
			{
				1,	/* Acid Resistant */
				0,	/* Fire Resistant */
				0,	/* Electric Resistant */
				0,	/* Perfect Armour */
				0,	/* Electric Sensitive */
				0,	/* Combustability */
				0,	/* Indestructable */
			},
		},
		section_has_sparkoflife|section_sprays_acid,
	},
	{
		"hnpchugger",
		"l11 tail",
		NULL,
		{
			4,	/* Health */
			0,		/* Armour */
			0, /* IsOnFire */
			{
				1,	/* Acid Resistant */
				0,	/* Fire Resistant */
				0,	/* Electric Resistant */
				0,	/* Perfect Armour */
				0,	/* Electric Sensitive */
				0,	/* Combustability */
				0,	/* Indestructable */
			},
		},
		section_has_sparkoflife|section_sprays_acid,
	},
	{
		"hnpchugger",
		"left a1 finger",
		NULL,
		{
			4,	/* Health */
			0,		/* Armour */
			0, /* IsOnFire */
			{
				1,	/* Acid Resistant */
				0,	/* Fire Resistant */
				0,	/* Electric Resistant */
				0,	/* Perfect Armour */
				0,	/* Electric Sensitive */
				0,	/* Combustability */
				0,	/* Indestructable */
			},
		},
		section_has_sparkoflife|section_sprays_acid,
	},
	{
		"hnpchugger",
		"left a2 finger",
		NULL,
		{
			4,	/* Health */
			0,		/* Armour */
			0, /* IsOnFire */
			{
				1,	/* Acid Resistant */
				0,	/* Fire Resistant */
				0,	/* Electric Resistant */
				0,	/* Perfect Armour */
				0,	/* Electric Sensitive */
				0,	/* Combustability */
				0,	/* Indestructable */
			},
		},
		section_has_sparkoflife|section_sprays_acid,
	},
	{
		"hnpchugger",
		"left a3 finger",
		NULL,
		{
			4,	/* Health */
			0,		/* Armour */
			0, /* IsOnFire */
			{
				1,	/* Acid Resistant */
				0,	/* Fire Resistant */
				0,	/* Electric Resistant */
				0,	/* Perfect Armour */
				0,	/* Electric Sensitive */
				0,	/* Combustability */
				0,	/* Indestructable */
			},
		},
		section_has_sparkoflife|section_sprays_acid,
	},
	{
		"hnpchugger",
		"left b1 finger",
		NULL,
		{
			4,	/* Health */
			0,		/* Armour */
			0, /* IsOnFire */
			{
				1,	/* Acid Resistant */
				0,	/* Fire Resistant */
				0,	/* Electric Resistant */
				0,	/* Perfect Armour */
				0,	/* Electric Sensitive */
				0,	/* Combustability */
				0,	/* Indestructable */
			},
		},
		section_has_sparkoflife|section_sprays_acid,
	},
	{
		"hnpchugger",
		"left b2 finger",
		NULL,
		{
			4,	/* Health */
			0,		/* Armour */
			0, /* IsOnFire */
			{
				1,	/* Acid Resistant */
				0,	/* Fire Resistant */
				0,	/* Electric Resistant */
				0,	/* Perfect Armour */
				0,	/* Electric Sensitive */
				0,	/* Combustability */
				0,	/* Indestructable */
			},
		},
		section_has_sparkoflife|section_sprays_acid,
	},
	{
		"hnpchugger",
		"left b3 finger",
		NULL,
		{
			4,	/* Health */
			0,		/* Armour */
			0, /* IsOnFire */
			{
				1,	/* Acid Resistant */
				0,	/* Fire Resistant */
				0,	/* Electric Resistant */
				0,	/* Perfect Armour */
				0,	/* Electric Sensitive */
				0,	/* Combustability */
				0,	/* Indestructable */
			},
		},
		section_has_sparkoflife|section_sprays_acid,
	},
	{
		"hnpchugger",
		"left c1 finger",
		NULL,
		{
			4,	/* Health */
			0,		/* Armour */
			0, /* IsOnFire */
			{
				1,	/* Acid Resistant */
				0,	/* Fire Resistant */
				0,	/* Electric Resistant */
				0,	/* Perfect Armour */
				0,	/* Electric Sensitive */
				0,	/* Combustability */
				0,	/* Indestructable */
			},
		},
		section_has_sparkoflife|section_sprays_acid,
	},
	{
		"hnpchugger",
		"left c2 finger",
		NULL,
		{
			4,	/* Health */
			0,		/* Armour */
			0, /* IsOnFire */
			{
				1,	/* Acid Resistant */
				0,	/* Fire Resistant */
				0,	/* Electric Resistant */
				0,	/* Perfect Armour */
				0,	/* Electric Sensitive */
				0,	/* Combustability */
				0,	/* Indestructable */
			},
		},
		section_has_sparkoflife|section_sprays_acid,
	},
	{
		"hnpchugger",
		"left c3 finger",
		NULL,
		{
			4,	/* Health */
			0,		/* Armour */
			0, /* IsOnFire */
			{
				1,	/* Acid Resistant */
				0,	/* Fire Resistant */
				0,	/* Electric Resistant */
				0,	/* Perfect Armour */
				0,	/* Electric Sensitive */
				0,	/* Combustability */
				0,	/* Indestructable */
			},
		},
		section_has_sparkoflife|section_sprays_acid,
	},
	{
		"hnpchugger",
		"left d1 finger",
		NULL,
		{
			4,	/* Health */
			0,		/* Armour */
			0, /* IsOnFire */
			{
				1,	/* Acid Resistant */
				0,	/* Fire Resistant */
				0,	/* Electric Resistant */
				0,	/* Perfect Armour */
				0,	/* Electric Sensitive */
				0,	/* Combustability */
				0,	/* Indestructable */
			},
		},
		section_has_sparkoflife|section_sprays_acid,
	},
	{
		"hnpchugger",
		"left d2 finger",
		NULL,
		{
			4,	/* Health */
			0,		/* Armour */
			0, /* IsOnFire */
			{
				1,	/* Acid Resistant */
				0,	/* Fire Resistant */
				0,	/* Electric Resistant */
				0,	/* Perfect Armour */
				0,	/* Electric Sensitive */
				0,	/* Combustability */
				0,	/* Indestructable */
			},
		},
		section_has_sparkoflife|section_sprays_acid,
	},
	{
		"hnpchugger",
		"left d3 finger",
		NULL,
		{
			4,	/* Health */
			0,		/* Armour */
			0, /* IsOnFire */
			{
				1,	/* Acid Resistant */
				0,	/* Fire Resistant */
				0,	/* Electric Resistant */
				0,	/* Perfect Armour */
				0,	/* Electric Sensitive */
				0,	/* Combustability */
				0,	/* Indestructable */
			},
		},
		section_has_sparkoflife|section_sprays_acid,
	},
	{
		"hnpchugger",
		"right a1 finger",
		NULL,
		{
			4,	/* Health */
			0,		/* Armour */
			0, /* IsOnFire */
			{
				1,	/* Acid Resistant */
				0,	/* Fire Resistant */
				0,	/* Electric Resistant */
				0,	/* Perfect Armour */
				0,	/* Electric Sensitive */
				0,	/* Combustability */
				0,	/* Indestructable */
			},
		},
		section_has_sparkoflife|section_sprays_acid,
	},
	{
		"hnpchugger",
		"right a2 finger",
		NULL,
		{
			4,	/* Health */
			0,		/* Armour */
			0, /* IsOnFire */
			{
				1,	/* Acid Resistant */
				0,	/* Fire Resistant */
				0,	/* Electric Resistant */
				0,	/* Perfect Armour */
				0,	/* Electric Sensitive */
				0,	/* Combustability */
				0,	/* Indestructable */
			},
		},
		section_has_sparkoflife|section_sprays_acid,
	},
	{
		"hnpchugger",
		"right a3 finger",
		NULL,
		{
			4,	/* Health */
			0,		/* Armour */
			0, /* IsOnFire */
			{
				1,	/* Acid Resistant */
				0,	/* Fire Resistant */
				0,	/* Electric Resistant */
				0,	/* Perfect Armour */
				0,	/* Electric Sensitive */
				0,	/* Combustability */
				0,	/* Indestructable */
			},
		},
		section_has_sparkoflife|section_sprays_acid,
	},
	{
		"hnpchugger",
		"right b1 finger",
		NULL,
		{
			4,	/* Health */
			0,		/* Armour */
			0, /* IsOnFire */
			{
				1,	/* Acid Resistant */
				0,	/* Fire Resistant */
				0,	/* Electric Resistant */
				0,	/* Perfect Armour */
				0,	/* Electric Sensitive */
				0,	/* Combustability */
				0,	/* Indestructable */
			},
		},
		section_has_sparkoflife|section_sprays_acid,
	},
	{
		"hnpchugger",
		"right b2 finger",
		NULL,
		{
			4,	/* Health */
			0,		/* Armour */
			0, /* IsOnFire */
			{
				1,	/* Acid Resistant */
				0,	/* Fire Resistant */
				0,	/* Electric Resistant */
				0,	/* Perfect Armour */
				0,	/* Electric Sensitive */
				0,	/* Combustability */
				0,	/* Indestructable */
			},
		},
		section_has_sparkoflife|section_sprays_acid,
	},
	{
		"hnpchugger",
		"right b3 finger",
		NULL,
		{
			4,	/* Health */
			0,		/* Armour */
			0, /* IsOnFire */
			{
				1,	/* Acid Resistant */
				0,	/* Fire Resistant */
				0,	/* Electric Resistant */
				0,	/* Perfect Armour */
				0,	/* Electric Sensitive */
				0,	/* Combustability */
				0,	/* Indestructable */
			},
		},
		section_has_sparkoflife|section_sprays_acid,
	},
	{
		"hnpchugger",
		"right c1 finger",
		NULL,
		{
			4,	/* Health */
			0,		/* Armour */
			0, /* IsOnFire */
			{
				1,	/* Acid Resistant */
				0,	/* Fire Resistant */
				0,	/* Electric Resistant */
				0,	/* Perfect Armour */
				0,	/* Electric Sensitive */
				0,	/* Combustability */
				0,	/* Indestructable */
			},
		},
		section_has_sparkoflife|section_sprays_acid,
	},
	{
		"hnpchugger",
		"right c2 finger",
		NULL,
		{
			4,	/* Health */
			0,		/* Armour */
			0, /* IsOnFire */
			{
				1,	/* Acid Resistant */
				0,	/* Fire Resistant */
				0,	/* Electric Resistant */
				0,	/* Perfect Armour */
				0,	/* Electric Sensitive */
				0,	/* Combustability */
				0,	/* Indestructable */
			},
		},
		section_has_sparkoflife|section_sprays_acid,
	},
	{
		"hnpchugger",
		"right c3 finger",
		NULL,
		{
			4,	/* Health */
			0,		/* Armour */
			0, /* IsOnFire */
			{
				1,	/* Acid Resistant */
				0,	/* Fire Resistant */
				0,	/* Electric Resistant */
				0,	/* Perfect Armour */
				0,	/* Electric Sensitive */
				0,	/* Combustability */
				0,	/* Indestructable */
			},
		},
		section_has_sparkoflife|section_sprays_acid,
	},
	{
		"hnpchugger",
		"right d1 finger",
		NULL,
		{
			4,	/* Health */
			0,		/* Armour */
			0, /* IsOnFire */
			{
				1,	/* Acid Resistant */
				0,	/* Fire Resistant */
				0,	/* Electric Resistant */
				0,	/* Perfect Armour */
				0,	/* Electric Sensitive */
				0,	/* Combustability */
				0,	/* Indestructable */
			},
		},
		section_has_sparkoflife|section_sprays_acid,
	},
	{
		"hnpchugger",
		"right d2 finger",
		NULL,
		{
			4,	/* Health */
			0,		/* Armour */
			0, /* IsOnFire */
			{
				1,	/* Acid Resistant */
				0,	/* Fire Resistant */
				0,	/* Electric Resistant */
				0,	/* Perfect Armour */
				0,	/* Electric Sensitive */
				0,	/* Combustability */
				0,	/* Indestructable */
			},
		},
		section_has_sparkoflife|section_sprays_acid,
	},
	{
		"hnpchugger",
		"right d3 finger",
		NULL,
		{
			4,	/* Health */
			0,		/* Armour */
			0, /* IsOnFire */
			{
				1,	/* Acid Resistant */
				0,	/* Fire Resistant */
				0,	/* Electric Resistant */
				0,	/* Perfect Armour */
				0,	/* Electric Sensitive */
				0,	/* Combustability */
				0,	/* Indestructable */
			},
		},
		section_has_sparkoflife|section_sprays_acid,
	},
	/* ALIENS */
	{
		"HNPCalien",
		"head",
		NULL,
		{
			10,	/* Health */
			5,		/* Armour */
			0, /* IsOnFire */
			{
				0,	/* Acid Resistant */
				0,	/* Fire Resistant */
				0,	/* Electric Resistant */
				1,	/* Perfect Armour */
				0,	/* Electric Sensitive */
				0,	/* Combustability */
				0,	/* Indestructable */
			},
		},
		section_flag_head|section_has_sparkoflife|section_sprays_acid,
	},
	{
		"HNPCalien",
		"chest",
		NULL,
		{
			20,	/* Health */
			5,		/* Armour */
			0, /* IsOnFire */
			{
				0,	/* Acid Resistant */
				0,	/* Fire Resistant */
				0,	/* Electric Resistant */
				1,	/* Perfect Armour */
				0,	/* Electric Sensitive */
				0,	/* Combustability */
				0,	/* Indestructable */
			},
		},
		section_has_sparkoflife|section_sprays_acid|section_flag_gibbwhenfragged,
	},
	{
		"HNPCalien",
		"abdom",
		NULL,
		{
			20,	/* Health */
			5,		/* Armour */
			0, /* IsOnFire */
			{
				0,	/* Acid Resistant */
				0,	/* Fire Resistant */
				0,	/* Electric Resistant */
				1,	/* Perfect Armour */
				0,	/* Electric Sensitive */
				0,	/* Combustability */
				0,	/* Indestructable */
			},
		},
		section_has_sparkoflife|section_sprays_acid,
	},
	{
		"HNPCalien",
		"left thigh",
		NULL,
		{
			20,	/* Health */
			5,		/* Armour */
			0, /* IsOnFire */
			{
				0,	/* Acid Resistant */
				0,	/* Fire Resistant */
				0,	/* Electric Resistant */
				1,	/* Perfect Armour */
				0,	/* Electric Sensitive */
				0,	/* Combustability */
				0,	/* Indestructable */
			},
		},
		section_sprays_acid|section_flag_left_leg,
	},
	{
		"HNPCalien",
		"left shin",
		NULL,
		{
			15,	/* Health */
			5,		/* Armour */
			0, /* IsOnFire */
			{
				0,	/* Acid Resistant */
				0,	/* Fire Resistant */
				0,	/* Electric Resistant */
				1,	/* Perfect Armour */
				0,	/* Electric Sensitive */
				0,	/* Combustability */
				0,	/* Indestructable */
			},
		},
		section_sprays_acid|section_flag_left_leg,
	},
	{
		"HNPCalien",
		"left foot",
		NULL,
		{
			5,	/* Health */
			5,		/* Armour */
			0, /* IsOnFire */
			{
				0,	/* Acid Resistant */
				0,	/* Fire Resistant */
				0,	/* Electric Resistant */
				1,	/* Perfect Armour */
				0,	/* Electric Sensitive */
				0,	/* Combustability */
				0,	/* Indestructable */
			},
		},
		section_sprays_acid|section_flag_left_foot,
	},
	{
		"HNPCalien",
		"right thigh",
		NULL,
		{
			20,	/* Health */
			5,		/* Armour */
			0, /* IsOnFire */
			{
				0,	/* Acid Resistant */
				0,	/* Fire Resistant */
				0,	/* Electric Resistant */
				1,	/* Perfect Armour */
				0,	/* Electric Sensitive */
				0,	/* Combustability */
				0,	/* Indestructable */
			},
		},
		section_sprays_acid|section_flag_right_leg,
	},
	{
		"HNPCalien",
		"right shin",
		NULL,
		{
			15,	/* Health */
			5,		/* Armour */
			0, /* IsOnFire */
			{
				0,	/* Acid Resistant */
				0,	/* Fire Resistant */
				0,	/* Electric Resistant */
				1,	/* Perfect Armour */
				0,	/* Electric Sensitive */
				0,	/* Combustability */
				0,	/* Indestructable */
			},
		},
		section_sprays_acid|section_flag_right_leg,
	},
	{
		"HNPCalien",
		"right foot",
		NULL,
		{
			5,	/* Health */
			5,		/* Armour */
			0, /* IsOnFire */
			{
				0,	/* Acid Resistant */
				0,	/* Fire Resistant */
				0,	/* Electric Resistant */
				1,	/* Perfect Armour */
				0,	/* Electric Sensitive */
				0,	/* Combustability */
				0,	/* Indestructable */
			},
		},
		section_sprays_acid|section_flag_right_foot,
	},
	{
		"HNPCalien",
		"pipe_rt",
		NULL,
		{
			5,	/* Health */
			5,		/* Armour */
			0, /* IsOnFire */
			{
				0,	/* Acid Resistant */
				0,	/* Fire Resistant */
				0,	/* Electric Resistant */
				1,	/* Perfect Armour */
				0,	/* Electric Sensitive */
				0,	/* Combustability */
				0,	/* Indestructable */
			},
		},
		section_sprays_acid,
	},
	{
		"HNPCalien",
		"pipe_rb",
		NULL,
		{
			5,	/* Health */
			5,		/* Armour */
			0, /* IsOnFire */
			{
				0,	/* Acid Resistant */
				0,	/* Fire Resistant */
				0,	/* Electric Resistant */
				1,	/* Perfect Armour */
				0,	/* Electric Sensitive */
				0,	/* Combustability */
				0,	/* Indestructable */
			},
		},
		section_sprays_acid,
	},
	{
		"HNPCalien",
		"pipe_lt",
		NULL,
		{
			5,	/* Health */
			5,		/* Armour */
			0, /* IsOnFire */
			{
				0,	/* Acid Resistant */
				0,	/* Fire Resistant */
				0,	/* Electric Resistant */
				1,	/* Perfect Armour */
				0,	/* Electric Sensitive */
				0,	/* Combustability */
				0,	/* Indestructable */
			},
		},
		section_sprays_acid,
	},
	{
		"HNPCalien",
		"pipe_lb",
		NULL,
		{
			5,	/* Health */
			5,		/* Armour */
			0, /* IsOnFire */
			{
				0,	/* Acid Resistant */
				0,	/* Fire Resistant */
				0,	/* Electric Resistant */
				1,	/* Perfect Armour */
				0,	/* Electric Sensitive */
				0,	/* Combustability */
				0,	/* Indestructable */
			},
		},
		section_sprays_acid,
	},
	{
		"HNPCalien",
		"spike",
		NULL,
		{
			5,	/* Health */
			5,		/* Armour */
			0, /* IsOnFire */
			{
				0,	/* Acid Resistant */
				0,	/* Fire Resistant */
				0,	/* Electric Resistant */
				1,	/* Perfect Armour */
				0,	/* Electric Sensitive */
				0,	/* Combustability */
				0,	/* Indestructable */
			},
		},
		section_sprays_acid,
	},
	{
		"HNPCalien",
		"neck",
		NULL,
		{
			5,	/* Health */
			5,		/* Armour */
			0, /* IsOnFire */
			{
				0,	/* Acid Resistant */
				0,	/* Fire Resistant */
				0,	/* Electric Resistant */
				1,	/* Perfect Armour */
				0,	/* Electric Sensitive */
				0,	/* Combustability */
				0,	/* Indestructable */
			},
		},
		section_has_sparkoflife|section_sprays_acid,
	},
	{
		"HNPCalien",
		"bite main",
		NULL,
		{
			3,	/* Health */
			5,		/* Armour */
			0, /* IsOnFire */
			{
				0,	/* Acid Resistant */
				0,	/* Fire Resistant */
				0,	/* Electric Resistant */
				1,	/* Perfect Armour */
				0,	/* Electric Sensitive */
				0,	/* Combustability */
				0,	/* Indestructable */
			},
		},
		section_sprays_acid,
	},
	{
		"HNPCalien",
		"bite bottom",
		NULL,
		{
			3,	/* Health */
			5,		/* Armour */
			0, /* IsOnFire */
			{
				0,	/* Acid Resistant */
				0,	/* Fire Resistant */
				0,	/* Electric Resistant */
				1,	/* Perfect Armour */
				0,	/* Electric Sensitive */
				0,	/* Combustability */
				0,	/* Indestructable */
			},
		},
		section_sprays_acid,
	},
	{
		"HNPCalien",
		"bite top",
		NULL,
		{
			3,	/* Health */
			5,		/* Armour */
			0, /* IsOnFire */
			{
				0,	/* Acid Resistant */
				0,	/* Fire Resistant */
				0,	/* Electric Resistant */
				1,	/* Perfect Armour */
				0,	/* Electric Sensitive */
				0,	/* Combustability */
				0,	/* Indestructable */
			},
		},
		section_sprays_acid,
	},
	{
		"HNPCalien",
		"right bicep",
		NULL,
		{
			10,	/* Health */
			5,		/* Armour */
			0, /* IsOnFire */
			{
				0,	/* Acid Resistant */
				0,	/* Fire Resistant */
				0,	/* Electric Resistant */
				1,	/* Perfect Armour */
				0,	/* Electric Sensitive */
				0,	/* Combustability */
				0,	/* Indestructable */
			},
		},
		section_sprays_acid|section_flag_right_arm,
	},
	{
		"HNPCalien",
		"right forearm",
		NULL,
		{
			10,	/* Health */
			5,		/* Armour */
			0, /* IsOnFire */
			{
				0,	/* Acid Resistant */
				0,	/* Fire Resistant */
				0,	/* Electric Resistant */
				1,	/* Perfect Armour */
				0,	/* Electric Sensitive */
				0,	/* Combustability */
				0,	/* Indestructable */
			},
		},
		section_sprays_acid|section_flag_right_arm,
	},
	{
		"HNPCalien",
		"right palm",
		NULL,
		{
			10,	/* Health */
			5,		/* Armour */
			0, /* IsOnFire */
			{
				0,	/* Acid Resistant */
				0,	/* Fire Resistant */
				0,	/* Electric Resistant */
				1,	/* Perfect Armour */
				0,	/* Electric Sensitive */
				0,	/* Combustability */
				0,	/* Indestructable */
			},
		},
		section_sprays_acid|section_flag_right_hand,
	},
	{
		"HNPCalien",
		"right thumb b",
		NULL,
		{
			2,	/* Health */
			5,		/* Armour */
			0, /* IsOnFire */
			{
				0,	/* Acid Resistant */
				0,	/* Fire Resistant */
				0,	/* Electric Resistant */
				1,	/* Perfect Armour */
				0,	/* Electric Sensitive */
				0,	/* Combustability */
				0,	/* Indestructable */
			},
		},
		section_sprays_acid,
	},
	{
		"HNPCalien",
		"right thumb a",
		NULL,
		{
			2,	/* Health */
			5,		/* Armour */
			0, /* IsOnFire */
			{
				0,	/* Acid Resistant */
				0,	/* Fire Resistant */
				0,	/* Electric Resistant */
				1,	/* Perfect Armour */
				0,	/* Electric Sensitive */
				0,	/* Combustability */
				0,	/* Indestructable */
			},
		},
		section_sprays_acid,
	},
	{
		"HNPCalien",
		"right finger b",
		NULL,
		{
			2,	/* Health */
			5,		/* Armour */
			0, /* IsOnFire */
			{
				0,	/* Acid Resistant */
				0,	/* Fire Resistant */
				0,	/* Electric Resistant */
				1,	/* Perfect Armour */
				0,	/* Electric Sensitive */
				0,	/* Combustability */
				0,	/* Indestructable */
			},
		},
		section_sprays_acid,
	},
	{
		"HNPCalien",
		"right finger a",
		NULL,
		{
			2,	/* Health */
			5,		/* Armour */
			0, /* IsOnFire */
			{
				0,	/* Acid Resistant */
				0,	/* Fire Resistant */
				0,	/* Electric Resistant */
				1,	/* Perfect Armour */
				0,	/* Electric Sensitive */
				0,	/* Combustability */
				0,	/* Indestructable */
			},
		},
		section_sprays_acid,
	},
	{
		"HNPCalien",
		"left bicep",
		NULL,
		{
			10,	/* Health */
			5,		/* Armour */
			0, /* IsOnFire */
			{
				0,	/* Acid Resistant */
				0,	/* Fire Resistant */
				0,	/* Electric Resistant */
				1,	/* Perfect Armour */
				0,	/* Electric Sensitive */
				0,	/* Combustability */
				0,	/* Indestructable */
			},
		},
		section_sprays_acid|section_flag_left_arm,
	},
	{
		"HNPCalien",
		"left forearm",
		NULL,
		{
			10,	/* Health */
			5,		/* Armour */
			0, /* IsOnFire */
			{
				0,	/* Acid Resistant */
				0,	/* Fire Resistant */
				0,	/* Electric Resistant */
				1,	/* Perfect Armour */
				0,	/* Electric Sensitive */
				0,	/* Combustability */
				0,	/* Indestructable */
			},
		},
		section_sprays_acid|section_flag_left_arm,
	},
	{
		"HNPCalien",
		"left palm",
		NULL,
		{
			10,	/* Health */
			5,		/* Armour */
			0, /* IsOnFire */
			{
				0,	/* Acid Resistant */
				0,	/* Fire Resistant */
				0,	/* Electric Resistant */
				1,	/* Perfect Armour */
				0,	/* Electric Sensitive */
				0,	/* Combustability */
				0,	/* Indestructable */
			},
		},
		section_sprays_acid|section_flag_left_hand,
	},
	{
		"HNPCalien",
		"left thumb b",
		NULL,
		{
			2,	/* Health */
			5,		/* Armour */
			0, /* IsOnFire */
			{
				0,	/* Acid Resistant */
				0,	/* Fire Resistant */
				0,	/* Electric Resistant */
				1,	/* Perfect Armour */
				0,	/* Electric Sensitive */
				0,	/* Combustability */
				0,	/* Indestructable */
			},
		},
		section_sprays_acid,
	},
	{
		"HNPCalien",
		"left thumb a",
		NULL,
		{
			2,	/* Health */
			5,		/* Armour */
			0, /* IsOnFire */
			{
				0,	/* Acid Resistant */
				0,	/* Fire Resistant */
				0,	/* Electric Resistant */
				1,	/* Perfect Armour */
				0,	/* Electric Sensitive */
				0,	/* Combustability */
				0,	/* Indestructable */
			},
		},
		section_sprays_acid,
	},
	{
		"HNPCalien",
		"left finger b",
		NULL,
		{
			2,	/* Health */
			5,		/* Armour */
			0, /* IsOnFire */
			{
				0,	/* Acid Resistant */
				0,	/* Fire Resistant */
				0,	/* Electric Resistant */
				1,	/* Perfect Armour */
				0,	/* Electric Sensitive */
				0,	/* Combustability */
				0,	/* Indestructable */
			},
		},
		section_sprays_acid,
	},
	{
		"HNPCalien",
		"left finger a",
		NULL,
		{
			2,	/* Health */
			5,		/* Armour */
			0, /* IsOnFire */
			{
				0,	/* Acid Resistant */
				0,	/* Fire Resistant */
				0,	/* Electric Resistant */
				1,	/* Perfect Armour */
				0,	/* Electric Sensitive */
				0,	/* Combustability */
				0,	/* Indestructable */
			},
		},
		section_sprays_acid,
	},
	{
		"HNPCalien",
		"l00 tail",
		NULL,
		{
			5,	/* Health */
			5,		/* Armour */
			0, /* IsOnFire */
			{
				0,	/* Acid Resistant */
				0,	/* Fire Resistant */
				0,	/* Electric Resistant */
				1,	/* Perfect Armour */
				0,	/* Electric Sensitive */
				0,	/* Combustability */
				0,	/* Indestructable */
			},
		},
		section_flag_nofurthergibbing|section_sprays_acid|section_flag_tail,
	},
	{
		"HNPCalien",
		"l01 tail",
		NULL,
		{
			5,	/* Health */
			5,		/* Armour */
			0, /* IsOnFire */
			{
				0,	/* Acid Resistant */
				0,	/* Fire Resistant */
				0,	/* Electric Resistant */
				1,	/* Perfect Armour */
				0,	/* Electric Sensitive */
				0,	/* Combustability */
				0,	/* Indestructable */
			},
		},
		section_flag_nofurthergibbing|section_sprays_acid|section_flag_tail,
	},
	{
		"HNPCalien",
		"l02 tail",
		NULL,
		{
			5,	/* Health */
			5,		/* Armour */
			0, /* IsOnFire */
			{
				0,	/* Acid Resistant */
				0,	/* Fire Resistant */
				0,	/* Electric Resistant */
				1,	/* Perfect Armour */
				0,	/* Electric Sensitive */
				0,	/* Combustability */
				0,	/* Indestructable */
			},
		},
		section_flag_nofurthergibbing|section_sprays_acid|section_flag_tail,
	},
	{
		"HNPCalien",
		"l03 tail",
		NULL,
		{
			5,	/* Health */
			5,		/* Armour */
			0, /* IsOnFire */
			{
				0,	/* Acid Resistant */
				0,	/* Fire Resistant */
				0,	/* Electric Resistant */
				1,	/* Perfect Armour */
				0,	/* Electric Sensitive */
				0,	/* Combustability */
				0,	/* Indestructable */
			},
		},
		section_flag_nofurthergibbing|section_sprays_acid|section_flag_tail,
	},
	{
		"HNPCalien",
		"l04 tail",
		NULL,
		{
			5,	/* Health */
			5,		/* Armour */
			0, /* IsOnFire */
			{
				0,	/* Acid Resistant */
				0,	/* Fire Resistant */
				0,	/* Electric Resistant */
				1,	/* Perfect Armour */
				0,	/* Electric Sensitive */
				0,	/* Combustability */
				0,	/* Indestructable */
			},
		},
		section_flag_nofurthergibbing|section_sprays_acid|section_flag_tail,
	},
	{
		"HNPCalien",
		"l05 tail",
		NULL,
		{
			5,	/* Health */
			5,		/* Armour */
			0, /* IsOnFire */
			{
				0,	/* Acid Resistant */
				0,	/* Fire Resistant */
				0,	/* Electric Resistant */
				1,	/* Perfect Armour */
				0,	/* Electric Sensitive */
				0,	/* Combustability */
				0,	/* Indestructable */
			},
		},
		section_flag_nofurthergibbing|section_sprays_acid|section_flag_tail,
	},
	{
		"HNPCalien",
		"l06 tail",
		NULL,
		{
			5,	/* Health */
			5,		/* Armour */
			0, /* IsOnFire */
			{
				0,	/* Acid Resistant */
				0,	/* Fire Resistant */
				0,	/* Electric Resistant */
				1,	/* Perfect Armour */
				0,	/* Electric Sensitive */
				0,	/* Combustability */
				0,	/* Indestructable */
			},
		},
		section_flag_nofurthergibbing|section_sprays_acid|section_flag_tail,
	},
	{
		"HNPCalien",
		"l07 tail",
		NULL,
		{
			5,	/* Health */
			5,		/* Armour */
			0, /* IsOnFire */
			{
				0,	/* Acid Resistant */
				0,	/* Fire Resistant */
				0,	/* Electric Resistant */
				1,	/* Perfect Armour */
				0,	/* Electric Sensitive */
				0,	/* Combustability */
				0,	/* Indestructable */
			},
		},
		section_flag_nofurthergibbing|section_sprays_acid|section_flag_tail,
	},
	{
		"HNPCalien",
		"l08 tail",
		NULL,
		{
			5,	/* Health */
			5,		/* Armour */
			0, /* IsOnFire */
			{
				0,	/* Acid Resistant */
				0,	/* Fire Resistant */
				0,	/* Electric Resistant */
				1,	/* Perfect Armour */
				0,	/* Electric Sensitive */
				0,	/* Combustability */
				0,	/* Indestructable */
			},
		},
		section_flag_nofurthergibbing|section_sprays_acid|section_flag_tail,
	},
	{
		"HNPCalien",
		"l09 tail",
		NULL,
		{
			5,	/* Health */
			5,		/* Armour */
			0, /* IsOnFire */
			{
				0,	/* Acid Resistant */
				0,	/* Fire Resistant */
				0,	/* Electric Resistant */
				1,	/* Perfect Armour */
				0,	/* Electric Sensitive */
				0,	/* Combustability */
				0,	/* Indestructable */
			},
		},
		section_flag_nofurthergibbing|section_sprays_acid|section_flag_tail,
	},
	{
		"HNPCalien",
		"l10 tail",
		NULL,
		{
			5,	/* Health */
			5,		/* Armour */
			0, /* IsOnFire */
			{
				0,	/* Acid Resistant */
				0,	/* Fire Resistant */
				0,	/* Electric Resistant */
				1,	/* Perfect Armour */
				0,	/* Electric Sensitive */
				0,	/* Combustability */
				0,	/* Indestructable */
			},
		},
		section_flag_nofurthergibbing|section_sprays_acid|section_flag_tail,
	},
	{
		"HNPCalien",
		"l11 tail",
		NULL,
		{
			5,	/* Health */
			5,		/* Armour */
			0, /* IsOnFire */
			{
				0,	/* Acid Resistant */
				0,	/* Fire Resistant */
				0,	/* Electric Resistant */
				1,	/* Perfect Armour */
				0,	/* Electric Sensitive */
				0,	/* Combustability */
				0,	/* Indestructable */
			},
		},
		section_flag_nofurthergibbing|section_sprays_acid|section_flag_tail,
	},
	/* PRAETORIAN GUARD */
	{
		"hnpcpretorian",
		"head",
		NULL,
		{
			50,		/* Health */
			120,	/* Armour */
			0, 		/* IsOnFire */
			{
				0,	/* Acid Resistant */
				0,	/* Fire Resistant */
				0,	/* Electric Resistant */
				1,	/* Perfect Armour */
				0,	/* Electric Sensitive */
				0,	/* Combustability */
				0,	/* Indestructable */
			},
		},
		section_flag_head|section_has_sparkoflife|section_sprays_acid,
	},
	{
		"hnpcpretorian",
		"chest",
		NULL,
		{
			100,	/* Health */
			60,		/* Armour */
			0, 		/* IsOnFire */
			{
				0,	/* Acid Resistant */
				0,	/* Fire Resistant */
				0,	/* Electric Resistant */
				1,	/* Perfect Armour */
				0,	/* Electric Sensitive */
				0,	/* Combustability */
				0,	/* Indestructable */
			},
		},
		section_has_sparkoflife|section_sprays_acid|section_flag_gibbwhenfragged,
	},
	{
		"hnpcpretorian",
		"abdom",
		NULL,
		{
			100,	/* Health */
			60,		/* Armour */
			0, 		/* IsOnFire */
			{
				0,	/* Acid Resistant */
				0,	/* Fire Resistant */
				0,	/* Electric Resistant */
				1,	/* Perfect Armour */
				0,	/* Electric Sensitive */
				0,	/* Combustability */
				0,	/* Indestructable */
			},
		},
		section_has_sparkoflife|section_sprays_acid,
	},
	{
		"hnpcpretorian",
		"left thigh",
		NULL,
		{
			100,	/* Health */
			60,		/* Armour */
			0, 		/* IsOnFire */
			{
				0,	/* Acid Resistant */
				0,	/* Fire Resistant */
				0,	/* Electric Resistant */
				1,	/* Perfect Armour */
				0,	/* Electric Sensitive */
				0,	/* Combustability */
				0,	/* Indestructable */
			},
		},
		section_sprays_acid|section_flag_left_leg,
	},
	{
		"hnpcpretorian",
		"left shin",
		NULL,
		{
			75,		/* Health */
			60,		/* Armour */
			0, 		/* IsOnFire */
			{
				0,	/* Acid Resistant */
				0,	/* Fire Resistant */
				0,	/* Electric Resistant */
				1,	/* Perfect Armour */
				0,	/* Electric Sensitive */
				0,	/* Combustability */
				0,	/* Indestructable */
			},
		},
		section_sprays_acid|section_flag_left_leg,
	},
	{
		"hnpcpretorian",
		"left ankle",
		NULL,
		{
			50,		/* Health */
			60,		/* Armour */
			0, 		/* IsOnFire */
			{
				0,	/* Acid Resistant */
				0,	/* Fire Resistant */
				0,	/* Electric Resistant */
				1,	/* Perfect Armour */
				0,	/* Electric Sensitive */
				0,	/* Combustability */
				0,	/* Indestructable */
			},
		},
		section_sprays_acid|section_flag_left_leg,
	},
	{
		"hnpcpretorian",
		"left foot",
		NULL,
		{
			25,		/* Health */
			60,		/* Armour */
			0, 		/* IsOnFire */
			{
				0,	/* Acid Resistant */
				0,	/* Fire Resistant */
				0,	/* Electric Resistant */
				1,	/* Perfect Armour */
				0,	/* Electric Sensitive */
				0,	/* Combustability */
				0,	/* Indestructable */
			},
		},
		section_sprays_acid|section_flag_left_foot,
	},
	{
		"hnpcpretorian",
		"right thigh",
		NULL,
		{
			100,	/* Health */
			60,		/* Armour */
			0, 		/* IsOnFire */
			{
				0,	/* Acid Resistant */
				0,	/* Fire Resistant */
				0,	/* Electric Resistant */
				1,	/* Perfect Armour */
				0,	/* Electric Sensitive */
				0,	/* Combustability */
				0,	/* Indestructable */
			},
		},
		section_sprays_acid|section_flag_right_leg,
	},
	{
		"hnpcpretorian",
		"right shin",
		NULL,
		{
			75,		/* Health */
			60,		/* Armour */
			0, 		/* IsOnFire */
			{
				0,	/* Acid Resistant */
				0,	/* Fire Resistant */
				0,	/* Electric Resistant */
				1,	/* Perfect Armour */
				0,	/* Electric Sensitive */
				0,	/* Combustability */
				0,	/* Indestructable */
			},
		},
		section_sprays_acid|section_flag_right_leg,
	},
	{
		"hnpcpretorian",
		"right ankle",
		NULL,
		{
			50,		/* Health */
			60,		/* Armour */
			0, 		/* IsOnFire */
			{
				0,	/* Acid Resistant */
				0,	/* Fire Resistant */
				0,	/* Electric Resistant */
				1,	/* Perfect Armour */
				0,	/* Electric Sensitive */
				0,	/* Combustability */
				0,	/* Indestructable */
			},
		},
		section_sprays_acid|section_flag_right_leg,
	},
	{
		"hnpcpretorian",
		"right foot",
		NULL,
		{
			25,		/* Health */
			60,		/* Armour */
			0, 		/* IsOnFire */
			{
				0,	/* Acid Resistant */
				0,	/* Fire Resistant */
				0,	/* Electric Resistant */
				1,	/* Perfect Armour */
				0,	/* Electric Sensitive */
				0,	/* Combustability */
				0,	/* Indestructable */
			},
		},
		section_sprays_acid|section_flag_right_foot,
	},
	{
		"hnpcpretorian",
		"neck",
		NULL,
		{
			25,		/* Health */
			60,		/* Armour */
			0, 		/* IsOnFire */
			{
				0,	/* Acid Resistant */
				0,	/* Fire Resistant */
				0,	/* Electric Resistant */
				1,	/* Perfect Armour */
				0,	/* Electric Sensitive */
				0,	/* Combustability */
				0,	/* Indestructable */
			},
		},
		section_has_sparkoflife|section_sprays_acid,
	},
	{
		"hnpcpretorian",
		"bite main",
		NULL,
		{
			15,		/* Health */
			60,		/* Armour */
			0, 		/* IsOnFire */
			{
				0,	/* Acid Resistant */
				0,	/* Fire Resistant */
				0,	/* Electric Resistant */
				1,	/* Perfect Armour */
				0,	/* Electric Sensitive */
				0,	/* Combustability */
				0,	/* Indestructable */
			},
		},
		section_sprays_acid,
	},
	{
		"hnpcpretorian",
		"bite bottom",
		NULL,
		{
			15,		/* Health */
			60,		/* Armour */
			0, 		/* IsOnFire */
			{
				0,	/* Acid Resistant */
				0,	/* Fire Resistant */
				0,	/* Electric Resistant */
				1,	/* Perfect Armour */
				0,	/* Electric Sensitive */
				0,	/* Combustability */
				0,	/* Indestructable */
			},
		},
		section_sprays_acid,
	},
	{
		"hnpcpretorian",
		"bite top",
		NULL,
		{
			15,		/* Health */
			60,		/* Armour */
			0, 		/* IsOnFire */
			{
				0,	/* Acid Resistant */
				0,	/* Fire Resistant */
				0,	/* Electric Resistant */
				1,	/* Perfect Armour */
				0,	/* Electric Sensitive */
				0,	/* Combustability */
				0,	/* Indestructable */
			},
		},
		section_sprays_acid,
	},
	{
		"hnpcpretorian",
		"right bicep",
		NULL,
		{
			50,		/* Health */
			60,		/* Armour */
			0, 		/* IsOnFire */
			{
				0,	/* Acid Resistant */
				0,	/* Fire Resistant */
				0,	/* Electric Resistant */
				1,	/* Perfect Armour */
				0,	/* Electric Sensitive */
				0,	/* Combustability */
				0,	/* Indestructable */
			},
		},
		section_sprays_acid|section_flag_right_arm,
	},
	{
		"hnpcpretorian",
		"right forearm",
		NULL,
		{
			50,		/* Health */
			60,		/* Armour */
			0, 		/* IsOnFire */
			{
				0,	/* Acid Resistant */
				0,	/* Fire Resistant */
				0,	/* Electric Resistant */
				1,	/* Perfect Armour */
				0,	/* Electric Sensitive */
				0,	/* Combustability */
				0,	/* Indestructable */
			},
		},
		section_sprays_acid|section_flag_right_arm,
	},
	{
		"hnpcpretorian",
		"right palm",
		NULL,
		{
			50,		/* Health */
			60,		/* Armour */
			0, 		/* IsOnFire */
			{
				0,	/* Acid Resistant */
				0,	/* Fire Resistant */
				0,	/* Electric Resistant */
				1,	/* Perfect Armour */
				0,	/* Electric Sensitive */
				0,	/* Combustability */
				0,	/* Indestructable */
			},
		},
		section_sprays_acid|section_flag_right_hand,
	},
	{
		"hnpcpretorian",
		"right thumb b",
		NULL,
		{
			10,		/* Health */
			60,		/* Armour */
			0, 		/* IsOnFire */
			{
				0,	/* Acid Resistant */
				0,	/* Fire Resistant */
				0,	/* Electric Resistant */
				1,	/* Perfect Armour */
				0,	/* Electric Sensitive */
				0,	/* Combustability */
				0,	/* Indestructable */
			},
		},
		section_sprays_acid,
	},
	{
		"hnpcpretorian",
		"right thumb a",
		NULL,
		{
			10,		/* Health */
			60,		/* Armour */
			0, 		/* IsOnFire */
			{
				0,	/* Acid Resistant */
				0,	/* Fire Resistant */
				0,	/* Electric Resistant */
				1,	/* Perfect Armour */
				0,	/* Electric Sensitive */
				0,	/* Combustability */
				0,	/* Indestructable */
			},
		},
		section_sprays_acid,
	},
	{
		"hnpcpretorian",
		"right finger b",
		NULL,
		{
			10,		/* Health */
			60,		/* Armour */
			0, 		/* IsOnFire */
			{
				0,	/* Acid Resistant */
				0,	/* Fire Resistant */
				0,	/* Electric Resistant */
				1,	/* Perfect Armour */
				0,	/* Electric Sensitive */
				0,	/* Combustability */
				0,	/* Indestructable */
			},
		},
		section_sprays_acid,
	},
	{
		"hnpcpretorian",
		"right finger a",
		NULL,
		{
			10,		/* Health */
			60,		/* Armour */
			0, 		/* IsOnFire */
			{
				0,	/* Acid Resistant */
				0,	/* Fire Resistant */
				0,	/* Electric Resistant */
				1,	/* Perfect Armour */
				0,	/* Electric Sensitive */
				0,	/* Combustability */
				0,	/* Indestructable */
			},
		},
		section_sprays_acid,
	},
	{
		"hnpcpretorian",
		"left bicep",
		NULL,
		{
			50,		/* Health */
			60,		/* Armour */
			0, 		/* IsOnFire */
			{
				0,	/* Acid Resistant */
				0,	/* Fire Resistant */
				0,	/* Electric Resistant */
				1,	/* Perfect Armour */
				0,	/* Electric Sensitive */
				0,	/* Combustability */
				0,	/* Indestructable */
			},
		},
		section_sprays_acid|section_flag_left_arm,
	},
	{
		"hnpcpretorian",
		"left forearm",
		NULL,
		{
			50,		/* Health */
			60,		/* Armour */
			0, 		/* IsOnFire */
			{
				0,	/* Acid Resistant */
				0,	/* Fire Resistant */
				0,	/* Electric Resistant */
				1,	/* Perfect Armour */
				0,	/* Electric Sensitive */
				0,	/* Combustability */
				0,	/* Indestructable */
			},
		},
		section_sprays_acid|section_flag_left_arm,
	},
	{
		"hnpcpretorian",
		"left palm",
		NULL,
		{
			50,		/* Health */
			60,		/* Armour */
			0, 		/* IsOnFire */
			{
				0,	/* Acid Resistant */
				0,	/* Fire Resistant */
				0,	/* Electric Resistant */
				1,	/* Perfect Armour */
				0,	/* Electric Sensitive */
				0,	/* Combustability */
				0,	/* Indestructable */
			},
		},
		section_sprays_acid|section_flag_left_hand,
	},
	{
		"hnpcpretorian",
		"left thumb b",
		NULL,
		{
			10,		/* Health */
			60,		/* Armour */
			0, 		/* IsOnFire */
			{
				0,	/* Acid Resistant */
				0,	/* Fire Resistant */
				0,	/* Electric Resistant */
				1,	/* Perfect Armour */
				0,	/* Electric Sensitive */
				0,	/* Combustability */
				0,	/* Indestructable */
			},
		},
		section_sprays_acid,
	},
	{
		"hnpcpretorian",
		"left thumb a",
		NULL,
		{
			10,		/* Health */
			60,		/* Armour */
			0, 		/* IsOnFire */
			{
				0,	/* Acid Resistant */
				0,	/* Fire Resistant */
				0,	/* Electric Resistant */
				1,	/* Perfect Armour */
				0,	/* Electric Sensitive */
				0,	/* Combustability */
				0,	/* Indestructable */
			},
		},
		section_sprays_acid,
	},
	{
		"hnpcpretorian",
		"left finger b",
		NULL,
		{
			10,		/* Health */
			60,		/* Armour */
			0, 		/* IsOnFire */
			{
				0,	/* Acid Resistant */
				0,	/* Fire Resistant */
				0,	/* Electric Resistant */
				1,	/* Perfect Armour */
				0,	/* Electric Sensitive */
				0,	/* Combustability */
				0,	/* Indestructable */
			},
		},
		section_sprays_acid,
	},
	{
		"hnpcpretorian",
		"left finger a",
		NULL,
		{
			10,		/* Health */
			60,		/* Armour */
			0, 		/* IsOnFire */
			{
				0,	/* Acid Resistant */
				0,	/* Fire Resistant */
				0,	/* Electric Resistant */
				1,	/* Perfect Armour */
				0,	/* Electric Sensitive */
				0,	/* Combustability */
				0,	/* Indestructable */
			},
		},
		section_sprays_acid,
	},
	{
		"hnpcpretorian",
		"l00 tail",
		NULL,
		{
			25,		/* Health */
			60,		/* Armour */
			0, 		/* IsOnFire */
			{
				0,	/* Acid Resistant */
				0,	/* Fire Resistant */
				0,	/* Electric Resistant */
				1,	/* Perfect Armour */
				0,	/* Electric Sensitive */
				0,	/* Combustability */
				0,	/* Indestructable */
			},
		},
		section_flag_nofurthergibbing|section_sprays_acid|section_flag_tail,
	},
	{
		"hnpcpretorian",
		"l01 tail",
		NULL,
		{
			25,		/* Health */
			60,		/* Armour */
			0, 		/* IsOnFire */
			{
				0,	/* Acid Resistant */
				0,	/* Fire Resistant */
				0,	/* Electric Resistant */
				1,	/* Perfect Armour */
				0,	/* Electric Sensitive */
				0,	/* Combustability */
				0,	/* Indestructable */
			},
		},
		section_flag_nofurthergibbing|section_sprays_acid|section_flag_tail,
	},
	{
		"hnpcpretorian",
		"l02 tail",
		NULL,
		{
			25,		/* Health */
			60,		/* Armour */
			0, 		/* IsOnFire */
			{
				0,	/* Acid Resistant */
				0,	/* Fire Resistant */
				0,	/* Electric Resistant */
				1,	/* Perfect Armour */
				0,	/* Electric Sensitive */
				0,	/* Combustability */
				0,	/* Indestructable */
			},
		},
		section_flag_nofurthergibbing|section_sprays_acid|section_flag_tail,
	},
	{
		"hnpcpretorian",
		"l03 tail",
		NULL,
		{
			25,		/* Health */
			60,		/* Armour */
			0, 		/* IsOnFire */
			{
				0,	/* Acid Resistant */
				0,	/* Fire Resistant */
				0,	/* Electric Resistant */
				1,	/* Perfect Armour */
				0,	/* Electric Sensitive */
				0,	/* Combustability */
				0,	/* Indestructable */
			},
		},
		section_flag_nofurthergibbing|section_sprays_acid|section_flag_tail,
	},
	{
		"hnpcpretorian",
		"l04 tail",
		NULL,
		{
			25,		/* Health */
			60,		/* Armour */
			0, 		/* IsOnFire */
			{
				0,	/* Acid Resistant */
				0,	/* Fire Resistant */
				0,	/* Electric Resistant */
				1,	/* Perfect Armour */
				0,	/* Electric Sensitive */
				0,	/* Combustability */
				0,	/* Indestructable */
			},
		},
		section_flag_nofurthergibbing|section_sprays_acid|section_flag_tail,
	},
	{
		"hnpcpretorian",
		"l05 tail",
		NULL,
		{
			25,		/* Health */
			60,		/* Armour */
			0, 		/* IsOnFire */
			{
				0,	/* Acid Resistant */
				0,	/* Fire Resistant */
				0,	/* Electric Resistant */
				1,	/* Perfect Armour */
				0,	/* Electric Sensitive */
				0,	/* Combustability */
				0,	/* Indestructable */
			},
		},
		section_flag_nofurthergibbing|section_sprays_acid|section_flag_tail,
	},
	{
		"hnpcpretorian",
		"l06 tail",
		NULL,
		{
			25,		/* Health */
			60,		/* Armour */
			0, 		/* IsOnFire */
			{
				0,	/* Acid Resistant */
				0,	/* Fire Resistant */
				0,	/* Electric Resistant */
				1,	/* Perfect Armour */
				0,	/* Electric Sensitive */
				0,	/* Combustability */
				0,	/* Indestructable */
			},
		},
		section_flag_nofurthergibbing|section_sprays_acid|section_flag_tail,
	},
	{
		"hnpcpretorian",
		"l07 tail",
		NULL,
		{
			25,		/* Health */
			60,		/* Armour */
			0, 		/* IsOnFire */
			{
				0,	/* Acid Resistant */
				0,	/* Fire Resistant */
				0,	/* Electric Resistant */
				1,	/* Perfect Armour */
				0,	/* Electric Sensitive */
				0,	/* Combustability */
				0,	/* Indestructable */
			},
		},
		section_flag_nofurthergibbing|section_sprays_acid|section_flag_tail,
	},
	{
		"hnpcpretorian",
		"l08 tail",
		NULL,
		{
			25,		/* Health */
			60,		/* Armour */
			0, 		/* IsOnFire */
			{
				0,	/* Acid Resistant */
				0,	/* Fire Resistant */
				0,	/* Electric Resistant */
				1,	/* Perfect Armour */
				0,	/* Electric Sensitive */
				0,	/* Combustability */
				0,	/* Indestructable */
			},
		},
		section_flag_nofurthergibbing|section_sprays_acid|section_flag_tail,
	},
	{
		"hnpcpretorian",
		"l09 tail",
		NULL,
		{
			25,		/* Health */
			60,		/* Armour */
			0, 		/* IsOnFire */
			{
				0,	/* Acid Resistant */
				0,	/* Fire Resistant */
				0,	/* Electric Resistant */
				1,	/* Perfect Armour */
				0,	/* Electric Sensitive */
				0,	/* Combustability */
				0,	/* Indestructable */
			},
		},
		section_flag_nofurthergibbing|section_sprays_acid|section_flag_tail,
	},
	{
		"hnpcpretorian",
		"l10 tail",
		NULL,
		{
			25,		/* Health */
			60,		/* Armour */
			0, 		/* IsOnFire */
			{
				0,	/* Acid Resistant */
				0,	/* Fire Resistant */
				0,	/* Electric Resistant */
				1,	/* Perfect Armour */
				0,	/* Electric Sensitive */
				0,	/* Combustability */
				0,	/* Indestructable */
			},
		},
		section_flag_nofurthergibbing|section_sprays_acid|section_flag_tail,
	},
	{
		"hnpcpretorian",
		"l11 tail",
		NULL,
		{
			25,		/* Health */
			60,		/* Armour */
			0, 		/* IsOnFire */
			{
				0,	/* Acid Resistant */
				0,	/* Fire Resistant */
				0,	/* Electric Resistant */
				1,	/* Perfect Armour */
				0,	/* Electric Sensitive */
				0,	/* Combustability */
				0,	/* Indestructable */
			},
		},
		section_flag_nofurthergibbing|section_sprays_acid|section_flag_tail,
	},
	/* PREDALIENS */
	{
		"hnpcpred_alien",
		"head",
		NULL,
		{
			80,	/* Health */
			40,	/* Armour */
			0, 	/* IsOnFire */
			{
				1,	/* Acid Resistant */
				0,	/* Fire Resistant */
				0,	/* Electric Resistant */
				1,	/* Perfect Armour */
				0,	/* Electric Sensitive */
				0,	/* Combustability */
				0,	/* Indestructable */
			},
		},
		section_flag_head|section_has_sparkoflife|section_sprays_acid,
	},
	{
		"hnpcpred_alien",
		"chest",
		NULL,
		{
			160,	/* Health */
			40,	/* Armour */
			0, 	/* IsOnFire */
			{
				1,	/* Acid Resistant */
				0,	/* Fire Resistant */
				0,	/* Electric Resistant */
				1,	/* Perfect Armour */
				0,	/* Electric Sensitive */
				0,	/* Combustability */
				0,	/* Indestructable */
			},
		},
		section_has_sparkoflife|section_sprays_acid|section_flag_gibbwhenfragged,
	},
	{
		"hnpcpred_alien",
		"tummy",
		NULL,
		{
			160,	/* Health */
			40,	/* Armour */
			0, 	/* IsOnFire */
			{
				1,	/* Acid Resistant */
				0,	/* Fire Resistant */
				0,	/* Electric Resistant */
				1,	/* Perfect Armour */
				0,	/* Electric Sensitive */
				0,	/* Combustability */
				0,	/* Indestructable */
			},
		},
		section_has_sparkoflife|section_sprays_acid|section_flag_gibbwhenfragged,
	},
	{
		"hnpcpred_alien",
		"arse",
		NULL,
		{
			160,	/* Health */
			40,	/* Armour */
			0, 	/* IsOnFire */
			{
				1,	/* Acid Resistant */
				0,	/* Fire Resistant */
				0,	/* Electric Resistant */
				1,	/* Perfect Armour */
				0,	/* Electric Sensitive */
				0,	/* Combustability */
				0,	/* Indestructable */
			},
		},
		section_has_sparkoflife|section_sprays_acid,
	},
	{
		"hnpcpred_alien",
		"left thigh",
		NULL,
		{
			160,	/* Health */
			40,	/* Armour */
			0,  /* IsOnFire */
			{
				1,	/* Acid Resistant */
				0,	/* Fire Resistant */
				0,	/* Electric Resistant */
				1,	/* Perfect Armour */
				0,	/* Electric Sensitive */
				0,	/* Combustability */
				0,	/* Indestructable */
			},
		},
		section_sprays_acid|section_flag_left_leg,
	},
	{
		"hnpcpred_alien",
		"left shin",
		NULL,
		{
			120,	/* Health */
			40,	/* Armour */
			0,  /* IsOnFire */
			{
				1,	/* Acid Resistant */
				0,	/* Fire Resistant */
				0,	/* Electric Resistant */
				1,	/* Perfect Armour */
				0,	/* Electric Sensitive */
				0,	/* Combustability */
				0,	/* Indestructable */
			},
		},
		section_sprays_acid|section_flag_left_leg,
	},
	{
		"hnpcpred_alien",
		"left foot",
		NULL,
		{
			40,	/* Health */
			40,	/* Armour */
			0,  /* IsOnFire */
			{
				1,	/* Acid Resistant */
				0,	/* Fire Resistant */
				0,	/* Electric Resistant */
				1,	/* Perfect Armour */
				0,	/* Electric Sensitive */
				0,	/* Combustability */
				0,	/* Indestructable */
			},
		},
		section_sprays_acid|section_flag_left_foot,
	},
	{
		"hnpcpred_alien",
		"lx",
		NULL,
		{
			40,	/* Health */
			40,	/* Armour */
			0,  /* IsOnFire */
			{
				1,	/* Acid Resistant */
				0,	/* Fire Resistant */
				0,	/* Electric Resistant */
				1,	/* Perfect Armour */
				0,	/* Electric Sensitive */
				0,	/* Combustability */
				0,	/* Indestructable */
			},
		},
		section_sprays_acid|section_flag_left_foot,
	},
	{
		"hnpcpred_alien",
		"toe a",
		NULL,
		{
			40,	/* Health */
			40,	/* Armour */
			0,  /* IsOnFire */
			{
				1,	/* Acid Resistant */
				0,	/* Fire Resistant */
				0,	/* Electric Resistant */
				1,	/* Perfect Armour */
				0,	/* Electric Sensitive */
				0,	/* Combustability */
				0,	/* Indestructable */
			},
		},
		section_sprays_acid|section_flag_left_foot,
	},
	{
		"hnpcpred_alien",
		"toe b",
		NULL,
		{
			40,	/* Health */
			40,	/* Armour */
			0,  /* IsOnFire */
			{
				1,	/* Acid Resistant */
				0,	/* Fire Resistant */
				0,	/* Electric Resistant */
				1,	/* Perfect Armour */
				0,	/* Electric Sensitive */
				0,	/* Combustability */
				0,	/* Indestructable */
			},
		},
		section_sprays_acid|section_flag_left_foot,
	},
	{
		"hnpcpred_alien",
		"toe c",
		NULL,
		{
			40,	/* Health */
			40,	/* Armour */
			0,  /* IsOnFire */
			{
				1,	/* Acid Resistant */
				0,	/* Fire Resistant */
				0,	/* Electric Resistant */
				1,	/* Perfect Armour */
				0,	/* Electric Sensitive */
				0,	/* Combustability */
				0,	/* Indestructable */
			},
		},
		section_sprays_acid|section_flag_left_foot,
	},
	{
		"hnpcpred_alien",
		"right thigh",
		NULL,
		{
			160,	/* Health */
			40,	/* Armour */
			0,  /* IsOnFire */
			{
				1,	/* Acid Resistant */
				0,	/* Fire Resistant */
				0,	/* Electric Resistant */
				1,	/* Perfect Armour */
				0,	/* Electric Sensitive */
				0,	/* Combustability */
				0,	/* Indestructable */
			},
		},
		section_sprays_acid|section_flag_right_leg,
	},
	{
		"hnpcpred_alien",
		"right shin",
		NULL,
		{
			120,	/* Health */
			40,	/* Armour */
			0,  /* IsOnFire */
			{
				1,	/* Acid Resistant */
				0,	/* Fire Resistant */
				0,	/* Electric Resistant */
				1,	/* Perfect Armour */
				0,	/* Electric Sensitive */
				0,	/* Combustability */
				0,	/* Indestructable */
			},
		},
		section_sprays_acid|section_flag_right_leg,
	},
	{
		"hnpcpred_alien",
		"right foot",
		NULL,
		{
			40,	/* Health */
			40,	/* Armour */
			0, 	/* IsOnFire */
			{
				1,	/* Acid Resistant */
				0,	/* Fire Resistant */
				0,	/* Electric Resistant */
				1,	/* Perfect Armour */
				0,	/* Electric Sensitive */
				0,	/* Combustability */
				0,	/* Indestructable */
			},
		},
		section_sprays_acid|section_flag_right_foot,
	},
	{
		"hnpcpred_alien",
		"rx",
		NULL,
		{
			40,	/* Health */
			40,	/* Armour */
			0,  /* IsOnFire */
			{
				1,	/* Acid Resistant */
				0,	/* Fire Resistant */
				0,	/* Electric Resistant */
				1,	/* Perfect Armour */
				0,	/* Electric Sensitive */
				0,	/* Combustability */
				0,	/* Indestructable */
			},
		},
		section_sprays_acid|section_flag_left_foot,
	},
	{
		"hnpcpred_alien",
		"toe d",
		NULL,
		{
			40,	/* Health */
			40,	/* Armour */
			0,  /* IsOnFire */
			{
				1,	/* Acid Resistant */
				0,	/* Fire Resistant */
				0,	/* Electric Resistant */
				1,	/* Perfect Armour */
				0,	/* Electric Sensitive */
				0,	/* Combustability */
				0,	/* Indestructable */
			},
		},
		section_sprays_acid|section_flag_left_foot,
	},
	{
		"hnpcpred_alien",
		"toe e",
		NULL,
		{
			40,	/* Health */
			40,	/* Armour */
			0,  /* IsOnFire */
			{
				1,	/* Acid Resistant */
				0,	/* Fire Resistant */
				0,	/* Electric Resistant */
				1,	/* Perfect Armour */
				0,	/* Electric Sensitive */
				0,	/* Combustability */
				0,	/* Indestructable */
			},
		},
		section_sprays_acid|section_flag_left_foot,
	},
	{
		"hnpcpred_alien",
		"toe f",
		NULL,
		{
			40,	/* Health */
			40,	/* Armour */
			0,  /* IsOnFire */
			{
				1,	/* Acid Resistant */
				0,	/* Fire Resistant */
				0,	/* Electric Resistant */
				1,	/* Perfect Armour */
				0,	/* Electric Sensitive */
				0,	/* Combustability */
				0,	/* Indestructable */
			},
		},
		section_sprays_acid|section_flag_left_foot,
	},
	{
		"hnpcpred_alien",
		"neck",
		NULL,
		{
			40,	/* Health */
			40,	/* Armour */
			0, 	/* IsOnFire */
			{
				1,	/* Acid Resistant */
				0,	/* Fire Resistant */
				0,	/* Electric Resistant */
				1,	/* Perfect Armour */
				0,	/* Electric Sensitive */
				0,	/* Combustability */
				0,	/* Indestructable */
			},
		},
		section_has_sparkoflife|section_sprays_acid,
	},
	{
		"hnpcpred_alien",
		"right bicep",
		NULL,
		{
			80,	/* Health */
			40,	/* Armour */
			0, 	/* IsOnFire */
			{
				1,	/* Acid Resistant */
				0,	/* Fire Resistant */
				0,	/* Electric Resistant */
				1,	/* Perfect Armour */
				0,	/* Electric Sensitive */
				0,	/* Combustability */
				0,	/* Indestructable */
			},
		},
		section_sprays_acid|section_flag_right_arm,
	},
	{
		"hnpcpred_alien",
		"right forearm",
		NULL,
		{
			80,	/* Health */
			40,		/* Armour */
			0, /* IsOnFire */
			{
				1,	/* Acid Resistant */
				0,	/* Fire Resistant */
				0,	/* Electric Resistant */
				1,	/* Perfect Armour */
				0,	/* Electric Sensitive */
				0,	/* Combustability */
				0,	/* Indestructable */
			},
		},
		section_sprays_acid|section_flag_right_arm,
	},
	{
		"hnpcpred_alien",
		"right hand",
		NULL,
		{
			80,	/* Health */
			40,		/* Armour */
			0, /* IsOnFire */
			{
				1,	/* Acid Resistant */
				0,	/* Fire Resistant */
				0,	/* Electric Resistant */
				1,	/* Perfect Armour */
				0,	/* Electric Sensitive */
				0,	/* Combustability */
				0,	/* Indestructable */
			},
		},
		section_sprays_acid|section_flag_right_hand,
	},
	{
		"hnpcpred_alien",
		"right thumb",
		NULL,
		{
			16,	/* Health */
			40,		/* Armour */
			0, /* IsOnFire */
			{
				1,	/* Acid Resistant */
				0,	/* Fire Resistant */
				0,	/* Electric Resistant */
				1,	/* Perfect Armour */
				0,	/* Electric Sensitive */
				0,	/* Combustability */
				0,	/* Indestructable */
			},
		},
		section_sprays_acid|section_flag_right_hand,
	},
	{
		"hnpcpred_alien",
		"right fing a",
		NULL,
		{
			16,	/* Health */
			40,		/* Armour */
			0, /* IsOnFire */
			{
				1,	/* Acid Resistant */
				0,	/* Fire Resistant */
				0,	/* Electric Resistant */
				1,	/* Perfect Armour */
				0,	/* Electric Sensitive */
				0,	/* Combustability */
				0,	/* Indestructable */
			},
		},
		section_sprays_acid|section_flag_right_hand,
	},
	{
		"hnpcpred_alien",
		"right fing b",
		NULL,
		{
			16,	/* Health */
			40,		/* Armour */
			0, /* IsOnFire */
			{
				1,	/* Acid Resistant */
				0,	/* Fire Resistant */
				0,	/* Electric Resistant */
				1,	/* Perfect Armour */
				0,	/* Electric Sensitive */
				0,	/* Combustability */
				0,	/* Indestructable */
			},
		},
		section_sprays_acid|section_flag_right_hand,
	},
	{
		"hnpcpred_alien",
		"right fing c",
		NULL,
		{
			16,	/* Health */
			40,		/* Armour */
			0, /* IsOnFire */
			{
				1,	/* Acid Resistant */
				0,	/* Fire Resistant */
				0,	/* Electric Resistant */
				1,	/* Perfect Armour */
				0,	/* Electric Sensitive */
				0,	/* Combustability */
				0,	/* Indestructable */
			},
		},
		section_sprays_acid|section_flag_right_hand,
	},
	{
		"hnpcpred_alien",
		"right arm fin",
		NULL,
		{
			20,	/* Health */
			40,		/* Armour */
			0, /* IsOnFire */
			{
				1,	/* Acid Resistant */
				0,	/* Fire Resistant */
				0,	/* Electric Resistant */
				1,	/* Perfect Armour */
				0,	/* Electric Sensitive */
				0,	/* Combustability */
				0,	/* Indestructable */
			},
		},
		section_sprays_acid
	},
	{
		"hnpcpred_alien",
		"left bicep",
		NULL,
		{
			80,	/* Health */
			40,	/* Armour */
			0, 	/* IsOnFire */
			{
				1,	/* Acid Resistant */
				0,	/* Fire Resistant */
				0,	/* Electric Resistant */
				1,	/* Perfect Armour */
				0,	/* Electric Sensitive */
				0,	/* Combustability */
				0,	/* Indestructable */
			},
		},
		section_sprays_acid|section_flag_left_arm,
	},
	{
		"hnpcpred_alien",
		"left forearm",
		NULL,
		{
			80,	/* Health */
			40,		/* Armour */
			0, /* IsOnFire */
			{
				1,	/* Acid Resistant */
				0,	/* Fire Resistant */
				0,	/* Electric Resistant */
				1,	/* Perfect Armour */
				0,	/* Electric Sensitive */
				0,	/* Combustability */
				0,	/* Indestructable */
			},
		},
		section_sprays_acid|section_flag_left_arm,
	},
	{
		"hnpcpred_alien",
		"left hand",
		NULL,
		{
			80,	/* Health */
			40,		/* Armour */
			0, /* IsOnFire */
			{
				1,	/* Acid Resistant */
				0,	/* Fire Resistant */
				0,	/* Electric Resistant */
				1,	/* Perfect Armour */
				0,	/* Electric Sensitive */
				0,	/* Combustability */
				0,	/* Indestructable */
			},
		},
		section_sprays_acid|section_flag_left_hand,
	},
	{
		"hnpcpred_alien",
		"left thumb",
		NULL,
		{
			16,	/* Health */
			40,		/* Armour */
			0, /* IsOnFire */
			{
				1,	/* Acid Resistant */
				0,	/* Fire Resistant */
				0,	/* Electric Resistant */
				1,	/* Perfect Armour */
				0,	/* Electric Sensitive */
				0,	/* Combustability */
				0,	/* Indestructable */
			},
		},
		section_sprays_acid|section_flag_left_hand,
	},
	{
		"hnpcpred_alien",
		"left fing a",
		NULL,
		{
			16,	/* Health */
			40,		/* Armour */
			0, /* IsOnFire */
			{
				1,	/* Acid Resistant */
				0,	/* Fire Resistant */
				0,	/* Electric Resistant */
				1,	/* Perfect Armour */
				0,	/* Electric Sensitive */
				0,	/* Combustability */
				0,	/* Indestructable */
			},
		},
		section_sprays_acid|section_flag_left_hand,
	},
	{
		"hnpcpred_alien",
		"left fing b",
		NULL,
		{
			16,	/* Health */
			40,		/* Armour */
			0, /* IsOnFire */
			{
				1,	/* Acid Resistant */
				0,	/* Fire Resistant */
				0,	/* Electric Resistant */
				1,	/* Perfect Armour */
				0,	/* Electric Sensitive */
				0,	/* Combustability */
				0,	/* Indestructable */
			},
		},
		section_sprays_acid|section_flag_left_hand,
	},
	{
		"hnpcpred_alien",
		"left fing c",
		NULL,
		{
			16,	/* Health */
			40,		/* Armour */
			0, /* IsOnFire */
			{
				1,	/* Acid Resistant */
				0,	/* Fire Resistant */
				0,	/* Electric Resistant */
				1,	/* Perfect Armour */
				0,	/* Electric Sensitive */
				0,	/* Combustability */
				0,	/* Indestructable */
			},
		},
		section_sprays_acid|section_flag_left_hand,
	},
	{
		"hnpcpred_alien",
		"left arm fin",
		NULL,
		{
			20,	/* Health */
			40,		/* Armour */
			0, /* IsOnFire */
			{
				1,	/* Acid Resistant */
				0,	/* Fire Resistant */
				0,	/* Electric Resistant */
				1,	/* Perfect Armour */
				0,	/* Electric Sensitive */
				0,	/* Combustability */
				0,	/* Indestructable */
			},
		},
		section_sprays_acid
	},


	{
		"hnpcpred_alien",
		"tail base",
		NULL,
		{
			40,	/* Health */
			40,	/* Armour */
			0, 	/* IsOnFire */
			{
				1,	/* Acid Resistant */
				0,	/* Fire Resistant */
				0,	/* Electric Resistant */
				1,	/* Perfect Armour */
				0,	/* Electric Sensitive */
				0,	/* Combustability */
				0,	/* Indestructable */
			},
		},
		section_flag_nofurthergibbing|section_sprays_acid|section_flag_tail,
	},

	{
		"hnpcpred_alien",
		"tail a",
		NULL,
		{
			40,	/* Health */
			40,	/* Armour */
			0, 	/* IsOnFire */
			{
				1,	/* Acid Resistant */
				0,	/* Fire Resistant */
				0,	/* Electric Resistant */
				1,	/* Perfect Armour */
				0,	/* Electric Sensitive */
				0,	/* Combustability */
				0,	/* Indestructable */
			},
		},
		section_flag_nofurthergibbing|section_sprays_acid|section_flag_tail,
	},
	{
		"hnpcpred_alien",
		"tail b",
		NULL,
		{
			40,	/* Health */
			40,	/* Armour */
			0, 	/* IsOnFire */
			{
				1,	/* Acid Resistant */
				0,	/* Fire Resistant */
				0,	/* Electric Resistant */
				1,	/* Perfect Armour */
				0,	/* Electric Sensitive */
				0,	/* Combustability */
				0,	/* Indestructable */
			},
		},
		section_flag_nofurthergibbing|section_sprays_acid|section_flag_tail,
	},
	{
		"hnpcpred_alien",
		"tail c",
		NULL,
		{
			40,	/* Health */
			40,	/* Armour */
			0, 	/* IsOnFire */
			{
				1,	/* Acid Resistant */
				0,	/* Fire Resistant */
				0,	/* Electric Resistant */
				1,	/* Perfect Armour */
				0,	/* Electric Sensitive */
				0,	/* Combustability */
				0,	/* Indestructable */
			},
		},
		section_flag_nofurthergibbing|section_sprays_acid|section_flag_tail,
	},
	{
		"hnpcpred_alien",
		"tail d",
		NULL,
		{
			40,	/* Health */
			40,	/* Armour */
			0, 	/* IsOnFire */
			{
				1,	/* Acid Resistant */
				0,	/* Fire Resistant */
				0,	/* Electric Resistant */
				1,	/* Perfect Armour */
				0,	/* Electric Sensitive */
				0,	/* Combustability */
				0,	/* Indestructable */
			},
		},
		section_flag_nofurthergibbing|section_sprays_acid|section_flag_tail,
	},
	{
		"hnpcpred_alien",
		"tail e",
		NULL,
		{
			40,	/* Health */
			40,	/* Armour */
			0, 	/* IsOnFire */
			{
				1,	/* Acid Resistant */
				0,	/* Fire Resistant */
				0,	/* Electric Resistant */
				1,	/* Perfect Armour */
				0,	/* Electric Sensitive */
				0,	/* Combustability */
				0,	/* Indestructable */
			},
		},
		section_flag_nofurthergibbing|section_sprays_acid|section_flag_tail,
	},
	{
		"hnpcpred_alien",
		"tail f",
		NULL,
		{
			40,	/* Health */
			40,	/* Armour */
			0, 	/* IsOnFire */
			{
				1,	/* Acid Resistant */
				0,	/* Fire Resistant */
				0,	/* Electric Resistant */
				1,	/* Perfect Armour */
				0,	/* Electric Sensitive */
				0,	/* Combustability */
				0,	/* Indestructable */
			},
		},
		section_flag_nofurthergibbing|section_sprays_acid|section_flag_tail,
	},
	{
		"hnpcpred_alien",
		"tail g",
		NULL,
		{
			40,	/* Health */
			40,	/* Armour */
			0, 	/* IsOnFire */
			{
				1,	/* Acid Resistant */
				0,	/* Fire Resistant */
				0,	/* Electric Resistant */
				1,	/* Perfect Armour */
				0,	/* Electric Sensitive */
				0,	/* Combustability */
				0,	/* Indestructable */
			},
		},
		section_flag_nofurthergibbing|section_sprays_acid|section_flag_tail,
	},
	{
		"hnpcpred_alien",
		"tail h",
		NULL,
		{
			40,	/* Health */
			40,	/* Armour */
			0, 	/* IsOnFire */
			{
				1,	/* Acid Resistant */
				0,	/* Fire Resistant */
				0,	/* Electric Resistant */
				1,	/* Perfect Armour */
				0,	/* Electric Sensitive */
				0,	/* Combustability */
				0,	/* Indestructable */
			},
		},
		section_flag_nofurthergibbing|section_sprays_acid|section_flag_tail,
	},
	{
		"hnpcpred_alien",
		"tail i",
		NULL,
		{
			40,	/* Health */
			40,	/* Armour */
			0, 	/* IsOnFire */
			{
				1,	/* Acid Resistant */
				0,	/* Fire Resistant */
				0,	/* Electric Resistant */
				1,	/* Perfect Armour */
				0,	/* Electric Sensitive */
				0,	/* Combustability */
				0,	/* Indestructable */
			},
		},
		section_flag_nofurthergibbing|section_sprays_acid|section_flag_tail,
	},
	/* MARINE */
	{
		"hnpcmarine",
		"pelvis",
		NULL,
		{
			10,	/* Health */
			8,		/* Armour */
			0, /* IsOnFire */
			{
				0,	/* Acid Resistant */
				0,	/* Fire Resistant */
				0,	/* Electric Resistant */
				0,	/* Perfect Armour */
				0,	/* Electric Sensitive */
				0,	/* Combustability */
				0,	/* Indestructable */
			},
		},
		section_has_sparkoflife|section_sprays_blood|section_flag_affectedbyheat,
	},
	{
		"hnpcmarine",
		"Lthigh",
		NULL,
		{
			10,	/* Health */
			8,		/* Armour */
			0, /* IsOnFire */
			{
				0,	/* Acid Resistant */
				0,	/* Fire Resistant */
				0,	/* Electric Resistant */
				0,	/* Perfect Armour */
				0,	/* Electric Sensitive */
				0,	/* Combustability */
				0,	/* Indestructable */
			},
		},
		section_has_sparkoflife|section_sprays_blood|section_flag_left_leg|section_flag_affectedbyheat,
	},
	{
		"hnpcmarine",
		"Lshin",
		NULL,
		{
			10,	/* Health */
			8,		/* Armour */
			0, /* IsOnFire */
			{
				0,	/* Acid Resistant */
				0,	/* Fire Resistant */
				0,	/* Electric Resistant */
				0,	/* Perfect Armour */
				0,	/* Electric Sensitive */
				0,	/* Combustability */
				0,	/* Indestructable */
			},
		},
		section_has_sparkoflife|section_sprays_blood|section_flag_left_leg|section_flag_affectedbyheat,
	},
	{
		"hnpcmarine",
		"Lfoot",
		NULL,
		{
			10,	/* Health */
			8,		/* Armour */
			0, /* IsOnFire */
			{
				0,	/* Acid Resistant */
				0,	/* Fire Resistant */
				0,	/* Electric Resistant */
				0,	/* Perfect Armour */
				0,	/* Electric Sensitive */
				0,	/* Combustability */
				0,	/* Indestructable */
			},
		},
		section_has_sparkoflife|section_sprays_blood|section_flag_left_foot|section_flag_affectedbyheat,
	},
	{
		"hnpcmarine",
		"Lpouch",
		NULL,
		{
			10,	/* Health */
			4,		/* Armour */
			0, /* IsOnFire */
			{
				0,	/* Acid Resistant */
				0,	/* Fire Resistant */
				0,	/* Electric Resistant */
				0,	/* Perfect Armour */
				0,	/* Electric Sensitive */
				0,	/* Combustability */
				0,	/* Indestructable */
			},
		},
		section_flag_doesnthurtsb|section_flag_passdamagetoparent,
	},
	{
		"hnpcmarine",
		"Rthigh",
		NULL,
		{
			10,	/* Health */
			8,		/* Armour */
			0, /* IsOnFire */
			{
				0,	/* Acid Resistant */
				0,	/* Fire Resistant */
				0,	/* Electric Resistant */
				0,	/* Perfect Armour */
				0,	/* Electric Sensitive */
				0,	/* Combustability */
				0,	/* Indestructable */
			},
		},
		section_has_sparkoflife|section_sprays_blood|section_flag_right_leg|section_flag_affectedbyheat,
	},
	{
		"hnpcmarine",
		"Rshin",
		NULL,
		{
			10,	/* Health */
			8,		/* Armour */
			0, /* IsOnFire */
			{
				0,	/* Acid Resistant */
				0,	/* Fire Resistant */
				0,	/* Electric Resistant */
				0,	/* Perfect Armour */
				0,	/* Electric Sensitive */
				0,	/* Combustability */
				0,	/* Indestructable */
			},
		},
		section_has_sparkoflife|section_sprays_blood|section_flag_right_leg|section_flag_affectedbyheat,
	},
	{
		"hnpcmarine",
		"Rfoot",
		NULL,
		{
			10,	/* Health */
			8,		/* Armour */
			0, /* IsOnFire */
			{
				0,	/* Acid Resistant */
				0,	/* Fire Resistant */
				0,	/* Electric Resistant */
				0,	/* Perfect Armour */
				0,	/* Electric Sensitive */
				0,	/* Combustability */
				0,	/* Indestructable */
			},
		},
		section_has_sparkoflife|section_sprays_blood|section_flag_right_foot|section_flag_affectedbyheat,
	},
	{
		"hnpcmarine",
		"Rpouch",
		NULL,
		{
			10,	/* Health */
			4,		/* Armour */
			0, /* IsOnFire */
			{
				0,	/* Acid Resistant */
				0,	/* Fire Resistant */
				0,	/* Electric Resistant */
				0,	/* Perfect Armour */
				0,	/* Electric Sensitive */
				0,	/* Combustability */
				0,	/* Indestructable */
			},
		},
		section_flag_doesnthurtsb|section_flag_passdamagetoparent,
	},
	{
		"hnpcmarine",
		"chest",
		NULL,
		{
			10,	/* Health */
			8,		/* Armour */
			0, /* IsOnFire */
			{
				0,	/* Acid Resistant */
				0,	/* Fire Resistant */
				0,	/* Electric Resistant */
				0,	/* Perfect Armour */
				0,	/* Electric Sensitive */
				0,	/* Combustability */
				0,	/* Indestructable */
			},
		},
		section_has_sparkoflife|section_sprays_blood|section_flag_heatsource|section_flag_fragonlyfordisks,
	},
	{
		"hnpcmarine",
		"Rbicep",
		NULL,
		{
			10,	/* Health */
			8,		/* Armour */
			0, /* IsOnFire */
			{
				0,	/* Acid Resistant */
				0,	/* Fire Resistant */
				0,	/* Electric Resistant */
				0,	/* Perfect Armour */
				0,	/* Electric Sensitive */
				0,	/* Combustability */
				0,	/* Indestructable */
			},
		},
		section_has_sparkoflife|section_sprays_blood|section_flag_right_arm|section_flag_affectedbyheat,
	},
	{
		"hnpcmarine",
		"Rforearm",
		NULL,
		{
			10,	/* Health */
			8,		/* Armour */
			0, /* IsOnFire */
			{
				0,	/* Acid Resistant */
				0,	/* Fire Resistant */
				0,	/* Electric Resistant */
				0,	/* Perfect Armour */
				0,	/* Electric Sensitive */
				0,	/* Combustability */
				0,	/* Indestructable */
			},
		},
		section_has_sparkoflife|section_sprays_blood|section_flag_right_arm|section_flag_affectedbyheat,
	},
	{
		"hnpcmarine",
		"Rpalm",
		NULL,
		{
			10,	/* Health */
			8,		/* Armour */
			0, /* IsOnFire */
			{
				0,	/* Acid Resistant */
				0,	/* Fire Resistant */
				0,	/* Electric Resistant */
				0,	/* Perfect Armour */
				0,	/* Electric Sensitive */
				0,	/* Combustability */
				0,	/* Indestructable */
			},
		},
		section_has_sparkoflife|section_sprays_blood|section_flag_right_hand|section_flag_affectedbyheat,
	},
	{
		"hnpcmarine",
		"R1fingers",
		NULL,
		{
			10,	/* Health */
			8,		/* Armour */
			0, /* IsOnFire */
			{
				0,	/* Acid Resistant */
				0,	/* Fire Resistant */
				0,	/* Electric Resistant */
				0,	/* Perfect Armour */
				0,	/* Electric Sensitive */
				0,	/* Combustability */
				0,	/* Indestructable */
			},
		},
		section_sprays_blood|section_flag_right_hand|section_flag_affectedbyheat,
	},
	{
		"hnpcmarine",
		"R2fingers",
		NULL,
		{
			10,	/* Health */
			8,		/* Armour */
			0, /* IsOnFire */
			{
				0,	/* Acid Resistant */
				0,	/* Fire Resistant */
				0,	/* Electric Resistant */
				0,	/* Perfect Armour */
				0,	/* Electric Sensitive */
				0,	/* Combustability */
				0,	/* Indestructable */
			},
		},
		section_sprays_blood|section_flag_right_hand|section_flag_affectedbyheat,
	},
	{
		"hnpcmarine",
		"R1thumb",
		NULL,
		{
			10,	/* Health */
			8,		/* Armour */
			0, /* IsOnFire */
			{
				0,	/* Acid Resistant */
				0,	/* Fire Resistant */
				0,	/* Electric Resistant */
				0,	/* Perfect Armour */
				0,	/* Electric Sensitive */
				0,	/* Combustability */
				0,	/* Indestructable */
			},
		},
		section_sprays_blood|section_flag_right_hand|section_flag_affectedbyheat,
	},
	{
		"hnpcmarine",
		"R2thumb",
		NULL,
		{
			10,	/* Health */
			8,		/* Armour */
			0, /* IsOnFire */
			{
				0,	/* Acid Resistant */
				0,	/* Fire Resistant */
				0,	/* Electric Resistant */
				0,	/* Perfect Armour */
				0,	/* Electric Sensitive */
				0,	/* Combustability */
				0,	/* Indestructable */
			},
		},
		section_sprays_blood|section_flag_right_hand|section_flag_affectedbyheat,
	},
	{
		"hnpcmarine",
		"Rpad",
		NULL,
		{
			10,	/* Health */
			8,		/* Armour */
			0, /* IsOnFire */
			{
				0,	/* Acid Resistant */
				0,	/* Fire Resistant */
				0,	/* Electric Resistant */
				0,	/* Perfect Armour */
				0,	/* Electric Sensitive */
				0,	/* Combustability */
				0,	/* Indestructable */
			},
		},
		section_flag_doesnthurtsb|section_flag_passdamagetoparent,
	},
	{
		"hnpcmarine",
		"Lbicep",
		NULL,
		{
			10,	/* Health */
			8,		/* Armour */
			0, /* IsOnFire */
			{
				0,	/* Acid Resistant */
				0,	/* Fire Resistant */
				0,	/* Electric Resistant */
				0,	/* Perfect Armour */
				0,	/* Electric Sensitive */
				0,	/* Combustability */
				0,	/* Indestructable */
			},
		},
		section_has_sparkoflife|section_sprays_blood|section_flag_left_arm|section_flag_affectedbyheat,
	},
	{
		"hnpcmarine",
		"Lforearm",
		NULL,
		{
			10,	/* Health */
			8,		/* Armour */
			0, /* IsOnFire */
			{
				0,	/* Acid Resistant */
				0,	/* Fire Resistant */
				0,	/* Electric Resistant */
				0,	/* Perfect Armour */
				0,	/* Electric Sensitive */
				0,	/* Combustability */
				0,	/* Indestructable */
			},
		},
		section_has_sparkoflife|section_sprays_blood|section_flag_left_arm|section_flag_affectedbyheat,
	},
	{
		"hnpcmarine",
		"Lpalm",
		NULL,
		{
			10,	/* Health */
			8,		/* Armour */
			0, /* IsOnFire */
			{
				0,	/* Acid Resistant */
				0,	/* Fire Resistant */
				0,	/* Electric Resistant */
				0,	/* Perfect Armour */
				0,	/* Electric Sensitive */
				0,	/* Combustability */
				0,	/* Indestructable */
			},
		},
		section_has_sparkoflife|section_sprays_blood|section_flag_left_hand|section_flag_affectedbyheat,
	},
	{
		"hnpcmarine",
		"L1fingers",
		NULL,
		{
			10,	/* Health */
			8,		/* Armour */
			0, /* IsOnFire */
			{
				0,	/* Acid Resistant */
				0,	/* Fire Resistant */
				0,	/* Electric Resistant */
				0,	/* Perfect Armour */
				0,	/* Electric Sensitive */
				0,	/* Combustability */
				0,	/* Indestructable */
			},
		},
		section_sprays_blood|section_flag_left_hand|section_flag_affectedbyheat,
	},
	{
		"hnpcmarine",
		"L2fingers",
		NULL,
		{
			10,	/* Health */
			8,		/* Armour */
			0, /* IsOnFire */
			{
				0,	/* Acid Resistant */
				0,	/* Fire Resistant */
				0,	/* Electric Resistant */
				0,	/* Perfect Armour */
				0,	/* Electric Sensitive */
				0,	/* Combustability */
				0,	/* Indestructable */
			},
		},
		section_sprays_blood|section_flag_left_hand|section_flag_affectedbyheat,
	},
	{
		"hnpcmarine",
		"L1thumb",
		NULL,
		{
			10,	/* Health */
			8,		/* Armour */
			0, /* IsOnFire */
			{
				0,	/* Acid Resistant */
				0,	/* Fire Resistant */
				0,	/* Electric Resistant */
				0,	/* Perfect Armour */
				0,	/* Electric Sensitive */
				0,	/* Combustability */
				0,	/* Indestructable */
			},
		},
		section_sprays_blood|section_flag_left_hand|section_flag_affectedbyheat,
	},
	{
		"hnpcmarine",
		"L2thumb",
		NULL,
		{
			10,	/* Health */
			8,		/* Armour */
			0, /* IsOnFire */
			{
				0,	/* Acid Resistant */
				0,	/* Fire Resistant */
				0,	/* Electric Resistant */
				0,	/* Perfect Armour */
				0,	/* Electric Sensitive */
				0,	/* Combustability */
				0,	/* Indestructable */
			},
		},
		section_sprays_blood|section_flag_left_hand|section_flag_affectedbyheat,
	},
	{
		"hnpcmarine",
		"Lpad",
		NULL,
		{
			10,	/* Health */
			8,		/* Armour */
			0, /* IsOnFire */
			{
				0,	/* Acid Resistant */
				0,	/* Fire Resistant */
				0,	/* Electric Resistant */
				0,	/* Perfect Armour */
				0,	/* Electric Sensitive */
				0,	/* Combustability */
				0,	/* Indestructable */
			},
		},
		section_flag_doesnthurtsb|section_flag_passdamagetoparent,
	},
	{
		"hnpcmarine",
		"light pack",
		NULL,
		{
			5,	/* Health */
			8,		/* Armour */
			0, /* IsOnFire */
			{
				0,	/* Acid Resistant */
				0,	/* Fire Resistant */
				0,	/* Electric Resistant */
				0,	/* Perfect Armour */
				0,	/* Electric Sensitive */
				0,	/* Combustability */
				0,	/* Indestructable */
			},
		},
		//section_sprays_sparks|section_flag_doesnthurtsb,
		section_flag_doesnthurtsb|section_flag_passdamagetoparent,
	},
	{
		"hnpcmarine",
		"light ",
		NULL,
		{
			5,	/* Health */
			8,		/* Armour */
			0, /* IsOnFire */
			{
				0,	/* Acid Resistant */
				0,	/* Fire Resistant */
				0,	/* Electric Resistant */
				0,	/* Perfect Armour */
				0,	/* Electric Sensitive */
				0,	/* Combustability */
				0,	/* Indestructable */
			},
		},
		//section_sprays_sparks|section_flag_doesnthurtsb,
		section_flag_doesnthurtsb|section_flag_passdamagetoparent,
	},
	{
		"hnpcmarine",
		"neck",
		NULL,
		{
			10,	/* Health */
			8,		/* Armour */
			0, /* IsOnFire */
			{
				0,	/* Acid Resistant */
				0,	/* Fire Resistant */
				0,	/* Electric Resistant */
				0,	/* Perfect Armour */
				0,	/* Electric Sensitive */
				0,	/* Combustability */
				0,	/* Indestructable */
			},
		},
		section_has_sparkoflife|section_sprays_blood|section_flag_affectedbyheat,
	},
	{
		"hnpcmarine",
		"head",
		NULL,
		{
			10,	/* Health */
			8,		/* Armour */
			0, /* IsOnFire */
			{
				0,	/* Acid Resistant */
				0,	/* Fire Resistant */
				0,	/* Electric Resistant */
				0,	/* Perfect Armour */
				0,	/* Electric Sensitive */
				0,	/* Combustability */
				0,	/* Indestructable */
			},
		},
		section_flag_head|section_has_sparkoflife|section_sprays_blood|section_flag_heatsource,
	},
	{
		"hnpcmarine",
		"neck guard",
		NULL,
		{
			10,		/* Health */
			8,		/* Armour */
			0, /* IsOnFire */
			{
				0,	/* Acid Resistant */
				0,	/* Fire Resistant */
				0,	/* Electric Resistant */
				0,	/* Perfect Armour */
				0,	/* Electric Sensitive */
				0,	/* Combustability */
				0,	/* Indestructable */
			},
		},
		section_flag_doesnthurtsb|section_flag_passdamagetoparent,
	},
	{
		"hnpcmarine",
		"helm",
		NULL,
		{
			10,		/* Health */
			8,		/* Armour */
			0, /* IsOnFire */
			{
				0,	/* Acid Resistant */
				0,	/* Fire Resistant */
				0,	/* Electric Resistant */
				0,	/* Perfect Armour */
				0,	/* Electric Sensitive */
				0,	/* Combustability */
				0,	/* Indestructable */
			},
		},
		section_flag_doesnthurtsb|section_flag_passdamagetoparent,
	},
	/* Predator */
	/* CDF 9/3/99 Removed heat source flags from chest and head. */
	#define PRED_SECTION_HEALTH	(100)
	#define PRED_SECTION_ARMOUR	(200)
	{
		"hnpcpredator",
		"Pelvis",
		NULL,
		{
			PRED_SECTION_HEALTH,	/* Health */
			PRED_SECTION_ARMOUR,	/* Armour */
			0, /* IsOnFire */
			{
				0,	/* Acid Resistant */
				0,	/* Fire Resistant */
				0,	/* Electric Resistant */
				0,	/* Perfect Armour */
				0,	/* Electric Sensitive */
				0,	/* Combustability */
				0,	/* Indestructable */
			},
		},
		section_has_sparkoflife|section_sprays_predoblood|section_flag_affectedbyheat,
	},
	{
		"hnpcpredator",
		"chest",
		NULL,
		{
			((PRED_SECTION_HEALTH)<<1),	/* Health */
			PRED_SECTION_ARMOUR,	/* Armour */
			0, /* IsOnFire */
			{
				0,	/* Acid Resistant */
				0,	/* Fire Resistant */
				0,	/* Electric Resistant */
				0,	/* Perfect Armour */
				0,	/* Electric Sensitive */
				0,	/* Combustability */
				0,	/* Indestructable */
			},
		},
		section_has_sparkoflife|section_sprays_predoblood,
	},
	{
		"hnpcpredator",
		"neck",
		NULL,
		{
			PRED_SECTION_HEALTH,	/* Health */
			PRED_SECTION_ARMOUR,	/* Armour */
			0, /* IsOnFire */
			{
				0,	/* Acid Resistant */
				0,	/* Fire Resistant */
				0,	/* Electric Resistant */
				0,	/* Perfect Armour */
				0,	/* Electric Sensitive */
				0,	/* Combustability */
				0,	/* Indestructable */
			},
		},
		section_has_sparkoflife|section_sprays_predoblood|section_flag_affectedbyheat,
	},
	{
		"hnpcpredator",
		"head",
		NULL,
		{
			PRED_SECTION_HEALTH,	/* Health */
			PRED_SECTION_ARMOUR,	/* Armour */
			0, /* IsOnFire */
			{
				0,	/* Acid Resistant */
				0,	/* Fire Resistant */
				0,	/* Electric Resistant */
				0,	/* Perfect Armour */
				0,	/* Electric Sensitive */
				0,	/* Combustability */
				0,	/* Indestructable */
			},
		},
		section_flag_head|section_has_sparkoflife|section_sprays_predoblood,
	},
	{
		"hnpcpredator",
		"L shoulder",
		NULL,
		{
			PRED_SECTION_HEALTH,	/* Health */
			PRED_SECTION_ARMOUR,	/* Armour */
			0, /* IsOnFire */
			{
				0,	/* Acid Resistant */
				0,	/* Fire Resistant */
				0,	/* Electric Resistant */
				0,	/* Perfect Armour */
				0,	/* Electric Sensitive */
				0,	/* Combustability */
				0,	/* Indestructable */
			},
		},
		section_has_sparkoflife|section_sprays_predoblood|section_flag_left_arm|section_flag_affectedbyheat,
	},
	{
		"hnpcpredator",
		"L bicep",
		NULL,
		{
			PRED_SECTION_HEALTH,	/* Health */
			PRED_SECTION_ARMOUR,	/* Armour */
			0, /* IsOnFire */
			{
				0,	/* Acid Resistant */
				0,	/* Fire Resistant */
				0,	/* Electric Resistant */
				0,	/* Perfect Armour */
				0,	/* Electric Sensitive */
				0,	/* Combustability */
				0,	/* Indestructable */
			},
		},
		section_has_sparkoflife|section_sprays_predoblood|section_flag_left_arm|section_flag_affectedbyheat,
	},
	{
		"hnpcpredator",
		"L elbow",
		NULL,
		{
			PRED_SECTION_HEALTH,	/* Health */
			PRED_SECTION_ARMOUR,	/* Armour */
			0, /* IsOnFire */
			{
				0,	/* Acid Resistant */
				0,	/* Fire Resistant */
				0,	/* Electric Resistant */
				0,	/* Perfect Armour */
				0,	/* Electric Sensitive */
				0,	/* Combustability */
				0,	/* Indestructable */
			},
		},
		section_has_sparkoflife|section_sprays_predoblood|section_flag_left_arm|section_flag_affectedbyheat,
	},
	{
		"hnpcpredator",
		"L forearm",
		NULL,
		{
			PRED_SECTION_HEALTH,	/* Health */
			PRED_SECTION_ARMOUR, 	/* Armour */
			0, /* IsOnFire */
			{
				0,	/* Acid Resistant */
				0,	/* Fire Resistant */
				0,	/* Electric Resistant */
				0,	/* Perfect Armour */
				0,	/* Electric Sensitive */
				0,	/* Combustability */
				0,	/* Indestructable */
			},
		},
		section_has_sparkoflife|section_sprays_predoblood|section_flag_left_arm|section_flag_affectedbyheat,
	},
	{
		"hnpcpredator",
		"L hand palm",
		NULL,
		{
			PRED_SECTION_HEALTH,	/* Health */
			PRED_SECTION_ARMOUR,	/* Armour */
			0, /* IsOnFire */
			{
				0,	/* Acid Resistant */
				0,	/* Fire Resistant */
				0,	/* Electric Resistant */
				0,	/* Perfect Armour */
				0,	/* Electric Sensitive */
				0,	/* Combustability */
				0,	/* Indestructable */
			},
		},
		section_has_sparkoflife|section_sprays_predoblood|section_flag_left_hand|section_flag_affectedbyheat,
	},
	{
		"hnpcpredator",
		"L hand thumb a",
		NULL,
		{
			PRED_SECTION_HEALTH,	/* Health */
			PRED_SECTION_ARMOUR, 	/* Armour */
			0, /* IsOnFire */
			{
				0,	/* Acid Resistant */
				0,	/* Fire Resistant */
				0,	/* Electric Resistant */
				0,	/* Perfect Armour */
				0,	/* Electric Sensitive */
				0,	/* Combustability */
				0,	/* Indestructable */
			},
		},
		section_has_sparkoflife|section_sprays_predoblood|section_flag_left_hand|section_flag_affectedbyheat,
	},
	{
		"hnpcpredator",
		"L hand thumb b",
		NULL,
		{
			PRED_SECTION_HEALTH,	/* Health */
			PRED_SECTION_ARMOUR,	/* Armour */
			0, /* IsOnFire */
			{
				0,	/* Acid Resistant */
				0,	/* Fire Resistant */
				0,	/* Electric Resistant */
				0,	/* Perfect Armour */
				0,	/* Electric Sensitive */
				0,	/* Combustability */
				0,	/* Indestructable */
			},
		},
		section_has_sparkoflife|section_sprays_predoblood|section_flag_left_hand|section_flag_affectedbyheat,
	},
	{
		"hnpcpredator",
		"L hand finger 1a",
		NULL,
		{
			PRED_SECTION_HEALTH,	/* Health */
			PRED_SECTION_ARMOUR, 	/* Armour */
			0, /* IsOnFire */
			{
				0,	/* Acid Resistant */
				0,	/* Fire Resistant */
				0,	/* Electric Resistant */
				0,	/* Perfect Armour */
				0,	/* Electric Sensitive */
				0,	/* Combustability */
				0,	/* Indestructable */
			},
		},
		section_has_sparkoflife|section_sprays_predoblood|section_flag_left_hand|section_flag_affectedbyheat,
	},
	{
		"hnpcpredator",
		"L hand finger 1b",
		NULL,
		{
			PRED_SECTION_HEALTH,	/* Health */
			PRED_SECTION_ARMOUR,	/* Armour */
			0, /* IsOnFire */
			{
				0,	/* Acid Resistant */
				0,	/* Fire Resistant */
				0,	/* Electric Resistant */
				0,	/* Perfect Armour */
				0,	/* Electric Sensitive */
				0,	/* Combustability */
				0,	/* Indestructable */
			},
		},
		section_has_sparkoflife|section_sprays_predoblood|section_flag_left_hand|section_flag_affectedbyheat,
	},
	{
		"hnpcpredator",
		"L hand finger 2a",
		NULL,
		{
			PRED_SECTION_HEALTH,	/* Health */
			PRED_SECTION_ARMOUR,	/* Armour */
			0, /* IsOnFire */
			{
				0,	/* Acid Resistant */
				0,	/* Fire Resistant */
				0,	/* Electric Resistant */
				0,	/* Perfect Armour */
				0,	/* Electric Sensitive */
				0,	/* Combustability */
				0,	/* Indestructable */
			},
		},
		section_has_sparkoflife|section_sprays_predoblood|section_flag_left_hand|section_flag_affectedbyheat,
	},
	{
		"hnpcpredator",
		"L hand finger 2b",
		NULL,
		{
			PRED_SECTION_HEALTH,	/* Health */
			PRED_SECTION_ARMOUR,	/* Armour */
			0, /* IsOnFire */
			{
				0,	/* Acid Resistant */
				0,	/* Fire Resistant */
				0,	/* Electric Resistant */
				0,	/* Perfect Armour */
				0,	/* Electric Sensitive */
				0,	/* Combustability */
				0,	/* Indestructable */
			},
		},
		section_has_sparkoflife|section_sprays_predoblood|section_flag_left_hand|section_flag_affectedbyheat,
	},
	{
		"hnpcpredator",
		"L hand finger 3a",
		NULL,
		{
			PRED_SECTION_HEALTH,	/* Health */
			PRED_SECTION_ARMOUR, 	/* Armour */
			0, /* IsOnFire */
			{
				0,	/* Acid Resistant */
				0,	/* Fire Resistant */
				0,	/* Electric Resistant */
				0,	/* Perfect Armour */
				0,	/* Electric Sensitive */
				0,	/* Combustability */
				0,	/* Indestructable */
			},
		},
		section_has_sparkoflife|section_sprays_predoblood|section_flag_left_hand|section_flag_affectedbyheat,
	},
	{
		"hnpcpredator",
		"L hand finger 3b",
		NULL,
		{
			PRED_SECTION_HEALTH,	/* Health */
			PRED_SECTION_ARMOUR, 	/* Armour */
			0, /* IsOnFire */
			{
				0,	/* Acid Resistant */
				0,	/* Fire Resistant */
				0,	/* Electric Resistant */
				0,	/* Perfect Armour */
				0,	/* Electric Sensitive */
				0,	/* Combustability */
				0,	/* Indestructable */
			},
		},
		section_has_sparkoflife|section_sprays_predoblood|section_flag_left_hand|section_flag_affectedbyheat,
	},
	{
		"hnpcpredator",
		"L hand finger 4a",
		NULL,
		{
			PRED_SECTION_HEALTH,	/* Health */
			PRED_SECTION_ARMOUR, 	/* Armour */
			0, /* IsOnFire */
			{
				0,	/* Acid Resistant */
				0,	/* Fire Resistant */
				0,	/* Electric Resistant */
				0,	/* Perfect Armour */
				0,	/* Electric Sensitive */
				0,	/* Combustability */
				0,	/* Indestructable */
			},
		},
		section_has_sparkoflife|section_sprays_predoblood|section_flag_left_hand|section_flag_affectedbyheat,
	},
	{
		"hnpcpredator",
		"L hand finger 4b",
		NULL,
		{
			PRED_SECTION_HEALTH,	/* Health */
			PRED_SECTION_ARMOUR, 	/* Armour */
			0, /* IsOnFire */
			{
				0,	/* Acid Resistant */
				0,	/* Fire Resistant */
				0,	/* Electric Resistant */
				0,	/* Perfect Armour */
				0,	/* Electric Sensitive */
				0,	/* Combustability */
				0,	/* Indestructable */
			},
		},
		section_has_sparkoflife|section_sprays_predoblood|section_flag_left_hand|section_flag_affectedbyheat,
	},
	{
		"hnpcpredator",
		"R shoulder",
		NULL,
		{
			PRED_SECTION_HEALTH,	/* Health */
			PRED_SECTION_ARMOUR, 	/* Armour */
			0, /* IsOnFire */
			{
				0,	/* Acid Resistant */
				0,	/* Fire Resistant */
				0,	/* Electric Resistant */
				0,	/* Perfect Armour */
				0,	/* Electric Sensitive */
				0,	/* Combustability */
				0,	/* Indestructable */
			},
		},
		section_has_sparkoflife|section_sprays_predoblood|section_flag_right_arm|section_flag_affectedbyheat,
	},
	{
		"hnpcpredator",
		"R bicep",
		NULL,
		{
			PRED_SECTION_HEALTH,	/* Health */
			PRED_SECTION_ARMOUR, 	/* Armour */
			0, /* IsOnFire */
			{
				0,	/* Acid Resistant */
				0,	/* Fire Resistant */
				0,	/* Electric Resistant */
				0,	/* Perfect Armour */
				0,	/* Electric Sensitive */
				0,	/* Combustability */
				0,	/* Indestructable */
			},
		},
		section_has_sparkoflife|section_sprays_predoblood|section_flag_right_arm|section_flag_affectedbyheat,
	},
	{
		"hnpcpredator",
		"R elbow",
		NULL,
		{
			PRED_SECTION_HEALTH,	/* Health */
			PRED_SECTION_ARMOUR, 	/* Armour */
			0, /* IsOnFire */
			{
				0,	/* Acid Resistant */
				0,	/* Fire Resistant */
				0,	/* Electric Resistant */
				0,	/* Perfect Armour */
				0,	/* Electric Sensitive */
				0,	/* Combustability */
				0,	/* Indestructable */
			},
		},
		section_has_sparkoflife|section_sprays_predoblood|section_flag_right_arm|section_flag_affectedbyheat,
	},
	{
		"hnpcpredator",
		"R forearm",
		NULL,
		{
			PRED_SECTION_HEALTH,	/* Health */
			PRED_SECTION_ARMOUR, 	/* Armour */
			0, /* IsOnFire */
			{
				0,	/* Acid Resistant */
				0,	/* Fire Resistant */
				0,	/* Electric Resistant */
				0,	/* Perfect Armour */
				0,	/* Electric Sensitive */
				0,	/* Combustability */
				0,	/* Indestructable */
			},
		},
		section_has_sparkoflife|section_sprays_predoblood|section_flag_right_arm|section_flag_affectedbyheat,
	},
	{
		"hnpcpredator",
		"R hand palm",
		NULL,
		{
			PRED_SECTION_HEALTH,	/* Health */
			PRED_SECTION_ARMOUR, 	/* Armour */
			0, /* IsOnFire */
			{
				0,	/* Acid Resistant */
				0,	/* Fire Resistant */
				0,	/* Electric Resistant */
				0,	/* Perfect Armour */
				0,	/* Electric Sensitive */
				0,	/* Combustability */
				0,	/* Indestructable */
			},
		},
		section_has_sparkoflife|section_sprays_predoblood|section_flag_right_hand|section_flag_affectedbyheat,
	},
	{
		"hnpcpredator",
		"R hand thumb a",
		NULL,
		{
			PRED_SECTION_HEALTH,	/* Health */
			PRED_SECTION_ARMOUR, 	/* Armour */
			0, /* IsOnFire */
			{
				0,	/* Acid Resistant */
				0,	/* Fire Resistant */
				0,	/* Electric Resistant */
				0,	/* Perfect Armour */
				0,	/* Electric Sensitive */
				0,	/* Combustability */
				0,	/* Indestructable */
			},
		},
		section_has_sparkoflife|section_sprays_predoblood|section_flag_right_hand|section_flag_affectedbyheat,
	},
	{
		"hnpcpredator",
		"R hand thumb b",
		NULL,
		{
			PRED_SECTION_HEALTH,	/* Health */
			PRED_SECTION_ARMOUR, 	/* Armour */
			0, /* IsOnFire */
			{
				0,	/* Acid Resistant */
				0,	/* Fire Resistant */
				0,	/* Electric Resistant */
				0,	/* Perfect Armour */
				0,	/* Electric Sensitive */
				0,	/* Combustability */
				0,	/* Indestructable */
			},
		},
		section_has_sparkoflife|section_sprays_predoblood|section_flag_right_hand|section_flag_affectedbyheat,
	},
	{
		"hnpcpredator",
		"R hand finger 1a",
		NULL,
		{
			PRED_SECTION_HEALTH,	/* Health */
			PRED_SECTION_ARMOUR, 	/* Armour */
			0, /* IsOnFire */
			{
				0,	/* Acid Resistant */
				0,	/* Fire Resistant */
				0,	/* Electric Resistant */
				0,	/* Perfect Armour */
				0,	/* Electric Sensitive */
				0,	/* Combustability */
				0,	/* Indestructable */
			},
		},
		section_has_sparkoflife|section_sprays_predoblood|section_flag_right_hand|section_flag_affectedbyheat,
	},
	{
		"hnpcpredator",
		"R hand finger 1b",
		NULL,
		{
			PRED_SECTION_HEALTH,	/* Health */
			PRED_SECTION_ARMOUR, 	/* Armour */
			0, /* IsOnFire */
			{
				0,	/* Acid Resistant */
				0,	/* Fire Resistant */
				0,	/* Electric Resistant */
				0,	/* Perfect Armour */
				0,	/* Electric Sensitive */
				0,	/* Combustability */
				0,	/* Indestructable */
			},
		},
		section_has_sparkoflife|section_sprays_predoblood|section_flag_right_hand|section_flag_affectedbyheat,
	},
	{
		"hnpcpredator",
		"R hand finger 2a",
		NULL,
		{
			PRED_SECTION_HEALTH,	/* Health */
			PRED_SECTION_ARMOUR, 	/* Armour */
			0, /* IsOnFire */
			{
				0,	/* Acid Resistant */
				0,	/* Fire Resistant */
				0,	/* Electric Resistant */
				0,	/* Perfect Armour */
				0,	/* Electric Sensitive */
				0,	/* Combustability */
				0,	/* Indestructable */
			},
		},
		section_has_sparkoflife|section_sprays_predoblood|section_flag_right_hand|section_flag_affectedbyheat,
	},
	{
		"hnpcpredator",
		"R hand finger 2b",
		NULL,
		{
			PRED_SECTION_HEALTH,	/* Health */
			PRED_SECTION_ARMOUR, 	/* Armour */
			0, /* IsOnFire */
			{
				0,	/* Acid Resistant */
				0,	/* Fire Resistant */
				0,	/* Electric Resistant */
				0,	/* Perfect Armour */
				0,	/* Electric Sensitive */
				0,	/* Combustability */
				0,	/* Indestructable */
			},
		},
		section_has_sparkoflife|section_sprays_predoblood|section_flag_right_hand|section_flag_affectedbyheat,
	},
	{
		"hnpcpredator",
		"R hand finger 3a",
		NULL,
		{
			PRED_SECTION_HEALTH,	/* Health */
			PRED_SECTION_ARMOUR, 	/* Armour */
			0, /* IsOnFire */
			{
				0,	/* Acid Resistant */
				0,	/* Fire Resistant */
				0,	/* Electric Resistant */
				0,	/* Perfect Armour */
				0,	/* Electric Sensitive */
				0,	/* Combustability */
				0,	/* Indestructable */
			},
		},
		section_has_sparkoflife|section_sprays_predoblood|section_flag_right_hand|section_flag_affectedbyheat,
	},
	{
		"hnpcpredator",
		"R hand finger 3b",
		NULL,
		{
			PRED_SECTION_HEALTH,	/* Health */
			PRED_SECTION_ARMOUR, 	/* Armour */
			0, /* IsOnFire */
			{
				0,	/* Acid Resistant */
				0,	/* Fire Resistant */
				0,	/* Electric Resistant */
				0,	/* Perfect Armour */
				0,	/* Electric Sensitive */
				0,	/* Combustability */
				0,	/* Indestructable */
			},
		},
		section_has_sparkoflife|section_sprays_predoblood|section_flag_right_hand|section_flag_affectedbyheat,
	},
	{
		"hnpcpredator",
		"R hand finger 4a",
		NULL,
		{
			PRED_SECTION_HEALTH,	/* Health */
			PRED_SECTION_ARMOUR, 	/* Armour */
			0, /* IsOnFire */
			{
				0,	/* Acid Resistant */
				0,	/* Fire Resistant */
				0,	/* Electric Resistant */
				0,	/* Perfect Armour */
				0,	/* Electric Sensitive */
				0,	/* Combustability */
				0,	/* Indestructable */
			},
		},
		section_has_sparkoflife|section_sprays_predoblood|section_flag_right_hand|section_flag_affectedbyheat,
	},
	{
		"hnpcpredator",
		"R hand finger 4b",
		NULL,
		{
			PRED_SECTION_HEALTH,	/* Health */
			PRED_SECTION_ARMOUR, 	/* Armour */
			0, /* IsOnFire */
			{
				0,	/* Acid Resistant */
				0,	/* Fire Resistant */
				0,	/* Electric Resistant */
				0,	/* Perfect Armour */
				0,	/* Electric Sensitive */
				0,	/* Combustability */
				0,	/* Indestructable */
			},
		},
		section_has_sparkoflife|section_sprays_predoblood|section_flag_right_hand|section_flag_affectedbyheat,
	},
	{
		"hnpcpredator",
		"R thi",
		NULL,
		{
			PRED_SECTION_HEALTH,	/* Health */
			PRED_SECTION_ARMOUR, 	/* Armour */
			0, /* IsOnFire */
			{
				0,	/* Acid Resistant */
				0,	/* Fire Resistant */
				0,	/* Electric Resistant */
				0,	/* Perfect Armour */
				0,	/* Electric Sensitive */
				0,	/* Combustability */
				0,	/* Indestructable */
			},
		},
		section_has_sparkoflife|section_sprays_predoblood|section_flag_right_leg|section_flag_affectedbyheat,
	},
	{
		"hnpcpredator",
		"R shin",
		NULL,
		{
			PRED_SECTION_HEALTH,	/* Health */
			PRED_SECTION_ARMOUR, 	/* Armour */
			0, /* IsOnFire */
			{
				0,	/* Acid Resistant */
				0,	/* Fire Resistant */
				0,	/* Electric Resistant */
				0,	/* Perfect Armour */
				0,	/* Electric Sensitive */
				0,	/* Combustability */
				0,	/* Indestructable */
			},
		},
		section_has_sparkoflife|section_sprays_predoblood|section_flag_right_leg|section_flag_affectedbyheat,
	},
	{
		"hnpcpredator",
		"R foot",
		NULL,
		{
			PRED_SECTION_HEALTH,	/* Health */
			PRED_SECTION_ARMOUR, 	/* Armour */
			0, /* IsOnFire */
			{
				0,	/* Acid Resistant */
				0,	/* Fire Resistant */
				0,	/* Electric Resistant */
				0,	/* Perfect Armour */
				0,	/* Electric Sensitive */
				0,	/* Combustability */
				0,	/* Indestructable */
			},
		},
		section_has_sparkoflife|section_sprays_predoblood|section_flag_right_foot|section_flag_affectedbyheat,
	},
	{
		"hnpcpredator",
		"L thi",
		NULL,
		{
			PRED_SECTION_HEALTH,	/* Health */
			PRED_SECTION_ARMOUR, 	/* Armour */
			0, /* IsOnFire */
			{
				0,	/* Acid Resistant */
				0,	/* Fire Resistant */
				0,	/* Electric Resistant */
				0,	/* Perfect Armour */
				0,	/* Electric Sensitive */
				0,	/* Combustability */
				0,	/* Indestructable */
			},
		},
		section_has_sparkoflife|section_sprays_predoblood|section_flag_left_leg|section_flag_affectedbyheat,
	},
	{
		"hnpcpredator",
		"L shin",
		NULL,
		{
			PRED_SECTION_HEALTH,	/* Health */
			PRED_SECTION_ARMOUR, 	/* Armour */
			0, /* IsOnFire */
			{
				0,	/* Acid Resistant */
				0,	/* Fire Resistant */
				0,	/* Electric Resistant */
				0,	/* Perfect Armour */
				0,	/* Electric Sensitive */
				0,	/* Combustability */
				0,	/* Indestructable */
			},
		},
		section_has_sparkoflife|section_sprays_predoblood|section_flag_left_leg|section_flag_affectedbyheat,
	},
	{
		"hnpcpredator",
		"L foot",
		NULL,
		{
			PRED_SECTION_HEALTH,	/* Health */
			PRED_SECTION_ARMOUR, 	/* Armour */
			0, /* IsOnFire */
			{
				0,	/* Acid Resistant */
				0,	/* Fire Resistant */
				0,	/* Electric Resistant */
				0,	/* Perfect Armour */
				0,	/* Electric Sensitive */
				0,	/* Combustability */
				0,	/* Indestructable */
			},
		},
		section_has_sparkoflife|section_sprays_predoblood|section_flag_left_foot|section_flag_affectedbyheat,
	},
	{
		"hnpcpredator",
		"staff center retracted",
		NULL,
		{
			1000,	/* Health */
			2000, 	/* Armour */
			0, /* IsOnFire */
			{
				0,	/* Acid Resistant */
				0,	/* Fire Resistant */
				0,	/* Electric Resistant */
				0,	/* Perfect Armour */
				0,	/* Electric Sensitive */
				0,	/* Combustability */
				0,	/* Indestructable */
			},
		},
		section_flag_doesnthurtsb,
	},
	/* CIVILIANS */

	{
		"hnpc_civvie",
		"pelvis presley",
		NULL,
		{
			8,	/* Health */
			1,		/* Armour */
			0, /* IsOnFire */
			{
				0,	/* Acid Resistant */
				0,	/* Fire Resistant */
				0,	/* Electric Resistant */
				0,	/* Perfect Armour */
				0,	/* Electric Sensitive */
				0,	/* Combustability */
				0,	/* Indestructable */
			},
		},
		section_has_sparkoflife|section_sprays_blood|section_flag_affectedbyheat,
	},

	{
		"hnpc_civvie",
		"male left thigh",
		NULL,
		{
			8,	/* Health */
			1,		/* Armour */
			0, /* IsOnFire */
			{
				0,	/* Acid Resistant */
				0,	/* Fire Resistant */
				0,	/* Electric Resistant */
				0,	/* Perfect Armour */
				0,	/* Electric Sensitive */
				0,	/* Combustability */
				0,	/* Indestructable */
			},
		},
		section_has_sparkoflife|section_sprays_blood|section_flag_left_leg|section_flag_affectedbyheat,
	},
	{
		"hnpc_civvie",
		"male left shin",
		NULL,
		{
			8,	/* Health */
			1,		/* Armour */
			0, /* IsOnFire */
			{
				0,	/* Acid Resistant */
				0,	/* Fire Resistant */
				0,	/* Electric Resistant */
				0,	/* Perfect Armour */
				0,	/* Electric Sensitive */
				0,	/* Combustability */
				0,	/* Indestructable */
			},
		},
		section_has_sparkoflife|section_sprays_blood|section_flag_left_leg|section_flag_affectedbyheat,
	},
	{
		"hnpc_civvie",
		"male left foot",
		NULL,
		{
			8,	/* Health */
			1,		/* Armour */
			0, /* IsOnFire */
			{
				0,	/* Acid Resistant */
				0,	/* Fire Resistant */
				0,	/* Electric Resistant */
				0,	/* Perfect Armour */
				0,	/* Electric Sensitive */
				0,	/* Combustability */
				0,	/* Indestructable */
			},
		},
		section_has_sparkoflife|section_sprays_blood|section_flag_left_foot|section_flag_affectedbyheat,
	},
	{
		"hnpc_civvie",
		"male right thigh",
		NULL,
		{
			8,	/* Health */
			1,		/* Armour */
			0, /* IsOnFire */
			{
				0,	/* Acid Resistant */
				0,	/* Fire Resistant */
				0,	/* Electric Resistant */
				0,	/* Perfect Armour */
				0,	/* Electric Sensitive */
				0,	/* Combustability */
				0,	/* Indestructable */
			},
		},
		section_has_sparkoflife|section_sprays_blood|section_flag_right_leg|section_flag_affectedbyheat,
	},
	{
		"hnpc_civvie",
		"male right shin",
		NULL,
		{
			8,	/* Health */
			1,		/* Armour */
			0, /* IsOnFire */
			{
				0,	/* Acid Resistant */
				0,	/* Fire Resistant */
				0,	/* Electric Resistant */
				0,	/* Perfect Armour */
				0,	/* Electric Sensitive */
				0,	/* Combustability */
				0,	/* Indestructable */
			},
		},
		section_has_sparkoflife|section_sprays_blood|section_flag_right_leg|section_flag_affectedbyheat,
	},
	{
		"hnpc_civvie",
		"male right foot",
		NULL,
		{
			8,	/* Health */
			1,		/* Armour */
			0, /* IsOnFire */
			{
				0,	/* Acid Resistant */
				0,	/* Fire Resistant */
				0,	/* Electric Resistant */
				0,	/* Perfect Armour */
				0,	/* Electric Sensitive */
				0,	/* Combustability */
				0,	/* Indestructable */
			},
		},
		section_has_sparkoflife|section_sprays_blood|section_flag_right_foot|section_flag_affectedbyheat,
	},
	{
		"hnpc_civvie",
		"stomach",
		NULL,
		{
			8,	/* Health */
			1,		/* Armour */
			0, /* IsOnFire */
			{
				0,	/* Acid Resistant */
				0,	/* Fire Resistant */
				0,	/* Electric Resistant */
				0,	/* Perfect Armour */
				0,	/* Electric Sensitive */
				0,	/* Combustability */
				0,	/* Indestructable */
			},
		},
		section_has_sparkoflife|section_sprays_blood|section_flag_affectedbyheat|section_flag_fragonlyfordisks,
	},
	{
		"hnpc_civvie",
		"chest",
		NULL,
		{
			8,	/* Health */
			1,		/* Armour */
			0, /* IsOnFire */
			{
				0,	/* Acid Resistant */
				0,	/* Fire Resistant */
				0,	/* Electric Resistant */
				0,	/* Perfect Armour */
				0,	/* Electric Sensitive */
				0,	/* Combustability */
				0,	/* Indestructable */
			},
		},
		section_has_sparkoflife|section_sprays_blood|section_flag_heatsource|section_flag_fragonlyfordisks,
	},
	{
		"hnpc_civvie",
		"neck",
		NULL,
		{
			8,	/* Health */
			1,		/* Armour */
			0, /* IsOnFire */
			{
				0,	/* Acid Resistant */
				0,	/* Fire Resistant */
				0,	/* Electric Resistant */
				0,	/* Perfect Armour */
				0,	/* Electric Sensitive */
				0,	/* Combustability */
				0,	/* Indestructable */
			},
		},
		section_has_sparkoflife|section_sprays_blood|section_flag_affectedbyheat,
	},
	{
		"hnpc_civvie",
		"head",
		NULL,
		{
			8,	/* Health */
			1,		/* Armour */
			0, /* IsOnFire */
			{
				0,	/* Acid Resistant */
				0,	/* Fire Resistant */
				0,	/* Electric Resistant */
				0,	/* Perfect Armour */
				0,	/* Electric Sensitive */
				0,	/* Combustability */
				0,	/* Indestructable */
			},
		},
		section_has_sparkoflife|section_sprays_blood|section_flag_heatsource,
	},
	{
		"hnpc_civvie",
		"male right bicep",
		NULL,
		{
			8,	/* Health */
			1,		/* Armour */
			0, /* IsOnFire */
			{
				0,	/* Acid Resistant */
				0,	/* Fire Resistant */
				0,	/* Electric Resistant */
				0,	/* Perfect Armour */
				0,	/* Electric Sensitive */
				0,	/* Combustability */
				0,	/* Indestructable */
			},
		},
		section_has_sparkoflife|section_sprays_blood|section_flag_right_arm|section_flag_affectedbyheat,
	},
	{
		"hnpc_civvie",
		"male right forearm",
		NULL,
		{
			8,	/* Health */
			1,		/* Armour */
			0, /* IsOnFire */
			{
				0,	/* Acid Resistant */
				0,	/* Fire Resistant */
				0,	/* Electric Resistant */
				0,	/* Perfect Armour */
				0,	/* Electric Sensitive */
				0,	/* Combustability */
				0,	/* Indestructable */
			},
		},
		section_has_sparkoflife|section_sprays_blood|section_flag_right_arm|section_flag_affectedbyheat,
	},
	{
		"hnpc_civvie",
		"right palm",
		NULL,
		{
			8,	/* Health */
			1,		/* Armour */
			0, /* IsOnFire */
			{
				0,	/* Acid Resistant */
				0,	/* Fire Resistant */
				0,	/* Electric Resistant */
				0,	/* Perfect Armour */
				0,	/* Electric Sensitive */
				0,	/* Combustability */
				0,	/* Indestructable */
			},
		},
		section_has_sparkoflife|section_sprays_blood|section_flag_right_hand|section_flag_affectedbyheat,
	},
	{
		"hnpc_civvie",
		"right fings top",
		NULL,
		{
			8,	/* Health */
			1,		/* Armour */
			0, /* IsOnFire */
			{
				0,	/* Acid Resistant */
				0,	/* Fire Resistant */
				0,	/* Electric Resistant */
				0,	/* Perfect Armour */
				0,	/* Electric Sensitive */
				0,	/* Combustability */
				0,	/* Indestructable */
			},
		},
		section_has_sparkoflife|section_sprays_blood|section_flag_right_hand|section_flag_affectedbyheat,
	},
	{
		"hnpc_civvie",
		"right fings end",
		NULL,
		{
			8,	/* Health */
			1,		/* Armour */
			0, /* IsOnFire */
			{
				0,	/* Acid Resistant */
				0,	/* Fire Resistant */
				0,	/* Electric Resistant */
				0,	/* Perfect Armour */
				0,	/* Electric Sensitive */
				0,	/* Combustability */
				0,	/* Indestructable */
			},
		},
		section_has_sparkoflife|section_sprays_blood|section_flag_right_hand|section_flag_affectedbyheat,
	},
	{
		"hnpc_civvie",
		"male right thumb",
		NULL,
		{
			8,	/* Health */
			1,		/* Armour */
			0, /* IsOnFire */
			{
				0,	/* Acid Resistant */
				0,	/* Fire Resistant */
				0,	/* Electric Resistant */
				0,	/* Perfect Armour */
				0,	/* Electric Sensitive */
				0,	/* Combustability */
				0,	/* Indestructable */
			},
		},
		section_has_sparkoflife|section_sprays_blood|section_flag_right_hand|section_flag_affectedbyheat,
	},
	{
		"hnpc_civvie",
		"male left bicep",
		NULL,
		{
			8,	/* Health */
			1,		/* Armour */
			0, /* IsOnFire */
			{
				0,	/* Acid Resistant */
				0,	/* Fire Resistant */
				0,	/* Electric Resistant */
				0,	/* Perfect Armour */
				0,	/* Electric Sensitive */
				0,	/* Combustability */
				0,	/* Indestructable */
			},
		},
		section_has_sparkoflife|section_sprays_blood|section_flag_left_arm|section_flag_affectedbyheat,
	},
	{
		"hnpc_civvie",
		"male left forearm",
		NULL,
		{
			8,	/* Health */
			1,		/* Armour */
			0, /* IsOnFire */
			{
				0,	/* Acid Resistant */
				0,	/* Fire Resistant */
				0,	/* Electric Resistant */
				0,	/* Perfect Armour */
				0,	/* Electric Sensitive */
				0,	/* Combustability */
				0,	/* Indestructable */
			},
		},
		section_has_sparkoflife|section_sprays_blood|section_flag_left_arm|section_flag_affectedbyheat,
	},
	{
		"hnpc_civvie",
		"left palm",
		NULL,
		{
			8,	/* Health */
			1,		/* Armour */
			0, /* IsOnFire */
			{
				0,	/* Acid Resistant */
				0,	/* Fire Resistant */
				0,	/* Electric Resistant */
				0,	/* Perfect Armour */
				0,	/* Electric Sensitive */
				0,	/* Combustability */
				0,	/* Indestructable */
			},
		},
		section_has_sparkoflife|section_sprays_blood|section_flag_left_hand|section_flag_affectedbyheat,
	},
	{
		"hnpc_civvie",
		"left fings top",
		NULL,
		{
			8,	/* Health */
			1,		/* Armour */
			0, /* IsOnFire */
			{
				0,	/* Acid Resistant */
				0,	/* Fire Resistant */
				0,	/* Electric Resistant */
				0,	/* Perfect Armour */
				0,	/* Electric Sensitive */
				0,	/* Combustability */
				0,	/* Indestructable */
			},
		},
		section_has_sparkoflife|section_sprays_blood|section_flag_left_hand|section_flag_affectedbyheat,
	},
	{
		"hnpc_civvie",
		"left fings end",
		NULL,
		{
			8,	/* Health */
			1,		/* Armour */
			0, /* IsOnFire */
			{
				0,	/* Acid Resistant */
				0,	/* Fire Resistant */
				0,	/* Electric Resistant */
				0,	/* Perfect Armour */
				0,	/* Electric Sensitive */
				0,	/* Combustability */
				0,	/* Indestructable */
			},
		},
		section_has_sparkoflife|section_sprays_blood|section_flag_left_hand|section_flag_affectedbyheat,
	},
	{
		"hnpc_civvie",
		"male left thumb",
		NULL,
		{
			8,	/* Health */
			1,		/* Armour */
			0, /* IsOnFire */
			{
				0,	/* Acid Resistant */
				0,	/* Fire Resistant */
				0,	/* Electric Resistant */
				0,	/* Perfect Armour */
				0,	/* Electric Sensitive */
				0,	/* Combustability */
				0,	/* Indestructable */
			},
		},
		section_has_sparkoflife|section_sprays_blood|section_flag_left_hand|section_flag_affectedbyheat,
	},
	{
		"hnpc_civvie",
		"hair",
		NULL,
		{
			10,		/* Health */
			8,		/* Armour */
			0, /* IsOnFire */
			{
				0,	/* Acid Resistant */
				0,	/* Fire Resistant */
				0,	/* Electric Resistant */
				0,	/* Perfect Armour */
				0,	/* Electric Sensitive */
				0,	/* Combustability */
				0,	/* Indestructable */
			},
		},
		section_flag_doesnthurtsb|section_flag_passdamagetoparent|section_flag_affectedbyheat,
	},
	/* A bit wusser than marines, and no armour! */
	/* Androids! */

	{
		"hnpc_civvie",
		"pelvis presley",
		Android_Hierarchy_Names,
		{
			8,	/* Health */
			1,		/* Armour */
			0, /* IsOnFire */
			{
				0,	/* Acid Resistant */
				0,	/* Fire Resistant */
				0,	/* Electric Resistant */
				0,	/* Perfect Armour */
				0,	/* Electric Sensitive */
				0,	/* Combustability */
				0,	/* Indestructable */
			},
		},
		section_has_sparkoflife|section_sprays_blood|section_flag_affectedbyheat,
	},

	{
		"hnpc_civvie",
		"male left thigh",
		Android_Hierarchy_Names,
		{
			8,	/* Health */
			1,		/* Armour */
			0, /* IsOnFire */
			{
				0,	/* Acid Resistant */
				0,	/* Fire Resistant */
				0,	/* Electric Resistant */
				0,	/* Perfect Armour */
				0,	/* Electric Sensitive */
				0,	/* Combustability */
				0,	/* Indestructable */
			},
		},
		section_has_sparkoflife|section_sprays_blood|section_flag_left_leg|section_flag_affectedbyheat,
	},
	{
		"hnpc_civvie",
		"male left shin",
		Android_Hierarchy_Names,
		{
			8,	/* Health */
			1,		/* Armour */
			0, /* IsOnFire */
			{
				0,	/* Acid Resistant */
				0,	/* Fire Resistant */
				0,	/* Electric Resistant */
				0,	/* Perfect Armour */
				0,	/* Electric Sensitive */
				0,	/* Combustability */
				0,	/* Indestructable */
			},
		},
		section_has_sparkoflife|section_sprays_blood|section_flag_left_leg|section_flag_affectedbyheat,
	},
	{
		"hnpc_civvie",
		"male left foot",
		Android_Hierarchy_Names,
		{
			8,	/* Health */
			1,		/* Armour */
			0, /* IsOnFire */
			{
				0,	/* Acid Resistant */
				0,	/* Fire Resistant */
				0,	/* Electric Resistant */
				0,	/* Perfect Armour */
				0,	/* Electric Sensitive */
				0,	/* Combustability */
				0,	/* Indestructable */
			},
		},
		section_has_sparkoflife|section_sprays_blood|section_flag_left_foot|section_flag_affectedbyheat,
	},
	{
		"hnpc_civvie",
		"male right thigh",
		Android_Hierarchy_Names,
		{
			8,	/* Health */
			1,		/* Armour */
			0, /* IsOnFire */
			{
				0,	/* Acid Resistant */
				0,	/* Fire Resistant */
				0,	/* Electric Resistant */
				0,	/* Perfect Armour */
				0,	/* Electric Sensitive */
				0,	/* Combustability */
				0,	/* Indestructable */
			},
		},
		section_has_sparkoflife|section_sprays_blood|section_flag_right_leg|section_flag_affectedbyheat,
	},
	{
		"hnpc_civvie",
		"male right shin",
		Android_Hierarchy_Names,
		{
			8,	/* Health */
			1,		/* Armour */
			0, /* IsOnFire */
			{
				0,	/* Acid Resistant */
				0,	/* Fire Resistant */
				0,	/* Electric Resistant */
				0,	/* Perfect Armour */
				0,	/* Electric Sensitive */
				0,	/* Combustability */
				0,	/* Indestructable */
			},
		},
		section_has_sparkoflife|section_sprays_blood|section_flag_right_leg|section_flag_affectedbyheat,
	},
	{
		"hnpc_civvie",
		"male right foot",
		Android_Hierarchy_Names,
		{
			8,	/* Health */
			1,		/* Armour */
			0, /* IsOnFire */
			{
				0,	/* Acid Resistant */
				0,	/* Fire Resistant */
				0,	/* Electric Resistant */
				0,	/* Perfect Armour */
				0,	/* Electric Sensitive */
				0,	/* Combustability */
				0,	/* Indestructable */
			},
		},
		section_has_sparkoflife|section_sprays_blood|section_flag_right_foot|section_flag_affectedbyheat,
	},
	{
		"hnpc_civvie",
		"stomach",
		Android_Hierarchy_Names,
		{
			8,	/* Health */
			1,		/* Armour */
			0, /* IsOnFire */
			{
				0,	/* Acid Resistant */
				0,	/* Fire Resistant */
				0,	/* Electric Resistant */
				0,	/* Perfect Armour */
				0,	/* Electric Sensitive */
				0,	/* Combustability */
				0,	/* Indestructable */
			},
		},
		section_has_sparkoflife|section_sprays_blood|section_flag_affectedbyheat|section_flag_fragonlyfordisks,
	},
	{
		"hnpc_civvie",
		"chest",
		Android_Hierarchy_Names,
		{
			8,	/* Health */
			1,		/* Armour */
			0, /* IsOnFire */
			{
				0,	/* Acid Resistant */
				0,	/* Fire Resistant */
				0,	/* Electric Resistant */
				0,	/* Perfect Armour */
				0,	/* Electric Sensitive */
				0,	/* Combustability */
				0,	/* Indestructable */
			},
		},
		section_has_sparkoflife|section_sprays_blood|section_flag_fragonlyfordisks,
	},
	{
		"hnpc_civvie",
		"neck",
		Android_Hierarchy_Names,
		{
			8,	/* Health */
			1,		/* Armour */
			0, /* IsOnFire */
			{
				0,	/* Acid Resistant */
				0,	/* Fire Resistant */
				0,	/* Electric Resistant */
				0,	/* Perfect Armour */
				0,	/* Electric Sensitive */
				0,	/* Combustability */
				0,	/* Indestructable */
			},
		},
		section_has_sparkoflife|section_sprays_blood|section_flag_affectedbyheat,
	},
	{
		"hnpc_civvie",
		"head",
		Android_Hierarchy_Names,
		{
			8,	/* Health */
			1,		/* Armour */
			0, /* IsOnFire */
			{
				0,	/* Acid Resistant */
				0,	/* Fire Resistant */
				0,	/* Electric Resistant */
				0,	/* Perfect Armour */
				0,	/* Electric Sensitive */
				0,	/* Combustability */
				0,	/* Indestructable */
			},
		},
		section_has_sparkoflife|section_sprays_blood,
	},
	{
		"hnpc_civvie",
		"male right bicep",
		Android_Hierarchy_Names,
		{
			8,	/* Health */
			1,		/* Armour */
			0, /* IsOnFire */
			{
				0,	/* Acid Resistant */
				0,	/* Fire Resistant */
				0,	/* Electric Resistant */
				0,	/* Perfect Armour */
				0,	/* Electric Sensitive */
				0,	/* Combustability */
				0,	/* Indestructable */
			},
		},
		section_sprays_blood|section_flag_right_arm|section_flag_affectedbyheat,
	},
	{
		"hnpc_civvie",
		"male right forearm",
		Android_Hierarchy_Names,
		{
			8,	/* Health */
			1,		/* Armour */
			0, /* IsOnFire */
			{
				0,	/* Acid Resistant */
				0,	/* Fire Resistant */
				0,	/* Electric Resistant */
				0,	/* Perfect Armour */
				0,	/* Electric Sensitive */
				0,	/* Combustability */
				0,	/* Indestructable */
			},
		},
		section_sprays_blood|section_flag_right_arm|section_flag_affectedbyheat,
	},
	{
		"hnpc_civvie",
		"right palm",
		Android_Hierarchy_Names,
		{
			8,	/* Health */
			1,		/* Armour */
			0, /* IsOnFire */
			{
				0,	/* Acid Resistant */
				0,	/* Fire Resistant */
				0,	/* Electric Resistant */
				0,	/* Perfect Armour */
				0,	/* Electric Sensitive */
				0,	/* Combustability */
				0,	/* Indestructable */
			},
		},
		section_sprays_blood|section_flag_right_hand|section_flag_affectedbyheat,
	},
	{
		"hnpc_civvie",
		"right fings top",
		Android_Hierarchy_Names,
		{
			8,	/* Health */
			1,		/* Armour */
			0, /* IsOnFire */
			{
				0,	/* Acid Resistant */
				0,	/* Fire Resistant */
				0,	/* Electric Resistant */
				0,	/* Perfect Armour */
				0,	/* Electric Sensitive */
				0,	/* Combustability */
				0,	/* Indestructable */
			},
		},
		section_sprays_blood|section_flag_right_hand|section_flag_affectedbyheat,
	},
	{
		"hnpc_civvie",
		"right fings end",
		Android_Hierarchy_Names,
		{
			8,	/* Health */
			1,		/* Armour */
			0, /* IsOnFire */
			{
				0,	/* Acid Resistant */
				0,	/* Fire Resistant */
				0,	/* Electric Resistant */
				0,	/* Perfect Armour */
				0,	/* Electric Sensitive */
				0,	/* Combustability */
				0,	/* Indestructable */
			},
		},
		section_sprays_blood|section_flag_right_hand|section_flag_affectedbyheat,
	},
	{
		"hnpc_civvie",
		"male right thumb",
		Android_Hierarchy_Names,
		{
			8,	/* Health */
			1,		/* Armour */
			0, /* IsOnFire */
			{
				0,	/* Acid Resistant */
				0,	/* Fire Resistant */
				0,	/* Electric Resistant */
				0,	/* Perfect Armour */
				0,	/* Electric Sensitive */
				0,	/* Combustability */
				0,	/* Indestructable */
			},
		},
		section_sprays_blood|section_flag_right_hand|section_flag_affectedbyheat,
	},
	{
		"hnpc_civvie",
		"male left bicep",
		Android_Hierarchy_Names,
		{
			8,	/* Health */
			1,		/* Armour */
			0, /* IsOnFire */
			{
				0,	/* Acid Resistant */
				0,	/* Fire Resistant */
				0,	/* Electric Resistant */
				0,	/* Perfect Armour */
				0,	/* Electric Sensitive */
				0,	/* Combustability */
				0,	/* Indestructable */
			},
		},
		section_sprays_blood|section_flag_left_arm|section_flag_affectedbyheat,
	},
	{
		"hnpc_civvie",
		"male left forearm",
		Android_Hierarchy_Names,
		{
			8,	/* Health */
			1,		/* Armour */
			0, /* IsOnFire */
			{
				0,	/* Acid Resistant */
				0,	/* Fire Resistant */
				0,	/* Electric Resistant */
				0,	/* Perfect Armour */
				0,	/* Electric Sensitive */
				0,	/* Combustability */
				0,	/* Indestructable */
			},
		},
		section_sprays_blood|section_flag_left_arm|section_flag_affectedbyheat,
	},
	{
		"hnpc_civvie",
		"left palm",
		Android_Hierarchy_Names,
		{
			8,	/* Health */
			1,		/* Armour */
			0, /* IsOnFire */
			{
				0,	/* Acid Resistant */
				0,	/* Fire Resistant */
				0,	/* Electric Resistant */
				0,	/* Perfect Armour */
				0,	/* Electric Sensitive */
				0,	/* Combustability */
				0,	/* Indestructable */
			},
		},
		section_sprays_blood|section_flag_left_hand|section_flag_affectedbyheat,
	},
	{
		"hnpc_civvie",
		"left fings top",
		Android_Hierarchy_Names,
		{
			8,	/* Health */
			1,		/* Armour */
			0, /* IsOnFire */
			{
				0,	/* Acid Resistant */
				0,	/* Fire Resistant */
				0,	/* Electric Resistant */
				0,	/* Perfect Armour */
				0,	/* Electric Sensitive */
				0,	/* Combustability */
				0,	/* Indestructable */
			},
		},
		section_sprays_blood|section_flag_left_hand|section_flag_affectedbyheat,
	},
	{
		"hnpc_civvie",
		"left fings end",
		Android_Hierarchy_Names,
		{
			8,	/* Health */
			1,		/* Armour */
			0, /* IsOnFire */
			{
				0,	/* Acid Resistant */
				0,	/* Fire Resistant */
				0,	/* Electric Resistant */
				0,	/* Perfect Armour */
				0,	/* Electric Sensitive */
				0,	/* Combustability */
				0,	/* Indestructable */
			},
		},
		section_sprays_blood|section_flag_left_hand|section_flag_affectedbyheat,
	},
	{
		"hnpc_civvie",
		"male left thumb",
		Android_Hierarchy_Names,
		{
			8,	/* Health */
			1,		/* Armour */
			0, /* IsOnFire */
			{
				0,	/* Acid Resistant */
				0,	/* Fire Resistant */
				0,	/* Electric Resistant */
				0,	/* Perfect Armour */
				0,	/* Electric Sensitive */
				0,	/* Combustability */
				0,	/* Indestructable */
			},
		},
		section_sprays_blood|section_flag_left_hand|section_flag_affectedbyheat,
	},
	{
		"hnpc_civvie",
		"hair",
		Android_Hierarchy_Names,
		{
			10,		/* Health */
			8,		/* Armour */
			0, /* IsOnFire */
			{
				0,	/* Acid Resistant */
				0,	/* Fire Resistant */
				0,	/* Electric Resistant */
				0,	/* Perfect Armour */
				0,	/* Electric Sensitive */
				0,	/* Combustability */
				0,	/* Indestructable */
			},
		},
		section_flag_doesnthurtsb|section_flag_passdamagetoparent|section_flag_affectedbyheat,
	},

	/* PREDATOR HUD. */
	{
		"pred_hud",
		"lbs",
		NULL,
		{
			100,	/* Health */
			100,	/* Armour */
			0, /* IsOnFire */
			{
				0,	/* Acid Resistant */
				0,	/* Fire Resistant */
				0,	/* Electric Resistant */
				0,	/* Perfect Armour */
				0,	/* Electric Sensitive */
				0,	/* Combustability */
				0,	/* Indestructable */
			},
		},
		section_flag_affectedbyheat,
	},
	{
		"pred_hud",
		"lba",
		NULL,
		{
			100,	/* Health */
			100,	/* Armour */
			0, /* IsOnFire */
			{
				0,	/* Acid Resistant */
				0,	/* Fire Resistant */
				0,	/* Electric Resistant */
				0,	/* Perfect Armour */
				0,	/* Electric Sensitive */
				0,	/* Combustability */
				0,	/* Indestructable */
			},
		},
		section_flag_heatsource,
	},
	{
		"pred_hud",
		"lbw",
		NULL,
		{
			100,	/* Health */
			100,	/* Armour */
			0, /* IsOnFire */
			{
				0,	/* Acid Resistant */
				0,	/* Fire Resistant */
				0,	/* Electric Resistant */
				0,	/* Perfect Armour */
				0,	/* Electric Sensitive */
				0,	/* Combustability */
				0,	/* Indestructable */
			},
		},
		section_flag_affectedbyheat,
	},
	{
		"pred_hud",
		"lbh",
		NULL,
		{
			100,	/* Health */
			100,	/* Armour */
			0, /* IsOnFire */
			{
				0,	/* Acid Resistant */
				0,	/* Fire Resistant */
				0,	/* Electric Resistant */
				0,	/* Perfect Armour */
				0,	/* Electric Sensitive */
				0,	/* Combustability */
				0,	/* Indestructable */
			},
		},
		section_flag_heatsource,
	},
	{
		"pred_hud",
		"lbffff",
		NULL,
		{
			100,	/* Health */
			100,	/* Armour */
			0, /* IsOnFire */
			{
				0,	/* Acid Resistant */
				0,	/* Fire Resistant */
				0,	/* Electric Resistant */
				0,	/* Perfect Armour */
				0,	/* Electric Sensitive */
				0,	/* Combustability */
				0,	/* Indestructable */
			},
		},
		section_flag_affectedbyheat,
	},
	{
		"pred_hud",
		"lbfff",
		NULL,
		{
			100,	/* Health */
			100,	/* Armour */
			0, /* IsOnFire */
			{
				0,	/* Acid Resistant */
				0,	/* Fire Resistant */
				0,	/* Electric Resistant */
				0,	/* Perfect Armour */
				0,	/* Electric Sensitive */
				0,	/* Combustability */
				0,	/* Indestructable */
			},
		},
		section_flag_affectedbyheat,
	},
	{
		"pred_hud",
		"lbff",
		NULL,
		{
			100,	/* Health */
			100,	/* Armour */
			0, /* IsOnFire */
			{
				0,	/* Acid Resistant */
				0,	/* Fire Resistant */
				0,	/* Electric Resistant */
				0,	/* Perfect Armour */
				0,	/* Electric Sensitive */
				0,	/* Combustability */
				0,	/* Indestructable */
			},
		},
		section_flag_affectedbyheat,
	},
	{
		"pred_hud",
		"lbf",
		NULL,
		{
			100,	/* Health */
			100,	/* Armour */
			0, /* IsOnFire */
			{
				0,	/* Acid Resistant */
				0,	/* Fire Resistant */
				0,	/* Electric Resistant */
				0,	/* Perfect Armour */
				0,	/* Electric Sensitive */
				0,	/* Combustability */
				0,	/* Indestructable */
			},
		},
		section_flag_affectedbyheat,
	},
	{
		"pred_hud",
		"lbt",
		NULL,
		{
			100,	/* Health */
			100,	/* Armour */
			0, /* IsOnFire */
			{
				0,	/* Acid Resistant */
				0,	/* Fire Resistant */
				0,	/* Electric Resistant */
				0,	/* Perfect Armour */
				0,	/* Electric Sensitive */
				0,	/* Combustability */
				0,	/* Indestructable */
			},
		},
		section_flag_affectedbyheat,
	},
	{
		"pred_hud",
		"rbs",
		NULL,
		{
			100,	/* Health */
			100,	/* Armour */
			0, /* IsOnFire */
			{
				0,	/* Acid Resistant */
				0,	/* Fire Resistant */
				0,	/* Electric Resistant */
				0,	/* Perfect Armour */
				0,	/* Electric Sensitive */
				0,	/* Combustability */
				0,	/* Indestructable */
			},
		},
		section_flag_affectedbyheat,
	},
	{
		"pred_hud",
		"rba",
		NULL,
		{
			100,	/* Health */
			100,	/* Armour */
			0, /* IsOnFire */
			{
				0,	/* Acid Resistant */
				0,	/* Fire Resistant */
				0,	/* Electric Resistant */
				0,	/* Perfect Armour */
				0,	/* Electric Sensitive */
				0,	/* Combustability */
				0,	/* Indestructable */
			},
		},
		section_flag_heatsource,
	},
	{
		"pred_hud",
		"rbw",
		NULL,
		{
			100,	/* Health */
			100,	/* Armour */
			0, /* IsOnFire */
			{
				0,	/* Acid Resistant */
				0,	/* Fire Resistant */
				0,	/* Electric Resistant */
				0,	/* Perfect Armour */
				0,	/* Electric Sensitive */
				0,	/* Combustability */
				0,	/* Indestructable */
			},
		},
		section_flag_affectedbyheat,
	},
	{
		"pred_hud",
		"rbh",
		NULL,
		{
			100,	/* Health */
			100,	/* Armour */
			0, /* IsOnFire */
			{
				0,	/* Acid Resistant */
				0,	/* Fire Resistant */
				0,	/* Electric Resistant */
				0,	/* Perfect Armour */
				0,	/* Electric Sensitive */
				0,	/* Combustability */
				0,	/* Indestructable */
			},
		},
		section_flag_heatsource,
	},
	{
		"pred_hud",
		"rbffff",
		NULL,
		{
			100,	/* Health */
			100,	/* Armour */
			0, /* IsOnFire */
			{
				0,	/* Acid Resistant */
				0,	/* Fire Resistant */
				0,	/* Electric Resistant */
				0,	/* Perfect Armour */
				0,	/* Electric Sensitive */
				0,	/* Combustability */
				0,	/* Indestructable */
			},
		},
		section_flag_affectedbyheat,
	},
	{
		"pred_hud",
		"rbfff",
		NULL,
		{
			100,	/* Health */
			100,	/* Armour */
			0, /* IsOnFire */
			{
				0,	/* Acid Resistant */
				0,	/* Fire Resistant */
				0,	/* Electric Resistant */
				0,	/* Perfect Armour */
				0,	/* Electric Sensitive */
				0,	/* Combustability */
				0,	/* Indestructable */
			},
		},
		section_flag_affectedbyheat,
	},
	{
		"pred_hud",
		"rbff",
		NULL,
		{
			100,	/* Health */
			100,	/* Armour */
			0, /* IsOnFire */
			{
				0,	/* Acid Resistant */
				0,	/* Fire Resistant */
				0,	/* Electric Resistant */
				0,	/* Perfect Armour */
				0,	/* Electric Sensitive */
				0,	/* Combustability */
				0,	/* Indestructable */
			},
		},
		section_flag_affectedbyheat,
	},
	{
		"pred_hud",
		"rbf",
		NULL,
		{
			100,	/* Health */
			100,	/* Armour */
			0, /* IsOnFire */
			{
				0,	/* Acid Resistant */
				0,	/* Fire Resistant */
				0,	/* Electric Resistant */
				0,	/* Perfect Armour */
				0,	/* Electric Sensitive */
				0,	/* Combustability */
				0,	/* Indestructable */
			},
		},
		section_flag_affectedbyheat,
	},
	{
		"pred_hud",
		"rbt",
		NULL,
		{
			100,	/* Health */
			100,	/* Armour */
			0, /* IsOnFire */
			{
				0,	/* Acid Resistant */
				0,	/* Fire Resistant */
				0,	/* Electric Resistant */
				0,	/* Perfect Armour */
				0,	/* Electric Sensitive */
				0,	/* Combustability */
				0,	/* Indestructable */
			},
		},
		section_flag_affectedbyheat,
	},

	{
		"pred_hud",
		"rps",
		NULL,
		{
			100,	/* Health */
			100,	/* Armour */
			0, /* IsOnFire */
			{
				0,	/* Acid Resistant */
				0,	/* Fire Resistant */
				0,	/* Electric Resistant */
				0,	/* Perfect Armour */
				0,	/* Electric Sensitive */
				0,	/* Combustability */
				0,	/* Indestructable */
			},
		},
		section_flag_affectedbyheat,
	},
	{
		"pred_hud",
		"rpa",
		NULL,
		{
			100,	/* Health */
			100,	/* Armour */
			0, /* IsOnFire */
			{
				0,	/* Acid Resistant */
				0,	/* Fire Resistant */
				0,	/* Electric Resistant */
				0,	/* Perfect Armour */
				0,	/* Electric Sensitive */
				0,	/* Combustability */
				0,	/* Indestructable */
			},
		},
		section_flag_heatsource,
	},
	{
		"pred_hud",
		"rpw",
		NULL,
		{
			100,	/* Health */
			100,	/* Armour */
			0, /* IsOnFire */
			{
				0,	/* Acid Resistant */
				0,	/* Fire Resistant */
				0,	/* Electric Resistant */
				0,	/* Perfect Armour */
				0,	/* Electric Sensitive */
				0,	/* Combustability */
				0,	/* Indestructable */
			},
		},
		section_flag_affectedbyheat,
	},
	{
		"pred_hud",
		"rph",
		NULL,
		{
			100,	/* Health */
			100,	/* Armour */
			0, /* IsOnFire */
			{
				0,	/* Acid Resistant */
				0,	/* Fire Resistant */
				0,	/* Electric Resistant */
				0,	/* Perfect Armour */
				0,	/* Electric Sensitive */
				0,	/* Combustability */
				0,	/* Indestructable */
			},
		},
		section_flag_heatsource,
	},
	{
		"pred_hud",
		"rpffff",
		NULL,
		{
			100,	/* Health */
			100,	/* Armour */
			0, /* IsOnFire */
			{
				0,	/* Acid Resistant */
				0,	/* Fire Resistant */
				0,	/* Electric Resistant */
				0,	/* Perfect Armour */
				0,	/* Electric Sensitive */
				0,	/* Combustability */
				0,	/* Indestructable */
			},
		},
		section_flag_affectedbyheat,
	},
	{
		"pred_hud",
		"rpfff",
		NULL,
		{
			100,	/* Health */
			100,	/* Armour */
			0, /* IsOnFire */
			{
				0,	/* Acid Resistant */
				0,	/* Fire Resistant */
				0,	/* Electric Resistant */
				0,	/* Perfect Armour */
				0,	/* Electric Sensitive */
				0,	/* Combustability */
				0,	/* Indestructable */
			},
		},
		section_flag_affectedbyheat,
	},
	{
		"pred_hud",
		"rpff",
		NULL,
		{
			100,	/* Health */
			100,	/* Armour */
			0, /* IsOnFire */
			{
				0,	/* Acid Resistant */
				0,	/* Fire Resistant */
				0,	/* Electric Resistant */
				0,	/* Perfect Armour */
				0,	/* Electric Sensitive */
				0,	/* Combustability */
				0,	/* Indestructable */
			},
		},
		section_flag_affectedbyheat,
	},
	{
		"pred_hud",
		"rpf",
		NULL,
		{
			100,	/* Health */
			100,	/* Armour */
			0, /* IsOnFire */
			{
				0,	/* Acid Resistant */
				0,	/* Fire Resistant */
				0,	/* Electric Resistant */
				0,	/* Perfect Armour */
				0,	/* Electric Sensitive */
				0,	/* Combustability */
				0,	/* Indestructable */
			},
		},
		section_flag_affectedbyheat,
	},
	{
		"pred_hud",
		"rpt",
		NULL,
		{
			100,	/* Health */
			100,	/* Armour */
			0, /* IsOnFire */
			{
				0,	/* Acid Resistant */
				0,	/* Fire Resistant */
				0,	/* Electric Resistant */
				0,	/* Perfect Armour */
				0,	/* Electric Sensitive */
				0,	/* Combustability */
				0,	/* Indestructable */
			},
		},
		section_flag_affectedbyheat,
	},
	{
		"pred_hud",
		"lds",
		NULL,
		{
			100,	/* Health */
			100,	/* Armour */
			0, /* IsOnFire */
			{
				0,	/* Acid Resistant */
				0,	/* Fire Resistant */
				0,	/* Electric Resistant */
				0,	/* Perfect Armour */
				0,	/* Electric Sensitive */
				0,	/* Combustability */
				0,	/* Indestructable */
			},
		},
		section_flag_affectedbyheat,
	},
	{
		"pred_hud",
		"lda",
		NULL,
		{
			100,	/* Health */
			100,	/* Armour */
			0, /* IsOnFire */
			{
				0,	/* Acid Resistant */
				0,	/* Fire Resistant */
				0,	/* Electric Resistant */
				0,	/* Perfect Armour */
				0,	/* Electric Sensitive */
				0,	/* Combustability */
				0,	/* Indestructable */
			},
		},
		section_flag_heatsource,
	},
	{
		"pred_hud",
		"ldw",
		NULL,
		{
			100,	/* Health */
			100,	/* Armour */
			0, /* IsOnFire */
			{
				0,	/* Acid Resistant */
				0,	/* Fire Resistant */
				0,	/* Electric Resistant */
				0,	/* Perfect Armour */
				0,	/* Electric Sensitive */
				0,	/* Combustability */
				0,	/* Indestructable */
			},
		},
		section_flag_affectedbyheat,
	},
	{
		"pred_hud",
		"ldh",
		NULL,
		{
			100,	/* Health */
			100,	/* Armour */
			0, /* IsOnFire */
			{
				0,	/* Acid Resistant */
				0,	/* Fire Resistant */
				0,	/* Electric Resistant */
				0,	/* Perfect Armour */
				0,	/* Electric Sensitive */
				0,	/* Combustability */
				0,	/* Indestructable */
			},
		},
		section_flag_heatsource,
	},
	{
		"pred_hud",
		"ldffff",
		NULL,
		{
			100,	/* Health */
			100,	/* Armour */
			0, /* IsOnFire */
			{
				0,	/* Acid Resistant */
				0,	/* Fire Resistant */
				0,	/* Electric Resistant */
				0,	/* Perfect Armour */
				0,	/* Electric Sensitive */
				0,	/* Combustability */
				0,	/* Indestructable */
			},
		},
		section_flag_affectedbyheat,
	},
	{
		"pred_hud",
		"ldfff",
		NULL,
		{
			100,	/* Health */
			100,	/* Armour */
			0, /* IsOnFire */
			{
				0,	/* Acid Resistant */
				0,	/* Fire Resistant */
				0,	/* Electric Resistant */
				0,	/* Perfect Armour */
				0,	/* Electric Sensitive */
				0,	/* Combustability */
				0,	/* Indestructable */
			},
		},
		section_flag_affectedbyheat,
	},
	{
		"pred_hud",
		"ldff",
		NULL,
		{
			100,	/* Health */
			100,	/* Armour */
			0, /* IsOnFire */
			{
				0,	/* Acid Resistant */
				0,	/* Fire Resistant */
				0,	/* Electric Resistant */
				0,	/* Perfect Armour */
				0,	/* Electric Sensitive */
				0,	/* Combustability */
				0,	/* Indestructable */
			},
		},
		section_flag_affectedbyheat,
	},
	{
		"pred_hud",
		"ldf",
		NULL,
		{
			100,	/* Health */
			100,	/* Armour */
			0, /* IsOnFire */
			{
				0,	/* Acid Resistant */
				0,	/* Fire Resistant */
				0,	/* Electric Resistant */
				0,	/* Perfect Armour */
				0,	/* Electric Sensitive */
				0,	/* Combustability */
				0,	/* Indestructable */
			},
		},
		section_flag_affectedbyheat,
	},
	{
		"pred_hud",
		"ldt",
		NULL,
		{
			100,	/* Health */
			100,	/* Armour */
			0, /* IsOnFire */
			{
				0,	/* Acid Resistant */
				0,	/* Fire Resistant */
				0,	/* Electric Resistant */
				0,	/* Perfect Armour */
				0,	/* Electric Sensitive */
				0,	/* Combustability */
				0,	/* Indestructable */
			},
		},
		section_flag_affectedbyheat,
	},
	{
		"pred_hud",
		"rds",
		NULL,
		{
			100,	/* Health */
			100,	/* Armour */
			0, /* IsOnFire */
			{
				0,	/* Acid Resistant */
				0,	/* Fire Resistant */
				0,	/* Electric Resistant */
				0,	/* Perfect Armour */
				0,	/* Electric Sensitive */
				0,	/* Combustability */
				0,	/* Indestructable */
			},
		},
		section_flag_affectedbyheat,
	},
	{
		"pred_hud",
		"rda",
		NULL,
		{
			100,	/* Health */
			100,	/* Armour */
			0, /* IsOnFire */
			{
				0,	/* Acid Resistant */
				0,	/* Fire Resistant */
				0,	/* Electric Resistant */
				0,	/* Perfect Armour */
				0,	/* Electric Sensitive */
				0,	/* Combustability */
				0,	/* Indestructable */
			},
		},
		section_flag_heatsource,
	},
	{
		"pred_hud",
		"rdw",
		NULL,
		{
			100,	/* Health */
			100,	/* Armour */
			0, /* IsOnFire */
			{
				0,	/* Acid Resistant */
				0,	/* Fire Resistant */
				0,	/* Electric Resistant */
				0,	/* Perfect Armour */
				0,	/* Electric Sensitive */
				0,	/* Combustability */
				0,	/* Indestructable */
			},
		},
		section_flag_affectedbyheat,
	},
	{
		"pred_hud",
		"rdh",
		NULL,
		{
			100,	/* Health */
			100,	/* Armour */
			0, /* IsOnFire */
			{
				0,	/* Acid Resistant */
				0,	/* Fire Resistant */
				0,	/* Electric Resistant */
				0,	/* Perfect Armour */
				0,	/* Electric Sensitive */
				0,	/* Combustability */
				0,	/* Indestructable */
			},
		},
		section_flag_heatsource,
	},
	{
		"pred_hud",
		"rdffff",
		NULL,
		{
			100,	/* Health */
			100,	/* Armour */
			0, /* IsOnFire */
			{
				0,	/* Acid Resistant */
				0,	/* Fire Resistant */
				0,	/* Electric Resistant */
				0,	/* Perfect Armour */
				0,	/* Electric Sensitive */
				0,	/* Combustability */
				0,	/* Indestructable */
			},
		},
		section_flag_affectedbyheat,
	},
	{
		"pred_hud",
		"rdfff",
		NULL,
		{
			100,	/* Health */
			100,	/* Armour */
			0, /* IsOnFire */
			{
				0,	/* Acid Resistant */
				0,	/* Fire Resistant */
				0,	/* Electric Resistant */
				0,	/* Perfect Armour */
				0,	/* Electric Sensitive */
				0,	/* Combustability */
				0,	/* Indestructable */
			},
		},
		section_flag_affectedbyheat,
	},
	{
		"pred_hud",
		"rdff",
		NULL,
		{
			100,	/* Health */
			100,	/* Armour */
			0, /* IsOnFire */
			{
				0,	/* Acid Resistant */
				0,	/* Fire Resistant */
				0,	/* Electric Resistant */
				0,	/* Perfect Armour */
				0,	/* Electric Sensitive */
				0,	/* Combustability */
				0,	/* Indestructable */
			},
		},
		section_flag_affectedbyheat,
	},
	{
		"pred_hud",
		"rdf",
		NULL,
		{
			100,	/* Health */
			100,	/* Armour */
			0, /* IsOnFire */
			{
				0,	/* Acid Resistant */
				0,	/* Fire Resistant */
				0,	/* Electric Resistant */
				0,	/* Perfect Armour */
				0,	/* Electric Sensitive */
				0,	/* Combustability */
				0,	/* Indestructable */
			},
		},
		section_flag_affectedbyheat,
	},
	{
		"pred_hud",
		"rdt",
		NULL,
		{
			100,	/* Health */
			100,	/* Armour */
			0, /* IsOnFire */
			{
				0,	/* Acid Resistant */
				0,	/* Fire Resistant */
				0,	/* Electric Resistant */
				0,	/* Perfect Armour */
				0,	/* Electric Sensitive */
				0,	/* Combustability */
				0,	/* Indestructable */
			},
		},
		section_flag_affectedbyheat,
	},
	{
		"pred_hud",
		"Staff L palm",
		NULL,
		{
			100,	/* Health */
			100,	/* Armour */
			0, /* IsOnFire */
			{
				0,	/* Acid Resistant */
				0,	/* Fire Resistant */
				0,	/* Electric Resistant */
				0,	/* Perfect Armour */
				0,	/* Electric Sensitive */
				0,	/* Combustability */
				0,	/* Indestructable */
			},
		},
		section_flag_affectedbyheat,
	},
	{
		"pred_hud",
		"Staff L little finger",
		NULL,
		{
			100,	/* Health */
			100,	/* Armour */
			0, /* IsOnFire */
			{
				0,	/* Acid Resistant */
				0,	/* Fire Resistant */
				0,	/* Electric Resistant */
				0,	/* Perfect Armour */
				0,	/* Electric Sensitive */
				0,	/* Combustability */
				0,	/* Indestructable */
			},
		},
		section_flag_affectedbyheat,
	},
	{
		"pred_hud",
		"Staff L mid finger",
		NULL,
		{
			100,	/* Health */
			100,	/* Armour */
			0, /* IsOnFire */
			{
				0,	/* Acid Resistant */
				0,	/* Fire Resistant */
				0,	/* Electric Resistant */
				0,	/* Perfect Armour */
				0,	/* Electric Sensitive */
				0,	/* Combustability */
				0,	/* Indestructable */
			},
		},
		section_flag_affectedbyheat,
	},
	{
		"pred_hud",
		"Staff L ring finger",
		NULL,
		{
			100,	/* Health */
			100,	/* Armour */
			0, /* IsOnFire */
			{
				0,	/* Acid Resistant */
				0,	/* Fire Resistant */
				0,	/* Electric Resistant */
				0,	/* Perfect Armour */
				0,	/* Electric Sensitive */
				0,	/* Combustability */
				0,	/* Indestructable */
			},
		},
		section_flag_affectedbyheat,
	},
	{
		"pred_hud",
		"Staff L thumb",
		NULL,
		{
			100,	/* Health */
			100,	/* Armour */
			0, /* IsOnFire */
			{
				0,	/* Acid Resistant */
				0,	/* Fire Resistant */
				0,	/* Electric Resistant */
				0,	/* Perfect Armour */
				0,	/* Electric Sensitive */
				0,	/* Combustability */
				0,	/* Indestructable */
			},
		},
		section_flag_affectedbyheat,
	},
	{
		"pred_hud",
		"Staff L index finger",
		NULL,
		{
			100,	/* Health */
			100,	/* Armour */
			0, /* IsOnFire */
			{
				0,	/* Acid Resistant */
				0,	/* Fire Resistant */
				0,	/* Electric Resistant */
				0,	/* Perfect Armour */
				0,	/* Electric Sensitive */
				0,	/* Combustability */
				0,	/* Indestructable */
			},
		},
		section_flag_affectedbyheat,
	},
	{
		"pred_hud",
		"staff L forearm",
		NULL,
		{
			100,	/* Health */
			100,	/* Armour */
			0, /* IsOnFire */
			{
				0,	/* Acid Resistant */
				0,	/* Fire Resistant */
				0,	/* Electric Resistant */
				0,	/* Perfect Armour */
				0,	/* Electric Sensitive */
				0,	/* Combustability */
				0,	/* Indestructable */
			},
		},
		section_flag_heatsource,
	},
	{
		"pred_hud",
		"Staff L bicep",
		NULL,
		{
			100,	/* Health */
			100,	/* Armour */
			0, /* IsOnFire */
			{
				0,	/* Acid Resistant */
				0,	/* Fire Resistant */
				0,	/* Electric Resistant */
				0,	/* Perfect Armour */
				0,	/* Electric Sensitive */
				0,	/* Combustability */
				0,	/* Indestructable */
			},
		},
		section_flag_affectedbyheat,
	},
	{
		"pred_hud",
		"Staff R palm",
		NULL,
		{
			100,	/* Health */
			100,	/* Armour */
			0, /* IsOnFire */
			{
				0,	/* Acid Resistant */
				0,	/* Fire Resistant */
				0,	/* Electric Resistant */
				0,	/* Perfect Armour */
				0,	/* Electric Sensitive */
				0,	/* Combustability */
				0,	/* Indestructable */
			},
		},
		section_flag_affectedbyheat,
	},
	{
		"pred_hud",
		"Staff R little finger",
		NULL,
		{
			100,	/* Health */
			100,	/* Armour */
			0, /* IsOnFire */
			{
				0,	/* Acid Resistant */
				0,	/* Fire Resistant */
				0,	/* Electric Resistant */
				0,	/* Perfect Armour */
				0,	/* Electric Sensitive */
				0,	/* Combustability */
				0,	/* Indestructable */
			},
		},
		section_flag_affectedbyheat,
	},
	{
		"pred_hud",
		"Staff R mid finger",
		NULL,
		{
			100,	/* Health */
			100,	/* Armour */
			0, /* IsOnFire */
			{
				0,	/* Acid Resistant */
				0,	/* Fire Resistant */
				0,	/* Electric Resistant */
				0,	/* Perfect Armour */
				0,	/* Electric Sensitive */
				0,	/* Combustability */
				0,	/* Indestructable */
			},
		},
		section_flag_affectedbyheat,
	},
	{
		"pred_hud",
		"Staff R ring finger",
		NULL,
		{
			100,	/* Health */
			100,	/* Armour */
			0, /* IsOnFire */
			{
				0,	/* Acid Resistant */
				0,	/* Fire Resistant */
				0,	/* Electric Resistant */
				0,	/* Perfect Armour */
				0,	/* Electric Sensitive */
				0,	/* Combustability */
				0,	/* Indestructable */
			},
		},
		section_flag_affectedbyheat,
	},
	{
		"pred_hud",
		"Staff R thumb",
		NULL,
		{
			100,	/* Health */
			100,	/* Armour */
			0, /* IsOnFire */
			{
				0,	/* Acid Resistant */
				0,	/* Fire Resistant */
				0,	/* Electric Resistant */
				0,	/* Perfect Armour */
				0,	/* Electric Sensitive */
				0,	/* Combustability */
				0,	/* Indestructable */
			},
		},
		section_flag_affectedbyheat,
	},
	{
		"pred_hud",
		"Staff R index finger",
		NULL,
		{
			100,	/* Health */
			100,	/* Armour */
			0, /* IsOnFire */
			{
				0,	/* Acid Resistant */
				0,	/* Fire Resistant */
				0,	/* Electric Resistant */
				0,	/* Perfect Armour */
				0,	/* Electric Sensitive */
				0,	/* Combustability */
				0,	/* Indestructable */
			},
		},
		section_flag_affectedbyheat,
	},
	{
		"pred_hud",
		"staff R forearm",
		NULL,
		{
			100,	/* Health */
			100,	/* Armour */
			0, /* IsOnFire */
			{
				0,	/* Acid Resistant */
				0,	/* Fire Resistant */
				0,	/* Electric Resistant */
				0,	/* Perfect Armour */
				0,	/* Electric Sensitive */
				0,	/* Combustability */
				0,	/* Indestructable */
			},
		},
		section_flag_heatsource,
	},
	{
		"pred_hud",
		"Staff R bicep",
		NULL,
		{
			100,	/* Health */
			100,	/* Armour */
			0, /* IsOnFire */
			{
				0,	/* Acid Resistant */
				0,	/* Fire Resistant */
				0,	/* Electric Resistant */
				0,	/* Perfect Armour */
				0,	/* Electric Sensitive */
				0,	/* Combustability */
				0,	/* Indestructable */
			},
		},
		section_flag_affectedbyheat,
	},
	/* XENOBORG */
	{
		"hnpc_xenoborg",
		"pelvis presley",
		NULL,
		{
			400,	/* Health */
			120,	/* Armour */
			0, 		/* IsOnFire */
			{
				1,	/* Acid Resistant */
				0,	/* Fire Resistant */
				0,	/* Electric Resistant */
				1,	/* Perfect Armour */
				0,	/* Electric Sensitive */
				0,	/* Combustability */
				0,	/* Indestructable */
			},
		},
		section_has_sparkoflife|section_sprays_acid,
	},
	{
		"hnpc_xenoborg",
		"right thigh",
		NULL,
		{
			270,	/* Health */
			120,	/* Armour */
			0, 		/* IsOnFire */
			{
				1,	/* Acid Resistant */
				0,	/* Fire Resistant */
				0,	/* Electric Resistant */
				1,	/* Perfect Armour */
				0,	/* Electric Sensitive */
				0,	/* Combustability */
				0,	/* Indestructable */
			},
		},
		section_flag_right_leg|section_has_sparkoflife|section_sprays_sparks,
	},
	{
		"hnpc_xenoborg",
		"right shin",
		NULL,
		{
			270,	/* Health */
			120,	/* Armour */
			0, 		/* IsOnFire */
			{
				1,	/* Acid Resistant */
				0,	/* Fire Resistant */
				0,	/* Electric Resistant */
				1,	/* Perfect Armour */
				0,	/* Electric Sensitive */
				0,	/* Combustability */
				0,	/* Indestructable */
			},
		},
		section_flag_right_leg|section_has_sparkoflife|section_sprays_sparks,
	},
	{
		"hnpc_xenoborg",
		"right foot",
		NULL,
		{
			270,	/* Health */
			120,	/* Armour */
			0, 		/* IsOnFire */
			{
				1,	/* Acid Resistant */
				0,	/* Fire Resistant */
				0,	/* Electric Resistant */
				1,	/* Perfect Armour */
				0,	/* Electric Sensitive */
				0,	/* Combustability */
				0,	/* Indestructable */
			},
		},
		section_flag_right_foot|section_has_sparkoflife|section_sprays_sparks,
	},
	{
		"hnpc_xenoborg",
		"left thigh",
		NULL,
		{
			270,	/* Health */
			50,		/* Armour */
			0, 		/* IsOnFire */
			{
				1,	/* Acid Resistant */
				0,	/* Fire Resistant */
				0,	/* Electric Resistant */
				1,	/* Perfect Armour */
				0,	/* Electric Sensitive */
				0,	/* Combustability */
				0,	/* Indestructable */
			},
		},
		section_flag_left_leg|section_has_sparkoflife|section_sprays_acid,
	},
	{
		"hnpc_xenoborg",
		"left shin",
		NULL,
		{
			270,	/* Health */
			50,		/* Armour */
			0, 		/* IsOnFire */
			{
				1,	/* Acid Resistant */
				0,	/* Fire Resistant */
				0,	/* Electric Resistant */
				1,	/* Perfect Armour */
				0,	/* Electric Sensitive */
				0,	/* Combustability */
				0,	/* Indestructable */
			},
		},
		section_flag_left_leg|section_has_sparkoflife|section_sprays_acid,
	},
	{
		"hnpc_xenoborg",
		"left foot",
		NULL,
		{
			270,	/* Health */
			120,	/* Armour */
			0, 		/* IsOnFire */
			{
				1,	/* Acid Resistant */
				0,	/* Fire Resistant */
				0,	/* Electric Resistant */
				1,	/* Perfect Armour */
				0,	/* Electric Sensitive */
				0,	/* Combustability */
				0,	/* Indestructable */
			},
		},
		section_flag_left_foot|section_has_sparkoflife|section_sprays_acid,
	},
	{
		"hnpc_xenoborg",
		"left foot",
		NULL,
		{
			270,	/* Health */
			120,	/* Armour */
			0, 		/* IsOnFire */
			{
				1,	/* Acid Resistant */
				0,	/* Fire Resistant */
				0,	/* Electric Resistant */
				1,	/* Perfect Armour */
				0,	/* Electric Sensitive */
				0,	/* Combustability */
				0,	/* Indestructable */
			},
		},
		section_flag_left_foot|section_has_sparkoflife|section_sprays_acid,
	},
	{
		"hnpc_xenoborg",
		"tail a",
		NULL,
		{
			50,		/* Health */
			50,		/* Armour */
			0, 		/* IsOnFire */
			{
				1,	/* Acid Resistant */
				0,	/* Fire Resistant */
				0,	/* Electric Resistant */
				1,	/* Perfect Armour */
				0,	/* Electric Sensitive */
				0,	/* Combustability */
				0,	/* Indestructable */
			},
		},
		section_flag_tail|section_flag_nofurthergibbing|section_sprays_acid,
	},
	{
		"hnpc_xenoborg",
		"tail b",
		NULL,
		{
			50,		/* Health */
			50,		/* Armour */
			0, 		/* IsOnFire */
			{
				1,	/* Acid Resistant */
				0,	/* Fire Resistant */
				0,	/* Electric Resistant */
				1,	/* Perfect Armour */
				0,	/* Electric Sensitive */
				0,	/* Combustability */
				0,	/* Indestructable */
			},
		},
		section_flag_tail|section_flag_nofurthergibbing|section_sprays_acid,
	},
	{
		"hnpc_xenoborg",
		"tail c",
		NULL,
		{
			50,		/* Health */
			50,		/* Armour */
			0, 		/* IsOnFire */
			{
				1,	/* Acid Resistant */
				0,	/* Fire Resistant */
				0,	/* Electric Resistant */
				1,	/* Perfect Armour */
				0,	/* Electric Sensitive */
				0,	/* Combustability */
				0,	/* Indestructable */
			},
		},
		section_flag_tail|section_flag_nofurthergibbing|section_sprays_acid,
	},
	{
		"hnpc_xenoborg",
		"tail d",
		NULL,
		{
			50,		/* Health */
			50,		/* Armour */
			0, 		/* IsOnFire */
			{
				1,	/* Acid Resistant */
				0,	/* Fire Resistant */
				0,	/* Electric Resistant */
				1,	/* Perfect Armour */
				0,	/* Electric Sensitive */
				0,	/* Combustability */
				0,	/* Indestructable */
			},
		},
		section_flag_tail|section_flag_nofurthergibbing|section_sprays_acid,
	},
	{
		"hnpc_xenoborg",
		"tail e",
		NULL,
		{
			50,		/* Health */
			50,		/* Armour */
			0, 		/* IsOnFire */
			{
				1,	/* Acid Resistant */
				0,	/* Fire Resistant */
				0,	/* Electric Resistant */
				1,	/* Perfect Armour */
				0,	/* Electric Sensitive */
				0,	/* Combustability */
				0,	/* Indestructable */
			},
		},
		section_flag_tail|section_flag_nofurthergibbing|section_sprays_acid,
	},
	{
		"hnpc_xenoborg",
		"tail f",
		NULL,
		{
			50,		/* Health */
			50,		/* Armour */
			0, 		/* IsOnFire */
			{
				1,	/* Acid Resistant */
				0,	/* Fire Resistant */
				0,	/* Electric Resistant */
				1,	/* Perfect Armour */
				0,	/* Electric Sensitive */
				0,	/* Combustability */
				0,	/* Indestructable */
			},
		},
		section_flag_tail|section_flag_nofurthergibbing|section_sprays_acid,
	},
	{
		"hnpc_xenoborg",
		"tail g",
		NULL,
		{
			50,		/* Health */
			50,		/* Armour */
			0, 		/* IsOnFire */
			{
				1,	/* Acid Resistant */
				0,	/* Fire Resistant */
				0,	/* Electric Resistant */
				1,	/* Perfect Armour */
				0,	/* Electric Sensitive */
				0,	/* Combustability */
				0,	/* Indestructable */
			},
		},
		section_flag_tail|section_flag_nofurthergibbing|section_sprays_acid,
	},
	{
		"hnpc_xenoborg",
		"tail h",
		NULL,
		{
			50,		/* Health */
			50,		/* Armour */
			0, 		/* IsOnFire */
			{
				1,	/* Acid Resistant */
				0,	/* Fire Resistant */
				0,	/* Electric Resistant */
				1,	/* Perfect Armour */
				0,	/* Electric Sensitive */
				0,	/* Combustability */
				0,	/* Indestructable */
			},
		},
		section_flag_tail|section_flag_nofurthergibbing|section_sprays_acid,
	},
	{
		"hnpc_xenoborg",
		"tail i",
		NULL,
		{
			50,		/* Health */
			50,		/* Armour */
			0, 		/* IsOnFire */
			{
				1,	/* Acid Resistant */
				0,	/* Fire Resistant */
				0,	/* Electric Resistant */
				1,	/* Perfect Armour */
				0,	/* Electric Sensitive */
				0,	/* Combustability */
				0,	/* Indestructable */
			},
		},
		section_flag_tail|section_flag_nofurthergibbing|section_sprays_acid,
	},
	{
		"hnpc_xenoborg",
		"tummy",
		NULL,
		{
			470,	/* Health */
			120,	/* Armour */
			0, 		/* IsOnFire */
			{
				1,	/* Acid Resistant */
				0,	/* Fire Resistant */
				0,	/* Electric Resistant */
				1,	/* Perfect Armour */
				0,	/* Electric Sensitive */
				0,	/* Combustability */
				0,	/* Indestructable */
			},
		},
		section_has_sparkoflife|section_sprays_acid|section_flag_never_frag,
	},
	{
		"hnpc_xenoborg",
		"chest",
		NULL,
		{
			500,	/* Health */
			100,	/* Armour */
			0, 		/* IsOnFire */
			{
				1,	/* Acid Resistant */
				0,	/* Fire Resistant */
				0,	/* Electric Resistant */
				1,	/* Perfect Armour */
				0,	/* Electric Sensitive */
				0,	/* Combustability */
				0,	/* Indestructable */
			},
		},
		section_has_sparkoflife|section_sprays_acid|section_flag_never_frag,
	},
	{
		"hnpc_xenoborg",
		"back pac",
		NULL,
		{
			400,	/* Health */
			120,	/* Armour */
			0, 		/* IsOnFire */
			{
				1,	/* Acid Resistant */
				0,	/* Fire Resistant */
				0,	/* Electric Resistant */
				1,	/* Perfect Armour */
				0,	/* Electric Sensitive */
				0,	/* Combustability */
				0,	/* Indestructable */
			},
		},
		section_sprays_sparks,
	},
	{
		"hnpc_xenoborg",
		"exhaust",
		NULL,
		{
			80,	/* Health */
			120,	/* Armour */
			0, 		/* IsOnFire */
			{
				1,	/* Acid Resistant */
				0,	/* Fire Resistant */
				0,	/* Electric Resistant */
				1,	/* Perfect Armour */
				0,	/* Electric Sensitive */
				0,	/* Combustability */
				0,	/* Indestructable */
			},
		},
		section_sprays_sparks,
	},
	{
		"hnpc_xenoborg",
		"pipe",
		NULL,
		{
			50,	/* Health */
			120,	/* Armour */
			0, 		/* IsOnFire */
			{
				1,	/* Acid Resistant */
				0,	/* Fire Resistant */
				0,	/* Electric Resistant */
				1,	/* Perfect Armour */
				0,	/* Electric Sensitive */
				0,	/* Combustability */
				0,	/* Indestructable */
			},
		},
		section_sprays_sparks,
	},
	{
		"hnpc_xenoborg",
		"shunt a",
		NULL,
		{
			20,	/* Health */
			120,	/* Armour */
			0, 		/* IsOnFire */
			{
				1,	/* Acid Resistant */
				0,	/* Fire Resistant */
				0,	/* Electric Resistant */
				1,	/* Perfect Armour */
				0,	/* Electric Sensitive */
				0,	/* Combustability */
				0,	/* Indestructable */
			},
		},
		section_sprays_sparks,
	},
	{
		"hnpc_xenoborg",
		"piss a",
		NULL,
		{
			20,	/* Health */
			120,	/* Armour */
			0, 		/* IsOnFire */
			{
				1,	/* Acid Resistant */
				0,	/* Fire Resistant */
				0,	/* Electric Resistant */
				1,	/* Perfect Armour */
				0,	/* Electric Sensitive */
				0,	/* Combustability */
				0,	/* Indestructable */
			},
		},
		section_sprays_sparks,
	},
	{
		"hnpc_xenoborg",
		"shunt b",
		NULL,
		{
			20,	/* Health */
			120,	/* Armour */
			0, 		/* IsOnFire */
			{
				1,	/* Acid Resistant */
				0,	/* Fire Resistant */
				0,	/* Electric Resistant */
				1,	/* Perfect Armour */
				0,	/* Electric Sensitive */
				0,	/* Combustability */
				0,	/* Indestructable */
			},
		},
		section_sprays_sparks,
	},
	{
		"hnpc_xenoborg",
		"piss b",
		NULL,
		{
			20,	/* Health */
			120,	/* Armour */
			0, 		/* IsOnFire */
			{
				1,	/* Acid Resistant */
				0,	/* Fire Resistant */
				0,	/* Electric Resistant */
				1,	/* Perfect Armour */
				0,	/* Electric Sensitive */
				0,	/* Combustability */
				0,	/* Indestructable */
			},
		},
		section_sprays_sparks,
	},
	{
		"hnpc_xenoborg",
		"shunt c",
		NULL,
		{
			20,	/* Health */
			120,	/* Armour */
			0, 		/* IsOnFire */
			{
				1,	/* Acid Resistant */
				0,	/* Fire Resistant */
				0,	/* Electric Resistant */
				1,	/* Perfect Armour */
				0,	/* Electric Sensitive */
				0,	/* Combustability */
				0,	/* Indestructable */
			},
		},
		section_sprays_sparks,
	},
	{
		"hnpc_xenoborg",
		"piss c",
		NULL,
		{
			20,	/* Health */
			120,	/* Armour */
			0, 		/* IsOnFire */
			{
				1,	/* Acid Resistant */
				0,	/* Fire Resistant */
				0,	/* Electric Resistant */
				1,	/* Perfect Armour */
				0,	/* Electric Sensitive */
				0,	/* Combustability */
				0,	/* Indestructable */
			},
		},
		section_sprays_sparks,
	},
	{
		"hnpc_xenoborg",
		"shunt d",
		NULL,
		{
			20,	/* Health */
			120,	/* Armour */
			0, 		/* IsOnFire */
			{
				1,	/* Acid Resistant */
				0,	/* Fire Resistant */
				0,	/* Electric Resistant */
				1,	/* Perfect Armour */
				0,	/* Electric Sensitive */
				0,	/* Combustability */
				0,	/* Indestructable */
			},
		},
		section_sprays_sparks,
	},
	{
		"hnpc_xenoborg",
		"piss d",
		NULL,
		{
			20,	/* Health */
			120,	/* Armour */
			0, 		/* IsOnFire */
			{
				1,	/* Acid Resistant */
				0,	/* Fire Resistant */
				0,	/* Electric Resistant */
				1,	/* Perfect Armour */
				0,	/* Electric Sensitive */
				0,	/* Combustability */
				0,	/* Indestructable */
			},
		},
		section_sprays_sparks,
	},
	{
		"hnpc_xenoborg",
		"right bicep",
		NULL,
		{
			220,	/* Health */
			50,	/* Armour */
			0, 		/* IsOnFire */
			{
				1,	/* Acid Resistant */
				0,	/* Fire Resistant */
				0,	/* Electric Resistant */
				1,	/* Perfect Armour */
				0,	/* Electric Sensitive */
				0,	/* Combustability */
				0,	/* Indestructable */
			},
		},
		section_flag_right_arm|section_sprays_acid,
	},
	{
		"hnpc_xenoborg",
		"right forearm",
		NULL,
		{
			220,	/* Health */
			50,	/* Armour */
			0, 		/* IsOnFire */
			{
				1,	/* Acid Resistant */
				0,	/* Fire Resistant */
				0,	/* Electric Resistant */
				1,	/* Perfect Armour */
				0,	/* Electric Sensitive */
				0,	/* Combustability */
				0,	/* Indestructable */
			},
		},
		section_flag_right_arm|section_sprays_acid,
	},
	{
		"hnpc_xenoborg",
		"left bicep",
		NULL,
		{
			220,	/* Health */
			120,	/* Armour */
			0, 		/* IsOnFire */
			{
				1,	/* Acid Resistant */
				0,	/* Fire Resistant */
				0,	/* Electric Resistant */
				1,	/* Perfect Armour */
				0,	/* Electric Sensitive */
				0,	/* Combustability */
				0,	/* Indestructable */
			},
		},
		section_flag_left_arm|section_sprays_sparks,
	},
	{
		"hnpc_xenoborg",
		"left forearm",
		NULL,
		{
			220,	/* Health */
			120,	/* Armour */
			0, 		/* IsOnFire */
			{
				1,	/* Acid Resistant */
				0,	/* Fire Resistant */
				0,	/* Electric Resistant */
				1,	/* Perfect Armour */
				0,	/* Electric Sensitive */
				0,	/* Combustability */
				0,	/* Indestructable */
			},
		},
		section_flag_left_arm|section_sprays_sparks,
	},
	{
		"hnpc_xenoborg",
		"neck",
		NULL,
		{
			190,		/* Health */
			120,	/* Armour */
			0, 		/* IsOnFire */
			{
				1,	/* Acid Resistant */
				0,	/* Fire Resistant */
				0,	/* Electric Resistant */
				1,	/* Perfect Armour */
				0,	/* Electric Sensitive */
				0,	/* Combustability */
				0,	/* Indestructable */
			},
		},
		section_has_sparkoflife|section_sprays_sparks,
	},
	{
		"hnpc_xenoborg",
		"head",
		NULL,
		{
			400,	/* Health */
			50,		/* Armour */
			0, 		/* IsOnFire */
			{
				1,	/* Acid Resistant */
				0,	/* Fire Resistant */
				0,	/* Electric Resistant */
				1,	/* Perfect Armour */
				0,	/* Electric Sensitive */
				0,	/* Combustability */
				0,	/* Indestructable */
			},
		},
		section_flag_head|section_has_sparkoflife, /* No longer sprays acid. */
	},
	{
		"hnpc_xenoborg",
		"sights",
		NULL,
		{
			50,		/* Health */
			120,	/* Armour */
			0, 		/* IsOnFire */
			{
				1,	/* Acid Resistant */
				0,	/* Fire Resistant */
				0,	/* Electric Resistant */
				1,	/* Perfect Armour */
				0,	/* Electric Sensitive */
				0,	/* Combustability */
				0,	/* Indestructable */
			},
		},
		section_sprays_sparks,
	},
	{
		"hnpc_xenoborg",
		"jaw",
		NULL,
		{
			50,		/* Health */
			120,	/* Armour */
			0, 		/* IsOnFire */
			{
				1,	/* Acid Resistant */
				0,	/* Fire Resistant */
				0,	/* Electric Resistant */
				1,	/* Perfect Armour */
				0,	/* Electric Sensitive */
				0,	/* Combustability */
				0,	/* Indestructable */
			},
		},
		section_sprays_sparks,
	},
	{
		"hnpc_xenoborg",
		"led",
		NULL,
		{
			20,		/* Health */
			20,		/* Armour */
			0, 		/* IsOnFire */
			{
				1,	/* Acid Resistant */
				0,	/* Fire Resistant */
				0,	/* Electric Resistant */
				1,	/* Perfect Armour */
				0,	/* Electric Sensitive */
				0,	/* Combustability */
				0,	/* Indestructable */
			},
		},
		section_has_sparkoflife|section_sprays_sparks,
	},
	/* Sentry Gun */
	{
		"sentry",
		"legs",
		NULL,
		{
			50,		/* Health */
			50,		/* Armour */
			0, 		/* IsOnFire */
			{
				1,	/* Acid Resistant */
				0,	/* Fire Resistant */
				0,	/* Electric Resistant */
				1,	/* Perfect Armour */
				0,	/* Electric Sensitive */
				0,	/* Combustability */
				0,	/* Indestructable */
			},
		},
		section_has_sparkoflife|section_sprays_sparks,
	},
	{
		"sentry",
		"pivot",
		NULL,
		{
			50,		/* Health */
			50,		/* Armour */
			0, 		/* IsOnFire */
			{
				1,	/* Acid Resistant */
				0,	/* Fire Resistant */
				0,	/* Electric Resistant */
				1,	/* Perfect Armour */
				0,	/* Electric Sensitive */
				0,	/* Combustability */
				0,	/* Indestructable */
			},
		},
		section_has_sparkoflife|section_sprays_sparks,
	},
	{
		"sentry",
		"gun",
		NULL,
		{
			50,		/* Health */
			50,		/* Armour */
			0, 		/* IsOnFire */
			{
				1,	/* Acid Resistant */
				0,	/* Fire Resistant */
				0,	/* Electric Resistant */
				1,	/* Perfect Armour */
				0,	/* Electric Sensitive */
				0,	/* Combustability */
				0,	/* Indestructable */
			},
		},
		section_has_sparkoflife|section_sprays_sparks,
	},
	/* Queen! */
	{
		"queen",
		"chest",
		NULL,
		{
			500*8,	/* Health */
			1000,	/* Armour */
			0, 		/* IsOnFire */
			{
				1,	/* Acid Resistant */
				0,	/* Fire Resistant */
				0,	/* Electric Resistant */
				1,	/* Perfect Armour */
				0,	/* Electric Sensitive */
				0,	/* Combustability */
				0,	/* Indestructable */
			},
		},
		section_has_sparkoflife|section_sprays_acid|section_flag_never_frag,
	},
	{
		"queen",
		"pelvis",
		NULL,
		{
			500*8,	/* Health */
			1000,	/* Armour */
			0, 		/* IsOnFire */
			{
				1,	/* Acid Resistant */
				0,	/* Fire Resistant */
				0,	/* Electric Resistant */
				1,	/* Perfect Armour */
				0,	/* Electric Sensitive */
				0,	/* Combustability */
				0,	/* Indestructable */
			},
		},
		section_has_sparkoflife|section_sprays_acid|section_flag_never_frag,
	},
	{
		"queen",
		"l00 tail",
		NULL,
		{
			20*8,	/* Health */
			500,	/* Armour */
			0, 		/* IsOnFire */
			{
				1,	/* Acid Resistant */
				0,	/* Fire Resistant */
				0,	/* Electric Resistant */
				1,	/* Perfect Armour */
				0,	/* Electric Sensitive */
				0,	/* Combustability */
				0,	/* Indestructable */
			},
		},
		section_sprays_acid,
	},
	{
		"queen",
		"l01 tail",
		NULL,
		{
			20*8,	/* Health */
			500,	/* Armour */
			0, 		/* IsOnFire */
			{
				1,	/* Acid Resistant */
				0,	/* Fire Resistant */
				0,	/* Electric Resistant */
				1,	/* Perfect Armour */
				0,	/* Electric Sensitive */
				0,	/* Combustability */
				0,	/* Indestructable */
			},
		},
		section_sprays_acid,
	},
	{
		"queen",
		"l02 tail",
		NULL,
		{
			20*8,	/* Health */
			500,	/* Armour */
			0, 		/* IsOnFire */
			{
				1,	/* Acid Resistant */
				0,	/* Fire Resistant */
				0,	/* Electric Resistant */
				1,	/* Perfect Armour */
				0,	/* Electric Sensitive */
				0,	/* Combustability */
				0,	/* Indestructable */
			},
		},
		section_sprays_acid,
	},
	{
		"queen",
		"l03 tail",
		NULL,
		{
			20*8,	/* Health */
			500,	/* Armour */
			0, 		/* IsOnFire */
			{
				1,	/* Acid Resistant */
				0,	/* Fire Resistant */
				0,	/* Electric Resistant */
				1,	/* Perfect Armour */
				0,	/* Electric Sensitive */
				0,	/* Combustability */
				0,	/* Indestructable */
			},
		},
		section_sprays_acid,
	},
	{
		"queen",
		"l04 tail",
		NULL,
		{
			20*8,	/* Health */
			500,	/* Armour */
			0, 		/* IsOnFire */
			{
				1,	/* Acid Resistant */
				0,	/* Fire Resistant */
				0,	/* Electric Resistant */
				1,	/* Perfect Armour */
				0,	/* Electric Sensitive */
				0,	/* Combustability */
				0,	/* Indestructable */
			},
		},
		section_sprays_acid,
	},
	{
		"queen",
		"l05 tail",
		NULL,
		{
			20*8,	/* Health */
			500,	/* Armour */
			0, 		/* IsOnFire */
			{
				1,	/* Acid Resistant */
				0,	/* Fire Resistant */
				0,	/* Electric Resistant */
				1,	/* Perfect Armour */
				0,	/* Electric Sensitive */
				0,	/* Combustability */
				0,	/* Indestructable */
			},
		},
		section_sprays_acid,
	},
	{
		"queen",
		"l06 tail",
		NULL,
		{
			20*8,	/* Health */
			500,	/* Armour */
			0, 		/* IsOnFire */
			{
				1,	/* Acid Resistant */
				0,	/* Fire Resistant */
				0,	/* Electric Resistant */
				1,	/* Perfect Armour */
				0,	/* Electric Sensitive */
				0,	/* Combustability */
				0,	/* Indestructable */
			},
		},
		section_sprays_acid,
	},
	{
		"queen",
		"l07 tail",
		NULL,
		{
			20*8,	/* Health */
			500,	/* Armour */
			0, 		/* IsOnFire */
			{
				1,	/* Acid Resistant */
				0,	/* Fire Resistant */
				0,	/* Electric Resistant */
				1,	/* Perfect Armour */
				0,	/* Electric Sensitive */
				0,	/* Combustability */
				0,	/* Indestructable */
			},
		},
		section_sprays_acid,
	},
	{
		"queen",
		"l08 tail",
		NULL,
		{
			20*8,	/* Health */
			500,	/* Armour */
			0, 		/* IsOnFire */
			{
				1,	/* Acid Resistant */
				0,	/* Fire Resistant */
				0,	/* Electric Resistant */
				1,	/* Perfect Armour */
				0,	/* Electric Sensitive */
				0,	/* Combustability */
				0,	/* Indestructable */
			},
		},
		section_sprays_acid,
	},
	{
		"queen",
		"l09 tail",
		NULL,
		{
			20*8,	/* Health */
			500,	/* Armour */
			0, 		/* IsOnFire */
			{
				1,	/* Acid Resistant */
				0,	/* Fire Resistant */
				0,	/* Electric Resistant */
				1,	/* Perfect Armour */
				0,	/* Electric Sensitive */
				0,	/* Combustability */
				0,	/* Indestructable */
			},
		},
		section_sprays_acid,
	},
	{
		"queen",
		"l10 tail",
		NULL,
		{
			20*8,	/* Health */
			500,	/* Armour */
			0, 		/* IsOnFire */
			{
				1,	/* Acid Resistant */
				0,	/* Fire Resistant */
				0,	/* Electric Resistant */
				1,	/* Perfect Armour */
				0,	/* Electric Sensitive */
				0,	/* Combustability */
				0,	/* Indestructable */
			},
		},
		section_sprays_acid,
	},
	{
		"queen",
		"l11 tail",
		NULL,
		{
			20*8,	/* Health */
			500,	/* Armour */
			0, 		/* IsOnFire */
			{
				1,	/* Acid Resistant */
				0,	/* Fire Resistant */
				0,	/* Electric Resistant */
				1,	/* Perfect Armour */
				0,	/* Electric Sensitive */
				0,	/* Combustability */
				0,	/* Indestructable */
			},
		},
		section_sprays_acid,
	},
	{
		"queen",
		"tummy",
		NULL,
		{
			500*8,	/* Health */
			1000,	/* Armour */
			0, 		/* IsOnFire */
			{
				1,	/* Acid Resistant */
				0,	/* Fire Resistant */
				0,	/* Electric Resistant */
				1,	/* Perfect Armour */
				0,	/* Electric Sensitive */
				0,	/* Combustability */
				0,	/* Indestructable */
			},
		},
		section_has_sparkoflife|section_sprays_acid|section_flag_never_frag,
	},
	{
		"queen",
		"right bicep",
		NULL,
		{
			300*8,	/* Health */
			1000,	/* Armour */
			0, 		/* IsOnFire */
			{
				1,	/* Acid Resistant */
				0,	/* Fire Resistant */
				0,	/* Electric Resistant */
				1,	/* Perfect Armour */
				0,	/* Electric Sensitive */
				0,	/* Combustability */
				0,	/* Indestructable */
			},
		},
		section_sprays_acid|section_flag_never_frag,
	},
	{
		"queen",
		"right fore arm",
		NULL,
		{
			300*8,	/* Health */
			1000,	/* Armour */
			0, 		/* IsOnFire */
			{
				1,	/* Acid Resistant */
				0,	/* Fire Resistant */
				0,	/* Electric Resistant */
				1,	/* Perfect Armour */
				0,	/* Electric Sensitive */
				0,	/* Combustability */
				0,	/* Indestructable */
			},
		},
		section_sprays_acid|section_flag_never_frag,
	},
	{
		"queen",
		"right hand",
		NULL,
		{
			300*8,	/* Health */
			1000,	/* Armour */
			0, 		/* IsOnFire */
			{
				1,	/* Acid Resistant */
				0,	/* Fire Resistant */
				0,	/* Electric Resistant */
				1,	/* Perfect Armour */
				0,	/* Electric Sensitive */
				0,	/* Combustability */
				0,	/* Indestructable */
			},
		},
		section_sprays_acid|section_flag_never_frag,
	},
	{
		"queen",
		"r fing",
		NULL,
		{
			20*8,	/* Health */
			100,	/* Armour */
			0, 		/* IsOnFire */
			{
				1,	/* Acid Resistant */
				0,	/* Fire Resistant */
				0,	/* Electric Resistant */
				1,	/* Perfect Armour */
				0,	/* Electric Sensitive */
				0,	/* Combustability */
				0,	/* Indestructable */
			},
		},
		section_sprays_acid,
	},
	{
		"queen",
		"rr fing",
		NULL,
		{
			20*8,	/* Health */
			100,	/* Armour */
			0, 		/* IsOnFire */
			{
				1,	/* Acid Resistant */
				0,	/* Fire Resistant */
				0,	/* Electric Resistant */
				1,	/* Perfect Armour */
				0,	/* Electric Sensitive */
				0,	/* Combustability */
				0,	/* Indestructable */
			},
		},
		section_sprays_acid,
	},
	{
		"queen",
		"rrr fing",
		NULL,
		{
			20*8,	/* Health */
			100,	/* Armour */
			0, 		/* IsOnFire */
			{
				1,	/* Acid Resistant */
				0,	/* Fire Resistant */
				0,	/* Electric Resistant */
				1,	/* Perfect Armour */
				0,	/* Electric Sensitive */
				0,	/* Combustability */
				0,	/* Indestructable */
			},
		},
		section_sprays_acid,
	},
	{
		"queen",
		"rrrr fing",
		NULL,
		{
			20*8,	/* Health */
			100,	/* Armour */
			0, 		/* IsOnFire */
			{
				1,	/* Acid Resistant */
				0,	/* Fire Resistant */
				0,	/* Electric Resistant */
				1,	/* Perfect Armour */
				0,	/* Electric Sensitive */
				0,	/* Combustability */
				0,	/* Indestructable */
			},
		},
		section_sprays_acid,
	},
	{
		"queen",
		"rrrrr fing",
		NULL,
		{
			20*8,	/* Health */
			100,	/* Armour */
			0, 		/* IsOnFire */
			{
				1,	/* Acid Resistant */
				0,	/* Fire Resistant */
				0,	/* Electric Resistant */
				1,	/* Perfect Armour */
				0,	/* Electric Sensitive */
				0,	/* Combustability */
				0,	/* Indestructable */
			},
		},
		section_sprays_acid,
	},
	{
		"queen",
		"rrrrrr fing",
		NULL,
		{
			20*8,	/* Health */
			100,	/* Armour */
			0, 		/* IsOnFire */
			{
				1,	/* Acid Resistant */
				0,	/* Fire Resistant */
				0,	/* Electric Resistant */
				1,	/* Perfect Armour */
				0,	/* Electric Sensitive */
				0,	/* Combustability */
				0,	/* Indestructable */
			},
		},
		section_sprays_acid,
	},
	{
		"queen",
		"left bicep",
		NULL,
		{
			300*8,	/* Health */
			1000,	/* Armour */
			0, 		/* IsOnFire */
			{
				1,	/* Acid Resistant */
				0,	/* Fire Resistant */
				0,	/* Electric Resistant */
				1,	/* Perfect Armour */
				0,	/* Electric Sensitive */
				0,	/* Combustability */
				0,	/* Indestructable */
			},
		},
		section_sprays_acid|section_flag_never_frag,
	},
	{
		"queen",
		"left fore arm",
		NULL,
		{
			300*8,	/* Health */
			1000,	/* Armour */
			0, 		/* IsOnFire */
			{
				1,	/* Acid Resistant */
				0,	/* Fire Resistant */
				0,	/* Electric Resistant */
				1,	/* Perfect Armour */
				0,	/* Electric Sensitive */
				0,	/* Combustability */
				0,	/* Indestructable */
			},
		},
		section_sprays_acid|section_flag_never_frag,
	},
	{
		"queen",
		"left hand",
		NULL,
		{
			300*8,	/* Health */
			1000,	/* Armour */
			0, 		/* IsOnFire */
			{
				1,	/* Acid Resistant */
				0,	/* Fire Resistant */
				0,	/* Electric Resistant */
				1,	/* Perfect Armour */
				0,	/* Electric Sensitive */
				0,	/* Combustability */
				0,	/* Indestructable */
			},
		},
		section_sprays_acid|section_flag_never_frag,
	},
	{
		"queen",
		"l fing",
		NULL,
		{
			20*8,	/* Health */
			100,	/* Armour */
			0, 		/* IsOnFire */
			{
				1,	/* Acid Resistant */
				0,	/* Fire Resistant */
				0,	/* Electric Resistant */
				1,	/* Perfect Armour */
				0,	/* Electric Sensitive */
				0,	/* Combustability */
				0,	/* Indestructable */
			},
		},
		section_sprays_acid,
	},
	{
		"queen",
		"ll fing",
		NULL,
		{
			20*8,	/* Health */
			100,	/* Armour */
			0, 		/* IsOnFire */
			{
				1,	/* Acid Resistant */
				0,	/* Fire Resistant */
				0,	/* Electric Resistant */
				1,	/* Perfect Armour */
				0,	/* Electric Sensitive */
				0,	/* Combustability */
				0,	/* Indestructable */
			},
		},
		section_sprays_acid,
	},
	{
		"queen",
		"lll fing",
		NULL,
		{
			20*8,	/* Health */
			100,	/* Armour */
			0, 		/* IsOnFire */
			{
				1,	/* Acid Resistant */
				0,	/* Fire Resistant */
				0,	/* Electric Resistant */
				1,	/* Perfect Armour */
				0,	/* Electric Sensitive */
				0,	/* Combustability */
				0,	/* Indestructable */
			},
		},
		section_sprays_acid,
	},
	{
		"queen",
		"llll fing",
		NULL,
		{
			20*8,	/* Health */
			100,	/* Armour */
			0, 		/* IsOnFire */
			{
				1,	/* Acid Resistant */
				0,	/* Fire Resistant */
				0,	/* Electric Resistant */
				1,	/* Perfect Armour */
				0,	/* Electric Sensitive */
				0,	/* Combustability */
				0,	/* Indestructable */
			},
		},
		section_sprays_acid,
	},
	{
		"queen",
		"lllll fing",
		NULL,
		{
			20*8,	/* Health */
			100,	/* Armour */
			0, 		/* IsOnFire */
			{
				1,	/* Acid Resistant */
				0,	/* Fire Resistant */
				0,	/* Electric Resistant */
				1,	/* Perfect Armour */
				0,	/* Electric Sensitive */
				0,	/* Combustability */
				0,	/* Indestructable */
			},
		},
		section_sprays_acid,
	},
	{
		"queen",
		"llllll fing",
		NULL,
		{
			20*8,	/* Health */
			100,	/* Armour */
			0, 		/* IsOnFire */
			{
				1,	/* Acid Resistant */
				0,	/* Fire Resistant */
				0,	/* Electric Resistant */
				1,	/* Perfect Armour */
				0,	/* Electric Sensitive */
				0,	/* Combustability */
				0,	/* Indestructable */
			},
		},
		section_sprays_acid,
	},

	{
		"queen",
		"right thigh",
		NULL,
		{
			400*8,	/* Health */
			1000,	/* Armour */
			0, 		/* IsOnFire */
			{
				1,	/* Acid Resistant */
				0,	/* Fire Resistant */
				0,	/* Electric Resistant */
				1,	/* Perfect Armour */
				0,	/* Electric Sensitive */
				0,	/* Combustability */
				0,	/* Indestructable */
			},
		},
		section_sprays_acid|section_flag_never_frag,
	},
	{
		"queen",
		"right shin",
		NULL,
		{
			400*8,	/* Health */
			1000,	/* Armour */
			0, 		/* IsOnFire */
			{
				1,	/* Acid Resistant */
				0,	/* Fire Resistant */
				0,	/* Electric Resistant */
				1,	/* Perfect Armour */
				0,	/* Electric Sensitive */
				0,	/* Combustability */
				0,	/* Indestructable */
			},
		},
		section_sprays_acid|section_flag_never_frag,
	},
	{
		"queen",
		"right bird",
		NULL,
		{
			400*8,	/* Health */
			1000,	/* Armour */
			0, 		/* IsOnFire */
			{
				1,	/* Acid Resistant */
				0,	/* Fire Resistant */
				0,	/* Electric Resistant */
				1,	/* Perfect Armour */
				0,	/* Electric Sensitive */
				0,	/* Combustability */
				0,	/* Indestructable */
			},
		},
		section_sprays_acid|section_flag_never_frag,
	},
	{
		"queen",
		"right foot",
		NULL,
		{
			400*8,	/* Health */
			1000,	/* Armour */
			0, 		/* IsOnFire */
			{
				1,	/* Acid Resistant */
				0,	/* Fire Resistant */
				0,	/* Electric Resistant */
				1,	/* Perfect Armour */
				0,	/* Electric Sensitive */
				0,	/* Combustability */
				0,	/* Indestructable */
			},
		},
		section_sprays_acid|section_flag_never_frag,
	},
	{
		"queen",
		"left thigh",
		NULL,
		{
			400*8,	/* Health */
			1000,	/* Armour */
			0, 		/* IsOnFire */
			{
				1,	/* Acid Resistant */
				0,	/* Fire Resistant */
				0,	/* Electric Resistant */
				1,	/* Perfect Armour */
				0,	/* Electric Sensitive */
				0,	/* Combustability */
				0,	/* Indestructable */
			},
		},
		section_sprays_acid|section_flag_never_frag,
	},
	{
		"queen",
		"left shin",
		NULL,
		{
			400*8,	/* Health */
			1000,	/* Armour */
			0, 		/* IsOnFire */
			{
				1,	/* Acid Resistant */
				0,	/* Fire Resistant */
				0,	/* Electric Resistant */
				1,	/* Perfect Armour */
				0,	/* Electric Sensitive */
				0,	/* Combustability */
				0,	/* Indestructable */
			},
		},
		section_sprays_acid|section_flag_never_frag,
	},
	{
		"queen",
		"left bird",
		NULL,
		{
			400*8,	/* Health */
			1000,	/* Armour */
			0, 		/* IsOnFire */
			{
				1,	/* Acid Resistant */
				0,	/* Fire Resistant */
				0,	/* Electric Resistant */
				1,	/* Perfect Armour */
				0,	/* Electric Sensitive */
				0,	/* Combustability */
				0,	/* Indestructable */
			},
		},
		section_sprays_acid|section_flag_never_frag,
	},
	{
		"queen",
		"left foot",
		NULL,
		{
			400*8,	/* Health */
			1000,	/* Armour */
			0, 		/* IsOnFire */
			{
				1,	/* Acid Resistant */
				0,	/* Fire Resistant */
				0,	/* Electric Resistant */
				1,	/* Perfect Armour */
				0,	/* Electric Sensitive */
				0,	/* Combustability */
				0,	/* Indestructable */
			},
		},
		section_sprays_acid|section_flag_never_frag,
	},
	{
		"queen",
		"lbx",
		NULL,
		{
			50*8,	/* Health */
			500,	/* Armour */
			0, 		/* IsOnFire */
			{
				1,	/* Acid Resistant */
				0,	/* Fire Resistant */
				0,	/* Electric Resistant */
				1,	/* Perfect Armour */
				0,	/* Electric Sensitive */
				0,	/* Combustability */
				0,	/* Indestructable */
			},
		},
		section_sprays_acid,
	},
	{
		"queen",
		"lfx",
		NULL,
		{
			50*8,	/* Health */
			500,	/* Armour */
			0, 		/* IsOnFire */
			{
				1,	/* Acid Resistant */
				0,	/* Fire Resistant */
				0,	/* Electric Resistant */
				1,	/* Perfect Armour */
				0,	/* Electric Sensitive */
				0,	/* Combustability */
				0,	/* Indestructable */
			},
		},
		section_sprays_acid,
	},
	{
		"queen",
		"lhx",
		NULL,
		{
			50*8,	/* Health */
			500,	/* Armour */
			0, 		/* IsOnFire */
			{
				1,	/* Acid Resistant */
				0,	/* Fire Resistant */
				0,	/* Electric Resistant */
				1,	/* Perfect Armour */
				0,	/* Electric Sensitive */
				0,	/* Combustability */
				0,	/* Indestructable */
			},
		},
		section_sprays_acid,
	},
	{
		"queen",
		"rbx",
		NULL,
		{
			50*8,	/* Health */
			500,	/* Armour */
			0, 		/* IsOnFire */
			{
				1,	/* Acid Resistant */
				0,	/* Fire Resistant */
				0,	/* Electric Resistant */
				1,	/* Perfect Armour */
				0,	/* Electric Sensitive */
				0,	/* Combustability */
				0,	/* Indestructable */
			},
		},
		section_sprays_acid,
	},
	{
		"queen",
		"rfx",
		NULL,
		{
			50*8,	/* Health */
			500,	/* Armour */
			0, 		/* IsOnFire */
			{
				1,	/* Acid Resistant */
				0,	/* Fire Resistant */
				0,	/* Electric Resistant */
				1,	/* Perfect Armour */
				0,	/* Electric Sensitive */
				0,	/* Combustability */
				0,	/* Indestructable */
			},
		},
		section_sprays_acid,
	},
	{
		"queen",
		"rhx",
		NULL,
		{
			50*8,	/* Health */
			500,	/* Armour */
			0, 		/* IsOnFire */
			{
				1,	/* Acid Resistant */
				0,	/* Fire Resistant */
				0,	/* Electric Resistant */
				1,	/* Perfect Armour */
				0,	/* Electric Sensitive */
				0,	/* Combustability */
				0,	/* Indestructable */
			},
		},
		section_sprays_acid,
	},
	{
		"queen",
		"s",
		NULL,
		{
			10*8,	/* Health */
			50,	/* Armour */
			0, 		/* IsOnFire */
			{
				1,	/* Acid Resistant */
				0,	/* Fire Resistant */
				0,	/* Electric Resistant */
				1,	/* Perfect Armour */
				0,	/* Electric Sensitive */
				0,	/* Combustability */
				0,	/* Indestructable */
			},
		},
		section_sprays_acid,
	},
	{
		"queen",
		"ss",
		NULL,
		{
			10*8,	/* Health */
			50,	/* Armour */
			0, 		/* IsOnFire */
			{
				1,	/* Acid Resistant */
				0,	/* Fire Resistant */
				0,	/* Electric Resistant */
				1,	/* Perfect Armour */
				0,	/* Electric Sensitive */
				0,	/* Combustability */
				0,	/* Indestructable */
			},
		},
		section_sprays_acid,
	},
	{
		"queen",
		"sss",
		NULL,
		{
			10*8,	/* Health */
			50,	/* Armour */
			0, 		/* IsOnFire */
			{
				1,	/* Acid Resistant */
				0,	/* Fire Resistant */
				0,	/* Electric Resistant */
				1,	/* Perfect Armour */
				0,	/* Electric Sensitive */
				0,	/* Combustability */
				0,	/* Indestructable */
			},
		},
		section_sprays_acid,
	},
	{
		"queen",
		"ssss",
		NULL,
		{
			10*8,	/* Health */
			50,	/* Armour */
			0, 		/* IsOnFire */
			{
				1,	/* Acid Resistant */
				0,	/* Fire Resistant */
				0,	/* Electric Resistant */
				1,	/* Perfect Armour */
				0,	/* Electric Sensitive */
				0,	/* Combustability */
				0,	/* Indestructable */
			},
		},
		section_sprays_acid,
	},
	{
		"queen",
		"sssss",
		NULL,
		{
			10*8,	/* Health */
			50,	/* Armour */
			0, 		/* IsOnFire */
			{
				1,	/* Acid Resistant */
				0,	/* Fire Resistant */
				0,	/* Electric Resistant */
				1,	/* Perfect Armour */
				0,	/* Electric Sensitive */
				0,	/* Combustability */
				0,	/* Indestructable */
			},
		},
		section_sprays_acid,
	},
	{
		"queen",
		"ssssss",
		NULL,
		{
			10*8,	/* Health */
			50,	/* Armour */
			0, 		/* IsOnFire */
			{
				1,	/* Acid Resistant */
				0,	/* Fire Resistant */
				0,	/* Electric Resistant */
				1,	/* Perfect Armour */
				0,	/* Electric Sensitive */
				0,	/* Combustability */
				0,	/* Indestructable */
			},
		},
		section_sprays_acid,
	},
	{
		"queen",
		"neck",
		NULL,
		{
			500*8,	/* Health */
			1000,	/* Armour */
			0, 		/* IsOnFire */
			{
				1,	/* Acid Resistant */
				0,	/* Fire Resistant */
				0,	/* Electric Resistant */
				1,	/* Perfect Armour */
				0,	/* Electric Sensitive */
				0,	/* Combustability */
				0,	/* Indestructable */
			},
		},
		section_sprays_acid|section_flag_never_frag,
	},
	{
		"queen",
		"pippa",
		NULL,
		{
			500*8,	/* Health */
			1000,	/* Armour */
			0, 		/* IsOnFire */
			{
				1,	/* Acid Resistant */
				0,	/* Fire Resistant */
				0,	/* Electric Resistant */
				1,	/* Perfect Armour */
				0,	/* Electric Sensitive */
				0,	/* Combustability */
				0,	/* Indestructable */
			},
		},
		section_sprays_acid|section_flag_never_frag,
	},
	{
		"queen",
		"face",
		NULL,
		{
			200*8,	/* Health */
			1000,	/* Armour */
			0, 		/* IsOnFire */
			{
				1,	/* Acid Resistant */
				0,	/* Fire Resistant */
				0,	/* Electric Resistant */
				1,	/* Perfect Armour */
				0,	/* Electric Sensitive */
				0,	/* Combustability */
				0,	/* Indestructable */
			},
		},
		section_has_sparkoflife|section_sprays_acid|section_flag_never_frag,
	},
	{
		"queen",
		"jaw",
		NULL,
		{
			50*8,	/* Health */
			1000,	/* Armour */
			0, 		/* IsOnFire */
			{
				1,	/* Acid Resistant */
				0,	/* Fire Resistant */
				0,	/* Electric Resistant */
				1,	/* Perfect Armour */
				0,	/* Electric Sensitive */
				0,	/* Combustability */
				0,	/* Indestructable */
			},
		},
		section_sprays_acid,
	},
	{
		"queen",
		"luke",
		NULL,
		{
			100*8,	/* Health */
			1000,	/* Armour */
			0, 		/* IsOnFire */
			{
				1,	/* Acid Resistant */
				0,	/* Fire Resistant */
				0,	/* Electric Resistant */
				1,	/* Perfect Armour */
				0,	/* Electric Sensitive */
				0,	/* Combustability */
				0,	/* Indestructable */
			},
		},
		section_sprays_acid,
		
	},
	/* Terminator. */
	{
		NULL,
		NULL,
		NULL,
		{
			0,	/* Health */
			0,		/* Armour */
			0, /* IsOnFire */
			{
				0,	/* Acid Resistant */
				0,	/* Fire Resistant */
				0,	/* Electric Resistant */
				1,	/* Perfect Armour */
				0,	/* Electric Sensitive */
				0,	/* Combustability */
				0,	/* Indestructable */
			},
		},
		0,
	},
};

SECTION_ATTACHMENT *GetThisSectionAttachment(char *riffname,char *section_name,char *hierarchy_name) {
	
	int a;
	SECTION_ATTACHMENT *result;

	a=0;
	result=NULL;	

	if (riffname==NULL) return(NULL);
	if (section_name==NULL) return(NULL);

	while (Global_Section_Attachments[a].Riffname!=NULL) {
		if (strcmp(riffname,Global_Section_Attachments[a].Riffname)==0) {
			if (strcmp(section_name,Global_Section_Attachments[a].Section_Name)==0) {
				/* If hierarchy_name is provided, it must match. */
				if (hierarchy_name) {
					if (Global_Section_Attachments[a].Hierarchy_Name) {
						char **this_name;
						/* Now, this is an array... */
						this_name=Global_Section_Attachments[a].Hierarchy_Name;
						while (*this_name) {
							if (strcmp(hierarchy_name,(*this_name))==0) {
								result=&Global_Section_Attachments[a];
								break;
							}
							this_name++;
						}
					}
				} else {
					result=&Global_Section_Attachments[a];
					break;
				}
			}
		}
		a++;
	}

	return(result);

}

