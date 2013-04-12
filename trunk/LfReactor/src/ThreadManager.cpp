//imeplement ThreadMannager
//andycai.sc@gmail.com 2013-03-07

#include "NameSpaceDefine.h"
#include ADD_QUOTE(INC_NAME_HEADER(NAMESPACE_NAME, ThreadManager.h))
#include ADD_QUOTE(INC_NAME_HEADER(NAMESPACE_NAME, LfEventObserver.h))

BEGIN_CXX_NAMESPACE_DEFINITION

ThreadManager::ThreadManager(SockReactor* reactor) : m_reactor(reactor), m_leaderThr(0)
{
}

ThreadManager::~ThreadManager()
{
}

void ThreadManager::addEventHandler(const Poco::Net::Socket& socket, const Poco::AbstractObserver& observer, Type type)
{
    if (AUTOSUSRES == type)
        m_reactor->addEventHandler(socket, LfEventObserver(socket, observer, *this));
    else if (MANUSUSRES == type)
        m_reactor->addEventHandler(socket, observer);
}

void ThreadManager::removeEventHandler(const Poco::Net::Socket& socket, const Poco::AbstractObserver& observer, Type type)
{
    if (AUTOSUSRES == type)
        m_reactor->removeEventHandler(socket, LfEventObserver(socket, observer, *this));
    else if (MANUSUSRES == type)
        m_reactor->removeEventHandler(socket, observer);
}

LfThread* ThreadManager::join(LfThread* lfThr)
{
	Poco::Mutex::ScopedLock locker(m_thrMutex);
	if (0 != m_leaderThr && m_leaderThr != lfThr)
		m_thrStack.push(lfThr);
	else
		m_leaderThr = lfThr;
	return m_leaderThr;
}

void ThreadManager::promoteNewLeader()
{
	Poco::Mutex::ScopedLock locker(m_thrMutex);
	if (m_thrStack.empty())
		m_leaderThr = 0;
	else
	{
		m_leaderThr = m_thrStack.top();
		m_thrStack.pop();
		m_leaderThr->active();
	}
}

void ThreadManager::handleEvents()
{
	m_reactor->handleEvents();
}

void ThreadManager::stopAll()
{
    Poco::Mutex::ScopedLock locker(m_thrMutex);
    while (!m_thrStack.empty())
    {
        LfThread* lfThr = m_thrStack.top();
        lfThr->stop();
        m_thrStack.pop();
    }

    if (m_leaderThr)
        m_leaderThr->stop();
}

void ThreadManager::suspendEventHandler(const Poco::Net::Socket& socket, const Poco::AbstractObserver& observer)
{
    m_reactor->suspendEventHandler(socket, observer);
}

void ThreadManager::resumeEventHandler(const Poco::Net::Socket& socket, const Poco::AbstractObserver& observer)
{
    m_reactor->resumeEventHandler(socket, observer);
}

END_CXX_NAMESPACE_DEFINITION
