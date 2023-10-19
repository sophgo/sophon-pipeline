简体中文 | [English](README_en.md)
# Note: 新的数据下载方式
目前sophon-pipeline所提供的模型、测试视频流等数据的下载方式由nas云盘变更为sftp，请安装以下依赖用于下载数据：
```
pip3 install dfss -i https://pypi.tuna.tsinghua.edu.cn/simple --upgrade
```
# sophon-pipeline介绍

## 1 简介

Sophon Pipeline提供一个简易的基于Pipeline的高性能加速框架，使用该框架能够将前处理/推理/后处理分别运行在3个线程上，最大化的实现并行, 用户只需要继承一个类，实现自己的前处理/后处理，就可以实现整个流程。

<details open>
<summary><b> Sophon Pipeline特性</b></summary>

- 单线程单模型
- 多线程单模型
- 多线程多模型
- 多模型串联
- 拉流、推流
- 视频结构可视化

</details>

**主要目录结构和模块说明：**

| 目录                   | 模块                                            | 功能说明                                                     |
| ---------------------- | ----------------------------------------------- | ------------------------------------------------------------ |
| [modules](./modules)   | [bmgui](./modules/bmgui)                        | 存放Sophon Pipeline用来GUI显示视频的模块                     |
|                        | [bmgui-lite](./modules/bmgui-lite)              | 由于SDK自带的OpenCV没有显示功能，此模块提供bm::imshow来显示视频 |
|                        | [bmutility](./modules/bmutility)                | 提供了基础库，字符串、定时器等                               |
|                        | [tracker](./modules/tracker)                    | 提供了CPU跟踪模块                                            |
| [examples](./examples) | [yolov5/v6/v7/v8](./examples/yolov5)            | yolo系列目标检测，支持yolov5、yolov6、yolov7、yolov8系列模型 |
|                        | [ppyoloe](./examples/ppyoloe)                   | PP-YOLOE系列目标检测，支持PP-YOLOE、PP-YOLOE+系列模型        |
|                        | [video_stitch](./examples/video_stitch)         | 4路yolov5s目标检测 + 拼接 + 编码 + RTSP服务                  |
|                        | [retinaface](./examples/retinaface)             | retinaface人脸检测                                           |
|                        | [multi](./examples/multi)                       | 并联运行两个yolov5目标检测                                   |
|                        | [face_recognition](./examples/face_recognition) | 串联运行人脸检测 + 人脸关键点 + 人脸特征提取                 |
|                        | [openpose](./examples/openpose)                 | openpose人体关键点检测                                       |
|                        | [face_detect](./examples/face_detect)           | ssh_squeezenet人脸检测                                       |
|                        | [yolact](./examples/yolact)                     | yolact检测                                                   |

**Sophon Pipeline的主要结构设计如下图：** 

![**avatar**](./docs/pics/sophon-pipeline.png)


<details>
<summary><b> 更新说明</b></summary>


| 版本  | 说明 |
|:---------: |:------------|
| **v0.3.7** | 添加yolact例程，适配1684x(x86 PCIe、SoC、arm PCIe)，1684(x86 PCIe、SoC、arm PCIe)；添加yolov5_opt、yolov7_opt模型 |
| **v0.3.5** | 添加ppyoloe例程，适配1684x(x86 PCIe、SoC、arm PCIe)，1684(x86 PCIe、SoC、arm PCIe) |
| **v0.3.4** | 添加yolov6、yolov7、yolov8例程，适配1684x(x86 PCIe、SoC)，1684(x86 PCIe、SoC)；添加1684x fp16模型；所有例程适配1684/1684X arm PCIe(银河麒麟V10) |
| **v0.3.1** | 添加openpose、face_detect例程，适配1684x(x86 PCIe、SoC)，1684(x86 PCIe、SoC) |
| **v0.3.0** | 添加multi、face_recognition例程，适配1684x(x86 PCIe、SoC)，1684(x86 PCIe、SoC) |
| **v0.2.0** | 添加retinaface例程，适配1684x(x86 PCIe、SoC)，1684(x86 PCIe、SoC) |
| **v0.1.2** | 添加yolov5、video_stitch例程，适配1684x(x86 PCIe、SoC)，1684(x86 PCIe、SoC) |

</details>



## 2 编译方法

### 2.1 环境准备

如果您在x86平台安装了PCIe加速卡z，并使用它测试本例程，您需要安装 `libsophon`、`sophon-opencv`和`sophon-ffmpeg`。`libsophon`的安装可参考《LIBSOPHON使用手册.pdf》，`sophon-opencv`和`sophon-ffmpeg`的安装可参考《MULTIMEDIA使用手册.pdf》。注：需要获取《LIBSOPHON使用手册.pdf》和《MULTIMEDIA使用手册.pdf》，请联系技术支持。

### 2.2 依赖安装

<details open>
<summary><b>sophon-pipeline主要依赖 </b></summary>

- libsophon
- sophon-ffmpeg
- sophon-opencv

</details>

<details>
<summary><b>具体版本依赖关系</b></summary>

| sophon-pipeline版本 | 依赖libsophon版本 | 依赖sophon-ffmpeg版本 | 依赖sophon-opencv版本 |
| :-----------------: | :---------------: | :-------------------: | :-------------------: |
|     **v0.3.7**      |      >=0.4.7      |        >=0.6.2        |        >=0.6.2        |
|     **v0.3.5**      |      >=0.4.6      |        >=0.6.0        |        >=0.6.0        |
|     **v0.3.4**      |      >=0.4.4      |        >=0.5.1        |        >=0.5.1        |
|     **v0.3.4**      |      >=0.4.4      |        >=0.5.1        |        >=0.5.1        |
|     **v0.3.1**      |      >=0.4.3      |        >=0.5.0        |        >=0.5.0        |
|     **v0.2.0**      |      >=0.4.2      |        >=0.4.0        |        >=0.4.0        |
|     **v0.1.2**      |      >=0.4.1      |        >=0.3.1        |        >=0.3.1        |

</details>


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

**交叉编译环境和相关依赖环境的准备步骤详见：**[arm_soc平台编译准备](./docs/docs_zh/arm_soc.md)。

> 如果遇到其他交叉编译问题，请参考《LIBSOPHON使用手册.pdf》的**4.2.2节 x86 交叉编译程序**章节。

#### 2.2.3 arm PCIe平台

对于arm PCIe平台，环境和相关依赖环境的准备步骤详见：[arm_pcie平台编译准备](./docs/docs_zh/arm_pcie.md)。

### 2.3 编译指令

#### 2.3.1 各个平台编译

``` bash
# 若编译需要x86平台上运行的程序，在x86平台机器上编译：
./tools/compile.sh x86 
# 若编译需要SoC平台上运行的程序，需要先根据2.2.2节准备好相关依赖，在x86平台机器上运行下述命令进行编译：
./tools/compile.sh soc ${soc-sdk} 
# 若编译需要arm64平台上运行的程序，在arm64平台机器上编译：
./tools/compile.sh arm64
```

编译完成后，demo程序将保存在`${SOPHON_PIPELINE}/release/${APP}/${PLATFORM}`文件夹下，若您是使用SoC平台，还需要将编译后的demo程序拷贝到SoC机器上运行。

## 3 运行方法

- [yolov5/v6/v7/v8](./docs/docs_zh/yolov5.md)
- [ppyoloe](./docs/docs_zh/ppyoloe.md)
- [video_stitch](./docs/docs_zh/video_stitch.md)
- [retinaface](./docs/docs_zh/retinaface.md)
- [multi](./docs/docs_zh/multi.md)
- [face_recognition](./docs/docs_zh/face_recognition.md)
- [openpose](./docs/docs_zh/openpose.md)
- [face_detect](./docs/docs_zh/face_detect.md)
- [yolact](./docs/docs_zh/yolact.md)

## 4 性能概览

<details>
<summary><b> SE7模型性能</b></summary>


|        例程及模型名称        | int8 inference(ms) | int8(FPS) | fp16 inference(ms) | fp16(FPS) |
| :--------------------------: | :----------------: | :-------: | :----------------: | :-------: |
|         **yolov5s**          |        3.29        |    182    |        6.27        |    129    |
|       **yolov5_opt🚀**        |        2.37        |    260    |        6.13        |    137    |
|         **yolov6s**          |        3.03        |    108    |        4.42        |    105    |
|          **yolov7**          |        8.93        |    98     |        22.5        |    40     |
|       **yolov7_opt🚀**        |        7.14        |    119    |       19.63        |    48     |
|         **yolov8s**          |        3.69        |    157    |        7.00        |    130    |
|        **ppyoloe_s**         |        5.39        |    167    |        8.46        |    115    |
|      **ppyoloe_plus_s**      |        5.10        |    160    |        7.86        |    115    |
|     **openpose_coco_18**     |        5.38        |    40     |       11.26        |    37     |
|     **openpose_body_25**     |        3.43        |    29     |        7.00        |    28     |
|       **yolact_base**        |       13.86        |    63     |         -          |     -     |
|     **yolact_darknet53**     |       12.96        |    68     |         -          |     -     |
|     **yolact_resnet50**      |       12.27        |    70     |         -          |     -     |
| **retinaface_mobilenet0.25** |        0.67        |   ≥500    |        0.81        |   ≥500    |
|       **face_detect**        |        1.16        |   ≥500    |        1.45        |   ≥500    |
|     **face_recognition**     |         -          |     -     |         -          |     -     |
|          **multi**           |         -          |     -     |         -          |     -     |
|       **video_stitch**       |         -          |     -     |         -          |     -     |

**测试说明：**

- 测试环境：SE7微服务器，8核cpu，cpu频率为2.3GHz，tpu频率为1000MHz，vpu频率为800MHz，jpu频率为800MHz。SDK版本为v23.03.01。测试视频和测试参数为各例程下默认视频和参数。
- inference只包含模型推理，不包含数据预处理和模型输出后处理部分，是平均一张图像推理耗时。
- FPS为各例程下SE7能运行的最大FPS值。

</details>

<details>
<summary><b> SE5模型性能</b></summary>


|        例程及模型名称        | int8 inference(ms) | int8(FPS) |
| :--------------------------: | :----------------: | :-------: |
|         **yolov5s**          |        6.21        |    131    |
|         **yolov6s**          |        8.01        |    90     |
|          **yolov7**          |       17.64        |    33     |
|         **yolov8s**          |        7.49        |    118    |
|        **ppyoloe_s**         |       20.49        |    46     |
|      **ppyoloe_plus_s**      |       19.62        |    50     |
|     **openpose_coco_18**     |        9.65        |    39     |
|     **openpose_body_25**     |        6.20        |    27     |
|       **yolact_base**        |       32.75        |    29     |
|     **yolact_darknet53**     |       30.15        |    31     |
|     **yolact_resnet50**      |       28.72        |    33     |
| **retinaface_mobilenet0.25** |        1.77        |   ≥475    |
|       **face_detect**        |        1.42        |   ≥475    |
|     **face_recognition**     |         -          |     -     |
|          **multi**           |         -          |     -     |
|       **video_stitch**       |         -          |     -     |

**测试说明：**

- 测试环境：SE5微服务器，8核cpu，cpu频率为2.3GHz，tpu频率为550MHz，vpu频率为640MHz，jpu频率为160MHz。SDK版本为v23.03.01。测试视频和测试参数为各例程下默认视频和参数。
- inference只包含模型推理，不包含数据预处理和模型输出后处理部分，是平均一张图像推理耗时。
- FPS为各例程下SE5能运行的最大FPS值。

</details>


## 5 FAQ

请参考[sophon-pipeline常见问题及解答](./docs/docs_zh/FAQ.md)
