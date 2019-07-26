#ifndef  OCTOLCD_MOVE_ACTIVITY
#define OCTOLCD_MOVE_ACTIVITY

#include <math.h>
#include <gtkmm.h>
#include <iomanip>
#include <cpprest/http_client.h>
#include "Activity.h"
#include "TuneActivity.h"
#include "Config.h"

class MoveActivity: public Activity
{
private:
    Gtk::Window *window;
    Gtk::Button *btnBack;
    Gtk::Button *btnIncrement;
    Gtk::Button *btnAdjust;
    Gtk::Button *btnXLeft;
    Gtk::Button *btnXRight;
    Gtk::Button *btnYBack;
    Gtk::Button *btnYForward;
    Gtk::Button *btnXYHome;
    Gtk::Button *btnZUp;
    Gtk::Button *btnZDown;
    Gtk::Button *btnZHome;
    int incrementStep;
    Activity *tuneActivity;
public:
    MoveActivity(Activity *parent);
    void show();
    void hide();
    void childActivityHidden( Activity *child );
    bool windowDestroyed( GdkEventAny* any_event );
    void backClicked();
    void onBtnIncrement();
    void onBtnAdjust();
    void onBtnXLeft();
    void onBtnXRight();
    void onBtnYBack();
    void onBtnYForward();
    void onBtnXYHome();
    void onBtnZUp();
    void onBtnZDown();
    void onBtnZHome();
    ~MoveActivity();
};

#endif
