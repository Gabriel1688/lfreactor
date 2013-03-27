//this acceptor use to acception connnect from client socket
//andycai.sc@gmail.com 2013-03-26

#ifndef SOCKACCEPTOR_H_INCLUDED
#define SOCKACCEPTOR_H_INCLUDED

#include "Poco/Net/Net.h"
#include "Poco/Net/ServerSocket.h"
#include "Poco/Net/StreamSocket.h"
#include "Poco/Observer.h"
#include "NameSpaceDefine.h"
#include ADD_QUOTE(INC_NAME_HEADER(NAMESPACE_NAME, ThreadManager.h))
#include ADD_QUOTE(INC_NAME_HEADER(NAMESPACE_NAME, SockNotification.h))

BEGIN_CXX_NAMESPACE_DEFINITION

template <typename ServiceHandler>
class SockAcceptor
{
public:
    explicit SockAcceptor(Poco::Net::ServerSocket& socket) :
        m_socket(socket), m_thrManager(0)
    {
    }

    SockAcceptor(Poco::Net::ServerSocket& socket, ThreadManager& thrManager) :
        m_socket(socket), m_thrManager(0)
    {
        registerAcceptor(thrManager);
    }

    virtual ~SockAcceptor()
    {
        unregisterAcceptor();
    }

    virtual void registerAcceptor(ThreadManager& thrManager)
    {
        m_thrManager = &thrManager;
        m_thrManager->registerEventHandler(m_socket, Poco::Observer<SockAcceptor, ReadableNotification>(*this, &SockAcceptor::onAccept));
    }

    virtual void unregisterAcceptor()
    {
        if (m_thrManager)
        {
            m_thrManager->removeEventHandler(m_socket, Poco::Observer<SockAcceptor, ReadableNotification>(*this, &SockAcceptor::onAccept));
        }
    }

    void onAccept(ReadableNotification* pNotification)
    {
        pNotification->release();
        Poco::Net::StreamSocket socket = m_socket.acceptConnection();
        createServiceHandler(socket);
    }

protected:
    virtual ServiceHandler* createServiceHandler(Poco::Net::StreamSocket& socket)
    {
        return new ServiceHandler(socket, *m_thrManager);
    }

    ThreadManager* threadMgr()
    {
        return m_thrManager;
    }

    Poco::Net::Socket& socket()
    {
        return m_socket;
    }

private:
    SockAcceptor();
    SockAcceptor(const SockAcceptor&);
    SockAcceptor& operator = (const SockAcceptor&);

    Poco::Net::ServerSocket m_socket;
    ThreadManager* m_thrManager;
};

END_CXX_NAMESPACE_DEFINITION

#endif // SOCKACCEPTOR_H_INCLUDED
