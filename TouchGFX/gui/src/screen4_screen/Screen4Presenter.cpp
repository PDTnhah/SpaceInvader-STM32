#include <gui/screen4_screen/Screen4View.hpp>
#include <gui/screen4_screen/Screen4Presenter.hpp>

Screen4Presenter::Screen4Presenter(Screen4View& v)
    : view(v)
{
}

void Screen4Presenter::activate()
{
}

void Screen4Presenter::deactivate()
{
}

int Screen4Presenter::getCurrentScore()
{
    return model->getCurrentScore();
}

int Screen4Presenter::getHighScore()
{
    return model->getHighScore();
}

bool Screen4Presenter::getIsVictory()
{
    return model->getIsVictory();
}

void Screen4Presenter::resetRun()
{
    model->resetRun();
}
