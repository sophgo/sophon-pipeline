# face_detect

## 1  Introduction

### 1.1 Definition

- Use pipeline to run face_detect face detection.
- When running N video streams with FPS of M, the FPS of detector `det` reaches `N * M / (1 + [skip])` or the average single-way speed reaches `M / (1 + [skip])`, where `[skip]` is the number of frame skips for interframe detection. This means that the current environment can satisfy the processing of N video streams with FPS of M with `[skip]` frames.

## 2 Compilation

Please refer to [sophon-pipeline compile instructions](../../README_en.md#23-Compile instructions)

## 3 Operation

### 3.1 Configuration file

Please note to modify the`${SOPHON_PIPELINE}/release/facedetect_demo/cameras_face_detect.json`configuration file before running：

```bash
{
  "cards": [													# If you need to configure multiple devices, you can add multiple groups of 'devid' and 'cameras' information of 'cards'
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
      "output_path": "output_path",                     		# Output address, only support rtsp, tcp format is protocol://ip:port/, for example rtsp://192.168.0.1:8554/test, tcp://172.28.1.1:5353. for rtsp push stream, the address is the address configured by rtsp server. For tcp, you need to open the port you configured.
      
      "obj_threshold": 0.05,									# Confidence threshold for post-processing the bmodel model corresponding to [path]
      "nms_threshold": 0.3										# nms threshold for post-processing the bmodel model corresponding to [path]
    }
  ]
}
```

> **NOTE**  
> 
> The number of threads and queue length can be defined by the device itself. In principle, the number of pre-processing threads and post-processing threads is set to the number of logical CPUs of the device. The number of inference threads for a single pipeline is typically 1.

### 3.2  Running method

  > **NOTE** 
  > face_detect_1684 int8 model NAS cloud download address：[face_demo.bmodel](http://disk-sophgo-vip.quickconnect.cn/sharing/oD4Jb0RVZ)
> 
>face_detect_1684X int8 model NAS cloud download address: [face_demo_1684X.bmodel](http://disk-sophgo-vip.quickconnect.cn/sharing/Yk3o6QdXD)
  >
  > face_demo_1684X fp16 model NAS cloud download address：[face_demo_1684X_fp16_4b.bmodeI](http://disk-sophgo-vip.quickconnect.cn/sharing/8pBKB4HVW)
  >
  > Download address of the video for testing：[elevator-1080p-25fps-4000kbps.h264](http://disk-sophgo-vip.quickconnect.cn/sharing/7ExA940x2)

Description of parameters

```bash
Usage: face_detect_demo [params]

        --config (value:./cameras_face_detect.json)
                cameras_face_detect.json The path to the configuration file, the default path is ./cameras_face_detect.json
        --help (value:true)
                Print help information
```

#### 3.2.1 x86 PCIe

**Take setting `cameras_face_detect.json` with `chan_num=1` as an example** The test example is as follows：

```bash
cd ${SOPHON_PIPELINE}/release/facedetect_demo
# ./x86/facedetect_demo --help Print help information
#Take x86 pcie 1684x as an example, copy the downloaded face_detect model to ${SOPHON_PIPELINE}/release/facedetect_demo directory and run it. 

./x86/facedetect_demo --config=./cameras_face_detect.json
```

The following message will be printed after execution：

```bash
# Take x86 pcie 1684x as an example
# First, print out each video stream and the corresponding chip-related information, and then print the total FPS of the 1-channel detector det and the speed information corresponding to the processing of the 0th video stream. The FPS and speed information are related to the hardware configuration of the current running device, it is normal for different devices to run different results, and it is normal for the FPS and speed information to fluctuate during the running procedure of the same device.

...
[2022-11-16:19:48:26] total fps =-nan,ch=0: speed=-nan
[2022-11-16:19:48:27] total fps =25.0,ch=0: speed=25.0
[2022-11-16:19:48:28] total fps =25.0,ch=0: speed=25.0
[2022-11-16:19:48:29] total fps =25.0,ch=0: speed=25.0
[2022-11-16:19:48:30] total fps =25.0,ch=0: speed=25.0
[2022-11-16:19:48:31] total fps =25.0,ch=0: speed=25.0
[2022-11-16:19:48:32] total fps =25.0,ch=0: speed=25.0
[2022-11-16:19:48:33] total fps =25.0,ch=0: speed=25.0
[2022-11-16:19:48:34] total fps =25.0,ch=0: speed=25.0

...
```

#### 3.2.2 arm SoC

Copy the `cameras_face_detect.json`, `soc` folder and the corresponding models and test videos from the cross-compiled `${SOPHON_PIPELINE}/release/facedetect_demo` folder to the same directory of the arm SoC running device, and modify the corresponding configuration of cameras_face_detect.json, and then **take setting `cameras_face_detect.json` with `chan_num=1` as an example** The test example is as follows：

```bash
cd ${SOPHON_PIPELINE_FACEDETECT}
# ./soc/facedetect_demo --help Print help information
# Take Soc 1684x as an example.
./soc/facedetect_demo --config=./cameras_face_detect.json 
```

The following message will be printed after execution：

```bash
# Take the arm SoC 1684x as an example
# First, print out each video stream and the corresponding chip-related information, and then print the total FPS of the 1-channel detector det and the speed information corresponding to the processing of the 0th video stream. The FPS and speed information are related to the hardware configuration of the current running device, it is normal for different devices to run different results, and it is normal for the FPS and speed information to fluctuate during the running procedure of the same device.

...
[2022-11-16:20:07:04] total fps =nan,ch=0: speed=nan
[2022-11-16:20:07:05] total fps =24.0,ch=0: speed=24.0
[2022-11-16:20:07:06] total fps =25.0,ch=0: speed=25.0
[2022-11-16:20:07:07] total fps =25.0,ch=0: speed=25.0
[2022-11-16:20:07:08] total fps =25.0,ch=0: speed=25.0
[2022-11-16:20:07:09] total fps =25.0,ch=0: speed=25.0
[2022-11-16:20:07:10] total fps =25.0,ch=0: speed=25.0
[2022-11-16:20:07:11] total fps =25.0,ch=0: speed=25.0

...
```

#### 3.2.3 arm PCIe

**Take setting `cameras_face_detect.json` with `chan_num=1` as an example** The test example is as follows：

```bash
cd ${SOPHON_PIPELINE}/release/facedetect_demo
# ./arm64/facedetect_demo --help Print help information
#Take arm pcie 1684x as an example, copy the downloaded face_detect model to ${SOPHON_PIPELINE}/release/facedetect_demo directory and run it. 

./arm64/facedetect_demo --config=./cameras_face_detect.json
```

The following message will be printed after execution：

```bash
# Take arm pcie 1684x as an example
# First, print out each video stream and the corresponding chip-related information, and then print the total FPS of the 1-channel detector det and the speed information corresponding to the processing of the 0th video stream. The FPS and speed information are related to the hardware configuration of the current running device, it is normal for different devices to run different results, and it is normal for the FPS and speed information to fluctuate during the running procedure of the same device.

...
[2023-03-22:18:48:26] total fps =-nan,ch=0: speed=-nan
[2023-03-22:18:48:27] total fps =25.0,ch=0: speed=25.0
[2023-03-22:18:48:28] total fps =25.0,ch=0: speed=25.0
[2023-03-22:18:48:29] total fps =25.0,ch=0: speed=25.0
[2023-03-22:18:48:30] total fps =25.0,ch=0: speed=25.0
[2023-03-22:18:48:31] total fps =25.0,ch=0: speed=25.0
[2023-03-22:18:48:32] total fps =25.0,ch=0: speed=25.0
[2023-03-22:18:48:33] total fps =25.0,ch=0: speed=25.0
[2023-03-22:18:48:34] total fps =25.0,ch=0: speed=25.0

...
```

### 3.3 Visualization

- Use [pipeline_client](./pipeline_client_visualization_en.md) to display live streams and detection results
