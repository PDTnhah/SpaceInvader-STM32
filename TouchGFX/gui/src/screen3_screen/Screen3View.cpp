#include <gui/screen3_screen/Screen3View.hpp>
#include <touchgfx/Unicode.hpp>
#include <gui/common/FrontendApplication.hpp>
#include "audio_service.h"
#include "main.h"


extern "C" {
    #include "stm32f4xx_hal.h"
    extern RNG_HandleTypeDef hrng;
}

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

    bossBullets[0] = &bossBullet1;
    bossBullets[1] = &bossBullet2;
    bossBullets[2] = &bossBullet3;
    bossBullets[3] = &bossBullet4;
    bossBullets[4] = &bossBullet5;
    bossBullets[5] = &bossBullet6;
    bossBullets[6] = &bossBullet7;
    bossBullets[7] = &bossBullet8;
    bossBullets[8] = &bossBullet9;
    bossBullets[9] = &bossBullet10;

    resetGame();
    updateHighScoreText();
    updateScoreText();
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
    teleportCooldown = 0;
    burstFireTimer = 0;
    burstBulletCount = 0;

    nextFireTick = 75;

    bossX = (SCREEN_W - BOSS_W) / 2;
    bossY = 10;
    bossDir = 1;
    isEnraged = false;
    currentBossSpeed = BOSS_SPEED;

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

void Screen3View::updateHighScoreText()
{
    int savedHighScore = presenter->getHighScore();

    if (score > savedHighScore)
    {
        savedHighScore = score;
    }

    Unicode::snprintf(highScoreBuf, 16, "%d", savedHighScore);
    highScoreText.setWildcard1(highScoreBuf);
    highScoreText.invalidate();
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

void Screen3View::fireBossBullets(bool isBurst)
{
    // Đếm số lượng đạn đang bay
    int activeCount = 0;
    for (int i = 0; i < BOSS_BULLET_MAX; i++)
    {
        if (bBulletActive[i]) activeCount++;
    }

    if (activeCount == BOSS_BULLET_MAX) return;

    bossFireCount++;

    int startX = bossX + (BOSS_W / 2) - (BULLET_W / 2);
    int startY = bossY + BOSS_H - 10;

    if (isBurst)
    {
        // Bắn 1 viên thẳng cực nhanh
        for (int i = 0; i < BOSS_BULLET_MAX; i++)
        {
            if (!bBulletActive[i])
            {
                bBulletActive[i] = true;
                bBulletX[i] = startX;
                bBulletY[i] = startY;
                bBulletDX[i] = 0;
                bBulletDY[i] = B_BULLET_SPEED + 1; // Giảm tốc độ burst fire xuống
                
                if (bossBullets[i] != 0) {
                    bossBullets[i]->invalidate();
                    bossBullets[i]->setXY(bBulletX[i], bBulletY[i]);
                    bossBullets[i]->setVisible(true);
                    bossBullets[i]->invalidate();
                }
                break;
            }
        }
        return;
    }

    // Không đủ slot để bắn chùm thì đợi
    if (BOSS_BULLET_MAX - activeCount < 3) return;

    uint32_t rngPattern = 0;
    bool fireFive = false;
    if (isEnraged)
    {
        fireFive = true;
    }
    else if (HAL_RNG_GenerateRandomNumber(&hrng, &rngPattern) == HAL_OK)
    {
        fireFive = (rngPattern % 4 == 0); // 25% cơ hội bắn 5 viên
    }

    int bulletCount = fireFive ? 5 : 3;

    // Nếu không đủ slot cho 5 viên thì giảm xuống 3
    if (BOSS_BULLET_MAX - activeCount < bulletCount) {
        bulletCount = 3;
    }

    uint32_t rngSpread = 0;
    int spreadSkew = 0;
    if (HAL_RNG_GenerateRandomNumber(&hrng, &rngSpread) == HAL_OK)
    {
        spreadSkew = (rngSpread % 3) - 1; // Lệch trái, thẳng, hoặc phải
    }

    int dxArray[5];
    if (bulletCount == 3)
    {
        dxArray[0] = -2 + spreadSkew;
        dxArray[1] = 0  + spreadSkew;
        dxArray[2] = 2  + spreadSkew;
    }
    else
    {
        dxArray[0] = -4;
        dxArray[1] = -2;
        dxArray[2] = 0;
        dxArray[3] = 2;
        dxArray[4] = 4;
    }

    int fired = 0;
    for (int i = 0; i < BOSS_BULLET_MAX && fired < bulletCount; i++)
    {
        if (!bBulletActive[i])
        {
            bBulletActive[i] = true;
            bBulletX[i] = startX;
            bBulletY[i] = startY;
            bBulletDX[i] = dxArray[fired];
            
            bBulletDY[i] = B_BULLET_SPEED;
            if (isEnraged)
            {
                bBulletDY[i] = 2; // Tốc độ cơ bản tăng lên 2 khi Enraged
                uint32_t rngJitter = 0;
                if (HAL_RNG_GenerateRandomNumber(&hrng, &rngJitter) == HAL_OK)
                {
                    bBulletDY[i] += (rngJitter % 2); // DY chỉ có thể lên 2 hoặc 3
                }
            }

            if (bossBullets[i] != 0) {
                bossBullets[i]->invalidate();
                bossBullets[i]->setXY(bBulletX[i], bBulletY[i]);
                bossBullets[i]->setVisible(true);
                bossBullets[i]->invalidate();
            }
            
            fired++;
        }
    }
}

void Screen3View::bossLogic()
{
    if (!isEnraged && bossHP <= BOSS_MAX_HP / 2)
    {
        isEnraged = true;
        currentBossSpeed = 3; // Nhanh hơn
    }

    // ── Burst Fire Logic ──
    if (burstFireTimer > 0)
    {
        burstFireTimer--;
        if (burstFireTimer == 0)
        {
            fireBossBullets(true);
        }
    }

    // ── Boss Teleport Logic ──
    if (isEnraged)
    {
        if (teleportCooldown > 0)
        {
            teleportCooldown--;
        }
        else
        {
            uint32_t rngTeleport = 0;
            if (HAL_RNG_GenerateRandomNumber(&hrng, &rngTeleport) == HAL_OK)
            {
                if (rngTeleport % 100 < 20) // 20% cơ hội teleport mỗi tick khi hồi chiêu xong (tăng mạnh để dễ demo)
                {
                    boss.invalidate();
                    bossX = rngTeleport % (SCREEN_W - BOSS_W);
                    boss.setX(bossX);
                    boss.invalidate();
                    teleportCooldown = 60; // Chờ 1 giây
                }
            }
        }
    }

    // ── 1. Boss di chuyển ngang ──
    boss.invalidate();
    bossX += bossDir * currentBossSpeed;

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

    // ── 2. Boss bắn ngẫu nhiên  ──

    if (tickCounter >= nextFireTick)
    {
        fireBossBullets();

        // Tính tick cho lần bắn tiếp theo
        uint32_t rngTime = 0;
        if (HAL_RNG_GenerateRandomNumber(&hrng, &rngTime) == HAL_OK)
        {
            if (isEnraged)
            {
                nextFireTick = tickCounter + 30 + (rngTime % 30); // 30-60 tick
                if (((rngTime >> 16) % 100) < 15) // Giảm xuống chỉ còn 15% cơ hội bắn bồi (Burst fire)
                {
                    burstFireTimer = 15;
                }
            }
            else
            {
                nextFireTick = tickCounter + 50 + (rngTime % 61); // Cách nhau 50-110 tick
            }
        }
        else
        {
            nextFireTick = tickCounter + (isEnraged ? 35 : 75); // Fallback an toàn
        }
    }

    // ── 3. Di chuyển đạn boss ──
    for (int i = 0; i < BOSS_BULLET_MAX; i++)
    {
        if (bBulletActive[i])
        {
            if (bossBullets[i] != 0) bossBullets[i]->invalidate();

            // Sóng Zic-zac cho đạn boss khi Enraged
            if (isEnraged && (tickCounter % 10 == 0)) {
                uint32_t rngWavy = 0;
                if (HAL_RNG_GenerateRandomNumber(&hrng, &rngWavy) == HAL_OK) {
                    bBulletDX[i] += ((rngWavy % 3) - 1);
                    if (bBulletDX[i] > 5) bBulletDX[i] = 5;
                    if (bBulletDX[i] < -5) bBulletDX[i] = -5;
                }
            }

            bBulletY[i] += bBulletDY[i];
            bBulletX[i] += bBulletDX[i];

            if (bossBullets[i] != 0) bossBullets[i]->setXY(bBulletX[i], bBulletY[i]);

            if (bBulletY[i] > SCREEN_H ||
                bBulletX[i] < -BULLET_W ||
                bBulletX[i] > SCREEN_W)
            {
                bBulletActive[i] = false;
                // Đảm bảo đạn trượt ra ngoài phải được ẩn sạch
                if (bossBullets[i] != 0) {
                    bossBullets[i]->setVisible(false);
                    bossBullets[i]->invalidate();
                }
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

                if (bossBullets[i] != 0) bossBullets[i]->invalidate();
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
                updateHighScoreText();

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
