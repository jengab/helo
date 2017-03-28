#include <pugixml.hpp>
#include <stdexcept>
#include "ConfigFile.h"

const wchar_t* ConfigFile::onlineTag=L"online";
const wchar_t* ConfigFile::mergeTag=L"MergeLimit";
const wchar_t* ConfigFile::headerTag=L"HeaderLen";
const wchar_t* ConfigFile::locTag=L"UsedLocal";
const wchar_t* ConfigFile::portTag=L"Port";
const wchar_t* ConfigFile::dbPathTag=L"DBPath";
const wchar_t* ConfigFile::logPathTag=L"LogPath";
const wchar_t* ConfigFile::regexpTag=L"RegExp";

/**
* Constructor. It reads "settings.xml" (only this path can be used as config file).
* And initializes the stored setting. After this method the file will be closed
* (can't be read anymore)
*
* @throws std::runtime_error If some essential settings can't be found in the
* config file
*/
ConfigFile::ConfigFile(){
	std::ifstream file("settings.xml");
	if(file.fail()) throw std::runtime_error("The config file couldn't be opened!");
	pugi::xml_document doc;
	doc.load(file);
	file.close();

	pugi::xml_node OnlineNode=doc.child(onlineTag);
	settings.lim=OnlineNode.child(mergeTag).attribute(L"value").as_double();
	if(settings.lim==0) throw std::runtime_error("Invalid config file! The goodness threshold is not set.\n");
	settings.port=OnlineNode.child(portTag).attribute(L"value").as_uint();
	if(settings.port==0) throw std::runtime_error("Invalid config file! The listen port is not set.\n");
	settings.HeaderLen=OnlineNode.child(headerTag).attribute(L"value").as_uint();
	if(settings.HeaderLen==0) throw std::runtime_error("Invalid config file! The header length is not set.\n");
	std::wstring wLoc=OnlineNode.child(locTag).attribute(L"value").value();
	settings.loc=std::string(wLoc.begin(),wLoc.end());
	std::wstring wPath=OnlineNode.child(dbPathTag).attribute(L"value").value();
	settings.DbFile=std::string(wPath.begin(),wPath.end());
	if(settings.DbFile.empty()) throw std::runtime_error("Invalid config file! The database file's path is not set.\n");
	std::wstring wLog=OnlineNode.child(logPathTag).attribute(L"value").value();
	LogPath=std::string(wLog.begin(),wLog.end());
	settings.regexp=OnlineNode.child(regexpTag).attribute(L"value").value();
	if(settings.regexp.empty()) settings.regexp=L"[\\s]+";
}

/**
* @return A Settings object filled with the loaded settings
*/
Settings ConfigFile::getConfig() const{
	return settings;
}

/**
* @return The read log file path
*/
std::string ConfigFile::getLogPath() const{
	return LogPath;
}

