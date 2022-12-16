# Detailed description of k8s communication protocol

## Current state message

This is retained message sent by SUA on start-up.
MQTT Topic: selfupdate/desiredstate
```
apiVersion: "sdv.eclipse.org/v1"
kind: SelfUpdateBundle
metadata:
  name: self-update-bundle-example
spec:
  bundleVersion: v1beta3
```

## Desired state

This message indicates that SUA needs to perform an update operation.
MQTT Topic: selfupdate/desiredstate
```
apiVersion: "sdv.eclipse.org/v1"
kind: SelfUpdateBundle
metadata:
  name: self-update-bundle-example
spec:
  bundleName: swdv-arm64-build42
  bundleVersion: v1beta3
  bundleDownloadUrl: https://example.com/repository/base/
  bundleTarget: base
```

## Desired state feedback

This message contains information about SUA state - what agent is doing - installing, downloading or idle.
MQTT Topic: selfupdate/desiredstatefeedback
```
apiVersion: "sdv.eclipse.org/v1"
kind: SelfUpdateBundle
metadata:
  name: self-update-bundle-example
spec:
  bundleName: swdv-arm64-build42
  bundleVersion: v1beta3
  bundleDownloadUrl: https://example.com/repository/base/
  bundleTarget: base
state:
  name: "idle|installing|etc."
  progress: 0|51|99|etc., 
  techCode: 0|1|5|etc., 
  message: "Cannot download from url|Bundle already installed|etc."
```

### state enum

State name field can have one of following values:

| State | Description | Additional payload data | 
|  ---------- |  ---------- | ---------- |
| uninitialized | When the SUA is not configured yet | - |
| idle |  Configured and waiting for messages | - |
| downloading | Downloading the bundle file  | progress |
| installing | Performing installation   | progress |
| installed |  Installation process was successful, new OS version is installed on inactive disc Slot. **Important: to finish the OTA process, reboot is required, and it shall be performed by Vehicle Update Manager.** |  - |
| failed | Error occurred  | techCode |

### techCode values
techCode field is providing additional details to the state value. It is especially useful for the **failed** state, as it can specify the reason of failure. 

| Value | Description |
|  ---- |  ---------- | 
| 0 | OK, no error|
| 1001 |  Download failed |
| 2001 |  Invalid Bundle|
| 3001 |  Installation failed |
| 4001 |  Update rejected, bundle version same as current OS version |
| 5001 |  Unknown Error |
