/* This file is part of the Team 28 Project
 * Licensing information can be found in the LICENSE file
 * (C) 2014 The Team 28 Authors. All rights reserved.
 */
#ifndef __COMMON_H__
#define __COMMON_H__

/* Standard C */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <getopt.h>
#include <setjmp.h>
#include <assert.h>
#include <time.h>
#include <math.h>

/* SDL */
#include <SDL/SDL.h>

/* Useful macros */
#ifdef __GNUC__
#  define UNUSED(x) UNUSED_ ## x __attribute__((__unused__))
#  define UNUSED_FUNCTION(x) __attribute__((__unused__)) UNUSED_ ## x
#else
#  define UNUSED(x) UNUSED_ ## x
#  define UNUSED_FUNCTION(x) UNUSED_ ## x
#endif

/* Emulator forward declaration */
typedef struct _emulator_t emulator_t;

/* Modules */
#include "memory.h"
#include "opcode.h"
#include "vfp.h"
#include "cpu.h"
#include "gpio.h"
#include "mbox.h"
#include "framebuffer.h"
#include "peripheral.h"
#include "nes.h"

/* Emulator */
#include "emulator.h"

#endif /*__COMMON_H__*/
