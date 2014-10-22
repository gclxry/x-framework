
#ifndef __view_drop_helper_h__
#define __view_drop_helper_h__

#pragma once

#include <objidl.h>

#include "base/basic_types.h"

namespace gfx
{
    class Point;
}

namespace view
{

    class View;

    // DropHelperΪ������ק������Ŀ����ͼ�ṩ֧��. DropHelper����ϵͳ�ϷŽ�������
    // ��ʹ��. ϵͳ��������ƶ�ʱ�����OnDragOver, ���ϷŽ���ʱ����OnDragExit����
    // OnDrop.
    class DropHelper
    {
    public:
        explicit DropHelper(View* root_view);
        ~DropHelper();

        // ��ǰ�Ϸ��¼���Ŀ����ͼ, ����ΪNULL.
        View* target_view() const { return target_view_; }

        // ���ش���DropHelper��RootView.
        View* root_view() const { return root_view_; }

        // ����target_view_ΪNULL, �������|view|.
        //
        // ����ͼ��RootView�Ƴ�ʱ����, ȷ��Ŀ����ͼ��Ϊ�Ƿ�ֵ.
        void ResetTargetViewIfEquals(View* view);

        // ��ק�����е�����ϵ�����ͼ��ʱ����. ����ΪĿ����ͼ����DragDropTypes���͵�
        // ���. ���û����ͼ�����Ϸ�, ����DRAG_NONE.
        DWORD OnDragOver(IDataObject* data_object,
            DWORD key_state,
            POINTL cursor_position,
            DWORD effect);

        // ��ק�����е�����ϵ�����ͼ����ʱ����.
        void OnDragLeave();

        // ��ק�����е�����Ϸŵ�����ͼʱ����. ����ֵ�μ�OnDragOver.
        //
        // ע��: ʵ���ڵ��ñ�����֮ǰ�������OnDragOver, �ṩ����ֵ����
        // drag_operation.
        DWORD OnDrop(IDataObject* data_object,
            DWORD key_state,
            POINTL cursor_position,
            DWORD effect);

        // ���ݸ���ͼ����ϵ�и���λ�ü����Ϸŵ�Ŀ����ͼ. ����������target_view_
        // ��, �����ظ���ѯCanDrop.
        View* CalculateTargetView(const gfx::Point& root_view_location,
            IDataObject* data_object);

    private:
        // CalculateTargetView��ʵ��. ���|deepest_view|�ǿ�, �᷵��RootView�����
        // ����|root_view_location|�������ͼ.
        View* CalculateTargetViewImpl(const gfx::Point& root_view_location,
            IDataObject* data_object,
            View** deepest_view);

        // ������ȷ���Ϸ�֪ͨ��Ϣ��Ŀ����ͼ. ���Ŀ����ͼΪ��, ʲôҲ����.
        void NotifyDragEntered(IDataObject* data_object,
            DWORD key_state,
            POINTL cursor_position,
            DWORD effect);
        DWORD NotifyDragOver(IDataObject* data_object,
            DWORD key_state,
            POINTL cursor_position,
            DWORD effect);
        void NotifyDragLeave();

        // �����������RootView.
        View* root_view_;

        // �����¼���Ŀ����ͼ.
        View* target_view_;

        // ��ǰ�Ϸ��������������ͼ.
        View* deepest_view_;

        DISALLOW_COPY_AND_ASSIGN(DropHelper);
    };

} //namespace view

#endif //__view_drop_helper_h__