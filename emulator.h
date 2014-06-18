/* This file is part of the Team 28 Project
 * Licensing information can be found in the LICENSE file
 * (C) 2014 The Team 28 Authors. All rights reserved.
 */

#ifndef __EMULATOR_H__
#define __EMULATOR_H__

/**
 * Emulator state
 */
struct _emulator_t
{
  int           terminated;

  /* Kernel image */
  const char   *image;

  /* Error handling */
  jmp_buf       err_jmp;
  char*         err_msg;

  /* Arguments */
  size_t        mem_size;
  uint32_t      start_addr;
  int           graphics;
  int           usage;
  int           quiet;
  int           nes_enabled;
  int           gpio_test_offset;

  /* Modules */
  framebuffer_t fb;
  memory_t      memory;
  cpu_t         cpu;
  gpio_t        gpio;
  mbox_t        mbox;
  peripheral_t  pr;
  vfp_t         vfp;
  nes_t         nes;

  /* System Timer */
  uint64_t      system_timer_base;

  /* Refresh */
  uint64_t      last_refresh;
};

void emulator_init(emulator_t* );
int emulator_is_running(emulator_t* );
uint64_t emulator_get_time();
uint64_t emulator_get_system_timer(emulator_t*);
void emulator_tick(emulator_t* );
void emulator_info(emulator_t*, const char *, ...);
void emulator_error(emulator_t*, const char *, ...);
void emulator_fatal(emulator_t*, const char *, ...) __attribute__((noreturn));
void emulator_destroy(emulator_t* );
void emulator_dump(emulator_t* );
void emulator_load(emulator_t* , const char*);

#endif /* __EMULATOR_H__ */

