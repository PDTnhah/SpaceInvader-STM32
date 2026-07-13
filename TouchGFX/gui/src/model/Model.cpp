#include <gui/model/Model.hpp>
#include <gui/model/ModelListener.hpp>

// Khởi tạo các biến mặc định khi mới bật nguồn mạch
Model::Model()
    : modelListener(0),
      currentScore(0),
      highScore(0),
      isVictory(false)
{
}

void Model::tick()
{
}

void Model::saveGameResult(int score, bool victory)
{
    currentScore = score;
    isVictory = victory;

    // Chỉ cập nhật highScore nếu điểm mới cao hơn
    if (score > highScore)
    {
        highScore = score;
    }
}

void Model::resetRun()
{
    // Reset lượt chơi mới
    // KHÔNG reset highScore
    currentScore = 0;
    isVictory = false;
}

int Model::getCurrentScore()
{
    return currentScore;
}

int Model::getHighScore()
{
    return highScore;
}

bool Model::getIsVictory()
{
    return isVictory;
}
