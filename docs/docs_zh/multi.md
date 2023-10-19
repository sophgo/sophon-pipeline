# multi

## 1 概述

### 1.1 定义

- 使用pipeline并联运行两个yolov5目标检测
- 当运行N路FPS为M的视频码流时，检测器`det`的FPS达到`N * M / (1 + [skip])`或者平均单路speed达到`M / (1 + [skip])`，其中`[skip]`为隔帧检测的跳帧数量。说明当前环境下，能够满足跳帧数为`[skip]`帧的N路FPS为M的视频码流的处理

## 2 编译

请参考[sophon-pipeline编译](../../README.md#23-编译指令)

## 3 运行

### 3.1 配置文件

运行请注意修改`${SOPHON_PIPELINE}/release/multi_demo/cameras_multi.json`配置：

```bash
{
  "cards": [													# 若需要配置多个device，可以在cards下添加多组devid和cameras信息
    {
      "devid": 0,												# 设备id
      "cameras": [												# 若需要配置多个视频码流，可以在cameras下添加多组address和chan_num信息。若配置了多个address或多个cards，总的视频码流路数为所有的[chan_num]数量之和
        {
          "address": "./elevator-1080p-25fps-4000kbps.h264",	# 需要测试视频码流的地址，如果是本地文件，只支持h264/h265格式
          "chan_num": 1,										# 将内容为上述[address]的视频码流配置[chan_num]数量的路数。默认设置为1，会接入1路的内容为上述[address]的视频码流。
          "model_names": ["ex1", "ex2"]							# 测试该[address]视频码流的模型名称，需要和此配置文件下面的[models]参数内的模型自定义名称[name]一致，表示使用该模型，多个模型的名字用逗号分开。本例程使用两个模型
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
      "name": "ex1",											# 对应于[path]=your_bmodel_1_path.bmodel的模型自定义名称
      "path": "your_bmodel_1_path.bmodel",						# 对应[name]=ex1的bmodel模型的路径
      "skip_frame_num": 1,										# 隔帧检测的跳帧数量。当设置为0时表示程序不跳帧检测，当设置为1时表示程序每间隔1帧做一次模型的pipeline。

      "obj_threshold": 0.5,										# 对应[path]=your_bmodel_1_path.bmodel的bmodel模型后处理的物体置信度阈值
      "nms_threshold": 0.5,										# 对应[path]=your_bmodel_1_path.bmodel的bmodel模型后处理的非极大值抑制阈值
      "class_threshold": 0.5,									# 对应[path]=your_bmodel_1_path.bmodel的bmodel模型后处理的类别置信度阈值
      "class_num": 80											# 对应[path]=your_bmodel_1_path.bmodel的bmodel模型的分类数量

    },
    {
      "name": "ex2",											# 对应于[path]=your_bmodel_2_path.bmodel的模型自定义名称
      "path": "your_bmodel_2_path.bmodel",						# 对应[name]=ex2的bmodel模型的路径
      "skip_frame_num": 1,										# 隔帧检测的跳帧数量。当设置为0时表示程序不跳帧检测，当设置为1时表示程序每间隔1帧做一次模型的pipeline。

      "obj_threshold": 0.5,										# 对应[path]=your_bmodel_2_path.bmodel的bmodel模型后处理的物体置信度阈值
      "nms_threshold": 0.5,										# 对应[path]=your_bmodel_2_path.bmodel的bmodel模型后处理的非极大值抑制阈值
      "class_threshold": 0.5,									# 对应[path]=your_bmodel_2_path.bmodel的bmodel模型后处理的类别置信度阈值
      "class_num": 80											# 对应[path]=your_bmodel_2_path.bmodel的bmodel模型的分类数量

    }
  ]
}
```

> **NOTE**  
> 
> 线程数和队列长度可根据设备情况自行定义。原则上，预处理线程数和后处理线程数设置为设备的逻辑CPU的个数。推理线程数单个pipeline一般为1。

### 3.2 运行方法

测试视频下载方式：
```bash
python3 -m dfss --url=open@sophgo.com:sophon-pipeline/common/elevator-1080p-25fps-4000kbps.h264
```

模型下载方式：
```bash
python3 -m dfss --url=sophon-pipeline/models/yolov5.tar.gz 
```

参数说明

```bash
Usage: multi_demo [params]

        --config (value:./cameras_multi.json)
                cameras_multi.json配置文件的路径，默认路径为./cameras_multi.json。
        --help (value:true)
                打印帮助信息
```

#### 3.2.1 x86 PCIe

**以设置`cameras_multi.json`的`chan_num=1`为例**测试示例如下：

```bash
cd ${SOPHON_PIPELINE}/release/multi_demo
# ./x86/multi_demo --help 查看命令行帮助信息
# 以x86 pcie 1684x为例,将下载好的yolov5模型拷贝到${SOPHON_PIPELINE}/release/multi_demo目录下运行
./x86/multi_demo --config=./cameras_multi.json
```

执行会打印如下信息：

```bash
# 以x86 pcie 1684x为例
# 先打印出每路(1路)视频码流及对应芯片相关信息，再打印1路检测器det的总FPS和第0路视频码流处理对应的speed信息。其中，FPS和speed信息与当前运行设备的硬件配置相关，不同设备运行结果不同属正常现象，且同一设备运行程序过程中FPS和speed信息有一定波动属于正常现象。FPS和speed信息如下所示：

...
[2022-11-19:02:04:25] total fps =-nan,ch=0: speed=-nan
[2022-11-19:02:04:26] total fps =24.0,ch=0: speed=12.0
[2022-11-19:02:04:27] total fps =24.0,ch=0: speed=12.0
[2022-11-19:02:04:28] total fps =24.0,ch=0: speed=12.0
[2022-11-19:02:04:29] total fps =25.0,ch=0: speed=13.0
[2022-11-19:02:04:30] total fps =26.0,ch=0: speed=13.0
[2022-11-19:02:04:31] total fps =26.0,ch=0: speed=13.0
[2022-11-19:02:04:32] total fps =26.0,ch=0: speed=13.0
...
```

#### 3.2.2 arm SoC

将交叉编译好的`${SOPHON_PIPELINE}/release/multi_demo`文件夹下的`cameras_multi.json`、`soc`文件夹以及对应的模型、测试视频一起拷贝到arm SoC运行设备的同一目录下，并修改好cameras_multi.json的相应配置，运行：

```bash
cd ${SOPHON_PIPELINE_MULTI}
# ./soc/multi_demo --help 查看命令行帮助信息
# 以arm SoC 1684x为例
./soc/multi_demo --config=./cameras_multi.json 
```

执行会打印如下信息：

```bash
# 以arm SoC 1684x为例
# 先打印出每路(1路)视频码流及对应芯片相关信息，再打印1路检测器det的总FPS和第0路视频码流处理对应的speed信息。其中，FPS和speed信息与当前运行设备的硬件配置相关，不同设备运行结果不同属正常现象，且同一设备运行程序过程中FPS和speed信息有一定波动属于正常现象。FPS和speed信息如下所示：

...
[2022-11-19:02:04:50] total fps =nan,ch=0: speed=nan
[2022-11-19:02:04:51] total fps =24.0,ch=0: speed=12.0
[2022-11-19:02:04:52] total fps =24.0,ch=0: speed=12.0
[2022-11-19:02:04:53] total fps =24.0,ch=0: speed=12.0
[2022-11-19:02:04:54] total fps =25.0,ch=0: speed=13.0
[2022-11-19:02:04:55] total fps =26.0,ch=0: speed=13.0
[2022-11-19:02:04:56] total fps =26.0,ch=0: speed=13.0
[2022-11-19:02:04:57] total fps =26.0,ch=0: speed=13.0
[2022-11-19:02:04:58] total fps =25.0,ch=0: speed=12.0
...
```

#### 3.2.3 arm PCIe

**以设置`cameras_multi.json`的`chan_num=1`为例**测试示例如下：

```bash
cd ${SOPHON_PIPELINE}/release/multi_demo
# ./arm64/multi_demo --help 查看命令行帮助信息
# 以arm pcie 1684x为例,将下载好的yolov5模型拷贝到${SOPHON_PIPELINE}/release/multi_demo目录下运行
./arm64/multi_demo --config=./cameras_multi.json
```

执行会打印如下信息：

```bash
# 以arm pcie 1684x为例
# 先打印出每路(1路)视频码流及对应芯片相关信息，再打印1路检测器det的总FPS和第0路视频码流处理对应的speed信息。其中，FPS和speed信息与当前运行设备的硬件配置相关，不同设备运行结果不同属正常现象，且同一设备运行程序过程中FPS和speed信息有一定波动属于正常现象。FPS和speed信息如下所示：

...
[2023-03-22:19:04:25] total fps =-nan,ch=0: speed=-nan
[2023-03-22:19:04:26] total fps =24.0,ch=0: speed=12.0
[2023-03-22:19:04:27] total fps =24.0,ch=0: speed=12.0
[2023-03-22:19:04:28] total fps =24.0,ch=0: speed=12.0
[2023-03-22:19:04:29] total fps =25.0,ch=0: speed=13.0
[2023-03-22:19:04:30] total fps =26.0,ch=0: speed=13.0
[2023-03-22:19:04:31] total fps =26.0,ch=0: speed=13.0
[2023-03-22:19:04:32] total fps =26.0,ch=0: speed=13.0
...
```

### 3.3 可视化

- 本例程不支持可视化
