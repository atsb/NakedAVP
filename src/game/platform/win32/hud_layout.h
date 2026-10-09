/* KJL 17:58:47 18/04/98 - layout defines */

#define HUD_FONT_WIDTH 15
//5
#define HUD_FONT_HEIGHT 15
//8
#define HUD_DIGITAL_NUMBERS_WIDTH 14
#define HUD_DIGITAL_NUMBERS_HEIGHT 22

#define HUDLayout_RightmostTextCentre	-40

#define	HUDLayout_Health_TopY			10
#define	HUDLayout_Armour_TopY			60

/* KJL 15:28:12 09/06/98 - the following are pixels from the bottom of the screen */
#define HUDLayout_Rounds_TopY			40
#define HUDLayout_Magazines_TopY 		90
#define HUDLayout_AmmoDesc_TopY 		115


#define HUDLayout_Colour_BrightWhite	0xffffffff
#define HUDLayout_Colour_MarineGreen	((255<<24)+(95<<16)+(179<<8)+(39))
#define HUDLayout_Colour_MarineRed	((255<<24)+(255<<16))
#define HUDLayout_Linespacing			16

#ifdef __cplusplus
extern "C"
{
#endif
extern char AAFontWidths[];
#ifdef __cplusplus
};
#endif
