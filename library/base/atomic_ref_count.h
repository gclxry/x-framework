
// ���ü���ԭ�Ӳ�������ĵײ�ʵ��. ��ֱ��ʹ��memory/ref_counted.h.

#ifndef __base_atomic_ref_count_h__
#define __base_atomic_ref_count_h__

#pragma once

#include "atomicops.h"

namespace base
{

    typedef subtle::Atomic32 AtomicRefCount;

    // �������ü���, "increment"�������0.
    inline void AtomicRefCountIncN(volatile AtomicRefCount* ptr,
        AtomicRefCount increment)
    {
        subtle::NoBarrier_AtomicIncrement(ptr, increment);
    }

    // �������ü���, "decrement"�������0, ���ؽ���Ƿ��0.
    inline bool AtomicRefCountDecN(volatile AtomicRefCount* ptr,
        AtomicRefCount decrement)
    {
        bool res = subtle::Barrier_AtomicIncrement(ptr, -decrement) != 0;
        return res;
    }

    // ���ü�����1.
    inline void AtomicRefCountInc(volatile AtomicRefCount* ptr)
    {
        AtomicRefCountIncN(ptr, 1);
    }

    // ���ü�����1, ���ؽ���Ƿ��0.
    inline bool AtomicRefCountDec(volatile AtomicRefCount* ptr)
    {
        return AtomicRefCountDecN(ptr, 1);
    }

    // �������ü����Ƿ�Ϊ1. ���ճ���ʵ�ַ�ʽ, ���ü���Ϊ1����ֻ�е�ǰ�߳�ӵ������,
    // ���������̹߳���. �����������ü����Ƿ�Ϊ1, ��ȷ�������Ƿ���Ҫ�������.
    inline bool AtomicRefCountIsOne(volatile AtomicRefCount* ptr)
    {
        bool res = subtle::Acquire_Load(ptr) == 1;
        return res;
    }

    // �������ü����Ƿ�Ϊ0. ���ճ���ʵ�ַ�ʽ, ���ü���Ϊ0, ������Ҫ����,
    // һ�����ü���Ӧ�ò�Ϊ0, ����һ�����ڵ��Լ��.
    inline bool AtomicRefCountIsZero(volatile AtomicRefCount* ptr)
    {
        bool res = subtle::Acquire_Load(ptr) == 0;
        return res;
    }

} //namespace base

#endif //__base_atomic_ref_count_h__