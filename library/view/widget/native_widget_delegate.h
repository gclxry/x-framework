
#ifndef __view_native_widget_delegate_h__
#define __view_native_widget_delegate_h__

#pragma once

#include <windows.h>

namespace gfx
{
    class Canvas;
    class Point;
    class Rect;
    class Size;
}

namespace view
{

    class InputMethodWin;
    class KeyEvent;
    class MouseEvent;
    class Widget;

    namespace internal
    {

        ////////////////////////////////////////////////////////////////////////////////
        // NativeWidgetDelegate
        //
        //  An interface implemented by the object that handles events sent by a
        //  NativeWidget implementation.
        //
        class NativeWidgetDelegate
        {
        public:
            virtual ~NativeWidgetDelegate() {}

            // Returns true if the window is modal.
            virtual bool IsModal() const = 0;

            // Returns true if the window is a dialog box.
            virtual bool IsDialogBox() const = 0;

            // Returns true if the window can be activated.
            virtual bool CanActivate() const = 0;

            virtual bool IsInactiveRenderingDisabled() const = 0;
            virtual void EnableInactiveRendering() = 0;

            // Called when the activation state of a window has changed.
            virtual void OnNativeWidgetActivationChanged(bool active) = 0;

            // Called when native focus moves from one native view to another.
            virtual void OnNativeFocus(HWND focused_view) = 0;
            virtual void OnNativeBlur(HWND focused_view) = 0;

            // Called when the window is shown/hidden.
            virtual void OnNativeWidgetVisibilityChanged(bool visible) = 0;

            // Called when the native widget is created.
            virtual void OnNativeWidgetCreated() = 0;

            // Called just before the native widget is destroyed. This is the delegate's
            // last chance to do anything with the native widget handle.
            virtual void OnNativeWidgetDestroying() = 0;

            // Called just after the native widget is destroyed.
            virtual void OnNativeWidgetDestroyed() = 0;

            // Returns the smallest size the window can be resized to by the user.
            virtual gfx::Size GetMinimumSize() = 0;

            // Called when the NativeWidget changed size to |new_size|.
            virtual void OnNativeWidgetSizeChanged(const gfx::Size& new_size) = 0;

            // Called when the user begins/ends to change the bounds of the window.
            virtual void OnNativeWidgetBeginUserBoundsChange() = 0;
            virtual void OnNativeWidgetEndUserBoundsChange() = 0;

            // Returns true if the delegate has a FocusManager.
            virtual bool HasFocusManager() const = 0;

            // Paints the widget using acceleration. If the widget is not using
            // accelerated painting this returns false and does nothing.
            virtual bool OnNativeWidgetPaintAccelerated(
                const gfx::Rect& dirty_region) = 0;

            // Paints the rootview in the canvas. This will also refresh the compositor
            // tree if necessary when accelerated painting is enabled.
            virtual void OnNativeWidgetPaint(gfx::Canvas* canvas) = 0;

            // Returns the non-client component (see views/window/hit_test.h) containing
            // |point|, in client coordinates.
            virtual int GetNonClientComponent(const gfx::Point& point) = 0;

            // Mouse and key event handlers.
            virtual bool OnKeyEvent(const KeyEvent& event) = 0;
            virtual bool OnMouseEvent(const MouseEvent& event) = 0;
            virtual void OnMouseCaptureLost() = 0;

            // SetCursor handlers.
            virtual bool OnNativeSetCursor(HWND window, UINT hit_test, UINT message) = 0;

            // Runs the specified native command. Returns true if the command is handled.
            virtual bool ExecuteCommand(int command_id) = 0;

            // Returns the input method of the widget this delegate is associated with.
            // Note that this does not use the top level widget, so may return NULL
            // if the widget doesn't have input method.
            virtual InputMethodWin* GetInputMethodDirect() = 0;

            virtual Widget* AsWidget() = 0;
            virtual const Widget* AsWidget() const = 0;
        };

    } //namespace internal
} //namespace view

#endif //__view_native_widget_delegate_h__