# Delper
一键优化你的LRF文件
[English](https://github.com/JasonL111/Delper/blob/main/README.en.md)
## 简介

这是一个用于整理和处理 DJI 设备生成的 `.LRF` 文件的 C 程序。它提供以下功能：
- 创建一个名为 `LRF` 的文件夹并将所有 `.LRF` 文件移动到其中；其余文件则移动到名为 `MP4` 的文件夹中。
- 将 `LRF` 文件夹中的所有 `.LRF` 文件重命名为 `.MP4` 后缀。
- 使用 H265 编解码器压缩 `LRF` 文件夹中的所有 `.MP4` 视频，并删除文件名中不包含 `_LRF` 的原始文件。

## 为什么需要它？

默认的 LRF 文件使用 H264 编码格式，这种格式通常占用较大的存储空间。使用此工具可以将其压缩为更小的 MP4 文件（使用 H265 编码），更节省空间，同时也更适合流媒体播放和日常观看。

## 安装方法

1. 下载已发布的可执行文件（.exe）。
2. 下载并安装 [FFmpeg](https://ffmpeg.org/download.html)。
3. 将 Delper.exe 的路径添加到系统环境变量 PATH 中。
4. 然后你就可以在命令行中直接使用它了。

## 使用方法
```bash
Delper
  --help    打印帮助信息
  -m        在当前目录下创建一个名为 'LRF' 的文件夹，并将所有 .LRF 文件移动到其中。
  -t        将 'LRF' 文件夹中所有 .LRF 文件重命名为 .MP4 后缀。
  -c        使用 H265 编解码器压缩 'LRF' 文件夹中的所有 MP4 视频。
  -d        删除 'LRF' 文件夹中所有文件名不包含 '_LRF' 的视频。
            （文件将被移动到回收站，而不是永久删除）
  -o        一键执行所有任务，依次执行 -m -t -c -d。
  ```
## 注意事项

- 要进行压缩操作，必须先下载 FFmpeg。
- 压缩后的视频质量为 3Mbps，音频质量保持不变。