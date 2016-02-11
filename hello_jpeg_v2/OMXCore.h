// Written by Matt Ownby, August 2012
// You are free to use this for educational/non-commercial use

#ifndef OMXCORE_H
#define OMXCORE_H

#include "OMXComponent.h"
#include "ILogger.h"
#include "MyDeleter.h"
#include <map>

using namespace std;

class IOMXCore
{
public:
	virtual IOMXComponent *GetHandle(const char *cpszComponentName) = 0;
};

typedef std::tr1::shared_ptr<IOMXCore> IOMXCoreSPtr;

class OMXCore : public IOMXCore, public MyDeleter
{
public:
	static IOMXCoreSPtr GetInstance(ILogger *pLogger);

	IOMXComponent *GetHandle(const char *cpszComponentName);

private:
	OMXCore(ILogger *pLogger);
	~OMXCore();

	void DeleteInstance() { delete this; }

	void Init();
	void Shutdown();

//////////////////////////////////////////////////////////////////

	OMX_CALLBACKTYPE m_callbacks;
	map<OMX_HANDLETYPE, IOMXComponentSPtr> m_mapComponents;
	ILogger *m_pLogger;
};

#endif // OMXCORE_H

