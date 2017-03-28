#ifndef CLUSTER_TEMPLATE_H
#define CLUSTER_TEMPLATE_H

#include <string>
#include <sstream>
#include <sqlite3.h>
#include "LogParser.h"

/**
* @file ClusterTemplate.h
*
* This file contains the ClusterTemplate class.
* @author Jenei Gábor <jengab@elte.hu>
*/

/**
* This class represents a cluster that only has a template and
* some statistics stored(average line length, goodness). The
* represented clusters have an assigned ID (identification number)
* as well.
*/
class ClusterTemplate{
private:
	std::vector<TokenDescriptor> Template;
	double goodness;
	double AvgLen;
	sqlite3_int64 id;

public:
	
	/**
	* Constructor
	* @param[in] Template The starting template of the cluster
	* @param[in] goodness The starting goodness value
	* @param[in] AvgLen The formerly calculated average line length of the cluster
	* @param[in] id The id number to use in the cluster (set 0 if you don't know it yet)
	*/
	ClusterTemplate(const std::vector<TokenDescriptor>& Template,
		double goodness,double AvgLen,sqlite3_int64 id):Template(Template),
		goodness(goodness),AvgLen(AvgLen),id(id){}
	
	/**
	* @return The current average line length of the cluster.
	*/
	double getAvgLen() const{return AvgLen;}
	
	/**
	* @return The current goodness of the cluster.
	*/
	double getGoodness() const{return goodness;}
	
	/**
	* @return A unique number that identifies the cluster briefly
	*/
	sqlite3_int64 getId() const{return id;}
	
	/**
	* @return The current cluster template
	*/
	const std::vector<TokenDescriptor>& getTemplate() const{return Template;}

	/**
	* This method sets the ID of the cluster (sometimes it gets known only after
	* building the object)
	*
	* @param[in] id The identifier number to set
	*/
	void setId(sqlite3_int64 id){this->id=id;}
	
	double getGoodness(const std::vector<TokenDescriptor>&) const;
	bool match(const std::vector<TokenDescriptor>&) const;
	void join(const std::vector<TokenDescriptor>&,double);
	std::string getValueStr() const;
	std::string getUpdateStr() const;
};

#endif
