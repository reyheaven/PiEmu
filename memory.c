/* This file is part of the Team 28 Project
 * Licensing information can be found in the LICENSE file
 * (C) 2014 The Team 28 Authors. All rights reserved.
 */
#include "common.h"

/**
 * Checks whether a port is a dma control port
 */
static inline int
dma_is_port(uint32_t addr)
{
  return 0x20007000 <= addr && addr < 0x20007FF4;
}

/**
 * Initialises the memory module
 * @param m    Reference to the memory structure
 * @param emu  Reference to the emulator structure
 * @param size Size of memory in bytes
 */
void
memory_init(memory_t* m, emulator_t* emu)
{
  m->emu = emu;
  m->data = (uint8_t*)malloc(emu->mem_size);
  memset(m->data, 0, emu->mem_size);
  assert(m->data);
}

/**
 * Prints out the non-zero bytes from memory
 * @param memory Reference to the memory structure
 */
void
memory_dump(memory_t* m)
{
  size_t i = 0;
  printf("Non-zero memory:\n");

  for (i = 0; i < m->emu->mem_size && i < 65535; i += 4)
  {
    uint32_t data = memory_read_dword_be(m, i);
    if (data != 0)
    {
      printf("0x%08zx: 0x%08x\n", i, data);
    }
  }
}

/**
 * Frees the memory
 * @param memory Reference to the memory structure
 */
void
memory_destroy(memory_t* m)
{
  if (!m)
  {
    return;
  }

  if (m->data)
  {
    free(m->data);
  }
}

/**
 * Reads a byte from memory
 * @param memory Reference to the memory structure
 * @param addr Memory location
 */
uint8_t
memory_read_byte(memory_t* m, uint32_t addr)
{
  addr = addr & 0x3FFFFFFF;

  /* SDRAM */
  if (__builtin_expect(addr < m->emu->mem_size, 1))
  {
    return m->data[addr];
  }

  emulator_error(m->emu, "Out of bounds memory access at address 0x%08x", addr);
  return 0;
}

/**
 * Reads a word from memory (little endian)
 * @param memory Reference to the memory structure
 * @param address Memory location
 */
uint16_t
memory_read_word_le(memory_t* m, uint32_t addr)
{
  uint32_t base;
  uint8_t off;

  addr = addr & 0x3FFFFFFF;

  /* SDRAM */
  if (__builtin_expect(addr + 1 < m->emu->mem_size, 1))
  {
    base = addr & ~0x01;
    off = addr & 0x01;

    return (m->data[base + ((off + 0) & 0x01)] <<  0) |
           (m->data[base + ((off + 1) & 0x01)] <<  8);
  }

  emulator_error(m->emu, "Out of bounds memory access at address 0x%08x", addr);
  return 0;
}

/**
 * Reads a double word from memory (little endian)
 * @param memory Reference to the memory structure
 * @param address Memory location
 */
uint32_t
memory_read_dword_le(memory_t* m, uint32_t addr)
{
  uint32_t base;
  uint8_t off;

  /* Apparently, SDRAM and IO peripherials are mapped to 4 different address
   * ranges by the VideoCore MMU. We can replicate that behaviour by ignoring
   * the two most significant bits of the address. We can do this thanks to
   * the fact that we ignore cacheing
   */
  addr = addr & 0x3FFFFFFF;

  /* SDRAM Read */
  if (__builtin_expect(addr + 3 < m->emu->mem_size, 1))
  {
    base = addr & ~0x03;
    off = addr & 0x03;

    return (m->data[base + ((off + 0) & 0x03)] <<  0) |
           (m->data[base + ((off + 1) & 0x03)] <<  8) |
           (m->data[base + ((off + 2) & 0x03)] << 16) |
           (m->data[base + ((off + 3) & 0x03)] << 24);
  }

  /* System Timer */
  if (addr == 0x20003004)
  {
    uint64_t timer_value = emulator_get_system_timer(m->emu);
    return (uint32_t)(timer_value & 0xffffffff);
  }
  else if (addr == 0x20003008)
  {
    uint64_t timer_value = emulator_get_system_timer(m->emu);
    return (uint32_t)((timer_value >> 32) & 0xffffffff);
  }

  /* GPIO registers */
  if (gpio_is_port(addr))
  {
    return gpio_read_port(&m->emu->gpio, addr);
  }

  /* Mailbox interface */
  if (mbox_is_port(addr))
  {
    return mbox_read(&m->emu->mbox, addr);
  }

  /* Peripherals */
  if (pr_is_aux_port(addr))
  {
    return pr_read(&m->emu->pr, addr);
  }

  /* DMA - just ignore it */
  if (dma_is_port(addr))
  {
    return 0;
  }

  emulator_error(m->emu, "Out of bounds memory access at address 0x%08x", addr);
  return 0;
}

/**
 * Writes a single byte to memory
 * @param m    Reference to the memory structure
 * @param addr Memory location
 * @param data Data to be written
 */
void
memory_write_byte(memory_t* m, uint32_t addr, uint8_t data)
{
  addr = addr & 0x3FFFFFFF;

  /* SDRAM */
  if (__builtin_expect(addr < m->emu->mem_size, 1))
  {
    m->data[addr] = data;
    return;
  }

  emulator_error(m->emu, "Out of bounds memory access at address 0x%08x", addr);
}

/**
 * Writes a word to memory
 * @param m    Reference to the memory structure
 * @param addr Memory location
 * @param data Data to be written
 */
void
memory_write_word_le(memory_t* m, uint32_t addr, uint16_t data)
{
  addr = addr & 0x3FFFFFFF;

  /* SDRAM */
  if (__builtin_expect(addr + 1 < m->emu->mem_size, 1))
  {
    m->data[addr + 0] = (data >> 0) & 0xFF;
    m->data[addr + 1] = (data >> 8) & 0xFF;
    return;
  }

  /* Framebuffer */
  if (fb_is_buffer(&m->emu->fb, addr))
  {
    fb_write_word(&m->emu->fb, addr, data);
    return;
  }

  emulator_error(m->emu, "Out of bounds memory access at address 0x%08x", addr);
}

/**
 * Writes a double word to memory
 * @param m    Reference to the memory structure
 * @param addr Memory location
 * @param data Data to be written
 */
void
memory_write_dword_le(memory_t* m, uint32_t addr, uint32_t data)
{
  addr = addr & 0x3FFFFFFF;

  /* SDRAM */
  if (__builtin_expect(addr + 3 < m->emu->mem_size, 1))
  {
    m->data[addr + 0] = (data >>  0) & 0xFF;
    m->data[addr + 1] = (data >>  8) & 0xFF;
    m->data[addr + 2] = (data >> 16) & 0xFF;
    m->data[addr + 3] = (data >> 24) & 0xFF;
    return;
  }

  /* GPIO registers */
  if (gpio_is_port(addr))
  {
    gpio_write_port(&m->emu->gpio, addr, data);
    return;
  }

  /* Mailbox interface */
  if (mbox_is_port(addr))
  {
    mbox_write(&m->emu->mbox, addr, data);
    return;
  }

  /* Framebuffer */
  if (fb_is_buffer(&m->emu->fb, addr))
  {
    fb_write_dword(&m->emu->fb, addr, data);
    return;
  }

  /* Peripherals */
  if (pr_is_aux_port(addr))
  {
    pr_write(&m->emu->pr, addr, data);
    return;
  }

  /* DMA - just ignore it */
  if (dma_is_port(addr))
  {
    return;
  }

  emulator_error(m->emu, "Out of bounds memory access at address 0x%08x", addr);
}

