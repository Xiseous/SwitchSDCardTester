/*
 * Minimal fsutils.h for SD Card Tester
 */

#ifndef _FS_UTILS_H_
#define _FS_UTILS_H_

#include <libs/fatfs/ff.h>
#include <utils/types.h>

static inline bool FileExists(const char *path) {
  FILINFO fno;
  return (f_stat(path, &fno) == FR_OK);
}

#endif
