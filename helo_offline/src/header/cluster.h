#ifndef CLUST_H
#define CLUST_H

#include <iostream>
#include <SQLiteCpp/SQLiteCpp.h>
#include "LogParser.h"
#include "SafeList.h"

/**
 * @file cluster.h
 *
 * This file contains the Cluster class
 * @author Jenei Gábor <jengab@elte.hu>
 */

///what percent of a column should be filled to be able to split there?
///
#define PERCENT_OF_FILL 0.5

/**
 * This class represents a cluster, it also stores all the statistics,
 * that we need to split this cluster to subclusters.
 */
class Cluster{

    private:
        std::shared_ptr<ListOfLines> Content;
        DictionaryPtr dict;
        std::vector< std::set<std::shared_ptr<TokenDescriptor>,WordComparator> > Values;
        std::vector<size_t> FilledColumns;
        std::vector<size_t> FilledNonNumColumns;
        size_t TotalLineCount;
        size_t MaxLineLen;
        double goodness;
        unsigned int id;
        size_t TotalLineLen;

        //private methods
        int getSplit();
        void CalcStatistics();

    public:

        void Split(SafeList<Cluster>&);
        ArrayOfWords getTemplate();
        void compressToTemplate();
        Cluster(const std::shared_ptr<ListOfLines>,const DictionaryPtr);
        double getGoodness();
        double getGoodness(Cluster&);
        void join(Cluster&);
        friend std::wostream& operator<<(std::wostream&,const Cluster&);
        friend SQLite::Database& operator<<(SQLite::Database&,const Cluster&);

        /**
         * @return The average line length in the original cluster (this is needed for online processing)
         */
        double getAvgLen() const{
            return ((double)TotalLineLen/TotalLineCount);
        }

        /**
         * This method sets a cluster Id, it is useful in the output file for further processing.
         * As the name says this attribute must be unique for each cluster
         * @param[in] Id The id to set
         */
        void setId(unsigned int Id){id=Id;}

        /**
         * @return Returns the set cluster id
         */
        unsigned int getId(){return id;}

        /**
         * A simple (almost useless) constructor, that builds an empty cluster.
         * It is used for temporal variables only, when we don't know yet the
         * contents of the cluster in the time of construction.
         */
        Cluster():TotalLineCount(0),MaxLineLen(0),goodness(1),id(0),TotalLineLen(0){}

        /**
         * An equality operator
         *
         * @param[in] other The cluster to compare to
         * @return true if the two clusters have the same memory address, false otherwise
         * @see Cluster::operator!=
         */
        bool operator==(const Cluster& other){return &other==this;}

        /**
         * An inequality operator
         *
         * @param[in] other the cluster to compare to
         * @return false if the two clusters have the same memory address, true otherwise
         * @see Cluster::operator==
         */
        bool operator!=(const Cluster& other){return &other!=this;}
};

/**
 * \c typedef for a SafeList of Cluster objects, it represents the output of the algorithm
 */
typedef SafeList<Cluster> ListOfClusters;

#endif
