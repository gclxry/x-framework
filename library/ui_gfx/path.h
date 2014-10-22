
#ifndef __ui_gfx_path_h__
#define __ui_gfx_path_h__

#pragma once

#include "base/basic_types.h"

#include "SkPath.h"

#include "point.h"

namespace gfx
{

    class Path : public SkPath
    {
    public:
        Path();
        Path(const Point* points, size_t count);
        ~Path();

        // ��SkPath����һ��HRGN. �����߸����ͷŷ��ص���Դ. ֻ֧�ֶ����·��.
        HRGN CreateNativeRegion() const;

        // �����������Ľ���. ������ӵ�з��ض��������Ȩ.
        static HRGN IntersectRegions(HRGN r1, HRGN r2);

        // �����������Ĳ���. ������ӵ�з��ض��������Ȩ.
        static HRGN CombineRegions(HRGN r1, HRGN r2);

        // �����������Ĳ. ������ӵ�з��ض��������Ȩ.
        static HRGN SubtractRegion(HRGN r1, HRGN r2);

    private:
        DISALLOW_COPY_AND_ASSIGN(Path);
    };

} //namespace gfx

#endif //__ui_gfx_path_h__