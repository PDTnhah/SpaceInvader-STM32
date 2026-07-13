#include <gui/screen4_screen/Screen4View.hpp>
#include <touchgfx/Unicode.hpp>
#include <gui/common/FrontendApplication.hpp>

Screen4View::Screen4View()
{
}

void Screen4View::setupScreen()
{
    Screen4ViewBase::setupScreen();

    updateResultUI();
}

void Screen4View::tearDownScreen()
{
    Screen4ViewBase::tearDownScreen();
}

void Screen4View::updateResultUI()
{
    // 1. Lấy điểm và trạng thái từ Model qua Presenter
    int finalScore = presenter->getCurrentScore();
    int highScore  = presenter->getHighScore();
    bool isVictory = presenter->getIsVictory();

    // 2. Cập nhật Final Score
    Unicode::snprintf(txtFinalScoreBuffer, TXTFINALSCORE_SIZE, "%d", finalScore);
    txtFinalScore.setWildcard1(txtFinalScoreBuffer);
    txtFinalScore.invalidate();

    // 3. Cập nhật High Score
    Unicode::snprintf(txtHighScoreBuffer, TXTHIGHSCORE_SIZE, "%d", highScore);
    txtHighScore.setWildcard1(txtHighScoreBuffer);
    txtHighScore.invalidate();

    // 4. Ẩn / hiện ảnh kết quả và nút
    if (isVictory)
    {
        imgVictory.setVisible(true);
        imgGameOver.setVisible(false);

        btnPlayAgain.setVisible(true);
        btnRetry.setVisible(false);
    }
    else
    {
        imgVictory.setVisible(false);
        imgGameOver.setVisible(true);

        btnPlayAgain.setVisible(false);
        btnRetry.setVisible(true);
    }

    // 5. Vẽ lại giao diện
    imgVictory.invalidate();
    imgGameOver.invalidate();

    btnPlayAgain.invalidate();
    btnRetry.invalidate();
}

void Screen4View::playAgainClicked()
{
    // Reset lượt chơi mới nhưng giữ highScore
    presenter->resetRun();

    // Quay lại màn chơi lính
    static_cast<FrontendApplication*>(Application::getInstance())->gotoScreen2ScreenNoTransition();
}

void Screen4View::retryClicked()
{
    // Reset lượt chơi mới nhưng giữ highScore
    presenter->resetRun();

    // Retry cũng quay lại màn chơi lính
    static_cast<FrontendApplication*>(Application::getInstance())->gotoScreen2ScreenNoTransition();
}
