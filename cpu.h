/* This file is part of the Team 28 Project
 * Licensing information can be found in the LICENSE file
 * (C) 2014 The Team 28 Authors. All rights reserved.
 */

#ifndef __CPU_H__
#define __CPU_H__

/**
 * ARM condition codes
 */
typedef enum
{
  CC_EQ = 0x00,
  CC_NE = 0x01,
  CC_CS = 0x02,
  CC_CC = 0x03,
  CC_MI = 0x04,
  CC_PL = 0x05,
  CC_VS = 0x06,
  CC_VC = 0x07,
  CC_HI = 0x08,
  CC_LS = 0x09,
  CC_GE = 0x0a,
  CC_LT = 0x0b,
  CC_GT = 0x0c,
  CC_LE = 0x0d,
  CC_AL = 0x0e
} armCond_t;

/**
 * ARM registers
 */
typedef enum
{
  R0  = 0x0,
  R1  = 0x1,
  R2  = 0x2,
  R3  = 0x3,
  R4  = 0x4,
  R5  = 0x5,
  R6  = 0x6,
  R7  = 0x7,
  R8  = 0x8,
  R9  = 0x9,
  R10 = 0xA,
  R11 = 0xB,
  R12 = 0xC,
  SP  = 0xD,
  LR  = 0xE,
  PC  = 0xF
} armReg_t;

/**
 * ARM operating modes
 */
typedef enum
{
  MODE_USR = 0x10,
  MODE_FIQ = 0x11,
  MODE_IRQ = 0x12,
  MODE_SVC = 0x13,
  MODE_ABT = 0x17,
  MODE_UND = 0x1B,
  MODE_SYS = 0x1F
} armMode_t;

/**
 * CPU data - registers, flags etc
 */
typedef struct
{
  memory_t    *memory;
  emulator_t  *emu;

  /* User mode registers */
  union
  {
    int32_t r[16];
    struct
    {
      int32_t r0;
      int32_t r1;
      int32_t r2;
      int32_t r3;
      int32_t r4;
      int32_t r5;
      int32_t r6;
      int32_t r7;
      int32_t r8;
      int32_t r9;
      int32_t r10;
      int32_t r11;
      int32_t r12;
      int32_t sp;
      int32_t lr;
      int32_t pc;
    } reg;
  } r_usr;

  /* Banked FIQ */
  union
  {
    int32_t r[7];
    struct
    {
      int32_t r8;
      int32_t r9;
      int32_t r10;
      int32_t r11;
      int32_t r12;
      int32_t r13;
      int32_t r14;
    } reg;
  } r_fiq;

  /* Banked SVC */
  union
  {
    int32_t r[2];
    struct
    {
      int32_t r13;
      int32_t r14;
    } reg;
  } r_svc;

  /* Banked UND */
  union
  {
    int32_t r[2];
    struct
    {
      int32_t r13;
      int32_t r14;
    } reg;
  } r_und;

  /* Banked ABT */
  union
  {
    int32_t r[2];
    struct
    {
      int32_t r13;
      int32_t r14;
    } reg;
  } r_abt;

  /* Banked IRQ */
  union
  {
    int32_t r[2];
    struct
    {
      int32_t r13;
      int32_t r14;
    } reg;
  } r_irq;

  /* Saved program status register */
  struct
  {
    uint32_t svc;
    uint32_t abt;
    uint32_t und;
    uint32_t irq;
    uint32_t fiq;
  } spsr;

  /* Current program status register */
  union
  {
    uint32_t r;
    struct
    {
      uint32_t m:5;
      uint32_t t:1;
      uint32_t f:1;
      uint32_t i:1;
      uint32_t  :20;
      uint32_t v:1;
      uint32_t c:1;
      uint32_t z:1;
      uint32_t n:1;
    } b;
  } cpsr;
} cpu_t;

uint32_t cpu_read_register(const cpu_t* cpu, armReg_t reg);
void cpu_write_register(cpu_t* cpu, armReg_t reg, uint32_t value);
void cpu_init(cpu_t*, emulator_t*);
void cpu_tick(cpu_t*);
void cpu_destroy(cpu_t*);
void cpu_dump(cpu_t*);

#endif /* __CPU_H__ */
