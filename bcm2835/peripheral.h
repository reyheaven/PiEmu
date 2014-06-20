/* This file is part of the Team 28 Project
 * Licensing information can be found in the LICENSE file
 * (C) 2014 The Team 28 Authors. All rights reserved.
 */
#ifndef __PERIPHERAL_H__
#define __PERIPHERAL_H__

/**
 * List of auxiliary peripherial ports
 */
typedef enum
{
  AUX_BASE           = 0x20215000,
  AUX_IRQ            = AUX_BASE + 0x00,
  AUX_ENABLES        = AUX_BASE + 0x04,
  AUX_MU_IO_REG      = AUX_BASE + 0x40,
  AUX_MU_IER_REG     = AUX_BASE + 0x44,
  AUX_MU_IIR_REG     = AUX_BASE + 0x48,
  AUX_MU_LCR_REG     = AUX_BASE + 0x4C,
  AUX_MU_MCR_REG     = AUX_BASE + 0x50,
  AUX_MU_LSR_REG     = AUX_BASE + 0x54,
  AUX_MU_MSR_REG     = AUX_BASE + 0x58,
  AUX_MU_SCRATCH     = AUX_BASE + 0x5C,
  AUX_MU_CNTL_REG    = AUX_BASE + 0x60,
  AUX_MU_STAT_REG    = AUX_BASE + 0x64,
  AUX_MU_BAUD_REG    = AUX_BASE + 0x68,
  AUX_SPI0_CNTL0_REG = AUX_BASE + 0x80,
  AUX_SPI0_CNTL1_REG = AUX_BASE + 0x84,
  AUX_SPI0_STAT_REG  = AUX_BASE + 0x88,
  AUX_SPI0_IO_REG    = AUX_BASE + 0x90,
  AUX_SPI0_PEEK_REG  = AUX_BASE + 0x94,
  AUX_SPI1_CNTL0_REG = AUX_BASE + 0xC0,
  AUX_SPI1_CNTL1_REG = AUX_BASE + 0xC4
} pr_port_t;

/**
 * Peripherials state
 */
typedef struct
{
  /* Reference to the emulator */
  emulator_t* emu;

  /* True if SPI 1 module is enabled */
  int spi1_enable;
  /* True if SPI 2 module is enabled */
  int spi2_enable;

  /* Transmit interrupt */
  int irq_tx;
  /* Receive interrupt */
  int irq_rx;

  /* True if uart is enabled */
  int uart_enable;
  /* Uart bits */
  int uart_bits;
  /* Baud rate counter */
  int uart_baud_rate_counter;
  /* Baud rate */
  int uart_baud_rate;
  /* UART DLAB */
  int uart_dlab;
} peripheral_t;

void pr_init(peripheral_t*, emulator_t*);
void pr_destroy(peripheral_t*);
void pr_write(peripheral_t*, uint32_t port, uint8_t data);
uint32_t pr_read(peripheral_t*, uint32_t port);
int pr_is_aux_port(uint32_t addr);

#endif /* __PERIPHERAL_H__ */
