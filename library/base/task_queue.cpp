
#include "task_queue.h"

#include "stl_utilinl.h"

TaskQueue::TaskQueue() {}

TaskQueue::~TaskQueue()
{
    // ӵ��ָ�������Ȩ, ����ɾ��ָ��.
    STLDeleteElements(&queue_);
}

void TaskQueue::Push(Task* task)
{
    DCHECK(task);

    // ��ӵ���β.
    queue_.push_back(task);
}

void TaskQueue::Clear()
{
    // ɾ������Ԫ�ص�ָ�벢��������.
    STLDeleteElements(&queue_);
}

bool TaskQueue::IsEmpty() const
{
    return queue_.empty();
}

void TaskQueue::Run()
{
    // �������Ϊ��ֱ�ӷ���.
    if(queue_.empty())
    {
        return;
    }

    std::deque<Task*> ready;
    queue_.swap(ready);

    // ִ�����е�����.
    std::deque<Task*>::const_iterator task;
    for(task=ready.begin(); task!=ready.end(); ++task)
    {
        // ִ�в�ɾ������.
        (*task)->Run();
        delete (*task);
    }
}