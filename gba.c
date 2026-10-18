#include "gba.h"

unsigned volatile short *videoBuffer = (unsigned short *)0x6000000;

unsigned short oldButtons = 0;
unsigned short buttons = 0;

int collision(int x1, int y1, int width1, int height1, int x2, int y2, int width2, int height2) {
    return y1 < y2 + height2 && y1 + height1 > y2 && x1 < x2 + width2 && x1 + width1 > x2;
}

void waitForVBlank() {
    while (REG_VCOUNT >= 160);
    while (REG_VCOUNT < 160);
}

void DMANow(int channel, volatile void *src, volatile void *dest, unsigned int ctrl) {
    unsigned int full = ctrl | DMA_ON;
    if ((channel == 1 || channel == 2) && (ctrl & DMA_AT_REFRESH)) {
        full |= 4;
    }
    DMA[channel].ctrl = 0;
    DMA[channel].src = src;
    DMA[channel].dest = dest;
    DMA[channel].ctrl = full;
}