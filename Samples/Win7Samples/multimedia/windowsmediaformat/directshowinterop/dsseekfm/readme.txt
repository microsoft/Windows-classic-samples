DirectShow Sample -- DSSeekFm
-----------------------------

Demonstrates how to:

- Use DirectShow's Windows Media SDK-based ASF Reader Filter to play
  ASF content in a DirectShow graph

- Use the Corona SDK's frame-seeking support (which isn't yet supported 
  by the DirectShow ASF Reader filter) to get the associated frame time to
  use when seeking the DirectShow graph. Note that ASF frame-based seeking
  is only supported with frame-indexed ASF files.

Specify the name of a frame-seekable ASF source file. 
With no other options specified, the file is played to completion. 
Otherwise, the file is seeked and played back for the specified duration.


Usage:

DSSeekFm [/f FrameStart NumFramesToPlay] [/l LoopModeIncrement] Filename

The following command-line switches are supported:

    /f Specifies the starting frame (1-based) and number of frames to play

    /l Loops until EOF, playing number of frames specified by /f switch, 
           and incrementing start frame by LoopModeIncrement

Example: DSSeekFm /f 100 50 frameseek.wmv
      This will seek to frame 100 and play 50 frames.

Example: DSSeekFm /f 100 10 /l 50 frameseek.wmv
     This will seek to frame 100, play 10 frames, advance 50 frames,
     play 10 more frames, advance 50 frames, etc. until completion.


REQUIREMENTS
------------

- DirectX 8.1 SDK (or later)
