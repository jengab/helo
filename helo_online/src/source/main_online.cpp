#include <iostream>
#include <thread>
#include <codecvt>
#include <boost/interprocess/sync/named_semaphore.hpp>
#include <boost/asio.hpp>
#include <pugixml.hpp>
#include "ConfigFile.h"
#include "ClusterParser.h"
#include "OutputHandler.h"

/**
* @file main_online.cpp
*
* This file contains the main() function of the online algorithm,
* and all the thread functions used in the algorithm.
* @author Jenei Gábor <jengab@elte.hu>
*/

using namespace boost::interprocess;
using boost::asio::ip::tcp;
using namespace std;

/// \c typedef for \c shared_ptr on boost TCP socket
///
typedef std::shared_ptr<tcp::socket> socketPtr;

/// \c typedef for \c shared_ptr on boost acceptor
///
typedef std::shared_ptr<tcp::acceptor> acceptorPtr;

/// This string identifies the stop event on the operating system
///
const char* SemaphoreName="STOP_HELO_ONLINE";

/**
* This is a thread function that waits for stop event,
* and closes the application if stop event happened.
* The thread gets blocked until stop event happens.
*
* @param[in] acceptor The acceptor object used for listening
*/
void StopHandler(acceptorPtr acceptor){
    named_semaphore(open_or_create,SemaphoreName,0).wait();
    try{
        tcp::endpoint dest=acceptor->local_endpoint();
        boost::asio::io_service srv;
        tcp::socket tempSock(srv);
        acceptor->close();
        tempSock.connect(dest);
        tempSock.shutdown(tcp::socket::shutdown_both);
        tempSock.close();
    }
    catch(boost::system::system_error&){}
}

/**
* This thread function implements the serving of a client connection.
* It reads the input TCP stream while it is not closed, and processes
* it line by line.
*
* @param[in] sock An object that represents the remote client socket
* @param[in,out] parser A pointer to the ClusterParser object that can be used to process new messages
* @param[in] settings The settings loaded from command prompt/config file
*/
void ServeRequest(socketPtr sock,std::shared_ptr<ClusterParser> parser,const Settings& settings){
    try{
        std::string msg;
        while(true){
            char byte;
            boost::system::error_code error;
            sock->read_some(boost::asio::buffer(&byte,1),error);
            msg+=byte;

            if(byte=='\n'){
                std::wstring NewMsg=std::wstring_convert<std::codecvt_utf8<wchar_t>>().from_bytes(msg);
                //OutputHandler::print(std::cout,NewMsg);
                parser->ProcessMessage(NewMsg);
                msg="";
            }
            if(error==boost::asio::error::eof){
                break;
            }
            else if(error){
                std::cout << "Error occurred in the server thread!\n";
                throw boost::system::system_error(error);
            }
        }
    }
    catch(const std::exception& e){
        OutputHandler::logException(settings.ErrorStream,"Exception in thread: ",e);
    }
}

/**
 * <p>This is the main() function of the online program.</p>
 *
 * <p>It reads the already made clusters from a database file.
 * And then waits for incoming syslog messages on a TCP port of
 * the local computer. After a message is received it gets processed.
 * The program modifies the database while it runs, adds new messages,
 * clusters, and sometimes just refreshes a cluster. The program also
 * waits for a stopping event, if this event happens it closes the TCP
 * port, and immediatelly returns. This is the only safe and regular way.
 * to shut down the program. The stop event happens if we run: \c helo_online \c stop</p>
 *
 * @param[in] argc The number of command line arguments
 * @param[in] argv The array of command line arguments
 *
 * The command line arguments can be:
 *
 * <table>
 * <tr><td>DbFile</td>
 * <td>The path to the SQLite3 database file (used for input and output). This parameter is necessary,
 *  if it is not set then the program looks for a configuration file at "settings.xml"</td></tr>
 * <tr><td>LogPath</td><td>The path for the error log file. It can be set by -lf\<value\> command line parameter.
 * If no parameter is set then the console will be used to show the error messages.</td></tr>
 * <tr><td>port</td>
 * <td>The TCP port to listen on, normally for syslog messages this is 514
 * (this is the default value) it can be set by -p\<value\></td></tr>
 * <tr><td>lim</td>
 * <td> The threshold value for joining (merging) clusters
 * (this parameter is optional, it must be between 0 and 1, and it can be set by -mt\<value\> command line
 * parameter, its default value is 0.4)</td></tr>
 * <tr><td>HeaderLen</td>
 * <td>The length of header part of log messages. (this parameter is optional, and
 * it can be set by -he\<value\> command line parameter, its default value is 4)</td></tr>
 * <tr><td>loc</td>
 * <td>A localization string that sets the localization used for reading the log file.
 * (this parameter is optional, it can be set by -lo\<name\> command line parameter. If there is
 * no localization set the program uses the currently used system localization)</td></tr>
 * <tr><td>regexp</td><td>The regular expression used for tokenizing the input messages. It can be given
 * in POSIX regex format, and it can be set by -re\<regexpr\> command line parameter. Defaultly white spaces
 * will be used as token separators.</td></tr>
 * </table>
 */
int main(int argc,char** argv){
    const string HelpMsg=string("Usage: helo_online <input_file> [options]\n Options: \n")+
                         string("  -p<value> : Sets the port where we receive the log messages (default: 514)\n")+
                         string("  -lo<value> : Sets the localization to use (this is the system localization by default\n")+
                         string("  -he<value> : Sets the length of header part of log messages (4 by default)\n")+
                         string("  -lf<value> : Sets the path of the log file to use\n")+
                         string("  -mt<value> : Sets the goodness threshold for cluster merging\n")+
                         string("  -re<value> : Sets the regular expression used for tokenization\n\n");

    Settings settings;
    settings.port=514;
    settings.HeaderLen=4;
    settings.lim=0.4;
    settings.loc="";
    settings.DbFile="";
    ofstream LogFile;
    string LogPath;

    if(argc>1){
        if(strcmp(argv[1],"stop")==0){
            cout << "Trying to stop the processor...\n";
            named_semaphore(open_or_create,SemaphoreName,0).post();
            return 0;
        }
        if(strcmp(argv[1],"-h")==0){
            cout << HelpMsg;
            return 0;
        }

        settings.DbFile=argv[1];

        for(int i=2;i<argc;++i){
            if(strncmp(argv[i],"-p",2)==0) settings.port=atoi(&argv[i][2]);
            if(strncmp(argv[i],"-he",3)==0) settings.HeaderLen=atoi(&argv[i][3]);
            if(strncmp(argv[i],"-lo",3)==0) settings.loc=string(&argv[i][3]);
            if(strncmp(argv[i],"-mt",3)==0) settings.lim=atoi(&argv[i][3]);

            if(strncmp(argv[i],"-re",3)==0){
                string tempStr(&argv[i][3]);
                settings.regexp=wstring(tempStr.begin(),tempStr.end());
            }

            if(strncmp(argv[i],"-lf",3)==0){
                LogFile.open(&argv[i][3],ios::out | ios::app);
                LogPath=std::string(&argv[i][3]);
                if(LogFile.fail()){
                    cerr << "The log file couldn't be created\n";
                    return -1;
                }
                settings.ErrorStream=&LogFile;
            }
        }
    }
    else{
        try{
            ConfigFile conf;
            settings=conf.getConfig();
            LogFile.open(conf.getLogPath(),ios::out | ios::app);
            LogPath=conf.getLogPath();
            if(LogFile.fail()){
                cerr << "The log file couldn't be created\n";
                return -1;
            }
            settings.ErrorStream=&LogFile;
        }
        catch(std::exception& e){
            OutputHandler::logException(settings.ErrorStream,"",e);
            cout << HelpMsg;
            LogFile.close();
            return -1;
        }
    }

    std::cout << "Starting HELO online. Applied parameters:\n" << " Listen port: " << settings.port << std::endl;
    std::cout << " Cluster file path: " << settings.DbFile << "\n Log file path: " << LogPath << "\n Header length: " << settings.HeaderLen;
    std::cout << "\n Regular expression: " << std::string(settings.regexp.begin(),settings.regexp.end()) << std::endl;

    std::shared_ptr<ClusterParser> proc;
    boost::asio::io_service srv;
    acceptorPtr acceptor=std::make_shared<tcp::acceptor>(srv);

    try{
        locale WordLocale=locale(settings.loc.c_str());
        locale local=locale(WordLocale,locale(),locale::numeric);
        proc=std::make_shared<ClusterParser>(settings);
        //proc->printToConsole();

        tcp::endpoint logger(tcp::v4(),settings.port);
        acceptor->open(logger.protocol());
        acceptor->set_option(tcp::acceptor::reuse_address(true));
        acceptor->bind(logger);
        acceptor->listen();
    }
    catch(const SQLite::Exception& e){
        OutputHandler::logException(settings.ErrorStream,"A problem occurred during reading the database: ",e);
        LogFile.close();
        return -1;
    }
    catch(const boost::system::system_error& e){
        OutputHandler::logException(settings.ErrorStream,"An error occurred during trying to listen on port: ",e);
        LogFile.close();
        return -1;
    }
    catch(runtime_error&){
        OutputHandler::print(settings.ErrorStream,std::string("The provided localization is invalid, try using the default (no parameter) instead!"));
        LogFile.close();
        return -1;
    }

    std::thread(StopHandler,acceptor).detach();
    while(true){
        try{
            std::shared_ptr<tcp::socket> remote=std::make_shared<tcp::socket>(srv);
            acceptor->accept(*remote);
            std::thread(ServeRequest,remote,proc,settings).detach();
        }
        catch(boost::system::system_error& e){

            if(e.code().value()==10004 || e.code().value()==9){ //system error codes are different on different platforms
                OutputHandler::print(settings.ErrorStream,std::string("Termination request, now shutting down...\n"));
                named_semaphore::remove(SemaphoreName);
                LogFile.close();
                return 0;
            }

            OutputHandler::logException(settings.ErrorStream,std::string("An error occurred during socket handling.\n Now closing the program...\n"),e);
            LogFile.close();
            return -1;
        }
    }
}
