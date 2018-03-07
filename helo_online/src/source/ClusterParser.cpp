#include "ClusterParser.h"
#include "LogParser.h"
#include "pugixml.hpp"
#include "OutputHandler.h"
#include <sstream>
#include <regex>
#include <codecvt>
#include <string.h>

inline std::wstring from_utf(const char* bytes) {
    std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;
    return converter.from_bytes(bytes);
}

inline std::string to_utf(const std::wstring& str) {
    std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;
    return converter.to_bytes(str);
}

/**
* Constructor
* @param[in] s A reference to an object that stores the loaded
* settings (Header length, regular expression, etc.)
*/
ClusterParser::ClusterParser(const Settings& s):settings(s){
	SQLite::Database db(settings.DbFile.c_str(),SQLITE_OPEN_READWRITE);
	SQLite::Statement query(db,"SELECT * FROM clusters");

	while(query.executeStep()){
		sqlite3_int64 id=query.getColumn(0).getInt64();

		std::vector<TokenDescriptor> Template;
		std::wistringstream TemplStr(from_utf((const char*)query.getColumn(1)));
		while(!TemplStr.eof()){
			std::wstring ActToken;
			TemplStr >> ActToken;
			if(ActToken.empty()) continue;
			wordtype type=Word;
			if(ActToken==L"+d") type=Number;
			Template.push_back(TokenDescriptor(ActToken,type));
		}
		if(Template.empty()) continue;

		double goodness=query.getColumn(2);
		double AvgLen=query.getColumn(3);
		Clusters.push_back(ClusterTemplate(Template,goodness,AvgLen,id));
	}
}

/*std::wistream& operator>>(std::wistream& is,ClusterParser& parser){
	pugi::xml_document doc;
	doc.load(is);

	for(pugi::xml_node& ActClust:doc){ //iterate on cluster nodes
		double goodness=ActClust.attribute(LogParser::GoodnessAttributeName).as_double();
		double AvgLen=ActClust.attribute(LogParser::AvgLenAttributeName).as_double();
		size_t Id=ActClust.attribute(LogParser::IdAttributeName).as_uint();
		std::vector<TokenDescriptor> Template;

		for(pugi::xml_node& Templ:ActClust){ //iterate on tokens
			Template.push_back(TokenDescriptor(Templ.attribute(LogParser::TokenValAttributeName).value()
				,(wordtype)Templ.attribute(LogParser::TokenTypeAttributeName).as_int()));
		}

		parser.Clusters.push_back(ClusterTemplate(Template,goodness,AvgLen,Id));
	}

	return is;
}*/

/**
* This method is mainly only for debugging. It writes out the
* read data in XML format. Normally this method is not used
*
* @param[in,out] os The stream to write to
* @param[in] parser The parser object to write out
* @return A reference to the provided output stream
*/
std::wostream& operator<<(std::wostream& os,ClusterParser& parser){
	pugi::xml_document doc;

	for(ClusterTemplate& ActTempl:parser.Clusters){
		pugi::xml_node ClustNode=doc.append_child(LogParser::ClusterNodeName);
		ClustNode.append_attribute(LogParser::IdAttributeName)=(unsigned int)ActTempl.getId();
		ClustNode.append_attribute(LogParser::GoodnessAttributeName)=ActTempl.getGoodness();
		ClustNode.append_attribute(LogParser::AvgLenAttributeName)=ActTempl.getAvgLen();

		for(const TokenDescriptor& ActTok:ActTempl.getTemplate()){
			pugi::xml_node TemplNode=ClustNode.append_child(LogParser::TokenNodeName);
			TemplNode.append_attribute(LogParser::TokenValAttributeName)=ActTok.TokenString.c_str();
			TemplNode.append_attribute(LogParser::TokenTypeAttributeName)=ActTok.TypeOfToken;
		}
	}

	doc.print(os);
	return os;
}

/**
* This method is used for debugging. It pints out the read data
* in plain text
*/
void ClusterParser::printToConsole() const{
	for(const ClusterTemplate& ActTempl:Clusters){
		std::wcout << ActTempl.getId() << " Cluster goodness: " << ActTempl.getGoodness() << " AvgLen: " << ActTempl.getAvgLen() << std::endl;
		for(const TokenDescriptor& ActToken:ActTempl.getTemplate()){
			std::wcout << ActToken.TokenString << " ";
		}
		std::wcout << std::endl;
	}
}

/**
* This method implements the processing of a newly arrived message.
* It does INSERT,UPDATE on the SQLite file. This method is thread-safe!
* The parameter string gets modified in this method!
*
* @param[in,out] line The syslog message encoded in UTF-8 to be processed.
*/
void ClusterParser::ProcessMessage(std::wstring& line){
	size_t start_pos=0;
	while((start_pos=line.find(L'\"',start_pos))!=std::wstring::npos){
		line.replace(start_pos,1,L"\'");
		++start_pos;
	}


	std::wistringstream Tokens(line);
	std::wstring msg;
	{
		std::wstring ActToken;
		std::wostringstream msgStream;
		size_t TokenCounter=0;
		for(std::wstringstream Tokens(line);!Tokens.eof();++TokenCounter){
			if(TokenCounter<settings.HeaderLen){
				Tokens >> ActToken;
				continue;
			}

			Tokens >> ActToken;
			msgStream << ActToken << " ";
		}
		msg=msgStream.str();
	}

	std::vector<TokenDescriptor> LineVect;
	std::wregex expr(settings.regexp);
	std::wsregex_token_iterator TokenIterator(msg.begin(),msg.end(),expr,-1);
	std::wsregex_token_iterator End;
	for(;TokenIterator!=End;++TokenIterator){
		std::wstring ActToken=*TokenIterator;

		wordtype ActType=LogParser::parse(ActToken);
		if(!ActToken.empty()) LineVect.push_back(TokenDescriptor(ActToken,ActType));
	}
	if(LineVect.empty()) return;

    std::lock_guard<std::mutex> writerGuard(WriteMutex);
    ClusterTemplate* ClusterAssigned=NULL;
	double MaxGoodness=0;
	for(ClusterTemplate& ActTempl:Clusters){
		if(ActTempl.match(LineVect)){ //exact match, we can assign the id only
			std::ostringstream query;
			try{
				SQLite::Database db(settings.DbFile.c_str(),SQLITE_OPEN_READWRITE);
				query << "INSERT INTO syslog VALUES(" << ActTempl.getId() << ",\"" << to_utf(msg) << "\")";
				db.exec(query.str().c_str());
			}
			catch(const SQLite::Exception& e){
				OutputHandler::logException(settings.ErrorStream,"An exception occured in the database. Query: "+query.str()+"\n",e);
			}
			return;
		}

		double ActGoodness=ActTempl.getGoodness(LineVect);
		if(ActGoodness>MaxGoodness){
			ClusterAssigned=&ActTempl;
			MaxGoodness=ActGoodness;
		}
	}

	SQLite::Database db(settings.DbFile.c_str(),SQLITE_OPEN_READWRITE);
	if(MaxGoodness>=settings.lim && ClusterAssigned!=NULL){
		ClusterAssigned->join(LineVect,MaxGoodness);
		try{
			SQLite::Transaction tr(db);
			db.exec(ClusterAssigned->getUpdateStr().c_str()); //update clusters
			std::ostringstream query;
			query << "INSERT INTO syslog VALUES(" << ClusterAssigned->getId() << ",\"" << to_utf(msg) << "\")";
			db.exec(query.str().c_str());
			tr.commit();
		}
		catch(const SQLite::Exception& e){
			OutputHandler::logException(settings.ErrorStream,"An error occured in the database: ",e);
		}
	}
	else{
		ClusterTemplate ActTempl(LineVect,1,LineVect.size(),0);
		try{
			SQLite::Transaction tr(db);
			db.exec(ActTempl.getValueStr().c_str()); //insert to clusters
			sqlite3_int64 Id=db.getLastInsertRowid();
			std::ostringstream query;
			query << "INSERT INTO syslog VALUES(" << Id << ",\"" << to_utf(msg) << "\")";
			db.exec(query.str().c_str());
			ActTempl.setId(Id);
			tr.commit();
			Clusters.push_back(ActTempl);
		}
		catch(const SQLite::Exception& e){
			OutputHandler::logException(settings.ErrorStream,"An error occured in the database: ",e);
		}
	}
}
