/* Emacs style mode select   -*- C++ -*-
 *-----------------------------------------------------------------------------
 *
 *
 *  PrBoom: a Doom port merged with LxDoom and LSDLDoom
 *  based on BOOM, a modified and improved DOOM engine
 *  Copyright (C) 1999 by
 *  id Software, Chi Hoang, Lee Killough, Jim Flynn, Rand Phares, Ty Halderman
 *  Copyright (C) 1999-2000 by
 *  Jess Haas, Nicolas Kalkhof, Colin Phipps, Florian Schulze
 *  Copyright 2005, 2006 by
 *  Florian Schulze, Colin Phipps, Neil Stevens, Andrey Budko
 *
 *  This program is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU General Public License
 *  as published by the Free Software Foundation; either version 2
 *  of the License, or (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
 *  02111-1307, USA.
 *
 * DESCRIPTION:
 *   Joystick handling for Linux
 *
 *-----------------------------------------------------------------------------
 */

#ifndef lint
#endif /* lint */

#include <stdlib.h>

#include "SDL.h"
#include "doomdef.h"
#include "doomtype.h"
#include "m_argv.h"
#include "d_event.h"
#include "d_main.h"
#include "i_joy.h"
#include "lprintf.h"

// emscripten
#include <emscripten.h>
#include <emscripten/html5.h>

int joyleft;
int joyright;
int joyup;
int joydown;

int usejoystick;

#ifdef HAVE_SDL_JOYSTICKGETAXIS
static SDL_Joystick *joystick;
#endif

static void I_EndJoystick(void)
{
  lprintf(LO_DEBUG, "I_EndJoystick : closing joystick\n");
}

void I_PollJoystick(void)
{
#ifdef HAVE_SDL_JOYSTICKGETAXIS
  event_t ev;
  Sint16 axis_value;

  if (!usejoystick || (!joystick)) return;
  ev.type = ev_joystick;
  ev.data1 =
    (SDL_JoystickGetButton(joystick, 0)<<0) |
    (SDL_JoystickGetButton(joystick, 1)<<1) |
    (SDL_JoystickGetButton(joystick, 2)<<2) |
    (SDL_JoystickGetButton(joystick, 3)<<3) |
    (SDL_JoystickGetButton(joystick, 4)<<4) |
    (SDL_JoystickGetButton(joystick, 5)<<5) |
    (SDL_JoystickGetButton(joystick, 6)<<6) |
    (SDL_JoystickGetButton(joystick, 7)<<7) |
    (SDL_JoystickGetButton(joystick, 8)<<8) |
    (SDL_JoystickGetButton(joystick, 9)<<9) |
    (SDL_JoystickGetButton(joystick, 10)<<10) |
    (SDL_JoystickGetButton(joystick, 11)<<11);

  // for (int i = 0; i < 16; i++) {
  //   int val = SDL_JoystickGetButton(joystick, i);
  //   if (val) {
  //     printf("Button down: %d %d\n", i, sizeof(ev.data1));
  //   }
  // }

  //axis_value = SDL_JoystickGetAxis(joystick, 0) / 3000;
  axis_value = SDL_JoystickGetAxis(joystick, 0);
  if (abs(axis_value)<15000) {
    axis_value = 
      SDL_JoystickGetButton(joystick, 14) ? -1 : 
        SDL_JoystickGetButton(joystick, 15) ? 1 : 0;
  } else {
    axis_value = axis_value > 0 ? 1 : -1;
  } 
  //if (abs(axis_value)<10) axis_value=0;
  ev.data2 = axis_value;
  axis_value = SDL_JoystickGetAxis(joystick, 1);
  if (abs(axis_value)<15000) {
    axis_value = 
      SDL_JoystickGetButton(joystick, 12) ? -1 : 
        SDL_JoystickGetButton(joystick, 13) ? 1 : 0;
  } else {
     axis_value = axis_value > 0 ? 1 : -1;
  }
  //axis_value = SDL_JoystickGetAxis(joystick, 1) / 3000;
  //if (abs(axis_value)<10) axis_value=0;
  ev.data3 = axis_value;

  D_PostEvent(&ev);
#endif
}

// emscripten
EM_BOOL gamepad_callback(int eventType, const EmscriptenGamepadEvent *e, void *userData)
{
  printf("%s: timeStamp: %g, connected: %d, index: %ld, numAxes: %d, numButtons: %d, id: \"%d\", mapping: \"%s\"\n",
    eventType, "Gamepad state", e->timestamp, e->connected, e->index,
    e->numAxes, e->numButtons, e->id, e->mapping);

  if (e->connected)
  {
    for(int i = 0; i < e->numAxes; ++i)
      printf("Axis %d: %g\n", i, e->axis[i]);

    for(int i = 0; i < e->numButtons; ++i)
      printf("Button %d: Digital: %d, Analog: %g\n", i, e->digitalButton[i], e->analogButton[i]);

    if (!joystick) {
      joystick = SDL_JoystickOpen(usejoystick-1);
      if (!joystick)
        lprintf(LO_ERROR, "Error opening joystick\n");
      else {
        atexit(I_EndJoystick);
        lprintf(LO_INFO, "Joystick: Opened %s\n", SDL_JoystickName(usejoystick-1));
        joyup = 32767;
        joydown = -32768;
        joyright = 32767;
        joyleft = -32768;
      }
    }
  }

  return 0;
}

static int translateKey(const EmscriptenKeyboardEvent *keyEvent) {
    int rc = 0;

    if (strlen(keyEvent->code) > 0) {
      switch (keyEvent->keyCode) {
          case 37:
              rc = KEYD_LEFTARROW;
              break;
          case 39:
              rc = KEYD_RIGHTARROW;
              break;
          case 40:
              rc = KEYD_DOWNARROW;
              break;
          case 38:
              rc = KEYD_UPARROW;
              break;
          case 27:
              rc = KEYD_ESCAPE;
              break;
          case 13:
              rc = KEYD_ENTER;
              break;
          case 9:
              rc = KEYD_TAB;
              break;
          case 112:
              rc = KEYD_F1;
              break;
          case 113:
              rc = KEYD_F2;
              break;
          case 114:
              rc = KEYD_F3;
              break;
          case 115:
              rc = KEYD_F4;
              break;
          case 116:
              rc = KEYD_F5;
              break;
          case 117:
              rc = KEYD_F6;
              break;
          case 118:
              rc = KEYD_F7;
              break;
          case 119:
              rc = KEYD_F8;
              break;
          case 120:
              rc = KEYD_F9;
              break;
          case 121:
              rc = KEYD_F10;
              break;
          case 122:
              rc = KEYD_F11;
              break;
          case 123:
              rc = KEYD_F12;
              break;
          case 8:
              rc = KEYD_BACKSPACE;
              break;
          case 46:
              rc = KEYD_DEL;
              break;
          case 45:
              rc = KEYD_INSERT;
              break;
          case 33:
              rc = KEYD_PAGEUP;
              break;
          case 34:
              rc = KEYD_PAGEDOWN;
              break;
          case 36:
              rc = KEYD_HOME;
              break;
          case 35:
              rc = KEYD_END;
              break;
          case 19:
              rc = KEYD_PAUSE;
              break;
          case 187:
              rc = KEYD_EQUALS;
              break;
          case 189:
              rc = KEYD_MINUS;
              break;
          case 96:
              rc = KEYD_KEYPAD0;
              break;
          case 97:
              rc = KEYD_KEYPAD1;
              break;
          case 98:
              rc = KEYD_KEYPAD2;
              break;
          case 99:
              rc = KEYD_KEYPAD3;
              break;
          case 100:
              rc = KEYD_KEYPAD4;
              break;
          case 101:
              rc = KEYD_KEYPAD5;
              break;
          case 102:
              rc = KEYD_KEYPAD6;
              break;
          case 103:
              rc = KEYD_KEYPAD7;
              break;
          case 104:
              rc = KEYD_KEYPAD8;
              break;
          case 105:
              rc = KEYD_KEYPAD9;
              break;
          case 107:
              rc = KEYD_KEYPADPLUS;
              break;
          case 109:
              rc = KEYD_KEYPADMINUS;
              break;
          case 111:
              rc = KEYD_KEYPADDIVIDE;
              break;
          case 106:
              rc = KEYD_KEYPADMULTIPLY;
              break;
          case 110:
              rc = KEYD_KEYPADPERIOD;
              break;
          case 16:
              rc = KEYD_RSHIFT;
              break;
          case 17:
              rc = KEYD_RCTRL;
              break;
          case 18:
              rc = KEYD_RALT;
              break;
          case 20:
              rc = KEYD_CAPSLOCK;
              break;
              /*case SDLK_KP_ENTER: rc = KEYD_KEYPADENTER;  break;*/
          case 188:
              rc = 44;
              break;
          case 190:
              rc = 46;
              break;
          case 222:
              rc = 39;
              break;
          case 219:
              rc = 91;
              break;
          case 221:
              rc = 93;
              break;
          case 192:
              rc = 96;
              break;
          case 220:
              rc = 92;
              break;
          case 191:
              rc = 47;
              break;              
          case 145:
          case 32:
          case 186:
              rc = keyEvent->keyCode;
              break;
      }

      if (keyEvent->keyCode >= 48 && keyEvent->keyCode <= 57) {
        rc = (int)keyEvent->keyCode;
      } else if (keyEvent->keyCode >= 65 && keyEvent->keyCode <= 90) {
        rc = (int)keyEvent->keyCode + 32;
      }
    }

    return rc;
}

static EM_BOOL keydown_cb(int eventType, const EmscriptenKeyboardEvent *keyEvent, void *userData) {
  int k = translateKey(keyEvent);

  // lprintf(LO_INFO, "js code:%d\n", keyEvent->keyCode);
  // lprintf(LO_INFO, "final code:%d\n", k);

  if (k > 0) {
      event_t event;
      event.type = ev_keydown;
      event.data1 = k;
      D_PostEvent(&event);
  }
}

static EM_BOOL keyup_cb(int eventType, const EmscriptenKeyboardEvent *keyEvent, void *userData) {
  int k = translateKey(keyEvent);
  if (k > 0) {
      event_t event;
      event.type = ev_keyup;
      event.data1 = k;
      D_PostEvent(&event);
  }
}

void I_InitJoystick(void)
{  

  emscripten_set_keydown_callback("#document", 0, true, keydown_cb);
  emscripten_set_keyup_callback("#document", 0, true, keyup_cb);

  // emscripten
  EMSCRIPTEN_RESULT ret = emscripten_set_gamepadconnected_callback(0, 1, gamepad_callback);
  ret = emscripten_set_gamepaddisconnected_callback(0, 1, gamepad_callback);

#ifdef HAVE_SDL_JOYSTICKGETAXIS
  const char* fname = "I_InitJoystick : ";
  int num_joysticks;

  if (!usejoystick) return;
  SDL_InitSubSystem(SDL_INIT_JOYSTICK);
  num_joysticks=SDL_NumJoysticks();
  if (M_CheckParm("-nojoy") || (usejoystick>num_joysticks) || (usejoystick<0)) {
    if ((usejoystick > num_joysticks) || (usejoystick < 0))
      lprintf(LO_WARN, "%sinvalid joystick %d\n", fname, usejoystick);
    else
      lprintf(LO_INFO, "%suser disabled\n", fname);
    return;
  }
  //joystick=SDL_JoystickOpen(usejoystick-1);
  joystick= SDL_JoystickOpen(usejoystick-1);
  if (!joystick)
    lprintf(LO_ERROR, "%serror opening joystick %s\n", fname, SDL_JoystickName(usejoystick-1));
  else {
    atexit(I_EndJoystick);
    lprintf(LO_INFO, "%sopened %s\n", fname, SDL_JoystickName(usejoystick-1));
    joyup = 32767;
    joydown = -32768;
    joyright = 32767;
    joyleft = -32768;
  }
#endif
}
