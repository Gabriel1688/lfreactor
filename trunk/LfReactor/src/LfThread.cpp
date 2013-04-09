//implement LfThread
//mlcai 2013-03-07

#include "NameSpaceDefine.h"
#include ADD_QUOTE(INC_NAME_HEADER(NAMESPACE_NAME, ThreadManager.h))

BEGIN_CXX_NAMESPACE_DEFINITION

LfThread::LfThread(ThreadManager* thrMgr) : m_thrManager(thrMgr),
m_isRunning(false), m_isLeader(false)
{
}

LfThread::~LfThread()
{
}

void LfThread::active()
{
    m_isLeader = true;
	m_condition.set();
}

void LfThread::stop()
{
	m_isRunning = false;
	m_isLeader = true;
	m_condition.set();
}

void LfThread::run()
{
	m_isRunning = true;

	while (m_isRunning)
	{
		if (m_thrManager->join(this) != this) //try to be a leader, if can't to be a follower
		{
		    while (!m_isLeader)
                m_condition.tryWait(100);
			m_isLeader = false;

			if (!m_isRunning)
                return;
		}

		m_thrManager->handleEvents();
	}
}

END_CXX_NAMESPACE_DEFINITION
