
#ifndef __base_platform_file_h__
#define __base_platform_file_h__

#pragma once

#include "file_path.h"
#include "time.h"

namespace base
{

    typedef HANDLE PlatformFile;
    const PlatformFile kInvalidPlatformFileValue = INVALID_HANDLE_VALUE;

    enum PlatformFileFlags
    {
        PLATFORM_FILE_OPEN = 1,
        PLATFORM_FILE_CREATE = 2,
        PLATFORM_FILE_OPEN_ALWAYS = 4,          // ���ܻ��½��ļ�.
        PLATFORM_FILE_CREATE_ALWAYS = 8,        // ���ܻḲ�����ļ�.
        PLATFORM_FILE_READ = 16,
        PLATFORM_FILE_WRITE = 32,
        PLATFORM_FILE_EXCLUSIVE_READ = 64,      // EXCLUSIVE��Windows��SHARE�����෴.
        PLATFORM_FILE_EXCLUSIVE_WRITE = 128,
        PLATFORM_FILE_ASYNC = 256,
        PLATFORM_FILE_TEMPORARY = 512,          // ����Windows�Ͽ���.
        PLATFORM_FILE_HIDDEN = 1024,            // ����Windows�Ͽ���.
        PLATFORM_FILE_DELETE_ON_CLOSE = 2048,
        PLATFORM_FILE_TRUNCATE = 4096,
        PLATFORM_FILE_WRITE_ATTRIBUTES = 8192,  // ����Windows�Ͽ���.

        PLATFORM_FILE_SHARE_DELETE = 32768,     // ����Windows�Ͽ���.
    };

    // �����ļ�ϵͳ���Ƶ��µĵ���ʧ�ܻ᷵��PLATFORM_FILE_ERROR_ACCESS_DENIED.
    // ���ڰ�ȫ���Ե��µĽ�ֹ�����᷵��PLATFORM_FILE_ERROR_SECURITY.
    enum PlatformFileError
    {
        PLATFORM_FILE_OK = 0,
        PLATFORM_FILE_ERROR_FAILED = -1,
        PLATFORM_FILE_ERROR_IN_USE = -2,
        PLATFORM_FILE_ERROR_EXISTS = -3,
        PLATFORM_FILE_ERROR_NOT_FOUND = -4,
        PLATFORM_FILE_ERROR_ACCESS_DENIED = -5,
        PLATFORM_FILE_ERROR_TOO_MANY_OPENED = -6,
        PLATFORM_FILE_ERROR_NO_MEMORY = -7,
        PLATFORM_FILE_ERROR_NO_SPACE = -8,
        PLATFORM_FILE_ERROR_NOT_A_DIRECTORY = -9,
        PLATFORM_FILE_ERROR_INVALID_OPERATION = -10,
        PLATFORM_FILE_ERROR_SECURITY = -11,
        PLATFORM_FILE_ERROR_ABORT = -12,
        PLATFORM_FILE_ERROR_NOT_A_FILE = -13,
        PLATFORM_FILE_ERROR_NOT_EMPTY = -14,
    };

    // ���ڱ����ļ�����Ϣ. ���Ҫ���ṹ������µ��ֶ�, ����ͬʱ����cpp�еĺ���.
    struct PlatformFileInfo
    {
        PlatformFileInfo();
        ~PlatformFileInfo();

        // �ļ��Ĵ�С(�ֽ�). ��is_directoryΪtrueʱδ����.
        int64 size;

        // �ļ���һ��Ŀ¼ʱΪtrue.
        bool is_directory;

        // �ļ���һ������ʱΪtrue.
        bool is_symbolic_link;

        // �ļ�����޸�ʱ��.
        base::Time last_modified;

        // �ļ�������ʱ��.
        base::Time last_accessed;

        // �ļ�����ʱ��.
        base::Time creation_time;
    };

    // �������ߴ��ļ�. ���ʹ��PLATFORM_FILE_OPEN_ALWAYS, �Ҵ�����Ч��|created|,
    // �������ļ�ʱ����|created|Ϊtrue, ���ļ�ʱ����|created|Ϊfalse.
    // |error_code|����ΪNULL.
    PlatformFile CreatePlatformFile(const FilePath& name,
        int flags, bool* created, PlatformFileError* error_code);

    // �ر��ļ����. �ɹ�����|true|, ʧ�ܷ���|false|.
    bool ClosePlatformFile(PlatformFile file);

    // Reads the given number of bytes (or until EOF is reached) starting with the
    // given offset. Returns the number of bytes read, or -1 on error. Note that
    // this function makes a best effort to read all data on all platforms, so it is
    // not intended for stream oriented files but instead for cases when the normal
    // expectation is that actually |size| bytes are read unless there is an error.
    int ReadPlatformFile(PlatformFile file, int64 offset, char* data, int size);

    // Reads the given number of bytes (or until EOF is reached) starting with the
    // given offset, but does not make any effort to read all data on all platforms.
    // Returns the number of bytes read, or -1 on error.
    int ReadPlatformFileNoBestEffort(PlatformFile file, int64 offset,
        char* data, int size);

    // Writes the given buffer into the file at the given offset, overwritting any
    // data that was previously there. Returns the number of bytes written, or -1
    // on error. Note that this function makes a best effort to write all data on
    // all platforms.
    int WritePlatformFile(PlatformFile file, int64 offset,
        const char* data, int size);

    // �����ļ��ĳ���. ���|length|���ڵ�ǰ�ļ�����, ���䲿�ֻ����0. �ļ�������
    // ����false.
    bool TruncatePlatformFile(PlatformFile file, int64 length);

    // �����ļ��Ļ��������ݵ�����.
    bool FlushPlatformFile(PlatformFile file);

    // �����ļ���������ʱ����޸�ʱ��.
    bool TouchPlatformFile(PlatformFile file, const Time& last_access_time,
        const Time& last_modified_time);

    // �����ļ��Ļ�����Ϣ.
    bool GetPlatformFileInfo(PlatformFile file, PlatformFileInfo* info);

    // PassPlatformFile���ڴ���PlatformFile������Ȩ��������, �౾���ӹ�
    // ����Ȩ.
    //
    // ʾ��:
    //
    //  void MaybeProcessFile(PassPlatformFile pass_file) {
    //    if(...) {
    //      PlatformFile file = pass_file.ReleaseValue();
    //      // Now, we are responsible for closing |file|.
    //    }
    //  }
    //
    //  void OpenAndMaybeProcessFile(const FilePath& path) {
    //    PlatformFile file = CreatePlatformFile(path, ...);
    //    MaybeProcessFile(PassPlatformFile(&file));
    //    if(file != kInvalidPlatformFileValue)
    //      ClosePlatformFile(file);
    //  }
    class PassPlatformFile
    {
    public:
        explicit PassPlatformFile(PlatformFile* value) : value_(value) {}

        // ���ض����д洢��PlatformFile, ֮������߻�ȡ����Ȩ, Ӧ�ø����ļ��Ĺر�.
        // �κκ����ĵ��ö������طǷ���PlatformFile.
        PlatformFile ReleaseValue()
        {
            PlatformFile temp = *value_;
            *value_ = kInvalidPlatformFileValue;
            return temp;
        }

    private:
        PlatformFile* value_;
    };

} //namespace base

#endif //__base_platform_file_h__