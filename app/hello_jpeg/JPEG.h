// Written by Matt Ownby, August 2012
// You are free to use this for educational/non-commercial use

#ifndef JPEG_H
#define JPEG_H

#include "OMXComponent.h"
#include "ILogger.h"

class JPEG
{
public:
	JPEG(ILogger *pLogger);

    int DoIt(const char *data, unsigned long size);

private:

	static unsigned int RefreshTimer();

	void OnDecoderOutputChanged();

	void OnDecoderOutputChangedAgain();

	///////

	IOMXComponent *m_pCompDecode, *m_pCompResize;
	ILogger *m_pLogger;
	int m_iInPortDecode, m_iOutPortDecode;
	int m_iInPortResize, m_iOutPortResize;
	void *m_pBufOutput;
	OMX_BUFFERHEADERTYPE *m_pHeaderOutput;
};

#endif // JPEG_H
