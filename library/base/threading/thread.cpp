
#include "thread.h"

#include "base/lazy_instance.h"
#include "base/synchronization/waitable_event.h"
#include "base/threading/thread_local.h"
#include "base/win/windows_version.h"

namespace base
{

    namespace
    {

        // ʹ���ֲ߳̾�������¼һ���߳��Ƿ�ͨ������Stop�����˳���. �������ǿ��Բ�׽��
        // ֱ�ӵ���MessageLoop::Quit()�����, ����Thread���������е�MessageLoop, ��ϣ
        // ��ʹ�������˳���ʽ.
        base::LazyInstance<base::ThreadLocalBoolean> lazy_tls_bool(
            base::LINKER_INITIALIZED);

    }

    // ���ڴ�����Ϣѭ���˳�.
    class ThreadQuitTask : public Task
    {
    public:
        virtual void Run()
        {
            MessageLoop::current()->Quit();
            Thread::SetThreadWasQuitProperly(true);
        }
    };

    // ���ڴ������ݸ�ThreadMain, ��StartWithOptions������ջ�Ϸ������.
    struct Thread::StartupData
    {
        // ��Ϊ��ջ�Ϸ���ṹ�����, �����������ֻ�ó�������.
        const Thread::Options& options;

        // ����ͬ���߳�����.
        WaitableEvent event;

        explicit StartupData(const Options& opt)
            : options(opt), event(false, false) {}
    };

    Thread::Thread(const char* name)
        : started_(false),
        stopping_(false),
        startup_data_(NULL),
        thread_(0),
        message_loop_(NULL),
        thread_id_(kInvalidThreadId),
        name_(name) {}

    Thread::~Thread()
    {
        Stop();
    }

    void Thread::SetThreadWasQuitProperly(bool flag)
    {
        lazy_tls_bool.Pointer()->Set(flag);
    }

    bool Thread::GetThreadWasQuitProperly()
    {
        bool quit_properly = true;
#ifndef NDEBUG
        quit_properly = lazy_tls_bool.Pointer()->Get();
#endif
        return quit_properly;
    }

    bool Thread::Start()
    {
        return StartWithOptions(Options());
    }

    bool Thread::StartWithOptions(const Options& options)
    {
        DCHECK(!message_loop_);

        SetThreadWasQuitProperly(false);

        StartupData startup_data(options);
        startup_data_ = &startup_data;

        if(!PlatformThread::Create(options.stack_size, this, &thread_))
        {
            DLOG(ERROR) << "failed to create thread";
            startup_data_ = NULL;
            return false;
        }

        // �ȴ��߳���������ʼ��message_loop_.
        startup_data.event.Wait();

        // ���ó�NULL, �������ǲ��ᱣ��һ��ջ�϶����ָ��.
        startup_data_ = NULL;
        started_ = true;

        DCHECK(message_loop_);
        return true;
    }

    void Thread::Stop()
    {
        if(!thread_was_started())
        {
            return;
        }

        StopSoon();

        // �ȴ��߳��˳�.
        //
        // TODO: �ܲ���, ������Ҫ����message_loop_����ֱ���߳��˳�. ��Щ�û�����
        // API. ʹ����ֹͣ.
        PlatformThread::Join(thread_);

        // �߳��˳�ʱ���message_loop_���ó�NULL.
        DCHECK(!message_loop_);

        // �̲߳���Ҫ�ٴν���.
        started_ = false;

        stopping_ = false;
    }

    void Thread::StopSoon()
    {
        // ֻ���������߳��е���(�������̵߳��߳�!=���߳�).
        DCHECK_NE(thread_id_, PlatformThread::CurrentId());

        if(stopping_ || !message_loop_)
        {
            return;
        }

        stopping_ = true;
        message_loop_->PostTask(new ThreadQuitTask());
    }

    void Thread::Run(MessageLoop* message_loop)
    {
        message_loop->Run();
    }

    void Thread::ThreadMain()
    {
        {
            // ���̵߳���Ϣѭ��.
            MessageLoop message_loop(startup_data_->options.message_loop_type);

            // ����̶߳���ĳ�ʼ��.
            thread_id_ = PlatformThread::CurrentId();
            PlatformThread::SetName(name_.c_str());
            message_loop.set_thread_name(name_);
            message_loop_ = &message_loop;

            // �����߳�������ĳ�ʼ������, ��֪ͨ�����߳�ǰ����.
            Init();

            startup_data_->event.Signal();
            // ������ʹ��startup_data_, ��Ϊ�����̴߳�ʱ�Ѿ�����.

            Run(message_loop_);

            // �����߳��������������.
            CleanUp();

            // ����MessageLoop::Quit�Ǳ�ThreadQuitTask���õ�.
            DCHECK(GetThreadWasQuitProperly());

            // ���ٽ����κ���Ϣ.
            message_loop_ = NULL;
        }
        thread_id_ = kInvalidThreadId;
    }

} //namespace base