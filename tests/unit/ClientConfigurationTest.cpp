#include <gtest/gtest.h>

#include "alibabacloud/oss2/ClientConfiguration.h"

using namespace alibabacloud::oss2;

TEST(ClientConfigurationTest, LoadDefault) {
    auto config = ClientConfiguration::loadDefault();
    EXPECT_FALSE(config.region.has_value());
}
