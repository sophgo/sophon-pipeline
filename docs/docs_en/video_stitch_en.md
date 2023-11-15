# video_stitch

## 1 Introduction

### 1.1 Definition

- Run YOLOv5 object detection using pipeline.
- When running N video streams with FPS of M, the FPS of detector `det` reaches `N * M / (1 + [skip])` or the average single-way speed reaches `M / (1 + [skip])`, where `[skip]` is the number of frame skips for interframe detection. This means that the current environment can satisfy the processing of N video streams with FPS of M with `[skip]` frames.
- Realize detection + splicing + encoding + RTSP service to display four video streams.

### 1.2 Note

**pcie mode needs to download and prepare vlc first, and use vlc to play rtsp://xxxxx video stream**.

```bash
# The following information will be printed after running the program, please check the rtsp address information according to the actual program print
# 
...
BMTimerQueue ctor
LIVE555 Media Server
        version 0.92 (LIVE555 Streaming Media library version 2018.09.05).
Play streams from this server using the URL
        rtsp://172.17.69.66:1554/bitmain	# Please check according to the actual printed rtsp address information
...
```

**no visual interface in soc mode**

## 2 Compilation

Please refer to [sophon-pipeline compile instructions](../../README_en.md#23-Compile instructions)

## 3 Operation

### 3.1 Configuration file

Please note to modify the`${SOPHON_PIPELINE}/release/video_stitch_demo/cameras_video_stitch.json`configuration file before running：

```bash
{
  "cards": [                      								# If you need to configure multiple devices, you can add multiple groups of 'devid' and 'cameras' information under 'cards'
    {
      "devid": 0,                  		 						# device id
      "cameras": [                    							# If you need to configure multiple video streams, you can add multiple sets of [address] and [chan_num] information in [cameras]. If multiple [addresses] or multiple [cards] are configured, the total number of video streams is the sum of all [chan_num] numbers
        {
          "address": "./elevator-1080p-25fps-4000kbps.h264", 	# Address of the video stream to be tested, if it is a local file, only h264/h265 formats are supported
          "chan_num": 1,                						# Configure the number of [chan_num] channels for the video stream with the content of [address] above. The default setting is 1, which will access 1 video stream with the above [address] content.
          "model_names": ["ex1"]            					# The model name for testing this [address] video stream needs to be the same as the model custom name [name] within the [models] parameter below this configuration file, indicating the use of this model, with multiple model names separated by commas.
        }
      ]
    }，
  ],
  
  "pipeline": {                     							# Number of threads and queue length in pipeline
    "preprocess": {
      "thread_num": 4,                  						# Number of pre-processing threads
      "queue_size": 16                  						# Maximum length of pre-processing queue
    },
    "inference": {
      "thread_num": 1,                  						# Number of inference threads
      "queue_size": 16                  						# Maximum length of inference queue
    },
    "postprocess": {
      "thread_num": 4,                  						# Number of post-processing threads
      "queue_size": 16                  						# Maximum length of post-processing queue
    }
  },
  "models":[
    {
      "name": "ex1",											# Custom name of the model corresponding to [path]
      "path": "your_bmodel_path.bmodel",	        			# The path to the bmodel model corresponding to [name]. The model must be the same as the command line parameter [model_pose] to configure the model.
      "skip_frame_num": 0,										# The number of frames to be skipped for inter-frame detection. When set to 0, the program does not skip frames, when set to 1, the program does a model pipeline every 1 frames.
      "output_path": "output_path",                     		#Output address, only support rtsp, tcp format is protocol://ip:port/, for example rtsp://192.168.0.1:8554/test, tcp://172.28.1.1:5353. for rtsp push stream, the address is the address configured by rtsp server. For tcp, you need to open the port you configured.
      
      "obj_threshold": 0.5,										# Confidence threshold for post-processing the bmodel model corresponding to [path]
      "nms_threshold": 0.5										# nms threshold for post-processing the bmodel model corresponding to [path]
      "class_threshold": 0.5,									# class_threshold threshold for post-processing the bmodel model corresponding to [path]
      "class_num": 80											# The number of classes used to post-process the bmodel model, corresponding to [path].
    }
  ]
}
```

> **NOTE**  
>
> The number of threads and the length of the queue can be defined by the device itself. In principle, the number of pre-processing threads and post-processing threads is set to the number of logical CPUs of the device. The number of inference threads for a single pipeline is typically 1.

### 3.2 Running method

Download address of the video for testing:
```bash
python3 -m dfss --url=open@sophgo.com:sophon-pipeline/common/elevator-1080p-25fps-4000kbps.h264
```

Download address of the bmodels for testing:
```bash
python3 -m dfss --url=sophon-pipeline/models/yolov5.tar.gz 
```

Description of parameters

```bash
Usage: video_stitch_demo [params]

        --config (value:./cameras_video_stitch.json)
                cameras_video_stitch.json The path to the configuration file, the default path is ./cameras_video_stitch.json。
        --help (value:true)
                Print help information
        
```

#### 3.2.1 x86 PCIe

**Taking setting`cameras_video_stitch.json` with`chan_num=1`as an example** The test example is as follows：

```bash
# ./x86/video_stitch_demo --help Print the help information
cd ${SOPHON_PIPELINE}/release/video_stitch_demo
# Take x86 pcie 1684x as an example, copy the downloaded video_stitch model to ${SOPHON_PIPELINE}/release/video_stitch_demo directory and run it.
./x86/video_stitch_demo --config=./cameras_video_stitch.json
```

The following message will be printed after execution：

```bash
# Take x86 pcie 1684x as an example
# First print out each video stream (4 in total) and the corresponding chip related information, and then print out the total FPS information of the 4 channels. fps information is related to the hardware configuration of the currently running device, and it is normal for different devices to run the results, while it is normal for the same device to have some fluctuations in fps information or occasional lag in vlc during the running procedure.
# FPS information is shown below.

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

Copy the `cameras_video_stitch.json`, `soc` folder and the corresponding models and test videos from the cross-compiled `${SOPHON_PIPELINE}/release/video_stitch_demo` folder to the same directory of the arm SoC running device, and modify the corresponding configuration of cameras_video_stitch.json, and then run:

```bash
cd ${SOPHON_PIPELINE_VIDEO_STITCH}
# ./soc/video_stitch_demo --help Print the help information
# Take Soc 1684x as an example.
./soc/video_stitch_demo  --config=./cameras_video_stitch.json
```

The following message will be printed after execution：

```bash
# Take the arm SoC 1684 as an example
# First print out each video stream (4 in total) and the corresponding chip related information, and then print out the total FPS information of the 4 channels. fps information is related to the hardware configuration of the currently running device, and it is normal for different devices to run the results, while it is normal for the same device to have some fluctuations in fps information or occasional lag in vlc during the running procedure.FPS information is shown below.

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

**Taking setting`cameras_video_stitch.json` with`chan_num=1`as an example** The test example is as follows：

```bash
# ./arm64/video_stitch_demo --help Print the help information
cd ${SOPHON_PIPELINE}/release/video_stitch_demo
# Take arm pcie 1684x as an example, copy the downloaded video_stitch model to ${SOPHON_PIPELINE}/release/video_stitch_demo directory and run it.
./arm64/video_stitch_demo --config=./cameras_video_stitch.json
```

The following message will be printed after execution：

```bash
# Take arm pcie 1684x as an example
# First print out each video stream (4 in total) and the corresponding chip related information, and then print out the total FPS information of the 4 channels. fps information is related to the hardware configuration of the currently running device, and it is normal for different devices to run the results, while it is normal for the same device to have some fluctuations in fps information or occasional lag in vlc during the running procedure.
# FPS information is shown below.

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

