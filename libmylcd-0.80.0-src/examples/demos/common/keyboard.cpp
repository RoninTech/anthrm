/*
 *     terraVox: an experimental voxel landscape engine
 *
 *     Copyright (C) 1999/2000 Alex J. Champandard
 *
 *
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 *
 *     This program is distributed in the hope that it will be useful,
 *     but WITHOUT ANY WARRANTY; without even the implied warranty of
 *     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *     GNU General Public License for more details.
 *
 *     You should have received a copy of the GNU General Public License
 *     along with this program; if not, write to the Free Software
 *     Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 *
 *     The tutorial that accompanies this source code can be found at
 *         http://www.flipcode.com
 *
 *     You can also visit the terraVox web site at
 *         http://atlas.cs.york.ac.uk/~ajc116/terraVox
 *
 *     And if you wish to contact me for any reason, I can be reached at
 *         alexjc@altavista.com
 *
 */


// thanks to:
//   Alaric B. Williams for writing "The Dark Art of writing DJGPP Hardware Interrupt Handlers"
//   Shawn Hargreaves for the huge amount of work he put into ALLEGRO

#include <pc.h>
#include <dpmi.h>
#include "keyboard.h"

static _go32_dpmi_seginfo newKeyHandler, oldKeyHandler;
static int keyInstalled = false;
static unsigned char *keyBuffer;

void keyHandler()
{
// read scan code
  unsigned char keyCode = inp(0x60);
// store scan code in buffer
  if (keyCode > 127) {
     keyBuffer[keyCode&0x7f] = 0;
  } else {
    keyBuffer[keyCode] = 1;
  }
// trap control+alt+del
//   if (keyBuffer[83] && keyBuffer[56] && keyBuffer[29])
//   {
//      exit(-1);
//   }
// acknowledge interrupt
  outp(0x20, 0x20);
}

void keyHandler_end() { }

KEYBOARD::KEYBOARD()
{
// check if installed
  if (keyInstalled == true) return;
// allocate array
  keyBuffer = new unsigned char [256];
  for (int i=0; i < 256; i++) keyBuffer[i] = 0;
// lock all used variables
  _go32_dpmi_lock_data( &keyBuffer, sizeof(keyBuffer));
// lock the code
  _go32_dpmi_lock_code( &keyHandler, (long)keyHandler - (long)keyHandler_end );
// make the procedure able to handle interrupts
  newKeyHandler.pm_offset = (long)keyHandler;
  _go32_dpmi_allocate_iret_wrapper(&newKeyHandler);
// backup old handler
  _go32_dpmi_get_protected_mode_interrupt_vector(0x09, &oldKeyHandler);
// set new handler
  _go32_dpmi_set_protected_mode_interrupt_vector(0x09, &newKeyHandler);
// remember the handler is installed
  keyInstalled = true;
}

KEYBOARD::~KEYBOARD()
{
// check if already installed
  if (keyInstalled == false) return;
// restore old handler
  _go32_dpmi_set_protected_mode_interrupt_vector(0x09, &oldKeyHandler);
// free code segment
  _go32_dpmi_free_iret_wrapper(&newKeyHandler);
// free memory
  delete [] keyBuffer;
// set state
  keyInstalled = false;
}

bool KEYBOARD::isKeyPressed( unsigned char code )
{
   return keyBuffer[code];
}

bool KEYBOARD::getKey( unsigned char code )
{
   if (keyBuffer[code])
   {
      keyBuffer[code] = false;
      return true;
   } else return false;
}
