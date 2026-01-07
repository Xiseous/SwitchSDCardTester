#pragma once
#include "gfx.h"
#include "menu.h"

#define COLOR_WHITE 0xFFFFFFFF
#define COLOR_BLACK 0xFF000000
#define COLOR_DEFAULT 0xFF000000
#define COLOR_GREY 0xFF888888
#define COLOR_DARKGREY 0xFF333333
#define COLOR_ORANGE 0xFFFFA500
#define COLOR_GREEN 0xFF008000
#define COLOR_VIOLET 0xFFEE82EE
#define COLOR_BLUE 0xFF0000FF
#define COLOR_RED 0xFF0000FF
#define COLOR_YELLOW 0xFFFF0000

#define COLORTORGB(color) (color & 0x00FFFFFF)
#define SETCOLOR(fg, bg) gfx_con_setcol(fg, 1, bg)
#define RESETCOLOR SETCOLOR(COLOR_WHITE, COLOR_BLACK);

#define RGBUnionToU32(optionUnion) (optionUnion | 0xFF000000)

void gfx_clearscreen();
int MakeHorizontalMenu(MenuEntry_t *entries, int len, int spacesBetween, u32 bg,
                       int startPos);
int MakeYesNoHorzMenu(int spacesBetween, u32 bg);
void gfx_printTopInfo();