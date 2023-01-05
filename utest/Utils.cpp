#include "Utils.h"

#include "gtest/gtest.h"

#include <sstream>
#include <string>

void EXPECT_EQ_MULTILINE(const std::string & a, const std::string & b)
{
    std::stringstream stream_a(a);
    std::stringstream stream_b(b);

    while(true) {
        std::string a;
        std::string b;

        stream_a >> a;
        stream_b >> b;

        EXPECT_EQ(a, b);

        if(!stream_a.eof() && !stream_b.eof()) {
            continue;
        }

        return;
    }
}
