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


#include <stdio.h>
#include <string.h>

class PACKFILE
{

public:

      PACKFILE( char *name );
      ~PACKFILE();

      void seek( char *name );
      void read( void *buf, long size );
      bool isOk();

private:

      struct TPackHeader {
          char Identity[4];
          long InfoPosition;
          long InfoLength;
      };

      struct TPackEntry {
          char Name[0x38];
          long Position;
          long Length;
      };

      FILE *pack_file;
      TPackHeader pack_header;
      char pack_name[12];
      bool pack_ok;

};
