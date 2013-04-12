// this class use to manage leader and followers in thread set
// andycai.sc@gmail.com 2013-3-7
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

	enum Type
	{
	    AUTOSUSRES = 0, //auto suspend and resume event
        MANUSUSRES //manual suspend and resume event
	};

	void addEventHandler(const Poco::Net::Socket& socket, const Poco::AbstractObserver& observer, Type type = AUTOSUSRES);

	void removeEventHandler(const Poco::Net::Socket& socket, const Poco::AbstractObserver& observer, Type type = AUTOSUSRES);

	LfThread* join(LfThread* lfThr);

	void promoteNewLeader();

	void handleEvents();

	void stopAll();

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
