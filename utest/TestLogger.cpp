#include "gtest/gtest.h"
#include "Logger.h"
#include "nlohmann/json.hpp"

#include <algorithm>
#include <fstream>
#include <regex>

namespace {

    bool isTimeFormatCorrect(const std::string & input)
    {
        std::regex r("\\d{4}-\\d{2}-\\d{2}T\\d{2}:\\d{2}:\\d{2}.\\d{6}[+-]\\d{2}:\\d{2}");
        std::smatch m;
        return std::regex_match(input, m, r);
    }

    std::string readAndAdaptLog(const std::string & path)
    {
        std::ifstream f;
        f.open(path);

        std::stringstream ss;
        ss << f.rdbuf();

        std::string log = ss.str();

        // replace last "," to parse json output by wrapping into array []
        log.erase(log.rfind(','));
        ss.str("");
        ss << "[" << log << "]";
        return ss.str();
    }

    TEST(TestLogger, allLogLevelsToFile)
    {
        sua::Logger::instance().init();
        sua::Logger::instance().setLogLevel(sua::Logger::Level::All);
        sua::Logger::trace("T");
        sua::Logger::debug("D");
        sua::Logger::info("I");
        sua::Logger::warning("W");
        sua::Logger::error("E");
        sua::Logger::critical("C");
        sua::Logger::instance().shutdown();

        nlohmann::json data = nlohmann::json::parse(readAndAdaptLog("../logs/log.json"));

        EXPECT_TRUE(isTimeFormatCorrect(data[0]["time"]));
        EXPECT_EQ(data[0]["level"], "trace");
        EXPECT_EQ(data[0]["message"], "T");

        EXPECT_TRUE(isTimeFormatCorrect(data[1]["time"]));
        EXPECT_EQ(data[1]["level"], "debug");
        EXPECT_EQ(data[1]["message"], "D");

        EXPECT_TRUE(isTimeFormatCorrect(data[2]["time"]));
        EXPECT_EQ(data[2]["level"], "info");
        EXPECT_EQ(data[2]["message"], "I");

        EXPECT_TRUE(isTimeFormatCorrect(data[3]["time"]));
        EXPECT_EQ(data[3]["level"], "warning");
        EXPECT_EQ(data[3]["message"], "W");

        EXPECT_TRUE(isTimeFormatCorrect(data[4]["time"]));
        EXPECT_EQ(data[4]["level"], "error");
        EXPECT_EQ(data[4]["message"], "E");

        EXPECT_TRUE(isTimeFormatCorrect(data[5]["time"]));
        EXPECT_EQ(data[5]["level"], "critical");
        EXPECT_EQ(data[5]["message"], "C");
    }

}
