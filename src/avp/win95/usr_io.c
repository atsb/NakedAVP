/*------------------------------- Patrick 21/10/96 ------------------------------
  Source for reading player inputs.
  Note that whilst ReadUserInput() reads raw input data, the functions in this
  file map those inputs onto the player movement structures (defined in Player
  Status).  This is, of course, entirely platform dependant.  Consoles will
  need their own equivalent functions.... 

  -------------------------------------------------------------------------------*/ 
#include "3dc.h"
#include "module.h"
#include "inline.h"

#include "stratdef.h"
#include "gamedef.h"
#include "gameplat.h"

#include "bh_types.h"
#include "ourasert.h"
#include "comp_shp.h"

#include "pmove.h"
#include "usr_io.h"
#include "hud.h"
#include "messagehistory.h"

#include "iofocus.h"

#include "paintball.h"
#include "ahudgadg.hpp"
#include "avp_menus.h"

extern int InGameMenusAreRunning(void);
extern void AvP_TriggerInGameMenus(void);
extern void Recall_Disc(void);
extern void ShowMultiplayerScores(void);


extern int NormalFrameTime;

FIXED_INPUT_CONFIGURATION FixedInputConfig =
{
	KEY_1,				// Weapon1;
	KEY_2,				// Weapon2;
	KEY_3,				// Weapon3;
	KEY_4,				// Weapon4;
	KEY_5,				// Weapon5;
	KEY_6,				// Weapon6;
	KEY_7,				// Weapon7;
	KEY_8,				// Weapon8;
	KEY_9,				// Weapon9;
	KEY_0,				// Weapon10;
	KEY_ESCAPE,			// PauseGame;

};

PLAYER_INPUT_CONFIGURATION MarineInputPrimaryConfig;
PLAYER_INPUT_CONFIGURATION MarineInputSecondaryConfig;
PLAYER_INPUT_CONFIGURATION PredatorInputPrimaryConfig;
PLAYER_INPUT_CONFIGURATION PredatorInputSecondaryConfig;
PLAYER_INPUT_CONFIGURATION AlienInputPrimaryConfig;
PLAYER_INPUT_CONFIGURATION AlienInputSecondaryConfig;


#if 1 // English
PLAYER_INPUT_CONFIGURATION DefaultMarineInputPrimaryConfig =
{
	KEY_UP,				// Forward;
	KEY_DOWN,			// Backward;
	KEY_NUMPAD4, 		// Left;
	KEY_NUMPAD6, 		// Right;

	KEY_RIGHTALT,		// Strafe;
	KEY_LEFT,	 		// StrafeLeft;
	KEY_RIGHT,	 		// StrafeRight;

	KEY_Q, 				// LookUp;
	KEY_Z, 				// LookDown;
	KEY_A,				// CentreView;

	KEY_LEFTSHIFT,		// Walk;
	KEY_RIGHTCTRL, 		// Crouch;
	KEY_RIGHTSHIFT,		// Jump;

	KEY_SPACE,			// Operate;

	KEY_LMOUSE, 		// FirePrimaryWeapon;
	KEY_RMOUSE, 		// FireSecondaryWeapon;

    {KEY_RBRACKET},  	// NextWeapon;
    {KEY_LBRACKET},  	// PreviousWeapon;
    {KEY_BACKSPACE},	// FlashbackWeapon;

    {KEY_SLASH},	  	// ImageIntensifier;
    {KEY_FSTOP},    	// ThrowFlare;
    {KEY_APOSTROPHE},  	// Jetpack;
    {KEY_SEMICOLON},	// Taunt
    {KEY_F1},
    {KEY_F11},
    {KEY_F12},
    {KEY_TAB}
};
PLAYER_INPUT_CONFIGURATION DefaultPredatorInputPrimaryConfig =
{
	KEY_UP,				// Forward;
	KEY_DOWN,			// Backward;
	KEY_NUMPAD4, 		// Left;
	KEY_NUMPAD6, 		// Right;

	KEY_RIGHTALT,		// Strafe;
	KEY_LEFT,	 		// StrafeLeft;
	KEY_RIGHT,	 		// StrafeRight;

	KEY_Q, 				// LookUp;
	KEY_Z, 				// LookDown;
	KEY_A,				// CentreView;

	KEY_LEFTSHIFT,		// Walk;
	KEY_RIGHTCTRL, 		// Crouch;
	KEY_RIGHTSHIFT,		// Jump;

	KEY_SPACE,			// Operate;

	KEY_LMOUSE, 		// FirePrimaryWeapon;
	KEY_RMOUSE, 		// FireSecondaryWeapon;
	
    {KEY_RBRACKET},		// NextWeapon;
    {KEY_LBRACKET}, 	// PreviousWeapon;
    {KEY_BACKSPACE},	// FlashbackWeapon;
	
    {KEY_FSTOP},	 	// Cloak;
    {KEY_SLASH},	 	// CycleVisionMode;
    {KEY_PAGEUP},		// ZoomIn;
    {KEY_PAGEDOWN},		// ZoomOut;
    {KEY_APOSTROPHE},  	// GrapplingHook
    {KEY_COMMA},		// RecallDisk
    {KEY_SEMICOLON},	// Taunt
    {KEY_F1},
    KEY_F11,
    KEY_F12,
    KEY_TAB
};

PLAYER_INPUT_CONFIGURATION DefaultAlienInputPrimaryConfig =
{
	KEY_UP,				// Forward;
	KEY_DOWN,			// Backward;
	KEY_NUMPAD4, 		// Left;
	KEY_NUMPAD6, 		// Right;

	KEY_RIGHTALT,		// Strafe;
	KEY_LEFT,	 		// StrafeLeft;
	KEY_RIGHT,	 		// StrafeRight;

	KEY_Q, 				// LookUp;
	KEY_Z, 				// LookDown;
	KEY_A,				// CentreView;

	KEY_LEFTSHIFT,		// Walk;
	KEY_RIGHTCTRL, 		// Crouch;
	KEY_RIGHTSHIFT,		// Jump;

	KEY_SPACE,			// Operate;

	KEY_LMOUSE, 		// FirePrimaryWeapon;
	KEY_RMOUSE, 		// FireSecondaryWeapon;

    {KEY_SLASH},		// AlternateVision;
    {KEY_FSTOP},		// Taunt;
    {KEY_F1},
    {KEY_F11},
    {KEY_F12},
    {KEY_TAB}
};
#elif 0	// Dutch
PLAYER_INPUT_CONFIGURATION DefaultMarineInputPrimaryConfig =
{
	KEY_UP,				// Forward;
	KEY_DOWN,			// Backward;
	KEY_NUMPAD4, 		// Left;
	KEY_NUMPAD6, 		// Right;

	KEY_RIGHTALT,		// Strafe;
	KEY_LEFT,	 		// StrafeLeft;
	KEY_RIGHT,	 		// StrafeRight;

	KEY_Q, 				// LookUp;
	KEY_Z, 				// LookDown;
	KEY_A,				// CentreView;

	KEY_LEFTSHIFT,		// Walk;
	KEY_RIGHTCTRL, 		// Crouch;
	KEY_RIGHTSHIFT,		// Jump;

	KEY_SPACE,			// Operate;

	KEY_LMOUSE, 		// FirePrimaryWeapon;
	KEY_RMOUSE, 		// FireSecondaryWeapon;

	KEY_ASTERISK,		// NextWeapon;
	KEY_DIACRITIC_UMLAUT,// PreviousWeapon;
	KEY_BACKSPACE,		// FlashbackWeapon;

	KEY_MINUS,	  		// ImageIntensifier;
	KEY_FSTOP,    		// ThrowFlare;
	KEY_DIACRITIC_ACUTE,	  	// Jetpack;
	KEY_PLUS,		// Taunt
	KEY_F1,
	KEY_F11,
	KEY_F12,
	KEY_TAB,
};
PLAYER_INPUT_CONFIGURATION DefaultPredatorInputPrimaryConfig =
{
	KEY_UP,				// Forward;
	KEY_DOWN,			// Backward;
	KEY_NUMPAD4, 		// Left;
	KEY_NUMPAD6, 		// Right;

	KEY_RIGHTALT,		// Strafe;
	KEY_LEFT,	 		// StrafeLeft;
	KEY_RIGHT,	 		// StrafeRight;

	KEY_Q, 				// LookUp;
	KEY_Z, 				// LookDown;
	KEY_A,				// CentreView;

	KEY_LEFTSHIFT,		// Walk;
	KEY_RIGHTCTRL, 		// Crouch;
	KEY_RIGHTSHIFT,		// Jump;

	KEY_SPACE,			// Operate;

	KEY_LMOUSE, 		// FirePrimaryWeapon;
	KEY_RMOUSE, 		// FireSecondaryWeapon;
	
	KEY_ASTERISK,		// NextWeapon;
	KEY_DIACRITIC_UMLAUT,// PreviousWeapon;
	KEY_BACKSPACE,		// FlashbackWeapon;
	
	KEY_FSTOP,	 		// Cloak;
	KEY_MINUS,	 		// CycleVisionMode;
	KEY_PAGEUP,			// ZoomIn;
	KEY_PAGEDOWN,		// ZoomOut;
	KEY_DIACRITIC_ACUTE,	  	// GrapplingHook
	KEY_COMMA,			// RecallDisk
	KEY_PLUS,		// Taunt
	KEY_F1,
	KEY_F11,
	KEY_F12,
	KEY_TAB,
};

PLAYER_INPUT_CONFIGURATION DefaultAlienInputPrimaryConfig =
{
	KEY_UP,				// Forward;
	KEY_DOWN,			// Backward;
	KEY_NUMPAD4, 		// Left;
	KEY_NUMPAD6, 		// Right;

	KEY_RIGHTALT,		// Strafe;
	KEY_LEFT,	 		// StrafeLeft;
	KEY_RIGHT,	 		// StrafeRight;

	KEY_Q, 				// LookUp;
	KEY_Z, 				// LookDown;
	KEY_A,				// CentreView;

	KEY_LEFTSHIFT,		// Walk;
	KEY_RIGHTCTRL, 		// Crouch;
	KEY_RIGHTSHIFT,		// Jump;

	KEY_SPACE,			// Operate;

	KEY_LMOUSE, 		// FirePrimaryWeapon;
	KEY_RMOUSE, 		// FireSecondaryWeapon;

	KEY_MINUS,			// AlternateVision;
	KEY_FSTOP,				// Taunt;
	KEY_F1,
	KEY_F11,
	KEY_F12,
	KEY_TAB,
};
#elif 0	// French
PLAYER_INPUT_CONFIGURATION DefaultMarineInputPrimaryConfig =
{
	KEY_UP,				// Forward;
	KEY_DOWN,			// Backward;
	KEY_NUMPAD4, 		// Left;
	KEY_NUMPAD6, 		// Right;

	KEY_RIGHTALT,		// Strafe;
	KEY_LEFT,	 		// StrafeLeft;
	KEY_RIGHT,	 		// StrafeRight;

	KEY_A, 				// LookUp;
	KEY_W, 				// LookDown;
	KEY_Q,				// CentreView;

	KEY_LEFTSHIFT,		// Walk;
	KEY_RIGHTCTRL, 		// Crouch;
	KEY_RIGHTSHIFT,		// Jump;

	KEY_SPACE,			// Operate;

	KEY_LMOUSE, 		// FirePrimaryWeapon;
	KEY_RMOUSE, 		// FireSecondaryWeapon;

	KEY_DOLLAR,  		// NextWeapon;
	KEY_DIACRITIC_CARET,  		// PreviousWeapon;
	KEY_BACKSPACE,		// FlashbackWeapon;

	KEY_EXCLAMATION,	  		// ImageIntensifier;
	KEY_COLON,    		// ThrowFlare;
	KEY_U_GRAVE,	  	// Jetpack;
	KEY_M,		// Taunt
	KEY_F1,
	KEY_F11,
	KEY_F12,
	KEY_TAB,
};
PLAYER_INPUT_CONFIGURATION DefaultPredatorInputPrimaryConfig =
{
	KEY_UP,				// Forward;
	KEY_DOWN,			// Backward;
	KEY_NUMPAD4, 		// Left;
	KEY_NUMPAD6, 		// Right;

	KEY_RIGHTALT,		// Strafe;
	KEY_LEFT,	 		// StrafeLeft;
	KEY_RIGHT,	 		// StrafeRight;

	KEY_A, 				// LookUp;
	KEY_W, 				// LookDown;
	KEY_Q,				// CentreView;

	KEY_LEFTSHIFT,		// Walk;
	KEY_RIGHTCTRL, 		// Crouch;
	KEY_RIGHTSHIFT,		// Jump;

	KEY_SPACE,			// Operate;

	KEY_LMOUSE, 		// FirePrimaryWeapon;
	KEY_RMOUSE, 		// FireSecondaryWeapon;
	
	KEY_DOLLAR,		// NextWeapon;
	KEY_DIACRITIC_CARET, 			// PreviousWeapon;
	KEY_BACKSPACE,		// FlashbackWeapon;
	
	KEY_COLON,	 		// Cloak;
	KEY_EXCLAMATION,	 		// CycleVisionMode;
	KEY_PAGEUP,			// ZoomIn;
	KEY_PAGEDOWN,		// ZoomOut;
	KEY_U_GRAVE,	  	// GrapplingHook
	KEY_SEMICOLON,			// RecallDisk
	KEY_M,		// Taunt
	KEY_F1,
	KEY_F11,
	KEY_F12,
	KEY_TAB,
};

PLAYER_INPUT_CONFIGURATION DefaultAlienInputPrimaryConfig =
{
	KEY_UP,				// Forward;
	KEY_DOWN,			// Backward;
	KEY_NUMPAD4, 		// Left;
	KEY_NUMPAD6, 		// Right;

	KEY_RIGHTALT,		// Strafe;
	KEY_LEFT,	 		// StrafeLeft;
	KEY_RIGHT,	 		// StrafeRight;

	KEY_A, 				// LookUp;
	KEY_W, 				// LookDown;
	KEY_Q,				// CentreView;

	KEY_LEFTSHIFT,		// Walk;
	KEY_RIGHTCTRL, 		// Crouch;
	KEY_RIGHTSHIFT,		// Jump;

	KEY_SPACE,			// Operate;

	KEY_LMOUSE, 		// FirePrimaryWeapon;
	KEY_RMOUSE, 		// FireSecondaryWeapon;

	KEY_EXCLAMATION,			// AlternateVision;
	KEY_SEMICOLON,				// Taunt;
	KEY_F1,
	KEY_F11,
	KEY_F12,
	KEY_TAB,
};
#elif 0	 // German
PLAYER_INPUT_CONFIGURATION DefaultMarineInputPrimaryConfig =
{
	KEY_UP,				// Forward;
	KEY_DOWN,			// Backward;
	KEY_NUMPAD4, 		// Left;
	KEY_NUMPAD6, 		// Right;

	KEY_RIGHTALT,		// Strafe;
	KEY_LEFT,	 		// StrafeLeft;
	KEY_RIGHT,	 		// StrafeRight;

	KEY_Q, 				// LookUp;
	KEY_Y, 				// LookDown;
	KEY_A,				// CentreView;

	KEY_LEFTSHIFT,		// Walk;
	KEY_RIGHTCTRL, 		// Crouch;
	KEY_RIGHTSHIFT,		// Jump;

	KEY_SPACE,			// Operate;

	KEY_LMOUSE, 		// FirePrimaryWeapon;
	KEY_RMOUSE, 		// FireSecondaryWeapon;

	KEY_PLUS,  		// NextWeapon;
	KEY_U_UMLAUT,  		// PreviousWeapon;
	KEY_BACKSPACE,		// FlashbackWeapon;

	KEY_MINUS,	  		// ImageIntensifier;
	KEY_FSTOP,    		// ThrowFlare;
	KEY_A_UMLAUT,	  	// Jetpack;
	KEY_O_UMLAUT,		// Taunt
	KEY_F1,
	KEY_F11,
	KEY_F12,
	KEY_TAB,
};
PLAYER_INPUT_CONFIGURATION DefaultPredatorInputPrimaryConfig =
{
	KEY_UP,				// Forward;
	KEY_DOWN,			// Backward;
	KEY_NUMPAD4, 		// Left;
	KEY_NUMPAD6, 		// Right;

	KEY_RIGHTALT,		// Strafe;
	KEY_LEFT,	 		// StrafeLeft;
	KEY_RIGHT,	 		// StrafeRight;

	KEY_Q, 				// LookUp;
	KEY_Y, 				// LookDown;
	KEY_A,				// CentreView;

	KEY_LEFTSHIFT,		// Walk;
	KEY_RIGHTCTRL, 		// Crouch;
	KEY_RIGHTSHIFT,		// Jump;

	KEY_SPACE,			// Operate;

	KEY_LMOUSE, 		// FirePrimaryWeapon;
	KEY_RMOUSE, 		// FireSecondaryWeapon;
	
	KEY_PLUS,		// NextWeapon;
	KEY_U_UMLAUT, 			// PreviousWeapon;
	KEY_BACKSPACE,		// FlashbackWeapon;
	
	KEY_FSTOP,	 		// Cloak;
	KEY_MINUS,	 		// CycleVisionMode;
	KEY_PAGEUP,			// ZoomIn;
	KEY_PAGEDOWN,		// ZoomOut;
	KEY_A_UMLAUT,	  	// GrapplingHook
	KEY_COMMA,			// RecallDisk
	KEY_O_UMLAUT,		// Taunt
	KEY_F1,
	KEY_F11,
	KEY_F12,
	KEY_TAB,
};

PLAYER_INPUT_CONFIGURATION DefaultAlienInputPrimaryConfig =
{
	KEY_UP,				// Forward;
	KEY_DOWN,			// Backward;
	KEY_NUMPAD4, 		// Left;
	KEY_NUMPAD6, 		// Right;

	KEY_RIGHTALT,		// Strafe;
	KEY_LEFT,	 		// StrafeLeft;
	KEY_RIGHT,	 		// StrafeRight;

	KEY_Q, 				// LookUp;
	KEY_Y, 				// LookDown;
	KEY_A,				// CentreView;

	KEY_LEFTSHIFT,		// Walk;
	KEY_RIGHTCTRL, 		// Crouch;
	KEY_RIGHTSHIFT,		// Jump;

	KEY_SPACE,			// Operate;

	KEY_LMOUSE, 		// FirePrimaryWeapon;
	KEY_RMOUSE, 		// FireSecondaryWeapon;

	KEY_MINUS,			// AlternateVision;
	KEY_FSTOP,				// Taunt;
	KEY_F1,
	KEY_F11,
	KEY_F12,
	KEY_TAB,
};
#elif 0	// Spanish
PLAYER_INPUT_CONFIGURATION DefaultMarineInputPrimaryConfig =
{
	KEY_UP,				// Forward;
	KEY_DOWN,			// Backward;
	KEY_NUMPAD4, 		// Left;
	KEY_NUMPAD6, 		// Right;

	KEY_RIGHTALT,		// Strafe;
	KEY_LEFT,	 		// StrafeLeft;
	KEY_RIGHT,	 		// StrafeRight;

	KEY_Q, 				// LookUp;
	KEY_Z, 				// LookDown;
	KEY_A,				// CentreView;

	KEY_LEFTSHIFT,		// Walk;
	KEY_RIGHTCTRL, 		// Crouch;
	KEY_RIGHTSHIFT,		// Jump;

	KEY_SPACE,			// Operate;

	KEY_LMOUSE, 		// FirePrimaryWeapon;
	KEY_RMOUSE, 		// FireSecondaryWeapon;

	KEY_PLUS,  		// NextWeapon;
	KEY_DIACRITIC_GRAVE,  		// PreviousWeapon;
	KEY_BACKSPACE,		// FlashbackWeapon;

	KEY_MINUS,	  		// ImageIntensifier;
	KEY_FSTOP,    		// ThrowFlare;
	KEY_DIACRITIC_ACUTE,	  	// Jetpack;
	KEY_N_TILDE,		// Taunt
	KEY_F1,
	KEY_F11,
	KEY_F12,
	KEY_TAB,
};
PLAYER_INPUT_CONFIGURATION DefaultPredatorInputPrimaryConfig =
{
	KEY_UP,				// Forward;
	KEY_DOWN,			// Backward;
	KEY_NUMPAD4, 		// Left;
	KEY_NUMPAD6, 		// Right;

	KEY_RIGHTALT,		// Strafe;
	KEY_LEFT,	 		// StrafeLeft;
	KEY_RIGHT,	 		// StrafeRight;

	KEY_Q, 				// LookUp;
	KEY_Z, 				// LookDown;
	KEY_A,				// CentreView;

	KEY_LEFTSHIFT,		// Walk;
	KEY_RIGHTCTRL, 		// Crouch;
	KEY_RIGHTSHIFT,		// Jump;

	KEY_SPACE,			// Operate;

	KEY_LMOUSE, 		// FirePrimaryWeapon;
	KEY_RMOUSE, 		// FireSecondaryWeapon;
	
	KEY_PLUS,		// NextWeapon;
	KEY_DIACRITIC_GRAVE, 			// PreviousWeapon;
	KEY_BACKSPACE,		// FlashbackWeapon;
	
	KEY_FSTOP,	 		// Cloak;
	KEY_MINUS,	 		// CycleVisionMode;
	KEY_PAGEUP,			// ZoomIn;
	KEY_PAGEDOWN,		// ZoomOut;
	KEY_DIACRITIC_ACUTE,	  	// GrapplingHook
	KEY_COMMA,			// RecallDisk
	KEY_N_TILDE,		// Taunt
	KEY_F1,
	KEY_F11,
	KEY_F12,
	KEY_TAB,
};

PLAYER_INPUT_CONFIGURATION DefaultAlienInputPrimaryConfig =
{
	KEY_UP,				// Forward;
	KEY_DOWN,			// Backward;
	KEY_NUMPAD4, 		// Left;
	KEY_NUMPAD6, 		// Right;

	KEY_RIGHTALT,		// Strafe;
	KEY_LEFT,	 		// StrafeLeft;
	KEY_RIGHT,	 		// StrafeRight;

	KEY_Q, 				// LookUp;
	KEY_Z, 				// LookDown;
	KEY_A,				// CentreView;

	KEY_LEFTSHIFT,		// Walk;
	KEY_RIGHTCTRL, 		// Crouch;
	KEY_RIGHTSHIFT,		// Jump;

	KEY_SPACE,			// Operate;

	KEY_LMOUSE, 		// FirePrimaryWeapon;
	KEY_RMOUSE, 		// FireSecondaryWeapon;

	KEY_MINUS,			// AlternateVision;
	KEY_FSTOP,				// Taunt;
	KEY_F1,
	KEY_F11,
	KEY_F12,
	KEY_TAB,
};
#endif
PLAYER_INPUT_CONFIGURATION DefaultMarineInputSecondaryConfig =
{
	KEY_VOID,			// Forward;
	KEY_VOID,			// Backward;
	KEY_VOID, 			// Left;
	KEY_VOID, 			// Right;

	KEY_VOID,			// Strafe;
	KEY_VOID,	 		// StrafeLeft;
	KEY_VOID,	 		// StrafeRight;

	KEY_NUMPAD8,		// LookUp;
	KEY_NUMPAD2,		// LookDown;
	KEY_NUMPAD5,		// CentreView;

	KEY_VOID,		// Walk;
	KEY_VOID, 		// Crouch;
	KEY_MMOUSE,			// Jump;

	KEY_CR,				// Operate;

	KEY_NUMPAD0, 		// FirePrimaryWeapon;
	KEY_NUMPADDEL, 		// FireSecondaryWeapon;

    {KEY_MOUSEWHEELUP},  	// NextWeapon;
    {KEY_MOUSEWHEELDOWN},	// PreviousWeapon;
    {KEY_VOID},			// FlashbackWeapon;

    {KEY_VOID},			// ImageIntensifier;
    {KEY_VOID}, 		// ThrowFlare;
    {KEY_VOID}, 		// Jetpack;
    {KEY_VOID},			// Taunt

    {KEY_VOID},
    {KEY_VOID},
    {KEY_VOID},
    {KEY_VOID}
};




PLAYER_INPUT_CONFIGURATION DefaultPredatorInputSecondaryConfig =
{
	KEY_VOID,			// Forward;
	KEY_VOID,			// Backward;
	KEY_VOID, 			// Left;
	KEY_VOID, 			// Right;

	KEY_VOID,			// Strafe;
	KEY_VOID,	 		// StrafeLeft;
	KEY_VOID,	 		// StrafeRight;

	KEY_NUMPAD8,		// LookUp;
	KEY_NUMPAD2,		// LookDown;
	KEY_NUMPAD5,		// CentreView;

	KEY_VOID,		// Walk;
	KEY_VOID, 		// Crouch;
	KEY_MMOUSE,			// Jump;

	KEY_CR,				// Operate;

	KEY_NUMPAD0, 		// FirePrimaryWeapon;
	KEY_NUMPADDEL, 		// FireSecondaryWeapon;

    {KEY_VOID},			// NextWeapon;
    {KEY_VOID}, 		// PreviousWeapon;
    {KEY_VOID},			// FlashbackWeapon;
	
    {KEY_VOID},	 		// Cloak;
    {KEY_VOID},	 		// CycleVisionMode;
    {KEY_MOUSEWHEELUP},	// ZoomIn;
    {KEY_MOUSEWHEELDOWN},	// ZoomOut;
    {KEY_VOID},	 		// GrapplingHook;
    {KEY_VOID},			// RecallDisk
    {KEY_VOID},			// Taunt
	
    {KEY_VOID},
    KEY_VOID,
    KEY_VOID,
    KEY_VOID

};

PLAYER_INPUT_CONFIGURATION DefaultAlienInputSecondaryConfig =
{
	KEY_VOID,				// Forward;
	KEY_VOID,			// Backward;
	KEY_VOID, 		// Left;
	KEY_VOID, 		// Right;

	KEY_VOID,		// Strafe;
	KEY_VOID,	 		// StrafeLeft;
	KEY_VOID,	 		// StrafeRight;

	KEY_NUMPAD8,		// LookUp;
	KEY_NUMPAD2,		// LookDown;
	KEY_NUMPAD5,		// CentreView;

	KEY_VOID,			// Walk;
	KEY_VOID, 			// Crouch;
	KEY_MMOUSE,			// Jump;

	KEY_CR,				// Operate;

	KEY_NUMPAD0, 		// FirePrimaryWeapon;
	KEY_NUMPADDEL, 		// FireSecondaryWeapon;
	
    {KEY_VOID}, 		// AlternateVision;
    {KEY_VOID},	 		// Taunt;
    {KEY_VOID},
    {KEY_VOID},
    {KEY_VOID},
    {KEY_VOID}
};


CONTROL_METHODS ControlMethods =
{
	/* analogue stuff */
	DEFAULT_MOUSEX_SENSITIVITY, //unsigned int MouseXSensitivity;
	DEFAULT_MOUSEY_SENSITIVITY, //unsigned int MouseYSensitivity;

	0,//unsigned int VAxisIsMovement :1; // else it's looking
	1,//unsigned int HAxisIsTurning :1; // else it's sidestepping

	0,//unsigned int FlipVerticalAxis :1;
	
	/* general stuff */
	0,//unsigned int AutoCentreOnMovement :1;
};

CONTROL_METHODS DefaultControlMethods =
{
	/* analogue stuff */
	DEFAULT_MOUSEX_SENSITIVITY, //unsigned int MouseXSensitivity;
	DEFAULT_MOUSEY_SENSITIVITY, //unsigned int MouseYSensitivity;

	0,//unsigned int VAxisIsMovement :1; // else it's looking
	1,//unsigned int HAxisIsTurning :1; // else it's sidestepping

	0,//unsigned int FlipVerticalAxis :1;
	
	/* general stuff */
	0,//unsigned int AutoCentreOnMovement :1;
};

JOYSTICK_CONTROL_METHODS JoystickControlMethods =
{
	0,//unsigned int JoystickEnabled;
	
	1,//unsigned int JoystickVAxisIsMovement; // else it's looking
	1,//unsigned int JoystickHAxisIsTurning;  // else it's sidestepping
	0,//unsigned int JoystickFlipVerticalAxis;

	0,//unsigned int JoystickPOVVAxisIsMovement; // else it's looking
	0,//unsigned int JoystickPOVHAxisIsTurning;	 // else it's sidestepping
	0,//unsigned int JoystickPOVFlipVerticalAxis;

	0,//unsigned int JoystickRudderEnabled;		  
	0,//unsigned int JoystickRudderAxisIsTurning; // else it's sidestepping
	
	0,//unsigned int JoystickTrackerBallEnabled;
	0,//unsigned int JoystickTrackerBallFlipVerticalAxis;
	DEFAULT_TRACKERBALL_HORIZONTAL_SENSITIVITY,//unsigned int JoystickTrackerBallHorizontalSensitivity;
	DEFAULT_TRACKERBALL_VERTICAL_SENSITIVITY,//unsigned int JoystickTrackerBallVerticalSensitivity;

};
JOYSTICK_CONTROL_METHODS DefaultJoystickControlMethods =
{
	0,//unsigned int JoystickEnabled;
	
	1,//unsigned int JoystickVAxisIsMovement; // else it's looking
	1,//unsigned int JoystickHAxisIsTurning;  // else it's sidestepping
	0,//unsigned int JoystickFlipVerticalAxis;

	0,//unsigned int JoystickPOVVAxisIsMovement; // else it's looking
	0,//unsigned int JoystickPOVHAxisIsTurning;	 // else it's sidestepping
	0,//unsigned int JoystickPOVFlipVerticalAxis;

	0,//unsigned int JoystickRudderEnabled;		  
	0,//unsigned int JoystickRudderAxisIsTurning; // else it's sidestepping
	
	0,//unsigned int JoystickTrackerBallEnabled;
	0,//unsigned int JoystickTrackerBallFlipVerticalAxis;
	DEFAULT_TRACKERBALL_HORIZONTAL_SENSITIVITY,//unsigned int JoystickTrackerBallHorizontalSensitivity;
	DEFAULT_TRACKERBALL_VERTICAL_SENSITIVITY,//unsigned int JoystickTrackerBallVerticalSensitivity;
};

/* Extern for global keyboard buffer */
extern unsigned char KeyboardInput[];
extern unsigned char DebouncedKeyboardInput[];
extern int GotJoystick;
extern int GotMouse;

/* initialise the player input structure(s) in the player_status block */
void InitPlayerGameInput(STRATEGYBLOCK* sbPtr)
{
	PLAYER_STATUS *playerStatusPtr;

    /* get the player status block ... */
    playerStatusPtr = (PLAYER_STATUS *) (sbPtr->SBdataptr);
    LOCALASSERT(playerStatusPtr);
	

	/* analogue type inputs */
	playerStatusPtr->Mvt_MotionIncrement = 0;
	playerStatusPtr->Mvt_TurnIncrement = 0;
	playerStatusPtr->Mvt_PitchIncrement = 0;
	playerStatusPtr->Mvt_AnalogueTurning = 0;
	playerStatusPtr->Mvt_AnaloguePitching = 0;
	playerStatusPtr->Mvt_SideStepIncrement = 0;

	/* request flags */
	playerStatusPtr->Mvt_InputRequests.Mask = 0;
	playerStatusPtr->Mvt_InputRequests.Mask2 = 0;

	/* KJL 14:23:54 8/7/97 - default to run */
	playerStatusPtr->Mvt_InputRequests.Flags.Rqst_Faster = 1;

}


/* This function maps raw inputs onto the players movement attributes in
   the player_status block.  It is called from the ExecuteFreeMovement
   function.
   NB Currently, only keyboard input is supported. */
void ReadPlayerGameInput(STRATEGYBLOCK* sbPtr)
{
	PLAYER_INPUT_CONFIGURATION *primaryInput;
	PLAYER_INPUT_CONFIGURATION *secondaryInput;
	PLAYER_STATUS *playerStatusPtr;

    /* get the player status block ... */
    playerStatusPtr = (PLAYER_STATUS *) (sbPtr->SBdataptr);
    LOCALASSERT(playerStatusPtr);
	
	/* start off by initialising the inputs */
	InitPlayerGameInput(sbPtr);

	switch (AvP.PlayerType)
	{
		default:
		case I_Marine:
		{
			primaryInput = &MarineInputPrimaryConfig;
			secondaryInput = &MarineInputSecondaryConfig;
			break;
		}
		case I_Predator:
		{
			primaryInput = &PredatorInputPrimaryConfig;
			secondaryInput = &PredatorInputSecondaryConfig;
			break;
		}
		case I_Alien:
		{
			primaryInput = &AlienInputPrimaryConfig;
			secondaryInput = &AlienInputSecondaryConfig;
			break;
		}
	}

	if ( IOFOCUS_AcceptControls() && !InGameMenusAreRunning())
	{
		/* now do forward,backward,left,right,up and down 
		   IMPORTANT:  The request flag and the movement 
		   increment must BOTH be set!
		*/
		if(KeyboardInput[primaryInput->Forward]
		 ||KeyboardInput[secondaryInput->Forward])
		{
			playerStatusPtr->Mvt_InputRequests.Flags.Rqst_Forward = 1;
			playerStatusPtr->Mvt_MotionIncrement = ONE_FIXED;
		}	
		if(KeyboardInput[primaryInput->Backward]
		 ||KeyboardInput[secondaryInput->Backward])
		{
			playerStatusPtr->Mvt_InputRequests.Flags.Rqst_Backward = 1;
			playerStatusPtr->Mvt_MotionIncrement = -ONE_FIXED;
		}
		if(KeyboardInput[primaryInput->Left]
		 ||KeyboardInput[secondaryInput->Left])
		{
			playerStatusPtr->Mvt_InputRequests.Flags.Rqst_TurnLeft = 1;
			playerStatusPtr->Mvt_TurnIncrement = -ONE_FIXED;
		}
		if(KeyboardInput[primaryInput->Right]
		 ||KeyboardInput[secondaryInput->Right])
		{
			playerStatusPtr->Mvt_InputRequests.Flags.Rqst_TurnRight = 1;
			playerStatusPtr->Mvt_TurnIncrement = ONE_FIXED;
		}

		if(KeyboardInput[primaryInput->StrafeLeft]
		 ||KeyboardInput[secondaryInput->StrafeLeft])
		{
			playerStatusPtr->Mvt_InputRequests.Flags.Rqst_SideStepLeft = 1;
			playerStatusPtr->Mvt_SideStepIncrement = -ONE_FIXED;
		}
		if(KeyboardInput[primaryInput->StrafeRight]
		 ||KeyboardInput[secondaryInput->StrafeRight])
		{	
			playerStatusPtr->Mvt_InputRequests.Flags.Rqst_SideStepRight = 1;
			playerStatusPtr->Mvt_SideStepIncrement = ONE_FIXED;
		}
		if(KeyboardInput[primaryInput->Walk]
		 ||KeyboardInput[secondaryInput->Walk])
			playerStatusPtr->Mvt_InputRequests.Flags.Rqst_Faster = 0;
		
		if(KeyboardInput[primaryInput->Strafe]
		 ||KeyboardInput[secondaryInput->Strafe])
			playerStatusPtr->Mvt_InputRequests.Flags.Rqst_Strafe = 1;

		if(KeyboardInput[primaryInput->Crouch]
		 ||KeyboardInput[secondaryInput->Crouch])
			playerStatusPtr->Mvt_InputRequests.Flags.Rqst_Crouch = 1;
		
		if(KeyboardInput[primaryInput->Jump]
		 ||KeyboardInput[secondaryInput->Jump])
			playerStatusPtr->Mvt_InputRequests.Flags.Rqst_Jump = 1;

		if(KeyboardInput[primaryInput->Operate]
		 ||KeyboardInput[secondaryInput->Operate])
			playerStatusPtr->Mvt_InputRequests.Flags.Rqst_Operate = 1;

		/* check for character specific abilities */
		if (playerStatusPtr->IsAlive)
		switch (AvP.PlayerType)
		{
			case I_Marine:
			{
				if(KeyboardInput[primaryInput->d.ImageIntensifier]
				 ||KeyboardInput[secondaryInput->d.ImageIntensifier])
					playerStatusPtr->Mvt_InputRequests.Flags.Rqst_ChangeVision = 1;

				if(DebouncedKeyboardInput[primaryInput->e.ThrowFlare]
				 ||DebouncedKeyboardInput[secondaryInput->e.ThrowFlare])
					ThrowAFlare();

				#if !(MARINE_DEMO||DEATHMATCH_DEMO)
				if(KeyboardInput[primaryInput->f.Jetpack]
				 ||KeyboardInput[secondaryInput->f.Jetpack])
					playerStatusPtr->Mvt_InputRequests.Flags.Rqst_Jetpack = 1;
				#endif
				
				if(KeyboardInput[primaryInput->g.MarineTaunt]
				 ||KeyboardInput[secondaryInput->g.MarineTaunt])
					StartPlayerTaunt();
				
				if(DebouncedKeyboardInput[primaryInput->h.Marine_MessageHistory]
				 ||DebouncedKeyboardInput[secondaryInput->h.Marine_MessageHistory])
					MessageHistory_DisplayPrevious();
					
				if(DebouncedKeyboardInput[primaryInput->i.Marine_Say]
				 ||DebouncedKeyboardInput[secondaryInput->i.Marine_Say])
					BringDownConsoleWithSayTypedIn();

				if(DebouncedKeyboardInput[primaryInput->j.Marine_SpeciesSay]
				 ||DebouncedKeyboardInput[secondaryInput->j.Marine_SpeciesSay])
					BringDownConsoleWithSaySpeciesTypedIn();

				if(KeyboardInput[primaryInput->k.Marine_ShowScores]
				 ||KeyboardInput[secondaryInput->k.Marine_ShowScores])
					ShowMultiplayerScores();
					

				break;
			}
			case I_Predator:
			{
				extern int CameraZoomLevel;
				
				if(KeyboardInput[primaryInput->d.Cloak]
				 ||KeyboardInput[secondaryInput->d.Cloak])
					playerStatusPtr->Mvt_InputRequests.Flags.Rqst_ChangeVision = 1;
				
				if(DebouncedKeyboardInput[primaryInput->e.CycleVisionMode]
				 ||DebouncedKeyboardInput[secondaryInput->e.CycleVisionMode])
					playerStatusPtr->Mvt_InputRequests.Flags.Rqst_CycleVisionMode = 1;

				#if !(PREDATOR_DEMO||DEATHMATCH_DEMO)
				if(DebouncedKeyboardInput[primaryInput->h.GrapplingHook]
				 ||DebouncedKeyboardInput[secondaryInput->h.GrapplingHook])
					playerStatusPtr->Mvt_InputRequests.Flags.Rqst_GrapplingHook = 1;
				#endif

				if(DebouncedKeyboardInput[primaryInput->f.ZoomIn]
				 ||DebouncedKeyboardInput[secondaryInput->f.ZoomIn])
				{
					if (CameraZoomLevel<3) CameraZoomLevel++;
				}
				if(DebouncedKeyboardInput[primaryInput->g.ZoomOut]
				 ||DebouncedKeyboardInput[secondaryInput->g.ZoomOut])
				{
					if (CameraZoomLevel>0) CameraZoomLevel--;
				}
				
				MaintainZoomingLevel();
				
				if(KeyboardInput[primaryInput->j.PredatorTaunt]
				 ||KeyboardInput[secondaryInput->j.PredatorTaunt])
					StartPlayerTaunt();

				if(KeyboardInput[primaryInput->i.RecallDisc]
				 ||KeyboardInput[secondaryInput->i.RecallDisc])
					Recall_Disc();
					
				if(DebouncedKeyboardInput[primaryInput->k.Predator_MessageHistory]
				 ||DebouncedKeyboardInput[secondaryInput->k.Predator_MessageHistory])
					MessageHistory_DisplayPrevious();
					
				if(DebouncedKeyboardInput[primaryInput->Predator_Say]
				 ||DebouncedKeyboardInput[secondaryInput->Predator_Say])
					BringDownConsoleWithSayTypedIn();

				if(DebouncedKeyboardInput[primaryInput->Predator_SpeciesSay]
				 ||DebouncedKeyboardInput[secondaryInput->Predator_SpeciesSay])
					BringDownConsoleWithSaySpeciesTypedIn();

				if(KeyboardInput[primaryInput->Predator_ShowScores]
				 ||KeyboardInput[secondaryInput->Predator_ShowScores])
					ShowMultiplayerScores();

				break;
			}

			case I_Alien:
			{
				if(KeyboardInput[primaryInput->a.AlternateVision]
				 ||KeyboardInput[secondaryInput->a.AlternateVision])
					playerStatusPtr->Mvt_InputRequests.Flags.Rqst_ChangeVision = 1;

				if(KeyboardInput[primaryInput->b.Taunt]
				 ||KeyboardInput[secondaryInput->b.Taunt])
					StartPlayerTaunt();
	
				if(DebouncedKeyboardInput[primaryInput->c.Alien_MessageHistory]
				 ||DebouncedKeyboardInput[secondaryInput->c.Alien_MessageHistory])
					MessageHistory_DisplayPrevious();
					
				if(DebouncedKeyboardInput[primaryInput->d.Alien_Say]
				 ||DebouncedKeyboardInput[secondaryInput->d.Alien_Say])
					BringDownConsoleWithSayTypedIn();

				if(DebouncedKeyboardInput[primaryInput->e.Alien_SpeciesSay]
				 ||DebouncedKeyboardInput[secondaryInput->e.Alien_SpeciesSay])
					BringDownConsoleWithSaySpeciesTypedIn();

				if(KeyboardInput[primaryInput->f.Alien_ShowScores]
				 ||KeyboardInput[secondaryInput->f.Alien_ShowScores])
					ShowMultiplayerScores();

				break;
			}
		}
		
		if(DebouncedKeyboardInput[FixedInputConfig.PauseGame])
			AvP_TriggerInGameMenus();
	//		playerStatusPtr->Mvt_InputRequests.Flags.Rqst_QuitGame = 1;
//			playerStatusPtr->Mvt_InputRequests.Flags.Rqst_PauseGame = 1;

		if(!PaintBallMode.IsOn)
		{
			if(KeyboardInput[primaryInput->FirePrimaryWeapon]
			 ||KeyboardInput[secondaryInput->FirePrimaryWeapon])
				playerStatusPtr->Mvt_InputRequests.Flags.Rqst_FirePrimaryWeapon = 1;
			
			if(KeyboardInput[primaryInput->LookUp]
			 ||KeyboardInput[secondaryInput->LookUp])
			{
				playerStatusPtr->Mvt_InputRequests.Flags.Rqst_LookUp = 1;
				playerStatusPtr->Mvt_PitchIncrement = -ONE_FIXED;
			}
			else if(KeyboardInput[primaryInput->LookDown]
			 ||KeyboardInput[secondaryInput->LookDown])
			{
				playerStatusPtr->Mvt_InputRequests.Flags.Rqst_LookDown = 1;
				playerStatusPtr->Mvt_PitchIncrement = ONE_FIXED;
			}
  			
			if(KeyboardInput[primaryInput->CentreView]
			 ||KeyboardInput[secondaryInput->CentreView])
				playerStatusPtr->Mvt_InputRequests.Flags.Rqst_CentreView = 1;

			if(KeyboardInput[primaryInput->a.NextWeapon]
			 ||KeyboardInput[secondaryInput->a.NextWeapon])
				playerStatusPtr->Mvt_InputRequests.Flags.Rqst_NextWeapon = 1;
			
			if(KeyboardInput[primaryInput->b.PreviousWeapon]
			 ||KeyboardInput[secondaryInput->b.PreviousWeapon])
				playerStatusPtr->Mvt_InputRequests.Flags.Rqst_PreviousWeapon = 1;

			if(DebouncedKeyboardInput[primaryInput->c.FlashbackWeapon]
			 ||DebouncedKeyboardInput[secondaryInput->c.FlashbackWeapon])
			{
				if (playerStatusPtr->PreviouslySelectedWeaponSlot!=playerStatusPtr->SelectedWeaponSlot)
				{
					playerStatusPtr->Mvt_InputRequests.Flags.Rqst_WeaponNo = playerStatusPtr->PreviouslySelectedWeaponSlot+1;
				}
			}
			
			if(KeyboardInput[primaryInput->FireSecondaryWeapon]
			 ||KeyboardInput[secondaryInput->FireSecondaryWeapon])
				playerStatusPtr->Mvt_InputRequests.Flags.Rqst_FireSecondaryWeapon = 1;
			
			/* fixed controls */
			if(KeyboardInput[FixedInputConfig.Weapon1])
				playerStatusPtr->Mvt_InputRequests.Flags.Rqst_WeaponNo = 1;
			
			#if !PREDATOR_DEMO
		  	if(KeyboardInput[FixedInputConfig.Weapon2])
				playerStatusPtr->Mvt_InputRequests.Flags.Rqst_WeaponNo = 2;
			#else
		  	if(DebouncedKeyboardInput[FixedInputConfig.Weapon2])
				playerStatusPtr->Mvt_InputRequests.Flags.Rqst_WeaponNo = 2;
			#endif
			if(KeyboardInput[FixedInputConfig.Weapon3])
				playerStatusPtr->Mvt_InputRequests.Flags.Rqst_WeaponNo = 3;
			
			#if !(MARINE_DEMO)
			if(KeyboardInput[FixedInputConfig.Weapon4])
				playerStatusPtr->Mvt_InputRequests.Flags.Rqst_WeaponNo = 4;
			#else
			if(DebouncedKeyboardInput[FixedInputConfig.Weapon4])
				playerStatusPtr->Mvt_InputRequests.Flags.Rqst_WeaponNo = 4;
			#endif
			
			#if !(PREDATOR_DEMO||MARINE_DEMO)
			if(KeyboardInput[FixedInputConfig.Weapon5])
				playerStatusPtr->Mvt_InputRequests.Flags.Rqst_WeaponNo = 5;
			#else
		  	if(DebouncedKeyboardInput[FixedInputConfig.Weapon5])
				playerStatusPtr->Mvt_InputRequests.Flags.Rqst_WeaponNo = 5;
			#endif

			#if !(MARINE_DEMO)
			if(KeyboardInput[FixedInputConfig.Weapon6])
				playerStatusPtr->Mvt_InputRequests.Flags.Rqst_WeaponNo = 6;
			#else
			if(DebouncedKeyboardInput[FixedInputConfig.Weapon6])
				playerStatusPtr->Mvt_InputRequests.Flags.Rqst_WeaponNo = 6;
			#endif
			
			if(KeyboardInput[FixedInputConfig.Weapon7])
				playerStatusPtr->Mvt_InputRequests.Flags.Rqst_WeaponNo = 7;
			
			if(KeyboardInput[FixedInputConfig.Weapon8])
				playerStatusPtr->Mvt_InputRequests.Flags.Rqst_WeaponNo = 8;
			
			if(KeyboardInput[FixedInputConfig.Weapon9])
				playerStatusPtr->Mvt_InputRequests.Flags.Rqst_WeaponNo = 9;
			
			if(KeyboardInput[FixedInputConfig.Weapon10])
				playerStatusPtr->Mvt_InputRequests.Flags.Rqst_WeaponNo = 10;
			
		 
		}
		#if !(PREDATOR_DEMO||MARINE_DEMO||ALIEN_DEMO||DEATHMATCH_DEMO)
		else // Cool - paintball mode				`
		{
			if(DebouncedKeyboardInput[primaryInput->a.NextWeapon]
			 ||DebouncedKeyboardInput[secondaryInput->a.NextWeapon])
			{
				PaintBallMode_ChangeSelectedDecalID(+1);
			}
			
			if(DebouncedKeyboardInput[primaryInput->b.PreviousWeapon]
			 ||DebouncedKeyboardInput[secondaryInput->b.PreviousWeapon])
			{
				PaintBallMode_ChangeSelectedDecalID(-1);
			}
				
			if(KeyboardInput[primaryInput->LookUp]
			 ||KeyboardInput[secondaryInput->LookUp])
			{
				PaintBallMode_ChangeSize(+1);
			}
			
			if(KeyboardInput[primaryInput->LookDown]
			 ||KeyboardInput[secondaryInput->LookDown])
			{
				PaintBallMode_ChangeSize(-1);
			}
			
			if(KeyboardInput[primaryInput->CentreView]
			 ||KeyboardInput[secondaryInput->CentreView])
			{
				PaintBallMode_Rotate();				
			}
						  
			if(DebouncedKeyboardInput[FixedInputConfig.Weapon1])
			{
				PaintBallMode_ChangeSubclass(+1);
			}
			
		  	if(DebouncedKeyboardInput[FixedInputConfig.Weapon2])
			{
				PaintBallMode_ChangeSubclass(-1);
			}

		  	if(DebouncedKeyboardInput[FixedInputConfig.Weapon3])
			{
				PaintBallMode.DecalIsInverted = ~PaintBallMode.DecalIsInverted;
			}

		  	if(DebouncedKeyboardInput[FixedInputConfig.Weapon4])
			{
				PaintBallMode_Randomise();
			}

		  	if(DebouncedKeyboardInput[FixedInputConfig.Weapon10])
			{
				extern void save_preplaced_decals();
				save_preplaced_decals();
			}
	

			if(DebouncedKeyboardInput[primaryInput->FirePrimaryWeapon]
			 ||DebouncedKeyboardInput[secondaryInput->FirePrimaryWeapon])
			{
				PaintBallMode_AddDecal();
			}
			if(DebouncedKeyboardInput[primaryInput->FireSecondaryWeapon]
			 ||DebouncedKeyboardInput[secondaryInput->FireSecondaryWeapon])
			{
				PaintBallMode_RemoveDecal();
			}
			
		}
		#endif
	}
	/* end of block conditional on input focus */


	/* KJL 10:16:49 04/29/97 - mouse control */




	if (GotMouse)
	{
		extern int MouseVelX;
		extern int MouseVelY;


		if(ControlMethods.HAxisIsTurning)
		{
			if(MouseVelX<0)
			{
				playerStatusPtr->Mvt_InputRequests.Flags.Rqst_TurnLeft = 1;
				playerStatusPtr->Mvt_AnalogueTurning = 1;
				playerStatusPtr->Mvt_TurnIncrement = ((int)MouseVelX)*ControlMethods.MouseXSensitivity;
			   
			}
			else if(MouseVelX>0)
			{
				playerStatusPtr->Mvt_InputRequests.Flags.Rqst_TurnRight = 1;
				playerStatusPtr->Mvt_AnalogueTurning = 1;
				playerStatusPtr->Mvt_TurnIncrement = ((int)MouseVelX)*ControlMethods.MouseXSensitivity;
			}

			/* KJL 17:36:37 9/9/97 - cap values if strafing */
		   	if(playerStatusPtr->Mvt_InputRequests.Flags.Rqst_Strafe)
			{
		   		if(playerStatusPtr->Mvt_TurnIncrement < -ONE_FIXED)
		   			playerStatusPtr->Mvt_TurnIncrement = -ONE_FIXED;
		   		if(playerStatusPtr->Mvt_TurnIncrement > ONE_FIXED)
		   			playerStatusPtr->Mvt_TurnIncrement = ONE_FIXED;
			}
			
		}
		else // it's sidestep
		{
			if(MouseVelX<0)
			{
				playerStatusPtr->Mvt_InputRequests.Flags.Rqst_SideStepLeft = 1;
				playerStatusPtr->Mvt_SideStepIncrement = ((int)MouseVelX)*ControlMethods.MouseXSensitivity;
			}
			else if(MouseVelX>0)
			{
				playerStatusPtr->Mvt_InputRequests.Flags.Rqst_SideStepRight = 1;
				playerStatusPtr->Mvt_SideStepIncrement = ((int)MouseVelX)*ControlMethods.MouseXSensitivity;
			}
	   		
	   		if(playerStatusPtr->Mvt_SideStepIncrement < -ONE_FIXED)
	   			playerStatusPtr->Mvt_SideStepIncrement = -ONE_FIXED;
	   		if(playerStatusPtr->Mvt_SideStepIncrement > ONE_FIXED)
	   			playerStatusPtr->Mvt_SideStepIncrement = ONE_FIXED;
			

		}

		if(ControlMethods.VAxisIsMovement)
		{
			
			if(MouseVelY<0)
			{
				playerStatusPtr->Mvt_InputRequests.Flags.Rqst_Forward = 1;
			 	playerStatusPtr->Mvt_MotionIncrement = -((int)MouseVelY)*ControlMethods.MouseYSensitivity;
			}
			else if(MouseVelY>0)
			{
				playerStatusPtr->Mvt_InputRequests.Flags.Rqst_Backward = 1;
			 	playerStatusPtr->Mvt_MotionIncrement = -((int)MouseVelY)*ControlMethods.MouseYSensitivity;
			}
	   	
	   		if(playerStatusPtr->Mvt_MotionIncrement < -ONE_FIXED)
	   			playerStatusPtr->Mvt_MotionIncrement = -ONE_FIXED;
	   		if(playerStatusPtr->Mvt_MotionIncrement > ONE_FIXED)
	   			playerStatusPtr->Mvt_MotionIncrement = ONE_FIXED;
		}
		else // it's looking
		{
			int newMouseVelY;

			if (ControlMethods.FlipVerticalAxis) newMouseVelY = -MouseVelY;
			else newMouseVelY = MouseVelY;

			if(newMouseVelY<0)
			{
				playerStatusPtr->Mvt_InputRequests.Flags.Rqst_LookUp = 1;
				playerStatusPtr->Mvt_AnaloguePitching = 1;
				playerStatusPtr->Mvt_PitchIncrement = ((int)newMouseVelY)*ControlMethods.MouseYSensitivity;
			}
			else if(newMouseVelY>0)
			{
				playerStatusPtr->Mvt_InputRequests.Flags.Rqst_LookDown = 1;
				playerStatusPtr->Mvt_AnaloguePitching = 1;
				playerStatusPtr->Mvt_PitchIncrement = ((int)newMouseVelY)*ControlMethods.MouseYSensitivity;
			}
		}	
								 
	}
	
	/* KJL 18:27:34 04/29/97 - joystick control */
	if (GotJoystick)
	{
		#define JOYSTICK_DEAD_ZONE 12000
		extern JOYINFOEX JoystickData;
		extern JOYCAPS JoystickCaps;
		
		
		int yAxis = (32768-JoystickData.dwYpos)*2;
		int xAxis = (JoystickData.dwXpos-32768)*2;
		
		if(JoystickControlMethods.JoystickVAxisIsMovement)
		{
			if(JoystickControlMethods.JoystickFlipVerticalAxis) yAxis=-yAxis;

			if(yAxis>JOYSTICK_DEAD_ZONE)
			{
				playerStatusPtr->Mvt_InputRequests.Flags.Rqst_Forward = 1;
				playerStatusPtr->Mvt_MotionIncrement = yAxis;
			}	
			else if(yAxis<-JOYSTICK_DEAD_ZONE)
			{
				playerStatusPtr->Mvt_InputRequests.Flags.Rqst_Backward = 1;
				playerStatusPtr->Mvt_MotionIncrement = yAxis;
			}
		}
		else // looking up/down
		{
			if(!JoystickControlMethods.JoystickFlipVerticalAxis) yAxis=-yAxis;

			if(yAxis>JOYSTICK_DEAD_ZONE)
			{
				playerStatusPtr->Mvt_InputRequests.Flags.Rqst_LookDown = 1;
				playerStatusPtr->Mvt_AnaloguePitching = 1;
				playerStatusPtr->Mvt_PitchIncrement = yAxis;
			}
			else if(yAxis<-JOYSTICK_DEAD_ZONE)
			{
				playerStatusPtr->Mvt_InputRequests.Flags.Rqst_LookUp = 1;
				playerStatusPtr->Mvt_AnaloguePitching = 1;
				playerStatusPtr->Mvt_PitchIncrement = yAxis;
			}
		}

		if (JoystickControlMethods.JoystickHAxisIsTurning)
		{
			if(xAxis<-JOYSTICK_DEAD_ZONE)
			{
				playerStatusPtr->Mvt_InputRequests.Flags.Rqst_TurnLeft = 1;
				playerStatusPtr->Mvt_AnalogueTurning = 1;
				playerStatusPtr->Mvt_TurnIncrement = xAxis;
			}
			else if(xAxis>JOYSTICK_DEAD_ZONE)
			{			  
				playerStatusPtr->Mvt_InputRequests.Flags.Rqst_TurnRight = 1;
				playerStatusPtr->Mvt_AnalogueTurning = 1;
				playerStatusPtr->Mvt_TurnIncrement = xAxis;
			}
		}
		else // strafing
		{
			if(xAxis<-JOYSTICK_DEAD_ZONE)
			{
				playerStatusPtr->Mvt_InputRequests.Flags.Rqst_SideStepLeft = 1;
				playerStatusPtr->Mvt_SideStepIncrement = xAxis;
			}
			else if(xAxis>JOYSTICK_DEAD_ZONE)
			{			  
				playerStatusPtr->Mvt_InputRequests.Flags.Rqst_SideStepRight = 1;
				playerStatusPtr->Mvt_SideStepIncrement = xAxis;
			}
		}
		
		/* check for rudder */
		if ((JoystickCaps.wCaps & JOYCAPS_HASR) && JoystickControlMethods.JoystickRudderEnabled)
		{
			int rAxis = (JoystickData.dwRpos-32768)*2;
			if (JoystickControlMethods.JoystickRudderAxisIsTurning)
			{
				if(rAxis>JOYSTICK_DEAD_ZONE)
				{
					playerStatusPtr->Mvt_InputRequests.Flags.Rqst_TurnRight = 1;
					playerStatusPtr->Mvt_AnalogueTurning = 1;
					playerStatusPtr->Mvt_TurnIncrement = rAxis;
				}
				else if(rAxis<-JOYSTICK_DEAD_ZONE)
				{			  
					playerStatusPtr->Mvt_InputRequests.Flags.Rqst_TurnLeft = 1;
					playerStatusPtr->Mvt_AnalogueTurning = 1;
					playerStatusPtr->Mvt_TurnIncrement = rAxis;
				}
			}
			else
			{
				if(rAxis>JOYSTICK_DEAD_ZONE)
				{
					playerStatusPtr->Mvt_InputRequests.Flags.Rqst_Strafe = 1;
					playerStatusPtr->Mvt_InputRequests.Flags.Rqst_TurnRight = 1;
					playerStatusPtr->Mvt_AnalogueTurning = 1;
					playerStatusPtr->Mvt_TurnIncrement = rAxis;
				}	
				else if(rAxis<-JOYSTICK_DEAD_ZONE)
				{
					playerStatusPtr->Mvt_InputRequests.Flags.Rqst_Strafe = 1;
					playerStatusPtr->Mvt_InputRequests.Flags.Rqst_TurnLeft = 1;
					playerStatusPtr->Mvt_AnalogueTurning = 1;
					playerStatusPtr->Mvt_TurnIncrement = rAxis;
				}	
			}
		}

		/* check joystick buttons */
		#if 0 
		if(JoystickData.dwButtons & JOY_BUTTON1)
			playerStatusPtr->Mvt_InputRequests.Flags.Rqst_FirePrimaryWeapon = 1;
		else if(JoystickData.dwButtons & JOY_BUTTON2)
			playerStatusPtr->Mvt_InputRequests.Flags.Rqst_FireSecondaryWeapon = 1;
		else if(JoystickData.dwButtons & JOY_BUTTON3)
			playerStatusPtr->Mvt_InputRequests.Flags.Rqst_NextWeapon = 1;
		else if(JoystickData.dwButtons & JOY_BUTTON4)
			playerStatusPtr->Mvt_InputRequests.Flags.Rqst_PreviousWeapon = 1;
		#endif

		/* Point Of View Hat */
		if (JoystickData.dwPOV<36000)
		{
			int theta = ((JoystickData.dwPOV * 4096) /36000);
			int verticalAxis = GetCos(theta);
			int horizontalAxis = GetSin(theta);

			if (JoystickControlMethods.JoystickPOVFlipVerticalAxis)
			{
				verticalAxis = -verticalAxis;
			}

			if (JoystickControlMethods.JoystickPOVVAxisIsMovement)
			{
				if(verticalAxis>0)
				{
					playerStatusPtr->Mvt_InputRequests.Flags.Rqst_Forward = 1;
					playerStatusPtr->Mvt_MotionIncrement = verticalAxis;
				}								  
				else if(verticalAxis<0)
				{
					playerStatusPtr->Mvt_InputRequests.Flags.Rqst_Backward = 1;
					playerStatusPtr->Mvt_MotionIncrement = verticalAxis;
				}
			}
			else
			{
				if(verticalAxis>0)
				{
					playerStatusPtr->Mvt_InputRequests.Flags.Rqst_LookUp = 1;
					playerStatusPtr->Mvt_PitchIncrement -= verticalAxis;
				}								  
				else if(verticalAxis<0)
				{
					playerStatusPtr->Mvt_InputRequests.Flags.Rqst_LookDown = 1;
					playerStatusPtr->Mvt_PitchIncrement -= verticalAxis;
				}
			}
			if (JoystickControlMethods.JoystickPOVHAxisIsTurning)
			{
				if(horizontalAxis>0)
				{
					playerStatusPtr->Mvt_InputRequests.Flags.Rqst_TurnRight = 1;
					playerStatusPtr->Mvt_AnalogueTurning = 1;
					playerStatusPtr->Mvt_TurnIncrement = horizontalAxis;
				}
				else if(horizontalAxis<0)
				{			  
					playerStatusPtr->Mvt_InputRequests.Flags.Rqst_TurnLeft = 1;
					playerStatusPtr->Mvt_AnalogueTurning = 1;
					playerStatusPtr->Mvt_TurnIncrement = horizontalAxis;
				}
			}
			else // strafing
			{
				if(horizontalAxis>0)
				{
					playerStatusPtr->Mvt_InputRequests.Flags.Rqst_SideStepRight = 1;
					playerStatusPtr->Mvt_SideStepIncrement = horizontalAxis;
				}
				else if(horizontalAxis<0)
				{			  
					playerStatusPtr->Mvt_InputRequests.Flags.Rqst_SideStepLeft = 1;
					playerStatusPtr->Mvt_SideStepIncrement = horizontalAxis;
				}
			}
		}
		if (JoystickControlMethods.JoystickTrackerBallEnabled)
		{
			int trackerballH = JoystickData.dwUpos - 32768;
			int trackerballV = JoystickData.dwVpos - 32768;

			if (JoystickControlMethods.JoystickTrackerBallFlipVerticalAxis)
			{
				trackerballV = -trackerballV;
			}

			if(trackerballH<0)
			{
				playerStatusPtr->Mvt_InputRequests.Flags.Rqst_TurnLeft = 1;
				playerStatusPtr->Mvt_AnalogueTurning = 1;
				playerStatusPtr->Mvt_TurnIncrement = trackerballH*JoystickControlMethods.JoystickTrackerBallHorizontalSensitivity;
			   
			}
			else if(trackerballH>0)
			{
				playerStatusPtr->Mvt_InputRequests.Flags.Rqst_TurnRight = 1;
				playerStatusPtr->Mvt_AnalogueTurning = 1;
				playerStatusPtr->Mvt_TurnIncrement = trackerballH*JoystickControlMethods.JoystickTrackerBallHorizontalSensitivity;

			}
			if(trackerballV<0)
			{
				playerStatusPtr->Mvt_InputRequests.Flags.Rqst_LookUp = 1;
				playerStatusPtr->Mvt_AnaloguePitching = 1;
				playerStatusPtr->Mvt_PitchIncrement = trackerballV*JoystickControlMethods.JoystickTrackerBallVerticalSensitivity;
			}
			else if(trackerballV>0)
			{
				playerStatusPtr->Mvt_InputRequests.Flags.Rqst_LookDown = 1;
				playerStatusPtr->Mvt_AnaloguePitching = 1;
				playerStatusPtr->Mvt_PitchIncrement = trackerballV*JoystickControlMethods.JoystickTrackerBallVerticalSensitivity;;
			}

		}

		#if 0
		textprint("%d\n%d\n%d\n%d\n%d\n%d\n%d\n%d\n",
			JoystickData.dwXpos,
			JoystickData.dwYpos,
			JoystickData.dwZpos,
			JoystickData.dwRpos,
			JoystickData.dwUpos,
			JoystickData.dwVpos,
			JoystickData.dwButtons,
			JoystickData.dwPOV);
		#endif
	}

	/* KJL 10:55:22 10/9/97 - HUD transparency */
	#if 0
	{
		extern signed int HUDTranslucencyLevel;

		if (KeyboardInput[KEY_F1])
		{
			HUDTranslucencyLevel-=NormalFrameTime>>9;
			if (HUDTranslucencyLevel<0) HUDTranslucencyLevel=0;
		}
		else if (KeyboardInput[KEY_F2])
		{
			HUDTranslucencyLevel+=NormalFrameTime>>9;
			if (HUDTranslucencyLevel>255) HUDTranslucencyLevel=255;
		}
	}
	#endif
	/* KJL 10:55:32 10/9/97 - screen size */
	#if 0
	if(KeyboardInput[KEY_F3])
		MakeViewingWindowLarger();
	else if(KeyboardInput[KEY_F4])
		MakeViewingWindowSmaller();
	#endif
	#if 0
	if (DebouncedKeyboardInput[KEY_F3])
	{
		MessageHistory_DisplayPrevious();
	}
	#endif
	if (DebouncedKeyboardInput[KEY_GRAVE]) IOFOCUS_Toggle();
}

void LoadKeyConfiguration(void)
{
	#if ALIEN_DEMO
	LoadAKeyConfiguration("alienavpkey.cfg");
	#else
	LoadAKeyConfiguration("avpkey.cfg");
	#endif

}

void SaveKeyConfiguration(void)
{
	#if ALIEN_DEMO
	SaveAKeyConfiguration("alienavpkey.cfg");
	#else
	SaveAKeyConfiguration("avpkey.cfg");
	#endif
}

void LoadAKeyConfiguration(char* Filename)
{
	#if 0
	FILE* file=fopen(Filename,"rb");
	if(!file)
	{
		MarineInputPrimaryConfig = DefaultMarineInputPrimaryConfig;
		MarineInputSecondaryConfig = DefaultMarineInputSecondaryConfig;
		PredatorInputPrimaryConfig = DefaultPredatorInputPrimaryConfig;
		PredatorInputSecondaryConfig = DefaultPredatorInputSecondaryConfig;
		AlienInputPrimaryConfig = DefaultAlienInputPrimaryConfig;
		AlienInputSecondaryConfig = DefaultAlienInputSecondaryConfig;
		return;
	}
	fread(&MarineInputPrimaryConfig,sizeof(PLAYER_INPUT_CONFIGURATION),1,file);
	fread(&MarineInputSecondaryConfig,sizeof(PLAYER_INPUT_CONFIGURATION),1,file);
	fread(&PredatorInputPrimaryConfig,sizeof(PLAYER_INPUT_CONFIGURATION),1,file);
	fread(&PredatorInputSecondaryConfig,sizeof(PLAYER_INPUT_CONFIGURATION),1,file);
	fread(&AlienInputPrimaryConfig,sizeof(PLAYER_INPUT_CONFIGURATION),1,file);
	fread(&AlienInputSecondaryConfig,sizeof(PLAYER_INPUT_CONFIGURATION),1,file);

	fread(&ControlMethods,sizeof(CONTROL_METHODS),1,file);
	fread(&JoystickControlMethods,sizeof(JOYSTICK_CONTROL_METHODS),1,file);
	
	fclose(file);
	#endif
}

void SaveAKeyConfiguration(char* Filename)
{
	#if 0
	FILE* file=fopen(Filename,"wb");
	if(!file) return;

	fwrite(&MarineInputPrimaryConfig,sizeof(PLAYER_INPUT_CONFIGURATION),1,file);
	fwrite(&MarineInputSecondaryConfig,sizeof(PLAYER_INPUT_CONFIGURATION),1,file);
	fwrite(&PredatorInputPrimaryConfig,sizeof(PLAYER_INPUT_CONFIGURATION),1,file);
	fwrite(&PredatorInputSecondaryConfig,sizeof(PLAYER_INPUT_CONFIGURATION),1,file);
	fwrite(&AlienInputPrimaryConfig,sizeof(PLAYER_INPUT_CONFIGURATION),1,file);
	fwrite(&AlienInputSecondaryConfig,sizeof(PLAYER_INPUT_CONFIGURATION),1,file);

	fwrite(&ControlMethods,sizeof(CONTROL_METHODS),1,file);
	fwrite(&JoystickControlMethods,sizeof(JOYSTICK_CONTROL_METHODS),1,file);

	fclose(file);
	#endif
}

void SaveDefaultPrimaryConfigs(void)
{
	FILE *file = OpenGameFile("default.cfg", FILEMODE_WRITEONLY, FILETYPE_CONFIG);
	if(!file) return;

	fwrite(&DefaultMarineInputPrimaryConfig,sizeof(PLAYER_INPUT_CONFIGURATION),1,file);
	fwrite(&DefaultPredatorInputPrimaryConfig,sizeof(PLAYER_INPUT_CONFIGURATION),1,file);
	fwrite(&DefaultAlienInputPrimaryConfig,sizeof(PLAYER_INPUT_CONFIGURATION),1,file);

	fclose(file);
}
void LoadDefaultPrimaryConfigs(void)
{
	FILE *file = OpenGameFile("default.cfg", FILEMODE_READONLY, FILETYPE_CONFIG);
	if(!file) return;

	fread(&DefaultMarineInputPrimaryConfig,sizeof(PLAYER_INPUT_CONFIGURATION),1,file);
	fread(&DefaultPredatorInputPrimaryConfig,sizeof(PLAYER_INPUT_CONFIGURATION),1,file);
	fread(&DefaultAlienInputPrimaryConfig,sizeof(PLAYER_INPUT_CONFIGURATION),1,file);

	fclose(file);
}
