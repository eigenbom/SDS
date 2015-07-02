/*
 * timeofday.cpp
 *
 *  Created on: 18/02/2009
 *      Author: ben
 */

#include "timeofday.h"

#ifdef WIN32

#include <windows.h>
#include <sys/time.h>

void __stdcall GetSystemTimeAsFileTime(FILETIME*);

void timeofday(struct timeval* p, void* tz /* IGNORED */)
{
	  union {
		 long long ns100; /*time since 1 Jan 1601 in 100ns units */
			 FILETIME ft;
	  } now;

  GetSystemTimeAsFileTime( &(now.ft) );
  p->tv_usec=(long)((now.ns100 / 10LL) % 1000000LL );
  p->tv_sec= (long)((now.ns100-(116444736000000000LL))/10000000LL);
}

#else

#include <sys/time.h>

void timeofday(struct timeval* p, void* tz /* IGNORED */)
{
	gettimeofday(p,tz);
}

#endif
