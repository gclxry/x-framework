
#include "rand_util.h"

#include <limits>
#include <stdlib.h>

#include "algorithm/base64/base64.h"
#include "logging.h"
#include "string_util.h"

namespace base
{

    uint32 RandUint32()
    {
        uint32 number;
        CHECK_EQ(rand_s(&number), 0);
        return number;
    }

    uint64 RandUint64()
    {
        uint32 first_half = RandUint32();
        uint32 second_half = RandUint32();
        return (static_cast<uint64>(first_half)<<32)+second_half;
    }

    int RandInt(int min, int max)
    {
        DCHECK(min <= max);

        uint64 range = static_cast<uint64>(max) - min + 1;
        int result = min + static_cast<int>(RandGenerator(range));
        DCHECK_GE(result, min);
        DCHECK_LE(result, max);
        return result;
    }

    uint64 RandGenerator(uint64 range)
    {
        DCHECK_GT(range, 0u);

        // We must discard random results above this number, as they would
        // make the random generator non-uniform (consider e.g. if
        // MAX_UINT64 was 7 and |range| was 5, then a result of 1 would be twice
        // as likely as a result of 3 or 4).
        uint64 max_acceptable_value =
            (std::numeric_limits<uint64>::max() / range) * range - 1;

        uint64 value;
        do
        {
            value = base::RandUint64();
        } while(value >= max_acceptable_value);

        return value % range;
    }

    double RandDouble()
    {
        return BitsToOpenEndedUnitInterval(RandUint64());
    }

    double BitsToOpenEndedUnitInterval(uint64 bits)
    {
        // Ϊ����߾���, ����λ�������������, Ȼ������ʵ���������õ�[0, 1)����
        // ��double��ֵ. IEEE 754����Ҫ����53λ.

        COMPILE_ASSERT(std::numeric_limits<double>::radix==2, otherwise_use_scalbn);
        static const int kBits = std::numeric_limits<double>::digits;
        uint64 random_bits = bits & ((GG_UINT64_C(1)<<kBits) - 1);
        double result = ldexp(static_cast<double>(random_bits), -1*kBits);
        DCHECK_GE(result, 0.0);
        DCHECK_LT(result, 1.0);
        return result;
    }

    void RandBytes(void* output, size_t output_length)
    {
        uint64 random_int;
        size_t random_int_size = sizeof(random_int);
        for(size_t i=0; i<output_length; i+=random_int_size)
        {
            random_int = RandUint64();
            size_t copy_count = std::min(output_length-i, random_int_size);
            memcpy(((uint8*)output)+i, &random_int, copy_count);
        }
    }

    std::string RandBytesAsString(size_t length)
    {
        std::string result;
        RandBytes(WriteInto(&result, length + 1), length);
        return result;
    }

    std::string Generate128BitRandomBase64String()
    {
        const int kNumberBytes = 128 / 8;
        std::string base64_encoded_bytes;
        base::Base64Encode(base::RandBytesAsString(kNumberBytes),
            &base64_encoded_bytes);
        return base64_encoded_bytes;
    }

} //namespace base