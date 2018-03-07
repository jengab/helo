#ifndef OUTPUT_HANDLER_H
#define OUTPUT_HANDLER_H

#include <mutex>

/**
* @file OutputHandler.h
*
* This file contains OutputHandler class, a thread-safe implementation
* of program output.
* @author Jenei Gábor <jengab@elte.hu>
*/

/**
* This class implements thread-safe output handling
*/
class OutputHandler{
private:
	static std::mutex locker;

public:

	/**
	* Prints a message to an output stream in a thread-safe way.
	*
	* @param[in] ostream The stream to print on
	* @param[in] str The message to print
	*/
	static void print(std::ostream* ostream,const std::string& str);

	/**
	* Logs an exception with a custom comment to an output stream in a thread-safe way
	*
	* @param[in] ostream The stream where we log
	* @param[in] str The manually set custom comment (leave it blank if you don't use it)
	* @param[in] ex The exception to log
	*/
	static void logException(std::ostream* ostream,const std::string& str,const std::exception& ex);
};

#endif
