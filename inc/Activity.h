#ifndef  OCTOLCD_ACTIVITY
#define OCTOLCD_ACTIVITY

#include <gdk/gdk.h>
#include <gtkmm.h>
#include <iostream>

class Activity
{
public:
    virtual void childActivityHidden(Activity *child) = 0;
    virtual void show() = 0;
    virtual void hide() = 0;
    virtual bool windowDestroyed( GdkEventAny* any_event ) = 0;
    virtual ~Activity() = 0;
    bool validWidget(Gtk::Widget *widget, std::string hint);
};



#endif
