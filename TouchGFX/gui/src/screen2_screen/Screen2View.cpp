#include <gui/screen2_screen/Screen2View.hpp>
#include <touchgfx/Unicode.hpp>
#include <gui/common/FrontendApplication.hpp>
#include "audio_service.h"
#include "main.h"

Screen2View::Screen2View()
    : isGameEnded(false),
      tickCounter(0),
      waveLevel(1),
      currentEnemySpeed(2),
      activeEnemiesCount(0),
      bg11Y(0),
      bg12Y(-320),
      bg21Y(0),
      bg22Y(-320),
      playerX(0),
      bulletX(0),
      bulletY(0),
      bulletActive(false),
      eBulletX(0),
      eBulletY(0),
      eBulletActive(false),
      enemyDir(1),
      score(0),
      touchLeft(false),
      touchRight(false)
{
    for (int i = 0; i < NUM_ENEMIES_MAX; i++)
    {
        enemyX[i] = 0;
        enemyY[i] = 0;
        enemyAlive[i] = false;
        enemyWidgets[i] = 0;
    }
}

void Screen2View::setupScreen()
{
    Screen2ViewBase::setupScreen();

    enemyWidgets[0]  = &enemy1;
    enemyWidgets[1]  = &enemy2;
    enemyWidgets[2]  = &enemy3;
    enemyWidgets[3]  = &enemy4;
    enemyWidgets[4]  = &enemy5;
    enemyWidgets[5]  = &enemy6;
    enemyWidgets[6]  = &enemy7;
    enemyWidgets[7]  = &enemy8;
    enemyWidgets[8]  = &enemy9;
    enemyWidgets[9]  = &enemy10;
    enemyWidgets[10] = &enemy11;
    enemyWidgets[11] = &enemy12;

    resetGame();
}

void Screen2View::tearDownScreen()
{
    cleanupGameObjects();
    Screen2ViewBase::tearDownScreen();
}

void Screen2View::resetGame()
{
    isGameEnded = false;
    tickCounter = 0;

    score = 0;
    waveLevel = 1;
    currentEnemySpeed = 2;
    activeEnemiesCount = 0;
    enemyDir = 1;

    // ── Setup background bản nhẹ ──────────────────────
    bg11Y = 0;

    bg21Y = 0;
    bg22Y = -SCREEN_H;

    bg11.invalidate();
    bg21.invalidate();
    bg22.invalidate();

    bg11.setXY(0, bg11Y);
    bg21.setXY(0, bg21Y);
    bg22.setXY(0, bg22Y);

    bg11.setVisible(true);

    // Layer sao
    bg21.setVisible(true);
    bg22.setVisible(true);

    bg11.invalidate();
    bg21.invalidate();
    bg22.invalidate();

    // ── Setup player ──────────────────────────────────
    playerX = (SCREEN_W - PLAYER_W) / 2;

    player.invalidate();
    player.setX(playerX);
    player.setY(PLAYER_Y);
    player.invalidate();

    bulletActive = false;
    player_bullet.invalidate();
    player_bullet.setVisible(false);
    player_bullet.invalidate();

    eBulletActive = false;
    enemy_bullet.invalidate();
    enemy_bullet.setVisible(false);
    enemy_bullet.invalidate();

    touchLeft = false;
    touchRight = false;

    // Ẩn toàn bộ enemy trước khi spawn wave
    for (int i = 0; i < NUM_ENEMIES_MAX; i++)
    {
        enemyAlive[i] = false;

        if (enemyWidgets[i] != 0)
        {
            enemyWidgets[i]->invalidate();
            enemyWidgets[i]->setVisible(false);
            enemyWidgets[i]->invalidate();
        }
    }

    updateScoreText();
    updateHighScoreText();

    spawnNextWave();
}

void Screen2View::updateScoreText()
{
    Unicode::snprintf(scoreBuf, 16, "%d", score);
    scoreText.setWildcard1(scoreBuf);
    scoreText.invalidate();
}

void Screen2View::updateHighScoreText()
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

void Screen2View::updateMovementInput()
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

void Screen2View::updateBackground()
{
    // Chỉ layer sao chạy

    if ((tickCounter % 2) != 0)
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

void Screen2View::finishGame(bool isVictory)
{
    if (isGameEnded)
    {
        return;
    }

    isGameEnded = true;

    cleanupGameObjects();

    presenter->saveGameResult(score, isVictory);

    static_cast<FrontendApplication*>(Application::getInstance())->gotoScreen4ScreenNoTransition();
}

void Screen2View::spawnNextWave()
{
    /*
     * Nếu muốn chơi đủ 3 wave rồi mới sang boss:
     * wave 1 -> wave 2 -> wave 3 -> Screen3 boss
     * thì điều kiện phải là waveLevel > WAVES_TO_BOSS.
     */
	if (waveLevel > WAVES_TO_BOSS)
	{
	    isGameEnded = true;

	    cleanupGameObjects();

	    presenter->saveGameResult(score, false);

	    static_cast<FrontendApplication*>(Application::getInstance())->gotoScreen3ScreenNoTransition();
	    return;
	}

    // Reset đạn người chơi khi sang wave mới
    bulletActive = false;
    player_bullet.invalidate();
    player_bullet.setVisible(false);
    player_bullet.invalidate();

    // Reset đạn enemy khi sang wave mới
    eBulletActive = false;
    enemy_bullet.invalidate();
    enemy_bullet.setVisible(false);
    enemy_bullet.invalidate();

    activeEnemiesCount = waveLevel * 4;

    if (activeEnemiesCount > NUM_ENEMIES_MAX)
    {
        activeEnemiesCount = NUM_ENEMIES_MAX;
    }

    currentEnemySpeed = 1 + (waveLevel / 2);
    enemyDir = 1;

    // Ẩn toàn bộ enemy cũ
    for (int i = 0; i < NUM_ENEMIES_MAX; i++)
    {
        enemyAlive[i] = false;

        if (enemyWidgets[i] != 0)
        {
            enemyWidgets[i]->invalidate();
            enemyWidgets[i]->setVisible(false);
            enemyWidgets[i]->invalidate();
        }
    }

    static uint32_t seed = 123;
    seed += tickCounter + waveLevel * 37;

    for (int i = 0; i < activeEnemiesCount; i++)
    {
        enemyAlive[i] = true;

        int row = i / 4;
        int col = i % 4;

        int baseX = 15 + col * (ENEMY_W + 20);
        int baseY = 20 + row * (ENEMY_H + 10);

        seed = seed * 1103515245 + 12345;
        int offsetX = ((seed >> 16) % 15) - 7;

        enemyX[i] = baseX + offsetX;

        if (enemyX[i] < 0)
        {
            enemyX[i] = 0;
        }

        if (enemyX[i] > SCREEN_W - ENEMY_W)
        {
            enemyX[i] = SCREEN_W - ENEMY_W;
        }

        enemyY[i] = baseY;

        if (enemyWidgets[i] != 0)
        {
            enemyWidgets[i]->invalidate();
            enemyWidgets[i]->setXY(enemyX[i], enemyY[i]);
            enemyWidgets[i]->setVisible(true);
            enemyWidgets[i]->invalidate();
        }
    }
}

bool Screen2View::checkHit(int eIndex)
{
    if (!bulletActive || !enemyAlive[eIndex])
    {
        return false;
    }

    bool xOverlap = (bulletX < enemyX[eIndex] + ENEMY_W - 4) &&
                    (bulletX + BULLET_W > enemyX[eIndex] + 4);

    bool yOverlap = (bulletY < enemyY[eIndex] + ENEMY_H - 4) &&
                    (bulletY + BULLET_H > enemyY[eIndex] + 4);

    return xOverlap && yOverlap;
}

void Screen2View::moveEnemies()
{
    bool hitWall = false;

    // Check enemy chạm tường hoặc chạm player
    for (int i = 0; i < activeEnemiesCount; i++)
    {
        if (enemyAlive[i])
        {
            if (enemyX[i] <= 0 || enemyX[i] >= SCREEN_W - ENEMY_W)
            {
                hitWall = true;
            }

            bool xOverlap = (enemyX[i] < playerX + PLAYER_W - 10) &&
                            (enemyX[i] + ENEMY_W > playerX + 10);

            bool yOverlap = (enemyY[i] + ENEMY_H > PLAYER_Y + 10);

            if (xOverlap && yOverlap)
            {
                finishGame(false);
                return;
            }
        }
    }

    // Nếu chạm tường thì đổi hướng và hạ xuống
    if (hitWall)
    {
        enemyDir = -enemyDir;

        for (int i = 0; i < activeEnemiesCount; i++)
        {
            if (enemyAlive[i])
            {
                enemyY[i] += ENEMY_DROP;

                if (enemyY[i] + ENEMY_H >= PLAYER_Y)
                {
                    finishGame(false);
                    return;
                }
            }
        }
    }

    // Di chuyển enemy
    for (int i = 0; i < activeEnemiesCount; i++)
    {
        if (enemyAlive[i])
        {
            enemyWidgets[i]->invalidate();

            enemyX[i] += enemyDir * currentEnemySpeed;

            if (enemyX[i] < 0)
            {
                enemyX[i] = 0;
            }

            if (enemyX[i] > SCREEN_W - ENEMY_W)
            {
                enemyX[i] = SCREEN_W - ENEMY_W;
            }

            enemyWidgets[i]->setXY(enemyX[i], enemyY[i]);
            enemyWidgets[i]->invalidate();
        }
    }
}

void Screen2View::enemyFireLogic()
{
    if (eBulletActive || isGameEnded)
    {
        return;
    }

    int aliveIdx[NUM_ENEMIES_MAX];
    int count = 0;

    for (int i = 0; i < activeEnemiesCount; i++)
    {
        if (enemyAlive[i])
        {
            aliveIdx[count++] = i;
        }
    }

    if (count > 0)
    {
        int fireRate = 100 - (waveLevel * 15);

        if (fireRate < 25)
        {
            fireRate = 25;
        }

        if (tickCounter % fireRate == 0)
        {
            int shooter = aliveIdx[(tickCounter * 13) % count];

            eBulletX = enemyX[shooter] + (ENEMY_W / 2) - (BULLET_W / 2);
            eBulletY = enemyY[shooter] + ENEMY_H;

            eBulletActive = true;

            enemy_bullet.invalidate();
            enemy_bullet.setXY(eBulletX, eBulletY);
            enemy_bullet.setVisible(true);
            enemy_bullet.invalidate();
        }
    }
}

void Screen2View::handleClickEvent(const touchgfx::ClickEvent& evt)
{
    Screen2ViewBase::handleClickEvent(evt);

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

void Screen2View::handleTickEvent()
{
    Screen2ViewBase::handleTickEvent();

    if (isGameEnded)
    {
        return;
    }

    tickCounter++;

    // Bật lại dòng này để nền cuộn nhẹ.
    updateBackground();
    updateMovementInput();

    // ── 1. Di chuyển player ───────────────────────────
    bool playerMoved = false;

    if (touchLeft)
    {
        playerX -= PLAYER_SPEED;

        if (playerX < 0)
        {
            playerX = 0;
        }

        playerMoved = true;
    }

    if (touchRight)
    {
        playerX += PLAYER_SPEED;

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

    // ── 2. Auto fire ──────────────────────────────────
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
            player_bullet.invalidate();
        }
    }

    // ── 4. Enemy di chuyển ────────────────────────────
    if (tickCounter % 3 == 0)
    {
        moveEnemies();

        if (isGameEnded)
        {
            return;
        }
    }

    // ── 5. Enemy bắn ──────────────────────────────────
    enemyFireLogic();

    // ── 6. Đạn enemy ──────────────────────────────────
    if (eBulletActive)
    {
        enemy_bullet.invalidate();

        eBulletY += BULLET_SPEED - 1;
        enemy_bullet.setY(eBulletY);

        if (eBulletY > SCREEN_H)
        {
            eBulletActive = false;
            enemy_bullet.setVisible(false);
            enemy_bullet.invalidate();
        }
        else
        {
            bool xOverlap = (eBulletX < playerX + PLAYER_W - 8) &&
                            (eBulletX + BULLET_W > playerX + 8);

            bool yOverlap = (eBulletY < PLAYER_Y + PLAYER_H - 8) &&
                            (eBulletY + BULLET_H > PLAYER_Y + 8);

            if (xOverlap && yOverlap)
            {
                finishGame(false);
                return;
            }

            enemy_bullet.invalidate();
        }
    }

    // ── 7. Check đạn player trúng enemy ───────────────
    int aliveCount = 0;

    for (int i = 0; i < activeEnemiesCount; i++)
    {
        if (checkHit(i))
        {
            enemyAlive[i] = false;
            AudioService_PlayHit();

            enemyWidgets[i]->invalidate();
            enemyWidgets[i]->setVisible(false);
            enemyWidgets[i]->invalidate();

            bulletActive = false;
            player_bullet.invalidate();
            player_bullet.setVisible(false);
            player_bullet.invalidate();

            score += 10;

            updateScoreText();
            updateHighScoreText();
        }

        if (enemyAlive[i])
        {
            aliveCount++;
        }
    }

    // ── 8. Qua wave / sang boss ───────────────────────
    if (aliveCount == 0)
    {
        waveLevel++;
        spawnNextWave();

        if (isGameEnded)
        {
            return;
        }
    }
}
void Screen2View::cleanupGameObjects()
{
    // Dừng trạng thái logic
    bulletActive = false;
    eBulletActive = false;
    touchLeft = false;
    touchRight = false;

    // Ẩn đạn người chơi
    player_bullet.invalidate();
    player_bullet.setVisible(false);
    player_bullet.invalidate();

    // Ẩn đạn enemy
    enemy_bullet.invalidate();
    enemy_bullet.setVisible(false);
    enemy_bullet.invalidate();

    // Ẩn toàn bộ enemy
    for (int i = 0; i < NUM_ENEMIES_MAX; i++)
    {
        enemyAlive[i] = false;

        if (enemyWidgets[i] != 0)
        {
            enemyWidgets[i]->invalidate();
            enemyWidgets[i]->setVisible(false);
            enemyWidgets[i]->invalidate();
        }
    }

    // Tắt lớp nền phụ để tránh render thừa
    bg21.invalidate();
    bg22.invalidate();
    bg21.invalidate();
    bg22.invalidate();
}
