/*
 * Minimal tools.h for SD Card Tester
 * Declares functions implemented in utils.c
 */

#ifndef _TE_TOOLS_H_
#define _TE_TOOLS_H_

#include <utils/types.h>

// Declared here, implemented in utils.c
void RebootToPayloadOrRcm();

// Stub - SD Card Tester doesn't need screenshot functionality
static inline void TakeScreenshot() {
  // No-op for SD Card Tester
}

#endif
