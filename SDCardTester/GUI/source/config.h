/*
 * SD Card Read Tester - Configuration
 * Copyright (c) 2026
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms and conditions of the GNU General Public License,
 * version 2, as published by the Free Software Foundation.
 */

#ifndef _CONFIG_H_
#define _CONFIG_H_

#include <utils/types.h>

// Stringification macros (two-level trick for proper macro expansion)
#define STR(x) #x
#define XSTR(x) STR(x)

// Version info (can be overridden by Makefile)
#ifndef SD_TESTER_VER_MJ
#define SD_TESTER_VER_MJ 1
#endif
#ifndef SD_TESTER_VER_MN
#define SD_TESTER_VER_MN 0
#endif
#ifndef SD_TESTER_VER_BF
#define SD_TESTER_VER_BF 0
#endif

// Test sizes
#define FAST_TEST_SECTORS (8 * 1024 * 1024) // 4096 MB (sectors are 512 bytes)
#define FAST_BUTTERFLY_ITER 512             // 512 iterations for fast butterfly

// Block size for reads (128 sectors = 64 KB per read)
#define BLOCKS_PER_READ 128

// Latency thresholds (microseconds)
#define LATENCY_SLOW_US 5000 // 5 ms = slow block warning
#define LATENCY_BAD_US 50000 // 50 ms = potential bad block

// Progress update frequency
#define PROGRESS_UPDATE_SECTORS 8192 // Update progress every 4 MB

// Memory addresses (IPL_HEAP_START is in bdk/memory_map.h)
#define IPL_STACK_TOP 0x83100000

// Hekate configuration structure (minimal version for compatibility)
typedef struct _hekate_config {
  u32 autoboot;
  u32 autoboot_list;
  u32 bootwait;
  u32 backlight;
  u32 autohosoff;
  u32 autonogc;
  u32 t210b01; // 1 if Mariko (T210B01), 0 if Erista (T210)
  u32 updater2p;
  u32 bootprotect;
  char *brand;
  char *tagline;
} hekate_config;

#endif
