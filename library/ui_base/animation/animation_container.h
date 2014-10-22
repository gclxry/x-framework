
#ifndef __ui_base_animation_container_h__
#define __ui_base_animation_container_h__

#pragma once

#include <set>

#include "base/timer.h"

namespace ui
{

    class AnimationContainerElement;
    class AnimationContainerObserver;

    // AnimationContainer��Animationʹ��, ����ײ��ʱ��. ÿ��Animation������
    // �ڲ�����һ��AnimationContainer. ����ͨ��Animation::SetContainer����һ��
    // Animations��ͬһ��AnimationContainer, ͬһ���Animations��֤ͬʱ���º�
    // ����.
    //
    // AnimationContainerʹ�������ü���. ÿ��Animation�������ڲ���
    // AnimationContainer����.
    class AnimationContainer : public base::RefCounted<AnimationContainer>
    {
    public:
        AnimationContainer();

        // Animation��Ҫ������ʱ�����. ����б�Ҫ����ʱ��.
        // ע��: Animation���Զ�����, ��Ҫֱ�ӵ���.
        void Start(AnimationContainerElement* animation);

        // Animation��Ҫֹͣ��ʱ�����. ���û������animations����, ֹͣʱ��.
        // ע��: Animation���Զ�����, ��Ҫֱ�ӵ���.
        void Stop(AnimationContainerElement* animation);

        void set_observer(AnimationContainerObserver* observer)
        {
            observer_ = observer;
        }

        // ���һ��ִ��animation��ʱ��.
        base::TimeTicks last_tick_time() const { return last_tick_time_; }

        // �Ƿ���ʱ��������?
        bool is_running() const { return !elements_.empty(); }

    private:
        friend class base::RefCounted<AnimationContainer>;

        typedef std::set<AnimationContainerElement*> Elements;

        ~AnimationContainer();

        // Timer�ص�����.
        void Run();

        // ����min_timer_interval_������ʱ��.
        void SetMinTimerInterval(base::TimeDelta delta);

        // ��������ʱ�ӵ���Сʱ����.
        base::TimeDelta GetMinInterval();

        // ֵ�����ֿ���:
        // . ���ֻ��һ��animation������ʱ�ӻ�û������, ��ʾ���animation��ʱ��.
        // . ���һ��ִ��animation��ʱ��(::Run������).
        base::TimeTicks last_tick_time_;

        // �����Ԫ��(animations)����.
        Elements elements_;

        // ʱ�����е���Сʱ����.
        base::TimeDelta min_timer_interval_;

        base::RepeatingTimer<AnimationContainer> timer_;

        AnimationContainerObserver* observer_;

        DISALLOW_COPY_AND_ASSIGN(AnimationContainer);
    };

} //namespace ui

#endif //__ui_base_animation_container_h__