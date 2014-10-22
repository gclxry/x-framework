
#ifndef __skia_vector_canvas_h__
#define __skia_vector_canvas_h__

#pragma once

#include "platform_canvas.h"

namespace skia
{

    // VectorCanvas��һ�������PlatformCanvas, �������VectorDevice������ƽ̨
    // ��صĻ�ͼ. ��ͬʱ����Skia������ƽ̨��ز���. ��֧��λͼ˫������Ϊû��
    // ʹ��λͼ.
    class VectorCanvas : public PlatformCanvas
    {
    public:
        explicit VectorCanvas(SkDevice* device);
        virtual ~VectorCanvas();

        virtual SkBounder* setBounder(SkBounder* bounder);
        virtual SkDrawFilter* setDrawFilter(SkDrawFilter* filter);

    private:
        // ��������豸�ǻ�������������λͼ��, ����true.
        bool IsTopDeviceVectorial() const;

        // ��֧�ֿ����͸�ֵ���캯��.
        VectorCanvas(const VectorCanvas&);
        const VectorCanvas& operator=(const VectorCanvas&);
    };

} //namespace skia

#endif //__skia_vector_canvas_h__