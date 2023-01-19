# Testing

In order to be able to test SUA, the proper test environment must be prepared, which would simulate/mock the components with which the SUA is interacting. Depending if the test is being done for the native version od containerized variant, the preparation steps will differ:

# Native

For the native variant all tools have to be installed on the host machine:

## Preconditions

1. Install the mosquitto server and client

```
sudo apt-get update
sudo apt-get install mosquitto
sudo apt-get install mosquitto-clients
```

2. Ensure the python3 is installed.

## Start MQTT broker

```
mosquitto
```

## Adjust the host ip in code

Set the IP of the machine where the mosquitto broker is running, for the simplest case it would be the `localhost`:

```
    SoftwareUpdateAgent::SoftwareUpdateAgent()
    {
        sua::MqttConfiguration conf;
        conf.brokerHost = "localhost"
```

## Run native application

```
./sdv-self-update-agent
```

## Host test bundle file

Ensure that the proper test bundle is put into hosted directory. Adjust the IP and Port as needed.

```
cd docs/testing/fileserver/bundle
python3 -m http.server --bind 127.0.0.1 5555
```

## Simulate sending the MQTT Start messages

The content of the yaml file shall be adjusted, so that url of hosted bundle is valid.

```
mosquitto_pub -t "selfupdate/desiredstate" -f docs/testing/mqtt/start.yaml 
```

## Subscribe to MQTT feedback messages

To test if the SUA is behaving correctly and sending proper mqtt messages, subscribe to the topics:

```
mosquitto_sub -t "selfupdate/currentstate"
mosquitto_sub -t "selfupdate/desiredstatefeedback"
```

## Trigger current state request
```
mosquitto_pub -t "selfupdate/currentstate/get" -f docs/testing/mqtt/current-state-get.json
```

# Container

For testing the container variant, it shall be ensured that all the components are connected to the same network and using proper ports mapping, so that they would be able to communicate with each other.

## Create the network

```
docker network create -d bridge my-network
```

## Deploy Mosquitto broker

```
docker run -it --network=my-network -p 1883:1883  -v /absolute/path/sdv-self-update-agent/docs/testing/mqtt/mosquitto.conf:/mosquitto/config/mosquitto.conf -v /mosquitto/data -v /mosquitto/log --name mosquitto eclipse-mosquitto
```

Hint: it is required to use the absolute path to the mosquitto configuration file.

## Deploy python http server

```
cd docs/testing/fileserver
docker build -t host .
docker run -it --network=my-network -p 5555:5555 --name fileserver host
```

The bundle file will be available under: `fileserver:5555/bundle` url, so the value in `start.yaml` shall be adjusted.

## Deploy SUA

```
docker run -it --network=my-network sua:latest
```

Hint: before building and running sua, ensure that the broker host ia having the same value as the name parameter, specified for mosquitto broker:

 ```
    SoftwareUpdateAgent::SoftwareUpdateAgent()
    {
        sua::MqttConfiguration conf;
        conf.brokerHost = "mosquitto"
        ...
```

# HowTo subscribe to relevant messages

```
mosquitto_sub -t "selfupdate/selfupdate/desiredstatefeedback" -h ipAddress_of_mosquitto_container
mosquitto_sub -t "selfupdate/currentstate" -h ipAddress_of_mosquitto_container
```

From this tab console we can observe if the SUA is sending proper messages.

Hint: to figure out the the ip address of mosquitto container, run following command:

```
docker container ls // copy the containerID of mosquitto container
docker container inspect containerID
```

and locate the `IPAddress` value.

# HowTo send the Start signal to trigger the process

```
mosquitto_pub -t "selfupdate/desiredstate" -f docs/testing/mqtt/start.yaml -h ipAddress_of_mosquitto_container
```

After sending this signal, you should be able to observe the SUA behavior on the console.
