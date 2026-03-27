#include <gtest/gtest.h>

#include "src/transform/SerdeUtils.h"

namespace alibabacloud {
namespace oss2 {
namespace transform {

TEST(SerdeUtilsTest, ToXmlText) {
    std::string val = "val-123";
    EXPECT_EQ("<string>val-123</string>", toXmlText(val, "string"));

    EXPECT_EQ("<int32>123</int32>", toXmlText(123, "int32"));

    EXPECT_EQ("<int64>1234</int64>", toXmlText((std::int64_t) 1234LL, "int64"));

    EXPECT_EQ("<bool>true</bool>", toXmlText(true, "bool"));
    EXPECT_EQ("<bool>false</bool>", toXmlText(false, "bool"));

    EXPECT_EQ("<double>1.200000</double>", toXmlText(1.2, "double"));
}


TEST(SerdeUtilsTest, ToBool) {
    thirdparty::tinyxml2::XMLDocument doc;
    auto elem = doc.NewElement("bool");

    EXPECT_EQ(false, toBool(elem));

    elem->SetText("true");
    EXPECT_EQ(true, toBool(elem));

    elem->SetText("True");
    EXPECT_EQ(false, toBool(elem));

    elem->SetText("false");
    EXPECT_EQ(false, toBool(elem));

    elem->SetText("False");
    EXPECT_EQ(false, toBool(elem));

    elem->SetText("invalid");
    EXPECT_EQ(false, toBool(elem));

    doc.DeleteNode(elem);
}

TEST(SerdeUtilsTest, ToInt32) {
    thirdparty::tinyxml2::XMLDocument doc;
    auto elem = doc.NewElement("int32");

    EXPECT_EQ(0, toInt32(elem));

    elem->SetText("0");
    EXPECT_EQ(0, toInt32(elem));

    elem->SetText("-10");
    EXPECT_EQ(-10, toInt32(elem));

    elem->SetText("100");
    EXPECT_EQ(100, toInt32(elem));

    // invalid
    elem->SetText("inavlid");
    EXPECT_EQ(0, toInt32(elem));
    elem->SetText("");
    EXPECT_EQ(0, toInt32(elem));
    elem->SetText("11111111111111111");
    EXPECT_EQ(INT32_MAX, toInt32(elem));

    doc.DeleteNode(elem);
}

TEST(SerdeUtilsTest, ToInt64) {
    thirdparty::tinyxml2::XMLDocument doc;
    auto elem = doc.NewElement("int64");
    EXPECT_EQ(0, toInt64(elem));

    elem->SetText("0");
    EXPECT_EQ(0, toInt64(elem));

    elem->SetText("-10");
    EXPECT_EQ(-10, toInt64(elem));

    elem->SetText("100");
    EXPECT_EQ(100, toInt64(elem));

    // invalid
    elem->SetText("inavlid");
    EXPECT_EQ(0, toInt64(elem));
    elem->SetText("");
    EXPECT_EQ(0, toInt64(elem));
    elem->SetText("11111111111111111");
    EXPECT_EQ(11111111111111111, toInt64(elem));

    doc.DeleteNode(elem);
}

TEST(SerdeUtilsTest, ToDouble) {
    thirdparty::tinyxml2::XMLDocument doc;
    auto elem = doc.NewElement("double");
    EXPECT_EQ(0.0, toDouble(elem));


    elem->SetText("0");
    EXPECT_EQ(0.0, toDouble(elem));

    elem->SetText("-10");
    EXPECT_EQ(-10.0, toDouble(elem));

    elem->SetText("100");
    EXPECT_EQ(100.0, toDouble(elem));

    elem->SetText("100.123");
    EXPECT_EQ(100.123, toDouble(elem));

    elem->SetText("11111111111111111");
    EXPECT_EQ(11111111111111111, toDouble(elem));

    // invalid
    elem->SetText("inavlid");
    EXPECT_EQ(0.0, toDouble(elem));
    elem->SetText("");
    EXPECT_EQ(0.0, toDouble(elem));

    doc.DeleteNode(elem);
}

TEST(SerdeUtilsTest, ToString) {
    thirdparty::tinyxml2::XMLDocument doc;
    auto elem = doc.NewElement("string");

    EXPECT_EQ("", toString(elem));

    elem->SetText("0");
    EXPECT_EQ("0", toString(elem));

    doc.DeleteNode(elem);
}

TEST(SerdeUtilsTest, ToString_url) {
    thirdparty::tinyxml2::XMLDocument doc;
    auto elem = doc.NewElement("string");

    EXPECT_EQ("", toString(elem, true));
    EXPECT_EQ("", toString(elem, false));

    elem->SetText("0");
    EXPECT_EQ("0", toString(elem, true));
    EXPECT_EQ("0", toString(elem, false));


    elem->SetText("%2F");
    EXPECT_EQ("/", toString(elem, true));
    EXPECT_EQ("%2F", toString(elem, false));


    doc.DeleteNode(elem);
}


} // namespace transform
} // namespace oss2
} // namespace alibabacloud