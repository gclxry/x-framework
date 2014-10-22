
#include "opaque_browser_frame_view.h"

#include "ui_gfx/canvas_skia.h"
#include "ui_gfx/font.h"
#include "ui_gfx/image/image.h"
#include "ui_gfx/path.h"
#include "ui_gfx/rect.h"

#include "ui_base/accessibility/accessible_view_state.h"
#include "ui_base/l10n/l10n_util.h"
#include "ui_base/resource/app_res_ids.h"
#include "ui_base/resource/resource_bundle.h"
#include "ui_base/theme_provider.h"

#include "view/controls/button/image_button.h"
#include "view/controls/image_view.h"
#include "view/widget/root_view.h"
#include "view/widget/widget_delegate.h"
#include "view/window/window_shape.h"

#include "../wanui_res/resource.h"

#include "browser_view.h"
#include "tab_contents.h"

namespace
{
    // The frame border is only visible in restored mode and is hardcoded to 4 px on
    // each side regardless of the system window border size.
    const int kFrameBorderThickness = 4;
    // Besides the frame border, there's another 11 px of empty space atop the
    // window in restored mode, to use to drag the window around.
    const int kNonClientRestoredExtraThickness = 11;
    // While resize areas on Windows are normally the same size as the window
    // borders, our top area is shrunk by 1 px to make it easier to move the window
    // around with our thinner top grabbable strip.  (Incidentally, our side and
    // bottom resize areas don't match the frame border thickness either -- they
    // span the whole nonclient area, so there's no "dead zone" for the mouse.)
    const int kTopResizeAdjust = 1;
    // In the window corners, the resize areas don't actually expand bigger, but the
    // 16 px at the end of each edge triggers diagonal resizing.
    const int kResizeAreaCornerSize = 16;
    // The titlebar never shrinks too short to show the caption button plus some
    // padding below it.
    const int kCaptionButtonHeightWithPadding = 19;
    // The content left/right images have a shadow built into them.
    const int kContentEdgeShadowThickness = 2;
    // The titlebar has a 2 px 3D edge along the top and bottom.
    const int kTitlebarTopAndBottomEdgeThickness = 2;
    // The icon is inset 2 px from the left frame border.
    const int kIconLeftSpacing = 2;
    // The icon never shrinks below 16 px on a side.
    const int kIconMinimumSize = 16;
    // There is a 4 px gap between the icon and the title text.
    const int kIconTitleSpacing = 4;
    // There is a 5 px gap between the title text and the caption buttons.
    const int kTitleLogoSpacing = 5;
    // The top 1 px of the tabstrip is shadow; in maximized mode we push this off
    // the top of the screen so the tabs appear flush against the screen edge.
    const int kTabstripTopShadowThickness = 1;
    // In restored mode, the New Tab button isn't at the same height as the caption
    // buttons, but the space will look cluttered if it actually slides under them,
    // so we stop it when the gap between the two is down to 5 px.
    const int kNewTabCaptionRestoredSpacing = 5;
    // In maximized mode, where the New Tab button and the caption buttons are at
    // similar vertical coordinates, we need to reserve a larger, 16 px gap to avoid
    // looking too cluttered.
    const int kNewTabCaptionMaximizedSpacing = 16;
    // How far to indent the tabstrip from the left side of the screen when there
    // is no avatar icon.
    const int kTabStripIndent = 1;

    // Converts |bounds| from |src|'s coordinate system to |dst|, and checks if
    // |pt| is contained within.
    bool ConvertedContainsCheck(gfx::Rect bounds, const view::View* src,
        const view::View* dst, const gfx::Point& pt)
    {
        DCHECK(src);
        DCHECK(dst);
        gfx::Point origin(bounds.origin());
        view::View::ConvertPointToView(src, dst, &origin);
        bounds.set_origin(origin);
        return bounds.Contains(pt);
    }

}

///////////////////////////////////////////////////////////////////////////////
// OpaqueBrowserFrameView, public:

OpaqueBrowserFrameView::OpaqueBrowserFrameView(BrowserFrame* frame,
                                               BrowserView* browser_view)
                                               : BrowserNonClientFrameView(),
                                               minimize_button_(new view::ImageButton(this)),
                                               maximize_button_(new view::ImageButton(this)),
                                               restore_button_(new view::ImageButton(this)),
                                               close_button_(new view::ImageButton(this)),
                                               window_icon_(NULL),
                                               frame_(frame),
                                               browser_view_(browser_view)
{
    ui::ThemeProvider* tp = frame_->GetThemeProvider();
    SkColor color = SkColorSetARGB(0, 0, 0, 0);
    SkBitmap* background = tp->GetBitmapNamed(IDR_THEME_WINDOW_CONTROL_BACKGROUND);
    minimize_button_->SetImage(view::CustomButton::BS_NORMAL,
        tp->GetBitmapNamed(IDR_MINIMIZE));
    minimize_button_->SetImage(view::CustomButton::BS_HOT,
        tp->GetBitmapNamed(IDR_MINIMIZE_H));
    minimize_button_->SetImage(view::CustomButton::BS_PUSHED,
        tp->GetBitmapNamed(IDR_MINIMIZE_P));
    if(browser_view_->IsBrowserTypeNormal())
    {
        minimize_button_->SetBackground(color, background,
            tp->GetBitmapNamed(IDR_MINIMIZE_BUTTON_MASK));
    }
    minimize_button_->SetAccessibleName(
        ui::GetStringUTF16(IDS_ACCNAME_MINIMIZE));
    AddChildView(minimize_button_);

    maximize_button_->SetImage(view::CustomButton::BS_NORMAL,
        tp->GetBitmapNamed(IDR_MAXIMIZE));
    maximize_button_->SetImage(view::CustomButton::BS_HOT,
        tp->GetBitmapNamed(IDR_MAXIMIZE_H));
    maximize_button_->SetImage(view::CustomButton::BS_PUSHED,
        tp->GetBitmapNamed(IDR_MAXIMIZE_P));
    if(browser_view_->IsBrowserTypeNormal())
    {
        maximize_button_->SetBackground(color, background,
            tp->GetBitmapNamed(IDR_MAXIMIZE_BUTTON_MASK));
    }
    maximize_button_->SetAccessibleName(
        ui::GetStringUTF16(IDS_ACCNAME_MAXIMIZE));
    AddChildView(maximize_button_);

    restore_button_->SetImage(view::CustomButton::BS_NORMAL,
        tp->GetBitmapNamed(IDR_RESTORE));
    restore_button_->SetImage(view::CustomButton::BS_HOT,
        tp->GetBitmapNamed(IDR_RESTORE_H));
    restore_button_->SetImage(view::CustomButton::BS_PUSHED,
        tp->GetBitmapNamed(IDR_RESTORE_P));
    if(browser_view_->IsBrowserTypeNormal())
    {
        restore_button_->SetBackground(color, background,
            tp->GetBitmapNamed(IDR_RESTORE_BUTTON_MASK));
    }
    restore_button_->SetAccessibleName(
        ui::GetStringUTF16(IDS_ACCNAME_RESTORE));
    AddChildView(restore_button_);

    close_button_->SetImage(view::CustomButton::BS_NORMAL,
        tp->GetBitmapNamed(IDR_CLOSE));
    close_button_->SetImage(view::CustomButton::BS_HOT,
        tp->GetBitmapNamed(IDR_CLOSE_H));
    close_button_->SetImage(view::CustomButton::BS_PUSHED,
        tp->GetBitmapNamed(IDR_CLOSE_P));
    if(browser_view_->IsBrowserTypeNormal())
    {
        close_button_->SetBackground(color, background,
            tp->GetBitmapNamed(IDR_CLOSE_BUTTON_MASK));
    }
    close_button_->SetAccessibleName(ui::GetStringUTF16(IDS_ACCNAME_CLOSE));
    AddChildView(close_button_);

    // Initializing the TabIconView is expensive, so only do it if we need to.
    if(browser_view_->ShouldShowWindowIcon())
    {
        window_icon_ = new TabIconView(this);
        window_icon_->set_is_light(true);
        AddChildView(window_icon_);
        window_icon_->Update();
    }
}

OpaqueBrowserFrameView::~OpaqueBrowserFrameView() {}

///////////////////////////////////////////////////////////////////////////////
// OpaqueBrowserFrameView, protected:

int OpaqueBrowserFrameView::GetReservedHeight() const
{
    return 0;
}

gfx::Rect OpaqueBrowserFrameView::GetBoundsForReservedArea() const
{
    gfx::Rect client_view_bounds = CalculateClientAreaBounds(width(), height());
    return gfx::Rect(
        client_view_bounds.x(),
        client_view_bounds.y() + client_view_bounds.height(),
        client_view_bounds.width(),
        GetReservedHeight());
}

int OpaqueBrowserFrameView::NonClientTopBorderHeight(
    bool restored,
    bool ignore_vertical_tabs) const
{
    view::WidgetDelegate* delegate = frame_->widget_delegate();
    // |delegate| may be NULL if called from callback of InputMethodChanged while
    // a window is being destroyed.
    // See more discussion at http://crosbug.com/8958
    if(delegate && delegate->ShouldShowWindowTitle())
    {
        return std::max(FrameBorderThickness(restored) + IconSize(),
            CaptionButtonY(restored) + kCaptionButtonHeightWithPadding) +
            TitlebarBottomThickness(restored);
    }

    return FrameBorderThickness(restored) -
        ((browser_view_->IsTabStripVisible() && !restored &&
        frame_->IsMaximized()) ? kTabstripTopShadowThickness : 0);
}

///////////////////////////////////////////////////////////////////////////////
// OpaqueBrowserFrameView, BrowserNonClientFrameView implementation:

gfx::Rect OpaqueBrowserFrameView::GetBoundsForTabStrip(
    view::View* tabstrip) const
{
    if(!tabstrip)
    {
        return gfx::Rect();
    }

    int tabstrip_x = NonClientBorderThickness() + kTabStripIndent;

    int maximized_spacing = kNewTabCaptionMaximizedSpacing;
    int tabstrip_width = minimize_button_->x() - tabstrip_x -
        (frame_->IsMaximized() ? maximized_spacing : kNewTabCaptionRestoredSpacing);
    return gfx::Rect(tabstrip_x, GetHorizontalTabStripVerticalOffset(false),
        std::max(0, tabstrip_width), tabstrip->GetPreferredSize().height());
}

int OpaqueBrowserFrameView::GetHorizontalTabStripVerticalOffset(
    bool restored) const
{
    return NonClientTopBorderHeight(restored, true) + ((!restored &&
        (frame_->IsMaximized() ||
        frame_->IsFullscreen())) ?
        0 : kNonClientRestoredExtraThickness);
}

void OpaqueBrowserFrameView::UpdateThrobber(bool running)
{
    if(window_icon_)
    {
        window_icon_->Update();
    }
}

gfx::Size OpaqueBrowserFrameView::GetMinimumSize()
{
    gfx::Size min_size(browser_view_->GetMinimumSize());
    int border_thickness = NonClientBorderThickness();
    min_size.Enlarge(2 * border_thickness,
        NonClientTopBorderHeight(false, false) + border_thickness);

    view::WidgetDelegate* delegate = frame_->widget_delegate();
    int min_titlebar_width = (2 * FrameBorderThickness(false)) +
        kIconLeftSpacing +
        (delegate && delegate->ShouldShowWindowIcon() ?
        (IconSize() + kTitleLogoSpacing) : 0);
    min_titlebar_width +=
        minimize_button_->GetMinimumSize().width() +
        restore_button_->GetMinimumSize().width() +
        close_button_->GetMinimumSize().width();
    min_size.set_width(std::max(min_size.width(), min_titlebar_width));
    return min_size;
}

///////////////////////////////////////////////////////////////////////////////
// OpaqueBrowserFrameView, view::NonClientFrameView implementation:

gfx::Rect OpaqueBrowserFrameView::GetBoundsForClientView() const
{
    return client_view_bounds_;
}

gfx::Rect OpaqueBrowserFrameView::GetWindowBoundsForClientBounds(
    const gfx::Rect& client_bounds) const
{
    int top_height = NonClientTopBorderHeight(false, false);
    int border_thickness = NonClientBorderThickness();
    return gfx::Rect(std::max(0, client_bounds.x() - border_thickness),
        std::max(0, client_bounds.y() - top_height),
        client_bounds.width() + (2 * border_thickness),
        client_bounds.height() + top_height + border_thickness);
}

int OpaqueBrowserFrameView::NonClientHitTest(const gfx::Point& point)
{
    if(!bounds().Contains(point))
    {
        return HTNOWHERE;
    }

    int frame_component = frame_->client_view()->NonClientHitTest(point);

    // See if we're in the sysmenu region.  We still have to check the tabstrip
    // first so that clicks in a tab don't get treated as sysmenu clicks.
    gfx::Rect sysmenu_rect(IconBounds());
    // In maximized mode we extend the rect to the screen corner to take advantage
    // of Fitts' Law.
    if(frame_->IsMaximized())
    {
        sysmenu_rect.SetRect(0, 0, sysmenu_rect.right(), sysmenu_rect.bottom());
    }
    sysmenu_rect.set_x(GetMirroredXForRect(sysmenu_rect));
    if(sysmenu_rect.Contains(point))
    {
        return (frame_component == HTCLIENT) ? HTCLIENT : HTSYSMENU;
    }

    if(frame_component != HTNOWHERE)
    {
        return frame_component;
    }

    // Then see if the point is within any of the window controls.
    if(close_button_->IsVisible() &&
        close_button_->GetMirroredBounds().Contains(point))
    {
        return HTCLOSE;
    }
    if(restore_button_->IsVisible() &&
        restore_button_->GetMirroredBounds().Contains(point))
    {
        return HTMAXBUTTON;
    }
    if(maximize_button_->IsVisible() &&
        maximize_button_->GetMirroredBounds().Contains(point))
    {
        return HTMAXBUTTON;
    }
    if(minimize_button_->IsVisible() &&
        minimize_button_->GetMirroredBounds().Contains(point))
    {
        return HTMINBUTTON;
    }

    view::WidgetDelegate* delegate = frame_->widget_delegate();
    if(!delegate)
    {
        LOG(WARNING) << "delegate is NULL, returning safe default.";
        return HTCAPTION;
    }
    int window_component = GetHTComponentForFrame(point, TopResizeHeight(),
        NonClientBorderThickness(), kResizeAreaCornerSize, kResizeAreaCornerSize,
        delegate->CanResize());
    // Fall back to the caption if no other component matches.
    return (window_component == HTNOWHERE) ? HTCAPTION : window_component;
}

void OpaqueBrowserFrameView::GetWindowMask(const gfx::Size& size,
                                           gfx::Path* window_mask)
{
    DCHECK(window_mask);

    if(frame_->IsMaximized() || frame_->IsFullscreen())
    {
        return;
    }

    view::GetDefaultWindowMask(size, window_mask);
}

void OpaqueBrowserFrameView::EnableClose(bool enable)
{
    close_button_->SetEnabled(enable);
}

void OpaqueBrowserFrameView::ResetWindowControls()
{
    restore_button_->SetState(view::CustomButton::BS_NORMAL);
    minimize_button_->SetState(view::CustomButton::BS_NORMAL);
    maximize_button_->SetState(view::CustomButton::BS_NORMAL);
    // The close button isn't affected by this constraint.
}

void OpaqueBrowserFrameView::UpdateWindowIcon()
{
    window_icon_->SchedulePaint();
}

///////////////////////////////////////////////////////////////////////////////
// OpaqueBrowserFrameView, view::View overrides:

void OpaqueBrowserFrameView::OnPaint(gfx::Canvas* canvas)
{
    if(frame_->IsFullscreen())
    {
        return; // Nothing is visible, so don't bother to paint.
    }

    if(frame_->IsMaximized())
    {
        PaintMaximizedFrameBorder(canvas);
    }
    else
    {
        PaintRestoredFrameBorder(canvas);
    }
    PaintTitleBar(canvas);
    if(browser_view_->IsToolbarVisible())
    {
        PaintToolbarBackground(canvas);
    }
    if(!frame_->IsMaximized())
    {
        PaintRestoredClientEdge(canvas);
    }
}

void OpaqueBrowserFrameView::Layout()
{
    LayoutWindowControls();
    LayoutTitleBar();
    client_view_bounds_ = CalculateClientAreaBounds(width(), height());
}

bool OpaqueBrowserFrameView::HitTest(const gfx::Point& l) const
{
    // If the point is outside the bounds of the client area, claim it.
    bool in_nonclient = NonClientFrameView::HitTest(l);
    if(in_nonclient)
    {
        return in_nonclient;
    }

    // Otherwise claim it only if it's in a non-tab portion of the tabstrip.
    if(!browser_view_->tabstrip())
    {
        return false;
    }
    gfx::Rect tabstrip_bounds(browser_view_->tabstrip()->bounds());
    gfx::Point tabstrip_origin(tabstrip_bounds.origin());
    View::ConvertPointToView(frame_->client_view(), this, &tabstrip_origin);
    tabstrip_bounds.set_origin(tabstrip_origin);
    if(l.y() > tabstrip_bounds.bottom())
    {
        return false;
    }

    // We convert from our parent's coordinates since we assume we fill its bounds
    // completely. We need to do this since we're not a parent of the tabstrip,
    // meaning ConvertPointToView would otherwise return something bogus.
    gfx::Point browser_view_point(l);
    View::ConvertPointToView(parent(), browser_view_, &browser_view_point);
    return browser_view_->IsPositionInWindowCaption(browser_view_point);
}

void OpaqueBrowserFrameView::GetAccessibleState(
    ui::AccessibleViewState* state)
{
    state->role = ui::AccessibilityTypes::ROLE_TITLEBAR;
}

///////////////////////////////////////////////////////////////////////////////
// OpaqueBrowserFrameView, view::ButtonListener implementation:

void OpaqueBrowserFrameView::ButtonPressed(view::Button* sender,
                                           const view::Event& event)
{
    if(sender == minimize_button_)
    {
        frame_->Minimize();
    }
    else if(sender == maximize_button_)
    {
        frame_->Maximize();
    }
    else if(sender == restore_button_)
    {
        frame_->Restore();
    }
    else if(sender == close_button_)
    {
        frame_->Close();
    }
}

///////////////////////////////////////////////////////////////////////////////
// OpaqueBrowserFrameView, TabIconView::TabContentsProvider implementation:

bool OpaqueBrowserFrameView::ShouldTabIconViewAnimate() const
{
    // This function is queried during the creation of the window as the
    // TabIconView we host is initialized, so we need to NULL check the selected
    // TabContents because in this condition there is not yet a selected tab.
    TabContents* current_tab = browser_view_->GetSelectedTabContents();
    return current_tab ? current_tab->IsLoading() : false;
}

SkBitmap OpaqueBrowserFrameView::GetFaviconForTabIconView()
{
    view::WidgetDelegate* delegate = frame_->widget_delegate();
    if(!delegate)
    {
        LOG(WARNING) << "delegate is NULL, returning safe default.";
        return SkBitmap();
    }
    return delegate->GetWindowIcon();
}

///////////////////////////////////////////////////////////////////////////////
// OpaqueBrowserFrameView, private:

int OpaqueBrowserFrameView::FrameBorderThickness(bool restored) const
{
    return (!restored && (frame_->IsMaximized() || frame_->IsFullscreen())) ?
        0 : kFrameBorderThickness;
}

int OpaqueBrowserFrameView::TopResizeHeight() const
{
    return FrameBorderThickness(false) - kTopResizeAdjust;
}

int OpaqueBrowserFrameView::NonClientBorderThickness() const
{
    // When we fill the screen, we don't show a client edge.
    return FrameBorderThickness(false) +
        ((frame_->IsMaximized() || frame_->IsFullscreen()) ?
        0 : kClientEdgeThickness);
}

void OpaqueBrowserFrameView::ModifyMaximizedFramePainting(
    int* theme_offset, SkBitmap** left_corner, SkBitmap** right_corner) {}

int OpaqueBrowserFrameView::CaptionButtonY(bool restored) const
{
    // Maximized buttons start at window top so that even if their images aren't
    // drawn flush with the screen edge, they still obey Fitts' Law.
    return (!restored && frame_->IsMaximized()) ?
        FrameBorderThickness(false) : kFrameShadowThickness;
}

int OpaqueBrowserFrameView::TitlebarBottomThickness(bool restored) const
{
    return kTitlebarTopAndBottomEdgeThickness +
        ((!restored && frame_->IsMaximized()) ? 0 : kClientEdgeThickness);
}

int OpaqueBrowserFrameView::IconSize() const
{
    // This metric scales up if either the titlebar height or the titlebar font
    // size are increased.
    return GetSystemMetrics(SM_CYSMICON);
}

gfx::Rect OpaqueBrowserFrameView::IconBounds() const
{
    int size = IconSize();
    int frame_thickness = FrameBorderThickness(false);
    int y;
    view::WidgetDelegate* delegate = frame_->widget_delegate();
    if(delegate && (delegate->ShouldShowWindowIcon() ||
        delegate->ShouldShowWindowTitle()))
    {
        // Our frame border has a different "3D look" than Windows'.  Theirs has a
        // more complex gradient on the top that they push their icon/title below;
        // then the maximized window cuts this off and the icon/title are centered
        // in the remaining space.  Because the apparent shape of our border is
        // simpler, using the same positioning makes things look slightly uncentered
        // with restored windows, so when the window is restored, instead of
        // calculating the remaining space from below the frame border, we calculate
        // from below the 3D edge.
        int unavailable_px_at_top = frame_->IsMaximized() ? frame_thickness :
            kTitlebarTopAndBottomEdgeThickness;
        // When the icon is shorter than the minimum space we reserve for the
        // caption button, we vertically center it.  We want to bias rounding to put
        // extra space above the icon, since the 3D edge (+ client edge, for
        // restored windows) below looks (to the eye) more like additional space
        // than does the 3D edge (or nothing at all, for maximized windows) above;
        // hence the +1.
        y = unavailable_px_at_top + (NonClientTopBorderHeight(false, false) -
            unavailable_px_at_top - size - TitlebarBottomThickness(false) + 1) / 2;
    }
    else
    {
        // For "browser mode" windows, we use the native positioning, which is just
        // below the top frame border.
        y = frame_thickness;
    }
    return gfx::Rect(frame_thickness + kIconLeftSpacing, y, size, size);
}

void OpaqueBrowserFrameView::PaintRestoredFrameBorder(gfx::Canvas* canvas)
{
    ui::ThemeProvider* tp = GetThemeProvider();

    SkBitmap* top_left_corner = tp->GetBitmapNamed(IDR_WINDOW_TOP_LEFT_CORNER);
    SkBitmap* top_right_corner = tp->GetBitmapNamed(IDR_WINDOW_TOP_RIGHT_CORNER);
    SkBitmap* top_edge = tp->GetBitmapNamed(IDR_WINDOW_TOP_CENTER);
    SkBitmap* right_edge = tp->GetBitmapNamed(IDR_WINDOW_RIGHT_SIDE);
    SkBitmap* left_edge = tp->GetBitmapNamed(IDR_WINDOW_LEFT_SIDE);
    SkBitmap* bottom_left_corner = tp->GetBitmapNamed(IDR_WINDOW_BOTTOM_LEFT_CORNER);
    SkBitmap* bottom_right_corner = tp->GetBitmapNamed(IDR_WINDOW_BOTTOM_RIGHT_CORNER);
    SkBitmap* bottom_edge = tp->GetBitmapNamed(IDR_WINDOW_BOTTOM_CENTER);

    // Fill with the frame color first so we have a constant background for
    // areas not covered by the theme image.
    SkBitmap* theme_frame = GetFrameBitmap();
    int top_area_height = theme_frame->height();
    if(browser_view_->IsTabStripVisible())
    {
        top_area_height = std::max(top_area_height,
            GetBoundsForTabStrip(browser_view_->tabstrip()).bottom());
    }
    SkColor frame_color = GetFrameColor();
    canvas->FillRectInt(frame_color, 0, 0, width(), top_area_height);

    // Now fill down the sides.
    canvas->FillRectInt(frame_color, 0, top_area_height, left_edge->width(),
        height() - top_area_height);
    canvas->FillRectInt(frame_color, width() - right_edge->width(),
        top_area_height, right_edge->width(),
        height() - top_area_height);
    // Now fill the bottom area.
    canvas->FillRectInt(frame_color, left_edge->width(),
        height() - bottom_edge->height(),
        width() - left_edge->width() - right_edge->width(),
        bottom_edge->height());

    // Draw the theme frame.
    canvas->TileImageInt(*theme_frame, 0, 0, width(), theme_frame->height());

    // Draw the theme frame overlay.
    //if(tp->HasCustomImage(IDR_THEME_FRAME_OVERLAY) &&
    //    browser_view_->IsBrowserTypeNormal())
    //{
    //    canvas->DrawBitmapInt(*tp->GetBitmapNamed(ShouldPaintAsActive() ?
    //        IDR_THEME_FRAME_OVERLAY : IDR_THEME_FRAME_OVERLAY_INACTIVE), 0, 0);
    //}

    // Top.
    int top_left_height = std::min(top_left_corner->height(),
        height() - bottom_left_corner->height());
    canvas->DrawBitmapInt(*top_left_corner, 0, 0, top_left_corner->width(),
        top_left_height, 0, 0, top_left_corner->width(), top_left_height, false);
    canvas->TileImageInt(*top_edge, top_left_corner->width(), 0,
        width() - top_right_corner->width(), top_edge->height());
    int top_right_height = std::min(top_right_corner->height(),
        height() - bottom_right_corner->height());
    canvas->DrawBitmapInt(*top_right_corner, 0, 0, top_right_corner->width(),
        top_right_height, width() - top_right_corner->width(), 0,
        top_right_corner->width(), top_right_height, false);
    // Note: When we don't have a toolbar, we need to draw some kind of bottom
    // edge here.  Because the App Window graphics we use for this have an
    // attached client edge and their sizing algorithm is a little involved, we do
    // all this in PaintRestoredClientEdge().

    // Right.
    canvas->TileImageInt(*right_edge, width() - right_edge->width(),
        top_right_height, right_edge->width(),
        height() - top_right_height - bottom_right_corner->height());

    // Bottom.
    canvas->DrawBitmapInt(*bottom_right_corner,
        width() - bottom_right_corner->width(),
        height() - bottom_right_corner->height());
    canvas->TileImageInt(*bottom_edge, bottom_left_corner->width(),
        height() - bottom_edge->height(),
        width() - bottom_left_corner->width() - bottom_right_corner->width(),
        bottom_edge->height());
    canvas->DrawBitmapInt(*bottom_left_corner, 0,
        height() - bottom_left_corner->height());

    // Left.
    canvas->TileImageInt(*left_edge, 0, top_left_height, left_edge->width(),
        height() - top_left_height - bottom_left_corner->height());
}

void OpaqueBrowserFrameView::PaintMaximizedFrameBorder(gfx::Canvas* canvas)
{
    ui::ThemeProvider* tp = GetThemeProvider();

    // Allow customization of these attributes.
    SkBitmap* left = NULL;
    SkBitmap* right = NULL;
    int top_offset = 0;
    ModifyMaximizedFramePainting(&top_offset, &left, &right);

    // We will be painting from top_offset to top_offset + theme_frame_height. If
    // this is less than GetBoundsForTabStrip.bottom, we need to paint the frame
    // color.
    SkBitmap* theme_frame = GetFrameBitmap();
    int theme_frame_bottom = top_offset + theme_frame->height();
    int top_area_height =
        GetBoundsForTabStrip(browser_view_->tabstrip()).bottom();
    if(top_area_height > theme_frame_bottom)
    {
        SkColor frame_color = GetFrameColor();
        canvas->FillRectInt(frame_color, 0, 0, width(), top_area_height);
    }

    // Draw the theme frame.  It must be aligned with the tabstrip as if we were
    // in restored mode.  Note that the top of the tabstrip is
    // kTabstripTopShadowThickness px off the top of the screen.
    int theme_background_y = -(GetHorizontalTabStripVerticalOffset(true) +
        kTabstripTopShadowThickness);
    int left_offset = 0, right_offset = 0;

    if(left || right)
    {
        // If we have either a left or right we should have both.
        DCHECK(left && right);
        left_offset = left->width();
        right_offset = right->width();
        canvas->DrawBitmapInt(*left, 0, 0);
        canvas->DrawBitmapInt(*right, width() - right_offset, 0);
    }

    canvas->TileImageInt(*theme_frame,
        left_offset,
        top_offset,
        width() - (left_offset + right_offset),
        theme_frame->height());
    // Draw the theme frame overlay
    //if(tp->HasCustomImage(IDR_THEME_FRAME_OVERLAY) &&
    //    browser_view_->IsBrowserTypeNormal())
    //{
    //    SkBitmap* theme_overlay = tp->GetBitmapNamed(ShouldPaintAsActive() ?
    //        IDR_THEME_FRAME_OVERLAY : IDR_THEME_FRAME_OVERLAY_INACTIVE);
    //    canvas->DrawBitmapInt(*theme_overlay, 0, theme_background_y);
    //}

    if(!browser_view_->IsToolbarVisible())
    {
        // There's no toolbar to edge the frame border, so we need to draw a bottom
        // edge.  The graphic we use for this has a built in client edge, so we clip
        // it off the bottom.
        SkBitmap* top_center = tp->GetBitmapNamed(IDR_APP_TOP_CENTER);
        int edge_height = top_center->height() - kClientEdgeThickness;
        canvas->TileImageInt(*top_center, 0,
            frame_->client_view()->y() - edge_height, width(), edge_height);
    }
}

void OpaqueBrowserFrameView::PaintTitleBar(gfx::Canvas* canvas)
{
    // The window icon is painted by the TabIconView.
    view::WidgetDelegate* delegate = frame_->widget_delegate();
    if(!delegate)
    {
        LOG(WARNING) << "delegate is NULL";
        return;
    }
    if(delegate->ShouldShowWindowTitle())
    {
        canvas->DrawStringInt(delegate->GetWindowTitle(),
            BrowserFrame::GetTitleFont(),
            SK_ColorWHITE,
            GetMirroredXForRect(title_bounds_),
            title_bounds_.y(),
            title_bounds_.width(),
            title_bounds_.height());
        /* TODO(pkasting):  If this window is active, we should also draw a drop
        * shadow on the title.  This is tricky, because we don't want to hardcode a
        * shadow color (since we want to work with various themes), but we can't
        * alpha-blend either (since the Windows text APIs don't really do this).
        * So we'd need to sample the background color at the right location and
        * synthesize a good shadow color. */
    }
}

void OpaqueBrowserFrameView::PaintToolbarBackground(gfx::Canvas* canvas)
{
    gfx::Rect toolbar_bounds(browser_view_->GetToolbarBounds());
    if(toolbar_bounds.IsEmpty())
    {
        return;
    }
    gfx::Point toolbar_origin(toolbar_bounds.origin());
    ConvertPointToView(browser_view_, this, &toolbar_origin);
    toolbar_bounds.set_origin(toolbar_origin);

    int x = toolbar_bounds.x();
    int w = toolbar_bounds.width();
    int y, h;
    y = toolbar_bounds.y();
    h = toolbar_bounds.bottom();

    // Gross hack: We split the toolbar images into two pieces, since sometimes
    // (popup mode) the toolbar isn't tall enough to show the whole image.  The
    // split happens between the top shadow section and the bottom gradient
    // section so that we never break the gradient.
    int split_point = kFrameShadowThickness * 2;
    int bottom_y = y + split_point;
    ui::ThemeProvider* tp = GetThemeProvider();
    SkBitmap* toolbar_left = tp->GetBitmapNamed(IDR_CONTENT_TOP_LEFT_CORNER);
    int bottom_edge_height = std::min(toolbar_left->height(), h) - split_point;

    // Split our canvas out so we can mask out the corners of the toolbar
    // without masking out the frame.
    canvas->SaveLayerAlpha(
        255, gfx::Rect(x - kClientEdgeThickness, y, w + kClientEdgeThickness * 3,
        h));
    canvas->AsCanvasSkia()->drawARGB(0, 255, 255, 255, SkXfermode::kClear_Mode);

    SkColor theme_toolbar_color = SkColorSetRGB(230, 230, 230);
    canvas->FillRectInt(theme_toolbar_color, x, bottom_y, w, bottom_edge_height);

    // Tile the toolbar image starting at the frame edge on the left and where the
    // horizontal tabstrip is (or would be) on the top.
    SkBitmap* theme_toolbar = tp->GetBitmapNamed(IDR_THEME_TOOLBAR);
    canvas->TileImageInt(*theme_toolbar, x,
        bottom_y - GetHorizontalTabStripVerticalOffset(false), x,
        bottom_y, w, theme_toolbar->height());

    // Draw rounded corners for the tab.
    SkBitmap* toolbar_left_mask =
        tp->GetBitmapNamed(IDR_CONTENT_TOP_LEFT_CORNER_MASK);
    SkBitmap* toolbar_right_mask =
        tp->GetBitmapNamed(IDR_CONTENT_TOP_RIGHT_CORNER_MASK);

    // We mask out the corners by using the DestinationIn transfer mode,
    // which keeps the RGB pixels from the destination and the alpha from
    // the source.
    SkPaint paint;
    paint.setXfermodeMode(SkXfermode::kDstIn_Mode);

    // Mask the left edge.
    int left_x = x - kContentEdgeShadowThickness;
    canvas->DrawBitmapInt(*toolbar_left_mask, 0, 0, toolbar_left_mask->width(),
        split_point, left_x, y, toolbar_left_mask->width(),
        split_point, false, paint);
    canvas->DrawBitmapInt(*toolbar_left_mask, 0,
        toolbar_left_mask->height() - bottom_edge_height,
        toolbar_left_mask->width(), bottom_edge_height, left_x, bottom_y,
        toolbar_left_mask->width(), bottom_edge_height, false, paint);

    // Mask the right edge.
    int right_x =
        x + w - toolbar_right_mask->width() + kContentEdgeShadowThickness;
    canvas->DrawBitmapInt(*toolbar_right_mask, 0, 0, toolbar_right_mask->width(),
        split_point, right_x, y, toolbar_right_mask->width(),
        split_point, false, paint);
    canvas->DrawBitmapInt(*toolbar_right_mask, 0,
        toolbar_right_mask->height() - bottom_edge_height,
        toolbar_right_mask->width(), bottom_edge_height, right_x, bottom_y,
        toolbar_right_mask->width(), bottom_edge_height, false, paint);
    canvas->Restore();

    canvas->DrawBitmapInt(*toolbar_left, 0, 0, toolbar_left->width(), split_point,
        left_x, y, toolbar_left->width(), split_point, false);
    canvas->DrawBitmapInt(*toolbar_left, 0,
        toolbar_left->height() - bottom_edge_height, toolbar_left->width(),
        bottom_edge_height, left_x, bottom_y, toolbar_left->width(),
        bottom_edge_height, false);

    SkBitmap* toolbar_center = tp->GetBitmapNamed(IDR_CONTENT_TOP_CENTER);
    canvas->TileImageInt(*toolbar_center, 0, 0, left_x + toolbar_left->width(),
        y, right_x - (left_x + toolbar_left->width()),
        split_point);

    SkBitmap* toolbar_right = tp->GetBitmapNamed(IDR_CONTENT_TOP_RIGHT_CORNER);
    canvas->DrawBitmapInt(*toolbar_right, 0, 0, toolbar_right->width(),
        split_point, right_x, y, toolbar_right->width(), split_point, false);
    canvas->DrawBitmapInt(*toolbar_right, 0,
        toolbar_right->height() - bottom_edge_height, toolbar_right->width(),
        bottom_edge_height, right_x, bottom_y, toolbar_right->width(),
        bottom_edge_height, false);

    // Draw the content/toolbar separator.
    canvas->FillRectInt(ui::ResourceBundle::toolbar_separator_color,
        x + kClientEdgeThickness, toolbar_bounds.bottom() - kClientEdgeThickness,
        w - (2 * kClientEdgeThickness), kClientEdgeThickness);
}

void OpaqueBrowserFrameView::PaintRestoredClientEdge(gfx::Canvas* canvas)
{
    ui::ThemeProvider* tp = GetThemeProvider();
    int client_area_top = frame_->client_view()->y();
    int image_top = client_area_top;

    gfx::Rect client_area_bounds = CalculateClientAreaBounds(width(), height());
    SkColor toolbar_color = SkColorSetRGB(230, 230, 230);

    if(browser_view_->IsToolbarVisible())
    {
        // The client edge images always start below the toolbar corner images.  The
        // client edge filled rects start there or at the bottom of the toolbar,
        // whichever is shorter.
        gfx::Rect toolbar_bounds(browser_view_->GetToolbarBounds());
        image_top += toolbar_bounds.y() +
            tp->GetBitmapNamed(IDR_CONTENT_TOP_LEFT_CORNER)->height();
        client_area_top = std::min(image_top,
            client_area_top + toolbar_bounds.bottom() - kClientEdgeThickness);
    }
    else if(!browser_view_->IsTabStripVisible())
    {
        // The toolbar isn't going to draw a client edge for us, so draw one
        // ourselves.
        SkBitmap* top_left = tp->GetBitmapNamed(IDR_APP_TOP_LEFT);
        SkBitmap* top_center = tp->GetBitmapNamed(IDR_APP_TOP_CENTER);
        SkBitmap* top_right = tp->GetBitmapNamed(IDR_APP_TOP_RIGHT);
        int top_edge_y = client_area_top - top_center->height();
        int height = client_area_top - top_edge_y;

        canvas->DrawBitmapInt(*top_left, 0, 0, top_left->width(), height,
            client_area_bounds.x() - top_left->width(), top_edge_y,
            top_left->width(), height, false);
        canvas->TileImageInt(*top_center, 0, 0, client_area_bounds.x(), top_edge_y,
            client_area_bounds.width(), std::min(height, top_center->height()));
        canvas->DrawBitmapInt(*top_right, 0, 0, top_right->width(), height,
            client_area_bounds.right(), top_edge_y,
            top_right->width(), height, false);

        // Draw the toolbar color across the top edge.
        canvas->FillRectInt(toolbar_color,
            client_area_bounds.x() - kClientEdgeThickness,
            client_area_top - kClientEdgeThickness,
            client_area_bounds.width() + (2 * kClientEdgeThickness),
            kClientEdgeThickness);
    }

    int client_area_bottom =
        std::max(client_area_top, height() - NonClientBorderThickness());
    int image_height = client_area_bottom - image_top;

    // Draw the client edge images.
    // Draw the client edge images.
    SkBitmap* right = tp->GetBitmapNamed(IDR_CONTENT_RIGHT_SIDE);
    canvas->TileImageInt(*right, client_area_bounds.right(), image_top,
        right->width(), image_height);
    canvas->DrawBitmapInt(
        *tp->GetBitmapNamed(IDR_CONTENT_BOTTOM_RIGHT_CORNER),
        client_area_bounds.right(), client_area_bottom);
    SkBitmap* bottom = tp->GetBitmapNamed(IDR_CONTENT_BOTTOM_CENTER);
    canvas->TileImageInt(*bottom, client_area_bounds.x(),
        client_area_bottom, client_area_bounds.width(),
        bottom->height());
    SkBitmap* bottom_left =
        tp->GetBitmapNamed(IDR_CONTENT_BOTTOM_LEFT_CORNER);
    canvas->DrawBitmapInt(*bottom_left,
        client_area_bounds.x() - bottom_left->width(), client_area_bottom);
    SkBitmap* left = tp->GetBitmapNamed(IDR_CONTENT_LEFT_SIDE);
    canvas->TileImageInt(*left, client_area_bounds.x() - left->width(),
        image_top, left->width(), image_height);

    // Draw the toolbar color so that the client edges show the right color even
    // where not covered by the toolbar image.  NOTE: We do this after drawing the
    // images because the images are meant to alpha-blend atop the frame whereas
    // these rects are meant to be fully opaque, without anything overlaid.
    canvas->FillRectInt(toolbar_color,
        client_area_bounds.x() - kClientEdgeThickness, client_area_top,
        kClientEdgeThickness,
        client_area_bottom + kClientEdgeThickness - client_area_top);
    canvas->FillRectInt(toolbar_color, client_area_bounds.x(), client_area_bottom,
        client_area_bounds.width(), kClientEdgeThickness);
    canvas->FillRectInt(toolbar_color, client_area_bounds.right(),
        client_area_top, kClientEdgeThickness,
        client_area_bottom + kClientEdgeThickness - client_area_top);
}

SkBitmap* OpaqueBrowserFrameView::GetFrameBitmap() const
{
    int resource_id;
    if(browser_view_->IsBrowserTypeNormal())
    {
        if(ShouldPaintAsActive())
        {
            resource_id = IDR_THEME_FRAME;
        }
        else
        {
            resource_id = IDR_THEME_FRAME_INACTIVE;
        }
        return GetThemeProvider()->GetBitmapNamed(resource_id);
    }
    // Never theme app and popup windows.
    ui::ResourceBundle& rb = ui::ResourceBundle::GetSharedInstance();
    if(ShouldPaintAsActive())
    {
        resource_id = IDR_FRAME;
    }
    else
    {
        resource_id = IDR_THEME_FRAME_INACTIVE;
    }
    return rb.GetBitmapNamed(resource_id);
}

SkColor OpaqueBrowserFrameView::GetFrameColor() const
{
    if(browser_view_->IsBrowserTypeNormal())
    {
        if(ShouldPaintAsActive())
        {
            return SkColorSetRGB(66, 116, 201);
        }
        return SkColorSetRGB(161, 182, 228);
    }
    // Never theme app and popup windows.
    if(ShouldPaintAsActive())
    {
        return ui::ResourceBundle::frame_color;
    }
    return ui::ResourceBundle::frame_color_inactive;
}

void OpaqueBrowserFrameView::LayoutWindowControls()
{
    bool is_maximized = frame_->IsMaximized();
    close_button_->SetImageAlignment(view::ImageButton::ALIGN_LEFT,
        view::ImageButton::ALIGN_BOTTOM);
    int caption_y = CaptionButtonY(false);
    // There should always be the same number of non-shadow pixels visible to the
    // side of the caption buttons.  In maximized mode we extend the rightmost
    // button to the screen corner to obey Fitts' Law.
    int right_extra_width = is_maximized ?
        (kFrameBorderThickness - kFrameShadowThickness) : 0;
    gfx::Size close_button_size = close_button_->GetPreferredSize();
    close_button_->SetBounds(width() - FrameBorderThickness(false) -
        right_extra_width - close_button_size.width(), caption_y,
        close_button_size.width() + right_extra_width,
        close_button_size.height());

    // When the window is restored, we show a maximized button; otherwise, we show
    // a restore button.
    bool is_restored = !is_maximized && !frame_->IsMinimized();
    view::ImageButton* invisible_button = is_restored ? restore_button_ :
        maximize_button_;
    invisible_button->SetVisible(false);

    view::ImageButton* visible_button = is_restored ? maximize_button_ :
        restore_button_;
    visible_button->SetVisible(true);
    visible_button->SetImageAlignment(view::ImageButton::ALIGN_LEFT,
        view::ImageButton::ALIGN_BOTTOM);
    gfx::Size visible_button_size = visible_button->GetPreferredSize();
    visible_button->SetBounds(close_button_->x() - visible_button_size.width(),
        caption_y, visible_button_size.width(),
        visible_button_size.height());

    minimize_button_->SetVisible(true);
    minimize_button_->SetImageAlignment(view::ImageButton::ALIGN_LEFT,
        view::ImageButton::ALIGN_BOTTOM);
    gfx::Size minimize_button_size = minimize_button_->GetPreferredSize();
    minimize_button_->SetBounds(
        visible_button->x() - minimize_button_size.width(), caption_y,
        minimize_button_size.width(),
        minimize_button_size.height());
}

void OpaqueBrowserFrameView::LayoutTitleBar()
{
    // The window title is based on the calculated icon position, even when there
    // is no icon.
    gfx::Rect icon_bounds(IconBounds());
    view::WidgetDelegate* delegate = frame_->widget_delegate();
    if(delegate && delegate->ShouldShowWindowIcon())
    {
        window_icon_->SetBoundsRect(icon_bounds);
    }

    // Size the title, if visible.
    if(delegate && delegate->ShouldShowWindowTitle())
    {
        int title_x = delegate->ShouldShowWindowIcon() ?
            icon_bounds.right() + kIconTitleSpacing : icon_bounds.x();
        int title_height = BrowserFrame::GetTitleFont().GetHeight();
        // We bias the title position so that when the difference between the icon
        // and title heights is odd, the extra pixel of the title is above the
        // vertical midline rather than below.  This compensates for how the icon is
        // already biased downwards (see IconBounds()) and helps prevent descenders
        // on the title from overlapping the 3D edge at the bottom of the titlebar.
        title_bounds_.SetRect(title_x,
            icon_bounds.y() + ((icon_bounds.height() - title_height - 1) / 2),
            std::max(0, minimize_button_->x() - kTitleLogoSpacing - title_x),
            title_height);
    }
}

gfx::Rect OpaqueBrowserFrameView::CalculateClientAreaBounds(int width,
                                                            int height) const
{
    int top_height = NonClientTopBorderHeight(false, false);
    int border_thickness = NonClientBorderThickness();
    return gfx::Rect(border_thickness, top_height,
        std::max(0, width - (2 * border_thickness)),
        std::max(0, height - GetReservedHeight() -
        top_height - border_thickness));
}