//implement LfEventObserver
//andycai.sc@gmail.com 2013-03-20

#include "NameSpaceDefine.h"
#include ADD_QUOTE(INC_NAME_HEADER(NAMESPACE_NAME, LfEventObserver.h))

BEGIN_CXX_NAMESPACE_DEFINITION

LfEventObserver::LfEventObserver(const Poco::Net::Socket& sock, const Poco::AbstractObserver& observer,
ThreadManager& thrMgr) : m_socket(sock), m_observer(observer.clone()), m_thrManager(thrMgr)
{
}

LfEventObserver::LfEventObserver(const LfEventObserver& lfObs) : m_socket(lfObs.m_socket),
m_observer(lfObs.m_observer), m_thrManager(lfObs.m_thrManager)
{
}

LfEventObserver::~LfEventObserver()
{
}

void LfEventObserver::notify(Poco::Notification* pNf) const
{
    m_thrManager.suspendEventHandler(m_socket, *this);
    m_thrManager.promoteNewLeader();

    m_observer->notify(pNf);

    m_thrManager.resumeEventHandler(m_socket, *this);
}

bool LfEventObserver::equals(const Poco::AbstractObserver& observer) const
{
    const LfEventObserver* lfObs = dynamic_cast<const LfEventObserver*>(&observer);
    return lfObs && m_observer->equals(*(lfObs->m_observer));
}

bool LfEventObserver::accepts(Poco::Notification* pNf) const
{
    return m_observer->accepts(pNf);
}

Poco::AbstractObserver* LfEventObserver::clone() const
{
    return new LfEventObserver(*this);
}

END_CXX_NAMESPACE_DEFINITION
