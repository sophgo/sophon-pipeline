# video_stitch

## 1 概述

### 1.1 定义

- 使用pipeline运行yolov5目标检测。
- 当运行N路FPS为M的视频码流时，检测器`det`的FPS达到`N * M / (1 + [skip])`或者平均单路speed达到`M / (1 + [skip])`，其中`[skip]`为隔帧检测的跳帧数量。说明当前环境下，能够满足跳帧数为`[skip]`帧的N路FPS为M的视频码流的处理
- 实现四路视频流的检测+拼接+编码+RTSP服务，显示四路视频流。

### 1.2 说明

**pcie模式下需要先下载准备vlc，使用vlc播放rtsp://xxxxx视频流** 

```bash
...
BMTimerQueue ctor
LIVE555 Media Server
        version 0.92 (LIVE555 Streaming Media library version 2018.09.05).
Play streams from this server using the URL
        rtsp://172.17.69.66:1554/bitmain
...
```

**soc模式下无可视化界面**

## 2 运行

### 2.1 配置文件

运行请注意修改`${SOPHON_PIPELINE}/release/video_stitch_demo/cameras_cvs.json`配置：

```bash
{
  "cards": [                      # 若需要配置多个device，可以在cards下添加多组devid和cameras信息
    {
      "devid": 0,                   # 设备id
      "cameras": [                    # 若需要配置多个视频码流，可以在cameras下添加多组address和chan_num信息。若配置了多个address或多个cards，总的视频码流路数为所有的[chan_num]数量之和,必须等于4
        {
          "address": "/data/workspace/media/face.h264", # 需要测试视频码流的地址
          "chan_num": 1,                # 将内容为上述[address]的视频码流配置[chan_num]数量的路数。默认设置为1，会接入1路的内容为上述[address]的视频码流。
          "model_names": ["ex1"]            # 测试该[address]视频码流的模型名称，需要和[models]参数内用户自定义的模型名称[name]一致，表示使用该模型
        }
      ]
    }，
  ],
  
  "pipeline": {                     # pipeline中的线程数和队列长度
    "preprocess": {
      "thread_num": 4,                  # 预处理线程数
      "queue_size": 16                  # 预处理队列最大长度
    },
    "inference": {
      "thread_num": 1,                  # 推理线程数
      "queue_size": 16                  # 推理队列最大长度
    },
    "postprocess": {
      "thread_num": 4,                  # 后处理线程数
      "queue_size": 16                  # 后处理队列最大长度
    }
  },
  "models":[
    {
      "name": "ex1",                  # 对应于[path]的模型用户自定义的名称,需要和[path]参数内的模型自定义名称[model_names]一致，表示使用该模型
      "path": "your_bmodel_path.bmodel",        # 对应[name]的bmodel模型的路径
      "skip_frame_num": 1,                # 隔帧检测的跳帧数量。当设置为1时表示程序每间隔1帧做一次模型的pipeline。
    }
  ]
}
```

> **NOTE**  
>
> 线程数和队列长度可根据设备情况自行定义。原则上，预处理线程数和后处理线程数设置为设备的逻辑CPU的个数。推理线程数单个pipeline一般为1。

### 2.2 运行方法

  > **NOTE**  
  > video_stitch_1684X模型NAS云盘下载地址：[yolov5s-640x384-int8-4b-1684x.bmodel](http://219.142.246.77:65000/sharing/eEe5HvnHQ)
  >
  > video_stitch_1684模型NAS云盘下载地址：[yolov5s-640x640-int8-4b.bmodel](http://219.142.246.77:65000/sharing/lMhYaEZZL)
  >
  > 测试视频下载地址：[station-1080p-25fps-2000kbps-3ref.h264](http://219.142.246.77:65000/sharing/8gTjG5lXB)

参数说明

```bash
Usage: video_stitch_demo [params]

        --config (value:./cameras_cvs.json)
                cameras_cvs.json配置文件的路径，默认路径为./cameras_cvs.json。
        --help (value:true)
                打印帮助信息
        
```

#### 2.2.1 x86 PCIe

**以设置`cameras_cvs.json`的`chan_num=1`为例**测试示例如下：

```bash
# ./x86/video_stitch_demo --help 查看命行帮助信息
cd ${SOPHON_PIPELINE}/release/video_stitch_demo
# x86模式下,将下载好的video_stitch模型拷贝到${SOPHON_PIPELINE}/release/video_stitch_demo目录下运行
./x86/video_stitch_demo --config=./cameras_cvs.json
```

执行命令后会打印如下信息：

```bash
# 以x86 pcie 1684x为例
# # 先打印出每路(4路)视频码流及对应芯片相关信息，再打印4路的总FPS信息。其中，FPS信息与当前运行设备的硬件配置相关，不同设备运行结果不同属正常现象，且同一设备运行程序过程中FPS信息有一定波动或vlc偶尔出现卡顿属于正常现象。
# FPS信息如下所示：

...
[2022-10-13:18:55:10] encode fps =-nan
[2022-10-13:18:59:29] encode fps =19.5
[2022-10-13:18:59:30] encode fps =18.8
[2022-10-13:18:59:31] encode fps =20.2
[2022-10-13:18:59:32] encode fps =19.5
[2022-10-13:18:59:33] encode fps =20.5
[2022-10-13:18:59:34] encode fps =20.8
[2022-10-13:18:59:35] encode fps =21.0
[2022-10-13:18:59:36] encode fps =23.2
[2022-10-13:18:59:37] encode fps =23.0
[2022-10-13:18:59:38] encode fps =25.5
[2022-10-13:18:55:11] encode fps =24.0
[2022-10-13:18:55:12] encode fps =24.5
[2022-10-13:18:55:13] encode fps =23.3
[2022-10-13:18:55:14] encode fps =24.5
...
```

#### 2.2.2 arm SoC

交叉编译好的`${SOPHON_PIPELINE}/release/video_stitch_demo`文件夹下的`cameras_cvs.json`、`soc`文件夹以及对应的模型、测试视频一起拷贝到arm SoC运行设备的同一目录下，并修改好cameras_cvs.json的相应配置，运行：

```bash
cd ${SOPHON_PIPELINE_VIDEO_STITCH}
# ./soc/video_stitch_demo --help 查看命行帮助信息
# 以arm SoC 1684为例
./soc/video_stitch_demo  --config=./cameras_cvs.json
```

执行会打印如下信息：

```bash
# 以arm SoC 1684为例
# 先打印出每路(4路)视频码流及对应芯片相关信息，再打印4路的总FPS信息。其中，FPS信息与当前运行设备的硬件配置相关，不同设备运行结果不同属正常现象，且同一设备运行程序过程中FPS信息有一定波动或vlc显示偶尔卡顿属于正常现象。FPS信息如下所示：

...
[2022-10-19:10:41:59] encode fps =1.0
[2022-10-19:10:42:00] encode fps =3.2
[2022-10-19:10:42:01] encode fps =4.0
[2022-10-19:10:42:02] encode fps =6.5
[2022-10-19:10:42:03] encode fps =8.0
[2022-10-19:10:42:04] encode fps =8.5
[2022-10-19:10:42:05] encode fps =8.0
[2022-10-19:10:42:06] encode fps =7.5
[2022-10-19:10:42:07] encode fps =6.0
[2022-10-19:10:42:08] encode fps =6.0
[2022-10-19:10:42:09] encode fps =6.0
[2022-10-19:10:42:10] encode fps =7.2
[2022-10-19:10:42:11] encode fps =8.0
[2022-10-19:10:42:12] encode fps =8.2
[2022-10-19:10:42:13] encode fps =8.0
...
```
