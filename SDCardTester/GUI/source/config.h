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

// Version info
#define SD_TESTER_VER_MJ 1
#define SD_TESTER_VER_MN 0
#define SD_TESTER_VER_BF 0

// Test sizes
#define FAST_TEST_SECTORS    (1024 * 1024)   // 512 MB (sectors are 512 bytes)
#define FAST_BUTTERFLY_ITER  512             // 512 iterations for fast butterfly

// Block size for reads (128 sectors = 64 KB per read)
#define BLOCKS_PER_READ      128

// Latency thresholds (microseconds)
#define LATENCY_SLOW_US      5000            // 5 ms = slow block warning
#define LATENCY_BAD_US       50000           // 50 ms = potential bad block

// Progress update frequency
#define PROGRESS_UPDATE_SECTORS  8192        // Update progress every 4 MB

// Memory addresses (same as other payloads)
#define IPL_STACK_TOP        0x83100000
#define IPL_HEAP_START       0x90000000

#endif
