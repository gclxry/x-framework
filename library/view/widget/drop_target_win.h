
#ifndef __view_drop_target_win_h__
#define __view_drop_target_win_h__

#pragma once

#include "ui_base/dragdrop/drop_target.h"

#include "drop_helper.h"

namespace view
{

    class View;
    namespace internal
    {
        class RootView;
    }

    // DropTargetWin����WidgetWin����ק����. ����Windows OLE���Ϸ���Ϣת��
    // ��View���Ϸ���Ϣ.
    //
    // DropTargetWinʹ��DropHelper����Ŀ����ͼ���Ϸ���Ϣ.
    class DropTargetWin : public ui::DropTarget
    {
    public:
        explicit DropTargetWin(internal::RootView* root_view);
        virtual ~DropTargetWin();

        // �������ִ����ק, ����ͼ���ǵ�ǰ�Ϸ�Ŀ����ͼ, �Ϸ�Ŀ������Ϊnull.
        // ����ͼ��RootView���Ƴ�ʱ����, ȷ��Ŀ����ͼ��Ϊ�Ƿ�ֵ.
        void ResetTargetViewIfEquals(View* view);

    protected:
        virtual DWORD OnDragOver(IDataObject* data_object,
            DWORD key_state,
            POINTL cursor_position,
            DWORD effect);

        virtual void OnDragLeave(IDataObject* data_object);

        virtual DWORD OnDrop(IDataObject* data_object,
            DWORD key_state,
            POINTL cursor_position,
            DWORD effect);

    private:
        view::DropHelper helper_;

        DISALLOW_COPY_AND_ASSIGN(DropTargetWin);
    };

} //namespace view

#endif //__view_drop_target_win_h__