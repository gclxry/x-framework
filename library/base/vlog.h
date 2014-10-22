
#ifndef __base_vlog_h__
#define __base_vlog_h__

#pragma once

#include <vector>

#include "basic_types.h"
#include "string_piece.h"

namespace base
{

    // ������־��¼�������õĸ�����.
    class VlogInfo
    {
    public:
        static const int kDefaultVlogLevel;

        // |v_switch|ָ��ȱʡ����󼤻���־��¼�ȼ�, Ĭ����0, һ��ʹ������.
        //
        // |vmodule_switch|����ÿ��ģ�����󼤻���־��¼�ȼ�, ����|v_switch|
        // �ṩ��ֵ.
        // ����: "my_module=2,foo*=3"���ı�����"my_module.*"��"foo*.*"(
        // ƥ��ʱ"-inl"��׺�ᱻԤ�Ⱥ��Ե�)Դ�ļ��д������־�ȼ�.
        //
        // |min_log_level|ָ��intָ��洢��־��¼�ȼ�. ����������Ϸ���
        // |v_switch|, �����ø�ֵ, ȱʡ����־��¼�ȼ��������ȡ.
        //
        // ����\����/��ģʽ��ƥ������·��������һ��ģ��. ����
        // "*/foo/bar/*=2"��ı�"foo/bar"Ŀ¼������Դ�ļ��д������־�ȼ�.
        VlogInfo(const std::string& v_switch,
            const std::string& vmodule_switch,
            int* min_log_level);
        ~VlogInfo();

        // ����ָ���ļ���vlog�ȼ�(ͨ���ǵ�ǰ�ļ�__FILE__).
        int GetVlogLevel(const base::StringPiece& file);

    private:
        void SetMaxVlogLevel(int level);
        int GetMaxVlogLevel() const;

        // VmodulePattern�洢��|vmodule_switch|������������ģʽ��ƥ����Ϣ.
        struct VmodulePattern;
        std::vector<VmodulePattern> vmodule_levels_;
        int* min_log_level_;

        DISALLOW_COPY_AND_ASSIGN(VlogInfo);
    };

    // ������ݵ��ַ�����vlogģʽƥ�䷵��true. vlogģʽ�����԰���*��?ͨ���.
    // ?��ȷƥ��һ���ַ�, *ƥ��0������ַ�. /��\�ַ���ƥ��/��\.
    //
    // ����:
    //   "kh?n"ƥ��"khan"  ��ƥ��"khn"��"khaan".
    //   "kh*n"ƥ��"khn", "khan", "khaaaaan".
    //   "/foo\bar"ƥ��"/foo/bar", "\foo\bar", "/foo\bar"(����C��ת�ƹ���).
    bool MatchVlogPattern(const base::StringPiece& string,
        const base::StringPiece& vlog_pattern);

} //namespace base

#endif //__base_vlog_h__