
#ifndef __view_default_theme_provider_h__
#define __view_default_theme_provider_h__

#pragma once

#include <vector>

#include "ui_base/theme_provider.h"

namespace ui
{
    class ResourceBundle;
}

namespace view
{

    class DefaultThemeProvider : public ui::ThemeProvider
    {
    public:
        DefaultThemeProvider();
        virtual ~DefaultThemeProvider();

        // Overridden from ThemeProvider.
        virtual void Init(Profile* profile);
        virtual SkBitmap* GetBitmapNamed(int id) const;
        virtual SkColor GetColor(int id) const;
        virtual bool GetDisplayProperty(int id, int* result) const;
        virtual bool ShouldUseNativeFrame() const;
        virtual bool HasCustomImage(int id) const;
        virtual RefCountedMemory* GetRawData(int id) const;

    private:
        DISALLOW_COPY_AND_ASSIGN(DefaultThemeProvider);
    };

} //namespace view

#endif //__view_default_theme_provider_h__