/* This file is part of the Team 28 Project
 * Licensing information can be found in the LICENSE file
 * (C) 2014 The Team 28 Authors. All rights reserved.
 */
#include "common.h"

void
nes_init(nes_t* nes, emulator_t* emu)
{
  nes->emu = emu;
  nes->last_latch = 0;
  nes->last_clock = 0;
  nes->counter = 0;
  memset(&nes->state, 0, sizeof(nes->state));
  memset(&nes->binding, 0, sizeof(nes->binding));

  /* Set up key bindings */
  nes->binding[NES_A]      = SDLK_SPACE;
  nes->binding[NES_B]      = SDLK_TAB;
  nes->binding[NES_START]  = SDLK_RETURN;
  nes->binding[NES_SELECT] = SDLK_p;
  nes->binding[NES_LEFT]   = SDLK_a;
  nes->binding[NES_RIGHT]  = SDLK_d;
  nes->binding[NES_UP]     = SDLK_w;
  nes->binding[NES_DOWN]   = SDLK_s;
}

static inline void
nes_write_button(nes_t* nes, uint32_t button)
{
  nes->emu->gpio.ports[NES_GPIO_PORT_DATA].state = nes->state[button] ? 0 : 1;
}

void
nes_gpio_write(nes_t* nes, uint32_t port, uint32_t value)
{
  if (port == NES_GPIO_PORT_LATCH)
  {
    /* On the rising edge of the latch, reset counter */
    if (nes->last_latch == 0 && value == 1)
    {
      nes->counter = 0;
      nes_write_button(nes, nes->counter++);
    }

    nes->last_latch = value;
  }
  else if (port == NES_GPIO_PORT_CLOCK)
  {
    /* On the rising edge of the clock, write the next button */
    if (value == 1 && nes->last_clock == 0)
    {
      if (nes->counter < NES_BUTTON_COUNT)
      {
        nes_write_button(nes, nes->counter);
      }
      else
      {
        nes->emu->gpio.ports[NES_GPIO_PORT_DATA].state = 1;
      }
      nes->counter++;
    }

    nes->last_clock = value;
  }
}

void
nes_on_key_down(nes_t* nes, SDLKey key)
{
  /* Search for the button bound to this SDLKey */
  for (int i = 0; i < NES_BUTTON_COUNT; ++i)
  {
    if (nes->binding[i] == key)
    {
      nes->state[i] = 1;
    }
  }
}

void
nes_on_key_up(nes_t* nes, SDLKey key)
{
  /* Search for the button bound to this SDLKey */
  for (int i = 0; i < NES_BUTTON_COUNT; ++i)
  {
    if (nes->binding[i] == key)
    {
      nes->state[i] = 0;
    }
  }
}
