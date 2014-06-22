/* This file is part of the Team 28 Project
 * Licensing information can be found in the LICENSE file
 * (C) 2014 The Team 28 Authors. All rights reserved.
 */
#include "common.h"

/**
 * Checks the condition and returns 1 if the instruction should be executed
 *
 * @param cpu Reference to the CPU structure
 * @param cc  Condition code
 * @return    1 if condition matches
 */
int
check_cond(const cpu_t* cpu, armCond_t cc)
{
  switch (cc)
  {
    case CC_EQ: return cpu->cpsr.b.z;
    case CC_NE: return !cpu->cpsr.b.z;
    case CC_CS: return cpu->cpsr.b.c;
    case CC_CC: return !cpu->cpsr.b.c;
    case CC_MI: return cpu->cpsr.b.n;
    case CC_PL: return !cpu->cpsr.b.n;
    case CC_VS: return cpu->cpsr.b.v;
    case CC_VC: return !cpu->cpsr.b.v;
    case CC_HI: return cpu->cpsr.b.c && !cpu->cpsr.b.z;
    case CC_LS: return !cpu->cpsr.b.c || cpu->cpsr.b.z;
    case CC_GE: return cpu->cpsr.b.n == cpu->cpsr.b.v;
    case CC_LT: return cpu->cpsr.b.n != cpu->cpsr.b.v;
    case CC_GT: return !cpu->cpsr.b.z && cpu->cpsr.b.n == cpu->cpsr.b.v;
    case CC_LE: return cpu->cpsr.b.z || cpu->cpsr.b.n != cpu->cpsr.b.v;
    case CC_AL: return 1; // Always executed
    default:
    {
      return 0;//emulator_fatal(cpu->emu, "Invalid condition code: 0xF");
    }
  }
}

/**
 * Reads the value of a register
 * @param cpu Reference to the CPU structure
 * @param reg Register index
 * @return Value of the register
 */
uint32_t
cpu_read_register(const cpu_t* cpu, armReg_t reg)
{
  assert(cpu != NULL);
  assert(reg <= 0xF);

  switch (reg)
  {
    case R0 ... R7:
    {
      /* First 8 regs not banked */
      return cpu->r_usr.r[reg];
    }
    case R8 ... R12:
    {
      if (cpu->cpsr.b.m == MODE_FIQ)
      {
        /* FIQ banked regs */
        return cpu->r_fiq.r[reg - 8];
      }
      else
      {
        /* USR/SYS regs */
        return cpu->r_usr.r[reg];
      }
    }
    case SP: case LR:
    {
      /* SP/LR always banked */
      switch (cpu->cpsr.b.m)
      {
        case MODE_USR: case MODE_SYS: return cpu->r_usr.r[reg];
        case MODE_FIQ: return cpu->r_fiq.r[reg - 8];
        case MODE_IRQ: return cpu->r_irq.r[reg - 13];
        case MODE_SVC: return cpu->r_svc.r[reg - 13];
        case MODE_ABT: return cpu->r_abt.r[reg - 13];
        case MODE_UND: return cpu->r_und.r[reg - 13];
      }
      emulator_fatal(cpu->emu, "Invalid mode");
    }
    case PC:
    {
      /* PC not banked */
      return cpu->r_usr.reg.pc + 4;
    }
  }

  __builtin_unreachable();
}

/**
 * Writes a new value to a register
 * @param cpu Reference to the CPU structure
 * @param reg Register index
 * @param value Value to be written
 */
void
cpu_write_register(cpu_t* cpu, armReg_t reg, uint32_t value)
{
  assert(cpu != NULL);
  assert(reg <= 0xF);

  switch (reg)
  {
    case R0 ... R7:
    {
      /* First 8 regs not banked */
      cpu->r_usr.r[reg] = value;
      break;
    }
    case R8 ... R12:
    {
      if (cpu->cpsr.b.m == MODE_FIQ)
      {
        /* FIQ banked regs */
        cpu->r_fiq.r[reg - 8] = value;
      }
      else
      {
        /* USR/SYS regs */
        cpu->r_usr.r[reg] = value;
      }
      break;
    }
    case SP: case LR:
    {
      /* SP/LR always banked */
      switch (cpu->cpsr.b.m)
      {
        case MODE_USR: case MODE_SYS: cpu->r_usr.r[reg] = value; break;
        case MODE_FIQ: cpu->r_fiq.r[reg - 8] = value; break;
        case MODE_IRQ: cpu->r_irq.r[reg - 13] = value; break;
        case MODE_SVC: cpu->r_svc.r[reg - 13] = value; break;
        case MODE_ABT: cpu->r_abt.r[reg - 13] = value; break;
        case MODE_UND: cpu->r_und.r[reg - 13] = value; break;
      }
      break;
    }
    case PC:
    {
      /* PC not banked */
      cpu->r_usr.reg.pc = value;
      break;
    }
  }
}


/* Reads the current mode's SPSR.
 *
 * @param cpu Reference to the CPU structure.
 */
static inline uint32_t
read_spsr(cpu_t* cpu)
{
  switch(cpu->cpsr.b.m)
  {
    case MODE_SVC: return cpu->spsr.svc;
    case MODE_ABT: return cpu->spsr.abt;
    case MODE_UND: return cpu->spsr.und;
    case MODE_IRQ: return cpu->spsr.irq;
    case MODE_FIQ: return cpu->spsr.fiq;
    default: emulator_error(cpu->emu, "Invalid mode for reading SPSR");
  }

  return 0;
}

/**
 * Write to the current mode's SPSR
 * @param cpu Reference to the CPU structure.
 * @param value Value to write
 */
static inline void
write_spsr(cpu_t* cpu, uint32_t value)
{
  switch (cpu->cpsr.b.m)
  {
    case MODE_SVC: cpu->spsr.svc = value; break;
    case MODE_ABT: cpu->spsr.abt = value; break;
    case MODE_UND: cpu->spsr.und = value; break;
    case MODE_IRQ: cpu->spsr.irq = value; break;
    case MODE_FIQ: cpu->spsr.fiq = value; break;
    default: emulator_error(cpu->emu, "Invalid mode for writing SPSR");
  }
}

/**
 * Enters given mode
 * @param cpu Reference to cpu structure
 * @param mode Mode to be entered
 */
inline static void
change_mode(cpu_t* cpu, armMode_t mode)
{
  switch (mode)
  {
    case MODE_USR ... MODE_SYS:
    {
      cpu->cpsr.b.m = mode;
      return;
    }
    default:
    {
      emulator_fatal(cpu->emu, "Invalid mode");
      return;
    }
  }
}

static inline int64_t
ts64(uint64_t u)
{
  union
  {
    uint64_t u;
    int64_t s;
  } r;
  r.u = u;
  return r.s;
}

static inline uint64_t
tu64(int64_t s)
{
  union
  {
    uint64_t u;
    int64_t s;
  } r;
  r.s = s;
  return r.u;
}

static inline void
instr_multiply_long(cpu_t* cpu, op_multiply_long_t* opcode)
{
  union
  {
    struct
    {
      uint32_t lo;
      uint32_t hi;
    };
    uint64_t large;
  } output;

  // Initialise "output" to current lo, hi registers if accumulating
  // or 0 otherwise.
  if (opcode->a)
  {
    output.lo = cpu_read_register(cpu, opcode->RdLo);
    output.hi = cpu_read_register(cpu, opcode->RdHi);
  }
  else
  {
    output.lo = 0;
    output.hi = 0;
  }

  // Operands
  uint32_t operandA = cpu_read_register(cpu, opcode->Rm);
  uint32_t operandB = cpu_read_register(cpu, opcode->Rs);

  // Calculate result
  if (opcode->u)
  {
    int64_t result = ts64(operandA) * ts64(operandB) + ts64(output.large);
    output.large = tu64(result);
  }
  else
  {
    output.large = (uint64_t)operandA * (uint64_t)operandB + output.large;
  }

  // Set CPSR flags
  if (opcode->s)
  {
    cpu->cpsr.b.n = output.hi >> 31;
    cpu->cpsr.b.z = output.large == 0LL;
  }

  // Write high and low
  cpu_write_register(cpu, opcode->RdLo, output.lo);
  cpu_write_register(cpu, opcode->RdHi, output.hi);
}

static inline void
instr_multiply(cpu_t* cpu, op_multiply_t* opcode)
{
  int32_t res;
  int32_t op1, op2, opA;

  op1 = cpu_read_register(cpu, opcode->Rm);
  op2 = cpu_read_register(cpu, opcode->Rs);

  if (opcode->a)
  {
    /* MLA */
    opA = cpu_read_register(cpu, opcode->Rn);
    res = opA + op1 * op2;
  }
  else
  {
    /* MUL */
    res = op1 * op2;
  }

  // Set flags
  if (opcode->s)
  {
    cpu->cpsr.b.z = res == 0;
    cpu->cpsr.b.n = res >> 31;
  }

  cpu_write_register(cpu, opcode->Rd, res);
}

/**
 * Implements the MRS instruction
 * @param cpu CPU context
 * @param opcode Instruction layout
 */
static void
instr_mrs(cpu_t* cpu, op_mrs_t* opcode)
{
  // Choose the correct destination register
  if (opcode->Ps == 0)
  {
    cpu_write_register(cpu, opcode->Rd, cpu->cpsr.r);
  }
  else
  {
    if (cpu->cpsr.b.m == MODE_USR)
    {
      cpu_write_register(cpu, opcode->Rd, read_spsr(cpu));
    }
    else
    {
      emulator_fatal(cpu->emu, "Cannot read from SPSR in user mode");
    }
  }
}

/**
 * Write PSR
 * @param cpu CPU context
 * @param Pd 0 if CPSR, 1 if SPSR
 * @param value Value to write to PSR
 * @param flags Set to true if writing flags only, false otherwise
 */
static inline void
write_psr(cpu_t* cpu, uint32_t Pd, uint32_t value, uint32_t flags)
{
  // If CPU is in user mode, always write flags
  if (flags || cpu->cpsr.b.m == MODE_USR)
  {
    // Leave out either 4 or 12 bits depending on "flags"
    uint32_t mask = 0xF0000000;
    value &= mask;

    // Choose the correct destination register
    if (Pd == 0)
    {
      // Carry over remaining bits not covered by the mask instead of
      // setting them to 0
      value |= cpu->cpsr.r & ~mask;
      cpu->cpsr.r = value;
    }
    else
    {
      if (cpu->cpsr.b.m == MODE_USR)
      {
        emulator_fatal(cpu->emu, "Cannot write to SPSR in user mode");
      }
      else
      {
        value |= read_spsr(cpu) & ~mask;
        write_spsr(cpu, value);
      }
    }
  }
  else
  {
    // Choose the correct destination register
    if (Pd == 0)
    {
      cpu->cpsr.r = value;
    }
    else
    {
      write_spsr(cpu, value);
    }
  }
}

/**
 * Implements the MSR instruction
 * @param cpu CPU context
 * @param opcode Instruction layout
 */
static void
instr_msr_psr(cpu_t* cpu, op_msr_psr_t* opcode)
{
  write_psr(cpu, opcode->Pd, cpu_read_register(cpu, opcode->Rm), 0);
}

/**
 * Rotates a number to the right
 *
 * @param value Number to be rotated
 * @param shift Number of bits shifted
 * @return Rotated number
 */
static inline int32_t
rotate_right(int32_t value, uint8_t shift)
{
  shift &= 0x1F;
  return (value >> shift) | (value << (32 - shift));
}

/**
 * Calculates operand2/offset for single data processing/transfer instructions
 * @param cpu Reference to the cpu structure
 * @param imm Unsigned immidiate value
 * @param s   Bit used to set flags if necessary
 * @return operand2/offset
 */
static inline int32_t
compute_offset_operand2(cpu_t *cpu, uint32_t imm, uint8_t s)
{
  assert(cpu);

  int32_t res = 0;
  uint32_t rm_data, shift_amount, shift_type;

  rm_data = cpu_read_register(cpu, imm & 0x0000000F);
  shift_type = (imm >> 5) & 0x00000003;

  /* if bit 4 is set shift is specified by bottom byte of register Rs(11 - 8)
     otherwise it is specified by a 5-bit unsigned integer (11 - 7) */
  if ((imm >> 4) & 0x00000001)
  {
    uint32_t rs = (imm >> 8) & 0x0000000F;

    /* PC must not be specified as the register offset(rm) */
    if (rs == PC)
    {
      emulator_fatal(cpu->emu, "PC cannot be used as offset");
    }

    shift_amount = cpu_read_register(cpu, rs) & 0x000000FF;
  }
  else
  {
    shift_amount = (imm >> 7) & 0x0000001F;
  }

  if (shift_amount == 0)
  {
    res = rm_data;
  }
  else
  {
    switch (shift_type)
    {
      case 0x0:
      {
        /* Logical left */
        if (shift_amount >= 32)
        {
          res = 0;
          if (s)
          {
            cpu->cpsr.b.c = shift_amount == 32 && (rm_data & 0x1);
          }
        }
        else
        {
          if (s)
          {
            cpu->cpsr.b.c = (rm_data >> (32 - shift_amount)) & 0x1;
          }
          res = rm_data << shift_amount;
        }
        break;
      }
      case 0x1:
      {
        /* Logical right */
        if (shift_amount >= 32)
        {
          res = 0;
          if (s)
          {
            cpu->cpsr.b.c = shift_amount == 32 && (rm_data >> 31);
          }
        }
        else
        {
          if (s)
          {
            cpu->cpsr.b.c = (rm_data >> (shift_amount - 1)) & 0x1;
          }
          res = rm_data >> shift_amount;
        }
        break;
      }
      case 0x2:
      {
        /* Arithmetic right */
        if (shift_amount >= 32)
        {
          uint32_t bit31 = (rm_data >> 31);
          res = bit31 ? 0xFFFFFFFF : 0x0;
          if (s)
          {
            cpu->cpsr.b.c = bit31;
          }
        }
        else
        {
          if (s)
          {
            cpu->cpsr.b.c = (rm_data >> (shift_amount - 1)) & 0x1;
          }
          res = ((int32_t) rm_data) >> shift_amount;
        }
        break;
      }
      case 0x3:
      {
        /* Rotate right */
        while (shift_amount > 32)
        {
          shift_amount -= 32;
        }

        if (shift_amount == 32)
        {
         res = rm_data;
          if (s)
          {
            cpu->cpsr.b.c = rm_data >> 31;
          }
        }
        else
        {
          if (s)
          {
            cpu->cpsr.b.c = (rm_data >> (shift_amount - 1)) & 0x1;
          }
          res = rotate_right(rm_data, shift_amount);
        }
        break;
      }
    }
  }
  return res;
}

/**
 * Write to PSR - flags only
 * @param cpu CPU context
 * @param opcode Instruction opcode
 */
static void
instr_msr_psrf(cpu_t* cpu, op_msr_psrf_t* opcode)
{
  if (opcode->i == 0)
  {
    // Register is stored in bits 0 to 3
    write_psr(cpu, opcode->Pd, cpu_read_register(cpu, opcode->src & 0xf), 1);
  }
  else
  {
    // Value is stored in bits 0 to 7, and shift is stored in bits 8 to 11
    uint32_t value = compute_offset_operand2(
      cpu, opcode->src & 0xff, (opcode->src >> 8) & 0xf);
    write_psr(cpu, opcode->Pd, value, 1);
  }
}

/**
 * Emulates a data processing instruction
 *
 * @param cpu    Reference to the CPU structure
 * @param opcode Decoded opcode
 */
static inline void
instr_single_data_processing(cpu_t* cpu, op_data_proc_t* opcode)
{
  int32_t op1, op2, res;
  int64_t res64;

  /* Get first operand */
  op1 = cpu_read_register(cpu, opcode->Rn);

  /* Read operand 2 */
  if (opcode->i)
  {
    /* Operand 2 is an immediate value */
    uint32_t rotate_amount = (opcode->imm >> 8) & 0xF;
    uint32_t imm = 0x0;

    /* Zero extend imm to 32 bits */
    imm |= opcode->imm & 0xFF;
    op2 = rotate_right(imm, rotate_amount * 2);
  }
  else
  {
    op2 = compute_offset_operand2(cpu, ((opcode->imm) & 0x00000FFF), opcode->s);
  }

  /* Examine the opcode and execute the operation */
  switch (opcode->op)
  {
    case 0x0: /* AND */
    case 0x8: /* TST */
    {
      res = op1 & op2;

      if (opcode->s || opcode->op == 0x08)
      {
        cpu->cpsr.b.z = res == 0;
        cpu->cpsr.b.n = res >> 31;
      }

      if (opcode->op == 0x0)
      {
        cpu_write_register(cpu, opcode->Rd, res);
      }
      return;
    }
    case 0x1: /* EOR */
    case 0x9: /* TEQ */
    {
      res = op1 ^ op2;
      if (opcode->s || opcode->op == 0x09)
      {
        cpu->cpsr.b.z = res == 0;
        cpu->cpsr.b.n = (res & (1 << 31)) != 0;
      }

      if (opcode->op == 0x1)
      {
        cpu_write_register(cpu, opcode->Rd, res);
      }
      return;
    }
    case 0x2: /* SUB */
    case 0xA: /* CMP */
    case 0x3: /* RSB */
    {
      if (opcode->op == 0x3)
      {
        op1 ^= op2;
        op2 ^= op1;
        op1 ^= op2;
      }

      res64 = (int64_t)op1 - (int64_t)op2;
      res = res64 & ((1LL << 32) - 1);

      if (opcode->s || opcode->op == 0x0A)
      {
        cpu->cpsr.b.z = res == 0;
        cpu->cpsr.b.n = (res & (1 << 31)) != 0;
        cpu->cpsr.b.c = ((res64 >> 32) & 1) == 0;

        cpu->cpsr.b.v = 0;
        if (op1 >= 0 && op2 < 0 && res < 0)
        {
          /* positive - negative = negative => overflow */
          cpu->cpsr.b.v = 1;
        }
        else if (op1 < 0 && op2 >= 0 && res >= 0)
        {
          /* negative - positive = positive => overflow */
          cpu->cpsr.b.v = 1;
        }
      }

      if (opcode->op == 0x2 || opcode->op == 0x3)
      {
        cpu_write_register(cpu, opcode->Rd, res);
      }

      return;
    }
    case 0x4: /* ADD */
    case 0xB: /* CMN */
    {
      res64 = (int64_t)op1 + (int64_t)op2;
      res = res64 & ((1LL << 32) - 1);

      if (opcode->s || opcode->op == 0xB)
      {
        cpu->cpsr.b.z = res == 0;
        cpu->cpsr.b.n = (res & (1 << 31)) != 0;
        cpu->cpsr.b.c = (res64 >> 32) != 0;

        cpu->cpsr.b.v = 0;
        if (op1 < 0 && op2 < 0 && res > 0)
        {
          /* negative + negative = positive => overflow */
          cpu->cpsr.b.v = 1;
        }

        if (op1 > 0 && op2 > 0 && res < 0)
        {
          /* positive + positive = negative => overflow */
          cpu->cpsr.b.v = 1;
        }
      }
      if (opcode->op == 0x4)
      {
        cpu_write_register(cpu, opcode->Rd, res);
      }
      return;
    }
    case 0x5: /* ADC */
    {
      res64 = (int64_t)op1 + (int64_t)op2 + (int64_t)cpu->cpsr.b.c;
      res = res64 & ((1LL << 32) - 1);

      if (opcode->s)
      {
        cpu->cpsr.b.z = res == 0;
        cpu->cpsr.b.n = res >> 31;
        if (op1 < 0 && op2 < 0 && res > 0)
        {
          /* negative + negative = positive => overflow */
          cpu->cpsr.b.v = 1;
        }

        if (op1 > 0 && op2 > 0 && res < 0)
        {
          /* positive + positive = negative => overflow */
          cpu->cpsr.b.v = 1;
        }
        cpu->cpsr.b.c = res64 >> 32 != 0;
      }
      cpu_write_register(cpu, opcode->Rd, res);
      return ;
    }
    case 0x6: /* SBC  => op1 - op2 + carry - 1 */
    case 0x7: /* RSC  => op2 - op1 + carry - 1 */
    {
      if (opcode->op == 0x7){
        int32_t t;

        t = op1;
        op1 = op2;
        op2 = t;
      }

      res64 = (int64_t)op1 - (int64_t)op2 + (int64_t)cpu->cpsr.b.c - 1LL;
      res = res64 & ((1LL << 32) - 1);

      if (opcode->s)
      {
        cpu->cpsr.b.z = res == 0;
        cpu->cpsr.b.n = (res & (1 << 31)) != 0;
        cpu->cpsr.b.c = (~(res64 >> 32)) != 0;

        cpu->cpsr.b.v = 0;
        if (op1 >= 0 && op2 < 0 && res < 0)
        {
          /* positive - negative = negative => overflow */
          cpu->cpsr.b.v = 1;
        }
        else if (op1 < 0 && op2 >= 0 && res >= 0)
        {
          /* negative - positive = positive => overflow */
          cpu->cpsr.b.v = 1;
        }
      }
      cpu_write_register(cpu, opcode->Rd, res);
      return;
    }
    case 0xC: /* ORR */
    {
      res = op1 | op2;
      if (opcode->s)
      {
        cpu->cpsr.b.z = res == 0;
        cpu->cpsr.b.n = res >> 31;
      }

      cpu_write_register(cpu, opcode->Rd, res);
      return;
    }
    case 0xD: /* MOV */
    {
      if (opcode->s)
      {
        cpu->cpsr.b.z = op2 == 0;
        cpu->cpsr.b.n = op2 >> 31;
      }

      cpu_write_register(cpu, opcode->Rd, op2);
      return;
    }
    case 0xE: /* BIC */
    {
      res = op1 & (~op2);

      if (opcode->s)
      {
        cpu->cpsr.b.z = res == 0;
        cpu->cpsr.b.n = res >> 31;
      }

      cpu_write_register(cpu, opcode->Rd, res);
      return;
    }
    case 0xF: /* MVN */
    {
      res = ~op2;
      if (opcode->s)
      {
        cpu->cpsr.b.z = res == 0;
        cpu->cpsr.b.n = res >> 31;
      }

      cpu_write_register(cpu, opcode->Rd, res);
      return;
    }
  }
}

/**
 * Executes a block data transfer instruction
 * @param cpu Reference to the CPU structure
 * @param opcode Opcode decoder
 */
static inline void
instr_block_data_transfer(cpu_t* cpu, op_block_data_trans_t* opcode)
{
  /* The register list can't be empty */
  if (opcode->rl == 0)
  {
    emulator_fatal(cpu->emu, "The register list cannot be empty");
  }

  /* The base register should never be PC */
  if (opcode->rn == PC)
  {
    emulator_fatal(cpu->emu, "Base register cannot be PC");
  }

  /* Assert the s bit is only set in privileged user mode */
  if (opcode->s)
  {
    if (cpu->cpsr.b.m == MODE_USR || cpu->cpsr.b.m == MODE_SYS)
    {
      emulator_fatal(cpu->emu, "Force user mode set in non-priveleged mode");
    }
  }

  uint32_t address = cpu_read_register(cpu, opcode->rn) & 0xfffffffc;
  uint32_t offset = (opcode->u) ? 4 : -4;
  int16_t reg;

  /* Push registers */
  for (
    reg = opcode->u ? 0 : 15;
    opcode->u ? (reg < 16) : (reg >= 0);
    reg += opcode->u ? 1 : -1)
  {
    /* Test whether to include register `reg` */
    if (!(opcode->rl & (1 << reg))){
      continue;
    }

    /* pre-increment addressing */
    if (opcode->p)
    {
      address += offset;
    }

    /* The spec is somewhat ambigous about when base is included in reg list.
     * It's unclear what the expected behaviour is for pre-increment addressing
     * This seems like the most logical functionality correct. */
    if (opcode->w && reg == opcode->rn)
    {
      cpu_write_register(cpu, opcode->rn, address);
    }

    /* Load/store from memory */
    if (opcode->l)
    {
      if (opcode->s)
      {
        cpu->r_usr.r[reg] = memory_read_dword_le(cpu->memory, address);
      }
      else
      {
        cpu_write_register(cpu, reg, memory_read_dword_le(cpu->memory, address));
      }
    }
    else
    {
      /* If S bit set, transfer user bank */
      if (opcode->s)
      {
        memory_write_dword_le(cpu->memory, address, cpu->r_usr.r[reg]);
      }
      else
      {
        memory_write_dword_le(cpu->memory, address, cpu_read_register(cpu, reg));
      }
    }

    /* Post-increment addressing */
    if (!opcode->p)
    {
      address += offset;
    }
  }

  /* If loading PC and user bank bit is set,
   * copy the spsr for the current mode to flags register */
  if (opcode->l && opcode->s && opcode->rl & (1 << PC))
  {
    cpu->cpsr.r = read_spsr(cpu);
  }

  /* Write-back if enabled and base is not in the register list*/
  if (opcode->w && (opcode->rl & (1 << opcode->rn)) == 0)
  {
    cpu_write_register(cpu, opcode->rn, address);
  }
}

/**
 * Executes a branch or a branch with link instruction
 * BL: add offset to PC and set LR to PC + 4
 * B:  add offset to PC
 *
 * @param cpu Reference to the CPU context
 * @param opcode Opcode for the branch instruction
 */
static inline void
instr_branch(cpu_t* cpu, op_branch_t* opcode)
{
  uint32_t offset, pc, lr;

  /* Sign extend offset */
  offset = opcode->offset << 2;
  if (offset & (1 << 25))
  {
    offset |= ~0x03FFFFFF;
  }

  /* Add offset to PC */
  pc = cpu_read_register(cpu, PC);
  lr = pc - 4;
  pc = pc + offset;
  cpu_write_register(cpu, PC, pc);

  /* Branch with link */
  if (opcode->l)
  {
    cpu_write_register(cpu, LR, lr);
  }
}

/**
 * Executes a branch with exchange operation
 * BX: Assigns PC to the contents of Rn
 *
 * @param cpu Reference to the CPU context
 * @param opcode Opcode for the BX instruction
 */
static inline void
instr_branch_exchange(cpu_t* cpu, op_branch_exchange_t* opcode)
{
  if (opcode->Rn & 0x1)
  {
    emulator_fatal(cpu->emu, "Cannot switch to THUMB instruction set");
  }

  /* Write new PC value */
  uint32_t pc = cpu_read_register(cpu, opcode->Rn);
  cpu_write_register(cpu, PC, pc);
}

/**
 * Emulates Single Data Transfer Instructions
 * @param cpu - Reference to cpu structure
 * @param opcode - Reference to the opcode structure
 */
static inline void
instr_single_data_trans(cpu_t* cpu, op_single_data_trans_t* opcode)
{
  assert(cpu);
  assert(opcode);

  uint32_t rn, offset, addr;
  rn = cpu_read_register(cpu, opcode->rn);

  /* If i bit is set offset is interpreted as a shifted register,
     otherwise it is interpreted as an unsigned 12 bit immediate offset */
  if (opcode->i)
  {
    offset = compute_offset_operand2(cpu, ((opcode->offset) & 0x00000FFF), 0);
  }
  else
  {
    offset = (opcode->offset) & 0x00000FFF;
  }

  /* if p bit is set perform pre-indexing, post-indexing otherwise */
  if (opcode->p)
  {
    rn = opcode->u ? (rn + offset) : (rn - offset);
    addr = rn;
  }
  else
  {
    addr = rn;
    rn = opcode->u ? (rn + offset) : (rn - offset);
  }

  /* if l bit is set perform load, store otherwise */
  if (opcode->l)
  {
     /* if b is set load byte, word otherwise */
    if (opcode->b)
    {
      cpu_write_register(
        cpu, opcode->rd, (uint32_t)memory_read_byte(cpu->memory, addr));
    }
    else
    {
      cpu_write_register(
        cpu, opcode->rd, memory_read_dword_le(cpu->memory, addr));
     }
  }
  else
  {
    /* if b is set store byte, word otherwise */
    if (opcode->b)
    {
      memory_write_byte(cpu->memory, addr, cpu_read_register(cpu, opcode->rd));
    }
    else
    {
      memory_write_dword_le(cpu->memory, addr, cpu_read_register(cpu, opcode->rd));
    }
  }

  /* if post-indexing or w bit is set write back into the base register */
  if (opcode->w || !(opcode->p))
  {
    /* w must not be set if PC is specified as the base register */
    if (opcode->rn == PC)
    {
      emulator_fatal(cpu->emu, "Writeback to PC not allowed");
    }

    cpu_write_register(cpu, opcode->rn, rn);
  }
}

/**
 * Emulates Single Data Swap Instructions
 * @param cpu - Reference to cpu structure
 * @param opcode - Reference to the opcode structure
 */
static inline void
instr_single_data_swap(cpu_t* cpu, op_single_data_swap_t* opcode)
{
  assert(cpu);
  assert(opcode);

  if (opcode->rd == PC || opcode->rn == PC || opcode->rm == PC)
  {
    emulator_fatal(cpu->emu,
      "PC cannot be used as an operand (Rd, Rn or Rm) in a SWAP instruction");
    return;
  }

  uint32_t swap_address = cpu_read_register(cpu, opcode->rn);

  /* if b bit is set swap byte quantity, word otherwise */
  if (opcode->b)
  {
    uint8_t tmp = memory_read_byte(cpu->memory, swap_address);
    memory_write_byte(
      cpu->memory, swap_address, cpu_read_register(cpu, opcode->rm));
    cpu_write_register(cpu, opcode->rd, tmp);
  }
  else
  {
    uint32_t tmp = memory_read_dword_le(cpu->memory, swap_address);
    memory_write_dword_le(
      cpu->memory, swap_address, cpu_read_register(cpu, opcode->rm));
    cpu_write_register(cpu, opcode->rd, tmp);
  }
}

/**
 * Method used by halfword and data transfer
 * @param cpu Reference to the cpu structure
 * @param op  Reference to the opcode structure
 * @param     Address address to store to/load from
 */
static inline void
hw_sd_transfer_fun_sel(cpu_t* cpu, op_hw_sd_trans_t* op, uint32_t address)
{
  /* bits s and h determine the operation */
  switch (op->sh)
  {
    case 0:
    {
      instr_single_data_swap(cpu, (op_single_data_swap_t*) op);
      break;
    }
    case 1:
    {
      /* if l is set load unsigned halfword form memory, store otherwise */
      if (op->l)
      {
        cpu_write_register(
          cpu, op->rd, memory_read_word_le(cpu->memory, address));
      }
      else
      {
        /* when PC is the source register (Rd) of a half-word store (strh),
           the stored address will be address of the instruction plus 12 */
        if (op->rd == PC)
        {
          address += 12;
        }

        memory_write_word_le(
          cpu->memory, address, (uint16_t)cpu_read_register(cpu, op->rd));
      }
      break;
    }
    case 2:
    {
      if (op->l)
      {
        int32_t val = 0x000000FF & memory_read_byte(cpu->memory, address);

        /* sign extend byte to 32 bits */
        if (val & (1 << 7))
        {
          val = val | 0xFFFFFF00;
        }
        cpu_write_register(cpu, op->rd, val);
      }
      else
      {
        emulator_fatal(cpu->emu,
          "l bit can't be 0, when signed operations have been selected");
      }
      break;
    }
    case 3:
    {
      if (op->l)
      {
        int32_t val = 0x0000FFFF & memory_read_word_le(cpu->memory, address);

        /* sign extend halfword to 32 bits */
        if (val & (1 << 15))
        {
          val = val | 0xFFFF0000;
        }
        cpu_write_register(cpu, op->rd, val);
      }
      else
      {
        emulator_fatal(cpu->emu,
          "l but can't be 0, when signed operaitons have been selected");
      }
      break;
    }
  }
}

/**
 * Emulates Halfword and signed data transfer (LDRH/STRH/LDRSB/LDRSH)
 * @param cpu Reference to the cpu structure
 * @param op Reference to the opcode structure
 */
static inline void
instr_hw_sd_transfer(cpu_t* cpu, op_hw_sd_trans_t* op)
{
  assert(cpu);
  assert(op);

  uint32_t base, offset;
  base = cpu_read_register(cpu, op->rn);
  offset = 0;

  /* calculate offset based on the offset type
   * if o bit is set use immediate offset, register offset otherwise */
  if (op->o)
  {
    offset = (offset | (op->rm_ln)) | ((op->hn) << 4);
  }
  else
  {
    if (op->rm_ln == PC)
    {
      emulator_fatal(cpu->emu, "PC used as offset");
      return;
    }

    offset = cpu_read_register(cpu, op->rm_ln);
  }

  /* if p bit is set add/substract offset before transfer,
   * after the transfer otherwise */
  if (op->p)
  {
    /* if u bit is set add offset to base, substract otherwise */
    if (op->u)
    {
      base += offset;
      hw_sd_transfer_fun_sel(cpu, op, base);
    }
    else
    {
      base -= offset;
      hw_sd_transfer_fun_sel(cpu, op, base);
    }
  }
  else
  {
    /* if u bit is set add offset to base, substract otherwise */
    if (op->u)
    {
      hw_sd_transfer_fun_sel(cpu, op, base);
      base += offset;
    }
    else
    {
      hw_sd_transfer_fun_sel(cpu, op, base);
      base -= offset;
    }
  }

  /* if w bit is set, write the new value in the base register */
  if (op->w || !(op->p))
  {
    if (op->rn == PC)
    {
      emulator_fatal(cpu->emu, "Cannot write back to PC");
      return;
    }
    cpu_write_register(cpu, op->rn, base);
  }
}

/**
 * Emulates a coprocessor data processing instruction
 * @param cpu Reference to the cpu structure
 * @param opcode Reference to the instruction structure
 */
static inline void
instr_coproc_data_proc(cpu_t* cpu, op_coproc_data_proc_t* opcode)
{
  assert(cpu);
  assert(opcode);

  uint32_t coproc = opcode->CP_number;

  switch (coproc)
  {
    case 10:
    {
      /* VFP single precisoin coprocessor */
      vfp_data_proc(&cpu->emu->vfp, opcode);
      break;
    }
    case 11:
    {
      /* VFP double precision coprocessor - unsupported */
      emulator_fatal(cpu->emu, "Double-precision VFP unsupported");
      break;
    }
    case 15:
    {
      /* Ignore this coprocessor */
      break;
    }
    default:
    {
      emulator_fatal(cpu->emu, "Unimplemented coprocessor CP%u", coproc);
    }
  }
}

/**
 * Emulates a coprocessor data processing instruction
 * @param cpu Reference to the cpu structure
 * @param opcode Reference to the instruction structure
 */
static inline void
instr_coproc_data_transfer(cpu_t* cpu, op_coproc_data_transfer_t* opcode)
{
  assert(cpu);
  assert(opcode);

  uint32_t coproc = opcode->CP_number;

  switch (coproc)
  {
    case 10:
    {
      /* VFP single precision coprocessor */
      vfp_data_transfer(&cpu->emu->vfp, opcode);
      break;
    }
    case 11:
    {
      /* VFP double precision coprocessor - unsupported */
      emulator_fatal(cpu->emu, "Double-precision VFP unsupported");
      break;
    }
    case 15:
    {
      /* Ignore this coprocessor */
      break;
    }
    default:
    {
      emulator_fatal(cpu->emu, "Unimplemented coprocessor CP%u", coproc);
    }
  }
}

/**
 * Emulates a coprocessor data processing instruction
 * @param cpu Reference to the cpu structure
 * @param opcode Reference to the instruction structure
 */
static inline void
instr_coproc_reg_transfer(cpu_t* cpu, op_coproc_reg_transfer_t* opcode)
{
  assert(cpu);
  assert(opcode);

  uint32_t coproc = opcode->CP_number;

  switch (coproc)
  {
    case 10:
    {
      /* VFP single precision coprocessor */
      vfp_reg_transfer(&cpu->emu->vfp, opcode);
      break;
    }
    case 11:
    {
      /* VFP double precision coprocessor - unsupported */
      emulator_fatal(cpu->emu, "Double-precision VFP unsupported");
      break;
    }
    case 15:
    {
      /* Ignore this coprocessor */
      break;
    }
    default:
    {
      emulator_fatal(cpu->emu, "Unimplemented coprocessor CP%u", coproc);
    }
  }
}

/**
 * Emulates software interrupt (SWI)
 * @param cpu Reference to the cpu structure
 * @param opcode Reference to the isntruction structure
 */
static inline void
instr_swi(cpu_t* cpu, op_swi_t* UNUSED(opcode))
{
  assert(cpu);

  /* Enter the supervisor mode */
  change_mode(cpu, MODE_SVC);

  /* Save PC in lr, with PC adjusted to the word after SWI instruction */
  cpu_write_register(cpu, LR, cpu_read_register(cpu, PC));

  /* Set PC to a fixed value (0x08) */
  cpu_write_register(cpu, PC, 0x08);

  /* Save CPSR in SPSR_svc */
  write_spsr(cpu, cpu->cpsr.r);
}

/**
 * Emulates undefined instruction
 * @param cpu Reference to the cpu structure
 */
static inline void
instr_undefined(cpu_t* cpu)
{
  /* Enter the undefined mode */
  change_mode(cpu, MODE_UND);

  /* Save PC in lr */
  cpu_write_register(cpu, LR, cpu_read_register(cpu, PC));

  /* take the undefined instruction trap */
  cpu_write_register(cpu, PC, 0x04);

  /* Save CPSR in SPSR_svc */
  write_spsr(cpu, cpu->cpsr.r);
}

static inline void
debug_break(cpu_t *cpu)
{
  char c;

  printf("Breakpoint reached!\nCommands\n");
  printf("\tc         - Dump CPU\n");
  printf("\tv         - Dump VFP\n");
  printf("\te         - Dump Emulator\n");
  printf("\tsa<n>     - Dump last n words on the stack (asc)\n");
  printf("\tsd<n>     - Dump last n words on the stack (dsc)\n");
  printf("\tma<n>r<r> - Dump n words at memory address in register r (asc)\n");
  printf("\tmd<n>r<r> - Dump n words at memory address in register r (dsc)\n");
  printf("\tq         - Quit the emulator\n");

  while(1)
  {
    printf("\n");
    printf("Enter a command: ");
    c = getchar();

    /* Switch on command */
    switch (c)
    {
      case 'c':
      {
        cpu_dump(cpu);
        break;
      }
      case 'e':
      {
        emulator_dump(cpu->emu);
        break;
      }
      case 'v':
      {
        vfp_dump(&cpu->emu->vfp);
        break;
      }
      case 's':
      {
        c = getchar();
        if (c == 'a')
        {
          /* Number of words on the stack to print */
          uint32_t n;
          int UNUSED(ret) = scanf("%i", &n); /* Hack to shut the compiler up */
          n <<= 2;

          /* Print n words from the stack decreasing in address from SP */
          uint32_t addr = cpu_read_register(cpu, SP);
          for (uint32_t i = 0; i < n; i += 4)
          {
            uint32_t offset = -i;
            uint32_t data = memory_read_dword_le(cpu->memory, addr + offset);
            printf("SP-%-2d \t0x%08x : 0x%08x : '%c%c%c%c'\n", -offset,
              addr + offset, data, (int)((data >> 24) & 0xff),
              (int)((data >> 16) & 0xff), (int)((data >> 8) & 0xff),
              (int)(data & 0xff));
          }
        }
        else if (c == 'd')
        {
          /* Number of words on the stack to print */
          uint32_t n;
          int UNUSED(ret) = scanf("%i", &n); /* Hack to shut the compiler up */
          n <<= 2;

          /* Print n words from the stack increasing in address from SP */
          uint32_t addr = cpu_read_register(cpu, SP);
          for (uint32_t i = 0; i < n; i += 4)
          {
            /* Reverse offset so smaller addresses are printed lower */
            uint32_t offset = n - i - 4;
            uint32_t data = memory_read_dword_le(cpu->memory, addr + offset);
            printf("SP+%-2d \t0x%08x : 0x%08x : '%c%c%c%c'\n", offset,
              addr + offset, data, (int)((data >> 24) & 0xff),
              (int)((data >> 16) & 0xff), (int)((data >> 8) & 0xff),
              (int)(data & 0xff));
          }
        }
        break;
      }
      case 'm':
      {
        c = getchar();
        if (c == 'a')
        {
          /* Number of words to print */
          uint32_t n, r;
          int UNUSED(ret) = scanf("%ir%i", &n, &r); /* Hack to shut the compiler up */
          n <<= 2;

          /* Print n words from the address pointed by r */
          uint32_t addr = cpu_read_register(cpu, r);
          for (uint32_t i = 0; i < n; i += 4)
          {
            uint32_t offset = -i;
            uint32_t data = memory_read_dword_le(cpu->memory, addr + offset);
            printf("r%i-%-2i \t0x%08x : 0x%08x : '%c%c%c%c'\n", r, -offset,
              addr + offset, data, (int)((data >> 24) & 0xff),
              (int)((data >> 16) & 0xff), (int)((data >> 8) & 0xff),
              (int)(data & 0xff));
          }
        }
        else if (c == 'd')
        {
          /* Number of words to print */
          uint32_t n, r;
          int UNUSED(ret) = scanf("%ir%i", &n, &r); /* Hack to shut the compiler up */
          n <<= 2;

          /* Print n words from the address pointed by r */
          uint32_t addr = cpu_read_register(cpu, r);
          for (uint32_t i = 0; i < n; i += 4)
          {
            /* Reverse offset so smaller addresses are printed lower */
            uint32_t offset = n - i - 4;
            uint32_t data = memory_read_dword_le(cpu->memory, addr + offset);
            printf("r%i+%-2i \t0x%08x : 0x%08x : '%c%c%c%c'\n", r, offset,
              addr + offset, data, (int)((data >> 24) & 0xff),
              (int)((data >> 16) & 0xff), (int)((data >> 8) & 0xff),
              (int)(data & 0xff));
          }
        }
        break;
      }
      case 'q':
      {
        cpu->emu->terminated = 1;
        return;
      }
      default:
      {
        return;
      }
    }

    /* Flush \n */
    getchar();
  }
}

/**
 * Initialises the CPU
 * @param cpu
 * @param emu
 * @param start_addr
 */
void
cpu_init(cpu_t* cpu, emulator_t* emu)
{
  cpu->emu = emu;
  cpu->memory = &emu->memory;

  /* Initialise registers to zero */
  memset(&cpu->r_usr, 0, sizeof(cpu->r_usr));
  memset(&cpu->r_fiq, 0, sizeof(cpu->r_fiq));
  memset(&cpu->r_irq, 0, sizeof(cpu->r_irq));
  memset(&cpu->r_abt, 0, sizeof(cpu->r_abt));
  memset(&cpu->r_und, 0, sizeof(cpu->r_und));
  memset(&cpu->r_svc, 0, sizeof(cpu->r_svc));

  /* Start in supervisor mode */
  cpu->cpsr.b.m = MODE_SVC;

  /* Initialise spsr to zero */
  memset(&cpu->spsr, 0, sizeof(cpu->spsr));

  /* Load start address */
  cpu_write_register(cpu, PC, emu->start_addr);
}

/**
 * Fecthes, decodes and executed a single instruction
 * @param cpu   Reference to the CPU structure
 */
void
cpu_tick(cpu_t* cpu)
{
  uint32_t instr = 0;
  uint32_t pc;

  /* Fetch a single instruction */
  pc = cpu->r_usr.reg.pc;
  instr = memory_read_dword_le(cpu->memory, pc);
  cpu->r_usr.reg.pc = pc + 4;

  /* Terminate on NOP */
  if (instr == 0x0)
  {
    cpu->emu->terminated = 1;
    return;
  }

  /* Ignore PLD */
  if (instr == 0xf5d1f100)
  {
    return;
  }

  /* Check condition */
  if (!check_cond(cpu, instr >> 28))
  {
    return;
  }

  /* For debug purposes, let WFI be a "break here" instruction, causing the
   * emulator to wait for input before continuing */
  if ((instr & 0x0fff00ff) == 0x03200003)
  {
    debug_break(cpu);
  }

  switch ((instr >> 24) & 0xF)
  {
    case 0x0: case 0x1: case 0x2: case 0x3:
    {
      if ((instr & 0x0FFFFFF0) == 0x012FFF10)
      {
        // Branch and exchange
        instr_branch_exchange(cpu, (op_branch_exchange_t*)&instr);
      }
      else if ((instr & 0x0FC000F0) == 0x00000090)
      {
        /* Multiply */
        instr_multiply(cpu, (op_multiply_t*)&instr);
      }
      else if ((instr & 0x0FC000F0) == 0x00800090)
      {
        /* Multiply Long */
        instr_multiply_long(cpu, (op_multiply_long_t*)&instr);
      }
      else if ((instr & 0x0F400FF0) == 0x01000090)
      {
        // Single data swap
        instr_single_data_swap(cpu, (op_single_data_swap_t*)&instr);
      }
      else if ((instr & 0x003F0FFF) == 0x000F0000)
      {
        // MRS
        instr_mrs(cpu, (op_mrs_t*)&instr);
      }
      else if ((instr & 0x0FBFFFF0) == 0x0129F000)
      {
        // MSR
        instr_msr_psr(cpu, (op_msr_psr_t*)&instr);
      }
      else if ((instr & 0x0DBFF000) == 0x0128F000)
      {
        // MSR flags only
        instr_msr_psrf(cpu, (op_msr_psrf_t*)&instr);
      }
      else if ((instr & 0x0E400F90) == 0x00000090)
      {
        // Halfword and signed data transfer register offset
        instr_hw_sd_transfer(cpu, (op_hw_sd_trans_t*)&instr);
      }
      else if ((instr & 0x0E400090) == 0x00400090)
      {
        // Halfword and singed data transfer immediate offset
        instr_hw_sd_transfer(cpu, (op_hw_sd_trans_t*)&instr);
      }
      else
      {
        // Data processing instruction
        instr_single_data_processing(cpu, (op_data_proc_t*) &instr);
      }
      break;
    }
    case 0x4: case 0x5: case 0x6: case 0x7:
    {
      if ((instr & 0x0E000010) == 0x06000010)
      {
        // Undefined
        instr_undefined(cpu);
      }
      else
      {
        // Single data transfer
        instr_single_data_trans(cpu, (op_single_data_trans_t*) &instr);
      }
      break;
    }
    case 0x8: case 0x9:
    {
      instr_block_data_transfer(cpu, (op_block_data_trans_t*)&instr);
      break;
    }
    case 0xA: case 0xB:
    {
      instr_branch(cpu, (op_branch_t*)&instr);
      break;
    }
    case 0xC: case 0xD:
    {
      instr_coproc_data_transfer(cpu, (op_coproc_data_transfer_t*)&instr);
      break;
    }
    case 0xE:
    {
      if ((instr & 0x10) == 0)
      {
        instr_coproc_data_proc(cpu, (op_coproc_data_proc_t*)&instr);
      }
      else
      {
        instr_coproc_reg_transfer(cpu, (op_coproc_reg_transfer_t*)&instr);
      }
      break;
    }
    case 0xF:
    {
      // Software Interrupt (SWI)
      instr_swi(cpu, (op_swi_t*)&instr);
      break;
    }
  }
}

/**
 * Destroys the CPU
 * @param cpu Reference to the CPU structure
 */
void
cpu_destroy(cpu_t *UNUSED(cpu))
{
}

/**
 * Prints the state of the registers to stdout
 * @param cpu Reference to the CPU structure
 */
void
cpu_dump(cpu_t* cpu)
{
  uint32_t reg;
  int i;

  printf("Registers:\n");
  for (i = 0; i <= 12; ++i)
  {
    reg = cpu_read_register(cpu, i);
    printf("$%-3d: %10d (0x%08x)\n", i, reg, reg);
  }

  /* Increase the displayed PC by 4 */
  reg = cpu_read_register(cpu, PC);
  printf("PC  : %10d (0x%08x)\n", reg, reg);
  reg = cpu->cpsr.r & (~0x1F);
  printf("CPSR: %10d (0x%08x)\n", reg, reg);
}
