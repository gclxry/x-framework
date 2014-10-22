
#include "platform_canvas.h"

#include <windows.h>
#include <psapi.h>

#include "bitmap_platform_device_win.h"

#pragma comment(lib, "psapi.lib")

namespace skia
{

    // ��������ʱ��ֹ�Ż�.
#pragma optimize("", off)

    // ʧ��ʱ����.
#define CHECK(condition) if(!(condition)) __debugbreak();

    // ʹ���̱���. ��λͼ����ʧ��ʱ����, ��������ȷ��ʧ�ܵ�ԭ��, �����ʵ��ĵط�����
    // ����. �������ǿ��Ը���������±�����ԭ��. ������Ҫ����λͼ�Ŀ��, ����������
    // �Ŵ���λͼ��λ����.
    //
    // ע����ɳ����Ⱦ����, ��������GetProcessMemoryInfo()�ᵼ�±���, ��Ϊ��ʱ�����
    // load psapi.dll, ��ô������ȷ��, �����ֱ����п�������������ͷ��.
    void CrashForBitmapAllocationFailure(int w, int h)
    {
        // �洢��չ�Ĵ�����Ϣ��һ������ʱ�����ҵ��ĵط�.
        struct
        {
            unsigned int last_error;
            unsigned int diag_error;
        } extended_error_info;
        extended_error_info.last_error = GetLastError();
        extended_error_info.diag_error = 0;

        // ���λͼ̫��, �п��ܷ���ʧ��.
        // ʹ��64M����=256MB, ÿ������4�ֽ������.
        const __int64 kGinormousBitmapPxl = 64000000;
        CHECK(static_cast<__int64>(w) * static_cast<__int64>(h) <
            kGinormousBitmapPxl);

        // ÿ������GDI�����������10K. ����ӽ����ֵ, �ܿ��ܳ�������.
        const unsigned int kLotsOfGDIObjects = 9990;
        unsigned int num_gdi_objects = GetGuiResources(GetCurrentProcess(),
            GR_GDIOBJECTS);
        if(num_gdi_objects == 0)
        {
            extended_error_info.diag_error = GetLastError();
            CHECK(0);
        }
        CHECK(num_gdi_objects < kLotsOfGDIObjects);

        // �������ʹ�������ַ�ռ�, ����λͼ��Ҫ���ڴ���ܲ���.
        const SIZE_T kLotsOfMem = 1500000000; // 1.5GB.
        PROCESS_MEMORY_COUNTERS pmc;
        if(!GetProcessMemoryInfo(GetCurrentProcess(), &pmc, sizeof(pmc)))
        {
            extended_error_info.diag_error = GetLastError();
            CHECK(0);
        }
        CHECK(pmc.PagefileUsage < kLotsOfMem);

        // ����������ȫ�����������.
        CHECK(0);
    }

    // ʹ���̱���. ��CrashForBitmapAllocationFailure()��ͬ, ��λͼ����ʧ��ʱ����
    // ����ǲ�����Ч�Ĺ���λͼ�������.
    void CrashIfInvalidSection(HANDLE shared_section)
    {
        DWORD handle_info = 0;
        CHECK(GetHandleInformation(shared_section, &handle_info) == TRUE);
    }

    // �ָ��Ż�ѡ��.
#pragma optimize("", on)

    PlatformCanvas::PlatformCanvas(int width, int height, bool is_opaque)
    {
        bool initialized = initialize(width, height, is_opaque, NULL);
        if(!initialized)
        {
            CrashForBitmapAllocationFailure(width, height);
        }
    }

    PlatformCanvas::PlatformCanvas(int width,
        int height,
        bool is_opaque,
        HANDLE shared_section)
    {
        bool initialized = initialize(width, height, is_opaque, shared_section);
        if(!initialized)
        {
            CrashIfInvalidSection(shared_section);
            CrashForBitmapAllocationFailure(width, height);
        }
    }

    PlatformCanvas::~PlatformCanvas() {}

    bool PlatformCanvas::initialize(int width,
        int height,
        bool is_opaque,
        HANDLE shared_section)
    {
        return initializeWithDevice(BitmapPlatformDevice::create(
            width, height, is_opaque, shared_section));
    }

} //namespace skia