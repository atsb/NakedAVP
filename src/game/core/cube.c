#include <stdio.h>
#include <inttypes.h>

#include "system.h"
#include "shape.h"

/*

CUBE_

*/

/*

Prototype CUBE Data 

*/

#define PLAYER_RADIUS 300
#define cube_scale 1/40

#define CUBE_PolyType I_Polygon

#define CUBE_colour1 col24(255,16,16)

int *CUBE_points[];
int *CUBE_normals[];
int *CUBE_vnormals[];
int *CUBE_items[];

SHAPEINSTR CUBE_instructions[];

int CUBE_points0[]; 

int CUBE_normals0[]; 

int CUBE_vnormals0[]; 

/* Items Data */
int CUBE_item0[];
int CUBE_item1[];
int CUBE_item2[];
int CUBE_item3[];
int CUBE_item4[];
int CUBE_item5[];


SHAPEHEADER CUBE_header={
	8,
	6,
	0,
	&CUBE_points[0],
	&CUBE_items[0],
	&CUBE_normals[0],
	&CUBE_vnormals[0],
	0,
	0,
	0, 0, 0,


	2000,
	PLAYER_RADIUS,
	-PLAYER_RADIUS,
	0,
	-2000,
	PLAYER_RADIUS,
	-PLAYER_RADIUS,
	&CUBE_instructions[0],
	0
};

SHAPEINSTR CUBE_instructions[]={
    {I_ShapePoints,8,&CUBE_points[0]},
    {I_ShapeNormals,6,&CUBE_normals[0]},
    {I_ShapeProject,8,&CUBE_points[0]},
    {I_ShapeVNormals,8,&CUBE_vnormals[0]},
    {I_ShapeItems,6,&CUBE_items[0]},
    {I_ShapeEnd,0,0}
};

int *CUBE_points[]={
	&CUBE_points0[0]
};

int CUBE_points0[]={
	PLAYER_RADIUS,        -2000,       -PLAYER_RADIUS,
	PLAYER_RADIUS,        0,        -PLAYER_RADIUS,
	-PLAYER_RADIUS,       0,        -PLAYER_RADIUS,
	-PLAYER_RADIUS,       -2000,       -PLAYER_RADIUS,
	PLAYER_RADIUS,        -2000,       PLAYER_RADIUS,
	PLAYER_RADIUS,        0,        PLAYER_RADIUS,
	-PLAYER_RADIUS,       0,        PLAYER_RADIUS,
	-PLAYER_RADIUS,       -2000,       PLAYER_RADIUS
};

int *CUBE_normals[]={
	&CUBE_normals0[0]
};

int CUBE_normals0[]={
	65536,0,0,
	0,65535,0,
	-65535,0,0,
	0,-65535,0,
	0,0,65535,
	0,0,-65535
};

int *CUBE_vnormals[]={
	&CUBE_vnormals0[0]
};

int CUBE_vnormals0[]={
	37837,-37837,-37837,
	37837,37837,-37837,
	-37837,37837,-37837,
	-37837,-37837,-37837,
	37837,-37837,37837,
	37837,37837,37837,
	-37837,37837,37837,
	-37837,-37837,37837
};


int *CUBE_items[]={
	&CUBE_item0[0],
	&CUBE_item1[0],
	&CUBE_item2[0],
	&CUBE_item3[0],
	&CUBE_item4[0],
	&CUBE_item5[0]
};


 int CUBE_item0[]={
	CUBE_PolyType,0*3,0,CUBE_colour1,
	0*1,1*1,5*1,4*1,
	Term
};
 int CUBE_item1[]={
	CUBE_PolyType,1*3,0,CUBE_colour1,
	1*1,2*1,6*1,5*1,
	Term
};
 int CUBE_item2[]={
	CUBE_PolyType,2*3,0,CUBE_colour1,
	2*1,3*1,7*1,6*1,
	Term
};
 int CUBE_item3[]={
	CUBE_PolyType,3*3,0,CUBE_colour1,
	3*1,0*1,4*1,7*1,
	Term
};
 int CUBE_item4[]={
	CUBE_PolyType,4*3,0,CUBE_colour1,
	6*1,7*1,4*1,5*1,
	Term
};
 int CUBE_item5[]={
	CUBE_PolyType,5*3,0,CUBE_colour1,
	2*1,1*1,0*1,3*1,
	Term
};
