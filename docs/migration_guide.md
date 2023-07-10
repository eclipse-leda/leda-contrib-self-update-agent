# Migration guide from YAML payloads to JSON (bfb) protocol

## Background
The communication protocol between SUA and backend was extended to support vehicle orchestration for large quantities of devices. Key differences are:
* Transition from YAML to JSON
* Fine-grained API (command-based approach instead of single-shot self-update)
* Extension of statuses (for detailed reflection of success, progress or failure)
Some fields are not relevant anymore meanwhile new were introduced. Below is a brief comparison between messages in YAML and in JSON format.

### Transition guide for current state message
From the current state message in YAML payload there is only one field 'bundleVersion' which is relevant. The corresponding field in JSON variant is 'version' in 'softwareNodes' for the device image. *NOTE*: The actual name could differ depending on the distro configuration.
```
apiVersion: sdv.eclipse.org/v1
kind: SelfUpdateBundle
metadata: 
    name: "self-update-bundle-example"
spec: 
    bundleVersion: 1.0
```

```
{
    "timestamp": 42,
    "payload": {
        "softwareNodes": [
            {
                "id": "self-update-agent",
                "version": "build-42",
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

### Transition guide for desired state message
For the desired state message there are two relevant fields 'bundleDownloadUrl' and 'bundleVersion' for take-over. The corresponding values are 'version' under the 'components' and 'value' inside the object with 'key'='image'. *NOTE*: There could be multiple entries inside 'components' array and 'config' section. Your implementation has to search for 'os-image' and 'image' values respectively.
```
apiVersion: "sdv.eclipse.org/v1"
kind: SelfUpdateBundle
metadata:
    name: self-update-bundle-example
spec:
    bundleDownloadUrl: http://url
    bundleName: arm64-bundle
    bundleTarget: base
    bundleVersion: 1.0
```
```
{
    "activityId": "random-uuid-as-string",
    "timestamp": 123456789,
    "payload": {
        "domains": [
            {
                "id": "self-update",
                "components": [
                    {
                        "id": "os-image",
                        "version": "1.1",
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

### Transition guide for state feedback
Below there is an example for state feedback ('techCode' is optional and available only in case of failure). For transition to JSON format all three fields from 'state' section are important (except 'techCode') and their corresponding places in JSON are 'status' and 'message' in sections 'payload' and 'actions', and 'progress' in section 'actions'. The detailed list of the available payload status and action status can be found in [link](docs/bfb.md) because for vehicle orchestration more sub-states were introduced for a fine-grained report of the self-update progress.
```
apiVersion: sdv.eclipse.org/v1
kind: SelfUpdateBundle
metadata:
    name: "self-update-bundle-example"
spec:
    bundleDownloadUrl: "http://url"
    bundleName: "arm64-bundle"
    bundleTarget: base
    bundleVersion: 1.0
state:
    message: Downloaded 10.0 MiB...
    name: downloading
    progress: 100
    techCode: 42
```
```
{
    "activityId": "id",
    "timestamp": 42,
    "payload": {
        "status": "ACTIVATION_SUCCESS",
        "message": "Self-update agent has activated the new OS image.",
        "actions": [
            {
                "component": {
                    "id": "self-update:os-image",
                    "version": "1.0"
                },
                "status": "UPDATED",
                "progress": 0,
                "message": "Self-update agent has activated the new OS image."
            }
        ]
    }
}
```
