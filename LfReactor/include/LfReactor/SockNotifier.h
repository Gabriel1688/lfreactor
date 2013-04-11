//this class use to notify sockets' event
//mlcai 2013-03-11

#ifndef SOCKNOTIFIER_H
#define SOCKNOTIFIER_H

#include "NameSpaceDefine.h"

#include "Poco/Net/Net.h"
#include "Poco/Net/Socket.h"
#include "Poco/RefCountedObject.h"
#include "Poco/NotificationCenter.h"
#include "Poco/Observer.h"
#include <set>

BEGIN_CXX_NAMESPACE_DEFINITION

class SockReactor;
class SockNotification;

class DYNAMIC_LIB SockNotifier: public Poco::RefCountedObject
{
public:
	explicit SockNotifier(const Poco::Net::Socket& sock);

	void addObserver(SockReactor* pReactor, const Poco::AbstractObserver& observer);

	void removeObserver(SockReactor* pReactor, const Poco::AbstractObserver& observer);

	bool accepts(SockNotification* pNotification);

	void dispatch(SockNotification* pNotification);

	bool hasObservers() const;

    std::size_t countObservers() const;

    bool hasObserver(const Poco::AbstractObserver& observer) const;

    void suspendObserver(SockReactor* pReactor, const Poco::AbstractObserver& observer);
    void resumeObserver(SockReactor* pReactor, const Poco::AbstractObserver& observer);

protected:
	~SockNotifier();

	SockNotification* clone(SockNotification* pNotification);

private:
	std::multiset<SockNotification*> m_notifications;
	Poco::NotificationCenter m_notifiCenter;
	Poco::Net::Socket  m_socket;

	typedef Poco::SharedPtr<Poco::AbstractObserver> AbstractObserverPtr;
	typedef std::vector<AbstractObserverPtr> ObserverList;

	ObserverList  m_observers; //use to record observers
	mutable Poco::Mutex m_obsMutex;
};

END_CXX_NAMESPACE_DEFINITION

#endif
