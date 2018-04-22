
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


#include "irc.h"



int sendIRCPacket (TIRCMYLCD *irc, char *msg)
{
	snprintf(irc->connection.send.text, irc->connection.send.size, "%s\r\n", msg);
	int len = strlen(irc->connection.send.text);
	return sendSocket(irc->connection.socket, irc->connection.send.text, &len);
}

void sendKick (TIRCMYLCD *irc, char *channel, char *who, char *message)
{
	snprintf(irc->connection.format.text, irc->connection.format.size,\
	  "kick %s %s :%s", channel, who, message);
	sendIRCPacket(irc, irc->connection.format.text);	
}

void sendClientAuth (TIRCMYLCD *irc, char *nick)
{
	snprintf(irc->connection.format.text, irc->connection.format.size,\
	  "NICK %s\r\nUSER %s host %s :%s", nick, irc->client.user, irc->host.address, irc->client.info);
	sendIRCPacket(irc, irc->connection.format.text);
}

void sendChannelJoin (TIRCMYLCD *irc, char *channel)
{
	snprintf(irc->connection.format.text, irc->connection.format.size,\
	  "JOIN %s", channel);
	sendIRCPacket(irc, irc->connection.format.text);
}

void sendPong (TIRCMYLCD *irc, char *ping)
{
	snprintf(irc->connection.format.text, irc->connection.format.size,\
	  "PONG %s", ping);
	sendIRCPacket(irc, irc->connection.format.text);
}

void sendVersion (TIRCMYLCD *irc, char *from)
{
	snprintf(irc->connection.format.text, irc->connection.format.size,\
	  "NOTICE %s :\x1VERSION %s\x1", from, irc->client.version);
	sendIRCPacket(irc, irc->connection.format.text);
}

void sendMode (TIRCMYLCD *irc, char *mode)
{
	snprintf(irc->connection.format.text, irc->connection.format.size,\
	  "mode %s %s\r\nMODE %s %s", irc->client.nick, mode, irc->client.nick, mode);
	sendIRCPacket(irc, irc->connection.format.text);	
}

void sendChannelMode (TIRCMYLCD *irc, char *channel, char *mode)
{
	snprintf(irc->connection.format.text, irc->connection.format.size,\
	  "MODE %s %s", channel, mode);
	sendIRCPacket(irc, irc->connection.format.text);	
}

void sendTopic (TIRCMYLCD *irc, char *channel, char *topic)
{
	snprintf(irc->connection.format.text, irc->connection.format.size,\
	  "TOPIC %s :%s", channel, topic);
	sendIRCPacket(irc, irc->connection.format.text);	
}

void sendChannelPart (TIRCMYLCD *irc, char *cname)
{
	snprintf(irc->connection.format.text, irc->connection.format.size,\
	  "PART %s :%s", cname, irc->client.partmsg);
	sendIRCPacket(irc, irc->connection.format.text);
}

void sendPrivMsgf (TIRCMYLCD *irc, char *to, char *message, ...)
{
	va_list args;
	va_start(args, message);
	memset(irc->connection.send.text, 0, irc->connection.send.size);
	vsprintf((char*)irc->connection.send.text, message, args);
	sendPrivMsg(irc, to, irc->connection.send.text);
}

void sendPrivMsg (TIRCMYLCD *irc, char *to, char *message)
{
	if (isChannelSymbol(to))
		addChannelTextf(irc, to, "(%s) %s", irc->client.nick, message);
	else{
		addChannelTextf(irc, to, "%s: %s", irc->client.nick, message);
		setChannelTopic(irc, to, to, "<query>");
		setQueryChannel(irc, to);
	}

	snprintf(irc->connection.format.text, irc->connection.format.size,\
	  "PRIVMSG %s :%s", to, message);
	sendIRCPacket(irc, irc->connection.format.text);

}

void sendQuit (TIRCMYLCD *irc)
{
	botCmdLogOff(irc); // shouldn't be here
	lSleep(10);
	snprintf(irc->connection.format.text, irc->connection.format.size,\
	  "QUIT :%s", irc->client.quitmsg);
	sendIRCPacket(irc, irc->connection.format.text);
	/*
	lSleep(50);
	closeConnection(irc);
	*/
}


