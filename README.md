# sophon-pipeline介绍

## 1 简介

Sophon Pipeline提供一个简易的基于Pipeline的高性能加速框架，使用该框架能够将前处理/推理/后处理分别运行在3个线程上，最大化的实现并行, 用户只需要继承一个类，实现自己的前处理/后处理，就可以实现整个流程。

**Sophon Pipeline特性：**

- 单线程单模型
- 多线程单模型
- 多线程多模型
- 多模型串联
- 拉流、推流
- 视频结构可视化

**主要目录结构和模块说明：**

| 目录                   | 模块                               | 功能说明                                                     |
| ---------------------- | ---------------------------------- | ------------------------------------------------------------ |
| [modules](./modules)   | [bmgui](./modules/bmgui)           | 存放Sophon Pipeline用来GUI显示视频的模块                     |
|                        | [bmgui-lite](./modules/bmgui-lite) | 由于SDK自带的OpenCV没有显示功能，此模块提供bm::imshow来显示视频，作为补充。 |
|                        | [bmutility](./modules/bmutility)   | 提供了基础库，字符串、定时器等                               |
|                        | [tracker](./modules/tracker)       | 提供了CPU跟踪模块                                            |
| [examples](./examples) | [cvs20](./examples/cvs20)          | 提供了算能一路的参考实现                                     |

**Sophon Pipeline的主要结构设计如下图：** 

![**avatar**](./docs/pics/sophon-pipeline.png)

## 2 编译方法

### 2.1 环境准备

**目前只支持soc模式**

通常在x86主机上交叉编译程序，使之能够在arm SoC平台运行。您需要在x86主机上使用SOPHON SDK搭建交叉编译环境，将程序所依赖的头文件和库文件打包至soc-sdk目录中。

**交叉编译环境和相关依赖环境的准备步骤详见：**[arm_soc平台编译准备](./docs/arm_soc.md)。

> 如果遇到其他交叉编译问题，请参考《LIBSOPHON使用手册.pdf》的**4.2.2节 x86 交叉编译程序**章节。

### 2.2 编译指令
```` bash
# 若编译需要SoC平台上运行的程序，需要先根据2.1节准备好相关依赖，再运行下述命令进行编译：
chmod +x tools/compile.sh
./tools/compile.sh soc ${soc-sdk} ${qtbase-5.14.2-aarch64} #注意，该命令是在x86上执行的。
````

编译完成后，可执行程序将保存在`${SOPHON_PIPELINE}/test_execs/`文件夹下。
可以将该文件夹整个拷贝至下文中提供的测试包中进行测试。

## 3 运行方法

下载cvs20测试程序包，请放到对应的板子上面，测试步骤：
  ```bash
  pip3 install dfss -i https://pypi.tuna.tsinghua.edu.cn/simple --upgrade
  python3 -m dfss --url=open@sophgo.com:sophon-pipeline/a2_bringup/test_pack_cvs20_v1.5.tar #v1.5是版本号，最新版本为test_pack_cvs20_latest.tar
  tar xvf test_pack_cvs20_v1.5.tar
  cd test_pack_cvs20 #这里面有个readme.md, 是对各个文件的介绍。
  ./setup.sh <exe> <chan_num> <display_num> <save_num> # <exe>即可执行程序，如果想要测试自己编译出来的可执行程序，直接用`${SOPHON_PIPELINE}/test_execs/`下的程序替换即可。
  #chan_num 表示跑几路，display_num 表示几路显示，save_num 表示几路编码
  ```

### 3.1 全流程测试
板子插上hdmi显示器，在`test_pack_cvs20`目录中运行如下命令：
```bash
./setup_cv186ah.sh test_execs/cvs20_all_gui_enc_h264_and_jpeg160fps 8 8 8 # 对应cv186ah
./setup.sh test_execs/cvs20_all_gui_enc_h264_and_jpeg160fps 16 16 10 # 对应bm1688
```

### 3.2 稳定性测试
按照3.1中的命令进行测试，时间不小于12小时。

### 3.3 停止程序及报错处理办法
cvs20 程序不会自动停止，需要通过`ctrl+c`手动停止。也就是说，稳定性测试无需额外的操作，只需要让它一直跑就行了，如果程序自动停止了，除程序本身报错之外，还需要收集dmesg信息贴到jira页面上：

```bash
dmesg > dmesg_<your name>_<test time>.log #建议log的名字也用姓名和时间打好标记，比如dmesg_liheng_231110_1840.log
```
将`dmesg_xxx.log`从板子上拿出来，贴到相应的jira上。 

4G版本可能会出现内存不足问题，比如程序运行报错：`bm_alloc_gemm failed`，如果出现这类错误，可以参考该内存修改方法：
```bash
cd /data/
mkdir memedit
wget -nd https://sophon-file.sophon.cn/sophon-prod-s3/drive/23/09/11/13/DeviceMemoryModificationKit.tgz
tar xvf DeviceMemoryModificationKit.tgz
cd DeviceMemoryModificationKit
tar xvf memory_edit_vx.x.tar.xz #注意，版本号可能会变
cd memory_edit
./memory_edit.sh -p #这个命令会打印当前的内存布局信息，当前版本可供修改的内存有2424MB，如果因为刷机包版本不同的原因导致可供修改的内存有升高或者降低，4G版本建议npu > 0.5G, vpp > 1.7G。
./memory_edit.sh -c -npu 512 -vpu 0 -vpp 1912 #npu heap降低为512MB，vpp heap提高到1912MB
sudo cp /data/memedit/DeviceMemoryModificationKit/memory_edit/boot.itb /boot/boot.itb && sync
sudo reboot
```

## 4 性能测试
请参考该wiki页面进行测试：
https://wiki.sophgo.com/pages/viewpage.action?pageId=112854015

结果填到这个wiki页面：
https://wiki.sophgo.com/pages/viewpage.action?pageId=112872612

如果只测cv186ah就只填cv186ah的内容即可，bm1688同理。