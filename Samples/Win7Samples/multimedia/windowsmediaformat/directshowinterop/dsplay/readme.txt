Windows Media Sample - DSPlay
-----------------------------

Usage:
======

    DSPlay <media filename>


Description:
============
    This sample is an interactive audio/video media file player.  
    It uses DirectShow to play Windows Media files (ASF, WMA, WMV) 
    without DRM protection.  The DirectX 9 sample PlayWndASF demonstrates 
    how to play DRM-protected files in DirectShow.

User Input:

    Simple user input is supported through the keyboard or through the 
    application's main menu bar.

    Keyboard                    Action
    --------                    ------
    P                           Play/Pause toggle
    S                           Stop and Rewind to beginning
    M                           Audio mute toggle
    F                           Full-screen mode toggle
    ESC or Q or X or F12	Quit to menu (closes file)  


NOTE:  This sample enforces a minimum window size (currently 200x120) to prevent
window problems when small videos are resized.  You may not resize a video to play
in a window smaller than the defined minimum size.


REQUIREMENTS
------------

- DirectX 8.1 SDK (or later)

NOTE:
You need to change the project settings to point to the correct location where
strmif.h, evcode.h, strmiids.lib and quartz.lib can be found. This location should
be where you installed the DirectX 8.1 SDK.
