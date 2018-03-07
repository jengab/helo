#ifndef LOG_PARSER_H
#define LOG_PARSER_H

#include <string>
#include <list>
#include <set>
#include <vector>
#include <sstream>
#include <memory>
#include <locale>

/**
 * @file LogParser.h
 *
 * This file contains the LogParser class and the utilities needed to implement LogParser
 * @author Jenei Gábor <jengab@elte.hu>
 */

/**
 * \enum wordtype
 * This \c enum represents the type of the read token (Word,Hybrid,Number)
 */
enum wordtype{
	Word,Hybrid,Number
};

/**
 * This class represents a token, it stores the token type and the actual string (value)
 * of the token
 */
class TokenDescriptor{
public:

	/**
	 * @param[in] str The token string to use
	 * @param[in] type The determined token type
	 * @see LogParser::parse()
	 */
	TokenDescriptor(std::wstring str,wordtype type):TokenString(str),TypeOfToken(type){}

	/**
	 * This constructor shouldn't be used generally, it is only for search,
	 * as comparison between tokens is defined according to the strings only.
	 * This constructor sets the TypeOfToken attribute to Word without minding
	 * whether it is correct or not.
	 *
	 * @param[in] str The token string to use
	 */
	TokenDescriptor(std::wstring str):TokenString(str),TypeOfToken(Word){}

	/**
	 * The actual value (string of token)
	 */
	std::wstring TokenString;

	/**
	 * The set type of token
	 * @see wordtype
	 */
	wordtype TypeOfToken;
};

/**
 * This class only implements a comparison for TokenDescriptor smart pointers.
 * The comparison is an ABC sort according to the token strings.
 */
class WordComparator{
public:

	/**
	 * This is the only method of this class, it implements the comparison between TokenDescriptor
	 * smart pointers, it is based on the token strings.
	 *
	 * @param[in] first The first token to compare
	 * @param[in] second The second token to compare
	 * @return true if first token string precedes the second one according to the ABC sort
	 */
	bool operator()(const std::shared_ptr<TokenDescriptor> first,const std::shared_ptr<TokenDescriptor> second){
		return first->TokenString<second->TokenString;
	}
};

/**
 * \c typedef for a std::vector of smart pointer of TokenDescriptor
 * This type represents one line of a cluster, or one line of the input file.
 * It can be viewed as an array of Word pointers. Pointer here is only because of
 * memory efficiency issues.
 */
typedef std::vector< std::shared_ptr<TokenDescriptor> > ArrayOfWords;

/**
 * \c typedef for a list of ArrayOfWords. This type represents a list of several lines,
 * it can be used as the content of a cluster/input file.
 */
typedef std::list<ArrayOfWords> ListOfLines;

/**
 * \c typedef for an std::set of TokenDescriptor pointers, this type is for a collection of words.
 * It was introduced because of memory efficiency issues, thus we may store each word only once.
 * (normally two appearances of the same  word would be stored twice.
 */
typedef std::set<std::shared_ptr<TokenDescriptor>,WordComparator> Dictionary;

/**
 * \c typedef for smart pointer to a Dictionary
 */
typedef std::shared_ptr<std::set<std::shared_ptr<TokenDescriptor>,WordComparator>> DictionaryPtr;

/**
 * @details <p>This class reads and preprocesses a log. The log can be given in the
 * form of any input stream. </p>
 * <p>It handles <b>wide streams</b>, thus we can handle also national characters
 * in the input. </p>
 * <p>After reading a line we <b>divide it to tokens</b>, and then we parse each token.
 * <b>Parsing</b> rules can be set by regular expressions, or else we have a default built in parsing.
 * Here parsing means only the classification and preprocess of the identified tokens.
 * Basically every token can be classified to one of the three distinct classes, these are described in
 * wordtype . </p>
 * <p><b>Preprocess</b> is only done for Hybrid tokens, their first few non alphabetic characters are
 * thrown away. After this we take all the alphabetic characters, and throw away again the
 * non alphabetic characters at the end of the token. Briefly we throw away the leading and the trailing
 * non alphabetic characters.</p>
 */
class LogParser{
private:
	LogParser();
	LogParser(LogParser&);

protected:
	std::shared_ptr<ListOfLines> Content;
	size_t HeaderLen;
	DictionaryPtr dict;
	std::wstring regexp;
	static void ProcessHybrid(std::wstring&);

public:
	LogParser(size_t,const std::wstring&);
	const DictionaryPtr getDictionary() const;
	const std::shared_ptr<ListOfLines> getContent() const;


	static wordtype parse(std::wstring&);

	/// The XML tag used in the cluster output
	/// to denote the end/beginning of a template description
	///
	static const wchar_t* TemplateNodeName;

	/// The XML attribute used in the cluster output
	/// to store goodness value.
	/// This is the attribute of the Cluster node
	///
	static const wchar_t* GoodnessAttributeName;

	/// The XML attribute used in the cluster output
	/// for average length.
	/// This is the attribute of the Cluster node
	///
	static const wchar_t* AvgLenAttributeName;

	/// The XML attribute used in the cluster output
	/// for the cluster's ID number value.
	/// This is the attribute of the Cluster node
	///
	static const wchar_t* IdAttributeName;

	/// The XML tag used in the cluster output
	/// to represent the beginning/end of a cluster.
	///
	static const wchar_t* ClusterNodeName;

	/// The XML tag used in the cluster output
	/// to represent the beginning/end of a token
	/// description.
	///
	static const wchar_t* TokenNodeName;

	/// The XML attribute that denotes the token's
	/// value. It's the attribute of token node
	///
	static const wchar_t* TokenValAttributeName;

	/// The XML attribute that denotes the token's
	/// type. It's the attribute of token node
	///
	static const wchar_t* TokenTypeAttributeName;

	friend std::wistream& operator>>(std::wistream& is,LogParser& parser);
	friend std::wostream& operator<<(std::wostream&,const LogParser&);

};

#endif
