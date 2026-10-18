#include "mode0.h"
#include "sprites.h"
#include "gba.h"
#include "analogSound.h"
#include "player_frames.h"
#include "enemy_frames.h"
#include "boss_frames.h"
#include "Assets/Pixel Heart Sprite Sheet 16x16.h"

void setupSounds(void);
void playSoundA(const signed char *data, int dataLength, int looping);
void clearSoundBState(void);
void pauseSounds(void);
void unpauseSounds(void);
void stopSounds(void);

#define SCREENBLOCK_MAP 28
#define SCREENBLOCK_PARALLAX 29
#define CHARBLOCK_TILES 0
#define CHARBLOCK_PARALLAX 1

#define TILE_FLOOR  0
#define TILE_WALL   1
#define TILE_FLOOR2 2
#define TILE_SKILL  3
#define TILE_FLOOR_CRACK 4
#define TILE_FLOOR_SKULL 5
#define TILE_FLOOR_MOSS  6
#define TILE_WALL_CRACK  7
#define TILE_WALL_VINE   8
#define TILE_TOTEM_TOP   9
#define TILE_TOTEM_BOT   10
#define TILE_WATER       11

#define OAM_PLAYER 0
#define OAM_ENEMY_BASE 1
#define MAX_ENEMY_SLOTS 20
#define OAM_PROJ_BASE (OAM_ENEMY_BASE + MAX_ENEMY_SLOTS)
#define MAX_PROJ      16
#define OAM_SLASH     (OAM_PROJ_BASE + MAX_PROJ)
#define OAM_HP0       (OAM_SLASH + 1)
#define OAM_BOSS_BAR0 (OAM_HP0 + 8)
#define BOSS_BAR_SEGS 16
#define OAM_XP_BAR0   (OAM_BOSS_BAR0 + BOSS_BAR_SEGS)
#define XP_BAR_SEGS   10
#define OAM_SP0       (OAM_XP_BAR0 + XP_BAR_SEGS)
#define SP_SEGS       8
#define OAM_XP_ORB0   (OAM_SP0 + SP_SEGS)

#define MAX_ENEMIES MAX_ENEMY_SLOTS
#define MAX_XP_ORBS 16
#define OAM_STAT_STR0 (OAM_XP_ORB0 + MAX_XP_ORBS)
#define STAT_SEGS     3
#define OAM_LEVEL_TENS (OAM_STAT_STR0 + 3 * STAT_SEGS)
#define OAM_LEVEL_ONES (OAM_LEVEL_TENS + 1)
#define BUBBLE_COUNT   8
#define OAM_BUBBLE0    (OAM_LEVEL_ONES + 1)
#define PROJ_SZ     8
#define XP_ORB_SZ   8
#define XP_PER_SKILL 100
#define DIGIT_TILE_BASE 40
#define PLAYER_DOWN0_TILE      64
#define PLAYER_DOWN1_TILE      72
#define PLAYER_UP0_TILE        80
#define PLAYER_UP1_TILE        88
#define PLAYER_RIGHT0_TILE     96
#define PLAYER_RIGHT1_TILE     104
#define PLAYER_LEFT0_TILE      112
#define PLAYER_LEFT1_TILE      120
#define PLAYER_SWORD_R0_TILE    128
#define PLAYER_SWORD_R1_TILE    132
#define PLAYER_SWORD_R2_TILE    136
#define PLAYER_SWORD_L0_TILE    140
#define PLAYER_SWORD_L1_TILE    144
#define PLAYER_SWORD_L2_TILE    148
#define PLAYER_SWORD_U0_TILE    152
#define PLAYER_SWORD_U1_TILE    156
#define PLAYER_SWORD_U2_TILE    160
#define PLAYER_SWORD_D0_TILE    164
#define PLAYER_SWORD_D1_TILE    168
#define PLAYER_SWORD_D2_TILE    172
#define HEART_FULL_TILE         176
#define HEART_EMPTY_TILE        180
#define BOSS_FRAME_TILE         192
#define SLIME_FRAME0_TILE        24
#define SLIME_FRAME1_TILE        28
#define PLANT_OPEN_TILE          32
#define PLANT_CLOSED_TILE        36
#define XP_ORB0_TILE             50
#define XP_ORB1_TILE             51
#define XP_ORB2_TILE             52
#define PROJ_SEED0_TILE          53
#define PROJ_SEED1_TILE          54
#define PROJ_RED0_TILE           55
#define PROJ_RED1_TILE           56
#define PROJ_BOSS0_TILE          57
#define PROJ_BOSS1_TILE          58
#define PROJ_PLAYER0_TILE        59
#define PROJ_PLAYER1_TILE        60

#define PLAYER_SIZE 16
#define PLAYER_BASE_MAX_HP 5

#define ET_PATROL  0
#define ET_SHOOT   1
#define ET_BRUISER 2
#define ET_BOSS    3

#define MAX_ROOMS 8
#define BOSS_MAX_HP      20
#define GAZE_IFRAMES_HIT 30
#define TILE_GAZE_OPEN   12
#define TILE_GAZE_CLOSED 13

#define ENEMY_PATROL_MOVE_PERIOD 10
#define ENEMY_BRUISER_MOVE_PERIOD 8
#define PROJ_MOVE_PERIOD          5

#define SPAWN_FROM_EAST_X (5 * 8)
#define SPAWN_FROM_WEST_X (25 * 8)
#define ROOM_TRANSITION_COOLDOWN 48
#define SPAWN_FROM_NORTH_Y (5 * 8)
#define SPAWN_FROM_SOUTH_Y (25 * 8)

static const signed char ROOM_EAST[5][MAX_ROOMS] = {
    {1, -1, 3, -1, 5, -1, 7, -1},
    {1, 2, 3, 4, 5, 6, 7, -1},
    {1, -1, 3, -1, 5, -1, 7, -1},
    {1, 2, 3, 4, 5, -1, 7, -1},
    {1, -1, 3, 4, -1, 6, 7, -1},
};
static const signed char ROOM_WEST[5][MAX_ROOMS] = {
    {-1, 0, -1, 2, -1, 4, -1, 6},
    {-1, 0, 1, 2, 3, 4, 5, 6},
    {-1, 0, -1, 2, -1, 4, -1, 6},
    {-1, 0, 1, 2, 3, 4, -1, 6},
    {-1, 0, -1, 2, 3, -1, 5, 6},
};
static const signed char ROOM_NORTH[5][MAX_ROOMS] = {
    {-1, 2, -1, -1, -1, 6, -1, -1},
    {-1, -1, -1, 1, -1, 3, -1, 5},
    {-1, 2, -1, 4, -1, 6, -1, -1},
    {-1, 2, -1, -1, -1, 6, -1, -1},
    {-1, 2, -1, -1, 5, -1, -1, -1},
};
static const signed char ROOM_SOUTH[5][MAX_ROOMS] = {
    {-1, -1, 1, 4, 3, -1, 5, -1},
    {-1, 3, -1, 5, -1, 7, -1, -1},
    {-1, -1, 1, -1, 3, -1, 5, -1},
    {-1, -1, 1, -1, -1, -1, 5, -1},
    {-1, -1, 1, -1, 3, 4, -1, -1},
};

typedef struct {
    int x, y;
    int vx, vy;
    int hp;
    int type;
    int roomId;
    int active;
    int shootCd;
    int swingRemain;
    int attackCd;
    int swingDealt;
    int hurtTimer;
    int knockX;
    int knockY;
} Enemy;

typedef struct {
    int x, y;
    int vx, vy;
    int active;
    int ttl;
    int kind;
    int owner;
} Projectile;

typedef struct {
    int x, y;
    int roomId;
    int amt;
    int active;
    int bob;
} XpOrb;

static int cameraX;
static int cameraY;

static const u16 SOLID_TILE_COLOR1[16] = {
    0x3223, 0x3223, 0x2112, 0x2112, 0x3212, 0x2123, 0x3323, 0x3233,
    0x3223, 0x3223, 0x2112, 0x2112, 0x3212, 0x2123, 0x1111, 0x1111,
};
static const u16 SOLID_TILE_COLOR2[16] = {
    0x1551, 0x1551, 0x2333, 0x3332, 0x3323, 0x3233, 0x2332, 0x3323,
    0x3233, 0x2333, 0x3323, 0x3332, 0x3332, 0x3323, 0x1551, 0x1551,
};
static const u16 SOLID_TILE_COLOR3[16] = {
    0x3333, 0x3333, 0x3333, 0x3333, 0x3333, 0x3333, 0x3333, 0x3333,
    0x3333, 0x3333, 0x3333, 0x3333, 0x3333, 0x3333, 0x3333, 0x3333,
};
static const u16 SOLID_TILE_FLOOR_ALT[16] = {
    0x9989, 0x9898, 0x9B98, 0x8998, 0x9989, 0x98B9, 0x8899, 0x9999,
    0xB998, 0x9899, 0x98B9, 0x8989, 0x9989, 0x989B, 0x8998, 0x9989,
};
static const u16 TOTEM_TOP_TILE[16] = {
    0x2100, 0x0012, 0x2210, 0x0122, 0x2221, 0x1222, 0x2321, 0x1232,
    0x2421, 0x1242, 0x1221, 0x1221, 0x2221, 0x1222, 0x2121, 0x1212,
};
static const u16 TOTEM_MID_TILE[16] = {
    0x2121, 0x1212, 0x2221, 0x1222, 0x3321, 0x1233, 0x3221, 0x1223,
    0x3221, 0x1223, 0x4321, 0x1234, 0x2221, 0x1222, 0x2121, 0x1212,
};
static const u16 TOTEM_BOT_TILE[16] = {
    0x2221, 0x1222, 0x2121, 0x1212, 0x1221, 0x1221, 0x1211, 0x1121,
    0x2115, 0x5112, 0x1155, 0x5511, 0x5550, 0x0555, 0x5500, 0x0055,
};
static const u16 DUNGEON_FLOOR_CRACK_TILE[16] = {
    0x3223, 0x3223, 0x2112, 0x2112, 0x4212, 0x2123, 0x3423, 0x3234,
    0x4223, 0x3224, 0x2112, 0x2112, 0x3212, 0x2123, 0x1111, 0x1111,
};
static const u16 DUNGEON_FLOOR_SKULL_TILE[16] = {
    0x7023, 0x3207, 0x7772, 0x2777, 0x7172, 0x2717, 0x1773, 0x3771,
    0x7723, 0x3277, 0x2212, 0x2122, 0x3212, 0x2123, 0x1111, 0x1111,
};
static const u16 DUNGEON_FLOOR_MOSS_TILE[16] = {
    0x6623, 0x3266, 0x6662, 0x2666, 0x3666, 0x6663, 0x3363, 0x3633,
    0x3223, 0x3223, 0x2112, 0x2112, 0x3212, 0x2123, 0x1111, 0x1111,
};
static const u16 DUNGEON_WALL_CRACK_TILE[16] = {
    0x1551, 0x1551, 0x2333, 0x3334, 0x3323, 0x3234, 0x2332, 0x3324,
    0x3233, 0x2334, 0x3323, 0x3334, 0x3332, 0x3324, 0x1551, 0x1554,
};
static const u16 DUNGEON_WALL_VINE_TILE[16] = {
    0x1551, 0x1551, 0x2363, 0x3632, 0x3363, 0x3633, 0x2362, 0x3623,
    0x3263, 0x2633, 0x3363, 0x3632, 0x3362, 0x3623, 0x1551, 0x1551,
};
static const u16 GAZE_TILE_OPEN[16] = {
    0x1001, 0x2112, 0x1110, 0x1110, 0x2112, 0x1001, 0x0000, 0x0000,
    0x2112, 0x1220, 0x2220, 0x2220, 0x1220, 0x2112, 0x0000, 0x0000,
};
static const u16 GAZE_TILE_CLOSED[16] = {
    0x1001, 0x2112, 0x1110, 0x1110, 0x2112, 0x1001, 0x0000, 0x0000,
    0x0000, 0x3330, 0x3330, 0x3330, 0x3330, 0x0000, 0x0000, 0x0000,
};
static const u16 GAZE_TILE_RING_0[16] = {
    0x0110, 0x1221, 0x2332, 0x2332, 0x1221, 0x0110, 0x0000, 0x0000,
    0x0110, 0x1221, 0x2332, 0x2332, 0x1221, 0x0110, 0x0000, 0x0000,
};
static const u16 GAZE_TILE_RING_1[16] = {
    0x0011, 0x0122, 0x1233, 0x1233, 0x0122, 0x0011, 0x0000, 0x0000,
    0x0011, 0x0122, 0x1233, 0x1233, 0x0122, 0x0011, 0x0000, 0x0000,
};
static const u16 SOLID_TILE_PINK_BG[16] = {
    0x1111, 0x1111, 0x1111, 0x1111, 0x1111, 0x1111, 0x1111, 0x1111,
    0x1111, 0x1111, 0x1111, 0x1111, 0x1111, 0x1111, 0x1111, 0x1111,
};
static const u16 HUD_BAR_SEG[16] = {
    0x0000, 0x0000, 0x0202, 0x0202, 0x0222, 0x2220, 0x0222, 0x2220,
    0x0122, 0x2210, 0x0111, 0x1110, 0x0111, 0x1110, 0x0122, 0x2210,
};

static const u16 DIGIT_TILES[10][16] = {
    {0x0110, 0x1001, 0x1001, 0x1001, 0x1001, 0x1001, 0x0110, 0x0000,
     0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000},
    {0x0010, 0x0110, 0x0010, 0x0010, 0x0010, 0x0010, 0x0111, 0x0000,
     0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000},
    {0x0110, 0x1001, 0x0001, 0x0010, 0x0100, 0x1000, 0x1111, 0x0000,
     0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000},
    {0x1110, 0x0001, 0x0001, 0x0110, 0x0001, 0x0001, 0x1110, 0x0000,
     0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000},
    {0x0001, 0x0011, 0x0101, 0x1001, 0x1111, 0x0001, 0x0001, 0x0000,
     0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000},
    {0x1111, 0x1000, 0x1000, 0x1110, 0x0001, 0x0001, 0x1110, 0x0000,
     0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000},
    {0x0111, 0x1000, 0x1000, 0x1110, 0x1001, 0x1001, 0x0110, 0x0000,
     0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000},
    {0x1111, 0x0001, 0x0010, 0x0100, 0x0100, 0x0100, 0x0100, 0x0000,
     0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000},
    {0x0110, 0x1001, 0x1001, 0x0110, 0x1001, 0x1001, 0x0110, 0x0000,
     0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000},
    {0x0110, 0x1001, 0x1001, 0x0111, 0x0001, 0x0001, 0x1110, 0x0000,
     0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000},
};

static u16 ramMap[1024];
static u16 parallaxMap[1024];

static int roomId;
static int currentFloor;
static int relaxMode;

static int playerX;
static int playerY;
static int playerDir;
static int playerHp;
static int playerMaxHp;
static int invincibleTimer;

static int xpStr;
static int xpCon;
static int xpMob;
static int skillPoints;
static int playerXp;

static Enemy enemies[MAX_ENEMIES];
static Projectile projs[MAX_PROJ];
static XpOrb xpOrbs[MAX_XP_ORBS];

static int attackTimer;
static int attackDealtDamage;
static int attackFacing;

static int enemiesAlive;

static unsigned int gameTick;

typedef struct {
    int active;
    int x;
    int y;
    int vy;
    int ttl;
} Bubble;
static Bubble bubbles[BUBBLE_COUNT];
static unsigned int bubbleRng;
static int roomTransitionCooldown;
static int healPoolTimer;

typedef enum {
    GAZE_MOVE = 0,
    GAZE_TELEGRAPH_BEAM,
    GAZE_BEAM_ACTIVE,
    GAZE_BEAM_RECOVER,
    GAZE_TELEGRAPH_SWOOP,
    GAZE_SWOOP_ACTIVE,
    GAZE_SWOOP_RECOVER,
    GAZE_VULNERABLE,
    GAZE_STUNNED,
} GazeState;

static GazeState gazeSt;
static int gazeTimer;
static int gazeLastAttack;
static int gazeEyeOpen;
static int gazeIframes;
static int gazeBeamDx;
static int gazeBeamDy;
static int gazeSwoopTx;
static int gazeSwoopTy;
static int gazeMoveWp;
static int gazeAttackHitLatch;

static int skillMenuOpenRequest;
static int snapStr;
static int snapCon;
static int snapMob;
static int snapPts;
static int snapXp;
static int editStr;
static int editCon;
static int editMob;
static int editPts;
static int skillMenuSel;

static int bossWestDoorLocked;
static int bossShotsSinceParry;
static int bossPatternTimer;
static int bossSpinAngle;
static int bossDefeated;
static int playerAnimFrame;
static int patrolAnimFrame;
static int audioRunning;

#define REG_SOUNDCNT_L_LOCAL (*(volatile u16 *)0x04000080)
#define REG_SOUNDCNT_H_LOCAL (*(volatile u16 *)0x04000082)
#define REG_SOUNDCNT_X_LOCAL (*(volatile u16 *)0x04000084)
#define REG_FIFO_A_LOCAL ((volatile u16 *)0x040000A0)
#define REG_FIFO_B_LOCAL ((volatile u16 *)0x040000A4)

#define AUDIO_LOOP_SAMPLES 256

static const signed char LOOP_BED[AUDIO_LOOP_SAMPLES] __attribute__((aligned(4))) = {
    0, 0, 1, 1, 1, 2, 2, 2,
    3, 3, 3, 4, 4, 4, 5, 5,
    5, 6, 6, 6, 7, 7, 7, 7,
    8, 8, 8, 9, 9, 9, 9, 10,
    10, 10, 10, 11, 11, 11, 11, 11,
    12, 12, 12, 12, 12, 13, 13, 13,
    13, 13, 13, 13, 13, 13, 14, 14,
    14, 14, 14, 14, 14, 14, 14, 14,
    14, 14, 14, 14, 14, 14, 14, 14,
    14, 14, 14, 13, 13, 13, 13, 13,
    13, 13, 13, 13, 12, 12, 12, 12,
    12, 11, 11, 11, 11, 11, 10, 10,
    10, 10, 9, 9, 9, 9, 8, 8,
    8, 7, 7, 7, 7, 6, 6, 6,
    5, 5, 5, 4, 4, 4, 3, 3,
    3, 2, 2, 2, 1, 1, 1, 0,
    0, 0, -1, -1, -1, -2, -2, -2,
    -3, -3, -3, -4, -4, -4, -5, -5,
    -5, -6, -6, -6, -7, -7, -7, -7,
    -8, -8, -8, -9, -9, -9, -9, -10,
    -10, -10, -10, -11, -11, -11, -11, -11,
    -12, -12, -12, -12, -12, -13, -13, -13,
    -13, -13, -13, -13, -13, -13, -14, -14,
    -14, -14, -14, -14, -14, -14, -14, -14,
    -14, -14, -14, -14, -14, -14, -14, -14,
    -14, -14, -14, -13, -13, -13, -13, -13,
    -13, -13, -13, -13, -12, -12, -12, -12,
    -12, -11, -11, -11, -11, -11, -10, -10,
    -10, -10, -9, -9, -9, -9, -8, -8,
    -8, -7, -7, -7, -7, -6, -6, -6,
    -5, -5, -5, -4, -4, -4, -3, -3,
    -3, -2, -2, -2, -1, -1, -1, 0,
};

static signed char dmaLoopBed[AUDIO_LOOP_SAMPLES] __attribute__((aligned(4)));

static void drawSprites(void);
static void spawnXpOrb(int x, int y, int room, int amt);
static void dropXpOrbs(int x, int y, int room, int enemyType);
static void updateXpOrbs(void);
static int inHealPool(void);
static int inSkillStationWorld(int *sx, int *sy, int *sw, int *sh);
static void updateGazeBoss(Enemy *e);
static void drawHudBars(void);
static void damagePlayerOnce(void);
static void unlockBossRoomWestDoor(void);
static void applyHitFeedback(Enemy *e, int fromX, int fromY);
static int getSwordHitbox(int *hx, int *hy, int *hw, int *hh);
static void spawnProjectileKind(int x, int y, int vx, int vy, int kind, int owner);
static void updateBossParryFight(Enemy *e);
static Enemy *findBoss(void);
static void onBossDefeated(void);
static void clearProjectiles(void);
static int roomEastLink(int id);
static int roomWestLink(int id);
static int roomNorthLink(int id);
static int roomSouthLink(int id);
static void buildParallaxMap(void);
static void mutateRuntimeAssets(void);
static void setupGameplaySound(void);
static void syncBgLayers(void);

static int moveSpeed(void) {
    int s = 2;
    if (xpMob >= 2) {
        s++;
    }
    if (xpMob >= 5) {
        s++;
    }
    if (s > 4) {
        s = 4;
    }
    return s;
}

static int floorIndex(void) {
    int f = currentFloor - 1;
    if (f < 0) {
        f = 0;
    }
    if (f > 4) {
        f = 4;
    }
    return f;
}

static int roomEastLink(int id) {
    return ROOM_EAST[floorIndex()][id];
}
static int roomWestLink(int id) {
    return ROOM_WEST[floorIndex()][id];
}
static int roomNorthLink(int id) {
    return ROOM_NORTH[floorIndex()][id];
}
static int roomSouthLink(int id) {
    return ROOM_SOUTH[floorIndex()][id];
}

static void setupGameplaySound(void) {
    int i;
    union {
        u32 w;
        signed char b[4];
    } u0;
    union {
        u32 w;
        signed char b[4];
    } u1;

    setupSounds();
    REG_SOUNDBIAS = 0x0200;
    REG_SOUNDCNT_X_LOCAL = SND_ENABLED;

    for (i = 0; i < AUDIO_LOOP_SAMPLES; i++) {
        dmaLoopBed[i] = LOOP_BED[i];
    }

    clearSoundBState();

    REG_SOUNDCNT_L_LOCAL = DMG_VOL_LEFT(5) | DMG_VOL_RIGHT(5) |
                           DMG_SND1_LEFT | DMG_SND1_RIGHT |
                           DMG_SND2_LEFT | DMG_SND2_RIGHT |
                           DMG_SND4_LEFT | DMG_SND4_RIGHT;

    for (i = 0; i < 4; i++) {
        u0.b[i] = dmaLoopBed[i];
        u1.b[i] = dmaLoopBed[i + 4];
    }
    *(volatile u32 *)0x040000A0 = u0.w;
    *(volatile u32 *)0x040000A0 = u1.w;
    *(volatile u32 *)0x040000A4 = 0;
    *(volatile u32 *)0x040000A4 = 0;

    playSoundA(dmaLoopBed, (int)sizeof(dmaLoopBed), 1);
}

static int swordDamage(void) {
    int d = 1 + xpStr;
    if (relaxMode) {
        d += 2;
    }
    return d;
}

static void refreshMaxHpFromCon(void) {
    int m = PLAYER_BASE_MAX_HP + xpCon;
    if (m > 8) {
        m = 8;
    }
    if (m < 3) {
        m = 3;
    }
    playerMaxHp = m;
    if (playerHp > playerMaxHp) {
        playerHp = playerMaxHp;
    }
}

int gameSkillMenuTakeOpenRequest(void) {
    int r = skillMenuOpenRequest;
    skillMenuOpenRequest = 0;
    return r;
}

void gameSkillMenuOpenSnapshot(void) {
    snapStr = xpStr;
    snapCon = xpCon;
    snapMob = xpMob;
    snapPts = skillPoints;
    snapXp = playerXp;
    editStr = snapStr;
    editCon = snapCon;
    editMob = snapMob;
    editPts = snapPts;
    skillMenuSel = 0;
}

void gameSkillMenuNav(int delta) {
    skillMenuSel += delta;
    if (skillMenuSel < 0) {
        skillMenuSel = 4;
    }
    if (skillMenuSel > 4) {
        skillMenuSel = 0;
    }
}

void gameSkillMenuPressA(void) {
    if (skillMenuSel == 0 && editPts > 0 && editStr < 8) {
        editStr++;
        editPts--;
        playAnalogSound(8);
    } else if (skillMenuSel == 1 && editPts > 0 && editCon < 8) {
        editCon++;
        editPts--;
        playAnalogSound(8);
    } else if (skillMenuSel == 2 && editPts > 0 && editMob < 8) {
        editMob++;
        editPts--;
        playAnalogSound(8);
    } else if (skillMenuSel == 3 && playerXp >= XP_PER_SKILL) {
        playerXp -= XP_PER_SKILL;
        editPts++;
        playAnalogSound(11);
    }
}

void gameSkillMenuSave(void) {
    xpStr = editStr;
    xpCon = editCon;
    xpMob = editMob;
    skillPoints = editPts;
    refreshMaxHpFromCon();
    if (playerHp > playerMaxHp) {
        playerHp = playerMaxHp;
    }
}

void gameSkillMenuDiscard(void) {
    xpStr = snapStr;
    xpCon = snapCon;
    xpMob = snapMob;
    skillPoints = snapPts;
    playerXp = snapXp;
    refreshMaxHpFromCon();
    if (playerHp > playerMaxHp) {
        playerHp = playerMaxHp;
    }
}

int gameSkillMenuGetSel(void) {
    return skillMenuSel;
}

void gameSkillMenuGetEdit(int *str, int *con, int *mob, int *pts, int *xp) {
    *str = editStr;
    *con = editCon;
    *mob = editMob;
    *pts = editPts;
    *xp = playerXp;
}

static int inHealPool(void) {
    if (roomId != 3 && roomId != 5) {
        return 0;
    }
    int cx = playerX + PLAYER_SIZE / 2;
    int cy = playerY + PLAYER_SIZE / 2;
    return cx >= 12 * 8 + 4 && cx < 20 * 8 - 4 && cy >= 12 * 8 + 4 && cy < 20 * 8 - 4;
}

static int inSkillStationWorld(int *sx, int *sy, int *sw, int *sh) {
    if (roomId != 3 && roomId != 5) {
        return 0;
    }
    *sx = 7 * 8;
    *sy = 12 * 8;
    *sw = 8;
    *sh = 24;
    return 1;
}

static void spawnXpOrb(int x, int y, int room, int amt) {
    for (int i = 0; i < MAX_XP_ORBS; i++) {
        if (!xpOrbs[i].active) {
            xpOrbs[i].x = x;
            xpOrbs[i].y = y;
            xpOrbs[i].roomId = room;
            xpOrbs[i].amt = amt;
            xpOrbs[i].active = 1;
            xpOrbs[i].bob = i * 5;
            return;
        }
    }
}

static void dropXpOrbs(int x, int y, int room, int enemyType) {
    if (enemyType == ET_BOSS) {
        for (int k = 0; k < 8; k++) {
            int ox = x + 4 + (k % 4) * 14 - 16;
            int oy = y + 4 + (k / 4) * 12;
            spawnXpOrb(ox, oy, room, 22);
        }
        return;
    }
    int n = 3;
    int val = 18;
    if (enemyType == ET_SHOOT) {
        n = 2;
        val = 27;
    }
    for (int k = 0; k < n; k++) {
        int ox = x + 4 + (k - n / 2) * 10;
        int oy = y + 4 + ((k * 5) % 12);
        spawnXpOrb(ox, oy, room, val);
    }
}

static void updateXpOrbs(void) {
    for (int i = 0; i < MAX_XP_ORBS; i++) {
        XpOrb *o = &xpOrbs[i];
        if (!o->active || o->roomId != roomId) {
            continue;
        }
        o->bob++;
        if (collision(playerX, playerY, PLAYER_SIZE, PLAYER_SIZE, o->x, o->y, XP_ORB_SZ, XP_ORB_SZ)) {
            playerXp += o->amt;
            o->active = 0;
            playAnalogSound(9);
        }
    }
}

static int inDoorVert(int cy) {
    return (cy >= 12 * 8 + 8) && (cy < 20 * 8 - 8);
}

static int inWestDoorBand(int cx, int cy) {
    return inDoorVert(cy) && (cx >= 10) && (cx <= 26);
}

static int inEastDoorBand(int cx, int cy) {
    return inDoorVert(cy) && (cx >= 31 * 8 - 26) && (cx <= 31 * 8 - 6);
}

static int inTopDoorBand(int cx, int cy) {
    return (cx >= 12 * 8 + 8) && (cx < 20 * 8 - 8) && (cy >= 10) && (cy <= 26);
}

static int inBottomDoorBand(int cx, int cy) {
    return (cx >= 12 * 8 + 8) && (cx < 20 * 8 - 8) && (cy >= 31 * 8 - 26) && (cy <= 31 * 8 - 10);
}

static void updateCamera(void) {
    int targetX = playerX + PLAYER_SIZE / 2 - SCREENWIDTH / 2;
    int targetY = playerY + PLAYER_SIZE / 2 - SCREENHEIGHT / 2;
    int maxX = 32 * 8 - SCREENWIDTH;
    int maxY = 32 * 8 - SCREENHEIGHT;
    if (targetX < 0) {
        targetX = 0;
    }
    if (targetY < 0) {
        targetY = 0;
    }
    if (targetX > maxX) {
        targetX = maxX;
    }
    if (targetY > maxY) {
        targetY = maxY;
    }
    cameraX = targetX;
    cameraY = targetY;
    REG_BG0HOFF = cameraX;
    REG_BG0VOFF = cameraY;
    REG_BG1HOFF = cameraX >> 1;
    REG_BG1VOFF = cameraY >> 1;
}

static void setTinySprite(int oamIndex, int x, int y, int tileStart, int palIndex) {
    shadowOAM[oamIndex].attr0 = ATTR0_Y(y) | ATTR0_REGULAR | ATTR0_SQUARE | ATTR0_4BPP;
    shadowOAM[oamIndex].attr1 = ATTR1_X(x) | ATTR1_TINY;
    shadowOAM[oamIndex].attr2 = ATTR2_TILEID_1D(tileStart) | ATTR2_PRIORITY(0) | ATTR2_PALROW(palIndex);
}

static void copy16TileTo(volatile u16 *dst, const u16 *src) {
    for (int i = 0; i < 16; i++) {
        dst[i] = src[i];
    }
}

static void loadCharBlockTiles(void) {
    volatile u16 *tiles = (volatile u16 *)0x06000000;
    copy16TileTo(tiles + TILE_FLOOR * 16, SOLID_TILE_COLOR1);
    copy16TileTo(tiles + TILE_WALL * 16, SOLID_TILE_COLOR2);
    copy16TileTo(tiles + TILE_FLOOR2 * 16, SOLID_TILE_FLOOR_ALT);
    copy16TileTo(tiles + TILE_SKILL * 16, TOTEM_MID_TILE);
    copy16TileTo(tiles + TILE_FLOOR_CRACK * 16, DUNGEON_FLOOR_CRACK_TILE);
    copy16TileTo(tiles + TILE_FLOOR_SKULL * 16, DUNGEON_FLOOR_SKULL_TILE);
    copy16TileTo(tiles + TILE_FLOOR_MOSS * 16, DUNGEON_FLOOR_MOSS_TILE);
    copy16TileTo(tiles + TILE_WALL_CRACK * 16, DUNGEON_WALL_CRACK_TILE);
    copy16TileTo(tiles + TILE_WALL_VINE * 16, DUNGEON_WALL_VINE_TILE);
    copy16TileTo(tiles + TILE_TOTEM_TOP * 16, TOTEM_TOP_TILE);
    copy16TileTo(tiles + TILE_TOTEM_BOT * 16, TOTEM_BOT_TILE);
    copy16TileTo(tiles + TILE_WATER * 16, SOLID_TILE_FLOOR_ALT);

    {
        volatile u16 *para = (volatile u16 *)(0x06000000 + CHARBLOCK_PARALLAX * 0x4000);
        copy16TileTo(para + 0 * 16, SOLID_TILE_FLOOR_ALT);
        {
            u16 star[16] = {
                0x0000, 0x0100, 0x0000, 0x0010, 0x0000, 0x0100, 0x0000, 0x0010,
                0x0000, 0x0100, 0x0000, 0x0010, 0x0000, 0x0100, 0x0000, 0x0010,
            };
            copy16TileTo(para + 1 * 16, star);
        }
    }
}

static void loadObjTiles(void) {
    volatile u16 *obj = (volatile u16 *)0x06010000;
    copy16TileTo(obj + 4 * 16, SOLID_TILE_COLOR2);
    copy16TileTo(obj + 5 * 16, SOLID_TILE_COLOR2);
    copy16TileTo(obj + 6 * 16, SOLID_TILE_COLOR2);
    copy16TileTo(obj + 7 * 16, SOLID_TILE_COLOR2);
    copy16TileTo(obj + 8 * 16, SOLID_TILE_COLOR3);
    copy16TileTo(obj + 9 * 16, SOLID_TILE_COLOR3);
    copy16TileTo(obj + 10 * 16, SOLID_TILE_COLOR3);
    copy16TileTo(obj + 11 * 16, HUD_BAR_SEG);
    copy16TileTo(obj + 12 * 16, GAZE_TILE_OPEN);
    copy16TileTo(obj + 13 * 16, GAZE_TILE_CLOSED);
    copy16TileTo(obj + 14 * 16, GAZE_TILE_RING_0);
    copy16TileTo(obj + 15 * 16, GAZE_TILE_RING_1);
    copy16TileTo(obj + 16 * 16, GAZE_TILE_CLOSED);
    copy16TileTo(obj + 17 * 16, GAZE_TILE_RING_1);
    copy16TileTo(obj + 18 * 16, GAZE_TILE_RING_0);
    copy16TileTo(obj + 19 * 16, GAZE_TILE_CLOSED);

    copy16TileTo(obj + 20 * 16, HUD_BAR_SEG);
    copy16TileTo(obj + 21 * 16, HUD_BAR_SEG);
    copy16TileTo(obj + 22 * 16, HUD_BAR_SEG);
    copy16TileTo(obj + 23 * 16, HUD_BAR_SEG);
    for (int t = 0; t < 8; t++) {
        copy16TileTo(obj + (PLAYER_DOWN0_TILE + t) * 16, playerDown0Tiles + t * 16);
        copy16TileTo(obj + (PLAYER_DOWN1_TILE + t) * 16, playerDown1Tiles + t * 16);
        copy16TileTo(obj + (PLAYER_UP0_TILE + t) * 16, playerUp0Tiles + t * 16);
        copy16TileTo(obj + (PLAYER_UP1_TILE + t) * 16, playerUp1Tiles + t * 16);
        copy16TileTo(obj + (PLAYER_RIGHT0_TILE + t) * 16, playerRight0Tiles + t * 16);
        copy16TileTo(obj + (PLAYER_RIGHT1_TILE + t) * 16, playerRight1Tiles + t * 16);
        copy16TileTo(obj + (PLAYER_LEFT0_TILE + t) * 16, playerLeft0Tiles + t * 16);
        copy16TileTo(obj + (PLAYER_LEFT1_TILE + t) * 16, playerLeft1Tiles + t * 16);
    }
    for (int t = 0; t < 4; t++) {
        copy16TileTo(obj + (PLAYER_SWORD_R0_TILE + t) * 16, playerSwordR0Tiles + t * 16);
        copy16TileTo(obj + (PLAYER_SWORD_R1_TILE + t) * 16, playerSwordR1Tiles + t * 16);
        copy16TileTo(obj + (PLAYER_SWORD_R2_TILE + t) * 16, playerSwordR2Tiles + t * 16);
        copy16TileTo(obj + (PLAYER_SWORD_L0_TILE + t) * 16, playerSwordL0Tiles + t * 16);
        copy16TileTo(obj + (PLAYER_SWORD_L1_TILE + t) * 16, playerSwordL1Tiles + t * 16);
        copy16TileTo(obj + (PLAYER_SWORD_L2_TILE + t) * 16, playerSwordL2Tiles + t * 16);
        copy16TileTo(obj + (PLAYER_SWORD_U0_TILE + t) * 16, playerSwordU0Tiles + t * 16);
        copy16TileTo(obj + (PLAYER_SWORD_U1_TILE + t) * 16, playerSwordU1Tiles + t * 16);
        copy16TileTo(obj + (PLAYER_SWORD_U2_TILE + t) * 16, playerSwordU2Tiles + t * 16);
        copy16TileTo(obj + (PLAYER_SWORD_D0_TILE + t) * 16, playerSwordD0Tiles + t * 16);
        copy16TileTo(obj + (PLAYER_SWORD_D1_TILE + t) * 16, playerSwordD1Tiles + t * 16);
        copy16TileTo(obj + (PLAYER_SWORD_D2_TILE + t) * 16, playerSwordD2Tiles + t * 16);
    }
    copy16TileTo(obj + (HEART_FULL_TILE + 0) * 16, Pixel_Heart_Sprite_Sheet_16x16Tiles + 0 * 16);
    copy16TileTo(obj + (HEART_FULL_TILE + 1) * 16, Pixel_Heart_Sprite_Sheet_16x16Tiles + 1 * 16);
    copy16TileTo(obj + (HEART_FULL_TILE + 2) * 16, Pixel_Heart_Sprite_Sheet_16x16Tiles + 6 * 16);
    copy16TileTo(obj + (HEART_FULL_TILE + 3) * 16, Pixel_Heart_Sprite_Sheet_16x16Tiles + 7 * 16);
    copy16TileTo(obj + (HEART_EMPTY_TILE + 0) * 16, Pixel_Heart_Sprite_Sheet_16x16Tiles + 4 * 16);
    copy16TileTo(obj + (HEART_EMPTY_TILE + 1) * 16, Pixel_Heart_Sprite_Sheet_16x16Tiles + 5 * 16);
    copy16TileTo(obj + (HEART_EMPTY_TILE + 2) * 16, Pixel_Heart_Sprite_Sheet_16x16Tiles + 10 * 16);
    copy16TileTo(obj + (HEART_EMPTY_TILE + 3) * 16, Pixel_Heart_Sprite_Sheet_16x16Tiles + 11 * 16);

    for (int d = 0; d < 10; d++) {
        copy16TileTo(obj + (DIGIT_TILE_BASE + d) * 16, DIGIT_TILES[d]);
    }

    for (int t = 0; t < 4; t++) {
        copy16TileTo(obj + (SLIME_FRAME0_TILE + t) * 16, slimeFrame0Tiles + t * 16);
        copy16TileTo(obj + (SLIME_FRAME1_TILE + t) * 16, slimeFrame1Tiles + t * 16);
        copy16TileTo(obj + (PLANT_OPEN_TILE + t) * 16, plantOpenTiles + t * 16);
        copy16TileTo(obj + (PLANT_CLOSED_TILE + t) * 16, plantClosedTiles + t * 16);
    }
    copy16TileTo(obj + XP_ORB0_TILE * 16, xpOrb0Tiles);
    copy16TileTo(obj + XP_ORB1_TILE * 16, xpOrb1Tiles);
    copy16TileTo(obj + XP_ORB2_TILE * 16, xpOrb2Tiles);

    copy16TileTo(obj + PROJ_SEED0_TILE * 16, projSeed0Tiles);
    copy16TileTo(obj + PROJ_SEED1_TILE * 16, projSeed1Tiles);
    copy16TileTo(obj + PROJ_RED0_TILE * 16, projRed0Tiles);
    copy16TileTo(obj + PROJ_RED1_TILE * 16, projRed1Tiles);
    copy16TileTo(obj + PROJ_BOSS0_TILE * 16, projBoss0Tiles);
    copy16TileTo(obj + PROJ_BOSS1_TILE * 16, projBoss1Tiles);
    copy16TileTo(obj + PROJ_PLAYER0_TILE * 16, projPlayer0Tiles);
    copy16TileTo(obj + PROJ_PLAYER1_TILE * 16, projPlayer1Tiles);

    for (int t = 0; t < 32; t++) {
        copy16TileTo(obj + (BOSS_FRAME_TILE + t) * 16, bossQueenTiles32x64 + t * 16);
    }
}

static u16 softenBossPalette555(u16 c, int idx) {
    int r;
    int g;
    int b;
    int sum;
    if (idx == 0) {
        return c;
    }
    r = c & 31;
    g = (c >> 5) & 31;
    b = (c >> 10) & 31;
    sum = r + g + b;
    if (sum >= 62) {
        r = (r * 23 + 8 * 8) >> 5;
        g = (g * 23 + 6 * 8) >> 5;
        b = (b * 23 + 10 * 8) >> 5;
        if (b < 31) {
            b++;
        }
        if (r > 28) {
            r--;
        }
    }
    return RGB(r, g, b);
}

static void loadPalettes(void) {
    BG_PALETTE[0] = RGB(0, 0, 0);
    BG_PALETTE[1] = RGB(2, 1, 3);
    BG_PALETTE[2] = RGB(5, 4, 5);
    BG_PALETTE[3] = RGB(11, 9, 10);
    BG_PALETTE[4] = RGB(14, 2, 2);
    BG_PALETTE[5] = RGB(4, 2, 5);
    BG_PALETTE[6] = RGB(5, 10, 5);
    BG_PALETTE[7] = RGB(28, 26, 25);
    BG_PALETTE[8]  = RGB(1, 2, 4);
    BG_PALETTE[9]  = RGB(3, 10, 16);
    BG_PALETTE[10] = RGB(8, 18, 24);
    BG_PALETTE[11] = RGB(28, 31, 31);
    BG_PALETTE[3 * 16 + 1] = RGB(2, 1, 3);
    BG_PALETTE[3 * 16 + 2] = RGB(11, 9, 10);
    BG_PALETTE[3 * 16 + 3] = RGB(31, 10, 25);
    BG_PALETTE[3 * 16 + 4] = RGB(31, 28, 31);
    BG_PALETTE[3 * 16 + 5] = RGB(5, 4, 5);

    SPRITE_PAL[0] = 0;
    SPRITE_PAL[1] = RGB(4, 22, 8);
    SPRITE_PAL[2] = RGB(28, 6, 6);
    SPRITE_PAL[3] = RGB(28, 28, 12);
    for (int i = 0; i < 16; i++) {
        SPRITE_PAL[1 * 16 + i] = playerFramePal[i];
        SPRITE_PAL[3 * 16 + i] = Pixel_Heart_Sprite_Sheet_16x16Pal[i];
        SPRITE_PAL[5 * 16 + i] = enemyFramePal[i];
    }
    SPRITE_PAL[2 * 16 + 1] = RGB(26, 10, 18);
    SPRITE_PAL[2 * 16 + 2] = RGB(14, 4, 10);
    SPRITE_PAL[2 * 16 + 3] = RGB(8, 2, 6);
    SPRITE_PAL[4 * 16 + 1] = RGB(31, 8, 24);
    SPRITE_PAL[4 * 16 + 2] = RGB(31, 20, 28);
    SPRITE_PAL[4 * 16 + 3] = RGB(24, 4, 16);
    SPRITE_PAL[6 * 16 + 1] = RGB(31, 31, 31);
    SPRITE_PAL[6 * 16 + 2] = RGB(10, 14, 24);
    SPRITE_PAL[6 * 16 + 3] = RGB(22, 22, 26);
    SPRITE_PAL[6 * 16 + 11] = RGB(20, 26, 31);
    SPRITE_PAL[6 * 16 + 12] = RGB(26, 30, 31);
    SPRITE_PAL[6 * 16 + 13] = RGB(31, 31, 31);

    for (int i = 0; i < 16; i++) {
        SPRITE_PAL[7 * 16 + i] = softenBossPalette555(bossQueenPal[i], i);
    }

}

static void buildRoomIntoRam(int id) {
    int baseFloor = TILE_FLOOR;
    for (int i = 0; i < 1024; i++) {
        int tx = i & 31;
        int ty = i >> 5;
        unsigned int h = (unsigned int)(tx * 31 + ty * 17 + id * 13);
        h = (h ^ (h >> 5)) * 2654435761u;
        unsigned int r = (h >> 17) & 0x3F;
        int t = baseFloor;
        if (id == 7) {
            if (r < 2) {
                t = TILE_FLOOR_CRACK;
            } else if (r == 19) {
                t = TILE_FLOOR_MOSS;
            }
        } else if (r < 4) {
            t = TILE_FLOOR_CRACK;
        } else if (r < 6) {
            t = TILE_FLOOR_MOSS;
        } else if (r < 7) {
            t = TILE_FLOOR_SKULL;
        }
        ramMap[i] = TILEMAP_ENTRY_TILEID(t);
    }
    for (int x = 0; x < 32; x++) {
        unsigned int h0 = (unsigned int)(x * 41 + id * 7);
        h0 = (h0 ^ (h0 >> 4)) * 2654435761u;
        unsigned int h1 = (unsigned int)(x * 23 + id * 19 + 91);
        h1 = (h1 ^ (h1 >> 4)) * 2654435761u;
        int wt0 = TILE_WALL;
        int wt1 = TILE_WALL;
        unsigned int r0 = (h0 >> 18) & 0xF;
        unsigned int r1 = (h1 >> 18) & 0xF;
        if (r0 < 2) wt0 = TILE_WALL_CRACK;
        else if (r0 < 4) wt0 = TILE_WALL_VINE;
        if (r1 < 2) wt1 = TILE_WALL_CRACK;
        else if (r1 < 4) wt1 = TILE_WALL_VINE;
        ramMap[x] = TILEMAP_ENTRY_TILEID(wt0);
        ramMap[31 * 32 + x] = TILEMAP_ENTRY_TILEID(wt1);
    }
    for (int y = 0; y < 32; y++) {
        unsigned int h0 = (unsigned int)(y * 53 + id * 11);
        h0 = (h0 ^ (h0 >> 4)) * 2654435761u;
        unsigned int h1 = (unsigned int)(y * 29 + id * 17 + 47);
        h1 = (h1 ^ (h1 >> 4)) * 2654435761u;
        int wt0 = TILE_WALL;
        int wt1 = TILE_WALL;
        unsigned int r0 = (h0 >> 18) & 0xF;
        unsigned int r1 = (h1 >> 18) & 0xF;
        if (r0 < 2) wt0 = TILE_WALL_CRACK;
        else if (r0 < 4) wt0 = TILE_WALL_VINE;
        if (r1 < 2) wt1 = TILE_WALL_CRACK;
        else if (r1 < 4) wt1 = TILE_WALL_VINE;
        ramMap[y * 32] = TILEMAP_ENTRY_TILEID(wt0);
        ramMap[y * 32 + 31] = TILEMAP_ENTRY_TILEID(wt1);
    }
    if (roomWestLink(id) >= 0) {
        for (int y = 12; y < 20; y++) {
            ramMap[y * 32 + 0] = TILEMAP_ENTRY_TILEID(baseFloor);
        }
    }
    if (id == 7 && bossWestDoorLocked) {
        for (int y = 12; y < 20; y++) {
            ramMap[y * 32 + 0] = TILEMAP_ENTRY_TILEID(TILE_WALL);
        }
    }
    if (roomEastLink(id) >= 0) {
        for (int y = 12; y < 20; y++) {
            ramMap[y * 32 + 31] = TILEMAP_ENTRY_TILEID(baseFloor);
        }
    }
    if (roomNorthLink(id) >= 0) {
        for (int x = 12; x < 20; x++) {
            ramMap[0 * 32 + x] = TILEMAP_ENTRY_TILEID(baseFloor);
        }
    }
    if (roomSouthLink(id) >= 0) {
        for (int x = 12; x < 20; x++) {
            ramMap[31 * 32 + x] = TILEMAP_ENTRY_TILEID(baseFloor);
        }
    }
    if (id == 7 && bossDefeated) {
        for (int y = 12; y < 20; y++) {
            ramMap[y * 32 + 31] = TILEMAP_ENTRY_TILEID(baseFloor);
        }
    }
    if (id == 3 || id == 5) {
        for (int py = 12; py < 20; py++) {
            for (int px = 12; px < 20; px++) {
                ramMap[py * 32 + px] = TILEMAP_ENTRY_TILEID(TILE_WATER);
            }
        }
        int tx = 7;
        int ty = 12;
        ramMap[ty * 32 + tx]       = TILEMAP_ENTRY_TILEID(TILE_TOTEM_TOP) | TILEMAP_ENTRY_PALROW(3);
        ramMap[(ty + 1) * 32 + tx] = TILEMAP_ENTRY_TILEID(TILE_SKILL)     | TILEMAP_ENTRY_PALROW(3);
        ramMap[(ty + 2) * 32 + tx] = TILEMAP_ENTRY_TILEID(TILE_TOTEM_BOT) | TILEMAP_ENTRY_PALROW(3);
    }
}

static void copyMapToVRAM(void) {
    volatile u16 *screenMap = (volatile u16 *)(0x06000000 + SCREENBLOCK_MAP * 2048);
    for (int i = 0; i < 1024; i++) {
        screenMap[i] = ramMap[i];
    }
}

static void buildParallaxMap(void) {
    for (int i = 0; i < 1024; i++) {
        parallaxMap[i] = TILEMAP_ENTRY_TILEID(0);
    }
    for (int y = 0; y < 32; y++) {
        for (int x = 0; x < 32; x++) {
            if (((x * 7 + y * 11) & 15) == 0) {
                parallaxMap[y * 32 + x] = TILEMAP_ENTRY_TILEID(1);
            }
        }
    }
    {
        volatile u16 *screenMap = (volatile u16 *)(0x06000000 + SCREENBLOCK_PARALLAX * 2048);
        for (int i = 0; i < 1024; i++) {
            screenMap[i] = parallaxMap[i];
        }
    }
}

static int mapTileAt(int tx, int ty) {
    if (tx < 0 || tx >= 32 || ty < 0 || ty >= 32) {
        return TILE_WALL;
    }
    return ramMap[ty * 32 + tx] & 0x3FF;
}

static int wallAtPixel(int x, int y) {
    int tx = x / 8;
    int ty = y / 8;
    int t = mapTileAt(tx, ty);
    return t == TILE_WALL || t == TILE_WALL_CRACK || t == TILE_WALL_VINE;
}

static int collidesWallRect(int x, int y, int w, int h) {
    return wallAtPixel(x, y) || wallAtPixel(x + w - 1, y) || wallAtPixel(x, y + h - 1) ||
           wallAtPixel(x + w - 1, y + h - 1);
}

static void tryMoveAxis(int *px, int *py, int dx, int dy) {
    if (dx != 0) {
        *px += dx;
        if (collidesWallRect(*px, *py, PLAYER_SIZE, PLAYER_SIZE)) {
            *px -= dx;
        }
    }
    if (dy != 0) {
        *py += dy;
        if (collidesWallRect(*px, *py, PLAYER_SIZE, PLAYER_SIZE)) {
            *py -= dy;
        }
    }
}

static void tryMoveEnemy(Enemy *e, int dx, int dy) {
    if (dx != 0) {
        e->x += dx;
        if (collidesWallRect(e->x, e->y, PLAYER_SIZE, PLAYER_SIZE)) {
            e->x -= dx;
            e->vx = -e->vx;
        }
    }
    if (dy != 0) {
        e->y += dy;
        if (collidesWallRect(e->x, e->y, PLAYER_SIZE, PLAYER_SIZE)) {
            e->y -= dy;
            e->vy = -e->vy;
        }
    }
}

static void checkRoomTransition(void) {
    if (roomTransitionCooldown > 0) {
        return;
    }
    int cx = playerX + PLAYER_SIZE / 2;
    int cy = playerY + PLAYER_SIZE / 2;
    if (BUTTON_HELD(BUTTON_RIGHT) && inEastDoorBand(cx, cy) && roomEastLink(roomId) >= 0) {
        roomId = roomEastLink(roomId);
        playerX = SPAWN_FROM_EAST_X;
        playerY = cy - PLAYER_SIZE / 2;
        roomTransitionCooldown = ROOM_TRANSITION_COOLDOWN;
        clearProjectiles();
        if (roomId == 7) {
            bossWestDoorLocked = 1;
            bossShotsSinceParry = 0;
            bossPatternTimer = 0;
            bossSpinAngle = 0;
        }
        buildRoomIntoRam(roomId);
        copyMapToVRAM();
        return;
    }
    if (BUTTON_HELD(BUTTON_LEFT) && inWestDoorBand(cx, cy) && roomWestLink(roomId) >= 0) {
        if (roomId == 7 && bossWestDoorLocked) {
            return;
        }
        roomId = roomWestLink(roomId);
        playerX = SPAWN_FROM_WEST_X;
        playerY = cy - PLAYER_SIZE / 2;
        roomTransitionCooldown = ROOM_TRANSITION_COOLDOWN;
        clearProjectiles();
        buildRoomIntoRam(roomId);
        copyMapToVRAM();
        return;
    }
    if (BUTTON_HELD(BUTTON_UP) && inTopDoorBand(cx, cy) && roomNorthLink(roomId) >= 0) {
        roomId = roomNorthLink(roomId);
        playerY = SPAWN_FROM_SOUTH_Y;
        playerX = cx - PLAYER_SIZE / 2;
        roomTransitionCooldown = ROOM_TRANSITION_COOLDOWN;
        clearProjectiles();
        if (roomId == 7) {
            bossWestDoorLocked = 1;
            bossShotsSinceParry = 0;
            bossPatternTimer = 0;
            bossSpinAngle = 0;
        }
        buildRoomIntoRam(roomId);
        copyMapToVRAM();
        return;
    }
    if (BUTTON_HELD(BUTTON_DOWN) && inBottomDoorBand(cx, cy) && roomSouthLink(roomId) >= 0) {
        roomId = roomSouthLink(roomId);
        playerY = SPAWN_FROM_NORTH_Y;
        playerX = cx - PLAYER_SIZE / 2;
        roomTransitionCooldown = ROOM_TRANSITION_COOLDOWN;
        clearProjectiles();
        buildRoomIntoRam(roomId);
        copyMapToVRAM();
    }
}

static void spawnProjectileKind(int x, int y, int vx, int vy, int kind, int owner) {
    for (int i = 0; i < MAX_PROJ; i++) {
        if (!projs[i].active) {
            projs[i].x = x;
            projs[i].y = y;
            projs[i].vx = vx;
            projs[i].vy = vy;
            projs[i].active = 1;
            projs[i].ttl = 220;
            projs[i].kind = kind;
            projs[i].owner = owner;
            return;
        }
    }
}

static void spawnProjectile(int x, int y, int vx, int vy) {
    spawnProjectileKind(x, y, vx, vy, 0, 1);
}

static void clearProjectiles(void) {
    for (int i = 0; i < MAX_PROJ; i++) {
        projs[i].active = 0;
        projs[i].ttl = 0;
        projs[i].kind = 0;
        projs[i].owner = 1;
    }
}

static void mutateRuntimeAssets(void) {
    int pulse = (gameTick >> 3) & 7;
    int up = (pulse < 4) ? pulse : (7 - pulse);

    BG_PALETTE[3 * 16 + 3] = RGB(24 + up, 4 + up / 2, 18 + up);
    BG_PALETTE[3 * 16 + 4] = RGB(28 + up / 2, 22 + up, 28);

    {
        int wp = (gameTick >> 2) & 7;
        int wu = (wp < 4) ? wp : (7 - wp);
        BG_PALETTE[9]  = RGB(2 + wu / 2, 8 + wu / 2, 14 + wu / 2);
        BG_PALETTE[10] = RGB(6 + wu, 16 + wu, 22 + wu);
        BG_PALETTE[11] = RGB(28, 31, 31);
    }

    {
        volatile u16 *tiles = (volatile u16 *)0x06000000;
        unsigned int t = gameTick;
        for (int row = 0; row < 8; row++) {
            int yShift = (row + (int)(t >> 3)) & 7;
            unsigned int seed = (unsigned int)(yShift * 7919u + (row * 31)) ^ ((t >> 6) & 0xFF);
            seed = (seed ^ (seed >> 5)) * 2654435761u;
            int b1 = (seed >> 13) & 7;
            int b2 = (seed >> 19) & 7;
            int p[8];
            for (int x = 0; x < 8; x++) {
                p[x] = ((x ^ row) & 1) ? 9 : 8;
            }
            if ((seed & 1) == 0) p[b1] = 11;
            if ((seed >> 7) & 1) p[b2] = 10;
            u16 lo = (u16)(p[0] | (p[1] << 4) | (p[2] << 8) | (p[3] << 12));
            u16 hi = (u16)(p[4] | (p[5] << 4) | (p[6] << 8) | (p[7] << 12));
            tiles[TILE_WATER * 16 + row * 2 + 0] = lo;
            tiles[TILE_WATER * 16 + row * 2 + 1] = hi;
        }
    }
}

static void gazeBossReset(void) {
    gazeSt = GAZE_MOVE;
    gazeTimer = 0;
    gazeLastAttack = 1;
    gazeEyeOpen = 0;
    gazeIframes = 0;
    gazeBeamDx = 1;
    gazeBeamDy = 0;
    gazeMoveWp = 0;
    gazeAttackHitLatch = 0;
}

static int gazePhaseOf(const Enemy *e) {
    if (e->hp * 100 > BOSS_MAX_HP * 60) {
        return 1;
    }
    if (e->hp * 100 > BOSS_MAX_HP * 25) {
        return 2;
    }
    return 3;
}

static int gazeSpdMult(const Enemy *e) {
    int p = gazePhaseOf(e);
    if (p == 1) {
        return 8;
    }
    if (p == 2) {
        return 9;
    }
    return 10;
}

static int gazeBossSwordVulnerable(void) {
    return gazeSt == GAZE_VULNERABLE && gazeEyeOpen && gazeIframes == 0;
}

#define GAZE_BEAM_LEN 72

static int gazeBeamHitsPlayer(int bx, int by, int bdx, int bdy) {
    int hx;
    int hy;
    int hw;
    int hh;
    if (bdx != 0) {
        if (bdx > 0) {
            hx = bx;
            hy = by - 7;
            hw = GAZE_BEAM_LEN;
            hh = 12;
        } else {
            hx = bx - GAZE_BEAM_LEN;
            hy = by - 7;
            hw = GAZE_BEAM_LEN;
            hh = 12;
        }
    } else {
        if (bdy > 0) {
            hx = bx - 7;
            hy = by;
            hw = 12;
            hh = GAZE_BEAM_LEN;
        } else {
            hx = bx - 7;
            hy = by - GAZE_BEAM_LEN;
            hw = 12;
            hh = GAZE_BEAM_LEN;
        }
    }
    return collision(playerX, playerY, PLAYER_SIZE, PLAYER_SIZE, hx, hy, hw, hh);
}

static void updateGazeBoss(Enemy *e) {
    if (gazeIframes > 0) {
        gazeIframes--;
    }
    static const int wpx[4] = {96, 200, 200, 96};
    static const int wpy[4] = {96, 96, 144, 144};
    int spd = gazeSpdMult(e);
    int ph = gazePhaseOf(e);
    int mv = 12 - ph;
    if (mv < 7) {
        mv = 7;
    }

    switch (gazeSt) {
        case GAZE_MOVE:
            gazeEyeOpen = 0;
            gazeTimer++;
            if ((gameTick % mv) == 0) {
                int tx = wpx[gazeMoveWp];
                int ty = wpy[gazeMoveWp];
                if (e->x < tx - 4) {
                    tryMoveEnemy(e, 1, 0);
                } else if (e->x > tx + 4) {
                    tryMoveEnemy(e, -1, 0);
                }
                if (e->y < ty - 4) {
                    tryMoveEnemy(e, 0, 1);
                } else if (e->y > ty + 4) {
                    tryMoveEnemy(e, 0, -1);
                }
                int adx = e->x - tx;
                int ady = e->y - ty;
                if (adx < 0) {
                    adx = -adx;
                }
                if (ady < 0) {
                    ady = -ady;
                }
                if (adx < 12 && ady < 12) {
                    gazeMoveWp = (gazeMoveWp + 1) % 4;
                }
            }
            if (gazeTimer >= 140 + ph * 18) {
                gazeTimer = 0;
                if (gazeLastAttack == 0) {
                    gazeLastAttack = 1;
                    gazeSt = GAZE_TELEGRAPH_SWOOP;
                } else {
                    gazeLastAttack = 0;
                    gazeSt = GAZE_TELEGRAPH_BEAM;
                }
            }
            break;

        case GAZE_TELEGRAPH_BEAM:
            gazeTimer++;
            gazeEyeOpen = ((gazeTimer >> 3) & 1);
            if (gazeTimer >= 72) {
                int pcx = playerX + 8;
                int pcy = playerY + 8;
                int ecx = e->x + 8;
                int ecy = e->y + 8;
                int adx = pcx - ecx;
                int ady = pcy - ecy;
                if (adx * adx > ady * ady) {
                    gazeBeamDx = (adx > 0) ? 1 : -1;
                    gazeBeamDy = 0;
                } else {
                    gazeBeamDy = (ady > 0) ? 1 : -1;
                    gazeBeamDx = 0;
                }
                if (adx == 0 && ady == 0) {
                    gazeBeamDx = 1;
                    gazeBeamDy = 0;
                }
                gazeSt = GAZE_BEAM_ACTIVE;
                gazeTimer = 0;
                gazeAttackHitLatch = 0;
                gazeEyeOpen = 0;
            }
            break;

        case GAZE_BEAM_ACTIVE:
            gazeTimer++;
            gazeEyeOpen = 0;
            if (!gazeAttackHitLatch && invincibleTimer == 0 &&
                gazeBeamHitsPlayer(e->x + 8, e->y + 8, gazeBeamDx, gazeBeamDy)) {
                damagePlayerOnce();
                gazeAttackHitLatch = 1;
            }
            if (gazeTimer >= 22) {
                gazeSt = GAZE_BEAM_RECOVER;
                gazeTimer = 0;
            }
            break;

        case GAZE_BEAM_RECOVER:
            gazeTimer++;
            gazeEyeOpen = 0;
            if (gazeTimer >= 32) {
                gazeSt = GAZE_VULNERABLE;
                gazeTimer = 0;
                gazeEyeOpen = 1;
            }
            break;

        case GAZE_TELEGRAPH_SWOOP:
            gazeTimer++;
            gazeEyeOpen = ((gazeTimer >> 3) & 1);
            if (gazeTimer >= 60) {
                gazeSwoopTx = playerX;
                gazeSwoopTy = playerY;
                gazeSt = GAZE_SWOOP_ACTIVE;
                gazeTimer = 0;
                gazeAttackHitLatch = 0;
                gazeEyeOpen = 0;
            }
            break;

        case GAZE_SWOOP_ACTIVE:
            gazeTimer++;
            gazeEyeOpen = 0;
            {
                int step = (2 * spd) / 10;
                if (step < 1) {
                    step = 1;
                }
                if (step > 2) {
                    step = 2;
                }
                int k;
                for (k = 0; k < step; k++) {
                    if (e->x < gazeSwoopTx - 2) {
                        tryMoveEnemy(e, 1, 0);
                    } else if (e->x > gazeSwoopTx + 2) {
                        tryMoveEnemy(e, -1, 0);
                    }
                    if (e->y < gazeSwoopTy - 2) {
                        tryMoveEnemy(e, 0, 1);
                    } else if (e->y > gazeSwoopTy + 2) {
                        tryMoveEnemy(e, 0, -1);
                    }
                }
            }
            if (!gazeAttackHitLatch && invincibleTimer == 0 &&
                collision(playerX, playerY, PLAYER_SIZE, PLAYER_SIZE, e->x - 2, e->y - 2, 20, 20)) {
                damagePlayerOnce();
                gazeAttackHitLatch = 1;
            }
            if (gazeTimer >= 40) {
                gazeSt = GAZE_SWOOP_RECOVER;
                gazeTimer = 0;
            }
            break;

        case GAZE_SWOOP_RECOVER:
            gazeTimer++;
            gazeEyeOpen = 0;
            if (gazeTimer >= 38) {
                gazeSt = GAZE_VULNERABLE;
                gazeTimer = 0;
                gazeEyeOpen = 1;
            }
            break;

        case GAZE_VULNERABLE:
            gazeTimer++;
            gazeEyeOpen = 1;
            if (gazeTimer >= 100) {
                gazeSt = GAZE_MOVE;
                gazeTimer = 0;
                gazeEyeOpen = 0;
            }
            break;

        case GAZE_STUNNED:
            gazeTimer++;
            gazeEyeOpen = 0;
            if (gazeTimer >= 30) {
                gazeSt = GAZE_MOVE;
                gazeTimer = 0;
            }
            break;

        default:
            gazeSt = GAZE_MOVE;
            gazeTimer = 0;
            break;
    }
}

static void enemyTryShoot(Enemy *e) {
    if (e->shootCd > 0) {
        e->shootCd--;
        return;
    }
    int pcx = playerX + 8;
    int pcy = playerY + 8;
    int ecx = e->x + 8;
    int ecy = e->y + 8;
    int adx = pcx - ecx;
    int ady = pcy - ecy;
    if (adx * adx + ady * ady > 120 * 120) {
        return;
    }
    int sx = 0;
    int sy = 0;
    if (adx * adx > ady * ady) {
        sx = (adx > 0) ? 1 : -1;
        sy = (ady != 0) ? (ady > 0 ? 1 : -1) : 0;
    } else {
        sy = (ady > 0) ? 1 : -1;
        sx = (adx != 0) ? (adx > 0 ? 1 : -1) : 0;
    }
    if (sx == 0 && sy == 0) {
        sy = 1;
    }
    spawnProjectileKind(e->x + 4, e->y + 4, sx, sy, 0, 1);
    e->shootCd = 95;
}

static void enemyInitOne(int slot, int r, int typ, int x, int y, int vx, int vy, int hp, int scd) {
    enemies[slot].active = 1;
    enemies[slot].roomId = r;
    enemies[slot].type = typ;
    enemies[slot].x = x;
    enemies[slot].y = y;
    enemies[slot].vx = vx;
    enemies[slot].vy = vy;
    enemies[slot].hp = hp;
    enemies[slot].shootCd = scd;
    enemies[slot].swingRemain = 0;
    enemies[slot].attackCd = 0;
    enemies[slot].swingDealt = 0;
    enemies[slot].hurtTimer = 0;
    enemies[slot].knockX = 0;
    enemies[slot].knockY = 0;
}

static Enemy *findBoss(void) {
    for (int i = 0; i < MAX_ENEMIES; i++) {
        if (enemies[i].active && enemies[i].type == ET_BOSS) {
            return &enemies[i];
        }
    }
    return 0;
}

static void initEnemies(void) {
    int i;
    for (i = 0; i < MAX_ENEMIES; i++) {
        enemies[i].active = 0;
    }
    for (i = 0; i < MAX_PROJ; i++) {
        projs[i].active = 0;
        projs[i].kind = 0;
        projs[i].owner = 1;
    }

    i = 0;
    {
        int hpBonus = currentFloor - 1;
        int shootBase = 25 - currentFloor * 2;
        if (shootBase < 12) {
            shootBase = 12;
        }
        enemyInitOne(i++, 0, ET_PATROL, 14 * 8, 11 * 8, 1, 0, 3 + hpBonus, 0);
        enemyInitOne(i++, 0, ET_PATROL, 22 * 8, 14 * 8, -1, 0, 3 + hpBonus, 0);
        enemyInitOne(i++, 0, ET_SHOOT, 26 * 8, 12 * 8, 0, 1, 2 + hpBonus, shootBase);

        enemyInitOne(i++, 1, ET_PATROL, 14 * 8, 17 * 8, -1, 0, 3 + hpBonus, 0);
        enemyInitOne(i++, 1, ET_PATROL, 21 * 8, 10 * 8, 1, 0, 3 + hpBonus, 0);
        enemyInitOne(i++, 1, ET_BRUISER, 17 * 8, 13 * 8, 0, 0, 3 + hpBonus, 0);

        enemyInitOne(i++, 2, ET_PATROL, 20 * 8, 14 * 8, 0, -1, 3 + hpBonus, 0);
        enemyInitOne(i++, 2, ET_SHOOT, 26 * 8, 18 * 8, -1, 0, 2 + hpBonus, shootBase + 5);

        enemyInitOne(i++, 4, ET_PATROL, 12 * 8, 12 * 8, 1, 0, 3 + hpBonus, 0);
        enemyInitOne(i++, 4, ET_SHOOT, 18 * 8, 11 * 8, 0, 1, 2 + hpBonus, shootBase + 8);
        enemyInitOne(i++, 4, ET_SHOOT, 26 * 8, 16 * 8, 0, -1, 2 + hpBonus, shootBase + 8);

        enemyInitOne(i++, 6, ET_BRUISER, 13 * 8, 14 * 8, 0, 0, 3 + hpBonus, 0);
        enemyInitOne(i++, 6, ET_BRUISER, 23 * 8, 14 * 8, 0, 0, 3 + hpBonus, 0);
        enemyInitOne(i++, 6, ET_PATROL, 18 * 8, 19 * 8, 1, 0, 3 + hpBonus, 0);

        if (currentFloor >= 2) {
            enemyInitOne(i++, 3, ET_SHOOT, 19 * 8, 14 * 8, 0, 1, 2 + hpBonus, shootBase);
        }
        if (currentFloor >= 3) {
            enemyInitOne(i++, 5, ET_BRUISER, 17 * 8, 14 * 8, 0, 0, 3 + hpBonus, 0);
        }
        if (currentFloor >= 4) {
            enemyInitOne(i++, 2, ET_BRUISER, 13 * 8, 11 * 8, 0, 0, 3 + hpBonus, 0);
        }

        enemyInitOne(i++, 7, ET_BOSS, 14 * 8, 10 * 8, 0, 0, BOSS_MAX_HP + hpBonus * 8, 16);
    }

    enemiesAlive = 0;
    for (i = 0; i < MAX_ENEMIES; i++) {
        if (enemies[i].active && enemies[i].hp > 0) {
            enemiesAlive++;
        }
    }
}

void gameSetRelaxMode(int enable) {
    relaxMode = enable ? 1 : 0;
}

void gameReset(void) {
    roomId = 0;
    currentFloor = 1;
    gameTick = 0;
    roomTransitionCooldown = 0;
    healPoolTimer = 0;
    playerX = 3 * 8;
    playerY = 10 * 8;
    playerDir = 0;
    xpStr = 0;
    xpCon = 0;
    xpMob = 0;
    skillPoints = 0;
    playerXp = 0;
    if (relaxMode) {
        skillPoints = 1;
        playerXp = 55;
    }
    skillMenuOpenRequest = 0;
    bossWestDoorLocked = 0;
    bossShotsSinceParry = 0;
    bossPatternTimer = 0;
    bossSpinAngle = 0;
    bossDefeated = 0;
    refreshMaxHpFromCon();
    playerHp = playerMaxHp;
    invincibleTimer = 0;
    attackTimer = 0;
    attackDealtDamage = 0;
    attackFacing = 0;
    initEnemies();
    gazeBossReset();
    for (int i = 0; i < MAX_XP_ORBS; i++) {
        xpOrbs[i].active = 0;
    }
    buildRoomIntoRam(roomId);
}

void gameRenderInit(void) {
    loadPalettes();
    loadCharBlockTiles();
    loadObjTiles();
    copyMapToVRAM();
    buildParallaxMap();

    REG_BG0CNT = BG_CHARBLOCK(CHARBLOCK_TILES) | BG_SCREENBLOCK(SCREENBLOCK_MAP) | BG_SIZE_SMALL |
                 BG_4BPP;
    REG_BG1CNT = BG_CHARBLOCK(CHARBLOCK_PARALLAX) | BG_SCREENBLOCK(SCREENBLOCK_PARALLAX) |
                 BG_SIZE_SMALL | BG_4BPP;
    syncBgLayers();
    updateCamera();

    hideSprites();
    drawSprites();
    updateOAM();
}

void gameAudioStart(void) {
    stopSounds();
    setupGameplaySound();
    audioRunning = 1;
}

void gameAudioStop(void) {
    if (audioRunning) {
        stopSounds();
        audioRunning = 0;
    }
}

void gameAudioPause(void) {
    if (audioRunning) {
        pauseSounds();
    }
}

void gameAudioResume(void) {
    if (audioRunning) {
        unpauseSounds();
    }
}

void gameSetFloor(int floor) {
    if (floor < 1) {
        floor = 1;
    }
    if (floor > 6) {
        floor = 6;
    }
    currentFloor = floor;

    roomId = 0;
    bossDefeated = 0;
    bossWestDoorLocked = 0;
    bossShotsSinceParry = 0;
    bossPatternTimer = 0;
    bossSpinAngle = 0;
    gameTick = 0;
    roomTransitionCooldown = 0;
    healPoolTimer = 0;
    playerX = 3 * 8;
    playerY = 10 * 8;
    playerDir = 0;
    invincibleTimer = 0;
    attackTimer = 0;
    attackDealtDamage = 0;
    attackFacing = 0;

    initEnemies();
    for (int i = 0; i < MAX_XP_ORBS; i++) {
        xpOrbs[i].active = 0;
    }
    buildRoomIntoRam(roomId);
}

int gameGetFloor(void) {
    return currentFloor;
}

static void setRegularSprite(int oamIndex, int x, int y, int tileStart, int palIndex) {
    shadowOAM[oamIndex].attr0 = ATTR0_Y(y) | ATTR0_REGULAR | ATTR0_SQUARE | ATTR0_4BPP;
    shadowOAM[oamIndex].attr1 = ATTR1_X(x) | ATTR1_SMALL;
    shadowOAM[oamIndex].attr2 = ATTR2_TILEID_1D(tileStart) | ATTR2_PRIORITY(0) | ATTR2_PALROW(palIndex);
}

static void setTallSprite(int oamIndex, int x, int y, int tileStart, int palIndex) {
    shadowOAM[oamIndex].attr0 = ATTR0_Y(y) | ATTR0_REGULAR | ATTR0_TALL | ATTR0_4BPP;
    shadowOAM[oamIndex].attr1 = ATTR1_X(x) | ATTR1_MEDIUM;
    shadowOAM[oamIndex].attr2 = ATTR2_TILEID_1D(tileStart) | ATTR2_PRIORITY(0) | ATTR2_PALROW(palIndex);
}

static void setTallSpriteFlip(int oamIndex, int x, int y, int tileStart, int palIndex, int hflip) {
    shadowOAM[oamIndex].attr0 = ATTR0_Y(y) | ATTR0_REGULAR | ATTR0_TALL | ATTR0_4BPP;
    shadowOAM[oamIndex].attr1 = ATTR1_X(x) | ATTR1_MEDIUM | (hflip ? ATTR1_HFLIP : 0);
    shadowOAM[oamIndex].attr2 = ATTR2_TILEID_1D(tileStart) | ATTR2_PRIORITY(0) | ATTR2_PALROW(palIndex);
}

static void setBossSprite(int oamIndex, int x, int y, int tileStart, int palIndex) {
    shadowOAM[oamIndex].attr0 = ATTR0_Y(y) | ATTR0_REGULAR | ATTR0_TALL | ATTR0_4BPP;
    shadowOAM[oamIndex].attr1 = ATTR1_X(x) | ATTR1_LARGE;
    shadowOAM[oamIndex].attr2 = ATTR2_TILEID_1D(tileStart) | ATTR2_PRIORITY(0) | ATTR2_PALROW(palIndex);
}

static void hideSpriteIndex(int oamIndex) {
    shadowOAM[oamIndex].attr0 = ATTR0_HIDE;
}

static int enemyPal(const Enemy *e) {
    if (e->type == ET_BOSS) {
        return 1;
    }
    return 5;
}

static int enemyTile(const Enemy *e) {
    if (e->type == ET_BOSS) {
        return 6;
    }
    if (e->type == ET_SHOOT) {
        int frame = (gameTick / 30) & 1;
        return frame ? PLANT_CLOSED_TILE : PLANT_OPEN_TILE;
    }
    int sframe = ((gameTick + e->x) / 18) & 1;
    return sframe ? SLIME_FRAME1_TILE : SLIME_FRAME0_TILE;
}

static void drawSprites(void) {
    hideSprites();
    int px = playerX - cameraX;
    int py = playerY - cameraY;

    int renderDir = playerDir;
    if (BUTTON_HELD(BUTTON_LEFT)) {
        renderDir = 1;
    } else if (BUTTON_HELD(BUTTON_RIGHT)) {
        renderDir = 0;
    } else if (BUTTON_HELD(BUTTON_UP)) {
        renderDir = 2;
    } else if (BUTTON_HELD(BUTTON_DOWN)) {
        renderDir = 3;
    }

    int movingFrame = (playerAnimFrame & 1);
    int playerTile = PLAYER_DOWN0_TILE;
    int playerFlip = 0;
    int faceDir = (attackTimer > 0) ? attackFacing : renderDir;
    if (faceDir == 0) {
        playerTile = movingFrame ? PLAYER_LEFT1_TILE : PLAYER_LEFT0_TILE;
        playerFlip = 1;
    } else if (faceDir == 1) {
        playerTile = movingFrame ? PLAYER_LEFT1_TILE : PLAYER_LEFT0_TILE;
        playerFlip = 0;
    } else if (faceDir == 2) {
        playerTile = movingFrame ? PLAYER_UP1_TILE : PLAYER_UP0_TILE;
    } else {
        playerTile = movingFrame ? PLAYER_DOWN1_TILE : PLAYER_DOWN0_TILE;
    }
    setTallSpriteFlip(OAM_PLAYER, px, py - 16, playerTile, 1, playerFlip);

    for (int i = 0; i < MAX_ENEMIES; i++) {
        int oam = OAM_ENEMY_BASE + i;
        Enemy *e = &enemies[i];
        if (e->active && e->hp > 0 && e->roomId == roomId) {
            if (e->type == ET_BOSS) {
                int hurtFlash = (e->hurtTimer > 0 && (e->hurtTimer & 1));
                if (hurtFlash) {
                    hideSpriteIndex(oam);
                } else {
                    int bx = e->x - cameraX - 8;
                    int by = e->y - cameraY - 32;
                    setBossSprite(oam, bx, by, BOSS_FRAME_TILE, 7);
                }
            } else {
                int pal = enemyPal(e);
                if (e->hurtTimer > 0 && (e->hurtTimer & 1)) {
                    pal = 2;
                }
                setRegularSprite(oam, e->x - cameraX, e->y - cameraY, enemyTile(e), pal);
            }
        } else {
            hideSpriteIndex(oam);
        }
    }

    for (int i = 0; i < MAX_PROJ; i++) {
        int oam = OAM_PROJ_BASE + i;
        if (projs[i].active) {
            int spinFrame = ((gameTick + i * 3) >> 2) & 1;
            int projTile;
            if (projs[i].owner == 0) {
                projTile = spinFrame ? PROJ_PLAYER1_TILE : PROJ_PLAYER0_TILE;
            } else if (projs[i].kind == 1) {
                projTile = spinFrame ? PROJ_RED1_TILE : PROJ_RED0_TILE;
            } else if (projs[i].kind == 2) {
                projTile = spinFrame ? PROJ_BOSS1_TILE : PROJ_BOSS0_TILE;
            } else {
                projTile = spinFrame ? PROJ_SEED1_TILE : PROJ_SEED0_TILE;
            }
            setTinySprite(oam, projs[i].x - cameraX, projs[i].y - cameraY, projTile, 5);
        } else {
            hideSpriteIndex(oam);
        }
    }

    if (attackTimer > 0) {
        int swingFrame = (24 - attackTimer) / 8;
        if (swingFrame < 0) swingFrame = 0;
        if (swingFrame > 2) swingFrame = 2;
        int sx = px;
        int sy = py - 8;
        int baseTile = PLAYER_SWORD_R0_TILE;
        if (attackFacing == 0) {
            baseTile = PLAYER_SWORD_R0_TILE;
            sx = px + 14;
            sy = py - 8 + (swingFrame - 1) * 3;
        } else if (attackFacing == 1) {
            baseTile = PLAYER_SWORD_L0_TILE;
            sx = px - 14;
            sy = py - 8 + (swingFrame - 1) * 3;
        } else if (attackFacing == 2) {
            baseTile = PLAYER_SWORD_U0_TILE;
            sx = px + (swingFrame - 1) * 3;
            sy = py - 26;
        } else {
            baseTile = PLAYER_SWORD_D0_TILE;
            sx = px - (swingFrame - 1) * 3;
            sy = py + 8;
        }
        int swordTile = baseTile + swingFrame * 4;
        setRegularSprite(OAM_SLASH, sx, sy, swordTile, 1);
    } else {
        hideSpriteIndex(OAM_SLASH);
    }

    for (int i = 0; i < 8; i++) {
        if (i < playerMaxHp) {
            int heartTile = (playerHp > i) ? HEART_FULL_TILE : HEART_EMPTY_TILE;
            setRegularSprite(OAM_HP0 + i, 4 + i * 18, 2, heartTile, 3);
        } else {
            hideSpriteIndex(OAM_HP0 + i);
        }
    }

    for (int i = 0; i < MAX_XP_ORBS; i++) {
        if (xpOrbs[i].active && xpOrbs[i].roomId == roomId) {
            int bob = (xpOrbs[i].bob / 6) & 1;
            int phase = ((gameTick + i * 5) / 8) % 4;
            int orbTile = XP_ORB1_TILE;
            if (phase == 0) orbTile = XP_ORB0_TILE;
            else if (phase == 1) orbTile = XP_ORB1_TILE;
            else if (phase == 2) orbTile = XP_ORB2_TILE;
            else orbTile = XP_ORB1_TILE;
            setTinySprite(OAM_XP_ORB0 + i, xpOrbs[i].x - cameraX, xpOrbs[i].y - cameraY - bob, orbTile, 5);
        } else {
            hideSpriteIndex(OAM_XP_ORB0 + i);
        }
    }

    for (int i = 0; i < BUBBLE_COUNT; i++) {
        if (bubbles[i].active) {
            int bphase = ((gameTick + i * 7) / 6) % 3;
            int btile = (bphase == 0) ? XP_ORB0_TILE : ((bphase == 1) ? XP_ORB1_TILE : XP_ORB2_TILE);
            setTinySprite(OAM_BUBBLE0 + i, bubbles[i].x - cameraX, bubbles[i].y - cameraY, btile, 6);
        } else {
            hideSpriteIndex(OAM_BUBBLE0 + i);
        }
    }

    drawHudBars();
}

static void drawHudBars(void) {
    int i;
    int bossAlive = 0;
    Enemy *boss = 0;
    for (i = 0; i < MAX_ENEMIES; i++) {
        if (enemies[i].active && enemies[i].type == ET_BOSS && enemies[i].hp > 0) {
            boss = &enemies[i];
            bossAlive = 1;
            break;
        }
    }
    if (roomId == 7 && bossAlive && boss) {
        int filled = (boss->hp * BOSS_BAR_SEGS + BOSS_MAX_HP - 1) / BOSS_MAX_HP;
        if (filled < 0) {
            filled = 0;
        }
        if (filled > BOSS_BAR_SEGS) {
            filled = BOSS_BAR_SEGS;
        }
        for (i = 0; i < BOSS_BAR_SEGS; i++) {
            if (i < filled) {
                setTinySprite(OAM_BOSS_BAR0 + i, 56 + i * 4, 146, 11, 2);
            } else {
                hideSpriteIndex(OAM_BOSS_BAR0 + i);
            }
        }
    } else {
        for (i = 0; i < BOSS_BAR_SEGS; i++) {
            hideSpriteIndex(OAM_BOSS_BAR0 + i);
        }
    }

    {
        int rem = playerXp % XP_PER_SKILL;
        int xpFill = (rem * XP_BAR_SEGS) / XP_PER_SKILL;
        if (xpFill < 0) {
            xpFill = 0;
        }
        if (xpFill > XP_BAR_SEGS) {
            xpFill = XP_BAR_SEGS;
        }
        for (i = 0; i < XP_BAR_SEGS; i++) {
            if (i < xpFill) {
                setTinySprite(OAM_XP_BAR0 + i, 8 + i * 4, 18, 11, 4);
            } else {
                hideSpriteIndex(OAM_XP_BAR0 + i);
            }
        }
    }

    for (i = 0; i < SP_SEGS; i++) {
        if (i < skillPoints && i < 8) {
            setTinySprite(OAM_SP0 + i, 52 + i * 5, 18, 8, 1);
        } else {
            hideSpriteIndex(OAM_SP0 + i);
        }
    }

    for (i = 0; i < 3 * STAT_SEGS; i++) {
        hideSpriteIndex(OAM_STAT_STR0 + i);
    }

    {
        int lvl = currentFloor;
        if (lvl < 0) {
            lvl = 0;
        }
        if (lvl > 99) {
            lvl = 99;
        }
        int tens = lvl / 10;
        int ones = lvl % 10;
        hideSpriteIndex(OAM_LEVEL_TENS);
        hideSpriteIndex(OAM_LEVEL_ONES);
    }
}

static void syncBgLayers(void) {
    if (roomId == 7) {
        REG_DISPCTL = MODE(0) | BG_ENABLE(0) | SPRITE_ENABLE | SPRITE_MODE_1D;
    } else {
        REG_DISPCTL = MODE(0) | BG_ENABLE(0) | BG_ENABLE(1) | SPRITE_ENABLE | SPRITE_MODE_1D;
    }
}

static void unlockBossRoomWestDoor(void) {
    bossWestDoorLocked = 0;
    if (roomId == 7) {
        buildRoomIntoRam(7);
        copyMapToVRAM();
    }
}

static void onBossDefeated(void) {
    bossDefeated = 1;
    unlockBossRoomWestDoor();
    if (roomId == 7) {
        buildRoomIntoRam(7);
        copyMapToVRAM();
    }
    playAnalogSound(16);
}

static void damagePlayerOnce(void) {
    if (invincibleTimer == 0 && playerHp > 0) {
        playerHp--;
        invincibleTimer = (roomId == 7) ? 80 : 50;
        if (relaxMode) {
            invincibleTimer += invincibleTimer >> 1;
        }
        playAnalogSound(playerHp <= 0 ? 17 : 5);
    }
}

static void applyHitFeedback(Enemy *e, int fromX, int fromY) {
    int dx = e->x - fromX;
    int dy = e->y - fromY;
    e->hurtTimer = 10;
    e->knockX = (dx >= 0) ? ((e->type == ET_BOSS) ? 1 : 2) : ((e->type == ET_BOSS) ? -1 : -2);
    e->knockY = (dy >= 0) ? ((e->type == ET_BOSS) ? 1 : 2) : ((e->type == ET_BOSS) ? -1 : -2);
}

static int getSwordHitbox(int *hx, int *hy, int *hw, int *hh) {
    if (attackTimer <= 0) {
        return 0;
    }
    *hx = playerX;
    *hy = playerY;
    *hw = 16;
    *hh = 16;
    switch (attackFacing) {
        case 0:
            *hx = playerX + 8;
            *hy = playerY + 2;
            *hw = 26;
            *hh = 14;
            break;
        case 1:
            *hx = playerX - 18;
            *hy = playerY + 2;
            *hw = 26;
            *hh = 14;
            break;
        case 2:
            *hx = playerX + 2;
            *hy = playerY - 18;
            *hw = 14;
            *hh = 26;
            break;
        case 3:
            *hx = playerX + 2;
            *hy = playerY + 8;
            *hw = 14;
            *hh = 26;
            break;
    }
    return 1;
}

static void updateEnemies(void) {
    for (int i = 0; i < MAX_ENEMIES; i++) {
        Enemy *e = &enemies[i];
        if (!e->active || e->hp <= 0 || e->roomId != roomId) {
            continue;
        }
        if (e->hurtTimer > 0) {
            e->hurtTimer--;
            if ((gameTick & 1) == 0) {
                tryMoveEnemy(e, e->knockX, e->knockY);
            }
        }

        if (e->attackCd > 0 && e->type != ET_BOSS) {
            e->attackCd--;
        }

        if (e->type == ET_BOSS) {
            updateBossParryFight(e);
        } else if (e->type == ET_PATROL) {
            if ((gameTick % ENEMY_PATROL_MOVE_PERIOD) == 0) {
                tryMoveEnemy(e, e->vx, e->vy);
            }
        } else if (e->type == ET_SHOOT) {
            if ((gameTick % ENEMY_PATROL_MOVE_PERIOD) == 0) {
                tryMoveEnemy(e, e->vx, e->vy);
            }
            enemyTryShoot(e);
        } else if (e->type == ET_BRUISER) {
            int sp = 1;
            if ((gameTick % ENEMY_BRUISER_MOVE_PERIOD) == 0) {
                if (playerX > e->x + 2) {
                    tryMoveEnemy(e, sp, 0);
                } else if (playerX < e->x - 2) {
                    tryMoveEnemy(e, -sp, 0);
                }
                if (playerY > e->y + 2) {
                    tryMoveEnemy(e, 0, sp);
                } else if (playerY < e->y - 2) {
                    tryMoveEnemy(e, 0, -sp);
                }
            }

            int dx = playerX - e->x;
            int dy = playerY - e->y;
            int d2 = dx * dx + dy * dy;
            if (e->swingRemain == 0 && d2 < 38 * 38 && e->attackCd == 0) {
                e->swingRemain = 22;
                e->swingDealt = 0;
            }
            if (e->swingRemain > 0) {
                e->swingRemain--;
                if (!e->swingDealt && e->swingRemain == 10) {
                    int hx = e->x - 8;
                    int hy = e->y - 8;
                    int hw = 32;
                    int hh = 32;
                    int adx = dx;
                    int ady = dy;
                    if (adx * adx > ady * ady) {
                        hy = e->y + 2;
                        hh = 12;
                        if (adx > 0) {
                            hx = e->x + 8;
                            hw = 28;
                        } else {
                            hx = e->x - 20;
                            hw = 28;
                        }
                    } else {
                        hx = e->x + 2;
                        hw = 12;
                        if (ady > 0) {
                            hy = e->y + 8;
                            hh = 28;
                        } else {
                            hy = e->y - 20;
                            hh = 28;
                        }
                    }
                    if (collision(playerX, playerY, PLAYER_SIZE, PLAYER_SIZE, hx, hy, hw, hh)) {
                        damagePlayerOnce();
                    }
                    e->swingDealt = 1;
                }
                if (e->swingRemain == 0) {
                    e->attackCd = 75;
                }
            }
        }

        if (invincibleTimer == 0 &&
            collision(playerX, playerY, PLAYER_SIZE, PLAYER_SIZE, e->x, e->y, PLAYER_SIZE,
                      PLAYER_SIZE)) {
            int shx;
            int shy;
            int shw;
            int shh;
            if (getSwordHitbox(&shx, &shy, &shw, &shh) &&
                collision(shx, shy, shw, shh, e->x, e->y, PLAYER_SIZE, PLAYER_SIZE)) {
                continue;
            }
            if (e->type != ET_BOSS) {
                damagePlayerOnce();
            }
        }
    }
}

static void updateBossParryFight(Enemy *e) {
    if (e->shootCd > 0) {
        e->shootCd--;
    }
    bossPatternTimer++;

    if ((bossPatternTimer & 7) == 0) {
        int wob = ((bossPatternTimer >> 3) & 1) ? 1 : -1;
        tryMoveEnemy(e, wob, 0);
    }

    if (e->shootCd > 0) {
        return;
    }

    int phase = 0;
    if (e->hp * 100 <= BOSS_MAX_HP * 66) {
        phase = 1;
    }
    if (e->hp * 100 <= BOSS_MAX_HP * 33) {
        phase = 2;
    }

    int pcx = playerX + 8;
    int pcy = playerY + 8;
    int ecx = e->x + 8;
    int ecy = e->y + 8;
    int dx = pcx - ecx;
    int dy = pcy - ecy;
    int ax = (dx >= 0) ? 2 : -2;
    int ay = (dy >= 0) ? 2 : -2;
    bossShotsSinceParry++;
    {
        int pattern = (bossPatternTimer / 96) % 4;
        if (currentFloor == 1) {
            if (pattern < 2) {
                spawnProjectileKind(e->x + 6, e->y + 6, ax, 0, 0, 1);
                spawnProjectileKind(e->x + 6, e->y + 6, ax, ay, 0, 1);
                spawnProjectileKind(e->x + 6, e->y + 6, ax, -ay, 0, 1);
            } else {
                static const int rvx[8] = {2, 2, 0, -2, -2, -2, 0, 2};
                static const int rvy[8] = {0, 2, 2, 2, 0, -2, -2, -2};
                for (int k = 0; k < 8; k++) {
                    if (phase == 0 && (k == 1 || k == 5)) {
                        continue;
                    }
                    spawnProjectileKind(e->x + 6, e->y + 6, rvx[k], rvy[k], 0, 1);
                }
            }
        } else if (currentFloor == 2) {
            if (pattern == 0) {
                static const int rvx[8] = {2, 2, 0, -2, -2, -2, 0, 2};
                static const int rvy[8] = {0, 2, 2, 2, 0, -2, -2, -2};
                for (int k = 0; k < 8; k++) {
                    spawnProjectileKind(e->x + 6, e->y + 6, rvx[k], rvy[k], 1, 1);
                }
            } else {
                spawnProjectileKind(e->x + 6, e->y + 6, 2, 0, 1, 1);
                spawnProjectileKind(e->x + 6, e->y + 6, -2, 0, 1, 1);
                spawnProjectileKind(e->x + 6, e->y + 6, 0, 2, 1, 1);
                spawnProjectileKind(e->x + 6, e->y + 6, 0, -2, 1, 1);
            }
        } else if (currentFloor == 3) {
            spawnProjectileKind(e->x + 6, e->y + 6, ax, 1, 2, 1);
            spawnProjectileKind(e->x + 6, e->y + 6, ax, -1, 2, 1);
            if (bossShotsSinceParry % 2 == 0) {
                spawnProjectileKind(e->x + 6, e->y + 6, ax, 0, 2, 1);
            }
            if (phase >= 1) {
                spawnProjectileKind(e->x + 6, e->y + 6, (ax > 0) ? 1 : -1, (ay > 0) ? 1 : -1, 2, 1);
            }
        } else if (currentFloor == 4) {
            bossSpinAngle = (bossSpinAngle + 1 + phase) & 7;
            static const int svx[8] = {2, 2, 0, -2, -2, -2, 0, 2};
            static const int svy[8] = {0, 2, 2, 2, 0, -2, -2, -2};
            spawnProjectileKind(e->x + 6, e->y + 6, svx[bossSpinAngle], svy[bossSpinAngle], 3, 1);
            spawnProjectileKind(e->x + 6, e->y + 6, svx[(bossSpinAngle + 3) & 7], svy[(bossSpinAngle + 3) & 7], 3, 1);
            if (phase >= 1) {
                spawnProjectileKind(e->x + 6, e->y + 6, svx[(bossSpinAngle + 5) & 7], svy[(bossSpinAngle + 5) & 7], 3, 1);
            }
        } else {
            static const int rvx[8] = {2, 2, 0, -2, -2, -2, 0, 2};
            static const int rvy[8] = {0, 2, 2, 2, 0, -2, -2, -2};
            bossSpinAngle = (bossSpinAngle + 2 + phase) & 7;
            spawnProjectileKind(e->x + 6, e->y + 6, ax, 0, 1, 1);
            spawnProjectileKind(e->x + 6, e->y + 6, ax, ay, 0, 1);
            spawnProjectileKind(e->x + 6, e->y + 6, rvx[bossSpinAngle], rvy[bossSpinAngle], 3, 1);
            spawnProjectileKind(e->x + 6, e->y + 6, rvx[(bossSpinAngle + 4) & 7], rvy[(bossSpinAngle + 4) & 7], 3, 1);
            if (phase >= 1) {
                spawnProjectileKind(e->x + 6, e->y + 6, 0, (dy >= 0) ? 2 : -2, 2, 1);
            }
            if (phase >= 2) {
                spawnProjectileKind(e->x + 6, e->y + 6, -ax, ay, 1, 1);
            }
        }
    }
    e->shootCd = 19 - phase * 2 - currentFloor;
    if (e->shootCd < 7) {
        e->shootCd = 7;
    }
}

static void updateProjectiles(void) {
    for (int i = 0; i < MAX_PROJ; i++) {
        Projectile *p = &projs[i];
        if (!p->active) {
            continue;
        }
        if ((gameTick % PROJ_MOVE_PERIOD) == 0) {
            p->x += p->vx;
            p->y += p->vy;
        }
        p->ttl--;
        if (p->ttl <= 0 || collidesWallRect(p->x, p->y, PROJ_SZ, PROJ_SZ)) {
            p->active = 0;
            continue;
        }
        if (invincibleTimer == 0 &&
            collision(playerX, playerY, PLAYER_SIZE, PLAYER_SIZE, p->x, p->y, PROJ_SZ, PROJ_SZ)) {
            damagePlayerOnce();
            p->active = 0;
        }
    }
}

int gameUpdate(void) {
    gameTick++;
    if (BUTTON_HELD(BUTTON_UP) || BUTTON_HELD(BUTTON_DOWN) || BUTTON_HELD(BUTTON_LEFT) ||
        BUTTON_HELD(BUTTON_RIGHT)) {
        playerAnimFrame = (gameTick / 16) & 1;
    } else {
        playerAnimFrame = 0;
    }
    patrolAnimFrame = (gameTick / 18) & 1;
    mutateRuntimeAssets();
    syncBgLayers();

    if (roomId == 3 || roomId == 5) {
        for (int i = 0; i < BUBBLE_COUNT; i++) {
            if (bubbles[i].active) {
                if ((gameTick & 1) == 0) bubbles[i].y--;
                bubbles[i].ttl--;
                if (bubbles[i].ttl <= 0) bubbles[i].active = 0;
            }
        }
        if ((gameTick % 6) == 0) {
            for (int i = 0; i < BUBBLE_COUNT; i++) {
                if (!bubbles[i].active) {
                    bubbleRng = bubbleRng * 1103515245u + 12345u;
                    int rx = (bubbleRng >> 8) % 56;
                    bubbleRng = bubbleRng * 1103515245u + 12345u;
                    int ry = (bubbleRng >> 8) % 24;
                    bubbles[i].active = 1;
                    bubbles[i].x = 12 * 8 + rx;
                    bubbles[i].y = 14 * 8 + ry;
                    bubbles[i].vy = -1;
                    bubbles[i].ttl = 50 + ((bubbleRng >> 8) & 31);
                    break;
                }
            }
        }
    } else {
        for (int i = 0; i < BUBBLE_COUNT; i++) bubbles[i].active = 0;
    }

    if (roomTransitionCooldown > 0) {
        roomTransitionCooldown--;
    }

    if (invincibleTimer > 0) {
        invincibleTimer--;
    }

    int ms = moveSpeed();
    if (BUTTON_HELD(BUTTON_UP)) {
        tryMoveAxis(&playerX, &playerY, 0, -ms);
        playerDir = 2;
    } else if (BUTTON_HELD(BUTTON_DOWN)) {
        tryMoveAxis(&playerX, &playerY, 0, ms);
        playerDir = 3;
    } else if (BUTTON_HELD(BUTTON_LEFT)) {
        tryMoveAxis(&playerX, &playerY, -ms, 0);
        playerDir = 1;
    } else if (BUTTON_HELD(BUTTON_RIGHT)) {
        tryMoveAxis(&playerX, &playerY, ms, 0);
        playerDir = 0;
    }

    checkRoomTransition();

    if (BUTTON_PRESSED(BUTTON_A) && attackTimer == 0) {
        attackTimer = 24;
        attackDealtDamage = 0;
        attackFacing = playerDir;
        playAnalogSound(7);
    }

    if (attackTimer > 0) {
        attackTimer--;
    }

    updateEnemies();
    updateProjectiles();
    updateXpOrbs();

    if (inHealPool() && playerHp < playerMaxHp) {
        healPoolTimer++;
        if (healPoolTimer >= 72) {
            healPoolTimer = 0;
            playerHp++;
            playAnalogSound(13);
        }
    } else {
        healPoolTimer = 0;
    }

    if (attackTimer > 0 && !attackDealtDamage) {
        int hx;
        int hy;
        int hw;
        int hh;
        if (!getSwordHitbox(&hx, &hy, &hw, &hh)) {
            hx = playerX;
            hy = playerY;
            hw = 16;
            hh = 16;
        }
        int dmg = swordDamage();
        {
            int ssx;
            int ssy;
            int ssw;
            int ssh;
            if (inSkillStationWorld(&ssx, &ssy, &ssw, &ssh) &&
                collision(hx, hy, hw, hh, ssx, ssy, ssw, ssh)) {
                skillMenuOpenRequest = 1;
                attackDealtDamage = 1;
            } else {
                int i;
                for (i = 0; i < MAX_ENEMIES; i++) {
                    Enemy *e = &enemies[i];
                    if (!e->active || e->hp <= 0 || e->roomId != roomId) {
                        continue;
                    }
                    int ehx = e->x;
                    int ehy = e->y;
                    int ehw = PLAYER_SIZE;
                    int ehh = PLAYER_SIZE;
                    if (e->type == ET_BOSS) {
                        ehx = e->x - 8;
                        ehy = e->y - 24;
                        ehw = 32;
                        ehh = 48;
                    }
                    if (collision(hx, hy, hw, hh, ehx, ehy, ehw, ehh)) {
                        e->hp -= dmg;
                        playAnalogSound(e->type == ET_BOSS ? 0 : 10);
                        applyHitFeedback(e, playerX, playerY);
                        if (e->hp <= 0) {
                            enemiesAlive--;
                            dropXpOrbs(e->x, e->y, e->roomId, e->type);
                            if (e->type == ET_BOSS) {
                                onBossDefeated();
                            }
                        }
                        attackDealtDamage = 1;
                        break;
                    }
                }
            }
        }
    }

    if (playerHp <= 0) {
        return 2;
    }
    if (roomId == 7 && bossDefeated) {
        int cx = playerX + PLAYER_SIZE / 2;
        int cy = playerY + PLAYER_SIZE / 2;
        if (inEastDoorBand(cx, cy)) {
            return 3;
        }
    }
    if (enemiesAlive <= 0) {
        if (bossDefeated) {
        } else {
            return 1;
        }
    }

    updateCamera();
    drawSprites();
    updateOAM();
    return 0;
}
