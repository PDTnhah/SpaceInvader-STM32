#ifndef SCREEN2PRESENTER_HPP
#define SCREEN2PRESENTER_HPP

#include <gui/model/ModelListener.hpp>
#include <mvp/Presenter.hpp>

using namespace touchgfx;

class Screen2View;

class Screen2Presenter : public touchgfx::Presenter, public ModelListener
{
public:
    Screen2Presenter(Screen2View& v);
    virtual void activate();
    virtual void deactivate();
    virtual ~Screen2Presenter() {}

    void saveGameResult(int score, bool victory);
    int getHighScore(); // <--- THÊM DÒNG NÀY VÀO ĐÂY

private:
    Screen2Presenter();
    Screen2View& view;
};

#endif // SCREEN2PRESENTER_HPP
