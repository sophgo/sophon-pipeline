# multi

## 1 Introduction

### 1.1  Definition

- Run two yolov5 object detection in parallel using pipeline
- When running N video streams with FPS of M, the FPS of detector `det` reaches `N * M / (1 + [skip])` or the average single-way speed reaches `M / (1 + [skip])`, where `[skip]` is the number of frame skips for interframe detection. This means that the current environment can satisfy the processing of N video streams with FPS of M with `[skip]` frames.

## 2 Compilation

Please refer to [sophon-pipeline compile instructions](../../README_en.md#23-Compile instructions)

## 3 Operation

### 3.1  Configuration file

Please note to modify the`${SOPHON_PIPELINE}/release/multi_demo/cameras_multi.json`configuration file before running：

```bash
{
  "cards": [													# If you need to configure multiple devices, you can add multiple groups of 'devid' and 'cameras' information of 'cards'
    {
      "devid": 0,												# device id
      "cameras": [												# If you need to configure multiple video streams, you can add multiple sets of [address] and [chan_num] information in [cameras]. If multiple [addresses] or multiple [cards] are configured, the total number of video streams is the sum of all [chan_num] numbers
        {
          "address": "./elevator-1080p-25fps-4000kbps.h264",	# Address of the video stream to be tested, if it is a local file, only h264/h265 formats are supported
          "chan_num": 1,										# Configure the number of [chan_num] channels for the video stream with the content of [address] above. The default setting is 1, which will access 1 video stream with the above [address] content.
          "model_names": ["ex1", "ex2"]							# The model name for testing this [address] video stream needs to be the same as the model custom name [name] within the [models] parameter below this configuration file, indicating the use of this model, with multiple model names separated by commas.
        }
      ]
    }，
  ],
  
  "pipeline": {													# Number of threads and queue length in pipeline
    "preprocess": {
      "thread_num": 4,											# Number of pre-processing threads
      "queue_size": 16											# Maximum length of pre-processing queue
    },
    "inference": {
      "thread_num": 1,											# Number of inference threads
      "queue_size": 16											# Maximum length of inference queue
    },
    "postprocess": {
      "thread_num": 4,											# Number of post-processing threads
      "queue_size": 16											# Maximum length of post-processing queue
    }
  },
  "models":[
    {
      "name": "ex1",											# Custom name of the model corresponding to [path]==your_bmodel_1_path.bmodel
      "path": "your_bmodel_1_path.bmodel",						# The path to the bmodel model corresponding to [name]=ex1.
      "skip_frame_num": 1,										# The number of frames to be skipped for inter-frame detection. When set to 0, the program does not skip frames, when set to 1, the program does a model pipeline every 1 frames.

      "obj_threshold": 0.5,										# Confidence threshold for post-processing the bmodel model corresponding to [path]=your_bmodel_1_path.bmodel
      "nms_threshold": 0.5										# nms threshold for post-processing the bmodel model corresponding to [path]=your_bmodel_1_path.bmodel
      "class_num": 80,									# The number of classes used to post-process the bmodel model, corresponding to [path]=your_bmodel_1_path.bmodel

    },
    {
      "name": "ex2",											# The model custom name corresponding to [path]=your_bmodel_2_path.bmodel
      "path": "your_bmodel_2_path.bmodel",						# The path to the bmodel model corresponding to [name]=ex2.
      "skip_frame_num": 1,										# The number of frames to be skipped for inter-frame detection. When set to 0, the program does not skip frames, when set to 1, the program does a model pipeline every 1 frames.
      "output_path": "output_path",                     		#Output address, only support rtsp, tcp format is protocol://ip:port/, for example rtsp://192.168.0.1:8554/test, tcp://172.28.1.1:5353. for rtsp push stream, the address is the address configured by rtsp server. For tcp, you need to open the port you configured.
      
      "obj_threshold": 0.5,										# Confidence threshold for post-processing the bmodel model corresponding to [path]=your_bmodel_2_path.bmodel
      "nms_threshold": 0.5										# nms threshold for post-processing the bmodel model corresponding to [path]=your_bmodel_2_path.bmodel
      "class_num": 80,									# The number of classes used to post-process the bmodel model, corresponding to [path]=your_bmodel_2_path.bmodel

    }
  ]
}
```

> **NOTE**  
>
> The number of threads and queue length can be defined by the device itself. In principle, the number of pre-processing threads and post-processing threads is set to the number of logical CPUs of the device. The number of inference threads for a single pipeline is typically 1.

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
Usage: multi_demo [params]

        --config (value:./cameras_multi.json)
                cameras_multi.json The path to the configuration file, the default path is ./cameras_multi.json。
        --help (value:true)
                Print help information
```

#### 3.2.1 x86 PCIe

**Take setting `cameras_multi.json` with `chan_num=1` as an example** The test example is as follows：

```bash
cd ${SOPHON_PIPELINE}/release/multi_demo
# ./x86/multi_demo --help Print help information
# Take x86 pcie 1684x as an example, copy the downloaded yolov5 model to ${SOPHON_PIPELINE}/release/multi_demo directory and run it.
./x86/multi_demo --config=./cameras_multi.json
```

The following message is printed when executed:

```bash
# Take x86 pcie 1684x as an example
# First, print out each video stream and the corresponding chip-related information, and then print the total FPS of the 1-channel detector det and the speed information corresponding to the processing of the 0th video stream. The FPS and speed information are related to the hardware configuration of the current running device, it is normal for different devices to run different results, and it is normal for the FPS and speed information to fluctuate during the running procedure of the same device.

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

Copy the `cameras_multi.json`, `soc` folder and the corresponding models and test videos from the cross-compiled `${SOPHON_PIPELINE}/release/multi_demo` folder to the same directory of the arm SoC running device. Please modify the corresponding configuration of cameras_ yolov5.json, and then run the command line:

```bash
cd ${SOPHON_PIPELINE_MULTI}
# ./soc/multi_demo --help Print help information
# Take Soc 1684x as an example.
./soc/multi_demo --config=./cameras_multi.json 
```

The following message will be printed after execution：

```bash
# Take the arm SoC 1684x as an example
# First, print out each video stream and the corresponding chip-related information, and then print the total FPS of the 1-channel detector det and the speed information corresponding to the processing of the 0th video stream. The FPS and speed information are related to the hardware configuration of the current running device, it is normal for different devices to run different results, and it is normal for the FPS and speed information to fluctuate during the running procedure of the same device.


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

**Take setting `cameras_multi.json` with `chan_num=1` as an example** The test example is as follows：

```bash
cd ${SOPHON_PIPELINE}/release/multi_demo
# ./arm64/multi_demo --help Print help information
# Take arm pcie 1684x as an example, copy the downloaded yolov5 model to ${SOPHON_PIPELINE}/release/multi_demo directory and run it.
./arm64/multi_demo --config=./cameras_multi.json
```

The following message is printed when executed:

```bash
# Take arm pcie 1684x as an example
# First, print out each video stream and the corresponding chip-related information, and then print the total FPS of the 1-channel detector det and the speed information corresponding to the processing of the 0th video stream. The FPS and speed information are related to the hardware configuration of the current running device, it is normal for different devices to run different results, and it is normal for the FPS and speed information to fluctuate during the running procedure of the same device.

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

### 3.3 Visualization

- This demo does not support visualization