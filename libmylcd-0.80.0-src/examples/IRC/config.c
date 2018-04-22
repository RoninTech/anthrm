
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

typedef struct{
	unsigned int tlines;
	ubyte	**lines;
	ubyte	*data;
}TASCIILINE;


void freeASCIILINE (TASCIILINE *al);
TASCIILINE *readFileA (const char *filename);


static int isYes (char *text)
{
	if (!strncmp(text,"yes",3))
		return 1;
	else if (!strncmp(text,"show",4))
		return 1;
	else
		return 0;
	
}

int configLoad (TIRCMYLCD *irc, TCONFIG *config, char *configpathfile)
{
	
	TASCIILINE *al = readFileA(configpathfile);
	if (al == NULL) return 0;
	
	int i = 0;
	char *line = NULL;
	char *rep = NULL;
	do{
		line = (char *)al->lines[i];
		rep = (char*)strchr(line, ' ');
		if (rep) *rep = 0;
		rep = (char*)strchr(line, ';');
		if (rep) *rep = 0;
		rep = (char*)strchr(line, 9);
		if (rep) *rep = 0;

		if (!strncmp(line, "port=", 5)){
			sscanf(line+5, "%d", &irc->host.port);
		}else if (!strncmp(line, "nick=", 5)){
			sscanf(line+5, "%s",irc->client.nick);
		}else if (!strncmp(line, "mode=", 5)){
			config->mode = isYes(line+5);
		}else if (!strncmp(line, "joins=", 6)){
			config->joins = isYes(line+6);
		}else if (!strncmp(line, "quits=", 6)){
			config->quits = isYes(line+6);
		}else if (!strncmp(line, "kicks=", 6)){
			config->kicks = isYes(line+6);
		}else if (!strncmp(line, "ctcps=", 6)){
			config->ctcps = isYes(line+6);
		}else if (!strncmp(line, "query=", 6)){
			config->query = isYes(line+6);
		}else if (!strncmp(line, "server=", 7)){
			sscanf(line+7, "%s", irc->host.address);
		}else if (!strncmp(line, "invite=", 7)){
			config->invite = isYes(line+7);
		}else if (!strncmp(line, "notices=", 8)){
			config->notices = isYes(line+8);
		}else if (!strncmp(line, "altnick=", 8)){
			sscanf(line+8, "%s",irc->client.altnick);
		}else if (!strncmp(line, "channel=", 8)){
			if (*(line+8) != 0){
				char *chan = config->channel[config->channels++];
				strcpy(chan,(line+8));
				rep = (char *)strchr((char *)chan,':');
				if (rep) *rep = 32;
			}
		}else if (!strncmp(line, "querywin=", 9)){
			config->querywin = isYes(line+9);
		}else if (!strncmp(line, "wordwrap=", 9)){
			config->wordwrap = isYes(line+9);
			if (!config->wordwrap)
				irc->config.renderflags = 0;
			else
				irc->config.renderflags = PF_WORDWRAP;
		}else if (!strncmp(line, "skipmotd=", 9)){
			config->skipmotd = isYes(line+9);
		//}else if (!strncmp(line, "highlight=", 10)){
			//sscanf(line+10, "%s",);
		}else if (!strncmp(line, "autorejoin=", 11)){
			config->autorejoin = isYes(line+11);
		}else if (!strncmp(line, "nickchange=", 11)){
			config->nickchange = isYes(line+11);
		}else if (!strncmp(line, "autojoininvite=", 15)){
			config->autojoininvite = isYes(line+15);

		}else if (!strncmp(line, "botOwnerIP=", 11)){
			sscanf(line+11, "%s",irc->config.bot.ownerIP);
		}else if (!strncmp(line, "botPassword=", 12)){
			sscanf(line+12, "%s",irc->config.bot.authKey);
		}else if (!strncmp(line, "botOwnerNick=", 13)){
			sscanf(line+13, "%s",irc->config.bot.ownerNick);
		}else if (!strncmp(line, "botOwnerAddr=", 13)){
			sscanf(line+13, "%s",irc->config.bot.ownerAddr);
		}	
	}while(++i < al->tlines);

	freeASCIILINE(al);
	return 1;
}


