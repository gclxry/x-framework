
#ifndef __ui_base_animation_delegate_h__
#define __ui_base_animation_delegate_h__

#pragma once

namespace ui
{

    class Animation;

    // AnimationDelegate
    //
    // ��Ҫ����animation��״̬֪ͨ, ��Ҫʵ������ӿ�.
    class AnimationDelegate
    {
    public:
        // animation���ʱ����.
        virtual void AnimationEnded(const Animation* animation) {}

        // animation����ʱ����.
        virtual void AnimationProgressed(const Animation* animation) {}

        // animationȡ��ʱ����.
        virtual void AnimationCanceled(const Animation* animation) {}

    protected:
        virtual ~AnimationDelegate() {}
    };

} //namespace ui

#endif //__ui_base_animation_delegate_h__