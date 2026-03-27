#include <gtest/gtest.h>

#include "src/utils/Utils.h"

namespace alibabacloud {

namespace oss2 {

namespace utils {

TEST(XmlUtilsTest, XmlEscapeTest) {
    EXPECT_EQ(XmlEscape(""), "");
    EXPECT_EQ(XmlEscape("\"<"), "&quot;&lt;");
    EXPECT_EQ(XmlEscape("abdeef"), "abdeef");
    EXPECT_EQ(XmlEscape(">abc<efj\"hij\'lmn&"), "&gt;abc&lt;efj&quot;hij&apos;lmn&amp;");

    std::string str;
    str.push_back((char) 0xE6);
    str.push_back((char) 0xB5);
    str.push_back((char) 0x8B);
    str.push_back((char) 0xE8);
    str.push_back((char) 0xAF);
    str.push_back((char) 0x95);
    EXPECT_EQ(XmlEscape(str), str);
}

} // namespace utils
} // namespace oss2
} // namespace alibabacloud