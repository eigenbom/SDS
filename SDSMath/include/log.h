/*
 * Log.h
 * Provides some basic logging facilities
 * by default it logs to 'default.log'
 *
 *  Created on: 19/11/2008
 *      Author: ben
 */

#ifndef LOG_H_
#define LOG_H_

#include <ostream>
#include <iostream>
#include <cassert>
#include <boost/tuple/tuple.hpp>


#ifdef DEBUG_LOG
#define LOG(msg) {Log::defaultLog() << msg;}
#endif

#ifndef DEBUG_LOG
#define LOG(msg) ;
#endif


class Log {
public:
	Log(std::ostream* oss = NULL, bool own = false);
	virtual ~Log();
	void setOwnStream(std::ostream* oss);
	void setSharedStream(std::ostream* oss);

	static Log& defaultLog();
	bool valid(){return mOStream!=NULL;}

	template<typename T> friend Log& operator<<(Log& l, const T& t);
	template<typename T> friend Log& operator<<(Log& l, const T* t);

	// friend Log& operator<<(Log& l, const char* str);

protected:
	static Log sDefaultLog;
	std::ostream* mOStream;
	bool mOwnStream;

};



template<typename T> Log& operator<<(Log& l, const T& t)
{
	//std::cerr << t;
	*l.mOStream << t;
	l.mOStream->flush();
	return l;
}

template<typename T> Log& operator<<(Log& l, const T* t)
{
	//std::cerr << t;
	*l.mOStream << t;
	l.mOStream->flush();
	return l;
}

template<typename T> Log& operator<<(Log& l, const boost::tuple<T,T,T>& t)
{
	return l << boost::get<0>(t) << boost::get<1>(t) << boost::get<2>(t);
}
/*
Log& operator<<(Log& l, const char* str)
{
	assert(l.valid());
	*l.mOStream << str;
	return l;
}
*/
#endif /* LOG_H_ */
