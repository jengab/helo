#ifndef CLUSTER_PARSER_H
#define CLUSTER_PARSER_H

#include <list>
#include <mutex>
#include <SQLiteCpp/SQLiteCpp.h>

#include "ClusterTemplate.h"
#include "ConfigFile.h"
#include "LogParser.h"

/**
* @file ClusterParser.h
*
* This file contains the ClusterParser class.
* @author Jenei Gábor <jengab@elte.hu>
*/

/**
* This class loads clusters from a database, and can be used to
* process new messages. It implements processing in a thread-safe way.
*/
class ClusterParser{
private:
    const Settings settings;
    const LogParser logParser;
    std::mutex WriteMutex;
    std::list<ClusterTemplate> Clusters;
    ClusterParser(const ClusterParser&):settings(),logParser(settings.HeaderLen, settings.regexp){}

public:
    ClusterParser(const Settings&);
    //friend std::wistream& operator>>(std::wistream&,ClusterParser&);
    friend std::wostream& operator<<(std::wostream&,ClusterParser&);
    void printToConsole() const;
    void ProcessMessage(std::wstring&);
};

#endif
