#include "MainActivity.h"

MainActivity::MainActivity( Glib::RefPtr<Gtk::Builder> builder ): window(nullptr)
{
    builder->get_widget("window_main", window);
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
