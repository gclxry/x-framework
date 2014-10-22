
#ifndef __base_base64_h__
#define __base_base64_h__

#pragma once

#include "base/string_piece.h"

namespace base
{

    // base64����. �ɹ�����trueʧ�ܷ���false. outputֻ�ڳɹ�ʱ���޸�.
    bool Base64Encode(const StringPiece& input, std::string* output);

    // base64����. �ɹ�����trueʧ�ܷ���false. outputֻ�ڳɹ�ʱ���޸�.
    bool Base64Decode(const StringPiece& input, std::string* output);

} //namespace base

#endif //__base_base64_h__