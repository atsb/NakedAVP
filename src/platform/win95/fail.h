/* LotU: Error handling functions.

   Copyright (C) 1995, Jamie Lokier.
   Written for Rebellion Developments, Ltd.

   Permission to use, copy, modify and distribute this file for any
   purpose by Rebellion Developments, Ltd. is hereby granted.  If you
   want to use this file outside the company, please let me know.
*/

#ifndef __fail_h
#define __fail_h 1

#ifdef __cplusplus
extern "C" {
#endif

#ifdef _MSC_VER
#define VARARG_DECL __cdecl
#else
#define VARARG_DECL
#endif

extern void VARARG_DECL fail (const char * __format, ...);

#ifdef __cplusplus
}
#endif

#endif /* __fail_h */
