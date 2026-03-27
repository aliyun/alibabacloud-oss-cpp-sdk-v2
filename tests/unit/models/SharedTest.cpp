#include <gtest/gtest.h>

#include "alibabacloud/oss2/models/Shared.h"

namespace alibabacloud {
namespace oss2 {
namespace models {

TEST(SharedTest, Owner_ConstructorAll) {
    auto owner = Owner();
    EXPECT_EQ(false, owner.id.has_value());
    EXPECT_EQ(false, owner.displayName.has_value());

    owner.setId("id-123");
    owner.setDisplayName("id-desc");
    EXPECT_EQ("id-123", owner.id);
    EXPECT_EQ("id-desc", owner.displayName);
}

TEST(SharedTest, AccessControlList_ConstructorAll) {
    auto acl = AccessControlList();
    EXPECT_EQ(false, acl.grant.has_value());

    acl.setGrant("public");
    EXPECT_EQ("public", acl.grant);
}


} // namespace models
} // namespace oss2
} // namespace alibabacloud