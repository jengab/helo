#include <chrono>
#include <iomanip>
#include <iostream>
#include "OutputHandler.h"

std::mutex OutputHandler::locker;

/**
 * Prints a message to an output stream in a thread-safe way.
 *
 * @param[in] ostream The stream to print on
 * @param[in] str The message to print
 */
void OutputHandler::print(std::ostream* ostream,const std::string& str){
    std::lock_guard<std::mutex> outputLock(locker);
    auto ActTime=std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
    (*ostream) << "[" << std::put_time(std::localtime(&ActTime), "%Y-%m-%d %X") << " Info] " << str;
}

/**
 * Logs an exception with a custom comment to an output stream in a thread-safe way
 *
 * @param[in] ostream The stream where we log
 * @param[in] str The manually set custom comment (leave it blank if you don't use it)
 * @param[in] ex The exception to log
 */
void OutputHandler::logException(std::ostream* ostream,const std::string& str,const std::exception& ex){
    std::lock_guard<std::mutex> outputLock(locker);
    auto ActTime=std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
    (*ostream) << "[" << std::put_time(std::localtime(&ActTime), "%Y-%m-%d %X") << " Error] " << str << ex.what() << std::endl;
}
