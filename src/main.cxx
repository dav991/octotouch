#include "main.h"

int main(int argc, char *argv[])
{
    libconfig::Config cfg;
    std::string host, apiKey, resourcePath, webcamSnapUrl;
    int tools, resW, resH;
    try
    {
      cfg.readFile( "octotouch.cfg" );
      const libconfig::Setting& root = cfg.getRoot();
      root["octopi"].lookupValue( "host", host );
      root["octopi"].lookupValue( "apiKey", apiKey );
      root["octopi"].lookupValue( "webcamSnapUrl", webcamSnapUrl );
      root["display"].lookupValue( "width", resW );
      root["display"].lookupValue( "height", resH );
      root["resources"].lookupValue( "location", resourcePath );
      std::cout << "octopi.host: " << host << std::endl;
      std::cout << "octopi.api : " << apiKey.substr(0,3) << "[...]" << std::endl;
      std::cout << "octopi.webcamSnapUrl: " << webcamSnapUrl << std::endl;
      std::cout << "display.width: " << resW << std::endl;
      std::cout << "display.height: " << resH << std::endl;
      std::cout << "resources.location: " << resourcePath << std::endl;
      Config::i()->setHost( host );
      Config::i()->setApiKey( apiKey );
      Config::i()->setDisplayWidth( resW );
      Config::i()->setDisplayHeight( resH );
      Config::i()->setResourcesFolder( resourcePath );
      Config::i()->setWebcamSnapUrl( webcamSnapUrl );
    }
    catch(const libconfig::FileIOException &fioex)
    {
        std::cerr << "I/O error while reading config file." << std::endl;
        return(1);
    }
    catch(const libconfig::ParseException &pex)
    {
        std::cerr << "Parse error at " << pex.getFile() << ":" << pex.getLine()
                  << " - " << pex.getError() << std::endl;
        return(2);
    }
    auto app = Gtk::Application::create(argc, argv,
      "com.intshift.octolcd");

    Glib::RefPtr<Gtk::CssProvider> cssProvider = Gtk::CssProvider::create();
    cssProvider->load_from_path("css/style.css");

    Glib::RefPtr<Gtk::StyleContext> styleContext = Gtk::StyleContext::create();

    Glib::RefPtr<Gdk::Screen> screen = Gdk::Screen::get_default();

    styleContext->add_provider_for_screen(screen, cssProvider, GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
    MainActivity *ma = nullptr;
    int ret = 3;
    try
    {
      ma = new MainActivity(app);
      ret = ma->start();
    }
    catch(std::exception& e)
    {
      std::cerr << "Exception: " << e.what() << std::endl;
    }
    if( ma != nullptr)
    {
      delete ma;
    }
    return ret;
}
