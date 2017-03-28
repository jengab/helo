#ifndef THREAD_POOL_H
#define THREAD_POOL_H

#include "SafeList.h"
#include "cluster.h"
#include <thread>

/**
 * @file ThreadPool.h
 *
 * This file contains the ThreadPool class
 * @author Jenei Gábor <jengab@elte.hu>
 */

/**
 * This class implements a producer-consumer model for clusters.
 * Thus cluster splitting can be done in a parallel way, which speeds up the algorithm.
 */
class ThreadPool{
private:
	double lim;
	ListOfClusters ClustersToSplit;
	ListOfClusters& OutputClusters;
	std::vector<std::thread> threads;
	std::mutex BusyLocker;
	std::mutex ErrorLocker;
	std::vector<bool> Busy;
	void ThreadFunction(size_t id);

public:
	ThreadPool(size_t,Cluster,ListOfClusters&,double);
	bool isBusy();
	void joinAll();
};

#endif
