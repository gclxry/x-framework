
#include "client_view.h"

#include "base/logging.h"

#include "ui_base/accessibility/accessible_view_state.h"

namespace view
{

    // static
    const char ClientView::kViewClassName[] = "view/ClientView";

    ///////////////////////////////////////////////////////////////////////////////
    // ClientView, public:

    ClientView::ClientView(Widget* widget, View* contents_view)
        : widget_(widget), contents_view_(contents_view) {}

    int ClientView::NonClientHitTest(const gfx::Point& point)
    {
        return bounds().Contains(point) ? HTCLIENT : HTNOWHERE;
    }

    DialogClientView* ClientView::AsDialogClientView()
    {
        return NULL;
    }

    const DialogClientView* ClientView::AsDialogClientView() const
    {
        return NULL;
    }

    bool ClientView::CanClose()
    {
        return true;
    }

    void ClientView::WidgetClosing() {}

    ///////////////////////////////////////////////////////////////////////////////
    // ClientView, View overrides:

    gfx::Size ClientView::GetPreferredSize()
    {
        // |contents_view_| is allowed to be NULL up until the point where this view
        // is attached to a Container.
        if(contents_view_)
        {
            return contents_view_->GetPreferredSize();
        }
        return gfx::Size();
    }

    void ClientView::Layout()
    {
        // |contents_view_| is allowed to be NULL up until the point where this view
        // is attached to a Container.
        if(contents_view_)
        {
            contents_view_->SetBounds(0, 0, width(), height());
        }
    }

    std::string ClientView::GetClassName() const
    {
        return kViewClassName;
    }

    void ClientView::ViewHierarchyChanged(bool is_add, View* parent, View* child)
    {
        if(is_add && child==this)
        {
            DCHECK(GetWidget());
            DCHECK(contents_view_); // |contents_view_| must be valid now!
            // Insert |contents_view_| at index 0 so it is first in the focus chain.
            // (the OK/Cancel buttons are inserted before contents_view_)
            AddChildViewAt(contents_view_, 0);
        }
    }

    void ClientView::OnBoundsChanged(const gfx::Rect& previous_bounds)
    {
        // Overridden to do nothing. The NonClientView manually calls Layout on the
        // ClientView when it is itself laid out, see comment in
        // NonClientView::Layout.
    }

    void ClientView::GetAccessibleState(ui::AccessibleViewState* state)
    {
        state->role = ui::AccessibilityTypes::ROLE_CLIENT;
    }

} //namespace view