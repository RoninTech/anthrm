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


#include "pack.h"

/*
 * Check if the file exists, and open it, and read the header
 */
PACKFILE::PACKFILE( char *name )
{
    char *test = "PACK";
    pack_ok = false;
    pack_file = fopen( name, "rb" );
    if ((ferror(pack_file)) || (ftell( pack_file )==-1)) return;
    fread(  &pack_header, sizeof( TPackHeader ), 1, pack_file );
    if (strncmp((const char*)pack_header.Identity, test, 4) == 0)
       pack_ok = true;
}

/*
 * Close the file, free the pointer
 */
PACKFILE::~PACKFILE()
{
   if (pack_file)
   {
       fclose( pack_file );
       delete pack_file;
   }
}

/*
 * Find the offset of a file in the pack
 */
void PACKFILE::seek( char *name )
{
     if (!pack_ok) return;
     TPackEntry Entry;
     fseek( pack_file, pack_header.InfoPosition, SEEK_SET );
     for (int i=0; i<pack_header.InfoLength>>6; i++)
     {
         fread( &Entry, sizeof( TPackEntry ), 1, pack_file );
         if (!stricmp( Entry.Name, name ))
         {
            fseek( pack_file, Entry.Position, SEEK_SET );
            return;
         }
     }
     pack_ok = false;
};

void PACKFILE::read( void *buf, long size )
{
     if (pack_ok) fread( buf, 1, size, pack_file );
};

bool PACKFILE::isOk()
{
     return pack_ok;
};

