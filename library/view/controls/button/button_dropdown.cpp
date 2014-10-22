
#include "button_dropdown.h"

#include "base/message_loop.h"

#include "ui_base/accessibility/accessible_view_state.h"
#include "ui_base/l10n/l10n_util.h"
#include "ui_base/resource/app_res_ids.h"

#include "view/controls/menu/menu_delegate.h"
#include "view/controls/menu/menu_item_view.h"
#include "view/controls/menu/menu_model_adapter.h"
#include "view/controls/menu/menu_runner.h"
#include "view/widget/widget.h"

namespace view
{

    // static
    const char ButtonDropDown::kViewClassName[] = "view/ButtonDropDown";

    // How long to wait before showing the menu
    static const int kMenuTimerDelay = 500;

    ////////////////////////////////////////////////////////////////////////////////
    //
    // ButtonDropDown - constructors, destructors, initialization, cleanup
    //
    ////////////////////////////////////////////////////////////////////////////////

    ButtonDropDown::ButtonDropDown(ButtonListener* listener,
        ui::MenuModel* model)
        : ImageButton(listener),
        model_(model),
        y_position_on_lbuttondown_(0),
        show_menu_factory_(this) {}

    ButtonDropDown::~ButtonDropDown() {}

    ////////////////////////////////////////////////////////////////////////////////
    //
    // ButtonDropDown - Events
    //
    ////////////////////////////////////////////////////////////////////////////////

    bool ButtonDropDown::OnMousePressed(const MouseEvent& event)
    {
        if(IsEnabled() && IsTriggerableEvent(event) && HitTest(event.location()))
        {
            // Store the y pos of the mouse coordinates so we can use them later to
            // determine if the user dragged the mouse down (which should pop up the
            // drag down menu immediately, instead of waiting for the timer)
            y_position_on_lbuttondown_ = event.y();

            // Schedule a task that will show the menu.
            MessageLoop::current()->PostDelayedTask(
                show_menu_factory_.NewRunnableMethod(&ButtonDropDown::ShowDropDownMenu,
                GetWidget()->GetNativeView()),
                kMenuTimerDelay);
        }
        return ImageButton::OnMousePressed(event);
    }

    bool ButtonDropDown::OnMouseDragged(const MouseEvent& event)
    {
        bool result = ImageButton::OnMouseDragged(event);

        if(!show_menu_factory_.empty())
        {
            // If the mouse is dragged to a y position lower than where it was when
            // clicked then we should not wait for the menu to appear but show
            // it immediately.
            if(event.y() > y_position_on_lbuttondown_+GetHorizontalDragThreshold())
            {
                show_menu_factory_.RevokeAll();
                ShowDropDownMenu(GetWidget()->GetNativeView());
            }
        }

        return result;
    }

    void ButtonDropDown::OnMouseReleased(const MouseEvent& event)
    {
        if(IsTriggerableEvent(event) ||
            (event.IsRightMouseButton() && !HitTest(event.location())))
        {
            ImageButton::OnMouseReleased(event);
        }

        if(IsTriggerableEvent(event))
        {
            show_menu_factory_.RevokeAll();
        }

        if(IsEnabled() && event.IsRightMouseButton() && HitTest(event.location()))
        {
            show_menu_factory_.RevokeAll();
            ShowDropDownMenu(GetWidget()->GetNativeView());
        }
    }

    std::string ButtonDropDown::GetClassName() const
    {
        return kViewClassName;
    }

    void ButtonDropDown::OnMouseExited(const MouseEvent& event)
    {
        // Starting a drag results in a MouseExited, we need to ignore it.
        // A right click release triggers an exit event. We want to
        // remain in a PUSHED state until the drop down menu closes.
        if(state_!=BS_DISABLED && !InDrag() && state_!=BS_PUSHED)
        {
            SetState(BS_NORMAL);
        }
    }

    void ButtonDropDown::ShowContextMenu(const gfx::Point& p,
        bool is_mouse_gesture)
    {
        show_menu_factory_.RevokeAll();
        ShowDropDownMenu(GetWidget()->GetNativeView());
        SetState(BS_HOT);
    }

    void ButtonDropDown::GetAccessibleState(ui::AccessibleViewState* state)
    {
        CustomButton::GetAccessibleState(state);
        state->role = ui::AccessibilityTypes::ROLE_BUTTONDROPDOWN;
        state->default_action = ui::GetStringUTF16(IDS_APP_ACCACTION_PRESS);
        state->state = ui::AccessibilityTypes::STATE_HASPOPUP;
    }

    bool ButtonDropDown::ShouldEnterPushedState(const MouseEvent& event)
    {
        // Enter PUSHED state on press with Left or Right mouse button. Remain
        // in this state while the context menu is open.
        return ((ui::EF_LEFT_BUTTON_DOWN|
            ui::EF_RIGHT_BUTTON_DOWN) & event.flags()) != 0;
    }

    void ButtonDropDown::ShowDropDownMenu(HWND window)
    {
        gfx::Rect lb = GetLocalBounds();

        // Both the menu position and the menu anchor type change if the UI layout
        // is right-to-left.
        gfx::Point menu_position(lb.origin());
        menu_position.Offset(0, lb.height()-1);
        if(base::i18n::IsRTL())
        {
            menu_position.Offset(lb.width()-1, 0);
        }

        View::ConvertPointToScreen(this, &menu_position);

        int left_bound = GetSystemMetrics(SM_XVIRTUALSCREEN);
        if(menu_position.x() < left_bound)
        {
            menu_position.set_x(left_bound);
        }

        // Create and run menu.  Display an empty menu if model is NULL.
        if(model_)
        {
            MenuModelAdapter menu_delegate(model_);
            menu_delegate.set_triggerable_event_flags(triggerable_event_flags());
            MenuRunner runner(menu_delegate.CreateMenu());
            if(runner.RunMenuAt(GetWidget(), NULL,
                gfx::Rect(menu_position, gfx::Size(0, 0)),
                MenuItemView::TOPLEFT,
                MenuRunner::HAS_MNEMONICS) == MenuRunner::MENU_DELETED)
            {
                return;
            }
        }
        else
        {
            MenuDelegate menu_delegate;
            MenuItemView* menu = new MenuItemView(&menu_delegate);
            MenuRunner runner(menu);
            if(runner.RunMenuAt(GetWidget(), NULL,
                gfx::Rect(menu_position, gfx::Size(0, 0)),
                view::MenuItemView::TOPLEFT,
                MenuRunner::HAS_MNEMONICS) == MenuRunner::MENU_DELETED)
            {
                return;
            }
        }

        // Need to explicitly clear mouse handler so that events get sent
        // properly after the menu finishes running. If we don't do this, then
        // the first click to other parts of the UI is eaten.
        SetMouseHandler(NULL);

        // Set the state back to normal after the drop down menu is closed.
        if(state_ != BS_DISABLED)
        {
            SetState(BS_NORMAL);
        }
    }

} //namespace view