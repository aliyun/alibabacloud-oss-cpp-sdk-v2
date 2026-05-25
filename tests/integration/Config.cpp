#include "Config.h"
#include <algorithm>
#include <chrono>
#include <filesystem>
#include <gtest/gtest.h>
#include <sstream>
#include <string.h>
#include <string>
#include <thread>

std::string Config::AccessKeyId = "";
std::string Config::AccessKeySecret = "";
std::string Config::Endpoint = "";
std::string Config::Region = "";
std::string Config::RamRoleArn = "";
std::string Config::RamUID = "";
std::string Config::UserID;
std::string Config::AccountID;
std::string Config::PayerAccessKeyId = "";
std::string Config::PayerAccessKeySecret = "";
std::string Config::PayerUID = "";

static std::string LeftTrim(const char* source) {
    std::string copy(source);
    copy.erase(copy.begin(), std::find_if(copy.begin(), copy.end(), [](unsigned char ch) { return !::isspace(ch); }));
    return copy;
}

static std::string RightTrim(const char* source) {
    std::string copy(source);
    copy.erase(std::find_if(copy.rbegin(), copy.rend(), [](unsigned char ch) { return !::isspace(ch); }).base(),
               copy.end());
    return copy;
}

static std::string Trim(const char* source) {
    return LeftTrim(RightTrim(source).c_str());
}

static std::string LeftTrimQuotes(const char* source) {
    std::string copy(source);
    copy.erase(copy.begin(), std::find_if(copy.begin(), copy.end(), [](int ch) { return !(ch == '"'); }));
    return copy;
}

static std::string RightTrimQuotes(const char* source) {
    std::string copy(source);
    copy.erase(std::find_if(copy.rbegin(), copy.rend(), [](int ch) { return !(ch == '"'); }).base(), copy.end());
    return copy;
}

static std::string TrimQuotes(const char* source) {
    return LeftTrimQuotes(RightTrimQuotes(source).c_str());
}

void Config::LoadCfgFromEnv() {
    const char* value;
    value = std::getenv("OSS_TEST_ACCESS_KEY_ID");
    if (value) {
        Config::AccessKeyId = TrimQuotes(Trim(value).c_str());
    }

    value = std::getenv("OSS_TEST_ACCESS_KEY_SECRET");
    if (value) {
        Config::AccessKeySecret = TrimQuotes(Trim(value).c_str());
    }

    value = std::getenv("OSS_TEST_ENDPOINT");
    if (value) {
        Config::Endpoint = TrimQuotes(Trim(value).c_str());
    }

    value = std::getenv("OSS_TEST_REGION");
    if (value) {
        Config::Region = TrimQuotes(Trim(value).c_str());
    }

    value = std::getenv("OSS_TEST_RAM_ROLE_ARN");
    if (value) {
        Config::RamRoleArn = TrimQuotes(Trim(value).c_str());
    }

    value = std::getenv("OSS_TEST_RAM_UID");
    if (value) {
        Config::RamUID = TrimQuotes(Trim(value).c_str());
    }

    value = std::getenv("OSS_TEST_USER_ID");
    if (value) {
        Config::UserID = TrimQuotes(Trim(value).c_str());
    }

    value = std::getenv("OSS_TEST_ACCOUNT_ID");
    if (value) {
        Config::AccountID = TrimQuotes(Trim(value).c_str());
    }

    value = std::getenv("OSS_TEST_PAYER_ACCESS_KEY_ID");
    if (value) {
        Config::PayerAccessKeyId = TrimQuotes(Trim(value).c_str());
    }

    value = std::getenv("OSS_TEST_PAYER_ACCESS_KEY_SECRET");
    if (value) {
        Config::PayerAccessKeySecret = TrimQuotes(Trim(value).c_str());
    }

    value = std::getenv("OSS_TEST_PAYER_UID");
    if (value) {
        Config::PayerUID = TrimQuotes(Trim(value).c_str());
    }
}

std::string Config::GenBucketName() {
    std::stringstream ss;
    auto tp = std::chrono::time_point_cast<std::chrono::milliseconds>(std::chrono::system_clock::now());
    ss << "cpp-sdk-test-bucket-" << tp.time_since_epoch().count();
    return ss.str();
}

std::string Config::GetTempDir() {
    auto tempDir = testing::internal::FilePath(testing::TempDir());
    auto subDir = testing::internal::FilePath("cpp-sdk-test");
    auto dir = testing::internal::FilePath::ConcatPaths(tempDir, subDir);
    dir.CreateFolder();
    return dir.string();
}

std::string Config::GenRandomFileName() {
    auto dir = testing::internal::FilePath(GetTempDir());
    auto baseName = testing::internal::FilePath("test-");
    auto filepath = testing::internal::FilePath::GenerateUniqueFileName(dir, baseName, "tmp");
    return filepath.string();
}

void Config::CleanTempDir() {
    std::filesystem::remove_all(GetTempDir());
}

void Config::WaitForCacheExpire(int sec){
    std::this_thread::sleep_for(std::chrono::seconds(sec));
}
