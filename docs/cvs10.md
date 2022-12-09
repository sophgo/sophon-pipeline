# CVS10

## 1 概述

### 1.1 定义

- 对一个1080P的视频码流进行目标检测和提取机非人的各种属性，称之为“CVS10”。CVS= Computer Vision Stream ；1 - 第一个典型应用 ；0 - 第0个版本

- 当运行N路FPS为M的视频码流时，检测器`det`的FPS达到`N * M / (1 + [skip])`或者平均单路speed达到`M / (1 + [skip])`，其中`[skip]`为隔帧检测的跳帧数量。说明当前环境下，能够满足跳帧数为`[skip]`帧的N路FPS为M的视频码流的处理

### 1.2 说明

**CVS10不支持画面显示。**

## 2 运行

### 2.1 配置文件

运行请注意修改`${SOPHON_PIPELINE}/release/cvs10/cameras_cvs.json`配置：

```bash
{
  "cards": [													# 若需要配置多个device，可以在cards下添加多组devid和cameras信息
    {
      "devid": 0,												# 设备id
      "cameras": [												# 若需要配置多个视频码流，可以在cameras下添加多组address和chan_num信息。若配置了多个address或多个cards，总的视频码流路数为所有的[chan_num]数量之和
        {
          "address": "./elevator-1080p-25fps-4000kbps.h264",	# 需要测试视频码流的地址
          "chan_num": 1,										# 将内容为上述[address]的视频码流配置[chan_num]数量的路数。默认设置为1，会接入1路的内容为上述[address]的视频码流。
          "model_names": ["ex1"]								# 测试该[address]视频码流的模型名称，需要和[models]参数内的模型自定义名称[name]一致，表示使用该模型
        }
      ]
    }，
  ],
  
  "pipeline": {													# pipeline中的线程数和队列长度
    "preprocess": {
      "thread_num": 4,											# 预处理线程数
      "queue_size": 16											# 预处理队列最大长度
    },
    "inference": {
      "thread_num": 1,											# 推理线程数
      "queue_size": 16											# 推理队列最大长度
    },
    "postprocess": {
      "thread_num": 4,											# 后处理线程数
      "queue_size": 16											# 后处理队列最大长度
    }
  },
  "models":[
    {
      "name": "ex1",											# 对应于[path]的模型自定义名称
      "path": "your_bmodel_path.bmodel",						# 对应[name]的bmodel模型的路径
      "skip_frame_num": 1,										# 隔帧检测的跳帧数量。当设置为1时表示程序每间隔1帧做一次模型的pipeline。
    }
  ]
}
```

> **NOTE**  
>
> 线程数和队列长度可根据设备情况自行定义。原则上，预处理线程数和后处理线程数设置为设备的逻辑CPU的个数。推理线程数单个pipeline一般为1。

### 2.2 运行方法

  > **NOTE**  
  > cvs10_1684X模型NAS云盘下载地址：[cvs10_1684x_int8_4b.bmodel](http://219.142.246.77:65000/sharing/v3MUZzdtv)
  >
  > cvs10_1684模型NAS云盘下载地址：[cvs10_1684_int8_4b.bmodel](http://219.142.246.77:65000/sharing/zuaXViy4z)
  >
  > 测试视频下载地址：[elevator-1080p-25fps-4000kbps.h264](http://219.142.246.77:65000/sharing/tU6pYuuau)

参数说明

```bash
Usage: cvs10 [params]

        --config (value:./cameras_cvs.json)
                cameras_cvs.json配置文件的路径，默认路径为./cameras_cvs.json。
        --feat_delay (value:1000)
                人脸特征提取器较人脸检测器启动延迟时间(毫秒)：默认设置为1000，表示人脸检测开始1000毫秒后，开启人脸特征提取
        --feat_num (value:8)
                每路视频码流提取的人脸特征数量，默认为8。
        --help (value:true)
                打印帮助信息
        --model_type (value:0)
                使用的模型类型。0: 使用face_detect方法 1: 使用resnet50方法。默认使用face_detect方法。
```

#### 2.2.1 x86 PCIe

**以设置`cameras_cvs.json`的`chan_num=32`为例**测试示例如下：

```bash
# ./x86/cvs10 --help 查看命行帮助信息
cd ${SOPHON_PIPELINE}/release/cvs10
# x86模式下,将下载好的cvs10模型拷贝到${SOPHON_PIPELINE}/release/cvs10目录下运行
./x86/cvs10 --config=./cameras_cvs.json
```

执行命令后会打印如下信息：

```bash
# 以x86 pcie 1684x为例
# 先打印出每路(32路)视频码流及对应芯片相关信息，再打印32路检测器det和人脸特征提取器feature的总FPS和第0路视频码流处理对应的speed信息。其中，FPS和speed信息与当前运行设备的硬件配置相关，不同设备运行结果不同属正常现象，且同一设备运行程序过程中FPS和speed信息有一定波动属于正常现象。
# FPS和speed信息如下所示：

...
[2022-09-08:20:29:20] det ([SUCCESS: 596/603]total fps =-nan,ch=0: speed=-nan) feature ([SUCCESS: 552/552]total fps=-nan,ch=0: speed=-nan)
[2022-09-08:20:29:21] det ([SUCCESS: 996/1003]total fps =400.0,ch=0: speed=12.0) feature ([SUCCESS: 792/799]total fps=240.0,ch=0: speed=8.0)
[2022-09-08:20:29:22] det ([SUCCESS: 1396/1403]total fps =400.0,ch=0: speed=12.5) feature ([SUCCESS: 1032/1036]total fps=240.0,ch=0: speed=8.0)
[2022-09-08:20:29:23] det ([SUCCESS: 1796/1803]total fps =400.0,ch=0: speed=12.3) feature ([SUCCESS: 1280/1280]total fps=242.7,ch=0: speed=8.0)
[2022-09-08:20:29:24] det ([SUCCESS: 2196/2203]total fps =400.0,ch=0: speed=12.5) feature ([SUCCESS: 1528/1528]total fps=244.0,ch=0: speed=8.0)
[2022-09-08:20:29:25] det ([SUCCESS: 2596/2602]total fps =400.0,ch=0: speed=12.5) feature ([SUCCESS: 1776/1776]total fps=246.0,ch=0: speed=8.0)
[2022-09-08:20:29:26] det ([SUCCESS: 2996/3003]total fps =400.0,ch=0: speed=12.5) feature ([SUCCESS: 2008/2021]total fps=244.0,ch=0: speed=8.0)
[2022-09-08:20:29:27] det ([SUCCESS: 3396/3403]total fps =400.0,ch=0: speed=12.5) feature ([SUCCESS: 2248/2248]total fps=242.0,ch=0: speed=8.0)
[2022-09-08:20:29:28] det ([SUCCESS: 3796/3803]total fps =400.0,ch=0: speed=12.5) feature ([SUCCESS: 2488/2492]total fps=240.0,ch=0: speed=6.0)
[2022-09-08:20:29:29] det ([SUCCESS: 4196/4203]total fps =400.0,ch=0: speed=12.5) feature ([SUCCESS: 2744/2744]total fps=242.0,ch=0: speed=6.0)
[2022-09-08:20:29:30] det ([SUCCESS: 4596/4603]total fps =400.0,ch=0: speed=12.5) feature ([SUCCESS: 3000/3000]total fps=248.0,ch=0: speed=6.0)
[2022-09-08:20:29:31] det ([SUCCESS: 4996/5003]total fps =400.0,ch=0: speed=12.5) feature ([SUCCESS: 3240/3242]total fps=248.0,ch=0: speed=6.0)
[2022-09-08:20:29:32] det ([SUCCESS: 5396/5403]total fps =400.0,ch=0: speed=12.5) feature ([SUCCESS: 3480/3482]total fps=248.0,ch=0: speed=8.0)
[2022-09-08:20:29:33] det ([SUCCESS: 5796/5803]total fps =400.0,ch=0: speed=12.5) feature ([SUCCESS: 3736/3736]total fps=248.0,ch=0: speed=8.0)
[2022-09-08:20:29:34] det ([SUCCESS: 6196/6203]total fps =400.0,ch=0: speed=12.5) feature ([SUCCESS: 3984/3991]total fps=246.0,ch=0: speed=8.0)
...
```

#### 2.2.2 arm SoC

交叉编译好的`${SOPHON_PIPELINE}/release/cvs10`文件夹下的`cameras_cvs.json`、`face.jpeg`、`soc`文件夹以及对应的模型、测试视频一起拷贝到arm SoC运行设备的同一目录下，并修改好cameras_cvs.json的相应配置，运行：

```bash
cd ${SOPHON_PIPELINE_CVS10}
# ./soc/cvs10 --help 查看命行帮助信息
# 以arm SoC 1684x为例
./soc/cvs10 --config=./cameras_cvs.json
```

执行会打印如下信息：

```bash
# 以arm SoC 1684x为例
# 先打印出每路(32路)视频码流及对应芯片相关信息，再打印32路检测器det和人脸特征提取器feature的总FPS和第0路视频码流处理对应的speed信息。其中，FPS和speed信息与当前运行设备的硬件配置相关，不同设备运行结果不同属正常现象，且同一设备运行程序过程中FPS和speed信息有一定波动属于正常现象。FPS和speed信息如下所示：

...
[2022-09-08:22:18:35] det ([SUCCESS: 2033/2042]total fps =nan,ch=0: speed=nan) feature ([SUCCESS: 1440/1440]total fps=nan,ch=0: speed=nan)
[2022-09-08:22:18:36] det ([SUCCESS: 2428/2448]total fps =395.0,ch=0: speed=12.0) feature ([SUCCESS: 1672/1682]total fps=232.0,ch=0: speed=8.0)
[2022-09-08:22:18:37] det ([SUCCESS: 2836/2841]total fps =401.5,ch=0: speed=12.5) feature ([SUCCESS: 1920/1920]total fps=240.0,ch=0: speed=8.0)
[2022-09-08:22:18:38] det ([SUCCESS: 3232/3249]total fps =399.7,ch=0: speed=12.3) feature ([SUCCESS: 2168/2169]total fps=242.7,ch=0: speed=8.0)
[2022-09-08:22:18:39] det ([SUCCESS: 3633/3641]total fps =400.0,ch=0: speed=12.5) feature ([SUCCESS: 2416/2423]total fps=244.0,ch=0: speed=8.0)
[2022-09-08:22:18:40] det ([SUCCESS: 4031/4048]total fps =400.8,ch=0: speed=12.5) feature ([SUCCESS: 2672/2672]total fps=250.0,ch=0: speed=8.0)
[2022-09-08:22:18:41] det ([SUCCESS: 4435/4441]total fps =399.8,ch=0: speed=12.5) feature ([SUCCESS: 2928/2928]total fps=252.0,ch=0: speed=8.0)
[2022-09-08:22:18:42] det ([SUCCESS: 4833/4848]total fps =400.2,ch=0: speed=12.5) feature ([SUCCESS: 3184/3184]total fps=254.0,ch=0: speed=8.0)
[2022-09-08:22:18:43] det ([SUCCESS: 5233/5243]total fps =399.9,ch=0: speed=12.5) feature ([SUCCESS: 3432/3440]total fps=253.9,ch=0: speed=8.0)
[2022-09-08:22:18:44] det ([SUCCESS: 5626/5648]total fps =398.8,ch=0: speed=12.5) feature ([SUCCESS: 3672/3672]total fps=250.0,ch=0: speed=8.0)
[2022-09-08:22:18:45] det ([SUCCESS: 6036/6042]total fps =400.2,ch=0: speed=12.5) feature ([SUCCESS: 3912/3912]total fps=246.0,ch=0: speed=8.0)
[2022-09-08:22:18:46] det ([SUCCESS: 6432/6449]total fps =399.8,ch=0: speed=12.5) feature ([SUCCESS: 4160/4161]total fps=244.0,ch=0: speed=8.0)
[2022-09-08:22:18:47] det ([SUCCESS: 6836/6842]total fps =400.9,ch=0: speed=12.5) feature ([SUCCESS: 4408/4408]total fps=244.1,ch=0: speed=8.0)
[2022-09-08:22:18:48] det ([SUCCESS: 7231/7248]total fps =401.2,ch=0: speed=12.5) feature ([SUCCESS: 4664/4664]total fps=248.0,ch=0: speed=8.0)
[2022-09-08:22:18:49] det ([SUCCESS: 7636/7641]total fps =400.0,ch=0: speed=12.5) feature ([SUCCESS: 4912/4912]total fps=250.0,ch=0: speed=8.0)
[2022-09-08:22:18:50] det ([SUCCESS: 8030/8048]total fps =399.5,ch=0: speed=12.5) feature ([SUCCESS: 5164/5168]total fps=251.0,ch=0: speed=8.0)
[2022-09-08:22:18:51] det ([SUCCESS: 8431/8441]total fps =398.8,ch=0: speed=12.5) feature ([SUCCESS: 5420/5424]total fps=253.0,ch=0: speed=8.0)
[2022-09-08:22:18:52] det ([SUCCESS: 8826/8848]total fps =398.8,ch=0: speed=12.5) feature ([SUCCESS: 5672/5677]total fps=252.0,ch=0: speed=8.0)
[2022-09-08:22:18:53] det ([SUCCESS: 9229/9241]total fps =398.2,ch=0: speed=12.5) feature ([SUCCESS: 5928/5928]total fps=254.0,ch=0: speed=8.0)
...
```

