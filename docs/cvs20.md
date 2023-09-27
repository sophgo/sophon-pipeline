# CVS20

## 1 概述

### 1.1 定义

- 对一个1080P的视频码流进行人脸检测和提取特征，称之为“CVS20”。CVS= Computer Vision Stream ；2 - 第二个典型应用 ；0 - 第0个版本

- 当运行N路FPS为M的视频码流时，检测器`det`的FPS达到`N * M `或者平均单路speed达到`M`，说明当前环境下，能够满足跳帧数为`[skip]`帧的N路FPS为M的视频码流的处理

## 2 运行

### 2.1 配置文件

运行请注意修改`${SOPHON_PIPELINE}/release/cvs20/cameras_cvs.json`配置：

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
  > cvs20测试程序包下载，请放到对应的bm1688 evb soc板子上面，里面有说明文件`readme.md`，可以根据readme操作：
    ```bash
    pip3 install dfss -i https://pypi.tuna.tsinghua.edu.cn/simple --upgrade
    python3 -m dfss --url=open@sophgo.com:sophon-pipeline/a2_bringup/test_pack_cvs20_0922.tar
    tar xvf test_pack_cvs20_0922.tar
    cd test_pack_cvs20_0922
    ./setup.sh <exe> <chan_num> # <exe>即可执行程序，如果想要测试自己编译出来的可执行程序，直接用`${SOPHON_PIPELINE}/test_execs/`下的程序替换即可。
    ```
