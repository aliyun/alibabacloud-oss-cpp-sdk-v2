#include "TestUtils.h"

#include <algorithm>
#include <fstream>
#include <gtest/gtest.h>
#include <ostream>
#include <random>
#include <string>

namespace TestUtils {

std::string GenRandomString(size_t length) {
    static const char charset[] =
            "0123456789"
            "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
            "abcdefghijklmnopqrstuvwxyz";

    static std::mt19937 rng(std::random_device{}());
    static std::uniform_int_distribution<std::size_t> dist(0, sizeof(charset) - 2);

    std::string str(length, 0);
    std::generate_n(str.begin(), length, [&]() { return charset[dist(rng)]; });
    return str;
}

void WriteRandomDatatoFile(std::string file, size_t length) {
    std::fstream of(std::forward<std::string>(file), std::ios::out | std::ios::binary | std::ios::trunc);
    of << GenRandomString(length);
    of.close();
}

std::string CreateRootDirectory() {
    auto tempDir = testing::internal::FilePath(testing::TempDir());
    auto subDir = testing::internal::FilePath("cpp-sdk-test");
    auto dir = testing::internal::FilePath::ConcatPaths(tempDir, subDir);
    dir.CreateFolder();
    return dir.string();
}

std::string GenRandomFile(size_t length) {
    auto dir = testing::internal::FilePath(CreateRootDirectory());
    auto baseName = testing::internal::FilePath("test-");
    auto filepath = testing::internal::FilePath::GenerateUniqueFileName(dir, baseName, "tmp");

    std::ofstream f(filepath.string(), std::ios::binary);
    f << GenRandomString(length);
    f.close();

    return filepath.string();
}

std::string GenRandomFileName() {
    auto dir = testing::internal::FilePath(CreateRootDirectory());
    auto baseName = testing::internal::FilePath("test-");
    auto filepath = testing::internal::FilePath::GenerateUniqueFileName(dir, baseName, "tmp");
    return filepath.string();
}

std::string GetFileContent(std::string file) {
    std::ifstream f(file, std::ios::binary);
    std::stringstream ss;
    ss << f.rdbuf();
    f.close();
    return ss.str();
}

std::shared_ptr<std::istream> GetRandomStream(size_t length) {
    std::shared_ptr<std::stringstream> stream = std::make_shared<std::stringstream>();
    *stream << GenRandomString(length);
    stream->seekg(0);
    stream->seekp(0, std::ios_base::end);
    return stream;
}

std::size_t GetIStreamLength(std::istream& stream) {
    auto currentPos = stream.tellg();
    if (currentPos == static_cast<std::streampos>(-1)) {
        currentPos = 0;
        stream.clear();
    }
    stream.seekg(0, stream.end);
    auto streamSize = stream.tellg();
    stream.seekg(currentPos, stream.beg);
    return static_cast<std::size_t>(streamSize);
}


} // namespace TestUtils