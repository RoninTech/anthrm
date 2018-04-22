
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

#ifndef _SEND_H_
#define _SEND_H_

void sendPrivMsg (TIRCMYLCD *irc, char *to, char *message);
void sendPrivMsgf (TIRCMYLCD *irc, char *to, char *message, ...);
void sendChannelMode (TIRCMYLCD *irc, char *channel, char *mode);
void sendKick (TIRCMYLCD *irc, char *channel, char *who, char *message);
void sendTopic (TIRCMYLCD *irc, char *channel, char *topic);

void sendQuit (TIRCMYLCD *irc);
void sendChannelJoin (TIRCMYLCD *irc, char *channel);
void sendClientAuth (TIRCMYLCD *irc, char *nick);
void sendMode (TIRCMYLCD *irc, char *mode);
void sendChannelPart (TIRCMYLCD *irc, char *cname);
void sendPong (TIRCMYLCD *irc, char *ping);
void sendVersion (TIRCMYLCD *irc, char *from);
int sendIRCPacket (TIRCMYLCD *irc, char *msg);
//void sendGetChannelTopic (TIRCMYLCD *irc, char *channel);


#endif

