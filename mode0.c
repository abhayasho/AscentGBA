#include "mode0.h"
#include "sprites.h"
#include "gba.h"

#define SCREENBLOCK_MAP 28
#define CHARBLOCK_TILES 0

#define TILE_FLOOR 0
#define TILE_WALL  1
#define TILE_FLOOR2 2

#define OAM_PLAYER 0
#define OAM_ENEMY  1
#define OAM_SLASH  2
#define OAM_HP0    10
#define PLAYER_MAX_HP 5

#define PLAYER_SIZE 16
#define MOVE_SPEED  2

static int cameraX;
static int cameraY;

static const u16 SOLID_TILE_COLOR1[16] = {
    0x1111, 0x1111, 0x1111, 0x1111, 0x1111, 0x1111, 0x1111, 0x1111,
    0x1111, 0x1111, 0x1111, 0x1111, 0x1111, 0x1111, 0x1111, 0x1111,
};
static const u16 SOLID_TILE_COLOR2[16] = {
    0x2222, 0x2222, 0x2222, 0x2222, 0x2222, 0x2222, 0x2222, 0x2222,
    0x2222, 0x2222, 0x2222, 0x2222, 0x2222, 0x2222, 0x2222, 0x2222,
};
static const u16 SOLID_TILE_COLOR3[16] = {
    0x3333, 0x3333, 0x3333, 0x3333, 0x3333, 0x3333, 0x3333, 0x3333,
    0x3333, 0x3333, 0x3333, 0x3333, 0x3333, 0x3333, 0x3333, 0x3333,
};
static const u16 SOLID_TILE_FLOOR_ALT[16] = {
    0x4444, 0x4444, 0x4444, 0x4444, 0x4444, 0x4444, 0x4444, 0x4444,
    0x4444, 0x4444, 0x4444, 0x4444, 0x4444, 0x4444, 0x4444, 0x4444,
};

static u16 ramMap[1024];

static int roomId;
static int playerX;
static int playerY;
static int playerDir;
static int playerHp;
static int invincibleTimer;

static int enemyX;
static int enemyY;
static int enemyHp;
static int enemyActive;

static int attackTimer;
static int attackDealtDamage;
static int attackFacing;

static void drawSprites(void);

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
}

static void loadObjTiles(void) {
    volatile u16 *obj = (volatile u16 *)0x06010000;
    copy16TileTo(obj + 0 * 16, SOLID_TILE_COLOR1);
    copy16TileTo(obj + 1 * 16, SOLID_TILE_COLOR1);
    copy16TileTo(obj + 2 * 16, SOLID_TILE_COLOR1);
    copy16TileTo(obj + 3 * 16, SOLID_TILE_COLOR1);
    copy16TileTo(obj + 4 * 16, SOLID_TILE_COLOR2);
    copy16TileTo(obj + 5 * 16, SOLID_TILE_COLOR2);
    copy16TileTo(obj + 6 * 16, SOLID_TILE_COLOR2);
    copy16TileTo(obj + 7 * 16, SOLID_TILE_COLOR2);
    copy16TileTo(obj + 8 * 16, SOLID_TILE_COLOR3);
    copy16TileTo(obj + 9 * 16, SOLID_TILE_COLOR3);
    copy16TileTo(obj + 10 * 16, SOLID_TILE_COLOR3);
    copy16TileTo(obj + 11 * 16, SOLID_TILE_COLOR3);
}

static void loadPalettes(void) {
    BG_PALETTE[0] = RGB(0, 0, 0);
    BG_PALETTE[1] = RGB(8, 6, 4);
    BG_PALETTE[2] = RGB(10, 10, 12);
    BG_PALETTE[4] = RGB(5, 4, 6);

    SPRITE_PAL[0] = 0;
    SPRITE_PAL[1] = RGB(4, 22, 8);
    SPRITE_PAL[2] = RGB(28, 6, 6);
    SPRITE_PAL[3] = RGB(28, 28, 12);
}

static void buildRoomIntoRam(int id) {
    int floorTile = (id == 2) ? TILE_FLOOR2 : TILE_FLOOR;
    for (int i = 0; i < 1024; i++) {
        ramMap[i] = TILEMAP_ENTRY_TILEID(floorTile);
    }
    for (int x = 0; x < 32; x++) {
        ramMap[x] = TILEMAP_ENTRY_TILEID(TILE_WALL);
        ramMap[31 * 32 + x] = TILEMAP_ENTRY_TILEID(TILE_WALL);
    }
    for (int y = 0; y < 32; y++) {
        ramMap[y * 32] = TILEMAP_ENTRY_TILEID(TILE_WALL);
        ramMap[y * 32 + 31] = TILEMAP_ENTRY_TILEID(TILE_WALL);
    }
    if (id == 0) {
        for (int y = 12; y < 20; y++) {
            ramMap[y * 32 + 31] = TILEMAP_ENTRY_TILEID(floorTile);
        }
    }
    if (id == 1) {
        int f = TILE_FLOOR;
        for (int y = 12; y < 20; y++) {
            ramMap[y * 32 + 0] = TILEMAP_ENTRY_TILEID(f);
            ramMap[y * 32 + 31] = TILEMAP_ENTRY_TILEID(f);
        }
    }
    if (id == 2) {
        for (int y = 12; y < 20; y++) {
            ramMap[y * 32 + 0] = TILEMAP_ENTRY_TILEID(TILE_FLOOR2);
        }
    }
}

static void copyMapToVRAM(void) {
    volatile u16 *screenMap = (volatile u16 *)(0x06000000 + SCREENBLOCK_MAP * 2048);
    for (int i = 0; i < 1024; i++) {
        screenMap[i] = ramMap[i];
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
    return t == TILE_WALL;
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

static void checkRoomTransition(void) {
    int cx = playerX + PLAYER_SIZE / 2;
    int cy = playerY + PLAYER_SIZE / 2;
    if (roomId == 0) {
        if (cx >= 31 * 8 - 6 && cy >= 12 * 8 && cy < 20 * 8) {
            roomId = 1;
            playerX = 2 * 8;
            playerY = cy - PLAYER_SIZE / 2;
            buildRoomIntoRam(roomId);
            copyMapToVRAM();
        }
    } else if (roomId == 1) {
        if (cx <= 2 * 8 + 4 && cy >= 12 * 8 && cy < 20 * 8) {
            roomId = 0;
            playerX = 29 * 8;
            playerY = cy - PLAYER_SIZE / 2;
            buildRoomIntoRam(roomId);
            copyMapToVRAM();
        } else if (cx >= 31 * 8 - 6 && cy >= 12 * 8 && cy < 20 * 8) {
            roomId = 2;
            playerX = 2 * 8;
            playerY = cy - PLAYER_SIZE / 2;
            buildRoomIntoRam(roomId);
            copyMapToVRAM();
        }
    } else if (roomId == 2) {
        if (cx <= 2 * 8 + 4 && cy >= 12 * 8 && cy < 20 * 8) {
            roomId = 1;
            playerX = 29 * 8;
            playerY = cy - PLAYER_SIZE / 2;
            buildRoomIntoRam(roomId);
            copyMapToVRAM();
        }
    }
}

void gameReset(void) {
    roomId = 0;
    playerX = 3 * 8;
    playerY = 10 * 8;
    playerDir = 0;
    playerHp = 5;
    invincibleTimer = 0;
    enemyX = 20 * 8;
    enemyY = 10 * 8;
    enemyHp = 4;
    enemyActive = 1;
    attackTimer = 0;
    attackDealtDamage = 0;
    attackFacing = 0;
    buildRoomIntoRam(roomId);
}

void gameRenderInit(void) {
    REG_DISPCTL = MODE(0) | BG_ENABLE(0) | SPRITE_ENABLE | SPRITE_MODE_1D;
    loadPalettes();
    loadCharBlockTiles();
    loadObjTiles();
    copyMapToVRAM();

    REG_BG0CNT = BG_CHARBLOCK(CHARBLOCK_TILES) | BG_SCREENBLOCK(SCREENBLOCK_MAP) | BG_SIZE_SMALL |
                 BG_4BPP;
    updateCamera();

    drawSprites();
    updateOAM();
}

static void setRegularSprite(int oamIndex, int x, int y, int tileStart, int palIndex) {
    shadowOAM[oamIndex].attr0 = ATTR0_Y(y) | ATTR0_REGULAR | ATTR0_SQUARE | ATTR0_4BPP;
    shadowOAM[oamIndex].attr1 = ATTR1_X(x) | ATTR1_SMALL;
    shadowOAM[oamIndex].attr2 = ATTR2_TILEID_1D(tileStart) | ATTR2_PRIORITY(0) | ATTR2_PALROW(palIndex);
}

static void hideSpriteIndex(int oamIndex) {
    shadowOAM[oamIndex].attr0 = ATTR0_HIDE;
}

static void drawSprites(void) {
    int px = playerX - cameraX;
    int py = playerY - cameraY;

    setRegularSprite(OAM_PLAYER, px, py, 0, 0);

    if (enemyActive && enemyHp > 0 && roomId == 0) {
        setRegularSprite(OAM_ENEMY, enemyX - cameraX, enemyY - cameraY, 4, 0);
    } else {
        hideSpriteIndex(OAM_ENEMY);
    }

    if (attackTimer > 0) {
        int sx = px;
        int sy = py;
        switch (attackFacing) {
            case 0:
                sx = px + 16;
                sy = py;
                break;
            case 1:
                sx = px - 16;
                sy = py;
                break;
            case 2:
                sx = px;
                sy = py - 16;
                break;
            case 3:
                sx = px;
                sy = py + 16;
                break;
        }
        setRegularSprite(OAM_SLASH, sx, sy, 8, 0);
    } else {
        hideSpriteIndex(OAM_SLASH);
    }

    for (int i = 0; i < PLAYER_MAX_HP; i++) {
        if (playerHp > i) {
            setTinySprite(OAM_HP0 + i, 4 + i * 10, 4, 0, 0);
        } else {
            hideSpriteIndex(OAM_HP0 + i);
        }
    }
}

int gameUpdate(void) {
    if (invincibleTimer > 0) {
        invincibleTimer--;
    }

    if (BUTTON_HELD(BUTTON_UP)) {
        tryMoveAxis(&playerX, &playerY, 0, -MOVE_SPEED);
        playerDir = 2;
    } else if (BUTTON_HELD(BUTTON_DOWN)) {
        tryMoveAxis(&playerX, &playerY, 0, MOVE_SPEED);
        playerDir = 3;
    } else if (BUTTON_HELD(BUTTON_LEFT)) {
        tryMoveAxis(&playerX, &playerY, -MOVE_SPEED, 0);
        playerDir = 1;
    } else if (BUTTON_HELD(BUTTON_RIGHT)) {
        tryMoveAxis(&playerX, &playerY, MOVE_SPEED, 0);
        playerDir = 0;
    }

    checkRoomTransition();

    if (BUTTON_PRESSED(BUTTON_A) && attackTimer == 0) {
        attackTimer = 14;
        attackDealtDamage = 0;
        attackFacing = playerDir;
    }

    if (attackTimer > 0) {
        attackTimer--;
    }

    if (enemyActive && enemyHp > 0 && roomId == 0 && attackTimer > 0 && !attackDealtDamage) {
        int hx = playerX;
        int hy = playerY;
        int hw = 16;
        int hh = 16;
        switch (attackFacing) {
            case 0:
                hx = playerX + 12;
                hy = playerY + 4;
                hw = 16;
                hh = 10;
                break;
            case 1:
                hx = playerX - 12;
                hy = playerY + 4;
                hw = 16;
                hh = 10;
                break;
            case 2:
                hx = playerX + 4;
                hy = playerY - 12;
                hw = 10;
                hh = 16;
                break;
            case 3:
                hx = playerX + 4;
                hy = playerY + 12;
                hw = 10;
                hh = 16;
                break;
        }
        if (collision(hx, hy, hw, hh, enemyX, enemyY, PLAYER_SIZE, PLAYER_SIZE)) {
            enemyHp--;
            attackDealtDamage = 1;
        }
    }

    if (enemyActive && enemyHp > 0 && roomId == 0 && invincibleTimer == 0) {
        if (collision(playerX, playerY, PLAYER_SIZE, PLAYER_SIZE, enemyX, enemyY, PLAYER_SIZE,
                      PLAYER_SIZE)) {
            playerHp--;
            invincibleTimer = 50;
        }
    }

    if (playerHp <= 0) {
        return 2;
    }
    if (enemyHp <= 0) {
        return 1;
    }

    updateCamera();
    drawSprites();
    updateOAM();
    return 0;
}
