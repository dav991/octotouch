#ifndef  OCTOLCD_FILES_ACTIVITY
#define OCTOLCD_FILES_ACTIVITY

#include <math.h>
#include <gtkmm.h>
#include <iomanip>
#include <list>
#include <cpprest/http_client.h>
#include "Activity.h"
#include "StatusActivity.h"
#include "Config.h"

class FileEntry {
public:
    std::string name;
    std::string path;
    long long timestamp;
    FileEntry( std::string name, std::string path, long long timestamp );
};

class FilesActivity: public Activity
{
private:
    int scrollWindowStart;
    int scrollItems;
    Gtk::Window *window;
    Gtk::Button *btnBack;
    Gtk::Label *lblStatus;
    Gtk::Grid *gridListWrapper;
    Activity *statusActivity;
    Glib::Dispatcher showStatusDispatcher;
    Glib::Dispatcher populateListDispatcher;
    std::vector<FileEntry> files;
    std::vector<Gtk::Label *> listLabels;
    Gtk::Button *btnScrollUp;
    Gtk::Button *btnScrollDown;
    Gtk::Button *btnScrollTop;
    void refreshData();
    void parseFiles( web::json::value files);
    void insertInOrder(FileEntry entry);
    void switchToStatus();
public:
    FilesActivity(Activity *parent);
    void show();
    void hide();
    void childActivityHidden( Activity *child );
    bool windowDestroyed( GdkEventAny* any_event );
    void backClicked();
    void clearList();
    void populateList();
    bool listItemClicked( GdkEventButton* button_event, int element );
    void onScrollUp();
    void onScrollDown();
    void onScrollTop();
    ~FilesActivity();
};

#endif
