/* This file is part of the Team 28 Project
 * Licensing information can be found in the LICENSE file
 * (C) 2014 The Team 28 Authors. All rights reserved.
 */

#ifndef __GPIO_H__
#define __GPIO_H__

#define GPIO_PORT_COUNT     54

/**
 * Struct describing a single gpio port
 */
typedef struct
{
  uint8_t func;
  uint8_t state;
} gpio_port_t;

/**
 * GPIO emulation
 */
typedef struct _gpio_t
{
  gpio_port_t *ports;
  emulator_t  *emu;
} gpio_t;

/**
 * GPIO registers
 */
typedef enum
{
  GPIO_BASE   = 0x20200000,
  GPIO_FSEL0  = GPIO_BASE + 0x00,
  GPIO_FSEL1  = GPIO_BASE + 0x04,
  GPIO_FSEL2  = GPIO_BASE + 0x08,
  GPIO_FSEL3  = GPIO_BASE + 0x0C,
  GPIO_FSEL4  = GPIO_BASE + 0x10,
  GPIO_FSEL5  = GPIO_BASE + 0x14,
  GPIO_SET0   = GPIO_BASE + 0x1C,
  GPIO_SET1   = GPIO_BASE + 0x20,
  GPIO_CLR0   = GPIO_BASE + 0x28,
  GPIO_CLR1   = GPIO_BASE + 0x2C,
  GPIO_LEV0   = GPIO_BASE + 0x34,
  GPIO_LEV1   = GPIO_BASE + 0x38,
  GPIO_EDS0   = GPIO_BASE + 0x40,
  GPIO_EDS1   = GPIO_BASE + 0x44,
  GPIO_REN0   = GPIO_BASE + 0x4C,
  GPIO_REN1   = GPIO_BASE + 0x50,
  GPIO_FEN0   = GPIO_BASE + 0x58,
  GPIO_FEN1   = GPIO_BASE + 0x5C,
  GPIO_HEN0   = GPIO_BASE + 0x64,
  GPIO_HEN1   = GPIO_BASE + 0x68,
  GPIO_LEN0   = GPIO_BASE + 0x70,
  GPIO_LEN1   = GPIO_BASE + 0x74,
  GPIO_AREN0  = GPIO_BASE + 0x7C,
  GPIO_AREN1  = GPIO_BASE + 0x80,
  GPIO_AFEN0  = GPIO_BASE + 0x88,
  GPIO_AFEN1  = GPIO_BASE + 0x8C,
  GPIO_PUD    = GPIO_BASE + 0x94,
  GPIO_UDCLK0 = GPIO_BASE + 0x98,
  GPIO_UDCLK1 = GPIO_BASE + 0x9C
} gpio_reg_t;

void      gpio_init(gpio_t*, emulator_t*);
void      gpio_destroy(gpio_t*);
uint32_t  gpio_read_port(gpio_t*, uint32_t);
void      gpio_write_port(gpio_t*, uint32_t, uint32_t);
int       gpio_is_port(uint32_t addr);

#endif /* __GPIO_H__ */