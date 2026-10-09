#ifndef _included_txioctrl_h_
#define _included_txioctrl_h_

#include "system.h"

#ifdef MaxImageGroups
#if MaxImageGroups < 2 /* optimize if this multiple groups are not required */
#undef MaxImageGroups
#endif /* MaxImageGroups < 2 */
#endif /* MaxImageGroups */

#ifdef MaxImageGroups

#ifdef __cplusplus
extern "C" {
#endif

void ImageGroupFreed(int img_group);

void MarkImageInUseByGroup(int img_group, int img_num_offset, int group_using);

int IsImageInUse(int img_group, int img_num_offset);

int CanDeleteImage(int img_group, int img_num_offset);

void NowDeleteImage(int img_group, int img_num_offset);

#if debug

void ImageGroupsDebugPrint(void);

void ImageGroupsDebugPrintInit(void);

#endif /* debug */

typedef void (*ImageNumberCallbackFunction) (int imgnum, void * user);

void EnumSharedImages(int group_num, int numimages, ImageNumberCallbackFunction callback_fn, void * user);

void EnumLeftoverImages(int group_num, int numimages, ImageNumberCallbackFunction callback_fn, void * user);

#ifdef __cplusplus
}
#endif

#endif /* MaxImageGroups */

#endif /* ! _included_txioctrl_h_ */
