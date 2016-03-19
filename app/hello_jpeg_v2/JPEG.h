// Written by Matt Ownby, August 2012
// You are free to use this for educational/non-commercial use

#ifndef JPEG_H
#define JPEG_H

#include "OMXComponent.h"
#include "ILogger.h"

#include <QImage>

class JPEG
{
public:
	JPEG(ILogger *pLogger);

    QImage DoIt(const char *data, unsigned long size);
    //int DoIt(const char *cpszFileName);

private:

	static unsigned int RefreshTimer();

    OMX_PARAM_PORTDEFINITIONTYPE OnDecoderOutputChanged();

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
