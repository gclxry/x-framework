
#ifndef __skia_skia_utils_win_h__
#define __skia_skia_utils_win_h__

#pragma once

#include "SkColor.h"

struct SkIRect;
struct SkPoint;
struct SkRect;
typedef unsigned long DWORD;
typedef DWORD COLORREF;
typedef struct tagPOINT POINT;
typedef struct tagRECT RECT;

namespace skia
{

    // Skia�ĵ�ת����Windows��POINT.
    POINT SkPointToPOINT(const SkPoint& point);

    // Windows��RECTת����Skia�ľ���.
    SkRect RECTToSkRect(const RECT& rect);

    // Windows��RECTת����Skia�ľ���.
    // ����ʹ����ͬ���ڴ��ʽ. ��skia_utils.cpp��ͨ��COMPILE_ASSERT()
    // ��֤.
    inline const SkIRect& RECTToSkIRect(const RECT& rect)
    {
        return reinterpret_cast<const SkIRect&>(rect);
    }

    // Skia�ľ���ת����Windows��RECT.
    // ����ʹ����ͬ���ڴ��ʽ. ��skia_utils.cpp��ͨ��COMPILE_ASSERT()
    // ��֤.
    inline const RECT& SkIRectToRECT(const SkIRect& rect)
    {
        return reinterpret_cast<const RECT&>(rect);
    }

    // ת��COLORREFs(0BGR)��Skia֧�ֵ�ARGB���з�ʽ.
    SkColor COLORREFToSkColor(COLORREF color);

    // ת��ARGB��COLORREFs(0BGR).
    COLORREF SkColorToCOLORREF(SkColor color);

} // namespace skia

#endif //__skia_skia_utils_win_h__