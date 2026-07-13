#ifndef MODEL_HPP
#define MODEL_HPP

class ModelListener;

class Model
{
public:
    Model();

    void bind(ModelListener* listener)
    {
        modelListener = listener;
    }

    void tick();

    // Lưu kết quả sau khi thua/thắng
    void saveGameResult(int score, bool victory);

    // Reset lượt chơi mới, nhưng giữ highScore
    void resetRun();

    int getCurrentScore();
    int getHighScore();
    bool getIsVictory();

protected:
    ModelListener* modelListener;

private:
    int currentScore;
    int highScore;
    bool isVictory;
};

#endif // MODEL_HPP
