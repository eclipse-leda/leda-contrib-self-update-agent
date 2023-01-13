# Detailed description of bfb (binding-for-backend) communication protocol

## Current state message

This is retained message sent by SUA on start-up.
MQTT Topic: selfupdate/desiredstate
```
{
    "activityId": "",
    "timestamp": 123456789,
    "payload": {
        "domains": [
            {
                "id": "self-update",
                "components": [
                    {
                        "id": "os-image",
                        "version": "1.0"
                    }
                ]
            }
        ]
    }
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

## Description of statuses

Following combination of payload_status and action_status are possible:
| payload_status | action_status | Description |
|----------------|---------------|-------------|
| IDENTIFYING | | SUA has received request for update and evaluating it |
| IDENTIFIED | | SUA has received request for update and will try to perform an update |
| RUNNING | DOWNLOADING | Downloading an image |
| RUNNING | DOWNLOAD_SUCCESS | Image is downloaded without errors |
| RUNNING | INSTALLING | Installing an image |
| INCOMPLETE | UPDATE_FAILURE | One of these has failed: download, update or self-update was rejected. Message will contain detailed description what has happened |
| COMPLETED | UPDATE_SUCCESS | Self-update succeeded |

## Description of other fields

| name | description |
|------|-------------|
| activityId | Random UUID as string (needs to be taken from desiredstate message and passed back in all feedback messages) |
| timestamp | Epoch time |
| version | Either current version or bundle version |
| progress | Percentage value of download/install progress |
