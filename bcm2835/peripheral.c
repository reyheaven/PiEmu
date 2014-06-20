/* This file is part of the Team 28 Project
 * Licensing information can be found in the LICENSE file
 * (C) 2014 The Team 28 Authors. All rights reserved.
 */
#include "common.h"

/**
 * Initialises peripherials
 * @param pr    Peripherials structure
 * @param emu   Reference to the emulator structure
 */
void
pr_init(peripheral_t* pr, emulator_t* emu)
{
  assert(pr);
  assert(emu);

  pr->emu = emu;

  /* SPI module 1 */
  pr->spi1_enable = 0;

  /* SPI module 2 */
  pr->spi2_enable = 0;

  /* Mini UART */
  pr->uart_enable = 0;
  pr->uart_bits = 7;
  pr->uart_dlab = 0;
}

/**
 * Cleans up memory used by peripherials
 * @param pr    Peripherials structure
 */
void
pr_destroy(peripheral_t* UNUSED(pr))
{
}

/**
 * Handles a write to a peripherial register
 * @param pr    Peripherials structure
 * @param addr  Port number
 * @param data  Data received
 */
void
pr_write(peripheral_t* pr, uint32_t addr, uint8_t data)
{
  assert(pr);

  switch (addr)
  {
    case AUX_ENABLES:
    {
      /* Enables / disables peripherials */
      pr->uart_enable = data & 0x1 ? 1 : 0;
      pr->spi1_enable = data & 0x2 ? 1 : 0;
      pr->spi2_enable = data & 0x4 ? 1 : 0;
      return;
    }
    case AUX_MU_IER_REG:
    {
      if (pr->uart_dlab)
      {
        /* MSB of baud rate register */
        pr->uart_baud_rate = (pr->uart_baud_rate & 0xFF) | ((data >> 8) & 0xFF);
      }
      else
      {
        /* Enables / disables interrupts */
        pr->irq_rx = data & 0x1 ? 1 : 0;
        pr->irq_tx = data & 0x1 ? 1 : 0;
      }
      return;
    }
    case AUX_MU_LCR_REG:
    {
      /* TODO: implement other bits */
      pr->uart_bits = data & 0x1 ? 8 : 7;
      return;
    }
    case AUX_MU_BAUD_REG:
    {
      pr->uart_baud_rate_counter = data & 0xFF;
      return;
    }
    case AUX_MU_IO_REG:
    {
      if (pr->uart_dlab)
      {
        /* LSB of baud rate register */
        pr->uart_baud_rate = (pr->uart_baud_rate & 0xFF00) | (data & 0xFF);
      }
      else
      {
        /* Dump output to stdout */
        emulator_info(pr->emu, "%c", data & 0xFF);
      }
      return;
    }
  }

  emulator_error(pr->emu, "Unsupported peripherial write: %08x", addr);
}

/**
 * Handles a read from a peripherial port
 * @param pr   Peripherial structure
 * @param addr Port address
 * @return     Read value
 */
uint32_t
pr_read(peripheral_t* pr, uint32_t addr)
{
  assert(pr);

  switch (addr)
  {
    case AUX_ENABLES:
    {
      /* Returns a bitmask of enabled peripherials */
      return (pr->uart_enable ? 0x1 : 0x0) |
             (pr->spi1_enable ? 0x2 : 0x0) |
             (pr->spi2_enable ? 0x4 : 0x0);
    }
    case AUX_MU_IER_REG:
    {
      /* Returns a bitmask of interrupt status */
      return (pr->irq_rx ? 0x1 : 0x0) |
             (pr->irq_tx ? 0x2 : 0x0);
    }
    case AUX_MU_LSR_REG:
    {
      /* Transmitter always ready */
      return 0x60;
    }
    case AUX_MU_IO_REG:
    {
      if (pr->uart_dlab)
      {
        /* LSB of baud rate register */
        return pr->uart_baud_rate & 0xFF;
      }
      else
      {
        /* Always read 0 */
        return 0x00;
      }
    }
  }

  emulator_error(pr->emu, "Unsupported peripherial read: %08x", addr);
  return 0;
}

/**
 * Checks whether a port is a peripherial or not
 * @param addr  Port to be tested
 * @return      1 if port is peripherial
 */
int
pr_is_aux_port(uint32_t addr)
{
  return addr >= AUX_BASE && addr <= AUX_SPI1_CNTL1_REG;
}
