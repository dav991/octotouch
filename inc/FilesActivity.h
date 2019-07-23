#ifndef  OCTOLCD_FILES_ACTIVITY
#define OCTOLCD_FILES_ACTIVITY

#include <math.h>
#include <gtkmm.h>
#include <iomanip>
#include <cpprest/http_client.h>
#include "Activity.h"
#include "Config.h"

class FilesActivity: public Activity
{
private:
    Gtk::Window *window;
    Gtk::Button *btnBack;
    Gtk::ListBox *listBoxFiles;
public:
    FilesActivity(Activity *parent);
    void show();
    void hide();
    void childActivityHidden( Activity *child );
    bool windowDestroyed( GdkEventAny* any_event );
    void backClicked();
    void clearList();
    void addItemToList( std::string text );
    ~FilesActivity();
};

#endif
