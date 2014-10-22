
#include "task.h"

Task::Task() {}

Task::~Task() {}

CancelableTask::CancelableTask() {}

CancelableTask::~CancelableTask() {}

namespace base
{

    ScopedTaskRunner::ScopedTaskRunner(Task* task) : task_(task) {}

    ScopedTaskRunner::~ScopedTaskRunner()
    {
        if(task_)
        {
            task_->Run();
            delete task_;
        }
    }

    Task* ScopedTaskRunner::Release()
    {
        Task* tmp = task_;
        task_ = NULL;
        return tmp;
    }

    namespace subtle
    {

        TaskClosureAdapter::TaskClosureAdapter(Task* task) : task_(task),
            should_leak_task_(&kTaskLeakingDefault) {}

        TaskClosureAdapter::TaskClosureAdapter(Task* task,
            bool* should_leak_task) : task_(task),
            should_leak_task_(should_leak_task) {}

        TaskClosureAdapter::~TaskClosureAdapter()
        {
            if(!*should_leak_task_)
            {
                delete task_;
            }
        }

        void TaskClosureAdapter::Run()
        {
            task_->Run();
            delete task_;
            task_ = NULL;
        }

        // Don't leak tasks by default.
        bool TaskClosureAdapter::kTaskLeakingDefault = false;

    } //namespace subtle

} //namespace base