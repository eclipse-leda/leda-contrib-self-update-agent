//    Copyright 2022 Contributors to the Eclipse Foundation
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
#include "Logger.h"
#include "Patterns/Dispatcher.h"
#include "TechCodes.h"

#include <cstdio>
#include <cstdlib>
#include <cstring>

#include <curl/curl.h>
#include <sys/stat.h>

namespace {
    const int        HTTP_OK                     = 200;
    const char       FILE_DIR[FILENAME_MAX]      = "/RaucUpdate";
    const char       FILE_PATH[FILENAME_MAX]     = "/RaucUpdate/temp_file";
    bool             cancelled                   = false;
    int              progressNotificationLimiter = 0;
    sua::Downloader* _downloader;

    size_t write_data(void* ptr, size_t size, size_t nmemb, FILE* stream);
    bool   download(const char* url);

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
            progress          = (int)progress_d;
        }

        if(progress >= progressNotificationLimiter) {
            sua::Dispatcher::instance().dispatch(sua::Downloader::EVENT_DOWNLOADING,
                                                 std::to_string(progress));
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

    bool download(const char* url)
    {
        CURLcode gres = curl_global_init(CURL_GLOBAL_ALL);
        if(gres != 0) {
            sua::Logger::critical("curl_global_init failed with code = {}", gres);
            return false;
        }

        CURL* easy_handle = curl_easy_init();
        if(!easy_handle) {
            sua::Logger::critical("curl_easy_init failed");
            return false;
        }

        long      response_code;
        const int dir_err = mkdir(FILE_DIR, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
        if(dir_err) {
            sua::Logger::error("Issue with creating a dir, error: {}", dir_err);
        }
        FILE* fp = fopen(FILE_PATH, "wb");
        curl_easy_setopt(easy_handle, CURLOPT_URL, url);
        curl_easy_getinfo(easy_handle, CURLINFO_RESPONSE_CODE, &response_code);
        curl_easy_setopt(easy_handle, CURLOPT_WRITEFUNCTION, write_data);
        curl_easy_setopt(easy_handle, CURLOPT_WRITEDATA, fp);
        curl_easy_setopt(easy_handle, CURLOPT_FOLLOWLOCATION, 1L);
        curl_easy_setopt(easy_handle, CURLOPT_PROGRESSDATA, &data);
        curl_easy_setopt(easy_handle, CURLOPT_PROGRESSFUNCTION, progress_callback);
        curl_easy_setopt(easy_handle, CURLOPT_NOPROGRESS, 0);
        CURLcode res = curl_easy_perform(easy_handle);

        sua::Logger::debug("curl_easy_perform ended with code = {}", res);

        long http_code = 0;
        curl_easy_getinfo(easy_handle, CURLINFO_RESPONSE_CODE, &http_code);
        curl_easy_cleanup(easy_handle);
        curl_global_cleanup();
        fclose(fp);
        progressNotificationLimiter = 0;

        sua::Logger::debug("CURLINFO_RESPONSE_CODE = {}", http_code);

        return http_code == HTTP_OK;
    }
} // namespace

namespace sua {

    const std::string Downloader::EVENT_DOWNLOADING = "Downloader/Downloading";
    const std::string Downloader::EVENT_DOWNLOADED  = "Downloader/Downloaded";
    const std::string Downloader::EVENT_FAILED      = "Downloader/Failed";

    bool Downloader::start(const std::string input)
    {
        Logger::trace("Downloader::start({})", input);
        _isWorking  = true;
        _downloader = this;
        Dispatcher::instance().dispatch(EVENT_DOWNLOADING, "0");

        const bool isOk = download(input.c_str());
        if(isOk) {
            Dispatcher::instance().dispatch(EVENT_DOWNLOADED,
                                            std::to_string(static_cast<int>(TechCode::OK)));
        } else {
            Dispatcher::instance().dispatch(
                EVENT_FAILED, std::to_string(static_cast<int>(TechCode::DownloadFailed)));
        }

        _isWorking = false;
        return _isWorking;
    }

} // namespace sua
