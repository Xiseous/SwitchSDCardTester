/*
 * SD Card Read Tester - Core Test Logic
 * Copyright (c) 2026
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms and conditions of the GNU General Public License,
 * version 2, as published by the Free Software Foundation.
 */

#include <mem/heap.h>
#include <soc/timer.h>
#include <storage/sd.h>
#include <storage/sdmmc.h>
#include <string.h>

#include "config.h"
#include "sd_tester.h"

// External SD storage from BDK
extern sdmmc_storage_t sd_storage;

// Speed mode strings
static const char *speed_mode_strings[] = {"Init Failed", "1-bit HS25",
                                           "4-bit HS25",  "UHS SDR82",
                                           "UHS SDR104",  "UHS DDR208"};

void sd_tester_init_result(sd_test_result_t *result) {
  memset(result, 0, sizeof(sd_test_result_t));
  result->min_latency_us = 0xFFFFFFFF; // Start with max value
}

const char *sd_tester_get_speed_mode_string(u32 mode) {
  if (mode > 5)
    mode = 0;
  return speed_mode_strings[mode];
}

void sd_tester_get_card_info(sd_card_info_t *info) {
  info->total_sectors = sd_storage.sec_cnt;
  info->capacity_mb = (u32)((u64)sd_storage.sec_cnt * 512 / (1024 * 1024));
  info->capacity_gb = info->capacity_mb / 1024;
  info->speed_mode = sd_tester_get_speed_mode_string(sd_get_mode());
}

u32 sd_tester_get_avg_latency(sd_test_result_t *result) {
  if (result->blocks_tested == 0)
    return 0;
  return (u32)(result->total_latency_us / result->blocks_tested);
}

int sd_tester_is_passed(sd_test_result_t *result) {
  return (result->read_errors == 0);
}

// Record a single latency measurement
static void record_latency(sd_test_result_t *result, u32 latency_us,
                           int read_ok) {
  result->blocks_tested++;

  if (!read_ok) {
    result->read_errors++;
    return;
  }

  result->blocks_passed++;
  result->total_latency_us += latency_us;

  if (latency_us < result->min_latency_us)
    result->min_latency_us = latency_us;
  if (latency_us > result->max_latency_us)
    result->max_latency_us = latency_us;

  if (latency_us > LATENCY_SLOW_US)
    result->slow_blocks++;
}

int sd_tester_run_sequential(sd_test_result_t *result, u32 sector_limit,
                             void (*progress_cb)(u32 current, u32 total,
                                                 u32 latency, u32 errors)) {
  u8 *buffer = (u8 *)malloc(BLOCKS_PER_READ * 512);
  if (!buffer)
    return -1;

  u32 total_sectors = sd_storage.sec_cnt;
  u32 test_sectors = (sector_limit == 0 || sector_limit > total_sectors)
                         ? total_sectors
                         : sector_limit;

  sd_tester_init_result(result);

  u32 last_progress = 0;

  for (u32 sector = 0; sector < test_sectors; sector += BLOCKS_PER_READ) {
    // Calculate how many sectors to read (handle end of card)
    u32 sectors_to_read = BLOCKS_PER_READ;
    if (sector + sectors_to_read > test_sectors)
      sectors_to_read = test_sectors - sector;

    // Timed read
    u32 start_us = get_tmr_us();
    int read_ok =
        sdmmc_storage_read(&sd_storage, sector, sectors_to_read, buffer);
    u32 latency_us = get_tmr_us() - start_us;

    // Record result
    record_latency(result, latency_us, read_ok);

    // Progress callback
    if (progress_cb && (sector - last_progress >= PROGRESS_UPDATE_SECTORS)) {
      progress_cb(sector, test_sectors, latency_us, result->read_errors);
      last_progress = sector;
    }
  }

  // Final progress update
  if (progress_cb)
    progress_cb(test_sectors, test_sectors, 0, result->read_errors);

  free(buffer);
  return 0;
}

int sd_tester_run_butterfly(sd_test_result_t *result, u32 iterations,
                            void (*progress_cb)(u32 current, u32 total,
                                                u32 lat_low, u32 lat_high)) {
  u8 *buffer = (u8 *)malloc(BLOCKS_PER_READ * 512);
  if (!buffer)
    return -1;

  u32 total_sectors = sd_storage.sec_cnt;
  u32 low = 0;
  u32 high = total_sectors - BLOCKS_PER_READ;

  // Calculate iterations - full test goes until pointers meet
  u32 max_iterations = (total_sectors / BLOCKS_PER_READ) / 2;
  u32 test_iterations = (iterations == 0 || iterations > max_iterations)
                            ? max_iterations
                            : iterations;

  sd_tester_init_result(result);

  for (u32 i = 0; i < test_iterations && low < high; i++) {
    // Read from low end
    u32 start_low = get_tmr_us();
    int read_ok_low =
        sdmmc_storage_read(&sd_storage, low, BLOCKS_PER_READ, buffer);
    u32 latency_low = get_tmr_us() - start_low;
    record_latency(result, latency_low, read_ok_low);

    // Read from high end
    u32 start_high = get_tmr_us();
    int read_ok_high =
        sdmmc_storage_read(&sd_storage, high, BLOCKS_PER_READ, buffer);
    u32 latency_high = get_tmr_us() - start_high;
    record_latency(result, latency_high, read_ok_high);

    // Move pointers
    low += BLOCKS_PER_READ;
    high -= BLOCKS_PER_READ;

    // Progress callback every 10 iterations
    if (progress_cb && (i % 10 == 0)) {
      progress_cb(i, test_iterations, latency_low, latency_high);
    }
  }

  // Final progress update
  if (progress_cb)
    progress_cb(test_iterations, test_iterations, 0, 0);

  free(buffer);
  return 0;
}
