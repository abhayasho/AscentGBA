#include "gba.h"
#include "digitalSound.h"

SOUND soundA;
SOUND soundB;

void setupSounds() {

    REG_SOUNDCNT_X = SND_ENABLED;
    REG_SOUNDBIAS = 0x0200;

	REG_SOUNDCNT_H = SND_OUTPUT_RATIO_100 |
                     DSA_OUTPUT_RATIO_50 |
                     DSA_TIMER0 |
                     DSA_FIFO_RESET |
                     DSA_OUTPUT_TO_BOTH |
                     DSB_OUTPUT_RATIO_50 |
                     DSB_TIMER1 |
                     DSB_FIFO_RESET |
                     DSB_OUTPUT_TO_BOTH;

	REG_SOUNDCNT_L = 0;

}

void playSoundA(const signed char* data, int dataLength, int looping) {
    DMANow(1, data, REG_FIFO_A, DMA_DESTINATION_FIXED | DMA_AT_REFRESH | DMA_REPEAT | DMA_32);

    REG_TM0CNT = 0;
    int cyclesPerSecond = PROCESSOR_CYCLES_PER_SECOND / SOUND_FREQ;
    REG_TM0D = 65536 - cyclesPerSecond;
    REG_TM0CNT = TIMER_ON;

    soundA.data = data;
    soundA.dataLength = dataLength;
    soundA.looping = looping;
    soundA.isPlaying = 1;
    soundA.durationInVBlanks = (VBLANK_FREQ * dataLength) / SOUND_FREQ;
    soundA.vBlankCount = 0;

}

void clearSoundBState(void) {
    soundB.data = 0;
    soundB.isPlaying = 0;
    soundB.looping = 0;
    REG_TM1CNT = TIMER_OFF;
    DMA[2].ctrl = 0;
}

void playSoundB(const signed char* data, int dataLength, int looping) {
    DMA[2].ctrl = 0;
    DMANow(2, data, REG_FIFO_B, DMA_DESTINATION_FIXED | DMA_AT_REFRESH | DMA_REPEAT | DMA_32);

    REG_TM1CNT = 0;
    u32 cyc = (u32)PROCESSOR_CYCLES_PER_SECOND * (u32)SOUND_CHORUS_B_DEN;
    u32 div = (u32)SOUND_FREQ * (u32)SOUND_CHORUS_B_NUM;
    int cyclesPerSecond = (int)(cyc / div);
    REG_TM1D = (unsigned short)(65536 - cyclesPerSecond);
    REG_TM1CNT = TIMER_ON;

    soundB.data = data;
    soundB.dataLength = dataLength;
    soundB.looping = looping;
    soundB.isPlaying = 1;
    soundB.durationInVBlanks = (VBLANK_FREQ * dataLength) / SOUND_FREQ;
    soundB.vBlankCount = 0;

}

void pauseSounds() {
    soundA.isPlaying = 0;
    REG_TM0CNT = TIMER_OFF;
    soundB.isPlaying = 0;
    REG_TM1CNT = TIMER_OFF;

}

void unpauseSounds() {
    soundA.isPlaying = 1;
    soundB.isPlaying = 1;
    if (soundA.data) {
        REG_TM0CNT = TIMER_OFF;
        DMA[1].ctrl = 0;
        DMANow(1, soundA.data, REG_FIFO_A,
               DMA_DESTINATION_FIXED | DMA_AT_REFRESH | DMA_REPEAT | DMA_32);
        REG_TM0D = (unsigned short)(65536 - (PROCESSOR_CYCLES_PER_SECOND / SOUND_FREQ));
        REG_TM0CNT = TIMER_ON;
    }
    if (soundB.data) {
        REG_TM1CNT = TIMER_OFF;
        DMA[2].ctrl = 0;
        DMANow(2, soundB.data, REG_FIFO_B,
               DMA_DESTINATION_FIXED | DMA_AT_REFRESH | DMA_REPEAT | DMA_32);
        {
            u32 cyc2 = (u32)PROCESSOR_CYCLES_PER_SECOND * (u32)SOUND_CHORUS_B_DEN;
            u32 div2 = (u32)SOUND_FREQ * (u32)SOUND_CHORUS_B_NUM;
            REG_TM1D = (unsigned short)(65536 - (int)(cyc2 / div2));
        }
        REG_TM1CNT = TIMER_ON;
    }
}

void stopSounds() {
    soundA.isPlaying = 0;
    REG_TM0CNT = TIMER_OFF;
    DMA[1].ctrl = 0;
    soundB.isPlaying = 0;
    REG_TM1CNT = TIMER_OFF;
    DMA[2].ctrl = 0;

}