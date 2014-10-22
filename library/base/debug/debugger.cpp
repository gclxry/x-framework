
#include "debugger.h"

#include "base/threading/platform_thread.h"

namespace
{

    // ��С�Ķ�ע�����.
    // ע��: ûʹ��CRT.
    bool RegReadString(HKEY root, const wchar_t* subkey, const wchar_t* value_name,
        wchar_t* buffer, int* len)
    {
        HKEY key = NULL;
        DWORD res = RegOpenKeyEx(root, subkey, 0, KEY_READ, &key);
        if(ERROR_SUCCESS!=res || key==NULL)
        {
            return false;
        }

        DWORD type = 0;
        DWORD buffer_size = *len * sizeof(wchar_t);
        // ��֧��REG_EXPAND_SZ
        res = RegQueryValueEx(key, value_name, NULL, &type,
            reinterpret_cast<BYTE*>(buffer), &buffer_size);
        if(ERROR_SUCCESS==res && buffer_size!=0 && type==REG_SZ)
        {
            // ȷ��buffer��NULL������.
            buffer[*len-1] = 0;
            *len = lstrlenW(buffer);
            RegCloseKey(key);
            return true;
        }
        RegCloseKey(key);
        return false;
    }

    // �滻input�е�ÿ��"%ld", ����Ч������������.
    // ע��: ûʹ��CRT.
    bool StringReplace(const wchar_t* input, int value, wchar_t* output,
        int output_len)
    {
        memset(output, 0, output_len*sizeof(wchar_t));
        int input_len = lstrlenW(input);

        for(int i=0; i<input_len; ++i)
        {
            int current_output_len = lstrlenW(output);

            if(input[i]==L'%' && input[i+1]==L'l' && input[i+2]==L'd')
            {
                // ȷ�����㹻��ʣ��ռ�.
                if((current_output_len+12) >= output_len)
                {
                    return false;
                }

                // ���۵�_itow().
                wsprintf(output+current_output_len, L"%d", value);
                i += 2;
            }
            else
            {
                if(current_output_len >= output_len)
                {
                    return false;
                }
                output[current_output_len] = input[i];
            }
        }
        return true;
    }

}

namespace base
{
    namespace debug
    {

        static bool is_debug_ui_suppressed = false;

        // ע��: û��ʹ��CRT.
        bool SpawnDebuggerOnProcess(unsigned process_id)
        {
            wchar_t reg_value[1026];
            int len = arraysize(reg_value);
            if(RegReadString(HKEY_LOCAL_MACHINE,
                L"SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\AeDebug",
                L"Debugger", reg_value, &len))
            {
                wchar_t command_line[1026];
                if(StringReplace(reg_value, process_id, command_line,
                    arraysize(command_line)))
                {
                    // ����������Ҳû��ϵ, ֻ�Ǹ��ӵ��ý��̻�ʧ��.
                    STARTUPINFO startup_info = { 0 };
                    startup_info.cb = sizeof(startup_info);
                    PROCESS_INFORMATION process_info = { 0 };

                    if(CreateProcess(NULL, command_line, NULL, NULL, FALSE, 0, NULL, NULL,
                        &startup_info, &process_info))
                    {
                        CloseHandle(process_info.hThread);
                        WaitForInputIdle(process_info.hProcess, 10000);
                        CloseHandle(process_info.hProcess);
                        return true;
                    }
                }
            }
            return false;
        }

        bool WaitForDebugger(int wait_seconds, bool silent)
        {
            for(int i=0; i<wait_seconds*10; ++i)
            {
                if(BeingDebugged())
                {
                    if(!silent)
                    {
                        BreakDebugger();
                    }
                    return true;
                }
                PlatformThread::Sleep(100);
            }
            return false;
        }

        bool BeingDebugged()
        {
            return ::IsDebuggerPresent() != 0;
        }

        void BreakDebugger()
        {
            if(IsDebugUISuppressed())
            {
                _exit(1);
            }

            __debugbreak();
#if defined(NDEBUG)
            _exit(1);
#endif
        }

        void SetSuppressDebugUI(bool suppress)
        {
            is_debug_ui_suppressed = suppress;
        }

        bool IsDebugUISuppressed()
        {
            return is_debug_ui_suppressed;
        }

    } //namespace debug
} //namespace base