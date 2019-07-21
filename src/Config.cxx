#include "Config.h"


Config* Config::instance = nullptr;

Config::Config()
{
}

Config* Config::getInstance()
{
	if (instance == nullptr)
    {
        instance = new Config();
    }
    return instance;
}

Config* Config::i()
{
	return Config::getInstance();
}

std::string Config::getHost()
{
	return this->host;
}

void Config::setHost( std::string host )
{
	this->host = host;
}

std::string Config::getApiKey()
{
	return this->apiKey;
}

void Config::setApiKey( std::string key )
{
	this->apiKey = key;
}

int Config::getDisplayWidth()
{
	return this->displayWidth;
}

void Config::setDisplayWidth( int w )
{
	this->displayWidth = w;
}

int Config::getDisplayHeight()
{
	return this->displayHeight;
}

void Config::setDisplayHeight( int h )
{
	this->displayHeight = h;
}

void Config::setResourcesFolder( std::string path )
{
	this->resourcesFolder = path;
}

std::string Config::getResourcesFolder()
{
	return this->resourcesFolder;
}