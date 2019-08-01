#ifndef  OCTOLCD_STATUS_ACTIVITY
#define OCTOLCD_STATUS_ACTIVITY

#include <math.h>
#include <gtkmm.h>
#include <iomanip>
#include <cpprest/http_client.h>
#include "Activity.h"
#include "TuneActivity.h"
#include "Config.h"

class StatusActivity: public Activity
{
private:
    Gtk::Window *window;
    Gtk::Button *btnBack;
    Gtk::Label *lblStatus;
    std::string statusText;
    Gtk::Label *lblFile;
    std::string fileText;
    Gtk::Label *lblPrintTime;
    std::string printTimeText;
    Gtk::Label *lblPrintTimeLeft;
    std::string printTimeLeftText;
    Gtk::Label *lblEstimatePrintTime;
    std::string estimatePrintTimeText;
    Gtk::ProgressBar *progressFile;
    Gtk::Button *btnPrint;
    Gtk::Button *btnPause;
    Gtk::Button *btnStop;
    Gtk::Button *btnTune;
    void refreshData();
    sigc::connection periodicTaskConnection;
    Activity *tuneActivity;
    Glib::Dispatcher statusDispatcher;
    std::string errorStatus;
    std::mutex errorStatusMutex;
public:
    StatusActivity(Activity *parent);
    void show();
    void hide();
    void childActivityHidden( Activity *child );
    bool windowDestroyed( GdkEventAny* any_event );
    void backClicked();
    bool periodicTask();
    void startPrint();
    void pausePrint();
    void stopPrint();
    void tunePrint();
    void errorStatusUpdate();
    ~StatusActivity();
};

#endif
