// Written by Matt Ownby, August 2012
// You are free to use this for educational/non-commercial use

#include "Logger.h"
#include <stdio.h>

Logger::Logger()
{
}

void Logger::Log(const string &s)
{
	printf("%s\n", s.c_str());
}
