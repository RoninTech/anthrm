
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

// http://www.irchelp.org/irchelp/rfc/rfc.html





#include "irc.h"
#include "../demos.h"

int main (int argc, char* argv[])
{

	if (!initDemoConfig("config.cfg"))
		return 0;

	hw->render->backGround = lGetRGBMask(frame, LMASK_WHITE);
	hw->render->foreGround = lGetRGBMask(frame, LMASK_BLACK);
	
	TIRCMYLCD *irc = newIRC();
	if (irc){
		if (initIRC(irc)){
			initSocket();
			initG15(irc, g15keycb);
			configLoad(irc, &irc->config, "irc.cfg");
			TTIRC_LOCK_CREATE();
		}else{
			freeIRC(irc);
			demoCleanup();
			return 1;
		}
	}else{
		demoCleanup();
		return 1;
	}

	irc->connection.socket = connectTo(irc->host.address, irc->host.port, IPPROTO_TCP);
	if (irc->connection.socket == SOCKET_ERROR){
		printf("unable to connect to host '%s:%i'\n",irc->host.address, irc->host.port);
	}else{

		TTIRC_LOCK();
		pthread_create((pthread_t *)&irc->connection.tid, NULL, (void *)messageProcesser, (void*)irc);
		TTIRC_UNLOCK();

		do{
			if (kbhit()){
				TTIRC_LOCK();
				if (!(irc->state=onKeyHit(irc, getch())))
					TTIRC_UNLOCK();
				else
					sendQuit(irc);
			}else{
				TTIRC_LOCK();
				if (!irc->toppage->update && !irc->toppage->pause){
					irc->toppage->update = 1;
					renderIRC(irc, irc->toppage);
				}
				drawChannel(irc, irc->toppage);
				
				if (irc->connection.socket == SOCKET_ERROR)
					irc->state = 1;
				TTIRC_UNLOCK();
				lSleep(60);
			}
		}while(!irc->state);

		TTIRC_UNLOCK();
		pthread_join(irc->connection.tid, NULL);
	}

	TTIRC_LOCK_DELETE();
	freeIRC(irc);
	demoCleanup();
	return 0;
}

void messageProcesser (void *ptr)
{
	
	TIRCMYLCD *irc = (TIRCMYLCD *)ptr;
	int state = 0;

	lSleep(100);
	TTIRC_LOCK();
	sendClientAuth(irc, irc->client.nick);
	TTIRC_UNLOCK();

	do{
		if (waitForMessage(irc)){
			TTIRC_LOCK();
			if (irc->connection.socket != SOCKET_ERROR){
				processMessage(irc);
				dispatchMessageQueue(irc);
			}else{
				state = 1;
			}
			TTIRC_UNLOCK();
		}else{
			TTIRC_LOCK();
			closeConnection(irc);
			TTIRC_UNLOCK();
			state = 1;
		}
	}while(!state);
}

int onKeyHit (TIRCMYLCD *irc, int key)
{
	if (key == '1'){
		onChanNext(irc);

	}else if (key == '2'){
		irc->currentChanPage++;
		if (irc->currentChanPage > 2)
			irc->currentChanPage = 0;

	}else if (key == '3'){
		onChanScrollUp(irc);
		drawChannel(irc, irc->toppage);
		lSleep(1);

	}else if (key == '4'){
		onChanScrollDown(irc);
		drawChannel(irc, irc->toppage);
		lSleep(1);
		
	}else if (key == 'p'){
		irc->toppage->pause ^= 1;
		if (!irc->toppage->pause)
			irc->toppage->update = 0;
	
	}else{
		return 1;
	}
	return 0;

}

DWORD g15keycb (int device, DWORD dwButtons, TMYLCDG15 *mylcdg15)
{

	TTIRC_LOCK();
	TIRCMYLCD *irc = (TIRCMYLCD *)mylcdg15->ptrUser;

	if ((dwButtons&LGLCDBUTTON_BUTTON0) && (dwButtons&LGLCDBUTTON_BUTTON3)){
		irc->state = 1;
		sendQuit(irc);

	}else if ((dwButtons&LGLCDBUTTON_BUTTON2) && (dwButtons&LGLCDBUTTON_BUTTON3)){
		if (irc->toppage != &irc->status)
			onChanPart(irc, irc->toppage);

	}else if (dwButtons&LGLCDBUTTON_BUTTON0){
		onKeyHit(irc, '1');

	}else if (dwButtons&LGLCDBUTTON_BUTTON1){
		onKeyHit(irc, '2');

	}else if (dwButtons&LGLCDBUTTON_BUTTON2){
		onKeyHit(irc, '3');

	}else if (dwButtons&LGLCDBUTTON_BUTTON3){
		onKeyHit(irc, '4');
	}

	TTIRC_UNLOCK();
	return 1;
}

void onChanScrollUp (TIRCMYLCD *irc)
{
	// font height == 7
	irc->toppage->yoffset += 7;
}

void onChanScrollDown (TIRCMYLCD *irc)
{
	// font height == 7
	irc->toppage->yoffset -= 7;
}

void onChanPrevious (TIRCMYLCD *irc)
{
	int i = irc->toppage->index;
	if (--i < 0){
		irc->toppage = &irc->status;
	}else{
		if (irc->channel[i]->active)
			irc->toppage = irc->channel[i];
	}
}

void onChanNext (TIRCMYLCD *irc)
{
	if (irc->toppage->index+1 > irc->channelTotal-1)
		irc->toppage = &irc->status;
	else if (irc->channel[irc->toppage->index+1]->active)
		irc->toppage = irc->channel[irc->toppage->index+1];
	
	irc->currentChanPage = 0;
	irc->toppage->yoffset = 0;
	irc->toppage->overlay.status = 1;	
	irc->toppage->overlay.time = GetTickCount();
}

void onChanPart (TIRCMYLCD *irc, TCHANNEL *channel)
{
	if (channel){
		if (channel->active && !channel->part){
			channel->part = 1;
			if (!channel->priv)
				sendChannelPart(irc, channel->name);
			//else
				//channel->active = 0;
			emptyChannel(channel);
		}
	}
}

void onChanInfo (TIRCMYLCD *irc, int time)
{
   	lClearFrame(irc->toppage->infoFrame);
	renderChanInfo(irc, irc->toppage, irc->toppage->infoFrame);
}


static int updateDisplay (TFRAME *frame)
{
	return lRefreshAsync(frame, 0);
}

void drawChannel (TIRCMYLCD *irc, TCHANNEL *channel)
{
	channel->yoffset = MAX(channel->yoffset, 0);
	//channel->yoffset = MIN(channel->yoffset, channel->render.frame->height - DHEIGHT);
   	displays[1].left = displays[0].left = 0;
  	displays[1].right = displays[0].right = DWIDTH-1;


	if (!irc->currentChanPage){
		channel->yoffset = MIN(channel->yoffset, channel->render.frame->height - DHEIGHT);
		displays[1].btm = displays[0].btm = (channel->render.frame->height-1) - channel->yoffset;
		displays[1].btm = displays[0].btm = MAX(displays[0].btm, DHEIGHT-1);
		displays[1].top = displays[0].top = MAX((displays[0].btm-DHEIGHT+1), 0);

		//printf("%i %i %i\n", channel->yoffset, channel->render.frame->height, displays[0].btm);
		
		if (channel->overlay.status){
			if (GetTickCount()-channel->overlay.time < channel->overlay.displaytime){
				TFRAME *temp = lCloneFrame(channel->render.frame);
				lCopyAreaEx(channel->overlay.frame, temp, (displays[0].right>>1) - ((channel->overlay.frame->width-1)>>1),\
			  	  (displays[0].top + (DHEIGHT>>1)) - ((channel->overlay.frame->height-1)>>1), 0, 0,\
			  	  channel->overlay.frame->width-1, channel->overlay.frame->height-1, 1, 1, LCASS_CPY);

				updateDisplay(temp);
				lDeleteFrame(temp);
				return;
			}else{
				channel->overlay.status = 0;
			}
		}
		updateDisplay(channel->render.frame);
		
	}else if (irc->currentChanPage == 1){
		onChanInfo(irc, 20);
		channel->yoffset = MIN(channel->yoffset, channel->infoFrame->height - DHEIGHT);
		displays[1].btm = displays[0].btm = (channel->infoFrame->height-1) - channel->yoffset;
		displays[1].btm = displays[0].btm = MAX(displays[0].btm ,DHEIGHT-1);
		displays[1].top = displays[0].top = MAX((displays[0].btm-DHEIGHT+1), 0);
		updateDisplay(channel->infoFrame);
		
	}else if (irc->currentChanPage == 2){
		drawUserList(channel, channel->userListFrame);
		channel->yoffset = MIN(channel->yoffset, channel->userListFrame->height - DHEIGHT);
		displays[1].btm = displays[0].btm = (channel->userListFrame->height-1) - channel->yoffset;
		displays[1].btm = displays[0].btm = MAX(displays[0].btm, DHEIGHT-1);
		displays[1].top = displays[0].top = MAX((displays[0].btm-DHEIGHT+1), 0);
		updateDisplay(channel->userListFrame);
	}
}

void renderIRC (TIRCMYLCD *irc, TCHANNEL *channel)
{
	memset(channel->render.buffer, 0 ,channel->render.bufferSize);

	if (!channel->render.pass || (channel->render.position == channel->render.lineTotal-1)){
		formatRenderBuffer(channel, 0, channel->render.position, channel->render.buffer);
	}else{
		char *buffer = channel->render.buffer;
		buffer = formatRenderBuffer(channel, channel->render.position+1, channel->render.lineTotal-1, buffer);
		formatRenderBuffer(channel, 0, channel->render.position, buffer);
	}

	lClearFrame(channel->render.frame);
	renderText(irc->toppage, channel->render.frame, channel->render.buffer, irc->config.renderflags|PF_CLIPDRAW|PF_NODEFAULTCHAR|PF_CLIPWRAP, channel->font);

	// draw top and bottom page marker
	//lDrawLineDotted(channel->render.frame, 0, 0, channel->render.frame->width-1, 0, LSP_SET);
	//lDrawLineDotted(channel->render.frame, 0, channel->render.frame->height-1, channel->render.frame->width-1, channel->render.frame->height-1,LSP_SET);
}

void drawUserList (TCHANNEL *channel, TFRAME *frame)
{
	char *nickbuffer = (char*)calloc(sizeof(char), IRC_NICKLENGTHMAX * channel->userListTotal);
	if (nickbuffer == NULL) return;
	char *buffer = nickbuffer;

	int i;
	for (i = 0; i<channel->userListTotal; i++){
		if (channel->user[i]->active){
			strcpy(buffer, channel->user[i]->nick);
			buffer += strlen(channel->user[i]->nick);
			*buffer++ = ' ';
			*buffer++ = ' ';
		}
	}

	lClearFrame(frame);
	renderText(channel, frame, nickbuffer, PF_CLIPTEXTH|PF_CLIPTEXTV|PF_NODEFAULTCHAR|PF_CLIPWRAP|PF_RESETXY|PF_WORDWRAP, channel->font);
	free(nickbuffer);
}

int renderText (TCHANNEL *channel, TFRAME *frame, char *buffer, int flags, int font)
{
	int total = lCountCharacters(hw, buffer);
	if (!total) return 0;
	unsigned int *glist = (unsigned int *)malloc(sizeof(unsigned int)*total);
	if (!glist) return 0;
	
	TLPRINTR rect = {0,0,DWIDTH,0,0,0,0,0};
	
	flags = PF_NODEFAULTCHAR|PF_LEFTJUSTIFY|PF_CLIPWRAP|PF_CLIPTEXTH|PF_CLIPTEXTV;
	
	lDecodeCharacterBuffer(hw, buffer, glist, total);
	lGetTextMetricsList(hw, glist, 0, total-1, flags, font, &rect);

	rect.by2 += 7;	// slight hack, 5x7.bdf font height = 7. make room for one extra line (see below)
	lResizeFrame(frame, DWIDTH, MAX(rect.by2+1, DHEIGHT), 0);
	rect.bx2 = DWIDTH;	// set maximum width

	if (!channel->priv){
		lPrintEx(frame, &rect, font, PF_RESETXY, LPRT_CPY,\
			"%s, %i users", channel->name, channel->userTotal);
	}else{
		lPrintEx(frame, &rect, font, PF_RESETXY, LPRT_CPY,\
			"%s", channel->name);
	}
	rect.bx1 = rect.by1 = 0;
	lPrintList(frame, glist, 0, total, &rect, font, flags|PF_NEWLINE|PF_RESETX, LPRT_CPY);
	free(glist);
	//lSaveImage(frame, "irc.bmp", IMG_BMP, frame->width, frame->height);
	return total;
}

void renderChanInfo (TIRCMYLCD *irc, TCHANNEL *channel, TFRAME *frame)
{
	TLPRINTR trect = {0, 0, frame->width-1, frame->height-1, 0, 1, 0, 0};

	if (channel == &irc->status){
		lPrintEx(frame, &trect, channel->font, PF_MIDDLEJUSTIFY|PF_CLIPWRAP, LPRT_CPY,\
		  "%s", irc->host.network);
		lDrawRectangleDotted(frame, 0, 0, frame->width, ++trect.ey, hw->render->foreGround);
		lPrintEx(frame, &trect, channel->font, PF_MIDDLEJUSTIFY|irc->config.renderflags|PF_NODEFAULTCHAR|PF_CLIPWRAP|PF_NEWLINE|PF_RESETX, LPRT_CPY,\
		  "%s", irc->host.address);
		lPrintEx(frame, &trect, channel->font, PF_MIDDLEJUSTIFY|irc->config.renderflags|PF_NODEFAULTCHAR|PF_CLIPWRAP|PF_NEWLINE|PF_RESETX, LPRT_CPY,\
		  "%i channels, %i users", getTotalChannels(irc), getTotalUsers(irc));
		lPrintEx(frame, &trect, channel->font, irc->config.renderflags|PF_NODEFAULTCHAR|PF_CLIPWRAP|PF_NEWLINE|PF_RESETX, LPRT_CPY,\
		  "%s", irc->toppage->topic.text);
	}else{
		if (!channel->priv){
			lPrintEx(frame, &trect, channel->font, PF_MIDDLEJUSTIFY|PF_CLIPWRAP, LPRT_CPY,\
			  "%s, %i users", channel->name, channel->userTotal);
		}else{
			lPrintEx(frame, &trect, channel->font, PF_MIDDLEJUSTIFY|PF_CLIPWRAP, LPRT_CPY,\
			  "%s", channel->name);
		}
		
		lDrawRectangleDotted(frame, 0, 0, frame->width, ++trect.ey, hw->render->foreGround);
		lPrintEx(frame, &trect, channel->font, irc->config.renderflags|PF_NODEFAULTCHAR|PF_CLIPWRAP|PF_NEWLINE|PF_RESETX, LPRT_CPY,\
		  "%s", channel->topic.text);
	}
}

char *formatRenderBuffer (TCHANNEL *channel, int from, int to, char *buffer)
{
	int i, passonce = 1;
	for (i = from; i<=to; i++){
		if (!passonce)
			*buffer++ = '\n';
		else
			passonce = 0;
		strcpy(buffer, channel->render.line[i].text);
		buffer += strlen(channel->render.line[i].text);
	}
	return buffer;
}

void closeConnection (TIRCMYLCD *irc)
{
	if (irc->connection.socket != SOCKET_ERROR){
		lSleep(1);
		closeSocket(irc->connection.socket);
		irc->connection.socket = SOCKET_ERROR;
	}
}

int initIRC (TIRCMYLCD *irc)
{
	DWIDTH = (displays[0].right - displays[0].left) + 1;
	DHEIGHT = (displays[0].btm - displays[0].top) + 1;

	irc->state = 0;
	irc->host.port = 6667;
	strcpy(irc->host.address, "localhost");
	strcpy(irc->host.network, "status");
	strcpy(irc->client.nick, "g15user");
	strcpy(irc->client.altnick, "g15user-");
	strcpy(irc->client.user, "otaku");
	strcpy(irc->client.version, "http://mylcd.sourceforge.net/ .");
	strcpy(irc->client.info, irc->client.version);
	strcpy(irc->client.quitmsg, irc->client.version);
	strcpy(irc->client.partmsg, irc->client.version);

	//irc->connection.tid = NULL;
	irc->connection.socket = SOCKET_ERROR;
	irc->connection.lastMessageSize = 0;
	irc->connection.send.size = 512;
	irc->connection.send.text[0] = 0;
	irc->connection.format.size = 512;
	irc->connection.format.text[0] = 0;
	irc->connection.recvBufferSize = 32768;
	irc->connection.recvBuffer = (char*)calloc(sizeof(char), irc->connection.recvBufferSize+8);
	if (irc->connection.recvBuffer == NULL)
		return 0;

	irc->cmds.msgTotal = 0;
	irc->cmds.queueTotal = 256;
	irc->cmds.incomplete.size = 0;
	irc->cmds.incomplete.text[0] = 0;

	irc->channelTotal = 0;
	irc->channel = (TCHANNEL **)calloc(sizeof(TCHANNEL**), 1);

	strcpy(irc->status.name, irc->host.network);
	strcpy(irc->status.topic.text, "server status page");
	strcpy(irc->status.topic.who, "myLCD");
	irc->status.active = 1;
	irc->status.part = 1;
	irc->status.priv = 0;
	irc->status.update = 1;
	irc->status.pause = 0;
	irc->status.index = -1;
	irc->status.font = LFTW_5x7;
	irc->status.userTotal = 0;
	irc->status.userListTotal = 0;
	irc->status.user = NULL;
	irc->status.yoffset = 0;
	
	irc->status.overlay.displaytime = 1500;
	irc->status.overlay.status = 0;
	irc->status.overlay.time = 0;
	irc->status.overlay.frame = lNewString(hw, DBPP, PF_INVERTTEXTRECT, irc->status.font, "  %s  ", irc->status.name);
					
	irc->status.render.frame = frame;
	irc->status.render.lineTotal = 32;
	irc->status.render.lineLength = 512;
	irc->status.render.bufferSize = (4 + irc->status.render.lineLength) * (4 + irc->status.render.lineTotal);
	irc->status.render.buffer = (char *)calloc(sizeof(char), irc->status.render.bufferSize+8);
	irc->status.render.pass = 0;
	irc->status.render.position = -1;
	irc->status.infoFrame = lNewString(hw, DBPP, 0, irc->status.font, " info page");
	irc->status.userListFrame = lNewString(hw, DBPP, 0, irc->status.font, " user list page");
	irc->toppage = &irc->status;

	irc->bot.authenticated = 0;
	irc->currentChanPage = 0;

	lSetCharacterEncoding(hw, CMT_ISO8859_16);
	lSetFontLineSpacing(hw, irc->status.font, lGetFontLineSpacing(hw, irc->status.font)-2);
	lCacheCharactersAll(hw, irc->status.font);

	return 1;
}

TIRCMYLCD *newIRC ()
{
	TIRCMYLCD *irc = (TIRCMYLCD*)calloc(1, sizeof(TIRCMYLCD));
	if (irc == NULL)
		return NULL;
	else
		return irc;
}

void freeIRC (TIRCMYLCD *irc)
{
	if (irc == NULL)
		return;
		
	int i = irc->channelTotal;
	while(i--)
		deleteChannel(channelIndexToChannel(irc, i));

	if (irc->status.render.buffer)
		free(irc->status.render.buffer);

	if (irc->status.overlay.frame)
		lDeleteFrame(irc->status.overlay.frame);

	if (irc->status.infoFrame)
		lDeleteFrame(irc->status.infoFrame);
		
	if (irc->status.userListFrame)
		lDeleteFrame(irc->status.userListFrame);		

	if (irc->connection.recvBuffer != NULL)
		free(irc->connection.recvBuffer);

	if (irc->channel)
		free(irc->channel);

	free(irc);
		
}



