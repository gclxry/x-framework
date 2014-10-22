
#ifndef __net_base_sdk_h__
#define __net_base_sdk_h__

#ifdef NET_BASE_SDK_EXPORT
#define NET_BASE_PUBLIC __declspec(dllexport)
#else
#define NET_BASE_PUBLIC __declspec(dllimport)
#endif

// ��������


// ��������ӿ�, ���з��������̰߳�ȫ, �����ڵ���StartDownloadEngine
// ���������߳�ִ��.
class IDownloadEngine
{
public:
    // �ر���������, ��������.
    virtual void Shutdown() = 0;
    // �ر���������, ��������Ч.
    virtual void Stop() = 0;

    virtual void AddTask(__int64 id, const wchar_t* const* urls,
        int urls_count) = 0;
};

// ��������, �ɹ����������������, ʧ�ܷ���NULL.
NET_BASE_PUBLIC IDownloadEngine* StartDownloadEngine();

#endif //__net_base_sdk_h__