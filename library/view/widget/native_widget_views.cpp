
#include "native_widget_views.h"

#include "native_widget_view.h"
#include "view/ime/input_method_win.h"
#include "root_view.h"
#include "view/view.h"
#include "view/view_delegate.h"
#include "window_manager.h"

namespace view
{

    ////////////////////////////////////////////////////////////////////////////////
    // NativeWidgetViews, public:

    NativeWidgetViews::NativeWidgetViews(internal::NativeWidgetDelegate* delegate)
        : delegate_(delegate),
        view_(NULL),
        active_(false),
        minimized_(false),
        always_on_top_(false),
        close_widget_factory_(this),
        hosting_widget_(NULL),
        ownership_(Widget::InitParams::NATIVE_WIDGET_OWNS_WIDGET),
        delete_native_view_(true) {}

    NativeWidgetViews::~NativeWidgetViews()
    {
        delegate_->OnNativeWidgetDestroying();

        if(view_ && delete_native_view_)
        {
            // We must prevent the NativeWidgetView from attempting to delete us.
            view_->set_delete_native_widget(false);
            delete view_;
        }

        delegate_->OnNativeWidgetDestroyed();
        if(ownership_ == Widget::InitParams::NATIVE_WIDGET_OWNS_WIDGET)
        {
            delete delegate_;
        }
    }

    View* NativeWidgetViews::GetView()
    {
        return view_;
    }

    const View* NativeWidgetViews::GetView() const
    {
        return view_;
    }

    void NativeWidgetViews::OnActivate(bool active)
    {
        // TODO(oshima): find out if we should check toplevel here.
        if(active_ == active)
        {
            return;
        }
        active_ = active;
        delegate_->OnNativeWidgetActivationChanged(active);

        // TODO(oshima): Focus change should be separated from window activation.
        // This will be fixed when we have WM API.
        Widget* widget = GetWidget();
        if(widget->is_top_level())
        {
            InputMethodWin* input_method = GetWidget()->GetInputMethod();
            if(active)
            {
                if(input_method)
                {
                    input_method->OnFocus();
                }
                // See description of got_initial_focus_in_ for details on this.
                widget->GetFocusManager()->RestoreFocusedView();
            }
            else
            {
                if(input_method)
                {
                    input_method->OnBlur();
                }
                widget->GetFocusManager()->StoreFocusedView();
            }
        }
        view_->SchedulePaint();
    }

    bool NativeWidgetViews::OnKeyEvent(const KeyEvent& key_event)
    {
        delegate_->OnKeyEvent(key_event);
        return true;
    }

    ////////////////////////////////////////////////////////////////////////////////
    // NativeWidgetViews, NativeWidget implementation:

    void NativeWidgetViews::InitNativeWidget(const Widget::InitParams& params)
    {
        ownership_ = params.ownership;
        always_on_top_ = params.keep_on_top;
        View* parent_view = NULL;
        if(params.parent_widget)
        {
            hosting_widget_ = params.parent_widget;
            parent_view = hosting_widget_->GetChildViewParent();
        }
        else
        {
            parent_view = ViewDelegate::view_delegate->GetDefaultParentView();
            hosting_widget_ = parent_view->GetWidget();
        }

        view_ = new internal::NativeWidgetView(this);
        view_->SetBoundsRect(params.bounds);
        // TODO(beng): re-enable this once we have a consolidated layer tree.
        view_->SetPaintToLayer(true);

        // With the default NATIVE_WIDGET_OWNS_WIDGET ownership, the
        // deletion of either of the NativeWidgetViews or NativeWidgetView
        // (e.g. via View hierarchy destruction) will delete the other.
        // With WIDGET_OWNS_NATIVE_WIDGET, NativeWidgetViews should only
        // be deleted by its Widget.
        if(ownership_ == Widget::InitParams::WIDGET_OWNS_NATIVE_WIDGET)
        {
            view_->set_delete_native_widget(false);
        }

        parent_view->AddChildView(view_);

        // TODO(beng): SetInitParams().
    }

    NonClientFrameView* NativeWidgetViews::CreateNonClientFrameView()
    {
        return NULL;
    }

    void NativeWidgetViews::UpdateFrameAfterFrameChange() {}

    bool NativeWidgetViews::ShouldUseNativeFrame() const
    {
        return false;
    }

    void NativeWidgetViews::FrameTypeChanged() {}

    Widget* NativeWidgetViews::GetWidget()
    {
        return delegate_->AsWidget();
    }

    const Widget* NativeWidgetViews::GetWidget() const
    {
        return delegate_->AsWidget();
    }

    HWND NativeWidgetViews::GetNativeView() const
    {
        return GetParentNativeWidget()->GetNativeView();
    }

    HWND NativeWidgetViews::GetNativeWindow() const
    {
        return GetParentNativeWidget()->GetNativeWindow();
    }

    Widget* NativeWidgetViews::GetTopLevelWidget()
    {
        // This can get called when this is in the process of being destroyed, and
        // view_ has already been unset.
        if(!view_)
        {
            return GetWidget();
        }
        if(view_->parent() == ViewDelegate::view_delegate->GetDefaultParentView())
        {
            return GetWidget();
        }
        // During Widget destruction, this function may be called after |view_| is
        // detached from a Widget, at which point this NativeWidget's Widget becomes
        // the effective toplevel.
        Widget* containing_widget = view_->GetWidget();
        return containing_widget ? containing_widget->GetTopLevelWidget()
            : GetWidget();
    }

    const ui::Compositor* NativeWidgetViews::GetCompositor() const
    {
        return hosting_widget_->GetCompositor();
    }

    ui::Compositor* NativeWidgetViews::GetCompositor()
    {
        return hosting_widget_->GetCompositor();
    }

    void NativeWidgetViews::CalculateOffsetToAncestorWithLayer(
        gfx::Point* offset,
        ui::Layer** layer_parent)
    {
        view_->CalculateOffsetToAncestorWithLayer(offset, layer_parent);
    }

    void NativeWidgetViews::ViewRemoved(View* view)
    {
        internal::NativeWidgetPrivate* parent = GetParentNativeWidget();
        if(parent)
        {
            parent->ViewRemoved(view);
        }
    }

    void NativeWidgetViews::SetNativeWindowProperty(const char* name, void* value)
    {
        if(value)
        {
            window_properties_[name] = value;
        }
        else
        {
            window_properties_.erase(name);
        }
    }

    void* NativeWidgetViews::GetNativeWindowProperty(const char* name) const
    {
        std::map<const char*, void*>::const_iterator iter =
            window_properties_.find(name);
        return iter != window_properties_.end() ? iter->second : NULL;
    }

    TooltipManager* NativeWidgetViews::GetTooltipManager() const
    {
        const internal::NativeWidgetPrivate* parent = GetParentNativeWidget();
        return parent ? parent->GetTooltipManager() : NULL;
    }

    bool NativeWidgetViews::IsScreenReaderActive() const
    {
        return GetParentNativeWidget()->IsScreenReaderActive();
    }

    void NativeWidgetViews::SendNativeAccessibilityEvent(
        View* view,
        ui::AccessibilityTypes::Event event_type)
    {
        return GetParentNativeWidget()->SendNativeAccessibilityEvent(view,
            event_type);
    }

    void NativeWidgetViews::SetMouseCapture()
    {
        WindowManager::Get()->SetMouseCapture(GetWidget());
    }

    void NativeWidgetViews::ReleaseMouseCapture()
    {
        WindowManager::Get()->ReleaseMouseCapture(GetWidget());
    }

    bool NativeWidgetViews::HasMouseCapture() const
    {
        return WindowManager::Get()->HasMouseCapture(GetWidget());
    }

    InputMethodWin* NativeWidgetViews::CreateInputMethod()
    {
        return NULL;
    }

    void NativeWidgetViews::CenterWindow(const gfx::Size& size)
    {
        const gfx::Size parent_size = GetView()->parent()->size();
        GetView()->SetBounds((parent_size.width() - size.width())/2,
            (parent_size.height() - size.height())/2,
            size.width(), size.height());
    }

    void NativeWidgetViews::GetWindowPlacement(
        gfx::Rect* bounds,
        ui::WindowShowState* show_state) const
    {
        *bounds = GetView()->bounds();
        *show_state = ui::SHOW_STATE_NORMAL;
    }

    void NativeWidgetViews::SetWindowTitle(const std::wstring& title) {}

    void NativeWidgetViews::SetWindowIcons(const SkBitmap& window_icon,
        const SkBitmap& app_icon) {}

    void NativeWidgetViews::SetAccessibleName(const std::wstring& name) {}

    void NativeWidgetViews::SetAccessibleRole(ui::AccessibilityTypes::Role role) {}

    void NativeWidgetViews::SetAccessibleState(
        ui::AccessibilityTypes::State state) {}

    void NativeWidgetViews::BecomeModal()
    {
        NOTIMPLEMENTED();
    }

    gfx::Rect NativeWidgetViews::GetWindowScreenBounds() const
    {
        if(GetWidget() == GetWidget()->GetTopLevelWidget())
        {
            return view_->bounds();
        }
        gfx::Point origin = view_->bounds().origin();
        View::ConvertPointToScreen(view_->parent(), &origin);
        return gfx::Rect(origin.x(), origin.y(), view_->width(), view_->height());
    }

    gfx::Rect NativeWidgetViews::GetClientAreaScreenBounds() const
    {
        return GetWindowScreenBounds();
    }

    gfx::Rect NativeWidgetViews::GetRestoredBounds() const
    {
        return GetWindowScreenBounds();
    }

    void NativeWidgetViews::SetBounds(const gfx::Rect& bounds)
    {
        // |bounds| are supplied in the coordinates of the parent.
        view_->SetBoundsRect(bounds);
    }

    void NativeWidgetViews::SetSize(const gfx::Size& size)
    {
        view_->SetSize(size);
    }

    void NativeWidgetViews::SetBoundsConstrained(const gfx::Rect& bounds,
        Widget* other_widget)
    {
        // TODO(beng): honor other_widget.
        SetBounds(bounds);
    }

    void NativeWidgetViews::MoveAbove(HWND native_view)
    {
        NOTIMPLEMENTED();
    }

    void NativeWidgetViews::MoveToTop()
    {
        view_->parent()->ReorderChildView(view_, -1);
    }

    void NativeWidgetViews::SetShape(HRGN region)
    {
        NOTIMPLEMENTED();
    }

    void NativeWidgetViews::Close()
    {
        Hide();
        if(close_widget_factory_.empty())
        {
            MessageLoop::current()->PostTask(
                close_widget_factory_.NewRunnableMethod(&NativeWidgetViews::CloseNow));
        }
    }

    void NativeWidgetViews::CloseNow()
    {
        delete view_;
        view_ = NULL;
    }

    void NativeWidgetViews::EnableClose(bool enable) {}

    void NativeWidgetViews::Show()
    {
        if(always_on_top_)
        {
            MoveToTop();
        }
        view_->SetVisible(true);
        GetWidget()->SetInitialFocus();
    }

    void NativeWidgetViews::Hide()
    {
        view_->SetVisible(false);
        if(HasMouseCapture())
        {
            ReleaseMouseCapture();
        }
    }

    void NativeWidgetViews::ShowWithWindowState(ui::WindowShowState show_state)
    {
        Show();
    }

    void NativeWidgetViews::ShowMaximizedWithBounds(
        const gfx::Rect& restored_bounds)
    {
        Show();
    }

    bool NativeWidgetViews::IsVisible() const
    {
        return view_->IsVisible();
    }

    void NativeWidgetViews::Activate()
    {
        // Enable WidgetObserverTest.ActivationChange when this is implemented.
        MoveToTop();
        OnActivate(true);
    }

    void NativeWidgetViews::Deactivate()
    {
        OnActivate(false);
    }

    bool NativeWidgetViews::IsActive() const
    {
        return active_;
    }

    void NativeWidgetViews::SetAlwaysOnTop(bool on_top)
    {
        always_on_top_ = on_top;
        // This is not complete yet. At least |MoveToTop| will need to be updated to
        // make sure a 'normal' window does not get on top of a window with
        // |always_on_top_| set.
        NOTIMPLEMENTED();
    }

    void NativeWidgetViews::Maximize()
    {
        NOTIMPLEMENTED();
    }

    void NativeWidgetViews::Minimize()
    {
        gfx::Rect view_bounds = view_->bounds();
        gfx::Rect parent_bounds = view_->parent()->bounds();

        restored_bounds_ = view_bounds;
        restored_transform_ = view_->GetTransform();

        float aspect_ratio = static_cast<float>(view_bounds.width()) /
            static_cast<float>(view_bounds.height());
        int target_size = 100;
        int target_height = target_size;
        int target_width = static_cast<int>(aspect_ratio * target_height);
        if(target_width > target_size)
        {
            target_width = target_size;
            target_height = static_cast<int>(target_width / aspect_ratio);
        }

        int target_x = 20;
        int target_y = parent_bounds.height() - target_size - 20;

        view_->SetBounds(target_x, target_y, view_bounds.width(), view_bounds.height());

        gfx::Transform transform;
        transform.SetScale((float)target_width / (float)view_bounds.width(),
            (float)target_height / (float)view_bounds.height());
        view_->SetTransform(transform);

        minimized_ = true;
    }

    bool NativeWidgetViews::IsMaximized() const
    {
        // NOTIMPLEMENTED();
        return false;
    }

    bool NativeWidgetViews::IsMinimized() const
    {
        return minimized_;
    }

    void NativeWidgetViews::Restore()
    {
        minimized_ = false;
        view_->SetBoundsRect(restored_bounds_);
        view_->SetTransform(restored_transform_);
    }

    void NativeWidgetViews::SetFullscreen(bool fullscreen)
    {
        NOTIMPLEMENTED();
    }

    bool NativeWidgetViews::IsFullscreen() const
    {
        // NOTIMPLEMENTED();
        return false;
    }

    void NativeWidgetViews::SetOpacity(unsigned char opacity)
    {
        NOTIMPLEMENTED();
    }

    void NativeWidgetViews::SetUseDragFrame(bool use_drag_frame)
    {
        NOTIMPLEMENTED();
    }

    bool NativeWidgetViews::IsAccessibleWidget() const
    {
        NOTIMPLEMENTED();
        return false;
    }

    void NativeWidgetViews::RunShellDrag(View* view,
        const ui::OSExchangeData& data,
        int operation)
    {
        GetParentNativeWidget()->RunShellDrag(view, data, operation);
    }

    void NativeWidgetViews::SchedulePaintInRect(const gfx::Rect& rect)
    {
        view_->SchedulePaintInRect(rect);
    }

    void NativeWidgetViews::SetCursor(HCURSOR cursor)
    {
        view_->set_cursor(cursor);
        GetParentNativeWidget()->SetCursor(cursor);
    }

    void NativeWidgetViews::ClearNativeFocus()
    {
        GetParentNativeWidget()->ClearNativeFocus();
    }

    void NativeWidgetViews::FocusNativeView(HWND native_view)
    {
        GetParentNativeWidget()->FocusNativeView(native_view);
    }

    bool NativeWidgetViews::ConvertPointFromAncestor(
        const Widget* ancestor, gfx::Point* point) const
    {
        // This method converts the point from ancestor's coordinates to
        // this widget's coordinate using recursion as the widget hierachy
        // is usually shallow.

        if(ancestor == GetWidget())
        {
            return true; // no conversion necessary
        }

        const Widget* parent_widget = view_->GetWidget();
        if(!parent_widget) // couldn't reach the ancestor.
        {
            return false;
        }

        if(parent_widget==ancestor ||
            parent_widget->ConvertPointFromAncestor(ancestor, point))
        {
            View::ConvertPointToView(parent_widget->GetRootView(), GetView(), point);
            return true;
        }
        return false;
    }

    ////////////////////////////////////////////////////////////////////////////////
    // NativeWidgetViews, private:

    internal::NativeWidgetPrivate* NativeWidgetViews::GetParentNativeWidget()
    {
        Widget* containing_widget = view_ ? view_->GetWidget() : NULL;
        return containing_widget ? static_cast<internal::NativeWidgetPrivate*>(
            containing_widget->native_widget()) :
        NULL;
    }

    const internal::NativeWidgetPrivate*
        NativeWidgetViews::GetParentNativeWidget() const
    {
        const Widget* containing_widget = view_ ? view_->GetWidget() : NULL;
        return containing_widget ? static_cast<const internal::NativeWidgetPrivate*>(
            containing_widget->native_widget()) :
        NULL;
    }

} //namespace view