
#include "screen.h"

#include <windows.h>

namespace ui
{

    // static
    gfx::Point Screen::GetCursorScreenPoint()
    {
        POINT pt;
        GetCursorPos(&pt);
        return gfx::Point(pt);
    }

    // static
    gfx::Rect Screen::GetMonitorWorkAreaNearestWindow(HWND window)
    {
        MONITORINFO monitor_info;
        monitor_info.cbSize = sizeof(monitor_info);
        GetMonitorInfo(MonitorFromWindow(window, MONITOR_DEFAULTTONEAREST),
            &monitor_info);
        return gfx::Rect(monitor_info.rcWork);
    }

    // static
    gfx::Rect Screen::GetMonitorAreaNearestWindow(HWND window)
    {
        MONITORINFO monitor_info;
        monitor_info.cbSize = sizeof(monitor_info);
        GetMonitorInfo(MonitorFromWindow(window, MONITOR_DEFAULTTONEAREST),
            &monitor_info);
        return gfx::Rect(monitor_info.rcMonitor);
    }

    static gfx::Rect GetMonitorAreaOrWorkAreaNearestPoint(const gfx::Point& point,
        bool work_area)
    {
        POINT initial_loc = { point.x(), point.y() };
        HMONITOR monitor = MonitorFromPoint(initial_loc, MONITOR_DEFAULTTONEAREST);
        MONITORINFO mi = { 0 };
        mi.cbSize = sizeof(mi);
        if(monitor && GetMonitorInfo(monitor, &mi))
        {
            return gfx::Rect(work_area ? mi.rcWork : mi.rcMonitor);
        }
        return gfx::Rect();
    }

    // static
    gfx::Rect Screen::GetMonitorWorkAreaNearestPoint(const gfx::Point& point)
    {
        return GetMonitorAreaOrWorkAreaNearestPoint(point, true);
    }

    // static
    gfx::Rect Screen::GetMonitorAreaNearestPoint(const gfx::Point& point)
    {
        return GetMonitorAreaOrWorkAreaNearestPoint(point, false);
    }

    HWND Screen::GetWindowAtCursorScreenPoint()
    {
        POINT location;
        return GetCursorPos(&location) ? WindowFromPoint(location) : NULL;
    }

} //namespace ui