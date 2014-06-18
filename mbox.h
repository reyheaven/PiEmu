/* This file is part of the Team 28 Project
 * Licensing information can be found in the LICENSE file
 * (C) 2014 The Team 28 Authors. All rights reserved.
 */
#ifndef __MBOX_H__
#define __MBOX_H__

/**
 * List of mailbox register addresses
 */
typedef enum
{
  MBOX_BASE   = 0x2000b880,
  MBOX_READ   = MBOX_BASE + 0x00,
  MBOX_POLL   = MBOX_BASE + 0x10,
  MBOX_SENDER = MBOX_BASE + 0x14,
  MBOX_STATUS = MBOX_BASE + 0x18,
  MBOX_CONFIG = MBOX_BASE + 0x1c,
  MBOX_WRITE  = MBOX_BASE + 0x20
} mbox_ports_t;

/**
 * Mailbox structure
 * Mailbox emulation is not completely accurate as all requests
 * are serviced immediately, so the status bits are always set
 * to ready
 */
typedef struct _mbox_t
{
  emulator_t *emu;
  uint8_t     last_channel;
} mbox_t;

void     mbox_init(mbox_t *mbox, emulator_t *emu);
void     mbox_destroy(mbox_t *mbox);
uint32_t mbox_read(mbox_t *mbox, uint32_t addr);
void     mbox_write(mbox_t *mbox, uint32_t addr, uint32_t val);

/**
 * Checks whether a given address is a mailbox port.
 * It seems that the CPU ignores the last 4 bits of the address, so
 * all writes are aligned on a 4 byte boundary.
 */
static inline int
mbox_is_port(uint32_t addr)
{
  addr &= ~0x3;
  return MBOX_BASE <= addr && addr <= MBOX_WRITE;
}

#endif /*__MBOX_H__*/
