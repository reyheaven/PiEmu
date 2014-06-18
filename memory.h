/* This file is part of the Team 28 Project
 * Licensing information can be found in the LICENSE file
 * (C) 2014 The Team 28 Authors. All rights reserved.
 */

#ifndef __MEMORY_H__
#define __MEMORY_H__

/**
 * Memory system
 */
typedef struct _memory_t
{
  uint8_t     *data;
  emulator_t  *emu;
} memory_t;

void      memory_init(memory_t*, emulator_t*);
void      memory_dump(memory_t*);
void      memory_destroy(memory_t*);
uint8_t   memory_read_byte(memory_t*, uint32_t);
uint16_t  memory_read_word_le(memory_t*, uint32_t);
uint32_t  memory_read_dword_le(memory_t*, uint32_t);
void      memory_write_byte(memory_t*, uint32_t, uint8_t);
void      memory_write_word_le(memory_t*, uint32_t, uint16_t);
void      memory_write_dword_le(memory_t*, uint32_t, uint32_t);

/**
 * Reads a word from memory (big endian)
 * @param memory Reference to the memory structure
 * @param address Memory location
 */
static inline uint16_t
memory_read_word_be(memory_t* m, uint32_t addr)
{
  return __builtin_bswap16(memory_read_word_le(m, addr));
}

/**
 * Reads a double word from memory (big endian order)
 * @param memory Reference to the memory structure
 * @param address Memory location
 */
static inline uint32_t
memory_read_dword_be(memory_t* m, uint32_t addr)
{
  return __builtin_bswap32(memory_read_dword_le(m, addr));
}

/**
 * Writes a word to memory (big endian order)
 * @param m    Reference to the memory structure
 * @param addr Memory location
 * @param data Data to be written
 */
static inline void
memory_write_word_be(memory_t* m, uint32_t addr, uint16_t data)
{
  memory_write_word_le(m, addr, __builtin_bswap16(data));
}

/**
 * Writes a double word to memory (big endian order)
 * @param m    Reference to the memory structure
 * @param addr Memory location
 * @param data Data to be written
 */
static inline void
memory_write_dword_be(memory_t* m, uint32_t addr, uint32_t data)
{
  memory_write_word_le(m, addr, __builtin_bswap16(data));
}

#endif /* __MEMORY_H__ */
