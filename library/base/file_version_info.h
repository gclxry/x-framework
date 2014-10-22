
#ifndef __base_file_version_info_h__
#define __base_file_version_info_h__

#include <windows.h>

#include <string>

#include "basic_types.h"
#include "memory/scoped_ptr.h"

// http://blogs.msdn.com/oldnewthing/archive/2004/10/25/247180.aspx
extern "C" IMAGE_DOS_HEADER __ImageBase;

class FilePath;

// ����Ļ�ȡ�ļ��İ汾��Ϣ.
class FileVersionInfo
{
public:
    ~FileVersionInfo();

    // ����ָ��·���ļ���FileVersionInfo����. ����(ͨ���ǲ����ڻ����޷���)����
    // NULL. ���صĶ���ʹ����֮����Ҫɾ��.
    static FileVersionInfo* CreateFileVersionInfo(const FilePath& file_path);

    // Creates a FileVersionInfo for the specified module. Returns NULL in case
    // of error. The returned object should be deleted when you are done with it.
    static FileVersionInfo* CreateFileVersionInfoForModule(HMODULE module);

    // ������ǰģ���FileVersionInfo����. ���󷵻�NULL. ���صĶ���ʹ����֮����Ҫ
    // ɾ��.
    // ����ǿ�������Ա���ȷ���㵱ǰģ��ֵ.
    __forceinline static FileVersionInfo*
        CreateFileVersionInfoForCurrentModule()
    {
        HMODULE module = reinterpret_cast<HMODULE>(&__ImageBase);
        return CreateFileVersionInfoForModule(module);
    }

    // ��ȡ�汾��Ϣ�ĸ�������, �����ڷ��ؿ�.
    virtual std::wstring company_name();
    virtual std::wstring company_short_name();
    virtual std::wstring product_name();
    virtual std::wstring product_short_name();
    virtual std::wstring internal_name();
    virtual std::wstring product_version();
    virtual std::wstring private_build();
    virtual std::wstring special_build();
    virtual std::wstring comments();
    virtual std::wstring original_filename();
    virtual std::wstring file_description();
    virtual std::wstring file_version();
    virtual std::wstring legal_copyright();
    virtual std::wstring legal_trademarks();
    virtual std::wstring last_change();
    virtual bool is_official_build();

    // ��ȡ��������, �����������.
    bool GetValue(const wchar_t* name, std::wstring& value);

    // ��GetValueһ��, ֻ�ǲ����ڵ�ʱ�򷵻ؿ��ַ���.
    std::wstring GetStringValue(const wchar_t* name);

    // ��ȡ�̶��İ汾��Ϣ, ��������ڷ���NULL.
    VS_FIXEDFILEINFO* fixed_file_info() { return fixed_file_info_; }

private:
    FileVersionInfo(void* data, int language, int code_page);

    scoped_ptr_malloc<char> data_;
    int language_;
    int code_page_;
    // ָ��data_�ڲ���ָ��, NULL��ʾ������.
    VS_FIXEDFILEINFO* fixed_file_info_;

    DISALLOW_COPY_AND_ASSIGN(FileVersionInfo);
};

#endif //__base_file_version_info_h__