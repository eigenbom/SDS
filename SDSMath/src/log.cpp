/*
 * Log.cpp
 *
 *  Created on: 19/11/2008
 *      Author: ben
 */

#include "log.h"

#include <fstream>

Log Log::sDefaultLog;

Log::Log(std::ostream* oss, bool own) {
	mOStream = oss;
	mOwnStream = own;
}

Log::~Log() {
	if (mOwnStream and valid())
	{
		delete mOStream;
	}
}

void Log::setOwnStream(std::ostream* oss)
{
	mOStream = oss;
	mOwnStream = true;
}

void Log::setSharedStream(std::ostream* oss)
{
	mOStream = oss;
	mOwnStream = false;
}

Log& Log::defaultLog()
{
	if (not sDefaultLog.valid())
	{
		std::ofstream* oss = new std::ofstream("default.log");
		sDefaultLog.setOwnStream(oss);
	}

	return sDefaultLog;
}
