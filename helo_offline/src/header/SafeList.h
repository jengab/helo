#ifndef SAFE_LIST_H
#define SAFE_LIST_H

#include <mutex>
#include <list>
#include <string>

/**
 * @file SafeList.h
 *
 * This file contains the SafeList class and other utilities needed to implement this class
 * @author Jenei Gábor <jengab@elte.hu>
 */

/**
 * This class represents an exception that can be thrown by SafeList
 */
class EmptyContainer{
private:
	std::string msg;

public:
	/**
	 * @return Returns the stored error message
	 */
	std::string what(){return msg;}

	/**
	 * Constructor
	 * @param[in] msg The error message to store (it maybe retrieved later by what() )
	 */
	EmptyContainer(std::string msg):msg(msg){}
};

/**
 * This class implements a thread-safe list
 *
 * @tparam T The type of list elements
 */
template<typename T>
class SafeList{
private:
	std::list<T> list;
	std::mutex mutex;
public:
	SafeList(){}
	
	/**
	 * Copy Constructor is never used in this project, but it maybe useful.
	 * This method locks and copies the represented list. Finally it deletes
	 * the right hand operand.
	 */
	SafeList(SafeList& other){
		std::lock_guard<std::mutex> g(mutex);
		list=other.list;
		other.list.clear();
	}

	~SafeList(){}

	/**
	 * \c typedef for list iterator
	 */
	typedef typename std::list<T>::iterator iterator;

	/**
	 * \c typedef for list constant iterator
	 */
	typedef typename std::list<T>::const_iterator const_iterator;

	/**
	 * This method is <b>NOT thread-safe</b>.
	 *
	 * @return Returns an iterator that points to the first list element
	 */
	iterator begin(){return list.begin();}

	/**
	 * This method is <b>NOT thread-safe</b>.
	 *
	 * @return Returns a constant iterator that points to the first list element
	 */
	const_iterator begin()const{return list.cbegin();}

	/**
	 * This method is <b>NOT thread-safe</b>.
	 *
	 * @return Returns an iterator that points to the last list element
	 */
	iterator end(){return list.end();}

	/**
	 * This method is <b>NOT thread-safe</b>.
	 *
	 * @return Returns a constant iterator that points to the last list element
	 */
	const_iterator end()const{return list.cend();}

	/**
	 * This method adds an element to the end of the list.
	 * @param[in] elem The element to add
	 */
	void push_back(const T& elem){
		std::lock_guard<std::mutex> g(mutex);
		list.push_back(elem);
	}

	/**
	 * @return Tells whether the list is empty
	 */
	bool empty(){
		std::lock_guard<std::mutex> g(mutex);
		return list.empty();
	}

	/**
	 * This method tests if the list is empty, and deletes the first element,
	 * and finally returns it.
	 *
	 * @throws Throws EmptyContainer if the list is empty
	 * @return The popped element
	 */
	T pop_front(){
		std::lock_guard<std::mutex> g(mutex);
		if(list.empty()) throw EmptyContainer("List is empty!");
		T ret=list.front();
		list.pop_front();
		return ret;
	}

	/**
	 * This method deletes a given element.
	 *
	 * @param[in] it An iterator that points to the element to delete
	 */
	iterator erase(typename std::list<T>::iterator it){
		std::lock_guard<std::mutex> g(mutex);
		return list.erase(it);
	}

};

#endif
