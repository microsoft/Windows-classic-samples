========================================================================
       Windows Application : AudioPlayer
========================================================================

Plays Windows Media files including files packaged with DRM. It is controlled through a GUI, and commands include Play, Pause, Seek and Stop.
The sample can play local files or files read from the Internet
(including those output to the Internet by using the WMVNetWrite sample).


NOTES:
1. This sample can be used to play DRM files after making the following changes to the project:
   1) The DRM stub library "wmstubdrm.lib" is required. The project settings may need to be
      altered to include the directory in which this library is found. 
   2) Add SUPPORT_DRM to the preprocessor definitions.
   Please contact wmla@microsoft.com to obtain the stub library, 
   or for more information go to: http://www.microsoft.com/windows/windowsmedia/

2. This sample shows how to handle one audio output. If you need to play a file containing 
   more than one audio output, you need to modify the source code and rebuild the sample.