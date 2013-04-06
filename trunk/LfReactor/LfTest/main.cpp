//this is lf test code, it's show how to use the library
//andycai.sc@gmail.com

#include "Poco/ThreadPool.h"
#include "Poco/Net/StreamSocket.h"
#include "LfReactor/SockAcceptor.h"

#include <cstring>
#include <cstdio>
#include <iostream>

using namespace Poco;
using namespace std;

Poco::Mutex consoleMutex;

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

        char msg[100];
        memset(msg, 0, sizeof(msg));

        int n = m_socket.receiveBytes(msg, sizeof(msg));

        if (n <= 0)
        {
            delete this;
            return;
        }

        {
            Mutex::ScopedLock lock(consoleMutex);
            cout << "thread " << Poco::Thread::currentTid() << " get message: " << msg << endl;
        }

        Poco::Thread::sleep(2000);

        memset(msg, 0, sizeof(msg));
        strcpy(msg, "bye");
        m_socket.sendBytes(msg, strlen(msg) + 1);
    }

private:
    Net::StreamSocket m_socket;
    LfReactor::ThreadManager* m_thMgr;
};

class TestClient : public Poco::Runnable
{
public:
    TestClient(const int no, const Net::SocketAddress& address) :
    m_testNo(no), m_address(address)
    {
    }

    ~TestClient()
    {
    }

    void run()
    {
        Net::StreamSocket socket;
        socket.connect(m_address);

        char msg[100];
        memset(msg, 0, sizeof(msg));
        sprintf(msg, "No. %d tester say hello to you!", m_testNo);
        socket.sendBytes(msg, strlen(msg) + 1);

        memset(msg, 0, sizeof(msg));
        socket.receiveBytes(msg, sizeof(msg));
        Mutex::ScopedLock lock(consoleMutex);
        cout << "No." << m_testNo << " tester receive replay message: " << msg << endl;
    }

private:
    int m_testNo;
    Net::SocketAddress m_address;
};

int main()
{
    LfReactor::ThreadManager thrMgr;
    Net::SocketAddress ssa;
    Net::ServerSocket socket(ssa);
    LfReactor::SockAcceptor<ServiceHandler> acceptor(socket, thrMgr);
    LfReactor::LfThread thr1(&thrMgr);
    LfReactor::LfThread thr2(&thrMgr);
    LfReactor::LfThread thr3(&thrMgr);
    LfReactor::LfThread thr4(&thrMgr);

    ThreadPool thrPool(4);
    thrPool.start(thr1);
    thrPool.start(thr2);
    thrPool.start(thr3);
    thrPool.start(thr4);

    TestClient test1(1, Net::SocketAddress("localhost", socket.address().port()));
    TestClient test2(2, Net::SocketAddress("localhost", socket.address().port()));
    TestClient test3(3, Net::SocketAddress("localhost", socket.address().port()));
    TestClient test4(4, Net::SocketAddress("localhost", socket.address().port()));
    TestClient test5(5, Net::SocketAddress("localhost", socket.address().port()));
    TestClient test6(6, Net::SocketAddress("localhost", socket.address().port()));
    TestClient test7(7, Net::SocketAddress("localhost", socket.address().port()));
    TestClient test8(8, Net::SocketAddress("localhost", socket.address().port()));

    ThreadPool testPool(8);
    testPool.start(test1);
    testPool.start(test2);
    testPool.start(test3);
    testPool.start(test4);
    testPool.start(test5);
    testPool.start(test6);
    testPool.start(test7);
    testPool.start(test8);

    testPool.joinAll();

    thrMgr.stopAll();
    thrPool.joinAll();

    return 0;
}
