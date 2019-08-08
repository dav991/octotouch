#ifndef  OCTOLCD_TUNE_ACTIVITY
#define OCTOLCD_TUNE_ACTIVITY

#include <math.h>
#include <gtkmm.h>
#include <iomanip>
#include <vector>
#include <cpprest/http_client.h>
#include "Activity.h"
#include "Config.h"

class TuneActivity: public Activity
{
private:
    Gtk::Window *window;
    Gtk::Button *btnBack;
    Gtk::Button *btnTempCycleIncrements;
    Gtk::Button *btnCooldown;
    Gtk::Button *btnDecreaseNozzleTemp;
    Gtk::Button *btnIncreaseNozzleTemp;
    Gtk::Button *btnDecreaseBedTemp;
    Gtk::Button *btnIncreaseBedTemp;
    Gtk::Button *btnIncreaseFlow;
    Gtk::Button *btnDecreaseFlow;
    Gtk::Button *btnIncreaseFeed;
    Gtk::Button *btnDecreaseFeed;
    Gtk::Button *btnLoad;
    Gtk::Button *btnUnload;
    Gtk::Button *btnSwap;
    Gtk::Label *lblToolTemp;
    Gtk::Label *lblBedTemp;
    Gtk::Label *lblFlow;
    Gtk::Label *lblFeed;
    Gtk::Label *lblStatus;
    int incrementValue;
    int toolTargetValue;
    int bedTargetValue;
    int flowRate;
    int feedRate;
    void refreshData();
    sigc::connection periodicTaskConnection;
    void requestToolTarget();
    void requestBedTarget();
    void requestFlowRate();
    void requestFeedRate();
    Glib::Dispatcher statusDispatcher;
    std::string errorStatus;
    std::mutex errorStatusMutex;
public:
    TuneActivity(Activity *parent);
    void show();
    void hide();
    void childActivityHidden( Activity *child );
    bool windowDestroyed( GdkEventAny* any_event );
    void backClicked();
    bool periodicTask();
    void switchIncrement();
    void decrementToolTemp();
    void incrementToolTemp();
    void decrementBedTemp();
    void incrementBedTemp();
    void decrementFlow();
    void incrementFlow();
    void decrementFeed();
    void incrementFeed();
    void cooldown();
    void loadFilament();
    void unloadFilament();
    void swapFilament();
    void errorStatusUpdate();
    ~TuneActivity();
};

#endif
