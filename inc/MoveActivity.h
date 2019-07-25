#ifndef  OCTOLCD_MOVE_ACTIVITY
#define OCTOLCD_MOVE_ACTIVITY

#include <math.h>
#include <gtkmm.h>
#include <iomanip>
#include <cpprest/http_client.h>
#include "Activity.h"
#include "Config.h"

class MoveActivity: public Activity
{
private:
    Gtk::Window *window;
    Gtk::Button *btnBack;
public:
    MoveActivity(Activity *parent);
    void show();
    void hide();
    void childActivityHidden( Activity *child );
    bool windowDestroyed( GdkEventAny* any_event );
    void backClicked();
    ~MoveActivity();
};

#endif
