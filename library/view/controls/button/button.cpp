
#include "button.h"

#include "ui_base/accessibility/accessible_view_state.h"

namespace view
{

    ////////////////////////////////////////////////////////////////////////////////
    // Button, public:

    Button::~Button() {}

    void Button::SetTooltipText(const string16& tooltip_text)
    {
        tooltip_text_ = tooltip_text;
        TooltipTextChanged();
    }

    void Button::SetAccessibleName(const string16& name)
    {
        accessible_name_ = name;
    }

    void Button::SetAccessibleKeyboardShortcut(const string16& shortcut)
    {
        accessible_shortcut_ = shortcut;
    }

    ////////////////////////////////////////////////////////////////////////////////
    // Button, View overrides:

    bool Button::GetTooltipText(const gfx::Point& p, string16* tooltip)
    {
        if(tooltip_text_.empty())
        {
            return false;
        }

        *tooltip = tooltip_text_;
        return true;
    }

    void Button::GetAccessibleState(ui::AccessibleViewState* state)
    {
        state->role = ui::AccessibilityTypes::ROLE_PUSHBUTTON;
        state->name = accessible_name_;
        state->keyboard_shortcut = accessible_shortcut_;
    }

    ////////////////////////////////////////////////////////////////////////////////
    // Button, protected:

    Button::Button(ButtonListener* listener)
        : listener_(listener),
        tag_(-1),
        mouse_event_flags_(0)
    {
        set_accessibility_focusable(true);
    }

    void Button::NotifyClick(const view::Event& event)
    {
        mouse_event_flags_ = event.IsMouseEvent() ? event.flags() : 0;
        // We can be called when there is no listener, in cases like double clicks on
        // menu buttons etc.
        if(listener_)
        {
            listener_->ButtonPressed(this, event);
        }
        // NOTE: don't attempt to reset mouse_event_flags_ as the listener may have
        // deleted us.
    }

} //namespace view