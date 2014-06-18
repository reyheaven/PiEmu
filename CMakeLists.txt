# This file is part of the Team 28 Project
# Licensing information can be found in the LICENSE file
# (C) 2014 The Team 28 Authors. All rights reserved.
cmake_minimum_required(VERSION 2.8)
project(PiEmu)

find_package(SDL REQUIRED)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra -Werror -std=gnu99" )
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -Og -g")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3")

set(SOURCES
  emulator.c
  memory.c
  vfp.c
  cpu.c
  gpio.c
  mbox.c
  framebuffer.c
  peripheral.c
  nes.c
)

set(HEADERS
  common.h
  emulator.h
  opcode.h
  memory.h
  vfp.h
  cpu.h
  gpio.h
  framebuffer.h
  peripheral.h
  nes.h
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
