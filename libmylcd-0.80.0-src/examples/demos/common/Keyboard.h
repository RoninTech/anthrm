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



#ifndef __KEYBOARD_H
#define __KEYBOARD_H

#include <stdio.h>
#include <conio.h>
#include <tchar.h>
#include "conio.h"

extern "C" int __cdecl _getch (void);

class KEYBOARD
{
   public:

   KEYBOARD();
   ~KEYBOARD();
   //bool isKeyPressed( unsigned char code );
 //  bool getKey( unsigned char code );

	bool getKey( unsigned char code )
	{
		if (kbhit()){
			int c = _getch();
			printf("%i %i\n",c,'y');
			return (c == code);
		}
		return 0;
	}
	
	bool isKeyPressed( unsigned char code )
	{
		if (kbhit()){
			int c = _getch();
			printf("%i %i\n",c,'y');
			return ((c == code));
		}
		return 0;
	}

   private:


};


KEYBOARD::~KEYBOARD()
{
}

KEYBOARD::KEYBOARD()
{
}



#endif
