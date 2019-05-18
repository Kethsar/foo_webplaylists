# foo_webplaylists
## unfinished
This is a component for [Foobar2000](https://www.foobar2000.org/) which hosts a simple HTTP server/API and HTML front-end to edit your playlists. Useful if you have Foobar running on a computer in another room and you want to edit the current playlist... I guess.

Honestly you should use [beefweb](https://github.com/hyperblast/beefweb) instead. That was made by someone that probably knows what they are doing, and is a full web-player front-end for Foobar and DeaDBeeF. I made this because I don't care for beefweb's 755 dependencies listed in its yarn.lock for the HTML front-end. Also I wanted the ability to search Foobar's media library instead of using the filesystem.

## Features
* None yet haha

## Building
You will need Visual Studio and the latest available [WTL](http://wtl.sourceforge.net/) and [Foobar2000 SDK](https://www.foobar2000.org/SDK)

* Open the Visual Studio solution file
* Open the project properties for foo_webplaylists
* Make sure "Configuration" is set to "All Configurations"
* Under "VC++ Directories" add the WTL Include path to the "Include Directories" setting (replacing C:\WTL\Include)
* Under "C/C++" -> "General" add the Foobar2000 SDK directory to the "Addition Include Directories" setting (replacing my directory)
* Under "Linker" -> "General" add the "FoobarSDK\foobar200\shared" directory (replacing my directory)
* Apply the settings and close the properties window
* Replace each of the foobar2000_* the the pfc projects with the ones from the SDK you downloaded
* Build the solution

## TODO
### Component
- [ ] Config page in foobar to set the HTTP server port.

### HTTP server
##### GET requests
- [x] Playlist Names
- [x] Playlist Data (limit of 10k songs or send nothing)

##### POST requests
- [x] Libray query
- [x] Moving playlist items {"playlist": index, "tracks": [indexes], "move_to": index}
- [x] Copy songs from one playlist to another { "to": index, "from": { "index": index, "tracks": [indexes] }}
- [ ] Copy songs from media library query to playlist { "playlist": index, "tracks": [{"path": path, "sub_index": si}] }
- [ ] Remove songs from playlist { "playlist": index, "tracks": [indexes] } 
- [x] Remove duplicates for playlist { "playlist": index }
		
### HTML front-end
- [ ] Allow moving playlist objects
  - [ ] On mouse move check mouse position relative to current items in playlist
  - [ ] set border at top or bottom depending
- [ ] Allow moving library query objects
- [ ] Right-click context menu
  - [ ] Use for Remove action
- [ ] Ctrl- and Shift-Click multi-select
- [ ] Display separators between grouped tracks in playlists

### Possible things to add after the above is completed
- [ ] Allow adding songs to Foobar's Playback Queue (would be a pain to allow removal though)
  - [ ] If added, also allow clearing the Playback Queue
- [ ] Basic playback controls (play, pause, stop, next track, previous track, etc.)
- [ ] Allow starting/stopping the server from the Foobar config page and do not just run on foobar start