#ifndef SCREEN3VIEW_HPP
#define SCREEN3VIEW_HPP

#include <gui_generated/screen3_screen/Screen3ViewBase.hpp>
#include <gui/screen3_screen/Screen3Presenter.hpp>

// ── Cấu hình Game ─────────────────────────────────────
#define SCREEN_W        240
#define SCREEN_H        320

#define PLAYER_W        49
#define PLAYER_H        49
#define PLAYER_Y        (SCREEN_H - PLAYER_H - 10)
#define PLAYER_SPEED    3

#define BOSS_W          80
#define BOSS_H          80
#define BOSS_SPEED      1

//tăng lên 50 để cân bằng game.
#define BOSS_MAX_HP     50

#define BULLET_W        16
#define BULLET_H        16
#define BULLET_SPEED    5

#define B_BULLET_SPEED  1

// Boss có tối đa 10 viên đạn
#define BOSS_BULLET_MAX 10
// ──────────────────────────────────────────────────────

class Screen3View : public Screen3ViewBase
{
public:
    Screen3View();
    virtual ~Screen3View() {}

    virtual void setupScreen();
    virtual void tearDownScreen();

    virtual void handleTickEvent();
    virtual void handleClickEvent(const touchgfx::ClickEvent& evt);

private:
    bool isGameEnded;
    uint32_t tickCounter;
    int score;

    // Background:
    // bg11 đứng yên.
    // bg21/bg22 là lớp sao di chuyển.
    int16_t bg21Y;
    int16_t bg22Y;

    // Người chơi
    int16_t playerX;
    int16_t bulletX;
    int16_t bulletY;
    bool bulletActive;

    bool touchLeft;
    bool touchRight;

    // Boss
    int16_t bossX;
    int16_t bossY;
    int8_t bossDir;
    int bossHP;
    bool isEnraged;
    int currentBossSpeed;
    int teleportCooldown;
    int burstFireTimer;
    int burstBulletCount;

    int bossFireCount;
    uint32_t nextFireTick;

    // Đạn boss
    int16_t bBulletX[BOSS_BULLET_MAX];
    int16_t bBulletY[BOSS_BULLET_MAX];
    int16_t bBulletDX[BOSS_BULLET_MAX];
    int16_t bBulletDY[BOSS_BULLET_MAX];
    bool bBulletActive[BOSS_BULLET_MAX];

    touchgfx::Image* bossBullets[BOSS_BULLET_MAX];

    // Score
    touchgfx::Unicode::UnicodeChar scoreBuf[16];

    void resetGame();
    void updateBackground();
    void updateScoreText();
    void updateMovementInput();
    void bossLogic();
    void fireBossBullets(bool isBurst = false);
    void cleanupGameObjects();
    void finishGame(bool isVictory);
};

#endif // SCREEN3VIEW_HPP
