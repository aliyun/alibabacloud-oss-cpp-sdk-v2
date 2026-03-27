
#include <fstream>
#include <iostream>
#include <memory>
#include <string>

namespace TestUtils {

std::string GenRandomString(size_t length);
void WriteRandomDatatoFile(std::string file, size_t length);
std::string GenRandomFile(size_t length);
std::string GenRandomFileName();
std::string CreateRootDirectory();
std::string GetFileContent(std::string file);
std::shared_ptr<std::istream> GetRandomStream(size_t length);
std::size_t GetIStreamLength(std::istream& stream);
} // namespace TestUtils