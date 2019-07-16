#ifndef  OCTOLCD_MAIN_ACTIVITY
#define OCTOLCD_MAIN_ACTIVITY

#include <gtkmm.h>
#include "Activity.h"
#include "Version.h"

class MainActivity: Activity
{
private:
    Gtk::Window *window;
    Gtk::Label *appName;
public:
    MainActivity();
    void show();
    void hide();
    int start(Glib::RefPtr< Gtk::Application > app);
    void childActivityHidden( Activity *child );
    void windowDestroyed();
    ~MainActivity();
};

#endif
