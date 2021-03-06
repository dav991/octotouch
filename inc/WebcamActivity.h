#ifndef  OCTOLCD_WEBCAM_ACTIVITY
#define OCTOLCD_WEBCAM_ACTIVITY

#include <math.h>
#include <gtkmm.h>
#include <iomanip>
#include <cpprest/http_client.h>
#include "Activity.h"
#include "Config.h"

class WebcamActivity: public Activity
{
private:
    Gtk::Window *window;
    Gtk::Button *btnBack;
    Gtk::Image *imageFrame;
    Gtk::Label *lblStatus;
    Glib::Dispatcher statusDispatcher;
    std::string errorStatus;
    std::mutex errorStatusMutex;
    sigc::connection periodicTaskConnection;
    Glib::RefPtr<Gdk::Pixbuf> webcamSnap;
    std::mutex webcamMutex;
    Glib::Dispatcher webcamDispatcher;
public:
    WebcamActivity(Activity *parent);
    void show();
    void hide();
    void childActivityHidden( Activity *child );
    bool windowDestroyed( GdkEventAny* any_event );
    void backClicked();
    void loadFrame();
    void errorStatusUpdate();
    bool periodicTask();
    void loadWebcamImageToUI();
    ~WebcamActivity();
};

#endif
