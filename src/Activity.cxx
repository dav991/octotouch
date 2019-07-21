#include "Activity.h"

bool Activity::validWidget(Gtk::Widget *widget, std::string hint)
{
	if(nullptr == widget)
	{
		std::cerr << hint << std::endl;
		Gtk::Main::quit();
		return false;
	}
	return true;
}

Activity::~Activity()
{
}
