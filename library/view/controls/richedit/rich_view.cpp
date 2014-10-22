
#include "rich_view.h"

#include "base/logging.h"

#include "ui_gfx/canvas.h"
#include "ui_gfx/color_utils.h"
#include "ui_gfx/font.h"

#include "ui_base/dragdrop/os_exchange_data_provider_win.h"
#include "ui_base/resource/resource_bundle.h"

#include "view/event/event_utils_win.h"
#include "view/metrics.h"
#include "view/widget/root_view.h"
#include "view/widget/widget.h"

#pragma comment(lib, "riched20.lib")
#pragma comment(lib, "imm32.lib")

// 8d33f740-cf58-11ce-a89d-00aa006cadc5
EXTERN_C const IID IID_ITextServices =
{
    0x8d33f740, 0xcf58, 0x11ce,
    { 0xa8, 0x9d, 0x00, 0xaa, 0x00, 0x6c, 0xad, 0xc5 }
};

// c5bdd8d0-d26e-11ce-a89e-00aa006cadc5
EXTERN_C const IID IID_ITextHost =
{
    0xc5bdd8d0, 0xd26e, 0x11ce,
    { 0xa8, 0x9e, 0x00, 0xaa, 0x00, 0x6c, 0xad, 0xc5 }
};

namespace
{

    #define LY_PER_INCH 1440

    HRESULT InitDefaultCharFormat(CHARFORMAT2W& cf)
    {
        gfx::Font font = ui::ResourceBundle::GetSharedInstance().GetFont(
            ui::ResourceBundle::BaseFont);
        LOGFONT lf;
        if(!GetObject(font.GetNativeFont(), sizeof(LOGFONT), &lf))
        {
            return E_FAIL;
        }

        cf.cbSize = sizeof(CHARFORMAT2W);

        HDC screen_ddc = GetDC(GetDesktopWindow());
        int pixels_per_inch = GetDeviceCaps(screen_ddc, LOGPIXELSY);
        cf.yHeight = abs(lf.lfHeight) * LY_PER_INCH / pixels_per_inch;
        ReleaseDC(GetDesktopWindow(), screen_ddc);

        cf.yOffset = 0;
        cf.crTextColor = gfx::GetSysSkColor(COLOR_WINDOWTEXT);

        cf.dwEffects = CFM_EFFECTS | CFE_AUTOBACKCOLOR;
        cf.dwEffects &= ~(CFE_PROTECTED | CFE_LINK);

        if(lf.lfWeight < FW_BOLD)
        {
            cf.dwEffects &= ~CFE_BOLD;
        }
        if(!lf.lfItalic)
        {
            cf.dwEffects &= ~CFE_ITALIC;
        }
        if(!lf.lfUnderline)
        {
            cf.dwEffects &= ~CFE_UNDERLINE;
        }
        if(!lf.lfStrikeOut)
        {
            cf.dwEffects &= ~CFE_STRIKEOUT;
        }

        cf.dwMask = CFM_ALL | CFM_BACKCOLOR;
        cf.bCharSet = lf.lfCharSet;
        cf.bPitchAndFamily = lf.lfPitchAndFamily;
        wcscpy(cf.szFaceName, lf.lfFaceName);

        return S_OK;
    }

    HRESULT InitDefaultParaFormat(PARAFORMAT2& pf)
    {	
        ZeroMemory(&pf, sizeof(PARAFORMAT2));

        pf.cbSize = sizeof(PARAFORMAT2);
        pf.dwMask = PFM_ALL;
        pf.wAlignment = PFA_LEFT;
        pf.cTabCount = 1;
        pf.rgxTabs[0] = lDefaultTab;

        return S_OK;
    }

}

namespace view
{

    static const long kInitTextMax = (32 * 1024) - 1;

    // static
    const char RichView::kViewClassName[] = "view/RichView";

    RichView::RichView(LONG style) : initialized_(false),
        margins_(3, 3, 3, 3)
    {
        ZeroMemory(&text_service_,
            sizeof(RichView)-offsetof(RichView, text_service_));

        cchTextMost_ = kInitTextMax;
        laccelpos_ = -1;
        fTransparent_ = TRUE;
        fRich_ = TRUE;
        dwStyle_ = style;

        set_focusable(true);
    }

    RichView::~RichView()
    {
        text_service_->OnTxInPlaceDeactivate();
        text_service_->Release();
    }

    bool RichView::IsReadOnly() const
    {
        return (dwStyle_ & ES_READONLY) != 0;
    }

    void RichView::SetReadOnly(bool read_only)
    {
        if (read_only)
        {
            dwStyle_ |= ES_READONLY;
        }
        else
        {
            dwStyle_ &= ~ES_READONLY;
        }

        // Notify control of property change
        text_service_->OnTxPropertyBitsChange(TXTBIT_READONLY, 
            read_only ? TXTBIT_READONLY : 0);
    }

    bool RichView::IsPassword() const
    {
        return (dwStyle_ & ES_PASSWORD) != 0;
    }

    void RichView::SetPassword(bool password)
    {
        if (password)
        {
            dwStyle_ |= ES_PASSWORD;
        }
        else
        {
            dwStyle_ &= ~ES_PASSWORD;
        }

        // Notify control of property change
        text_service_->OnTxPropertyBitsChange(TXTBIT_USEPASSWORD, 
            password ? TXTBIT_USEPASSWORD : 0);
    }

    bool RichView::IsMultiLine() const
    {
        return (dwStyle_ & ES_MULTILINE) != 0;
    }

    void RichView::SetMultiLine(bool multi_line)
    {
        if (multi_line)
        {
            dwStyle_ |= ES_MULTILINE;
        }
        else
        {
            dwStyle_ &= ~ES_MULTILINE;
        }

        // Notify control of property change
        text_service_->OnTxPropertyBitsChange(TXTBIT_MULTILINE, 
            multi_line ? TXTBIT_WORDWRAP : 0);
    }

    const gfx::Insets& RichView::GetMargins() const
    {
        return margins_;
    }

    void RichView::SetMargins(const gfx::Insets& margins)
    {
        margins_ = margins;

        // Notify control of property change
        text_service_->OnTxPropertyBitsChange(TXTBIT_VIEWINSETCHANGE, 0);
    }

    bool RichView::SetText(const std::wstring& text)
    {
        if(text_service_ && SUCCEEDED(text_service_->TxSetText(text.c_str())))
        {
            return true;
        }
        return false;
    }

    // Overridden from View:

    void RichView::OnEnabledChanged()
    {
        View::OnEnabledChanged();
    }

    void RichView::OnPaint(gfx::Canvas* canvas)
    {
        View::OnPaint(canvas);

        HDC dc = canvas->BeginPlatformPaint();
        text_service_->TxDraw(DVASPECT_CONTENT, 0, NULL, NULL, dc, NULL,
            NULL, NULL, NULL, NULL, NULL, TXTVIEW_ACTIVE);
        canvas->EndPlatformPaint();
    }

    bool RichView::OnSetCursor(const gfx::Point& p)
    {
        text_service_->OnTxSetCursor(DVASPECT_CONTENT, -1, NULL, NULL,
            NULL, NULL, NULL, p.x(), p.y());
        return true;
    }

    bool RichView::OnMousePressed(const MouseEvent& e)
    {
        RequestFocus();

        MSG msg = e.native_event();
        msg.lParam = MAKELPARAM(e.location().x(), e.location().y());
        LRESULT lr;
        text_service_->TxSendMessage(msg.message, msg.wParam, msg.lParam, &lr);
        return true;
    }

    void RichView::OnMouseReleased(const MouseEvent& e)
    {
        MSG msg = e.native_event();
        msg.lParam = MAKELPARAM(e.location().x(), e.location().y());
        LRESULT lr;
        text_service_->TxSendMessage(WM_LBUTTONUP, msg.wParam, msg.lParam, &lr);
    }

    bool RichView::OnMouseDragged(const MouseEvent& e)
    {
        MSG msg = e.native_event();
        msg.lParam = MAKELPARAM(e.location().x(), e.location().y());
        LRESULT lr;
        text_service_->TxSendMessage(WM_MOUSEMOVE, msg.wParam, msg.lParam, &lr);
        return true;
    }

    void RichView::OnMouseMoved(const MouseEvent& e)
    {
        MSG msg = e.native_event();
        msg.lParam = MAKELPARAM(e.location().x(), e.location().y());
        LRESULT lr;
        text_service_->TxSendMessage(msg.message, msg.wParam, msg.lParam, &lr);
    }

    bool RichView::OnMouseWheel(const MouseWheelEvent& e)
    {
        WPARAM wparam = SB_LINEUP;
        if(e.offset() < 0)
        {
            wparam = SB_LINEDOWN;
        }

        int scroll = abs(e.offset() / MouseWheelEvent::kWheelDelta);
        scroll *= GetWheelScrollLines();

        for(int i=0; i<scroll; ++i)
        {
            text_service_->TxSendMessage(WM_VSCROLL, wparam, 0, 0);
        }
        return true;
    }

    bool RichView::OnKeyPressed(const KeyEvent& e)
    {
        const MSG& msg = e.native_event();
        LRESULT lr;
        text_service_->TxSendMessage(msg.message, msg.wParam, msg.lParam, &lr);
        return false;
    }

    bool RichView::OnKeyReleased(const KeyEvent& e)
    {
        const MSG& msg = e.native_event();
        LRESULT lr;
        text_service_->TxSendMessage(msg.message, msg.wParam, msg.lParam, &lr);
        return false;
    }

    void RichView::DragEnter(IDataObject* data_object,
        DWORD key_state,
        POINTL cursor_position,
        DWORD* effect)
    {
        base::win::ScopedComPtr<IDropTarget> drop_target(GetDropTarget());
        if(drop_target)
        {
            drop_target->DragEnter(data_object, key_state,
                cursor_position, effect);
        }
        else
        {
            *effect = DROPEFFECT_NONE;
        }
    }

    void RichView::DragOver(DWORD key_state,
        POINTL cursor_position,
        DWORD* effect)
    {
        base::win::ScopedComPtr<IDropTarget> drop_target(GetDropTarget());
        if(drop_target)
        {
            drop_target->DragOver(key_state, cursor_position, effect);
        }
        else
        {
            *effect = DROPEFFECT_NONE;
        }
    }

    void RichView::DragLeave()
    {
        base::win::ScopedComPtr<IDropTarget> drop_target(GetDropTarget());
        if(drop_target)
        {
            drop_target->DragLeave();
        }
    }

    void RichView::Drop(IDataObject* data_object,
        DWORD key_state,
        POINTL cursor_position,
        DWORD* effect)
    {
        base::win::ScopedComPtr<IDropTarget> drop_target(GetDropTarget());
        if(drop_target)
        {
            drop_target->Drop(data_object, key_state,
                cursor_position, effect);
        }
        else
        {
            *effect = DROPEFFECT_NONE;
        }
    }

    LRESULT RichView::OnImeMessages(UINT message, WPARAM w_param,
        LPARAM l_param, BOOL* handled)
    {
        LRESULT lr;
        text_service_->TxSendMessage(message, w_param, l_param, &lr);
        *handled = FALSE;
        return lr;
    }

    // IUnknown
    STDMETHODIMP_(ULONG) RichView::AddRef()
    {
        return 1;
    }

    STDMETHODIMP_(ULONG) RichView::Release()
    {
        return 1;
    }

    STDMETHODIMP RichView::QueryInterface(REFIID iid, void** object)
    {
        HRESULT hr = S_OK;
        *object = NULL;
        if(iid==IID_IUnknown || iid==IID_ITextHost)
        {
            *object = static_cast<ITextHost*>(this);
        }
        else
        {
            hr = E_NOINTERFACE;
        }
        if(SUCCEEDED(hr))
        {
            static_cast<IUnknown*>(*object)->AddRef();
        }
        return hr;
    }

    // ITextHost
    HDC RichView::TxGetDC()
    {
        HDC hdc = ::GetDC(GetWidget()->GetNativeView());
        InitializeDC(hdc);
        gfx::Point orig;
        ConvertPointToWidget(this, &orig);
        SetViewportOrgEx(hdc, orig.x(), orig.y(), NULL);
        return hdc;
    }

    INT RichView::TxReleaseDC(HDC hdc)
    {
        return ::ReleaseDC(GetWidget()->GetNativeView(), hdc);
    }

    BOOL RichView::TxShowScrollBar(INT fnBar, BOOL fShow)
    {
        return TRUE;
    }

    BOOL RichView::TxEnableScrollBar(INT fuSBFlags, INT fuArrowflags)
    {
        return TRUE;
    }

    BOOL RichView::TxSetScrollRange(INT fnBar, LONG nMinPos,
        INT nMaxPos, BOOL fRedraw)
    {
        return TRUE;
    }

    BOOL RichView::TxSetScrollPos(INT fnBar, INT nPos, BOOL fRedraw)
    {
        WPARAM wParam = MAKEWPARAM(SB_THUMBPOSITION, nPos);
        text_service_->TxSendMessage(WM_VSCROLL, wParam, 0L, 0);
        return TRUE;
    }

    void RichView::TxInvalidateRect(LPCRECT prc, BOOL fMode)
    {
        SchedulePaint();
    }

    void RichView::TxViewChange(BOOL fUpdate)
    {
        SchedulePaint();
    }

    BOOL RichView::TxCreateCaret(HBITMAP hbmp, INT xWidth, INT yHeight)
    {
        return ::CreateCaret(GetWidget()->GetNativeView(), hbmp,
            xWidth, yHeight);
    }

    BOOL RichView::TxShowCaret(BOOL fShow)
    {
        if(fShow)
        {
            return ::ShowCaret(GetWidget()->GetNativeView());
        }

        return ::HideCaret(GetWidget()->GetNativeView());
    }

    BOOL RichView::TxSetCaretPos(INT x, INT y)
    {
        gfx::Point pt(x, y);
        ConvertPointToWidget(this, &pt);
        return ::SetCaretPos(pt.x(), pt.y());
    }

    BOOL RichView::TxSetTimer(UINT idTimer, UINT uTimeout)
    {
        if(GetWidget())
        {
            fTimer_ = TRUE;
            return ::SetTimer(GetWidget()->GetNativeView(),
                idTimer, uTimeout, NULL);
        }
        return FALSE;
    }

    void RichView::TxKillTimer(UINT idTimer)
    {
        if(GetWidget())
        {
            ::KillTimer(GetWidget()->GetNativeView(), idTimer);
            fTimer_ = FALSE;
        }
    }

    void RichView::TxScrollWindowEx(INT dx, INT dy,
        LPCRECT lprcScroll, LPCRECT lprcClip,
        HRGN hrgnUpdate, LPRECT lprcUpdate, UINT fuScroll)
    {
    }

    void RichView::TxSetCapture(BOOL fCapture)
    {
    }

    void RichView::TxSetFocus()
    {
    }

    void RichView::TxSetCursor(HCURSOR hcur, BOOL fText)
    {
        ::SetCursor(hcur);
    }

    BOOL RichView::TxScreenToClient(LPPOINT lppt)
    {
        gfx::Point point(*lppt);
        ConvertPointToView(NULL, this, &point);
        *lppt = point.ToPOINT();
        return TRUE;
    }

    BOOL RichView::TxClientToScreen(LPPOINT lppt)
    {
        gfx::Point point(*lppt);
        ConvertPointToScreen(this, &point);
        *lppt = point.ToPOINT();
        return TRUE;
    }

    HRESULT RichView::TxActivate(LONG* plOldState)
    {
        return S_OK;
    }

    HRESULT RichView::TxDeactivate(LONG lNewState)
    {
        return S_OK;
    }

    HRESULT RichView::TxGetClientRect(LPRECT prc)
    {
        *prc = GetLocalBounds().ToRECT();
        return S_OK;
    }

    HRESULT RichView::TxGetViewInset(LPRECT prc)
    {
        ::SetRect(prc, margins_.left(), margins_.top(),
            margins_.right(), margins_.bottom());
        return S_OK;	
    }

    HRESULT RichView::TxGetCharFormat(const CHARFORMATW** ppCF)
    {
        *ppCF = &cf_;
        return S_OK;
    }

    HRESULT RichView::TxGetParaFormat(const PARAFORMAT** ppPF)
    {
        *ppPF = &pf_;
        return S_OK;
    }

    COLORREF RichView::TxGetSysColor(int nIndex)
    {
        if(nIndex == COLOR_WINDOW)
        {
            if(!fNotSysBkgnd_)
            {
                return GetSysColor(COLOR_WINDOW);
            }
            return crBackground_;
        }

        return GetSysColor(nIndex);
    }

    HRESULT RichView::TxGetBackStyle(TXTBACKSTYLE* pstyle)
    {
        *pstyle = !fTransparent_ ? TXTBACK_OPAQUE : TXTBACK_TRANSPARENT;
        return S_OK;
    }

    HRESULT RichView::TxGetMaxLength(DWORD* plength)
    {
        *plength = cchTextMost_;
        return S_OK;
    }

    HRESULT RichView::TxGetScrollBars(DWORD* pdwScrollBar)
    {
        *pdwScrollBar =  dwStyle_ & (WS_VSCROLL | WS_HSCROLL | ES_AUTOVSCROLL | 
            ES_AUTOHSCROLL | ES_DISABLENOSCROLL);

        return S_OK;
    }

    HRESULT RichView::TxGetPasswordChar(TCHAR* pch)
    {
        *pch = chPasswordChar_;
        return S_OK;
    }

    HRESULT RichView::TxGetAcceleratorPos(LONG* pcp)
    {
        *pcp = laccelpos_;
        return S_OK;
    }

    HRESULT RichView::TxGetExtent(LPSIZEL lpExtent)
    {
        return E_NOTIMPL;
    }

    HRESULT RichView::OnTxCharFormatChange(const CHARFORMATW* pcf)
    {
        DCHECK(pcf);
        size_t copy_bytes = std::min(cf_.cbSize, pcf->cbSize);
        memcpy(&cf_, pcf, copy_bytes);

        return S_OK;
    }

    HRESULT RichView::OnTxParaFormatChange(const PARAFORMAT* ppf)
    {
        DCHECK(ppf);
        size_t copy_bytes = std::min(pf_.cbSize, ppf->cbSize);
        memcpy(&pf_, ppf, copy_bytes);

        return S_OK;
    }

    HRESULT RichView::TxGetPropertyBits(DWORD dwMask, DWORD* pdwBits)
    {
        DWORD dwProperties = 0;

        if (fRich_)
        {
            dwProperties = TXTBIT_RICHTEXT;
        }

        if (dwStyle_ & ES_MULTILINE)
        {
            dwProperties |= TXTBIT_MULTILINE;
        }

        if (dwStyle_ & ES_READONLY)
        {
            dwProperties |= TXTBIT_READONLY;
        }


        if (dwStyle_ & ES_PASSWORD)
        {
            dwProperties |= TXTBIT_USEPASSWORD;
        }

        if (!(dwStyle_ & ES_NOHIDESEL))
        {
            dwProperties |= TXTBIT_HIDESELECTION;
        }

        if (fEnableAutoWordSel_)
        {
            dwProperties |= TXTBIT_AUTOWORDSEL;
        }

        if (fVertical_)
        {
            dwProperties |= TXTBIT_VERTICAL;
        }

        if (fWordWrap_)
        {
            dwProperties |= TXTBIT_WORDWRAP;
        }

        if (fAllowBeep_)
        {
            dwProperties |= TXTBIT_ALLOWBEEP;
        }

        if (fSaveSelection_)
        {
            dwProperties |= TXTBIT_SAVESELECTION;
        }

        *pdwBits = dwProperties & dwMask; 
        return S_OK;
    }

    HRESULT RichView::TxNotify(DWORD iNotify, void* pv)
    {
        return S_OK;
    }

    HIMC RichView::TxImmGetContext()
    {
        return NULL;
    }

    void RichView::TxImmReleaseContext(HIMC himc)
    {
        ImmReleaseContext(GetWidget()->GetNativeView(), himc);
    }

    HRESULT RichView::TxGetSelectionBarWidth(LONG* lSelBarWidth)
    {
        *lSelBarWidth = lSelBarWidth_;
        return S_OK;
    }

    void RichView::Layout()
    {
        View::Layout();
        if(text_service_)
        {
            text_service_->OnTxPropertyBitsChange(TXTBIT_CLIENTRECTCHANGE,
                TXTBIT_CLIENTRECTCHANGE);
        }
    }

    void RichView::OnFocus()
    {
        View::OnFocus();
        text_service_->TxSendMessage(WM_SETFOCUS, 0, 0, 0);
    }

    void RichView::OnBlur()
    {
        text_service_->TxSendMessage(WM_KILLFOCUS, 0, 0, 0);
        View::OnBlur();
    }

    void RichView::ViewHierarchyChanged(bool is_add, View* parent, View* child)
    {
        if(is_add && GetWidget() && !initialized_)
        {
            if(FAILED(InitDefaultCharFormat(cf_)))
            {
                NOTREACHED();
            }

            if(FAILED(InitDefaultParaFormat(pf_)))
            {
                NOTREACHED();
            }

            initialized_ = true;

            if(!(dwStyle_ & (ES_AUTOHSCROLL|WS_HSCROLL)))
            {
                fWordWrap_ = TRUE;
            }

            if(!(dwStyle_ & ES_LEFT))
            {
                if(dwStyle_ & ES_CENTER)
                {
                    pf_.wAlignment = PFA_CENTER;
                }
                else if(dwStyle_ & ES_RIGHT)
                {
                    pf_.wAlignment = PFA_RIGHT;
                }
            }

            fInplaceActive_ = TRUE;

            base::win::ScopedComPtr<IUnknown> unknown;
            if(FAILED(CreateTextServices(NULL, this, unknown.Receive())))
            {
                NOTREACHED();
            }

            if(FAILED(unknown.QueryInterface(IID_ITextServices, (void**)&text_service_)))
            {
                NOTREACHED();
            }
            
            // notify Text Services that we are in place active
            RECT rcClient = GetLocalBounds().ToRECT();
            if(FAILED(text_service_->OnTxInPlaceActivate(&rcClient)))
            {
                NOTREACHED();
            }
        }
    }

    std::string RichView::GetClassName() const
    {
        return kViewClassName;
    }

    IDropTarget* RichView::GetDropTarget()
    {
        IDropTarget* drop_target = NULL;
        text_service_->TxGetDropTarget(&drop_target);
        return drop_target;
    }

    // static
    void RichView::InitializeDC(HDC context)
    {
        // ��������仯.
        // ���������GM_ADVANCEDͼ��ģʽ, GDI���߼��ռ���������ʱ�ӷ�����ƻ�. ��
        // �ȼ�����GM_ADVANCEDͼ��ģʽ��, ���Ŀ��Ƶ�ͻ�������ȫ����DC�����絽�豸
        // �任.
        BOOL res = SetGraphicsMode(context, GM_ADVANCED);
        SkASSERT(res != 0);

        // ���ö���.
        res = SetStretchBltMode(context, HALFTONE);
        SkASSERT(res != 0);
        // ����SetStretchBltMode()�ĵ�, ���������SetBrushOrgEx().
        res = SetBrushOrgEx(context, 0, 0, NULL);
        SkASSERT(res != 0);

        // ����ȱʡ�ķ���.
        res = SetArcDirection(context, AD_CLOCKWISE);
        SkASSERT(res != 0);

        // ����ȱʡ����ɫ.
        res = SetBkColor(context, RGB(255, 255, 255));
        SkASSERT(res != CLR_INVALID);
        res = SetTextColor(context, RGB(0, 0, 0));
        SkASSERT(res != CLR_INVALID);
        res = SetDCBrushColor(context, RGB(255, 255, 255));
        SkASSERT(res != CLR_INVALID);
        res = SetDCPenColor(context, RGB(0, 0, 0));
        SkASSERT(res != CLR_INVALID);

        // ����ȱʡ��͸����.
        res = SetBkMode(context, OPAQUE);
        SkASSERT(res != 0);
        res = SetROP2(context, R2_COPYPEN);
        SkASSERT(res != 0);
    }

} //namespace view