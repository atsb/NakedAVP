#define MAX_DEVICES 4
#define MAX_VIDEOMODES 100


typedef struct
{
	int Width;
	int Height;
	int ColourDepth;
} VIDEOMODEDESC;

typedef struct
{
	GUID DDGUID;
	int DDGUIDIsSet;

    DDDEVICEIDENTIFIER  DeviceInfo;
    DDDEVICEIDENTIFIER  DeviceInfoHost;
	DDCAPS 				DriverCaps;

	VIDEOMODEDESC		VideoModes[MAX_VIDEOMODES];
	int					NumberOfVideoModes;

} DEVICEANDVIDEOMODESDESC;

typedef struct
{
	GUID DDGUID;
	int  DDGUIDIsSet;
	int  Width;
	int  Height;
	int  ColourDepth;
	
} DEVICEANDVIDEOMODE;

extern DEVICEANDVIDEOMODESDESC DeviceDescriptions[MAX_DEVICES];
extern int NumberOfDevices;
extern int CurrentlySelectedDevice;
extern int CurrentlySelectedVideoMode;

