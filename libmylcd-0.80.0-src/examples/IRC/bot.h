
// libmylcd
// An LCD framebuffer library
// Michael McElligott
// okio@users.sourceforge.net

//  Copyright (c) 2005-2009  Michael McElligott
// 
//  This program is free software; you can redistribute it and/or
//  modify it under the terms of the GNU LIBRARY GENERAL PUBLIC LICENSE
//  as published by the Free Software Foundation; either version 2
//  of the License, or (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU LIBRARY GENERAL PUBLIC LICENSE for details.


#ifndef _BOT_H_
#define _BOT_H_


void botCmdLogOff (TIRCMYLCD *irc);
void botCmdSay (TIRCMYLCD *irc, char *message);
int botAuthenticateUser (TIRCMYLCD *irc,  char *who, char *address, char *key);
void botCmdShutdown (TIRCMYLCD *irc, char *from, char *message);
void botCmdMode (TIRCMYLCD *irc, char *from, char *message);
void botCmdKick (TIRCMYLCD *irc, char *from, char *message);
int botCmdAuth (TIRCMYLCD *irc, char *address, char *from, char *message);
void botProcessPrivMessage (TIRCMYLCD *irc, char *user, char *address, char *from, char *message);


#endif


