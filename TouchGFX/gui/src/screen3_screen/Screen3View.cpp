#include <gui/screen3_screen/Screen3View.hpp>
#include <touchgfx/Unicode.hpp>
#include <gui/common/FrontendApplication.hpp>
#include "audio_service.h"
#include "main.h"

Screen3View::Screen3View()
    : isGameEnded(false),
      tickCounter(0),
      score(0),
      bg21Y(0),
      bg22Y(-SCREEN_H),
      playerX(0),
      bulletX(0),
      bulletY(0),
      bulletActive(false),
      touchLeft(false),
      touchRight(false),
      bossX(0),
      bossY(0),
      bossDir(1),
      bossHP(BOSS_MAX_HP),
      bossFireCount(0)
{
    for (int i = 0; i < BOSS_BULLET_MAX; i++)
    {
        bBulletX[i] = 0;
        bBulletY[i] = 0;
        bBulletDX[i] = 0;
        bBulletActive[i] = false;
        bossBullets[i] = 0;
    }
}

void Screen3View::setupScreen()
{
    Screen3ViewBase::setupScreen();

    // Trong TouchGFX Designer cần có:
    // bossBullet1, bossBullet2, bossBullet3, bossBullet4, bossBullet5
    bossBullets[0] = &bossBullet1;
    bossBullets[1] = &bossBullet2;
    bossBullets[2] = &bossBullet3;
    bossBullets[3] = &bossBullet4;
    bossBullets[4] = &bossBullet5;

    resetGame();
}

void Screen3View::tearDownScreen()
{
    cleanupGameObjects();
    Screen3ViewBase::tearDownScreen();
}

void Screen3View::resetGame()
{
    isGameEnded = false;
    tickCounter = 0;

    score = presenter->getCurrentScore();

    // ── Background ────────────────────────────────────
    // bg11 đứng yên, chỉ set một lần.
    bg11.invalidate();
    bg11.setXY(0, 0);
    bg11.setVisible(true);
    bg11.invalidate();

    // bg21/bg22 là lớp sao cuộn.
    bg21Y = 0;
    bg22Y = -SCREEN_H;

    bg21.invalidate();
    bg22.invalidate();

    bg21.setXY(0, bg21Y);
    bg22.setXY(0, bg22Y);

    bg21.setVisible(true);
    bg22.setVisible(true);

    bg21.invalidate();
    bg22.invalidate();

    // ── Player ────────────────────────────────────────
    playerX = (SCREEN_W - PLAYER_W) / 2;

    player.invalidate();
    player.setX(playerX);
    player.setY(PLAYER_Y);
    player.invalidate();

    bulletActive = false;

    player_bullet.invalidate();
    player_bullet.setVisible(false);
    player_bullet.invalidate();

    touchLeft = false;
    touchRight = false;

    // ── Boss ──────────────────────────────────────────
    bossHP = BOSS_MAX_HP;
    bossFireCount = 0;

    bossX = (SCREEN_W - BOSS_W) / 2;
    bossY = 10;
    bossDir = 1;

    boss.invalidate();
    boss.setXY(bossX, bossY);
    boss.setVisible(true);
    boss.invalidate();

    // ── Boss bullets ──────────────────────────────────
    for (int i = 0; i < BOSS_BULLET_MAX; i++)
    {
        bBulletX[i] = 0;
        bBulletY[i] = 0;
        bBulletDX[i] = 0;
        bBulletActive[i] = false;

        if (bossBullets[i] != 0)
        {
            bossBullets[i]->invalidate();
            bossBullets[i]->setVisible(false);
            bossBullets[i]->invalidate();
        }
    }

    updateScoreText();
}

void Screen3View::cleanupGameObjects()
{
    bulletActive = false;
    touchLeft = false;
    touchRight = false;

    player_bullet.invalidate();
    player_bullet.setVisible(false);
    player_bullet.invalidate();

    for (int i = 0; i < BOSS_BULLET_MAX; i++)
    {
        bBulletActive[i] = false;

        if (bossBullets[i] != 0)
        {
            bossBullets[i]->invalidate();
            bossBullets[i]->setVisible(false);
            bossBullets[i]->invalidate();
        }
    }
}

void Screen3View::updateScoreText()
{
    Unicode::snprintf(scoreBuf, 16, "%d", score);
    scoreText.setWildcard1(scoreBuf);
    scoreText.invalidate();
}

void Screen3View::updateMovementInput()
{
    static bool lastHwLeft = false;
    static bool lastHwRight = false;

    bool hardwareLeft = (HAL_GPIO_ReadPin(GPIOG, GPIO_PIN_2) == GPIO_PIN_RESET);
    bool hardwareRight = (HAL_GPIO_ReadPin(GPIOG, GPIO_PIN_3) == GPIO_PIN_RESET);

    // Chỉ set touchLeft / Right khi có sự thay đổi (nhấn xuống)
    if (hardwareLeft && !lastHwLeft)
    {
        touchLeft = true;
        touchRight = false;
    }
    else if (!hardwareLeft && lastHwLeft)
    {
        touchLeft = false;
    }

    if (hardwareRight && !lastHwRight)
    {
        touchRight = true;
        touchLeft = false;
    }
    else if (!hardwareRight && lastHwRight)
    {
        touchRight = false;
    }

    lastHwLeft = hardwareLeft;
    lastHwRight = hardwareRight;
}

void Screen3View::updateBackground()
{
    /*
     * Background mới:
     * - bg11 đứng yên.
     * - bg21/bg22 là lớp sao di chuyển.
     * - Update mỗi 4 tick để nhẹ hơn.
     */

    if (tickCounter % 4 != 0)
    {
        return;
    }

    bg21.invalidate();
    bg22.invalidate();

    bg21Y += 1;
    bg22Y += 1;

    if (bg21Y >= SCREEN_H)
    {
        bg21Y = -SCREEN_H;
    }

    if (bg22Y >= SCREEN_H)
    {
        bg22Y = -SCREEN_H;
    }

    bg21.setY(bg21Y);
    bg22.setY(bg22Y);

    bg21.invalidate();
    bg22.invalidate();
}

void Screen3View::finishGame(bool isVictory)
{
    if (isGameEnded)
    {
        return;
    }

    isGameEnded = true;

    cleanupGameObjects();

    if (isVictory)
    {
        score += 500;
    }

    presenter->saveGameResult(score, isVictory);

    static_cast<FrontendApplication*>(Application::getInstance())->gotoScreen4ScreenNoTransition();
}

void Screen3View::fireBossBullets()
{
    // Không bắn loạt mới nếu còn viên nào đang bay
    for (int i = 0; i < BOSS_BULLET_MAX; i++)
    {
        if (bBulletActive[i])
        {
            return;
        }
    }

    bossFireCount++;

    // Pattern: 3 3 3 3 5 3 3 3 3 5 ...
    bool fireFive = (bossFireCount % 5 == 0);
    int bulletCount = fireFive ? 5 : 3;

    int startX = bossX + (BOSS_W / 2) - (BULLET_W / 2);
    int startY = bossY + BOSS_H - 10;

    if (bulletCount == 3)
    {
        // 3 viên: trái, giữa, phải
        bBulletDX[0] = -1;
        bBulletDX[1] = 0;
        bBulletDX[2] = 1;

        for (int i = 0; i < 3; i++)
        {
            bBulletActive[i] = true;
            bBulletX[i] = startX;
            bBulletY[i] = startY;

            bossBullets[i]->invalidate();
            bossBullets[i]->setXY(bBulletX[i], bBulletY[i]);
            bossBullets[i]->setVisible(true);
            bossBullets[i]->invalidate();
        }

        // Ẩn viên 4, 5 nếu có
        for (int i = 3; i < BOSS_BULLET_MAX; i++)
        {
            bBulletActive[i] = false;
            bossBullets[i]->invalidate();
            bossBullets[i]->setVisible(false);
            bossBullets[i]->invalidate();
        }
    }
    else
    {
        // 5 viên: xòe rộng hơn
        bBulletDX[0] = -2;
        bBulletDX[1] = -1;
        bBulletDX[2] = 0;
        bBulletDX[3] = 1;
        bBulletDX[4] = 2;

        for (int i = 0; i < BOSS_BULLET_MAX; i++)
        {
            bBulletActive[i] = true;
            bBulletX[i] = startX;
            bBulletY[i] = startY;

            bossBullets[i]->invalidate();
            bossBullets[i]->setXY(bBulletX[i], bBulletY[i]);
            bossBullets[i]->setVisible(true);
            bossBullets[i]->invalidate();
        }
    }
}

void Screen3View::bossLogic()
{
    // ── 1. Boss di chuyển ngang ───────────────────────
    boss.invalidate();

    bossX += bossDir * BOSS_SPEED;

    if (bossX <= 0)
    {
        bossX = 0;
        bossDir = 1;
    }
    else if (bossX >= SCREEN_W - BOSS_W)
    {
        bossX = SCREEN_W - BOSS_W;
        bossDir = -1;
    }

    boss.setX(bossX);
    boss.invalidate();

    // ── 2. Boss bắn mỗi 75 tick ───────────────────────
    // Tăng từ 60 lên 75 để boss bắn thưa hơn.
    if (tickCounter % 80 == 0)
    {
        fireBossBullets();
    }

    // ── 3. Di chuyển đạn boss ─────────────────────────
    for (int i = 0; i < BOSS_BULLET_MAX; i++)
    {
        if (bBulletActive[i])
        {
            bossBullets[i]->invalidate();

            bBulletY[i] += B_BULLET_SPEED;
            bBulletX[i] += bBulletDX[i];

            bossBullets[i]->setXY(bBulletX[i], bBulletY[i]);

            if (bBulletY[i] > SCREEN_H ||
                bBulletX[i] < -BULLET_W ||
                bBulletX[i] > SCREEN_W)
            {
                bBulletActive[i] = false;
                bossBullets[i]->setVisible(false);
                bossBullets[i]->invalidate();
            }
            else
            {
                bool xOverlap = (bBulletX[i] < playerX + PLAYER_W - 8) &&
                                (bBulletX[i] + BULLET_W > playerX + 8);

                bool yOverlap = (bBulletY[i] < PLAYER_Y + PLAYER_H - 8) &&
                                (bBulletY[i] + BULLET_H > PLAYER_Y + 8);

                if (xOverlap && yOverlap)
                {
                    finishGame(false);
                    return;
                }

                bossBullets[i]->invalidate();
            }
        }
    }
}

void Screen3View::handleClickEvent(const touchgfx::ClickEvent& evt)
{
    Screen3ViewBase::handleClickEvent(evt);

    int touchX = evt.getX();

    if (evt.getType() == touchgfx::ClickEvent::PRESSED)
    {
        if (touchX < SCREEN_W / 2)
        {
            touchLeft = true;
            touchRight = false;
        }
        else
        {
            touchRight = true;
            touchLeft = false;
        }
    }
    else if (evt.getType() == touchgfx::ClickEvent::RELEASED)
    {
        touchLeft = false;
        touchRight = false;
    }
}

void Screen3View::handleTickEvent()
{
    Screen3ViewBase::handleTickEvent();

    if (isGameEnded)
    {
        return;
    }

    tickCounter++;

    updateBackground();
    updateMovementInput();

    // ── 1. Di chuyển player ───────────────────────────
    bool playerMoved = false;

    if (touchLeft)
    {
        playerX -= PLAYER_SPEED * 2; // Tăng tốc độ dịch 1 đoạn cho rõ

        if (playerX < 0)
        {
            playerX = 0;
        }

        playerMoved = true;
    }

    if (touchRight)
    {
        playerX += PLAYER_SPEED * 2; // Tăng tốc độ dịch 1 đoạn cho rõ

        if (playerX > SCREEN_W - PLAYER_W)
        {
            playerX = SCREEN_W - PLAYER_W;
        }

        playerMoved = true;
    }

    if (playerMoved)
    {
        player.invalidate();
        player.setX(playerX);
        player.invalidate();
    }

    // ── 2. Auto fire player ───────────────────────────
    if (!bulletActive)
    {
        bulletActive = true;
        AudioService_PlayLaser();

        bulletX = playerX + (PLAYER_W / 2) - (BULLET_W / 2);
        bulletY = PLAYER_Y - BULLET_H;

        player_bullet.invalidate();
        player_bullet.setXY(bulletX, bulletY);
        player_bullet.setVisible(true);
        player_bullet.invalidate();
    }

    // ── 3. Di chuyển đạn player ───────────────────────
    if (bulletActive)
    {
        player_bullet.invalidate();

        bulletY -= BULLET_SPEED;
        player_bullet.setY(bulletY);

        if (bulletY < -BULLET_H)
        {
            bulletActive = false;
            player_bullet.setVisible(false);
            player_bullet.invalidate();
        }
        else
        {
            bool xOverlap = (bulletX < bossX + BOSS_W - 5) &&
                            (bulletX + BULLET_W > bossX + 5);

            bool yOverlap = (bulletY < bossY + BOSS_H - 10) &&
                            (bulletY + BULLET_H > bossY + 10);

            if (xOverlap && yOverlap)
            {
                bulletActive = false;
                player_bullet.setVisible(false);
                player_bullet.invalidate();

                AudioService_PlayHit();
                bossHP -= 1;
                score += 5;

                updateScoreText();

                if (bossHP <= 0)
                {
                    finishGame(true);
                    return;
                }
            }
            else
            {
                player_bullet.invalidate();
            }
        }
    }

    // ── 4. Boss logic ─────────────────────────────────
    bossLogic();

    if (isGameEnded)
    {
        return;
    }

    // ── 5. Boss va vào player ─────────────────────────
    bool crashX = (bossX < playerX + PLAYER_W - 10) &&
                  (bossX + BOSS_W > playerX + 10);

    bool crashY = (bossY + BOSS_H > PLAYER_Y + 10);

    if (crashX && crashY)
    {
        finishGame(false);
    }
}
