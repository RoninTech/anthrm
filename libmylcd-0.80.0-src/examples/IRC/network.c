
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
#include "numerics.h"

extern THWD *hw;
extern int DWIDTH;
extern int DHEIGHT;
extern int DBPP;


int getTotalChannels (TIRCMYLCD *irc)
{
	int i;
	int tChannels = 0;
	for (i = 0; i < irc->channelTotal; i++){
		if (irc->channel[i]->active)
			if ((!irc->channel[i]->part) && (!irc->channel[i]->priv))
				tChannels++;
	}
	
	return tChannels;
}

int getTotalUsers (TIRCMYLCD *irc)
{
	int i;
	int tUsers = 0;
	for (i = 0; i < irc->channelTotal; i++){
		if (irc->channel[i]->active)
			if ((!irc->channel[i]->part) && (!irc->channel[i]->priv))
				tUsers += irc->channel[i]->userTotal;
	}
	
	return tUsers;
}

int addChannelTextf (TIRCMYLCD *irc, char *cname, char *text, ...)
{
	va_list args;
	va_start(args, text);
	memset(irc->toppage->render.buffer, 0, irc->toppage->render.bufferSize);
	vsprintf((char*)irc->toppage->render.buffer, (char*)text, args);

	addChannelText(irc, cname, irc->toppage->render.buffer);
	return 0;
}

int addChannelText (TIRCMYLCD *irc, char *cname, char *text)
{
	TCHANNEL *channel;
	if ((channel=channelNameToChannel(irc, cname)) == NULL){
		createChannel(irc, cname);
		if ((channel=channelNameToChannel(irc, cname)) == NULL){
			printf("addChannelText(): unable to reference channel %s\n",cname);
			return 0;
		}
	}

	renderAddLine(channel, text);
	return 1;
}

void renderAddLine (TCHANNEL *channel, char *text)
{
	if (++channel->render.position > channel->render.lineTotal-1){
		channel->render.position = 0;
		channel->render.pass++;
	}
	strncpy(channel->render.line[channel->render.position].text, text, channel->render.lineLength-1);
	channel->update = 0;
}

int waitForMessage (TIRCMYLCD *irc)
{
	TTIRC_LOCK();
    memset(irc->connection.recvBuffer, 0, irc->connection.recvBufferSize);
    if (irc->cmds.incomplete.size)
    	memcpy(irc->connection.recvBuffer, irc->cmds.incomplete.text, irc->cmds.incomplete.size);
	TTIRC_UNLOCK();
	
	irc->connection.lastMessageSize =\
	  readSocket(irc->connection.socket, irc->connection.recvBuffer + irc->cmds.incomplete.size,\
	  irc->connection.recvBufferSize-irc->cmds.incomplete.size-1);

	if (irc->connection.lastMessageSize != SOCKET_ERROR)
		irc->connection.lastMessageSize += irc->cmds.incomplete.size;

	return (irc->connection.lastMessageSize > 0);
}

int processMessage (TIRCMYLCD *irc)
{
	irc->cmds.msgTotal = 0;
	int start = 0;
	int i;
	
	for (i=0; i < irc->connection.lastMessageSize; i++){
		if (irc->connection.recvBuffer[i] == '\r'){
			irc->connection.recvBuffer[i] = 0;
			memcpy(irc->cmds.queue[irc->cmds.msgTotal++].text, irc->connection.recvBuffer+start, MIN((i-start)+1, 511));
			if (irc->connection.recvBuffer[++i] == '\n') i++;
			start = i;
		}
	}

	if (i-start > 1){
		irc->cmds.incomplete.size = i-start; 
		memcpy(irc->cmds.incomplete.text, &irc->connection.recvBuffer[start], irc->cmds.incomplete.size);
		irc->cmds.incomplete.text[irc->cmds.incomplete.size] = 0;
	}else{
		irc->cmds.incomplete.size = 0;
	}

	return 1;
}


int getChannelIndex (TIRCMYLCD *irc, char *cname)
{
	int i;
	for (i = 0; i < irc->channelTotal; i++){
		if (!strncmp(irc->channel[i]->name, cname, 32)){
			//if (!irc->channel[i].active)
				return i;
		}
	}
	return -1;
}

TCHANNEL *channelNameToChannel (TIRCMYLCD *irc, char *cname)
{
	int i;
	for (i = 0; i < irc->channelTotal; i++){
		if (!strncmp(irc->channel[i]->name, cname, 32)){
			if (irc->channel[i]->active)
				return irc->channel[i];
		}
	}
	
	if (!strncmp(irc->status.name, cname, 32))
		return &irc->status;
	else
		return NULL;
}

TCHANNEL *channelIndexToChannel (TIRCMYLCD *irc, int i)
{
	if (i < irc->channelTotal)
		return irc->channel[i];
	else
		return NULL;
}

int createChannelIndex (TIRCMYLCD *irc, char *cname)
{
	irc->channel = (TCHANNEL **)realloc(irc->channel, (++irc->channelTotal+1) * sizeof(TCHANNEL**));
	irc->channel[irc->channelTotal-1] = (TCHANNEL *)calloc(1, sizeof(TCHANNEL));
	return irc->channelTotal-1;
}

int createChannel (TIRCMYLCD *irc, char *cname)
{
	int i;
	if ((i=getChannelIndex(irc, cname)) < 0)
		i = createChannelIndex(irc, cname);

	TCHANNEL *channel = channelIndexToChannel(irc, i);
	if (channel != NULL){
		strcpy(channel->name, cname);
		strcpy(channel->topic.text, "<topic not set>");
		strcpy(channel->topic.who, "<>");
		channel->active = 1;
		channel->part = 0;
		channel->priv = 0;
		channel->update = 1;
		channel->pause = 0;
		channel->index = i;
		channel->yoffset = 0;
		channel->font = irc->status.font;
		channel->userTotal = 0;
		int i = channel->userListTotal = 32;
		channel->user = (TUSER **)calloc(sizeof(TUSER**), channel->userListTotal);
		while(i--)
			channel->user[i] = (TUSER *)calloc(sizeof(TUSER), 1);
		
		channel->overlay.status = 0;
		channel->overlay.time = 0;
		channel->overlay.displaytime = irc->status.overlay.displaytime;
		if (!channel->overlay.frame)
			channel->overlay.frame = lNewString(hw, DBPP, PF_INVERTTEXTRECT, channel->font, "  %s  ", channel->name);

		if (!channel->render.frame)
			channel->render.frame = lNewFrame(irc->status.render.frame->hw, DWIDTH, DHEIGHT, DBPP);

		if (!channel->infoFrame)
			channel->infoFrame = lNewFrame(irc->status.render.frame->hw, DWIDTH, DHEIGHT, DBPP);

		if (!channel->userListFrame)
			channel->userListFrame = lNewFrame(irc->status.render.frame->hw, DWIDTH, DHEIGHT, DBPP);


		channel->render.bufferSize = irc->status.render.bufferSize;
		channel->render.buffer = (char *)calloc(sizeof(char),channel->render.bufferSize);
		channel->render.lineLength = irc->status.render.lineLength;
		channel->render.lineTotal = irc->status.render.lineTotal;
		channel->render.pass = 0;
		channel->render.position = -1;
		
		irc->toppage = channel;
		return 1;
	}

	return 0;
}

void deleteChannel (TCHANNEL *channel)
{
	if (channel != NULL){
		channel->active = 0;
		emptyChannel(channel);

		if (channel->render.frame)
			lDeleteFrame(channel->render.frame);

		if (channel->render.buffer)
			free(channel->render.buffer);

		if (channel->overlay.frame)
			lDeleteFrame(channel->overlay.frame);

		if (channel->infoFrame)
			lDeleteFrame(channel->infoFrame);
		
		if (channel->userListFrame)
			lDeleteFrame(channel->userListFrame);	
			

		//channel->overlay.frame = NULL;
		//channel->render.buffer = NULL;
		//channel->render.frame = NULL;
		free(channel);
	}
}

void removeUserFromChannel (TIRCMYLCD *irc, char *cname, char *nick)
{
	TCHANNEL *channel;
	if ((channel=channelNameToChannel(irc, cname)) == NULL){
		return;
	}else{
		int n;
		for (n = 0; n < channel->userListTotal; n++){
			if (channel->user[n]->active){
				if (!strncmp(channel->user[n]->nick, nick, 63)){
					channel->user[n]->active = 0;
					channel->userTotal--;
					return;					
				}
			}
		}		
	}
}

int isUserInChannel (TIRCMYLCD *irc, char *cname, char *nick)
{
	TCHANNEL *channel;
	if ((channel=channelNameToChannel(irc, cname)) == NULL){
		return 0;
	}else{
		int n;
		for (n = 0; n < channel->userListTotal; n++){
			if (channel->user[n]->active){
				if (!strncmp(channel->user[n]->nick, nick, 63))
					return 1;
			}
		}		
	}

	return 0;
}

void userNickChange (TIRCMYLCD *irc, char *oldNick, char *newNick)
{
	int c,n;
	for (c = 0; c < irc->channelTotal; c++){
		for (n = 0; n < irc->channel[c]->userListTotal; n++){
			if (irc->channel[c]->user[n]->active){
				if (!strncmp(irc->channel[c]->user[n]->nick, oldNick, 63)){
					strncpy(irc->channel[c]->user[n]->nick, newNick, 63);
					if (irc->config.nickchange)
						addChannelTextf(irc, irc->channel[c]->name, "* %s becomes %s", oldNick, newNick);
				}
			}
		}
	}
}

void removeUserFromChannels (TIRCMYLCD *irc, char *nick, char *message)
{
	int c,n;
	for (c = 0; c < irc->channelTotal; c++){
		for (n = 0; n < irc->channel[c]->userListTotal; n++){
			if (irc->channel[c]->user[n]->active){
				if (!strncmp(irc->channel[c]->user[n]->nick, nick, 63)){
					irc->channel[c]->user[n]->active = 0;
					irc->channel[c]->userTotal--;
					if (irc->config.quits){
						if (message != NULL)
							addChannelTextf(irc, irc->channel[c]->name, "* %s quit (%s)", nick, message);
						else
							addChannelTextf(irc, irc->channel[c]->name, "* %s quit", nick);
					}
				}
			}
		}
	}
}

void setChannelUserCeiling (TCHANNEL *channel, int total)
{
	channel->user = (TUSER **)realloc(channel->user, sizeof(TUSER**) * total);
	int i;
	for (i = channel->userListTotal; i < total; i++)
		channel->user[i] = (TUSER *)calloc(sizeof(TUSER), 1);
	channel->userListTotal = total;
}

void addUserToChannel (TIRCMYLCD *irc, char *cname, char *user)
{
	TCHANNEL *channel;
	if ((channel=channelNameToChannel(irc, cname)) == NULL){
		printf("addUserToChannel(): invalid channel -%s-\n", cname);
		return;
	}else{
		if (!insertChannelUser(channel, user)){
			setChannelUserCeiling(channel, channel->userListTotal+32);
			insertChannelUser(channel, user);
		}
		
	}
}

int insertChannelUser (TCHANNEL *channel, char *user)
{
	int i;
	for (i = 0; i < channel->userListTotal; i++){
		if (!channel->user[i]->active){
			channel->user[i]->active = 1;
			strncpy(channel->user[i]->nick, user, 63);
			channel->userTotal++;
			return 1;
		}
	}
	return 0;
}

static char *getStatusChannelName (TIRCMYLCD *irc)
{
	return irc->status.name;
}

int isChannelSymbol (char *symbol)
{
	return ((*symbol == '#') || (*symbol == '&') || (*symbol == '+'));
}

void setChannelTopic (TIRCMYLCD *irc, char *cname, char *who, char *topic)
{
	TCHANNEL *channel = channelNameToChannel(irc, cname);
	if (channel != NULL){
		if (strlen(topic))
			strncpy(channel->topic.text, filterColourCodes(topic), 511);
		if (strlen(who))
			strncpy(channel->topic.who, who, 63);
	}
	
	return;
}

void setQueryChannel (TIRCMYLCD *irc, char *cname)
{
	TCHANNEL *channel = channelNameToChannel(irc, cname);
	if (channel != NULL)
		channel->priv = 1;
}

int msgNumeric (TIRCMYLCD *irc, int item)
{
	char *buffer = 1+(char *)strchr(irc->cmds.queue[item].text, ' ');
	int ircCode = atoi(buffer);
	if (!ircCode) return 0;	
	
	buffer = 1+(char *)strchr(buffer, ' ');	// jump to name
	buffer = 1+(char *)strchr(buffer, ' ');	// jump to text
	if (*buffer == ':')	buffer++;				// we don't want the colon

	switch (ircCode){
		case RPL_WELCOME:
			addChannelText(irc, getStatusChannelName(irc), buffer);
			sendMode(irc, "+i");
			
			strcpy(irc->status.topic.text, buffer);
			buffer = irc->cmds.queue[item].text;
			if (*buffer == ':')	buffer++;
			strcpy(irc->host.address, (char*)strtok(buffer, " "));
			break;
		case RPL_ISUPPORT:
		
			addChannelText(irc, getStatusChannelName(irc), buffer);
			char *network = strstr(buffer, "NETWORK=");
			if (network)
				strcpy((char*)irc->host.network, (char*)strtok((network+8)," "));
			break;
		case RPL_MOTD:
			if (!irc->config.skipmotd)
				addChannelText(irc, getStatusChannelName(irc), buffer);
			break;
		case RPL_MOTDSTART:
			break;
		case RPL_NAMREPLY:
		{
			if (*buffer == '=') buffer++;
			if (*buffer == ' ') buffer++;
			if (!isChannelSymbol(buffer))
				buffer = 1+(char *)strchr(buffer, ' ');	// skip operator symbol

			char *channel = buffer;
			char *users = (char *)strchr(buffer, ' ');
			*(users++) = 0;
			if (*users == ':') users++;
			//addChannelTextf(irc, getStatusChannelName(irc), "users in %s: %s", channel, users);

			char *user = (char*)strtok(users, " \0");
			do{
				if ((*user == '@')||(*user == '+'))
					user++;
				if (!isUserInChannel(irc, channel, user))
					addUserToChannel(irc, channel, user);
			}while((user=(char*)strtok(NULL, " \0")));

			break;
		}
		case RPL_ENDOFNAMES:
			break;

		case ERR_INVALIDUSERNAME:
			printf("%s\n",buffer);
			break;

		case ERR_NICKNAMEINUSE:
			addChannelText(irc, getStatusChannelName(irc), "** nick in use, trying alt nick...");
		case ERR_NICKCOLLISION:
			strncpy(irc->client.nick, irc->client.altnick, 63);
			sendClientAuth(irc, (char*)irc->client.nick);
			break;

		case RPL_TOPIC:
		{
			char *channel = (char *)strtok(buffer, " ");
			buffer = (char *)strtok(NULL, "\0");
			if (*buffer == ':') buffer++;
			filterColourCodes(buffer);
			addChannelTextf(irc, channel, "topic: %s", buffer);
			setChannelTopic(irc, channel, channel, buffer);
			break;
		}
		case RPL_TOPICWHOTIME:
		{
			char *channel = (char *)strtok(buffer, " ");
			char *who = (char *)strtok(NULL, " ");
			setChannelTopic(irc, channel, who, "");
			break;
		}
		case RPL_ENDOFMOTD:
		{	
			int i;
			for(i = 0; i<irc->config.channels; i++){
				sendChannelJoin(irc, irc->config.channel[i]);
				lSleep(200);
			}

		   #if 0
			int t = 200;
			// channels to test with
			sendChannelJoin(irc,"#pcw"); lSleep(t);
			sendChannelJoin(irc,"#cod2.wars"); lSleep(t);
			sendChannelJoin(irc,"#3on3"); lSleep(t);
			sendChannelJoin(irc,"#5on5.css"); lSleep(t);
			sendChannelJoin(irc,"#6on6.et"); lSleep(t);
			sendChannelJoin(irc,"#3on3.et"); lSleep(t);
			sendChannelJoin(irc,"#2on2"); lSleep(t);
			sendChannelJoin(irc,"#4on4"); lSleep(t);
			sendChannelJoin(irc,"#matsi"); lSleep(t);
			sendChannelJoin(irc,"#dotapickup.euro"); lSleep(t);
			sendChannelJoin(irc,"#arkku.net"); lSleep(t);
			sendChannelJoin(irc,"#clanmatch"); lSleep(t);
			sendChannelJoin(irc,"#wow.stormreaver"); lSleep(t);
			sendChannelJoin(irc,"#esl.fifa"); lSleep(t);
			sendChannelJoin(irc,"#skilled.pcw"); lSleep(t);
			sendChannelJoin(irc,"#clanwar.fr"); lSleep(t);
			sendChannelJoin(irc,"#esl.de"); lSleep(t);
			sendChannelJoin(irc,"#2on2.css"); lSleep(t);
			sendChannelJoin(irc,"#clansuche"); lSleep(t);
			sendChannelJoin(irc,"#3on3.css"); lSleep(t);
			sendChannelJoin(irc,"#exotic"); lSleep(t);
			sendChannelJoin(irc,"#warfinder"); lSleep(t);
		   #endif

			break;
		}
		default:
			addChannelText(irc, getStatusChannelName(irc), buffer);
			break;
	}
	return 1;
}



void msgPing (TIRCMYLCD *irc, int item)
{
	char *buffer = irc->cmds.queue[item].text;
	if (*buffer == ':') buffer++;
	buffer = 1+(char*)strchr(buffer, ' ');		// skip PING
	sendPong(irc, buffer);
}

void msgNoticeA (TIRCMYLCD *irc, int item)
{
	char *buffer = 1+(char*)strchr(irc->cmds.queue[item].text, ':');
	filterColourCodes(buffer);
	addChannelText(irc, getStatusChannelName(irc), buffer);
}

void msgNoticeB (TIRCMYLCD *irc, int item)
{
	char *buffer = irc->cmds.queue[item].text;
	if (*buffer == ':') buffer++;

	char *who = (char *)strtok(buffer, " ");	// ip
	buffer = (char *)strtok(NULL, " ");			// NOTICE command
	buffer = (char *)strtok(NULL, " ");			// user
	buffer = (char *)strtok(NULL, "\0");		// message
	if (*buffer == ':') buffer++;	
	if (strchr(who,'!')) who = (char *)strtok(who, "!");
	filterColourCodes(buffer);
	
	if (irc->config.notices)
		addChannelTextf(irc, irc->toppage->name, "-%s- %s", who, buffer);
}

void msgNick (TIRCMYLCD *irc, int item)
{
	char *buffer = irc->cmds.queue[item].text;
	if (*buffer == ':') buffer++;
	char *who = (char*)strtok(buffer,"!");	// old nick
	buffer = (char*)strtok(NULL," ");			// ip
	buffer = (char*)strtok(NULL," ");			// NICK command
	buffer = (char*)strtok(NULL,"\0");			// new nick
	if (*buffer == ':') buffer++;
	userNickChange(irc, who, buffer);
}

void msgQuit (TIRCMYLCD *irc, int item)
{

	char *buffer = irc->cmds.queue[item].text;
	if (*buffer == ':') buffer++;
	char *who = (char*)strtok(buffer,"!");	// name
	buffer = (char*)strtok(NULL," ");			// ip
	buffer = (char*)strtok(NULL," ");			// QUIT command
	buffer = (char*)strtok(NULL,"\0");			// leaving message
	if (*buffer == ':') buffer++;
	removeUserFromChannels(irc, who, buffer);
}

void msgPart (TIRCMYLCD *irc, int item)
{
	char *buffer = irc->cmds.queue[item].text;
	if (*buffer == ':') buffer++;
	char *who = (char*)strtok(buffer,"!");	// name	
	buffer = (char*)strtok(NULL," ");			// ip
	buffer = (char*)strtok(NULL," ");			// PART command
	char *channel = (char*)strtok(NULL," ");	// channel
	buffer = (char*)strtok(NULL,"\0");			// reason, if any
	
	removeUserFromChannel(irc, channel, who);

	if (irc->config.quits || isUserMe(irc, who)){
		if (buffer){
			if (*buffer == ':') buffer++;
			if (buffer)
				addChannelTextf(irc, channel, "* %s has left (%s)", who, buffer);
			return;
		}else{
			addChannelTextf(irc, channel, "* %s has left", who);
		}

		if (isUserMe(irc, who)){
			TCHANNEL *chan = channelNameToChannel(irc, channel);
			if (chan){
				emptyChannel(chan);
				chan->part = 1;
			}
		}
	}
}

void msgMode (TIRCMYLCD *irc, int item)
{
	if (!irc->config.mode)
		return;

	char *from, *channel;
	char *user, *mode;
	char *buffer = irc->cmds.queue[item].text;
	if (*buffer == ':') buffer++;				// remove colon
	
	from = (char *)strtok(buffer," ");
	buffer = (char*)strtok(NULL," ");			// MODE command
	buffer = (char*)strtok(NULL," ");			// mode (channel or user)

	if (isChannelSymbol(buffer)){
		channel = buffer;
		mode = (char*)strtok(NULL,"\0");		// mode setting plus specific mode details
		from = (char *)strtok(from,"!");
		addChannelTextf(irc, channel, "** %s sets mode %s", from, mode);
	}else{
		user = buffer;
		mode = (char*)strtok(NULL,"\0");
	  	addChannelTextf(irc, getStatusChannelName(irc), "%s sets mode %s", user, mode);
	}
}

int isUserMe (TIRCMYLCD *irc, char *name)
{
	if (!strncmp(irc->client.nick, name, 63))
		return 1;
	else
		return 0;
}

void msgKick (TIRCMYLCD *irc, int item)
{
	char *buffer = irc->cmds.queue[item].text;
	if (*buffer == ':') buffer++;
	char *name = (char*)strtok(buffer,"!");	// who	
	buffer = (char*)strtok(NULL," ");			// ip
	buffer = (char*)strtok(NULL," ");			// KICK command
	char *cname = (char*)strtok(NULL," ");	// channel
	char *who = (char*)strtok(NULL," ");		// who
	buffer = (char*)strtok(NULL,"\0");			// reason
	if (*buffer == ':') buffer++;

	removeUserFromChannel(irc, cname, who);
	if (irc->config.kicks || isUserMe(irc, who))
		addChannelTextf(irc, cname, "** %s kicked %s (%s)", name, who, buffer);

	if (isUserMe(irc, who)){
		TCHANNEL *channel = channelNameToChannel(irc, cname);
		if (channel){
			emptyChannel(channel);
			channel->part = 1;
		}

		if (irc->config.autorejoin)
			sendChannelJoin(irc, cname);
	}
}

void msgTopic (TIRCMYLCD *irc, int item)
{
	char *buffer = irc->cmds.queue[item].text;
	if (*buffer == ':') buffer++;
	char *who = (char*)strtok(buffer,"!");	// who	
	buffer = (char*)strtok(NULL," ");			// ip
	buffer = (char*)strtok(NULL," ");			// TOPIC command
	char *channel = (char*)strtok(NULL," ");	// channel
	buffer = (char*)strtok(NULL,"\0");			// new topic
	if (*buffer == ':') buffer++;

	filterColourCodes(buffer);
	addChannelTextf(irc, channel, "** %s sets topic to: %s", who, buffer);
	setChannelTopic(irc, channel, who, buffer);
}

void msgInvite (TIRCMYLCD *irc, int item)
{
	char *buffer = irc->cmds.queue[item].text;
	if (*buffer == ':') buffer++;
	char *who = (char*)strtok(buffer,"!");	// who	
	buffer = (char*)strtok(NULL," ");			// ip
	buffer = (char*)strtok(NULL," ");			// INVITE command
	buffer = (char*)strtok(NULL," ");			// nick for self
	buffer = (char*)strtok(NULL," ");			// to channel

	if (irc->config.invite)
		addChannelTextf(irc, irc->toppage->name, "%s has invited you to %s", who, buffer);

	if (irc->config.autojoininvite)
		sendChannelJoin(irc, buffer);
}

void msgJoin (TIRCMYLCD *irc, int item)
{

	char *buffer = irc->cmds.queue[item].text;
	if (*buffer == ':') buffer++;
	char *name = (char*)strtok(buffer,"!");	// who	
	buffer = (char*)strtok(NULL," ");			// ip
	buffer = (char*)strtok(NULL," ");			// JOIN command
	char *channel = (char*)strtok(NULL,"\0");	// channel joined
	if (*channel == ':') channel++;

	if (isUserMe(irc, name)){
		createChannel(irc, channel);
		//sendGetChannelTopic(irc, channel);
		addUserToChannel(irc, channel, name);
		addChannelTextf(irc, channel, "* %s joins %s", name, channel);
	}else{
		addUserToChannel(irc, channel, name);
		if (irc->config.joins)
			addChannelTextf(irc, channel, "* %s joins %s", name, channel);
	}
}

void msgCTCP (TIRCMYLCD *irc, int item, char *from, char *to, char *ctcp)
{
	if (!irc->config.ctcps)
			return;

	if (!strncmp(ctcp,"PING",4)){
		addChannelTextf(irc, irc->toppage->name, "* PING from %s", from);
		  
	}else if (!strncmp(ctcp,"TIME",4)){
		addChannelTextf(irc, irc->toppage->name, "* TIME from %s", from);
		  
	}else if (!strncmp(ctcp,"CHAT",4)){
		addChannelTextf(irc, irc->toppage->name, "* CTCP CHAT from %s", from);
		  
	}else if (!strncmp(ctcp,"PAGE",4)){
		addChannelTextf(irc, irc->toppage->name, "* CTCP PAGE from %s", from);
		  
	}else if (!strncmp(ctcp,"ACTION",6)){
		char *msg = (char *)strchr(ctcp, 1);
		if (msg != ctcp){		//	if address is the same then so is the token
			msg = 1+(char *)strchr(ctcp, ' ');
			msg = (char*)strtok(msg, "\1");
			if (!isChannelSymbol(to)) to = from;
			addChannelTextf(irc, to, "* %s %s", from, msg);

		}else{
			addChannelText(irc, to, from);
		}
	}else if (!strncmp(ctcp,"FINGER",6)){
		addChannelTextf(irc, irc->toppage->name, "* FINGER from %s", from);

	}else if (!strncmp(ctcp,"VERSION",7)){
		addChannelTextf(irc, irc->toppage->name, "* VERSION from %s", from);
		sendVersion(irc, from);

	}else if (!strncmp(ctcp,"USERINFO",8)){
		addChannelTextf(irc, irc->toppage->name, "* USERINFO from %s", from);

	}else if (!strncmp(ctcp,"DCC SEND",8)){
		addChannelTextf(irc, irc->toppage->name, "* DCC send from %s", from);

	}else if (!strncmp(ctcp,"DCC CHAT",8)){
		addChannelTextf(irc, irc->toppage->name, "* DCC chat from %s", from);

	}else if (!strncmp(ctcp,"XDCC LIST",9)){
		addChannelTextf(irc, irc->toppage->name, "* XDCC list from %s", from);

	}else if (!strncmp(ctcp,"CDCC LIST",9)){
		addChannelTextf(irc, irc->toppage->name, "* CDCC list from %s", from);

	}else if (!strncmp(ctcp,"CLIENTINFO",10)){
		addChannelTextf(irc, irc->toppage->name, "* CLIENTINFO from %s", from);

	}else{
		//printf("unhandled ctcp: -%s- -%s- -%s-\n", from, to, ctcp);
		return;
	}
}

void msgPrivMsg (TIRCMYLCD *irc, int item)
{
	char *buffer = irc->cmds.queue[item].text;
	if (*buffer == ':') buffer++;
	char *name = (char*)strtok(buffer,"!");	// from
	char *address = (char*)strtok(NULL," ");	// ip
	buffer = (char*)strtok(NULL," ");			// PRIVMSG command
	char *to = (char*)strtok(NULL," ");		// to/channel (this connection/client)
	buffer = (char*)strtok(NULL,"\0"); 		// actual chat message
	if (*buffer == ':')	buffer++;				// remove colon
	filterColourCodes(buffer);					// remove mIRC colour codes

	if (isChannelSymbol(to)){					// channel chat
		if (isCTCP(buffer))
			msgCTCP(irc, item, name, to, ++buffer);
		else
			addChannelTextf(irc, to, "(%s) %s", name, buffer);

		return;
	}else if (isCTCP(buffer)){					// CTCP command
		msgCTCP(irc, item, name, to, ++buffer);
	}else{
		if (*buffer == ' ')	buffer++;
		if (irc->config.query && *buffer){
			if (!irc->config.querywin){
				addChannelTextf(irc, irc->toppage->name, "-> %s: %s", name, buffer);
			}else{
				addChannelTextf(irc, name, "%s: %s", name, buffer);
				setChannelTopic(irc, name, name, "<query>");
				setQueryChannel(irc, name);		// set channel as query 
			}
			
			char *user = (char*)strtok(address,"@");
			if (!user){
				user = (char *)"\0";
			}else{
				if (*user == '~') user++;
				address= (char*)strtok(NULL,"\0");
			}
			botProcessPrivMessage(irc, user, address, name, buffer);
		}
	}
}

void emptyChannel (TCHANNEL *channel)
{
	if (channel){
		if (channel->user != NULL){
			int i = channel->userListTotal;
			while(i--){
				if (channel->user[i] != NULL)
					free(channel->user[i]);
			}
			free(channel->user);
			channel->user = NULL;
			channel->userListTotal = 0;
			channel->userTotal = 0;
		}
	}
}

int isCTCP (char *buffer)
{
	return (*buffer == 1);
}

// remove mIRC colour code
char *filterColourCodes (char *string)
{
	char *text = string;
	int len = strlen(text);
	int i;

	for (i=0; i<len; i++){
		if (*text++ == 0x03){
			if ((*text > 47) && (*text < 58)){			// foreground colour
				*text++ = 2;
				if ((*text > 47) && (*text < 58))
					*text++ = 2;

				if (*text == ','){						// background
					*text++ = 2;
					if ((*text > 47) && (*text < 58)){
						*text++ = 2;
						if ((*text > 47) && (*text < 58))
							*text++ = 2;
					}
				}
			}
		}
	}
	return string;
}

void msgMessage (TIRCMYLCD *irc, int item)
{
	char *buffer = 1+(char *)strchr(irc->cmds.queue[item].text, ' ');
	if (!strncmp(buffer,"PRIVMSG",7)){
		msgPrivMsg(irc, item);
	}else if (!strncmp(buffer,"JOIN",4)){
		msgJoin(irc, item);
	}else if (!strncmp(buffer,"MODE",4)){
		msgMode(irc, item);
	}else if (!strncmp(buffer,"NICK",4)){
		msgNick(irc, item);
	}else if (!strncmp(buffer,"QUIT",4)){
		msgQuit(irc, item);
	}else if (!strncmp(buffer,"PART",4)){
		msgPart(irc, item);
	}else if (!strncmp(buffer,"KICK",4)){
		msgKick(irc, item);
	}else if (!strncmp(buffer,"TOPIC",5)){
		msgTopic(irc, item);
	}else if (!strncmp(buffer,"NOTICE",6)){
		msgNoticeB(irc, item);
	}else if (!strncmp(buffer,"INVITE",6)){
		msgInvite(irc, item);
	}//else
		//printf("unhandled message: -%s-\n", buffer);
}

int dispatchMessageQueue (TIRCMYLCD *irc)
{

	char *msg;
	int i;
	for (i = 0; i < irc->cmds.msgTotal; i++){
		msg = irc->cmds.queue[i].text;
		if (*msg == '\r') msg++;
		if (*msg == '\n') msg++;
		if (*msg == ':'){
			if (!msgNumeric(irc, i))
				msgMessage(irc, i);
		}else if (!strncmp(msg,"PING",4)){
			msgPing(irc, i);
		}else if (!strncmp(msg,"ERROR",5)){
			addChannelText(irc, getStatusChannelName(irc), msg);
		}else if (!strncmp(msg,"NOTICE",6)){
			msgNoticeA(irc, i);
		}else{
			//printf("unhandled queue msg: -%s-\n", msg);
			if (!msgNumeric(irc, i))
				msgMessage(irc, i);
		}
	}
	return 1;
}


