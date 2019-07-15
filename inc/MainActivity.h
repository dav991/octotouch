#ifndef  OCTOLCD_MAIN_ACTIVITY
#define OCTOLCD_MAIN_ACTIVITY

#include <gtkmm.h>
#include "Activity.h"
#include "Version.h"

class MainActivity: Activity
{
private:
    Gtk::Window *window;
public:
    MainActivity( Glib::RefPtr<Gtk::Builder> builder );
    void show();
    void hide();
    int start(Glib::RefPtr< Gtk::Application > app);
    void childActivityHidden( Activity *child );
    void windowDestroyed();
};

#endif
