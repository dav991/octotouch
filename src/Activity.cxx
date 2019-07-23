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
void Activity::notify( std::string notification, std::string value )
{
	if( nullptr == parent)
	{
		return;
	}
	parent->notify( notification, value );
}

Activity::~Activity()
{
}
