#include "gba.h"
#include "menu_ui.h"
#include "mode0.h"
#include "mode4.h"
#include "sprites.h"

typedef enum {
    STATE_SPLASH = 0,
    STATE_INSTRUCTIONS,
    STATE_GAME,
    STATE_PAUSE,
    STATE_WIN,
    STATE_LOSE
} AppState;

static AppState state = STATE_SPLASH;

static void setupMenuPalette(void) {
    BG_PALETTE[0] = RGB(0, 0, 0);
    BG_PALETTE[1] = RGB(4, 8, 18);
    BG_PALETTE[2] = RGB(6, 14, 8);
    BG_PALETTE[3] = RGB(12, 12, 14);
    BG_PALETTE[4] = RGB(22, 18, 4);
    BG_PALETTE[5] = RGB(18, 4, 4);
    BG_PALETTE[6] = RGB(31, 31, 31);
    BG_PALETTE[7] = RGB(18, 20, 24);
}

static void enterMode4Screen(unsigned char paletteIndex) {
    REG_DISPCTL = MODE(4) | BG_ENABLE(2);
    videoBuffer = FRONTBUFFER;
    hideSprites();
    updateOAM();
    fillScreen4(paletteIndex);
}

static void goSplash(void) {
    setupMenuPalette();
    enterMode4Screen(1);
    uiDrawSplash();
    state = STATE_SPLASH;
}

static void goInstructions(void) {
    setupMenuPalette();
    enterMode4Screen(2);
    uiDrawInstructions();
    state = STATE_INSTRUCTIONS;
}

static void goPause(void) {
    setupMenuPalette();
    enterMode4Screen(3);
    uiDrawPause();
    state = STATE_PAUSE;
}

static void goWin(void) {
    setupMenuPalette();
    enterMode4Screen(4);
    uiDrawWin();
    state = STATE_WIN;
}

static void goLose(void) {
    setupMenuPalette();
    enterMode4Screen(5);
    uiDrawLose();
    state = STATE_LOSE;
}

int main(void) {
    goSplash();

    while (1) {
        waitForVBlank();

        buttons = ~(REG_BUTTONS)&0x03FF;

        switch (state) {
            case STATE_SPLASH:
                if (BUTTON_PRESSED(BUTTON_START)) {
                    gameReset();
                    gameRenderInit();
                    state = STATE_GAME;
                } else if (BUTTON_PRESSED(BUTTON_SELECT)) {
                    goInstructions();
                }
                break;

            case STATE_INSTRUCTIONS:
                if (BUTTON_PRESSED(BUTTON_B)) {
                    goSplash();
                } else if (BUTTON_PRESSED(BUTTON_START) || BUTTON_PRESSED(BUTTON_A)) {
                    gameReset();
                    gameRenderInit();
                    state = STATE_GAME;
                }
                break;

            case STATE_GAME:
                if (BUTTON_PRESSED(BUTTON_START)) {
                    goPause();
                    break;
                }
                {
                    int g = gameUpdate();
                    if (g == 1) {
                        goWin();
                    } else if (g == 2) {
                        goLose();
                    }
                }
                break;

            case STATE_PAUSE:
                if (BUTTON_PRESSED(BUTTON_START)) {
                    gameRenderInit();
                    state = STATE_GAME;
                } else if (BUTTON_PRESSED(BUTTON_SELECT)) {
                    gameReset();
                    goSplash();
                }
                break;

            case STATE_WIN:
                if (BUTTON_PRESSED(BUTTON_START) || BUTTON_PRESSED(BUTTON_A)) {
                    gameReset();
                    goSplash();
                }
                break;

            case STATE_LOSE:
                if (BUTTON_PRESSED(BUTTON_START) || BUTTON_PRESSED(BUTTON_A)) {
                    gameReset();
                    goSplash();
                }
                break;
        }

        oldButtons = buttons;
    }
}
