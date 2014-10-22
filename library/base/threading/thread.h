
#ifndef __base_thread_h__
#define __base_thread_h__

#pragma once

#include "base/message_loop.h"
#include "base/message_loop_proxy.h"
#include "platform_thread.h"

namespace base
{

    // һ���򵥵��̳߳���, �����߳��д���һ��MessageLoop. ʹ���̵߳�MessageLoop��
    // ���ô���������߳���ִ��. ����������ʱ, �߳�Ҳ����ֹ. �߳���Ϣѭ�����Ŷӵ�
    // �����������߳̽���ǰ���ᱻִ��.
    //
    // �߳�ֹͣ��, ����˳����:
    //
    //  (1) Thread::CleanUp()
    //  (2) MessageLoop::~MessageLoop
    //  (3.b)  MessageLoop::DestructionObserver::WillDestroyCurrentMessageLoop
    class Thread : PlatformThread::Delegate
    {
    public:
        struct Options
        {
            Options() : message_loop_type(MessageLoop::TYPE_DEFAULT),
                stack_size(0) {}
            Options(MessageLoop::Type type, size_t size)
                : message_loop_type(type), stack_size(size) {}

            // ָ���̷߳������Ϣѭ��������.
            MessageLoop::Type message_loop_type;

            // ָ���߳�����ʹ�õ����ջ�ռ�, ���̵߳ĳ�ʼջ�ռ䲻һ����ͬ.
            // 0��ʾӦ��ʹ��ȱʡ�����ֵ.
            size_t stack_size;
        };

        // ���캯��.
        // name���̵߳��ַ�����ʶ.
        explicit Thread(const char* name);

        // �̵߳���������, �б�Ҫ����ֹͣ�߳�.
        //
        // ע��: �����Thread����, ϣ���Լ���CleanUp����������, ����Ҫ���Լ�������
        // �����е���Stop().
        virtual ~Thread();

        // �����߳�. ����̳߳ɹ���������true, ���򷵻�false, message_loop()�Ƿ�
        // ��Ϊ��ȡ�����������ֵ.
        //
        // ע��: ��Windowsƽ̨, ���������ڼ�����������ʱ�����. ����DllMain������,
        // ȫ�ֶ���������ʱ, atexit()�ص�ʱ.
        bool Start();

        // �����߳�. ȱʡ��options���췽ʽ��Start()��Ϊһ��.
        //
        // ע��: ��Windowsƽ̨, ���������ڼ�����������ʱ�����. ����DllMain������,
        // ȫ�ֶ���������ʱ, atexit()�ص�ʱ.
        bool StartWithOptions(const Options& options);

        // ֪ͨ�߳��˳�, ���߳��˳�����������. �������غ�, �̶߳�����ȫ����, ���Ե���
        // �¹���Ķ���ʹ��(�����ٴ�����).
        //
        // Stop���Զ�ε���, ����߳��Ѿ�ֹͣ, �����κδ���.
        //
        // ע��: �������ǿ�ѡ��, ��ΪThread�������������Զ�ֹͣ�߳�.
        void Stop();

        // ֹͣ�߳̾����˳�.
        //
        // ����: ��Ҫ����ʹ�ñ�����, ���з���. ���ñ������ᵼ�º���message_loop()��
        // �طǷ�ֵ. �������������Ϊ�˽��Windowsƽ̨�ºʹ�ӡ�������̵߳���������.
        // �κ����������Ӧ��ʹ��Stop().
        //
        // ���ܶ�ε���StopSoon, ��������Σ��, �ڷ���message_loop()ʱ������ʱ������.
        // һ�������߳��˳���, ����Stop()�����̶߳���.
        void StopSoon();

        // ���ر��̵߳�MessageLoopProxy. ʹ��MessageLoopProxy��PostTask����������
        // ����߳�ִ�д���. ֻ���̵߳���Start�ɹ���, ��������ֵ�Ų�Ϊ��. Stop�߳�
        // ֮���ٴε���, �����᷵��NULL.
        //
        // ע��: ��Ҫֱ�ӵ��÷��ص�MessageLoop��Quit����. Ӧ�����̵߳�Stop�������.
        MessageLoop* message_loop() const { return message_loop_; }

        // ���ر��̵߳�MessageLoopProxy. ʹ��MessageLoopProxy��PostTask����������
        // ����߳�ִ�д���. ֻ���̵߳���Start�ɹ���, ��������ֵ�Ų�Ϊ��. ��ʹ�߳�
        // �˳�, �����߻�ӵ���������.
        scoped_refptr<MessageLoopProxy> message_loop_proxy()
        {
            return message_loop_->message_loop_proxy();
        }

        // ��ȡ�̵߳�����(����ʱ��ʾ��).
        const std::string& thread_name() { return name_; }

        // �����߳̾��.
        PlatformThreadHandle thread_handle() { return thread_; }

        // �߳�ID.
        PlatformThreadId thread_id() const { return thread_id_; }

        // ����߳��Ѿ�������δֹͣ����true. �߳�����ʱ, thread_id_��Ϊ0.
        bool IsRunning() const { return thread_id_ != kInvalidThreadId; }

    protected:
        // ������Ϣѭ��ǰ����.
        virtual void Init() {}

        // ������Ϣѭ��.
        virtual void Run(MessageLoop* message_loop);

        // ��Ϣѭ�����������.
        virtual void CleanUp() {}

        static void SetThreadWasQuitProperly(bool flag);
        static bool GetThreadWasQuitProperly();

        void set_message_loop(MessageLoop* message_loop)
        {
            message_loop_ = message_loop;
        }

    private:
        bool thread_was_started() const { return started_; }

        // ThreadDelegate����:
        virtual void ThreadMain();

        // �߳��Ƿ������ɹ�?
        bool started_;

        // true��ʾ����ֹͣ, ��Ӧ���ٷ���|message_loop_|, �����ǿ�ֵ���߷Ƿ�ֵ.
        bool stopping_;

        // �������ݸ�ThreadMain.
        struct StartupData;
        StartupData* startup_data_;

        // �̵߳ľ��.
        PlatformThreadHandle thread_;

        // �̵߳���Ϣѭ��, ���߳�����ʱ�Ϸ�, �ɱ��������߳�����.
        MessageLoop* message_loop_;

        // �߳�ID.
        PlatformThreadId thread_id_;

        // �߳�����, ���ڵ���.
        std::string name_;

        friend class ThreadQuitTask;

        DISALLOW_COPY_AND_ASSIGN(Thread);
    };

} //namespace base

#endif //__base_thread_h__