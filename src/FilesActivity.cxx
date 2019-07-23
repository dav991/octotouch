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
    addItemToList( "test1" );
    addItemToList( "test2" );
    listBoxFiles->show_all_children();
    window->show();
}

void FilesActivity::hide()
{
    window->hide();
    clearList();
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

void FilesActivity::clearList()
{
    std::vector<Gtk::Widget*> children = listBoxFiles->get_children();
    for( auto it = children.begin(); it!= children.end(); it++ )
    {
        listBoxFiles->remove(**it);
    }
}

void FilesActivity::addItemToList( std::string text )
{
    auto button = Gtk::manage( new Gtk::Button( text ) );
    button->get_style_context()->add_class("btn");
    button->get_style_context()->add_class("listItem");
    listBoxFiles->append( *button );
}

FilesActivity::~FilesActivity()
{
    delete window;
}
