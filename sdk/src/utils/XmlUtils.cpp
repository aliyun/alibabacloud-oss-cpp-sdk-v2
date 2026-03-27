
#include "Utils.h"
#include <sstream>


namespace alibabacloud {
namespace oss2 {
namespace utils {


std::string XmlEscape(const std::string& value) {
    struct Entity {
        const char* pattern;
        char value;
    };

    static const Entity entities[] = {{"&quot;", '\"'}, {"&amp;", '&'}, {"&apos;", '\''},
                                      {"&lt;", '<'},    {"&gt;", '>'},  {"&#13;", '\r'}};

    if (value.empty()) {
        return value;
    }

    std::stringstream ss;
    for (size_t i = 0; i < value.size(); i++) {
        bool flag = false;
        for (size_t j = 0; j < (sizeof(entities) / sizeof(entities[0])); j++) {
            if (value[i] == entities[j].value) {
                flag = true;
                ss << entities[j].pattern;
                break;
            }
        }

        if (!flag) {
            ss << value[i];
        }
    }

    return ss.str();
}

} // namespace utils
} // namespace oss2
} // namespace alibabacloud