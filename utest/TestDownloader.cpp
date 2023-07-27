#include "gtest/gtest.h"

#include "Context.h"
#include "Download/Downloader.h"

#include <spdlog/fmt/fmt.h>

namespace {

    class TestDownloaderP
        : public ::testing::TestWithParam<std::string>
    { };

    TEST_P(TestDownloaderP, downloadViaUnsupportedProtocolFails)
    {
        sua::Context ctx;
        sua::Downloader d(ctx);

        auto url = fmt::format("{}://127.0.0.1/bundle", GetParam());
        auto res = d.start(url);

        EXPECT_EQ(std::get<0>(res), sua::TechCode::DownloadFailed);
    }

    INSTANTIATE_TEST_CASE_P(
        TestDownloaderViaUnsupportedProtocols,
        TestDownloaderP,
        ::testing::Values(
            "ftp", "http", "sftp", "smb", "smbs", "ldap", "ldaps"
        )
    );

}


