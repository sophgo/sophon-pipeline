# CVS11

## 1 概述

### 1.1 定义

- 对一个1080P的视频码流进行目标检测和提取机非人的各种属性，称之为“CVS11”。CVS= Computer Vision Stream ；1 - 第一个典型应用 ；1 - 第1个版本
- 当运行N路FPS为M的视频码流时，检测器`det`的FPS达到`N * M / (1 + [skip])`或者平均单路speed达到`M / (1 + [skip])`，其中`[skip]`为隔帧检测的跳帧数量。说明当前环境下，能够满足跳帧数为`[skip]`帧的N路FPS为M的视频码流的处理

### 1.2 说明

**CVS11不支持画面显示。**

## 2 运行

### 2.1 配置文件

运行请注意修改`${SOPHON_PIPELINE}/release/cvs11/cameras_cvs.json`配置：

```bash
{
  "cards": [											# 若需要配置多个device，可以在cards下添加多组devid和cameras信息
    {
      "devid": 0,										# 设备id
      "cameras": [										# 若需要配置多个视频码流，可以在cameras下添加多组address和chan_num信息。若配置了多个address或多个cards，总的视频码流路数为所有的[chan_num]数量之和
        {
          "address": "/data/workspace/media/face.h264",	# 需要测试视频码流的地址
          "chan_num": 1,								# 将内容为上述[address]的视频码流配置[chan_num]数量的路数。默认设置为1，会接入1路的内容为上述[address]的视频码流。
          "model_names": ["ex1"]						# 测试该[address]视频码流的模型名称，需要和[models]参数内的模型自定义名称[name]一致，表示使用该模型
        }
      ]
    }，
  ],
  
  "pipeline": {											# pipeline中的线程数和队列长度
    "preprocess": {
      "thread_num": 4,									# 预处理线程数
      "queue_size": 16									# 预处理队列最大长度
    },
    "inference": {
      "thread_num": 1,									# 推理线程数
      "queue_size": 16									# 推理队列最大长度
    },
    "postprocess": {
      "thread_num": 4,									# 后处理线程数
      "queue_size": 16									# 后处理队列最大长度
    }
  },
  "models":[
    {
      "name": "ex1",									# 对应于[path]的模型自定义名称
      "path": "your_bmodel_path.bmodel",				# 对应[name]的bmodel模型的路径
      "skip_frame_num": 1,								# 隔帧检测的跳帧数量。当设置为1时表示程序每间隔1帧做一次模型的pipeline。
    }
  ]
}
```

> **NOTE**  
>
> 线程数和队列长度可根据设备情况自行定义。原则上，预处理线程数和后处理线程数设置为设备的逻辑CPU的个数。推理线程数单个pipeline一般为1。

### 2.2 运行方法

  > **NOTE**  
  > cvs11_1684X模型NAS云盘下载地址：[cvs11_1684x_retinaface025_iresnet18_int8_4b.bmodel](http://219.142.246.77:65000/sharing/rt2I1oPAi)
  >
  > cvs11_1684模型NAS云盘下载地址：[cvs11_1684_retinaface025_iresnet18_int8_4b.bmodel](http://219.142.246.77:65000/sharing/6AJkOxzth)
  >
  > 测试视频下载地址：[station-1080p-25fps-2000kbps-3ref.h264](http://219.142.246.77:65000/sharing/8gTjG5lXB)

参数说明

```bash
Usage: cvs11 [params]

        --config (value:./cameras_cvs.json)
                cameras_cvs.json配置文件的路径，默认路径为./cameras_cvs.json。
        --enable_l2_ddr_reduction (value:1)
                是否使用L2 ddr reduction方法减少内存占用。0:不使用；1:使用。默认使用L2 ddr reduction方法。
        --feat_delay (value:1000)
                人脸特征提取器较人脸检测器启动延迟时间(毫秒)：默认设置为1000，表示人脸检测开始1000毫秒后，开启人脸特征提取
        --feat_num (value:8)
                每路视频码流每次检测后提取的人脸特征数量，默认为8。
        --help (value:true)
                打印帮助信息
```

#### 2.2.1 x86 PCIe

**以设置`cameras_cvs.json`的`chan_num=16`为例**测试示例如下：

```bash
cd ${SOPHON_PIPELINE}/release/cvs11
# ./x86/cvs11 --help 查看命行帮助信息
# 以x86 pcie 1684x为例,将下载好的cvs11模型拷贝到${SOPHON_PIPELINE}/release/cvs11目录下运行
./x86/cvs11 --config=./cameras_cvs.json
```

执行会打印如下信息：

```bash
# 以x86 pcie 1684x为例
# 先打印出每路(16路)视频码流及对应芯片相关信息，再打印16路检测器det和人脸特征提取器feature的总FPS和第0路视频码流处理对应的speed信息。其中，FPS和speed信息与当前运行设备的硬件配置相关，不同设备运行结果不同属正常现象，且同一设备运行程序过程中FPS和speed信息有一定波动属于正常现象。FPS和speed信息如下所示：

...
[2022-09-08:22:36:29] det ([SUCCESS: 228/231]total fps =-nan,ch=0: speed=-nan) feature ([SUCCESS: 240/241]total fps=-nan,ch=0: speed=-nan)
[2022-09-08:22:36:30] det ([SUCCESS: 428/432]total fps =200.0,ch=0: speed=13.0) feature ([SUCCESS: 360/366]total fps=120.0,ch=0: speed=8.0)
[2022-09-08:22:36:31] det ([SUCCESS: 628/631]total fps =200.0,ch=0: speed=12.5) feature ([SUCCESS: 480/488]total fps=120.0,ch=0: speed=8.0)
[2022-09-08:22:36:32] det ([SUCCESS: 828/832]total fps =200.0,ch=0: speed=12.7) feature ([SUCCESS: 604/608]total fps=121.3,ch=0: speed=8.0)
[2022-09-08:22:36:33] det ([SUCCESS: 1028/1031]total fps =200.0,ch=0: speed=12.5) feature ([SUCCESS: 728/728]total fps=122.0,ch=0: speed=8.0)
[2022-09-08:22:36:34] det ([SUCCESS: 1228/1232]total fps =200.0,ch=0: speed=12.5) feature ([SUCCESS: 852/856]total fps=123.0,ch=0: speed=8.0)
[2022-09-08:22:36:35] det ([SUCCESS: 1428/1431]total fps =200.0,ch=0: speed=12.5) feature ([SUCCESS: 968/968]total fps=122.0,ch=0: speed=8.0)
[2022-09-08:22:36:36] det ([SUCCESS: 1628/1632]total fps =200.0,ch=0: speed=12.5) feature ([SUCCESS: 1088/1088]total fps=121.0,ch=0: speed=8.0)
[2022-09-08:22:36:37] det ([SUCCESS: 1827/1831]total fps =199.8,ch=0: speed=12.5) feature ([SUCCESS: 1208/1208]total fps=120.0,ch=0: speed=8.0)
[2022-09-08:22:36:38] det ([SUCCESS: 2028/2032]total fps =200.0,ch=0: speed=12.5) feature ([SUCCESS: 1328/1328]total fps=119.0,ch=0: speed=8.0)
[2022-09-08:22:36:39] det ([SUCCESS: 2228/2231]total fps =200.0,ch=0: speed=12.5) feature ([SUCCESS: 1448/1448]total fps=120.0,ch=0: speed=8.0)
[2022-09-08:22:36:40] det ([SUCCESS: 2428/2432]total fps =200.0,ch=0: speed=12.5) feature ([SUCCESS: 1568/1568]total fps=120.0,ch=0: speed=8.0)
[2022-09-08:22:36:41] det ([SUCCESS: 2628/2631]total fps =200.2,ch=0: speed=12.5) feature ([SUCCESS: 1688/1688]total fps=120.0,ch=0: speed=8.0)
[2022-09-08:22:36:42] det ([SUCCESS: 2828/2832]total fps =200.0,ch=0: speed=12.5) feature ([SUCCESS: 1816/1816]total fps=122.0,ch=0: speed=8.0)
[2022-09-08:22:36:43] det ([SUCCESS: 3028/3031]total fps =200.0,ch=0: speed=12.5) feature ([SUCCESS: 1928/1928]total fps=120.0,ch=0: speed=8.0)
...
```

#### 2.2.2 arm SoC

将交叉编译好的`${SOPHON_PIPELINE}/release/cvs11`文件夹下的`cameras_cvs.json`、`face.jpeg`、`soc`文件夹以及对应的模型、测试视频一起拷贝到arm SoC运行设备的同一目录下，并修改好cameras_cvs.json的相应配置，运行：

```bash
cd ${SOPHON_PIPELINE_CVS11}
# ./soc/cvs11 --help 查看命行帮助信息
# 以arm SoC 1684x为例
./soc/cvs11 --config=./cameras_cvs.json
```

执行会打印如下信息：

```bash
# 以arm SoC 1684x为例
# 先打印出每路(16路)视频码流及对应芯片相关信息，再打印16路检测器det和人脸特征提取器feature的总FPS和第0路视频码流处理对应的speed信息。其中，FPS和speed信息与当前运行设备的硬件配置相关，不同设备运行结果不同属正常现象，且同一设备运行程序过程中FPS和speed信息有一定波动属于正常现象。FPS和speed信息如下所示：

...
[2022-09-08:22:39:07] det ([SUCCESS: 533/546]total fps =nan,ch=0: speed=nan) feature ([SUCCESS: 416/416]total fps=nan,ch=0: speed=nan)
[2022-09-08:22:39:08] det ([SUCCESS: 737/741]total fps =204.0,ch=0: speed=13.0) feature ([SUCCESS: 540/544]total fps=124.0,ch=0: speed=8.0)
[2022-09-08:22:39:09] det ([SUCCESS: 933/945]total fps =200.0,ch=0: speed=12.5) feature ([SUCCESS: 664/664]total fps=124.0,ch=0: speed=8.0)
[2022-09-08:22:39:10] det ([SUCCESS: 1135/1141]total fps =200.7,ch=0: speed=12.7) feature ([SUCCESS: 792/792]total fps=125.3,ch=0: speed=8.0)
[2022-09-08:22:39:11] det ([SUCCESS: 1332/1345]total fps =199.8,ch=0: speed=12.5) feature ([SUCCESS: 916/920]total fps=125.0,ch=0: speed=8.0)
[2022-09-08:22:39:12] det ([SUCCESS: 1537/1541]total fps =200.0,ch=0: speed=12.5) feature ([SUCCESS: 1032/1032]total fps=123.0,ch=0: speed=8.0)
[2022-09-08:22:39:13] det ([SUCCESS: 1732/1745]total fps =199.8,ch=0: speed=12.5) feature ([SUCCESS: 1156/1160]total fps=123.0,ch=0: speed=8.0)
[2022-09-08:22:39:14] det ([SUCCESS: 1935/1941]total fps =200.0,ch=0: speed=12.5) feature ([SUCCESS: 1272/1278]total fps=120.0,ch=0: speed=8.0)
[2022-09-08:22:39:15] det ([SUCCESS: 2133/2145]total fps =200.2,ch=0: speed=12.5) feature ([SUCCESS: 1400/1400]total fps=121.0,ch=0: speed=8.0)
[2022-09-08:22:39:16] det ([SUCCESS: 2334/2341]total fps =199.2,ch=0: speed=12.5) feature ([SUCCESS: 1528/1528]total fps=124.0,ch=0: speed=8.0)
[2022-09-08:22:39:17] det ([SUCCESS: 2530/2546]total fps =199.2,ch=0: speed=12.5) feature ([SUCCESS: 1652/1656]total fps=124.0,ch=0: speed=8.0)
[2022-09-08:22:39:18] det ([SUCCESS: 2736/2741]total fps =200.2,ch=0: speed=12.5) feature ([SUCCESS: 1768/1768]total fps=124.0,ch=0: speed=8.0)
[2022-09-08:22:39:19] det ([SUCCESS: 2932/2945]total fps =199.8,ch=0: speed=12.5) feature ([SUCCESS: 1888/1888]total fps=122.0,ch=0: speed=8.0)
[2022-09-08:22:39:20] det ([SUCCESS: 3137/3141]total fps =200.8,ch=0: speed=12.5) feature ([SUCCESS: 2016/2016]total fps=122.0,ch=0: speed=8.0)
...
```

