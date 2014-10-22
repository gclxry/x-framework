
#ifndef __ui_base_compositor_h__
#define __ui_base_compositor_h__

#pragma once

#include "base/memory/ref_counted.h"
#include "base/observer_list.h"

#include "ui_gfx/size.h"
#include "ui_gfx/transform.h"

class SkCanvas;

namespace gfx
{
    class Point;
    class Rect;
    class Transform;
}

namespace ui
{

    class CompositorObserver;
    class Layer;

    struct TextureDrawParams
    {
        TextureDrawParams() : transform(), blend(false), compositor_size() {}

        // The transform to be applied to the texture.
        gfx::Transform transform;

        // If this is true, then the texture is blended with the pixels behind it.
        // Otherwise, the drawn pixels clobber the old pixels.
        bool blend;

        // The size of the surface that the texture is drawn to.
        gfx::Size compositor_size;

        // Copy and assignment are allowed.
    };

    // Textures are created by a Compositor for managing an accelerated view.
    // Any time a View with a texture needs to redraw itself it invokes SetCanvas().
    // When the view is ready to be drawn Draw() is invoked.
    //
    // Texture is really a proxy to the gpu. Texture does not itself keep a copy of
    // the bitmap.
    //
    // Views own the Texture.
    class Texture : public base::RefCounted<Texture>
    {
    public:
        // Sets the canvas of this texture. The origin is at |origin|.
        // |overall_size| gives the total size of texture.
        virtual void SetCanvas(const SkCanvas& canvas,
            const gfx::Point& origin,
            const gfx::Size& overall_size) = 0;

        // Draws the portion of the texture contained within clip_bounds
        virtual void Draw(const ui::TextureDrawParams& params,
            const gfx::Rect& bounds_in_texture) = 0;

    protected:
        virtual ~Texture() {}

    private:
        friend class base::RefCounted<Texture>;
    };

    // An interface to allow the compositor to communicate with its owner.
    class CompositorDelegate
    {
    public:
        // Requests the owner to schedule a paint.
        virtual void ScheduleCompositorPaint() = 0;
    };

    // Compositor object to take care of GPU painting.
    // A Browser compositor object is responsible for generating the final
    // displayable form of pixels comprising a single widget's contents. It draws an
    // appropriately transformed texture for each transformed view in the widget's
    // view hierarchy.
    class Compositor : public base::RefCounted<Compositor>
    {
    public:
        // Create a compositor from the provided handle.
        static Compositor* Create(CompositorDelegate* delegate,
            HWND widget, const gfx::Size& size);

        // Creates a new texture. The caller owns the returned object.
        virtual Texture* CreateTexture() = 0;

        // Blurs the specific region in the compositor.
        virtual void Blur(const gfx::Rect& bounds) = 0;

        // Schedules a paint on the widget this Compositor was created for.
        void SchedulePaint()
        {
            delegate_->ScheduleCompositorPaint();
        }

        // Sets the root of the layer tree drawn by this Compositor.
        // The Compositor does not own the root layer.
        void set_root_layer(Layer* root_layer)
        {
            root_layer_ = root_layer;
        }

        // Draws the scene created by the layer tree and any visual effects. If
        // |force_clear| is true, this will cause the compositor to clear before
        // compositing.
        void Draw(bool force_clear);

        // Notifies the compositor that the size of the widget that it is
        // drawing to has changed.
        void WidgetSizeChanged(const gfx::Size& size)
        {
            size_ = size;
            OnWidgetSizeChanged();
        }

        // Returns the size of the widget that is being drawn to.
        const gfx::Size& size() { return size_; }

        // Layers do not own observers. It is the responsibility of the observer to
        // remove itself when it is done observing.
        void AddObserver(CompositorObserver* observer);
        void RemoveObserver(CompositorObserver* observer);

    protected:
        Compositor(CompositorDelegate* delegate, const gfx::Size& size);
        virtual ~Compositor();

        // Notifies the compositor that compositing is about to start.
        virtual void OnNotifyStart(bool clear) = 0;

        // Notifies the compositor that compositing is complete.
        virtual void OnNotifyEnd() = 0;

        virtual void OnWidgetSizeChanged() = 0;

        CompositorDelegate* delegate() { return delegate_; }

    private:
        // Notifies the compositor that compositing is about to start. See Draw() for
        // notes about |force_clear|.
        void NotifyStart(bool force_clear);

        // Notifies the compositor that compositing is complete.
        void NotifyEnd();

        CompositorDelegate* delegate_;
        gfx::Size size_;

        // The root of the Layer tree drawn by this compositor.
        Layer* root_layer_;

        ObserverList<CompositorObserver> observer_list_;

        friend class base::RefCounted<Compositor>;
    };

} //namespace ui

#endif //__ui_base_compositor_h__