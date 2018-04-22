
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


static int isAuthd (TIRCMYLCD *irc)
{
	return irc->bot.authenticated;
}

static int isBotMaster (TIRCMYLCD *irc, char *who)
{
	return (!strncmp(irc->bot.master, who, 63));
}

void botCmdLogOff (TIRCMYLCD *irc)
{
	if (isAuthd(irc)){
		sendPrivMsg(irc, irc->bot.master, "good bye");
		*irc->bot.master = 0;
		irc->bot.authenticated = 0;
	}
}

void botCmdSay (TIRCMYLCD *irc, char *message)
{
	char *to = (char*)strtok(message, " ");
	if (to){
		message = (char*)strtok(NULL, "\0");
		if (message)
			sendPrivMsg(irc, to, message);
	}
}

int botAuthenticateUser (TIRCMYLCD *irc,  char *who, char *address, char *key)
{
	if (!strncmp(irc->config.bot.authKey, key, 63)){
		if (!strncmp(irc->config.bot.ownerAddr, address, 63) || !strncmp(irc->config.bot.ownerIP, address, 64))
			//if (!strncmp(irc->config.bot.ownerNick, who, 63)
				return 1;
	}
	return 0;
}

void botCmdShutdown (TIRCMYLCD *irc, char *from, char *message)
{
	sendPrivMsg(irc, from, "shutting down");
	lSleep(10);
	sendQuit(irc);
}

void botCmdKick (TIRCMYLCD *irc, char *from, char *message)
{
	char *channel = (char*)strtok(message, " ");
	if (channel){
		char *who = (char*)strtok(NULL, " ");
		if (who){
			message = (char*)strtok(NULL, "\0");
			if (!message)
				sendKick(irc, channel, who, (char*)"good bye");
			else
				sendKick(irc, channel, who, message);
		}
	}
}

void botCmdMode (TIRCMYLCD *irc, char *from, char *message)
{
	char *channel = (char*)strtok(message, " ");
	if (channel){
		message = (char*)strtok(NULL, "\0");
		if (message)
			sendChannelMode(irc, channel, message);
	}
}

int botCmdAuth (TIRCMYLCD *irc, char *address, char *from, char *message)
{
	if (!botAuthenticateUser(irc, from, address, message+5)){
		return 0;
	}else{
		if (isAuthd(irc)){
			if (isBotMaster(irc, from)){
				sendPrivMsg(irc, from, "you are already logged on");
				return 1;
			}else{
				sendPrivMsgf(irc, irc->bot.master, "access control removed, %s has logged on", from);
			}
		}

		strncpy(irc->bot.master, from, 63);
		sendPrivMsg(irc, from, "authentication successful");
		sendPrivMsgf(irc, from, "Hello %s", irc->config.bot.ownerNick);
		irc->bot.authenticated = 1;
		return 1;
	}
}

void botCmdHelp (TIRCMYLCD *irc, char *from, char *message)
{
	sendPrivMsg(irc, from, "-help-");
	sendPrivMsg(irc, from, "join - join a channel");
	sendPrivMsg(irc, from, "part - leave channel");
	sendPrivMsg(irc, from, "kick - kick user from channel");
	sendPrivMsg(irc, from, "mode - set channel mode (ops, voice, etc..)");
	sendPrivMsg(irc, from, "say - speak in channel");
	sendPrivMsg(irc, from, "msg - send private message to user");
	sendPrivMsg(irc, from, "gettopic - show channel topic");
	sendPrivMsg(irc, from, "settopic - set channel topic");
	sendPrivMsg(irc, from, "listcommands - list commands and usage");
	sendPrivMsg(irc, from, "about - general application info");
	sendPrivMsg(irc, from, "AUTH - login to bot");
	sendPrivMsg(irc, from, "LOGOFF - log off bot");
	sendPrivMsg(irc, from, "SHUTDOWN - shut-down bot");
	sendPrivMsg(irc, from, "-end of help-");
}

void botCmdAbout (TIRCMYLCD *irc, char *from)
{
	sendPrivMsgf(irc, from, "libmylcd irc v%.2f.%i",libmylcdVERSIONmj, libmylcdVERSIONmi);
	sendPrivMsg(irc, from, "http://mylcd.sourceforge.net/");
	sendPrivMsg(irc, from, "by Michael McElligott");
}

void botCmdListCmds (TIRCMYLCD *irc, char *from)
{
	sendPrivMsg(irc, from, "-commands- (commands are case sensitive)");
	sendPrivMsg(irc, from, "help");
	sendPrivMsg(irc, from, "gettopic #channel");
	sendPrivMsg(irc, from, "settopic #channel text");
	sendPrivMsg(irc, from, "join #channel password");
	sendPrivMsg(irc, from, "part #channel");
	sendPrivMsg(irc, from, "kick #channel user");
	sendPrivMsg(irc, from, "mode #channel +-mode user");
	sendPrivMsg(irc, from, "say #channel text");
	sendPrivMsg(irc, from, "msg user text");
	sendPrivMsg(irc, from, "about");
	sendPrivMsg(irc, from, "AUTH password");
	sendPrivMsg(irc, from, "LOGOFF");
	sendPrivMsg(irc, from, "SHUTDOWN");
	sendPrivMsg(irc, from, "-end of commands-");
}

void botCmdStatus (TIRCMYLCD *irc, char *from)
{
	sendPrivMsgf(irc, from, "connected to %s", irc->host.network);
	sendPrivMsgf(irc, from, "%s:%i", irc->host.address, irc->host.port);
	sendPrivMsgf(irc, from, "in %i channels with %i users", getTotalChannels(irc), getTotalUsers(irc));
	sendPrivMsg(irc, from, "-in channels-");
	
	int i;
	for (i = 0; i<irc->channelTotal; i++){
		if (irc->channel[i]->active)
			if ((!irc->channel[i]->part) && (!irc->channel[i]->priv))
				sendPrivMsgf(irc, from, "%s - %i users",irc->channel[i]->name, irc->channel[i]->userTotal);
	}
	sendPrivMsg(irc, from, "-end of channels-");
}

void botCmdSetTopic (TIRCMYLCD *irc, char *message)
{
	char *cname = (char*)strtok(message," ");
	message = (char*)strtok(NULL,"\0");
	if (cname && message){
		if (*cname && *message)
			sendTopic(irc, cname, message);
	}
}

void botCmdGetTopic (TIRCMYLCD *irc, char *from, char *cname)
{
	cname = (char*)strtok(cname," ");
	if (cname){
		TCHANNEL *channel = channelNameToChannel(irc, cname);
		if (channel){
			sendPrivMsgf(irc, from, "-topic for %s-",cname);
			sendPrivMsgf(irc, from, "%s", channel->topic.text);
			sendPrivMsgf(irc, from, "set by %s", channel->topic.who);
			sendPrivMsg(irc, from, "-end of topic-");
		}else{
			sendPrivMsgf(irc, from, "not in channel %s", cname);
		}
	}
}

void botProcessPrivMessage (TIRCMYLCD *irc, char *user, char *address, char *from, char *message)
{

	//printf("## -%s- -%s- -%s- -%s-\n", user, address, from, message);

	if (!strncmp(message, "AUTH ", 5)){
		if (!botCmdAuth(irc, address, from, message))
			return;
	}

	if (!isAuthd(irc) || !isBotMaster(irc, from))
		return;

	if (!strncmp(message, "say ", 4)){
		botCmdSay(irc, message+4);
	}else if (!strncmp(message, "msg ", 4)){
		botCmdSay(irc, message+4);
	}else if (!strncmp(message, "help", 4)){
		botCmdHelp(irc, from, message+4);
	}else if (!strncmp(message, "join ", 5)){
		sendChannelJoin(irc, message+5);
	}else if (!strncmp(message, "part ", 5)){
		onChanPart(irc, channelNameToChannel(irc, message+5));
	}else if (!strncmp(message, "mode ", 5)){
		botCmdMode(irc, from, message+5);
	}else if (!strncmp(message, "kick ", 5)){
		botCmdKick(irc, from, message+5);
	}else if (!strncmp(message, "about", 5)){
		botCmdAbout(irc, from);
	}else if (!strncmp(message, "status", 6)){
		botCmdStatus(irc, from);
	}else if (!strncmp(message, "LOGOFF", 6)){
		botCmdLogOff(irc);
	}else if (!strncmp(message, "HELLO ", 6)){
	}else if (!strncmp(message, "SHUTDOWN", 8)){
		botCmdShutdown(irc, from, message);
	}else if (!strncmp(message, "gettopic ", 9)){
		botCmdGetTopic(irc, from, message+9);
	}else if (!strncmp(message, "settopic ", 9)){
		botCmdSetTopic(irc, message+9);
	}else if (!strncmp(message, "listcommands", 12)){
		botCmdListCmds(irc, from);
	}
}

