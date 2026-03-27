#include <gtest/gtest.h>

#include "src/internal/Url.h"

namespace alibabacloud {
namespace oss2 {
namespace internal {


TEST(UrlTest, UrlFunctionTest)
{
    Url url1;
    Url url2;
    EXPECT_EQ(url1 == url2, true);
    url1.setHost("test.com");
    EXPECT_EQ(url1 != url2, true);
    url1.fragment();
    url1.authority();
    url1.setPort(1);
    url1.setUserName("test");
    url1.authority();
    url1.fromString("#test");
    std::string str;
    url1.fromString(str);

    url2.setFragment("test");
    EXPECT_EQ(url2.isEmpty(), false);

    Url url3;
    url3.isValid();
    url3.setUserName("test");
    url3.isValid();

    url1.port();
    url1.password();
    url1.path();
    url3.setAuthority(str);
    url3.setAuthority("@test:test");
    url3.setHost(str);
    url3.setPassword("test");
    url3.setUserInfo("test");
    url3.setUserInfo(":test");
    url3.setUserName("test");
    url3.userName();

    url2.toString();
    url2.userInfo();
    url2.setHost("test");
    url2.setUserName("test");
    url2.setPassword("test");
    url2.toString();
    url2.userInfo();
}
}
}
}