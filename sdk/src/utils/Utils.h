
#pragma once

#include "alibabacloud/oss2/Types.h"

#include <array>
#include <ctime>
#include <iostream>
#include <string>
#include <cstddef>
#include <stdint.h>

namespace alibabacloud {
namespace oss2 {
namespace utils {

// http utils
std::string UrlEncode(const std::string& src);
std::string UrlDecode(const std::string& src);
std::string UrlEncodePath(const std::string& src);

std::string ToQueryString(const ParameterCollection& parameters);
ParameterCollection ToEncodedParameters(const std::string& url);

// basd64 utils
std::string Base64Encode(const std::string& src);
std::string Base64Encode(const std::byte* src, std::size_t len);
std::string Base64EncodeUrlSafe(const std::string& src);
std::string Base64EncodeUrlSafe(const std::byte* src, std::size_t len);
std::string Base64Decode(const std::string& src);

// md5 utils
std::string CalcContentMD5(const std::string& data);
std::string CalcContentMD5(const char* data, size_t size);

// hash utils
void HmacSh256(const void* data, size_t numDataBytes, const void* key, size_t numKeyBytes, unsigned char out[32]);
std::string HashSh256(const void* data, size_t numDataBytes);

// crc64 utils
uint64_t CalcCRC64(uint64_t crc, void* buf, size_t len);
uint64_t CombineCRC64(uint64_t crc1, uint64_t crc2, uintmax_t len2);
uint64_t CalcCRC64(uint64_t crc, void* buf, size_t len, bool little);

// xml utils
std::string XmlEscape(const std::string& value);

// string utils
void StringReplace(std::string& src, const std::string& s1, const std::string& s2);
std::string StringJoin(const std::vector<std::string>& elements, std::string_view delimiter);
std::string LeftTrim(const char* source);
std::string RightTrim(const char* source);
std::string Trim(const char* source);
std::string LeftTrimQuotes(const char* source);
std::string RightTrimQuotes(const char* source);
std::string TrimQuotes(const char* source);
std::string ToLower(const char* source);
std::string ToUpper(const char* source);

// time utils
std::string ToGmtTime(std::time_t& t);
std::string ToUtcTime(std::time_t& t);
std::time_t UtcToUnixTime(const std::string& t);
std::time_t ToUnixTime(const std::string& str, const std::string& fmt);
std::string FormatUnixTime(const std::time_t& t, const std::string& fmt);

// mimetype utils
const std::string& LookupMimeType(const std::string& name);


} // namespace utils
} // namespace oss2
} // namespace alibabacloud