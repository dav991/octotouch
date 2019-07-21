#include "TuneActivity.h"



TuneActivity::TuneActivity(Activity *parent):
    window(nullptr),
    incrementValue(0),
    toolTargetValue(0),
    bedTargetValue(0),
    flowrate(100),
    feedrate(100)
{
    this->parent = parent;
    Glib::RefPtr<Gtk::Builder> builder = Gtk::Builder::create_from_file( Config::i()->getResourcesFolder() + "glade/tuneWindow.glade" );
    builder->get_widget( "windowTune", window );
    builder->get_widget( "btnBack", btnBack );
    builder->get_widget( "btnTempCycleIncrements", btnTempCycleIncrements );
    builder->get_widget( "btnCooldown", btnCooldown );
    builder->get_widget( "btnDecreaseNozzleTemp", btnDecreaseNozzleTemp );
    builder->get_widget( "btnIncreaseNozzleTemp", btnIncreaseNozzleTemp );
    builder->get_widget( "btnDecreaseBedTemp", btnDecreaseBedTemp );
    builder->get_widget( "btnIncreaseBedTemp", btnIncreaseBedTemp );
    builder->get_widget( "btnIncreaseFlow", btnIncreaseFlow );
    builder->get_widget( "btnDecreaseFlow", btnDecreaseFlow );
    builder->get_widget( "lblToolTemp", lblToolTemp );
    builder->get_widget( "lblBedTemp", lblBedTemp );
    builder->get_widget( "lblToolTemp", lblToolTemp );
    builder->get_widget( "lblFlow", lblFlow );
    builder->get_widget( "lblFeed", lblFeed );
    builder->get_widget( "btnIncreaseFeed", btnIncreaseFeed );
    builder->get_widget( "btnDecreaseFeed", btnDecreaseFeed );
    builder->get_widget( "lblStatus", lblStatus );

    if( !validWidget( window, "windowStatus missing from tuneWindow.glade" ) ) return;
    if( !validWidget( btnBack, "btnBack missing from tuneWindow.glade" ) ) return;
    if( !validWidget( btnTempCycleIncrements, "btnTempCycleIncrements missing from tuneWindow.glade" ) ) return;
    if( !validWidget( btnCooldown, "btnCooldown missing from tuneWindow.glade" ) ) return;
    if( !validWidget( btnDecreaseNozzleTemp, "btnDecreaseNozzleTemp missing from tuneWindow.glade" ) ) return;
    if( !validWidget( btnIncreaseNozzleTemp, "btnIncreaseNozzleTemp missing from tuneWindow.glade" ) ) return;
    if( !validWidget( btnDecreaseBedTemp, "btnDecreaseBedTemp missing from tuneWindow.glade" ) ) return;
    if( !validWidget( btnIncreaseBedTemp, "btnIncreaseBedTemp missing from tuneWindow.glade" ) ) return;
    if( !validWidget( btnIncreaseFlow, "btnIncreaseFlow missing from tuneWindow.glade" ) ) return;
    if( !validWidget( btnDecreaseFlow, "btnDecreaseFlow missing from tuneWindow.glade" ) ) return;
    if( !validWidget( lblToolTemp, "lblToolTemp missing from tuneWindow.glade" ) ) return;
    if( !validWidget( lblBedTemp, "lblBedTemp missing from tuneWindow.glade" ) ) return;
    if( !validWidget( lblToolTemp, "lblToolTemp missing from tuneWindow.glade" ) ) return;
    if( !validWidget( lblFlow, "lblFlow missing from tuneWindow.glade" ) ) return;
    if( !validWidget( lblFeed, "lblFeed missing from tuneWindow.glade" ) ) return;
     if( !validWidget( lblStatus, "lblStatus missing from tuneWindow.glade" ) ) return;

    window->signal_delete_event().connect( sigc::mem_fun( this, &TuneActivity::windowDestroyed ) );
    window->set_default_size( Config::i()->getDisplayWidth(), Config::i()->getDisplayHeight() );
    btnBack->signal_clicked().connect( sigc::mem_fun( this, &TuneActivity::backClicked ) );
    btnTempCycleIncrements->signal_clicked().connect( sigc::mem_fun( this, &TuneActivity::switchIncrement ) );
    btnCooldown->signal_clicked().connect( sigc::mem_fun( this, &TuneActivity::cooldown ) );
    btnDecreaseNozzleTemp->signal_clicked().connect( sigc::mem_fun( this, &TuneActivity::decrementToolTemp ) ); 
    btnIncreaseNozzleTemp->signal_clicked().connect( sigc::mem_fun( this, &TuneActivity::incrementToolTemp ) );
    btnDecreaseBedTemp->signal_clicked().connect( sigc::mem_fun( this, &TuneActivity::decrementBedTemp) );
    btnIncreaseBedTemp->signal_clicked().connect( sigc::mem_fun( this, &TuneActivity::incrementBedTemp) );
    btnIncreaseFlow->signal_clicked().connect( sigc::mem_fun( this, &TuneActivity::incrementFlow) );
    btnDecreaseFlow->signal_clicked().connect( sigc::mem_fun( this, &TuneActivity::decrementFlow) );
    btnIncreaseFeed->signal_clicked().connect( sigc::mem_fun( this, &TuneActivity::incrementFeed) );
    btnDecreaseFeed->signal_clicked().connect( sigc::mem_fun( this, &TuneActivity::decrementFeed) );
    switchIncrement();
}

void TuneActivity::show()
{
    window->show();
    refreshData();
    periodicTaskConnection = Glib::signal_timeout().connect_seconds(sigc::mem_fun(*this, &TuneActivity::periodicTask), 5);
}

void TuneActivity::refreshData()
{
    utility::string_t address = U(Config::i()->getHost());
    web::http::uri uri = web::http::uri(address);
    web::http::client::http_client api(web::http::uri_builder(uri).append_path(U("api/printer")).to_uri());
    web::http::http_request request(web::http::methods::GET);
    request.headers().add(U("X-Api-Key"), U(Config::i()->getApiKey()));
    api.request(request).then(
        [=](web::http::http_response  response)
        {
            if( response.status_code() < 200 || response.status_code() > 299 )
            {
                std::cerr 
                    << "Request failed with error " 
                    <<  response.status_code() 
                    << " and message: "
                    << response.reason_phrase()
                    << std::endl;
                return;
            }
            auto json = response.extract_json().get();
            //lblToolTemp update
            if( json["temperature"]["tool0"]["actual"].is_null() || json["temperature"]["tool0"]["target"].is_null())
            {
                lblToolTemp->set_text( "0/0" );
                toolTargetValue = 0;
            }
            else
            {
                toolTargetValue = json["temperature"]["tool0"]["target"].as_double();
                lblToolTemp->set_text(
                    Glib::ustring::compose( 
                        "%1/%2",
                        Glib::ustring::format( std::fixed, std::setprecision(1), json["temperature"]["tool0"]["actual"].as_double() ),
                        Glib::ustring::format( std::fixed, std::setprecision(1), json["temperature"]["tool0"]["target"].as_double() )
                    )
                );
            }
            //lblBedTemp update
            if( json["temperature"]["bed"]["actual"].is_null() || json["temperature"]["bed"]["target"].is_null())
            {
                lblBedTemp->set_text( "0/0" );
                bedTargetValue = 0;
            }
            else
            {
                bedTargetValue = json["temperature"]["bed"]["target"].as_double();
                lblBedTemp->set_text(
                    Glib::ustring::compose( 
                        "%1/%2",
                        Glib::ustring::format( std::fixed, std::setprecision(1), json["temperature"]["bed"]["actual"].as_double() ),
                        Glib::ustring::format( std::fixed, std::setprecision(1), json["temperature"]["bed"]["target"].as_double() )
                    )
                );
            }
        });
}

void TuneActivity::hide()
{
    window->hide();
    periodicTaskConnection.disconnect();
}

void TuneActivity::childActivityHidden( Activity *child )
{
}

bool TuneActivity::windowDestroyed( GdkEventAny* any_event )
{
    this->backClicked();
    return true;
}

void TuneActivity::backClicked()
{
    this->hide();
    parent->childActivityHidden(this);
}

bool TuneActivity::periodicTask()
{
    refreshData();
    return true;
}

void TuneActivity::switchIncrement()
{
    switch(incrementValue)
    {
        case 1:
            incrementValue = 5;
            break;
        case 5:
            incrementValue = 10;
            break;
        case 10:
            incrementValue = 20;
            break;
        case 20:
            incrementValue = 1;
            break;
        default:
            incrementValue = 1;
            break;
    }
    btnTempCycleIncrements->set_label( Glib::ustring::compose("Step: %1\u2103", incrementValue) );
}

void TuneActivity::decrementToolTemp()
{
    if( toolTargetValue < incrementValue )
    {
        return;
    }
    toolTargetValue -= incrementValue;
    requestToolTarget();
}

void TuneActivity::incrementToolTemp()
{
    // TODO maybe make this limit configurable?
    if( (toolTargetValue + incrementValue) > 270 )
    {
        return ;
    }
    toolTargetValue += incrementValue;
    requestToolTarget();
}


void TuneActivity::requestToolTarget()
{
    utility::string_t address = U(Config::i()->getHost());
    web::http::uri uri = web::http::uri(address);
    web::http::client::http_client api(web::http::uri_builder(uri).append_path(U("api/printer/tool")).to_uri());
    web::json::value requestBody = web::json::value::object();
    requestBody[U("command")] = web::json::value::string("target");
    requestBody[U("targets")] = web::json::value::object();
    requestBody[U("targets")][U("tool0")] = web::json::value::number(toolTargetValue);
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
        });
}

void TuneActivity::decrementBedTemp()
{
    std::cout << "TuneActivity::" << __func__ << "(): current bedTargetValue:" << bedTargetValue << std::endl;
    if( bedTargetValue < incrementValue )
    {
        return;
    }
    bedTargetValue -= incrementValue;
    std::cout << "TuneActivity::" << __func__ << "(): new bedTargetValue:" << bedTargetValue << std::endl;
    requestBedTarget();
}

void TuneActivity::incrementBedTemp()
{
    std::cout << "TuneActivity::" << __func__ << "(): current bedTargetValue:" << bedTargetValue << std::endl;
    // TODO maybe make this limit configurable?
    if( (bedTargetValue + incrementValue) > 110 )
    {
        return ;
    }
    bedTargetValue += incrementValue;
    std::cout << "TuneActivity::" << __func__ << "(): new bedTargetValue:" << bedTargetValue << std::endl;
    requestBedTarget();
}

void TuneActivity::requestBedTarget()
{
    utility::string_t address = U(Config::i()->getHost());
    web::http::uri uri = web::http::uri(address);
    web::http::client::http_client api(web::http::uri_builder(uri).append_path(U("api/printer/bed")).to_uri());
    web::json::value requestBody = web::json::value::object();
    requestBody[U("command")] = web::json::value::string("target");
    requestBody[U("target")] = web::json::value::number(bedTargetValue);
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
        });
}

void TuneActivity::decrementFlow()
{

}

void TuneActivity::incrementFlow()
{

}

void TuneActivity::requestFlowRate()
{

}

void TuneActivity::decrementFeed()
{

}

void TuneActivity::incrementFeed()
{

}


void TuneActivity::requestFeedRate()
{

}

void TuneActivity::cooldown()
{
    toolTargetValue = 0;
    bedTargetValue = 0;
    requestToolTarget();
    requestBedTarget();
}

void TuneActivity::loadFilament()
{
    //send the M701 command
    std::vector<web::json::value> commands = {web::json::value::string("M701")};
    utility::string_t address = U(Config::i()->getHost());
    web::http::uri uri = web::http::uri(address);
    web::http::client::http_client api(web::http::uri_builder(uri).append_path(U("api/printer/command")).to_uri());
    web::json::value requestBody = web::json::value::object();
    requestBody[U("commands")] = web::json::value::array(commands);
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
        });
}

void TuneActivity::unloadFilament()
{
    //send the M702 command
    std::vector<web::json::value> commands = {web::json::value::string("M702")};
    utility::string_t address = U(Config::i()->getHost());
    web::http::uri uri = web::http::uri(address);
    web::http::client::http_client api(web::http::uri_builder(uri).append_path(U("api/printer/command")).to_uri());
    web::json::value requestBody = web::json::value::object();
    requestBody[U("commands")] = web::json::value::array(commands);
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
        });
}

void TuneActivity::swapFilament()
{
    //send the M600 command
    std::vector<web::json::value> commands = {web::json::value::string("M600")};
    utility::string_t address = U(Config::i()->getHost());
    web::http::uri uri = web::http::uri(address);
    web::http::client::http_client api(web::http::uri_builder(uri).append_path(U("api/printer/command")).to_uri());
    web::json::value requestBody = web::json::value::object();
    requestBody[U("commands")] = web::json::value::array(commands);
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
        });
}

TuneActivity::~TuneActivity()
{
    delete window;
}
