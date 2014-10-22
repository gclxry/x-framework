
#ifndef __base_threading_thread_restrictions_h__
#define __base_threading_thread_restrictions_h__

#pragma once

#include "base/basic_types.h"

namespace base
{

    // �ض����߳���ĳЩ��Ϊ���ܻᱻ��ֹ. ThreadRestrictions����ʵ����Щ����.
    // ���ֹ��������:
    //
    // *��Ҫ����IO(�����߳̽���)
    // *��Ҫ����Singleton/LazyInstance(���ܵ��±���)
    //
    // ���������ʵ�ֱ���������:
    //
    // 1) ���һ���̲߳�����IO����, ������:
    //      base::ThreadRestrictions::SetIOAllowed(false);
    //    ȱʡ���, �߳����������IO��.
    //
    // 2) �������÷��ʴ���ʱ, ��Ҫ��鵱ǰ�߳��Ƿ�����:
    //      base::ThreadRestrictions::AssertIOAllowed();
    //
    // ThreadRestrictions��release����ʱʲô������; ֻ��debug����Ч.
    //
    // �����ʾ: ��Ӧ����ʲô�ط����AssertIOAllowed? ����Ǽ������ʴ���
    // ��ʱ��, �ڵײ�Խ����Խ��. ���׼�������ڲ������еĵ���. ����, ���
    // ��ĺ���GoDoSomeBlockingDiskCall()ֻ������������������fopen(), ��
    // Ӧ���ڸ������������AssertIOAllowed���.

    class ThreadRestrictions
    {
    public:
        // ����һ��ScopedAllowIO, ����ǰ�߳���ʱ����IO. ��������������
        // ���ᵼ�´���.
        class ScopedAllowIO
        {
        public:
            ScopedAllowIO() { previous_value_ = SetIOAllowed(true); }
            ~ScopedAllowIO() { SetIOAllowed(previous_value_); }
        private:
            // �Ƿ��������IO.
            bool previous_value_;

            DISALLOW_COPY_AND_ASSIGN(ScopedAllowIO);
        };

        // ����һ��ScopedAllowSingleton, ����ǰ�߳���ʱʹ�õ���. ������
        // �����������ᵼ�´���.
        class ScopedAllowSingleton
        {
        public:
            ScopedAllowSingleton() { previous_value_ = SetSingletonAllowed(true); }
            ~ScopedAllowSingleton() { SetSingletonAllowed(previous_value_); }
        private:
            // �Ƿ�������ʵ���.
            bool previous_value_;

            DISALLOW_COPY_AND_ASSIGN(ScopedAllowSingleton);
        };

#ifndef NDEBUG
        // ���õ�ǰ�߳��Ƿ�����IO����. �߳�����ȱʡ�������.
        // ���ص���ǰ��ֵ.
        static bool SetIOAllowed(bool allowed);

        // ��鵱ǰ�߳��Ƿ�����IO����, �������DCHECK. �ںδ����м��μ�
        // �������ע��.
        static void AssertIOAllowed();

        // ���õ�ǰ�߳��Ƿ�����ʹ�õ���. ���ص���ǰ��ֵ.
        static bool SetSingletonAllowed(bool allowed);

        // ��鵱ǰ�߳��Ƿ�����ʹ�õ���, �������DCHECK. 
        static void AssertSingletonAllowed();
#else
        // ��Release�汾��, ���������Ŀպ���, �����ᱻ�������Ż���.
        static bool SetIOAllowed(bool allowed) { return true; }
        static void AssertIOAllowed() {}
        static bool SetSingletonAllowed(bool allowed) { return true; }
        static void AssertSingletonAllowed() {}
#endif

    private:
        DISALLOW_IMPLICIT_CONSTRUCTORS(ThreadRestrictions);
    };

} //namespace base

#endif //__base_threading_thread_restrictions_h__