#ifndef SCREEN3PRESENTER_HPP
#define SCREEN3PRESENTER_HPP

#include <gui/model/ModelListener.hpp>
#include <mvp/Presenter.hpp>

using namespace touchgfx;

class Screen3View;

class Screen3Presenter : public touchgfx::Presenter, public ModelListener
{
public:
    Screen3Presenter(Screen3View& v);
    virtual void activate();
    virtual void deactivate();
    virtual ~Screen3Presenter() {}

    // ── HÀM LƯU VÀ LẤY ĐIỂM ──
    void saveGameResult(int score, bool victory);
    int getCurrentScore();
    int getHighScore();

private:
    Screen3Presenter();
    Screen3View& view;
};

#endif // SCREEN3PRESENTER_HPP
