#include "FilesActivity.h"



FilesActivity::FilesActivity(Activity *parent): 
    window(nullptr),
    showStatusDispatcher()
{
    this->parent = parent;
    statusActivity = new StatusActivity(this);
    Glib::RefPtr<Gtk::Builder> builder = Gtk::Builder::create_from_file( Config::i()->getResourcesFolder() + "glade/filesWindow.glade" );
    builder->get_widget( "windowFiles", window );
    builder->get_widget( "btnBack", btnBack );
    builder->get_widget( "listBoxFiles", listBoxFiles);
    builder->get_widget( "lblStatus", lblStatus);

    if( !validWidget( window, "windowFiles missing from filesWindow.glade" ) ) return;
    if( !validWidget( btnBack, "btnBack missing from filesWindow.glade" ) ) return;
    if( !validWidget( listBoxFiles, "listBoxFiles missing from filesWindow.glade" ) ) return;
    if( !validWidget( lblStatus, "lblStatus missing from filesWindow.glade" ) ) return;

    window->signal_delete_event().connect (sigc::mem_fun(this, &FilesActivity::windowDestroyed) );
    window->set_default_size( Config::i()->getDisplayWidth(), Config::i()->getDisplayHeight() );
    btnBack->signal_clicked().connect( sigc::mem_fun(this, &FilesActivity::backClicked) );
    showStatusDispatcher.connect( sigc::mem_fun( this, &FilesActivity::switchToStatus ) );
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
    std::vector<Gtk::Widget*> children = listBoxFiles->get_children();
    for( auto it = children.begin(); it!= children.end(); it++ )
    {
        listBoxFiles->remove(**it);
    }
    files.clear();
}

void FilesActivity::addItemToList( FileEntry entry )
{
    auto button = Gtk::manage( new Gtk::Button( entry.name ) );
    Gtk::Label *btnLabel = dynamic_cast< Gtk::Label *>(button->get_child());
    btnLabel->set_lines(-1);
    btnLabel->set_line_wrap(true);
    btnLabel->set_line_wrap_mode(Pango::WrapMode::WRAP_WORD_CHAR);
    button->get_style_context()->add_class("btn");
    button->get_style_context()->add_class("listItem");
    button->signal_clicked().connect( sigc::bind<std::string>( sigc::mem_fun( *this, &FilesActivity::listItemClicked), entry.path ) );
    button->show();
    listBoxFiles->append( *button );
}

void FilesActivity::listItemClicked( std::string data )
{
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
                lblStatus->set_text(Glib::ustring::compose("Error: %1\n%2", response.status_code(), response.reason_phrase()));
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
                    lblStatus->set_text(
                        Glib::ustring::compose( "Error: %1", e.what())
                    );
                    std::cerr << "Exception: " << e.what() << std::endl;
                }
            }
        });
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
    for( int i = 0; i < files.size(); ++i )
    {
        if( files[i].timestamp < entry.timestamp )
        {
            files.insert( files.begin()+i, entry );
            break;
        }
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
    for( auto it = files.begin(); it != files.end(); ++it )
    {
        addItemToList( *it );
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
                lblStatus->set_text(
                    Glib::ustring::compose( "Connection error: %1\n%2", response.status_code(), response.reason_phrase())
                );
                return;
            }
            auto json = response.extract_json().get();
            if( json["files"].is_null() )
            {
                lblStatus->set_text( "Files cannot be retrieved" );
                return;
            }
            files.reserve( json["files"].size() );
            parseFiles( json["files"] );
            populateList();
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

FilesActivity::~FilesActivity()
{
    delete window;
    delete statusActivity;
}


FileEntry::FileEntry( std::string name, std::string path, long long timestamp ): name(name), path(path), timestamp(timestamp)
{

}