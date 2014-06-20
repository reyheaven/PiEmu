# This file is part of the Team 28 Project
# Licensing information can be found in the LICENSE file
# (C) 2014 The Team 28 Authors. All rights reserved.
cmake_minimum_required(VERSION 2.8)
project(PiEmu)

find_package(SDL REQUIRED)

include_directories(${CMAKE_SOURCE_DIR})

set(CMAKE_C_FLAGS           "${CMAKE_C_FLAGS} -Wall -Wextra -Werror -std=gnu11")
set(CMAKE_C_FLAGS_DEBUG     "${CMAKE_C_FLAGS_DEBUG} -Og -g")
set(CMAKE_C_FLAGS_RELEASE   "${CMAKE_C_FLAGS_RELEASE} -O3")
set(CMAKE_CXX_FLAGS         "${CMAKE_CXX_FLAGS} -Wall -Wextra -Werror -std=gnu++11 -fno-rtti")
set(CMAKE_CXX_FLAGS_DEBUG   "${CMAKE_CXX_FLAGS_DEBUG} -Og -g")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3")

set(SOURCES
  emulator.c
  memory.c
  vfp.c
  cpu.c
  nes.c
  bcm2835/gpio.c
  bcm2835/mbox.c
  bcm2835/framebuffer.c
  bcm2835/peripheral.c
)

set(HEADERS
  common.h
  emulator.h
  opcode.h
  memory.h
  vfp.h
  cpu.h
  nes.h
  bcm2835/gpio.h
  bcm2835/mbox.h
  bcm2835/framebuffer.h
  bcm2835/peripheral.h
)

set(LIBS
  m
  ${SDL_LIBRARY}
)

include_directories(
  ${SDL_INCLUDE_DIRS}
)

# Executable
add_executable(
  piemu
  main.c
  ${SOURCES}
  ${HEADERS}
)

target_link_libraries(
  piemu
  ${LIBS}
)
