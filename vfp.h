/* This file is part of the Team 28 Project
 * Licensing information can be found in the LICENSE file
 * (C) 2014 The Team 28 Authors. All rights reserved.
 */
#ifndef __VFP_H__
#define __VFP_H__

typedef struct
{
  emulator_t* emu;

  /* Registers */
  struct
  {
    /* General Purpose */
    uint32_t s[32];

    /* FPSID */
    uint32_t fpsid;

    /* FPSCR */
    union
    {
      uint32_t r;
      struct
      {
        uint32_t ioc    : 1;
        uint32_t dzc    : 1;
        uint32_t ofc    : 1;
        uint32_t ufc    : 1;
        uint32_t ixc    : 1;
        uint32_t        : 3;
        uint32_t ioe    : 1;
        uint32_t dze    : 1;
        uint32_t ofe    : 1;
        uint32_t ufe    : 1;
        uint32_t ixe    : 1;
        uint32_t        : 3;
        uint32_t len    : 3;
        uint32_t        : 1;
        uint32_t stride : 2;
        uint32_t rmode  : 2;
        uint32_t fz     : 1;
        uint32_t        : 3;
        uint32_t flags  : 4;
      } b;
    } fpscr;

    /* FPEXC */
    union
    {
      uint32_t r;
      struct
      {
        uint32_t        : 30;
        uint32_t en     : 1;
        uint32_t ex     : 1;
      } b;
    } fpexc;
  } reg;
} vfp_t;

void vfp_init(vfp_t*, emulator_t*);
void vfp_destroy(vfp_t *);
void vfp_dump(vfp_t *);

void vfp_data_proc(vfp_t*, op_coproc_data_proc_t* instr);
void vfp_data_transfer(vfp_t*, op_coproc_data_transfer_t* instr);
void vfp_reg_transfer(vfp_t*, op_coproc_reg_transfer_t* instr);

#endif /* __VFP_H__ */
