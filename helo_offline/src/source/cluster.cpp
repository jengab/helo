#include <stdexcept>
#include <map>
#include <codecvt>
#include "cluster.h"
#include "pugixml.hpp"

/**
 * @param[in] lines A list of lines with the contents of the cluster
 * @param[in] dict A set that contains the words used in the cluster
 */
Cluster::Cluster(const std::shared_ptr<ListOfLines> lines,const DictionaryPtr dict):Content(lines),dict(dict),id(0),TotalLineLen(0){
    TotalLineCount=Content->size();
    CalcStatistics();
}

/**
 * @return The column's number to split on, -1 if there is no ideal column
 */
int Cluster::getSplit(){
    double max=-1;
    int pos=-1;
    for(size_t ind=0;ind<MaxLineLen;++ind){
        size_t NoDistinctValues=Values[ind].size();
        if(NoDistinctValues>1 &&
                max<(double)FilledNonNumColumns[ind]/NoDistinctValues &&
                (double)FilledNonNumColumns[ind]/Content->size()>PERCENT_OF_FILL
          ){
            max=(double)FilledNonNumColumns[ind]/NoDistinctValues;
            pos=ind;
        }
    }

    return pos;
}

/**
 * Splits a cluster to several smaller subclusters
 *
 * @param[in,out] ClusterList A reference to a list where output clusters can be stored
 * @throws std::invalid_argument if the cluster can't be split yet (there is no proper split position)
 */
void Cluster::Split(ListOfClusters& ClusterList){
    int Position=getSplit();
    if(Position==-1) throw std::invalid_argument("The cluster is not splitable yet!\n");
    std::map<std::shared_ptr<TokenDescriptor>,std::shared_ptr<ListOfLines>,WordComparator> Clusters;

    for(ArrayOfWords& ActLine:*Content){
        std::shared_ptr<TokenDescriptor> ClusterLabel;

        try{
            std::shared_ptr<TokenDescriptor> TokDesc=ActLine.at(Position);
            if(TokDesc->TypeOfToken==Number){
                ClusterLabel=*(dict->find(std::make_shared<TokenDescriptor>(L"+d")));
            }
            else{
                ClusterLabel=TokDesc;
            }
        }
        catch(const std::out_of_range&){
            ClusterLabel=*(dict->find(std::make_shared<TokenDescriptor>(L"+n")));
        }

        try{
            Clusters.at(ClusterLabel)->push_back(ActLine);
        }
        catch(std::out_of_range&){
            Clusters.insert(std::make_pair(ClusterLabel,std::make_shared<ListOfLines>()));
            Clusters[ClusterLabel]->push_back(ActLine);
        }
    }

    for(const auto& ActEntry:Clusters){
        ClusterList.push_back(Cluster(ActEntry.second,dict));
    }
}

void Cluster::CalcStatistics(){
    double AvgLen=0;
    MaxLineLen=0;
    FilledColumns.clear();
    FilledNonNumColumns.clear();
    Values.clear();

    for(const ArrayOfWords& ActLine:*Content){
        if(ActLine.size()>MaxLineLen){
            MaxLineLen=ActLine.size();
            Values.resize(MaxLineLen);
            FilledColumns.resize(MaxLineLen);
            FilledNonNumColumns.resize(MaxLineLen);
        }
        AvgLen+=ActLine.size();

        for(size_t ActPos=0;ActPos<ActLine.size();++ActPos){
            Values[ActPos].insert(ActLine[ActPos]);
            if(ActLine[ActPos]->TypeOfToken!=Number) FilledNonNumColumns[ActPos]++;
            FilledColumns[ActPos]++;
        }
    }

    size_t CommonWordCounter=0;
    for(size_t i=0;i<Values.size();++i){
        if(Values[i].size()==1 && FilledColumns[i]==Content->size()) CommonWordCounter++;
    }

    if(TotalLineLen==0) TotalLineLen=AvgLen;
    AvgLen/=Content->size();

    if(Content->size()==0 || MaxLineLen==0){
        goodness=1;
    }
    else{
        goodness=(double)CommonWordCounter/AvgLen;
    }
}

/**
 * @return The calculated goodness (a measure of similarity for lines contained in the cluster) value
 */
double Cluster::getGoodness(){
    return goodness;
}

/**
 * This method compares two clusters, it is used in the merging stage of the algorithm.
 *
 * @param[in] other A reference to the other cluster
 * @return The relative goodness of the clusters (it is a measure of closeness here).
 */
double Cluster::getGoodness(Cluster& other){
    if(Content->empty() ||other.Content->empty()) return 0;
    size_t CommonWordCounter=0;
    ArrayOfWords& this_template=(*(Content->begin()));
    ArrayOfWords& other_template=(*(other.Content->begin()));
    size_t this_length=this_template.size();
    size_t other_length=other_template.size();
    double AvgLineLen=(double)(this_length+other_length)/2;

    for(size_t ActPos=0;ActPos<this_length && ActPos<other_length;++ActPos){
        if(this_template[ActPos]->TokenString==other_template[ActPos]->TokenString) CommonWordCounter++;
    }

    return (double)CommonWordCounter/AvgLineLen;
}

/**
 * @return A template that describes the cluster
 */
ArrayOfWords Cluster::getTemplate(){
    ArrayOfWords Template;

    std::vector<wordtype> AggregatedType(MaxLineLen,Number);
    for(const ArrayOfWords& ActLine:*Content){
        size_t LineLen=ActLine.size();
        for(size_t i=0;i<LineLen;++i){
            if(ActLine[i]->TypeOfToken!=Number) AggregatedType[i]=ActLine[i]->TypeOfToken;
        }
    }

    ArrayOfWords& FirstLine=(*(Content->begin()));
    for(size_t WordCounter=0;WordCounter<MaxLineLen;++WordCounter){
        if(FilledColumns[WordCounter]==Content->size()){ //isn't it +n
            if(Values[WordCounter].size()>1){ //is it constant?
                if(AggregatedType[WordCounter]!=Number){
                    Template.push_back(*(dict->find(std::make_shared<TokenDescriptor>(L"*"))));
                }
                else{
                    Template.push_back(*(dict->find(std::make_shared<TokenDescriptor>(L"+d"))));
                }

            }
            else{
                Template.push_back(FirstLine[WordCounter]);
            }
        }
        else{
            Template.push_back(*(dict->find(std::make_shared<TokenDescriptor>(L"+n"))));
            break;
        }
    }
    return Template;
}

/**
 * This method deletes all lines of the cluster, and stores only a template line
 * calculated from the former content.
 */
void Cluster::compressToTemplate(){
    ArrayOfWords Template=getTemplate();
    Content->clear();
    Content->push_back(Template);
}

/**
 * This method joins two very similar clusters, because they were probably split unnecessarily.
 * Finally a common cluster template is made inside the first cluster.
 *
 * @param[in,out] other A reference to the cluster to join with. This cluster's content will be
 * totally deleted
 */
void Cluster::join(Cluster& other){
    size_t SumLen=0;
    for(const ArrayOfWords& ActLine:*other.Content){
        SumLen+=ActLine.size();
        Content->push_back(ActLine);
    }

    TotalLineLen+=SumLen;
    TotalLineCount+=other.TotalLineCount;
    other.Content->clear();

    ListOfLines::iterator it=Content->begin();
    ArrayOfWords& FirstLine=(*it++);
    ArrayOfWords& SecondLine=(*it);
    ArrayOfWords Template;
    for(size_t i=0;i<FirstLine.size() && i<SecondLine.size();++i){
        if(FirstLine[i]->TokenString==L"+n" || SecondLine[i]->TokenString==L"+n"){
            Template.push_back(*dict->find(std::make_shared<TokenDescriptor>(L"+n")));
            Content->clear();
            Content->push_back(Template);
            return;
        }

        wordtype AggregateType=Number;
        if(FirstLine[i]->TypeOfToken!=Number || SecondLine[i]->TypeOfToken!=Number) AggregateType=Word;
        if(FirstLine[i]->TokenString==SecondLine[i]->TokenString){ //insert the string
            Template.push_back(FirstLine[i]);
        }
        else{
            if(AggregateType==Number){ //insert +d
                Template.push_back(*dict->find(std::make_shared<TokenDescriptor>(L"+d")));
            }
            else{ //insert *
                Template.push_back(*dict->find(std::make_shared<TokenDescriptor>(L"*")));
            }
        }
    }

    if(FirstLine.size()!=SecondLine.size()){ //push_back +n
        Template.push_back(*dict->find(std::make_shared<TokenDescriptor>(L"+n")));
    }
    Content->clear();
    Content->push_back(Template);
}

/**
 * This method implements how a cluster can be written to a stream
 *
 * @param[in,out] o A reference to the output stream to write to
 * @param[in] c A constant reference to the cluster to write out
 * @return A reference to the output stream that was used for writing
 */
std::wostream& operator<<(std::wostream& o,const Cluster& c){
    pugi::xml_document doc;

    pugi::xml_node ClustNode=doc.append_child(LogParser::ClusterNodeName);
    ClustNode.append_attribute(LogParser::IdAttributeName)=c.id;
    ClustNode.append_attribute(LogParser::GoodnessAttributeName)=c.goodness;
    ClustNode.append_attribute(LogParser::AvgLenAttributeName)=c.getAvgLen();

    for(const std::shared_ptr<TokenDescriptor> ActWord:(*c.Content->begin())){
        pugi::xml_node TokenNode=ClustNode.append_child(LogParser::TokenNodeName);
        TokenNode.append_attribute(LogParser::TokenValAttributeName)=ActWord->TokenString.c_str();
        TokenNode.append_attribute(LogParser::TokenTypeAttributeName)=ActWord->TypeOfToken;
    }

    doc.print(o);
    return o;
}

/**
 * This method implements cluster output to a SQLite database.
 * The database must be already opened for writing before using this method
 *
 * @param[in,out] db A reference to the Database object, which implements the database API
 * @param[in] c The cluster to write to the database
 * @return Reference to the provided Database object
 */
SQLite::Database& operator<<(SQLite::Database& db,const Cluster& c){
    std::ostringstream query;
    try{
        query << "INSERT INTO clusters VALUES (NULL,\"";
        for(const std::shared_ptr<TokenDescriptor> ActWord:(*c.Content->begin())){
            query << std::wstring_convert<std::codecvt_utf8<wchar_t>>().to_bytes(ActWord->TokenString) << " ";
        }

        query << "\"," << c.goodness << "," << c.getAvgLen() << ")";
        db.exec(query.str().c_str());
    }
    catch(const SQLite::Exception& e){
        std::wcerr << "An SQL error occured: " << e.what() << std::endl << "Query: " << query.str().c_str() << std::endl;
    }

    return db;
}
