#pragma once

#include <cstdint>
#include <string>

inline bool isFieldMissing(const std::string& value) {
    return value.empty();
}
inline bool isFieldMissing(std::int64_t value) {
    return value < 0;
}
