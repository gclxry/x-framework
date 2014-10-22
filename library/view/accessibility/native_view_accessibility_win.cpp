
#include "native_view_accessibility_win.h"

#include "ui_base/view_prop.h"

#include "view/widget/native_widget_win.h"

#pragma comment(lib, "oleacc.lib")

namespace view
{
    const char kViewNativeHostPropForAccessibility[] =
        "View_NativeViewHostHWNDForAccessibility";
}

// static
scoped_refptr<NativeViewAccessibilityWin> NativeViewAccessibilityWin::Create(
    view::View* view)
{
    CComObject<NativeViewAccessibilityWin>* instance = NULL;
    HRESULT hr = CComObject<NativeViewAccessibilityWin>::CreateInstance(
        &instance);
    DCHECK(SUCCEEDED(hr));
    instance->set_view(view);
    return scoped_refptr<NativeViewAccessibilityWin>(instance);
}

NativeViewAccessibilityWin::NativeViewAccessibilityWin() : view_(NULL) {}

NativeViewAccessibilityWin::~NativeViewAccessibilityWin() {}

// TODO(ctguil): Handle case where child View is not contained by parent.
STDMETHODIMP NativeViewAccessibilityWin::accHitTest(
    LONG x_left, LONG y_top, VARIANT* child)
{
    if(!child)
    {
        return E_INVALIDARG;
    }

    if(!view_)
    {
        return E_FAIL;
    }

    gfx::Point point(x_left, y_top);
    view::View::ConvertPointToView(NULL, view_, &point);

    if(!view_->HitTest(point))
    {
        // If containing parent is not hit, return with failure.
        child->vt = VT_EMPTY;
        return S_FALSE;
    }

    view::View* view = view_->GetEventHandlerForPoint(point);
    if(view == view_)
    {
        // No child hit, return parent id.
        child->vt = VT_I4;
        child->lVal = CHILDID_SELF;
    }
    else
    {
        child->vt = VT_DISPATCH;
        child->pdispVal = view->GetNativeViewAccessible();
        child->pdispVal->AddRef();
    }
    return S_OK;
}

HRESULT NativeViewAccessibilityWin::accDoDefaultAction(VARIANT var_id)
{
    if(!IsValidId(var_id))
    {
        return E_INVALIDARG;
    }

    // The object does not support the method. This value is returned for
    // controls that do not perform actions, such as edit fields.
    return DISP_E_MEMBERNOTFOUND;
}

STDMETHODIMP NativeViewAccessibilityWin::accLocation(
    LONG* x_left, LONG* y_top, LONG* width, LONG* height, VARIANT var_id)
{
    if(!IsValidId(var_id) || !x_left || !y_top || !width || !height)
    {
        return E_INVALIDARG;
    }

    if(!view_)
    {
        return E_FAIL;
    }

    if(!view_->bounds().IsEmpty())
    {
        *width  = view_->width();
        *height = view_->height();
        gfx::Point topleft(view_->bounds().origin());
        view::View::ConvertPointToScreen(
            view_->parent()?view_->parent():view_, &topleft);
        *x_left = topleft.x();
        *y_top  = topleft.y();
    }
    else
    {
        return E_FAIL;
    }
    return S_OK;
}

STDMETHODIMP NativeViewAccessibilityWin::accNavigate(
    LONG nav_dir, VARIANT start, VARIANT* end)
{
    if(start.vt != VT_I4 || !end)
    {
        return E_INVALIDARG;
    }

    if(!view_)
    {
        return E_FAIL;
    }

    switch(nav_dir)
    {
    case NAVDIR_FIRSTCHILD:
    case NAVDIR_LASTCHILD:
        {
            if(start.lVal != CHILDID_SELF)
            {
                // Start of navigation must be on the View itself.
                return E_INVALIDARG;
            }
            else if(!view_->has_children())
            {
                // No children found.
                return S_FALSE;
            }

            // Set child_id based on first or last child.
            int child_id = 0;
            if(nav_dir == NAVDIR_LASTCHILD)
            {
                child_id = view_->child_count() - 1;
            }

            view::View* child = view_->child_at(child_id);
            end->vt = VT_DISPATCH;
            end->pdispVal = child->GetNativeViewAccessible();
            end->pdispVal->AddRef();
            return S_OK;
        }
    case NAVDIR_LEFT:
    case NAVDIR_UP:
    case NAVDIR_PREVIOUS:
    case NAVDIR_RIGHT:
    case NAVDIR_DOWN:
    case NAVDIR_NEXT:
        {
            // Retrieve parent to access view index and perform bounds checking.
            view::View* parent = view_->parent();
            if(!parent)
            {
                return E_FAIL;
            }

            if(start.lVal == CHILDID_SELF)
            {
                int view_index = parent->GetIndexOf(view_);
                // Check navigation bounds, adjusting for View child indexing (MSAA
                // child indexing starts with 1, whereas View indexing starts with 0).
                if(!IsValidNav(nav_dir, view_index, -1, parent->child_count()-1))
                {
                    // Navigation attempted to go out-of-bounds.
                    end->vt = VT_EMPTY;
                    return S_FALSE;
                }
                else
                {
                    if(IsNavDirNext(nav_dir))
                    {
                        view_index += 1;
                    }
                    else
                    {
                        view_index -=1;
                    }
                }

                view::View* child = parent->child_at(view_index);
                end->pdispVal = child->GetNativeViewAccessible();
                end->vt = VT_DISPATCH;
                end->pdispVal->AddRef();
                return S_OK;
            }
            else
            {
                // Check navigation bounds, adjusting for MSAA child indexing (MSAA
                // child indexing starts with 1, whereas View indexing starts with 0).
                if(!IsValidNav(nav_dir, start.lVal, 0, parent->child_count()+1))
                {
                    // Navigation attempted to go out-of-bounds.
                    end->vt = VT_EMPTY;
                    return S_FALSE;
                }
                else
                {
                    if(IsNavDirNext(nav_dir))
                    {
                        start.lVal += 1;
                    }
                    else
                    {
                        start.lVal -= 1;
                    }
                }

                HRESULT result = this->get_accChild(start, &end->pdispVal);
                if(result == S_FALSE)
                {
                    // Child is a leaf.
                    end->vt = VT_I4;
                    end->lVal = start.lVal;
                }
                else if(result == E_INVALIDARG)
                {
                    return E_INVALIDARG;
                }
                else
                {
                    // Child is not a leaf.
                    end->vt = VT_DISPATCH;
                }
            }
            break;
        }
    default:
        return E_INVALIDARG;
    }
    // Navigation performed correctly. Global return for this function, if no
    // error triggered an escape earlier.
    return S_OK;
}

STDMETHODIMP NativeViewAccessibilityWin::get_accChild(VARIANT var_child,
                                                      IDispatch** disp_child)
{
    if(var_child.vt!=VT_I4 || !disp_child)
    {
        return E_INVALIDARG;
    }

    if(!view_)
    {
        return E_FAIL;
    }

    LONG child_id = V_I4(&var_child);

    if(child_id == CHILDID_SELF)
    {
        // Remain with the same dispatch.
        return S_OK;
    }

    view::View* child_view = NULL;
    if(child_id > 0)
    {
        int child_id_as_index = child_id - 1;
        if(child_id_as_index < view_->child_count())
        {
            // Note: child_id is a one based index when indexing children.
            child_view = view_->child_at(child_id_as_index);
        }
        else
        {
            // Attempt to retrieve a child view with the specified id.
            child_view = view_->GetViewByID(child_id);
        }
    }
    else
    {
        // Negative values are used for events fired using the view's
        // NativeWidgetWin.
        view::NativeWidgetWin* widget = static_cast<view::NativeWidgetWin*>(
            view_->GetWidget()->native_widget());
        child_view = widget->GetAccessibilityViewEventAt(child_id);
    }

    if(!child_view)
    {
        // No child found.
        *disp_child = NULL;
        return E_FAIL;
    }

    *disp_child = child_view->GetNativeViewAccessible();
    (*disp_child)->AddRef();
    return S_OK;
}

STDMETHODIMP NativeViewAccessibilityWin::get_accChildCount(LONG* child_count)
{
    if(!child_count || !view_)
    {
        return E_INVALIDARG;
    }

    if(!view_)
    {
        return E_FAIL;
    }

    *child_count = view_->child_count();
    return S_OK;
}

STDMETHODIMP NativeViewAccessibilityWin::get_accDefaultAction(
    VARIANT var_id, BSTR* def_action)
{
    if(!IsValidId(var_id) || !def_action)
    {
        return E_INVALIDARG;
    }

    if(!view_)
    {
        return E_FAIL;
    }

    ui::AccessibleViewState state;
    view_->GetAccessibleState(&state);
    string16 temp_action = state.default_action;

    if(!temp_action.empty())
    {
        *def_action = SysAllocString(temp_action.c_str());
    }
    else
    {
        return S_FALSE;
    }

    return S_OK;
}

STDMETHODIMP NativeViewAccessibilityWin::get_accDescription(
    VARIANT var_id, BSTR* desc)
{
    if(!IsValidId(var_id) || !desc)
    {
        return E_INVALIDARG;
    }

    if(!view_)
    {
        return E_FAIL;
    }

    string16 temp_desc;

    view_->GetTooltipText(gfx::Point(), &temp_desc);
    if(!temp_desc.empty())
    {
        *desc = SysAllocString(temp_desc.c_str());
    }
    else
    {
        return S_FALSE;
    }

    return S_OK;
}

STDMETHODIMP NativeViewAccessibilityWin::get_accFocus(VARIANT* focus_child)
{
    if(!focus_child)
    {
        return E_INVALIDARG;
    }

    if(!view_)
    {
        return E_FAIL;
    }

    view::View* focus = NULL;
    view::FocusManager* focus_manager = view_->GetFocusManager();
    if(focus_manager)
    {
        focus = focus_manager->GetFocusedView();
    }
    if(focus == view_)
    {
        // This view has focus.
        focus_child->vt = VT_I4;
        focus_child->lVal = CHILDID_SELF;
    }
    else if(focus && view_->Contains(focus))
    {
        // Return the child object that has the keyboard focus.
        focus_child->pdispVal = focus->GetNativeViewAccessible();
        focus_child->pdispVal->AddRef();
        return S_OK;
    }
    else
    {
        // Neither this object nor any of its children has the keyboard focus.
        focus_child->vt = VT_EMPTY;
    }
    return S_OK;
}

STDMETHODIMP NativeViewAccessibilityWin::get_accKeyboardShortcut(
    VARIANT var_id, BSTR* acc_key)
{
    if(!IsValidId(var_id) || !acc_key)
    {
        return E_INVALIDARG;
    }

    if(!view_)
    {
        return E_FAIL;
    }

    ui::AccessibleViewState state;
    view_->GetAccessibleState(&state);
    string16 temp_key = state.keyboard_shortcut;

    if(!temp_key.empty())
    {
        *acc_key = SysAllocString(temp_key.c_str());
    }
    else
    {
        return S_FALSE;
    }

    return S_OK;
}

STDMETHODIMP NativeViewAccessibilityWin::get_accName(
    VARIANT var_id, BSTR* name)
{
    if(!IsValidId(var_id) || !name)
    {
        return E_INVALIDARG;
    }

    if(!view_)
    {
        return E_FAIL;
    }

    // Retrieve the current view's name.
    ui::AccessibleViewState state;
    view_->GetAccessibleState(&state);
    string16 temp_name = state.name;
    if(!temp_name.empty())
    {
        // Return name retrieved.
        *name = SysAllocString(temp_name.c_str());
    }
    else
    {
        // If view has no name, return S_FALSE.
        return S_FALSE;
    }

    return S_OK;
}

STDMETHODIMP NativeViewAccessibilityWin::get_accParent(
    IDispatch** disp_parent)
{
    if(!disp_parent)
    {
        return E_INVALIDARG;
    }

    if(!view_)
    {
        return E_FAIL;
    }

    view::View* parent_view = view_->parent();

    if(!parent_view)
    {
        // This function can get called during teardown of WidetWin so we
        // should bail out if we fail to get the HWND.
        if(!view_->GetWidget() || !view_->GetWidget()->GetNativeView())
        {
            *disp_parent = NULL;
            return S_FALSE;
        }

        // For a View that has no parent (e.g. root), point the accessible parent
        // to the default implementation, to interface with Windows' hierarchy
        // and to support calls from e.g. WindowFromAccessibleObject.
        HRESULT hr = ::AccessibleObjectFromWindow(
            view_->GetWidget()->GetNativeView(),
            OBJID_WINDOW, IID_IAccessible,
            reinterpret_cast<void**>(disp_parent));

        if(!SUCCEEDED(hr))
        {
            *disp_parent = NULL;
            return S_FALSE;
        }

        return S_OK;
    }

    *disp_parent = parent_view->GetNativeViewAccessible();
    (*disp_parent)->AddRef();
    return S_OK;
}

STDMETHODIMP NativeViewAccessibilityWin::get_accRole(
    VARIANT var_id, VARIANT* role)
{
    if(!IsValidId(var_id) || !role)
    {
        return E_INVALIDARG;
    }

    if(!view_)
    {
        return E_FAIL;
    }

    ui::AccessibleViewState state;
    view_->GetAccessibleState(&state);
    role->vt = VT_I4;
    role->lVal = MSAARole(state.role);
    return S_OK;
}

STDMETHODIMP NativeViewAccessibilityWin::get_accState(
    VARIANT var_id, VARIANT* state)
{
    if(!IsValidId(var_id) || !state)
    {
        return E_INVALIDARG;
    }

    if(!view_)
    {
        return E_FAIL;
    }

    state->vt = VT_I4;

    // Retrieve all currently applicable states of the parent.
    SetState(state, view_);

    // Make sure that state is not empty, and has the proper type.
    if(state->vt == VT_EMPTY)
    {
        return E_FAIL;
    }

    return S_OK;
}

STDMETHODIMP NativeViewAccessibilityWin::get_accValue(
    VARIANT var_id, BSTR* value)
{
    if(!IsValidId(var_id) || !value)
    {
        return E_INVALIDARG;
    }

    if(!view_)
    {
        return E_FAIL;
    }

    // Retrieve the current view's value.
    ui::AccessibleViewState state;
    view_->GetAccessibleState(&state);
    string16 temp_value = state.value;

    if(!temp_value.empty())
    {
        // Return value retrieved.
        *value = SysAllocString(temp_value.c_str());
    }
    else
    {
        // If view has no value, fall back into the default implementation.
        *value = NULL;
        return E_NOTIMPL;
    }

    return S_OK;
}

// Helper functions.

bool NativeViewAccessibilityWin::IsNavDirNext(int nav_dir) const
{
    if(nav_dir==NAVDIR_RIGHT || nav_dir==NAVDIR_DOWN ||
        nav_dir==NAVDIR_NEXT)
    {
        return true;
    }
    return false;
}

bool NativeViewAccessibilityWin::IsValidNav(
    int nav_dir, int start_id, int lower_bound, int upper_bound) const
{
    if (IsNavDirNext(nav_dir))
    {
        if((start_id+1) > upper_bound)
        {
            return false;
        }
    }
    else
    {
        if((start_id-1) <= lower_bound)
        {
            return false;
        }
    }
    return true;
}

bool NativeViewAccessibilityWin::IsValidId(const VARIANT& child) const
{
    // View accessibility returns an IAccessible for each view so we only support
    // the CHILDID_SELF id.
    return (VT_I4==child.vt) && (CHILDID_SELF==child.lVal);
}

void NativeViewAccessibilityWin::SetState(
    VARIANT* msaa_state, view::View* view)
{
    // Ensure the output param is initialized to zero.
    msaa_state->lVal = 0;

    // Default state; all views can have accessibility focus.
    msaa_state->lVal |= STATE_SYSTEM_FOCUSABLE;

    if(!view)
    {
        return;
    }

    if(!view->IsEnabled())
    {
        msaa_state->lVal |= STATE_SYSTEM_UNAVAILABLE;
    }
    if(!view->IsVisible())
    {
        msaa_state->lVal |= STATE_SYSTEM_INVISIBLE;
    }
    if(view->IsHotTracked())
    {
        msaa_state->lVal |= STATE_SYSTEM_HOTTRACKED;
    }
    if(view->HasFocus())
    {
        msaa_state->lVal |= STATE_SYSTEM_FOCUSED;
    }

    // Add on any view-specific states.
    ui::AccessibleViewState view_state;
    view->GetAccessibleState(&view_state);
    msaa_state->lVal |= MSAAState(view_state.state);
}

// IAccessible functions not supported.

STDMETHODIMP NativeViewAccessibilityWin::get_accSelection(VARIANT* selected)
{
    if(selected)
    {
        selected->vt = VT_EMPTY;
    }
    return E_NOTIMPL;
}

STDMETHODIMP NativeViewAccessibilityWin::accSelect(
    LONG flagsSelect, VARIANT var_id)
{
    return E_NOTIMPL;
}

STDMETHODIMP NativeViewAccessibilityWin::get_accHelp(
    VARIANT var_id, BSTR* help)
{
    if(help)
    {
        *help = NULL;
    }
    return E_NOTIMPL;
}

STDMETHODIMP NativeViewAccessibilityWin::get_accHelpTopic(
    BSTR* help_file, VARIANT var_id, LONG* topic_id)
{
    if(help_file)
    {
        *help_file = NULL;
    }
    if(topic_id)
    {
        *topic_id = static_cast<LONG>(-1);
    }
    return E_NOTIMPL;
}

STDMETHODIMP NativeViewAccessibilityWin::put_accName(
    VARIANT var_id, BSTR put_name)
{
    // Deprecated.
    return E_NOTIMPL;
}

STDMETHODIMP NativeViewAccessibilityWin::put_accValue(
    VARIANT var_id, BSTR put_val)
{
    // Deprecated.
    return E_NOTIMPL;
}

int32 NativeViewAccessibilityWin::MSAAEvent(ui::AccessibilityTypes::Event event)
{
    switch(event)
    {
    case ui::AccessibilityTypes::EVENT_ALERT:
        return EVENT_SYSTEM_ALERT;
    case ui::AccessibilityTypes::EVENT_FOCUS:
        return EVENT_OBJECT_FOCUS;
    case ui::AccessibilityTypes::EVENT_MENUSTART:
        return EVENT_SYSTEM_MENUSTART;
    case ui::AccessibilityTypes::EVENT_MENUEND:
        return EVENT_SYSTEM_MENUEND;
    case ui::AccessibilityTypes::EVENT_MENUPOPUPSTART:
        return EVENT_SYSTEM_MENUPOPUPSTART;
    case ui::AccessibilityTypes::EVENT_MENUPOPUPEND:
        return EVENT_SYSTEM_MENUPOPUPEND;
    case ui::AccessibilityTypes::EVENT_NAME_CHANGED:
        return EVENT_OBJECT_NAMECHANGE;
    case ui::AccessibilityTypes::EVENT_TEXT_CHANGED:
        return EVENT_OBJECT_VALUECHANGE;
    case ui::AccessibilityTypes::EVENT_SELECTION_CHANGED:
        return EVENT_OBJECT_TEXTSELECTIONCHANGED;
    case ui::AccessibilityTypes::EVENT_VALUE_CHANGED:
        return EVENT_OBJECT_VALUECHANGE;
    default:
        // Not supported or invalid event.
        NOTREACHED();
        return -1;
    }
}

int32 NativeViewAccessibilityWin::MSAARole(ui::AccessibilityTypes::Role role)
{
    switch(role)
    {
    case ui::AccessibilityTypes::ROLE_ALERT:
        return ROLE_SYSTEM_ALERT;
    case ui::AccessibilityTypes::ROLE_APPLICATION:
        return ROLE_SYSTEM_APPLICATION;
    case ui::AccessibilityTypes::ROLE_BUTTONDROPDOWN:
        return ROLE_SYSTEM_BUTTONDROPDOWN;
    case ui::AccessibilityTypes::ROLE_BUTTONMENU:
        return ROLE_SYSTEM_BUTTONMENU;
    case ui::AccessibilityTypes::ROLE_CHECKBUTTON:
        return ROLE_SYSTEM_CHECKBUTTON;
    case ui::AccessibilityTypes::ROLE_COMBOBOX:
        return ROLE_SYSTEM_COMBOBOX;
    case ui::AccessibilityTypes::ROLE_DIALOG:
        return ROLE_SYSTEM_DIALOG;
    case ui::AccessibilityTypes::ROLE_GRAPHIC:
        return ROLE_SYSTEM_GRAPHIC;
    case ui::AccessibilityTypes::ROLE_GROUPING:
        return ROLE_SYSTEM_GROUPING;
    case ui::AccessibilityTypes::ROLE_LINK:
        return ROLE_SYSTEM_LINK;
    case ui::AccessibilityTypes::ROLE_LOCATION_BAR:
        return ROLE_SYSTEM_GROUPING;
    case ui::AccessibilityTypes::ROLE_MENUBAR:
        return ROLE_SYSTEM_MENUBAR;
    case ui::AccessibilityTypes::ROLE_MENUITEM:
        return ROLE_SYSTEM_MENUITEM;
    case ui::AccessibilityTypes::ROLE_MENUPOPUP:
        return ROLE_SYSTEM_MENUPOPUP;
    case ui::AccessibilityTypes::ROLE_OUTLINE:
        return ROLE_SYSTEM_OUTLINE;
    case ui::AccessibilityTypes::ROLE_OUTLINEITEM:
        return ROLE_SYSTEM_OUTLINEITEM;
    case ui::AccessibilityTypes::ROLE_PAGETAB:
        return ROLE_SYSTEM_PAGETAB;
    case ui::AccessibilityTypes::ROLE_PAGETABLIST:
        return ROLE_SYSTEM_PAGETABLIST;
    case ui::AccessibilityTypes::ROLE_PANE:
        return ROLE_SYSTEM_PANE;
    case ui::AccessibilityTypes::ROLE_PROGRESSBAR:
        return ROLE_SYSTEM_PROGRESSBAR;
    case ui::AccessibilityTypes::ROLE_PUSHBUTTON:
        return ROLE_SYSTEM_PUSHBUTTON;
    case ui::AccessibilityTypes::ROLE_RADIOBUTTON:
        return ROLE_SYSTEM_RADIOBUTTON;
    case ui::AccessibilityTypes::ROLE_SCROLLBAR:
        return ROLE_SYSTEM_SCROLLBAR;
    case ui::AccessibilityTypes::ROLE_SEPARATOR:
        return ROLE_SYSTEM_SEPARATOR;
    case ui::AccessibilityTypes::ROLE_STATICTEXT:
        return ROLE_SYSTEM_STATICTEXT;
    case ui::AccessibilityTypes::ROLE_TEXT:
        return ROLE_SYSTEM_TEXT;
    case ui::AccessibilityTypes::ROLE_TITLEBAR:
        return ROLE_SYSTEM_TITLEBAR;
    case ui::AccessibilityTypes::ROLE_TOOLBAR:
        return ROLE_SYSTEM_TOOLBAR;
    case ui::AccessibilityTypes::ROLE_WINDOW:
        return ROLE_SYSTEM_WINDOW;
    case ui::AccessibilityTypes::ROLE_CLIENT:
    default:
        // This is the default role for MSAA.
        return ROLE_SYSTEM_CLIENT;
    }
}

int32 NativeViewAccessibilityWin::MSAAState(ui::AccessibilityTypes::State state)
{
    int32 msaa_state = 0;
    if(state & ui::AccessibilityTypes::STATE_CHECKED)
    {
        msaa_state |= STATE_SYSTEM_CHECKED;
    }
    if(state & ui::AccessibilityTypes::STATE_COLLAPSED)
    {
        msaa_state |= STATE_SYSTEM_COLLAPSED;
    }
    if(state & ui::AccessibilityTypes::STATE_DEFAULT)
    {
        msaa_state |= STATE_SYSTEM_DEFAULT;
    }
    if(state & ui::AccessibilityTypes::STATE_EXPANDED)
    {
        msaa_state |= STATE_SYSTEM_EXPANDED;
    }
    if(state & ui::AccessibilityTypes::STATE_HASPOPUP)
    {
        msaa_state |= STATE_SYSTEM_HASPOPUP;
    }
    if(state & ui::AccessibilityTypes::STATE_HOTTRACKED)
    {
        msaa_state |= STATE_SYSTEM_HOTTRACKED;
    }
    if(state & ui::AccessibilityTypes::STATE_INVISIBLE)
    {
        msaa_state |= STATE_SYSTEM_INVISIBLE;
    }
    if(state & ui::AccessibilityTypes::STATE_LINKED)
    {
        msaa_state |= STATE_SYSTEM_LINKED;
    }
    if(state & ui::AccessibilityTypes::STATE_OFFSCREEN)
    {
        msaa_state |= STATE_SYSTEM_OFFSCREEN;
    }
    if(state & ui::AccessibilityTypes::STATE_PRESSED)
    {
        msaa_state |= STATE_SYSTEM_PRESSED;
    }
    if(state & ui::AccessibilityTypes::STATE_PROTECTED)
    {
        msaa_state |= STATE_SYSTEM_PROTECTED;
    }
    if(state & ui::AccessibilityTypes::STATE_READONLY)
    {
        msaa_state |= STATE_SYSTEM_READONLY;
    }
    if(state & ui::AccessibilityTypes::STATE_SELECTED)
    {
        msaa_state |= STATE_SYSTEM_SELECTED;
    }
    if(state & ui::AccessibilityTypes::STATE_FOCUSED)
    {
        msaa_state |= STATE_SYSTEM_FOCUSED;
    }
    if(state & ui::AccessibilityTypes::STATE_UNAVAILABLE)
    {
        msaa_state |= STATE_SYSTEM_UNAVAILABLE;
    }
    return msaa_state;
}