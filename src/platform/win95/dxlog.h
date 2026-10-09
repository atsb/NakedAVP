#ifndef _included_dxlog_h_
#define _included_dxlog_h_

#include "system.h"

#ifdef __cplusplus
extern "C" {
#endif

#if debug

void dx_err_log(int error, int line, char const * file);
void dx_str_log(char const * str, int line, char const * file);
void dx_line_log(int line, char const * file);
void dx_strf_log(char const * fmt, ...);

#define LOGDXERR(error) dx_err_log(error,__LINE__,__FILE__)
#define LOGDXSTR(str) dx_str_log(str,__LINE__,__FILE__)
#define LOGDXFMT(args) (dx_line_log(__LINE__,__FILE__),dx_strf_log args)

#else

/* TODO: make these static inline to hush the compiler */
void dx_err_log(int error, int line, char const * file);
void dx_str_log(char const * str, int line, char const * file);
void dx_line_log(int line, char const * file);
void dx_strf_log(char const * fmt, ...);

#define LOGDXERR(error) dx_err_log(error,__LINE__,__FILE__)
#define LOGDXSTR(str) dx_str_log(str,__LINE__,__FILE__)
#define LOGDXFMT(args) (dx_line_log(__LINE__,__FILE__),dx_strf_log args)

/*
#define LOGDXERR(error) (void)0
#define LOGDXSTR(str) (void)0
#define LOGDXFMT(args) (void)0
*/
#endif


#ifdef __cplusplus
}
#endif

#endif /* ! _included_dxlog_h_ */
