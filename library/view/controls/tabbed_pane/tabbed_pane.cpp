
#include "tabbed_pane.h"

#include "base/logging.h"

#include "ui_base/accessibility/accessible_view_state.h"
#include "ui_base/keycodes/keyboard_codes_win.h"

#include "native_tabbed_pane_wrapper.h"
#include "tabbed_pane_listener.h"
#include "view/controls/native/native_view_host.h"
#include "view/widget/widget.h"

namespace view
{

    // static
    const char TabbedPane::kViewClassName[] = "view/TabbedPane";

    TabbedPane::TabbedPane() : native_tabbed_pane_(NULL), listener_(NULL)
    {
        set_focusable(true);
    }

    TabbedPane::~TabbedPane() {}

    int TabbedPane::GetTabCount()
    {
        return native_tabbed_pane_->GetTabCount();
    }

    int TabbedPane::GetSelectedTabIndex()
    {
        return native_tabbed_pane_->GetSelectedTabIndex();
    }

    View* TabbedPane::GetSelectedTab()
    {
        return native_tabbed_pane_->GetSelectedTab();
    }

    void TabbedPane::AddTab(const std::wstring& title, View* contents)
    {
        native_tabbed_pane_->AddTab(title, contents);
        PreferredSizeChanged();
    }

    void TabbedPane::AddTabAtIndex(int index,
        const std::wstring& title,
        View* contents,
        bool select_if_first_tab)
    {
        native_tabbed_pane_->AddTabAtIndex(index, title, contents,
            select_if_first_tab);
        PreferredSizeChanged();
    }

    View* TabbedPane::RemoveTabAtIndex(int index)
    {
        View* tab = native_tabbed_pane_->RemoveTabAtIndex(index);
        PreferredSizeChanged();
        return tab;
    }

    void TabbedPane::SelectTabAt(int index)
    {
        native_tabbed_pane_->SelectTabAt(index);
    }

    void TabbedPane::SetAccessibleName(const string16& name)
    {
        accessible_name_ = name;
    }

    gfx::Size TabbedPane::GetPreferredSize()
    {
        return native_tabbed_pane_ ?
            native_tabbed_pane_->GetPreferredSize() : gfx::Size();
    }

    void TabbedPane::CreateWrapper()
    {
        native_tabbed_pane_ = NativeTabbedPaneWrapper::CreateNativeWrapper(this);
    }

    void TabbedPane::LoadAccelerators()
    {
        // Ctrl+Shift+Tab
        AddAccelerator(Accelerator(ui::VKEY_TAB, true, true, false));
        // Ctrl+Tab
        AddAccelerator(Accelerator(ui::VKEY_TAB, false, true, false));
    }

    void TabbedPane::Layout()
    {
        if(native_tabbed_pane_)
        {
            native_tabbed_pane_->GetView()->SetBounds(0, 0, width(), height());
        }
    }

    void TabbedPane::ViewHierarchyChanged(bool is_add, View* parent, View* child)
    {
        if(is_add && !native_tabbed_pane_)
        {
            CreateWrapper();
            AddChildView(native_tabbed_pane_->GetView());
            LoadAccelerators();
        }
    }

    bool TabbedPane::AcceleratorPressed(const Accelerator& accelerator)
    {
        // We only accept Ctrl+Tab keyboard events.
        DCHECK(accelerator.key_code() ==
            ui::VKEY_TAB && accelerator.IsCtrlDown());

        int tab_count = GetTabCount();
        if(tab_count <= 1)
        {
            return false;
        }
        int selected_tab_index = GetSelectedTabIndex();
        int next_tab_index = accelerator.IsShiftDown() ?
            (selected_tab_index - 1) % tab_count :
            (selected_tab_index + 1) % tab_count;
        // Wrap around.
        if(next_tab_index < 0)
        {
            next_tab_index += tab_count;
        }
        SelectTabAt(next_tab_index);
        return true;
    }

    std::string TabbedPane::GetClassName() const
    {
        return kViewClassName;
    }

    void TabbedPane::OnFocus()
    {
        // Forward the focus to the wrapper.
        if(native_tabbed_pane_)
        {
            native_tabbed_pane_->SetFocus();

            View* selected_tab = GetSelectedTab();
            if(selected_tab)
            {
                selected_tab->GetWidget()->NotifyAccessibilityEvent(
                    selected_tab, ui::AccessibilityTypes::EVENT_FOCUS, true);
            }
        }
        else
        {
            View::OnFocus(); // Will focus the RootView window (so we still get
                             // keyboard messages).
        }
    }

    void TabbedPane::OnPaintFocusBorder(gfx::Canvas* canvas)
    {
        if(NativeViewHost::kRenderNativeControlFocus)
        {
            View::OnPaintFocusBorder(canvas);
        }
    }

    void TabbedPane::GetAccessibleState(ui::AccessibleViewState* state)
    {
        state->role = ui::AccessibilityTypes::ROLE_PAGETABLIST;
        state->name = accessible_name_;
    }

} //namespace view