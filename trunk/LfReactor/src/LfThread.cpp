// implement LfThread
// andycai.sc@gmail.com 2013-03-07
//
// Permission is hereby granted, free of charge, to any person or organization
// obtaining a copy of the software and accompanying documentation covered by
// this license (the "Software") to use, reproduce, display, distribute,
// execute, and transmit the Software, and to prepare derivative works of the
// Software, and to permit third-parties to whom the Software is furnished to
// do so, all subject to the following:
//
// The copyright notices in the Software and this entire statement, including
// the above license grant, this restriction and the following disclaimer,
// must be included in all copies of the Software, in whole or in part, and
// all derivative works of the Software, unless such copies or derivative
// works are solely in the form of machine-executable object code generated by
// a source language processor.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE, TITLE AND NON-INFRINGEMENT. IN NO EVENT
// SHALL THE COPYRIGHT HOLDERS OR ANYONE DISTRIBUTING THE SOFTWARE BE LIABLE
// FOR ANY DAMAGES OR OTHER LIABILITY, WHETHER IN CONTRACT, TORT OR OTHERWISE,
// ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
// DEALINGS IN THE SOFTWARE.
//

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