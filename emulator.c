/* This file is part of the Team 28 Project
 * Licensing information can be found in the LICENSE file
 * (C) 2014 The Team 28 Authors. All rights reserved.
 */
#include "common.h"
#include <sys/time.h>

/**
 * Initialises the emulator
 *
 * @param emu Reference to the emulator structure
 */
void
emulator_init(emulator_t* emu)
{
  cpu_init(&emu->cpu, emu);
  vfp_init(&emu->vfp, emu);
  memory_init(&emu->memory, emu);
  gpio_init(&emu->gpio, emu);
  mbox_init(&emu->mbox, emu);
  fb_init(&emu->fb, emu);
  pr_init(&emu->pr, emu);
  nes_init(&emu->nes, emu);
  emu->terminated = 0;
  emu->system_timer_base = emulator_get_time() * 1000;
  emu->last_refresh = 0;
}

/**
 * Loads a binary image into memory
 *
 * @param emu Reference to the emulator structure
 */
void
emulator_load(emulator_t* emu, const char *fname)
{
  FILE *finput;
  size_t file_size;
  void* memory_start = emu->memory.data + emu->start_addr;

  /* Throw error if file unopenable. */
  if (!(finput = fopen(fname, "rb")))
  {
    emulator_error(emu, "Cannot open file '%s'", fname);
  }


  fseek(finput, 0L, SEEK_END);
  file_size = ftell(finput);
  fseek(finput, 0L, SEEK_SET);

  /* Check for buffer overflow */
  if(emu->start_addr + file_size > emu->mem_size)
  {
    emulator_fatal(emu, "Not enough memory for kernel");
  }

  /* Copy instructions into memory and error if incomplete */
  if (fread(memory_start, 1, file_size, finput) != file_size)
  {
    emulator_error(emu, "Could not read entire file '%s'", fname);
  }

  fclose(finput);
}

/**
 * Checks whether the emulator is still active
 *
 * @param emu Reference to the emulator structure
 */
int
emulator_is_running(emulator_t* emu)
{
  return !emu->terminated;
}

/**
 * Get the current time in milliseconds
 */
uint64_t
emulator_get_time()
{
  struct timeval tv;
  gettimeofday(&tv, NULL);
  return (tv.tv_sec) * 1000 + (tv.tv_usec) / 1000;
}

/**
 * Get the value of the system timer
 */
uint64_t
emulator_get_system_timer(emulator_t* emu)
{
  struct timeval tv;
  gettimeofday(&tv, NULL);
  uint64_t us = (tv.tv_sec) * 1000000 + tv.tv_usec;
  return us - emu->system_timer_base;
}

/**
 * Executes a single step
 *
 * @param emu Reference to the emulator structure
 */
void
emulator_tick(emulator_t* emu)
{
  cpu_tick(&emu->cpu);

  /* When graphics are emulated, we execute a screen refresh after 34ms has
   * passed (30 frames per second) */
  uint32_t frame_time = 20;
  if (emu->graphics)
  {
    uint64_t now = emulator_get_time();
    if ((now - emu->last_refresh) > frame_time)
    {
      fb_tick(&emu->fb);
      emu->last_refresh = now;
    }
  }
}

void
emulator_destroy(emulator_t* emu)
{
  fb_destroy(&emu->fb);
  pr_destroy(&emu->pr);
  mbox_destroy(&emu->mbox);
  gpio_destroy(&emu->gpio);
  cpu_destroy(&emu->cpu);
  vfp_destroy(&emu->vfp);
  memory_destroy(&emu->memory);
}

/**
 * Prints out the state of the emulator
 *
 * @param emu Reference to the emulator structure
 */
void
emulator_dump(emulator_t* emu)
{
  cpu_dump(&emu->cpu);
  memory_dump(&emu->memory);
}

/**
 * Prints a useful info message.
 *
 * @param emu Reference to the emulator structure
 * @param fmt Printf-like format string
 */
void
emulator_info(emulator_t* emu, const char * fmt, ...)
{
  int size = 100, n;
  char* str = NULL;
  va_list ap;

  if (emu->quiet)
  {
    return;
  }

  str = (char*)malloc(size);
  assert(str);

  while (1)
  {
    va_start(ap, fmt);
    n = vsnprintf(str, size, fmt, ap);
    va_end(ap);

    if (-1 < n && n < size)
    {
      break;
    }

    size = n > -1 ? (n + 1) : (size << 1);
    str = (char*)realloc(emu->err_msg, size);
    assert(str);
  }

  printf("Info: %s\n", str);
}

/**
 * Prints an error message and exits if the strict flag is set
 *
 * @param emu Reference to the emulator structure
 * @param fmt Printf-like format string
 */
void
emulator_error(emulator_t* emu, const char * fmt, ...)
{
  int size = 100, n;
  char* tmp = NULL;
  va_list ap;

  if (emu->quiet)
  {
    return;
  }

  emu->err_msg = (char*)malloc(size);
  assert(emu->err_msg);

  while (1)
  {
    va_start(ap, fmt);
    n = vsnprintf(emu->err_msg, size, fmt, ap);
    va_end(ap);

    if (-1 < n && n < size)
    {
      break;
    }

    size = n > -1 ? (n + 1) : (size << 1);
    tmp = (char*)realloc(emu->err_msg, size);
    assert(tmp);

    emu->err_msg = tmp;
  }

  printf("Error: %s\n", emu->err_msg);
}

/**
 * Kills the emulator, printing a message to stdout
 *
 * @param emu Reference to the emulator structure
 * @param fmt Printf-like format string
 */
void
emulator_fatal(emulator_t* emu, const char * fmt, ...)
{
  int size = 100, n;
  char * tmp;
  va_list ap;

  emu->err_msg = (char*)malloc(size);
  assert(emu->err_msg);

  while (1)
  {
    va_start(ap, fmt);
    n = vsnprintf(emu->err_msg, size, fmt, ap);
    va_end(ap);

    if (-1 < n && n < size)
    {
      break;
    }

    size = n > -1 ? (n + 1) : (size << 1);
    if (!(tmp = (char*)realloc(emu->err_msg, size)))
    {
      free(emu->err_msg);
      break;
    }

    emu->err_msg = tmp;
  }

  longjmp(emu->err_jmp, 1);
}
