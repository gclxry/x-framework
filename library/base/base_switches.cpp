
#include "base_switches.h"

namespace base
{

    // ָ��ȱʡ����󼤻���־��¼�ȼ�, Ĭ����0, һ��ʹ������.
    const char kV[]                             = "v";

    // ����ÿ��ģ�����󼤻���־��¼�ȼ�, ����--v�ṩ��ֵ. ����:
    // "my_module=2,foo*=3"���ı�����"my_module.*"��"foo*.*"(
    // ƥ��ʱ"-inl"��׺�ᱻԤ�Ⱥ��Ե�)Դ�ļ��д������־�ȼ�.
    //
    // ����\����/��ģʽ��ƥ������·��������һ��ģ��. ����
    // "*/foo/bar/*=2"��ı�"foo/bar"Ŀ¼������Դ�ļ��д���
    // ����־�ȼ�.
    const char kVModule[]                       = "vmodule";

} //namespace base