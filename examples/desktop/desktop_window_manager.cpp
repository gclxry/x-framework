
#include "desktop_window_manager.h"

#include "ui_gfx/point.h"
#include "ui_gfx/rect.h"

#include "view/event/event.h"
#include "view/widget/native_widget_private.h"
#include "view/widget/native_widget_view.h"
#include "view/widget/native_widget_views.h"
#include "view/widget/widget_delegate.h"
#include "view/window/non_client_view.h"

namespace
{

    class MoveWindowController : public view::desktop::WindowController
    {
    public:
        MoveWindowController(view::Widget* widget, const gfx::Point& start)
            : target_(widget), offset_(start) {}

        virtual ~MoveWindowController() {}

        bool OnMouseEvent(const view::MouseEvent& event)
        {
            if(event.type()== ui::ET_MOUSE_DRAGGED)
            {
                gfx::Point origin = event.location().Subtract(offset_);
                gfx::Rect rect = target_->GetWindowScreenBounds();
                rect.set_origin(origin);
                target_->SetBounds(rect);
                return true;
            }
            return false;
        }

    private:
        view::Widget* target_;
        gfx::Point offset_;

        DISALLOW_COPY_AND_ASSIGN(MoveWindowController);
    };

    // Simple resize controller that handle all resize as if the bottom
    // right corner is selected.
    class ResizeWindowController : public view::desktop::WindowController
    {
    public:
        ResizeWindowController(view::Widget* widget) : target_(widget) {}

        virtual ~ResizeWindowController() {}

        bool OnMouseEvent(const view::MouseEvent& event)
        {
            if(event.type()== ui::ET_MOUSE_DRAGGED)
            {
                gfx::Point location = event.location();
                gfx::Rect rect = target_->GetWindowScreenBounds();
                gfx::Point size = location.Subtract(rect.origin());
                target_->SetSize(gfx::Size(std::max(10, size.x()),
                    std::max(10, size.y())));
                return true;
            }
            return false;
        }

    private:
        view::Widget* target_;

        DISALLOW_COPY_AND_ASSIGN(ResizeWindowController);
    };

}

namespace view
{
    namespace desktop
    {

        WindowController::WindowController() {}

        WindowController::~WindowController() {}

        ////////////////////////////////////////////////////////////////////////////////
        // DesktopWindowManager, public:

        DesktopWindowManager::DesktopWindowManager(Widget* desktop)
            : desktop_(desktop), mouse_capture_(NULL), active_widget_(NULL){}

        DesktopWindowManager::~DesktopWindowManager()
        {
            DCHECK_EQ(0U, toplevels_.size()) << "Window manager getting destroyed "
                << "before all the windows are closed.";
        }

        void DesktopWindowManager::UpdateWindowsAfterScreenSizeChanged(
            const gfx::Rect& new_size)
        {
            for(std::vector<Widget*>::iterator i=toplevels_.begin();
                i!=toplevels_.end(); ++i)
            {
                Widget* toplevel = *i;
                if(!toplevel->IsMaximized())
                {
                    continue;
                }

                // If the window is maximized, then resize it!
                toplevel->SetSize(new_size.size());
            }
        }

        ////////////////////////////////////////////////////////////////////////////////
        // DesktopWindowManager, WindowManager implementation:

        void DesktopWindowManager::StartMoveDrag(
            view::Widget* widget,
            const gfx::Point& point)
        {
            DCHECK(!window_controller_.get());
            DCHECK(!HasMouseCapture());
            if(!widget->IsMaximized() && !widget->IsMinimized())
            {
                gfx::Point new_point = point;
                if(desktop_->non_client_view())
                {
                    gfx::Rect client = desktop_->non_client_view()->frame_view()->
                        GetBoundsForClientView();
                    new_point.Offset(client.x(), client.y());
                }
                SetMouseCapture();
                window_controller_.reset(new MoveWindowController(widget, new_point));
            }
        }

        void DesktopWindowManager::StartResizeDrag(
            view::Widget* widget, const gfx::Point& point, int hittest_code)
        {
            DCHECK(!window_controller_.get());
            DCHECK(!HasMouseCapture());
            if(!widget->IsMaximized() && !widget->IsMinimized() &&
                (widget->widget_delegate() || widget->widget_delegate()->CanResize()))
            {
                SetMouseCapture();
                window_controller_.reset(new ResizeWindowController(widget));
            }
        }

        bool DesktopWindowManager::SetMouseCapture(view::Widget* widget)
        {
            if(mouse_capture_)
            {
                return false;
            }
            if(mouse_capture_ == widget)
            {
                return true;
            }
            DCHECK(!HasMouseCapture());
            SetMouseCapture();
            mouse_capture_ = widget;
            return true;
        }

        bool DesktopWindowManager::ReleaseMouseCapture(view::Widget* widget)
        {
            if(!widget || mouse_capture_!=widget)
            {
                return false;
            }
            DCHECK(HasMouseCapture());
            ReleaseMouseCapture();
            mouse_capture_ = NULL;
            return true;
        }

        bool DesktopWindowManager::HasMouseCapture(const view::Widget* widget) const
        {
            return widget && mouse_capture_==widget;
        }

        bool DesktopWindowManager::HandleKeyEvent(
            view::Widget* widget, const view::KeyEvent& event)
        {
            return active_widget_ ?
                static_cast<NativeWidgetViews*>(active_widget_->native_widget_private())
                ->OnKeyEvent(event) : false;
        }

        bool DesktopWindowManager::HandleMouseEvent(
            view::Widget* widget, const view::MouseEvent& event)
        {
            if(mouse_capture_)
            {
                view::MouseEvent translated(event, widget->GetRootView(),
                    mouse_capture_->GetRootView());
                mouse_capture_->OnMouseEvent(translated);
                return true;
            }

            if(event.type() == ui::ET_MOUSE_PRESSED)
            {
                ActivateWidgetAtLocation(widget, event.location());
            }
            else if(event.type()==ui::ET_MOUSEWHEEL && active_widget_)
            {
                return active_widget_->OnMouseEvent(event);
            }

            if(window_controller_.get())
            {
                if(!window_controller_->OnMouseEvent(event))
                {
                    ReleaseMouseCapture();
                    window_controller_.reset();
                }
                return true;
            }

            return false;
        }

        void DesktopWindowManager::Register(Widget* widget)
        {
            DCHECK(!widget->HasObserver(this));
            if(widget->is_top_level())
            {
                toplevels_.push_back(widget);
            }
            widget->AddObserver(this);
        }

        ////////////////////////////////////////////////////////////////////////////////
        // DesktopWindowManager, private:

        void DesktopWindowManager::OnWidgetClosing(Widget* widget)
        {
            if(active_widget_ && active_widget_==widget)
            {
                active_widget_ = NULL;
            }

            if(widget->is_top_level())
            {
                for(std::vector<Widget*>::iterator i=toplevels_.begin();
                    i!=toplevels_.end(); ++i)
                {
                    if(*i == widget)
                    {
                        toplevels_.erase(i);
                        break;
                    }
                }
            }
        }

        void DesktopWindowManager::OnWidgetVisibilityChanged(Widget* widget,
            bool visible) {}

        void DesktopWindowManager::OnWidgetActivationChanged(Widget* widget,
            bool active)
        {
            if(active)
            {
                if(active_widget_)
                {
                    active_widget_->Deactivate();
                }
                active_widget_ = widget;
            }
            else if(widget == active_widget_)
            {
                active_widget_ = NULL;
            }
        }

        void DesktopWindowManager::SetMouseCapture()
        {
            return desktop_->native_widget_private()->SetMouseCapture();
        }

        void DesktopWindowManager::ReleaseMouseCapture()
        {
            return desktop_->native_widget_private()->ReleaseMouseCapture();
        }

        bool DesktopWindowManager::HasMouseCapture() const
        {
            return desktop_->native_widget_private()->HasMouseCapture();
        }

        void DesktopWindowManager::Activate(Widget* widget)
        {
            if(widget && widget->IsActive())
            {
                return;
            }

            if(widget)
            {
                if(!widget->HasObserver(this))
                {
                    widget->AddObserver(this);
                }
                widget->Activate();
            }
        }

        bool DesktopWindowManager::ActivateWidgetAtLocation(Widget* widget,
            const gfx::Point& point)
        {
            View* target = widget->GetRootView()->GetEventHandlerForPoint(point);

            if(target->GetClassName() == internal::NativeWidgetView::kViewClassName)
            {
                internal::NativeWidgetView* native_widget_view =
                    static_cast<internal::NativeWidgetView*>(target);
                view::Widget* target_widget = native_widget_view->GetAssociatedWidget();
                if(!target_widget->IsActive() && target_widget->CanActivate())
                {
                    Activate(target_widget);
                    return true;
                }
            }

            return false;
        }

    } //namespace desktop
} //namespace view