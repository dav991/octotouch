#include "MoveActivity.h"



MoveActivity::MoveActivity(Activity *parent): 
    window(nullptr)
{
    this->parent = parent;
    Glib::RefPtr<Gtk::Builder> builder = Gtk::Builder::create_from_file( Config::i()->getResourcesFolder() + "glade/moveWindow.glade" );
    builder->get_widget( "windowMove", window );
    builder->get_widget( "btnBack", btnBack );

    if( !validWidget( window, "windowStatus missing from webcamWindow.glade" ) ) return;
    if( !validWidget( btnBack, "btnBack missing from webcamWindow.glade" ) ) return;

    window->signal_delete_event().connect (sigc::mem_fun(this, &MoveActivity::windowDestroyed) );
    window->set_default_size( Config::i()->getDisplayWidth(), Config::i()->getDisplayHeight() );
    btnBack->signal_clicked().connect( sigc::mem_fun(this, &MoveActivity::backClicked) );
}

void MoveActivity::show()
{
    window->show();
}

void MoveActivity::hide()
{
    window->hide();
}

void MoveActivity::childActivityHidden( Activity *child )
{
    show();
}

bool MoveActivity::windowDestroyed( GdkEventAny* any_event )
{
    this->backClicked();
    return true;
}

void MoveActivity::backClicked()
{
    this->hide();
    parent->childActivityHidden(this);
}

MoveActivity::~MoveActivity()
{
    delete window;
}
