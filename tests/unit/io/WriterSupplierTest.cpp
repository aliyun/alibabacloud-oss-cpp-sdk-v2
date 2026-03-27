#include <gtest/gtest.h>

#include "alibabacloud/oss2/io/ByteStream.h"

#include "../TestUtils.h"

#include <fstream>
#include <sstream>

namespace alibabacloud {
namespace oss2 {

TEST(OStreamSupplierTest, StringStreamWriter) {
    auto ss = std::make_shared<std::stringstream>();
    auto supplier = OStreamSupplier::from(
            [ss]() -> std::shared_ptr<std::ostream> {
                ss->clear();
                ss->seekg(0);
                ss->seekp(0);
                return ss;
            },
            true);
    EXPECT_EQ(false, supplier->isOneShot());

    std::string data = "hello world";

    std::size_t i = 0;
    for (i = 0; i < data.length(); i++) {
        auto w = supplier->getOStream();
        *w << data.substr(0, i + 1);
        EXPECT_EQ(data.substr(0, i + 1), ss->str());
        if (supplier->isOneShot()) {
            break;
        }
    }

    EXPECT_EQ(i, data.length());
}

TEST(OStreamSupplierTest, FStramWriter) {
    auto filepath = TestUtils::GenRandomFileName();

    auto supplier = OStreamSupplier::from(
            [filepath]() -> std::shared_ptr<std::ostream> {
                return std::make_shared<std::fstream>(filepath,
                                                      std::ios_base::out | std::ios_base::trunc | std::ios::binary);
            },
            true);
    EXPECT_EQ(false, supplier->isOneShot());

    std::string data = "hello world";

    std::size_t i = 0;
    for (i = 0; i < data.length(); i++) {
        auto w = supplier->getOStream();
        *w << data.substr(0, i + 1);
        w->flush();
        auto got = TestUtils::GetFileContent(filepath);
        EXPECT_EQ(data.substr(0, i + 1), got);
        if (supplier->isOneShot()) {
            break;
        }
    }

    EXPECT_EQ(i, data.length());
}


TEST(OStreamSupplierTest, NonReuse) {
    auto ss = std::make_shared<std::stringstream>();
    auto supplier = OStreamSupplier::from(
            [ss]() -> std::shared_ptr<std::ostream> {
                ss->clear();
                ss->seekg(0);
                ss->seekp(0);
                return ss;
            },
            false);
    EXPECT_EQ(true, supplier->isOneShot());

    std::string data = "hello world";

    std::size_t i = 0;
    for (i = 0; i < data.length();) {
        auto w = supplier->getOStream();
        *w << data.substr(0, i + 1);
        EXPECT_EQ(data.substr(0, i + 1), ss->str());
        i++;
        if (supplier->isOneShot()) {
            break;
        }
    }
    EXPECT_EQ(1, i);
}


} // namespace oss2
} // namespace alibabacloud