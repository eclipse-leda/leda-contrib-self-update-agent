# License
Apache 2.0

# Contribution
Follow guidelines from Eclipse Leda: https://eclipse-leda.github.io/leda/docs/project-info/contribution-guidelines

# Security Policy
This project implements the Eclipse Foundation Security Policy: https://www.eclipse.org/security

# Reporting a Vulnerability
Please report vulnerabilities to the Eclipse Foundation Security Team at security@eclipse.org

# Data privacy notice:
Data privacy notice is here: [link](docs/data-privacy-notice.md)

# SDV Self Update Agent
The self update agent (SUA) is a component responsible for the OS Update process. 
SUA is communicating on MQTT interface via usage of defined messages. Internally, SUA uses [RAUC](https://rauc.io/) to perform the update. 

Following sequence diagram shows the happy path example of communication between components. 

```mermaid
sequenceDiagram
    participant m as MQTT Broker 
    participant s as SUA
    participant r as RAUC

    s -->> m: connect

    loop Wait for OTA trigger

    Note left of s: Initial start
    s ->> m: Current state (installed version from booted partition)

    Note left of s: Trigger for OTA
    m ->> s: Desired state request (new version and url to bundle)
    s ->> m: Feedback (update actions are identified)

    Note left of s: Command for Download
    m ->> s: Download command
    s ->> s: Download bundle
    s ->> m: Feedback (downloading/downloaded/failed)

    Note left of s: Command for Update
    m ->> s: Update command
    s ->> r: Flash image to partition
    r ->> r: Flashing...
    s ->> m: Feedback (updating with percentage)
    r ->> s: Flash completed/failed
    s ->> m: Feedback (updated/failed)

    Note left of s: Command for Activate
    m ->> s: Activate command
    s ->> r: Switch partitions (booted <-> other)
    r ->> s: Switch completed/failed
    s ->> m: Feedback (activated/failed)

    Note left of s: Command for Cleanup
    m ->> s: Cleanup command
    s ->> s: Remove temporary files
    s ->> m: Cleanup completed + status from previously failed state<br>(completed/failed)

    end
```

```mermaid
stateDiagram
    Uninitialized --> Connected: Connected
    Connected --> Identified: Start (OTA trigger)
    Identified --> Downloading: Command download
    Identified --> Failed: If OTA trigger is invalid
    Downloading --> Updating: Command update
    Downloading --> Failed: If download has failed
    Updating --> Activate: Command activate
    Updating --> Failed: If update has failed
    Activate --> Cleanup: Command cleanup
    Activate --> Failed: If activate has failed
    Failed --> Cleanup: Command cleanup
    Cleanup --> Idle
```
Important: Uninitialized state is the default entry state or state in case connection is lost. To simplify reading of the diagram arrows from other states to Unitialized have been removed.

MQTT communication is done over 5 MQTT topics:

## Trigger OTA
| Topic | Direction | Description |
|-------|-----------|-------------|
| selfupdate/desiredstate | IN | This message triggers the update process. The payload shall contain all data necessary to obtain the update bundle and to install it. |

## Trigger self-update step/action
| Topic | Direction | Description |
|-------|-----------|-------------|
| selfupdate/desiredstate/command | IN | This message triggers the single step in update process (download/flash/activate/cleanup). |

## Report current state
| Topic | Direction | Description |
|-------|-----------|-------------|
| selfupdate/currentstate | OUT | This message is being sent either once on SUA start or as an answer to response received by selfupdate/currentstate/get. It contains information about the currently installed OS version. |

## Get current state
| Topic | Direction | Description |
|-------|-----------|-------------|
| selfupdate/currentstate/get | IN | This message can be received at any point of time. Indicates that SUA should send back the version of the installed OS as current state. |

## Report status of self-update process
| Topic | Direction | Description |
|-------|-----------|-------------|
| selfupdate/desiredstatefeedback | OUT | This message is being sent by SUA to share the current progress of the triggered update process. This is the *OUT* counterpart of *selfupdate/desiredstate* input message. |

Detailed description of Update Agent API can be found here: [link](docs/bfb.md).
Migration guide for users from old YAML payloads to new JSON format can be found here: [link](docs/migration_guide.md).

# Checkout
SUA links to some 3rd party libraries, which are fetched as submodules, therefore the cloning shall be performed with recursive option:

```
git clone --recursive https://github.com/eclipse-leda/leda-contrib-self-update-agent.git
```
or if was cloned non recursively
```
git submodule init
git submodule update
```

# HowTo Build
Instructions for building are available on: [link](docs/building/README.md)

# HowTo Deploy
Instructions for deploying are available on: [link](docs/deploying/README.md)

# HowTo Test
Instructions for testing are available on: [link](docs/testing/README.md)
