//implement SockNotifier
//mlcai 2013-03-18

#include "NameSpaceDefine.h"
#include ADD_QUOTE(INC_NAME_HEADER(NAMESPACE_NAME, SockNotifier.h))
#include ADD_QUOTE(INC_NAME_HEADER(NAMESPACE_NAME, SockReactor.h))
#include ADD_QUOTE(INC_NAME_HEADER(NAMESPACE_NAME, SockNotification.h))

BEGIN_CXX_NAMESPACE_DEFINITION

SockNotifier::SockNotifier(const Poco::Net::Socket& socket):
	m_socket(socket)
{
}

SockNotifier::~SockNotifier()
{
}

void SockNotifier::addObserver(SockReactor* pReactor, const Poco::AbstractObserver& observer)
{
	m_notifiCenter.addObserver(observer);
	{
	    Poco::Mutex::ScopedLock locker(m_obsMutex);
	    m_observers.push_back(observer.clone());
	}

    resumeObserver(pReactor, observer);
}

void SockNotifier::removeObserver(SockReactor* pReactor, const Poco::AbstractObserver& observer)
{
	m_notifiCenter.removeObserver(observer);
	{
	    Poco::Mutex::ScopedLock lock(m_obsMutex);
        for (ObserverList::iterator it = m_observers.begin(); it != m_observers.end(); ++it)
        {
            if (observer.equals(**it))
            {
                m_observers.erase(it);
                break;
            }
        }
	}

	suspendObserver(pReactor, observer);
}

void SockNotifier::dispatch(SockNotification* pNotification)
{
	static Poco::Net::Socket nullSocket;

	Poco::AutoPtr<SockNotification> pNf = clone(pNotification);

	pNf->setSocket(m_socket);
	pNf->duplicate();
	try
	{
		m_notifiCenter.postNotification(pNf);
	}
	catch (...)
	{
		pNf->setSocket(nullSocket);
		throw;
	}
	pNf->setSocket(nullSocket);
}

bool SockNotifier::accepts(SockNotification* pNotification)
{
	return m_notifications.find(pNotification) != m_notifications.end();
}

void SockNotifier::suspendObserver(SockReactor* pReactor, const Poco::AbstractObserver& observer)
{
    std::multiset<SockNotification*>::iterator it = m_notifications.end();
    if (observer.accepts(pReactor->m_pReadableNotification))
		it = m_notifications.find(pReactor->m_pReadableNotification.get());
	else if (observer.accepts(pReactor->m_pWritableNotification))
		it = m_notifications.find(pReactor->m_pWritableNotification.get());
	else if (observer.accepts(pReactor->m_pErrorNotification))
		it = m_notifications.find(pReactor->m_pErrorNotification.get());
	else if (observer.accepts(pReactor->m_pTimeoutNotification))
		it = m_notifications.find(pReactor->m_pTimeoutNotification.get());
	if (it != m_notifications.end())
		m_notifications.erase(it);
}

void SockNotifier::resumeObserver(SockReactor* pReactor, const Poco::AbstractObserver& observer)
{
    if (observer.accepts(pReactor->m_pReadableNotification))
		m_notifications.insert(pReactor->m_pReadableNotification.get());
	else if (observer.accepts(pReactor->m_pWritableNotification))
		m_notifications.insert(pReactor->m_pWritableNotification.get());
	else if (observer.accepts(pReactor->m_pErrorNotification))
		m_notifications.insert(pReactor->m_pErrorNotification.get());
	else if (observer.accepts(pReactor->m_pTimeoutNotification))
		m_notifications.insert(pReactor->m_pTimeoutNotification.get());
}

bool SockNotifier::hasObservers() const
{
	return m_notifiCenter.hasObservers();
}

std::size_t SockNotifier::countObservers() const
{
	return m_notifiCenter.countObservers();
}

bool SockNotifier::hasObserver(const Poco::AbstractObserver& observer) const
{
    Poco::Mutex::ScopedLock lock(m_obsMutex);
    for (ObserverList::const_iterator it = m_observers.begin(); it != m_observers.end(); ++it)
    {
        if (observer.equals(**it))
        {
            return true;
        }
    }

    return false;
}

SockNotification* SockNotifier::clone(SockNotification* pNotification)
{
    SockNotification* pNf;
    if (dynamic_cast<ReadableNotification*>(pNotification))
    {
        pNf = new ReadableNotification(&pNotification->source());
    }
    else if (dynamic_cast<WritableNotification*>(pNotification))
    {
        pNf = new WritableNotification(&pNotification->source());
    }
    else if (dynamic_cast<ErrorNotification*>(pNotification))
    {
        pNf = new ErrorNotification(&pNotification->source());
    }
    else if (dynamic_cast<TimeoutNotification*>(pNotification))
    {
        pNf = new TimeoutNotification(&pNotification->source());
    }
    else if (dynamic_cast<IdleNotification*>(pNotification))
    {
        pNf = new IdleNotification(&pNotification->source());
    }
    else
    {
        pNf = new ShutdownNotification(&pNotification->source());
    }

    return pNf;
}

END_CXX_NAMESPACE_DEFINITION
