VLCStream - A media player for the G19 (in composite mode) using the libVLC 1.1.8 (or later) API.


Install:
Get VLC 1.1.8 (or later) from: http://www.videolan.org/

Unpack the Vlcstream archive in to the VLC install location.
(vlcstream.exe and vlc.exe should be in the same directory.)


Executing:
Run vlcstream.exe, or through the commandline pass the media file to vlcstream.exe
Eg; vlcstream "f:\path\to\file.ext"
Eg; vlcstream "f:\path\to\a playlist.m3u8" 12


Controls:
To enable mouse control: Shift+Control+A or Q or Enter.
Middle Mouse button to end.
Right mouse button to center cursor.

Play selected track (red highlight): Control + Play
Stop playback: Control + Stop
Select previous track: Control + Prev
Jump to next track: Control + Next
Adjust volume up/down: Control + Volume keys
Take a Screenshot: Control + Alt + S

VLCStream also provides an internal command prompt for advanced playback control and playlist manipulation.
To enable command prompt: Control+Shift+L or P.
Escape to close prompt and return keyboard control back to Windows.

All commands are preceded by a '#' (hash)
All other requests without the hash (#) are assumed to be search queries.
Eg; '#volume 85' to set the volume to 85% (track must be playing to take effect)
Eg; 'hands clean (' will search through all track paths and meta details (if available) for the string 'hands clean ('

Command format: #cmd argu1 argu2 etc..

Command list:
nn                                 - go to playlist or track nn
list                               - print available commands

(playlist manager)
plm                                - go to playlist manager
plm #n                             - select and go to playlist #n
plm delete #n                      - remove playlist n
plm new <name>                	   - create a new playlist
plm merge #s #d nodup              - merge playlist #s to the end of #d, deleting #s. add nodup (optional) to prevent duplicating tracks
plm copy #s #d nodup               - add playlist #s in to #d. add nodup (optional) to prevent duplicating tracks
plm first                          - select first playlist
plm last                           - select last playlist
plm forward                        - select and go to next playlist (scroll down by one)
plm back                           - select and go to previous playlist (scroll up by one)
plm dump                           - debug use only. dump playlist names to console
plm save <name>                    - save all playlists to playlist <name>. 'name' should not contain the file extension (ie; .m3u8)
                                     reload with '#pl ld #0 name.m3u8'

(playlist/pl)
pl                                 - open active playlist page
playlist up                        - scroll up playlist
playlist down                      - scroll down playlist
playlist prune                     - remove any item which points to an invalid path
playlist copy #f-t #p              - copy tracks #f-t to playlist #p (f:from, t:to, p:playlist)
playlist del #f-t                  - delete tracks from #f to t
playlist clear                     - remove all tracks from playlist
playlist setname <name>            - rename selected playlist
playlist add <dir/track>           - add track or contents of a directory in to selected playlist
playlist save #n <path>            - save playlist #n to <path> (utf8 - m3u8 format only)
playlist load #n <path>            - import .m3u8 playlist <path> in to playlist #n. use #0 to import multilists/recursive/root playlists
playlist remove <mtag> <tag>       - remove tracks from selected playlist matching this tag (eg; #pl remove artist alanis)
playlist exclude <mtag> <tag>      - as above but creates a playlist with tracks removed
playlist extract <mtag> <tag>      - create playlist from selected playlist with only those tracks which match this tag
playlist decompose <mtag>          - separate selected playlist in to multiple playlists (eg; #pl decom album)

sorta <mtag>                       - sort selected playlist in ascending order by meta tag (album, artist, path, etc..)
sortd <mtag>                       - as above but in descending order

play                               - play selected track
stop                               - stop playing track
next                               - go to and play next track
pause                              - pause playing track
meta                               - open track meta properties page
quit                               - exit program
exit                               - close the input prompt
close                              - close all pages
clock                              - show the clock
about                              - print general appl info
config                             - go to config page
find <mtag> <query>                - find a track by its meta information(eg; #find album under rug)
getmeta                            - try to preload all track meta data and artwork of selected playlist
artsize                            - set artwork image size (1-20). does not apply to playlist manager page
backlight <n>                      - set USBD480 backlight intensity (1-255)
snapshot <filename>                - save screenshot
time h:m:s                         - jump to position in track (hh:mm:ss)
fastforward n                      - skip forward n seconds
rewind n                           - go back n seconds
getlengths                         - retreive all media track lengths'

fps on                             - switch on fps display
fps off                            - switch off
fps nn                             - set display update rate to nn (1-100). this does not effect the video/movie fps

mouse on                           - switch on mouse control (oe use left control+left shift+a or q)
mouse off                          - switch off mouse control

aspect auto                        - set video aspect ratio to auto detect
aspect <n:n>                       - force pixel aspect ratio to x:y (eg; 16:9)
aspect <n.n>                       - force pixel aspect ratio to x.y (eg; 1.77)

rgbswap on                         - enable video RGB Red/Blue component swap. 
rgbswap off                        - disable above swap

skin reload                        - reload skin data
skin next                          - selects another skin

idle on                            - enable idle mode (default). display will go in to a reduced update rate after ~300 seconds
idle off                           - disable idle feature

open                               - open file browser
open <dir/track>                   - import directory or track in to selected playlist
load <dir/track>                   - as above

volume n                           - set volume from 0 to 100 (only valid when media is playing)
                                     0 = mute

visual                             - enable audio visualization
visual off                         - disable audio visualization
visual n                           - set which audio visual to use (eg; 3)
                                     0:none but displays available track meta  (default)
                                     1:VU meter
                                     2:Spectrometer
                                     3:Pineapple
                                     4:Spectrum
                                     5:Scope
                                  for Logitech G19
                                     6:Goom 320x240
                                     7:Goom 224x168
                                     8:Goom 160x90
                                  for USBD480
                                     6:Goom 480x272
                                     7:Goom 320x180
                                     8:Goom 160x90


Feedback and suggestions welcomed.

---------------

Note regarding the artwork:
libVLC will attempt to automatically download track artwork, then once saved locally which Vlcstream will then display.
The speed at which VLC acquires the track image artwork depends on several factors, but mostly related to your internet bandwidth.
Once Vlcstream has acquired the image location (MRL), it is then be recorded as part of the playlist for faster retrieval.
On exit, Vlcstream will save the current playlist to 'vlcsplaylist.m3u8' which is then automatically imported at startup.




---------------------------------

Michael McElligott
web: http://mylcd.sourceforge.net

Contact:
Email: okio@users.sourceforge.net
IRC: #g15forums.com @ irc.quakenet.org
Forum: okio @ http://www.logitechusers.com/forumdisplay.php?f=16

---------------------------------





