
#include "bounds_animator.h"

#include "base/memory/scoped_ptr.h"

#include "ui_base/animation/animation_container.h"
#include "ui_base/animation/slide_animation.h"

#include "view/view.h"

// Duration in milliseconds for animations.
static const int kAnimationDuration = 200;

namespace view
{

    BoundsAnimator::BoundsAnimator(View* parent) : parent_(parent),
        observer_(NULL), container_(new ui::AnimationContainer())
    {
        container_->set_observer(this);
    }

    BoundsAnimator::~BoundsAnimator()
    {
        // Reset the delegate so that we don't attempt to notify our observer from
        // the destructor.
        container_->set_observer(NULL);

        // Delete all the animations, but don't remove any child views. We assume the
        // view owns us and is going to be deleted anyway.
        for(ViewToDataMap::iterator i=data_.begin(); i!=data_.end(); ++i)
        {
            CleanupData(false, &(i->second), i->first);
        }
    }

    void BoundsAnimator::AnimateViewTo(View* view, const gfx::Rect& target)
    {
        DCHECK(view);
        DCHECK_EQ(view->parent(), parent_);

        Data existing_data;

        if(IsAnimating(view))
        {
            // Don't immediatly delete the animation, that might trigger a callback from
            // the animationcontainer.
            existing_data = data_[view];

            RemoveFromMaps(view);
        }

        // NOTE: we don't check if the view is already at the target location. Doing
        // so leads to odd cases where no animations may be present after invoking
        // AnimateViewTo. AnimationProgressed does nothing when the bounds of the
        // view don't change.

        Data& data = data_[view];
        data.start_bounds = view->bounds();
        data.target_bounds = target;
        data.animation = CreateAnimation();

        animation_to_view_[data.animation] = view;

        data.animation->Show();

        CleanupData(true, &existing_data, NULL);
    }

    void BoundsAnimator::SetTargetBounds(View* view, const gfx::Rect& target)
    {
        if(!IsAnimating(view))
        {
            AnimateViewTo(view, target);
            return;
        }

        data_[view].target_bounds = target;
    }

    void BoundsAnimator::SetAnimationForView(View* view, ui::SlideAnimation* animation)
    {
        DCHECK(animation);

        scoped_ptr<ui::SlideAnimation> animation_wrapper(animation);

        if(!IsAnimating(view))
        {
            return;
        }

        // We delay deleting the animation until the end so that we don't prematurely
        // send out notification that we're done.
        scoped_ptr<ui::Animation> old_animation(ResetAnimationForView(view));

        data_[view].animation = animation_wrapper.release();
        animation_to_view_[animation] = view;

        animation->set_delegate(this);
        animation->SetContainer(container_.get());
        animation->Show();
    }

    const ui::SlideAnimation* BoundsAnimator::GetAnimationForView(View* view)
    {
        return !IsAnimating(view) ? NULL : data_[view].animation;
    }

    void BoundsAnimator::SetAnimationDelegate(View* view,
        AnimationDelegate* delegate, bool delete_when_done)
    {
        DCHECK(IsAnimating(view));

        data_[view].delegate = delegate;
        data_[view].delete_delegate_when_done = delete_when_done;
    }

    void BoundsAnimator::StopAnimatingView(View* view)
    {
        if(!IsAnimating(view))
        {
            return;
        }

        data_[view].animation->Stop();
    }

    bool BoundsAnimator::IsAnimating(View* view) const
    {
        return data_.find(view) != data_.end();
    }

    bool BoundsAnimator::IsAnimating() const
    {
        return !data_.empty();
    }

    void BoundsAnimator::Cancel()
    {
        if(data_.empty())
        {
            return;
        }

        while(!data_.empty())
        {
            data_.begin()->second.animation->Stop();
        }

        // Invoke AnimationContainerProgressed to force a repaint and notify delegate.
        AnimationContainerProgressed(container_.get());
    }

    ui::SlideAnimation* BoundsAnimator::CreateAnimation()
    {
        ui::SlideAnimation* animation = new ui::SlideAnimation(this);
        animation->SetContainer(container_.get());
        animation->SetSlideDuration(kAnimationDuration);
        animation->SetTweenType(ui::Tween::EASE_OUT);
        return animation;
    }

    void BoundsAnimator::RemoveFromMaps(View* view)
    {
        DCHECK(data_.count(view) > 0);
        DCHECK(animation_to_view_.count(data_[view].animation) > 0);

        animation_to_view_.erase(data_[view].animation);
        data_.erase(view);
    }

    void BoundsAnimator::CleanupData(bool send_cancel, Data* data, View* view)
    {
        if(send_cancel && data->delegate)
        {
            data->delegate->AnimationCanceled(data->animation);
        }

        if(data->delete_delegate_when_done)
        {
            delete static_cast<OwnedAnimationDelegate*>(data->delegate);
            data->delegate = NULL;
        }

        if(data->animation)
        {
            data->animation->set_delegate(NULL);
            delete data->animation;
            data->animation = NULL;
        }
    }

    ui::Animation* BoundsAnimator::ResetAnimationForView(View* view)
    {
        if(!IsAnimating(view))
        {
            return NULL;
        }

        ui::Animation* old_animation = data_[view].animation;
        animation_to_view_.erase(old_animation);
        data_[view].animation = NULL;
        // Reset the delegate so that we don't attempt any processing when the
        // animation calls us back.
        old_animation->set_delegate(NULL);
        return old_animation;
    }

    void BoundsAnimator::AnimationEndedOrCanceled(const ui::Animation* animation,
        AnimationEndType type)
    {
        DCHECK(animation_to_view_.find(animation) != animation_to_view_.end());

        View* view = animation_to_view_[animation];
        DCHECK(view);

        // Make a copy of the data as Remove empties out the maps.
        Data data = data_[view];

        RemoveFromMaps(view);

        if(data.delegate)
        {
            if(type == ANIMATION_ENDED)
            {
                data.delegate->AnimationEnded(animation);
            }
            else
            {
                DCHECK_EQ(ANIMATION_CANCELED, type);
                data.delegate->AnimationCanceled(animation);
            }
        }

        CleanupData(false, &data, view);
    }

    void BoundsAnimator::AnimationProgressed(const ui::Animation* animation)
    {
        DCHECK(animation_to_view_.find(animation) != animation_to_view_.end());

        View* view = animation_to_view_[animation];
        DCHECK(view);
        const Data& data = data_[view];
        gfx::Rect new_bounds = animation->CurrentValueBetween(
            data.start_bounds, data.target_bounds);
        if(new_bounds != view->bounds())
        {
            gfx::Rect total_bounds = new_bounds.Union(view->bounds());

            // Build up the region to repaint in repaint_bounds_. We'll do the repaint
            // when all animations complete (in AnimationContainerProgressed).
            if(repaint_bounds_.IsEmpty())
            {
                repaint_bounds_ = total_bounds;
            }
            else
            {
                repaint_bounds_ = repaint_bounds_.Union(total_bounds);
            }

            view->SetBoundsRect(new_bounds);
        }

        if(data.delegate)
        {
            data.delegate->AnimationProgressed(animation);
        }
    }

    void BoundsAnimator::AnimationEnded(const ui::Animation* animation)
    {
        AnimationEndedOrCanceled(animation, ANIMATION_ENDED);
    }

    void BoundsAnimator::AnimationCanceled(const ui::Animation* animation)
    {
        AnimationEndedOrCanceled(animation, ANIMATION_CANCELED);
    }

    void BoundsAnimator::AnimationContainerProgressed(ui::AnimationContainer* container)
    {
        if(!repaint_bounds_.IsEmpty())
        {
            // Adjust for rtl.
            repaint_bounds_.set_x(parent_->GetMirroredXWithWidthInView(
                repaint_bounds_.x(), repaint_bounds_.width()));
            parent_->SchedulePaintInRect(repaint_bounds_);
            repaint_bounds_.SetRect(0, 0, 0, 0);
        }

        if(observer_ && !IsAnimating())
        {
            // Notify here rather than from AnimationXXX to avoid deleting
            // the animation while the animaion is calling us.
            observer_->OnBoundsAnimatorDone(this);
        }
    }

    void BoundsAnimator::AnimationContainerEmpty(ui::AnimationContainer* container) {}

} //namespace view