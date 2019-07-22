#ifndef  OCTOLCD_WEBCAM_ACTIVITY
#define OCTOLCD_WEBCAM_ACTIVITY

#include <math.h>
#include <gtkmm.h>
#include <iomanip>
#include <cpprest/http_client.h>
#include "Activity.h"
#include "TuneActivity.h"
#include "Config.h"

class WebcamActivity: public Activity
{
private:
    Gtk::Window *window;
    Gtk::Button *btnBack;
    Activity *parent;
public:
    WebcamActivity(Activity *parent);
    void show();
    void hide();
    void childActivityHidden( Activity *child );
    bool windowDestroyed( GdkEventAny* any_event );
    void backClicked();
    ~WebcamActivity();
};

#endif
