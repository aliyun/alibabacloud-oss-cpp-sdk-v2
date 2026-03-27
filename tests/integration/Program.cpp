#include "Config.h"
#include <gtest/gtest.h>

int main(int argc, char** argv) {
    Config::LoadCfgFromEnv();
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}