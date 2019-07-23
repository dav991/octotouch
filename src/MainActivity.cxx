#include "MainActivity.h"

MainActivity::MainActivity( Glib::RefPtr< Gtk::Application > app ): window(nullptr)
{
    this->app = app;
    app->hold();
    statusActivity = new StatusActivity(this);
    webcamActivity = new WebcamActivity(this);
    filesActivity = new FilesActivity(this);
    Glib::RefPtr<Gtk::Builder> builder = Gtk::Builder::create_from_file( Config::i()->getResourcesFolder() + "glade/mainWindow.glade" );
    builder->get_widget( "windowMain", window );
    builder->get_widget( "lblAppName", lblAppName );
    builder->get_widget( "btnStatus", btnStatus );
    builder->get_widget( "btnWebcam", btnWebcam );
    builder->get_widget( "btnFiles", btnFiles );
    if( !validWidget( window, "windowMain missing from mainWindow.glade" ) ) return;
    if( !validWidget( lblAppName, "lblAppName missing from mainWindow.glade" ) ) return;
    if( !validWidget( btnStatus, "btnStatus missing from mainWindow.glade" ) ) return;
    if( !validWidget( btnWebcam, "btnWebcam missing from mainWindow.glade" ) ) return;
    if( !validWidget( btnFiles, "btnFiles missing from mainWindow.glade" ) ) return;
    window->signal_delete_event().connect( sigc::mem_fun( this, &MainActivity::windowDestroyed ) );
    window->set_default_size( Config::i()->getDisplayWidth(), Config::i()->getDisplayHeight() );
    lblAppName->set_text( Glib::ustring::compose("%1 %2.%3", lblAppName->get_text(), Octotouch_VERSION_MAJOR, Octotouch_VERSION_MINOR) );
    btnStatus->signal_clicked().connect( sigc::mem_fun( this, &MainActivity::statusClicked ) );
    btnWebcam->signal_clicked().connect( sigc::mem_fun( this, &MainActivity::webcamClicked ) );
	btnFiles->signal_clicked().connect( sigc::mem_fun( this, &MainActivity::filesClicked ) );
}

void MainActivity::show()
{
    window->show();
}

void MainActivity::hide()
{
    window->hide();
}

int MainActivity::start()
{
    app->run(*window);
}

void MainActivity::childActivityHidden( Activity *child )
{
    this->show();
}

bool MainActivity::windowDestroyed( GdkEventAny* any_event )
{
    this->hide();
    app->release();
    return true;
}

void MainActivity::statusClicked()
{
    this->hide();
    this->statusActivity->show();
}

void MainActivity::webcamClicked()
{
    this->hide();
    this->webcamActivity->show();
}

void MainActivity::filesClicked()
{
	this->hide();
	this->filesActivity->show();
}

void MainActivity::notify( std::string notification, std::string value )
{
    if( notification.compare("file_selected") == 0 )
    {
        this->filesActivity->show();
    }
}

MainActivity::~MainActivity()
{
    delete window;
    delete statusActivity;
    delete webcamActivity;
    delete filesActivity;
}
