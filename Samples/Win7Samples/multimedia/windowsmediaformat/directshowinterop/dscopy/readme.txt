Windows Media Sample -- DSCopy
------------------------------

Uses DirectShow interfaces to transcode one or more files to an ASF file.


Usage:

DSCopy [/v] [/l] [/f] [/m] [/n Version] /p Profile Source1 [Source2 ...] Target

The following command-line switches are supported:
    /v Verbose mode
    /l Lists all available system profiles (versions 4,7,8,9)
    /f Selects frame-based indexing (instead of default temporal indexing)
    /m Enables multipass encoding
    /n Selects a system profile version (4, 7, 8, or 9)
    /p Specifies the profile number

Specify an ASF profile using the /p switch.  If you omit this switch, ASFCopy 
displays a list of the standard system profiles and exits.

Specify the name of one or more source files and the name of the target file. 
If you specify more than one source file, the application multiplexes all of 
the source files.  You must specify a profile that matches the streams 
contained in the source files, or else the application will not work correctly. 
For example, if you specify Video for Web Servers (56 Kbps), the combined 
source files must have exactly one video stream and one audio stream.

NOTE: If the source media files contain more streams than the number of streams 
supported by a selected profile, then the additional streams are not encoded.


REQUIREMENTS
------------

- DirectX 8.1 SDK (or higher)
