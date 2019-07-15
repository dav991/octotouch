#include <stdio.h>
#include <gtkmm.h>
#include "MainActivity.h"

int main(int argc, char *argv[])
{
    auto app = Gtk::Application::create(argc, argv,
      "com.intshift.octolcd");

    Glib::RefPtr<Gtk::Builder> builder = Gtk::Builder::create_from_file("glade/ui.glade");

    Glib::RefPtr<Gtk::CssProvider> cssProvider = Gtk::CssProvider::create();
	cssProvider->load_from_path("css/style.css");

	Glib::RefPtr<Gtk::StyleContext> styleContext = Gtk::StyleContext::create();

	Glib::RefPtr<Gdk::Screen> screen = Gdk::Screen::get_default();

    styleContext->add_provider_for_screen(screen, cssProvider, GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);

    auto ma = new MainActivity(builder);
    int ret = ma->start(app);
    delete ma;
    return ret;
}
