
#ifndef __view_rich_view_h__
#define __view_rich_view_h__

#pragma once

#include <windows.h>
#include <richedit.h>
#include <textserv.h>

#include "view/view.h"

namespace view
{

    // ʵ���޴��ڵ�richedit�ؼ�.
    class RichView : public View, public ITextHost
    {
    public:
        // RichView������.
        static const char kViewClassName[];

        explicit RichView(LONG style);
        virtual ~RichView();

        // ��ȡ/����ֻ������.
        bool IsReadOnly() const;
        void SetReadOnly(bool read_only);

        // ��ȡ/������������.
        bool IsPassword() const;
        void SetPassword(bool password);

        // ��ȡ/���ö�������.
        bool IsMultiLine() const;
        void SetMultiLine(bool multi_line);

        // ��ȡ/���ñ߽�.
        const gfx::Insets& GetMargins() const;
        void SetMargins(const gfx::Insets& margins);

        // �����ı�.
        bool SetText(const std::wstring& text);

        // Overridden from View:
        virtual void OnEnabledChanged();
        virtual void OnPaint(gfx::Canvas* canvas);
        virtual bool OnSetCursor(const gfx::Point& p);
        virtual bool OnMousePressed(const MouseEvent& e);
        virtual bool OnMouseDragged(const MouseEvent& e);
        virtual void OnMouseReleased(const MouseEvent& e);
        virtual void OnMouseMoved(const MouseEvent& e);
        virtual bool OnKeyPressed(const KeyEvent& e);
        virtual bool OnKeyReleased(const KeyEvent& e);
        virtual bool OnMouseWheel(const MouseWheelEvent& e);
        virtual LRESULT OnImeMessages(UINT message,
            WPARAM w_param,
            LPARAM l_param,
            BOOL* handled);
        virtual void DragEnter(IDataObject* data_object,
            DWORD key_state,
            POINTL cursor_position,
            DWORD* effect);
        virtual void DragOver(DWORD key_state,
            POINTL cursor_position,
            DWORD* effect);
        virtual void DragLeave();
        virtual void Drop(IDataObject* data_object,
            DWORD key_state,
            POINTL cursor_position,
            DWORD* effect);

        // IUnknown
        STDMETHOD_(ULONG, AddRef)();
        STDMETHOD_(ULONG, Release)();
        STDMETHOD(QueryInterface)(REFIID iid, void** object);

        // ITextHost
        virtual HDC TxGetDC();
        virtual INT TxReleaseDC(HDC hdc);
        virtual BOOL TxShowScrollBar(INT fnBar, BOOL fShow);
        virtual BOOL TxEnableScrollBar(INT fuSBFlags, INT fuArrowflags);
        virtual BOOL TxSetScrollRange(INT fnBar, LONG nMinPos,
            INT nMaxPos, BOOL fRedraw);
        virtual BOOL TxSetScrollPos(INT fnBar, INT nPos, BOOL fRedraw);
        virtual void TxInvalidateRect(LPCRECT prc, BOOL fMode);
        virtual void TxViewChange(BOOL fUpdate);
        virtual BOOL TxCreateCaret(HBITMAP hbmp, INT xWidth, INT yHeight);
        virtual BOOL TxShowCaret(BOOL fShow);
        virtual BOOL TxSetCaretPos(INT x, INT y);
        virtual BOOL TxSetTimer(UINT idTimer, UINT uTimeout);
        virtual void TxKillTimer(UINT idTimer);
        virtual void TxScrollWindowEx(INT dx, INT dy,
            LPCRECT lprcScroll, LPCRECT lprcClip,
            HRGN hrgnUpdate, LPRECT lprcUpdate, UINT fuScroll);
        virtual void TxSetCapture(BOOL fCapture);
        virtual void TxSetFocus();
        virtual void TxSetCursor(HCURSOR hcur, BOOL fText);
        virtual BOOL TxScreenToClient(LPPOINT lppt);
        virtual BOOL TxClientToScreen(LPPOINT lppt);
        virtual HRESULT TxActivate(LONG* plOldState);
        virtual HRESULT TxDeactivate(LONG lNewState);
        virtual HRESULT TxGetClientRect(LPRECT prc);
        virtual HRESULT TxGetViewInset(LPRECT prc);
        virtual HRESULT TxGetCharFormat(const CHARFORMATW** ppCF);
        virtual HRESULT TxGetParaFormat(const PARAFORMAT** ppPF);
        virtual COLORREF TxGetSysColor(int nIndex);
        virtual HRESULT TxGetBackStyle(TXTBACKSTYLE* pstyle);
        virtual HRESULT TxGetMaxLength(DWORD* plength);
        virtual HRESULT TxGetScrollBars(DWORD* pdwScrollBar);
        virtual HRESULT TxGetPasswordChar(TCHAR* pch);
        virtual HRESULT TxGetAcceleratorPos(LONG* pcp);
        virtual HRESULT TxGetExtent(LPSIZEL lpExtent);
        virtual HRESULT OnTxCharFormatChange(const CHARFORMATW* pcf);
        virtual HRESULT OnTxParaFormatChange(const PARAFORMAT* ppf);
        virtual HRESULT TxGetPropertyBits(DWORD dwMask, DWORD* pdwBits);
        virtual HRESULT TxNotify(DWORD iNotify, void* pv);
        virtual HIMC TxImmGetContext();
        virtual void TxImmReleaseContext(HIMC himc);
        virtual HRESULT TxGetSelectionBarWidth(LONG* lSelBarWidth);

    protected:
        virtual void Layout();
        virtual void OnFocus();
        virtual void OnBlur();
        virtual void ViewHierarchyChanged(bool is_add, View* parent, View* child);
        virtual std::string GetClassName() const;

    private:
        IDropTarget* GetDropTarget();

        // ��ʼ��DC��ȱʡ���ú���ɫ.
        static void InitializeDC(HDC context);

        // �Ƿ��ѳ�ʼ��.
        bool initialized_;

        // �߽�.
        gfx::Insets margins_;

        ITextServices* text_service_;

        // Properties
        DWORD       dwStyle_;               // style bits
        DWORD       dwExStyle_;             // extended style bits

        unsigned    fBorder_            :1; // control has border
        unsigned    fCustRect_          :1; // client changed format rect
        unsigned    fInBottomless_      :1; // inside bottomless callback
        unsigned    fInDialogBox_       :1; // control is in a dialog box
        unsigned    fEnableAutoWordSel_ :1; // enable Word style auto word selection?
        unsigned    fVertical_          :1; // vertical writing
        unsigned    fIconic_            :1; // control window is iconic
        unsigned    fHidden_            :1; // control window is hidden
        unsigned    fNotSysBkgnd_       :1; // not using system background color
        unsigned    fWindowLocked_      :1; // window is locked (no update)
        unsigned    fRegisteredForDrop_ :1; // whether host has registered for drop
        unsigned    fVisible_           :1; // Whether window is visible or not.
        unsigned    fResized_           :1; // resized while hidden
        unsigned    fWordWrap_          :1; // Whether control should word wrap
        unsigned    fAllowBeep_         :1; // Whether beep is allowed
        unsigned    fRich_              :1; // Whether control is rich text
        unsigned    fSaveSelection_     :1; // Whether to save the selection when inactive
        unsigned    fInplaceActive_     :1; // Whether control is inplace active
        unsigned    fTransparent_       :1; // Whether control is transparent
        unsigned    fTimer_             :1; // A timer is set

        LONG        lSelBarWidth_;          // Width of the selection bar

        COLORREF    crBackground_;          // background color
        LONG        cchTextMost_;           // maximum text size
        DWORD       dwEventMask_;           // Event mask to pass on to parent window

        LONG        icf_;
        LONG        ipf_;

        CHARFORMAT2W cf_;                   // Default character format

        PARAFORMAT2 pf_;                    // Default paragraph format

        LONG        laccelpos_;             // Accelerator position

        WCHAR       chPasswordChar_;        // Password character
    };

} //namespace view

#endif //__view_rich_view_h__