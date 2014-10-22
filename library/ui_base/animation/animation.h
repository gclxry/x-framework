
#ifndef __ui_base_animation_h__
#define __ui_base_animation_h__

#pragma once

#include "base/memory/ref_counted.h"

#include "animation_container_element.h"

namespace gfx
{
    class Rect;
}

namespace ui
{

    class AnimationContainer;
    class AnimationDelegate;

    // ��������. ���Դ�LinearAnimation��SlideAnimation��ThrobAnimation����
    // MultiAnimation������ʵ�ֶ���, ֻ����Ҫʵ���µĶ�������ʱ����Ҫ�����������.
    //
    // ������Ҫ����Step. ������ǰ��ʱ����, GetCurrentValue()���ض�����ʱ��ֵ.
    class Animation : public AnimationContainerElement
    {
    public:
        explicit Animation(base::TimeDelta timer_interval);
        virtual ~Animation();

        // ��������. ����Ѿ�����, ʲô������.
        void Start();

        // ֹͣ����. ���û������, ʲô������.
        void Stop();

        // ����ʹ�õĶ������ߵõ���ǰ״̬��ֵ.
        virtual double GetCurrentValue() const = 0;

        // ���ݵ�ǰֵ����һ������|start|��|target|֮���ֵ. Ҳ����:
        //   (target - start) * GetCurrentValue() + start
        double CurrentValueBetween(double start, double target) const;
        int CurrentValueBetween(int start, int target) const;
        gfx::Rect CurrentValueBetween(const gfx::Rect& start_bounds,
            const gfx::Rect& target_bounds) const;

        // ���ô���.
        void set_delegate(AnimationDelegate* delegate) { delegate_ = delegate; }

        // ���ù���ʱ�ӵ�����. ����NULL��ʾ����һ���µ�AnimationContainer.
        void SetContainer(AnimationContainer* container);

        bool is_animating() const { return is_animating_; }

        base::TimeDelta timer_interval() const { return timer_interval_; }

        // ������۽ϸߵĶ�����Ҫ��Ⱦ����true.
        // ���Ự����(����Զ������)�ͷ���������, ���Ƿ���Ҫ��Ⱦ���۽ϸߵĶ���.
        static bool ShouldRenderRichAnimation();

    protected:
        // ��Start�е���, ����������Ϊ������׼��.
        virtual void AnimationStarted() {}

        // ���������Ƴ���, ��Stop�е���, ��ʱ����δ������.
        virtual void AnimationStopped() {}

        // ����ֹͣʱ����, ȷ���Ƿ���Ҫ����ȡ��. �������true, ֪ͨ��������ȡ��,
        // ����֪ͨ��������ֹͣ.
        virtual bool ShouldSendCanceledFromStop() { return false; }

        AnimationContainer* container() { return container_.get(); }
        base::TimeTicks start_time() const { return start_time_; }
        AnimationDelegate* delegate() { return delegate_; }

        virtual void SetStartTime(base::TimeTicks start_time);
        virtual void Step(base::TimeTicks time_now) = 0;
        virtual base::TimeDelta GetTimerInterval() const { return timer_interval_; }

    private:
        // �������.
        const base::TimeDelta timer_interval_;

        // �Ƿ�������.
        bool is_animating_;

        // ����; ����Ϊ��.
        AnimationDelegate* delegate_;

        // ���ڵ�����. ���ڶ���ʱ��Ϊ��.
        scoped_refptr<AnimationContainer> container_;

        // ����ʱ��.
        base::TimeTicks start_time_;

        DISALLOW_COPY_AND_ASSIGN(Animation);
    };

} //namespace ui

#endif //__ui_base_animation_h__