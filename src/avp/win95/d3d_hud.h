#ifndef __AVP_WIN95_D3D_HUD_H__
#define __AVP_WIN95_D3D_HUD_H__

#include "langenum.h"

extern int TransparencyLevelOfHUD;
/* takes values of 0 (not drawn) to 255 (opaque) */

struct VertexTag
{
	int X;
	int Y;
	int U;
	int V;
};

typedef struct
{
	int ImageNumber;

	int TopLeftX;
	int TopLeftY;
	
	int TopLeftU;
	int TopLeftV;

	int Width;
	int Height;

	int Scale;

	unsigned char Red;
	unsigned char Green;
	unsigned char Blue;
	unsigned char Translucency; /* 0 == invisible, 255 == opaque */ 

} HUDImageDesc;


typedef struct
{
	char Character;
	int X;
	int Y;

	unsigned char Red;
	unsigned char Green;
	unsigned char Blue;
	unsigned char Alpha;

} HUDCharDesc;

extern void D3D_HUD_Setup(void);
extern void D3D_HUDQuad_Output(int imageNumber,struct VertexTag *quadVerticesPtr,unsigned int colour);
 
extern void D3D_DrawHUDFontCharacter(HUDCharDesc *charDescPtr);
extern void D3D_DrawHUDDigit(HUDCharDesc *charDescPtr);


extern void D3D_BlitWhiteChar(int x, int y, unsigned char c);



/* KJL 17:55:48 18/04/98 - called by HUD.c */
extern void Render_HealthAndArmour(unsigned int health, unsigned int armour);
extern void Render_MarineAmmo(enum TEXTSTRING_ID ammoText, enum TEXTSTRING_ID magazinesText, unsigned int magazines, enum TEXTSTRING_ID roundsText, unsigned int rounds, int primaryAmmo);
void D3D_DrawHUDPredatorDigit(HUDCharDesc *charDescPtr, int scale);
void Draw_HUDImage(HUDImageDesc *imageDescPtr);

#endif
