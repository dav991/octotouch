#include "FilesActivity.h"



FilesActivity::FilesActivity(Activity *parent): 
    window(nullptr),
    showStatusDispatcher(),
    scrollWindowStart(0)
{
    this->parent = parent;
    statusActivity = new StatusActivity(this);
    Glib::RefPtr<Gtk::Builder> builder = Gtk::Builder::create_from_file( Config::i()->getResourcesFolder() + "glade/filesWindow.glade" );
    builder->get_widget( "windowFiles", window );
    builder->get_widget( "btnBack", btnBack );
    builder->get_widget( "lblStatus", lblStatus );
    builder->get_widget( "gridListWrapper", gridListWrapper );
    builder->get_widget( "btnScrollUp", btnScrollUp);
    builder->get_widget( "btnScrollDown", btnScrollDown);
    builder->get_widget( "btnScrollTop", btnScrollTop);

    if( !validWidget( window, "windowFiles missing from filesWindow.glade" ) ) return;
    if( !validWidget( btnBack, "btnBack missing from filesWindow.glade" ) ) return;
    if( !validWidget( lblStatus, "lblStatus missing from filesWindow.glade" ) ) return;
    if( !validWidget( gridListWrapper, "gridListWrapper missing from filesWindow.glade" ) ) return;
    if( !validWidget( btnScrollUp, "btnScrollUp missing from filesWindow.glade" ) ) return;
    if( !validWidget( btnScrollDown, "btnScrollDown missing from filesWindow.glade" ) ) return;
    if( !validWidget( btnScrollTop, "btnScrollTop missing from filesWindow.glade" ) ) return;

    window->signal_delete_event().connect (sigc::mem_fun(this, &FilesActivity::windowDestroyed) );
    window->set_default_size( Config::i()->getDisplayWidth(), Config::i()->getDisplayHeight() );
    btnBack->signal_clicked().connect( sigc::mem_fun(this, &FilesActivity::backClicked) );
    showStatusDispatcher.connect( sigc::mem_fun( this, &FilesActivity::switchToStatus ) );
    populateListDispatcher.connect( sigc::mem_fun( this, &FilesActivity::populateList ) );
    statusDispatcher.connect( sigc::mem_fun( this, &FilesActivity::errorStatusUpdate ) );
    btnScrollUp->signal_clicked().connect( sigc::mem_fun( this, &FilesActivity::onScrollUp ) );
    btnScrollDown->signal_clicked().connect( sigc::mem_fun( this, &FilesActivity::onScrollDown ) );
    btnScrollTop->signal_clicked().connect( sigc::mem_fun( this, &FilesActivity::onScrollTop ) );

    std::vector<Gtk::Widget*> children = gridListWrapper->get_children();
    int i = 0;
    scrollItems = children.size();
    for( auto it = children.begin(); it!= children.end(); it++ )
    {
        auto box = dynamic_cast<Gtk::EventBox *>(*it);
        auto label = dynamic_cast<Gtk::Label *>(box->get_child ());
        listLabels.insert( listLabels.begin(), label );
        box->signal_button_press_event ().connect( 
            sigc::bind<int>( sigc::mem_fun( this, &FilesActivity::listItemClicked ),  scrollItems - i - 1) 
        );
        ++i;
    }
    
}

void FilesActivity::show()
{
    refreshData();
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
    files.clear();
}

bool FilesActivity::listItemClicked( GdkEventButton* button_event, int element )
{
    if( (scrollWindowStart + element) >= files.size()  )
    {
        return true;
    }
    std::string data = files[scrollWindowStart + element].path;
    utility::string_t address = U(Config::i()->getHost());
    web::http::uri uri = web::http::uri(address);
    web::http::client::http_client api(web::http::uri_builder(uri).append_path(U("api/files/")).append_path(U(data)).to_uri());
    web::json::value requestBody = web::json::value::object();
    requestBody[U("command")] = web::json::value::string("select");
    requestBody[U("print")] = web::json::value::boolean(false);
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
                std::cerr << "FilesActivity::listItemClicked error: " << 
                    Glib::ustring::compose("Error: %1\n%2", response.status_code(), response.reason_phrase()) << std::endl;
                std::lock_guard<std::mutex> lock( errorStatusMutex );
                errorStatus = Glib::ustring::compose("Error: %1\n%2", response.status_code(), response.reason_phrase());
                statusDispatcher.emit();
                return;
            }
            showStatusDispatcher.emit();
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
    return true;
}

void FilesActivity::switchToStatus()
{
    hide();
    statusActivity->show();
}

void FilesActivity::insertInOrder(FileEntry entry)
{
    if( files.size() == 0)
    {
        files.push_back(entry);
    }
    int i;
    for( i = 0; i < files.size(); ++i )
    {
        if( files[i].timestamp < entry.timestamp )
        {
            files.insert( files.begin()+i, entry );
            break;
        }
    }
    if( i == files.size() )
    {
        files.push_back(entry);
    }
}

void FilesActivity::parseFiles( web::json::value array)
{
    if( !array.is_array() )
    {
        return;
    }
    for( int i = 0; i < array.size(); ++i)
    {
        if( !array[i]["type"].is_null() && array[i]["type"].as_string().compare("machinecode") == 0)
        {
            if( array[i]["name"].is_null() 
                || array[i]["origin"].is_null() 
                || array[i]["path"].is_null() 
                || array[i]["date"].is_null())
            {
                continue;
            }
            insertInOrder( 
                FileEntry( 
                    array[i]["path"].as_string(),
                    Glib::ustring::compose( "%1/%2", array[i]["origin"].as_string(), array[i]["path"].as_string()),
                    array[i]["date"].as_number().to_int64()
                )
            );
        }
        else if( !array[i]["type"].is_null() && array[i]["type"].as_string().compare("folder") == 0)
        {
            parseFiles( array[i]["children"] );
        }
    }
}

void FilesActivity::populateList()
{
    for( int i = 0; i < scrollItems; ++i)
    {
        if( scrollWindowStart + i < files.size() )
        {
            listLabels[i]->set_text(
                Glib::ustring::compose( "%1) %2", scrollWindowStart + i + 1, files[scrollWindowStart + i].path)
            );
        }
        else
        {
            listLabels[i]->set_text("--");
        }
    }
}

void FilesActivity::refreshData()
{
    utility::string_t address = U(Config::i()->getHost());
    web::http::uri uri = web::http::uri(address);
    web::http::client::http_client api(web::http::uri_builder(uri).append_path(U("/api/files?recursive=true")).to_uri());
    web::http::http_request request(web::http::methods::GET);
    request.headers().add(U("X-Api-Key"), U(Config::i()->getApiKey()));
    api.request(request)
        .then( [=](web::http::http_response  response)
        {
            if( response.status_code() < 200 || response.status_code() > 299 )
            {
                std::cerr << "FilesActivity::listItemClicked error: " << 
                    Glib::ustring::compose("Error: %1\n%2", response.status_code(), response.reason_phrase()) << std::endl;
                std::lock_guard<std::mutex> lock( errorStatusMutex );
                errorStatus = Glib::ustring::compose("Error: %1\n%2", response.status_code(), response.reason_phrase());
                statusDispatcher.emit();
                return;
            }
            auto json = response.extract_json().get();
            parseFiles( json["files"] );
            scrollWindowStart = 0;
            populateListDispatcher.emit();
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

void FilesActivity::onScrollUp()
{
    if( scrollWindowStart <= 0)
    {
        return;
    }
    --scrollWindowStart;
    populateList();
}

void FilesActivity::onScrollDown()
{
    if( (scrollWindowStart + scrollItems) > files.size() )
    {
        return;
    }
    ++scrollWindowStart;
    populateList();
}

void FilesActivity::onScrollTop()
{
    scrollWindowStart = 0;
    populateList();
}

void FilesActivity::errorStatusUpdate()
{
    std::lock_guard<std::mutex> lock( errorStatusMutex );
    lblStatus->set_text( errorStatus );
}

FilesActivity::~FilesActivity()
{
    delete window;
    delete statusActivity;
}


FileEntry::FileEntry( std::string name, std::string path, long long timestamp ): name(name), path(path), timestamp(timestamp)
{

}