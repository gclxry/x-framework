
// ����: �ֲ߳̾��洢�úò�����ô����, ��ȷ�����������Ҫ����÷���. ��Ҫ
// ������Ż�, ������һ��Lock���ɴﵽĿ��.
//
// ��Щ��Բ���ϵͳ��TLS�洢���ƽ����˷�װ. �ڹ��캯���з���һ��TLS���,
// ������ʱ���ͷ�, û���������ڴ����. ����ζ��ʹ��ThreadLocalPointerʱ,
// ������ȷ�Ĺ����Լ�������ڴ�. ��Щ�಻���������ָ��, ���߳��˳�ʱû
// ���κζ������.
//
// ThreadLocalPointer<Type>��װType*, �����𴴽�������, �����ڴ������
// �����ط�������ȷ. ��һ���߳��״ε���Get()�᷵��NULL, ��Ҫͨ��Set()
// ��������ָ��.
//
// ThreadLocalBoolean��װbool, ȱʡ��false, ��Ҫͨ��Set()��������ֵ.
//
// �̰߳�ȫ��: ThreadLocalStorageʵ���ڴ���֮�����̰߳�ȫ��. ����붯̬
// ����ʵ��, ���봦����̳߳�ͻ. Ҳ����˵��������ľ�̬��ʼ���ǲ�̫�õ�.
//
// ʹ��ʾ��:
//     // MyClass�฽����һ���߳���. �洢ָ�뵽TLS, ��������ʵ��current().
//     MyClass::MyClass() {
//       DCHECK(Singleton<ThreadLocalPointer<MyClass> >::get()->Get() == NULL);
//       Singleton<ThreadLocalPointer<MyClass> >::get()->Set(this);
//     }
//
//     MyClass::~MyClass() {
//       DCHECK(Singleton<ThreadLocalPointer<MyClass> >::get()->Get() != NULL);
//       Singleton<ThreadLocalPointer<MyClass> >::get()->Set(NULL);
//     }
//
//     // Return the current MyClass associated with the calling thread, can be
//     // NULL if there isn't a MyClass associated.
//     MyClass* MyClass::current() {
//       return Singleton<ThreadLocalPointer<MyClass> >::get()->Get();
//     }

#ifndef __base_thread_local_h__
#define __base_thread_local_h__

#pragma once

#include "base/basic_types.h"

namespace base
{
    namespace internal
    {

        // ��ƽ̨APIs������. ��Ҫֱ��ʹ��.
        struct ThreadLocalPlatform
        {
            typedef unsigned long SlotType;

            static void AllocateSlot(SlotType& slot);
            static void FreeSlot(SlotType& slot);
            static void* GetValueFromSlot(SlotType& slot);
            static void SetValueInSlot(SlotType& slot, void* value);
        };

    } //namespace internal

    template<typename Type>
    class ThreadLocalPointer
    {
    public:
        ThreadLocalPointer() : slot_()
        {
            internal::ThreadLocalPlatform::AllocateSlot(slot_);
        }

        ~ThreadLocalPointer()
        {
            internal::ThreadLocalPlatform::FreeSlot(slot_);
        }

        Type* Get()
        {
            return static_cast<Type*>(
                internal::ThreadLocalPlatform::GetValueFromSlot(slot_));
        }

        void Set(Type* ptr)
        {
            internal::ThreadLocalPlatform::SetValueInSlot(slot_, ptr);
        }

    private:
        typedef internal::ThreadLocalPlatform::SlotType SlotType;

        SlotType slot_;

        DISALLOW_COPY_AND_ASSIGN(ThreadLocalPointer<Type>);
    };

    class ThreadLocalBoolean
    {
    public:
        ThreadLocalBoolean() {}
        ~ThreadLocalBoolean() {}

        bool Get()
        {
            return tlp_.Get() != NULL;
        }

        void Set(bool val)
        {
            tlp_.Set(val ? this : NULL);
        }

    private:
        ThreadLocalPointer<void> tlp_;

        DISALLOW_COPY_AND_ASSIGN(ThreadLocalBoolean);
    };

} //namespace base

#endif //__base_thread_local_h__