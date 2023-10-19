# video_stitch

## 1 概述

### 1.1 定义

- 使用pipeline运行yolov5目标检测。
- 当运行N路FPS为M的视频码流时，检测器`det`的FPS达到`N * M / (1 + [skip])`或者平均单路speed达到`M / (1 + [skip])`，其中`[skip]`为隔帧检测的跳帧数量。说明当前环境下，能够满足跳帧数为`[skip]`帧的N路FPS为M的视频码流的处理
- 实现四路视频流的检测+拼接+编码+RTSP服务，显示四路视频流。

### 1.2 说明

**pcie模式下需要先下载准备vlc，使用vlc播放rtsp://xxxxx视频流** 

```bash
# 运行程序后将会打印以下信息，请根据实际程序打印的rtsp地址信息查看
# 示例：
...
BMTimerQueue ctor
LIVE555 Media Server
        version 0.92 (LIVE555 Streaming Media library version 2018.09.05).
Play streams from this server using the URL
        rtsp://172.17.69.66:1554/bitmain	# 请根据实际打印的rtsp地址信息查看
...
```

**soc模式下无可视化界面**

## 2 编译

请参考[sophon-pipeline编译](../../README.md#23-编译指令)

## 3 运行

### 3.1 配置文件

运行请注意修改`${SOPHON_PIPELINE}/release/video_stitch_demo/cameras_video_stitch.json`配置：

```bash
{
  "cards": [                      								# 若需要配置多个device，可以在cards下添加多组devid和cameras信息
    {
      "devid": 0,                  		 						# 设备id
      "cameras": [                    							# 若需要配置多个视频码流，可以在cameras下添加多组address和chan_num信息。若配置了多个address或多个cards，总的视频码流路数为所有的[chan_num]数量之和,必须小于等于4。当设置的视频路数不够4路时，2路时内部会各复制两路，3路时将第一路复制两路，其余做检测拼接。
        {
          "address": "./elevator-1080p-25fps-4000kbps.h264", 	# 需要测试视频码流的地址，如果是本地文件，只支持h264/h265格式
          "chan_num": 1,                						# 将内容为上述[address]的视频码流配置[chan_num]数量的路数。默认设置为1，会接入1路的内容为上述[address]的视频码流。
          "model_names": ["ex1"]            					# 测试该[address]视频码流的模型名称，需要和[models]参数内用户自定义的模型名称[name]一致，表示使用该模型
        }
      ]
    }，
  ],
  
  "pipeline": {                     							# pipeline中的线程数和队列长度
    "preprocess": {
      "thread_num": 4,                  						# 预处理线程数
      "queue_size": 16                  						# 预处理队列最大长度
    },
    "inference": {
      "thread_num": 1,                  						# 推理线程数
      "queue_size": 16                  						# 推理队列最大长度
    },
    "postprocess": {
      "thread_num": 4,                  						# 后处理线程数
      "queue_size": 16                  						# 后处理队列最大长度
    }
  },
  "models":[
    {
      "name": "ex1",                  							# 对应于[path]的模型用户自定义的名称,需要和[path]参数内的模型自定义名称[model_names]一致，表示使用该模型
      "path": "your_bmodel_path.bmodel",        				# 对应[name]的bmodel模型的路径
      "skip_frame_num": 1,                						# 隔帧检测的跳帧数量。当设置为1时表示程序每间隔1帧做一次模型的pipeline。
      
      "obj_threshold": 0.5,										# 对应[path]的bmodel模型后处理的物体置信度阈值
      "nms_threshold": 0.5,										# 对应[path]的bmodel模型后处理的非极大值抑制阈值
      "class_threshold": 0.5,									# 对应[path]的bmodel模型后处理的类别置信度阈值
      "class_num": 80											# 对应[path]的bmodel模型的分类数量
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
Usage: video_stitch_demo [params]

        --config (value:./cameras_video_stitch.json)
                cameras_video_stitch.json配置文件的路径，默认路径为./cameras_video_stitch.json。
        --help (value:true)
                打印帮助信息
        
```

#### 3.2.1 x86 PCIe

**以设置`cameras_video_stitch.json`的`chan_num=1`为例**测试示例如下：

```bash
# ./x86/video_stitch_demo --help 查看命令行帮助信息
cd ${SOPHON_PIPELINE}/release/video_stitch_demo
# x86模式下,将下载好的video_stitch模型拷贝到${SOPHON_PIPELINE}/release/video_stitch_demo目录下运行
./x86/video_stitch_demo --config=./cameras_video_stitch.json
```

执行命令后会打印如下信息：

```bash
# 以x86 pcie 1684x为例
# # 先打印出每路(4路)视频码流及对应芯片相关信息，再打印4路的总FPS信息。其中，FPS信息与当前运行设备的硬件配置相关，不同设备运行结果不同属正常现象，且同一设备运行程序过程中FPS信息有一定波动或vlc偶尔出现卡顿属于正常现象。
# FPS信息如下所示：

...
[2022-11-08:20:17:10] encode fps =-nan
[2022-11-08:20:17:11] encode fps =12.0
[2022-11-08:20:17:12] encode fps =12.0
[2022-11-08:20:17:13] encode fps =12.3
[2022-11-08:20:17:14] encode fps =12.5
[2022-11-08:20:17:15] encode fps =12.8
[2022-11-08:20:17:16] encode fps =12.5
[2022-11-08:20:17:17] encode fps =12.5
[2022-11-08:20:17:18] encode fps =12.5
[2022-11-08:20:17:19] encode fps =12.2
[2022-11-08:20:17:20] encode fps =12.8
[2022-11-08:20:17:21] encode fps =12.5
...
```

#### 3.2.2 arm SoC

交叉编译好的`${SOPHON_PIPELINE}/release/video_stitch_demo`文件夹下的`cameras_video_stitch.json`、`soc`文件夹以及对应的模型、测试视频一起拷贝到arm SoC运行设备的同一目录下，并修改好cameras_video_stitch.json的相应配置，运行：

```bash
cd ${SOPHON_PIPELINE_VIDEO_STITCH}
# ./soc/video_stitch_demo --help 查看命令行帮助信息
# 以arm SoC 1684为例
./soc/video_stitch_demo  --config=./cameras_video_stitch.json
```

执行会打印如下信息：

```bash
# 以arm SoC 1684为例
# 先打印出每路(4路)视频码流及对应芯片相关信息，再打印4路的总FPS信息。其中，FPS信息与当前运行设备的硬件配置相关，不同设备运行结果不同属正常现象，且同一设备运行程序过程中FPS信息有一定波动或vlc显示偶尔卡顿属于正常现象。FPS信息如下所示：

...
[2022-10-19:10:41:58] encode fps =nan
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

#### 3.2.3 arm PCIe

**以设置`cameras_video_stitch.json`的`chan_num=1`为例**测试示例如下：

```bash
# ./arm64/video_stitch_demo --help 查看命令行帮助信息
cd ${SOPHON_PIPELINE}/release/video_stitch_demo
# arm pcie模式下,将下载好的video_stitch模型拷贝到${SOPHON_PIPELINE}/release/video_stitch_demo目录下运行
./arm64/video_stitch_demo --config=./cameras_video_stitch.json
```

执行命令后会打印如下信息：

```bash
# arm pcie 1684x为例
# # 先打印出每路(4路)视频码流及对应芯片相关信息，再打印4路的总FPS信息。其中，FPS信息与当前运行设备的硬件配置相关，不同设备运行结果不同属正常现象，且同一设备运行程序过程中FPS信息有一定波动或vlc偶尔出现卡顿属于正常现象。
# FPS信息如下所示：

...
[2023-03-22:19:17:10] encode fps =-nan
[2023-03-22:19:17:11] encode fps =12.0
[2023-03-22:19:17:12] encode fps =12.0
[2023-03-22:19:17:13] encode fps =12.3
[2023-03-22:19:17:14] encode fps =12.5
[2023-03-22:19:17:15] encode fps =12.8
[2023-03-22:19:17:16] encode fps =12.5
[2023-03-22:19:17:17] encode fps =12.5
[2023-03-22:19:17:18] encode fps =12.5
[2023-03-22:19:17:19] encode fps =12.2
[2023-03-22:19:17:20] encode fps =12.8
[2023-03-22:19:17:21] encode fps =12.5
...
```