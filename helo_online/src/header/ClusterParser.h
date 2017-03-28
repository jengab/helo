#ifndef CLUSTER_PARSER_H
#define CLUSTER_PARSER_H

#include <list>
#include <boost/thread/mutex.hpp>
#include <SQLiteCpp/SQLiteCpp.h>

#include "ClusterTemplate.h"
#include "ConfigFile.h"

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
	boost::mutex WriteMutex;
	std::list<ClusterTemplate> Clusters;
	ClusterParser(const ClusterParser&):settings(){}

public:
	ClusterParser(const Settings&);
	//friend std::wistream& operator>>(std::wistream&,ClusterParser&);
	friend std::wostream& operator<<(std::wostream&,ClusterParser&);
	void printToConsole() const;
	void ProcessMessage(std::wstring&);
};

#endif
