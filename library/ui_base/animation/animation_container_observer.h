
#ifndef __ui_base_animation_container_observer_h__
#define __ui_base_animation_container_observer_h__

#pragma once

namespace ui
{

    class AnimationContainer;

    // ���������animationsÿ�θ��¶���֪ͨ�۲���.
    class AnimationContainerObserver
    {
    public:
        // ���������ʱ�Ӵ���ʱ, ������animations������ɺ���ñ�����.
        virtual void AnimationContainerProgressed(
            AnimationContainer* container) = 0;

        // �����������animationsΪ��ʱ����.
        virtual void AnimationContainerEmpty(AnimationContainer* container) = 0;

    protected:
        virtual ~AnimationContainerObserver() {}
    };

} //namespace ui

#endif //__ui_base_animation_container_observer_h__