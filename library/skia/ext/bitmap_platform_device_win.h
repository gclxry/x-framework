
#ifndef __skia_bitmap_platform_device_win_h__
#define __skia_bitmap_platform_device_win_h__

#pragma once

#include "platform_device_win.h"

namespace skia
{

    // BitmapPlatformDevice��SkBitmap�Ļ�����װ, ΪSkCanvas�ṩ��ͼ����. �豸Ϊ
    // Windows�ṩ��һ����д�ı���. BitmapPlatformDeviceʹ��CreateDIBSection()��
    // ��һ��Skia֧�ֵ�λͼ, �����Ϳ��Ի���ClearType����. �����������豸��λͼ��,
    // �������Ա�����.
    //
    // �豸ӵ����������, ���豸����ʱ, ��������Ҳ�����Ч��. ��һ�㲻ͬ��SKIA,
    // ������������ʹ�������ü���. ��Skia��, �����������һ��λͼ����豸��λͼ
    // ��ֵ, ���������. ��������, �滻��λͼ�����豸�Ƿ���ʱ��Ҳ��ɲ��Ϸ���,
    // �ⳣ������һЩ���޵�����. ����, ��Ҫ��������λͼ������豸���������ݸ�ֵ,
    // ȷ��ʹ�õ��ǿ���.
    class BitmapPlatformDevice : public PlatformDevice, public SkDevice
    {
    public:
        // ���̺���. screen_dc���ڴ���λͼ, �����ں����д洢. �����������ȷ֪��λͼ
        // ��ȫ��͸���������������is_opaqueΪtrue, ������������һЩ�Ż�.
        //
        // shared_section�����ǿ�ѡ��(����NULLʹ��ȱʡ��Ϊ). ���shared_section�ǿ�,
        // ��������һ��CreateFileMapping���ص��ļ�ӳ�����. ϸ�ڲμ�CreateDIBSection.
        static BitmapPlatformDevice* create(HDC screen_dc, int width, int height,
            bool is_opaque, HANDLE shared_section);

        // ������һ��, ֻ�Ǻ��������ȡscreen_dc.
        static BitmapPlatformDevice* create(int width, int height, bool is_opaque,
            HANDLE shared_section);

        virtual ~BitmapPlatformDevice();

        // Retrieves the bitmap DC, which is the memory DC for our bitmap data. The
        // bitmap DC is lazy created.
        virtual HDC BeginPlatformPaint();
        virtual void EndPlatformPaint();

        virtual void DrawToNativeContext(HDC dc, int x, int y, const RECT* src_rect);

        // ���ظ����ı仯�Ͳü�����HDC. ����SkDevice��.
        virtual void setMatrixClip(const SkMatrix& transform,
            const SkRegion& region, const SkClipStack&);

    protected:
        // ˢ��Windows DC, �Ա�Skia�ܹ�ֱ�ӷ�����������. ����SkDevice��, ��Skia��ʼ
        // ������������ʱ����.
        virtual void onAccessBitmap(SkBitmap* bitmap);

        virtual SkDevice* onCreateCompatibleDevice(SkBitmap::Config, int width,
            int height, bool isOpaque);

    private:
        // ���ü�������, �ܱ�����豸�乲��, ��֤��������͸�ֵ������������. �����豸
        // ʹ�õ�λͼ�Ѿ������ü�����, ֧�ֿ���.
        class BitmapPlatformDeviceData;

        // ˽�й��캯��. ����Ӧ�������ù���.
        BitmapPlatformDevice(BitmapPlatformDeviceData* data,
            const SkBitmap& bitmap);

        // �豸����������, ��֤�ǿ�. ���Ǵ洢���������.
        BitmapPlatformDeviceData* data_;
    };

} //namespace skia

#endif //__skia_bitmap_platform_device_win_h__