/* KJL 10:19:41 30/03/98 - ConsoleLog.hpp

	This file handles the mirroring of the console text 
	to a log file.
	
*/
#ifndef ConsoleLog_h_included
#define ConsoleLog_h_included

#ifdef __cplusplus
extern "C"
{
#endif
	extern void OutputToConsoleLogfile(char *messagePtr);
	extern void OutputBugReportToConsoleLogfile(char *messagePtr);
#ifdef __cplusplus
};
#endif

#endif
