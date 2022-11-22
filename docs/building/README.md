# Building

The SUA can be used in both forms:

- as a native application,
- as an image running inside of a container.

This section describes the building steps for both variants.

If the SUA is to be used on the board with a different architecture than the host machine on which it is being built, the cross compilation is necessary.

# Native

## Install prerequisites

To be able to compile, following libraries and tools have to be installed:

### Toolchain

```
sudo apt install binutils cmake make autoconf file libtool git build-essential libcurl4-openssl-dev libselinux1-dev libmount-dev libmount1 libblkid-dev
```

### Compilers (for x86_64)

```
sudo apt install gcc g++
```

### Compilers (for Raspberry Pi board)

```
sudo apt install gcc-9-aarch64-linux-gnu g++-9-aarch64-linux-gnu
```

### Build tool Meson for glib

```
sudo apt-get install python3 python3-pip python3-setuptools python3-wheel ninja-build meson
pip3 install meson
```

## Build for amd64

```
./scripts/build_openssl_amd64.sh
./scripts/build_glib_amd64.sh
cd build_amd64
cmake -DCMAKE_INSTALL_PREFIX=../dist_amd64 -DCMAKE_TOOLCHAIN_FILE=../cmake/linux/amd64/toolchain.cmake -DOPENSSL_ROOT_DIR=../build_amd64 -DOPENSSL_CRYPTO_LIBRARY=../build_amd64/lib/libcrypto.so ..
make install
```

## Build for aarch64 (Raspberry Pi)

```
./scripts/build_openssl_arm64.sh
./scripts/build_glib_arm64.sh
cd build_arm64
cmake -DCMAKE_INSTALL_PREFIX=../dist_arm64 -DCMAKE_TOOLCHAIN_FILE=../cmake/linux/arm64/toolchain.cmake -DOPENSSL_ROOT_DIR=../build_arm64 -DOPENSSL_CRYPTO_LIBRARY=../build_arm64/lib/libcrypto.so ..
make install
```

## Run

The SUA binary will be placed under `dist_amd64/bin` / `dist_arm64/bin`. There is an optional parameter that can be specified: `-p path` location where the the downloaded bundles shall be stored:

```
cd dist_amd64/bin
LD_LIBRARY_PATH=../lib ./sdv-self-update-agent
```

```
cd dist_amd64/bin
LD_LIBRARY_PATH=../lib ./sdv-self-update-agent -p /data/download
```

## Hint about usage of devcontainer

This project provides the `.devcontainer`, which some of required for building tools. You can select the *open in the container* option in VS and this way use predefined development environment, instead of installing all the tools natively. The `.devcontainer` will be also used when you develop in *Codespaces*.

### Windows

If the docker and VS proxy settings are correctly set, the .devcontainer environment shall work on windows out of the box.

### Fix for OSD6 Linux distribution

For the case when using local VS Code (on OSD6) and opening in the remote container, the adjustment of proxy settings is necessary:

```
vim /opt/osd/proxy/config.ini
```

and then append additional server address for binding:

```
[server]
binds = 127.0.0.1:3128,172.17.0.1:3128
```

After this restart OSD proxy

```
osd-proxy-restart
```

Then in the .devcontainer/Dockerfile activate those lines

```
ENV http_proxy "http://172.17.0.1:3128"
ENV https_proxy "http://172.17.0.1:3128"
```

And finally trigger "Rebuild in Container" command from VSCode

# Containerized

SUA can be built as multi-arch image, and run inside of the container. This is preferred way of deployment, as it provides a necessary sandbox runtime environment and also allows to easily adjust the configuration settings (ports, ENV variables) via yaml.

Also the building process is easier, as all necessary tools are provided by the build platform - as also the build itself is being performed inside of the container.

## Build multi-arch image

### Build and load to local docker registry

Useful for testing on host machine: (specify only the host's architecture):

```
docker buildx build -f Dockerfile.arm64  --progress plain --platform "linux/arm64" -t sua:arm64 --load .
docker buildx build -f Dockerfile.amd64  --progress plain --platform "linux/amd64" -t sua:amd64 --load .
```

### Build and push to remote container registry (ghrc)

```
docker buildx build -f Dockerfile.arm64  --progress plain --platform "linux/arm64" -t ghcr.io/softwaredefinedvehicle/sdv-self-update-agent/sua:testing_arm64 --push .
docker buildx build -f Dockerfile.amd64  --progress plain --platform "linux/amd64" -t ghcr.io/softwaredefinedvehicle/sdv-self-update-agent/sua:testing_amd64 --push .
```

### Create multiarch image using remote container registry (ghrc)

```
docker buildx imagetools create -t ghcr.io/softwaredefinedvehicle/sdv-self-update-agent/sua:testing ghcr.io/softwaredefinedvehicle/sdv-self-update-agent/sua:testing_arm64 ghcr.io/softwaredefinedvehicle/sdv-self-update-agent/sua:testing_amd64
```

## Run

```
docker run -it sua:latest
```

## Hint

When running with proxy (corporate for example), it is helpful to uncomment following lines in the Dockerfile to configure the proxy properly

```
#uncomment when building on OSD6 environment
ENV http_proxy "http://172.17.0.1:3128"
ENV https_proxy "http://172.17.0.1:3128"
```
