
#ifndef __skia_bitmap_platform_device_data_h__
#define __skia_bitmap_platform_device_data_h__

#include "bitmap_platform_device_win.h"

namespace skia
{

    class BitmapPlatformDevice::BitmapPlatformDeviceData : public SkRefCnt
    {
    public:
        explicit BitmapPlatformDeviceData(HBITMAP bitmap);

        // ����/����hdc_, λͼʹ�õ��ڴ�DC.
        HDC GetBitmapDC();
        void ReleaseBitmapDC();
        bool IsBitmapDCCreated() const;

        // ���ñ任�Ͳü�����. ����ֱ�Ӹ����豸����, ������������. ���иı佫����
        // �´ε���LoadConfigʱ��Ч.
        void SetMatrixClip(const SkMatrix& transform, const SkRegion& region);

        // ���ص�ǰ�任�Ͳü����豸����. ��ʹ|bitmap_context_|Ϊ��Ҳ���Ե���(�޲���).
        void LoadConfig();

        const SkMatrix& transform() const
        {
            return transform_;
        }

        HBITMAP bitmap_context()
        {
            return bitmap_context_;
        }

    private:
        virtual ~BitmapPlatformDeviceData();

        // ��������ͼ���豸, ������λͼ�л���.
        HBITMAP bitmap_context_;

        // ��������DC, ������λͼ�л���. �μ�GetBitmapDC().
        HDC hdc_;

        // ���б任��ü�û�����õ��豸����ʱΪtrue. �豸������ÿ���ı�����ʱ����,
        // �任��ü�����ÿ�ζ����. �������Խ�ʡ����Ҫ�ļ��ر任�Ͳü���ʱ��.
        bool config_dirty_;

        // �豸�����ı任����: ��Ҫ����ά���������, ��Ϊ�豸������û������ʱ��
        // �����Ѿ���Ҫ�������ֵ.
        SkMatrix transform_;

        // ��ǰ�ü���.
        SkRegion clip_region_;

        // ��ֹ�����͸�ֵ���캯��.
        BitmapPlatformDeviceData(const BitmapPlatformDeviceData&);
        BitmapPlatformDeviceData& operator=(const BitmapPlatformDeviceData&);
    };

} //namespace skia

#endif //__skia_bitmap_platform_device_data_h__