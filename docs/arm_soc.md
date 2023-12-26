# arm_soc平台编译准备

## 1 概述

arm SoC平台，内部已经集成了相应的libsophon、sophon-opencv和sophon-ffmpeg运行库包，位于`/opt/sophon/`下。

通常在x86主机上交叉编译程序，使之能够在arm SoC平台运行。您需要在x86主机上使用SOPHON SDK搭建交叉编译环境，将程序所依赖的头文件和库文件打包至soc-sdk目录中。

## 2 编译环境准备

本章节需要提前准备libsophon-soc和sophon-mw包，请联系技术支持获取。

### 2.1 安装交叉编译工具链

```bash
sudo apt-get install gcc-aarch64-linux-gnu g++-aarch64-linux-gnu
```

### 2.2 准备libsophon

```bash
# 创建依赖文件的根目录
mkdir -p soc-sdk
# 解压sophon-img release包里的libsophon_soc_${x.y.z}_aarch64.tar.gz，其中x.y.z为版本号
tar -zxf libsophon_soc_${x.y.z}_aarch64.tar.gz
# 将相关的库目录和头文件目录拷贝到依赖文件根目录下
cp -rf libsophon_soc_${x.y.z}_aarch64/opt/sophon/libsophon-${x.y.z}/lib ${soc-sdk}
cp -rf libsophon_soc_${x.y.z}_aarch64/opt/sophon/libsophon-${x.y.z}/include ${soc-sdk}
```

### 2.3 准备ffmpeg和opencv

```bash
# 解压sophon-mw或sophon_media包里的sophon-mw-soc_${x.y.z}_aarch64.tar.gz，其中x.y.z为版本号
tar -zxf sophon-mw-soc_${x.y.z}_aarch64.tar.gz
# 将sophon-ffmpeg和official-opencv的库目录和头文件目录拷贝到依赖文件根目录下
cp -rf sophon-mw-soc_${x.y.z}_aarch64/opt/sophon/sophon-ffmpeg_${x.y.z}/lib ${soc-sdk}
cp -rf sophon-mw-soc_${x.y.z}_aarch64/opt/sophon/sophon-ffmpeg_${x.y.z}/include ${soc-sdk}
cd ${soc-sdk}
pip3 install dfss -i https://pypi.tuna.tsinghua.edu.cn/simple --upgrade
python3 -m dfss --url=open@sophgo.com:sophon-pipeline/a2_bringup/official_opencv.tar.gz 
tar xvf official_opencv.tar.gz
```

### 2.4 准备第三方库qtbase
可以自行编译公版qt，也可以下载我们准备好的qtbase库，下载方式如下：
```bash
python3 -m dfss --url=open@sophgo.com:sophon-pipeline/a2_bringup/qtbase.zip
sudo apt install unzip #如果有unzip可以跳过这步
unzip qtbase.zip #qtbase-5.14.2-aarch64
```
