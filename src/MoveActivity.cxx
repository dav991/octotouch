#include "MoveActivity.h"



MoveActivity::MoveActivity(Activity *parent): 
    window(nullptr),
    incrementStep(0)
{
    this->parent = parent;
    tuneActivity = new TuneActivity(this);
    Glib::RefPtr<Gtk::Builder> builder = Gtk::Builder::create_from_file( Config::i()->getResourcesFolder() + "glade/moveWindow.glade" );
    builder->get_widget( "windowMove", window );
    builder->get_widget( "btnBack", btnBack );
    
    
    builder->get_widget( "btnIncrement", btnIncrement );
    builder->get_widget( "btnAdjust", btnAdjust );
    builder->get_widget( "btnXLeft", btnXLeft );
    builder->get_widget( "btnXRight", btnXRight );
    builder->get_widget( "btnYBack", btnYBack );
    builder->get_widget( "btnYForward", btnYForward );
    builder->get_widget( "btnXYHome", btnXYHome );
    builder->get_widget( "btnZUp", btnZUp );
    builder->get_widget( "btnZDown", btnZDown );
    builder->get_widget( "btnZHome", btnZHome );
    builder->get_widget( "lblStatus", lblStatus );

    if( !validWidget( window, "windowStatus missing from moveWindow.glade" ) ) return;
    if( !validWidget( btnBack, "btnBack missing from moveWindow.glade" ) ) return;
    if( !validWidget( btnIncrement, "btnIncrement missing from moveWindow.glade" ) ) return;
    if( !validWidget( btnAdjust, "btnAdjust missing from moveWindow.glade" ) ) return;
    if( !validWidget( btnXLeft, "btnXLeft missing from moveWindow.glade" ) ) return;
    if( !validWidget( btnXRight, "btnXRight missing from moveWindow.glade" ) ) return;
    if( !validWidget( btnYBack, "btnYBack missing from moveWindow.glade" ) ) return;
    if( !validWidget( btnYForward, "btnYForward missing from moveWindow.glade" ) ) return;
    if( !validWidget( btnXYHome, "btnXYHome missing from moveWindow.glade" ) ) return;
    if( !validWidget( btnZUp, "btnZUp missing from moveWindow.glade" ) ) return;
    if( !validWidget( btnZDown, "btnZDown missing from moveWindow.glade" ) ) return;
    if( !validWidget( btnZHome, "btnZHome missing from moveWindow.glade" ) ) return;
    if( !validWidget( lblStatus, "btnZHome missing from moveWindow.glade" ) ) return;

    window->signal_delete_event().connect (sigc::mem_fun(this, &MoveActivity::windowDestroyed) );
    window->set_default_size( Config::i()->getDisplayWidth(), Config::i()->getDisplayHeight() );
    btnBack->signal_clicked().connect( sigc::mem_fun(this, &MoveActivity::backClicked) );

    btnIncrement->signal_clicked().connect( sigc::mem_fun( this, &MoveActivity::onBtnIncrement) );
    btnAdjust->signal_clicked().connect( sigc::mem_fun( this, &MoveActivity::onBtnAdjust) );
    btnXLeft->signal_clicked().connect( sigc::mem_fun( this, &MoveActivity::onBtnXLeft) );
    btnXRight->signal_clicked().connect( sigc::mem_fun( this, &MoveActivity::onBtnXRight) );
    btnYBack->signal_clicked().connect( sigc::mem_fun( this, &MoveActivity::onBtnYBack) );
    btnYForward->signal_clicked().connect( sigc::mem_fun( this, &MoveActivity::onBtnYForward) );
    btnXYHome->signal_clicked().connect( sigc::mem_fun( this, &MoveActivity::onBtnXYHome) );
    btnZUp->signal_clicked().connect( sigc::mem_fun( this, &MoveActivity::onBtnZUp) );
    btnZDown->signal_clicked().connect( sigc::mem_fun( this, &MoveActivity::onBtnZDown) );
    btnZHome->signal_clicked().connect( sigc::mem_fun( this, &MoveActivity::onBtnZHome) );
    statusDispatcher.connect( sigc::mem_fun( this, &MoveActivity::errorStatusUpdate ) );
    onBtnIncrement();
}

void MoveActivity::show()
{
    window->show();
}

void MoveActivity::hide()
{
    window->hide();
}

void MoveActivity::childActivityHidden( Activity *child )
{
    show();
}

bool MoveActivity::windowDestroyed( GdkEventAny* any_event )
{
    this->backClicked();
    return true;
}

void MoveActivity::backClicked()
{
    this->hide();
    parent->childActivityHidden(this);
}

void MoveActivity::onBtnIncrement()
{
    switch( incrementStep )
    {
        case 1:
            incrementStep = 2;
            break;
        case 2:
            incrementStep = 5;
            break;
        case 5:
            incrementStep = 10;
            break;
        case 10:
            incrementStep = 20;
            break;
        case 20:
            incrementStep = 50;
            break;
        default:
            incrementStep = 1;
            break;
    }
    btnIncrement->set_label( Glib::ustring::compose("Inc: %1mm", incrementStep) );
}

void MoveActivity::onBtnAdjust()
{
    hide();
    tuneActivity->show();
}

void MoveActivity::onBtnXLeft()
{
    utility::string_t address = U(Config::i()->getHost());
    web::http::uri uri = web::http::uri(address);
    web::http::client::http_client api(web::http::uri_builder(uri).append_path(U("api/printer/printhead")).to_uri());
    web::json::value requestBody = web::json::value::object();
    requestBody[U("command")] = web::json::value::string("jog");
    requestBody[U("x")] = web::json::value::number(-incrementStep);
    requestBody[U("y")] = web::json::value::number(0);
    requestBody[U("z")] = web::json::value::number(0);
    utility::stringstream_t stream;
    requestBody.serialize(stream);
    web::http::http_request request(web::http::methods::POST);
    request.set_body(stream.str(), utf8string("application/json"));
    request.headers().add(U("X-Api-Key"), U(Config::i()->getApiKey()));
    api.request(request)
		.then([=](web::http::http_response response)
        {
            if(response.status_code() < 200 || response.status_code() > 299)
            {
                std::lock_guard<std::mutex> lock( errorStatusMutex );
                errorStatus = Glib::ustring::compose("Error: %1\n%2", response.status_code(), response.reason_phrase());
                statusDispatcher.emit();
                std::cerr << "MoveActivity::onBtnXLeft error: " << 
                    Glib::ustring::compose("Error: %1\n%2", response.status_code(), response.reason_phrase()) << std::endl;
                return;
            }
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

void MoveActivity::onBtnXRight()
{
    utility::string_t address = U(Config::i()->getHost());
    web::http::uri uri = web::http::uri(address);
    web::http::client::http_client api(web::http::uri_builder(uri).append_path(U("api/printer/printhead")).to_uri());
    web::json::value requestBody = web::json::value::object();
    requestBody[U("command")] = web::json::value::string("jog");
    requestBody[U("x")] = web::json::value::number(incrementStep);
    requestBody[U("y")] = web::json::value::number(0);
    requestBody[U("z")] = web::json::value::number(0);
    utility::stringstream_t stream;
    requestBody.serialize(stream);
    web::http::http_request request(web::http::methods::POST);
    request.set_body(stream.str(), utf8string("application/json"));
    request.headers().add(U("X-Api-Key"), U(Config::i()->getApiKey()));
    api.request(request)
		.then([=](web::http::http_response response)
        {
            if(response.status_code() < 200 || response.status_code() > 299)
            {
                std::lock_guard<std::mutex> lock( errorStatusMutex );
                errorStatus = Glib::ustring::compose("Error: %1\n%2", response.status_code(), response.reason_phrase());
                statusDispatcher.emit();
                std::cerr << "MoveActivity::onBtnXLeft error: " << 
                    Glib::ustring::compose("Error: %1\n%2", response.status_code(), response.reason_phrase()) << std::endl;
                return;
            }
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

void MoveActivity::onBtnYBack()
{
    utility::string_t address = U(Config::i()->getHost());
    web::http::uri uri = web::http::uri(address);
    web::http::client::http_client api(web::http::uri_builder(uri).append_path(U("api/printer/printhead")).to_uri());
    web::json::value requestBody = web::json::value::object();
    requestBody[U("command")] = web::json::value::string("jog");
    requestBody[U("x")] = web::json::value::number(0);
    requestBody[U("y")] = web::json::value::number(-incrementStep);
    requestBody[U("z")] = web::json::value::number(0);
    utility::stringstream_t stream;
    requestBody.serialize(stream);
    web::http::http_request request(web::http::methods::POST);
    request.set_body(stream.str(), utf8string("application/json"));
    request.headers().add(U("X-Api-Key"), U(Config::i()->getApiKey()));
    api.request(request)
		.then([=](web::http::http_response response)
        {
            if(response.status_code() < 200 || response.status_code() > 299)
            {
                std::lock_guard<std::mutex> lock( errorStatusMutex );
                errorStatus = Glib::ustring::compose("Error: %1\n%2", response.status_code(), response.reason_phrase());
                statusDispatcher.emit();
                std::cerr << "MoveActivity::onBtnXLeft error: " << 
                    Glib::ustring::compose("Error: %1\n%2", response.status_code(), response.reason_phrase()) << std::endl;
                return;
            }
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

void MoveActivity::onBtnYForward()
{
    utility::string_t address = U(Config::i()->getHost());
    web::http::uri uri = web::http::uri(address);
    web::http::client::http_client api(web::http::uri_builder(uri).append_path(U("api/printer/printhead")).to_uri());
    web::json::value requestBody = web::json::value::object();
    requestBody[U("command")] = web::json::value::string("jog");
    requestBody[U("x")] = web::json::value::number(0);
    requestBody[U("y")] = web::json::value::number(incrementStep);
    requestBody[U("z")] = web::json::value::number(0);
    utility::stringstream_t stream;
    requestBody.serialize(stream);
    web::http::http_request request(web::http::methods::POST);
    request.set_body(stream.str(), utf8string("application/json"));
    request.headers().add(U("X-Api-Key"), U(Config::i()->getApiKey()));
    api.request(request)
		.then([=](web::http::http_response response)
        {
            if(response.status_code() < 200 || response.status_code() > 299)
            {
                std::lock_guard<std::mutex> lock( errorStatusMutex );
                errorStatus =Glib::ustring::compose("Error: %1\n%2", response.status_code(), response.reason_phrase());
                statusDispatcher.emit();
                std::cerr << "MoveActivity::onBtnXLeft error: " << 
                    Glib::ustring::compose("Error: %1\n%2", response.status_code(), response.reason_phrase()) << std::endl;
                return;
            }
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

void MoveActivity::onBtnXYHome()
{
    std::vector<web::json::value> commands = {web::json::value::string("x"), web::json::value::string("y")};
    utility::string_t address = U(Config::i()->getHost());
    web::http::uri uri = web::http::uri(address);
    web::http::client::http_client api(web::http::uri_builder(uri).append_path(U("api/printer/printhead")).to_uri());
    web::json::value requestBody = web::json::value::object();
    requestBody[U("command")] = web::json::value::string("home");
    requestBody[U("axes")] = web::json::value::array(commands);
    utility::stringstream_t stream;
    requestBody.serialize(stream);
    web::http::http_request request(web::http::methods::POST);
    request.set_body(stream.str(), utf8string("application/json"));
    request.headers().add(U("X-Api-Key"), U(Config::i()->getApiKey()));
    api.request(request)
		.then([=](web::http::http_response response)
        {
            if(response.status_code() < 200 || response.status_code() > 299)
            {
                std::lock_guard<std::mutex> lock( errorStatusMutex );
                errorStatus = Glib::ustring::compose("Error: %1\n%2", response.status_code(), response.reason_phrase());
                statusDispatcher.emit();
                std::cerr << "MoveActivity::onBtnXLeft error: " << 
                    Glib::ustring::compose("Error: %1\n%2", response.status_code(), response.reason_phrase()) << std::endl;
                return;
            }
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

void MoveActivity::onBtnZUp()
{
    utility::string_t address = U(Config::i()->getHost());
    web::http::uri uri = web::http::uri(address);
    web::http::client::http_client api(web::http::uri_builder(uri).append_path(U("api/printer/printhead")).to_uri());
    web::json::value requestBody = web::json::value::object();
    requestBody[U("command")] = web::json::value::string("jog");
    requestBody[U("x")] = web::json::value::number(0);
    requestBody[U("y")] = web::json::value::number(0);
    requestBody[U("z")] = web::json::value::number(incrementStep);
    utility::stringstream_t stream;
    requestBody.serialize(stream);
    web::http::http_request request(web::http::methods::POST);
    request.set_body(stream.str(), utf8string("application/json"));
    request.headers().add(U("X-Api-Key"), U(Config::i()->getApiKey()));
    api.request(request)
		.then([=](web::http::http_response response)
        {
            if(response.status_code() < 200 || response.status_code() > 299)
            {
                std::lock_guard<std::mutex> lock( errorStatusMutex );
                errorStatus = Glib::ustring::compose("Error: %1\n%2", response.status_code(), response.reason_phrase());
                statusDispatcher.emit();
                /*lblStatus->set_text(Glib::ustring::compose("Error: %1\n%2", response.status_code(), response.reason_phrase()));*/
                std::cerr << "MoveActivity::onBtnXLeft error: " << 
                    Glib::ustring::compose("Error: %1\n%2", response.status_code(), response.reason_phrase()) << std::endl;
                return;
            }
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

void MoveActivity::onBtnZDown()
{
    utility::string_t address = U(Config::i()->getHost());
    web::http::uri uri = web::http::uri(address);
    web::http::client::http_client api(web::http::uri_builder(uri).append_path(U("api/printer/printhead")).to_uri());
    web::json::value requestBody = web::json::value::object();
    requestBody[U("command")] = web::json::value::string("jog");
    requestBody[U("x")] = web::json::value::number(0);
    requestBody[U("y")] = web::json::value::number(0);
    requestBody[U("z")] = web::json::value::number(-incrementStep);
    utility::stringstream_t stream;
    requestBody.serialize(stream);
    web::http::http_request request(web::http::methods::POST);
    request.set_body(stream.str(), utf8string("application/json"));
    request.headers().add(U("X-Api-Key"), U(Config::i()->getApiKey()));
    api.request(request)
		.then([=](web::http::http_response response)
        {
            if(response.status_code() < 200 || response.status_code() > 299)
            {
                std::lock_guard<std::mutex> lock( errorStatusMutex );
                errorStatus =Glib::ustring::compose("Error: %1\n%2", response.status_code(), response.reason_phrase());
                statusDispatcher.emit();
                std::cerr << "MoveActivity::onBtnXLeft error: " << 
                    Glib::ustring::compose("Error: %1\n%2", response.status_code(), response.reason_phrase()) << std::endl;
                return;
            }
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

void MoveActivity::onBtnZHome()
{
    std::vector<web::json::value> commands = {web::json::value::string("z")};
    utility::string_t address = U(Config::i()->getHost());
    web::http::uri uri = web::http::uri(address);
    web::http::client::http_client api(web::http::uri_builder(uri).append_path(U("api/printer/printhead")).to_uri());
    web::json::value requestBody = web::json::value::object();
    requestBody[U("command")] = web::json::value::string("home");
    requestBody[U("axes")] = web::json::value::array(commands);
    utility::stringstream_t stream;
    requestBody.serialize(stream);
    web::http::http_request request(web::http::methods::POST);
    request.set_body(stream.str(), utf8string("application/json"));
    request.headers().add(U("X-Api-Key"), U(Config::i()->getApiKey()));
    api.request(request)
		.then([=](web::http::http_response response)
        {
            if(response.status_code() < 200 || response.status_code() > 299)
            {
                std::lock_guard<std::mutex> lock( errorStatusMutex );
                errorStatus = Glib::ustring::compose("Error: %1\n%2", response.status_code(), response.reason_phrase());
                statusDispatcher.emit();
                std::cerr << "MoveActivity::onBtnXLeft error: " << 
                    Glib::ustring::compose("Error: %1\n%2", response.status_code(), response.reason_phrase()) << std::endl;
                return;
            }
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

void MoveActivity::errorStatusUpdate()
{
    std::lock_guard<std::mutex> lock( errorStatusMutex );
    lblStatus->set_text( errorStatus );
}

MoveActivity::~MoveActivity()
{
    delete window;
    delete tuneActivity;
}
