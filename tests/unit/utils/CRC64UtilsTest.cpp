#include <gtest/gtest.h>

#include "src/utils/Utils.h"

namespace alibabacloud {

namespace oss2 {

namespace utils {

TEST(CRC64Test, CalcCRCTest) {
    std::string data1("123456789");
    uint64_t crc1_pat = UINT64_C(0x995dc9bbdf1939fa);
    uint64_t crc1 = CalcCRC64(0, (void*) (data1.c_str()), data1.size());
    EXPECT_EQ(crc1, crc1_pat);


    std::string data2("This is a test of the emergency broadcast system.");
    uint64_t crc2_pat = UINT64_C(0x27db187fc15bbc72);
    uint64_t crc2 = CalcCRC64(0, (void*) (data2.c_str()), data2.size());
    EXPECT_EQ(crc2, crc2_pat);
}

TEST(CRC64Test, CalcCRCWithEndingFlagTest) {
    std::string data1("123456789");
    std::string data2("This is a test of the emergency broadcast system.");

    // little
    uint64_t crc1_pat = UINT64_C(0x995dc9bbdf1939fa);
    uint64_t crc1 = CalcCRC64(0, (void*) (data1.c_str()), data1.size(), true);
    EXPECT_EQ(crc1, crc1_pat);

    uint64_t crc2_pat = UINT64_C(0x27db187fc15bbc72);
    uint64_t crc2 = CalcCRC64(0, (void*) (data2.c_str()), data2.size(), true);
    EXPECT_EQ(crc2, crc2_pat);

    // big
    const char* str1 = "12345678";
    const char* str2 = "87654321";
    crc1 = CalcCRC64(0, (void*) str1, 8, false);
    crc2 = CalcCRC64(0, (void*) str2, 8, true);
}


TEST(CRC64Test, CombineCRCTest) {
    std::string data1("123456789");
    uint64_t crc1_pat = UINT64_C(0x995dc9bbdf1939fa);
    uint64_t crc1 = CalcCRC64(0, (void*) (data1.c_str()), data1.size());
    EXPECT_EQ(crc1, crc1_pat);

    std::string data2("This is a test of the emergency broadcast system.");
    uint64_t crc2_pat = UINT64_C(0x27db187fc15bbc72);
    uint64_t crc2 = CalcCRC64(0, (void*) (data2.c_str()), data2.size());
    EXPECT_EQ(crc2, crc2_pat);

    std::string data3;
    data3.append(data1).append(data2);
    uint64_t crc3 = CalcCRC64(0, (void*) (data3.c_str()), data3.size());
    uint64_t crc4 = CombineCRC64(crc1, crc2, data2.size());
    EXPECT_EQ(crc3, crc4);

    uint64_t crc5 = CombineCRC64(crc1, crc2, data2.size());
    EXPECT_EQ(crc3, crc5);
}
} // namespace utils
} // namespace oss2
} // namespace alibabacloud