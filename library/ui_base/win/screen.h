
#ifndef __ui_base_screen_h__
#define __ui_base_screen_h__

#pragma once

#include <windows.h>

#include "ui_gfx/point.h"
#include "ui_gfx/rect.h"

namespace ui
{

    // A utility class for getting various info about screen size, monitors,
    // cursor position, etc.
    // TODO(erikkay) add more of those methods here
    class Screen
    {
    public:
        static gfx::Point GetCursorScreenPoint();

        // Returns the work area of the monitor nearest the specified window.
        static gfx::Rect GetMonitorWorkAreaNearestWindow(HWND view);

        // Returns the bounds of the monitor nearest the specified window.
        static gfx::Rect GetMonitorAreaNearestWindow(HWND view);

        // Returns the work area of the monitor nearest the specified point.
        static gfx::Rect GetMonitorWorkAreaNearestPoint(const gfx::Point& point);

        // Returns the monitor area (not the work area, but the complete bounds) of
        // the monitor nearest the specified point.
        static gfx::Rect GetMonitorAreaNearestPoint(const gfx::Point& point);

        // Returns the window under the cursor.
        static HWND GetWindowAtCursorScreenPoint();
    };

} //namespace ui

#endif //__ui_base_screen_h__