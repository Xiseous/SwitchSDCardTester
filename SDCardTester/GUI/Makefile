ifeq ($(strip $(DEVKITARM)),)
$(error "Please set DEVKITARM in your environment. export DEVKITARM=<path to>devkitARM")
endif

include $(DEVKITARM)/base_rules

################################################################################

IPL_LOAD_ADDR := 0x40008000
VERSION_MAJOR := 1
VERSION_MINOR := 0
VERSION_BUGFX := 0

################################################################################

TARGET := SDCardTester_GUI
BUILDDIR := build
OUTPUTDIR := output
SOURCEDIR = source
BDKDIR := bdk
LVGLDIR := $(BDKDIR)/libs/lvgl
BDKINC := -I./$(BDKDIR) -I./$(LVGLDIR)
VPATH = $(dir ./$(SOURCEDIR)/) $(dir $(wildcard ./$(SOURCEDIR)/*/)) $(dir $(wildcard ./$(SOURCEDIR)/*/*/))
VPATH += $(dir $(wildcard ./$(BDKDIR)/)) $(dir $(wildcard ./$(BDKDIR)/*/)) $(dir $(wildcard ./$(BDKDIR)/*/*/)) $(dir $(wildcard ./$(BDKDIR)/*/*/*/))
VPATH += $(dir $(wildcard ./$(LVGLDIR)/)) $(dir $(wildcard ./$(LVGLDIR)/*/)) $(dir $(wildcard ./$(LVGLDIR)/*/*/))

# Main source files
OBJS = $(addprefix $(BUILDDIR)/$(TARGET)/, \
	start.o main.o sd_tester.o gfx.o \
)

# Hardware from BDK
OBJS += $(addprefix $(BUILDDIR)/$(TARGET)/, \
	bpmp.o ccplex.o clock.o di.o i2c.o irq.o timer.o \
	gpio.o pinmux.o pmc.o se.o smmu.o tsec.o uart.o \
	fuse.o kfuse.o \
	mc.o sdram.o minerva.o \
	sdmmc.o sdmmc_driver.o emmc.o sd.o \
	bq24193.o max17050.o max7762x.o max77620-rtc.o regulator_5v.o \
	hw_init.o exception_handlers.o \
)

# Utilities from BDK
OBJS += $(addprefix $(BUILDDIR)/$(TARGET)/, \
	btn.o util.o ini.o sprintf.o heap.o \
)

# FatFS from BDK
OBJS += $(addprefix $(BUILDDIR)/$(TARGET)/, \
	diskio.o ff.o ffunicode.o ffsystem.o \
)

# LVGL core
OBJS += $(addprefix $(BUILDDIR)/$(TARGET)/, \
	lv_group.o lv_indev.o lv_obj.o lv_refr.o lv_style.o lv_vdb.o \
)

# LVGL draw
OBJS += $(addprefix $(BUILDDIR)/$(TARGET)/, \
	lv_draw.o lv_draw_rbasic.o lv_draw_vbasic.o lv_draw_arc.o lv_draw_img.o \
	lv_draw_label.o lv_draw_line.o lv_draw_rect.o lv_draw_triangle.o \
)

# LVGL hal
OBJS += $(addprefix $(BUILDDIR)/$(TARGET)/, \
	lv_hal_disp.o lv_hal_indev.o lv_hal_tick.o \
)

# LVGL fonts (basic)
OBJS += $(addprefix $(BUILDDIR)/$(TARGET)/, \
	lv_font_builtin.o \
)

# LVGL misc
OBJS += $(addprefix $(BUILDDIR)/$(TARGET)/, \
	lv_anim.o lv_area.o lv_circ.o lv_color.o lv_font.o lv_ll.o lv_math.o lv_mem.o lv_task.o lv_txt.o lv_gc.o lv_log.o \
)

# LVGL objects
OBJS += $(addprefix $(BUILDDIR)/$(TARGET)/, \
	lv_bar.o lv_btn.o lv_btnm.o lv_cb.o lv_cont.o lv_ddlist.o lv_img.o lv_label.o lv_line.o lv_list.o \
	lv_mbox.o lv_page.o lv_roller.o lv_slider.o lv_sw.o lv_tabview.o lv_ta.o lv_win.o lv_imgbtn.o \
)

# LVGL themes
OBJS += $(addprefix $(BUILDDIR)/$(TARGET)/, \
	lv_theme.o \
)

GFX_INC   := '"../source/gfx/gfx.h"'
FFCFG_INC := '"fatfs_conf.h"'

################################################################################

CUSTOMDEFINES := -DIPL_LOAD_ADDR=$(IPL_LOAD_ADDR)
CUSTOMDEFINES += -DSD_TESTER_VER_MJ=$(VERSION_MAJOR) -DSD_TESTER_VER_MN=$(VERSION_MINOR) -DSD_TESTER_VER_BF=$(VERSION_BUGFX)
CUSTOMDEFINES += -DGFX_INC=$(GFX_INC) -DFFCFG_INC=$(FFCFG_INC)
CUSTOMDEFINES += '-DSTRINGIFY(x)=\#x'

ARCH := -march=armv4t -mtune=arm7tdmi -mthumb -mthumb-interwork
CFLAGS = $(ARCH) -Os -nostdlib -ffunction-sections -fdata-sections -fomit-frame-pointer -fno-inline -std=gnu11 -Wall -Wno-missing-braces $(CUSTOMDEFINES)
LDFLAGS = $(ARCH) -nostartfiles -lgcc -Wl,--nmagic,--gc-sections -Xlinker --defsym=IPL_LOAD_ADDR=$(IPL_LOAD_ADDR)

################################################################################

.PHONY: all clean

all: $(OUTPUTDIR)/$(TARGET).bin
	@echo -n "Payload size is "
	$(eval BIN_SIZE = $(shell wc -c < $(OUTPUTDIR)/$(TARGET).bin))
	@echo $(BIN_SIZE)
	@echo "Max recommended size is 126296 Bytes."

clean:
	@rm -rf $(BUILDDIR)
	@rm -rf $(OUTPUTDIR)

$(OUTPUTDIR)/$(TARGET).bin: $(BUILDDIR)/$(TARGET)/$(TARGET).elf
	@mkdir -p "$(@D)"
	$(OBJCOPY) -S -O binary $< $@

$(BUILDDIR)/$(TARGET)/$(TARGET).elf: $(OBJS)
	$(CC) $(LDFLAGS) -T $(SOURCEDIR)/link.ld $^ -o $@

$(BUILDDIR)/$(TARGET)/%.o: %.c
	@mkdir -p "$(@D)"
	$(CC) $(CFLAGS) $(BDKINC) -c $< -o $@

$(BUILDDIR)/$(TARGET)/%.o: %.S
	@mkdir -p "$(@D)"
	$(CC) $(CFLAGS) -c $< -o $@
