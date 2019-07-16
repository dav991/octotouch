#include "MainActivity.h"

MainActivity::MainActivity(): window(nullptr)
{
    Glib::RefPtr<Gtk::Builder> builder = Gtk::Builder::create_from_file("glade/mainWindow.glade");
    builder->get_widget("windowMain", window);
    builder->get_widget("lblAppName", appName);
    if( nullptr != appName )
    {
        appName->set_text( Glib::ustring::compose("%1 %2.%3", appName->get_text(), Octotouch_VERSION_MAJOR, Octotouch_VERSION_MINOR) );
    }
}

void MainActivity::show()
{
    window->show();
}

void MainActivity::hide()
{
    window->hide();
}

int MainActivity::start(Glib::RefPtr< Gtk::Application > app)
{
    app->run(*window);
}

void MainActivity::childActivityHidden( Activity *child )
{
}

void MainActivity::windowDestroyed()
{

}

MainActivity::~MainActivity()
{
    delete window;
}
