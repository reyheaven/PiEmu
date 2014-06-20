/* This file is part of the Team 28 Project
 * Licensing information can be found in the LICENSE file
 * (C) 2014 The Team 28 Authors. All rights reserved.
 */
#include "common.h"

/**
 * Initialises the framebuffer interface
 * @param fb  Reference to the framebuffer structure
 * @param emu Reference to the emulator structure
 */
void
fb_init(framebuffer_t* fb, emulator_t* emu)
{
  assert(fb);
  assert(emu);

  fb->emu = emu;

  /* If not in graphic mode, do not create a window */
  if (!fb->emu->graphics)
  {
    return;
  }

  /* Create the window */
  SDL_Init(SDL_INIT_EVERYTHING);
  fb->width = 640;
  fb->height = 480;
  fb->depth = 32;
  fb->surface =
    SDL_SetVideoMode(fb->width, fb->height, fb->depth, SDL_SWSURFACE);

  /* Set the window caption */
  SDL_WM_SetCaption("Raspberry Pi Emulator", NULL);
}

/**
 * Cleans up memory used by the framebuffer
 * @param fb  Reference to the framebuffer structure
 */
void
fb_destroy(framebuffer_t* fb)
{
  if (!fb || !fb->emu->graphics)
  {
    return;
  }

  /* Destroy SDL */
  SDL_FreeSurface(fb->surface);
  SDL_Quit();
  fb->surface = 0;

  /* Free framebuffer */
  if (fb->framebuffer)
  {
    free(fb->framebuffer);
    fb->framebuffer = NULL;
  }
}

/*
 * Set the pixel at (x, y) to the given value
 * NOTE: The surface must be locked before calling this!
 */
void
put_pixel(SDL_Surface *surface, int x, int y, uint32_t pixel)
{
  int bpp = surface->format->BytesPerPixel;

  /* Here p is the address to the pixel we want to set */
  uint8_t* p = (uint8_t*)surface->pixels + y * surface->pitch + x * bpp;

  switch(bpp)
  {
    case 1:
    {
      *p = pixel;
      break;
    }
    case 2:
    {
      *(uint16_t*)p = pixel;
      break;
    }
    case 3:
    {
      if(SDL_BYTEORDER == SDL_BIG_ENDIAN)
      {
        p[0] = (pixel >> 16) & 0xff;
        p[1] = (pixel >> 8) & 0xff;
        p[2] = pixel & 0xff;
      }
      else
      {
        p[0] = pixel & 0xff;
        p[1] = (pixel >> 8) & 0xff;
        p[2] = (pixel >> 16) & 0xff;
      }
      break;
    }
    case 4:
    {
      *(uint32_t*)p = pixel;
      break;
    }
  }
}

/**
 * Gets the specified pixel colour at a particular location and converts
 * it to the output format.
 *
 * @param fb Reference to the framebuffer structure
 * @param x X position
 * @param y Y position
 */
uint32_t fb_get_pixel(framebuffer_t* fb, uint32_t x, uint32_t y)
{
  assert(fb);

  if (!fb->framebuffer)
  {
    return SDL_MapRGB(fb->surface->format, 0xff, 0x00, 0xff);
  }

  switch (fb->fb_bpp)
  {
    case 1: // 1 byte per pixel - 8 bit colour
    {
      uint8_t key = fb->framebuffer[y * fb->fb_pitch + x * fb->fb_bpp];
      uint16_t value = fb->fb_palette[key];

      /* Format: RRRRRGGGGGGBBBBB */
      uint32_t r = (value >> 11) & 0x1F;
      uint32_t g = (value >> 5) & 0x3F;
      uint32_t b = value & 0x1F;

      /* Map to [0-255] */
      r = (r * 255) / 31;
      g = (g * 255) / 63;
      b = (b * 255) / 31;

      /* Return colour */
      return SDL_MapRGB(fb->surface->format, r, g, b);
    }
    case 2: // 2 bytes per pixel - R5G6B5
    {
      uint16_t value =
        fb->framebuffer[y * fb->fb_pitch + x * fb->fb_bpp] +
        (fb->framebuffer[y * fb->fb_pitch + x * fb->fb_bpp + 1] << 8);

      /* Format: BBBBBGGGGGGRRRRR */
      uint32_t r = value & 0x1F;
      uint32_t g = (value >> 5) & 0x3F;
      uint32_t b = (value >> 11) & 0x1F;

      /* Map to [0-255] */
      r = (r * 255) / 31;
      g = (g * 255) / 63;
      b = (b * 255) / 31;

      /* Return colour */
      return SDL_MapRGB(fb->surface->format, r, g, b);
    }
    case 3: // 3 bytes per pixel - RGB8
    case 4: // 4 bytes per pixel - XRGB8
    {
      uint32_t value =
        fb->framebuffer[y * fb->fb_pitch + x * fb->fb_bpp] +
        (fb->framebuffer[y * fb->fb_pitch + x * fb->fb_bpp + 1] << 8) +
        (fb->framebuffer[y * fb->fb_pitch + x * fb->fb_bpp + 2] << 16) +
        (fb->framebuffer[y * fb->fb_pitch + x * fb->fb_bpp + 3] << 24);

      /* Format: BBBBBBBBGGGGGGGGRRRRRRRR */
      return SDL_MapRGB(fb->surface->format,
        value & 0xFF, (value >> 8) & 0xFF, (value >> 16) & 0xFF);
    }
    default:
    {
      emulator_error(fb->emu, "Unsupported pixel format");
      break;
    }
  }

  return 0;
}

/**
 * Updates the display, displaying the framebuffer and handling events
 *
 * @param fb Reference to the framebuffer structure
 */
void
fb_tick(framebuffer_t* fb)
{
  assert(fb);
  assert(fb->emu->graphics);

  /* Handle all SDL events */
  SDL_Event event;
  while (SDL_PollEvent(&event))
  {
    if (event.type == SDL_QUIT)
    {
      fb->emu->terminated = 1;
    }

    /* Route keyboard presses to the NES module if enabled */
    if (event.type == SDL_KEYDOWN)
    {
      switch (event.key.keysym.sym)
      {
        case SDLK_1 ... SDLK_9:
        {
          int port = (int)event.key.keysym.sym - SDLK_1;
          fb->emu->gpio.ports[fb->emu->gpio_test_offset + port].state = 1;
          break;
        }
        default:
        {
          if (fb->emu->nes_enabled)
          {
            nes_on_key_down(&fb->emu->nes, event.key.keysym.sym);
          }
          break;
        }
      }
    }
    else if (event.type == SDL_KEYUP)
    {
      switch (event.key.keysym.sym)
      {
        case SDLK_1 ... SDLK_9:
        {
          int port = (int)event.key.keysym.sym - SDLK_1;
          fb->emu->gpio.ports[fb->emu->gpio_test_offset + port].state = 0;
          break;
        }
        default:
        {
          if (fb->emu->nes_enabled)
          {
            nes_on_key_up(&fb->emu->nes, event.key.keysym.sym);
          }
          break;
        }
      }
    }
  }

  /* Lock the surface */
  if (SDL_MUSTLOCK(fb->surface))
  {
    SDL_LockSurface(fb->surface);
  }

  /* Copy the pixels to SDL */
  for (uint32_t y = 0; y < fb->height; ++y)
  {
    for (uint32_t x = 0; x < fb->width; ++x)
    {
      put_pixel(fb->surface, x, y, fb_get_pixel(fb, x, y));
    }
  }

  /* Unlock the surface */
  if (SDL_MUSTLOCK(fb->surface))
  {
    SDL_UnlockSurface(fb->surface);
  }

  /* Display to screen */
  SDL_Flip(fb->surface);
}

/**
 * Handles a framebuffer request received through the mailbox interface
 *
 * @param fb   Framebuffer structure
 * @param addr Address received from the mailbox
 */
void
fb_request(framebuffer_t *fb, uint32_t addr)
{
  size_t i;
  framebuffer_req_t req;

  assert(fb);

  /* Clear error flag */
  fb->error = 0;

  /* Graphic flag must be set*/
  if (!fb->emu->graphics)
  {
    emulator_error(fb->emu, "Graphic mode must be enabled for framebuffer");
    fb->error = 1;
    return;
  }

  /* Check whether address is valid */
  if (addr < 0x40000000)
  {
    emulator_error(fb->emu, "Invalid framebuffer address");
    fb->error = 1;
    return;
  }

  /* Respond to the framebuffer request */
  addr -= 0x40000000;
  for (i = 0; i < sizeof(req.data) / sizeof(req.data[0]); ++i)
  {
    req.data[i] = memory_read_dword_le(&fb->emu->memory, addr + (i << 2));
  }

  /* Free old framebuffer */
  if (fb->framebuffer)
  {
    free(fb->framebuffer);
    fb->framebuffer = NULL;
  }

  /* Read palette if 8 bit colour
   * We're assuming that the palette comes immediately after the request */
  if (req.fb.depth == 8)
  {
    for (i = 0; i < 256; ++i)
    {
      fb->fb_palette[i] = memory_read_word_le(&fb->emu->memory, addr + sizeof(req) + i * 2);
    }
  }

  /* Allocate a nice frame buffer, placed after the main memory */
  fb->fb_bpp = req.fb.depth >> 3;
  fb->fb_pitch = req.fb.virt_width * fb->fb_bpp;
  req.fb.pitch = fb->fb_pitch + (4 - (fb->fb_pitch % 4)) % 4;
  req.fb.size = fb->fb_size = fb->fb_pitch * req.fb.virt_height;
  fb->framebuffer = malloc(fb->fb_size);
  req.fb.addr = fb->fb_address = fb->emu->mem_size;
  fb->width = req.fb.virt_width;
  fb->height = req.fb.virt_height;

  assert(fb->framebuffer);
  memset(fb->framebuffer, 0, fb->fb_size);

  /* Write back structure into memory */
  for (i = 0; i < sizeof(req.data) / sizeof(req.data[0]); ++i)
  {
    memory_write_dword_le(&fb->emu->memory, addr + (i << 2), req.data[i]);
  }

  /* Change the window caption */
  SDL_WM_SetCaption("Raspberry Pi Emulator", NULL);

  /* Change the window size */
  fb->surface = SDL_SetVideoMode(fb->width, fb->height, fb->depth, SDL_SWSURFACE);
}

void
fb_write_word(framebuffer_t* fb, uint32_t address, uint16_t data)
{
  uint32_t addr;

  assert(fb);
  assert(fb->emu->graphics);
  assert(fb->framebuffer);

  addr = address - fb->fb_address;
  fb->framebuffer[addr + 0] = (data >> 0) & 0xFF;
  fb->framebuffer[addr + 1] = (data >> 8) & 0xFF;
}

/**
 * Handles a write to framebuffer data
 *
 * @param fb      Framebuffer structure
 * @param address Address of the pixel
 * @param data    Data to be written to the buffer
 */
void
fb_write_dword(framebuffer_t* fb, uint32_t address, uint32_t data)
{
  uint32_t addr;

  assert(fb);
  assert(fb->emu->graphics);
  assert(fb->framebuffer);

  addr = address - fb->fb_address;
  fb->framebuffer[addr + 0] = (data >> 0) & 0xFF;
  fb->framebuffer[addr + 1] = (data >> 8) & 0xFF;
  fb->framebuffer[addr + 2] = (data >> 16) & 0xFF;
  fb->framebuffer[addr + 3] = (data >> 24) & 0xFF;
}

/**
 * Checks whether an address is in the memory range of the framebuffer
 *
 * @param fb      Framebuffer structure
 * @param address Query address
 * @return        True if address is in range
 */
int
fb_is_buffer(framebuffer_t *fb, uint32_t address)
{
  assert(fb);

  /* Graphic mode must be enabled */
  if (!fb->emu->graphics)
  {
    return 0;
  }

  return fb->fb_address <= address && address < fb->fb_address + fb->fb_size;
}
