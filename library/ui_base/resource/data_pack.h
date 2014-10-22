
// DataPack��ʾ��(key, value)Ϊ���ݵĴ����ļ�ֻ����ͼ. ���ڴ洢��̬��Դ,
// �����ַ�����ͼ��.

#ifndef __ui_base_data_pack_h__
#define __ui_base_data_pack_h__

#pragma once

#include <map>

#include "base/basic_types.h"
#include "base/memory/scoped_ptr.h"

class FilePath;
class RefCountedStaticMemory;

namespace base
{
    class MemoryMappedFile;
    class StringPiece;
}

namespace ui
{

    class DataPack
    {
    public:
        DataPack();
        ~DataPack();

        // ��|path|���ش���ļ�, ��������ʱ����false.
        bool Load(const FilePath& path);

        // ͨ��|resource_id|��ȡ��Դ, ������ݵ�|data|. ���ݹ�DataPack��������,
        // ��Ҫ�޸�. ���û�ҵ���Դid, ����false.
        bool GetStringPiece(uint16 resource_id, base::StringPiece* data) const;

        // ����GetStringPiece(), ���Ƿ����ڴ��ָ��. ���ӿ�����ͼ������,
        // StringPiece�ӿ�һ�����ڱ����ַ���.
        RefCountedStaticMemory* GetStaticMemory(uint16 resource_id) const;

        // ��|resources|д�뵽·��Ϊ|path|�Ĵ���ļ�.
        static bool WritePack(const FilePath& path,
            const std::map<uint16, base::StringPiece>& resources);

    private:
        // �ڴ�ӳ������.
        scoped_ptr<base::MemoryMappedFile> mmap_;

        // �����е���Դ����.
        size_t resource_count_;

        DISALLOW_COPY_AND_ASSIGN(DataPack);
    };

} //namespace ui

#endif //__ui_base_data_pack_h__