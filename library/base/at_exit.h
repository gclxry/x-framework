
#ifndef __base_at_exit_h__
#define __base_at_exit_h__

#pragma once

#include <stack>

#include "callback.h"
#include "synchronization/lock.h"

namespace base
{

    // AtExitManager�ṩ������CRT��atexit()�Ĺ���, ��ͬ������������˺�ʱ����
    // �ص�����.��Windows�µ�DLL�������ش����ʱ��, ��������������лص�����.
    // AtExitManagerһ������Singleton��.
    //
    // ʹ�úܼ�, ֻ��Ҫ��main()����WinMain()����������ͷ����һ��ջ����:
    //     int main(...)
    //     {
    //         base::AtExitManager exit_manager;
    //     }
    // ��exit_manager�뿪�������ʱ��, ����ע��Ļص������͵�������������
    // �ᱻ����.
    class AtExitManager
    {
    public:
        typedef void (*AtExitCallbackType)(void*);

        AtExitManager();
        // ����������������ע��Ļص�, ����֮������ע��ص�.
        ~AtExitManager();

        // ע���˳�ʱ�Ļص�����. ����ԭ����void func(void*).
        static void RegisterCallback(AtExitCallbackType func, void* param);

        // Registers the specified task to be called at exit.
        static void RegisterTask(base::Closure task);

        // ��LIFO�����ע��Ļص�����, ��������֮�����ע���µĻص�.
        static void ProcessCallbacksNow();

    private:
        Lock lock_;
        std::stack<base::Closure> stack_;

        DISALLOW_COPY_AND_ASSIGN(AtExitManager);
    };

} //namespace base

#endif //__base_at_exit_h__