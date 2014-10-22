
#ifndef __ui_gfx_canvas_h__
#define __ui_gfx_canvas_h__

#pragma once

#include <windows.h>

#include "base/basic_types.h"
#include "base/string16.h"

// TODO(beng): remove these includes when we no longer depend on SkTypes.
#include "SkColor.h"
#include "SkXfermode.h"

namespace gfx
{

    class Brush;
    class CanvasSkia;
    class Font;
    class Point;
    class Rect;
    class Transform;

    class Canvas
    {
    public:
        // ָ��DrawStringInt������Ⱦ�ı��Ķ��뷽ʽ.
        enum
        {
            TEXT_ALIGN_LEFT = 1,
            TEXT_ALIGN_CENTER = 2,
            TEXT_ALIGN_RIGHT = 4,
            TEXT_VALIGN_TOP = 8,
            TEXT_VALIGN_MIDDLE = 16,
            TEXT_VALIGN_BOTTOM = 32,

            // ָ�������ı�.
            MULTI_LINE = 64,

            // ȱʡ�����DrawStringInt���ر���ǰ׺('&')�ַ�. Ҳ�����ַ���"&foo"��
            // ��Ⱦ��"&foo". ����Ⱦ���м���ǰ׺�ַ�����Դ�ı�ʱ, ǰ׺���Դ�����»�
            // ��(SHOW_PREFIX)������ȫ����Ⱦ(HIDE_PREFIX).
            SHOW_PREFIX = 128,
            HIDE_PREFIX = 256,

            // ��ֹʡ�Ժ�.
            NO_ELLIPSIS = 512,

            // ָ�����ʿ��Ա������и�. ֻ����MULTI_LINEһ����.
            CHARACTER_BREAK = 1024,

            // ָʾDrawStringInt()ʹ��RTL��������Ⱦ�ı�. ����������û��Ҫ�������
            // ��־λ, ��Ϊ�ı��ķ�������Ϣ��������Unicode�ַ�����ʽǶ�����ַ�����.
            // �������������LTR, ���ǲ�����뷽�����ַ�, ��Ϊ��Щƽ̨(����û�а�װ
            // RTL�����Ӣ�İ�Windows XP)��֧����Щ�ַ�. ����, ��������LTRʱ, ���
            // ��־��������ȾRTL�����Ե��ı�.
            FORCE_RTL_DIRECTIONALITY = 2048,

            // Similar to FORCE_RTL_DIRECTIONALITY, but left-to-right.
            // See FORCE_RTL_DIRECTIONALITY for details.
            FORCE_LTR_DIRECTIONALITY = 4096,
        };

        virtual ~Canvas() {}

        // ����һ���յĻ���. ʹ��ǰ�����ʼ��.
        static Canvas* CreateCanvas();

        // �����ض���С�Ļ���.
        static Canvas* CreateCanvas(int width, int height, bool is_opaque);

        // ��ջ�ϱ���һ�ݻ�ͼ״̬�Ŀ���, ����Ե���Restore()֮ǰ, ���еĲ���
        // ������ݿ�����.
        virtual void Save() = 0;

        // ��Save()һ��, ֻ�ǻ�����һ��ͼ����, �ڵ���Restore()ʱ��ָ����alphaֵ
        // �뻭���ں�.
        // |layer_bounds|��ͼ���ڵ�ǰ����任�µķ�Χ.
        virtual void SaveLayerAlpha(uint8 alpha) = 0;
        virtual void SaveLayerAlpha(uint8 alpha, const Rect& layer_bounds) = 0;

        // �����ڵ���Save*()��ָ���ͼ״̬. Restore()����Save*()ʱ�ᷢ������.
        virtual void Restore() = 0;

        // ������װ������������.
        // ����ü�Ϊ�շ���true. ��ϸ�μ�clipRect.
        virtual bool ClipRectInt(int x, int y, int w, int h) = 0;

        // ������װ������������.
        // ��ϸ�μ�translate().
        virtual void TranslateInt(int x, int y) = 0;

        // ������װ������������.
        // ��ϸ�μ�scale().
        virtual void ScaleInt(int x, int y) = 0;

        // ���ض���ɫ��SkXfermode::kSrcOver_Mode��ʽ���ָ������.
        virtual void FillRectInt(const SkColor& color,
            int x, int y, int w, int h) = 0;

        // ���ض���ɫ��mode��ʽ���ָ������.
        virtual void FillRectInt(const SkColor& color,
            int x, int y, int w, int h, SkXfermode::Mode mode) = 0;

        // ���ض���ˢ���ָ������.
        virtual void FillRectInt(const Brush* brush,
            int x, int y, int w, int h) = 0;

        // ���ض������ø�����ɫ��SkXfermode::kSrcOver_Mode�仯ģʽ���ƾ��ο�.
        //
        // ע��: �����ֻ����Ҫ����һ����, ��ʹ��DrawLineInt.
        virtual void DrawRectInt(const SkColor& color,
            int x, int y, int w, int h) = 0;

        // ���ض������ø�����ɫ�ͱ仯ģʽ���ƾ��ο�.
        //
        // ע��: �����ֻ����Ҫ����һ����, ��ʹ��DrawLineInt.
        virtual void DrawRectInt(const SkColor& color,
            int x, int y, int w, int h,
            SkXfermode::Mode mode) = 0;

        // ���ض���ͼ�������ƾ���.
        virtual void DrawRectInt(int x, int y, int w, int h,
            const SkPaint& paint) = 0;

        // ���ض���ɫ������.
        virtual void DrawLineInt(const SkColor& color,
            int x1, int y1, int x2, int y2) = 0;

        // ���ض�λ�û���λͼ. λͼ�����Ͻ���Ⱦ���Ǹ�λ��.
        virtual void DrawBitmapInt(const SkBitmap& bitmap, int x, int y) = 0;

        // ���ض�λ���ø����Ļ�ͼ��������λͼ. λͼ�����Ͻ���Ⱦ���Ǹ�λ��.
        virtual void DrawBitmapInt(const SkBitmap& bitmap,
            int x, int y, const SkPaint& paint) = 0;

        // ���Ʋ���λͼ���ض�λ��. src������Ӧλͼ������, ���Ƶ�dest���궨���
        // ����.
        //
        // ���Դ�Ŀ��ߺ�Ŀ�ĵĲ�һ��, ͼ�񽫻�����. ��Сʱ, ǿ�ҽ������λͼ
        // ����buildMipMap(false)��ȷ������һ����С��ͼ(mipmap), ���������и���
        // �������. ����|filter|����ʹ��λͼ�Ĺ���, �����ز���ʹ�õ���������㷨.
        //
        // SkPaint�ṩ��ѡ����.
        virtual void DrawBitmapInt(const SkBitmap& bitmap,
            int src_x, int src_y, int src_w, int src_h,
            int dest_x, int dest_y, int dest_w, int dest_h,
            bool filter) = 0;
        virtual void DrawBitmapInt(const SkBitmap& bitmap,
            int src_x, int src_y, int src_w, int src_h,
            int dest_x, int dest_y, int dest_w, int dest_h,
            bool filter, const SkPaint& paint) = 0;

        // ���ض���ɫ�������ڸ�����λ�û����ı�. �ı�ˮƽ���������, ��ֱ����
        // ���ж���, �������вü�. ����ı�̫��, ���ȡ����ĩβ���'...'.
        virtual void DrawStringInt(const string16& text,
            const Font& font,
            const SkColor& color,
            int x, int y, int w, int h) = 0;
        virtual void DrawStringInt(const string16& text,
            const Font& font,
            const SkColor& color,
            const Rect& display_rect) = 0;

        // ���ض���ɫ�������ڸ�����λ�û����ı�. ���һ������ָ���ı���Ⱦ�ķ�ʽ.
        // ������TEXT_ALIGN_CENTER��TEXT_ALIGN_RIGHT��TEXT_ALIGN_LEFT�е�һ��.
        virtual void DrawStringInt(const string16& text,
            const Font& font,
            const SkColor& color,
            int x, int y, int w, int h,
            int flags) = 0;

        // ���ƴ��Ļ�ɫ����������ʾ����.
        virtual void DrawFocusRect(int x, int y, int width, int height) = 0;

        // ���ض�����ƽ��ͼ��.
        virtual void TileImageInt(const SkBitmap& bitmap,
            int x, int y, int w, int h) = 0;
        virtual void TileImageInt(const SkBitmap& bitmap,
            int src_x, int src_y,
            int dest_x, int dest_y, int w, int h) = 0;

        // ���ر��ػ�ͼ��������ƽ̨��صĻ�ͼ����. ������EndPlatformPaint()��Ե���.
        virtual HDC BeginPlatformPaint() = 0;

        // ��������ʹ�ñ��ػ�ͼ��������ƽ̨��صĻ�ͼ, �豸��BeginPlatformPaint()
        // ���ص�.
        virtual void EndPlatformPaint() = 0;

        // �Ի���Ӧ�ñ任.
        virtual void ConcatTransform(const Transform& transform) = 0;

        // ��ȡ�ײ�SkCanvas�ķ���.
        virtual CanvasSkia* AsCanvasSkia();
        virtual const CanvasSkia* AsCanvasSkia() const;
    };

    class CanvasPaint
    {
    public:
        virtual ~CanvasPaint() {}

        // ����һ������, ����ʱ���Ƶ�|view|. �����Ĵ�С����|view|�Ŀͻ���.
        static CanvasPaint* CreateCanvasPaint(HWND view);

        // ���������һ����Ҫ�ػ����Ч�����򷵻�true.
        virtual bool IsValid() const = 0;

        // ������Ч����.
        virtual Rect GetInvalidRect() const = 0;

        // ���صײ�Ļ���.
        virtual Canvas* AsCanvas() = 0;
    };

} //namespace gfx

#endif //__ui_gfx_canvas_h__