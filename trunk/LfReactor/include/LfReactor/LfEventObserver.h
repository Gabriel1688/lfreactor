//this class use to handle lf event then notify the real observer
//andycai.sc@gmail.com 2013-03-20

#ifndef LFEVENTOBSERVER_H_INCLUDED
#define LFEVENTOBSERVER_H_INCLUDED

#include "Poco/AbstractObserver.h"
#include "Poco/SharedPtr.h"
#include "Poco/Net/Socket.h"
#include "NameSpaceDefine.h"
#include ADD_QUOTE(INC_NAME_HEADER(NAMESPACE_NAME, ThreadManager.h))

BEGIN_CXX_NAMESPACE_DEFINITION

class DYNAMIC_LIB LfEventObserver : public Poco::AbstractObserver
{
public:
    LfEventObserver(const Poco::Net::Socket& sock, const Poco::AbstractObserver& observer,
                    ThreadManager& thrMgr);
    LfEventObserver(const LfEventObserver& lfObs);
    ~LfEventObserver();

    void notify(Poco::Notification* pNf) const;

    bool equals(const Poco::AbstractObserver& observer) const;

    bool accepts(Poco::Notification* pNf) const;

    Poco::AbstractObserver* clone() const;

private:
    Poco::Net::Socket m_socket;
    Poco::SharedPtr<Poco::AbstractObserver> m_observer;
    ThreadManager& m_thrManager;
};

END_CXX_NAMESPACE_DEFINITION

#endif // LFEVENTOBSERVER_H_INCLUDED
