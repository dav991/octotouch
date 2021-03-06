#ifndef  OCTOLCD_MAIN_ACTIVITY
#define OCTOLCD_MAIN_ACTIVITY

#include <gtkmm.h>
#include "Activity.h"
#include "Version.h"
#include "StatusActivity.h"
#include "WebcamActivity.h"
#include "FilesActivity.h"
#include "MoveActivity.h"

class MainActivity: public Activity
{
private:
    Gtk::Window *window;
    Gtk::Label *lblAppName;
    Gtk::Button *btnStatus;
    Gtk::Button *btnWebcam;
    Gtk::Button *btnFiles;
    Gtk::Button *btnMove;
// Child activities
    Activity *statusActivity;
    Activity *webcamActivity;
    Activity *filesActivity;
    Activity *moveActivity;
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
    void webcamClicked();
    void filesClicked();
    void moveClicked();
    ~MainActivity();
};

#endif
