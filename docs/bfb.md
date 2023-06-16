# Detailed description of bfb (binding-for-backend) communication protocol

## Current state message
This is retained message sent by SUA on start-up or as an answer to response for current state (then it is supplied with activityId field).
MQTT Topic: selfupdate/currentstate
```
{
    "activityId": "random-uuid",
    "timestamp": 123456789,
    "payload": {
        "softwareNodes": [
            {
                "id": "self-update-agent",
                "version": "build-42,
                "name": "OTA NG Self Update Agent",
                "type": "APPLICATION"
            },
            {
                "id": "self-update:leda-deviceimage",
                "version": "1.0",
                "name": "Official Leda device image",
                "type": "IMAGE"
            }
        ],
        "hardwareNodes": [],
        "associations": [
            {
                "sourceId": "self-update-agent",
                "targetId": "self-update:leda-deviceimage"
            }
        ]
    }
}
```

## Current state request
This message is a trigger to send back version of installed OS. Can be received at any point of time.
MQTT Topic: selfupdate/currentstate/get
```
{
    "activityId": "random-uuid",
    "timestamp": 123456789
}
```

## Desired state
This message indicates that SUA needs to perform an update operation.
MQTT Topic: selfupdate/desiredstate
```
{
    "activityId": "random-uuid",
    "timestamp": 123456789,
    "payload": {
        "domains": [
            {
                "id": "self-update",
                "components": [
                    {
                        "id": "os-image",
                        "version": "1.0",
                        "config": [
                            {
                                "key": "image",
                                "value": "http://example.com/downloads/os-image-1.1.bin"
                            }
                        ]
                    }
                ]
            }
        ]
    }
}
```

## Desired state feedback
This message contains information about SUA state - what agent is doing - installing, downloading or idle.
MQTT Topic: selfupdate/desiredstatefeedback
```
{
    "activityId": "random-uuid",
    "timestamp": 123456789,
    "payload": {
        "status": "payload_status",
        "message": "status message as a string",
        "actions": [
            {
                "component": {
                    "id": "self-update:os-image",
                    "version": "1.0"
                },
                "status": "action_status",
                "progress": 42,
                "message": "action message as a string"
            }
        ]
    }
}
```

## Command for single self-update action/step
This message contains information about SUA action - what must be done as a single step - download, update, activate, cleanup.
MQTT Topic: selfupdate/desiredstate/command
```
{
    "activityId": "random-uuid-as-string",
    "timestamp": 123456789,
    "payload": {
        "baseline": "BASELINE NAME",
        "command": "command_type"
    }
}
```

## Description of statuses
Following combinations of payload_status and action_status are possible:
| payload_status        | action_status    | Description |
|-----------------------|------------------|-------------|
| IDENTIFYING           |                  | SUA has received request for update and evaluating it |
| IDENTIFICATION_FAILED |                  | SUA has received request for update and is unable to perform self-update |
| IDENTIFIED            | IDENTIFIED       | SUA has received request for update and will try to perform an update |
| DOWNLOADING           | DOWNLOADING      | Downloading an image |
| DOWNLOAD_SUCCESS      | DOWNLOAD_SUCCESS | Image is downloaded without errors |
| DOWNLOAD_FAILURE      | DOWNLOAD_FAILURE | Image was not downloaded |
| UPDATING              | UPDATING         | Installing the downloaded image |
| UPDATE_SUCCESS        | UPDATING         | Image installed |
| UPDATE_FAILURE        | UPDATE_FAILURE   | Image was not installed due to error |
| ACTIVATING            | UPDATING         | SUA is activating the partition with new image |
| ACTIVATION_SUCCESS    | UPDATED          | SUA has activated the partition with new image |
| ACTIVATION_FAILURE    | UPDATE_FAILURE   | SUA has failed to activate the partition with new image |
| COMPLETE              | UPDATE_SUCCESS   | Self-update finished without errors |
| INCOMPLETE            | UPDATE_FAILURE<br>DOWNLOAD_FAILURE | Self-update is incomplete. Action status and action<br>message will preserve error from one of the previous steps: download/flash/activate.|

## Description of command types
| Command  | Description |
|----------|-------------|
| DOWNLOAD | Start download of bundle |
| UPDATE   | Flash image to partition |
| ACTIVATE | Make 'other' partition bootable |
| CLEANUP  | Remove temporary files |

## Description of other fields
| Name       | Description |
|------------|-------------|
| activityId | Random UUID as string (needs to be taken from desiredstate message and passed back in all feedback messages) |
| timestamp  | Epoch time (in seconds since January 1, 1970) |
| version    | Reflects version of SUA running on the device, OS version or bundle version |
| progress   | Percentage value of download/install progress |

