
#include "windows_version.h"

#include <windows.h>

namespace base
{
    namespace win
    {

        // static
        OSInfo* OSInfo::GetInstance()
        {
            // Note: we don't use the Singleton class because it depends on AtExitManager,
            // and it's convenient for other modules to use this classs without it. This
            // pattern is copied from gurl.cc.
            static OSInfo* info;
            if(!info)
            {
                OSInfo* new_info = new OSInfo();
                if(InterlockedCompareExchangePointer(
                    reinterpret_cast<PVOID*>(&info), new_info, NULL))
                {
                    delete new_info;
                }
            }
            return info;
        }

        OSInfo::OSInfo() : version_(VERSION_PRE_XP),
            architecture_(OTHER_ARCHITECTURE),
            wow64_status_(GetWOW64StatusForProcess(GetCurrentProcess()))
        {
            OSVERSIONINFOEX version_info = { sizeof(version_info) };
            GetVersionEx(reinterpret_cast<OSVERSIONINFO*>(&version_info));
            version_number_.major = version_info.dwMajorVersion;
            version_number_.minor = version_info.dwMinorVersion;
            version_number_.build = version_info.dwBuildNumber;
            if((version_number_.major==5) && (version_number_.minor>0))
            {
                version_ = (version_number_.minor==1) ? VERSION_XP :
                    VERSION_SERVER_2003;
            }
            else if(version_number_.major == 6)
            {
                if(version_info.wProductType == VER_NT_WORKSTATION)
                {
                    version_ = (version_number_.minor==0) ? VERSION_VISTA :
                        VERSION_WIN7;
                }
                else
                {
                    version_ = VERSION_SERVER_2008;
                }
            }
            else if(version_number_.major > 6)
            {
                version_ = VERSION_WIN7;
            }
            service_pack_.major = version_info.wServicePackMajor;
            service_pack_.minor = version_info.wServicePackMinor;

            SYSTEM_INFO system_info = { 0 };
            GetNativeSystemInfo(&system_info);
            switch(system_info.wProcessorArchitecture)
            {
            case PROCESSOR_ARCHITECTURE_INTEL: architecture_ = X86_ARCHITECTURE; break;
            case PROCESSOR_ARCHITECTURE_AMD64: architecture_ = X64_ARCHITECTURE; break;
            case PROCESSOR_ARCHITECTURE_IA64:  architecture_ = IA64_ARCHITECTURE; break;
            }
            processors_ = system_info.dwNumberOfProcessors;
            allocation_granularity_ = system_info.dwAllocationGranularity;
        }

        OSInfo::~OSInfo() {}

        // static
        OSInfo::WOW64Status OSInfo::GetWOW64StatusForProcess(HANDLE process_handle)
        {
            typedef BOOL (WINAPI* IsWow64ProcessFunc)(HANDLE, PBOOL);
            IsWow64ProcessFunc is_wow64_process = reinterpret_cast<IsWow64ProcessFunc>(
                GetProcAddress(GetModuleHandle(L"kernel32.dll"), "IsWow64Process"));
            if(!is_wow64_process)
            {
                return WOW64_DISABLED;
            }
            BOOL is_wow64 = FALSE;
            if(!(*is_wow64_process)(process_handle, &is_wow64))
            {
                return WOW64_UNKNOWN;
            }
            return is_wow64 ? WOW64_ENABLED : WOW64_DISABLED;
        }

        Version GetVersion()
        {
            return OSInfo::GetInstance()->version();
        }

    } //namespace win
} //namespace base