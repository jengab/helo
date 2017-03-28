#include "ThreadPool.h"

/**
 * @param[in] noThreads The number of threads to create
 * @param[in] StartingCluster The first cluster that contains the whole file
 * @param[out] Output A reference to a list where good enough clusters (goodness>=threshold)
 * can be stored
 * @param[in] lim The threshold value for goodness
 */
ThreadPool::ThreadPool(size_t noThreads,Cluster StartingCluster,ListOfClusters& Output,double lim):lim(lim),OutputClusters(Output){
	ClustersToSplit.push_back(StartingCluster);
	threads.resize(noThreads);
	Busy.resize(noThreads);

	for(size_t i=0;i<noThreads;++i){

		threads[i]=std::thread(&ThreadPool::ThreadFunction,this,i);
	}
}

/**
 * Tells whether the pool is currently busy (if there is any thread working)
 */
bool ThreadPool::isBusy(){
	std::lock_guard<std::mutex> guard(BusyLocker);
	bool ret=false;
	for(size_t i=0;i<Busy.size() && !ret;++i) ret|=Busy[i];
	
	return ret;
}

/**
 * The thread function for the threads created in the pool
 * @param[in] id The thread ID (it is needed for the handling of busy array)
 */
void ThreadPool::ThreadFunction(size_t id){
	do{
		Cluster OwnCluster;
		try{
			OwnCluster=ClustersToSplit.pop_front();
			std::lock_guard<std::mutex> g(BusyLocker);
			Busy[id]=true;
		}
		catch(EmptyContainer&){
			std::this_thread::sleep_for(std::chrono::milliseconds(300));
			continue;
		}

		ListOfClusters OwnList;
		bool IsSplitable=true;

		try{
			OwnCluster.Split(OwnList);
		}
		catch(const std::invalid_argument& e){
			std::lock_guard<std::mutex> g(ErrorLocker);
			std::cerr << e.what();
			IsSplitable=false;
		}

		for(Cluster& ActClust:OwnList){
			if(ActClust.getGoodness()>=lim || !IsSplitable){
				OutputClusters.push_back(ActClust);
			}
			else{
				ClustersToSplit.push_back(ActClust);
			}
		}
		
		{
			std::lock_guard<std::mutex> g(BusyLocker);
			Busy[id]=false;
		}
	}while(isBusy() || !ClustersToSplit.empty());
}

/**
 * This method joins all threads in the pool (waits for all the threads to terminate)
 */
void ThreadPool::joinAll(){
	for(std::thread& ActThread:threads) ActThread.join();
} 
