
#ifndef __base_algorithm_sha1_h__
#define __base_algorithm_sha1_h__

#pragma once

#include <string>

namespace base
{

    enum
    {
        SHA1_LENGTH = 20 // SHA-1�Ĺ�ϣֵ����.
    };

    // ���������ַ���|str|��SHA-1��ϣ�����ع�ϣֵ.
    std::string SHA1HashString(const std::string& str);

    // ����|data|��������|len|�ֽڳ��ȵ�SHA-1��ϣ�洢��|hash|��.
    // |hash|���ȱ�������SHA1_LENGTH�ֽڳ���.
    void SHA1HashBytes(const unsigned char* data, size_t len, unsigned char* hash);

} //namespace base

#endif //__base_algorithm_sha1_h__