#include "WebcamActivity.h"



WebcamActivity::WebcamActivity(Activity *parent): 
    window(nullptr)
{
    this->parent = parent;
    Glib::RefPtr<Gtk::Builder> builder = Gtk::Builder::create_from_file( Config::i()->getResourcesFolder() + "glade/webcamWindow.glade" );
    builder->get_widget( "windowWebcam", window );
    builder->get_widget( "btnBack", btnBack );
    builder->get_widget( "imageFrame", imageFrame );

    if( !validWidget( window, "windowStatus missing from webcamWindow.glade" ) ) return;
    if( !validWidget( btnBack, "btnBack missing from webcamWindow.glade" ) ) return;
    if( !validWidget( imageFrame, "imageFrame missing from webcamWindow.glade" ) ) return;

    window->signal_delete_event().connect (sigc::mem_fun(this, &WebcamActivity::windowDestroyed) );
    window->set_default_size( Config::i()->getDisplayWidth(), Config::i()->getDisplayHeight() );
    btnBack->signal_clicked().connect( sigc::mem_fun(this, &WebcamActivity::backClicked) );
}

void WebcamActivity::loadFrame()
{
    web::http::client::http_client api(web::http::uri_builder("https://upload.wikimedia.org/").append_path(U("wikipedia/en/a/a9/Example.jpg")).to_uri());
    web::http::http_request request(web::http::methods::GET);
    api.request(request)
        .then([=](web::http::http_response response)
        {
            if(response.status_code() < 200 || response.status_code() > 299)
            {
                //lblStatus->set_text(Glib::ustring::compose("Error: %1\n%2", response.status_code(), response.reason_phrase()));
                std::cerr << Glib::ustring::compose("Error: %1\n%2", response.status_code(), response.reason_phrase()) << std::endl;
                return;
            }
            std::vector< unsigned char > imageBytes = response.extract_vector().get();
            std::cerr << "We have " << imageBytes.size() << " bytes in the response " << std::endl;
            Glib::RefPtr< Gdk::PixbufLoader > pixBufLoader = Gdk::PixbufLoader::create("jpeg");
            pixBufLoader->write( imageBytes.data(), imageBytes.size() );
            imageFrame->set( pixBufLoader->get_pixbuf() );
            pixBufLoader->close();
        })
        .then([=] (pplx::task<void> previous_task) mutable {
			if (previous_task._GetImpl()->_HasUserException()) {
				try {
					auto holder = previous_task._GetImpl()->_GetExceptionHolder();
					holder->_RethrowUserException();
				} catch (std::exception& e) {
					/* lblStatus->set_text(
						Glib::ustring::compose( "Error: %1", e.what())
                    );*/
					std::cerr << "Exception: " << e.what() << std::endl;
				}
			}
		});
}

void WebcamActivity::show()
{
    window->show();
    loadFrame();
}

void WebcamActivity::hide()
{
    window->hide();
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

WebcamActivity::~WebcamActivity()
{
    delete window;
}
