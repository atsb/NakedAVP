#ifndef _included_debuglog_h_
#define _included_debuglog_h_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdarg.h>

typedef struct LogFile LOGFILE;

int vlfprintf(LOGFILE * lfp, char const * format, va_list args );

int lfprintf(LOGFILE * lfp, char const * format, ... );

int lfputs(LOGFILE * lfp, char const * str);

LOGFILE * lfopen(char const * fname);

void lfclose(LOGFILE * lfp);

#ifdef __cplusplus
}
#endif

#endif /* ! _included_debuglog_h_ */
