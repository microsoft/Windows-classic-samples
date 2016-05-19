========================================================================
       WIN32 Console Application : WMSyncReader
========================================================================

This sample shows how to read media files using IWMSyncReader without
creating any extra thread or using callbacks. The following features are implemented :

    Reading compressed or decompressed samples
    Time-based seeking
    Frame-based seeking
    IStream derived source 

Usage :
 
WMSyncReader -i <infile> [-st <start> -e <end> -c -a -v -f]
        infile   = input file; cannot be network stream
        start    = begin of stream to play [ms]
        end      = end of stream to play [ms]; 0 means EOF
        -c       = get compressed samples, but do not play them
        -a       = read audio stream if present and play it
        -v       = read video stream if present
        -f       = range can be specified in frames rather than time
                   Both types of index have to be present in content to use this option

        REMARK: St least one of -a or -v should be set to get any samples.

/////////////////////////////////////////////////////////////////////////////
