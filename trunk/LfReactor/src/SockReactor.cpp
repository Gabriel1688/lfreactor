// implement SockReactor
// andycai.sc@gmail.com 2013-03-07
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

#include <cstdio>
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

void SockReactor::addEventHandler(const Poco::Net::Socket& socket, const Poco::AbstractObserver& observer)
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
    try
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
            Poco::Net::Socket socket(m_readableSockets.front());
            m_readableSockets.erase(m_readableSockets.begin());
            dispatch(socket, m_pReadableNotification);
        }
        else if (m_writeableSockets.size() != 0)
        {
            Poco::Net::Socket socket(m_writeableSockets.front());
            m_writeableSockets.erase(m_writeableSockets.begin());
            dispatch(socket, m_pWritableNotification);
        }
        else if (m_exceptSockets.size() != 0)
        {
            Poco::Net::Socket socket(m_exceptSockets.front());
            m_exceptSockets.erase(m_exceptSockets.begin());
            dispatch(socket, m_pErrorNotification);
        }
    }
    catch (Poco::Exception& exc)
    {
        m_readableSockets.clear();
        m_writeableSockets.clear();
        m_exceptSockets.clear();
        Poco::ErrorHandler::handle(exc);
        return;
    }
    catch (std::exception& exc)
    {
        m_readableSockets.clear();
        m_writeableSockets.clear();
        m_exceptSockets.clear();
        Poco::ErrorHandler::handle(exc);
        return;
    }
	catch (...)
    {
        m_readableSockets.clear();
        m_writeableSockets.clear();
        m_exceptSockets.clear();
        Poco::ErrorHandler::handle();
        return;
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
	{
		Poco::FastMutex::ScopedLock lock(m_eventHMutex);
		delegates.reserve(m_eventHandlers.size());
		for (EventHandlerMap::iterator it = m_eventHandlers.begin(); it != m_eventHandlers.end(); ++it)
		{
		    if (it->second->accepts(pNotification))
                delegates.push_back(it->second);
		}
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