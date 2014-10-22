
#ifndef __ui_gfx_image_h__
#define __ui_gfx_image_h__

#pragma once

#include <map>
#include <vector>

#include "base/memory/ref_counted.h"

class SkBitmap;

namespace gfx
{

    namespace internal
    {
        class ImageRep;
        class ImageStorage;
    }

    // ����ͼ�����͵ķ�װ��, �����Ǳ���ƽ̨��GdkBitmap/NSImage����SkBitmap.
    // ͨ�����������ؿ���ת��������ͼ������. �ڲ��Ỻ��ת������ͼ������ظ�ת��.
    //
    // ��ʼ��ͼ������Լ�ת����ͼ������������ں�Image����һ��.
    class Image
    {
    public:
        enum RepresentationType
        {
            kImageRepGdk,
            kImageRepCocoa,
            kImageRepSkia,
        };

        typedef std::map<RepresentationType, internal::ImageRep*> RepresentationMap;

        // ����ȱʡͼ�����͵�Image. ����ӹ�bitmap����Ȩ.
        explicit Image(const SkBitmap* bitmap);

        // To create an Image that supports multiple resolutions pass a vector
        // of bitmaps, one for each resolution.
        explicit Image(const std::vector<const SkBitmap*>& bitmaps);

        // Initializes a new Image by AddRef()ing |other|'s internal storage.
        Image(const Image& other);

        // Copies a reference to |other|'s storage.
        Image& operator=(const Image& other);

        // ɾ��ͼ���Լ��ڲ������ת��ͼ��.
        ~Image();

        // Converts the Image to the desired representation and stores it internally.
        // The returned result is a weak pointer owned by and scoped to the life of
        // the Image.
        const SkBitmap* ToSkBitmap() const;

        // Performs a conversion, like above, but returns a copy of the result rather
        // than a weak pointer. The caller is responsible for deleting the result.
        // Note that the result is only a copy in terms of memory management; the
        // backing pixels are shared amongst all copies (a fact of each of the
        // converted representations, rather than a limitation imposed by Image) and
        // so the result should be considered immutable.
        const SkBitmap* CopySkBitmap() const;

        // ͼ������ת������.
        operator const SkBitmap*() const;
        operator const SkBitmap&() const;

        // Gets the number of bitmaps in this image. This may cause a conversion
        // to a bitmap representation. Note, this function and GetSkBitmapAtIndex()
        // are primarily meant to be used by the theme provider.
        size_t GetNumberOfSkBitmaps() const;

        // Gets the bitmap at the given index. This may cause a conversion
        // to a bitmap representation. Note, the internal ordering of bitmaps is not
        // guaranteed.
        const SkBitmap* GetSkBitmapAtIndex(size_t index) const;

        // �鿴�Ƿ���ָ�����͵�ͼ��.
        bool HasRepresentation(RepresentationType type) const;

        // Returns the number of representations.
        size_t RepresentationCount() const;

        // ��|other|�����ڲ��洢��ͼ��.
        void SwapRepresentations(gfx::Image* other);

    private:
        // ����ȱʡ��ImageRep.
        internal::ImageRep* DefaultRepresentation() const;

        // ����ָ�����͵�ImageRep, ��Ҫʱת��������.
        internal::ImageRep* GetRepresentation(RepresentationType rep) const;

        // �洢��map.
        void AddRepresentation(internal::ImageRep* rep) const;

        // Internal class that holds all the representations. This allows the Image to
        // be cheaply copied.
        scoped_refptr<internal::ImageStorage> storage_;
    };

} //namespace gfx

#endif //__ui_gfx_image_h__