# Preparation for arm_soc platform compilation

## 1 Overview

For the SoC platform, the corresponding `libsophon`, `sophon-opencv` and `sophon-ffmpeg` runtime library packages have been integrated internally, located under `/opt/sophon/`, and can be directly used in the runtime environment.

 Usually, the program is cross-compiled on the x86 host computer so that it can run on the SoC platform. You need to create a cross-compilation environment on the x86 host using SOPHON SDK and package the header and library files that the program depends on into the soc-sdk directory.

## 2 Prepare the compile environment

This section requires preparation of libsophon-soc and sophon-mw packages in advance, please access [ Sophgo offical site ](https://developer.sophon.ai/site/index/material/31/all.html) to download them.

### 2.1 Installing the cross-compilation tool chain

```bash
sudo apt-get install gcc-aarch64-linux-gnu g++-aarch64-linux-gnu
```

### 2.2 prepare libsophon

```bash
# Create the root directory of the dependency files
mkdir -p soc-sdk
# Extract libsophon_soc_${x.y.z}_aarch64.tar.gz from the sophon-img release package, where x.y.z is the version number
tar -zxf libsophon_soc_${x.y.z}_aarch64.tar.gz
# Copy the relevant library and header directories to the root of the dependency file
cp -rf libsophon_soc_${x.y.z}_aarch64/opt/sophon/libsophon-${x.y.z}/lib ${soc-sdk}
cp -rf libsophon_soc_${x.y.z}_aarch64/opt/sophon/libsophon-${x.y.z}/include ${soc-sdk}
```

### 2.3 prepare ffmpeg and opencv

```bash
# Extract sophon-mw-soc_${x.y.z}_aarch64.tar.gz from the sophon-mw package, where x.y.z is the version number
tar -zxf sophon-mw-soc_${x.y.z}_aarch64.tar.gz
# Copy the ffmpeg and opencv library directories and header directories to the root of the dependency file
cp -rf sophon-mw-soc_${x.y.z}_aarch64/opt/sophon/sophon-ffmpeg_${x.y.z}/lib ${soc-sdk}
cp -rf sophon-mw-soc_${x.y.z}_aarch64/opt/sophon/sophon-ffmpeg_${x.y.z}/include ${soc-sdk}
cp -rf sophon-mw-soc_${x.y.z}_aarch64/opt/sophon/sophon-opencv_${x.y.z}/lib ${soc-sdk}
cp -rf sophon-mw-soc_${x.y.z}_aarch64/opt/sophon/sophon-opencv_${x.y.z}/include ${soc-sdk}
```

### 2.4 Prepare third-party libraries

Depends on libbeigen3-dev, libgflags-dev, libgoogle-log-dev, libexiv2-dev.

#### 2.4.1 Prepare and set up a qemu virtual environment

```bash
# install qemu
sudo apt-get install -y qemu-user-static debootstrap
# Create the folder and build the virtual environment, mapping it to the rootfs folder
mkdir rootfs
cd rootfs
# build rootfs of ubuntu 20.04
sudo qemu-debootstrap --arch=arm64 focal .
sudo chroot . qemu-aarch64-static /bin/bash

# After entering qemu，install libeigen3-dev、libgflags-dev、libgoogle-glog-dev、libexiv2-dev
apt-get install -y software-properties-common
apt-add-repository universe
apt-get update
apt-get install -y libeigen3-dev libgflags-dev libgoogle-glog-dev libexiv2-dev

# Use the exit command to exit the qemu virtual environment
exit
```

#### 2.4.2 Copy third-party library headers and libraries

```bash
# Exit the qemu virtual environment
# libgoogle-glog-dev
cp -rf ${rootfs}/usr/lib/aarch64-linux-gnu/libglog* ${soc-sdk}/lib
cp -rf ${rootfs}/usr/include/glog ${soc-sdk}/include
# libgflags-dev
cp -rf ${rootfs}/usr/lib/aarch64-linux-gnu/libgflags* ${soc-sdk}/lib
cp -rf ${rootfs}/usr/include/gflags ${soc-sdk}/include
# libexiv2-dev
cp -rf ${rootfs}/usr/lib/aarch64-linux-gnu/libexiv2* ${soc-sdk}/lib
cp -rf ${rootfs}/usr/include/exiv2 ${soc-sdk}/include
# libeigen3-dev
cp -rf ${rootfs}/usr/include/eigen3 ${soc-sdk}/include
```

> At this point, the steps to prepare the cross-compilation environment and related dependencies are completed. Next, you can compile the programs that need to run on the SoC platform.
