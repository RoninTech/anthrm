
Use console keys 1,2,3 and 4 to navigate.


Communicating and controlling the client/bot is via a private message channel (a query window).
(commands are case sensitive.)

Command list:-
join - join a channel
part - leave channel
kick - kick user from channel
mode - set channel mode (ops, voice, etc..)
say - speak in channel
msg - send private message to user
gettopic - show channel topic
settopic - set channel topic
listcommands - list commands and usage
about - general application info
AUTH - login to bot
LOGOFF - log off bot
SHUTDOWN - shut-down bot
	
Command usage:-
"gettopic #channel"
"settopic #channel text"
"join #channel password"
"part #channel"
"kick #channel user"
"mode #channel +-mode user"
"say #channel text"
"msg user text"
"about"
"help"
"AUTH password"
"LOGOFF"
"SHUTDOWN"

Authenticating with the bot is through a standard password challenge.
Bot will then verify the users IP address against the owners preconfigured IP address.
If a match is found on both accounts then access is granted.
If either password or address do not match access is denied.
Access password and owners IP address are configured in irc.cfg


note:
irc.exe requires pthreadGC2.dll





