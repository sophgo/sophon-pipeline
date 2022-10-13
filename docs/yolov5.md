# yolov5

## 1 概述

### 1.1 定义

- 使用pipeline运行yolov5目标检测。
- 当运行N路FPS为M的视频码流时，检测器`det`的FPS达到`N * M / (1 + [skip])`或者平均单路speed达到`M / (1 + [skip])`，其中`[skip]`为隔帧检测的跳帧数量。说明当前环境下，能够满足跳帧数为`[skip]`帧的N路FPS为M的视频码流的处理


## 2 运行

### 2.1 配置文件

运行请注意修改`${SOPHON_PIPELINE}/release/cvs11/cameras_yolov5.json`配置：

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
      "output_path": "output_path"                      # 输出地址
    }
  ]
}
```

> **NOTE**  
>
> 线程数和队列长度可根据设备情况自行定义。原则上，预处理线程数和后处理线程数设置为设备的逻辑CPU的个数。推理线程数单个pipeline一般为1。

### 2.2 运行方法

  > **NOTE**  
  > yolov5_1684X模型NAS云盘下载地址：/release/model_zoo/yolov5/BM1684X/yolov5s_640_coco_v6.1_3output_int8_1b.bmodel
  >
  > yolov5_1684模型NAS云盘下载地址：/release/model_zoo/yolov5/yolov5s_640_coco_v6.1_3output_int8_1b.bmodel
  >
  > 测试视频下载地址：[test.h264](http://219.142.246.77:65000/sharing/D5Y8Pkx44)

参数说明

```bash
Usage: yolov5s_demo [params]

        --config (value:./cameras_yolov5.json)
                cameras_yolov5.json配置文件的路径，默认路径为./cameras_yolov5.json。
        --help (value:true)
                打印帮助信息
```

#### 2.2.1 x86 PCIe

**以设置`cameras_yolov5.json`的`chan_num=16`为例**测试示例如下：

```bash
cd ${SOPHON_PIPELINE}/release/yolov5
# ./x86/yolov5 --help 查看命行帮助信息
# 以x86 pcie 1684x为例,将下载好的yolov5模型拷贝到${SOPHON_PIPELINE}/release/yolov5目录下运行
./x86/yolov5s_demo --config=./cameras_yolov5.json
```

执行会打印如下信息：

```bash
# 以x86 pcie 1684x为例
# 先打印出每路(16路)视频码流及对应芯片相关信息，再打印16路检测器det和人脸特征提取器feature的总FPS和第0路视频码流处理对应的speed信息。其中，FPS和speed信息与当前运行设备的硬件配置相关，不同设备运行结果不同属正常现象，且同一设备运行程序过程中FPS和speed信息有一定波动属于正常现象。FPS和speed信息如下所示：

...
[2022-10-13:10:22:13] det ([SUCCESS: 196/0]total fps =150.0,ch=0: speed=12.0) feature ([SUCCESS: 0/0]total fps=0.0,ch=0: speed=0.0)
[2022-10-13:10:22:14] det ([SUCCESS: 348/0]total fps =151.0,ch=0: speed=9.0) feature ([SUCCESS: 0/0]total fps=0.0,ch=0: speed=0.0)
[2022-10-13:10:22:15] det ([SUCCESS: 496/0]total fps =150.0,ch=0: speed=10.3) feature ([SUCCESS: 0/0]total fps=0.0,ch=0: speed=0.0)
[2022-10-13:10:22:16] det ([SUCCESS: 642/0]total fps =149.0,ch=0: speed=9.2) feature ([SUCCESS: 0/0]total fps=0.0,ch=0: speed=0.0)
[2022-10-13:10:22:17] det ([SUCCESS: 786/0]total fps =147.5,ch=0: speed=8.8) feature ([SUCCESS: 0/0]total fps=0.0,ch=0: speed=0.0)
[2022-10-13:10:22:18] det ([SUCCESS: 923/0]total fps =143.8,ch=0: speed=8.5) feature ([SUCCESS: 0/0]total fps=0.0,ch=0: speed=0.0)
[2022-10-13:10:22:19] det ([SUCCESS: 1048/0]total fps =138.0,ch=0: speed=7.8) feature ([SUCCESS: 0/0]total fps=0.0,ch=0: speed=0.0)
[2022-10-13:10:22:20] det ([SUCCESS: 1181/0]total fps =134.8,ch=0: speed=11.5) feature ([SUCCESS: 0/0]total fps=0.0,ch=0: speed=0.0)
[2022-10-13:10:22:21] det ([SUCCESS: 1316/0]total fps =132.5,ch=0: speed=11.0) feature ([SUCCESS: 0/0]total fps=0.0,ch=0: speed=0.0)
...
```

#### 2.2.2 arm SoC

将交叉编译好的`${SOPHON_PIPELINE}/release/yolov5`文件夹下的`cameras_yolov5.json`、`test.264`、`soc`文件夹以及对应的模型、测试视频一起拷贝到arm SoC运行设备的同一目录下，并修改好cameras_yolov5.json的相应配置，运行：

```bash
cd ${SOPHON_PIPELINE_YOLOV5}
# ./soc/yolov5 --help 查看命行帮助信息
# 以arm SoC 1684x为例,${xyz}表示1684或1684x
./soc/yolov5s_demo --config=./cameras_yolov5.json 
```

执行会打印如下信息：

```bash
# 以arm SoC 1684x为例
# 先打印出每路(16路)视频码流及对应芯片相关信息，再打印16路检测器det和人脸特征提取器feature的总FPS和第0路视频码流处理对应的speed信息。其中，FPS和speed信息与当前运行设备的硬件配置相关，不同设备运行结果不同属正常现象，且同一设备运行程序过程中FPS和speed信息有一定波动属于正常现象。FPS和speed信息如下所示：

...
[2022-10-13:10:25:39] det ([SUCCESS: 35/0]total fps =35.0,ch=0: speed=0.0) feature ([SUCCESS: 0/0]total fps=0.0,ch=0: speed=0.0)
[2022-10-13:10:25:40] det ([SUCCESS: 73/0]total fps =36.5,ch=0: speed=1.0) feature ([SUCCESS: 0/0]total fps=0.0,ch=0: speed=0.0)
[2022-10-13:10:25:41] det ([SUCCESS: 111/0]total fps =37.0,ch=0: speed=1.3) feature ([SUCCESS: 0/0]total fps=0.0,ch=0: speed=0.0)
[2022-10-13:10:25:42] det ([SUCCESS: 149/0]total fps =37.2,ch=0: speed=1.0) feature ([SUCCESS: 0/0]total fps=0.0,ch=0: speed=0.0)
[2022-10-13:10:25:43] det ([SUCCESS: 187/0]total fps =38.0,ch=0: speed=1.8) feature ([SUCCESS: 0/0]total fps=0.0,ch=0: speed=0.0)
[2022-10-13:10:25:44] det ([SUCCESS: 224/0]total fps =37.8,ch=0: speed=2.8) feature ([SUCCESS: 0/0]total fps=0.0,ch=0: speed=0.0)
[2022-10-13:10:25:45] det ([SUCCESS: 261/0]total fps =37.5,ch=0: speed=2.8) feature ([SUCCESS: 0/0]total fps=0.0,ch=0: speed=0.0)
[2022-10-13:10:25:46] det ([SUCCESS: 298/0]total fps =37.2,ch=0: speed=2.8) feature ([SUCCESS: 0/0]total fps=0.0,ch=0: speed=0.0)
[2022-10-13:10:25:47] det ([SUCCESS: 335/0]total fps =37.0,ch=0: speed=2.2) feature ([SUCCESS: 0/0]total fps=0.0,ch=0: speed=0.0)
[2022-10-13:10:25:48] det ([SUCCESS: 372/0]total fps =37.0,ch=0: speed=1.2) feature ([SUCCESS: 0/0]total fps=0.0,ch=0: speed=0.0)
[2022-10-13:10:25:49] det ([SUCCESS: 410/0]total fps =37.2,ch=0: speed=1.5) feature ([SUCCESS: 0/0]total fps=0.0,ch=0: speed=0.0)
[2022-10-13:10:25:50] det ([SUCCESS: 446/0]total fps =37.0,ch=0: speed=1.8) feature ([SUCCESS: 0/0]total fps=0.0,ch=0: speed=0.0)
[2022-10-13:10:25:51] det ([SUCCESS: 482/0]total fps =36.8,ch=0: speed=1.8) feature ([SUCCESS: 0/0]total fps=0.0,ch=0: speed=0.0)
...
```

