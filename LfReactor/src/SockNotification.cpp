//implement SockNotifications
//mlcai 2013-03-18

#include "NameSpaceDefine.h"
#include ADD_QUOTE(INC_NAME_HEADER(NAMESPACE_NAME, SockNotification.h))

BEGIN_CXX_NAMESPACE_DEFINITION

SockNotification::SockNotification(SockReactor* pReactor):
	m_pReactor(pReactor)
{
}


SockNotification::~SockNotification()
{
}


void SockNotification::setSocket(const Poco::Net::Socket& socket)
{
	m_socket = socket;
}

SockReactor& SockNotification::source()
{
    return *m_pReactor
}

Poco::Net::Socket& SockNotification::socket()
{
    return m_socket;
}

ReadableNotification::ReadableNotification(SockReactor* pReactor):
	SockNotification(pReactor)
{
}


ReadableNotification::~ReadableNotification()
{
}


WritableNotification::WritableNotification(SockReactor* pReactor):
	SockNotification(pReactor)
{
}


WritableNotification::~WritableNotification()
{
}


ErrorNotification::ErrorNotification(SockReactor* pReactor):
	SockNotification(pReactor)
{
}


ErrorNotification::~ErrorNotification()
{
}


TimeoutNotification::TimeoutNotification(SockReactor* pReactor):
	SockNotification(pReactor)
{
}


TimeoutNotification::~TimeoutNotification()
{
}


IdleNotification::IdleNotification(SockReactor* pReactor):
	SockNotification(pReactor)
{
}


IdleNotification::~IdleNotification()
{
}


ShutdownNotification::ShutdownNotification(SockReactor* pReactor):
	SockNotification(pReactor)
{
}


ShutdownNotification::~ShutdownNotification()
{
}

END_CXX_NAMESPACE_DEFINITION
