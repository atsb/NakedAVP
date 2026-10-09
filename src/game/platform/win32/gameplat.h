/* 
	This contains project platfrom specifc externs 
	and prototypes. Only valid when we are calling one
	p\p function from another.
*/



extern void LoadRifFile();



/* record of the histance passed to winmain */


extern HINSTANCE AVP_HInstance;
extern int AVP_NCmd;

extern int AVP_ChangeDisplayMode
					(
						HINSTANCE hInst, 
						int nCmd, 
						int NewVideoMode, 
						int NewWindowMode,
						int NewZBufferMode, 
						int NewRasterisationMode, 
						int NewSoftwareScanDrawMode, 
						int NewDXMemoryMode
					);
				


extern int WindowMode;
extern int WindowRequestMode;
extern int VideoRequestMode;
extern int ZBufferRequestMode;
extern int RasterisationRequestMode;
extern int SoftwareScanDrawRequestMode;
extern int DXMemoryRequestMode;
