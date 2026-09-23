# Snowboy KWS

English version: [README.en.md](README.en.md)

这是一个面向嵌入式 ARMv7 Linux 的独立 Snowboy 语音唤醒 C++ 适配器。仓库内附带
的二进制依赖已在 Rockchip RV1106 上验证，目标 ABI 为 uClibc、Cortex-A7、
hard-float、NEON 和 VFPv4。

检测器只消费调用方传入的 PCM，不直接打开 ALSA、不创建采集线程，也不依赖具体
产品的音频服务。

## 目录内容

- `include/`、`src/`：独立 C++ 检测适配器。
- `examples/file_detector.cpp`：从 WAVE 或裸 PCM 文件执行检测的示例。
- `cmake/toolchains/`：可配置的 ARMv7/uClibc CMake 工具链。
- `third_party/snowboy/`：Snowboy 头文件、ARMv7 引擎和 `common.res`。
- `third_party/openblas/`：Snowboy 所需的 ARMv7 OpenBLAS 静态库。
- `models/`：用户自备唤醒词模型的放置目录。

本项目不包含个人唤醒词模型。Snowboy 许可证明确说明，除上游特定模型外，其他
`.pmdl`、`.umdl` 模型有各自的授权条款。只有确认拥有使用和再分发权利后，才应把
模型放进公开仓库。

## 构建要求

- CMake 3.16 或更新版本。
- 与目标根文件系统匹配的 ARMv7 hard-float C++ 交叉编译器。
- 包含目标 C/C++ 运行库的 sysroot。
- 已获得授权的 Snowboy `.pmdl` 或 `.umdl` 唤醒词模型。

仓库附带的静态库是 ARM 二进制，不能链接为 x86 主机程序。

## 编译

`CROSS_COMPILE` 应设置为完整的交叉编译器前缀。如果编译器没有内置正确的
sysroot，还需要设置 `SYSROOT`：

```sh
export CROSS_COMPILE=/opt/toolchain/bin/arm-rockchip830-linux-uclibcgnueabihf-
export SYSROOT=/opt/buildroot/host/arm-buildroot-linux-uclibcgnueabihf/sysroot
./scripts/build-armv7-uclibc.sh
```

输出文件为：

```text
build/armv7-uclibc/libsnowboy_kws.a
build/armv7-uclibc/snowboy_kws_file
```

可以通过环境变量指定构建目录和并行数：

```sh
BUILD_DIR=/tmp/snowboy-build BUILD_JOBS=8 ./scripts/build-armv7-uclibc.sh
```

## 文件检测示例

示例程序接受 16 kHz、单声道、S16_LE PCM。WAVE 输入会自动校验；无文件头的
裸 PCM 需要增加 `--raw`：

```sh
./snowboy_kws_file \
  --resource third_party/snowboy/resources/common.res \
  --model models/your-hotword.pmdl \
  --input test.wav \
  --sensitivity 0.5
```

个人模型通常不启用 `--frontend`。通用模型是否启用前端，应遵循对应模型的训练
配置和说明。

## 集成接口

```cpp
#include <snowboy_kws/detector.h>

snowboy_kws::Detector detector(
    "common.res", "hotword.pmdl", "0.5", false, 1.0f);
if (!detector.valid()) {
    // detector.error() 返回初始化错误。
}

// 持续传入 16 kHz 单声道 S16 PCM；1600 个采样点等于 100 ms。
snowboy_kws::Detection result = detector.accept(samples, sample_count);
if (result.detected) {
    // result.result 是从 1 开始的唤醒词序号。
}
```

CMake 工程可通过 `add_subdirectory()` 引入本目录，然后链接
`snowboy_kws::snowboy_kws`。每次输入必须为 1 到 4096 个采样点。

## 兼容性说明

- Snowboy 静态库使用旧版 libstdc++ ABI。
- Snowboy 和 OpenBLAS 静态库要求 ARMv7 hard-float ABI。
- 当前已验证平台为 RV1106/uClibc；ABI 兼容的其他 Cortex-A7 Linux 平台可能可用，
  但尚未验证。
- Snowboy 已于 2020 年停止维护，上游不再提供支持。
- 部分 Snowboy 二进制包含运行时授权检查，产品发布前必须在每一种目标设备上验证。

## 许可证

适配器采用 Apache-2.0。附带的 Snowboy 文件遵循 Snowboy 的 Apache-2.0 许可证，
OpenBLAS 遵循 BSD-3-Clause。再分发前请阅读 [NOTICE](NOTICE) 和
[THIRD_PARTY.md](THIRD_PARTY.md)。唤醒词模型未包含在本项目中，需单独确认授权。
