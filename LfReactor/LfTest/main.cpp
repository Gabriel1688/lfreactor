//this is lf test code, it's show how to use the library
//andycai.sc@gmail.com

#include "Poco/ThreadPool.h"
#include "Poco/Net/StreamSocket.h"
#include "LfReactor/SockAcceptor.h"
#include "LfReactor/SockConnector.h"

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
        m_thMgr->addEventHandler(m_socket, Poco::Observer<ServiceHandler, LfReactor::ReadableNotification>(*this, &ServiceHandler::onReadable));
    }

    ~ServiceHandler()
    {
        m_thMgr->removeEventHandler(m_socket, Poco::Observer<ServiceHandler, LfReactor::ReadableNotification>(*this, &ServiceHandler::onReadable));
    }

    void onReadable(LfReactor::ReadableNotification* pNotification)
    {
        pNotification->release();

        Mutex::ScopedLock lock(consoleMutex);
        printf("thread %lu with servid %lu service handler onReadable\n", Poco::Thread::currentTid(), (unsigned long)this);

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
            printf("thread %lu get message: %s\n", Poco::Thread::currentTid(), msg);
        }

        //Poco::Thread::sleep(1000);

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

        int i = 3;
        do
        {
            char msg[100];
            memset(msg, 0, sizeof(msg));
            sprintf(msg, "No. %d tester %d times say hello to you!", m_testNo, i);
            socket.sendBytes(msg, strlen(msg) + 1);

            memset(msg, 0, sizeof(msg));
            socket.receiveBytes(msg, sizeof(msg));
            //Mutex::ScopedLock lock(consoleMutex);
            //cout << "No." << m_testNo << " tester receive replay message: " << msg << endl;
            --i;
        } while (i > 0);
    }

private:
    int m_testNo;
    Net::SocketAddress m_address;
};

class ClientHandler
{
public:
    ClientHandler(Net::StreamSocket& socket, LfReactor::ThreadManager& thMgr) :
        m_socket(socket), m_thMgr(&thMgr)
    {
        m_thMgr->addEventHandler(m_socket, Poco::Observer<ClientHandler, LfReactor::ReadableNotification>(*this, &ClientHandler::onReadable));
        m_thMgr->addEventHandler(m_socket, Poco::Observer<ClientHandler, LfReactor::WritableNotification>(*this, &ClientHandler::onWriteable));
        Mutex::ScopedLock lock(consoleMutex);
        printf("thread %lu create client handler %lu\n", Poco::Thread::currentTid(), (unsigned long)this);
    }

    ~ClientHandler()
    {
        m_thMgr->removeEventHandler(m_socket, Poco::Observer<ClientHandler, LfReactor::ReadableNotification>(*this, &ClientHandler::onReadable));
    }

    void onWriteable(LfReactor::WritableNotification* pNotification)
    {
        pNotification->release();

        Mutex::ScopedLock lock(consoleMutex);
        printf("thread %lu with clientid %lu client handler onWriteable\n", Poco::Thread::currentTid(), (unsigned long)this);

        char msg[100];
        memset(msg, 0, sizeof(msg));
        strcpy(msg, "client handler connect to you!");
        m_socket.sendBytes(msg, strlen(msg) + 1);
        m_thMgr->removeEventHandler(m_socket, Poco::Observer<ClientHandler, LfReactor::WritableNotification>(*this, &ClientHandler::onWriteable));
    }

    void onReadable(LfReactor::ReadableNotification* pNotification)
    {
        pNotification->release();

        char msg[100];
        memset(msg, 0, sizeof(msg));

        int n = m_socket.receiveBytes(msg, sizeof(msg));

        if (n <= 0)
        {
            Mutex::ScopedLock lock(consoleMutex);
            cout << "thread " << Poco::Thread::currentTid() << " delete clieid " << (unsigned long)this << " client handler" << endl;
            delete this;
            return;
        }

        {
            Mutex::ScopedLock lock(consoleMutex);
            cout << "thread " << Poco::Thread::currentTid() << " client handler get message: " << msg << endl;
        }

        m_socket.shutdownSend();
    }

private:
    Net::StreamSocket m_socket;
    LfReactor::ThreadManager* m_thMgr;
};

int main()
{
    LfReactor::SockReactor reactor;
    LfReactor::ThreadManager thrMgr(&reactor);
    Net::SocketAddress ssa;
    Net::ServerSocket socket(ssa);
    Net::SocketAddress addr("localhost", socket.address().port());
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

    LfReactor::ThreadManager cliThrMgr;
    LfReactor::SockConnector<ClientHandler> connector1(addr, cliThrMgr);
    LfReactor::SockConnector<ClientHandler> connector2(addr, cliThrMgr);
    LfReactor::SockConnector<ClientHandler> connector3(addr, cliThrMgr);
    LfReactor::SockConnector<ClientHandler> connector4(addr, cliThrMgr);
    LfReactor::LfThread cliThr1(&cliThrMgr);
    LfReactor::LfThread cliThr2(&cliThrMgr);
    LfReactor::LfThread cliThr3(&cliThrMgr);
    LfReactor::LfThread cliThr4(&cliThrMgr);

    ThreadPool cliThrPool(4);
    cliThrPool.start(cliThr1);
    cliThrPool.start(cliThr2);
    cliThrPool.start(cliThr3);
    cliThrPool.start(cliThr4);

    TestClient test1(1, addr);
    TestClient test2(2, addr);
    TestClient test3(3, addr);
    TestClient test4(4, addr);
    TestClient test5(5, addr);
    TestClient test6(6, addr);
    TestClient test7(7, addr);
    TestClient test8(8, addr);

    /*ThreadPool testPool(8);
    testPool.start(test1);
    testPool.start(test2);
    testPool.start(test3);
    testPool.start(test4);
    testPool.start(test5);
    testPool.start(test6);
    testPool.start(test7);
    testPool.start(test8);

    testPool.joinAll();*/

    //cliThrMgr.stopAll();
    cliThrPool.joinAll();

    thrMgr.stopAll();
    thrPool.joinAll();

    cout << "services end" << endl;

    return 0;
}
