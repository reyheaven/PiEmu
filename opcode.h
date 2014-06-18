/* This file is part of the Team 28 Project
 * Licensing information can be found in the LICENSE file
 * (C) 2014 The Team 28 Authors. All rights reserved.
 */
#ifndef __CPU_INSTR_H__
#define __CPU_INSTR_H__

/**
 * Multiply long encoding
 */
typedef struct
{
  uint32_t Rm:4;
  uint32_t   :4;
  uint32_t Rs:4;
  uint32_t RdLo:4;
  uint32_t RdHi:4;
  uint32_t  s:1;
  uint32_t  a:1;
  uint32_t  u:1;
  uint32_t   :9;
} op_multiply_long_t;

/**
 * Multiply encoding
 */
typedef struct
{
  uint32_t Rm:4;
  uint32_t   :4;
  uint32_t Rs:4;
  uint32_t Rn:4;
  uint32_t Rd:4;
  uint32_t  s:1;
  uint32_t  a:1;
  uint32_t   :10;
} op_multiply_t;

/**
 * MRS encoding
 */
typedef struct
{
  uint32_t     : 12;
  uint32_t Rd  : 4;
  uint32_t     : 6;
  uint32_t Ps  : 1;
  uint32_t     : 9;
} op_mrs_t;

/**
 * MSR encoding
 */
typedef struct
{
  uint32_t Rm  : 4;
  uint32_t     : 18;
  uint32_t Pd  : 1;
  uint32_t     : 9;
} op_msr_psr_t;

/**
 * Data Processing encoding
 */
typedef struct
{
  uint32_t imm : 12;
  uint32_t Rd  : 4;
  uint32_t Rn  : 4;
  uint32_t s   : 1;
  uint32_t op  : 4;
  uint32_t i   : 1;
  uint32_t     : 6;
} op_data_proc_t;

/**
 * MSR flag bits only encoding
 */
typedef struct
{
  uint32_t src : 12;
  uint32_t     : 10;
  uint32_t Pd  : 1;
  uint32_t     : 2;
  uint32_t i   : 1;
  uint32_t     : 6;
} op_msr_psrf_t;

/**
 * Block data transfer encoding
 */
typedef struct
{
  uint32_t rl : 16;
  uint32_t rn : 4;
  uint32_t l  : 1;
  uint32_t w  : 1;
  uint32_t s  : 1;
  uint32_t u  : 1;
  uint32_t p  : 1;
  uint32_t    : 7;
} op_block_data_trans_t;

/**
 * Branch encoding
 */
typedef struct
{
  uint32_t offset : 24;
  uint32_t l      : 1;
  uint32_t        : 7;
} op_branch_t;

/**
 * Branch with exchange encoding
 */
typedef struct
{
  uint32_t Rn : 4;
  uint32_t : 28;
} op_branch_exchange_t;

/**
 * Single data transfer instruction encoding
 */
typedef struct
{
  uint32_t offset: 12;
  uint32_t rd: 4;
  uint32_t rn: 4;
  uint32_t l: 1;
  uint32_t w: 1;
  uint32_t b: 1;
  uint32_t u: 1;
  uint32_t p: 1;
  uint32_t i: 1;
  uint32_t  : 6;
} op_single_data_trans_t;

/**
 * Single data swap instruction encoding
 */
typedef struct
{
  uint32_t rm: 4;
  uint32_t   : 8;
  uint32_t rd: 4;
  uint32_t rn: 4;
  uint32_t   : 2;
  uint32_t b : 1;
  uint32_t   : 5;
  uint32_t cond: 4;
} op_single_data_swap_t;

/**
 * Halford and signed data transfer instruction encoding
 */
typedef struct
{
  uint32_t rm_ln: 4;
  uint32_t   : 1;
  uint32_t sh: 2;
  uint32_t   : 1;
  uint32_t hn: 4;
  uint32_t rd: 4;
  uint32_t rn: 4;
  uint32_t  l: 1;
  uint32_t  w: 1;
  uint32_t  o: 1;
  uint32_t  u: 1;
  uint32_t  p: 1;
  uint32_t   : 3;
  uint32_t cond: 4;
} op_hw_sd_trans_t;

/**
 * Coprocessor data procedure encoding
 */
typedef struct
{
  uint32_t CRm : 4;
  uint32_t : 1;
  uint32_t CP : 3;
  uint32_t CP_number : 4;
  uint32_t CRd : 4;
  uint32_t CRn : 4;
  uint32_t CP_opcode : 4;
  uint32_t : 8;
} op_coproc_data_proc_t;

/**
 * Coprocessor data transfer encoding
 */
typedef struct
{
  uint32_t offset : 8;
  uint32_t CP_number : 4;
  uint32_t CRd : 4;
  uint32_t CRn : 4;
  uint32_t l : 1;
  uint32_t w : 1;
  uint32_t n : 1;
  uint32_t u : 1;
  uint32_t p : 1;
  uint32_t : 7;
} op_coproc_data_transfer_t;

/**
 * Coprocessor register transfer encoding
 */
typedef struct
{
  uint32_t CRm : 4;
  uint32_t : 1;
  uint32_t CP : 3;
  uint32_t CP_number : 4;
  uint32_t Rd : 4;
  uint32_t CRn : 4;
  uint32_t l : 1;
  uint32_t CP_opcode : 3;
  uint32_t : 8;
} op_coproc_reg_transfer_t;

/**
 * Software interrupt encoding
 */
typedef struct
{
  uint32_t comment: 24;
  uint32_t        : 8;
} op_swi_t;

#endif /* __CPU_INSTR_H__ */
