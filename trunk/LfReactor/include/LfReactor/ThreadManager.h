//this class use to manage leader and followers in thread set
//mlcai 2013-3-7

#ifndef THREADMANAGER_H
#define THREADMANAGER_H

#include <stack>
#include "NameSpaceDefine.h"
#include ADD_QUOTE(INC_NAME_HEADER(NAMESPACE_NAME, SockReactor.h))
#include ADD_QUOTE(INC_NAME_HEADER(NAMESPACE_NAME, LfThread.h))

BEGIN_CXX_NAMESPACE_DEFINITION

class LfEventObserver;
class DYNAMIC_LIB ThreadManager
{
public:
	ThreadManager(SockReactor* SockReactor = &SockReactor::instance());
	~ThreadManager();

	void registerEventHandler(const Poco::Net::Socket& socket, const Poco::AbstractObserver& observer);

	void removeEventHandler(const Poco::Net::Socket& socket, const Poco::AbstractObserver& observer);

	LfThread* join(LfThread* lfThr);

	void promoteNewLeader();

	void handleEvents();

private:
    void suspendEventHandler(const Poco::Net::Socket& socket, const Poco::AbstractObserver& observer);

	void resumeEventHandler(const Poco::Net::Socket& socket, const Poco::AbstractObserver& observer);

	SockReactor* m_reactor;

	Poco::Mutex m_thrMutex;
	std::stack<LfThread*> m_thrStack;
	LfThread* m_leaderThr;

	friend class LfEventObserver;
};

END_CXX_NAMESPACE_DEFINITION

#endif
