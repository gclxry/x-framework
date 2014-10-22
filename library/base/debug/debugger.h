
#ifndef __base_debugger_h__
#define __base_debugger_h__

#pragma once

namespace base
{
    namespace debug
    {

        // ����ϵͳ��ע���JIT�����������ӵ�ָ������.
        bool SpawnDebuggerOnProcess(unsigned process_id);

        // �ȴ�wait_seconds���Ա���������ӵ���ǰ����. ��silent==false, ��⵽
        // ����������׳��쳣.
        bool WaitForDebugger(int wait_seconds, bool silent);

        // �ڵ�����������?
        bool BeingDebugged();

        // �ڵ��������ж�, ǰ���Ǵ��ڵ�����.
        void BreakDebugger();

        // ���ڲ��Դ������õ�. ���ڿ����ڷ�������ʱ�Ƿ���ʾ�Ի��򲢽��������.
        // һ�����Զ������Ե�ʱ��ʹ��.
        void SetSuppressDebugUI(bool suppress);
        bool IsDebugUISuppressed();

    } //namespace debug
} //namespace base

#endif //__base_debugger_h__