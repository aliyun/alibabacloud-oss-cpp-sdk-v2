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

TEST(XmlUtilsTest, XmlEscapeKeyTest) {
    // empty
    EXPECT_EQ(XmlEscapeKey(""), "");

    // plain ascii passes through
    EXPECT_EQ(XmlEscapeKey("abc/def.txt"), "abc/def.txt");

    // standard entities use the Go-style numeric forms for quote/apostrophe
    EXPECT_EQ(XmlEscapeKey(">abc<efj\"hij\'lmn&"), "&gt;abc&lt;efj&#34;hij&#39;lmn&amp;");

    // legal whitespace becomes numeric character references
    EXPECT_EQ(XmlEscapeKey("a\tb\nc\rd"), "a&#x9;b&#xA;c&#xD;d");

    // control characters outside the XML 1.0 set become &#xNN;
    {
        std::string in;
        in.push_back((char) 0x01);
        in.push_back('k');
        in.push_back((char) 0x1F);
        in.push_back((char) 0x0B);
        EXPECT_EQ(XmlEscapeKey(in), "&#x01;k&#x1F;&#x0B;");
    }

    // valid multibyte utf-8 passes through unchanged
    {
        std::string in;
        in.push_back((char) 0xE6);
        in.push_back((char) 0xB5);
        in.push_back((char) 0x8B);
        EXPECT_EQ(XmlEscapeKey(in), in);
    }

    // invalid utf-8 byte is replaced with U+FFFD (EF BF BD)
    {
        std::string in;
        in.push_back('a');
        in.push_back((char) 0xFF);
        in.push_back('b');
        std::string want = "a";
        want.push_back((char) 0xEF);
        want.push_back((char) 0xBF);
        want.push_back((char) 0xBD);
        want.push_back('b');
        EXPECT_EQ(XmlEscapeKey(in), want);
    }
}

} // namespace utils
} // namespace oss2
} // namespace alibabacloud