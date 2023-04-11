# FAQ

sophon-pipeline常见问题及解答

## 1 常见问题

**Q: 当运行推理路数较多时，遇到`bm_malloc_device_byte_heap error`或`bm::BMImage::create_batch Assertion BM_SUCCESS==ret failed`等内存分配错误的问题**

**A**: 

这种错误主要是设备内存分配失败。失败的原因有两种：

(1) 设备内存不够用，可以通过如下方法确认：

SOC模式下，使用以下命令查看：

```bash
cat /sys/kernel/debug/ion/bm_vpp_heap_dump/summary
```

PCIe模式下，bm-smi工具可以查看设备内存空间。

解决方法：

设备内存不够就需要通过优化程序来减少对设备内存的占用，或者通过修改 dts 文件中的内存布局来增加对应的设备内存。详细可以参考官方《SOPHON BSP开发参考手册.pdf》的相关说明。注：需要获取《SOPHON BSP开发参考手册.pdf》，请联系技术支持。



(2) 句柄数超过系统限制，原因有可能是因为句柄泄漏，或者系统句柄数设置过小，可以用 如下方法确认：

出现错误后，使用

```bash
dmesg > err.log
```

搜索err.log可见关键字"error_code=-24"、"Too many open files"等关键字。这个是因为我们使用 ION 管理内存， 而 ION 又依赖于 dma_buf，后者会对每个分出来的 buffer 分配一个文件描述符 fd 进行管理。系统对同时处在打开状态的 fd 总数是有限制的。

那么请查看系统定义的最大句柄数：

```bash
ulimit -n
```

是否为20480。如果不是的话，请再检查`/etc/security/limits.conf`文件，是否有如下两行：

```bash
* soft nofile 20480
* hard nofile 20480 
```

解决方法：

在排除代码本身的内存泄漏或者句柄泄漏问题后，可以通过以下两种方式加大系统最大句柄数来解决句柄的限制问题：

(a) 使用`ulimit`命令临时性修改，当终端或服务器被重启后失效

```bash
ulimit -HSn 20480
```

(b) 在`/etc/security/limits.conf`文件末尾添加：

```
* soft nofile 20480
* hard nofile 20480
```

添加完成后重启服务器即可。

> 注意：如果上述方法没有解决问题，请先通过 ps 命令获取业务进程 ID，然后 cat /proc/$pid/limits， 检查“Max open files”那一行是否为 20480，如果不是的话，请在直接启动您业务程序的 shell 脚本（比如上面 log 示例里的 run.sh）开头添加“ulimit -n 20480”。重新运行业务，通过 cat /proc/$pid/limits 复核是否修改成功。 如果以上方法都检查了没问题，那可能您的确申请了太多 buf，可以通过上述方法继续增大 max open files 数量。但最大也不应超过 cat /proc/sys/fs/file-max 显示的数量。



**Q: 使用pipeline_client可视化时，pipeline_client报错：.BMvidDecCreateW get chip info failed!!! error sending a packet for decoding. decode failed!**

<img src="../pics/faq/pipeline_client_rtsp_decode_failed.png" style="zoom:50%">

**A:**

由于在安装SophonSDK后，ffmpeg会链接到sophon-mw-ffmpeg的依赖库，若使用`ldd /usr/bin/ffmpeg`命令可见很多依赖链接到了`/opt/sophon/sophon-ffmpeg-latest/lib/*`下。

解决方法：

需要链接回公版ffmpeg的库上，可使用命令：

```bash
export LD_LIBRARY_PATH=/lib/x86_64-linux-gnu:$PATH
```

此时再次使用`ldd /usr/bin/ffmpeg`命令可见依赖链接回到`/lib/x86_64-linux-gnu/*`。

同理，若出现pipeline_client占用了sophon的tpu，也用同样方法解决。

<img src="../pics/faq/pipeline_client_bmffmpeg.jpg" style="zoom:50%">

