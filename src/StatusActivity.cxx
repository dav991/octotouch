#include "StatusActivity.h"



StatusActivity::StatusActivity(Activity *parent): 
    window(nullptr)
{
    this->parent = parent;
    tuneActivity = new TuneActivity(this);
    Glib::RefPtr<Gtk::Builder> builder = Gtk::Builder::create_from_file( Config::i()->getResourcesFolder() + "glade/statusWindow.glade" );
    builder->get_widget( "windowStatus", window );
    builder->get_widget( "btnBack", btnBack );
    builder->get_widget( "lblStatus", lblStatus );
    builder->get_widget( "lblFile", lblFile );
    builder->get_widget( "lblPrintTime", lblPrintTime );
    builder->get_widget( "lblPrintTimeLeft", lblPrintTimeLeft );
    builder->get_widget( "lblEstimatePrintTime", lblEstimatePrintTime );
    builder->get_widget( "progressFile", progressFile );
    builder->get_widget( "btnPrint", btnPrint );
    builder->get_widget( "btnPause", btnPause );
    builder->get_widget( "btnStop", btnStop );
    builder->get_widget( "btnTune", btnTune );

    if( !validWidget( window, "windowStatus missing from statusWindow.glade" ) ) return;
    if( !validWidget( btnBack, "btnBack missing from statusWindow.glade" ) ) return;
    if( !validWidget( lblStatus, "lblStatus missing from statusWindow.glade" ) ) return;
    if( !validWidget( lblFile, "lblFile missing from statusWindow.glade" ) ) return;
    if( !validWidget( lblPrintTime, "lblPrintTime missing from statusWindow.glade" ) ) return;
    if( !validWidget( lblPrintTimeLeft, "lblPrintTimeLeft missing from statusWindow.glade" ) ) return;
    if( !validWidget( lblEstimatePrintTime, "lblEstimatePrintTime missing from statusWindow.glade" ) ) return;
    if( !validWidget( progressFile, "progressFile missing from statusWindow.glade" ) ) return;
    if( !validWidget( btnPrint, "btnPrint missing from statusWindow.glade" ) ) return;
    if( !validWidget( btnPause, "btnPause missing from statusWindow.glade" ) ) return;
    if( !validWidget( btnStop, "btnStop missing from statusWindow.glade" ) ) return;
    if( !validWidget( btnTune, "btnTune missing from statusWindow.glade" ) ) return;

    window->signal_delete_event().connect (sigc::mem_fun(this, &StatusActivity::windowDestroyed) );
    window->set_default_size( Config::i()->getDisplayWidth(), Config::i()->getDisplayHeight() );
    btnBack->signal_clicked().connect( sigc::mem_fun(this, &StatusActivity::backClicked) );
    btnPrint->signal_clicked().connect( sigc::mem_fun(this, &StatusActivity::startPrint) );
    btnPause->signal_clicked().connect( sigc::mem_fun(this, &StatusActivity::pausePrint) );
    btnStop->signal_clicked().connect( sigc::mem_fun(this, &StatusActivity::stopPrint) );
    btnTune->signal_clicked().connect( sigc::mem_fun(this, &StatusActivity::tunePrint) );
}

void StatusActivity::show()
{
    window->show();
    refreshData();
    periodicTaskConnection = Glib::signal_timeout().connect_seconds(sigc::mem_fun(*this, &StatusActivity::periodicTask), 10);
}

void StatusActivity::refreshData()
{
    utility::string_t address = U(Config::i()->getHost());
    web::http::uri uri = web::http::uri(address);
    web::http::client::http_client api(web::http::uri_builder(uri).append_path(U("api/job")).to_uri());
    web::http::http_request request(web::http::methods::GET);
    request.headers().add(U("X-Api-Key"), U(Config::i()->getApiKey()));
    api.request(request)
        .then( [=](web::http::http_response  response)
        {
            if( response.status_code() < 200 || response.status_code() > 299 )
            {
                lblStatus->set_text(
                    Glib::ustring::compose( "Connection error: %1\n%2", response.status_code(), response.reason_phrase())
                );
                return;
            }
            auto json = response.extract_json().get();
            //lblFile update
            if( json["job"]["file"]["name"].is_null() )
            {
                lblFile->set_text( "File not selected" );
            }
            else
            {
                lblFile->set_text(json["job"]["file"]["name"].as_string());
            }
            //lblStatus update
            if( json["state"].is_null() )
            {
                lblStatus->set_text( "Error retriving status" );
            }
            else
            {
                lblStatus->set_text( Glib::ustring::compose( "Status: %1", json["state"].as_string() ) );
            }
            
            //lblPrintTime update
            int printTime = 0;
            if( json["progress"]["printTime"].is_null() )
            {
                lblPrintTime->set_text("Time: -:--");
            }
            else
            {
                printTime = json["progress"]["printTime"].as_integer();
                lblPrintTime->set_text(Glib::ustring::compose("Time: %1:%2",
                    printTime/3600,
                    Glib::ustring::format(std::setfill(L'0'), std::setw(2), (printTime%3600)/60) ) );
            }
            //lblPrintTimeLeft update
            int printTimeLeft = 0;
            if( json["progress"]["printTimeLeft"].is_null() )
            {
                lblPrintTimeLeft->set_text("Left: -:--");
            }
            else
            {
                printTimeLeft = json["progress"]["printTimeLeft"].as_integer();
                lblPrintTimeLeft->set_text(Glib::ustring::compose("Left: %1:%2",
                    printTimeLeft/3600,
                    Glib::ustring::format(std::setfill(L'0'), std::setw(2), (printTimeLeft%3600)/60) ) );
            }
            //lblPrintTimeTotal update
            int printTimeTotal = printTime + printTimeLeft;
            if( printTimeTotal == 0)
            {
                lblEstimatePrintTime->set_text( "Total: -:--" );
            }
            else
            {
                lblEstimatePrintTime->set_text( Glib::ustring::compose( "Total: %1:%2",
                    printTimeTotal/3600,
                    Glib::ustring::format( std::setfill(L'0'), std::setw(2), (printTimeTotal%3600)/60) ) );
            }
            //progressFile update
            if( printTimeTotal == 0 )
            {
                progressFile->set_fraction( 0. );
                progressFile->set_text( "0.00%" );
            }
            else
            {
                double progressFileFraction = (double)printTime/(double)printTimeTotal*100.;
                progressFile->set_fraction( progressFileFraction/100 );
                progressFile->set_text( Glib::ustring::compose("%1%%", round( progressFileFraction*100.)/100. ) );
            }
        })
        .then([=] (pplx::task<void> previous_task) mutable {
            if (previous_task._GetImpl()->_HasUserException()) {
                try {
                    auto holder = previous_task._GetImpl()->_GetExceptionHolder();
                    holder->_RethrowUserException();
                } catch (std::exception& e) {
                    lblStatus->set_text(
                        Glib::ustring::compose( "Error: %1", e.what())
                    );
                    std::cerr << "Exception: " << e.what() << std::endl;
                }
            }
        });
}

void StatusActivity::hide()
{
    window->hide();
    periodicTaskConnection.disconnect();
}

void StatusActivity::childActivityHidden( Activity *child )
{
    show();
}

bool StatusActivity::windowDestroyed( GdkEventAny* any_event )
{
    this->backClicked();
    return true;
}

void StatusActivity::backClicked()
{
    this->hide();
    parent->childActivityHidden(this);
}

bool StatusActivity::periodicTask()
{
    refreshData();
    return true;
}

void StatusActivity::startPrint()
{
    utility::string_t address = U(Config::i()->getHost());
    web::http::uri uri = web::http::uri(address);
    web::http::client::http_client api(web::http::uri_builder(uri).append_path(U("api/job")).to_uri());
    web::json::value requestBody = web::json::value::object();
    requestBody[U("command")] = web::json::value::string("start");
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
                lblStatus->set_text(Glib::ustring::compose("Error: %1\n%2", response.status_code(), response.reason_phrase()));
                return;
            }
            refreshData();
        })
        .then([=] (pplx::task<void> previous_task) mutable {
            if (previous_task._GetImpl()->_HasUserException()) {
                try {
                    auto holder = previous_task._GetImpl()->_GetExceptionHolder();
                    holder->_RethrowUserException();
                } catch (std::exception& e) {
                    lblStatus->set_text(
                        Glib::ustring::compose( "Error: %1", e.what())
                    );
                    std::cerr << "Exception: " << e.what() << std::endl;
                }
            }
        });
}

void StatusActivity::pausePrint()
{
    utility::string_t address = U(Config::i()->getHost());
    web::http::uri uri = web::http::uri(address);
    web::http::client::http_client api(web::http::uri_builder(uri).append_path(U("api/job")).to_uri());
    web::json::value requestBody = web::json::value::object();
    requestBody[U("command")] = web::json::value::string("pause");
    requestBody[U("action")] = web::json::value::string("toggle");
    utility::stringstream_t stream;
    requestBody.serialize(stream);
    web::http::http_request request(web::http::methods::POST);
    request.set_body(stream.str(), utf8string("application/json"));
    request.headers().add(U("X-Api-Key"), U(Config::i()->getApiKey()));
    api.request(request).then([=](web::http::http_response response)
        {
            if(response.status_code() < 200 || response.status_code() > 299)
            {
                lblStatus->set_text(Glib::ustring::compose("Error: %1\n%2", response.status_code(), response.reason_phrase()));
                return;
            }
            refreshData();
        })
        .then([=] (pplx::task<void> previous_task) mutable {
            if (previous_task._GetImpl()->_HasUserException()) {
                try {
                    auto holder = previous_task._GetImpl()->_GetExceptionHolder();
                    holder->_RethrowUserException();
                } catch (std::exception& e) {
                    lblStatus->set_text(
                        Glib::ustring::compose( "Error: %1", e.what())
                    );
                    std::cerr << "Exception: " << e.what() << std::endl;
                }
            }
        });
}

void StatusActivity::stopPrint()
{
    utility::string_t address = U(Config::i()->getHost());
    web::http::uri uri = web::http::uri(address);
    web::http::client::http_client api(web::http::uri_builder(uri).append_path(U("api/job")).to_uri());
    web::json::value requestBody = web::json::value::object();
    requestBody[U("command")] = web::json::value::string("cancel");
    utility::stringstream_t stream;
    requestBody.serialize(stream);
    web::http::http_request request(web::http::methods::POST);
    request.set_body(stream.str(), utf8string("application/json"));
    request.headers().add(U("X-Api-Key"), U(Config::i()->getApiKey()));
    api.request(request).then([=](web::http::http_response response)
        {
            if(response.status_code() < 200 || response.status_code() > 299)
            {
                lblStatus->set_text(Glib::ustring::compose("Error: %1\n%2", response.status_code(), response.reason_phrase()));
                return;
            }
            refreshData();
        })
        .then([=] (pplx::task<void> previous_task) mutable {
            if (previous_task._GetImpl()->_HasUserException()) {
                try {
                    auto holder = previous_task._GetImpl()->_GetExceptionHolder();
                    holder->_RethrowUserException();
                } catch (std::exception& e) {
                    lblStatus->set_text(
                        Glib::ustring::compose( "Error: %1", e.what())
                    );
                    std::cerr << "Exception: " << e.what() << std::endl;
                }
            }
        });
}

void StatusActivity::tunePrint()
{
    hide();
    tuneActivity->show();
}

StatusActivity::~StatusActivity()
{
    delete window;
    delete tuneActivity;
}
