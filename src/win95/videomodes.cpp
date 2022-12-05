extern "C"
{

#include "3dc.h"
#include "videomodes.h"
#include "awTexLd.h" // to set the surface format for Aw gfx dd surface loads

extern LPDIRECTDRAW            lpDD;
DEVICEANDVIDEOMODESDESC DeviceDescriptions[MAX_DEVICES];
int NumberOfDevices;
int CurrentlySelectedDevice=0;
int CurrentlySelectedVideoMode=0;

D3DDEVICEDESC D3DDeviceDesc;
int D3DDeviceDescIsValid;



extern void GetDeviceAndVideoModePrefences(void);
static void SelectBasicDeviceAndVideoMode(void);
static void SetDeviceAndVideoModePreferences(void);
extern void SaveDeviceAndVideoModePreferences(void);
extern void LoadDeviceAndVideoModePreferences(void);

HRESULT WINAPI D3DDeviceEnumerator(LPGUID lpGuid,
        LPSTR lpDeviceDescription, LPSTR lpDeviceName,
        LPD3DDEVICEDESC lpHWDesc, LPD3DDEVICEDESC lpHELDesc, LPVOID lpContext)
{
    /* hardware only please */
    if (!lpHWDesc->dcmColorModel)
	{
		return D3DENUMRET_OK;
    } 

	/* that'll do */
	D3DDeviceDesc = *lpHWDesc;
	D3DDeviceDescIsValid = 1;

    return D3DENUMRET_CANCEL;
}





HRESULT WINAPI EnumVideoModesCallback(LPDDSURFACEDESC2 pddsd, LPVOID Context)
{
	DEVICEANDVIDEOMODESDESC *dPtr = &DeviceDescriptions[NumberOfDevices];
	VIDEOMODEDESC *vmPtr = &(dPtr->VideoModes[dPtr->NumberOfVideoModes]);

    vmPtr->Width = pddsd->dwWidth;
    vmPtr->Height = pddsd->dwHeight;
    vmPtr->ColourDepth = pddsd->ddpfPixelFormat.dwRGBBitCount;

	if (vmPtr->ColourDepth<=8 || vmPtr->Width<512 || vmPtr->Height<384)
		return DDENUMRET_OK;

	if (vmPtr->ColourDepth<=16)
	{
		if (!(D3DDeviceDesc.dwDeviceRenderBitDepth & DDBD_16))
		{
			return DDENUMRET_OK;
		}
	}
	else if (vmPtr->ColourDepth<=24)
	{
		if (!(D3DDeviceDesc.dwDeviceRenderBitDepth & DDBD_24))
		{
			return DDENUMRET_OK;
		}
	}
	else if (vmPtr->ColourDepth<=32)
	{
		if (!(D3DDeviceDesc.dwDeviceRenderBitDepth & DDBD_32))
		{
			return DDENUMRET_OK;
		}
	}
			
	dPtr->NumberOfVideoModes++;
    
	if (dPtr->NumberOfVideoModes < MAX_VIDEOMODES)
	{
       return DDENUMRET_OK;
	}
	else
	{
	   return DDENUMRET_CANCEL;
	}
}


//-----------------------------------------------------------------------------
// Name: DDEnumCallbackEx()
// Desc: This callback gets the information for each device enumerated
//-----------------------------------------------------------------------------
BOOL WINAPI DDEnumCallbackEx(GUID *pGUID, LPSTR pDescription, LPSTR pName, LPVOID pContext, HMONITOR hm)
{
    LPDIRECTDRAW            pDD;            // DD1 interface, used to get DD4 interface
    LPDIRECTDRAW4           pDD4 = NULL;	// DirectDraw4 interface
	LPDIRECT3D          	pD3D = NULL;
    HRESULT                 hRet;
	DDCAPS					HELCaps;

    // Create the main DirectDraw object
    hRet = DirectDrawCreate(pGUID, &pDD, NULL);
    if (hRet != DD_OK)
    {
        return DDENUMRET_CANCEL;
    }

    // Fetch DirectDraw4 interface
    hRet = pDD->QueryInterface(IID_IDirectDraw4, (LPVOID *)&pDD4);
    if (hRet != DD_OK)
    {
        return DDENUMRET_CANCEL;
    }

	memset(&DeviceDescriptions[NumberOfDevices].DriverCaps, 0, sizeof(DDCAPS));
	DeviceDescriptions[NumberOfDevices].DriverCaps.dwSize = sizeof(DDCAPS);
	memset(&HELCaps, 0, sizeof(DDCAPS));
	HELCaps.dwSize = sizeof(DDCAPS);

	hRet = pDD->GetCaps(&DeviceDescriptions[NumberOfDevices].DriverCaps, &HELCaps);

	if ((hRet != DD_OK) || !(DeviceDescriptions[NumberOfDevices].DriverCaps.dwCaps & DDCAPS_3D))
	{
		// Finished with the DirectDraw object, so release it
		if (pDD4) pDD4->Release();
		return DDENUMRET_OK;
	}

    // Get the device information and save it
    if (pGUID)
    {
    	DeviceDescriptions[NumberOfDevices].DDGUID = *pGUID;
    	DeviceDescriptions[NumberOfDevices].DDGUIDIsSet = 1;
	}
	else
	{
    	DeviceDescriptions[NumberOfDevices].DDGUIDIsSet = 0;
	}
    hRet = pDD4->GetDeviceIdentifier(&DeviceDescriptions[NumberOfDevices].DeviceInfo,0);
    hRet = pDD4->GetDeviceIdentifier(&DeviceDescriptions[NumberOfDevices].DeviceInfoHost,DDGDI_GETHOSTIDENTIFIER);

	// check available video modes
	DeviceDescriptions[NumberOfDevices].NumberOfVideoModes = 0;

	hRet = pDD->QueryInterface(IID_IDirect3D, (LPVOID*) &pD3D);
    
    hRet = pD3D->EnumDevices(D3DDeviceEnumerator, NULL);
	if (pD3D) pD3D->Release();

	if (!D3DDeviceDescIsValid)
	{
		// Finished with the DirectDraw object, so release it
		if (pDD4) pDD4->Release();
		return DDENUMRET_OK;
	}

	
    hRet = pDD4->EnumDisplayModes(0, NULL, 0, EnumVideoModesCallback);


    // Finished with the DirectDraw object, so release it
    if (pDD4) pDD4->Release();

    // Bump to the next open slot or finish the callbacks if full
    if (NumberOfDevices < MAX_DEVICES)
        NumberOfDevices++;
    else
        return DDENUMRET_CANCEL;
    return DDENUMRET_OK;
}




int EnumerateCardsAndVideoModes(void)
{
    LPDIRECTDRAWENUMERATEEX pDirectDrawEnumerateEx;
    HINSTANCE               hDDrawDLL;

	NumberOfDevices=0;

    // Do a GetModuleHandle and GetProcAddress in order to get the
    // DirectDrawEnumerateEx function
    hDDrawDLL = GetModuleHandle("DDRAW");
    if (!hDDrawDLL)
    {
        return -1;
    }
    pDirectDrawEnumerateEx = (LPDIRECTDRAWENUMERATEEX )GetProcAddress(hDDrawDLL,"DirectDrawEnumerateExA");
    if (pDirectDrawEnumerateEx)
    {
        pDirectDrawEnumerateEx(DDEnumCallbackEx, NULL,
                                DDENUM_ATTACHEDSECONDARYDEVICES |
                                DDENUM_DETACHEDSECONDARYDEVICES |
                                DDENUM_NONDISPLAYDEVICES);
    }
    else
    {
		return -1;
    }

    if (0 == NumberOfDevices)
    {
        return -1;
    }
	
	LoadDeviceAndVideoModePreferences();
    return 0;
}

char buffer[32];
extern char *GetVideoModeDescription2(void)
{
	strncpy(buffer, DeviceDescriptions[CurrentlySelectedDevice].DeviceInfo.szDescription,24);
	buffer[24] = 0;
	return buffer;
}

extern char *GetVideoModeDescription3(void)
{
	DEVICEANDVIDEOMODESDESC *dPtr = &DeviceDescriptions[CurrentlySelectedDevice];
	VIDEOMODEDESC *vmPtr = &(dPtr->VideoModes[CurrentlySelectedVideoMode]);
	sprintf(buffer,"%dx%dx%d",vmPtr->Width,vmPtr->Height,vmPtr->ColourDepth);
	return buffer;
}

extern void PreviousVideoMode2(void)
{
	if (CurrentlySelectedVideoMode--<=0)
	{
		if(CurrentlySelectedDevice--<=0)
		{
			CurrentlySelectedDevice = NumberOfDevices-1;
		}
		CurrentlySelectedVideoMode = DeviceDescriptions[CurrentlySelectedDevice].NumberOfVideoModes-1;
	}
}

extern void NextVideoMode2(void)
{
	if (++CurrentlySelectedVideoMode>=DeviceDescriptions[CurrentlySelectedDevice].NumberOfVideoModes)
	{
		CurrentlySelectedVideoMode = 0;
		if(++CurrentlySelectedDevice>=NumberOfDevices)
		{
			CurrentlySelectedDevice = 0;
		}
	}
}

void GetCorrectDirectDrawObject(void)
{
	extern int SelectDirectDrawObject(LPGUID pGUID);
  	finiObjectsExceptDD();
  	finiObjects();

	if(DeviceDescriptions[CurrentlySelectedDevice].DDGUIDIsSet)
	{
		SelectDirectDrawObject(&DeviceDescriptions[CurrentlySelectedDevice].DDGUID);
	}
	else
	{
		SelectDirectDrawObject(NULL);
   	}
    TestInitD3DObject();
}   	


DEVICEANDVIDEOMODE PreferredDeviceAndVideoMode;

extern void GetDeviceAndVideoModePrefences(void)
{
	int i;
			
	CurrentlySelectedDevice = -1;
	CurrentlySelectedVideoMode = -1;

	for (i=0; i<NumberOfDevices; i++)
	{
		if (PreferredDeviceAndVideoMode.DDGUID == DeviceDescriptions[i].DDGUID)
		{
			CurrentlySelectedDevice = i;
			break;
		}
	}
	if (CurrentlySelectedDevice==-1)
	{
		SelectBasicDeviceAndVideoMode();
		return;
	}

	DEVICEANDVIDEOMODESDESC *dPtr = &DeviceDescriptions[CurrentlySelectedDevice];
	for (i=0; i<dPtr->NumberOfVideoModes; i++)
	{
		if ((PreferredDeviceAndVideoMode.Width == dPtr->VideoModes[i].Width)
		  &&(PreferredDeviceAndVideoMode.Height == dPtr->VideoModes[i].Height)
		  &&(PreferredDeviceAndVideoMode.ColourDepth == dPtr->VideoModes[i].ColourDepth))
		{
			CurrentlySelectedVideoMode = i;
			break;
		}
	}
	if (CurrentlySelectedDevice==-1)
	{
		SelectBasicDeviceAndVideoMode();
		return;
	}

	SetDeviceAndVideoModePreferences();
}


static void SelectBasicDeviceAndVideoMode(void)
{
	CurrentlySelectedDevice = -1;
	CurrentlySelectedVideoMode = -1;

	/* look for a basic 640x480x16 bit mode */
	for (int d=0; d<NumberOfDevices; d++)
	{
		DEVICEANDVIDEOMODESDESC *dPtr = &DeviceDescriptions[d];

		for (int i=0; i<dPtr->NumberOfVideoModes; i++)
		{
			if ((640 == dPtr->VideoModes[i].Width)
			  &&(480 == dPtr->VideoModes[i].Height)
			  &&(16  == dPtr->VideoModes[i].ColourDepth))
			{
				CurrentlySelectedVideoMode = i;
				CurrentlySelectedDevice = d;
				break;
				/* note this only breaks out of the video mode loop;
				   all drivers are scanned through */
			}
		}
	}
	
	if (CurrentlySelectedDevice==-1)
	{
		/* good grief, just pick the first possible thing */
		CurrentlySelectedDevice = 0;
		CurrentlySelectedVideoMode = 0;
	}

	SetDeviceAndVideoModePreferences();
}

static void SetDeviceAndVideoModePreferences(void)
{
	DEVICEANDVIDEOMODESDESC *dPtr = &DeviceDescriptions[CurrentlySelectedDevice];
	VIDEOMODEDESC *vmPtr = &(dPtr->VideoModes[CurrentlySelectedVideoMode]);

	PreferredDeviceAndVideoMode.DDGUID = dPtr->DDGUID;
	PreferredDeviceAndVideoMode.DDGUIDIsSet = dPtr->DDGUIDIsSet;
	PreferredDeviceAndVideoMode.Width = vmPtr->Width;
	PreferredDeviceAndVideoMode.Height = vmPtr->Height;
	PreferredDeviceAndVideoMode.ColourDepth = vmPtr->ColourDepth;
	
}

extern void SaveDeviceAndVideoModePreferences(void)
{
	SetDeviceAndVideoModePreferences();

	FILE* file=fopen("AvP_Video.cfg","wb");
	if(!file) return;
	fwrite(&PreferredDeviceAndVideoMode,sizeof(DEVICEANDVIDEOMODE),1,file);
	fclose(file);
}
extern void LoadDeviceAndVideoModePreferences(void)
{
	FILE* file=fopen("AvP_Video.cfg","rb");
	if(!file)
	{
		SelectBasicDeviceAndVideoMode();
		return;
	}
	fread(&PreferredDeviceAndVideoMode,sizeof(DEVICEANDVIDEOMODE),1,file);
	fclose(file);
	GetDeviceAndVideoModePrefences();
}

};