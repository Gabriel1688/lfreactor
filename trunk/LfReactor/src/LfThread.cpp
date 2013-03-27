//implement LfThread
//mlcai 2013-03-07

#include "NameSpaceDefine.h"
#include ADD_QUOTE(INC_NAME_HEADER(NAMESPACE_NAME, ThreadManager.h))

BEGIN_CXX_NAMESPACE_DEFINITION

LfThread::LfThread(ThreadManager* thrMgr) : m_thrManager(thrMgr),m_isRunning(false)
{
}

LfThread::~LfThread()
{
}

void LfThread::active()
{
	m_condition.set();
}

void LfThread::stop()
{
	m_isRunning = false;
}

void LfThread::run()
{
	m_isRunning = true;

	while (m_isRunning)
	{
		if (m_thrManager->join(this) != this) //try to be a leader, if can't to be a follower
			m_condition.wait();

		m_thrManager->handleEvents();
	}
}

END_CXX_NAMESPACE_DEFINITION
