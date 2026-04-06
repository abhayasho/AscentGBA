#include "menu_ui.h"
#include "gba.h"
#include "mode4.h"

#include "menu_font.inc"

static void drawChar5x7(int x, int y, char c, u8 color, int scale) {
    int idx = (int)c - 32;
    if (idx < 0 || idx >= 95) {
        idx = 0;
    }
    const u8 *g = FONT5X7[idx];
    for (int col = 0; col < 5; col++) {
        u8 b = g[col];
        for (int row = 0; row < 7; row++) {
            if (b & (1 << row)) {
                for (int sy = 0; sy < scale; sy++) {
                    for (int sx = 0; sx < scale; sx++) {
                        int px = x + col * scale + sx;
                        int py = y + row * scale + sy;
                        if (px >= 0 && px < SCREENWIDTH && py >= 0 && py < SCREENHEIGHT) {
                            setPixel4(px, py, color);
                        }
                    }
                }
            }
        }
    }
}

static void drawString5x7(int x, int y, const char *s, u8 color, int scale) {
    int cx = x;
    int cy = y;
    while (*s) {
        if (*s == '\n') {
            cy += 7 * scale + scale * 2;
            cx = x;
            s++;
            continue;
        }
        drawChar5x7(cx, cy, *s, color, scale);
        cx += 6 * scale;
        if (cx > SCREENWIDTH - 6 * scale) {
            cx = x;
            cy += 7 * scale + scale;
        }
        s++;
    }
}

void uiDrawSplash(void) {
    drawRect4(8, 8, 224, 2, 6);
    drawString5x7(36, 28, "ASCENT", 6, 2);
    drawString5x7(8, 64, "START: BEGIN GAME", 6, 1);
    drawString5x7(8, 80, "SELECT: HELP", 6, 1);
}

void uiDrawInstructions(void) {
    drawString5x7(4, 8, "INSTRUCTIONS", 6, 1);
    drawString5x7(4, 28, "DPAD: MOVE", 6, 1);
    drawString5x7(4, 42, "A: SWORD", 6, 1);
    drawString5x7(4, 56, "START: PAUSE", 6, 1);
    drawString5x7(4, 70, "SELECT: MENU", 6, 1);
    drawString5x7(4, 90, "B: BACK", 6, 1);
    drawString5x7(4, 110, "START OR A: PLAY", 6, 1);
}

void uiDrawPause(void) {
    drawString5x7(16, 60, "PAUSED", 6, 2);
    drawString5x7(8, 92, "START: RESUME", 6, 1);
    drawString5x7(8, 108, "SELECT: QUIT", 6, 1);
}

void uiDrawWin(void) {
    drawString5x7(40, 56, "YOU WIN", 6, 2);
    drawString5x7(8, 92, "START: MENU", 6, 1);
}

void uiDrawLose(void) {
    drawString5x7(32, 56, "YOU LOSE", 6, 2);
    drawString5x7(8, 92, "START: MENU", 6, 1);
}
