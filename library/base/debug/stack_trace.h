
#ifndef __base_stack_trace_h__
#define __base_stack_trace_h__

// ��������صĸ�������. ���ڲ��Գ����Ƿ��ڵ������������Լ�
// �Ƿ���Ҫ�ڵ��������ж�.

#pragma once

#include <iosfwd>

struct _EXCEPTION_POINTERS;

namespace base
{
    namespace debug
    {

        // ��ջ�����ڵ��Ե�ʱ��ǳ�����. �����ڶ��������StackTrace��Ա(
        // һ����#ifndef NDEBUG��), ��������֪�������������ﴴ����.
        class StackTrace
        {
        public:
            // �ڵ�ǰλ�ù���һ��stacktrace.
            StackTrace();

            // ȱʡ�Ŀ�������͸�ֵ���������������.

            // Ϊ�쳣����stacktrace.
            // ע��: ���������û��dbghelp 5.1��ϵͳ�ϻ��׳��Ҳ�����ڵ�
            // (an import not found (StackWalk64))���쳣.
            StackTrace(_EXCEPTION_POINTERS* exception_pointers);

            // Creates a stacktrace from an existing array of instruction
            // pointers (such as returned by Addresses()).  |count| will be
            // trimmed to |kMaxTraces|.
            StackTrace(const void* const* trace, size_t count);

            // Copying and assignment are allowed with the default functions.
            ~StackTrace();

            // ��ö�ջ��Ϣ����
            //   count: ���ض�ջ��Ϣ������.
            const void* const* Addresses(size_t* count);
            // ��ӡ��ջ������Ϣ����׼���.
            void PrintBacktrace();
            // ���ݽ������ű�д������.
            void OutputToStream(std::ostream* os);

        private:
            // �μ�http://msdn.microsoft.com/en-us/library/bb204633.aspx,
            // FramesToSkip��FramesToCapture֮�ͱ���С��63, �������ö�ջ����
            // ��������ֵΪ62, ��ʹ����ƽ̨���ṩ�����ֵ, һ��Ҳûʲô����.
            static const int MAX_TRACES = 62;
            void* trace_[MAX_TRACES];
            int count_;
        };

    } //namespace debug
} //namespace base

#endif //__base_stack_trace_h__