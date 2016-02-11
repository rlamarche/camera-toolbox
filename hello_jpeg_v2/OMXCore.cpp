// Written by Matt Ownby, August 2012
// You are free to use this for educational/non-commercial use

#include "OMXCore.h"
#include <stdexcept>

using namespace std;

IOMXCoreSPtr OMXCore::GetInstance(ILogger *pLogger)
{
	return IOMXCoreSPtr(new OMXCore(pLogger), OMXCore::deleter());
}

IOMXComponent *OMXCore::GetHandle(const char *cpszComponentName)
{
	OMX_HANDLETYPE hComponent = 0;

	// instantiate OMXComponent with handle value
	IOMXComponentSPtr comp = OMXComponent::GetInstance(m_pLogger);
	OMXComponent *pComponent = (OMXComponent *) comp.get();	// need to access private SetHandle method

	OMX_ERRORTYPE err = OMX_GetHandle(&hComponent, (char *) cpszComponentName, pComponent, &m_callbacks);
	if (err != OMX_ErrorNone)
	{
		throw runtime_error("OMX_GetHandle failed");
	}

	// set handle on target component now that we have it
	pComponent->SetHandle(hComponent);

	m_mapComponents[hComponent] = comp;

	return pComponent;
}

OMXCore::OMXCore(ILogger *pLogger) :
m_pLogger(pLogger)
{
	Init();
}

OMXCore::~OMXCore()
{
	Shutdown();
}

void OMXCore::Init()
{
	m_callbacks.EventHandler = OMXComponent::EventHandlerCallback;
	m_callbacks.EmptyBufferDone = OMXComponent::EmptyBufferDoneCallback;
	m_callbacks.FillBufferDone = OMXComponent::FillBufferDoneCallback;

	if (OMX_Init() != OMX_ErrorNone)
	{
		throw runtime_error("OMX_Init failed");
	}
}

void OMXCore::Shutdown()
{
	// free all handles
	for (map<OMX_HANDLETYPE, IOMXComponentSPtr>::iterator mi = m_mapComponents.begin();
		mi != m_mapComponents.end(); mi++)
	{
		OMX_FreeHandle(mi->first);
	}
	m_mapComponents.clear();

	if (OMX_Deinit() != OMX_ErrorNone)
	{
		throw runtime_error("OMX_Deinit failed");
	}
}
