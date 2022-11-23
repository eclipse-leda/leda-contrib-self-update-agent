# See here for image contents: https://github.com/microsoft/vscode-dev-containers/tree/v0.195.0/containers/cpp/.devcontainer/base.Dockerfile
# [Choice] Debian / Ubuntu version (use Debian 11/9, Ubuntu 18.04/21.04 on local arm64/Apple Silicon): debian-11, debian-10, debian-9, ubuntu-21.04, ubuntu-20.04, ubuntu-18.04
ARG VARIANT="bullseye"
FROM mcr.microsoft.com/vscode/devcontainers/cpp:0-${VARIANT}

# ENV http_proxy "http://172.17.0.1:3128"
# ENV https_proxy "http://172.17.0.1:3128"

# Required packages.
RUN apt-get update && export DEBIAN_FRONTEND=noninteractive \
    && apt-get -y install python3-pip python3-setuptools python3-wheel ninja-build \
    libblkid-dev libselinux-dev gcc-9-aarch64-linux-gnu g++-9-aarch64-linux-gnu \
    mosquitto mosquitto-clients \
    && pip3 install meson

# Optional packages.
RUN apt-get -y install vim file iputils-ping
