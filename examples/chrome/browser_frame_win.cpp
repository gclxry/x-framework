
#include "browser_frame_win.h"

#include <dwmapi.h>
#include <shellapi.h>

#include <set>

#include "ui_gfx/font.h"

#include "ui_base/theme_provider.h"

#include "view/view_delegate.h"
#include "view/widget/native_widget_win.h"
#include "view/widget/widget.h"
#include "view/window/non_client_view.h"

#include "browser_frame_views.h"
#include "browser_list.h"
#include "browser_view.h"

#pragma comment(lib, "dwmapi.lib")

// static
static const int kClientEdgeThickness = 3;
static const int kTabDragWindowAlpha = 200;
// We need to offset the DWMFrame into the toolbar so that the blackness
// doesn't show up on our rounded corners.
static const int kDWMFrameTopOffset = 3;
// If not -1, windows are shown with this state.
static int explicit_show_state = -1;

///////////////////////////////////////////////////////////////////////////////
// BrowserFrameWin, public:

BrowserFrameWin::BrowserFrameWin(BrowserFrame* browser_frame,
                                 BrowserView* browser_view)
                                 : view::NativeWidgetWin(browser_frame),
                                 browser_view_(browser_view),
                                 browser_frame_(browser_frame) {}

BrowserFrameWin::~BrowserFrameWin() {}

// static
void BrowserFrameWin::SetShowState(int state)
{
    explicit_show_state = state;
}

///////////////////////////////////////////////////////////////////////////////
// BrowserFrameWin, view::NativeWidgetWin overrides:

int BrowserFrameWin::GetShowState() const
{
    if(explicit_show_state != -1)
    {
        return explicit_show_state;
    }

    STARTUPINFO si = { 0 };
    si.cb = sizeof(si);
    si.dwFlags = STARTF_USESHOWWINDOW;
    GetStartupInfo(&si);
    return si.wShowWindow;
}

gfx::Insets BrowserFrameWin::GetClientAreaInsets() const
{
    // Use the default client insets for an opaque frame or a glass popup/app
    // frame.
    if(!GetWidget()->ShouldUseNativeFrame() ||
        !browser_view_->IsBrowserTypeNormal())
    {
        return NativeWidgetWin::GetClientAreaInsets();
    }

    int border_thickness = GetSystemMetrics(SM_CXSIZEFRAME);
    // In fullscreen mode, we have no frame. In restored mode, we draw our own
    // client edge over part of the default frame.
    if(IsFullscreen())
    {
        border_thickness = 0;
    }
    else if(!IsMaximized())
    {
        border_thickness -= kClientEdgeThickness;
    }
    return gfx::Insets(0, border_thickness, border_thickness, border_thickness);
}

void BrowserFrameWin::UpdateFrameAfterFrameChange()
{
    // We need to update the glass region on or off before the base class adjusts
    // the window region.
    UpdateDWMFrame();
    NativeWidgetWin::UpdateFrameAfterFrameChange();
}

void BrowserFrameWin::OnEndSession(BOOL ending, UINT logoff)
{
    BrowserList::SessionEnding();
}

void BrowserFrameWin::OnInitMenuPopup(HMENU menu, UINT position,
                                      BOOL is_system_menu)
{
    browser_view_->PrepareToRunSystemMenu(menu);
}

void BrowserFrameWin::OnWindowPosChanged(WINDOWPOS* window_pos)
{
    NativeWidgetWin::OnWindowPosChanged(window_pos);
    UpdateDWMFrame();

    // Windows lies to us about the position of the minimize button before a
    // window is visible.  We use this position to place the OTR avatar in RTL
    // mode, so when the window is shown, we need to re-layout and schedule a
    // paint for the non-client frame view so that the icon top has the correct
    // position when the window becomes visible.  This fixes bugs where the icon
    // appears to overlay the minimize button.
    // Note that we will call Layout every time SetWindowPos is called with
    // SWP_SHOWWINDOW, however callers typically are careful about not specifying
    // this flag unless necessary to avoid flicker.
    // This may be invoked during creation on XP and before the non_client_view
    // has been created.
    if(window_pos->flags&SWP_SHOWWINDOW && GetWidget()->non_client_view())
    {
        GetWidget()->non_client_view()->Layout();
        GetWidget()->non_client_view()->SchedulePaint();
    }
}

void BrowserFrameWin::OnScreenReaderDetected()
{
    //BrowserAccessibilityState::GetInstance()->OnScreenReaderDetected();
    NativeWidgetWin::OnScreenReaderDetected();
}

bool BrowserFrameWin::ShouldUseNativeFrame() const
{
    // App panel windows draw their own frame.
    if(browser_view_->IsBrowserTypePanel())
    {
        return false;
    }

    // We don't theme popup or app windows, so regardless of whether or not a
    // theme is active for normal browser windows, we don't want to use the custom
    // frame for popups/apps.
    if(!browser_view_->IsBrowserTypeNormal() &&
        NativeWidgetWin::ShouldUseNativeFrame())
    {
        return true;
    }

    // Otherwise, we use the native frame when we're told we should by the theme
    // provider (e.g. no custom theme is active).
    return GetWidget()->GetThemeProvider()->ShouldUseNativeFrame();
}

////////////////////////////////////////////////////////////////////////////////
// BrowserFrameWin, NativeBrowserFrame implementation:

view::NativeWidget* BrowserFrameWin::AsNativeWidget()
{
    return this;
}

const view::NativeWidget* BrowserFrameWin::AsNativeWidget() const
{
    return this;
}

int BrowserFrameWin::GetMinimizeButtonOffset() const
{
    TITLEBARINFOEX titlebar_info;
    titlebar_info.cbSize = sizeof(TITLEBARINFOEX);
    SendMessage(GetNativeView(), WM_GETTITLEBARINFOEX, 0, (WPARAM)&titlebar_info);

    POINT minimize_button_corner =
    {
        titlebar_info.rgrect[2].left,
        titlebar_info.rgrect[2].top
    };
    MapWindowPoints(HWND_DESKTOP, GetNativeView(), &minimize_button_corner, 1);

    return minimize_button_corner.x;
}

void BrowserFrameWin::TabStripDisplayModeChanged()
{
    UpdateDWMFrame();
}

///////////////////////////////////////////////////////////////////////////////
// BrowserFrameWin, private:

void BrowserFrameWin::UpdateDWMFrame()
{
    // Nothing to do yet, or we're not showing a DWM frame.
    if(!GetWidget()->client_view() || !browser_frame_->ShouldUseNativeFrame())
    {
        return;
    }

    MARGINS margins = { 0 };
    if(browser_view_->IsBrowserTypeNormal())
    {
        // In fullscreen mode, we don't extend glass into the client area at all,
        // because the GDI-drawn text in the web content composited over it will
        // become semi-transparent over any glass area.
        if(!IsMaximized() && !IsFullscreen())
        {
            margins.cxLeftWidth = kClientEdgeThickness + 1;
            margins.cxRightWidth = kClientEdgeThickness + 1;
            margins.cyBottomHeight = kClientEdgeThickness + 1;
            margins.cyTopHeight = kClientEdgeThickness + 1;
        }
        // In maximized mode, we only have a titlebar strip of glass, no side/bottom
        // borders.
        if(!browser_view_->IsFullscreen())
        {
            gfx::Rect tabstrip_bounds(
                browser_frame_->GetBoundsForTabStrip(browser_view_->tabstrip()));
            margins.cyTopHeight = tabstrip_bounds.bottom() + kDWMFrameTopOffset;
        }
    }
    else
    {
        // For popup and app windows we want to use the default margins.
    }
    DwmExtendFrameIntoClientArea(GetNativeView(), &margins);
}

////////////////////////////////////////////////////////////////////////////////
// BrowserFrame, public:

// static
const gfx::Font& BrowserFrame::GetTitleFont()
{
    static gfx::Font* title_font =
        new gfx::Font(view::NativeWidgetWin::GetWindowTitleFont());
    return *title_font;
}

////////////////////////////////////////////////////////////////////////////////
// NativeBrowserFrame, public:

// static
NativeBrowserFrame* NativeBrowserFrame::CreateNativeBrowserFrame(
    BrowserFrame* browser_frame,
    BrowserView* browser_view)
{
    if(view::Widget::IsPureViews() &&
        view::ViewDelegate::view_delegate->GetDefaultParentView())
    {
        return new BrowserFrameViews(browser_frame, browser_view);
    }
    return new BrowserFrameWin(browser_frame, browser_view);
}