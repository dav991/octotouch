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
    Gtk::Window *window;
    Gtk::Button *btnBack;
    Gtk::ListBox *listBoxFiles;
    Gtk::Label *lblStatus;
    Activity *statusActivity;
    Glib::Dispatcher showStatusDispatcher;
    std::vector<FileEntry> files;
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
    void addItemToList( FileEntry entry );
    void listItemClicked( std::string data );
    ~FilesActivity();
};

#endif
