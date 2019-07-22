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
    Activity *parent;
    Gtk::Label *lblStatus;
    Gtk::Label *lblFile;
    Gtk::Label *lblPrintTime;
    Gtk::Label *lblPrintTimeLeft;
    Gtk::Label *lblEstimatePrintTime;
    Gtk::ProgressBar *progressFile;
    Gtk::Button *btnPrint;
    Gtk::Button *btnPause;
    Gtk::Button *btnStop;
    Gtk::Button *btnTune;
    void refreshData();
    sigc::connection periodicTaskConnection;
    Activity *tuneActivity;
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
    ~StatusActivity();
};

#endif
