# Quake for embedded devices

QuakEMBD is yet another WinQuake port for embedded devices, primarily for ARM Cortex-M devices.

Based on original Quake GPL source: [https://github.com/id-Software/Quake](https://github.com/id-Software/Quake)

## Limitations

* Sound, input, and music is not yet supported.
* Many other features may not be supported or left untested.

## Will it run Quake?

Currently the following devices are supported.

* [STM32H747I-DISCO](https://www.st.com/ja/evaluation-tools/stm32h747i-disco.html)
  * Locate `*.PAK` files under `<micro-sd-card>/quakembd/id1`
  * You can use [STM32CubeProgrammer](https://www.st.com/en/development-tools/stm32cubeprog.html) to program `quakembd.bin` file

## How to build

Use CMake with [GNU Arm Embedded Toolchain](https://developer.arm.com/tools-and-software/open-source-software/developer-tools/gnu-toolchain/gnu-rm/downloads) installed.

The defaut toolchain file assumes:
* GNU Arm Embedded Toolchain is installed under `~/gcc-arm-none-eabi-9-2019-q4-major`
* STM32Cube package is installed under `~/STM32Cube/Repository/STM32Cube_FW_H7_V1.7.0`

See `port/boards/stm32h747i_disco/gcc/toolchain.cmake` file for details.

```
$ mkdir build && cd build
$ cmake \
-DCMAKE_TOOLCHAIN_FILE=../port/boards/stm32h747i_disco/gcc/toolchain.cmake \
-DCMAKE_BUILD_TYPE=RELEASE \
-DBOARD_NAME=stm32h747i_disco \
-GNinja ..
$ ninja
```