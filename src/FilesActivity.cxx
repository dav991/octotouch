#include "FilesActivity.h"



FilesActivity::FilesActivity(Activity *parent): 
    window(nullptr)
{
    this->parent = parent;
    Glib::RefPtr<Gtk::Builder> builder = Gtk::Builder::create_from_file( Config::i()->getResourcesFolder() + "glade/filesWindow.glade" );
    builder->get_widget( "windowFiles", window );
    builder->get_widget( "btnBack", btnBack );
    builder->get_widget( "listBoxFiles", listBoxFiles);

    if( !validWidget( window, "windowFiles missing from filesWindow.glade" ) ) return;
    if( !validWidget( btnBack, "btnBack missing from filesWindow.glade" ) ) return;
    if( !validWidget( listBoxFiles, "listBoxFiles missing from filesWindow.glade" ) ) return;

    window->signal_delete_event().connect (sigc::mem_fun(this, &FilesActivity::windowDestroyed) );
    window->set_default_size( Config::i()->getDisplayWidth(), Config::i()->getDisplayHeight() );
    btnBack->signal_clicked().connect( sigc::mem_fun(this, &FilesActivity::backClicked) );
}

void FilesActivity::show()
{
    
    Gtk::Button* pButton = new Gtk::Button("_Something", true);
    pButton->get_style_context()->add_class("btn");
    pButton->get_style_context()->add_class("listItem");
    listBoxFiles->insert( *pButton, -1 );
    listBoxFiles->show_all_children();
    window->show();
}

void FilesActivity::hide()
{
    window->hide();
}

void FilesActivity::childActivityHidden( Activity *child )
{
    show();
}

bool FilesActivity::windowDestroyed( GdkEventAny* any_event )
{
    this->backClicked();
    return true;
}

void FilesActivity::backClicked()
{
    this->hide();
    parent->childActivityHidden(this);
}

FilesActivity::~FilesActivity()
{
    delete window;
}
