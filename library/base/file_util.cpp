
#include "file_util.h"

#include <limits>

#include "logging.h"
#include "metric/histogram.h"
#include "string_number_conversions.h"
#include "string_util.h"
#include "threading/thread_restrictions.h"
#include "utf_string_conversions.h"
#include "win/scoped_handle.h"
#include "win/windows_version.h"

namespace base
{

    namespace
    {

        const DWORD kFileShareAll = FILE_SHARE_READ | FILE_SHARE_WRITE |
            FILE_SHARE_DELETE;

        // Helper for NormalizeFilePath(), defined below.
        bool DevicePathToDriveLetterPath(const FilePath& device_path,
            FilePath* drive_letter_path)
        {
            base::ThreadRestrictions::AssertIOAllowed();

            // Get the mapping of drive letters to device paths.
            const int kDriveMappingSize = 1024;
            wchar_t drive_mapping[kDriveMappingSize] = { '\0' };
            if(!::GetLogicalDriveStrings(kDriveMappingSize-1, drive_mapping))
            {
                LOG(ERROR) << "Failed to get drive mapping.";
                return false;
            }

            // The drive mapping is a sequence of null terminated strings.
            // The last string is empty.
            wchar_t* drive_map_ptr = drive_mapping;
            wchar_t device_name[MAX_PATH];
            wchar_t drive[] = L" :";

            // For each string in the drive mapping, get the junction that links
            // to it.  If that junction is a prefix of |device_path|, then we
            // know that |drive| is the real path prefix.
            while(*drive_map_ptr)
            {
                drive[0] = drive_map_ptr[0]; // Copy the drive letter.

                if(QueryDosDevice(drive, device_name, MAX_PATH) &&
                    StartsWith(device_path.value(), device_name, true))
                {
                    *drive_letter_path = FilePath(drive +
                        device_path.value().substr(wcslen(device_name)));
                    return true;
                }
                // Move to the next drive letter string, which starts one
                // increment after the '\0' that terminates the current string.
                while(*drive_map_ptr++) ;
            }

            // No drive matched.  The path does not start with a device junction
            // that is mounted as a drive letter.  This means there is no drive
            // letter path to the volume that holds |device_path|, so fail.
            return false;
        }

    }

    bool AbsolutePath(FilePath* path)
    {
        base::ThreadRestrictions::AssertIOAllowed();
        wchar_t file_path_buf[MAX_PATH];
        if(!_wfullpath(file_path_buf, path->value().c_str(), MAX_PATH))
        {
            return false;
        }
        *path = FilePath(file_path_buf);
        return true;
    }

    int CountFilesCreatedAfter(const FilePath& path, const Time& comparison_time)
    {
        ThreadRestrictions::AssertIOAllowed();

        int file_count = 0;
        FILETIME comparison_filetime(comparison_time.ToFileTime());

        WIN32_FIND_DATA find_file_data;
        // ���������ļ�.
        std::wstring filename_spec = path.Append(L"*").value();
        HANDLE find_handle = FindFirstFile(filename_spec.c_str(), &find_file_data);
        if(find_handle != INVALID_HANDLE_VALUE)
        {
            do
            {
                // ��ǰĿ¼�͸�Ŀ¼������.
                if((wcscmp(find_file_data.cFileName, L"..")==0) ||
                    (wcscmp(find_file_data.cFileName, L".")==0))
                {
                    continue;
                }

                long result = CompareFileTime(&find_file_data.ftCreationTime,
                    &comparison_filetime);
                // �ļ�����ʱ����ڻ������ڱȶ�ʱ��.
                if((result==1) || (result==0))
                {
                    ++file_count;
                }
            } while(FindNextFile(find_handle,  &find_file_data));
            FindClose(find_handle);
        }

        return file_count;
    }

    int64 ComputeDirectorySize(const FilePath& root_path)
    {
        int64 running_size = 0;
        FileEnumerator file_iter(root_path, true, FileEnumerator::FILES);
        for(FilePath current=file_iter.Next(); !current.empty();
            current=file_iter.Next())
        {
            FileEnumerator::FindInfo info;
            file_iter.GetFindInfo(&info);
            LARGE_INTEGER li = { info.nFileSizeLow, info.nFileSizeHigh };
            running_size += li.QuadPart;
        }
        return running_size;
    }

    int64 ComputeFilesSize(const FilePath& directory,
        const std::wstring& pattern)
    {
        int64 running_size = 0;
        FileEnumerator file_iter(directory, false, FileEnumerator::FILES, pattern);
        for(FilePath current=file_iter.Next(); !current.empty();
            current=file_iter.Next())
        {
            FileEnumerator::FindInfo info;
            file_iter.GetFindInfo(&info);
            LARGE_INTEGER li = { info.nFileSizeLow, info.nFileSizeHigh };
            running_size += li.QuadPart;
        }
        return running_size;
    }

    bool Delete(const FilePath& path, bool recursive)
    {
        ThreadRestrictions::AssertIOAllowed();

        if(path.value().length() >= MAX_PATH)
        {
            return false;
        }

        if(!recursive)
        {
            // ������ݹ�ɾ��, �ȼ��|path|�Ƿ�ΪĿ¼, �����, ��RemoveDirectory�Ƴ�.
            PlatformFileInfo file_info;
            if(GetFileInfo(path, &file_info) && file_info.is_directory)
            {
                return RemoveDirectory(path.value().c_str()) != 0;
            }

            // ����, ·����ʾ�ļ�, ͨ������߲�����. �ȳ�����DeleteFile, ��Ϊ�������
            // �ȽϿ�. ���DeleteFileʧ��, ��������SHFileOperation��ɲ���.
            if(DeleteFile(path.value().c_str()) != 0)
            {
                return true;
            }
        }

        // SHFILEOPSTRUCTҪ��·��������NULL�ս�, ��˱���ʹ��wcscpy, ��Ϊ
        // wcscpy_s����ʣ��Ļ�����д���NULLֵ.
        wchar_t double_terminated_path[MAX_PATH+1] = { 0 };
#pragma warning(suppress:4996) // �����"����ʹ��wcscpy"�ľ���.
        wcscpy(double_terminated_path, path.value().c_str());

        SHFILEOPSTRUCT file_operation = { 0 };
        file_operation.wFunc = FO_DELETE;
        file_operation.pFrom = double_terminated_path;
        file_operation.fFlags = FOF_NOERRORUI | FOF_SILENT | FOF_NOCONFIRMATION;
        if(!recursive)
        {
            file_operation.fFlags |= FOF_NORECURSION | FOF_FILESONLY;
        }
        int err = SHFileOperation(&file_operation);

        // Since we're passing flags to the operation telling it to be silent,
        // it's possible for the operation to be aborted/cancelled without err
        // being set (although MSDN doesn't give any scenarios for how this can
        // happen).  See MSDN for SHFileOperation and SHFILEOPTSTRUCT.
        if(file_operation.fAnyOperationsAborted)
        {
            return false;
        }

        // ��һЩWindows�汾��ɾ����Ŀ¼ʱ����ERROR_FILE_NOT_FOUND(0x2), ��ʱ����
        // Ӧ�÷���ERROR_FILE_NOT_FOUND��ʱ��ȴ����0x402. MSDN��˵Vista�����ϵ�
        // �汾���᷵��0x402.
        return (err==0 || err==ERROR_FILE_NOT_FOUND || err==0x402);
    }

    bool DeleteAfterReboot(const FilePath& path)
    {
        ThreadRestrictions::AssertIOAllowed();

        if(path.value().length() >= MAX_PATH)
        {
            return false;
        }

        return MoveFileEx(path.value().c_str(), NULL,
            MOVEFILE_DELAY_UNTIL_REBOOT|MOVEFILE_REPLACE_EXISTING) != FALSE;
    }

    bool GetFileInfo(const FilePath& file_path, PlatformFileInfo* results)
    {
        ThreadRestrictions::AssertIOAllowed();

        WIN32_FILE_ATTRIBUTE_DATA attr;
        if(!GetFileAttributesEx(file_path.value().c_str(),
            GetFileExInfoStandard, &attr))
        {
            return false;
        }

        ULARGE_INTEGER size;
        size.HighPart = attr.nFileSizeHigh;
        size.LowPart = attr.nFileSizeLow;
        results->size = size.QuadPart;

        results->is_directory =
            (attr.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0;
        results->last_modified = Time::FromFileTime(attr.ftLastWriteTime);
        results->last_accessed = Time::FromFileTime(attr.ftLastAccessTime);
        results->creation_time = Time::FromFileTime(attr.ftCreationTime);

        return true;
    }

    FILE* OpenFile(const FilePath& filename, const char* mode)
    {
        ThreadRestrictions::AssertIOAllowed();
        std::wstring w_mode = ASCIIToWide(std::string(mode));
        return _wfsopen(filename.value().c_str(), w_mode.c_str(), _SH_DENYNO);
    }

    bool CloseFile(FILE* file)
    {
        if(file == NULL)
        {
            return true;
        }
        return fclose(file) == 0;
    }

    bool GetTempDir(FilePath* path)
    {
        ThreadRestrictions::AssertIOAllowed();

        wchar_t temp_path[MAX_PATH+1];
        DWORD path_len = ::GetTempPath(MAX_PATH, temp_path);
        if(path_len>=MAX_PATH || path_len<=0)
        {
            return false;
        }
        // TODO(evanm): the old behavior of this function was to always strip the
        // trailing slash.  We duplicate this here, but it shouldn't be necessary
        // when everyone is using the appropriate FilePath APIs.
        *path = FilePath(temp_path).StripTrailingSeparators();
        return true;
    }

    bool CreateTemporaryFile(FilePath* path)
    {
        ThreadRestrictions::AssertIOAllowed();

        FilePath temp_file;

        if(!GetTempDir(path))
        {
            return false;
        }

        if(CreateTemporaryFileInDir(*path, &temp_file))
        {
            *path = temp_file;
            return true;
        }

        return false;
    }

    bool CreateTemporaryFileInDir(const FilePath& dir,
        FilePath* temp_file)
    {
        ThreadRestrictions::AssertIOAllowed();

        wchar_t temp_name[MAX_PATH+1];

        if(!GetTempFileName(dir.value().c_str(), L"", 0, temp_name))
        {
            PLOG(WARNING) << "Failed to get temporary file name in " << dir.value();
            return false;
        }

        DWORD path_len = GetLongPathName(temp_name, temp_name, MAX_PATH);
        if(path_len>MAX_PATH+1 || path_len==0)
        {
            PLOG(WARNING) << "Failed to get long path name for " << temp_name;
            return false;
        }

        std::wstring temp_file_str;
        temp_file_str.assign(temp_name, path_len);
        *temp_file = FilePath(temp_file_str);
        return true;
    }

    FILE* CreateAndOpenTemporaryFile(FilePath* path)
    {
        FilePath directory;
        if(!GetTempDir(&directory))
        {
            return NULL;
        }

        return CreateAndOpenTemporaryFileInDir(directory, path);
    }

    // On POSIX we have semantics to create and open a temporary file
    // atomically.
    // TODO(jrg): is there equivalent call to use on Windows instead of
    // going 2-step?
    FILE* CreateAndOpenTemporaryFileInDir(const FilePath& dir, FilePath* path)
    {
        ThreadRestrictions::AssertIOAllowed();
        if(!CreateTemporaryFileInDir(dir, path))
        {
            return NULL;
        }
        // �Զ�����ģʽ���ļ�, ����fwrite��������. Windowsƽ̨����\r\n�滻\n.
        // �μ�: http://msdn.microsoft.com/en-us/library/h9t88zwz(VS.71).aspx
        return OpenFile(*path, "wb+");
    }

    bool ReadFileToString(const FilePath& path, std::string* contents)
    {
        FILE* file = OpenFile(path, "rb");
        if(!file)
        {
            return false;
        }

        char buf[1 << 16];
        size_t len;
        while((len=fread(buf, 1, sizeof(buf), file)) > 0)
        {
            if(contents)
            {
                contents->append(buf, len);
            }
        }
        CloseFile(file);

        return true;
    }

    bool Move(const FilePath& from_path, const FilePath& to_path)
    {
        base::ThreadRestrictions::AssertIOAllowed();

        // NOTE: I suspect we could support longer paths, but that would involve
        // analyzing all our usage of files.
        if(from_path.value().length()>=MAX_PATH ||
            to_path.value().length()>=MAX_PATH)
        {
            return false;
        }
        if(MoveFileEx(from_path.value().c_str(), to_path.value().c_str(),
            MOVEFILE_COPY_ALLOWED | MOVEFILE_REPLACE_EXISTING) != 0)
        {
            return true;
        }

        // Keep the last error value from MoveFileEx around in case the below
        // fails.
        bool ret = false;
        DWORD last_error = ::GetLastError();

        if(DirectoryExists(from_path))
        {
            // MoveFileEx fails if moving directory across volumes. We will simulate
            // the move by using Copy and Delete. Ideally we could check whether
            // from_path and to_path are indeed in different volumes.
            ret = CopyAndDeleteDirectory(from_path, to_path);
        }

        if(!ret)
        {
            // Leave a clue about what went wrong so that it can be (at least) picked
            // up by a PLOG entry.
            ::SetLastError(last_error);
        }

        return ret;
    }

    int ReadFile(const FilePath& filename, char* data, int size)
    {
        ThreadRestrictions::AssertIOAllowed();
        win::ScopedHandle file(CreateFile(filename.value().c_str(),
            GENERIC_READ,
            FILE_SHARE_READ|FILE_SHARE_WRITE,
            NULL,
            OPEN_EXISTING,
            FILE_FLAG_SEQUENTIAL_SCAN,
            NULL));
        if(!file)
        {
            return -1;
        }

        DWORD read;
        if(::ReadFile(file, data, size, &read, NULL) &&
            static_cast<int>(read)==size)
        {
            return read;
        }
        return -1;
    }

    int WriteFile(const FilePath& filename, const char* data, int size)
    {
        ThreadRestrictions::AssertIOAllowed();
        win::ScopedHandle file(CreateFileW(filename.value().c_str(),
            GENERIC_WRITE,
            0,
            NULL,
            CREATE_ALWAYS,
            0,
            NULL));
        if(!file)
        {
            LOG(WARNING) << "CreateFile failed for path " << filename.value()
                << " error code=" << GetLastError();
            return -1;
        }

        DWORD written;
        BOOL result = ::WriteFile(file, data, size, &written, NULL);
        if(result && static_cast<int>(written)==size)
        {
            return written;
        }

        if(!result)
        {
            // WriteFile failed.
            LOG(WARNING) << "writing file " << filename.value()
                << " failed, error code=" << GetLastError();
        }
        else
        {
            // Didn't write all the bytes.
            LOG(WARNING) << "wrote" << written << " bytes to " <<
                filename.value() << " expected " << size;
        }
        return -1;
    }

    bool ReplaceFile(const FilePath& from_path, const FilePath& to_path)
    {
        ThreadRestrictions::AssertIOAllowed();

        // ȷ��Ŀ���ļ�����.
        HANDLE target_file = ::CreateFile(
            to_path.value().c_str(),
            0,
            FILE_SHARE_READ|FILE_SHARE_WRITE,
            NULL,
            CREATE_NEW,
            FILE_ATTRIBUTE_NORMAL,
            NULL);
        if(target_file != INVALID_HANDLE_VALUE)
        {
            ::CloseHandle(target_file);
        }
        // д���繲��ʱ, �����޷��޸�ACLs. ��ʱ����ACL����
        // (REPLACEFILE_IGNORE_MERGE_ERRORS).
        return ::ReplaceFile(to_path.value().c_str(),
            from_path.value().c_str(), NULL,
            REPLACEFILE_IGNORE_MERGE_ERRORS, NULL, NULL) ? true : false;
    }

    bool PathExists(const FilePath& path)
    {
        ThreadRestrictions::AssertIOAllowed();
        return (GetFileAttributes(path.value().c_str()) != INVALID_FILE_ATTRIBUTES);
    }

    bool DirectoryExists(const FilePath& path)
    {
        ThreadRestrictions::AssertIOAllowed();
        DWORD fileattr = GetFileAttributes(path.value().c_str());
        if(fileattr != INVALID_FILE_ATTRIBUTES)
        {
            return (fileattr & FILE_ATTRIBUTE_DIRECTORY) != 0;
        }
        return false;
    }

    bool GetCurrentDirectory(FilePath* dir)
    {
        ThreadRestrictions::AssertIOAllowed();

        wchar_t system_buffer[MAX_PATH];
        system_buffer[0] = 0;
        DWORD len = ::GetCurrentDirectory(MAX_PATH, system_buffer);
        if(len==0 || len>MAX_PATH)
        {
            return false;
        }
        // TODO(evanm): the old behavior of this function was to always strip the
        // trailing slash.  We duplicate this here, but it shouldn't be necessary
        // when everyone is using the appropriate FilePath APIs.
        std::wstring dir_str(system_buffer);
        *dir = FilePath(dir_str).StripTrailingSeparators();
        return true;
    }

    bool SetCurrentDirectory(const FilePath& directory)
    {
        ThreadRestrictions::AssertIOAllowed();
        BOOL ret = ::SetCurrentDirectory(directory.value().c_str());
        return ret != 0;
    }

    bool ShellCopy(const FilePath& from_path, const FilePath& to_path,
        bool recursive)
    {
        base::ThreadRestrictions::AssertIOAllowed();

        // NOTE: I suspect we could support longer paths, but that would involve
        // analyzing all our usage of files.
        if(from_path.value().length()>=MAX_PATH ||
            to_path.value().length()>=MAX_PATH)
        {
            return false;
        }

        // SHFILEOPSTRUCT wants the path to be terminated with two NULLs,
        // so we have to use wcscpy because wcscpy_s writes non-NULLs
        // into the rest of the buffer.
        wchar_t double_terminated_path_from[MAX_PATH + 1] = { 0 };
        wchar_t double_terminated_path_to[MAX_PATH + 1] = { 0 };
#pragma warning(suppress:4996) // don't complain about wcscpy deprecation
        wcscpy(double_terminated_path_from, from_path.value().c_str());
#pragma warning(suppress:4996) // don't complain about wcscpy deprecation
        wcscpy(double_terminated_path_to, to_path.value().c_str());

        SHFILEOPSTRUCT file_operation = { 0 };
        file_operation.wFunc = FO_COPY;
        file_operation.pFrom = double_terminated_path_from;
        file_operation.pTo = double_terminated_path_to;
        file_operation.fFlags = FOF_NOERRORUI | FOF_SILENT | FOF_NOCONFIRMATION |
            FOF_NOCONFIRMMKDIR;
        if(!recursive)
        {
            file_operation.fFlags |= FOF_NORECURSION | FOF_FILESONLY;
        }

        return (SHFileOperation(&file_operation) == 0);
    }

    bool CopyDirectory(const FilePath& from_path, const FilePath& to_path,
        bool recursive)
    {
        base::ThreadRestrictions::AssertIOAllowed();

        if(recursive)
        {
            return ShellCopy(from_path, to_path, true);
        }

        // The following code assumes that from path is a directory.
        DCHECK(DirectoryExists(from_path));

        // Instead of creating a new directory, we copy the old one to include the
        // security information of the folder as part of the copy.
        if(!PathExists(to_path))
        {
            // Except that Vista fails to do that, and instead do a recursive copy if
            // the target directory doesn't exist.
            if(base::win::GetVersion() >= base::win::VERSION_VISTA)
            {
                CreateDirectory(to_path);
            }
            else
            {
                ShellCopy(from_path, to_path, false);
            }
        }

        FilePath directory = from_path.Append(L"*.*");
        return ShellCopy(directory, to_path, false);
    }

    bool CopyAndDeleteDirectory(const FilePath& from_path,
        const FilePath& to_path)
    {
        base::ThreadRestrictions::AssertIOAllowed();
        if(CopyDirectory(from_path, to_path, true))
        {
            if(Delete(from_path, true))
            {
                return true;
            }
            // Like Move, this function is not transactional, so we just
            // leave the copied bits behind if deleting from_path fails.
            // If to_path exists previously then we have already overwritten
            // it by now, we don't get better off by deleting the new bits.
        }
        return false;
    }

    bool CreateNewTempDirectory(const FilePath::StringType& prefix,
        FilePath* new_temp_path)
    {
        ThreadRestrictions::AssertIOAllowed();

        FilePath system_temp_dir;
        if(!GetTempDir(&system_temp_dir))
        {
            return false;
        }

        return CreateTemporaryDirInDir(system_temp_dir, prefix, new_temp_path);
    }

    bool CreateTemporaryDirInDir(const FilePath& base_dir,
        const FilePath::StringType& prefix,
        FilePath* new_dir)
    {
        ThreadRestrictions::AssertIOAllowed();

        FilePath path_to_create;
        srand(static_cast<uint32>(time(NULL)));

        for(int count=0; count<50; ++count)
        {
            // Try create a new temporary directory with random generated name. If
            // the one exists, keep trying another path name until we reach some limit.
            string16 new_dir_name;
            new_dir_name.assign(prefix);
            new_dir_name.append(IntToString16(rand() % kint16max));

            path_to_create = base_dir.Append(new_dir_name);
            if(::CreateDirectory(path_to_create.value().c_str(), NULL))
            {
                *new_dir = path_to_create;
                return true;
            }
        }

        return false;
    }

    bool CreateDirectory(const FilePath& full_path)
    {
        ThreadRestrictions::AssertIOAllowed();

        // If the path exists, we've succeeded if it's a directory, failed otherwise.
        const wchar_t* full_path_str = full_path.value().c_str();
        DWORD fileattr = ::GetFileAttributes(full_path_str);
        if(fileattr != INVALID_FILE_ATTRIBUTES)
        {
            if((fileattr&FILE_ATTRIBUTE_DIRECTORY) != 0)
            {
                DVLOG(1) << "CreateDirectory(" << full_path_str << "), "
                    << "directory already exists.";
                return true;
            }
            LOG(WARNING) << "CreateDirectory(" << full_path_str << "), "
                << "conflicts with existing file.";
            return false;
        }

        // Invariant:  Path does not exist as file or directory.

        // Attempt to create the parent recursively.  This will immediately return
        // true if it already exists, otherwise will create all required parent
        // directories starting with the highest-level missing parent.
        FilePath parent_path(full_path.DirName());
        if(parent_path.value() == full_path.value())
        {
            return false;
        }
        if(!CreateDirectory(parent_path))
        {
            DLOG(WARNING) << "Failed to create one of the parent directories.";
            return false;
        }

        if(!::CreateDirectory(full_path_str, NULL))
        {
            DWORD error_code = ::GetLastError();
            if(error_code==ERROR_ALREADY_EXISTS && DirectoryExists(full_path))
            {
                // This error code ERROR_ALREADY_EXISTS doesn't indicate whether we
                // were racing with someone creating the same directory, or a file
                // with the same path.  If DirectoryExists() returns true, we lost the
                // race to create the same directory.
                return true;
            }
            else
            {
                LOG(WARNING) << "Failed to create directory " << full_path_str
                    << ", last error is " << error_code << ".";
                return false;
            }
        }
        else
        {
            return true;
        }
    }

    bool IsDot(const FilePath& path)
    {
        return FILE_PATH_LITERAL(".") == path.BaseName().value();
    }

    bool IsDotDot(const FilePath& path)
    {
        return FILE_PATH_LITERAL("..") == path.BaseName().value();
    }


    FileEnumerator::FileEnumerator(const FilePath& root_path,
        bool recursive,
        FileType file_type)
        : recursive_(recursive),
        file_type_(file_type),
        has_find_data_(false),
        find_handle_(INVALID_HANDLE_VALUE)
    {
        // INCLUDE_DOT_DOT must not be specified if recursive.
        DCHECK(!(recursive && (INCLUDE_DOT_DOT&file_type_)));
        pending_paths_.push(root_path);
    }

    FileEnumerator::FileEnumerator(const FilePath& root_path,
        bool recursive,
        FileType file_type,
        const FilePath::StringType& pattern)
        : recursive_(recursive),
        file_type_(file_type),
        has_find_data_(false),
        pattern_(pattern),
        find_handle_(INVALID_HANDLE_VALUE)
    {
        // INCLUDE_DOT_DOT must not be specified if recursive.
        DCHECK(!(recursive && (INCLUDE_DOT_DOT&file_type_)));
        pending_paths_.push(root_path);
    }

    FileEnumerator::~FileEnumerator()
    {
        if(find_handle_ != INVALID_HANDLE_VALUE)
        {
            FindClose(find_handle_);
        }
    }

    void FileEnumerator::GetFindInfo(FindInfo* info)
    {
        DCHECK(info);

        if(!has_find_data_)
        {
            return;
        }

        memcpy(info, &find_data_, sizeof(*info));
    }

    bool FileEnumerator::IsDirectory(const FindInfo& info)
    {
        return (info.dwFileAttributes&FILE_ATTRIBUTE_DIRECTORY) != 0;
    }

    // static
    FilePath FileEnumerator::GetFilename(const FindInfo& find_info)
    {
        return FilePath(find_info.cFileName);
    }

    // static
    int64 FileEnumerator::GetFilesize(const FindInfo& find_info)
    {
        ULARGE_INTEGER size;
        size.HighPart = find_info.nFileSizeHigh;
        size.LowPart = find_info.nFileSizeLow;
        DCHECK_LE(static_cast<int64>(size.QuadPart),
            std::numeric_limits<int64>::max());
        return static_cast<int64>(size.QuadPart);
    }

    // static
    base::Time FileEnumerator::GetLastModifiedTime(const FindInfo& find_info)
    {
        return Time::FromFileTime(find_info.ftLastWriteTime);
    }

    FilePath FileEnumerator::Next()
    {
        ThreadRestrictions::AssertIOAllowed();

        while(has_find_data_ || !pending_paths_.empty())
        {
            if(!has_find_data_)
            {
                // ��һ��FindFirstFile��������, ��ʼ�µĲ���.
                root_path_ = pending_paths_.top();
                pending_paths_.pop();

                // ��ʼ�µĲ���.
                FilePath src = root_path_;

                if(pattern_.empty())
                {
                    src = src.Append(L"*"); // ƥ��ģʽΪ�ձ�ʾƥ������.
                }
                else
                {
                    src = src.Append(pattern_);
                }

                find_handle_ = FindFirstFile(src.value().c_str(), &find_data_);
                has_find_data_ = true;
            }
            else
            {
                // ������һ���ļ�/Ŀ¼.
                if(!FindNextFile(find_handle_, &find_data_))
                {
                    FindClose(find_handle_);
                    find_handle_ = INVALID_HANDLE_VALUE;
                }
            }

            if(INVALID_HANDLE_VALUE == find_handle_)
            {
                has_find_data_ = false;

                // ��ʱ���һ��Ŀ¼������, ����Ҫ���ж�������һ��Ŀ¼������. ƥ��ģʽֻ��
                // ��Ŀ¼������ʱ��Ч, ������������Ŀ¼����������ļ�.
                pattern_ = FilePath::StringType();

                continue;
            }

            FilePath cur_file(find_data_.cFileName);
            if(ShouldSkip(cur_file))
            {
                continue;
            }

            // ��������·��.
            cur_file = root_path_.Append(find_data_.cFileName);

            if(find_data_.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
            {
                if(recursive_)
                {
                    // ���|cur_file|��Ŀ¼, ����Ҫ�ݹ�����, ���Ŀ¼��pending_paths_,
                    // �����ڵ�ǰĿ¼ɨ�����֮�������|cur_file|.
                    pending_paths_.push(cur_file);
                }
                if(file_type_ & FileEnumerator::DIRECTORIES)
                {
                    return cur_file;
                }
            }
            else if(file_type_ & FileEnumerator::FILES)
            {
                return cur_file;
            }
        }

        return FilePath();
    }

    bool FileEnumerator::ShouldSkip(const FilePath& path)
    {
        FilePath::StringType basename = path.BaseName().value();
        return IsDot(path) || (IsDotDot(path) && !(INCLUDE_DOT_DOT&file_type_));
    }


    MemoryMappedFile::MemoryMappedFile()
        : file_(INVALID_HANDLE_VALUE),
        file_mapping_(INVALID_HANDLE_VALUE),
        data_(NULL),
        length_(INVALID_FILE_SIZE) {}

    MemoryMappedFile::~MemoryMappedFile()
    {
        CloseHandles();
    }

    bool MemoryMappedFile::Initialize(PlatformFile file)
    {
        if(IsValid())
        {
            return false;
        }

        file_ = file;

        if(!MapFileToMemoryInternal())
        {
            CloseHandles();
            return false;
        }

        return true;
    }

    bool MemoryMappedFile::Initialize(const FilePath& file_name)
    {
        if(IsValid())
        {
            return false;
        }

        if(!MapFileToMemory(file_name))
        {
            CloseHandles();
            return false;
        }

        return true;
    }

    bool MemoryMappedFile::IsValid()
    {
        return data_ != NULL;
    }

    bool MemoryMappedFile::MapFileToMemory(const FilePath& file_name)
    {
        file_ = CreatePlatformFile(file_name,
            PLATFORM_FILE_OPEN|PLATFORM_FILE_READ, NULL, NULL);

        if(file_ == kInvalidPlatformFileValue)
        {
            LOG(ERROR) << "Couldn't open " << file_name.value();
            return false;
        }

        return MapFileToMemoryInternal();
    }

    bool MemoryMappedFile::MapFileToMemoryInternal()
    {
        return MapFileToMemoryInternalEx(0);
    }

    bool MemoryMappedFile::MapFileToMemoryInternalEx(int flags)
    {
        ThreadRestrictions::AssertIOAllowed();

        if(file_ == INVALID_HANDLE_VALUE)
        {
            return false;
        }

        length_ = ::GetFileSize(file_, NULL);
        if(length_ == INVALID_FILE_SIZE)
        {
            return false;
        }

        file_mapping_ = ::CreateFileMapping(file_, NULL, PAGE_READONLY|flags,
            0, 0, NULL);
        if(!file_mapping_)
        {
            // According to msdn, system error codes are only reserved up to 15999.
            // http://msdn.microsoft.com/en-us/library/ms681381(v=VS.85).aspx.
            UMA_HISTOGRAM_ENUMERATION("MemoryMappedFile.CreateFileMapping",
                GetLastSystemErrorCode(), 16000);
            return false;
        }

        data_ = static_cast<uint8*>(::MapViewOfFile(file_mapping_,
            FILE_MAP_READ, 0, 0, 0));
        if(!data_)
        {
            UMA_HISTOGRAM_ENUMERATION("MemoryMappedFile.MapViewOfFile",
                GetLastSystemErrorCode(), 16000);
        }
        return data_ != NULL;
    }

    void MemoryMappedFile::CloseHandles()
    {
        if(data_)
        {
            ::UnmapViewOfFile(data_);
        }
        if(file_mapping_ != INVALID_HANDLE_VALUE)
        {
            ::CloseHandle(file_mapping_);
        }
        if(file_ != INVALID_HANDLE_VALUE)
        {
            ::CloseHandle(file_);
        }

        data_ = NULL;
        file_mapping_ = file_ = INVALID_HANDLE_VALUE;
        length_ = INVALID_FILE_SIZE;
    }

} //namespace base