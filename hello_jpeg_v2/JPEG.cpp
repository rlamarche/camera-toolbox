// Written by Matt Ownby, August 2012
// You are free to use this for educational/non-commercial use

#include "JPEG.h"
#include "OMXCore.h"
#include <sys/time.h>	// for gettimeofday
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdexcept>
#include <assert.h>
#include <vector>	// for memory buffers


#include <QImage>

using namespace std;

#define TIMEOUT_MS 2000

JPEG::JPEG(ILogger *m_pLogger) :
m_pCompDecode(NULL),
m_pCompResize(NULL),
m_pLogger(m_pLogger),
m_pBufOutput(NULL),
m_pHeaderOutput(NULL)
{
}

QImage JPEG::DoIt(const char *data, unsigned long size)
//int JPEG::DoIt(const char *cpszFileName)
{
    QImage image;
	try
	{
/*
        FILE *THEFILE = fopen(cpszFileName, "rb");

        if (THEFILE == NULL)
		{
			throw runtime_error("fopen failed");
        }
*/
		IOMXCoreSPtr core = OMXCore::GetInstance(m_pLogger);
		IOMXCore *pCore = core.get();

		m_pCompDecode = pCore->GetHandle("OMX.broadcom.image_decode");
		m_pCompResize= pCore->GetHandle("OMX.broadcom.resize");

		OMX_PORT_PARAM_TYPE port;
		port.nSize = sizeof(OMX_PORT_PARAM_TYPE);
		port.nVersion.nVersion = OMX_VERSION;

		// get ports for decoder
		m_pCompDecode->GetParameter(OMX_IndexParamImageInit, &port);

		if (port.nPorts != 2)
		{
			throw runtime_error("Unexpected number of ports returned");
		}

		m_iInPortDecode = port.nStartPortNumber;
		m_iOutPortDecode = port.nStartPortNumber+1;

		// get ports for resizer
		m_pCompResize->GetParameter(OMX_IndexParamImageInit, &port);

		if (port.nPorts != 2)
		{
			throw runtime_error("Unexpected number of ports returned");
		}

		m_iInPortResize = port.nStartPortNumber;
		m_iOutPortResize = port.nStartPortNumber+1;

		// disable all ports to get to a sane state
		m_pCompDecode->SendCommand(OMX_CommandPortDisable, m_iInPortDecode, NULL);
		m_pCompDecode->WaitForEvent(OMX_EventCmdComplete, OMX_CommandPortDisable, m_iInPortDecode, TIMEOUT_MS);
		m_pCompDecode->SendCommand(OMX_CommandPortDisable, m_iOutPortDecode, NULL);
		m_pCompDecode->WaitForEvent(OMX_EventCmdComplete, OMX_CommandPortDisable, m_iOutPortDecode, TIMEOUT_MS);
		m_pCompResize->SendCommand(OMX_CommandPortDisable, m_iInPortResize, NULL);
		m_pCompResize->WaitForEvent(OMX_EventCmdComplete, OMX_CommandPortDisable, m_iInPortResize, TIMEOUT_MS);
		m_pCompResize->SendCommand(OMX_CommandPortDisable, m_iOutPortResize, NULL);
		m_pCompResize->WaitForEvent(OMX_EventCmdComplete, OMX_CommandPortDisable, m_iOutPortResize, TIMEOUT_MS);

		// move decoder component into idle state
		m_pCompDecode->SendCommand(OMX_CommandStateSet, OMX_StateIdle, NULL);

		// wait for state change event
		m_pCompDecode->WaitForEvent(OMX_EventCmdComplete, OMX_CommandStateSet, OMX_StateIdle, TIMEOUT_MS);

		// set input format
		OMX_IMAGE_PARAM_PORTFORMATTYPE imagePortFormat;
		memset(&imagePortFormat, 0, sizeof(imagePortFormat));
		imagePortFormat.nSize = sizeof(imagePortFormat);
		imagePortFormat.nVersion.nVersion = OMX_VERSION;
		imagePortFormat.nPortIndex = m_iInPortDecode;
		imagePortFormat.eCompressionFormat = OMX_IMAGE_CodingJPEG;
		m_pCompDecode->SetParameter(OMX_IndexParamImagePortFormat, &imagePortFormat);

		// query input buffer requirements
		OMX_PARAM_PORTDEFINITIONTYPE portdef;
		portdef.nSize = sizeof(OMX_PARAM_PORTDEFINITIONTYPE);
		portdef.nVersion.nVersion = OMX_VERSION;
		portdef.nPortIndex = m_iInPortDecode;
		m_pCompDecode->GetParameter(OMX_IndexParamPortDefinition, &portdef);

		// enable input port
		m_pCompDecode->SendCommand(OMX_CommandPortEnable, m_iInPortDecode, NULL);

		int iBufferCount = portdef.nBufferCountActual;
		vector<OMX_BUFFERHEADERTYPE *> vpBufHeaders;	// vector to hold all of the buffer headers

		for (int i = 0; i < iBufferCount; i++)
		{
			void *pBuf = 0;
			if (posix_memalign(&pBuf, portdef.nBufferAlignment, portdef.nBufferSize) != 0)
			{
				throw runtime_error("posix_memalign failed");
			}

			OMX_BUFFERHEADERTYPE *pHeader = NULL;

			m_pCompDecode->UseBuffer(&pHeader, m_iInPortDecode,
				(void *) i,	// the index will be our private data
				portdef.nBufferSize, (OMX_U8 *) pBuf);

			vpBufHeaders.push_back(pHeader);	// add buffer header to our vector (the vector index will match 'i')
		}

		// wait for port enable event to be finished (it should finish once we give it buffers)
		m_pCompDecode->WaitForEvent(OMX_EventCmdComplete, OMX_CommandPortEnable, m_iInPortDecode, TIMEOUT_MS);

		// move component into executing state so it can begin consuming our buffers
		m_pCompDecode->SendCommand(OMX_CommandStateSet, OMX_StateExecuting, NULL);
		m_pCompDecode->WaitForEvent(OMX_EventCmdComplete, OMX_CommandStateSet, OMX_StateExecuting, TIMEOUT_MS);

		m_pBufOutput = NULL;
		m_pHeaderOutput = NULL;
		unsigned int uStartTime = RefreshTimer();
		size_t stFramesDecoded = 0;

#ifndef VERBOSE
        const size_t FRAMES_TO_DECODE = 1;	// arbitrary value to get a good idea of the frame decoding speed
#else
		const size_t FRAMES_TO_DECODE = 1;	// if in verbose mode, only decode 1 frame so user does not get spammed
#endif // VERBOSE

        OMX_PARAM_PORTDEFINITIONTYPE portdef2;
        while (stFramesDecoded < FRAMES_TO_DECODE)
		{
			bool bDone = false;
			bool bFillCalled = false;
			int iBufIdx = 0;

			// go to beginning of stream
            //rewind(THEFILE);

			// feed source buffer into component
            unsigned long remaining_size = size;
            unsigned long stBytesRead;
            void* buf = (void*) data;
			while (!bDone)
			{
				// get buffer to fill
				OMX_BUFFERHEADERTYPE *pBufHeader = vpBufHeaders[iBufIdx];
				iBufIdx++;

				// wraparound if necessary
				if (iBufIdx >= iBufferCount)
				{
					iBufIdx = 0;
				}

                stBytesRead = pBufHeader->nAllocLen > remaining_size ? remaining_size : pBufHeader->nAllocLen;
                memcpy(pBufHeader->pBuffer, (void*) buf, stBytesRead);
                buf += stBytesRead;
                remaining_size -= stBytesRead;
                //size_t stBytesRead = fread(pBufHeader->pBuffer, 1, pBufHeader->nAllocLen, THEFILE);

				pBufHeader->nFilledLen = stBytesRead;
				pBufHeader->nOffset = 0;
				pBufHeader->nFlags = 0;

				// if we are at EOF
				if (stBytesRead < pBufHeader->nAllocLen)
				{
					pBufHeader->nFlags = OMX_BUFFERFLAG_EOS;	// notify that the stream is done
					bDone = true;
				}

				m_pCompDecode->EmptyThisBuffer(pBufHeader);
				bool bGotEmpty = false;

				// we need to get an empty ack after calling EmptyThisBuffer,
				//   the only thing that can stand in our way is getting a port settings changed event.
				// For large JPEGs, more than one port settings changed event may occur (I haven't figured out why, I just handle it)
				// We also expect a port settings changed event if we have no output buffer and can't proceed without one.
				while ((!bGotEmpty) || (m_pBufOutput == NULL))
				{
					// we could get either a "empty done" or a "port settings changed" at this point
					IEventSPtr ev = m_pCompDecode->WaitForEventOrEmpty(OMX_EventPortSettingsChanged, m_iOutPortDecode, 0, pBufHeader, TIMEOUT_MS);

					IEvent *pEv = ev.get();
					OMXEventData *pEvOMX = pEv->ToEvent();
					EmptyBufferDoneData *pEvEmpty = pEv->ToEmpty();

					// if it's a port settings changed event
					if (pEvOMX != NULL)
					{
						// if we don't yet have an output buffer, then create one now
						if (m_pBufOutput == 0)
						{
                            portdef2 = OnDecoderOutputChanged();	// setup output buffer
						}
						// else we already have an output buffer, so handle the event differently
						else
						{
							OnDecoderOutputChangedAgain();
						}
					}
					// else if this is an empty event
					else if (pEvEmpty != NULL)
					{
						bGotEmpty = true;
					}
					// else this is unexpected
					else
					{
						throw runtime_error("Unexpected IEvent");
					}
				}

				assert(m_pBufOutput != 0);
				assert(m_pHeaderOutput != 0);

				if (!bFillCalled)
				{
					m_pCompResize->FillThisBuffer(m_pHeaderOutput);
					bFillCalled = true;	// don't want to call FillThisBuffer twice
				}
			} // end while !bDone

			// both of these should be true at this point, if they're not it's a bug in my code
			assert(m_pBufOutput != 0);
			assert(bFillCalled);

			// wait for fill to finish so we can grab our final buffer
			m_pCompResize->WaitForFill(m_pHeaderOutput, TIMEOUT_MS);

			// wait for "end of stream" events from decoder and resizer
			m_pCompDecode->WaitForEvent(OMX_EventBufferFlag, m_iOutPortDecode, OMX_BUFFERFLAG_EOS, TIMEOUT_MS);
			m_pCompResize->WaitForEvent(OMX_EventBufferFlag, m_iOutPortResize, OMX_BUFFERFLAG_EOS, TIMEOUT_MS);

			// at this point, we should have no events queued up at all; if we do, we have probably missed one somewhere
			assert(m_pCompDecode->GetPendingEventCount() == 0);
			assert(m_pCompDecode->GetPendingEmptyCount() == 0);
			assert(m_pCompDecode->GetPendingFillCount() == 0);
			assert(m_pCompResize->GetPendingEventCount() == 0);
			assert(m_pCompResize->GetPendingEmptyCount() == 0);
			assert(m_pCompResize->GetPendingFillCount() == 0);

			stFramesDecoded++;
			
		} // end while benchmarking

		unsigned int uTotalMs = RefreshTimer() - uStartTime;

		// only show benchmarking data if we aren't in verbose mode (verbosity will throw off results)
#ifndef VERBOSE
/*
		char sStats[300];
		snprintf(sStats, sizeof(sStats), "Total elapsed milliseconds: %u", uTotalMs);
        m_pLogger->Log(sStats);
		snprintf(sStats, sizeof(sStats), "Total frames decoded: %u", FRAMES_TO_DECODE);
		m_pLogger->Log(sStats);
		snprintf(sStats, sizeof(sStats), "Total frames / second is %f", (FRAMES_TO_DECODE * 1000.0) / uTotalMs);
		m_pLogger->Log(sStats);
*/
#endif // VERBOSE


        //TOTOTOTOTO
        image = QImage(portdef2.format.image.nFrameWidth, portdef2.format.image.nFrameHeight, QImage::Format_RGBA8888);
        memcpy(image.bits(), m_pHeaderOutput->pBuffer, image.byteCount());


        //fclose(THEFILE);

		// write filled buffer out to file
/*
        FILE *FDST = fopen("/home/pi/output.raw", "wb");
		if (!FDST) throw runtime_error("Unable to create output file");

		fwrite(m_pHeaderOutput->pBuffer, 1, m_pHeaderOutput->nAllocLen, FDST);
		fclose(FDST);

		m_pLogger->Log("Write outraw.raw");
*/
		// clean-up

		// 3.4.3.1 of official spec says:
		// change components to Idle
		// change components to Loaded
		// call OMX_TeardownTunnel
		// call OMX_FreeBuffer
		// de-allocate memory after calling FreeBuffer (keep a pointer in memory before calling OMX_FreeBuffer)
		// call FreeHandle

		// flush tunnel
		m_pCompDecode->SendCommand(OMX_CommandFlush, m_iOutPortDecode, NULL);
		m_pCompDecode->WaitForEvent(OMX_EventCmdComplete, OMX_CommandFlush, m_iOutPortDecode, TIMEOUT_MS);
		m_pCompResize->SendCommand(OMX_CommandFlush, m_iInPortResize, NULL);
		m_pCompResize->WaitForEvent(OMX_EventCmdComplete, OMX_CommandFlush, m_iInPortResize, TIMEOUT_MS);

		// disable input decoder port
		m_pCompDecode->SendCommand(OMX_CommandPortDisable, m_iInPortDecode, NULL);

		// OMX_FreeBuffer on all input buffers
		for (vector<OMX_BUFFERHEADERTYPE *>::iterator vi = vpBufHeaders.begin();
			vi != vpBufHeaders.end(); vi++)
		{
			void *pBuffer = (*vi)->pBuffer;
			m_pCompDecode->FreeBuffer(m_iInPortDecode, *vi);
			free(pBuffer);
		}

		// wait for disable to finish
		m_pCompDecode->WaitForEvent(OMX_EventCmdComplete, OMX_CommandPortDisable, m_iInPortDecode, TIMEOUT_MS);

		// disable output resizer port
		m_pCompResize->SendCommand(OMX_CommandPortDisable, m_iOutPortResize, NULL);

		// OMX_FreeBuffer on output buffer
		m_pCompResize->FreeBuffer(m_iOutPortResize, m_pHeaderOutput);
		free(m_pBufOutput);

		// wait for disable to finish
		m_pCompResize->WaitForEvent(OMX_EventCmdComplete, OMX_CommandPortDisable, m_iOutPortResize, TIMEOUT_MS);

		// disable the rest of the ports
		m_pCompDecode->SendCommand(OMX_CommandPortDisable, m_iOutPortDecode, NULL);
		m_pCompResize->SendCommand(OMX_CommandPortDisable, m_iInPortResize, NULL);
		m_pCompDecode->WaitForEvent(OMX_EventCmdComplete, OMX_CommandPortDisable, m_iOutPortDecode, TIMEOUT_MS);
		m_pCompResize->WaitForEvent(OMX_EventCmdComplete, OMX_CommandPortDisable, m_iInPortResize, TIMEOUT_MS);

		// OMX_SetupTunnel with 0's to remove tunnel
		m_pCompDecode->RemoveTunnel(m_iOutPortDecode);
		m_pCompResize->RemoveTunnel(m_iInPortResize);

		// change handle states to IDLE
		m_pCompDecode->SendCommand(OMX_CommandStateSet, OMX_StateIdle, NULL);
		m_pCompResize->SendCommand(OMX_CommandStateSet, OMX_StateIdle, NULL);

		// wait for state change complete
		m_pCompDecode->WaitForEvent(OMX_EventCmdComplete, OMX_CommandStateSet, OMX_StateIdle, TIMEOUT_MS);
		m_pCompResize->WaitForEvent(OMX_EventCmdComplete, OMX_CommandStateSet, OMX_StateIdle, TIMEOUT_MS);

		// change handle states to LOADED
		m_pCompDecode->SendCommand(OMX_CommandStateSet, OMX_StateLoaded, NULL);
		m_pCompResize->SendCommand(OMX_CommandStateSet, OMX_StateLoaded, NULL);

		// wait for state change complete
		m_pCompDecode->WaitForEvent(OMX_EventCmdComplete, OMX_CommandStateSet, OMX_StateLoaded, TIMEOUT_MS);
		m_pCompResize->WaitForEvent(OMX_EventCmdComplete, OMX_CommandStateSet, OMX_StateLoaded, TIMEOUT_MS);
	}
	catch (std::exception &ex)
	{
		m_pLogger->Log("Got exception:");
		m_pLogger->Log(ex.what());
        return image;
	}

    return image;
}

unsigned int JPEG::RefreshTimer()
{
	unsigned int result = 0;	
	struct timeval tv;
	
	if (gettimeofday(&tv, NULL) == 0)
	{
		tv.tv_sec &= 0x003FFFFF;        // to prevent overflow from *1000, we only care about relative time anyway
		result = (unsigned int) ((tv.tv_sec * 1000) + (tv.tv_usec / 1000));
	}
	else
	{
		throw runtime_error("gettimeofday failed");
	}

	return result;
}

OMX_PARAM_PORTDEFINITIONTYPE JPEG::OnDecoderOutputChanged()
{
	OMX_PARAM_PORTDEFINITIONTYPE portdef;

	// need to setup the input for the resizer with the output of the decoder
	portdef.nSize = sizeof(OMX_PARAM_PORTDEFINITIONTYPE);
	portdef.nVersion.nVersion = OMX_VERSION;
	portdef.nPortIndex = m_iOutPortDecode;
	m_pCompDecode->GetParameter(OMX_IndexParamPortDefinition, &portdef);

	unsigned int uWidth = (unsigned int) portdef.format.image.nFrameWidth;
	unsigned int uHeight = (unsigned int) portdef.format.image.nFrameHeight;

	// tell resizer input what the decoder output will be providing
	portdef.nPortIndex = m_iInPortResize;
	m_pCompResize->SetParameter(OMX_IndexParamPortDefinition, &portdef);

	// establish tunnel between decoder output and resizer input
	m_pCompDecode->SetupTunnel(m_iOutPortDecode, m_pCompResize, m_iInPortResize);

	// enable output of decoder and input of resizer (ie enable tunnel)
	m_pCompDecode->SendCommand(OMX_CommandPortEnable, m_iOutPortDecode, NULL);
	m_pCompResize->SendCommand(OMX_CommandPortEnable, m_iInPortResize, NULL);

	// put resizer in idle state (this allows the outport of the decoder to become enabled)
	m_pCompResize->SendCommand(OMX_CommandStateSet, OMX_StateIdle, NULL);

	// wait for state change complete
	m_pCompResize->WaitForEvent(OMX_EventCmdComplete, OMX_CommandStateSet, OMX_StateIdle, TIMEOUT_MS);

	// once the state changes, both ports should become enabled and the resizer output should generate a settings changed event
	m_pCompDecode->WaitForEvent(OMX_EventCmdComplete, OMX_CommandPortEnable, m_iOutPortDecode, TIMEOUT_MS);
	m_pCompResize->WaitForEvent(OMX_EventCmdComplete, OMX_CommandPortEnable, m_iInPortResize, TIMEOUT_MS);
	m_pCompResize->WaitForEvent(OMX_EventPortSettingsChanged, m_iOutPortResize, 0, TIMEOUT_MS);

	// NOTE : OpenMAX official spec says that upon receving OMX_EventPortSettingsChanged event, the
	//   port shall be disabled and then re-enabled (see 3.1.1.4.4 of IL v1.2.0 specification),
	//   but since we have not enabled the port, I don't think we need to do anything.

	// query output buffer requirements for resizer
	portdef.nSize = sizeof(OMX_PARAM_PORTDEFINITIONTYPE);
	portdef.nVersion.nVersion = OMX_VERSION;
	portdef.nPortIndex = m_iOutPortResize;
	m_pCompResize->GetParameter(OMX_IndexParamPortDefinition, &portdef);

	// change output color format and dimensions to match input
	portdef.format.image.eCompressionFormat = OMX_IMAGE_CodingUnused;
	portdef.format.image.eColorFormat = OMX_COLOR_Format32bitABGR8888;
	portdef.format.image.nFrameWidth = uWidth;
	portdef.format.image.nFrameHeight = uHeight;
	portdef.format.image.nStride = 0;
	portdef.format.image.nSliceHeight = 0;
	portdef.format.image.bFlagErrorConcealment = OMX_FALSE;

	m_pCompResize->SetParameter(OMX_IndexParamPortDefinition, &portdef);

	// grab output requirements again to get actual buffer size requirement (and buffer count requirement!)
	m_pCompResize->GetParameter(OMX_IndexParamPortDefinition, &portdef);

	// we assume this, if it's not true, more code will need to be written to handle it
	assert(portdef.nBufferCountActual == 1);

	// move resizer into executing state
	m_pCompResize->SendCommand(OMX_CommandStateSet, OMX_StateExecuting, NULL);
	m_pCompResize->WaitForEvent(OMX_EventCmdComplete, OMX_CommandStateSet, OMX_StateExecuting, TIMEOUT_MS);

	// show some logging so user knows it's working
    /*
	char sLog[300];
	snprintf(sLog, sizeof(sLog), "Width: %u Height: %u Output Color Format: 0x%x Buffer Size: %u",
		(unsigned int) portdef.format.image.nFrameWidth,
		(unsigned int) portdef.format.image.nFrameHeight,
		(unsigned int) portdef.format.image.eColorFormat,
		(unsigned int) portdef.nBufferSize);
	m_pLogger->Log(sLog);
    */

	// enable output port of resizer
	m_pCompResize->SendCommand(OMX_CommandPortEnable, m_iOutPortResize, NULL);

	if (posix_memalign(&m_pBufOutput, portdef.nBufferAlignment, portdef.nBufferSize) != 0)
	{
		throw runtime_error("posix_memalign failed");
	}
	m_pCompResize->UseBuffer(&m_pHeaderOutput, m_iOutPortResize, NULL, portdef.nBufferSize, (OMX_U8 *) m_pBufOutput);

	// wait for output port enable event to be finished (it should finish once we call UseBuffer)
	m_pCompResize->WaitForEvent(OMX_EventCmdComplete, OMX_CommandPortEnable, m_iOutPortResize, TIMEOUT_MS);

    return portdef;
}

void JPEG::OnDecoderOutputChangedAgain()
{
	m_pCompDecode->SendCommand(OMX_CommandPortDisable, m_iOutPortDecode, NULL);
	m_pCompResize->SendCommand(OMX_CommandPortDisable, m_iInPortResize, NULL);

	m_pCompDecode->WaitForEvent(OMX_EventCmdComplete, OMX_CommandPortDisable, m_iOutPortDecode, TIMEOUT_MS);
	m_pCompResize->WaitForEvent(OMX_EventCmdComplete, OMX_CommandPortDisable, m_iInPortResize, TIMEOUT_MS);

	OMX_PARAM_PORTDEFINITIONTYPE portdef;

	// need to setup the input for the resizer with the output of the decoder
	portdef.nSize = sizeof(OMX_PARAM_PORTDEFINITIONTYPE);
	portdef.nVersion.nVersion = OMX_VERSION;
	portdef.nPortIndex = m_iOutPortDecode;
	m_pCompDecode->GetParameter(OMX_IndexParamPortDefinition, &portdef);

	// tell resizer input what the decoder output will be providing
	portdef.nPortIndex = m_iInPortResize;
	m_pCompResize->SetParameter(OMX_IndexParamPortDefinition, &portdef);

	// enable output of decoder and input of resizer (ie enable tunnel)
	m_pCompDecode->SendCommand(OMX_CommandPortEnable, m_iOutPortDecode, NULL);
	m_pCompResize->SendCommand(OMX_CommandPortEnable, m_iInPortResize, NULL);

	// once the state changes, both ports should become enabled and the resizer output should generate a settings changed event
	m_pCompDecode->WaitForEvent(OMX_EventCmdComplete, OMX_CommandPortEnable, m_iOutPortDecode, TIMEOUT_MS);
	m_pCompResize->WaitForEvent(OMX_EventCmdComplete, OMX_CommandPortEnable, m_iInPortResize, TIMEOUT_MS);

	// from what I have observed, this event is sent even if the settings have not changed (there is really no reason for them to change since it is a resizer)
	m_pCompResize->WaitForEvent(OMX_EventPortSettingsChanged, m_iOutPortResize, 0, TIMEOUT_MS);

	// disable port to be compliant with spec
	// UPDATE: this causes an error even though spec says we should do it
//	m_pCompResize->SendCommand(OMX_CommandPortDisable, m_iOutPortResize, NULL);
//	m_pCompResize->WaitForEvent(OMX_EventCmdComplete, OMX_CommandPortDisable, m_iOutPortResize, TIMEOUT_MS);

	// query output buffer requirements for resizer for debugging benefit
	//portdef.nSize = sizeof(OMX_PARAM_PORTDEFINITIONTYPE);
	//portdef.nVersion.nVersion = OMX_VERSION;
	//portdef.nPortIndex = m_iOutPortResize;
	//m_pCompResize->GetParameter(OMX_IndexParamPortDefinition, &portdef);

	// enable output port of resizer
//	m_pCompResize->SendCommand(OMX_CommandPortEnable, m_iOutPortResize, NULL);
//	m_pCompResize->WaitForEvent(OMX_EventCmdComplete, OMX_CommandPortEnable, m_iOutPortResize, TIMEOUT_MS);
}
