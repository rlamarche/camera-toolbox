// Written by Matt Ownby, August 2012
// You are free to use this for educational/non-commercial use

#ifndef LOGGER_H
#define LOGGER_H

#include "ILogger.h"

class Logger : public ILogger
{
public:
	Logger();
	void Log(const string &);
};

#endif // LOGGER_H

