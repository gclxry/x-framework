
#include "glass_browser_frame_view.h"

#include "base/command_line.h"
#include "base/utf_string_conversions.h"

#include "SkPaint.h"

#include "ui_gfx/canvas.h"
#include "ui_gfx/icon_util.h"
#include "ui_gfx/image/image.h"

#include "ui_base/resource/app_res_ids.h"
#include "ui_base/resource/resource_bundle.h"
#include "ui_base/theme_provider.h"

#include "view/window/client_view.h"

#include "../wanui_res/resource.h"

#include "browser_view.h"

HICON GlassBrowserFrameView::throbber_icons_[GlassBrowserFrameView::kThrobberIconCount];

namespace
{
    // There are 3 px of client edge drawn inside the outer frame borders.
    const int kNonClientBorderThickness = 3;
    // Besides the frame border, there's another 11 px of empty space atop the
    // window in restored mode, to use to drag the window around.
    const int kNonClientRestoredExtraThickness = 11;
    // In the window corners, the resize areas don't actually expand bigger, but the
    // 16 px at the end of the top and bottom edges triggers diagonal resizing.
    const int kResizeAreaCornerSize = 16;
    // The content left/right images have a shadow built into them.
    const int kContentEdgeShadowThickness = 2;
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
}

///////////////////////////////////////////////////////////////////////////////
// GlassBrowserFrameView, public:

GlassBrowserFrameView::GlassBrowserFrameView(BrowserFrame* frame,
                                             BrowserView* browser_view)
                                             : BrowserNonClientFrameView(),
                                             frame_(frame),
                                             browser_view_(browser_view),
                                             throbber_running_(false),
                                             throbber_frame_(0)
{
    if(browser_view_->ShouldShowWindowIcon())
    {
        InitThrobberIcons();
    }
}

GlassBrowserFrameView::~GlassBrowserFrameView() {}

///////////////////////////////////////////////////////////////////////////////
// GlassBrowserFrameView, BrowserNonClientFrameView implementation:

gfx::Rect GlassBrowserFrameView::GetBoundsForTabStrip(
    view::View* tabstrip) const
{
    int minimize_button_offset =
        std::min(frame_->GetMinimizeButtonOffset(), width());
    int tabstrip_x = NonClientBorderThickness();
    // In RTL languages, we have moved an avatar icon left by the size of window
    // controls to prevent it from being rendered over them. So, we use its x
    // position to move this tab strip left when maximized. Also, we can render
    // a tab strip until the left end of this window without considering the size
    // of window controls in RTL languages.
    if(base::i18n::IsRTL())
    {
        minimize_button_offset = width();
    }
    int tabstrip_width = minimize_button_offset - tabstrip_x -
        (frame_->IsMaximized() ? kNewTabCaptionMaximizedSpacing :
        kNewTabCaptionRestoredSpacing);
    return gfx::Rect(tabstrip_x, GetHorizontalTabStripVerticalOffset(false),
        std::max(0, tabstrip_width),
        tabstrip->GetPreferredSize().height());
}

int GlassBrowserFrameView::GetHorizontalTabStripVerticalOffset(
    bool restored) const
{
    return NonClientTopBorderHeight(restored, true);
}

void GlassBrowserFrameView::UpdateThrobber(bool running)
{
    if(throbber_running_)
    {
        if(running)
        {
            DisplayNextThrobberFrame();
        }
        else
        {
            StopThrobber();
        }
    }
    else if(running)
    {
        StartThrobber();
    }
}

gfx::Size GlassBrowserFrameView::GetMinimumSize()
{
    gfx::Size min_size(browser_view_->GetMinimumSize());

    // Account for the client area insets.
    gfx::Insets insets = GetClientAreaInsets();
    min_size.Enlarge(insets.width(), insets.height());
    // Client area insets do not include the shadow thickness.
    min_size.Enlarge(2 * kContentEdgeShadowThickness, 0);

    // Ensure that the minimum width is enough to hold a tab strip with minimum
    // width at its usual insets.
    if(browser_view_->IsTabStripVisible())
    {
        AbstractTabStripView* tabstrip = browser_view_->tabstrip();
        int min_tabstrip_width = tabstrip->GetMinimumSize().width();
        int min_tabstrip_area_width =
            width() - GetBoundsForTabStrip(tabstrip).width() + min_tabstrip_width;
        min_size.set_width(std::max(min_tabstrip_area_width, min_size.width()));
    }

    return min_size;
}

///////////////////////////////////////////////////////////////////////////////
// GlassBrowserFrameView, view::NonClientFrameView implementation:

gfx::Rect GlassBrowserFrameView::GetBoundsForClientView() const
{
    return client_view_bounds_;
}

gfx::Rect GlassBrowserFrameView::GetWindowBoundsForClientBounds(
    const gfx::Rect& client_bounds) const
{
    HWND hwnd = frame_->GetNativeWindow();
    if(!browser_view_->IsTabStripVisible() && hwnd)
    {
        // If we don't have a tabstrip, we're either a popup or an app window, in
        // which case we have a standard size non-client area and can just use
        // AdjustWindowRectEx to obtain it. We check for a non-NULL window handle in
        // case this gets called before the window is actually created.
        RECT rect = client_bounds.ToRECT();
        AdjustWindowRectEx(&rect, GetWindowLong(hwnd, GWL_STYLE), FALSE,
            GetWindowLong(hwnd, GWL_EXSTYLE));
        return gfx::Rect(rect);
    }

    gfx::Insets insets = GetClientAreaInsets();
    return gfx::Rect(std::max(0, client_bounds.x() - insets.left()),
        std::max(0, client_bounds.y() - insets.top()),
        client_bounds.width() + insets.width(),
        client_bounds.height() + insets.height());
}

int GlassBrowserFrameView::NonClientHitTest(const gfx::Point& point)
{
    // If the browser isn't in normal mode, we haven't customized the frame, so
    // Windows can figure this out.  If the point isn't within our bounds, then
    // it's in the native portion of the frame, so again Windows can figure it
    // out.
    if(!browser_view_->IsBrowserTypeNormal() || !bounds().Contains(point))
    {
        return HTNOWHERE;
    }

    int frame_component = frame_->client_view()->NonClientHitTest(point);

    // See if we're in the sysmenu region.  We still have to check the tabstrip
    // first so that clicks in a tab don't get treated as sysmenu clicks.
    int nonclient_border_thickness = NonClientBorderThickness();
    if(gfx::Rect(nonclient_border_thickness, GetSystemMetrics(SM_CXSIZEFRAME),
        GetSystemMetrics(SM_CXSMICON),
        GetSystemMetrics(SM_CYSMICON)).Contains(point))
        return (frame_component == HTCLIENT) ? HTCLIENT : HTSYSMENU;

    if(frame_component != HTNOWHERE)
    {
        return frame_component;
    }

    int frame_border_thickness = FrameBorderThickness();
    int window_component = GetHTComponentForFrame(point, frame_border_thickness,
        nonclient_border_thickness, frame_border_thickness,
        kResizeAreaCornerSize - frame_border_thickness,
        frame_->widget_delegate()->CanResize());
    // Fall back to the caption if no other component matches.
    return (window_component == HTNOWHERE) ? HTCAPTION : window_component;
}

///////////////////////////////////////////////////////////////////////////////
// GlassBrowserFrameView, view::View overrides:

void GlassBrowserFrameView::OnPaint(gfx::Canvas* canvas)
{
    if(!browser_view_->IsTabStripVisible())
    {
        return; // Nothing is visible, so don't bother to paint.
    }

    PaintToolbarBackground(canvas);
    if(!frame_->IsMaximized())
    {
        PaintRestoredClientEdge(canvas);
    }
}

void GlassBrowserFrameView::Layout()
{
    LayoutClientView();
}

bool GlassBrowserFrameView::HitTest(const gfx::Point& l) const
{
    return !frame_->client_view()->bounds().Contains(l);
}

///////////////////////////////////////////////////////////////////////////////
// GlassBrowserFrameView, private:

int GlassBrowserFrameView::FrameBorderThickness() const
{
    return (frame_->IsMaximized() || frame_->IsFullscreen()) ?
        0 : GetSystemMetrics(SM_CXSIZEFRAME);
}

int GlassBrowserFrameView::NonClientBorderThickness() const
{
    if(frame_->IsMaximized() || frame_->IsFullscreen())
    {
        return 0;
    }

    return kNonClientBorderThickness;
}

int GlassBrowserFrameView::NonClientTopBorderHeight(
    bool restored,
    bool ignore_vertical_tabs) const
{
    if(!restored && frame_->IsFullscreen())
    {
        return 0;
    }
    return GetSystemMetrics(SM_CYSIZEFRAME) +
        ((!restored && browser_view_->IsMaximized()) ?
        -kTabstripTopShadowThickness : kNonClientRestoredExtraThickness);
}

void GlassBrowserFrameView::PaintToolbarBackground(gfx::Canvas* canvas)
{
    ui::ThemeProvider* tp = GetThemeProvider();

    gfx::Rect toolbar_bounds(browser_view_->GetToolbarBounds());
    gfx::Point toolbar_origin(toolbar_bounds.origin());
    View::ConvertPointToView(browser_view_, this, &toolbar_origin);
    toolbar_bounds.set_origin(toolbar_origin);
    int x = toolbar_bounds.x();
    int w = toolbar_bounds.width();
    int left_x = x - kContentEdgeShadowThickness;

    SkBitmap* theme_toolbar = tp->GetBitmapNamed(IDR_THEME_TOOLBAR);
    SkBitmap* toolbar_left = tp->GetBitmapNamed(IDR_CONTENT_TOP_LEFT_CORNER);
    SkBitmap* toolbar_center = tp->GetBitmapNamed(IDR_CONTENT_TOP_CENTER);

    // Tile the toolbar image starting at the frame edge on the left and where
    // the tabstrip is on the top.
    int y = toolbar_bounds.y();
    int dest_y = y + (kFrameShadowThickness * 2);
    canvas->TileImageInt(*theme_toolbar, x,
        dest_y - GetHorizontalTabStripVerticalOffset(false), x,
        dest_y, w, theme_toolbar->height());

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

    // Mask out the top left corner.
    canvas->DrawBitmapInt(*toolbar_left_mask, left_x, y, paint);

    // Mask out the top right corner.
    int right_x =
        x + w + kContentEdgeShadowThickness - toolbar_right_mask->width();
    canvas->DrawBitmapInt(*toolbar_right_mask, right_x, y, paint);

    // Draw left edge.
    canvas->DrawBitmapInt(*toolbar_left, left_x, y);

    // Draw center edge.
    canvas->TileImageInt(*toolbar_center, left_x + toolbar_left->width(), y,
        right_x - (left_x + toolbar_left->width()), toolbar_center->height());

    // Right edge.
    canvas->DrawBitmapInt(*tp->GetBitmapNamed(IDR_CONTENT_TOP_RIGHT_CORNER),
        right_x, y);

    // Draw the content/toolbar separator.
    canvas->FillRectInt(ui::ResourceBundle::toolbar_separator_color,
        x + kClientEdgeThickness, toolbar_bounds.bottom() - kClientEdgeThickness,
        w - (2 * kClientEdgeThickness), kClientEdgeThickness);
}

void GlassBrowserFrameView::PaintRestoredClientEdge(gfx::Canvas* canvas)
{
    ui::ThemeProvider* tp = GetThemeProvider();
    gfx::Rect client_area_bounds = CalculateClientAreaBounds(width(), height());

    // The client edges start below the toolbar upper corner images regardless
    // of how tall the toolbar itself is.
    int client_area_top = frame_->client_view()->y() +
        browser_view_->GetToolbarBounds().y() +
        tp->GetBitmapNamed(IDR_CONTENT_TOP_LEFT_CORNER)->height();
    int client_area_bottom =
        std::max(client_area_top, height() - NonClientBorderThickness());
    int client_area_height = client_area_bottom - client_area_top;

    // Draw the client edge images.
    SkBitmap* right = tp->GetBitmapNamed(IDR_CONTENT_RIGHT_SIDE);
    canvas->TileImageInt(*right, client_area_bounds.right(), client_area_top,
        right->width(), client_area_height);
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
        client_area_top, left->width(), client_area_height);

    // Draw the toolbar color so that the client edges show the right color even
    // where not covered by the toolbar image.  NOTE: We do this after drawing the
    // images because the images are meant to alpha-blend atop the frame whereas
    // these rects are meant to be fully opaque, without anything overlaid.
    SkColor toolbar_color = SkColorSetRGB(230, 230, 230);
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

void GlassBrowserFrameView::LayoutClientView()
{
    client_view_bounds_ = CalculateClientAreaBounds(width(), height());
}

gfx::Insets GlassBrowserFrameView::GetClientAreaInsets() const
{
    if(!browser_view_->IsTabStripVisible())
    {
        return gfx::Insets();
    }

    const int top_height = NonClientTopBorderHeight(false, false);
    const int border_thickness = NonClientBorderThickness();
    return gfx::Insets(top_height,
        border_thickness,
        border_thickness,
        border_thickness);
}

gfx::Rect GlassBrowserFrameView::CalculateClientAreaBounds(int width,
                                                           int height) const
{
    gfx::Rect bounds(0, 0, width, height);
    bounds.Inset(GetClientAreaInsets());
    return bounds;
}

void GlassBrowserFrameView::StartThrobber()
{
    if(!throbber_running_)
    {
        throbber_running_ = true;
        throbber_frame_ = 0;
        InitThrobberIcons();
        SendMessage(frame_->GetNativeWindow(), WM_SETICON,
            static_cast<WPARAM>(ICON_SMALL),
            reinterpret_cast<LPARAM>(throbber_icons_[throbber_frame_]));
    }
}

void GlassBrowserFrameView::StopThrobber()
{
    if(throbber_running_)
    {
        throbber_running_ = false;

        HICON frame_icon = NULL;

        // Check if hosted BrowserView has a window icon to use.
        if(browser_view_->ShouldShowWindowIcon())
        {
            SkBitmap icon = browser_view_->GetWindowIcon();
            if(!icon.isNull())
            {
                frame_icon = IconUtil::CreateHICONFromSkBitmap(icon);
            }
        }

        // Fallback to class icon.
        if(!frame_icon)
        {
            frame_icon = reinterpret_cast<HICON>(GetClassLongPtr(
                frame_->GetNativeWindow(), GCLP_HICONSM));
        }

        // This will reset the small icon which we set in the throbber code.
        // WM_SETICON with NULL icon restores the icon for title bar but not
        // for taskbar. See http://crbug.com/29996
        SendMessage(frame_->GetNativeWindow(), WM_SETICON,
            static_cast<WPARAM>(ICON_SMALL),
            reinterpret_cast<LPARAM>(frame_icon));
    }
}

void GlassBrowserFrameView::DisplayNextThrobberFrame()
{
    throbber_frame_ = (throbber_frame_ + 1) % kThrobberIconCount;
    SendMessage(frame_->GetNativeWindow(), WM_SETICON,
        static_cast<WPARAM>(ICON_SMALL),
        reinterpret_cast<LPARAM>(throbber_icons_[throbber_frame_]));
}

// static
void GlassBrowserFrameView::InitThrobberIcons()
{
    static bool initialized = false;
    if(!initialized)
    {
        ui::ResourceBundle &rb = ui::ResourceBundle::GetSharedInstance();
        for(int i=0; i<kThrobberIconCount; ++i)
        {
            throbber_icons_[i] = rb.LoadThemeIcon(IDI_THROBBER_01 + i);
            DCHECK(throbber_icons_[i]);
        }
        initialized = true;
    }
}