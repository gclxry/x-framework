
#include "process.h"

#include "logging.h"
#include "process_util.h"
#include "win/windows_version.h"

namespace base
{

    void Process::Close()
    {
        if(!process_)
        {
            return;
        }
        // Don't call CloseHandle on a pseudo-handle.
        if(process_ != ::GetCurrentProcess())
        {
            // TODO(apatrick): Call NtCloseHandle directly, without going through the
            // import table to determine if CloseHandle is being hooked.
            // http://crbug.com/81449.
            HMODULE module = GetModuleHandle(L"ntdll.dll");
            typedef UINT (WINAPI *CloseHandlePtr)(HANDLE handle);
            CloseHandlePtr close_handle = reinterpret_cast<CloseHandlePtr>(
                GetProcAddress(module, "NtClose"));
            close_handle(process_);
        }
        process_ = NULL;
    }

    void Process::Terminate(int result_code)
    {
        if(!process_)
        {
            return;
        }
        // TODO(apatrick): Call NtTerminateProcess directly, without going through the
        // import table to determine if TerminateProcess is being hooked.
        // http://crbug.com/81449.
        HMODULE module = GetModuleHandle(L"ntdll.dll");
        typedef UINT (WINAPI *TerminateProcessPtr)(HANDLE handle, UINT code);
        TerminateProcessPtr terminate_process = reinterpret_cast<TerminateProcessPtr>(
            GetProcAddress(module, "NtTerminateProcess"));
        terminate_process(process_, result_code);
    }

    bool Process::IsProcessBackgrounded() const
    {
        if(!process_)
        {
            return false; // Failure case.
        }
        DWORD priority = GetPriority();
        if(priority == 0)
        {
            return false; // Failure case.
        }
        return ((priority==BELOW_NORMAL_PRIORITY_CLASS) ||
            (priority==IDLE_PRIORITY_CLASS));
    }

    bool Process::SetProcessBackgrounded(bool value)
    {
        if(!process_)
        {
            return false;
        }
        // Vista and above introduce a real background mode, which not only
        // sets the priority class on the threads but also on the IO generated
        // by it. Unfortunately it can only be set for the calling process.
        DWORD priority;
        if((win::GetVersion()>=win::VERSION_VISTA) &&
            (process_==::GetCurrentProcess()))
        {
            priority = value ? PROCESS_MODE_BACKGROUND_BEGIN :
                PROCESS_MODE_BACKGROUND_END;
        }
        else
        {
            priority = value ? BELOW_NORMAL_PRIORITY_CLASS : NORMAL_PRIORITY_CLASS;
        }

        return (::SetPriorityClass(process_, priority) != 0);
    }

    ProcessId Process::pid() const
    {
        if(process_ == 0)
        {
            return 0;
        }

        return GetProcId(process_);
    }

    bool Process::is_current() const
    {
        return process_ == GetCurrentProcess();
    }

    // static
    Process Process::Current()
    {
        return Process(GetCurrentProcess());
    }

    int Process::GetPriority() const
    {
        DCHECK(process_);
        return GetPriorityClass(process_);
    }

} //namespace base