#include "gba.h"
#include "menu_ui.h"
#include "mode0.h"
#include "mode4.h"
#include "sprites.h"
#include "analogSound.h"

typedef enum {
    STATE_SPLASH = 0,
    STATE_INSTRUCTIONS,
    STATE_GAME,
    STATE_SKILL_MENU,
    STATE_PAUSE,
    STATE_WIN,
    STATE_LOSE,
    STATE_FLOOR_COMPLETE
} AppState;

static AppState state = STATE_SPLASH;
static int skillMenuNeedsRedraw = 0;
static int relaxModeEnabled = 0;

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
    gameAudioStop();
    setupMenuPalette();
    enterMode4Screen(1);
    uiDrawSplash();
    state = STATE_SPLASH;
}

static void goInstructions(void) {
    gameAudioStop();
    setupMenuPalette();
    enterMode4Screen(2);
    uiDrawInstructions();
    state = STATE_INSTRUCTIONS;
}

static void goPause(void) {
    gameAudioPause();
    setupMenuPalette();
    enterMode4Screen(3);
    uiDrawPause();
    state = STATE_PAUSE;
}

static void goWin(void) {
    gameAudioStop();
    setupMenuPalette();
    enterMode4Screen(4);
    uiDrawWin();
    playAnalogSound(16);
    state = STATE_WIN;
}

static void goLose(void) {
    gameAudioStop();
    setupMenuPalette();
    enterMode4Screen(5);
    uiDrawLose();
    playAnalogSound(17);
    state = STATE_LOSE;
}

static void goFloorComplete(void) {
    gameAudioStop();
    setupMenuPalette();
    enterMode4Screen(3);
    uiDrawFloorComplete(gameGetFloor());
    playAnalogSound(11);
    state = STATE_FLOOR_COMPLETE;
}

int main(void) {
    REG_SOUNDCNT_X = SND_ENABLED;
    REG_SOUNDBIAS = 0x0200;
    goSplash();

    while (1) {
        waitForVBlank();

        buttons = ~(REG_BUTTONS)&0x03FF;

        switch (state) {
            case STATE_SPLASH:
                if (BUTTON_PRESSED(BUTTON_B) && BUTTON_HELD(BUTTON_SELECT)) {
                    relaxModeEnabled ^= 1;
                }
                if (BUTTON_PRESSED(BUTTON_START)) {
                    gameSetRelaxMode(relaxModeEnabled);
                    gameReset();
                    gameRenderInit();
                    gameAudioStart();
                    state = STATE_GAME;
                } else if (BUTTON_PRESSED(BUTTON_SELECT)) {
                    goInstructions();
                }
                break;

            case STATE_INSTRUCTIONS:
                if (BUTTON_PRESSED(BUTTON_B)) {
                    goSplash();
                } else if (BUTTON_PRESSED(BUTTON_START) || BUTTON_PRESSED(BUTTON_A)) {
                    gameSetRelaxMode(relaxModeEnabled);
                    gameReset();
                    gameRenderInit();
                    gameAudioStart();
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
                    if (gameSkillMenuTakeOpenRequest()) {
                        gameAudioPause();
                        gameSkillMenuOpenSnapshot();
                        setupMenuPalette();
                        enterMode4Screen(3);
                        {
                            int es;
                            int ec;
                            int em;
                            int ep;
                            int ex;
                            gameSkillMenuGetEdit(&es, &ec, &em, &ep, &ex);
                            uiDrawSkillMenu(gameSkillMenuGetSel(), es, ec, em, ep, ex);
                        }
                        skillMenuNeedsRedraw = 0;
                        state = STATE_SKILL_MENU;
                        break;
                    }
                    if (g == 1) {
                        goWin();
                    } else if (g == 2) {
                        goLose();
                    } else if (g == 3) {
                        goFloorComplete();
                    }
                }
                break;

            case STATE_SKILL_MENU:
                if (BUTTON_PRESSED(BUTTON_UP)) {
                    gameSkillMenuNav(-1);
                    skillMenuNeedsRedraw = 1;
                    playAnalogSound(7);
                } else if (BUTTON_PRESSED(BUTTON_DOWN)) {
                    gameSkillMenuNav(1);
                    skillMenuNeedsRedraw = 1;
                    playAnalogSound(7);
                } else if (BUTTON_PRESSED(BUTTON_A)) {
                    if (gameSkillMenuGetSel() == 4) {
                        gameSkillMenuSave();
                        gameRenderInit();
                        gameAudioResume();
                        state = STATE_GAME;
                        break;
                    }
                    gameSkillMenuPressA();
                    skillMenuNeedsRedraw = 1;
                } else if (BUTTON_PRESSED(BUTTON_SELECT)) {
                    gameSkillMenuSave();
                    gameRenderInit();
                    gameAudioResume();
                    state = STATE_GAME;
                    break;
                } else if (BUTTON_PRESSED(BUTTON_B)) {
                    gameSkillMenuDiscard();
                    gameRenderInit();
                    gameAudioResume();
                    state = STATE_GAME;
                    break;
                }
                if (skillMenuNeedsRedraw) {
                    setupMenuPalette();
                    fillScreen4(3);
                    {
                        int es;
                        int ec;
                        int em;
                        int ep;
                        int ex;
                        gameSkillMenuGetEdit(&es, &ec, &em, &ep, &ex);
                        uiDrawSkillMenu(gameSkillMenuGetSel(), es, ec, em, ep, ex);
                    }
                    skillMenuNeedsRedraw = 0;
                }
                break;

            case STATE_PAUSE:
                if (BUTTON_PRESSED(BUTTON_START)) {
                    gameRenderInit();
                    gameAudioResume();
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

            case STATE_FLOOR_COMPLETE:
                if (BUTTON_PRESSED(BUTTON_START) || BUTTON_PRESSED(BUTTON_A)) {
                    int nextFloor = gameGetFloor() + 1;
                    if (nextFloor > 6) {
                        goWin();
                    } else {
                        gameSetFloor(nextFloor);
                        gameRenderInit();
                        gameAudioStart();
                        state = STATE_GAME;
                    }
                }
                break;
        }

        oldButtons = buttons;
    }
}
