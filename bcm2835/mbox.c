/* This file is part of the Team 28 Project
 * Licensing information can be found in the LICENSE file
 * (C) 2014 The Team 28 Authors. All rights reserved.
 */
#include "common.h"

/**
 * Initialises the mailbox
 */
void
mbox_init(mbox_t *mbox, emulator_t *emu)
{
  mbox->emu = emu;
  mbox->last_channel = 0x0;
}

/**
 * Cleans up memory used by the mailbox
 */
void
mbox_destroy(mbox_t *UNUSED(mbox))
{
}

/**
 * Reads data from a mailbox port
 * @param mbox Mailbox structure
 * @param addr Port address
 */
uint32_t
mbox_read(mbox_t *mbox, uint32_t addr)
{
  addr &= ~0x3;
  assert(mbox_is_port(addr));

  /* Check which port is being read */
  switch (addr)
  {
    case MBOX_READ:
    {
      /* Always return 0 + last channel id */
      switch (mbox->last_channel)
      {
        case 1:
        {
          /* Return non zero after a failed request */
          return mbox->last_channel | (mbox->emu->fb.error ? ~0xF : 0x0);
        }
        default:
        {
          return mbox->last_channel;
        }
      }
      break;
    }
    case MBOX_STATUS:
    {
      /* Bit 31 == 0: ready to receive
         Bit 30 == 0: ready to send */
      return 0;
    }
  }

  emulator_error(mbox->emu, "Mailbox unimplemented 0x%08x", addr);
  return 0;
}

/**
 * Writes data to a mailbox port
 * @param mbox Mailbox structure
 * @param addr Port address
 * @param val  Value to be written
 */
void
mbox_write(mbox_t *mbox, uint32_t addr, uint32_t val)
{
  uint8_t channel;
  uint32_t data;

  addr &= ~0x3;
  assert(mbox_is_port(addr));

  /* Retrieve data & channel */
  channel = val & 0xF;
  data = val & ~0xF;

  /* Save the channel of the last request */
  mbox->last_channel = channel;

  /* Check which port is being written */
  switch (addr)
  {
    case MBOX_WRITE:
    {
      switch (channel)
      {
        case 1:   /* Framebuffer */
        {
          fb_request(&mbox->emu->fb, data);
          return;
        }
        default:
        {
          emulator_error(mbox->emu, "Wrong channel 0x%x", channel);
          return;
        }
      }
      break;
    }
  }

  emulator_error(mbox->emu, "Mailbox unimplemented 0x%08x", addr);
}
