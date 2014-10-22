
#ifndef __base_scoped_handle_h__
#define __base_scoped_handle_h__

#pragma once

#include <windows.h>

#include "base/logging.h"

namespace base
{
    namespace win
    {

        // ���õĹ�����, �������ǹر�.
        // ��Ľӿں�ScopedStdioHandleһ��, ����һ��IsValid()����, ��ΪWindows
        // �ϵķǷ����������NULL����INVALID_HANDLE_VALUE(-1).
        //
        // ʾ��:
        //     ScopedHandle hfile(CreateFile(...));
        //     if(!hfile.Get())
        //       ...process error
        //     ReadFile(hfile.Get(), ...);
        //
        // ת�ƾ��������Ȩ:
        //     secret_handle_ = hfile.Take();
        //
        // ��ʽ�Ĺرվ��:
        //     hfile.Close();
        class ScopedHandle
        {
        public:
            ScopedHandle() : handle_(NULL) {}

            explicit ScopedHandle(HANDLE h) : handle_(NULL)
            {
                Set(h);
            }

            ~ScopedHandle()
            {
                Close();
            }

            // �÷������Դ�������INVALID_HANDLE_VALUE�ıȽ�, ��Ϊ������NULL
            // ��ʾ����.
            bool IsValid() const
            {
                return handle_ != NULL;
            }

            void Set(HANDLE new_handle)
            {
                Close();

                // Windows�ķǷ������ʾ�����ǲ�һ�µ�, ����ͳһʹ��NULL.
                if(new_handle != INVALID_HANDLE_VALUE)
                {
                    handle_ = new_handle;
                }
            }

            HANDLE Get()
            {
                return handle_;
            }

            operator HANDLE() { return handle_; }

            HANDLE Take()
            {
                // ת������Ȩ.
                HANDLE h = handle_;
                handle_ = NULL;
                return h;
            }

            void Close()
            {
                if(handle_)
                {
                    if(!CloseHandle(handle_))
                    {
                        NOTREACHED();
                    }
                    handle_ = NULL;
                }
            }

        private:
            HANDLE handle_;
            DISALLOW_COPY_AND_ASSIGN(ScopedHandle);
        };

    } //namespace win
} //namespace base

#endif //__base_scoped_handle_h__