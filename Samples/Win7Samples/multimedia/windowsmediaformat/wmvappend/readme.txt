========================================================================
       WIN32 Console Application : WMVAppend
========================================================================

Takes two Windows Media files for input, and attempts to create an output file with the contents of the first followed by the second.

The sample compares the profiles of the two input files to ensure they are similar enough to be appended. If this is not the case, an error message appears. For example, an error message occurs when one file is audio only and the second is an audio-video file, or when two audio files have different bit rates. 

Usage:

WMVAppend -o <output filename> -i1 <first input filename> -i2 <second input filename> [-a <attribute index> ]

The attribute index is an optional parameter.
If 1 is entered, the attributes from the first input file become those of the output.
If 2 is entered, the attributes from the second input file are used. 
The default is 1.

Note: 
The sample accepts variable bit rate (VBR) sources. However, when comparing the profiles of the two VBR sources, the sample ignores the average bitrate difference because two VBR streams will have different average bitrates even if they were created using the same profile.  WMVAppend can not compare the peak bitrate of unconstrained bitrate based VBR streams, or the quality level of quality based VBR streams because this information doesn't exist in the source files. It is therefore the user's responsibility to make sure that two source files are created using the same profile. Otherwise, invalid content can be created. 