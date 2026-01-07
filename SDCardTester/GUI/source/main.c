/*
 * SD Card Read Tester - GUI Version (Nyx/LVGL)
 * Main Entry Point
 *
 * Copyright (c) 2026
 *
 * Based on Hekate Nyx GUI
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms and conditions of the GNU General Public License,
 * version 2, as published by the Free Software Foundation.
 */

#include <stdlib.h>
#include <string.h>

#include "config.h"
#include <bdk.h>

#include "gfx/gfx.h"
#include <libs/lvgl/lvgl.h>

#include "sd_tester.h"

// Boot configuration
hekate_config h_cfg;
boot_cfg_t __attribute__((section("._boot_cfg"))) b_cfg;

extern void pivot_stack(u32 stack_top);

// GUI state
static lv_obj_t *main_win = NULL;
static lv_obj_t *progress_bar = NULL;
static lv_obj_t *status_label = NULL;
static lv_obj_t *result_label = NULL;

// Styles
static lv_style_t style_title;
static lv_style_t style_header;
static lv_style_t style_result;

// Forward declarations
static void create_main_menu(void);
static void run_test_gui(test_mode_t mode);

// Progress callback adapter for GUI
static sd_test_result_t *current_result = NULL;
static u32 current_mode = 0;

static void gui_seq_progress(u32 current, u32 total, u32 latency, u32 errors) {
  if (total > 0) {
    u32 percent = current * 100 / total;
    lv_bar_set_value(progress_bar, percent);

    char buf[128];
    s_printf(buf, "Sequential: %d%% | Latency: %d us | Errors: %d", percent,
             latency, errors);
    lv_label_set_text(status_label, buf);

    // Process LV tasks to update display
    lv_task_handler();
  }
}

static void gui_btf_progress(u32 current, u32 total, u32 lat_low,
                             u32 lat_high) {
  if (total > 0) {
    u32 percent = current * 100 / total;
    lv_bar_set_value(progress_bar, percent);

    char buf[128];
    s_printf(buf, "Butterfly: %d%% | Low: %d us | High: %d us", percent,
             lat_low, lat_high);
    lv_label_set_text(status_label, buf);

    lv_task_handler();
  }
}

// Message box button callback
static lv_res_t mbox_action(lv_obj_t *mbox, const char *txt) {
  lv_obj_del(mbox->par); // Delete dark background (parent)
  create_main_menu();
  return LV_RES_INV;
}

// Display results in a message box
static void display_results_gui(test_mode_t mode, sd_test_result_t *seq,
                                sd_test_result_t *btf) {
  // Create dark background
  lv_obj_t *dark_bg = lv_obj_create(lv_scr_act(), NULL);
  lv_obj_set_size(dark_bg, LV_HOR_RES, LV_VER_RES);

  static lv_style_t darken_style;
  lv_style_copy(&darken_style, &lv_style_plain);
  darken_style.body.main_color = LV_COLOR_BLACK;
  darken_style.body.grad_color = LV_COLOR_BLACK;
  darken_style.body.opa = LV_OPA_70;
  lv_obj_set_style(dark_bg, &darken_style);

  // Create message box
  static const char *mbox_btns[] = {"OK", ""};
  lv_obj_t *mbox = lv_mbox_create(dark_bg, NULL);
  lv_mbox_set_recolor(mbox, true);

  char result_buf[1024];
  char *p = result_buf;

  s_printf(p, "#00CCFF SD Card Test Results#\n\n");
  p += strlen(p);

  // Sequential results
  if (mode == TEST_SEQ_FAST || mode == TEST_SEQ_FULL || mode == TEST_ALL_FAST ||
      mode == TEST_ALL_FULL) {
    u32 avg = sd_tester_get_avg_latency(seq);
    s_printf(p, "#FFBA00 Sequential Read Test#\n");
    p += strlen(p);
    s_printf(p, "Blocks: %d | Errors: %d\n", seq->blocks_tested,
             seq->read_errors);
    p += strlen(p);
    s_printf(p, "Latency: Min %d / Max %d / Avg %d us\n",
             seq->min_latency_us == 0xFFFFFFFF ? 0 : seq->min_latency_us,
             seq->max_latency_us, avg);
    p += strlen(p);
    s_printf(p, "Slow blocks (>5ms): %d\n\n", seq->slow_blocks);
    p += strlen(p);
  }

  // Butterfly results
  if (mode == TEST_BTF_FAST || mode == TEST_BTF_FULL || mode == TEST_ALL_FAST ||
      mode == TEST_ALL_FULL) {
    u32 avg = sd_tester_get_avg_latency(btf);
    s_printf(p, "#FFBA00 Butterfly Read Test#\n");
    p += strlen(p);
    s_printf(p, "Blocks: %d | Errors: %d\n", btf->blocks_tested,
             btf->read_errors);
    p += strlen(p);
    s_printf(p, "Latency: Min %d / Max %d / Avg %d us\n",
             btf->min_latency_us == 0xFFFFFFFF ? 0 : btf->min_latency_us,
             btf->max_latency_us, avg);
    p += strlen(p);
    s_printf(p, "Slow blocks (>5ms): %d\n\n", btf->slow_blocks);
    p += strlen(p);
  }

  // Overall result
  int passed = 1;
  u32 total_errors = 0;
  if (seq) {
    if (!sd_tester_is_passed(seq))
      passed = 0;
    total_errors += seq->read_errors;
  }
  if (btf) {
    if (!sd_tester_is_passed(btf))
      passed = 0;
    total_errors += btf->read_errors;
  }

  if (passed) {
    s_printf(p, "#96FF00 [PASSED]# SD card passed all tests!");
  } else {
    s_printf(p, "#FF0000 [FAILED]# %d read errors detected!", total_errors);
  }

  lv_mbox_set_text(mbox, result_buf);
  lv_mbox_add_btns(mbox, mbox_btns, mbox_action);
  lv_obj_set_width(mbox, LV_HOR_RES * 2 / 3);
  lv_obj_align(mbox, NULL, LV_ALIGN_CENTER, 0, 0);
}

// Run test with GUI progress
static void run_test_gui(test_mode_t mode) {
  // Clear main window content
  lv_obj_clean(main_win);

  // Create test progress UI
  lv_obj_t *title = lv_label_create(main_win, NULL);
  lv_label_set_recolor(title, true);
  lv_label_set_text(title, "#00CCFF Running Test...#");
  lv_obj_align(title, NULL, LV_ALIGN_IN_TOP_MID, 0, LV_DPI / 4);

  // Progress bar
  progress_bar = lv_bar_create(main_win, NULL);
  lv_obj_set_size(progress_bar, LV_HOR_RES * 2 / 3, LV_DPI / 4);
  lv_obj_align(progress_bar, NULL, LV_ALIGN_CENTER, 0, -LV_DPI / 2);
  lv_bar_set_value(progress_bar, 0);

  // Status label
  status_label = lv_label_create(main_win, NULL);
  lv_label_set_text(status_label, "Initializing...");
  lv_obj_align(status_label, progress_bar, LV_ALIGN_OUT_BOTTOM_MID, 0,
               LV_DPI / 4);

  lv_task_handler();

  // Run tests
  sd_test_result_t seq_result = {0}, btf_result = {0};
  sd_test_result_t *seq_ptr = NULL, *btf_ptr = NULL;

  switch (mode) {
  case TEST_SEQ_FAST:
    sd_tester_run_sequential(&seq_result, FAST_TEST_SECTORS, gui_seq_progress);
    seq_ptr = &seq_result;
    break;
  case TEST_SEQ_FULL:
    sd_tester_run_sequential(&seq_result, 0, gui_seq_progress);
    seq_ptr = &seq_result;
    break;
  case TEST_BTF_FAST:
    sd_tester_run_butterfly(&btf_result, FAST_BUTTERFLY_ITER, gui_btf_progress);
    btf_ptr = &btf_result;
    break;
  case TEST_BTF_FULL:
    sd_tester_run_butterfly(&btf_result, 0, gui_btf_progress);
    btf_ptr = &btf_result;
    break;
  case TEST_ALL_FAST:
    sd_tester_run_sequential(&seq_result, FAST_TEST_SECTORS, gui_seq_progress);
    lv_bar_set_value(progress_bar, 0);
    sd_tester_run_butterfly(&btf_result, FAST_BUTTERFLY_ITER, gui_btf_progress);
    seq_ptr = &seq_result;
    btf_ptr = &btf_result;
    break;
  case TEST_ALL_FULL:
    sd_tester_run_sequential(&seq_result, 0, gui_seq_progress);
    lv_bar_set_value(progress_bar, 0);
    sd_tester_run_butterfly(&btf_result, 0, gui_btf_progress);
    seq_ptr = &seq_result;
    btf_ptr = &btf_result;
    break;
  }

  display_results_gui(mode, seq_ptr, btf_ptr);
}

// Button actions
static lv_res_t btn_test_seq_fast(lv_obj_t *btn) {
  run_test_gui(TEST_SEQ_FAST);
  return LV_RES_OK;
}
static lv_res_t btn_test_seq_full(lv_obj_t *btn) {
  run_test_gui(TEST_SEQ_FULL);
  return LV_RES_OK;
}
static lv_res_t btn_test_btf_fast(lv_obj_t *btn) {
  run_test_gui(TEST_BTF_FAST);
  return LV_RES_OK;
}
static lv_res_t btn_test_btf_full(lv_obj_t *btn) {
  run_test_gui(TEST_BTF_FULL);
  return LV_RES_OK;
}
static lv_res_t btn_test_all_fast(lv_obj_t *btn) {
  run_test_gui(TEST_ALL_FAST);
  return LV_RES_OK;
}
static lv_res_t btn_test_all_full(lv_obj_t *btn) {
  run_test_gui(TEST_ALL_FULL);
  return LV_RES_OK;
}

static lv_res_t btn_exit(lv_obj_t *btn) {
  sd_end();
  power_set_state(POWER_OFF_REBOOT);
  return LV_RES_OK;
}

// Create a styled button
static lv_obj_t *create_btn(lv_obj_t *parent, const char *label,
                            lv_action_t action) {
  lv_obj_t *btn = lv_btn_create(parent, NULL);
  lv_btn_set_fit(btn, true, true);
  lv_btn_set_action(btn, LV_BTN_ACTION_CLICK, action);

  lv_obj_t *lbl = lv_label_create(btn, NULL);
  lv_label_set_text(lbl, label);

  return btn;
}

// Create main menu
static void create_main_menu(void) {
  // Clear screen
  lv_obj_clean(lv_scr_act());

  // Create main container
  main_win = lv_cont_create(lv_scr_act(), NULL);
  lv_obj_set_size(main_win, LV_HOR_RES, LV_VER_RES);
  lv_cont_set_layout(main_win, LV_LAYOUT_COL_M);

  // Title
  lv_obj_t *title = lv_label_create(main_win, NULL);
  lv_label_set_recolor(title, true);
  lv_label_set_text(title,
                    "#00CCFF SD Card Read Tester v" STRINGIFY(
                        SD_TESTER_VER_MJ) "." STRINGIFY(SD_TESTER_VER_MN) "#");

  // Card info
  sd_card_info_t card_info;
  sd_tester_get_card_info(&card_info);

  lv_obj_t *card_lbl = lv_label_create(main_win, NULL);
  char card_buf[64];
  s_printf(card_buf, "Card: %d GB (%s)", card_info.capacity_gb,
           card_info.speed_mode);
  lv_label_set_text(card_lbl, card_buf);

  // Separator
  lv_obj_t *sep1 = lv_label_create(main_win, NULL);
  lv_label_set_text(sep1, "");

  // Fast tests section
  lv_obj_t *fast_lbl = lv_label_create(main_win, NULL);
  lv_label_set_recolor(fast_lbl, true);
  lv_label_set_text(fast_lbl, "#96FF00 Fast Tests (Quick)#");

  lv_obj_t *btn_cont1 = lv_cont_create(main_win, NULL);
  lv_cont_set_layout(btn_cont1, LV_LAYOUT_ROW_M);
  lv_cont_set_fit(btn_cont1, true, true);

  create_btn(btn_cont1, "Sequential 512MB", btn_test_seq_fast);
  create_btn(btn_cont1, "Butterfly 512 iter", btn_test_btf_fast);
  create_btn(btn_cont1, "All Fast", btn_test_all_fast);

  // Full tests section
  lv_obj_t *full_lbl = lv_label_create(main_win, NULL);
  lv_label_set_recolor(full_lbl, true);
  lv_label_set_text(full_lbl, "#FFBA00 Full Tests (Thorough)#");

  lv_obj_t *btn_cont2 = lv_cont_create(main_win, NULL);
  lv_cont_set_layout(btn_cont2, LV_LAYOUT_ROW_M);
  lv_cont_set_fit(btn_cont2, true, true);

  create_btn(btn_cont2, "Sequential Full", btn_test_seq_full);
  create_btn(btn_cont2, "Butterfly Full", btn_test_btf_full);
  create_btn(btn_cont2, "All Full", btn_test_all_full);

  // Exit section
  lv_obj_t *sep2 = lv_label_create(main_win, NULL);
  lv_label_set_text(sep2, "");

  create_btn(main_win, "Exit", btn_exit);
}

// LVGL display flush callback
static void disp_flush(int32_t x1, int32_t y1, int32_t x2, int32_t y2,
                       const lv_color_t *color_p) {
  // Implement based on Nyx's display driver
  gfx_set_rect_argb((u32 *)color_p, x2 - x1 + 1, y2 - y1 + 1, x1, y1);
  lv_flush_ready();
}

void ipl_main(void) {
  // Hardware initialization
  hw_init();
  pivot_stack(IPL_STACK_TOP);
  heap_init((void *)IPL_HEAP_START);

  memset(&h_cfg, 0, sizeof(hekate_config));

  // Mount SD card
  int sd_mounted = sd_mount();

  // Initialize display
  display_init();
  u32 *fb = display_init_window_a_pitch();
  gfx_init_ctxt(fb, 720, 1280, 720);
  gfx_con_init();

  display_backlight_pwm_init();
  display_backlight_brightness(100, 1000);

  bpmp_clk_rate_set(BPMP_CLK_DEFAULT_BOOST);

  if (!sd_mounted) {
    gfx_printf("\n\nError: Failed to mount SD card!\n");
    gfx_printf("Press POWER to exit...\n");
    while (!(btn_read() & BTN_POWER))
      ;
    power_set_state(POWER_OFF_REBOOT);
  }

  // Initialize LVGL
  lv_init();

  // Register display driver
  lv_disp_drv_t disp_drv;
  lv_disp_drv_init(&disp_drv);
  disp_drv.disp_flush = disp_flush;
  lv_disp_drv_register(&disp_drv);

  // Create main menu
  create_main_menu();

  // Main loop
  while (1) {
    lv_task_handler();
    msleep(5);
  }
}
