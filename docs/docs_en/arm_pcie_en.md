# Preparation for arm_pcie platform compilation

## Overview

This document is intended to guide the installation of sophon-pipeline dependencies on arm64 machines.**Usually, the hardware architecture corresponding to x86_64 machines is x86_64, while the hardware architecture corresponding to arm64 machines is aarch64.**。

- For the installation of libsophon and sophon-mw, please refer to the official documents "LIBSOPHON Manual" and "MULTIMEDIA User Manual".
- For the installation of other environment dependencies, please refer to the installation instructions for the corresponding system.

## aarch64 Kylin Linux Advanced Server V10 (Tercel)

Taking Kylin Linux Advanced Server V10 (Tercel) as an example.

**Install Eigen**

```bash
wget https://gitlab.com/libeigen/eigen/-/archive/3.4.0/eigen-3.4.0.tar.gz
tar -zxvf eigen-3.4.0.tar.gz
cd eigen-3.4.0
mkdir build && cd build
cmake ..
sudo make install
```

**Install gflags**

```bash
git clone https://github.com/gflags/gflags.git 
cd gflags
mkdir build && cd build
export CXXFLAGS="-fPIC" && cmake .. && make VERBOSE=1
make 
sudo make install
```

**Install glog**

```bash
git clone https://github.com/google/glog.git
cd glog/
git checkout -b main v0.4.0
./autogen.sh
./configure
make -j4
sudo make install	
# libraries have been installed in: /usr/local/lib
# create a symbolic link
# The following command is used to solve the issue which cound not find glog in different environment. Using 'ls /usr' to check whether the lib64 directory is existed. If lib64 is existed, run the following command. If lib64 is not existed, skip the following command.
sudo ln -s /usr/local/lib/libglog.so.0 /usr/lib64/libglog.so.0
```

**Install exiv2**

```bash
git clone https://github.com/Exiv2/exiv2.git
cd exiv2
git checkout v0.27.6
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
cmake --build .
sudo cmake --build . --target install
```

> At this stage, the preparation steps for the compilation environment and related dependencies of aarch64 Kylin Linux Advanced Server V10 (Tercel) have been completed. Next, you can compile the program that needs to run on the arm PCIe platform.

