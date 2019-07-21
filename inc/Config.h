#ifndef  OCTOLCD_CONFIG
#define OCTOLCD_CONFIG

#include<string>

class Config
{
private:
	static Config* instance;
	Config();
	std::string host;
	std::string apiKey;
	int displayWidth;
	int displayHeight;
    std::string resourcesFolder;
public:
    static Config* getInstance();
    static Config* i();
    std::string getHost();
    void setHost( std::string host );
    std::string getApiKey();
    void setApiKey( std::string key );
    int getDisplayWidth();
    void setDisplayWidth( int w );
    int getDisplayHeight();
    void setDisplayHeight( int h );
    void setResourcesFolder( std::string path );
    std::string getResourcesFolder();
};


#endif
