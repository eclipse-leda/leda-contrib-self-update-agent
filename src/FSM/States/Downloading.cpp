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

#include "FSM/States/Downloading.h"
#include "Download/Downloader.h"
#include "FSM/States/Failed.h"
#include "FSM/States/Idle.h"
#include "FSM/States/Installing.h"
#include "FSM/States/Uninitialized.h"
#include "Utils/BundleChecker.h"
#include "Logger.h"

namespace sua {
    void cleanUpDownload()
    {
        // TODO, add logic to clean up the download, move the cleanup logic to the donwloader
        Logger::info("Cleaning up download");
    }

    Downloading::Downloading(std::shared_ptr<FSM>& context, const MessageState payload)
        : State(context, payload)
    {
        subscribe(Downloader::EVENT_DOWNLOADED, [this](const std::string& payload) {
            Logger::info("New state for Downloading: Downloaded, payload = {}", payload);
            _payload.stateTechCode = std::stoi(payload);
            _payload.stateMessage  = "Downloaded completed";
            _payload.stateProgress = 100;
            handle(FotaEvent::DownloadReady, _payload);
        });

        subscribe(Downloader::EVENT_FAILED, [this](const std::string& payload) {
            Logger::info("New state for Downloading: Failed, payload = {}", payload);
            _payload.stateTechCode = std::stoi(payload);
            _payload.stateProgress = 0;
            _payload.stateMessage  = "Downloading failed with error code = " + payload;
            handle(FotaEvent::DownloadError, _payload);
        });

        subscribe(Downloader::EVENT_DOWNLOADING, [this](const std::string& payload) {
            Logger::info("New state for Downloading: Downloading, payload = {}", payload);
            _payload.stateProgress = std::stoi(payload);
            _payload.stateTechCode = 0;
            _payload.stateMessage  = "Downloading, progress = " + payload;
            handle(FotaEvent::DownloadProgress, _payload);
        });
    }

    void Downloading::onEntryTemplate()
    {
        _downloader = std::make_unique<Downloader>();
        _downloader->start(_payload.bundleDownloadUrl);
    }

    void Downloading::adjustEntryPayloadTemplate()
    {
        _payload.stateMessage  = "Entered Downloading state";
        _payload.stateProgress = 0;
        _payload.stateTechCode = 0;
    }

    void Downloading::handleTemplate(const FotaEvent event, const MessageState payload)
    {
        std::shared_ptr<State> nextState;

        switch(event) {
        case FotaEvent::DownloadReady:
            if(BundleChecker().isBundleVersionConsistent(payload.bundleVersion,
                                                              _context->_installerAgent, 
                                                              _context->_selfupdatesFilePath)) {   
                Logger::info("Downloaded bundle is valid, conntinue..");
                setPayload(payload);
                nextState = std::make_shared<Installing>(_context, payload);
            } else {
                // declared in yaml version shall be same as the downloaded one, if not then prevent installation
                Logger::info("Bundle version declared in yaml != downloaded bundle version, Reject update request with error 2001");
                MessageState msg  = payload;
                msg.stateProgress = 0;
                msg.stateTechCode = 2001;
                msg.stateMessage = "Invalid Bundle";
                nextState        = std::make_shared<Failed>(_context, msg);
            }
            transitTo(nextState);
            break;
        case FotaEvent::DownloadError:
            cleanUpDownload();
            setPayload(payload);
            nextState = std::make_shared<Failed>(_context, payload);
            transitTo(nextState);
            break;
        case FotaEvent::DownloadProgress:
            setPayload(payload);
            publishCurrentState();
            break;
        case FotaEvent::ConnectivityLost:
            cleanUpDownload();
            setPayload(payload);
            nextState = std::make_shared<Uninitialized>(_context, payload);
            transitTo(nextState);
            break;
        default:
            handleBadEvent(event, payload); // TODO, put reject, not only log
            break;
        }
    }

    FotaState Downloading::getState() const
    {
        return FotaState::Downloading;
    }
} // namespace sua
