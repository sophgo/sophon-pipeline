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

| 目录                   | 模块                                            | 功能说明                                                     |
| ---------------------- | ----------------------------------------------- | ------------------------------------------------------------ |
| [modules](./modules)   | [bmgui](./modules/bmgui)                        | 存放Sophon Pipeline用来GUI显示视频的模块                     |
|                        | [bmgui-lite](./modules/bmgui-lite)              | 由于SDK自带的OpenCV没有显示功能，此模块提供bm::imshow来显示视频 |
|                        | [bmutility](./modules/bmutility)                | 提供了基础库，字符串、定时器等                               |
|                        | [tracker](./modules/tracker)                    | 提供了CPU跟踪模块                                            |
| [examples](./examples) | [yolov5](./examples/yolov5)                     | yolov5s目标检测                                              |
|                        | [video_stitch](./examples/video_stitch)         | 4路yolov5s目标检测 + 拼接 + 编码 + RTSP服务                  |
|                        | [retinaface](./examples/retinaface)             | retinaface人脸检测                                           |
|                        | [multi](./examples/multi)                       | 并联运行两个yolov5目标检测                                   |
|                        | [face_recognition](./examples/face_recognition) | 串联运行人脸检测 + 人脸关键点 + 人脸特征提取                 |
|                        | [openpose](./examples/openpose) |openpose人体关键点检测               |
|                        | [face_detect](./examples/face_detect) | ssh_squeezenet人脸检测  |

**Sophon Pipeline的主要结构设计如下图：** 

![**avatar**](./docs/pics/sophon-pipeline.png)

**更新说明**

| 版本  | 说明                                                         |
| ----- | ------------------------------------------------------------ |
| 0.3.1 | 添加openpose、face_detect例程，适配1684x(x86 PCIe、SoC)，1684(x86 PCIe、SoC) |
| 0.3.0 | 添加multi、face_recognition例程，适配1684x(x86 PCIe、SoC)，1684(x86 PCIe、SoC) |
| 0.2.0 | 添加retinaface例程，适配1684x(x86 PCIe、SoC)，1684(x86 PCIe、SoC) |
| 0.1.2 | 添加yolov5、video_stitch例程，适配1684x(x86 PCIe、SoC)，1684(x86 PCIe、SoC) |

## 2 编译方法

### 2.1 环境准备

如果您在x86平台安装了PCIe加速卡，并使用它测试本例程，您需要安装 `libsophon`、`sophon-opencv`和`sophon-ffmpeg`。`libsophon`的安装可参考《LIBSOPHON使用手册.pdf》，`sophon-opencv`和`sophon-ffmpeg`的安装可参考《MULTIMEDIA使用手册.pdf》。注：需要获取《LIBSOPHON使用手册.pdf》和《MULTIMEDIA使用手册.pdf》，请联系技术支持。

### 2.2 依赖安装

sophon-pipeline主要依赖 

- libsophon
- sophon-ffmpeg
- sophon-opencv

具体版本依赖关系见下表：

| sophon-pipeline版本 | 依赖的libsophon版本 | 依赖的sophon-ffmpeg版本 | 依赖的sophon-opencv版本 |
| ------------------- | ------------------- | ----------------------- | ----------------------- |
| 0.3.1              | >=0.4.4             | >=0.5.1                 | >=0.5.1                 |
| 0.3.0               | >=0.4.3             | >=0.5.0                 | >=0.5.0                 |
| 0.2.0               | >=0.4.2             | >=0.4.0                 | >=0.4.0                 |
| 0.1.2               | >=0.4.1             | >=0.3.1                 | >=0.3.1                 |

#### 2.2.1 x86 PCIe平台

> Ubuntu 安装QT依赖：
````bash
sudo apt install -y qtbase5-dev
````

> Tracker 功能需要安装 Eigen 依赖：
```bash
sudo apt-get install -y libeigen3-dev
```

> 其他依赖：

```bash
sudo apt-get install -y libgflags-dev libgoogle-glog-dev libexiv2-dev
```

#### 2.2.2 arm SoC平台

对于arm SoC平台，内部已经集成了相应的libsophon、sophon-opencv和sophon-ffmpeg运行库包，位于`/opt/sophon/`下。

通常在x86主机上交叉编译程序，使之能够在arm SoC平台运行。您需要在x86主机上使用SOPHON SDK搭建交叉编译环境，将程序所依赖的头文件和库文件打包至soc-sdk目录中。

**交叉编译环境和相关依赖环境的准备步骤详见：**[arm_soc平台编译准备](./docs/arm_soc.md)。

> 如果遇到其他交叉编译问题，请参考《LIBSOPHON使用手册.pdf》的**4.2.2节 x86 交叉编译程序**章节。

### 2.3 编译指令

#### 2.3.1 各个平台编译

``` bash
# 若编译需要x86平台上运行的程序：
./tools/compile.sh x86 
# 若编译需要SoC平台上运行的程序，需要先根据2.2.2节准备好相关依赖，再运行下述命令进行编译：
./tools/compile.sh soc ${soc-sdk} 
```

编译完成后，demo程序将保存在`${SOPHON_PIPELINE}/release/${APP}/${PLATFORM}`文件夹下，若您是使用SoC平台，还需要将编译后的demo程序拷贝到SoC机器上运行。

## 3 运行方法

- [yolov5](./docs/yolov5.md)

- [video_stitch](./docs/video_stitch.md)

- [retinaface](./docs/retinaface.md)

- [multi](./docs/multi.md)

- [face_recognition](./docs/face_recognition.md)

- [openpose](./docs/openpose.md)

- [face_detect](./docs/face_detect.md)

## 4 FAQ

请参考[sophon-pipeline常见问题及解答](./docs/FAQ.md)
