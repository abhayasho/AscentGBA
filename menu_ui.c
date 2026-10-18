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

static void drawStringShadow(int x, int y, const char *s, u8 fg, u8 shadow, int scale) {
    drawString5x7(x + scale, y + scale, s, shadow, scale);
    drawString5x7(x, y, s, fg, scale);
}

static void drawHBands(int y0, int y1, const u8 *colors, int count) {
    int total = y1 - y0;
    if (total <= 0 || count <= 0) {
        return;
    }
    for (int i = 0; i < count; i++) {
        int by = y0 + (i * total) / count;
        int by_end = y0 + ((i + 1) * total) / count;
        int h = by_end - by;
        if (h <= 0) {
            continue;
        }
        drawRect4(0, by, SCREENWIDTH, h, colors[i]);
    }
}

static void drawCircleStep(int cx, int cy, int rOuter, int rInner, u8 color) {
    int r2o = rOuter * rOuter;
    int r2i = rInner * rInner;
    for (int dy = -rOuter; dy <= rOuter; dy++) {
        int yy = cy + dy;
        if (yy < 0 || yy >= SCREENHEIGHT) {
            continue;
        }
        int yy2 = dy * dy;
        for (int dx = -rOuter; dx <= rOuter; dx++) {
            int xx = cx + dx;
            if (xx < 0 || xx >= SCREENWIDTH) {
                continue;
            }
            int d = dx * dx + yy2;
            if (d <= r2o && d >= r2i) {
                setPixel4(xx, yy, color);
            }
        }
    }
}

static void drawDiamond(int cx, int cy, int half, u8 color) {
    for (int dy = -half; dy <= half; dy++) {
        int yy = cy + dy;
        if (yy < 0 || yy >= SCREENHEIGHT) {
            continue;
        }
        int spread = half - (dy < 0 ? -dy : dy);
        int x0 = cx - spread;
        int w = spread * 2 + 1;
        if (x0 < 0) {
            w += x0;
            x0 = 0;
        }
        if (x0 + w > SCREENWIDTH) {
            w = SCREENWIDTH - x0;
        }
        if (w > 0) {
            drawRect4(x0, yy, w, 1, color);
        }
    }
}

static void uiSetupSplashPalette(void) {
    BG_PALETTE[0]  = RGB(0, 0, 0);
    BG_PALETTE[1]  = RGB(2, 1, 4);
    BG_PALETTE[2]  = RGB(5, 2, 9);
    BG_PALETTE[3]  = RGB(9, 4, 14);
    BG_PALETTE[4]  = RGB(14, 6, 18);
    BG_PALETTE[5]  = RGB(20, 9, 22);
    BG_PALETTE[6]  = RGB(31, 31, 31);
    BG_PALETTE[7]  = RGB(31, 12, 22);
    BG_PALETTE[8]  = RGB(31, 22, 28);
    BG_PALETTE[9]  = RGB(31, 28, 18);
    BG_PALETTE[10] = RGB(8, 6, 12);
    BG_PALETTE[11] = RGB(14, 11, 18);
    BG_PALETTE[12] = RGB(20, 16, 22);
    BG_PALETTE[13] = RGB(28, 14, 4);
    BG_PALETTE[14] = RGB(31, 24, 10);
    BG_PALETTE[15] = RGB(20, 4, 4);
}

static void uiSetupWinPalette(void) {
    BG_PALETTE[0]  = RGB(0, 0, 0);
    BG_PALETTE[1]  = RGB(4, 2, 10);
    BG_PALETTE[2]  = RGB(10, 4, 16);
    BG_PALETTE[3]  = RGB(18, 8, 18);
    BG_PALETTE[4]  = RGB(26, 12, 18);
    BG_PALETTE[5]  = RGB(31, 18, 14);
    BG_PALETTE[6]  = RGB(31, 31, 31);
    BG_PALETTE[7]  = RGB(31, 24, 8);
    BG_PALETTE[8]  = RGB(31, 14, 22);
    BG_PALETTE[9]  = RGB(31, 28, 18);
    BG_PALETTE[10] = RGB(28, 28, 22);
    BG_PALETTE[11] = RGB(31, 31, 22);
    BG_PALETTE[12] = RGB(31, 18, 28);
    BG_PALETTE[13] = RGB(13, 4, 8);
    BG_PALETTE[14] = RGB(31, 22, 12);
    BG_PALETTE[15] = RGB(20, 8, 14);
}

static void uiSetupLosePalette(void) {
    BG_PALETTE[0]  = RGB(0, 0, 0);
    BG_PALETTE[1]  = RGB(2, 0, 2);
    BG_PALETTE[2]  = RGB(6, 1, 3);
    BG_PALETTE[3]  = RGB(10, 2, 4);
    BG_PALETTE[4]  = RGB(16, 3, 4);
    BG_PALETTE[5]  = RGB(22, 4, 6);
    BG_PALETTE[6]  = RGB(31, 31, 31);
    BG_PALETTE[7]  = RGB(31, 4, 6);
    BG_PALETTE[8]  = RGB(20, 14, 16);
    BG_PALETTE[9]  = RGB(31, 18, 18);
    BG_PALETTE[10] = RGB(8, 4, 6);
    BG_PALETTE[11] = RGB(12, 6, 8);
    BG_PALETTE[12] = RGB(18, 10, 12);
}

static void uiSetupMenuPalette(void) {
    BG_PALETTE[0]  = RGB(0, 0, 0);
    BG_PALETTE[1]  = RGB(2, 1, 4);
    BG_PALETTE[2]  = RGB(5, 3, 8);
    BG_PALETTE[3]  = RGB(8, 6, 12);
    BG_PALETTE[4]  = RGB(14, 10, 16);
    BG_PALETTE[5]  = RGB(22, 8, 14);
    BG_PALETTE[6]  = RGB(31, 31, 31);
    BG_PALETTE[7]  = RGB(31, 14, 22);
    BG_PALETTE[8]  = RGB(31, 22, 28);
    BG_PALETTE[9]  = RGB(31, 28, 18);
    BG_PALETTE[10] = RGB(11, 6, 14);
}

void uiDrawSplash(void) {
    uiSetupSplashPalette();

    static const u8 sky[] = {5, 5, 4, 4, 3, 3, 2, 2, 1};
    drawHBands(0, 96, sky, 9);
    drawRect4(0, 96, SCREENWIDTH, SCREENHEIGHT - 96, 0);

    static const unsigned char stars[][2] = {
        {10, 8}, {26, 16}, {44, 6}, {62, 22}, {78, 10}, {96, 28},
        {114, 6}, {132, 24}, {150, 12}, {168, 28}, {186, 6}, {220, 16},
        {6, 30}, {38, 34}, {52, 40}, {84, 36}, {120, 38}, {154, 32},
        {180, 40}, {214, 32}, {230, 42}, {16, 56}, {68, 60}, {148, 58}
    };
    for (unsigned i = 0; i < sizeof(stars) / sizeof(stars[0]); i++) {
        setPixel4(stars[i][0], stars[i][1], 6);
    }
    drawRect4(40, 12, 2, 2, 9);
    drawRect4(174, 22, 2, 2, 9);
    drawRect4(100, 50, 2, 2, 9);

    int mx = 200;
    int my = 22;
    drawCircleStep(mx, my, 10, 0, 9);
    drawCircleStep(mx + 4, my - 1, 8, 0, 4);

    static const struct {
        unsigned char x, y, w, h;
    } peaks[] = {
        {0, 78, 24, 18}, {4, 74, 14, 4}, {8, 70, 6, 4},
        {18, 84, 26, 12}, {26, 80, 10, 4},
        {38, 80, 22, 16}, {44, 76, 12, 4},
        {54, 86, 24, 10},
        {78, 82, 14, 14},
        {148, 82, 16, 14},
        {158, 80, 24, 16}, {166, 76, 12, 4},
        {178, 84, 22, 12}, {184, 80, 10, 4},
        {196, 78, 26, 18}, {204, 74, 14, 4}, {208, 70, 6, 4},
        {218, 84, 22, 12}
    };
    for (unsigned i = 0; i < sizeof(peaks) / sizeof(peaks[0]); i++) {
        drawRect4(peaks[i].x, peaks[i].y, peaks[i].w, peaks[i].h, 10);
    }

    int tx = 108;
    int ty = 56;
    drawRect4(tx + 2, ty + 12, 20, 28, 10);
    drawRect4(tx, ty + 10, 24, 2, 11);
    drawRect4(tx + 1, ty + 4, 4, 6, 11);
    drawRect4(tx + 10, ty + 4, 4, 6, 11);
    drawRect4(tx + 19, ty + 4, 4, 6, 11);
    drawRect4(tx + 7, ty + 16, 3, 4, 0);
    drawRect4(tx + 14, ty + 16, 3, 4, 0);
    drawRect4(tx + 9, ty + 24, 6, 6, 14);
    drawRect4(tx + 10, ty + 32, 4, 8, 0);

    drawRect4(tx - 2, ty + 14, 2, 4, 13);
    drawRect4(tx - 2, ty + 12, 2, 2, 14);
    drawRect4(tx + 24, ty + 14, 2, 4, 13);
    drawRect4(tx + 24, ty + 12, 2, 2, 14);

    drawString5x7(63, 109, "ASCENT", 7, 3);
    drawString5x7(67, 113, "ASCENT", 7, 3);
    drawString5x7(63, 113, "ASCENT", 7, 3);
    drawString5x7(67, 109, "ASCENT", 7, 3);
    drawString5x7(65, 111, "ASCENT", 6, 3);

    drawString5x7(48, 138, "5 SENSES  5 FLOORS", 8, 1);

    drawRect4(0, 150, SCREENWIDTH, 10, 1);
    drawRect4(0, 150, SCREENWIDTH, 1, 7);
    drawString5x7(6, 152, "START: PLAY", 6, 1);
    drawString5x7(150, 152, "SELECT: HELP", 6, 1);

    drawRect4(0, 0, SCREENWIDTH, 1, 7);
    drawRect4(0, SCREENHEIGHT - 1, SCREENWIDTH, 1, 7);
    drawRect4(0, 0, 1, SCREENHEIGHT, 7);
    drawRect4(SCREENWIDTH - 1, 0, 1, SCREENHEIGHT, 7);
}

void uiDrawWin(void) {
    uiSetupWinPalette();

    static const u8 sky[] = {1, 1, 2, 2, 3, 3, 4, 4, 5, 5, 7, 7, 5, 4, 3, 2};
    drawHBands(0, SCREENHEIGHT, sky, 16);

    int sunCx = 120;
    int sunCy = 56;

    drawCircleStep(sunCx, sunCy, 60, 56, 4);
    drawCircleStep(sunCx, sunCy, 56, 50, 3);
    drawCircleStep(sunCx, sunCy, 50, 44, 5);
    drawCircleStep(sunCx, sunCy, 44, 38, 7);
    drawCircleStep(sunCx, sunCy, 38, 30, 14);
    drawCircleStep(sunCx, sunCy, 30, 22, 9);
    drawCircleStep(sunCx, sunCy, 22, 14, 11);
    drawCircleStep(sunCx, sunCy, 14, 0, 6);

    static const int rays[] = {
        -88, 0,  88, 0,
        -80, -36, 80, -36,
        -80, 36,  80, 36,
        -56, -64, 56, -64,
        -56, 64,  56, 64,
        -28, -78, 28, -78,
        -28, 78,  28, 78
    };
    for (unsigned i = 0; i < sizeof(rays) / sizeof(rays[0]); i += 2) {
        int rx = sunCx + rays[i] - 4;
        int ry = sunCy + rays[i + 1] - 4;
        if (rx < 0 || ry < 0 || rx + 8 > SCREENWIDTH || ry + 8 > SCREENHEIGHT) {
            continue;
        }
        drawDiamond(rx + 4, ry + 4, 4, 11);
        drawDiamond(rx + 4, ry + 4, 2, 6);
    }

    static const unsigned char sparkles[][2] = {
        {12, 18}, {32, 8}, {60, 4}, {200, 6}, {220, 18}, {228, 30},
        {16, 40}, {12, 90}, {220, 90}, {6, 130}, {232, 130},
        {44, 130}, {196, 130}, {72, 142}, {168, 142}
    };
    for (unsigned i = 0; i < sizeof(sparkles) / sizeof(sparkles[0]); i++) {
        setPixel4(sparkles[i][0], sparkles[i][1], 11);
    }

    drawStringShadow(75, 102, "VICTORY", 6, 13, 2);
    drawString5x7(38, 124, "ALL SENSES RECLAIMED", 9, 1);
    drawString5x7(54, 136, "THE SOUL ASCENDS", 8, 1);

    drawString5x7(72, 150, "START: MAIN MENU", 6, 1);

    drawRect4(0, 0, SCREENWIDTH, 2, 7);
    drawRect4(0, SCREENHEIGHT - 2, SCREENWIDTH, 2, 7);
    drawRect4(0, 0, 2, SCREENHEIGHT, 7);
    drawRect4(SCREENWIDTH - 2, 0, 2, SCREENHEIGHT, 7);
}

void uiDrawLose(void) {
    uiSetupLosePalette();

    static const u8 sky[] = {1, 1, 2, 2, 3, 3, 4, 4, 5, 4, 3, 2};
    drawHBands(0, SCREENHEIGHT, sky, 12);

    static const unsigned char ash[][2] = {
        {18, 30}, {40, 22}, {62, 38}, {84, 18}, {110, 30}, {132, 22},
        {158, 38}, {184, 24}, {210, 30}, {32, 60}, {200, 60},
        {12, 90}, {44, 100}, {76, 92}, {120, 100}, {164, 96}, {200, 92},
        {228, 100}, {28, 134}, {72, 138}, {120, 142}, {174, 138}, {216, 134}
    };
    for (unsigned i = 0; i < sizeof(ash) / sizeof(ash[0]); i++) {
        setPixel4(ash[i][0], ash[i][1], 8);
    }

    int cx = 120;
    int cy = 60;
    drawDiamond(cx, cy, 24, 2);
    drawDiamond(cx, cy, 22, 5);
    drawDiamond(cx, cy, 18, 7);
    drawDiamond(cx, cy, 12, 4);

    drawStringShadow(60, 88, "GAME OVER", 9, 10, 2);
    drawString5x7(50, 116, "THE DARK RECLAIMS YOU", 7, 1);
    drawString5x7(80, 132, "RISE AGAIN?", 6, 1);
    drawString5x7(72, 148, "START: TRY AGAIN", 6, 1);

    drawRect4(0, 0, SCREENWIDTH, 2, 7);
    drawRect4(0, SCREENHEIGHT - 2, SCREENWIDTH, 2, 7);
}

void uiDrawPause(void) {
    uiSetupMenuPalette();

    static const u8 bg[] = {1, 1, 2, 2, 3, 4, 3, 2, 1};
    drawHBands(0, SCREENHEIGHT, bg, 9);

    drawRect4(20, 36, 200, 88, 2);
    drawRect4(22, 38, 196, 84, 4);
    drawRect4(24, 40, 192, 80, 1);

    drawStringShadow(86, 56, "PAUSED", 6, 7, 2);
    drawString5x7(48, 88, "START: RESUME GAME", 6, 1);
    drawString5x7(48, 102, "SELECT: QUIT TO MENU", 6, 1);

    drawRect4(0, 0, SCREENWIDTH, 1, 7);
    drawRect4(0, SCREENHEIGHT - 1, SCREENWIDTH, 1, 7);
}

void uiDrawInstructions(void) {
    uiSetupMenuPalette();

    static const u8 bg[] = {1, 1, 2, 2, 3, 3, 2, 2, 1};
    drawHBands(0, SCREENHEIGHT, bg, 9);

    drawRect4(0, 0, SCREENWIDTH, 14, 5);
    drawString5x7(54, 4, "HOW TO ASCEND", 6, 1);

    drawString5x7(6, 22, "GOAL: REGAIN 5 SENSES,", 6, 1);
    drawString5x7(6, 32, "REACH THE FINAL SOUL.", 6, 1);

    drawString5x7(6, 50, "DPAD MOVE  A SLASH", 8, 1);
    drawString5x7(6, 60, "B HOLD  START PAUSE", 8, 1);

    drawString5x7(6, 78, "TOTEM: SPEND 100 XP", 6, 1);
    drawString5x7(6, 88, "POOL: HEAL ON FOOT", 6, 1);

    drawString5x7(6, 106, "BOSS: PARRY RED ORBS", 7, 1);
    drawString5x7(6, 116, "FIVE PARRIES = WIN", 7, 1);

    drawString5x7(6, 134, "CHEAT TITLE SEL+B EASY", 7, 1);

    drawRect4(0, SCREENHEIGHT - 14, SCREENWIDTH, 14, 5);
    drawString5x7(8, SCREENHEIGHT - 10, "B BACK", 6, 1);
    drawString5x7(140, SCREENHEIGHT - 10, "A OR START PLAY", 6, 1);
}

static void drawNumSimple(int x, int y, int v, u8 color, int scale) {
    char buf[8];
    int n = 0;
    int t = v;
    int i;
    int j;
    if (t < 0) {
        t = 0;
    }
    if (t == 0) {
        drawString5x7(x, y, "0", color, scale);
        return;
    }
    while (t > 0 && n < 7) {
        buf[n++] = (char)('0' + (t % 10));
        t /= 10;
    }
    buf[n] = '\0';
    for (i = 0, j = n - 1; i < j; i++, j--) {
        char tmp = buf[i];
        buf[i] = buf[j];
        buf[j] = tmp;
    }
    drawString5x7(x, y, buf, color, scale);
}

void uiDrawSkillMenu(int sel, int str, int con, int mob, int pts, int xp) {
    uiSetupMenuPalette();

    static const u8 bg[] = {1, 2, 3, 4, 3, 2, 1};
    drawHBands(0, SCREENHEIGHT, bg, 7);

    drawRect4(14, 14, 212, 132, 10);
    drawRect4(16, 16, 208, 128, 3);
    drawRect4(18, 18, 204, 124, 1);

    drawRect4(18, 18, 204, 12, 5);
    drawString5x7(60, 21, "SKILL TOTEM", 6, 1);

    static const int rows[5] = {44, 58, 72, 86, 116};
    for (int r = 0; r < 5; r++) {
        if (sel == r) {
            drawRect4(24, rows[r] - 1, 4, 7, 7);
            drawRect4(28, rows[r] + 2, 2, 1, 7);
        }
    }
    drawString5x7(34, 44, "STR", 6, 1);
    drawNumSimple(96, 44, str, 7, 1);
    drawString5x7(34, 58, "CON", 6, 1);
    drawNumSimple(96, 58, con, 8, 1);
    drawString5x7(34, 72, "MOB", 6, 1);
    drawNumSimple(96, 72, mob, 9, 1);

    drawString5x7(34, 86, "CONVERT 100 XP", 6, 1);
    drawNumSimple(150, 86, xp, 9, 1);

    drawString5x7(34, 100, "SKILL PTS", 6, 1);
    drawNumSimple(110, 100, pts, 7, 1);

    drawString5x7(34, 116, "SAVE AND CLOSE", 6, 1);

    drawString5x7(22, 132, "A ADD/SAVE  B CANCEL  SEL SAVE", 8, 1);
}

void uiDrawFloorComplete(int completedFloor) {
    uiSetupWinPalette();

    static const u8 sky[] = {1, 2, 3, 4, 5, 7, 5, 4, 3, 2};
    drawHBands(0, SCREENHEIGHT, sky, 10);

    int cx = 120;
    int cy = 56;
    drawCircleStep(cx, cy, 32, 28, 5);
    drawCircleStep(cx, cy, 28, 22, 14);
    drawCircleStep(cx, cy, 22, 16, 9);
    drawCircleStep(cx, cy, 16, 10, 11);
    drawCircleStep(cx, cy, 10, 0, 6);

    static const int sparkles[][2] = {
        {30, 30}, {50, 18}, {72, 32}, {200, 18}, {214, 30}, {30, 90},
        {64, 100}, {180, 96}, {220, 92}, {12, 60}, {228, 60}
    };
    for (unsigned i = 0; i < sizeof(sparkles) / sizeof(sparkles[0]); i++) {
        setPixel4(sparkles[i][0], sparkles[i][1], 11);
    }

    const char *headline = "FLOOR ONE COMPLETE";
    const char *sense = "SIGHT REGAINED";
    const char *nextFloor = "FLOOR 2: SOUND";
    if (completedFloor == 2) {
        headline = "FLOOR TWO COMPLETE";
        sense = "SOUND REGAINED";
        nextFloor = "FLOOR 3: TASTE";
    } else if (completedFloor == 3) {
        headline = "FLOOR THREE COMPLETE";
        sense = "TASTE REGAINED";
        nextFloor = "FLOOR 4: SMELL";
    } else if (completedFloor == 4) {
        headline = "FLOOR FOUR COMPLETE";
        sense = "SMELL REGAINED";
        nextFloor = "FLOOR 5: TOUCH";
    } else if (completedFloor == 5) {
        headline = "FLOOR FIVE COMPLETE";
        sense = "TOUCH REGAINED";
        nextFloor = "FINAL: THE SOUL";
    } else if (completedFloor == 6) {
        headline = "SOUL FLOOR COMPLETE";
        sense = "SOUL AWAKENED";
        nextFloor = "FINAL ASCENT";
    }

    drawStringShadow(40, 100, headline, 6, 13, 1);
    drawString5x7(48, 116, sense, 8, 1);
    drawStringShadow(36, 130, nextFloor, 9, 13, 1);

    drawString5x7(56, 150, "START / A: CONTINUE", 6, 1);

    drawRect4(0, 0, SCREENWIDTH, 2, 7);
    drawRect4(0, SCREENHEIGHT - 2, SCREENWIDTH, 2, 7);
}
