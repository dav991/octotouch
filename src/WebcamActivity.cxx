#include "WebcamActivity.h"



WebcamActivity::WebcamActivity(Activity *parent): 
    window(nullptr)
{
    this->parent = parent;
    Glib::RefPtr<Gtk::Builder> builder = Gtk::Builder::create_from_file( Config::i()->getResourcesFolder() + "glade/webcamWindow.glade" );
    builder->get_widget( "windowWebcam", window );
    builder->get_widget( "btnBack", btnBack );
    builder->get_widget( "imageFrame", imageFrame );
    builder->get_widget( "lblStatus", lblStatus );

    if( !validWidget( window, "windowStatus missing from webcamWindow.glade" ) ) return;
    if( !validWidget( btnBack, "btnBack missing from webcamWindow.glade" ) ) return;
    if( !validWidget( imageFrame, "imageFrame missing from webcamWindow.glade" ) ) return;
    if( !validWidget( lblStatus, "lblStatus missing from webcamWindow.glade" ) ) return;

    window->signal_delete_event().connect (sigc::mem_fun(this, &WebcamActivity::windowDestroyed) );
    window->set_default_size( Config::i()->getDisplayWidth(), Config::i()->getDisplayHeight() );
    btnBack->signal_clicked().connect( sigc::mem_fun(this, &WebcamActivity::backClicked) );
    statusDispatcher.connect( sigc::mem_fun( this, &WebcamActivity::errorStatusUpdate ) );
}

void WebcamActivity::loadFrame()
{
    web::http::client::http_client api(web::http::uri_builder(Config::i()->getWebcamSnapUrl()).to_uri());
    web::http::http_request request(web::http::methods::GET);
    api.request(request)
        .then([=](web::http::http_response response)
        {
            if(response.status_code() < 200 || response.status_code() > 299)
            {
                std::lock_guard<std::mutex> lock( errorStatusMutex );
                errorStatus = Glib::ustring::compose("Error: %1\n%2", response.status_code(), response.reason_phrase());
                statusDispatcher.emit();
                std::cerr << Glib::ustring::compose("Error: %1\n%2", response.status_code(), response.reason_phrase()) << std::endl;
                return;
            }
            std::vector< unsigned char > imageBytes = response.extract_vector().get();
            Glib::RefPtr< Gdk::PixbufLoader > pixBufLoader = Gdk::PixbufLoader::create("jpeg");
            pixBufLoader->write( imageBytes.data(), imageBytes.size() );
            auto size = imageFrame->get_allocation();
            imageFrame->set( pixBufLoader->get_pixbuf()->scale_simple( size.get_width()-10, size.get_height()-10, Gdk::InterpType::INTERP_BILINEAR ) );
            pixBufLoader->close();
        })
        .then([=] (pplx::task<void> previous_task) mutable {
            if (previous_task._GetImpl()->_HasUserException()) {
                try {
                    auto holder = previous_task._GetImpl()->_GetExceptionHolder();
                    holder->_RethrowUserException();
                } catch (std::exception& e) {
                    std::lock_guard<std::mutex> lock( errorStatusMutex );
                    errorStatus = Glib::ustring::compose( "Error: %1", e.what());
                    statusDispatcher.emit();
                    std::cerr << "Exception: " << e.what() << std::endl;
                }
            }
        });
}

bool WebcamActivity::periodicTask()
{
    loadFrame();
    return true;
}

void WebcamActivity::show()
{
    window->show();
    periodicTaskConnection = Glib::signal_timeout().connect_seconds(sigc::mem_fun(*this, &WebcamActivity::periodicTask), 1);
}

void WebcamActivity::hide()
{
    window->hide();
    periodicTaskConnection.disconnect();
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

void WebcamActivity::errorStatusUpdate()
{
    std::lock_guard<std::mutex> lock( errorStatusMutex );
    lblStatus->set_text( errorStatus );
}

WebcamActivity::~WebcamActivity()
{
    delete window;
}
