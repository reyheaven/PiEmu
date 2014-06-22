PiEmu
=====

Emulator for the Raspberry Pi

Build
-----

Currently, the emulation only builds on Linux. 
SDL 1.2 and CMake 2.8 are required to build the project:

    mkdir build
    cd build
    cmake ..
    make

Usage
-----

    --nes:      Emulate the NES controller over GPIO
    --addr=x:   Set the address where the kernel is loaded
    --graphics: Emulate graphics
    --quiet:    Silence status messages
    --memory=x: Set the size of SRAM
    
PiFox
---

In order to run PiFox, you can try:

    ./piemu [path-to-img] --memory=256M --quiet --graphics --addr=65536 --nes

NES controller
--------------

|NES   | Key   |
|------|-------|
|A     | Space |
|B     | Tab   |
|Start | Enter |
|Select| P     |
|Up    | W     |
|Left  | A     |
|Down  | S     |
|Right | D     |

