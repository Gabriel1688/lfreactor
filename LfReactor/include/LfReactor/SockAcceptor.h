// this acceptor use to acception connnect from client socket
// andycai.sc@gmail.com 2013-03-26
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
        m_thrManager->addEventHandler(m_socket, Poco::Observer<SockAcceptor, ReadableNotification>(*this, &SockAcceptor::onAccept));
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
