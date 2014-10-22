
#ifndef __base_message_pump_default_h__
#define __base_message_pump_default_h__

#pragma once

#include "message_pump.h"
#include "synchronization/waitable_event.h"
#include "time.h"

namespace base
{

    class MessagePumpDefault : public MessagePump
    {
    public:
        MessagePumpDefault();
        ~MessagePumpDefault() {}

        virtual void Run(Delegate* delegate);
        virtual void Quit();
        virtual void ScheduleWork();
        virtual void ScheduleDelayedWork(const TimeTicks& delayed_work_time);

    private:
        // ��־λ����Ϊfalse��ʾRunѭ����Ҫ����.
        bool keep_running_;

        // ����ֱ�������鷢��.
        WaitableEvent event_;

        // ��Ҫ����DoDelayedWork��ʱ���.
        TimeTicks delayed_work_time_;

        DISALLOW_COPY_AND_ASSIGN(MessagePumpDefault);
    };

} //namespace base

#endif //__base_message_pump_default_h__