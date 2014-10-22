
// This file describes a central switchboard for notifications that might
// happen in various parts of the application, and allows users to register
// observers for various classes of events that they're interested in.

#ifndef __notification_service_h__
#define __notification_service_h__

#pragma once

#include <map>

#include "base/observer_list.h"

#include "notification_details.h"
#include "notification_source.h"

class NotificationObserver;

class NotificationService
{
public:
    // Returns the NotificationService object for the current thread, or NULL if
    // none.
    static NotificationService* current();

    // Normally instantiated when the thread is created.  Not all threads have
    // a NotificationService.  Only one instance should be created per thread.
    NotificationService();
    ~NotificationService();

    // Synchronously posts a notification to all interested observers.
    // Source is a reference to a NotificationSource object representing
    // the object originating the notification (can be
    // NotificationService::AllSources(), in which case
    // only observers interested in all sources will be notified).
    // Details is a reference to an object containing additional data about
    // the notification.  If no additional data is needed, NoDetails() is used.
    // There is no particular order in which the observers will be notified.
    void Notify(int type,
        const NotificationSource& source,
        const NotificationDetails& details);

    // Returns a NotificationSource that represents all notification sources
    // (for the purpose of registering an observer for events from all sources).
    static Source<void> AllSources() { return Source<void>(NULL); }

    // Returns the same value as AllSources(). This function has semantic
    // differences to the programmer: We have checked that this AllSources()
    // usage is safe in the face of multiple profiles. Objects that were
    // singletons now will always have multiple instances, one per browser
    // context.
    //
    // Some usage is safe, where the Source is checked to see if it's a member of
    // a container before use. But, we want the number of AllSources() calls to
    // drop to almost nothing, because most usages are not multiprofile safe and
    // were done because it was easier to listen to everything.
    static Source<void> AllBrowserContextsAndSources()
    {
        return Source<void>(NULL);
    }

    // Returns a NotificationDetails object that represents a lack of details
    // associated with a notification.  (This is effectively a null pointer.)
    static Details<void> NoDetails() { return Details<void>(NULL); }

private:
    friend class NotificationRegistrar;

    typedef ObserverList<NotificationObserver> NotificationObserverList;
    typedef std::map<uintptr_t, NotificationObserverList*> NotificationSourceMap;
    typedef std::map<int, NotificationSourceMap> NotificationObserverMap;
    typedef std::map<int, int> NotificationObserverCount;

    // Convenience function to determine whether a source has a
    // NotificationObserverList in the given map;
    static bool HasKey(const NotificationSourceMap& map,
        const NotificationSource& source);

    // NOTE: Rather than using this directly, you should use a
    // NotificationRegistrar.
    //
    // Registers a NotificationObserver to be called whenever a matching
    // notification is posted.  Observer is a pointer to an object subclassing
    // NotificationObserver to be notified when an event matching the other two
    // parameters is posted to this service.  Type is the type of events to be
    // notified about (or content::NOTIFICATION_ALL to receive events of all
    // types).
    // Source is a NotificationSource object (created using
    // "Source<classname>(pointer)"), if this observer only wants to
    // receive events from that object, or NotificationService::AllSources()
    // to receive events from all sources.
    //
    // A given observer can be registered only once for each combination of
    // type and source.  If the same object is registered more than once,
    // it must be removed for each of those combinations of type and source later.
    //
    // The caller retains ownership of the object pointed to by observer.
    void AddObserver(NotificationObserver* observer,
        int type, const NotificationSource& source);

    // NOTE: Rather than using this directly, you should use a
    // NotificationRegistrar.
    //
    // Removes the object pointed to by observer from receiving notifications
    // that match type and source.  If no object matching the parameters is
    // currently registered, this method is a no-op.
    void RemoveObserver(NotificationObserver* observer,
        int type, const NotificationSource& source);

    // Keeps track of the observers for each type of notification.
    // Until we get a prohibitively large number of notification types,
    // a simple array is probably the fastest way to dispatch.
    NotificationObserverMap observers_;

#ifndef NDEBUG
    // Used to check to see that AddObserver and RemoveObserver calls are
    // balanced.
    NotificationObserverCount observer_counts_;
#endif

    DISALLOW_COPY_AND_ASSIGN(NotificationService);
};

#endif //__notification_service_h__