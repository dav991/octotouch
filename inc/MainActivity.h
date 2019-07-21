#ifndef  OCTOLCD_MAIN_ACTIVITY
#define OCTOLCD_MAIN_ACTIVITY

#include <gtkmm.h>
#include "Activity.h"
#include "Version.h"
#include "StatusActivity.h"

class MainActivity: public Activity
{
private:
    Gtk::Window *window;
    Gtk::Label *lblAppName;
    Gtk::Button *btnStatus;
// Child activities
    Activity *statusActivity;
// App reference
    Glib::RefPtr< Gtk::Application > app;
public:
    MainActivity( Glib::RefPtr< Gtk::Application > app );
    void show();
    void hide();
    int start();
    void childActivityHidden( Activity *child );
    bool windowDestroyed( GdkEventAny* any_event );
    void statusClicked();
    ~MainActivity();
};

#endif
