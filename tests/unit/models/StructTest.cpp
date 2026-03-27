#include <gtest/gtest.h>

#include "alibabacloud/oss2/models/Shared.h"

namespace alibabacloud {
namespace oss2 {
namespace models {

struct StructObject final {
    std::string key;

    // Provide setter interfaces via template
    template <typename ValueT = std::string>
    StructObject& setKey(ValueT&& value) {
        key = std::forward<ValueT>(value);
        return *this;
    }
};


TEST(StructTest, StringType) {
    StructObject obj;

    // const char*
    obj.setKey("hello");
    EXPECT_EQ("hello", obj.key);

    // std::string&
    std::string val1 = "hel&";
    obj.setKey(val1);
    EXPECT_EQ("hel&", obj.key);

    // std::string&&
    std::string val2 = "hel&&";
    obj.setKey(std::move(val2));
    EXPECT_EQ("hel&&", obj.key);
}


} // namespace models
} // namespace oss2
} // namespace alibabacloud