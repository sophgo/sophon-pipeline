# sophon-pipeline介绍

## 1 简介

Sophon Pipeline提供一个简易的基于Pipeline的高性能加速框架，使用该框架能够将前处理/推理/后处理分别运行在3个线程上，最大化的实现并行, 用户只需要继承一个类，实现自己的前处理/后处理，就可以实现整个流程。

**Sophon Pipeline特性：**

- 单线程单模型
- 多线程单模型
- 多线程多模型
- 多模型串联
- 拉流、推流
- 视频结构可视化

**主要目录结构和模块说明：**

| 目录                   | 模块                               | 功能说明                                                     |
| ---------------------- | ---------------------------------- | ------------------------------------------------------------ |
| [modules](./modules)   | [bmgui](./modules/bmgui)           | 存放Sophon Pipeline用来GUI显示视频的模块                     |
|                        | [bmgui-lite](./modules/bmgui-lite) | 由于SDK自带的OpenCV没有显示功能，此模块提供bm::imshow来显示视频，作为补充。 |
|                        | [bmutility](./modules/bmutility)   | 提供了基础库，字符串、定时器等                               |
|                        | [tracker](./modules/tracker)       | 提供了CPU跟踪模块                                            |
| [examples](./examples) | [cvs10](./examples/cvs10)          | 提供了算能一路的参考实现                                     |
|                        | [cvs11](./examples/cvs11)          | 提供了算能一路的参考实现                                     |

**Sophon Pipeline的主要结构设计如下图：** 

![**avatar**](./docs/pics/sophon-pipeline.png)

## 2 编译方法

### 2.1 环境准备

如果您在x86平台安装了PCIe加速卡，并使用它测试本例程，您需要安装 `libsophon`、`sophon-opencv`和`sophon-ffmpeg`。`libsophon`的安装可参考《LIBSOPHON使用手册.pdf》，`sophon-opencv`和`sophon-ffmpeg`的安装可参考《MULTIMEDIA使用手册.pdf》。注：需要获取《LIBSOPHON使用手册.pdf》和《MULTIMEDIA使用手册.pdf》，请联系技术支持。

### 2.2 依赖安装

sophon-pipeline主要依赖 

- libsophon
- sophon-ffmpeg
- sophon-opencv

#### 2.2.1 x86 PCIe平台

> 相关依赖：
```bash
sudo apt-get install -y libgflags-dev libgoogle-glog-dev
```

#### 2.2.2 arm SoC平台

对于arm SoC平台，内部已经集成了相应的libsophon、sophon-opencv和sophon-ffmpeg运行库包，位于`/opt/sophon/`下。

通常在x86主机上交叉编译程序，使之能够在arm SoC平台运行。您需要在x86主机上使用SOPHON SDK搭建交叉编译环境，将程序所依赖的头文件和库文件打包至soc-sdk目录中。

**交叉编译环境和相关依赖环境的准备步骤详见：**[arm_soc平台编译准备](./docs/arm_soc.md)。

> 如果遇到其他交叉编译问题，请参考《LIBSOPHON使用手册.pdf》的**4.2.2节 x86 交叉编译程序**章节。

### 2.3 编译指令

#### 2.3.1 各个平台编译

```` bash
# 若编译需要x86平台上运行的程序：
./tools/compile.sh x86 
# 若编译需要SoC平台上运行的程序，需要先根据2.2.2节准备好相关依赖，再运行下述命令进行编译：
./tools/compile.sh soc ${soc-sdk} ${qt-sdk}
````

编译完成后，demo程序将保存在`${SOPHON_PIPELINE}/release/${APP}/${PLATFORM}`文件夹下。

## 3 运行方法

---
- [cvs20](./docs/cvs20.md)