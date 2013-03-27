//define socket notifications
//mlcai 2013-03-18

#ifndef SOCKNOTIFICATION_H_INCLUDED
#define SOCKNOTIFICATION_H_INCLUDED

#include "Poco/Net/Net.h"
#include "Poco/Net/Socket.h"
#include "Poco/Notification.h"

#include "NameSpaceDefine.h"

BEGIN_CXX_NAMESPACE_DEFINITION

class SockReactor;

class DYNAMIC_LIB SockNotification: public Poco::Notification
{
public:
	explicit SockNotification(SockReactor* pReactor);

	virtual ~SockNotification();

	SockReactor& source();

	Poco::Net::Socket& socket();

private:
	void setSocket(const Poco::Net::Socket& socket);

	SockReactor* m_pReactor;
	Poco::Net::Socket  m_socket;

	friend class SockNotifier;
};


class DYNAMIC_LIB ReadableNotification: public SockNotification
{
public:
	ReadableNotification(SockReactor* pReactor);

	~ReadableNotification();
};


class DYNAMIC_LIB WritableNotification: public SockNotification
{
public:
	WritableNotification(SockReactor* pReactor);

	~WritableNotification();
};


class DYNAMIC_LIB ErrorNotification: public SockNotification
{
public:
	ErrorNotification(SockReactor* pReactor);

	~ErrorNotification();
};


class DYNAMIC_LIB TimeoutNotification: public SockNotification
{
public:
	TimeoutNotification(SockReactor* pReactor);

	~TimeoutNotification();
};


class DYNAMIC_LIB IdleNotification: public SockNotification
{
public:
	IdleNotification(SockReactor* pReactor);

	~IdleNotification();
};


class DYNAMIC_LIB ShutdownNotification: public SockNotification
{
public:
	ShutdownNotification(SockReactor* pReactor);

	~ShutdownNotification();
};

END_CXX_NAMESPACE_DEFINITION

#endif // SOCKNOTIFICATION_H_INCLUDED
