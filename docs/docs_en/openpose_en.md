# openpose

## 1 Introduction

### 1.1 Definition

- Use pipeline to run openpose for human critical point detection.
- When running N video streams with FPS of M, the FPS of detector `det` reaches `N * M / (1 + [skip])` or the average single-way speed reaches `M / (1 + [skip])`, where `[skip]` is the number of frame skips for interframe detection. This means that the current environment can satisfy the processing of N video streams with FPS of M with `[skip]` frames.

## 2 Compilation

Please refer to [sophon-pipeline compile instructions](../../README_en.md#23-Compile instructions)

## 3 Operation

### 3.1Configuration file

Please note to modify the `${SOPHON_PIPELINE}/release/openpose_demo/cameras_openpose.json` configuration file before running：

```bash
{
  "cards": [													# If you need to configure multiple devices, you can add multiple groups of 'devid' and 'cameras' information under 'cards'
    {
      "devid": 0,												# device id
      "cameras": [												# If you need to configure multiple video streams, you can add multiple sets of [address] and [chan_num] information in [cameras]. If multiple [addresses] or multiple [cards] are configured, the total number of video streams is the sum of all [chan_num] numbers
        {
          "address": "./elevator-1080p-25fps-4000kbps.h264",	# Address of the video stream to be tested, if it is a local file, only h264/h265 formats are supported
          "chan_num": 1,										# Configure the number of [chan_num] channels for the video stream with the content of [address] above. The default setting is 1, which will access 1 video stream with the above [address] content.
          "model_names": ["ex1"]								# The model name for testing this [address] video stream needs to be the same as the model custom name [name] within the [models] parameter below this configuration file, indicating the use of this model, with multiple model names separated by commas.
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
      "name": "ex1",											# Custom name of the model corresponding to [path]
      "path": "your_bmodel_path.bmodel",	        			# The path to the bmodel model corresponding to [name]. The model must be the same as the command line parameter [model_pose] to configure the model.
      "skip_frame_num": 0,										# The number of frames to be skipped for inter-frame detection. When set to 0, the program does not skip frames, when set to 1, the program does a model pipeline every 1 frames.
      "output_path": "output_path",                     		#Output address, only support rtsp, tcp format is protocol://ip:port/, for example rtsp://192.168.0.1:8554/test, tcp://172.28.1.1:5353. for rtsp push stream, the address is the address configured by rtsp server. For tcp, you need to open the port you configured.
      
      "obj_threshold": 0.6,										# Confidence threshold for post-processing the bmodel model corresponding to [path]
      "nms_threshold": 0.5										# nms threshold for post-processing the bmodel model corresponding to [path]
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

Download address of the bmodels for testing
```bash
python3 -m dfss --url=sophon-pipeline/models/openpose.tar.gz 
```

Description of parameters

```bash
Usage: openpose_demo [params]
		--config (value:./cameras_openpose.json)
                cameras_openpose.json The path to the configuration file, the default path is ./cameras_openpose.json.
        --help (value:true)
                Print help information
        --model_pose (value:coco_18)
                Select the model to use. body_25 means use 25 body parts model, coco_18 means use 18 body parts model. The default value is coco_18. This parameter must be consistent with the json configuration model.
```

#### 3.2.1 x86 PCIe

**Take setting `cameras_openpose.json` with `chan_num=1` as an example** The test example is as follows：

```bash
cd ${SOPHON_PIPELINE}/release/openpose_demo
# ./x86/openpose_demo --help Print help information
# Take x86 pcie 1684x as an example, copy the downloaded openpose model to ${SOPHON_PIPELINE}/release/openpose_demo directory and run it. When the model configured in the json file is coco_18 model, you need to set the --model_pose parameter to coco_18, when the model configured in the json file is body_25 model, you need to set the --model_pose parameter to body_25
./x86/openpose_demo --config=./cameras_openpose.json --model_pose=coco_18
```

The following message will be printed after execution：

```bash
# Take x86 pcie 1684x as an example
# First, print out each video stream and the corresponding chip-related information, and then print the total FPS of the 1-channel detector det and the speed information corresponding to the processing of the 0th video stream. The FPS and speed information are related to the hardware configuration of the current running device, it is normal for different devices to run different results, and it is normal for the FPS and speed information to fluctuate during the running procedure of the same device.

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

Copy the `cameras_openpose.json`, `soc` folder and the corresponding models and test videos from the `${SOPHON_PIPELINE}/release/openpose_demo` folder to the same directory of the arm SoC running device, and modify the cameras_openpose.json. **Take `cameras_openpose.json` with `chan_num=1` as an example** and run：

```bash
cd ${SOPHON_PIPELINE_OPENPOSE}
# ./soc/openpose_demo --help Print help information
# Take arm Soc 1684x as an example, When the model configured in the json file is coco_18 model, you need to set the --model_pose parameter to coco_18, when the model configured in the json file is body_25 model, you need to set the --model_pose parameter to body_25
./soc/openpose_demo --config=./cameras_openpose.json --model_pose=coco_18
```

The following message will be printed after execution：

```bash
# Take arm SoC 1684x as an example
# First, print out each video stream and the corresponding chip-related information, and then print the total FPS of the 1-channel detector det and the speed information corresponding to the processing of the 0th video stream. The FPS and speed information are related to the hardware configuration of the current running device, it is normal for different devices to run different results, and it is normal for the FPS and speed information to fluctuate during the running procedure of the same device.

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

**Take setting `cameras_openpose.json` with `chan_num=1` as an example** The test example is as follows：

```bash
cd ${SOPHON_PIPELINE}/release/openpose_demo
# ./arm64/openpose_demo --help Print help information
# Take arm pcie 1684x as an example, copy the downloaded openpose model to ${SOPHON_PIPELINE}/release/openpose_demo directory and run it. When the model configured in the json file is coco_18 model, you need to set the --model_pose parameter to coco_18, when the model configured in the json file is body_25 model, you need to set the --model_pose parameter to body_25
./arm64/openpose_demo --config=./cameras_openpose.json --model_pose=coco_18
```

The following message will be printed after execution：

```bash
# Take arm pcie 1684x as an example
# First, print out each video stream and the corresponding chip-related information, and then print the total FPS of the 1-channel detector det and the speed information corresponding to the processing of the 0th video stream. The FPS and speed information are related to the hardware configuration of the current running device, it is normal for different devices to run different results, and it is normal for the FPS and speed information to fluctuate during the running procedure of the same device.

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

### 3.3 Visualization

- Use [pipeline_client](./pipeline_client_visualization_en.md) to display live streams and detection results
