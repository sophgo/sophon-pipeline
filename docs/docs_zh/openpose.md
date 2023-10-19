# openpose

## 1 概述

### 1.1 定义

- 使用pipeline运行openpose人体关键点检测。
- 当运行N路FPS为M的视频码流时，检测器`det`的FPS达到`N * M / (1 + [skip])`或者平均单路speed达到`M / (1 + [skip])`，其中`[skip]`为隔帧检测的跳帧数量。说明当前环境下，能够满足跳帧数为`[skip]`帧的N路FPS为M的视频码流的处理

## 2 编译

请参考[sophon-pipeline编译](../../README.md#23-编译指令)

## 3 运行

### 3.1 配置文件

运行请注意修改`${SOPHON_PIPELINE}/release/openpose_demo/cameras_openpose.json`配置：

```bash
{
  "cards": [													# 若需要配置多个device，可以在cards下添加多组devid和cameras信息
    {
      "devid": 0,												# 设备id
      "cameras": [												# 若需要配置多个视频码流，可以在cameras下添加多组address和chan_num信息。若配置了多个address或多个cards，总的视频码流路数为所有的[chan_num]数量之和
        {
          "address": "./elevator-1080p-25fps-4000kbps.h264",	# 需要测试视频码流的地址，如果是本地文件，只支持h264/h265格式
          "chan_num": 1,										# 将内容为上述[address]的视频码流配置[chan_num]数量的路数。默认设置为1，会接入1路的内容为上述[address]的视频码流。
          "model_names": ["ex1"]								# 测试该[address]视频码流的模型名称，需要和此配置文件下面的[models]参数内的模型自定义名称[name]一致，表示使用该模型，多个模型的名字用逗号分开。
        }
      ]
    },
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
      "path": "your_bmodel_path.bmodel",	        			# 对应[name]的bmodel模型的路径。模型必须与命令行参数[model_pose]配置模型一致。
      "skip_frame_num": 0,										# 隔帧检测的跳帧数量。当设置为0时表示程序不跳帧检测，当设置为1时表示程序每间隔1帧做一次模型的pipeline。
      "output_path": "output_path",                     		# 输出地址，只支持rtsp，tcp 格式为protocol://ip:port/, 例如rtsp://192.168.0.1:8554/test ， tcp://172.28.1.1:5353。对于rtsp推流，地址为rtsp server配置的地址。对于tcp，需要开放自己配置的端口。
      
      "obj_threshold": 0.6,										# 对应[path]的bmodel模型后处理的置信度阈值
      "nms_threshold": 0.5										# 对应[path]的bmodel模型后处理的非极大值抑制阈值
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
python3 -m dfss --url=sophon-pipeline/models/openpose.tar.gz 
```

参数说明

```bash
Usage: openpose_demo [params]
		--config (value:./cameras_openpose.json)
                cameras_openpose.json配置文件的路径，默认路径为./cameras_openpose.json。
        --help (value:true)
                打印帮助信息
        --model_pose (value:coco_18)
                选择使用的模型。body_25表示使用25 body parts模型，coco_18表示使用18 body parts模型。默认值为coco_18。此参数必须与json配置模型一致。
```

#### 3.2.1 x86 PCIe

**以设置`cameras_openpose.json`的`chan_num=1`为例**测试示例如下：

```bash
cd ${SOPHON_PIPELINE}/release/openpose_demo
# ./x86/openpose_demo --help 查看命令行帮助信息
# 以x86 pcie 1684x为例,将下载好的openpose模型拷贝到${SOPHON_PIPELINE}/release/openpose_demo目录下运行。当json文件配置使用的模型为coco_18模型时，需设置--model_pose参数为coco_18，当json文件配置使用的模型为body_25模型时，需设置--model_pose参数为body_25
./x86/openpose_demo --config=./cameras_openpose.json --model_pose=coco_18
```

执行会打印如下信息：

```bash
# 以x86 pcie 1684x为例
# 先打印出每路(1路)视频码流及对应芯片相关信息，再打印1路检测器det的总FPS和第0路视频码流处理对应的speed信息。其中，FPS和speed信息与当前运行设备的硬件配置相关，不同设备运行结果不同属正常现象，且同一设备运行程序过程中FPS和speed信息有一定波动属于正常现象。FPS和speed信息如下所示：

...
[2022-11-19:01:09:29] total fps =-nan,ch=0: speed=-nan
[2022-11-19:01:09:30] total fps =24.0,ch=0: speed=24.0
[2022-11-19:01:09:31] total fps =24.0,ch=0: speed=24.0
[2022-11-19:01:09:32] total fps =24.0,ch=0: speed=24.0
[2022-11-19:01:09:33] total fps =25.0,ch=0: speed=25.0
[2022-11-19:01:09:34] total fps =25.0,ch=0: speed=25.0
[2022-11-19:01:09:35] total fps =25.0,ch=0: speed=25.0
[2022-11-19:01:09:36] total fps =25.0,ch=0: speed=25.0
[2022-11-19:01:09:37] total fps =25.0,ch=0: speed=25.0
...
```

#### 3.2.2 arm SoC

将交叉编译好的`${SOPHON_PIPELINE}/release/openpose_demo`文件夹下的`cameras_openpose.json`、`soc`文件夹以及对应的模型、测试视频一起拷贝到arm SoC运行设备的同一目录下，并修改好cameras_openpose.json的相应配置，**以设置`cameras_openpose.json`的`chan_num=1`为例**，运行：

```bash
cd ${SOPHON_PIPELINE_OPENPOSE}
# ./soc/openpose_demo --help 查看命令行帮助信息
# 以arm SoC 1684x coco_18模型为例。当json文件配置使用的模型为coco_18模型时，需设置--model_pose参数为coco_18，当json文件配置使用的模型为body_25模型时，需设置--model_pose参数为body_25
./soc/openpose_demo --config=./cameras_openpose.json --model_pose=coco_18 
```

执行会打印如下信息：

```bash
# 以arm SoC 1684x为例
# 先打印出每路(1路)视频码流及对应芯片相关信息，再打印1路检测器det的总FPS和第0路视频码流处理对应的speed信息。其中，FPS和speed信息与当前运行设备的硬件配置相关，不同设备运行结果不同属正常现象，且同一设备运行程序过程中FPS和speed信息有一定波动属于正常现象。FPS和speed信息如下所示：

...
[2022-11-19:01:08:39] total fps =nan,ch=0: speed=nan
[2022-11-19:01:08:40] total fps =24.0,ch=0: speed=24.0
[2022-11-19:01:08:41] total fps =24.0,ch=0: speed=24.0
[2022-11-19:01:08:42] total fps =24.0,ch=0: speed=24.0
[2022-11-19:01:08:43] total fps =25.0,ch=0: speed=25.0
[2022-11-19:01:08:44] total fps =25.0,ch=0: speed=25.0
[2022-11-19:01:08:45] total fps =25.0,ch=0: speed=25.0
[2022-11-19:01:08:46] total fps =25.0,ch=0: speed=25.0
...
```

#### 3.2.3 arm PCIe

**以设置`cameras_openpose.json`的`chan_num=1`为例**测试示例如下：

```bash
cd ${SOPHON_PIPELINE}/release/openpose_demo
# ./arm64/openpose_demo --help 查看命令行帮助信息
# arm pcie 1684x为例,将下载好的openpose模型拷贝到${SOPHON_PIPELINE}/release/openpose_demo目录下运行。当json文件配置使用的模型为coco_18模型时，需设置--model_pose参数为coco_18，当json文件配置使用的模型为body_25模型时，需设置--model_pose参数为body_25
./arm64/openpose_demo --config=./cameras_openpose.json --model_pose=coco_18
```

执行会打印如下信息：

```bash
# 以arm pcie 1684x为例
# 先打印出每路(1路)视频码流及对应芯片相关信息，再打印1路检测器det的总FPS和第0路视频码流处理对应的speed信息。其中，FPS和speed信息与当前运行设备的硬件配置相关，不同设备运行结果不同属正常现象，且同一设备运行程序过程中FPS和speed信息有一定波动属于正常现象。FPS和speed信息如下所示：

...
[2023-03-22:19:09:29] total fps =-nan,ch=0: speed=-nan
[2023-03-22:19:09:30] total fps =24.0,ch=0: speed=24.0
[2023-03-22:19:09:31] total fps =24.0,ch=0: speed=24.0
[2023-03-22:19:09:32] total fps =24.0,ch=0: speed=24.0
[2023-03-22:19:09:33] total fps =25.0,ch=0: speed=25.0
[2023-03-22:19:09:34] total fps =25.0,ch=0: speed=25.0
[2023-03-22:19:09:35] total fps =25.0,ch=0: speed=25.0
[2023-03-22:19:09:36] total fps =25.0,ch=0: speed=25.0
[2023-03-22:19:09:37] total fps =25.0,ch=0: speed=25.0
...
```

### 3.3 可视化

- 使用[pipeline_client](./pipeline_client_visualization.md)显示实时流和检测结果
