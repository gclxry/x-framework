
#include "native_combobox_view.h"

#include <algorithm>

#include "base/logging.h"
#include "base/utf_string_conversions.h"

#include "ui_gfx/canvas_skia.h"
#include "ui_gfx/font.h"
#include "ui_gfx/path.h"

#include "ui_base/models/combobox_model.h"
#include "ui_base/resource/resource_bundle.h"

#include "combobox.h"
#include "view/controls/focusable_border.h"
#include "view/controls/menu/menu_runner.h"
#include "view/controls/menu/submenu_view.h"
#include "view/widget/widget.h"

namespace
{

    // Define the size of the insets.
    const int kTopInsetSize = 4;
    const int kLeftInsetSize = 4;
    const int kBottomInsetSize = 4;
    const int kRightInsetSize = 4;

    // Limit how small a combobox can be.
    const int kMinComboboxWidth = 148;

    // Size of the combobox arrow.
    const int kComboboxArrowSize = 9;
    const int kComboboxArrowOffset = 7;
    const int kComboboxArrowMargin = 12;

    // Color settings for text and border.
    // These are tentative, and should be derived from theme, system
    // settings and current settings.
    const SkColor kDefaultBorderColor = SK_ColorGRAY;
    const SkColor kTextColor = SK_ColorBLACK;

    // Define the id of the first item in the menu (since it needs to be > 0)
    const int kFirstMenuItemId = 1000;

}

namespace view
{

    const char NativeComboboxView::kViewClassName[] = "view/NativeComboboxView";

    NativeComboboxView::NativeComboboxView(Combobox* parent)
        : combobox_(parent),
        text_border_(new FocusableBorder()),
        dropdown_open_(false),
        selected_item_(-1),
        content_width_(0),
        content_height_(0)
    {
        set_border(text_border_);
    }

    NativeComboboxView::~NativeComboboxView() {}

    ////////////////////////////////////////////////////////////////////////////////
    // NativeComboboxView, View overrides:

    bool NativeComboboxView::OnMousePressed(const MouseEvent& mouse_event)
    {
        combobox_->RequestFocus();
        if(mouse_event.IsLeftMouseButton())
        {
            UpdateFromModel();
            ShowDropDownMenu();
        }

        return true;
    }

    bool NativeComboboxView::OnMouseDragged(const MouseEvent& mouse_event)
    {
        return true;
    }

    bool NativeComboboxView::OnKeyPressed(const KeyEvent& key_event)
    {
        // TODO(oshima): handle IME.
        DCHECK(key_event.type() == ui::ET_KEY_PRESSED);

        // Check if we are in the default state (-1) and set to first item.
        if(selected_item_ == -1)
        {
            selected_item_ = 0;
        }

        int new_item = selected_item_;
        switch(key_event.key_code())
        {
            // move to the next element if any
        case ui::VKEY_DOWN:
            if(new_item < (combobox_->model()->GetItemCount() - 1))
            {
                new_item++;
            }
            break;

            // move to the end of the list
        case ui::VKEY_END:
        case ui::VKEY_NEXT:
            new_item = combobox_->model()->GetItemCount() - 1;
            break;

            // move to the top of the list
        case ui::VKEY_HOME:
        case ui::VKEY_PRIOR:
            new_item = 0;
            break;

            // move to the previous element if any
        case ui::VKEY_UP:
            if(new_item > 0)
            {
                new_item--;
            }
            break;

        default:
            return false;
        }

        if(new_item != selected_item_)
        {
            selected_item_ = new_item;
            combobox_->SelectionChanged();
            SchedulePaint();
        }

        return true;
    }

    bool NativeComboboxView::OnKeyReleased(const KeyEvent& key_event)
    {
        return true;
    }

    void NativeComboboxView::OnPaint(gfx::Canvas* canvas)
    {
        text_border_->set_has_focus(combobox_->HasFocus());
        OnPaintBackground(canvas);
        PaintText(canvas);
        OnPaintBorder(canvas);
    }

    void NativeComboboxView::OnFocus()
    {
        NOTREACHED();
    }

    void NativeComboboxView::OnBlur()
    {
        NOTREACHED();
    }

    /////////////////////////////////////////////////////////////////
    // NativeComboboxView, NativeComboboxWrapper overrides:

    void NativeComboboxView::UpdateFromModel()
    {
        int max_width = 0;
        const gfx::Font& font = GetFont();

        MenuItemView* menu = new MenuItemView(this);
        // MenuRunner owns |menu|.
        dropdown_list_menu_runner_.reset(new MenuRunner(menu));

        int num_items = combobox_->model()->GetItemCount();
        for(int i=0; i<num_items; ++i)
        {
            string16 text = combobox_->model()->GetItemAt(i);

            // Inserting the Unicode formatting characters if necessary so that the
            // text is displayed correctly in right-to-left UIs.
            base::i18n::AdjustStringForLocaleDirection(&text);

            menu->AppendMenuItem(i+kFirstMenuItemId, UTF16ToWide(text),
                MenuItemView::NORMAL);
            max_width = std::max(max_width, font.GetStringWidth(text));
        }

        content_width_ = max_width;
        content_height_ = font.GetFontSize();
    }

    void NativeComboboxView::UpdateSelectedItem()
    {
        selected_item_ = combobox_->selected_item();
    }

    void NativeComboboxView::UpdateEnabled()
    {
        SetEnabled(combobox_->IsEnabled());
    }

    int NativeComboboxView::GetSelectedItem() const
    {
        return selected_item_;
    }

    bool NativeComboboxView::IsDropdownOpen() const
    {
        return dropdown_open_;
    }

    gfx::Size NativeComboboxView::GetPreferredSize()
    {
        if(content_width_ == 0)
        {
            UpdateFromModel();
        }

        // TODO(saintlou) the preferred size will drive the local bounds
        // which in turn is used to set the minimum width for the dropdown
        gfx::Insets insets = GetInsets();
        return gfx::Size(std::min(kMinComboboxWidth,
            content_width_+2*(insets.width())),
            content_height_+2*(insets.height()));
    }

    View* NativeComboboxView::GetView()
    {
        return this;
    }

    void NativeComboboxView::SetFocus()
    {
        text_border_->set_has_focus(true);
    }

    bool NativeComboboxView::HandleKeyPressed(const KeyEvent& e)
    {
        return OnKeyPressed(e);
    }

    bool NativeComboboxView::HandleKeyReleased(const KeyEvent& e)
    {
        return true;
    }

    void NativeComboboxView::HandleFocus() 
    {
        SchedulePaint();
    }

    void NativeComboboxView::HandleBlur() {}

    HWND NativeComboboxView::GetTestingHandle() const
    {
        NOTREACHED();
        return NULL;
    }

    /////////////////////////////////////////////////////////////////
    // NativeComboboxView, view::MenuDelegate overrides:
    // (note that the id received is offset by kFirstMenuItemId)

    bool NativeComboboxView::IsItemChecked(int id) const
    {
        return false;
    }

    bool NativeComboboxView::IsCommandEnabled(int id) const
    {
        return true;
    }

    void NativeComboboxView::ExecuteCommand(int id)
    {
        // revert menu offset to map back to combobox model
        id -= kFirstMenuItemId;
        DCHECK_LT(id, combobox_->model()->GetItemCount());
        selected_item_ = id;
        combobox_->SelectionChanged();
        SchedulePaint();
    }

    bool NativeComboboxView::GetAccelerator(int id, Accelerator* accel)
    {
        return false;
    }

    /////////////////////////////////////////////////////////////////
    // NativeComboboxView private methods:

    const gfx::Font& NativeComboboxView::GetFont() const
    {
        ui::ResourceBundle& rb = ui::ResourceBundle::GetSharedInstance();
        return rb.GetFont(ui::ResourceBundle::BaseFont);
    }

    // tip_x and tip_y are the coordinates of the tip of an arrow head which is
    // drawn as an isoscele triangle
    // shift_x and shift_y are offset from tip_x and tip_y to specify the relative
    // positions of the 2 equal angles (ie not the angle at the tip of the arrow)
    // Note: the width of the base (the side opposite the tip) is 2 * shift_x
    void NativeComboboxView::DrawArrow(gfx::Canvas* canvas,
        int tip_x,
        int tip_y,
        int shift_x,
        int shift_y) const
    {
        SkPaint paint;
        paint.setStyle(SkPaint::kStrokeAndFill_Style);
        paint.setColor(kTextColor);
        paint.setAntiAlias(true);
        gfx::Path path;
        path.incReserve(4);
        path.moveTo(SkIntToScalar(tip_x), SkIntToScalar(tip_y));
        path.lineTo(SkIntToScalar(tip_x+shift_x), SkIntToScalar(tip_y+shift_y));
        path.lineTo(SkIntToScalar(tip_x-shift_x), SkIntToScalar(tip_y+shift_y));
        path.close();
        canvas->AsCanvasSkia()->drawPath(path, paint);
    }

    void NativeComboboxView::PaintText(gfx::Canvas* canvas)
    {
        gfx::Insets insets = GetInsets();

        canvas->Save();
        canvas->ClipRectInt(insets.left(), insets.top(),
            width()-insets.width(), height()-insets.height());

        int x = insets.left();
        int y = insets.top();
        int text_height = height() - insets.height();
        SkColor text_color = kTextColor;

        int index = GetSelectedItem();
        if(index<0 || index>combobox_->model()->GetItemCount())
        {
            index = 0;
        }
        string16 text = combobox_->model()->GetItemAt(index);

        const gfx::Font& font = GetFont();
        int width = font.GetStringWidth(text);

        canvas->DrawStringInt(text, font, text_color, x, y, width, text_height);

        // draw the double arrow
        gfx::Rect lb = GetLocalBounds();
        DrawArrow(canvas,
            lb.width()-(kComboboxArrowSize/2)-kComboboxArrowOffset,
            lb.height()/2-kComboboxArrowSize,
            kComboboxArrowSize/2,
            kComboboxArrowSize-2);
        DrawArrow(canvas,
            lb.width()-(kComboboxArrowSize/2)-kComboboxArrowOffset,
            lb.height()/2+kComboboxArrowSize,
            -kComboboxArrowSize/2,
            -(kComboboxArrowSize-2));

        // draw the margin
        canvas->DrawLineInt(kDefaultBorderColor,
            lb.width()-kComboboxArrowSize-kComboboxArrowMargin,
            kTopInsetSize,
            lb.width()-kComboboxArrowSize-kComboboxArrowMargin,
            lb.height()-kBottomInsetSize);

        canvas->Restore();
    }

    void NativeComboboxView::ShowDropDownMenu()
    {
        if(!dropdown_list_menu_runner_.get())
        {
            UpdateFromModel();
        }

        // Extend the menu to the width of the combobox.
        SubmenuView* submenu = dropdown_list_menu_runner_->GetMenu()->CreateSubmenu();
        submenu->set_minimum_preferred_width(size().width());

        gfx::Rect lb = GetLocalBounds();
        gfx::Point menu_position(lb.origin());
        View::ConvertPointToScreen(this, &menu_position);
        if(menu_position.x() < 0)
        {
            menu_position.set_x(0);
        }

        gfx::Rect bounds(menu_position, lb.size());

        dropdown_open_ = true;
        if(dropdown_list_menu_runner_->RunMenuAt(
            NULL, NULL, bounds, MenuItemView::TOPLEFT,
            MenuRunner::HAS_MNEMONICS) == MenuRunner::MENU_DELETED)
        {
            return;
        }
        dropdown_open_ = false;

        // Need to explicitly clear mouse handler so that events get sent
        // properly after the menu finishes running. If we don't do this, then
        // the first click to other parts of the UI is eaten.
        SetMouseHandler(NULL);
    }

} //namespace view