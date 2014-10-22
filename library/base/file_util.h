
#ifndef __base_file_util_h__
#define __base_file_util_h__

#pragma once

#include <stack>

#include "file_path.h"
#include "platform_file.h"

namespace base
{

    //-----------------------------------------------------------------------------
    // ǣ�����ļ�ϵͳ���ʺ��޸ĵĺ���:

    // Convert provided relative path into an absolute path.  Returns false on
    // error. On POSIX, this function fails if the path does not exist.
    bool AbsolutePath(FilePath* path);

    // ����pathĿ¼����ʱ��|comparison_time|���߸��������ļ�����.
    // ����".."����".", Ŀ¼���ݹ����.
    int CountFilesCreatedAfter(const FilePath& path,
        const Time& comparison_time);

    // ����|root_path|��ȫ���ļ������ֽ���. ���|root_path|�����ڷ���0.
    //
    // ������ʵ��ʹ����FileEnumerator��, �����κ�ƽ̨�¶�����ܿ�.
    int64 ComputeDirectorySize(const FilePath& root_path);

    // ����|directory|��(���ݹ�)ƥ��ģʽ|pattern|��ȫ���ļ������ֽ���.
    // ���|directory|�����ڷ���0.
    //
    // ������ʵ��ʹ����FileEnumerator��, �����κ�ƽ̨�¶�����ܿ�.
    int64 ComputeFilesSize(const FilePath& directory,
        const std::wstring& pattern);

    // ɾ��·��, �������ļ�����Ŀ¼. �����Ŀ¼, ��recursiveΪtrueʱ,
    // ��ɾ��Ŀ¼�������ݰ�����Ŀ¼, �����Ƴ�Ŀ¼(��Ŀ¼).
    // �ɹ�����true, ���򷵻�false.
    //
    // ����: ʹ��recursive==true�ȼ���"rm -rf", ��ҪС��.
    bool Delete(const FilePath& path, bool recursive);

    // ���Ȳ���ϵͳ���´�������ʱ��ɾ���ļ���Ŀ¼.
    // ע��:
    // 1) ��ɾ�����ļ�/Ŀ¼Ӧ������ʱĿ¼.
    // 2) ��ɾ����Ŀ¼����Ϊ��.
    bool DeleteAfterReboot(const FilePath& path);

    // �����ļ�·������Ϣ.
    bool GetFileInfo(const FilePath& file_path, PlatformFileInfo* info);

    // ��װfopen����. �ɹ����طǿյ�FILE*.
    FILE* OpenFile(const FilePath& filename, const char* mode);

    // �ر�OpenFile�򿪵��ļ�. �ɹ�����true.
    bool CloseFile(FILE* file);

    // ����ϵͳ�ṩ����ʱĿ¼.
    bool GetTempDir(FilePath* path);

    // ������ʱ�ļ�. |path|��ȫ·��, �����ļ��ɹ���������true. ��������ʱ
    // �ļ�Ϊ��, ���о������ر�.
    bool CreateTemporaryFile(FilePath* path);

    // ����CreateTemporaryFile, �ļ���|dir|Ŀ¼�д���.
    bool CreateTemporaryFileInDir(const FilePath& dir, FilePath* temp_file);

    // ����������ʱ�ļ�. �ļ���Ȩ��Ϊ��/д. |path|Ϊȫ·��.
    // ���ش��ļ��ľ��, �������󷵻�NULL.
    FILE* CreateAndOpenTemporaryFile(FilePath* path);
    // ����CreateAndOpenTemporaryFile, �ļ���|dir|Ŀ¼�д���.
    FILE* CreateAndOpenTemporaryFileInDir(const FilePath& dir, FilePath* path);

    // ��ȡ|path|�ļ����ݵ�|contents|, �ɹ�����true. |contents|����Ϊ��, ��ʱ
    // ���������жϴ����ļ��Ƿ����.
    bool ReadFileToString(const FilePath& path, std::string* contents);

    // ���ļ���ȡָ�����ֽ����ݵ�������. ����ʵ�ʶ�ȡ�����ֽ���, ���󷵻�-1.
    int ReadFile(const FilePath& filename, char* data, int size);

    // д�뻺�������ݵ��ļ�, ����֮ǰ������. ����ʵ��д����ֽ���, ���󷵻�-1.
    int WriteFile(const FilePath& filename, const char* data, int size);

    // Moves the given path, whether it's a file or a directory.
    // If a simple rename is not possible, such as in the case where the paths are
    // on different volumes, this will attempt to copy and delete. Returns
    // true for success.
    bool Move(const FilePath& from_path, const FilePath& to_path);

    // �޸��ļ���|from_path|Ϊ|to_path|. ����·����������ͬ�ľ���, ��������ʧ��.
    // Ŀ���ļ�������ʱ�ᱻ����. ������ʱ�ļ�ʱ����ѡ���������������Move. Windows
    // ƽ̨��Ŀ���ļ������Իᱣ��. �ɹ�����true.
    bool ReplaceFile(const FilePath& from_path, const FilePath& to_path);

    // �������·�����ڱ����ļ�ϵͳ�򷵻�true, ���򷵻�false.
    bool PathExists(const FilePath& path);

    // Returns true if the given path exists and is a directory, false otherwise.
    bool DirectoryExists(const FilePath& path);

    // ��ȡ���̵�ǰ����Ŀ¼.
    bool GetCurrentDirectory(FilePath* path);

    // ���ý��̵�ǰ����Ŀ¼.
    bool SetCurrentDirectory(const FilePath& path);

    // Copies the given path, and optionally all subdirectories and their contents
    // as well.
    // If there are files existing under to_path, always overwrite.
    // Returns true if successful, false otherwise.
    // Don't use wildcards on the names, it may stop working without notice.
    //
    // If you only need to copy a file use CopyFile, it's faster.
    bool CopyDirectory(const FilePath& from_path,
        const FilePath& to_path,
        bool recursive);

    // Copy from_path to to_path recursively and then delete from_path recursively.
    // Returns true if all operations succeed.
    // This function simulates Move(), but unlike Move() it works across volumes.
    // This fuction is not transactional.
    bool CopyAndDeleteDirectory(const FilePath& from_path, const FilePath& to_path);

    // Create a new directory. If prefix is provided, the new directory name is in
    // the format of prefixyyyy.
    // NOTE: prefix is ignored in the POSIX implementation.
    // If success, return true and output the full path of the directory created.
    bool CreateNewTempDirectory(const FilePath::StringType& prefix,
        FilePath* new_temp_path);

    // Create a directory within another directory.
    // Extra characters will be appended to |prefix| to ensure that the
    // new directory does not have the same name as an existing directory.
    bool CreateTemporaryDirInDir(const FilePath& base_dir,
        const FilePath::StringType& prefix,
        FilePath* new_dir);

    // Creates a directory, as well as creating any parent directories, if they
    // don't exist. Returns 'true' on successful creation, or if the directory
    // already exists.  The directory is only readable by the current user.
    bool CreateDirectory(const FilePath& full_path);

    // Returns true if the given path's base name is ".".
    bool IsDot(const FilePath& path);

    // Returns true if the given path's base name is "..".
    bool IsDotDot(const FilePath& path);

    // ö��·���������ļ�, ����֤����.
    // ����������������ʽ, ��Ҫ�����߳���ʹ��.
    class FileEnumerator
    {
    public:
        typedef WIN32_FIND_DATA FindInfo;

        enum FileType
        {
            FILES                 = 1 << 0,
            DIRECTORIES           = 1 << 1,
            INCLUDE_DOT_DOT       = 1 << 2,
        };

        // |root_path|�Ǳ�������ʼĿ¼, ���ܲ��Է�б�߽�β.
        //
        // ���|recursive|��true, ��ݹ������Ŀ¼. ���ù�����ȱ�����ʽ, ����
        // ��ǰĿ¼���ļ�������Ŀ¼���ļ�����.
        //
        // |file_type|ָ����ƥ���ļ�����Ŀ¼�������߶�ƥ��.
        //
        // |pattern|�ǿ�ѡ���ļ�ƥ��ģʽ, ʵ��shell�Ĳ�������, ����"*.txt"����
        // "Foo???.doc". ����ĳЩƥ��ģʽ���ǿ�ƽ̨��, ��Ϊ�ײ���õ���OS���
        // ����. һ����˵, Windows��ƥ������Ҫ��������ƽ̨, ���Ȳ���. ���û
        // ָ��, ƥ�������ļ�.
        // ע��: ƥ��ģʽ���޶�root_pathĿ¼��Ч, �ݹ����Ŀ¼�²�������.
        FileEnumerator(const FilePath& root_path,
            bool recursive,
            FileType file_type);
        FileEnumerator(const FilePath& root_path,
            bool recursive,
            FileType file_type,
            const FilePath::StringType& pattern);
        ~FileEnumerator();

        // ���û����һ��·�����ؿ��ַ���.
        FilePath Next();

        // д�ļ���Ϣ��|info|.
        void GetFindInfo(FindInfo* info);

        // ���FindInfo�ǲ���Ŀ¼.
        static bool IsDirectory(const FindInfo& info);

        static FilePath GetFilename(const FindInfo& find_info);
        static int64 GetFilesize(const FindInfo& find_info);
        static base::Time GetLastModifiedTime(const FindInfo& find_info);

    private:
        // ���ö�ٿ�����������·������true.
        bool ShouldSkip(const FilePath& path);

        // ��find_data_�Ϸ�ʱΪtrue.
        bool has_find_data_;
        WIN32_FIND_DATA find_data_;
        HANDLE find_handle_;

        FilePath root_path_;
        bool recursive_;
        FileType file_type_;
        std::wstring pattern_; // ������ƥ��ʱ�ַ���Ϊ��.

        // ��¼������Ȳ����л���Ҫ��������Ŀ¼.
        std::stack<FilePath> pending_paths_;

        DISALLOW_COPY_AND_ASSIGN(FileEnumerator);
    };


    class MemoryMappedFile
    {
    public:
        // ȱʡ���캯��, ���г�Ա������Ϊ�Ƿ�/��ֵ.
        MemoryMappedFile();
        ~MemoryMappedFile();

        // ��һ�����ڵ��ļ���ӳ�䵽�ڴ���. ����Ȩ����Ϊֻ��. ��������Ѿ�ָ��һ��
        // �Ϸ����ڴ�ӳ���ļ�, ���û�ʧ�ܲ�����false. ����޷����ļ����ļ�������
        // �����ڴ�ӳ��ʧ��, ��������false. �Ժ���ܻ�����ָ������Ȩ��.
        bool Initialize(const FilePath& file_name);
        // ������һ��, ֻ���ļ��������Ѵ򿪵�. MemoryMappedFile��ӹ�|file|������Ȩ,
        // ����֮���ر�.
        bool Initialize(PlatformFile file);

        const uint8* data() const { return data_; }
        size_t length() const { return length_; }

        // file_��ָ��һ���򿪵��ڴ�ӳ���ļ��ĺϷ������?
        bool IsValid();

    private:
        // ��ָ���ļ�, ���ݸ�MapFileToMemoryInternal().
        bool MapFileToMemory(const FilePath& file_name);

        // ӳ���ļ����ڴ�, ����data_Ϊ�ڴ��ַ. ����ɹ�����true, �κ�ʧ�����ζ���
        // ����false. Initialize()�ĸ�������.
        bool MapFileToMemoryInternal();

        // MapFileToMemoryInternal���øú���, ���Դ���ӳ��εı�־λ.
        bool MapFileToMemoryInternalEx(int flags);

        // �ر����д򿪵ľ��. �����Ժ���ܻ��ɹ�����.
        void CloseHandles();

        PlatformFile file_;
        HANDLE file_mapping_;
        uint8* data_;
        size_t length_;

        DISALLOW_COPY_AND_ASSIGN(MemoryMappedFile);
    };

} //namespace base

#endif //__base_file_util_h__