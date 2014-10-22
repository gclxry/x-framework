
#ifndef __ui_base_view_prop_h__
#define __ui_base_view_prop_h__

#pragma once

#include "base/basic_types.h"
#include "base/memory/ref_counted.h"

namespace ui
{

    // ViewPropά����ͼ�ļ�/ֵ��, �����滻Win32��SetProp, ���ǲ����ô����ڴ����.
    // ViewProp��SetProp������һ��, ��ͼ/�����������һ�δ�����ViewProp.
    class ViewProp
    {
    public:
        // ������ͼ/����ֵ. ����Ѿ�����, ���滻.
        //
        // ViewProp���´��char*, ָ����������.
        ViewProp(HWND view, const char* key, void* data);
        ~ViewProp();

        // ������ͼ/����Ӧ������, �����ڷ���NULL.
        static void* GetValue(HWND view, const char* key);

        // ���ؼ�.
        const char* Key() const;

    private:
        class Data;

        // �洢ʵ�ʵ�����.
        scoped_refptr<Data> data_;

        DISALLOW_COPY_AND_ASSIGN(ViewProp);
    };

} //namespace ui

#endif //__ui_base_view_prop_h__