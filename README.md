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
# 若编译需要SoC平台上运行的程序，需要先根据2.2.2节准备好相关依赖，再运行下述命令进行编译：
./tools/compile.sh soc ${soc-sdk} ${qtbase-5.14.2-aarch64}
````

编译完成后，可执行程序程序将保存在`${SOPHON_PIPELINE}/test_execs/`文件夹下。
可以将该文件夹整个拷贝至下文中提供的测试包中进行测试。

## 3 运行方法

> **NOTE**  
> cvs20测试程序包下载，请放到对应的bm1688 evb soc板子上面，测试步骤：
  ```bash
  pip3 install dfss -i https://pypi.tuna.tsinghua.edu.cn/simple --upgrade
  python3 -m dfss --url=open@sophgo.com:sophon-pipeline/a2_bringup/test_pack_cvs20_latest.tar
  tar xvf test_pack_cvs20_0927_2.tar
  cd test_pack_cvs20 #这里面有个readme.md, 是对各个文件的介绍。
  ./setup.sh <exe> <chan_num> # <exe>即可执行程序，如果想要测试自己编译出来的可执行程序，直接用`${SOPHON_PIPELINE}/test_execs/`下的程序替换即可。
  ```
### 3.1 单核 4路 全流程
将`test_pack_cvs20/cameras_cvs.json`中 `models: paths:` 对应的键值 `xxx.bmodel` 改为`cvs20_int8_4b.bmodel`，板子插上hdmi显示器，在`test_pack_cvs20`目录中运行如下命令：
```
./setup.sh test_execs/cvs20_all_gui 4
```

### 3.2 双核 4路 全流程
将`test_pack_cvs20/cameras_cvs.json`中 `models: paths:` 对应的键值 `xxx.bmodel` 改为`cvs20_int8_4b_2core.bmodel`，板子插上hdmi显示器，在`test_pack_cvs20`目录中运行如下命令：
```
./setup.sh test_execs/cvs20_all_gui 4
```

### 3.3 稳定性测试
按照3.1或3.2中的命令进行测试，时间不小于2个小时。

### 3.3 停止程序及报错处理办法
cvs20 程序不会自动停止，需要通过`ctrl+c`手动停止。也就是说，稳定性测试无需额外的操作，只需要让它一直跑就行了，如果程序自动停止了，除程序本身报错之外，请收集dmesg信息贴到jira页面上：

```
dmesg > dmesg.log
```
将`dmesg.log`从板子上拿出来，贴到相应的jira上。 

## 4 ddr 3200 性能测试
请参考该wiki页面进行测试：
https://wiki.sophgo.com/pages/viewpage.action?pageId=102741616

在某处新建一个wiki页面，注意应公开权限，让所有人都能看到，然后将你的测试结果填到那里。
