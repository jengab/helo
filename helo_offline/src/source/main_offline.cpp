#include <thread>
#include <fstream>
#include <iostream>
#include <string.h>

#include "ThreadPool.h"

/**
 * @file main_offline.cpp
 *
 * This file contains the main() function for HELO offline algorithm
 * @author Jenei Gábor <jengab@elte.hu>
 */

using namespace std;

/**
 * <p>This is the main() function of the offline program.</p>
 *
 * <p>First it reads the input file, and builds up an inner
 * representation of the file. For this purpose it uses Parser</p>
 *
 * <p>On the second step it makes a ThreadPool, and provides a cluster
 * filled by the data got from the Parser object as input parameter for
 * this ThreadPool. The ThreadPool does the whole splitting phase of
 * HELO in a parallel way. </p>
 *
 * <p>After the parallel run is done it generates a template for all clusters.</p>
 *
 * <p>After the templates are done it does an \f$O(c^2)\f$ algorithm, where
 * c denotes the number of found clusters in the former step (this is very
 * likely to be low compared to the file size). In this step we compare each
 * cluster to all others, if they are similar enough (the threshold is defined
 * as the input of this function) then we join them, thus we reduce the number
 * of clusters and we reunite those clusters that were probably split indirectly.</p>
 *
 * <p>Finally the result is written to the output file.</p>
 *
 * @param[in] argc The number of command line arguments
 * @param[in] argv The array of command line arguments
 *
 * The command line arguments can be:
 *
 * <table>
 * <tr><td>input_file</td>
 * <td>The path to the input log file (this is the first command line parameter)</td></tr>
 * <tr><td>output_file</td>
 * <td>The path to the output cluster file (this is the second command line parameter)</td></tr>
 * <tr><td>lim</td>
 * <td> The threshold value for splitting process (second step)
 * (this parameter is optional, it must be between 0 and 1, and it can be set by -st\<value\> command line
 * parameter, its default value is 0.4)</td></tr>
 * <tr><td>MergeThreshold</td><td>The minimal goodness limit used in the fourth step of the algorithm.
 * Defaultly this value is 0.8, and it can be set by -mt\<value\> command line parameter</td></tr>
 * <tr><td>HeaderLen</td>
 * <td>The length of header part of log messages. (this parameter is optional, and
 * it can be set by -he\<value\> command line parameter, its default value is 4)</td></tr>
 * <tr><td>loc</td>
 * <td>A localization string that sets the localization used for reading the log file.
 * (this parameter is optional, it can be set by -lo\<name\> command line parameter. If there is
 * no localization set the program uses the currently used system localization)</td></tr>
 * <tr><td>UseDb</td><td>A boolean parameter which is true if and only if the output should be written
 * to a SQLite3 database file. It can be set true by setting a -d parameter, while not setting this parameter
 * indicates that the output will be written in XML format.</td></tr>
 * <tr><td>regexp</td><td>The regular expression used for tokenizing the input messages. It can be given
 * in POSIX regex format, and it can be set by -re\<regexpr\> command line parameter. Defaultly white spaces
 * will be used as token separators.</td></tr>
 * </table>
 */
int main(int argc,char* argv[]){
	unsigned int numCPU=thread::hardware_concurrency();
	unsigned int HeaderLen=4;
	double lim=0.4;
	double MergeLimit=0.8;
	bool UseDb=false;
	string loc="";
	wstring regexp(L"[\\s]+");

	const string HelpMessage=string("Usage: ")+string(argv[0])+
			string(" <input_file> <output_file> [options]\n Options:\n  -st<value> - sets the goodness threshold")+
			string("\n  \t(default: 0.4), this value must be between 0 and 1\n")+
			string("  -he<value> The length (number of words) of the header part of log messages\n \t(default: 4)\n")+
			string("  -lo<name> The localization used to read the input file \n  \t(system language is default)\n")+
			string("  -d The program will write the results into a SQLite file if this option is used\n")+
			string("  -re<value> <value> can be an extended POSIX regular expression, it sets the regex for tokenization\n")+
			string("  -mt<value> Sets the merge threshold value (default value is 0.8)\n");

	if(argc<3){
		cerr << HelpMessage;
		return -1;
	}

	if(strcmp(argv[1],"-h")==0){
		cout << HelpMessage;
		return 0;
	}

	if(argc>3){
		for(int i=3;i<argc;++i){
			if(strncmp(argv[i],"-he",3)==0) HeaderLen=atoi(&argv[i][3]);
			if(strncmp(argv[i],"-mt",3)==0) MergeLimit=atof(&argv[i][3]);
			if(strncmp(argv[i],"-st",3)==0) lim=atof(&argv[i][3]);
			if(strncmp(argv[i],"-lo",3)==0) loc=string(&argv[i][3]);
			if(strncmp(argv[i],"-d",2)==0) UseDb=true;
			if(strncmp(argv[i],"-re",3)==0){
				string tempStr(&argv[i][3]);
				regexp=wstring(tempStr.begin(),tempStr.end());
			}
		}
	}

	if(lim<=0 || lim>1){
		cerr << "Wrong threshold value! It must be more than 0, and less, or equal to 1!\n";
		return -1;
	}

	Cluster FirstCluster;
	try{
		locale WordLocale=locale(loc.c_str());
		locale local=locale(WordLocale,locale(),locale::numeric);
		locale::global(local);
		cout << "Starting Helo! Number of CPU cores: " << numCPU << endl;
		cout << "Applied options:\n localization: " << WordLocale.name() << "\n Length of header part (in words): " << HeaderLen;
		cout << "\n Goodness limit: " << lim << "\n Merge limit: " << MergeLimit;
		cout << "\n Regular expression: " << string(regexp.begin(),regexp.end()) << endl;

		LogParser File((size_t)HeaderLen,regexp);
		wifstream ifile;
		ifile.exceptions(ios::failbit);
		ifile.open(argv[1],ios::binary | ios::in);
		ifile >> File;
		ifile.close();
		FirstCluster=Cluster(File.getContent(),File.getDictionary());
#ifdef DEBUG
		wcout << File << endl;
#endif
	}
	catch(const ios::failure& e){
		cerr << "A problem occurred during reading the input file: " << e.what() << endl;
		cerr << "Maybe the used localization doesn't fit the input file?\n";
		return -1;
	}
	catch(runtime_error&){
		cerr << "The provided localization or regular expression is invalid.\n";
		return -1;
	}

	cout << "Beginning of multithreaded run!\n";
	ListOfClusters OutputClusters;
	ThreadPool worker(numCPU,FirstCluster,OutputClusters,lim);
	try{
		worker.joinAll();
	}
	catch(std::exception& e){
		cerr << "Multithreaded run failed! Error: " << e.what();
		return -1;
	}

#ifdef DEBUG
	for(const Cluster& ActClust:OutputClusters){
		wcout << ActClust << endl;
	}
#endif

	cout << "Multithreaded run is done, making templates...\n";
	for(Cluster& ActClust:OutputClusters){
		ActClust.compressToTemplate();
	}

	cout << "Templates are done, joining similar templates...\n";
	for(Cluster& OuterCluster:OutputClusters){
		for(auto it=OutputClusters.begin();it!=OutputClusters.end();){
			if(OuterCluster==*it){
				++it;
				continue;
			}

			if(OuterCluster.getGoodness(*it)>=MergeLimit){
				OuterCluster.join(*it);
				it=OutputClusters.erase(it);
			}
			else{
				++it;
			}
		}
	}

	cout << "Templates are merged! Writing the result to file...\n";
	try{
		if(UseDb){
			SQLite::Database db(argv[2], SQLite::OPEN_READWRITE | SQLite::OPEN_CREATE);
			SQLite::Transaction transaction(db);
			db.exec("PRAGMA foreign_keys=ON");
			db.exec("CREATE TABLE IF NOT EXISTS clusters (clustid INTEGER PRIMARY KEY,template text NOT NULL,goodness real NOT NULL,AvgLen real NOT NULL)");
			db.exec("CREATE TABLE IF NOT EXISTS syslog (clustid INTEGER,msg text NOT NULL,FOREIGN KEY(clustid) REFERENCES clusters(clustid))");

			for(Cluster& ActClust:OutputClusters){
				db << ActClust;
			}
			transaction.commit();
		}
		else{
			wofstream ofile;
			ofile.exceptions(ios::failbit);
			ofile.open(argv[2],ios::out | ios::binary);

			size_t counter=1;
			for(Cluster& ActClust:OutputClusters){
				ActClust.setId(counter++);
				ofile << ActClust;
			}
			ofile.close();
		}
	}
	catch(const ios::failure& e){
		cerr << "Couldn't write result to output file!\nCause:" << e.what() << endl;
		int counter=1;
		for(Cluster& ActClust:OutputClusters){
			ActClust.setId(counter++);
			wcout << ActClust;
		}
		return -1;
	}
	catch(const SQLite::Exception& e){
		cerr << "An exception occured during writing to database: " << e.what() << endl;
		int counter=1;
		for(Cluster& ActClust:OutputClusters){
			ActClust.setId(counter++);
			wcout << ActClust;
		}
		return -1;
	}

	cout << "Successful run, see the output file (filename: " << argv[2] << ")\n";
	return 0;
}
