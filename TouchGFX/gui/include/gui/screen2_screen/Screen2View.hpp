#ifndef SCREEN2VIEW_HPP
#define SCREEN2VIEW_HPP

#include <gui_generated/screen2_screen/Screen2ViewBase.hpp>
#include <gui/screen2_screen/Screen2Presenter.hpp>

// ── Cấu hình Game ─────────────────────────────────────
#define SCREEN_W        240
#define SCREEN_H        320

#define PLAYER_W        49
#define PLAYER_H        49
#define PLAYER_Y        (SCREEN_H - PLAYER_H - 10)
#define PLAYER_SPEED    3

#define ENEMY_W         33
#define ENEMY_H         25
#define ENEMY_DROP      10

#define BULLET_W        16
#define BULLET_H        16
#define BULLET_SPEED    5

#define NUM_ENEMIES_MAX 12
#define WAVES_TO_BOSS   3
// ──────────────────────────────────────────────────────

class Screen2View : public Screen2ViewBase
{
public:
    Screen2View();
    virtual ~Screen2View() {}

    virtual void setupScreen();
    virtual void tearDownScreen();

    virtual void handleTickEvent();
    virtual void handleClickEvent(const touchgfx::ClickEvent& evt);

private:

    bool isGameEnded;
    uint32_t tickCounter;

    int waveLevel;
    int currentEnemySpeed;
    int activeEnemiesCount;

    int nextEnemyFireTick;

    // Background
    int16_t bg11Y;
    int16_t bg12Y;
    int16_t bg21Y;
    int16_t bg22Y;

    // Player
    int16_t playerX;
    int16_t bulletX;
    int16_t bulletY;
    bool bulletActive;

    // Enemy bullet
    int16_t eBulletX;
    int16_t eBulletY;
    bool eBulletActive;

    // Enemy
    int16_t enemyX[NUM_ENEMIES_MAX];
    int16_t enemyY[NUM_ENEMIES_MAX];
    bool enemyAlive[NUM_ENEMIES_MAX];
    bool enemyKamikaze[NUM_ENEMIES_MAX];
    int8_t enemyDir;

    touchgfx::Image* enemyWidgets[NUM_ENEMIES_MAX];

    // Score
    int score;
    touchgfx::Unicode::UnicodeChar scoreBuf[16];
    touchgfx::Unicode::UnicodeChar highScoreBuf[16];

    // Touch control
    bool touchLeft;
    bool touchRight;

    void resetGame();
    void updateBackground();
    void updateScoreText();
    void updateHighScoreText();
    void cleanupGameObjects();
    void updateMovementInput();

    bool checkHit(int eIndex);
    void moveEnemies();
    void enemyFireLogic();
    void spawnNextWave();
    void finishGame(bool isVictory);
};

#endif // SCREEN2VIEW_HPP
