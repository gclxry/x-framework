
#include "bitmap_platform_device_win.h"

#include <windows.h>

#include "SkMatrix.h"
#include "SkRefCnt.h"
#include "SkRegion.h"
#include "SkUtils.h"

#include "bitmap_platform_device_data.h"

namespace skia
{

    BitmapPlatformDevice::BitmapPlatformDeviceData::BitmapPlatformDeviceData(
        HBITMAP hbitmap) : bitmap_context_(hbitmap), hdc_(NULL),
        config_dirty_(true) // ��Ҫ����һ�μ�������.
    {
        // ��ʼ���ü���Ϊ����λͼ.
        BITMAP bitmap_data;
        if(GetObject(bitmap_context_, sizeof(BITMAP), &bitmap_data))
        {
            SkIRect rect;
            rect.set(0, 0, bitmap_data.bmWidth, bitmap_data.bmHeight);
            clip_region_ = SkRegion(rect);
        }

        transform_.reset();
    }

    BitmapPlatformDevice::BitmapPlatformDeviceData::~BitmapPlatformDeviceData()
    {
        if(hdc_)
        {
            ReleaseBitmapDC();
        }

        // ͬʱ����λͼ���ݺ�λͼ���.
        DeleteObject(bitmap_context_);
    }

    HDC BitmapPlatformDevice::BitmapPlatformDeviceData::GetBitmapDC()
    {
        if(!hdc_)
        {
            hdc_ = CreateCompatibleDC(NULL);
            InitializeDC(hdc_);
            HGDIOBJ old_bitmap = SelectObject(hdc_, bitmap_context_);
            // �ڴ�DC�մ���ʱ, ��ʾ����Ϊ1*1���صĺڰ�λͼ. ��������ѡ���Լ���λͼ,
            // ��ɾ��ǰ������ѡ��.
            DeleteObject(old_bitmap);
        }

        LoadConfig();
        return hdc_;
    }

    void BitmapPlatformDevice::BitmapPlatformDeviceData::ReleaseBitmapDC()
    {
        SkASSERT(hdc_);
        DeleteDC(hdc_);
        hdc_ = NULL;
    }

    bool BitmapPlatformDevice::BitmapPlatformDeviceData::IsBitmapDCCreated() const
    {
        return hdc_ != NULL;
    }

    void BitmapPlatformDevice::BitmapPlatformDeviceData::SetMatrixClip(
        const SkMatrix& transform, const SkRegion& region)
    {
        transform_ = transform;
        clip_region_ = region;
        config_dirty_ = true;
    }

    void BitmapPlatformDevice::BitmapPlatformDeviceData::LoadConfig()
    {
        if(!config_dirty_ || !hdc_)
        {
            return; // ʲô������.
        }
        config_dirty_ = false;

        // �任.
        LoadTransformToDC(hdc_, transform_);
        LoadClippingRegionToDC(hdc_, clip_region_, transform_);
    }

    // ʹ�þ�̬�������������ͨ�Ĺ��캯��, ���������ڵ��ù���ǰ������������.
    // ֮����Ҫ��ô������Ϊ���û��๹�캯��ʱ��Ҫ��������.
    BitmapPlatformDevice* BitmapPlatformDevice::create(HDC screen_dc,
        int width, int height, bool is_opaque, HANDLE shared_section)
    {
        SkBitmap bitmap;

        // CreateDIBSection��֧�ִ�����λͼ, ����ֻ�ܴ���һ����С��.
        if((width==0) || (height==0))
        {
            width = 1;
            height = 1;
        }

        BITMAPINFOHEADER hdr = { 0 };
        hdr.biSize = sizeof(BITMAPINFOHEADER);
        hdr.biWidth = width;
        hdr.biHeight = -height; // ���ű�ʾ���϶��µ�λͼ.
        hdr.biPlanes = 1;
        hdr.biBitCount = 32;
        hdr.biCompression = BI_RGB; // ��ѹ��.
        hdr.biSizeImage = 0;
        hdr.biXPelsPerMeter = 1;
        hdr.biYPelsPerMeter = 1;
        hdr.biClrUsed = 0;
        hdr.biClrImportant = 0;

        void* data = NULL;
        HBITMAP hbitmap = CreateDIBSection(screen_dc,
            reinterpret_cast<BITMAPINFO*>(&hdr), 0,
            &data, shared_section, 0);
        if(!hbitmap)
        {
            return NULL;
        }

        bitmap.setConfig(SkBitmap::kARGB_8888_Config, width, height);
        bitmap.setPixels(data);
        bitmap.setIsOpaque(is_opaque);

        // ��������Ǵ����, ��Ҫ������!
        if(!shared_section)
        {
            if(is_opaque)
            {
#ifndef NDEBUG
                // Ϊ�˸�������bugs, ���ñ���ɫΪĳ�ֺ����ױ����ֵ���ɫ.
                bitmap.eraseARGB(255, 0, 255, 128); // ������.
#endif
            }
            else
            {
                bitmap.eraseARGB(0, 0, 0, 0);
            }
        }

        // �豸����ӹ�HBITMAP������Ȩ. ���ݶ���ĳ�ʼ���ü���Ϊ1, ���Ϲ���
        // ����Ҫ��.
        return new BitmapPlatformDevice(new BitmapPlatformDeviceData(hbitmap),
            bitmap);
    }

    // static
    BitmapPlatformDevice* BitmapPlatformDevice::create(int width,
        int height, bool is_opaque, HANDLE shared_section)
    {
        HDC screen_dc = GetDC(NULL);
        BitmapPlatformDevice* device = BitmapPlatformDevice::create(
            screen_dc, width, height, is_opaque, shared_section);
        ReleaseDC(NULL, screen_dc);
        return device;
    }

    // �豸ӵ��HBITMAP, ͬʱҲӵ���������������, ����ת������Ȩ��SkDevice��λͼ.
    BitmapPlatformDevice::BitmapPlatformDevice(BitmapPlatformDeviceData* data,
        const SkBitmap& bitmap) : SkDevice(bitmap), data_(data)
    {
        // ���ݶ����Ѿ���create()�����ù�.
        SetPlatformDevice(this, this);
    }

    BitmapPlatformDevice::~BitmapPlatformDevice()
    {
        data_->unref();
    }

    HDC BitmapPlatformDevice::BeginPlatformPaint()
    {
        return data_->GetBitmapDC();
    }

    void BitmapPlatformDevice::EndPlatformPaint()
    {
        PlatformDevice::EndPlatformPaint();
    }

    void BitmapPlatformDevice::DrawToNativeContext(HDC dc, int x, int y,
        const RECT* src_rect)
    {
        bool created_dc = !data_->IsBitmapDCCreated();
        HDC source_dc = BeginPlatformPaint();

        RECT temp_rect;
        if(!src_rect)
        {
            temp_rect.left = 0;
            temp_rect.right = width();
            temp_rect.top = 0;
            temp_rect.bottom = height();
            src_rect = &temp_rect;
        }

        int copy_width = src_rect->right - src_rect->left;
        int copy_height = src_rect->bottom - src_rect->top;

        // ������Ҫ��λͼ���ñ任, ����(0, 0)�����������Ͻ�.
        SkMatrix identity;
        identity.reset();

        LoadTransformToDC(source_dc, identity);
        if(isOpaque())
        {
            BitBlt(dc,
                x,
                y,
                copy_width,
                copy_height,
                source_dc,
                src_rect->left,
                src_rect->top,
                SRCCOPY);
        }
        else
        {
            SkASSERT(copy_width!=0 && copy_height!=0);
            BLENDFUNCTION blend_function = { AC_SRC_OVER, 0, 255, AC_SRC_ALPHA };
            GdiAlphaBlend(dc,
                x,
                y,
                copy_width,
                copy_height,
                source_dc,
                src_rect->left,
                src_rect->top,
                copy_width,
                copy_height,
                blend_function);
        }
        LoadTransformToDC(source_dc, data_->transform());

        EndPlatformPaint();
        if(created_dc)
        {
            data_->ReleaseBitmapDC();
        }
    }

    void BitmapPlatformDevice::setMatrixClip(const SkMatrix& transform,
        const SkRegion& region, const SkClipStack&)
    {
        data_->SetMatrixClip(transform, region);
    }

    void BitmapPlatformDevice::onAccessBitmap(SkBitmap* bitmap)
    {
        // �Ż�: ����ֻӦ����DC������GDI����ʱ�ŵ���flush.
        if(data_->IsBitmapDCCreated())
        {
            GdiFlush();
        }
    }

    SkDevice* BitmapPlatformDevice::onCreateCompatibleDevice(
        SkBitmap::Config config, int width, int height, bool isOpaque)
    {
        SkASSERT(config == SkBitmap::kARGB_8888_Config);
        return BitmapPlatformDevice::create(width, height, isOpaque, NULL);
    }

} //namespace skia