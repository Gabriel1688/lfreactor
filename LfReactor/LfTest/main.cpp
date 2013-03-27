//this is lf test code, it's show how to use the library
//andycai.sc@gmail.com

#include "Poco/ThreadPool.h"
#include "Poco/Net/StreamSocket.h"
#include "LfReactor/SockAcceptor.h"

#include <iostream>

using namespace Poco;
using namespace std;

class ServiceHandler
{
public:
    ServiceHandler(Net::StreamSocket& socket, LfReactor::ThreadManager& thMgr) :
        m_socket(socket), m_thMgr(&thMgr)
    {
        m_thMgr->registerEventHandler(m_socket, Poco::Observer<ServiceHandler, LfReactor::ReadableNotification>(*this, &ServiceHandler::onReadable));
    }

    ~ServiceHandler()
    {
        m_thMgr->removeEventHandler(m_socket, Poco::Observer<ServiceHandler, LfReactor::ReadableNotification>(*this, &ServiceHandler::onReadable));
    }

    void onReadable(LfReactor::ReadableNotification* pNotification)
    {
        pNotification->release();
    }

private:
    Net::StreamSocket m_socket;
    LfReactor::ThreadManager* m_thMgr;
};

int main()
{
    LfReactor::ThreadManager thrMgr;
    Net::ServerSocket socket;
    LfReactor::SockAcceptor<ServiceHandler> acceptor(socket, thrMgr);
    LfReactor::LfThread thr1(&thrMgr);
    LfReactor::LfThread thr2(&thrMgr);
    LfReactor::LfThread thr3(&thrMgr);
    LfReactor::LfThread thr4(&thrMgr);

    ThreadPool thrPool(3);
    thrPool.start(thr1);
    thrPool.start(thr2);
    thrPool.start(thr3);

    thr4.run();

    return 0;
}
