#include "sprites.h"

OBJ_ATTR shadowOAM[128];

// Hides all sprites in the shadowOAM; Must DMA the shadowOAM into the OAM after calling this function
void hideSprites() {
    for (int i = 0; i < 128; i++) {
        shadowOAM[i].attr0 = ATTR0_HIDE;
    }
}

void updateOAM() {
    volatile u16 *dst = (volatile u16 *)OAM;
    u16 *src = (u16 *)shadowOAM;
    for (int i = 0; i < 512; i++) {
        dst[i] = src[i];
    }
}