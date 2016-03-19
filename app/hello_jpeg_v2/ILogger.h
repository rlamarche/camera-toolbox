// Written by Matt Ownby, August 2012
// You are free to use this for educational/non-commercial use

#ifndef ILOGGER_H
#define ILOGGER_H

#include <string>

using namespace std;

class ILogger
{
public:
	virtual void Log(const string &) = 0;
};

#endif // ILOGGER_H

