#include <genesis.h>

#define SCREEN_W 40
#define SCREEN_H 28

#define PLAYER_Y 23
#define PLAYER_MIN_X 1
#define PLAYER_MAX_X 38

#define MAX_OBJECTS 6
#define GAME_SECONDS 60
#define FPS 60

typedef struct
{
    s16 x;
    s16 y;
    u8 active;
    u8 kind;       // 0 = beer, 1 = cable
} FallingObject;

static FallingObject objects[MAX_OBJECTS];

static u16 rngState = 0x4D53;
static s16 playerX = 20;
static u32 frameCount = 0;
static u32 score = 0;
static s16 lives = 3;
static u8 gameOver = FALSE;
static u8 won = FALSE;

static u16 fastRand(void)
{
    // Tiny 16-bit LFSR: deterministic, cheap, and good enough for falling junk.
    u16 lsb = rngState & 1;
    rngState >>= 1;
    if (lsb) rngState ^= 0xB400u;
    return rngState;
}

static void clearPlayfield(void)
{
    VDP_clearPlane(BG_A, TRUE);
}

static void drawFrame(void)
{
    VDP_drawText("THE LOST CHORDS: FEEDBACK!", 6, 1);
    VDP_drawText("----------------------------------------", 0, 3);
    VDP_drawText("D-PAD: MOVE       START: RESTART", 3, 26);
}

static void drawHUD(void)
{
    char buf[32];

    sprintf(buf, "SCORE %05lu", score);
    VDP_clearText(1, 4, 16);
    VDP_drawText(buf, 1, 4);

    sprintf(buf, "LIVES %d", lives);
    VDP_clearText(29, 4, 10);
    VDP_drawText(buf, 29, 4);

    u32 seconds = frameCount / FPS;
    u32 remain = (seconds >= GAME_SECONDS) ? 0 : (GAME_SECONDS - seconds);
    sprintf(buf, "TIME %02lu", remain);
    VDP_clearText(17, 4, 10);
    VDP_drawText(buf, 17, 4);
}

static void resetObjects(void)
{
    for (u16 i = 0; i < MAX_OBJECTS; i++)
    {
        objects[i].active = FALSE;
        objects[i].x = 0;
        objects[i].y = 0;
        objects[i].kind = 0;
    }
}

static void resetGame(void)
{
    playerX = 20;
    frameCount = 0;
    score = 0;
    lives = 3;
    gameOver = FALSE;
    won = FALSE;
    rngState ^= (u16)GET_VCOUNTER;

    clearPlayfield();
    drawFrame();
    resetObjects();
    drawHUD();

    VDP_drawText("@", playerX, PLAYER_Y);
}

static void spawnObject(void)
{
    for (u16 i = 0; i < MAX_OBJECTS; i++)
    {
        if (!objects[i].active)
        {
            objects[i].active = TRUE;
            objects[i].x = 1 + (fastRand() % 38);
            objects[i].y = 6;
            objects[i].kind = (fastRand() & 1);
            return;
        }
    }
}

static void eraseObject(FallingObject *o)
{
    if (o->active && o->y >= 6 && o->y <= 24)
        VDP_drawText(" ", o->x, o->y);
}

static void drawObject(FallingObject *o)
{
    if (!o->active) return;
    VDP_drawText(o->kind ? "~" : "*", o->x, o->y);
}

static void updateObjects(void)
{
    // Roughly one vertical cell every 8 frames.
    if ((frameCount % 8) != 0) return;

    for (u16 i = 0; i < MAX_OBJECTS; i++)
    {
        FallingObject *o = &objects[i];
        if (!o->active) continue;

        eraseObject(o);
        o->y++;

        // Collision with the band member.
        if (o->y == PLAYER_Y && o->x == playerX)
        {
            lives--;
            o->active = FALSE;
            VDP_drawText("OUCH!", 17, 14);

            if (lives <= 0)
            {
                gameOver = TRUE;
                return;
            }
            continue;
        }

        // Survived object = points.
        if (o->y > 24)
        {
            o->active = FALSE;
            score += 25;
            continue;
        }

        drawObject(o);
    }

    // Clear temporary message.
    if ((frameCount % 120) == 0)
        VDP_clearText(15, 14, 12);
}

static void handleInput(void)
{
    u16 joy = JOY_readJoypad(JOY_1);

    if (gameOver || won)
    {
        if (joy & BUTTON_START)
            resetGame();
        return;
    }

    s16 oldX = playerX;

    if ((joy & BUTTON_LEFT) && playerX > PLAYER_MIN_X) playerX--;
    if ((joy & BUTTON_RIGHT) && playerX < PLAYER_MAX_X) playerX++;

    if (oldX != playerX)
    {
        VDP_drawText(" ", oldX, PLAYER_Y);
        VDP_drawText("@", playerX, PLAYER_Y);
    }
}

static void finishGame(void)
{
    won = TRUE;
    VDP_clearText(0, 10, 40);
    VDP_clearText(0, 11, 40);
    VDP_clearText(0, 12, 40);
    VDP_clearText(0, 13, 40);
    VDP_drawText("YOU SURVIVED THE GIG.", 9, 11);
    VDP_drawText("THE SOUND GUY LOOKS DISAPPOINTED.", 3, 13);
    VDP_drawText("PRESS START FOR ANOTHER DISASTER", 3, 16);
}

static void showGameOver(void)
{
    VDP_drawText("GAME OVER - TOO MUCH FEEDBACK!", 5, 12);
    VDP_drawText("PRESS START TO TRY AGAIN", 8, 15);
}

int main(bool hardReset)
{
    JOY_init();

    // This prototype intentionally targets 60 Hz only.
    // On a PAL console it stops here instead of silently running slower.
    if (SYS_isPAL())
    {
        VDP_drawText("THE LOST CHORDS: FEEDBACK!", 6, 8);
        VDP_drawText("THIS BUILD IS 60 HZ / NTSC ONLY.", 4, 12);
        VDP_drawText("USE A 60 HZ MOD OR NTSC CONSOLE.", 4, 14);

        while (TRUE)
            SYS_doVBlankProcess();
    }

    resetGame();

    while (TRUE)
    {
        handleInput();

        if (!gameOver && !won)
        {
            frameCount++;

            // Spawn rate ramps up gradually during the minute.
            u16 interval = 45;
            if (frameCount > 20 * FPS) interval = 34;
            if (frameCount > 40 * FPS) interval = 25;

            if ((frameCount % interval) == 0)
                spawnObject();

            updateObjects();

            if ((frameCount % 15) == 0)
                drawHUD();

            if (frameCount >= (GAME_SECONDS * FPS))
                finishGame();
        }
        else if (gameOver)
        {
            showGameOver();
        }

        SYS_doVBlankProcess();
    }

    return 0;
}
