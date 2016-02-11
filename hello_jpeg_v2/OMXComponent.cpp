// Written by Matt Ownby, August 2012
// You are free to use this for educational/non-commercial use

#include "OMXComponent.h"
#include "Event.h"
#include <stdexcept>
#include <sys/time.h>	// for gettimeofday
#include <unistd.h>	// for gettimeofday
#include <stdio.h>	// for sprintf
#include <assert.h>

using namespace std;

OMX_HANDLETYPE OMXComponent::GetHandle()
{
	return m_handle;
}

void OMXComponent::GetParameter(OMX_INDEXTYPE nParamIndex, OMX_PTR pPtr)
{
	if (OMX_GetParameter(m_handle, nParamIndex, pPtr) != OMX_ErrorNone)
	{
		throw runtime_error("OMX_GetParameter failed");
	}
}

void OMXComponent::SetParameter(OMX_INDEXTYPE nParamIndex, OMX_PTR pPtr)
{
	OMX_ERRORTYPE err = OMX_SetParameter(m_handle, nParamIndex, pPtr);

	if (err != OMX_ErrorNone)
	{
		throw runtime_error("OMX_SetParameter failed");
	}
}

void OMXComponent::SendCommand(OMX_COMMANDTYPE cmd, int nParam, OMX_PTR pCmdData)
{
	if (OMX_SendCommand(m_handle, cmd, nParam, pCmdData) != OMX_ErrorNone)
	{
		throw runtime_error("OMX_SendCommand failed");
	}
}

void OMXComponent::SetupTunnel(OMX_U32 u32SrcPort, IOMXComponent *pDstComponent, OMX_U32 u32DstPort)
{
	OMX_HANDLETYPE hDst = pDstComponent->GetHandle();
	OMX_ERRORTYPE err = OMX_SetupTunnel(m_handle, u32SrcPort, hDst, u32DstPort);

	if (err != OMX_ErrorNone)
	{
		throw runtime_error("OMX_SetupTunnel failed");
	}
}

void OMXComponent::RemoveTunnel(OMX_U32 u32Port)
{
	OMX_ERRORTYPE err = OMX_SetupTunnel(m_handle, u32Port, NULL, 0);
	if (err != OMX_ErrorNone)
	{
		throw runtime_error("RemoveTunnel failed");
	}
}

void OMXComponent::UseBuffer(OMX_BUFFERHEADERTYPE **ppHeader, OMX_U32 nPortIndex, OMX_PTR pAppPrivate, OMX_U32 nSizeBytes, OMX_U8 *pBuffer)
{
	OMX_ERRORTYPE err = OMX_UseBuffer(m_handle, ppHeader, nPortIndex, pAppPrivate, nSizeBytes, pBuffer);

	if (err != OMX_ErrorNone)
	{
		throw runtime_error("OMX_UseBuffer failed");
	}
}

void OMXComponent::EmptyThisBuffer(OMX_BUFFERHEADERTYPE *pHeader)
{
	if (OMX_EmptyThisBuffer(m_handle, pHeader) != OMX_ErrorNone)
	{
		throw runtime_error("OMX_EmptyThisBuffer failed");
	}
}

void OMXComponent::FillThisBuffer(OMX_BUFFERHEADERTYPE *pHeader)
{
	if (OMX_FillThisBuffer(m_handle, pHeader) != OMX_ErrorNone)
	{
		throw runtime_error("OMX_FillThisBuffer failed");
	}
}

void OMXComponent::FreeBuffer(OMX_U32 nPortIdx, OMX_BUFFERHEADERTYPE *pBuffer)
{
	OMX_ERRORTYPE err = OMX_FreeBuffer(m_handle, nPortIdx, pBuffer);

	if (err != OMX_ErrorNone)
	{
		throw runtime_error("OMX_FreeBuffer failed");
	}
}

static void add_timespecs(struct timespec &time, long millisecs)
{
	time.tv_sec  += millisecs / 1000;
	time.tv_nsec += (millisecs % 1000) * 1000000;
	if (time.tv_nsec > 1000000000)
	{
		time.tv_sec  += 1;
		time.tv_nsec -= 1000000000;
	}
}

bool OMXComponent::IsEventPending(OMX_EVENTTYPE eEvent, OMX_U32 nData1, OMX_U32 nData2)
{
	bool bRes = false;

	Lock();

	for (list<OMXEventData>::iterator li = m_lstEvents.begin(); li != m_lstEvents.end(); li++)
	{
		// if we match the event type
		if (li->eEvent == eEvent)
		{
			// if we also match the data types
			if ((nData1 == li->nData1) && (nData2 == li->nData2))
			{
				bRes = true;
				break;
			}
		}
	}

	Unlock();

	return bRes;
}

IEventSPtr OMXComponent::WaitForEvent(OMX_EVENTTYPE eEvent, OMX_U32 nData1, OMX_U32 nData2, unsigned int uTimeoutMs)
{
	list<IEventSPtr> lstEvents;

	OMXEventData evOMX;
	evOMX.eEvent = eEvent;
	evOMX.nData1 = nData1;
	evOMX.nData2 = nData2;
	evOMX.pEventData = NULL;

	lstEvents.push_back(IEventSPtr(new OMXEvent(evOMX)));

	return WaitForGeneric(lstEvents, uTimeoutMs);
}

IEventSPtr OMXComponent::WaitForEmpty(const OMX_BUFFERHEADERTYPE* pEmptyBuffer, unsigned int uTimeoutMs)
{
	list<IEventSPtr> lstEvents;

	EmptyBufferDoneData evEmpty;
	evEmpty.pBuffer = pEmptyBuffer;
	lstEvents.push_back(IEventSPtr(new EmptyBufferDoneEvent(evEmpty)));

	return WaitForGeneric(lstEvents, uTimeoutMs);
}

IEventSPtr OMXComponent::WaitForFill(const OMX_BUFFERHEADERTYPE* pFillBuffer, unsigned int uTimeoutMs)
{
	list<IEventSPtr> lstEvents;

	FillBufferDoneData evFill;
	evFill.pBuffer = pFillBuffer;
	lstEvents.push_back(IEventSPtr(new FillBufferDoneEvent(evFill)));

	return WaitForGeneric(lstEvents, uTimeoutMs);
}

IEventSPtr OMXComponent::WaitForAnything(unsigned int uTimeoutMs)
{
	struct timespec tsEnd;
	clock_gettime(CLOCK_REALTIME, &tsEnd);
	add_timespecs(tsEnd, uTimeoutMs);

	IEventSPtr pRes;
	bool bFound = false;
	bool bFailure = false;

	while (!bFound)
	{
		Lock();

		// if we have an event
		if (!m_lstEvents.empty())
		{
			IEvent *pEvent = new OMXEvent(m_lstEvents.front());
			pRes = IEventSPtr(pEvent);
			m_lstEvents.pop_front();
			bFound = true;
		}

		// if we have an empty buffer done
		else if (!m_lstEmpty.empty())
		{
			pRes = IEventSPtr(new EmptyBufferDoneEvent(m_lstEmpty.front()));
			m_lstEmpty.pop_front();
			bFound = true;
		}
		// else if we have a fill buffer done
		else if (!m_lstFill.empty())
		{
			pRes = IEventSPtr(new FillBufferDoneEvent(m_lstFill.front()));
			m_lstFill.pop_front();
			bFound = true;
		}

		// if we didn't get a match we need to wait for success or a timeout
		if (!bFound)
		{
			// If we got an error or timed out, then we're done
			// (this implicitly unlocks the mutex during the waiting period, then relocks it upon returning!)
			if (!m_pLocker->WaitForEvent(&tsEnd))
			{
				bFailure = true;
			}
		}

		Unlock();

		if (bFailure) throw runtime_error("Waiting timed out");

	}	// end for

	return pRes;
}

IEventSPtr OMXComponent::WaitForEventOrEmpty(OMX_EVENTTYPE eEvent, OMX_U32 nData1, OMX_U32 nData2, const OMX_BUFFERHEADERTYPE* pEmptyBuffer, unsigned int uTimeoutMs)
{
	IEventSPtr pRes;

	list<IEventSPtr> lstEvents;

	EmptyBufferDoneData evEmpty;
	evEmpty.pBuffer = pEmptyBuffer;

	lstEvents.push_back(IEventSPtr(new EmptyBufferDoneEvent(evEmpty)));

	OMXEventData evOMX;
	evOMX.eEvent = eEvent;
	evOMX.nData1 = nData1;
	evOMX.nData2 = nData2;
	evOMX.pEventData = NULL;

	lstEvents.push_back(IEventSPtr(new OMXEvent(evOMX)));

	pRes = WaitForGeneric(lstEvents, uTimeoutMs);

	return pRes;
}

size_t OMXComponent::GetPendingEventCount()
{
	return m_lstEvents.size();
}

size_t OMXComponent::GetPendingEmptyCount()
{
	return m_lstEmpty.size();
}

size_t OMXComponent::GetPendingFillCount()
{
	return m_lstFill.size();
}

IEventSPtr OMXComponent::WaitForGeneric(const list<IEventSPtr> &lstEvents, unsigned int uTimeoutMs)
{
	IEventSPtr pRes;

	struct timespec tsEnd;
	clock_gettime(CLOCK_REALTIME, &tsEnd);
	add_timespecs(tsEnd, uTimeoutMs);

	bool bMatch = false;
	bool bFailure = false;

	// go until we either fail or succeed
	while ((!bFailure) && (!bMatch))
	{
		// go through all things we _can_ match with ...
		for (list<IEventSPtr>::const_iterator lsi = lstEvents.begin(); lsi != lstEvents.end(); lsi++)
		{
			// determine what type of event this is
			IEvent *pEvent = lsi->get();
			OMXEventData *pOMX = pEvent->ToEvent();
			EmptyBufferDoneData *pEmpty = pEvent->ToEmpty();
			FillBufferDoneData *pFill = pEvent->ToFill();

			Lock();

			if (pEmpty != NULL)
			{
				for (list<EmptyBufferDoneData>::iterator li = m_lstEmpty.begin(); li != m_lstEmpty.end(); li++)
				{
					if (li->pBuffer == pEmpty->pBuffer)
					{
						bMatch = true;
						m_lstEmpty.erase(li);
						pRes = *lsi;
						break;
					}
				}
			}

			// if we are waiting for an OMXEvent
			else if (pOMX != NULL)
			{
				for (list<OMXEventData>::iterator li = m_lstEvents.begin(); li != m_lstEvents.end(); li++)
				{
					// if we found the match
					if ((li->eEvent == pOMX->eEvent) && (pOMX->nData1 == li->nData1) && (pOMX->nData2 == li->nData2))
					{
						bMatch = true;
						m_lstEvents.erase(li);	// remove this item from the list because the caller knows about it now
						pRes = *lsi;
						break;	// must break here to about segfault since we just called erase()
					}
				}
			}
			else if (pFill != NULL)
			{
				for (list<FillBufferDoneData>::iterator li = m_lstFill.begin(); li != m_lstFill.end(); li++)
				{
					if (li->pBuffer == pFill->pBuffer)
					{
						bMatch = true;
						m_lstFill.erase(li);
						pRes = *lsi;
						break;
					}
				}
			}
			// else this should never happen
			else
			{
				assert(false);
			}

			Unlock();

			// if we found a match, we're done
			if (bMatch)
			{
				break;
			}
		}	// end for

		// if we didn't get a match we need to wait for success or a timeout
		if (!bMatch)
		{
			Lock();

			// If we got an error or timed out, then we're done
			// (this implicitly unlocks the mutex during the waiting period, then relocks it upon returning!)
			if (!m_pLocker->WaitForEvent(&tsEnd))
			{
				bFailure = true;
			}

			Unlock();
		}

	} // end while we haven't succeeded or failed

	if (bFailure)
	{
		throw runtime_error("Waiting timed out");
	}

	return pRes;
}

////////////////////////////////////////////////////////////////////////////////////////////

IOMXComponentSPtr OMXComponent::GetInstance(ILogger *pLogger)
{
	return IOMXComponentSPtr(new OMXComponent(pLogger), OMXComponent::deleter());
}

OMXComponent::OMXComponent(ILogger *pLogger) :
m_pLogger(pLogger)
{
	m_locker = Locker::GetInstance();
	m_pLocker = m_locker.get();
}

OMXComponent::~OMXComponent()
{
}

void OMXComponent::SetHandle(OMX_HANDLETYPE hComponent)
{
	m_handle = hComponent;
}

void OMXComponent::Lock()
{
	m_pLocker->Lock();
}

void OMXComponent::Unlock()
{
	m_pLocker->Unlock();
}

OMX_ERRORTYPE OMXComponent::EventHandlerCallback(OMX_HANDLETYPE hComponent, OMX_PTR pAppData,
												 OMX_EVENTTYPE eEvent, OMX_U32 nData1, OMX_U32 nData2, OMX_PTR pEventData)
{
	OMXComponent *pInstance = (OMXComponent *) pAppData;
	return pInstance->EventHandler(eEvent, nData1, nData2, pEventData);
}

OMX_ERRORTYPE OMXComponent::EmptyBufferDoneCallback(OMX_HANDLETYPE hComponent, OMX_PTR pAppData, OMX_BUFFERHEADERTYPE* pBuffer)
{
	OMXComponent *pInstance = (OMXComponent *) pAppData;
	return pInstance->EmptyBufferDone(pBuffer);
}

OMX_ERRORTYPE OMXComponent::FillBufferDoneCallback(OMX_HANDLETYPE hComponent, OMX_PTR pAppData, OMX_BUFFERHEADERTYPE* pBufferHeader)
{
	OMXComponent *pInstance = (OMXComponent *) pAppData;
	return pInstance->FillBufferDone(pBufferHeader);
}

OMX_ERRORTYPE OMXComponent::EventHandler(OMX_EVENTTYPE eEvent, OMX_U32 nData1, OMX_U32 nData2, OMX_PTR pEventData)
{
#ifdef VERBOSE
	const char *cpszType = NULL;
	switch (eEvent)
	{
	case OMX_EventCmdComplete:
		cpszType = "OMX_EventCmdComplete";
		break;
	case OMX_EventError:
		cpszType = "OMX_EventError";
		break;
	case OMX_EventMark:
		cpszType = "OMX_EventMark";
		break;
	case OMX_EventPortSettingsChanged:
		cpszType = "OMX_EventPortSettingsChanged";
		break;
	case OMX_EventBufferFlag:
		cpszType = "OMX_EventBufferFlag";
		break;
	default:
		cpszType = "OMX_Event?? (add to switch statement)";
		break;
	}
	char s[160];
	snprintf(s, sizeof(s), "Handle %x got event: %s (%u) ndata1: %u ndata2: %u pEventData %x", (unsigned int) m_handle, cpszType, (unsigned int) eEvent, (unsigned int) nData1, (unsigned int) nData2, (unsigned int) pEventData);
#endif // VERBOSE

	OMXEventData e;
	e.eEvent = eEvent;
	e.nData1 = nData1;
	e.nData2 = nData2;
	e.pEventData = pEventData;

	Lock();
#ifdef VERBOSE
	m_pLogger->Log(s);
#endif // VERBOSE
	m_lstEvents.push_back(e);
	m_pLocker->GenerateEvent();
	Unlock();
	return OMX_ErrorNone;
}

OMX_ERRORTYPE OMXComponent::EmptyBufferDone(OMX_BUFFERHEADERTYPE* pBuffer)
{
	EmptyBufferDoneData dat;
	dat.pBuffer = pBuffer;

	Lock();
#ifdef VERBOSE
	m_pLogger->Log("Got EmptyBufferDone");
#endif // VERBOSE

	m_lstEmpty.push_back(dat);
	m_pLocker->GenerateEvent();
	Unlock();
	return OMX_ErrorNone;
}

OMX_ERRORTYPE OMXComponent::FillBufferDone(OMX_BUFFERHEADERTYPE* pBuffer)
{
	FillBufferDoneData dat;
	dat.pBuffer = pBuffer;

	Lock();
#ifdef VERBOSE
	m_pLogger->Log("Got FillBufferDone");
#endif // VERBOSE

	m_lstFill.push_back(dat);
	m_pLocker->GenerateEvent();
	Unlock();
	return OMX_ErrorNone;
}
