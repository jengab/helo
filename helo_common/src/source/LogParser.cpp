#include <regex>
#include "LogParser.h"

const wchar_t* LogParser::TemplateNodeName(L"template");
const wchar_t* LogParser::GoodnessAttributeName(L"goodness");
const wchar_t* LogParser::AvgLenAttributeName(L"AvgLen");
const wchar_t* LogParser::ClusterNodeName(L"cluster");
const wchar_t* LogParser::IdAttributeName(L"id");
const wchar_t* LogParser::TokenTypeAttributeName(L"type");
const wchar_t* LogParser::TokenValAttributeName(L"value");
const wchar_t* LogParser::TokenNodeName(L"token");

/**
 * @param[in] HeaderLen The length of header part (irrelevant prefix) of log messages (on syslog protocol this is typically 4)
 * @param[in] regexp The regular expression used for tokenizing the log messages
 */
LogParser::LogParser(size_t HeaderLen,const std::wstring& regexp):HeaderLen(HeaderLen),regexp(regexp){
  Content=std::make_shared<ListOfLines>();
  dict=std::make_shared<Dictionary>();
  dict->insert(std::make_shared<TokenDescriptor>(L"*",Word));
  dict->insert(std::make_shared<TokenDescriptor>(L"+d",Number));
  dict->insert(std::make_shared<TokenDescriptor>(L"+n",Word));
}

/**
 * This method processes a Hybrid token, it replaces the original token with its longest Word prefix.
 *
 * @param[in,out] str The token to process
 */
void LogParser::ProcessHybrid(std::wstring& str) const {
  size_t len=str.length();
  size_t j=0;
  bool firstAlphaFound=false;
  size_t firstAlphaIndex=0;
  size_t lastAlphaIndex=0;

  for(j=0;j<len;++j){
    if(!firstAlphaFound && iswalpha(str[j])){
      firstAlphaFound=true;
      firstAlphaIndex=j;
    }

    if(firstAlphaFound && !iswalpha(str[j])){
      lastAlphaIndex=j;
      break;
    }
  }

  str=str.substr(firstAlphaIndex,lastAlphaIndex-firstAlphaIndex);

}

/**
 * This method categorizes a token to one of the following categories:
 *  Word, Number, Hybrid. Hybrid tokens are those who are not word and nor
 * they are Numbers. If the token is Hybrid then it gets modified also by
 * ProcessHybrid method.
 *
 * @param[in,out] word The token to classify
 * @return The determined category, and the processed token
 */
wordtype LogParser::parse(std::wstring& word) const {
  wordtype ret=Word;
  unsigned int Index=0;
  unsigned int WordLen=word.length();
  bool accept=true;
  bool IsHex=false;

  if(iswdigit(word[0]) || word[0]=='-' || word[0]=='+'){
    ret=Number;
  }
  Index++;

  if(!iswalnum(word[0]) && ret!=Number){
    ProcessHybrid(word);
    return Hybrid;
  }

  if(word[0]=='0' && word[1]=='x'){
    ret=Number;
    Index=2;
    IsHex=true;
  }

  while(Index<WordLen){

    if(Index==WordLen-1 && word[Index]=='\n') return ret;

    if(IsHex==true && (!iswalnum(word[Index]) || (word[Index]>'F' && word[Index]<'a') || word[Index]>'f')){
      ProcessHybrid(word);
      return Hybrid;
    }

    //current state is number, but received alpha
    if(IsHex!=true && ret==Number && iswalpha(word[Index])){
      ProcessHybrid(word);
      return Hybrid;
    }

    //current state is word, but received digit
    if(ret==Word && iswdigit(word[Index])){
      ProcessHybrid(word);
      return Hybrid;
    }

    //hybrid return for not alphanumerical input
    if(!iswalnum(word[Index])){
      if(ret==Number && accept==true && (word[Index]=='.' || word[Index]==',')){
        accept=false;
      }
      else{
        ProcessHybrid(word);
        return Hybrid;
      }
    }
    Index++;
  }

  return ret;
}

/**
 * @return The set of words that could be found in the input after preprocessing
 */
const DictionaryPtr LogParser::getDictionary() const{
  return dict;
}

/**
 * @return The preprocessed content of the input
 */
const std::shared_ptr<ListOfLines> LogParser::getContent() const{
  return Content;
}

/**
 * This method implements how a BaseParser object can be written to a stream.
 * It is used only for debugging, thus we can write to the console the
 * preprocessed file, and can compare it to the original file.
 *
 * @param[in,out] o The stream to write to
 * @param[in] parser The LogParser object to write out
 * @return The output stream used for writing
 */
std::wostream& operator<<(std::wostream& o,const LogParser& parser){
  for(const ArrayOfWords& ActLine:*(parser.Content)){
    for(const std::shared_ptr<TokenDescriptor> ActWord:ActLine){
      o << ActWord->TokenString << " ";
    }
    o << std::endl;
  }

  return o;
}


/**
 * This method implements the read and preprocess of an input stream
 * to a LogParser object. Usually the stream here is the input log file.
 *
 * @param[in,out] is The stream which gives us the input(log)
 * @param[in,out] parser The LogParser object to be filled with content
 * @return A reference to the used input stream
 */
std::wistream& operator>>(std::wistream& is,LogParser& parser){
  while(!is.eof()){
    std::wstring msg;

    try{
      std::wstring ActLine;
      getline(is,ActLine);
      if(ActLine.empty()) continue;

      std::wostringstream msgStream;
      size_t TokenCounter=0;
      std::wstring ActToken;
      for(std::wistringstream iss(ActLine);!iss.eof();TokenCounter++){
        if(TokenCounter<parser.HeaderLen){ //drop header tokens away
          iss >> ActToken;
          continue;
        }

        iss >> ActToken;
        msgStream << ActToken << " ";
      }
      msg=msgStream.str();
    }
    catch(...){
      if(!is.eof()){
        throw;
      }
    }

    std::wregex expr(parser.regexp);
    std::wsregex_token_iterator WordIterator(msg.begin(), msg.end(), expr, -1);
    std::wsregex_token_iterator End;

    ArrayOfWords LineArray;
    for(;WordIterator!=End;++WordIterator){
      std::wstring ActWord=*WordIterator;
      wordtype TokenType=parser.parse(ActWord);
      if(ActWord.empty()) continue;

      std::shared_ptr<TokenDescriptor> ActDesc=std::make_shared<TokenDescriptor>(ActWord,TokenType);
      auto it=parser.dict->find(ActDesc);
      if(it==parser.dict->end()){
        parser.dict->insert(ActDesc);
        LineArray.push_back(ActDesc);
      }
      else{
        LineArray.push_back(*it);
      }
    }

    if(!LineArray.empty()){
      parser.Content->push_back(LineArray);
    }
  }

  return is;
}
