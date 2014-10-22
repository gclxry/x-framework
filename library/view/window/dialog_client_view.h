
#ifndef __view_dialog_client_view_h__
#define __view_dialog_client_view_h__

#pragma once

#include "ui_gfx/font.h"

#include "client_view.h"
#include "view/controls/button/button.h"
#include "view/focus/focus_manager.h"

namespace view
{

    class DialogDelegate;
    class NativeTextButton;
    class Widget;
    namespace internal
    {
        class RootView;
    }

    ///////////////////////////////////////////////////////////////////////////////
    // DialogClientView
    //
    //  This ClientView subclass provides the content of a typical dialog box,
    //  including a strip of buttons at the bottom right of the window, default
    //  accelerator handlers for accept and cancel, and the ability for the
    //  embedded contents view to provide extra UI to be shown in the row of
    //  buttons.
    //
    //  DialogClientView also provides the ability to set an arbitrary view that is
    //  positioned beneath the buttons.
    //
    class DialogClientView : public ClientView,
        public ButtonListener,
        public FocusChangeListener
    {
    public:
        DialogClientView(Widget* widget, View* contents_view);
        virtual ~DialogClientView();

        // Adds the dialog buttons required by the supplied DialogDelegate to the
        // view.
        void ShowDialogButtons();

        // Updates the enabled state and label of the buttons required by the
        // supplied DialogDelegate
        void UpdateDialogButtons();

        // Accept the changes made in the window that contains this ClientView.
        void AcceptWindow();

        // Cancel the changes made in the window that contains this ClientView.
        void CancelWindow();

        // Accessors in case the user wishes to adjust these buttons.
        NativeTextButton* ok_button() const { return ok_button_; }
        NativeTextButton* cancel_button() const { return cancel_button_; }

        // Sets the view that is positioned along the bottom of the buttons. The
        // bottom view is positioned beneath the buttons at the full width of the
        // dialog. If there is an existing bottom view it is removed and deleted.
        void SetBottomView(View* bottom_view);

        // Overridden from View:
        virtual void NativeViewHierarchyChanged(
            bool attached,
            HWND native_view,
            internal::RootView* root_view);

        // Overridden from ClientView:
        virtual bool CanClose();
        virtual void WidgetClosing();
        virtual int NonClientHitTest(const gfx::Point& point);
        virtual DialogClientView* AsDialogClientView();
        virtual const DialogClientView* AsDialogClientView() const;

        // FocusChangeListener implementation:
        virtual void FocusWillChange(View* focused_before,
            View* focused_now);

    protected:
        // View overrides:
        virtual void OnPaint(gfx::Canvas* canvas);
        virtual void PaintChildren(gfx::Canvas* canvas);
        virtual void Layout();
        virtual void ViewHierarchyChanged(bool is_add, View* parent, View* child);
        virtual gfx::Size GetPreferredSize();
        virtual bool AcceleratorPressed(const Accelerator& accelerator);

        // ButtonListener implementation:
        virtual void ButtonPressed(Button* sender, const Event& event);

    private:
        // Paint the size box in the bottom right corner of the window if it is
        // resizable.
        void PaintSizeBox(gfx::Canvas* canvas);

        // Returns the width of the specified dialog button using the correct font.
        int GetButtonWidth(int button) const;
        int GetButtonsHeight() const;

        // Position and size various sub-views.
        void LayoutDialogButtons();
        void LayoutContentsView();

        // Makes the specified button the default button.
        void SetDefaultButton(NativeTextButton* button);

        bool has_dialog_buttons() const { return ok_button_ || cancel_button_; }

        // Create and add the extra view, if supplied by the delegate.
        void CreateExtraView();

        // Returns the DialogDelegate for the window.
        DialogDelegate* GetDialogDelegate() const;

        // Closes the widget.
        void Close();

        // Updates focus listener.
        void UpdateFocusListener();

        static void InitClass();

        // The dialog buttons.
        NativeTextButton* ok_button_;
        NativeTextButton* cancel_button_;

        // The button that is currently the default button if any.
        NativeTextButton* default_button_;

        // The button-level extra view, NULL unless the dialog delegate supplies one.
        View* extra_view_;

        // See description of DialogDelegate::GetSizeExtraViewHeightToButtons for
        // details on this.
        bool size_extra_view_height_to_buttons_;

        // The layout rect of the size box, when visible.
        gfx::Rect size_box_bounds_;

        // True if we've notified the delegate the window is closing and the delegate
        // allosed the close. In some situations it's possible to get two closes (see
        // http://crbug.com/71940). This is used to avoid notifying the delegate
        // twice, which can have bad consequences.
        bool notified_delegate_;

        // true if focus listener is added.
        bool listening_to_focus_;

        // When ancestor gets changed focus manager gets changed as well.
        FocusManager* saved_focus_manager_;

        // View positioned along the bottom, beneath the buttons.
        View* bottom_view_;

        // Static resource initialization
        static gfx::Font* dialog_button_font_;

        DISALLOW_COPY_AND_ASSIGN(DialogClientView);
    };

} //namespace view

#endif //__view_dialog_client_view_h__