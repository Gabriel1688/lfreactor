//define LfThread for Leader followers pattern
//mlcai 2013-03-07

#ifndef LFTHREAD_H
#define LFTHREAD_H

#include "NameSpaceDefine.h"
#include <Poco/Runnable.h>
#include <Poco/Event.h>

BEGIN_CXX_NAMESPACE_DEFINITION

class ThreadManager;
class DYNAMIC_LIB LfThread : public Poco::Runnable
{
public:
	LfThread(ThreadManager* thrMgr);
	~LfThread();

	void active();

	void stop();

    void run();

private:
	ThreadManager* m_thrManager;
	Poco::Event m_condition;
	bool m_isRunning;
	bool m_isLeader;
};

END_CXX_NAMESPACE_DEFINITION

#endif
