//connector pattern use in thrMgr
//andycai.sc@gmail.com 2013-04-07

#ifndef SOCKCONNECTOR_H_INCLUDED
#define SOCKCONNECTOR_H_INCLUDED

#include "Poco/Net/Net.h"
#include "Poco/Net/ServerSocket.h"
#include "Poco/Net/StreamSocket.h"
#include "Poco/Observer.h"
#include "NameSpaceDefine.h"
#include ADD_QUOTE(INC_NAME_HEADER(NAMESPACE_NAME, ThreadManager.h))
#include ADD_QUOTE(INC_NAME_HEADER(NAMESPACE_NAME, SockNotification.h))

BEGIN_CXX_NAMESPACE_DEFINITION

template <class ServiceHandler>
class SockConnector
{
public:
	explicit SockConnector(Poco::Net::SocketAddress& address):
		m_thrManager(0)
	{
		m_socket.connectNB(address);
	}

	SockConnector(Poco::Net::SocketAddress& address, ThreadManager& thrMgr):
		m_thrManager(0)
	{
		m_socket.connectNB(address);
		registerConnector(thrMgr);
	}

	virtual ~SockConnector()
	{
		unregisterConnector();
	}

	virtual void registerConnector(ThreadManager& thrMgr)
	{
		m_thrManager = &thrMgr;
		m_thrManager->addEventHandler(m_socket, Poco::Observer<SockConnector, ReadableNotification>(*this, &SockConnector::onReadable), ThreadManager::MANUSUSRES);
		m_thrManager->addEventHandler(m_socket, Poco::Observer<SockConnector, WritableNotification>(*this, &SockConnector::onWritable), ThreadManager::MANUSUSRES);
		m_thrManager->addEventHandler(m_socket, Poco::Observer<SockConnector, ErrorNotification>(*this, &SockConnector::onError), ThreadManager::MANUSUSRES);
	}

	virtual void unregisterConnector()
	{
		if (m_thrManager)
		{
			m_thrManager->removeEventHandler(m_socket, Poco::Observer<SockConnector, ReadableNotification>(*this, &SockConnector::onReadable), ThreadManager::MANUSUSRES);
			m_thrManager->removeEventHandler(m_socket, Poco::Observer<SockConnector, WritableNotification>(*this, &SockConnector::onWritable), ThreadManager::MANUSUSRES);
			m_thrManager->removeEventHandler(m_socket, Poco::Observer<SockConnector, ErrorNotification>(*this, &SockConnector::onError), ThreadManager::MANUSUSRES);
		}
	}

	void onReadable(ReadableNotification* pNotification)
	{
		pNotification->release();
		int err = m_socket.impl()->socketError();
		if (err)
		{
			onError(err);
			unregisterConnector();
		}
		else
		{
			onConnect();
		}
	}

	void onWritable(WritableNotification* pNotification)
	{
		pNotification->release();
		onConnect();
	}

	void onConnect()
	{
		m_socket.setBlocking(true);
		createServiceHandler();
		unregisterConnector();
	}

	void onError(ErrorNotification* pNotification)
	{
		pNotification->release();
		onError(m_socket.impl()->socketError());
		unregisterConnector();
	}

protected:
	virtual ServiceHandler* createServiceHandler()
	{
		return new ServiceHandler(m_socket, *m_thrManager);
	}

	virtual void onError(int errorCode)
	{
	}

	ThreadManager* thrMgr()
	{
		return m_thrManager;
	}

	Poco::Net::Socket& socket()
	{
		return m_socket;
	}

private:
	SockConnector();
	SockConnector(const SockConnector&);
	SockConnector& operator = (const SockConnector&);

	Poco::Net::StreamSocket m_socket;
	ThreadManager* m_thrManager;
};

END_CXX_NAMESPACE_DEFINITION

#endif // SOCKCONNECTOR_H_INCLUDED
