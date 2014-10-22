
#ifndef __ui_base_events_h__
#define __ui_base_events_h__

#pragma once

namespace ui
{

    // �¼�����. (���ڻ��windows��ͷ�ļ���ͻ, ���Լ���ǰ׺)
    enum EventType
    {
        ET_UNKNOWN = 0,
        ET_MOUSE_PRESSED,
        ET_MOUSE_DRAGGED,
        ET_MOUSE_RELEASED,
        ET_MOUSE_MOVED,
        ET_MOUSE_ENTERED,
        ET_MOUSE_EXITED,
        ET_KEY_PRESSED,
        ET_KEY_RELEASED,
        ET_MOUSEWHEEL,
        ET_DROP_TARGET_EVENT,
        ET_FOCUS_CHANGE,
    };

    // ��ǰ֧�ֵ��¼����.
    enum EventFlags
    {
        EF_CAPS_LOCK_DOWN     = 1 << 0,
        EF_SHIFT_DOWN         = 1 << 1,
        EF_CONTROL_DOWN       = 1 << 2,
        EF_ALT_DOWN           = 1 << 3,
        EF_LEFT_BUTTON_DOWN   = 1 << 4,
        EF_MIDDLE_BUTTON_DOWN = 1 << 5,
        EF_RIGHT_BUTTON_DOWN  = 1 << 6,
        EF_COMMAND_DOWN       = 1 << 7, // ֻ����OSX��.
    };

    // ����¼�����ϸ���.
    enum MouseEventFlags
    {
        EF_IS_DOUBLE_CLICK = 1 << 16,
        EF_IS_NON_CLIENT = 1 << 17
    };

} //namespace ui

#endif //__ui_base_events_h__