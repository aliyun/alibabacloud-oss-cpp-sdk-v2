#include <gtest/gtest.h>

#include "alibabacloud/oss2/Types.h"

#include <sstream>

using namespace alibabacloud::oss2;

class TestRequestModel : public RequestModel {
  public:
    using RequestModel::getHeaderOrEmpty;
    using RequestModel::getParameterOrEmpty;
    using RequestModel::getHeaderAsInt32Or;
    using RequestModel::getHeaderAsInt64Or;
    using RequestModel::getHeaderAsBoolOr;
    using RequestModel::getParameterAsInt32Or;
    using RequestModel::getParameterAsInt64Or;
    using RequestModel::getParameterAsBoolOr;
};

TEST(TypesTest, RequestModelGetHeaderOrEmpty) {
    TestRequestModel model;
    model.addHeader("Content-Type", "application/json");
    model.addHeader("x-oss-custom", "custom-value");

    EXPECT_EQ("application/json", model.getHeaderOrEmpty("Content-Type"));
    EXPECT_EQ("custom-value", model.getHeaderOrEmpty("x-oss-custom"));
    EXPECT_EQ("", model.getHeaderOrEmpty("non-existent"));
}

TEST(TypesTest, RequestModelGetParameterOrEmpty) {
    TestRequestModel model;
    model.addParameter("versionId", "v123");
    model.addParameter("acl", "");

    EXPECT_EQ("v123", model.getParameterOrEmpty("versionId"));
    EXPECT_EQ("", model.getParameterOrEmpty("acl"));
    EXPECT_EQ("", model.getParameterOrEmpty("non-existent"));
}

TEST(TypesTest, RequestModelGetHeaderAsInt32Or) {
    TestRequestModel model;
    model.addHeader("Content-Length", "1024");
    model.addHeader("x-oss-invalid", "not-a-number");

    EXPECT_EQ(1024, model.getHeaderAsInt32Or("Content-Length"));
    EXPECT_EQ(-1, model.getHeaderAsInt32Or("non-existent"));
    EXPECT_EQ(999, model.getHeaderAsInt32Or("non-existent", 999));
    EXPECT_EQ(0, model.getHeaderAsInt32Or("x-oss-invalid", 0));
}

TEST(TypesTest, RequestModelGetHeaderAsInt64Or) {
    TestRequestModel model;
    model.addHeader("x-oss-large-number", "9223372036854775807");

    EXPECT_EQ(9223372036854775807LL, model.getHeaderAsInt64Or("x-oss-large-number"));
    EXPECT_EQ(-1LL, model.getHeaderAsInt64Or("non-existent"));
    EXPECT_EQ(100LL, model.getHeaderAsInt64Or("non-existent", 100));
}

TEST(TypesTest, RequestModelGetHeaderAsBoolOr) {
    TestRequestModel model;
    model.addHeader("x-oss-true", "true");
    model.addHeader("x-oss-false", "false");
    model.addHeader("x-oss-anything", "anything");

    EXPECT_TRUE(model.getHeaderAsBoolOr("x-oss-true"));
    EXPECT_FALSE(model.getHeaderAsBoolOr("x-oss-false"));
    EXPECT_FALSE(model.getHeaderAsBoolOr("x-oss-anything"));
    EXPECT_FALSE(model.getHeaderAsBoolOr("non-existent"));
    EXPECT_TRUE(model.getHeaderAsBoolOr("non-existent", true));
}

TEST(TypesTest, RequestModelGetParameterAsInt32Or) {
    TestRequestModel model;
    model.addParameter("count", "42");

    EXPECT_EQ(42, model.getParameterAsInt32Or("count"));
    EXPECT_EQ(-1, model.getParameterAsInt32Or("non-existent"));
    EXPECT_EQ(100, model.getParameterAsInt32Or("non-existent", 100));
}

TEST(TypesTest, RequestModelGetParameterAsInt64Or) {
    TestRequestModel model;
    model.addParameter("size", "123456789012345");

    EXPECT_EQ(123456789012345LL, model.getParameterAsInt64Or("size"));
    EXPECT_EQ(-1LL, model.getParameterAsInt64Or("non-existent"));
}

TEST(TypesTest, RequestModelGetParameterAsBoolOr) {
    TestRequestModel model;
    model.addParameter("enabled", "true");
    model.addParameter("disabled", "false");

    EXPECT_TRUE(model.getParameterAsBoolOr("enabled"));
    EXPECT_FALSE(model.getParameterAsBoolOr("disabled"));
    EXPECT_FALSE(model.getParameterAsBoolOr("non-existent"));
    EXPECT_TRUE(model.getParameterAsBoolOr("non-existent", true));
}

TEST(TypesTest, RequestModelGetHeaders) {
    TestRequestModel model;
    model.addHeader("key1", "value1");
    model.addHeader("key2", "value2");

    const auto& headers = model.getHeaders();
    EXPECT_EQ(2, headers.size());
    EXPECT_EQ("value1", headers.at("key1"));
    EXPECT_EQ("value2", headers.at("key2"));
}

TEST(TypesTest, RequestModelGetParameters) {
    TestRequestModel model;
    model.addParameter("param1", "val1");
    model.addParameter("param2", "val2");

    const auto& params = model.getParameters();
    EXPECT_EQ(2, params.size());
    EXPECT_EQ("val1", params.at("param1"));
    EXPECT_EQ("val2", params.at("param2"));
}

TEST(TypesTest, ResultModelGetHeaders) {
    HeaderCollection headers;
    headers["Content-Type"] = "application/json";

    ResultModel model(200, headers);
    const auto& modelHeaders = model.getHeaders();
    EXPECT_EQ("application/json", modelHeaders.at("Content-Type"));
}

TEST(TypesTest, ResultModelGetRequestId) {
    HeaderCollection headers;
    headers["x-oss-request-id"] = "request-12345";

    ResultModel result(200, headers);
    EXPECT_EQ("request-12345", result.getRequestId());
}

TEST(TypesTest, ResultModelGetStatusCode) {
    ResultModel result(200, HeaderCollection());
    EXPECT_EQ(200, result.getStatusCode());

    ResultModel result2(404, HeaderCollection());
    EXPECT_EQ(404, result2.getStatusCode());
}

TEST(TypesTest, CaseSensitiveLessComparison) {
    caseSensitiveLess cmp;

    EXPECT_TRUE(cmp("abc", "def"));
    EXPECT_FALSE(cmp("def", "abc"));
    EXPECT_FALSE(cmp("abc", "abc"));
    EXPECT_TRUE(cmp("ABC", "abc"));
}

TEST(TypesTest, CaseInsensitiveLessComparison) {
    caseInsensitiveLess cmp;

    EXPECT_TRUE(cmp("abc", "def"));
    EXPECT_FALSE(cmp("def", "abc"));
    EXPECT_FALSE(cmp("abc", "abc"));
    EXPECT_FALSE(cmp("ABC", "abc"));
    EXPECT_FALSE(cmp("abc", "ABC"));
    EXPECT_TRUE(cmp("aaa", "aab"));
}

TEST(TypesTest, ProgressCallbackTest) {
    std::size_t capturedIncrement = 0;
    std::size_t capturedTransferred = 0;
    std::int64_t capturedTotal = 0;
    std::uintptr_t capturedUserdata = 0;

    auto callback = [&capturedIncrement, &capturedTransferred, &capturedTotal, &capturedUserdata](
                            std::size_t increment, std::size_t transferred, std::int64_t total, std::uintptr_t userdata) {
        capturedIncrement = increment;
        capturedTransferred = transferred;
        capturedTotal = total;
        capturedUserdata = userdata;
    };

    ProgressCallback pc;
    pc.callback = callback;
    pc.userdata = 42;

    pc(100, 500, 1000);

    EXPECT_EQ(100, capturedIncrement);
    EXPECT_EQ(500, capturedTransferred);
    EXPECT_EQ(1000, capturedTotal);
    EXPECT_EQ(42, capturedUserdata);
}

TEST(TypesTest, ProgressCallbackWithNullCallback) {
    ProgressCallback pc;
    pc.callback = nullptr;
    pc.userdata = 42;

    pc(100, 500, 1000);
}

TEST(TypesTest, MetaDataCaseInsensitive) {
    MetaData meta;
    meta["Content-Type"] = "application/json";
    meta["content-type"] = "text/plain";

    EXPECT_EQ(1, meta.size());
    EXPECT_EQ("text/plain", meta["content-type"]);
}

TEST(TypesTest, ParameterCollectionCaseSensitive) {
    ParameterCollection params;
    params["Key"] = "value1";
    params["key"] = "value2";

    EXPECT_EQ(2, params.size());
    EXPECT_EQ("value1", params["Key"]);
    EXPECT_EQ("value2", params["key"]);
}
