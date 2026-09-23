# Snowboy KWS

A small standalone C++ adapter for running the legacy Snowboy hotword engine on
embedded ARMv7 Linux systems. The bundled binaries have been validated on
Rockchip RV1106 with uClibc, Cortex-A7, hard-float, NEON, and VFPv4.

The library consumes PCM supplied by the caller. It does not open ALSA, create
threads, or depend on an application-specific audio service.

Chinese documentation: [README.zh-CN.md](README.zh-CN.md)

## Repository contents

- `include/` and `src/`: the standalone C++ adapter.
- `examples/file_detector.cpp`: detection from WAVE or raw PCM files.
- `cmake/toolchains/`: a configurable ARMv7/uClibc CMake toolchain.
- `third_party/snowboy/`: Snowboy header, ARMv7 engine, and `common.res`.
- `third_party/openblas/`: the ARMv7 OpenBLAS archive required by Snowboy.
- `models/`: a placeholder for user-supplied hotword models.

No personal hotword model is included. Snowboy models may have licensing terms
that differ from the engine license, so only use and redistribute a model when
you have the rights to do so.

## Requirements

- CMake 3.16 or newer.
- An ARMv7 hard-float C++ cross compiler compatible with the target rootfs.
- A target sysroot containing the target C and C++ runtimes.
- A licensed Snowboy `.pmdl` or `.umdl` hotword model.

The bundled archives cannot be linked into an x86 host binary.

## Build

Set `CROSS_COMPILE` to the full compiler prefix. Set `SYSROOT` when the compiler
does not already provide the correct target sysroot.

```sh
export CROSS_COMPILE=/opt/toolchain/bin/arm-rockchip830-linux-uclibcgnueabihf-
export SYSROOT=/opt/buildroot/host/arm-buildroot-linux-uclibcgnueabihf/sysroot
./scripts/build-armv7-uclibc.sh
```

The outputs are:

```text
build/armv7-uclibc/libsnowboy_kws.a
build/armv7-uclibc/snowboy_kws_file
```

To use a different build directory or parallelism:

```sh
BUILD_DIR=/tmp/snowboy-build BUILD_JOBS=8 ./scripts/build-armv7-uclibc.sh
```

## File example

The example accepts 16 kHz, mono, signed 16-bit little-endian PCM. WAVE input
is validated automatically; pass `--raw` for headerless PCM.

```sh
./snowboy_kws_file \
  --resource third_party/snowboy/resources/common.res \
  --model models/your-hotword.pmdl \
  --input test.wav \
  --sensitivity 0.5
```

Personal models are normally used without `--frontend`. Universal models may
require `--frontend`; follow the settings specified for the model.

## Library API

```cpp
#include <snowboy_kws/detector.h>

snowboy_kws::Detector detector(
    "common.res", "hotword.pmdl", "0.5", false, 1.0f);
if (!detector.valid()) {
    // detector.error() describes initialization failures.
}

// Feed short chunks of 16 kHz mono S16 PCM. 1600 samples is 100 ms.
snowboy_kws::Detection result = detector.accept(samples, sample_count);
if (result.detected) {
    // result.result is the one-based hotword index.
}
```

When embedding with CMake, add this directory with `add_subdirectory()` and
link `snowboy_kws::snowboy_kws`. The adapter enforces Snowboy's expected audio
contract: 16 kHz, mono, signed 16-bit PCM. Each input chunk must contain between
1 and 4096 samples.

## Compatibility notes

- The bundled Snowboy engine uses the pre-C++11 libstdc++ ABI.
- The bundled engine and OpenBLAS archives require ARMv7 hard-float.
- The tested target is RV1106/uClibc. Other Cortex-A7 Linux boards may work when
  their ABI is compatible, but are not currently verified.
- Snowboy was discontinued in 2020 and no longer receives upstream support.
- Some Snowboy binaries contain runtime license checks. Validate the engine on
  every intended device before shipping a product.

## Licensing

The adapter is distributed under Apache-2.0. Bundled Snowboy files are covered
by the Snowboy Apache-2.0 license. OpenBLAS is BSD-3-Clause. See [NOTICE](NOTICE)
and [THIRD_PARTY.md](THIRD_PARTY.md) before redistribution.

Hotword models are deliberately excluded and remain subject to their own
licenses.
