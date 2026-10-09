#ifndef _included_endianio_h_
#define _included_endianio_h_

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <stdio.h>

#include "datatype.h"

BYTE GetByte(FILE *fp);
WORD GetLittleWord(FILE *fp);
DWORD GetLittleDword(FILE *fp);

VOID PutByte(BYTE v, FILE *fp);
VOID PutLittleWord(WORD v, FILE *fp);
VOID PutLittleDword(DWORD v, FILE *fp);

#ifdef __cplusplus
};
#endif /* __cplusplus */

#endif /* _included_endianio_h_ */

