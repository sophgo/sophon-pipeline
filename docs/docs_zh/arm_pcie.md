# arm_pcie平台编译准备

## 概述

本文档用于指导完成arm64机器的sophon-pipeline依赖环境安装。**通常x86_64机器对应的硬件架构为x86_64，arm64机器对应的硬件架构为aarch64**。

- libsophon、sophon-mw的安装请参考官方文档《LIBSOPHON使用手册》和《MULTIMEDIA使用手册》
- 其他环境依赖的安装请参考对应系统的安装说明

## aarch64 银河麒麟V10

以Kylin Linux Advanced Server V10 (Tercel)为例

**安装Eigen**

```bash
wget https://gitlab.com/libeigen/eigen/-/archive/3.4.0/eigen-3.4.0.tar.gz
tar -zxvf eigen-3.4.0.tar.gz
cd eigen-3.4.0
mkdir build && cd build
cmake ..
sudo make install
```

**安装gflags**

```bash
git clone https://github.com/gflags/gflags.git 
cd gflags
mkdir build && cd build
export CXXFLAGS="-fPIC" && cmake .. && make VERBOSE=1
make 
sudo make install
```

**安装glog**

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
# 以下操作为解决不同环境下找不到glog库的问题，用ls /usr查看是否存在lib64目录，如果存在，则执行以下步骤；如果不存在，则跳过以下步骤
sudo ln -s /usr/local/lib/libglog.so.0 /usr/lib64/libglog.so.0
```

**安装exiv2**

```bash
git clone https://github.com/Exiv2/exiv2.git
cd exiv2
git checkout v0.27.6
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
cmake --build .
sudo cmake --build . --target install
```

> 这里，aarch64 银河麒麟V10的编译环境和相关依赖环境的准备步骤已经准备完成，接下来可以编译需要在arm PCIe平台上运行的程序。

