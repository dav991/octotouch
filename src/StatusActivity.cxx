#include "StatusActivity.h"



StatusActivity::StatusActivity(Activity *parent): 
    window(nullptr),
    fileText("File not selected"),
	statusText("Loading..."),
	printTimeText("Time: -:--"),
	printTimeLeftText("Left: -:--"),
	estimatePrintTimeText("Total: -:--"),
	progressFileValue(0.),
	progressFileText("0.00%")
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
    refreshDispatcher.connect( sigc::mem_fun( this, &StatusActivity::loadDataToUI ) );
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
                std::lock_guard<std::mutex> lock( uiMutex );
                statusText = Glib::ustring::compose( "Connection error: %1\n%2", response.status_code(), response.reason_phrase());
                refreshDispatcher.emit();
                return;
            }
            auto json = response.extract_json().get();
            //lblFile update
            if( json["job"]["file"]["name"].is_null() )
            {
            	fileText = "File not selected";
            }
            else
            {
                fileText = json["job"]["file"]["name"].as_string();
            }
            //lblStatus update
            if( json["state"].is_null() )
            {
                statusText = "Error retriving status" ;
            }
            else
            {
                statusText = Glib::ustring::compose( "Status: %1", json["state"].as_string() );
            }
            
            //lblPrintTime update
            int printTime = 0;
            if( json["progress"]["printTime"].is_null() )
            {
                printTimeText = "Time: -:--";
            }
            else
            {
                printTime = json["progress"]["printTime"].as_integer();
                printTimeText = Glib::ustring::compose("Time: %1:%2",
                    printTime/3600,
                    Glib::ustring::format(std::setfill(L'0'), std::setw(2), (printTime%3600)/60) );
            }
            //lblPrintTimeLeft update
            int printTimeLeft = 0;
            if( json["progress"]["printTimeLeft"].is_null() )
            {
                printTimeLeftText = "Left: -:--";
            }
            else
            {
                printTimeLeft = json["progress"]["printTimeLeft"].as_integer();
                printTimeLeftText = Glib::ustring::compose("Left: %1:%2",
                    printTimeLeft/3600,
                    Glib::ustring::format(std::setfill(L'0'), std::setw(2), (printTimeLeft%3600)/60) );
            }
            //lblPrintTimeTotal update
            int printTimeTotal = printTime + printTimeLeft;
            if( printTimeTotal == 0)
            {
                estimatePrintTimeText = "Total: -:--" ;
            }
            else
            {
                estimatePrintTimeText = Glib::ustring::compose( "Total: %1:%2",
                    printTimeTotal/3600,
                    Glib::ustring::format( std::setfill(L'0'), std::setw(2), (printTimeTotal%3600)/60) );
            }
            //progressFile update
            if( printTimeTotal == 0 )
            {
                progressFileValue = 0.;
                progressFileText = "0.00%";
            }
            else
            {
                double progressFileFraction = (double)printTime/(double)printTimeTotal*100.;
                progressFileValue = progressFileFraction/100;
                progressFileText = Glib::ustring::compose("%1%%", round( progressFileFraction*100.)/100. );
            }
            refreshDispatcher.emit();
        })
        .then([=] (pplx::task<void> previous_task) mutable {
            if (previous_task._GetImpl()->_HasUserException()) {
                try {
                    auto holder = previous_task._GetImpl()->_GetExceptionHolder();
                    holder->_RethrowUserException();
                } catch (std::exception& e) {
                    std::lock_guard<std::mutex> lock( uiMutex );
                    statusText = Glib::ustring::compose( "Error: %1", e.what());
                    refreshDispatcher.emit();
                    std::cerr << "Exception: " << e.what() << std::endl;
                }
            }
        });
}

void StatusActivity::loadDataToUI()
{
	std::lock_guard<std::mutex> lock( uiMutex );
	lblFile->set_text( fileText );
	lblStatus->set_text( statusText );
	lblPrintTime->set_text( printTimeText );
	lblPrintTimeLeft->set_text( printTimeLeftText );
	lblEstimatePrintTime->set_text( estimatePrintTimeText );
	progressFile->set_fraction( progressFileValue );
	progressFile->set_text( progressFileText );
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
                std::lock_guard<std::mutex> lock( uiMutex );
                statusText = Glib::ustring::compose("Error: %1\n%2", response.status_code(), response.reason_phrase());
                refreshDispatcher.emit();
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
                    std::lock_guard<std::mutex> lock( uiMutex );
                    statusText = Glib::ustring::compose( "Error: %1", e.what());
                    refreshDispatcher.emit();
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
                std::lock_guard<std::mutex> lock( uiMutex );
                statusText = Glib::ustring::compose("Error: %1\n%2", response.status_code(), response.reason_phrase());
                refreshDispatcher.emit();
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
                    std::lock_guard<std::mutex> lock( uiMutex );
                    statusText = Glib::ustring::compose( "Error: %1", e.what());
                    refreshDispatcher.emit();
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
                std::lock_guard<std::mutex> lock( uiMutex );
                statusText = Glib::ustring::compose("Error: %1\n%2", response.status_code(), response.reason_phrase());
                refreshDispatcher.emit();
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
                    std::lock_guard<std::mutex> lock( uiMutex );
                    statusText = Glib::ustring::compose( "Error: %1", e.what());
                    refreshDispatcher.emit();
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
