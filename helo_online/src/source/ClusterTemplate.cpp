#include "ClusterTemplate.h"

#include <boost/spirit/home/support/utf8.hpp>

/**
* This method tests if a log messages matches the represented cluster template
* 
* @param[in] msg The syslog message to test
* @return true if the message matches the template represented by the object, false otherwise
*/
bool ClusterTemplate::match(const std::vector<TokenDescriptor>& msg) const{
	for(size_t i=0;i<msg.size() && i<Template.size();++i){
		if(Template[i].TokenString==L"*") continue; //Anything matches *
		if(Template[i].TokenString==L"+d" && msg[i].TypeOfToken==Number) continue; //accept number matching
		if(Template[i].TokenString==L"+n") return true;
		if(msg[i].TokenString!=Template[i].TokenString) return false;
	}

	return msg.size()==Template.size();
}

/**
* This method calculates the goodness that would be correct if we appended the
* provided message to the represented cluster. This method does not change the cluster!
* 
* @param[in] msg The message to try
* @return The goodness value that is correct if msg was added to the cluster
*/
double ClusterTemplate::getGoodness(const std::vector<TokenDescriptor>& msg) const{
	double NewAvgLen=(AvgLen+msg.size())/2;
	size_t CommonWordCounter=0;

	for(size_t i=0;i<msg.size() && i<Template.size();++i){
		if(Template[i].TokenString==L"+n") break;
		if(Template[i].TokenString==msg[i].TokenString || //match for exact match
			(Template[i].TokenString==L"+d" && msg[i].TypeOfToken==Number)){ //match for number
			++CommonWordCounter;
		}
	}

	return CommonWordCounter/NewAvgLen;
}

/**
* This method adds a given line to the represented cluster, it calculates the
* new goodness, average length and template values. It really modifies the object.
* 
* @param[in] line The line to append to the cluster
* @param[in] NewGoodness The new goodness value for the cluster. Use getGoodness to determine it!
*/
void ClusterTemplate::join(const std::vector<TokenDescriptor>& line,double NewGoodness){
	goodness=NewGoodness;
	AvgLen=(AvgLen+line.size())/2;

	for(size_t i=0;i<Template.size() && i<line.size();++i){
		if(Template[i].TokenString==L"+n") break;
		if(Template[i].TokenString!=line[i].TokenString && Template[i].TokenString!=L"*"){ //constant tokens and * are not modified
			if(line[i].TypeOfToken!=Number){  //received token is not a number => template only can be (*,Word)
				Template[i].TypeOfToken=Word;
				Template[i].TokenString=L"*";
			}
			else if(Template[i].TokenString!=L"+d"){ //received token is a number and the template is not +d
				if(Template[i].TypeOfToken==Number){ //template is also number, so we can provide +d
					Template[i].TokenString=L"+d";
				}
				else{ //template is not a number so we must set * and Word
					Template[i].TokenString=L"*";
				}
			}
		}
	}

	if(line.size()>Template.size()){
		Template.push_back(TokenDescriptor(L"+n"));
	}
	else if(line.size()<Template.size()){
		for(size_t i=Template.size();i>line.size()+1;--i) Template.pop_back();
		Template[line.size()]=TokenDescriptor(L"+n");
	}
}

/**
* @return The SQLite command that inserts the cluster to the clusters' table
* (This is normally used on clusters that are not already present in the database)
*/
std::string ClusterTemplate::getValueStr() const{
	std::ostringstream values;
	values << "INSERT INTO clusters VALUES(NULL,\"";

	for(const TokenDescriptor& ActToken:Template){
		values << boost::spirit::to_utf8<wchar_t>(ActToken.TokenString) << " ";
	}

	values << "\"," << goodness << "," << AvgLen << ")";

	return values.str();
}

/**
* @return The SQLite command that updates the cluster
* (this is normally used for such clusters that are already
* stored in the database)
*/
std::string ClusterTemplate::getUpdateStr() const{
	std::ostringstream update;
	update << "UPDATE clusters SET goodness=" << goodness << ",AvgLen=" << AvgLen << ",template=\"";

	for(const TokenDescriptor& ActToken:Template){
		update << boost::spirit::to_utf8<wchar_t>(ActToken.TokenString) << " ";
	}

	update << "\"" << "WHERE clustid=" << id;

	return update.str();
}