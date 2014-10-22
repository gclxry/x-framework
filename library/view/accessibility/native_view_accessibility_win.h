
#ifndef __view_native_view_accessibility_win_h__
#define __view_native_view_accessibility_win_h__

#pragma once

#include <atlbase.h>
#include <atlcom.h>

#include "base/memory/scoped_ptr.h"

#include "ui_base/accessibility/accessible_view_state.h"

#include "view/controls/native/native_view_host.h"
#include "view/view.h"

// ע��: ��Ҫ��NativeViewAccessibilityWin�������ֿռ�"view"��;
// Visual Studio 2005������ATL::CComObject���������ֿռ���.

////////////////////////////////////////////////////////////////////////////////
//
// NativeViewAccessibilityWin
//
// ΪViewʵ��MSAA IAccessible COM�ӿ�, Ϊ�ṩ��Ļ�Ķ��߻���������������(AT)�ṩ
// ������֧��.
//
////////////////////////////////////////////////////////////////////////////////
class ATL_NO_VTABLE NativeViewAccessibilityWin
    : public CComObjectRootEx<CComMultiThreadModel>,
    public IDispatchImpl<IAccessible, &IID_IAccessible, &LIBID_Accessibility>
{
public:
    BEGIN_COM_MAP(NativeViewAccessibilityWin)
        COM_INTERFACE_ENTRY2(IDispatch, IAccessible)
        COM_INTERFACE_ENTRY(IAccessible)
    END_COM_MAP()

    // ��ͼ�ɷ����Դ�������.
    static scoped_refptr<NativeViewAccessibilityWin> Create(view::View* view);

    virtual ~NativeViewAccessibilityWin();

    void set_view(view::View* view) { view_ = view; }

    // ֧�ֵ�IAccessible����.

    // ������Ļ�ϸ��������Ԫ�ػ����Ӷ���.
    STDMETHODIMP accHitTest(LONG x_left, LONG y_top, VARIANT* child);

    // ִ�ж����ȱʡ����.
    STDMETHODIMP accDoDefaultAction(VARIANT var_id);

    // ����ָ������ĵ�ǰ��Ļλ��.
    STDMETHODIMP accLocation(LONG* x_left,
        LONG* y_top,
        LONG* width,
        LONG* height,
        VARIANT var_id);

    // ����������UIԪ�ز�����.
    STDMETHODIMP accNavigate(LONG nav_dir, VARIANT start, VARIANT* end);

    // ����ָ�����ӵ�IDispatch�ӿ�ָ��.
    STDMETHODIMP get_accChild(VARIANT var_child, IDispatch** disp_child);

    // ���ؿɷ��ʵĺ�����Ŀ.
    STDMETHODIMP get_accChildCount(LONG* child_count);

    // ���ض���ȱʡ�������ַ�������.
    STDMETHODIMP get_accDefaultAction(VARIANT var_id, BSTR* default_action);

    // ������ʾ��Ϣ.
    STDMETHODIMP get_accDescription(VARIANT var_id, BSTR* desc);

    // ���ض����Ƿ��м��̽���.
    STDMETHODIMP get_accFocus(VARIANT* focus_child);

    // ����ָ������Ŀ�ݼ�.
    STDMETHODIMP get_accKeyboardShortcut(VARIANT var_id, BSTR* access_key);

    // ����ָ�����������.
    STDMETHODIMP get_accName(VARIANT var_id, BSTR* name);

    // ���ض����׵�IDispatch�ӿ�ָ��.
    STDMETHODIMP get_accParent(IDispatch** disp_parent);

    // ����ָ������Ľ�ɫ������Ϣ.
    STDMETHODIMP get_accRole(VARIANT var_id, VARIANT* role);

    // ����ָ������ĵ�ǰ״̬.
    STDMETHODIMP get_accState(VARIANT var_id, VARIANT* state);

    // ����ָ������ĵ�ǰֵ.
    STDMETHODIMP get_accValue(VARIANT var_id, BSTR* value);

    // ��֧�ֵ�IAccessible����.

    // ��ͼ��ѡ���ǲ���Ӧ�õ�.
    STDMETHODIMP get_accSelection(VARIANT* selected);
    STDMETHODIMP accSelect(LONG flags_sel, VARIANT var_id);

    // ��֧�ְ�������.
    STDMETHODIMP get_accHelp(VARIANT var_id, BSTR* help);
    STDMETHODIMP get_accHelpTopic(BSTR* help_file,
        VARIANT var_id, LONG* topic_id);

    // �����Ĺ���, ���ﲻʵ��.
    STDMETHODIMP put_accName(VARIANT var_id, BSTR put_name);
    STDMETHODIMP put_accValue(VARIANT var_id, BSTR put_val);

    // �¼�(accessibility_types.h�ж����)ת����MSAA�¼�, ������.
    static int32 MSAAEvent(ui::AccessibilityTypes::Event event);

    // ��ɫ(accessibility_types.h�ж����)ת����MSAA��ɫ, ������.
    static int32 MSAARole(ui::AccessibilityTypes::Role role);

    // ״̬(accessibility_types.h�ж����)ת����MSAA״̬, ������.
    static int32 MSAAState(ui::AccessibilityTypes::State state);

private:
    NativeViewAccessibilityWin();

    // �ж�accNavigate�ĵ�������, ���Ϻ�ǰӳ��Ϊǰ, �ҡ��ºͺ�ӳ��Ϊ��.
    // ������������Ǻ󷵻�true, ���򷵻�false.
    bool IsNavDirNext(int nav_dir) const;

    // �жϵ���Ŀ���Ƿ�������Χ. ������򷵻�true, ���򷵻�false.
    bool IsValidNav(int nav_dir,
        int start_id,
        int lower_bound,
        int upper_bound) const;

    // �ж�child�Ƿ�Ϸ�.
    bool IsValidId(const VARIANT& child) const;

    // ������ͼ��Ӧ��״̬�ĸ�������.
    void SetState(VARIANT* msaa_state, view::View* view);

    // ����CComObject�����๹�캯��.
    template<class Base> friend class CComObject;

    // ��ͼ��Ա.
    view::View* view_;

    DISALLOW_COPY_AND_ASSIGN(NativeViewAccessibilityWin);
};

#endif //__view_native_view_accessibility_win_h__