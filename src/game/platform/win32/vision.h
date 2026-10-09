#ifndef _included_vision_h_
#define _included_vision_h_

#ifdef __cplusplus
extern "C" {
#endif

//#include "d3_func.h"

/*KJL****************************************************************************************
*                                    P R O T O T Y P E S	                                *
****************************************************************************************KJL*/
enum PREDOVISION_ID
{
	PREDOVISION_NORMAL,
	PREDOVISION_SPECTRUM
};
struct PredOVisionDescriptor
{
	enum PREDOVISION_ID	VisionMode;
	int 				FadeLevel;
	unsigned char		VisionIsChanging :1;
	unsigned char		FadingToWhite :1;
};

enum MARINEOVISION_ID
{
	MARINEOVISION_NORMAL,
	MARINEOVISION_INTENSIFIED
};
struct MarineOVisionDescriptor
{
	enum MARINEOVISION_ID	VisionMode;
	int 					FadeLevel;
	unsigned char			VisionIsChanging :1;
	unsigned char			FadingToWhite :1;
};

/* JH 29/5/97 - define a structure to control how D3D does the lighting */
enum D3DLCC_Mode
{
	LCCM_NORMAL, /* default behaviour, white light - all other parms ignored */
	LCCM_CONSTCOLOUR, /* r,g,b specify a colour of a light to use instead of white */
	LCCM_FUNCTIONOFINTENSITY /* GetR, GetG, GetB are function pointers to get r,g,b as function of intensity */
};
struct D3DLightColourControl
{
	enum D3DLCC_Mode ctrl;

	/* these should be in [0..ONE_FIXED] */
	int r;
	int g;
	int b;

	/* these should return from [0..255]
	   their parameters are [0..255] */
	int (*GetR)(int);
	int (*GetG)(int);
	int (*GetB)(int);
};
extern struct D3DLightColourControl d3d_light_ctrl;

/* KJL 17:07:29 10/02/98 - all new vision modes */
enum VISION_MODE_ID
{
	VISION_MODE_NORMAL,
	VISION_MODE_ALIEN_SENSE,

	VISION_MODE_IMAGEINTENSIFIER,
	VISION_MODE_PRED_THERMAL,
	VISION_MODE_PRED_SEEALIENS,
	VISION_MODE_PRED_SEEPREDTECH,

	NUMBER_OF_VISION_MODES
};

extern enum VISION_MODE_ID CurrentVisionMode;



extern struct D3DOverlayColourControl d3d_overlay_ctrl;
extern void HandleD3DScreenFading(void);

extern void SetupPredOVision(void);
extern void HandlePredOVision(void);
extern void SetupMarineOVision(void);
extern void HandleMarineOVision(void);
extern void HandleAlienOVision(void);

extern int IsVisionChanging(void);

extern void ChangePredatorVisionMode(void);


extern void SetupVision(void);
#ifdef __cplusplus
}
#endif

#endif /* ! _included_vision_h_ */
