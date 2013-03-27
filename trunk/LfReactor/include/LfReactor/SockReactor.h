//define common SockReactor frame can use in leader followers thread
//mlcai 2013-03-07

#ifndef SockReactor_H
#define SockReactor_H

#include <map>
#include "Poco/Net/Net.h"
#include "Poco/Net/Socket.h"
#include "Poco/Runnable.h"
#include "Poco/Timespan.h"
#include "Poco/Observer.h"
#include "Poco/AutoPtr.h"

#include "NameSpaceDefine.h"

BEGIN_CXX_NAMESPACE_DEFINITION

class SockNotification;
class SockNotifier;
class ThreadManager;

class DYNAMIC_LIB SockReactor
{
public:
	SockReactor();
	explicit SockReactor(const Poco::Timespan& timeout);

	virtual ~SockReactor();

	void setTimeout(const Poco::Timespan& timeout);


	const Poco::Timespan& getTimeout() const;


	void registerEventHandler(const Poco::Net::Socket& socket, const Poco::AbstractObserver& observer);

	void removeEventHandler(const Poco::Net::Socket& socket, const Poco::AbstractObserver& observer);

	void handleEvents();


protected:
	virtual void onTimeout();

	virtual void onIdle();

	virtual void onShutdown();

	void dispatch(const Poco::Net::Socket& socket, SockNotification* pNotification);

	void dispatch(SockNotification* pNotification);

	void suspendEventHandler(const Poco::Net::Socket& socket, const Poco::AbstractObserver& observer);

	void resumeEventHandler(const Poco::Net::Socket& socket, const Poco::AbstractObserver& observer);

private:
	typedef Poco::AutoPtr<SockNotifier> NotifierPtr;
	typedef Poco::AutoPtr<SockNotification> NotificationPtr;
	typedef std::map<Poco::Net::Socket, NotifierPtr> EventHandlerMap;

	enum
	{
		DEFAULT_TIMEOUT = 250000
	};


	Poco::Timespan	m_timeout;

	EventHandlerMap	m_eventHandlers;
	Poco::FastMutex m_eventHMutex;

	NotificationPtr	m_pReadableNotification;
	NotificationPtr m_pWritableNotification;
	NotificationPtr m_pErrorNotification;
	NotificationPtr m_pTimeoutNotification;
	NotificationPtr m_pIdleNotification;
	NotificationPtr m_pShutdownNotification;

	Poco::Net::Socket::SocketList m_readableSockets;
	Poco::Net::Socket::SocketList m_writeableSockets;
	Poco::Net::Socket::SocketList m_exceptSockets;


	friend class SockNotifier;
	friend class ThreadManager;
};

END_CXX_NAMESPACE_DEFINITION

#endif
