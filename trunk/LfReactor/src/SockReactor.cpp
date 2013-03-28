//implement SockReactor
//mlcai 2013-03-07

#include "NameSpaceDefine.h"
#include ADD_QUOTE(INC_NAME_HEADER(NAMESPACE_NAME, SockReactor.h))
#include ADD_QUOTE(INC_NAME_HEADER(NAMESPACE_NAME, SockNotification.h))
#include ADD_QUOTE(INC_NAME_HEADER(NAMESPACE_NAME, SockNotifier.h))

#include "Poco/ErrorHandler.h"
#include "Poco/Exception.h"

BEGIN_CXX_NAMESPACE_DEFINITION

SockReactor SockReactor::_sockReactor;

SockReactor::SockReactor() : m_timeout(DEFAULT_TIMEOUT),
m_pReadableNotification(new ReadableNotification(this)),
m_pWritableNotification(new WritableNotification(this)),
m_pErrorNotification(new ErrorNotification(this)),
m_pTimeoutNotification(new TimeoutNotification(this)),
m_pIdleNotification(new IdleNotification(this)),
m_pShutdownNotification(new ShutdownNotification(this))
{
}

SockReactor::SockReactor(const Poco::Timespan& timeout) : m_timeout(timeout),
m_pReadableNotification(new ReadableNotification(this)),
m_pWritableNotification(new WritableNotification(this)),
m_pErrorNotification(new ErrorNotification(this)),
m_pTimeoutNotification(new TimeoutNotification(this)),
m_pIdleNotification(new IdleNotification(this)),
m_pShutdownNotification(new ShutdownNotification(this))
{
}

SockReactor::~SockReactor()
{
}

void SockReactor::setTimeout(const Poco::Timespan& timeout)
{
    m_timeout = timeout;
}

const Poco::Timespan& SockReactor::getTimeout() const
{
    return m_timeout;
}

void SockReactor::registerEventHandler(const Poco::Net::Socket& socket, const Poco::AbstractObserver& observer)
{
    NotifierPtr pNotifier;
	{
		Poco::FastMutex::ScopedLock lock(m_eventHMutex);

		EventHandlerMap::iterator it = m_eventHandlers.find(socket);
		if (it == m_eventHandlers.end())
		{
			pNotifier = new SockNotifier(socket);
			m_eventHandlers.insert(EventHandlerMap::value_type(socket, pNotifier));
		}
		else
            pNotifier = it->second;
	}
	pNotifier->addObserver(this, observer);
}

void SockReactor::removeEventHandler(const Poco::Net::Socket& socket, const Poco::AbstractObserver& observer)
{
	NotifierPtr pNotifier;
	{
		Poco::FastMutex::ScopedLock lock(m_eventHMutex);

		EventHandlerMap::iterator it = m_eventHandlers.find(socket);
		if (it != m_eventHandlers.end())
		{
			pNotifier = it->second;
			if (pNotifier->countObservers() == 1)
			{
				m_eventHandlers.erase(it);
			}
		}
	}
	if (pNotifier)
	{
		pNotifier->removeObserver(this, observer);
	}
}

void SockReactor::handleEvents()
{
    if (m_readableSockets.empty() && m_writeableSockets.empty() && m_exceptSockets.empty())
    {
        int nSockets = 0;
        {
            Poco::FastMutex::ScopedLock lock(m_eventHMutex);
            for (EventHandlerMap::iterator it = m_eventHandlers.begin(); it != m_eventHandlers.end(); ++it)
            {
                if (it->second->accepts(m_pReadableNotification))
                {
                    m_readableSockets.push_back(it->first);
                    nSockets++;
                }
                if (it->second->accepts(m_pWritableNotification))
                {
                    m_writeableSockets.push_back(it->first);
                    nSockets++;
                }
                if (it->second->accepts(m_pErrorNotification))
                {
                    m_exceptSockets.push_back(it->first);
                    nSockets++;
                }
            }
        }
        if (nSockets == 0)
        {
            onIdle();
        }
        else if (!Poco::Net::Socket::select(m_readableSockets, m_writeableSockets, m_exceptSockets, m_timeout))
        {
            onTimeout();
            m_readableSockets.clear();
            m_writeableSockets.clear();
            m_exceptSockets.clear();
            return;
        }
    }

    if (m_readableSockets.size() != 0)
    {
        dispatch(m_readableSockets.front(), m_pReadableNotification);
        m_readableSockets.erase(m_readableSockets.begin());
    }

    if (m_writeableSockets.size() != 0)
    {
        dispatch(m_writeableSockets.front(), m_pWritableNotification);
        m_writeableSockets.erase(m_writeableSockets.begin());
    }

    if (m_exceptSockets.size() != 0)
    {
        dispatch(m_exceptSockets.front(), m_pErrorNotification);
        m_exceptSockets.erase(m_exceptSockets.begin());
    }
}

SockReactor& SockReactor::instance()
{
    return _sockReactor;
}

void SockReactor::onTimeout()
{
    dispatch(m_pTimeoutNotification);
}

void SockReactor::onIdle()
{
	dispatch(m_pIdleNotification);
}


void SockReactor::onShutdown()
{
	dispatch(m_pShutdownNotification);
}

void SockReactor::dispatch(const Poco::Net::Socket& socket, SockNotification* pNotification)
{
    NotifierPtr pNotifier;
    {
        Poco::FastMutex::ScopedLock locker(m_eventHMutex);
        EventHandlerMap::iterator it = m_eventHandlers.find(socket);
		if (it != m_eventHandlers.end())
			pNotifier = it->second;
		else
			return;
    }
    pNotifier->dispatch(pNotification);
}

void SockReactor::dispatch(SockNotification* pNotification)
{
    std::vector<NotifierPtr> delegates;
	delegates.reserve(m_eventHandlers.size());
	{
		Poco::FastMutex::ScopedLock lock(m_eventHMutex);
		for (EventHandlerMap::iterator it = m_eventHandlers.begin(); it != m_eventHandlers.end(); ++it)
			delegates.push_back(it->second);
	}
	for (std::vector<NotifierPtr>::iterator it = delegates.begin(); it != delegates.end(); ++it)
	{
		(*it)->dispatch(pNotification);
	}
}

void SockReactor::suspendEventHandler(const Poco::Net::Socket& socket, const Poco::AbstractObserver& observer)
{
    NotifierPtr pNotifier;
	{
		Poco::FastMutex::ScopedLock lock(m_eventHMutex);

		EventHandlerMap::iterator it = m_eventHandlers.find(socket);
		if (it != m_eventHandlers.end())
		{
			pNotifier = it->second;
			pNotifier->suspendObserver(this, observer);
		}
	}
}

void SockReactor::resumeEventHandler(const Poco::Net::Socket& socket, const Poco::AbstractObserver& observer)
{
    NotifierPtr pNotifier;
	{
		Poco::FastMutex::ScopedLock lock(m_eventHMutex);

		EventHandlerMap::iterator it = m_eventHandlers.find(socket);
		if (it != m_eventHandlers.end())
		{
			pNotifier = it->second;
			if (pNotifier->hasObserver(observer))
                pNotifier->resumeObserver(this, observer);
		}
	}
}

END_CXX_NAMESPACE_DEFINITION