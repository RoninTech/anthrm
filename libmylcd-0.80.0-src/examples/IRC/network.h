
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


#ifndef _NETWORK_H_
#define _NETWORK_H_

int msgNumeric (TIRCMYLCD *irc, int item);
void msgPing (TIRCMYLCD *irc, int item);
void msgJoin (TIRCMYLCD *irc, int item);
void msgMode (TIRCMYLCD *irc, int item);
void msgNick (TIRCMYLCD *irc, int item);
void msgNoticeA (TIRCMYLCD *irc, int item);
void msgNoticeB (TIRCMYLCD *irc, int item);
void msgQuit (TIRCMYLCD *irc, int item);
void msgPart (TIRCMYLCD *irc, int item);
void msgCTCP (TIRCMYLCD *irc, int item, char *name, char *to, char *ctcp);
void msgTopic (TIRCMYLCD *irc, int item);
void msgInvite (TIRCMYLCD *irc, int item);
void msgKick (TIRCMYLCD *irc, int item);
void msgPrivMsg (TIRCMYLCD *irc, int item);
void msgMessage (TIRCMYLCD *irc, int item);

int isCTCP (char *buffer);
int isChannelSymbol (char *symbol);
int isUserMe (TIRCMYLCD *irc, char *name);
int isUserInChannel (TIRCMYLCD *irc, char *cname, char *nick);

int getTotalUsers (TIRCMYLCD *irc);
int getTotalChannels (TIRCMYLCD *irc);

int waitForMessage (TIRCMYLCD *irc);
int processMessage (TIRCMYLCD *irc);
int dispatchMessageQueue (TIRCMYLCD *irc);

void emptyChannel (TCHANNEL *channel);
void deleteChannel (TCHANNEL *channel);
int getChannelIndex (TIRCMYLCD *irc, char *cname);
int createChannel (TIRCMYLCD *irc, char *cname);
int createChannelIndex (TIRCMYLCD *irc, char *cname);
TCHANNEL *channelIndexToChannel (TIRCMYLCD *irc, int i);
TCHANNEL *channelNameToChannel (TIRCMYLCD *irc, char *cname);

void setChannelTopic (TIRCMYLCD *irc, char *channel, char *who, char *topic);
void setQueryChannel (TIRCMYLCD *irc, char *cname);
void addUserToChannel (TIRCMYLCD *irc, char *cname, char *user);
void setChannelUserCeiling (TCHANNEL *channel, int total);

int insertChannelUser (TCHANNEL *channel, char *user);
void removeUserFromChannel (TIRCMYLCD *irc, char *cname, char *nick);
void removeUserFromChannels (TIRCMYLCD *irc, char *nick, char *message);
void userNickChange (TIRCMYLCD *irc, char *oldNick, char *newNick);

int addChannelTextf (TIRCMYLCD *irc, char *cname, char *text, ...);
int addChannelText (TIRCMYLCD *irc, char *cname, char *text);
void renderAddLine (TCHANNEL *channel, char *text);

char *filterColourCodes (char *string);

#endif

