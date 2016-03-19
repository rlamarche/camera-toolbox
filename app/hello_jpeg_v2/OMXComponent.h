// Written by Matt Ownby, August 2012
// You are free to use this for educational/non-commercial use

#ifndef OMXCOMPONENT_H
#define OMXCOMPONENT_H

//#include "IL/OMX_Broadcom.h"
#include "IL/OMX_Component.h"
#include "MyDeleter.h"
#include "ILogger.h"
#include "IEvent.h"
#include "Locker.h"
#include <list>

// uncomment this to get verbose logging of events
//#define VERBOSE

using namespace std;

class IOMXComponent;

class IOMXComponent
{
public:
	virtual OMX_HANDLETYPE GetHandle() = 0;
	virtual void GetParameter(OMX_INDEXTYPE nParamIndex, OMX_PTR pPtr) = 0;
	virtual void SetParameter(OMX_INDEXTYPE nParamIndex, OMX_PTR pPtr) = 0;
	virtual void SendCommand(OMX_COMMANDTYPE cmd, int nParam, void *pCmdData) = 0;
	virtual void SetupTunnel(OMX_U32 u32SrcPort, IOMXComponent *pDstComponent, OMX_U32 u32DstPort) = 0;
	virtual void RemoveTunnel(OMX_U32 u32Port) = 0;
	virtual void UseBuffer(OMX_BUFFERHEADERTYPE **ppHeader, OMX_U32 nPortIndex, OMX_PTR pAppPrivate, OMX_U32 nSizeBytes, OMX_U8 *pBuffer) = 0;
	virtual void EmptyThisBuffer(OMX_BUFFERHEADERTYPE *pHeader) = 0;
	virtual void FillThisBuffer(OMX_BUFFERHEADERTYPE *pHeader) = 0;
	virtual void FreeBuffer(OMX_U32 nPortIdx, OMX_BUFFERHEADERTYPE *pBuffer) = 0;
	virtual bool IsEventPending(OMX_EVENTTYPE eEvent, OMX_U32 nData1, OMX_U32 nData2) = 0;
	virtual IEventSPtr WaitForEvent(OMX_EVENTTYPE eEvent, OMX_U32 nData1, OMX_U32 nData2, unsigned int uTimeoutMs) = 0;
	virtual IEventSPtr WaitForEmpty(const OMX_BUFFERHEADERTYPE* pBuf, unsigned int uTimeoutMs) = 0;
	virtual IEventSPtr WaitForFill(const OMX_BUFFERHEADERTYPE* pBuf, unsigned int uTimeoutMs) = 0;
	virtual IEventSPtr WaitForAnything(unsigned int uTimeouts) = 0;
	virtual IEventSPtr WaitForEventOrEmpty(OMX_EVENTTYPE eEvent, OMX_U32 nData1, OMX_U32 nData2, const OMX_BUFFERHEADERTYPE* pEmptyBuffer, unsigned int uTimeoutMs) = 0;
	virtual size_t GetPendingEventCount() = 0;
	virtual size_t GetPendingEmptyCount() = 0;
	virtual size_t GetPendingFillCount() = 0;
};

typedef std::tr1::shared_ptr<IOMXComponent> IOMXComponentSPtr;

class OMXComponent : public IOMXComponent, public MyDeleter
{
	friend class OMXCore;	// only let the core allocate instances to ensure that everything is properly shut down

public:
	OMX_HANDLETYPE GetHandle();
	void GetParameter(OMX_INDEXTYPE nParamIndex, OMX_PTR pPtr);
	void SetParameter(OMX_INDEXTYPE nParamIndex, OMX_PTR pPtr);
	void SendCommand(OMX_COMMANDTYPE cmd, int nParam, OMX_PTR pCmdData);
	void SetupTunnel(OMX_U32 u32SrcPort, IOMXComponent *pDstComponent, OMX_U32 u32DstPort);
	void RemoveTunnel(OMX_U32 u32Port);
	void UseBuffer(OMX_BUFFERHEADERTYPE **ppHeader, OMX_U32 nPortIndex, OMX_PTR pAppPrivate, OMX_U32 nSizeBytes, OMX_U8 *pBuffer);
	void EmptyThisBuffer(OMX_BUFFERHEADERTYPE *pHeader);
	void FillThisBuffer(OMX_BUFFERHEADERTYPE *pHeader);
	void FreeBuffer(OMX_U32 nPortIdx, OMX_BUFFERHEADERTYPE *pBuffer);
	bool IsEventPending(OMX_EVENTTYPE eEvent, OMX_U32 nData1, OMX_U32 nData2);
	IEventSPtr WaitForEvent(OMX_EVENTTYPE eEvent, OMX_U32 nData1, OMX_U32 nData2, unsigned int uTimeoutMs);
	IEventSPtr WaitForEmpty(const OMX_BUFFERHEADERTYPE* pBuf, unsigned int uTimeoutMs);
	IEventSPtr WaitForFill(const OMX_BUFFERHEADERTYPE* pBuf, unsigned int uTimeoutMs);
	IEventSPtr WaitForAnything(unsigned int uTimeouts);
	IEventSPtr WaitForEventOrEmpty(OMX_EVENTTYPE eEvent, OMX_U32 nData1, OMX_U32 nData2, const OMX_BUFFERHEADERTYPE* pEmptyBuffer, unsigned int uTimeoutMs);
	size_t GetPendingEventCount();
	size_t GetPendingEmptyCount();
	size_t GetPendingFillCount();

private:
	IEventSPtr WaitForGeneric(const list<IEventSPtr> &lstEvents, unsigned int uTimeoutMs);

	static IOMXComponentSPtr GetInstance(ILogger *pLogger);

	OMXComponent(ILogger *);
	~OMXComponent();

	void DeleteInstance() { delete this; }

	// this must be deferred because OMXCore doesn't know what its value is until after we are instantiated
	void SetHandle(OMX_HANDLETYPE hComponent);

	void Lock();

	void Unlock();

/////////////////////////

        static OMX_ERRORTYPE EventHandlerCallback(OMX_HANDLETYPE hComponent, OMX_PTR pAppData,
                OMX_EVENTTYPE eEvent, OMX_U32 nData1, OMX_U32 nData2, OMX_PTR pEventData);
        static OMX_ERRORTYPE EmptyBufferDoneCallback(
                OMX_HANDLETYPE hComponent, OMX_PTR pAppData, OMX_BUFFERHEADERTYPE* pBuffer);
        static OMX_ERRORTYPE FillBufferDoneCallback(
                OMX_HANDLETYPE hComponent, OMX_PTR pAppData, OMX_BUFFERHEADERTYPE* pBufferHeader);

	OMX_ERRORTYPE EventHandler(OMX_EVENTTYPE eEvent, OMX_U32 nData1, OMX_U32 nData2, OMX_PTR pEventData);
	OMX_ERRORTYPE EmptyBufferDone(OMX_BUFFERHEADERTYPE* pBuffer);
	OMX_ERRORTYPE FillBufferDone(OMX_BUFFERHEADERTYPE* pBuffer);

/////////////////////////

	OMX_HANDLETYPE m_handle;
	ILogger *m_pLogger;
	ILockerSPtr m_locker;
	ILocker *m_pLocker;

	list<OMXEventData> m_lstEvents;
	list<EmptyBufferDoneData> m_lstEmpty;
	list<FillBufferDoneData> m_lstFill;
};

#endif // OMXCOMPONENT_H

