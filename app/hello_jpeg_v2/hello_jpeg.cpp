// Written by Matt Ownby, August 2012
// You are free to use this for educational/non-commercial use

#include <stdio.h>
#include "bcm_host.h"
#include "Logger.h"
#include "JPEG.h"

int main (int argc, char **argv)
{
	if (argc < 2) {
		printf("Usage: %s <input filename>\n", argv[0]);
		return (1);
	}

	bcm_host_init();

	Logger logstdout;
	ILogger *pLogger = &logstdout;
	JPEG j(pLogger);

	int iRes = j.DoIt(argv[1]);

	bcm_host_deinit();

	return iRes;
}

