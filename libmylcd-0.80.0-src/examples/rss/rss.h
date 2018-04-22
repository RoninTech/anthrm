

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


// http://blogs.law.harvard.edu/tech/rss

#include "mylcd.h"




typedef struct {
	ubyte		*id;			// eg: <guid>http://some.server.com/weblogItem3207</guid>
	int			isPermaLink;	// eg: <guid isPermaLink="true">http://inessential.com/2002/09/01.php#a2</guid>
}TRSSGUID;


typedef struct {				// eg1: <category>Grateful Dead</category>
								// eg2: <category domain="http://www.fool.com/cusips">MSFT</category>
	ubyte		*Cat;			
	ubyte		*Domain;
}TRSSCATRY;


typedef struct {				// eg: <enclosure url="http://www.scripting.com/mp3s/weatherReportSuite.mp3" length="12216320" type="audio/mpeg" />
								// more info here: http://www.thetwowayweb.com/payloadsforrss
	ubyte		*URL;
	ubyte		*type;
	int			length;
}TRSSENCLOSE;


typedef struct {
	ubyte		*URL;			// eg: <source url="http://www.tomalak.org/links2.xml">Tomalak's Realm</source>
	ubyte		*Title;
}TRSSSOURCE;

typedef struct {
	ubyte		*Title;
	ubyte		*Descr;
	ubyte		*Name;
	ubyte		*Link;
}TRSSTINPUT;


typedef struct {
	ubyte		*Cloud;			//	more info : http://blogs.law.harvard.edu/tech/soapMeetsRss#rsscloudInterface
	ubyte		*Domain;
}TRSSCLOUD;


typedef struct {
	ubyte		*URL;			// link to image
	ubyte		*Title;
	ubyte		*Link;			// link to site
	int			width;
	int			height;
}TRSSIMAGE;


typedef struct {
	ubyte		*Title;			// http://blogs.law.harvard.edu/tech/rss#ltauthorgtSubelementOfLtitemgt
	ubyte		*Link;
	ubyte		*Descr;
	ubyte		*Author;		// eg: <author>lawyer@boyer.net (Lawyer Boyer)</author>
	ubyte		*Comments;		// more info: http://backend.userland.com/weblogComments
	ubyte		*PubDate;		// eg: <pubDate>Sun, 19 May 2002 15:21:36 GMT</pubDate>

	TRSSCATRY	Category;
	TRSSENCLOSE	Enclosure;
	TRSSSOURCE	Source;
	TRSSGUID	guid;			// eg: <guid>http://some.server.com/weblogItem3207</guid>
}TRSSITEM;

typedef struct {
	ubyte		*Title;
	ubyte		*Link;
	ubyte		*Descr;
	
	ubyte		*Lang;			// eg: en-us	more info: http://blogs.law.harvard.edu/tech/stories/storyReader$15
	ubyte		*Copyright;
	ubyte		*ManEditor;		// eg: geo@herald.com (George Matesky)
	ubyte		*Webmaster;		// eg: betty@herald.com (Betty Guernsey)
	ubyte		*PubDate;		// eg: Sat, 07 Sep 2002 00:00:01 GMT
	ubyte		*LastBuild;		// eg: Sat, 07 Sep 2002 09:42:31 GMT
	ubyte		*Generator;		// eg: MightyInHouse Content System v2.3
	ubyte		*Docs;			// eg: http://blogs.law.harvard.edu/tech/rss
	ubyte		*Rating;		// more info: http://www.w3.org/PICS/ pics rating for the channel.
	ubyte		*SkipHours;		// more info: http://blogs.law.harvard.edu/tech/skipHoursDays#skiphours
	ubyte		*SkipDays;		// more info: http://blogs.law.harvard.edu/tech/skipHoursDays#skipdays
	int			TTL;			// eg: <ttl>60</ttl>		minutes,time to live

	TRSSCATRY	Category;		// eg: <category>Newspapers</category>
	TRSSCLOUD	Cloud;			// eg: <cloud domain="rpc.sys.com" port="80" path="/RPC2" registerProcedure="pingMe" protocol="soap"/>
	TRSSIMAGE	Image;
	TRSSTINPUT	TextInput;		// more info: http://blogs.law.harvard.edu/tech/rss#lttextinputgtSubelementOfLtchannelgt
	
	int			totalItems;
	TRSSITEM	Item[128];		// 
	TRSSITEM	*I;	
}TRSSCHANNEL;

typedef struct {
	ubyte		*page;			// RSS page
	ubyte		*pageSrc;		// link or path to above feed

	ubyte		*XMLtag;		// xml tag
	ubyte		*RSStag;		// rss tag

	int			totalChannels;
	TRSSCHANNEL	Channel[8];
	TRSSCHANNEL	*C;
	
	ubyte		*store;			// RSS (string) storage space
	int			pos;			// number of bytes allocated thus far
	int			ssize;			// length of store, ie number of bytes
	
}TRSS;



TRSS *newRSS (ubyte *buffer, int pageLen);
int freeRSS (TRSS *rss);
int parseRSS (TRSS *RSS);

char *GetUrl (const char *url, size_t *totalRead);

#if (defined(__LINUX__))
int write_file (ubyte *path, ubyte *buffer, long len);
ubyte *read_file (ubyte *path);
#endif
