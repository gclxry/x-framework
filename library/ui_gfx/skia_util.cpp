
#include "skia_util.h"

#include "SkBitmap.h"
#include "SkColorPriv.h"
#include "SkShader.h"
#include "SkGradientShader.h"

#include "rect.h"

namespace gfx
{

    SkRect RectToSkRect(const gfx::Rect& rect)
    {
        SkRect r;
        r.set(SkIntToScalar(rect.x()), SkIntToScalar(rect.y()),
            SkIntToScalar(rect.right()), SkIntToScalar(rect.bottom()));
        return r;
    }

    gfx::Rect SkRectToRect(const SkRect& rect)
    {
        return gfx::Rect(static_cast<int>(rect.fLeft),
            static_cast<int>(rect.fTop),
            static_cast<int>(rect.width()),
            static_cast<int>(rect.height()));
    }

    SkShader* CreateGradientShader(int start_point, int end_point,
        SkColor start_color, SkColor end_color)
    {
        SkColor grad_colors[2] = { start_color, end_color };
        SkPoint grad_points[2];
        grad_points[0].set(SkIntToScalar(0), SkIntToScalar(start_point));
        grad_points[1].set(SkIntToScalar(0), SkIntToScalar(end_point));

        return SkGradientShader::CreateLinear(grad_points, grad_colors,
            NULL, 2, SkShader::kRepeat_TileMode);
    }

    bool BitmapsAreEqual(const SkBitmap& bitmap1, const SkBitmap& bitmap2)
    {
        void* addr1 = NULL;
        void* addr2 = NULL;
        size_t size1 = 0;
        size_t size2 = 0;

        bitmap1.lockPixels();
        addr1 = bitmap1.getAddr32(0, 0);
        size1 = bitmap1.getSize();
        bitmap1.unlockPixels();

        bitmap2.lockPixels();
        addr2 = bitmap2.getAddr32(0, 0);
        size2 = bitmap2.getSize();
        bitmap2.unlockPixels();

        return (size1==size2) && (0==memcmp(addr1, addr2, bitmap1.getSize()));
    }

    std::string RemoveAcceleratorChar(const std::string& s,
        char accelerator_char)
    {
        bool escaped = false;
        std::string accelerator_removed;
        accelerator_removed.reserve(s.size());
        for(size_t i=0; i<s.size(); ++i)
        {
            if(s[i]!=accelerator_char || escaped)
            {
                accelerator_removed.push_back(s[i]);
                escaped = false;
            }
            else
            {
                escaped = true;
            }
        }

        return accelerator_removed;
    }

} //namespace gfx