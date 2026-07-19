#include <gui/screen3_screen/Screen3View.hpp>
#include <gui/screen3_screen/Screen3Presenter.hpp>

Screen3Presenter::Screen3Presenter(Screen3View& v)
    : view(v)
{
}

void Screen3Presenter::activate()
{
}

void Screen3Presenter::deactivate()
{
}

void Screen3Presenter::saveGameResult(int score, bool victory)
{
    model->saveGameResult(score, victory);
}

int Screen3Presenter::getCurrentScore()
{
    return model->getCurrentScore();
}

int Screen3Presenter::getHighScore()
{
    return model->getHighScore();
}
