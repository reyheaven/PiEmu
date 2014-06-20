/* This file is part of the Team 28 Project
 * Licensing information can be found in the LICENSE file
 * (C) 2014 The Team 28 Authors. All rights reserved.
 */
#ifndef __FRAMEBUFFER_H__
#define __FRAMEBUFFER_H__

/**
 * Framebuffer request structure
 */
typedef union
{
  uint32_t data[10];
  struct
  {
    uint32_t phys_width;
    uint32_t phys_height;
    uint32_t virt_width;
    uint32_t virt_height;
    uint32_t pitch;
    uint32_t depth;
    uint32_t off_x;
    uint32_t off_y;
    uint32_t addr;
    uint32_t size;
  } fb;
} framebuffer_req_t;

/**
 * Framebuffer data
 */
typedef struct
{
  /* Emulator reference */
  emulator_t*   emu;

  /* Framebuffer */
  uint8_t*      framebuffer;
  size_t        fb_bpp;
  size_t        fb_pitch;
  size_t        fb_size;
  uint32_t      fb_address;
  uint16_t      fb_palette[256];

  /* Flag if set if query is malformed */
  int           error;

  /* Window */
  SDL_Surface*  surface;
  uint32_t      width;
  uint32_t      height;
  uint32_t      depth;
} framebuffer_t;

void fb_init(framebuffer_t*, emulator_t*);
void fb_create_window(framebuffer_t*, uint32_t width, uint32_t height);
void fb_destroy(framebuffer_t*);
void fb_tick(framebuffer_t*);
void fb_dump(framebuffer_t*);
void fb_request(framebuffer_t*, uint32_t address);
void fb_write_word(framebuffer_t*, uint32_t address, uint16_t data);
void fb_write_dword(framebuffer_t*, uint32_t address, uint32_t data);
int  fb_is_buffer(framebuffer_t*, uint32_t address);

#endif /* __FRAMEBUFFER_H__ */
