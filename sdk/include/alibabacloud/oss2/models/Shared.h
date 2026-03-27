#pragma once

#include "alibabacloud/oss2/Types.h"

#include <optional>
#include <string>


namespace alibabacloud {
namespace oss2 {
namespace models {

/*
 * The container for the information about the bucket owner.
 */
struct ALIBABACLOUD_OSS_API Owner final {
    // The user ID of the bucket owner.
    std::optional<std::string> id;

    // The name of the bucket owner, which is the same as the user ID.
    std::optional<std::string> displayName;


    // Provide setter interfaces via template
    template <typename ValueT = std::string>
    Owner& setId(ValueT&& value) {
        id = std::forward<ValueT>(value);
        return *this;
    }

    template <typename ValueT = std::string>
    Owner& setDisplayName(ValueT&& value) {
        displayName = std::forward<ValueT>(value);
        return *this;
    }
};


/*
 * The class of the container that stores the ACL information.
 */
struct ALIBABACLOUD_OSS_API AccessControlList final {
    // The ACL of the bucket.
    std::optional<std::string> grant;


    // Provide setter interfaces via template
    template <typename ValueT = std::string>
    AccessControlList& setGrant(ValueT&& value) {
        grant = std::forward<ValueT>(value);
        return *this;
    }
};


/*
 * The container that stores the ACL information.
 */
struct ALIBABACLOUD_OSS_API AccessControlPolicy final {
    // The container that stores the information about the bucket owner.
    std::optional<Owner> owner;

    // The class of the container that stores the ACL information.
    std::optional<AccessControlList> accessControlList;


    // Provide setter interfaces via template
    template <typename ValueT = Owner>
    AccessControlPolicy& setOwner(ValueT&& value) {
        owner = std::forward<ValueT>(value);
        return *this;
    }

    template <typename ValueT = AccessControlList>
    AccessControlPolicy& setAccessControlList(ValueT&& value) {
        accessControlList = std::forward<ValueT>(value);
        return *this;
    }
};


/*
 * The container used to store the tag that you want to configure.
 */
struct ALIBABACLOUD_OSS_API Tag final {
    // The key of a tag. *   A tag key can be up to 64 bytes in length.*   A tag key cannot start with `http://`,
    // `https://`, or `Aliyun`.*   A tag key must be UTF-8 encoded.*   A tag key cannot be left empty.
    std::string key;

    // The value of the tag that you want to add or modify. *   A tag value can be up to 128 bytes in length.*   A tag
    // value must be UTF-8 encoded.*   The tag value can be left empty.
    std::string value;


    // Provide setter interfaces via template
    template <typename ValueT = std::string>
    Tag& setKey(ValueT&& value) {
        key = std::forward<ValueT>(value);
        return *this;
    }

    template <typename ValueT = std::string>
    Tag& setValue(ValueT&& val) {
        value = std::forward<ValueT>(val);
        return *this;
    }
};


/*
 * The container for tags.
 */
struct ALIBABACLOUD_OSS_API TagSet final {
    // The tags.
    std::vector<Tag> tags;


    // Provide setter interfaces via template
    template <typename ValueT = std::vector<Tag>>
    TagSet& setTags(ValueT&& value) {
        tags = std::forward<ValueT>(value);
        return *this;
    }
};

/*
 * The container that stores the tags.
 */
struct ALIBABACLOUD_OSS_API Tagging final {
    // The container used to store a set of tags.
    std::optional<TagSet> tagSet;


    // Provide setter interfaces via template
    template <typename ValueT = TagSet>
    Tagging& setTagSet(ValueT&& value) {
        tagSet = std::forward<ValueT>(value);
        return *this;
    }
};

/*
 * If the delimiter parameter is specified in the request, the response contains CommonPrefixes. Objects whose names
 * contain the same string from the prefix to the next occurrence of the delimiter are grouped as a single result
 * element in CommonPrefixes.
 */
struct ALIBABACLOUD_OSS_API CommonPrefix final {
    // The prefix contained in the names of returned objects.
    std::string prefix;

    // Provide setter interfaces via template
    template <typename ValueT = std::string>
    CommonPrefix& setPrefix(ValueT&& value) {
        prefix = std::forward<ValueT>(value);
        return *this;
    }
};


} // namespace models
} // namespace oss2
} // namespace alibabacloud