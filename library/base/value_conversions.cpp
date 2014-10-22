
#include "value_conversions.h"

#include "file_path.h"
#include "utf_string_conversions.h"
#include "value.h"

namespace base
{

    namespace
    {

        // |Value|�ڲ��洢���ַ�����UTF-8��ʽ, ������Ҫ���б���ת��.

        std::string FilePathToUTF8(const FilePath& file_path)
        {
            return UTF16ToUTF8(file_path.value());
        }

        FilePath UTF8ToFilePath(const std::string& str)
        {
            std::wstring result;
            result = UTF8ToUTF16(str);
            return FilePath(result);
        }

    }

    StringValue* CreateFilePathValue(const FilePath& in_value)
    {
        return new StringValue(FilePathToUTF8(in_value));
    }

    bool GetValueAsFilePath(const Value& value, FilePath* file_path)
    {
        std::string str;
        if(!value.GetAsString(&str))
        {
            return false;
        }
        if(file_path)
        {
            *file_path = UTF8ToFilePath(str);
        }
        return true;
    }

} //namespace base