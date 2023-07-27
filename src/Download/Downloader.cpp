//    Copyright 2023 Contributors to the Eclipse Foundation
//
//    Licensed under the Apache License, Version 2.0 (the "License");
//    you may not use this file except in compliance with the License.
//    You may obtain a copy of the License at
//
//        http://www.apache.org/licenses/LICENSE-2.0
//
//    Unless required by applicable law or agreed to in writing, software
//    distributed under the License is distributed on an "AS IS" BASIS,
//    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
//    See the License for the specific language governing permissions and
//    limitations under the License.
//
//    SPDX-License-Identifier: Apache-2.0

#include "Download/Downloader.h"
#include "Context.h"
#include "Logger.h"
#include "Patterns/Dispatcher.h"
#include "TechCodes.h"

#include <cstdio>
#include <cstdlib>
#include <cstring>

#include <tuple>

#include <dirent.h>
#include <curl/curl.h>
#include <sys/stat.h>

namespace {

    const int        HTTP_OK                     = 200;
    char             FILE_DIR[FILENAME_MAX]      = "/RaucUpdate";
    char             FILE_PATH[FILENAME_MAX]     = "/RaucUpdate/temp_file";
    bool             cancelled                   = false;
    int              progressNotificationLimiter = 0;

    struct progress {
        char*  unused;
        size_t size;
    };

    static size_t
    progress_callback(void* clientp, double dltotal, double dlnow, double ultotal, double ulnow)
    {
        int progress = 0;
        if(dltotal > 0) {
            double progress_d = 100.0 * dlnow / dltotal;
            progress          = std::max(0, std::min(100, (int)progress_d));
        }

        // Skip 100% progress for DOWNLOADING event
        if(progress == 100) {
            return 0;
        }

        const uint64_t total      = static_cast<uint64_t>(dltotal);
        const uint64_t downloaded = static_cast<uint64_t>(dlnow);

        if(progress >= progressNotificationLimiter) {
            std::map<std::string, std::string> payload;
            payload["downloaded"] = std::to_string(downloaded);
            payload["total"     ] = std::to_string(total);
            payload["percentage"] = std::to_string(progress);
            sua::Dispatcher::instance().dispatch(sua::Downloader::EVENT_DOWNLOADING, payload);
            progressNotificationLimiter += 10;
        }

        if(cancelled) {
            return 1;
        }

        return 0;
    }

    struct progress data;

    size_t write_data(void* ptr, size_t size, size_t nmemb, FILE* stream)
    {
        size_t written = fwrite(ptr, size, nmemb, stream);
        return written;
    }

    class CurlGlobalGuard {
    public:
        CURLcode init() {
            return curl_global_init(CURL_GLOBAL_ALL);
        }

        ~CurlGlobalGuard() {
            curl_global_cleanup();
        }
    };

    class CurlEasyHandleGuard {
    public:
        CURL * init() {
            _handle = curl_easy_init();
            return _handle;
        }

        ~CurlEasyHandleGuard() {
            if(_handle) {
                curl_easy_cleanup(_handle);
            }
        }

    private:
        CURL * _handle = nullptr;
    };

    sua::DownloadResult download(const std::string & caPath, const std::string & caFile, const char* url)
    {
        CurlGlobalGuard global_guard;
        auto init_status = global_guard.init();
        if(init_status != 0) {
            sua::Logger::critical("curl_global_init failed with code = {}", init_status);
            return std::make_tuple(sua::TechCode::DownloadFailed, "libcurl init failed");
        }

        CurlEasyHandleGuard easy_guard;
        auto h = easy_guard.init();

        if(!h) {
            sua::Logger::critical("curl_easy_init failed");
            return std::make_tuple(sua::TechCode::DownloadFailed, "libcurl init failed");
        }

        DIR* dir = opendir(FILE_DIR);
        if (dir) {
            closedir(dir);
        } else {
            const int dir_err = mkdir(FILE_DIR, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
            if(dir_err) {
                sua::Logger::critical("Issue with creating a dir {}, error: {}", FILE_DIR, dir_err);
                return std::make_tuple(sua::TechCode::DownloadFailed, "temporary file folder could not be created");
            }
        }

        long response_code;
        FILE* fp = fopen(FILE_PATH, "wb");
        if(!fp) {
            sua::Logger::critical("Failed to open '{}' for writing", FILE_PATH);
            return std::make_tuple(sua::TechCode::DownloadFailed, "temporary file could not be opened for writing");
        }

        curl_easy_setopt(h, CURLOPT_URL, url);
        curl_easy_getinfo(h, CURLINFO_RESPONSE_CODE, &response_code);
        if(!caFile.empty()) {
            curl_easy_setopt(h, CURLOPT_CAINFO, caFile.c_str());
        } else {
            curl_easy_setopt(h, CURLOPT_CAPATH, caPath.c_str());
        }
        curl_easy_setopt(h, CURLOPT_SSL_VERIFYPEER, 1L);
        curl_easy_setopt(h, CURLOPT_SSL_VERIFYHOST, 2L);
        curl_easy_setopt(h, CURLOPT_WRITEFUNCTION, write_data);
        curl_easy_setopt(h, CURLOPT_WRITEDATA, fp);
        curl_easy_setopt(h, CURLOPT_FOLLOWLOCATION, 1L);
        curl_easy_setopt(h, CURLOPT_PROGRESSDATA, &data);
        curl_easy_setopt(h, CURLOPT_PROGRESSFUNCTION, progress_callback);
        curl_easy_setopt(h, CURLOPT_NOPROGRESS, 0);
        curl_easy_setopt(h, CURLOPT_PROTOCOLS_STR, "https");
        CURLcode res = curl_easy_perform(h);

        sua::Logger::debug("curl_easy_perform ended with code = '{}'", res);

        long http_code = 0;
        curl_easy_getinfo(h, CURLINFO_RESPONSE_CODE, &http_code);
        fclose(fp);
        progressNotificationLimiter = 0;

        sua::Logger::debug("CURLINFO_RESPONSE_CODE = {}", http_code);
        if(http_code != 200) {
            auto e = curl_easy_strerror(res);
            sua::Logger::error(e);
            return std::make_tuple(sua::TechCode::DownloadFailed, e);
        }

        return std::make_tuple(sua::TechCode::OK, "");
    }

} // namespace

namespace sua {

    const std::string Downloader::EVENT_DOWNLOADING = "Downloader/Downloading";

    Downloader::Downloader(Context & context)
        : _context(context)
    {
        const std::string filepath = _context.updatesDirectory + _context.tempFileName;
        strncpy(FILE_DIR, _context.updatesDirectory.c_str(), FILENAME_MAX - 1);
        strncpy(FILE_PATH, filepath.c_str(), FILENAME_MAX - 1);
    }

    DownloadResult Downloader::start(const std::string & input)
    {
        return download(_context.caDirectory, _context.caFilepath, input.c_str());
    }

} // namespace sua
