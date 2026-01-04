/*
 * SD Card Read Tester - Text Version
 * Main Entry Point
 *
 * Copyright (c) 2026
 *
 * Based on CommonProblemResolver and Hekate BDK examples
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms and conditions of the GNU General Public License,
 * version 2, as published by the Free Software Foundation.
 */

#include <string.h>

#include "config.h"
#include <display/di.h>
#include <gfx_utils.h>
#include <mem/heap.h>
#include <mem/minerva.h>
#include <soc/bpmp.h>
#include <soc/hw_init.h>
#include <soc/timer.h>
#include <storage/sd.h>
#include <storage/sdmmc.h>
#include <utils/btn.h>
#include <utils/util.h>

#include "gfx/gfx.h"
#include "gfx/gfxutils.h"
#include "gfx/menu.h"
#include "hid/hid.h"
#include "sd_tester.h"
#include "utils/vector.h"

// Boot configuration
hekate_config h_cfg;
boot_cfg_t __attribute__((section("._boot_cfg"))) b_cfg;

extern void pivot_stack(u32 stack_top);

// Color definitions
#define COLOR_TITLE 0xFF00CCFF
#define COLOR_HEADER 0xFFFFBA00
#define COLOR_SUCCESS 0xFF96FF00
#define COLOR_ERROR 0xFFFF0000
#define COLOR_WARNING 0xFFFFDD00
#define COLOR_INFO 0xFFCCCCCC
#define COLOR_HIGHLIGHT 0xFF00FFCC

// Progress callback for sequential test
static void seq_progress_callback(u32 current, u32 total, u32 latency,
                                  u32 errors) {
  u32 percent = (total > 0) ? (current * 100 / total) : 0;
  gfx_con_setpos(0, 300);
  gfx_printf(
      "%kSequential:%k %3d%% | Sector: %d/%d | Latency: %d us | Errors: %d    ",
      COLOR_HEADER, COLOR_INFO, percent, current, total, latency, errors);
}

// Progress callback for butterfly test
static void btf_progress_callback(u32 current, u32 total, u32 lat_low,
                                  u32 lat_high) {
  u32 percent = (total > 0) ? (current * 100 / total) : 0;
  gfx_con_setpos(0, 320);
  gfx_printf(
      "%kButterfly:%k %3d%% | Iter: %d/%d | Low: %d us | High: %d us    ",
      COLOR_HEADER, COLOR_INFO, percent, current, total, lat_low, lat_high);
}

// Display test results
static void display_result(const char *test_name, sd_test_result_t *result) {
  u32 avg = sd_tester_get_avg_latency(result);

  gfx_printf("%k-- %s --%k\n", COLOR_HEADER, test_name, COLOR_INFO);
  gfx_printf("  Blocks Tested: %d\n", result->blocks_tested);
  gfx_printf("  Read Errors:   %s%d%k\n",
             result->read_errors > 0 ? "\x1b[31m" : "", result->read_errors,
             COLOR_INFO);
  gfx_printf("  Slow Blocks:   %d (>%d ms)\n", result->slow_blocks,
             LATENCY_SLOW_US / 1000);
  gfx_printf("  Latency - Min: %d us, Max: %d us, Avg: %d us\n\n",
             result->min_latency_us == 0xFFFFFFFF ? 0 : result->min_latency_us,
             result->max_latency_us, avg);
}

// Run a test and display results
static void run_and_display_test(test_mode_t mode) {
  gfx_clearscreen();
  gfx_printf("\n%k=== SD Card Read Tester v%d.%d ===%k\n\n", COLOR_TITLE,
             SD_TESTER_VER_MJ, SD_TESTER_VER_MN, COLOR_INFO);

  // Get card info
  sd_card_info_t card_info;
  sd_tester_get_card_info(&card_info);
  gfx_printf("Card: %d GB (%s)\n\n", card_info.capacity_gb,
             card_info.speed_mode);

  sd_test_result_t seq_result, btf_result;
  int ran_seq = 0, ran_btf = 0;

  switch (mode) {
  case TEST_SEQ_FAST:
    gfx_printf("%kRunning Fast Sequential Test (512 MB)...%k\n\n",
               COLOR_WARNING, COLOR_INFO);
    sd_tester_run_sequential(&seq_result, FAST_TEST_SECTORS,
                             seq_progress_callback);
    ran_seq = 1;
    break;

  case TEST_SEQ_FULL:
    gfx_printf("%kRunning Full Sequential Test (%d GB)...%k\n\n", COLOR_WARNING,
               card_info.capacity_gb, COLOR_INFO);
    sd_tester_run_sequential(&seq_result, 0, seq_progress_callback);
    ran_seq = 1;
    break;

  case TEST_BTF_FAST:
    gfx_printf("%kRunning Fast Butterfly Test (%d iterations)...%k\n\n",
               COLOR_WARNING, FAST_BUTTERFLY_ITER, COLOR_INFO);
    sd_tester_run_butterfly(&btf_result, FAST_BUTTERFLY_ITER,
                            btf_progress_callback);
    ran_btf = 1;
    break;

  case TEST_BTF_FULL:
    gfx_printf("%kRunning Full Butterfly Test...%k\n\n", COLOR_WARNING,
               COLOR_INFO);
    sd_tester_run_butterfly(&btf_result, 0, btf_progress_callback);
    ran_btf = 1;
    break;

  case TEST_ALL_FAST:
    gfx_printf("%kRunning All Fast Tests...%k\n\n", COLOR_WARNING, COLOR_INFO);
    sd_tester_run_sequential(&seq_result, FAST_TEST_SECTORS,
                             seq_progress_callback);
    ran_seq = 1;
    gfx_printf("\n");
    sd_tester_run_butterfly(&btf_result, FAST_BUTTERFLY_ITER,
                            btf_progress_callback);
    ran_btf = 1;
    break;

  case TEST_ALL_FULL:
    gfx_printf("%kRunning All Full Tests...%k\n\n", COLOR_WARNING, COLOR_INFO);
    sd_tester_run_sequential(&seq_result, 0, seq_progress_callback);
    ran_seq = 1;
    gfx_printf("\n");
    sd_tester_run_butterfly(&btf_result, 0, btf_progress_callback);
    ran_btf = 1;
    break;
  }

  // Display results
  gfx_printf("\n\n%k=== Test Results ===%k\n\n", COLOR_TITLE, COLOR_INFO);

  if (ran_seq)
    display_result("Sequential Read Test", &seq_result);
  if (ran_btf)
    display_result("Butterfly Read Test", &btf_result);

  // Overall pass/fail
  int passed = 1;
  if (ran_seq && !sd_tester_is_passed(&seq_result))
    passed = 0;
  if (ran_btf && !sd_tester_is_passed(&btf_result))
    passed = 0;

  if (passed) {
    gfx_printf("%k[PASSED]%k - SD card passed all read tests!\n", COLOR_SUCCESS,
               COLOR_INFO);
  } else {
    u32 total_errors = 0;
    if (ran_seq)
      total_errors += seq_result.read_errors;
    if (ran_btf)
      total_errors += btf_result.read_errors;
    gfx_printf("%k[FAILED]%k - %d read errors detected!\n", COLOR_ERROR,
               COLOR_INFO, total_errors);
  }

  gfx_printf("\nPress any button to continue...\n");
  hidWait();
}

// Main menu
static void show_main_menu(void) {
  while (1) {
    gfx_clearscreen();
    gfx_printf("\n%k=== SD Card Read Tester v%d.%d ===%k\n\n", COLOR_TITLE,
               SD_TESTER_VER_MJ, SD_TESTER_VER_MN, COLOR_INFO);

    // Show card info
    sd_card_info_t card_info;
    sd_tester_get_card_info(&card_info);
    gfx_printf("Card: %d GB (%s)\n", card_info.capacity_gb,
               card_info.speed_mode);
    gfx_printf("Total Sectors: %d\n\n", card_info.total_sectors);

    // Build menu entries
    MenuEntry_t entries[] = {
        {.name = "Fast Sequential (512 MB)", .optionUnion = 0xFFFFFFFF},
        {.name = "Fast Butterfly (512 iter)", .optionUnion = 0xFFFFFFFF},
        {.name = "Full Sequential (Full Card)", .optionUnion = 0xFFFFBA00},
        {.name = "Full Butterfly (Full Card)", .optionUnion = 0xFFFFBA00},
        {.name = "------------------------",
         .optionUnion = SKIPBIT | 0xFF555555},
        {.name = "Run All Fast Tests", .optionUnion = 0xFF96FF00},
        {.name = "Run All Full Tests", .optionUnion = 0xFFFFDD00},
        {.name = "------------------------",
         .optionUnion = SKIPBIT | 0xFF555555},
        {.name = "Exit", .optionUnion = 0xFFFF4444},
    };

    Vector_t menu = vecFromArray(entries, 9, sizeof(MenuEntry_t));
    gfx_printf("Select a test:\n\n");
    int sel = newMenu(&menu, 0, 40, 12, ENABLEB, 9);

    switch (sel) {
    case 0:
      run_and_display_test(TEST_SEQ_FAST);
      break;
    case 1:
      run_and_display_test(TEST_BTF_FAST);
      break;
    case 2:
      run_and_display_test(TEST_SEQ_FULL);
      break;
    case 3:
      run_and_display_test(TEST_BTF_FULL);
      break;
    case 5:
      run_and_display_test(TEST_ALL_FAST);
      break;
    case 6:
      run_and_display_test(TEST_ALL_FULL);
      break;
    case 8:
      return; // Exit
    default:
      break;
    }
  }
}

void ipl_main(void) {
  // Hardware initialization
  hw_init();

  // Pivot stack for more space
  pivot_stack(IPL_STACK_TOP);

  // Initialize heap
  heap_init((void *)IPL_HEAP_START);

  // Set default configuration
  memset(&h_cfg, 0, sizeof(hekate_config));

  // Mount SD card first (before display, for error messages)
  int sd_mounted = sd_mount();

  // Initialize display
  display_init();
  u32 *fb = display_init_window_a_pitch();
  gfx_init_ctxt(fb, 720, 1280, 720);
  gfx_con_init();

  display_backlight_pwm_init();
  display_backlight_brightness(100, 1000);

  // Overclock BPMP
  bpmp_clk_rate_set(BPMP_CLK_DEFAULT_BOOST);

  // Initialize HID
  hidInit();

  gfx_clearscreen();

  if (!sd_mounted) {
    gfx_printf("\n%kError:%k Failed to mount SD card!\n", COLOR_ERROR,
               COLOR_INFO);
    gfx_printf("Make sure a valid SD card is inserted.\n\n");
    gfx_printf("Press any button to exit...\n");
    hidWait();
  } else {
    // Show main menu
    show_main_menu();
  }

  // Cleanup
  sd_end();

  gfx_clearscreen();
  gfx_printf("\n%kSD Card Tester exiting...%k\n", COLOR_INFO, COLOR_INFO);
  msleep(500);

  // Power off or reboot
  power_set_state(POWER_OFF_REBOOT);

  // Halt if we get here
  while (1)
    bpmp_halt();
}
