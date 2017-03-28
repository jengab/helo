#ifndef CONFIG_FILE_H
#define CONFIG_FILE_H

#include <fstream>
#include <iostream>

/**
* @file ConfigFile.h
*
* This file contains Settings and ConfigFile classes.
* It implements the features needed to work with an XML config file.
* @author Jenei Gábor <jengab@elte.hu>
*/

/**
* This class stores all the data needed to run the HELO online algorithm
* It only has public members, just like a \c struct
*/
class Settings{
public:
	/// The port to listen on
	///
	unsigned short port;
	
	/// The locale string to use as global locale (if this is empty the
	/// system's default locale will be used)
	///
	std::string loc;
	
	/// The path of the SQLite3 file to use
	///
	std::string DbFile;
	
	/// The header length used in the protocol (on syslog this is 4 by default)
	///
	unsigned int HeaderLen;
	
	/// The goodness limit that is used as a minimal goodness on joining a message to a cluster
	///
	double lim;
	
	/// The regular expression in POSIX regex format that is used to tokenize the log message
	///
	std::wstring regexp;
	
	/// A pointer to the output stream that is used for writing out error messages
	///
	std::ostream* ErrorStream;

	Settings():port(514),loc(""),DbFile(""),HeaderLen(4),lim(1.0),regexp(L"[\\s]+"),ErrorStream(&std::cout){}
};

/**
* This class represents a config file. It implements all
* the methods needed to load the configuration from the file
*/
class ConfigFile{
private:
	Settings settings;
	std::string LogPath;

public:
	ConfigFile();
	Settings getConfig() const;
	std::string getLogPath() const;

	/// The XML tag that signs the beginning of online settings
	///
	static const wchar_t* onlineTag;
	
	/// The XML tag that denotes the merge limit setting
	///
	static const wchar_t* mergeTag;
	
	/// The XML tag that denotes the header length setting
	///
	static const wchar_t* headerTag;
	
	/// The XML tag that denotes the locale string setting
	///
	static const wchar_t* locTag;
	
	/// The XML tag that represents the port setting
	///
	static const wchar_t* portTag;
	
	/// The XML tag that represents the database path setting
	///
	static const wchar_t* dbPathTag;
	
	/// The XML tag that represents the log file's path setting
	///
	static const wchar_t* logPathTag;
	
	/// The XML tag that denotes the regular expression setting
	///
	static const wchar_t* regexpTag;
};

#endif
