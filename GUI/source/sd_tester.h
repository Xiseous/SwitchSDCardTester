/*
 * SD Card Read Tester - Test Logic Header
 * Copyright (c) 2026
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms and conditions of the GNU General Public License,
 * version 2, as published by the Free Software Foundation.
 */

#ifndef _SD_TESTER_H_
#define _SD_TESTER_H_

#include <utils/types.h>

// Test mode enumeration
typedef enum {
  TEST_SEQ_FAST, // Fast Sequential (512 MB)
  TEST_SEQ_FULL, // Full Sequential (entire card)
  TEST_BTF_FAST, // Fast Butterfly (512 iterations)
  TEST_BTF_FULL, // Full Butterfly (entire card)
  TEST_ALL_FAST, // Run both fast tests
  TEST_ALL_FULL, // Run both full tests
} test_mode_t;

// Test result structure
typedef struct {
  u32 blocks_tested;
  u32 blocks_passed;
  u32 read_errors;
  u32 slow_blocks;
  u32 min_latency_us;
  u32 max_latency_us;
  u64 total_latency_us;
} sd_test_result_t;

// SD card info structure
typedef struct {
  u32 capacity_mb;
  u32 capacity_gb;
  u32 total_sectors;
  const char *speed_mode;
} sd_card_info_t;

// Function prototypes
void sd_tester_init_result(sd_test_result_t *result);
void sd_tester_get_card_info(sd_card_info_t *info);
const char *sd_tester_get_speed_mode_string(u32 mode);

// Test execution functions
int sd_tester_run_sequential(sd_test_result_t *result, u32 sector_limit,
                             void (*progress_cb)(u32 current, u32 total,
                                                 u32 latency, u32 errors));
int sd_tester_run_butterfly(sd_test_result_t *result, u32 iterations,
                            void (*progress_cb)(u32 current, u32 total,
                                                u32 lat_low, u32 lat_high));

// Result helpers
u32 sd_tester_get_avg_latency(sd_test_result_t *result);
int sd_tester_is_passed(sd_test_result_t *result);

#endif
