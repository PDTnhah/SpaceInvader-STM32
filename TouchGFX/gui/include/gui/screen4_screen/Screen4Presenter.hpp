#ifndef SCREEN4PRESENTER_HPP
#define SCREEN4PRESENTER_HPP

#include <gui/model/ModelListener.hpp>
#include <mvp/Presenter.hpp>

using namespace touchgfx;

class Screen4View;

class Screen4Presenter : public touchgfx::Presenter, public ModelListener
{
public:
    Screen4Presenter(Screen4View& v);

    virtual void activate();
    virtual void deactivate();

    virtual ~Screen4Presenter() {}

    int getCurrentScore();
    int getHighScore();
    bool getIsVictory();

    void resetRun();

private:
    Screen4Presenter();

    Screen4View& view;
};

#endif // SCREEN4PRESENTER_HPP
