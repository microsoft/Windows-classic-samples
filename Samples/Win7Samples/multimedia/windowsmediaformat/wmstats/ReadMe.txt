========================================================================
       WIN32 Console Application : WMStats
========================================================================

WMStats is a sample that displays Reader and Writer statistics. 

WMStats can be run in three modes :
-(f)ile   : reads in a file, then writes it out to a new file, then displays DataUnitExtension info and reader and writer statistics.
-(s)erver : sends streams read from a file to a network and displays writer statistics.
-(c)lient : reads a stream sent by server, then displays reader statistics.

Multiple instances of WMStats can also be used concurrently on one machine.  Start one instance as a server to send the stream to the network and then run a second instance as a client to verify that the server is streaming correctly.

Usage:
wmstats -m <mode> -i <infile> -o <output> [-s <statistics_delay>]
	mode	 = mode : f- file, s - server or c - client
	infile	 = WMV file name or URL for server or client mode
	output	 = for file   : WMV output file name
		   for server : port number
		   for client : not valid
	statistics_delay = [sec] delay of statistics report; default 20 seconds

Samples :
file                    : wmstats -m f -i c:\wmroot\welcome2.asf -o c:\wmroot\test.asf
client (input: network) : wmstats -m c -i http://banhammer:32800
client (input: file)    : wmstats -m c -i c:\wmroot\welcome2.asf
server                  : wmstats -m s -i c:\wmroot\welcome2.asf -o 32800

Sample Output:

-File Mode
WMStats initialized successfully. 

 ExtGUID : {C6BD9450-867F-4907-83A3-C77921B733AD} 	ExtDataSize : 2 	ExtData[0x] :  21 00  	ExtSystInfoSize : 0 	ExtSystInfo[0x] :

------------- READER STATISTICS ------------
	Bandwidth [bps]     : 0
	PacketsReceived     : 129
	PacketsRecovered    : 0
	PacketsLost         : 0
	Quality []         : 100
-------------------------------------------------------


------------- WRITER STATISTICS ------------
	Connected clients                              : 0

	SUMMARY STATISTICS: 
	SampleCount                                : 436
	ByteCount                                  : 342744
	DroppedSampleCount [1000 * (samples/s)]    : 0
	DroppedByteCount                           : 0
	CurrentBitrate [bps]                       : 221058
	AverageBitrate [bps]                       : 213727
	ExpectedBitrate [bps]                      : 221000
	CurrentSampleRate [1000 * (samples/s)]     : 35178
	AverageSampleRate [1000 * (samples/s)]     : 35331
	ExpectedSampleRate [1000 * (samples/s)]    : 35208

	Stream Number                              : 1
	SampleCount                                : 69
	ByteCount                                  : 52992
	DroppedSampleCount [1000 * (samples/s)]    : 0
	DroppedByteCount                           : 0
	CurrentBitrate [bps]                       : 32000
	AverageBitrate [bps]                       : 32000
	ExpectedBitrate [bps]                      : 32000
	CurrentSampleRate [1000 * (samples/s)]     : 5208
	AverageSampleRate [1000 * (samples/s)]     : 5284
	ExpectedSampleRate [1000 * (samples/s)]    : 5208

	Stream Number                              : 2
	SampleCount                                : 367
	ByteCount                                  : 289752
	DroppedSampleCount [1000 * (samples/s)]    : 0
	DroppedByteCount                           : 0
	CurrentBitrate [bps]                       : 189058
	AverageBitrate [bps]                       : 181727
	ExpectedBitrate [bps]                      : 189000
	CurrentSampleRate [1000 * (samples/s)]     : 29970
	AverageSampleRate [1000 * (samples/s)]     : 30047
	ExpectedSampleRate [1000 * (samples/s)]    : 30000
-------------------------------------------------------

	BitratePlusOverhead                        : 225551
	CurrentSampleDropRateInQueue               : 0
	CurrentSampleDropRateInCodec               : 0
	CurrentSampleDropRateInMultiplexer         : 0
	TotalSampleDropsInQueue                    : 0
	TotalSampleDropsInCodec                    : 0
	TotalSampleDropsInMultiplexer              : 0


-Client Mode:
WMStats initialized succefully as a CLIENT

EndOfStream detected in reader.

------------- READER STATISTICS ------------
	Bandwidth [bps]     : 0
	PacketsReceived     : 258
	PacketsRecovered    : 0
	PacketsLost         : 0
	Quality []         : 100



-Server Mode:
WMStats initialized succefully as a SERVER;  port : 32800 :

 ExtGUID : {C6BD9450-867F-4907-83A3-C77921B733AD} 	ExtDataSize : 2 	ExtData[0x] :  21 00  	ExtSystInfoSize : 0 	ExtSystInfo[0x] :

------------- WRITER STATISTICS ------------
	Connected clients                              : 4

	SUMMARY STATISTICS: 
	SampleCount                                : 395
	ByteCount                                  : 311392
	DroppedSampleCount [1000 * (samples/s)]    : 0
	DroppedByteCount                           : 0
	CurrentBitrate [bps]                       : 216967
	AverageBitrate [bps]                       : 213583
	ExpectedBitrate [bps]                      : 221000
	CurrentSampleRate [1000 * (samples/s)]     : 35273
	AverageSampleRate [1000 * (samples/s)]     : 35349
	ExpectedSampleRate [1000 * (samples/s)]    : 35208

	Stream Number                              : 1
	SampleCount                                : 63
	ByteCount                                  : 48384
	DroppedSampleCount [1000 * (samples/s)]    : 0
	DroppedByteCount                           : 0
	CurrentBitrate [bps]                       : 32000
	AverageBitrate [bps]                       : 32000
	ExpectedBitrate [bps]                      : 32000
	CurrentSampleRate [1000 * (samples/s)]     : 5208
	AverageSampleRate [1000 * (samples/s)]     : 5292
	ExpectedSampleRate [1000 * (samples/s)]    : 5208

	Stream Number                              : 2
	SampleCount                                : 332
	ByteCount                                  : 263008
	DroppedSampleCount [1000 * (samples/s)]    : 0
	DroppedByteCount                           : 0
	CurrentBitrate [bps]                       : 184967
	AverageBitrate [bps]                       : 181583
	ExpectedBitrate [bps]                      : 189000
	CurrentSampleRate [1000 * (samples/s)]     : 30065
	AverageSampleRate [1000 * (samples/s)]     : 30057
	ExpectedSampleRate [1000 * (samples/s)]    : 30000
-------------------------------------------------------

	BitratePlusOverhead                        : 225405
	CurrentSampleDropRateInQueue               : 0
	CurrentSampleDropRateInCodec               : 0
	CurrentSampleDropRateInMultiplexer         : 0
	TotalSampleDropsInQueue                    : 0
	TotalSampleDropsInCodec                    : 0
	TotalSampleDropsInMultiplexer              : 0
-------------------------------------------------------
