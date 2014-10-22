
#ifndef __ui_base_animation_container_element_h__
#define __ui_base_animation_container_element_h__

#pragma once

#include "base/time.h"

namespace ui
{

    // AnimationContainer������Ԫ�ؽӿ�, ��Animationʵ��.
    class AnimationContainerElement
    {
    public:
        // ����animation������ʱ��. ��AnimationContainer::Start����.
        virtual void SetStartTime(base::TimeTicks start_time) = 0;

        // ��animation����ʱ����.
        virtual void Step(base::TimeTicks time_now) = 0;

        // ����animation��ʱ����. ���һ��Ԫ����Ҫ�ı����ֵ, ��Ҫ��
        // ����Stop, Ȼ��Start.
        virtual base::TimeDelta GetTimerInterval() const = 0;

    protected:
        virtual ~AnimationContainerElement() {}
    };

} //namespace ui

#endif //__ui_base_animation_container_element_h__