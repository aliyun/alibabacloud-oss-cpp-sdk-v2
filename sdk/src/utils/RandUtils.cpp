
#include "Utils.h"

#include <mutex>
#include <random>
#include <thread>

namespace alibabacloud {
namespace oss2 {
namespace utils {

uint32_t GetRandomValue() {
    static size_t const processRandomSeed = std::random_device{}();
    static std::mt19937 threadRandomSeedGen(static_cast<std::mt19937::result_type>(processRandomSeed));
    static std::mutex threadRandomSeedGenMtx;
    thread_local std::mt19937 gen([]() -> std::mt19937::result_type {
        std::lock_guard<std::mutex> lock(threadRandomSeedGenMtx);
        return static_cast<std::mt19937::result_type>(
            std::hash<std::thread::id>{}(std::this_thread::get_id()) ^ threadRandomSeedGen());
    }());

    return gen();
}

} // namespace utils
} // namespace oss2
} // namespace alibabacloud
