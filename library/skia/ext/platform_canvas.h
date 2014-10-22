
#ifndef __skia_platform_canvas_h__
#define __skia_platform_canvas_h__

#pragma once

#include "SkCanvas.h"

#include "platform_device_win.h"

namespace skia
{

    // PlatformCanvas��һ�����⻯�Ĺ���SkCanvas, �������PlatformDevice������ƽ̨
    // ��صĻ�ͼ. ��ͬʱ����Skia������ƽ̨��ز���.
    class PlatformCanvas : public SkCanvas
    {
    public:
        // ���ʹ���޲����汾�Ĺ��캯��, �����ֶ�����initialize().
        PlatformCanvas();
        // �����Ҫ����λͼ, �Ҳ���Ҫ͸����, ��������is_opaque: ��ô������һЩ
        // �Ż�.
        PlatformCanvas(int width, int height, bool is_opaque);
        // ����shared_section��BitmapPlatformDevice::create.
        PlatformCanvas(int width, int height, bool is_opaque, HANDLE shared_section);
        
        virtual ~PlatformCanvas();

        // �����ʹ�õ��������޲����Ĺ��캯��, ���ǵڶ�����ʼ������.
        bool initialize(int width, int height, bool is_opaque,
            HANDLE shared_section=NULL);

        // ���ظ�����ȵ�strideֵ(һ�����õ��ֽ���). ��Ϊÿ������ʹ��32-bits, ����
        // �����4*width. Ȼ��, ���ڶ����ԭ����ܻ�������.
        static size_t StrideForWidth(unsigned width);

    private:
        // initialize()�������õĸ�������.
        bool initializeWithDevice(SkDevice* device);

        // δʵ��. ������ֹ���˵���SkCanvas���������. SkCanvas�İ汾�����麯��, ��
        // �����ǲ�����100%����ֹ, ��ϣ�������������ǵ�ע��, ����ʹ���������. ����
        // SkCanvas�İ汾�ᴴ��һ���µĲ������豸, ���������ʹ��CoreGraphics������
        // ��ͼ, �������.
        virtual SkDevice* setBitmapDevice(const SkBitmap& bitmap);

        // ��ֹ�����͸�ֵ���캯��.
        PlatformCanvas(const PlatformCanvas&);
        PlatformCanvas& operator=(const PlatformCanvas&);
    };

    // Returns the SkDevice pointer of the topmost rect with a non-empty
    // clip. In practice, this is usually either the top layer or nothing, since
    // we usually set the clip to new layers when we make them.
    //
    // If there is no layer that is not all clipped out, this will return a
    // dummy device so callers do not have to check. If you are concerned about
    // performance, check the clip before doing any painting.
    //
    // This is different than SkCanvas' getDevice, because that returns the
    // bottommost device.
    //
    // Danger: the resulting device should not be saved. It will be invalidated
    // by the next call to save() or restore().
    SkDevice* GetTopDevice(const SkCanvas& canvas);

    // Creates a canvas with raster bitmap backing.
    // Set is_opaque if you are going to erase the bitmap and not use
    // transparency: this will enable some optimizations.
    SkCanvas* CreateBitmapCanvas(int width, int height, bool is_opaque);

    // Non-crashing version of CreateBitmapCanvas
    // returns NULL if allocation fails for any reason.
    // Use this instead of CreateBitmapCanvas in places that are likely to
    // attempt to allocate very large canvases (therefore likely to fail),
    // and where it is possible to recover gracefully from the failed allocation.
    SkCanvas* TryCreateBitmapCanvas(int width, int height, bool is_opaque);

    // Returns true if native platform routines can be used to draw on the
    // given canvas. If this function returns false, BeginPlatformPaint will
    // return NULL PlatformSurface.
    bool SupportsPlatformPaint(const SkCanvas* canvas);

    // Draws into the a native platform surface, |context|.  Forwards to
    // DrawToNativeContext on a PlatformDevice instance bound to the top device.
    // If no PlatformDevice instance is bound, is a no-operation.
    void DrawToNativeContext(SkCanvas* canvas, HDC context,
        int x, int y, const RECT* src_rect);

    // Sets the opacity of each pixel in the specified region to be opaque.
    void MakeOpaque(SkCanvas* canvas, int x, int y, int width, int height);

    // These calls should surround calls to platform drawing routines, the
    // surface returned here can be used with the native platform routines.
    //
    // Call EndPlatformPaint when you are done and want to use skia operations
    // after calling the platform-specific BeginPlatformPaint; this will
    // synchronize the bitmap to OS if necessary.
    HDC BeginPlatformPaint(SkCanvas* canvas);
    void EndPlatformPaint(SkCanvas* canvas);

    // Helper class for pairing calls to BeginPlatformPaint and EndPlatformPaint.
    // Upon construction invokes BeginPlatformPaint, and upon destruction invokes
    // EndPlatformPaint.
    class ScopedPlatformPaint
    {
    public:
        explicit ScopedPlatformPaint(SkCanvas* canvas) : canvas_(canvas)
        {
            platform_surface_ = BeginPlatformPaint(canvas);
        }
        ~ScopedPlatformPaint() { EndPlatformPaint(canvas_); }

        // Returns the PlatformSurface to use for native platform drawing calls.
        HDC GetPlatformSurface() { return platform_surface_; }

    private:
        SkCanvas* canvas_;
        HDC platform_surface_;

        // Disallow copy and assign
        ScopedPlatformPaint(const ScopedPlatformPaint&);
        ScopedPlatformPaint& operator=(const ScopedPlatformPaint&);
    };

} //namespace skia

#endif //__skia_platform_canvas_h__