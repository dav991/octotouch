#include "WebcamActivity.h"



WebcamActivity::WebcamActivity(Activity *parent): 
    window(nullptr)
{
    this->parent = parent;
    Glib::RefPtr<Gtk::Builder> builder = Gtk::Builder::create_from_file( Config::i()->getResourcesFolder() + "glade/webcamWindow.glade" );
    builder->get_widget( "windowWebcam", window );
    builder->get_widget( "btnBack", btnBack );

    if( !validWidget( window, "windowStatus missing from webcamWindow.glade" ) ) return;
    if( !validWidget( btnBack, "btnBack missing from webcamWindow.glade" ) ) return;

    window->signal_delete_event().connect (sigc::mem_fun(this, &WebcamActivity::windowDestroyed) );
    window->set_default_size( Config::i()->getDisplayWidth(), Config::i()->getDisplayHeight() );
    btnBack->signal_clicked().connect( sigc::mem_fun(this, &WebcamActivity::backClicked) );
}

void WebcamActivity::show()
{
    window->show();
}

void WebcamActivity::hide()
{
    window->hide();
}

void WebcamActivity::childActivityHidden( Activity *child )
{
    show();
}

bool WebcamActivity::windowDestroyed( GdkEventAny* any_event )
{
    this->backClicked();
    return true;
}

void WebcamActivity::backClicked()
{
    this->hide();
    parent->childActivityHidden(this);
}

WebcamActivity::~WebcamActivity()
{
    delete window;
}
