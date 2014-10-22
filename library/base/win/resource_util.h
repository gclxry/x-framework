
#ifndef __base_resource_util_h__
#define __base_resource_util_h__

#pragma once

#include <windows.h>

namespace base
{
    namespace win
    {

        // ��dll�л�ȡ������Դ.
        bool GetDataResourceFromModule(HMODULE module, int resource_id,
            void** data, size_t* length);
    
    } //namespace win
} //namespace base

#endif //__base_resource_util_h__