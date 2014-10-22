
#ifndef __base_observer_list_threadsafe_h__
#define __base_observer_list_threadsafe_h__

#pragma once

#include <algorithm>
#include <map>

#include "callback_old.h"
#include "message_loop.h"
#include "message_loop_proxy.h"
#include "observer_list.h"
#include "task.h"

///////////////////////////////////////////////////////////////////////////////
//
// ����:
//
// �̰߳�ȫ�Ĺ۲�������. ��observer_list����(��observer_list.h), ���ڶ��߳�
// �����¸���׳.
//
// ֧����������:
//   * �۲��߿����������̵߳�֪ͨ��Ϣ��ע��. �ص��۲��߷����ڹ۲��߳�ʼ����
//     AddObserver()���߳�.
//   * �����߳̿���ͨ��Notify()����֪ͨ��Ϣ.
//   * �۲��߿����ڻص�ʱ���б����Ƴ��Լ�.
//   * ���һ���߳�����֪ͨ�۲���, ��ʱһ���۲������ڴ��б����Ƴ��Լ�, ֪ͨ
//     �ᱻ����.
//
// �̰߳�ȫ�Ĺ۲��߶��бȷ��̰߳�ȫ�汾�Ĳ������֪ͨ��Ϣ��ʵʱ�Բ�һЩ, ͨ
// ֪��Ϣ����ͨ��PostTask()Ͷ�ݵ������߳�, ���̰߳�ȫ�汾��֪ͨ��Ϣ��ͬ����
// ����.
//
// ʵ��˵��:
// ObserverListThreadSafeΪÿ���߳�ά��һ��ObserverList. ֪ͨ�۲���ʱ, ֻ�Ǽ�
// �ĵ���Ϊÿ��ע����̵߳���PostTask, ÿ���߳̽�֪ͨ�Լ���ObserverList.
//
///////////////////////////////////////////////////////////////////////////////

// ObserverListThreadSafeǰ������, ΪObserverListThreadSafeTraits.
template<class ObserverType>
class ObserverListThreadSafe;

// ���ڽ��VS2005��֧��:
// friend class
//     base::RefCountedThreadSafe<ObserverListThreadSafe<ObserverType> >;
// ������������Ԫ, ���԰�ʵ�ʵ�ɾ����������Ϊ��Ԫ. ��Ϊ
// RefCountedThreadSafe::DeleteInternal()��private��, ������Ҫ�����Լ���
// ģ��������.
template<class T>
struct ObserverListThreadSafeTraits
{
    static void Destruct(const ObserverListThreadSafe<T>* x)
    {
        delete x;
    }
};

template<class ObserverType>
class ObserverListThreadSafe
    : public base::RefCountedThreadSafe<
    ObserverListThreadSafe<ObserverType>,
    ObserverListThreadSafeTraits<ObserverType> >
{
public:
    typedef typename ObserverList<ObserverType>::NotificationType
        NotificationType;

    ObserverListThreadSafe()
        : type_(ObserverListBase<ObserverType>::NOTIFY_ALL) {}
    explicit ObserverListThreadSafe(NotificationType type) : type_(type) {}

    // ���һ���۲��ߵ��б�.
    void AddObserver(ObserverType* obs)
    {
        ObserverList<ObserverType>* list = NULL;
        MessageLoop* loop = MessageLoop::current();
        {
            base::AutoLock lock(list_lock_);
            if(observer_lists_.find(loop) == observer_lists_.end())
            {
                observer_lists_[loop] = new ObserverListContext(type_);
            }
            list = &(observer_lists_[loop]->list);
        }
        list->AddObserver(obs);
    }

    // ���б����Ƴ�һ���۲���.
    // ����з����۲��ߵ�֪ͨ��Ϣ, ���ǽ��ᱻ��ֹ.
    // RemoveObserver�����ڵ���AddObserver����ͬ�߳��б�����.
    void RemoveObserver(ObserverType* obs)
    {
        ObserverListContext* context = NULL;
        ObserverList<ObserverType>* list = NULL;
        MessageLoop* loop = MessageLoop::current();
        if(!loop)
        {
            return; // �ر�ʱ, current()�����Ѿ�Ϊ��.
        }
        
        {
            base::AutoLock lock(list_lock_);
            typename ObserversListMap::iterator it = observer_lists_.find(loop);
            if(it == observer_lists_.end())
            {
                // This will happen if we try to remove an observer on a thread
                // we never added an observer for.
                return;
            }
            context = it->second;
            list = &context->list;

            // If we're about to remove the last observer from the list,
            // then we can remove this observer_list entirely.
            if(list->HasObserver(obs) && list->size()==1)
            {
                observer_lists_.erase(it);
            }
        }
        list->RemoveObserver(obs);

        // ���RemoveObserver�ĵ�������һ��֪ͨ��Ϣ, �б�Ĵ�С�Ƿ����.
        // ���ﲻ��ɾ��, ������NotifyWrapper��ɱ����Ժ�ɾ��.
        if(list->size() == 0)
        {
            delete context;
        }
    }

    // ֪ͨ����.
    // �̰߳�ȫ�Ļص����б��е�ÿ���۲���.
    // ע��, ��Щ���ö����첽��. �㲻�ܼ���Notify�������ʱ���еĹ۲���
    // ���Ѿ���֪ͨ, ��Ϊ֪ͨ��������Ͷ����.
    template<class Method>
    void Notify(Method m)
    {
        UnboundMethod<ObserverType, Method, Tuple0> method(m, MakeTuple());
        Notify<Method, Tuple0>(method);
    }

    template<class Method, class A>
    void Notify(Method m, const A &a)
    {
        UnboundMethod<ObserverType, Method, Tuple1<A> > method(m, MakeTuple(a));
        Notify<Method, Tuple1<A> >(method);
    }

private:
    friend struct ObserverListThreadSafeTraits<ObserverType>;

    struct ObserverListContext
    {
        explicit ObserverListContext(NotificationType type)
            : loop(base::MessageLoopProxy::current()),
            list(type) {}

        scoped_refptr<base::MessageLoopProxy> loop;
        ObserverList<ObserverType> list;

        DISALLOW_COPY_AND_ASSIGN(ObserverListContext);
    };

    ~ObserverListThreadSafe()
    {
        typename ObserversListMap::const_iterator it;
        for(it=observer_lists_.begin(); it!=observer_lists_.end(); ++it)
        {
            delete (*it).second;
        }
        observer_lists_.clear();
    }

    template<class Method, class Params>
    void Notify(const UnboundMethod<ObserverType, Method, Params>& method)
    {
        base::AutoLock lock(list_lock_);
        typename ObserversListMap::iterator it;
        for(it=observer_lists_.begin(); it!=observer_lists_.end(); ++it)
        {
            ObserverListContext* context = (*it).second;
            context->loop->PostTask(
                NewRunnableMethod(this,
                &ObserverListThreadSafe<ObserverType>::
                template NotifyWrapper<Method, Params>, context, method));
        }
    }

    // Ϊÿ���̵߳�ObserverList����֪ͨ��Ϣ�ĵ��÷�װ. �������ñ����ڷǰ�ȫ
    // ObserverList�������߳��б�����.
    template<class Method, class Params>
    void NotifyWrapper(ObserverListContext* context,
        const UnboundMethod<ObserverType, Method, Params>& method)
    {

        // ����б��Ƿ���Ҫ֪ͨ.
        {
            base::AutoLock lock(list_lock_);
            typename ObserversListMap::iterator it =
                observer_lists_.find(MessageLoop::current());

            // ObserverList�����ѱ��Ƴ�. ʵ�����п������Ƴ�����������ӵ�! ���
            // ���б�ѭ����������Щ���, ����û��Ҫ������֪ͨ.
            if(it==observer_lists_.end() || it->second!=context)
            {
                return;
            }
        }

        {
            typename ObserverList<ObserverType>::Iterator it(context->list);
            ObserverType* obs;
            while((obs=it.GetNext()) != NULL)
            {
                method.Run(obs);
            }
        }

        // ����б���û�й۲���, ����ɾ����.
        if(context->list.size() == 0)
        {
            {
                base::AutoLock lock(list_lock_);
                // �Ƴ�|list|, ���������. �������۲�����һ��֪ͨ��Ϣ��ͬʱ�Ƴ�
                // ʱ�ᷢ���������. ��http://crbug.com/55725.
                typename ObserversListMap::iterator it =
                    observer_lists_.find(MessageLoop::current());
                if(it!=observer_lists_.end() && it->second==context)
                {
                    observer_lists_.erase(it);
                }
            }
            delete context;
        }
    }

    typedef std::map<MessageLoop*, ObserverListContext*> ObserversListMap;

    base::Lock list_lock_; // ����observer_lists_.
    ObserversListMap observer_lists_;
    const NotificationType type_;

    DISALLOW_COPY_AND_ASSIGN(ObserverListThreadSafe);
};

#endif //__base_observer_list_threadsafe_h__