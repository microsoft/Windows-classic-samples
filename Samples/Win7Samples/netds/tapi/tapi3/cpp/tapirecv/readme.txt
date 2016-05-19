___________________________________________________

  Copyright (C) 1999-2003 Microsoft Corporation. All rights reserved.

___________________________________________________



TAPI 3.0 TAPISend Media Streaming Terminal Sample Application



Overview:


The purpose of TAPISend sample is to illustrate the use of Media Streaming 
Terminal for injecting live audio data into a TAPI media stream.

TAPISend initiates a TAPI call, opens a wave file and uses Media Streaming 
Terminal (MST) to send audio data from the file to the remote machine. When
the file is processed, TAPISend disconnects the call and exits

TAPISend is a command line application. It produces extensive logging to the
console window and can be terminated by pressing ctrl+break or closing the 
application's windows. 

To run the sample, start TAPIRecv.exe and specify the name of the file to be
sent, destination address, and address type as command line arguments:

TAPISend <file_name> <destination_address> <address_type>

The following address types are allowed (the application is 
case-insensitive): PHONENUMBER, CONFERENCE, EMAIL, MACHINE, and IP.

Usage examples:

TAPISend sound.wav 212-121-1212 phonenumber
TAPISend music.wav MYMACHINE machine
TAPISend recording.wav 127.0.0.1 IP

If the application is started with the number of arguments other than three, it
displays usage information.

For information on using Media Streaming Terminal to extract data from a TAPI
stream, refer to the TAPIRecv sample.



Building the Sample


Before building the TAPISend sample application you need to 
	- Install DirectX9SDK
	- build $(MSSDK)\samples\Multimedia\DirectShow\BaseClasses sample
	to produce strmbase.lib or strmbasd.lib file before building TAPISend sample
	-Set the build configuration of the BaseClasses sample to "Debug" or "Release" 
		using Visdual Studio's build configuration manager to build the libraries
	-run the setenv.bat file in the Platform SDK root dir

	- Set the enviroment variable DXSDK_DIR to point to the root directory of the DirectX9 SDK
	(e.g. set DXSDK_DIR=C:\DXSDK )
        NOTE: With latest Direct X SDK , DXSDK_DIR is already set, set it manually only if the env var is not set

	You can add this to the setenv.bat so DXSDK_DIR will be set 
	whenever you run setenv.bat.  A good place to add this 
	line is right after the set MSSDK= line

	-build $(MSSDK)\Samples\NetDS\Tapi\Tapi3\Cpp\Msp\MSPBase sample to produce MSPBaseSample.lib
	Type nmake in the MSPBase directory

If all the necessary are built correctly you may build the TAPISend sample application.
Type "nmake" in the TAPISend directory.  This will build TAPISend.exe.

This sample uses DirectShow\BaseClasses so you need to have Multimedia samples
installed in order to build this sample.




Application Flow


The application attempts to make a TAPI call to the destination specified in
the command line. 

Note that for simplicity TAPISend does not do TAPI message processing (it 
does not create and register with TAPI an object implementing 
ITTAPIEventNotification). All processing is done on a single thread. Please 
check documentation, TAPIRecv and other samples for more information on TAPI 
event notification processing.

When the call is connected, TAPISend creates an MST for capture. (The word 
"capture" is used in DirectShow sense, and indicates the fact that MST 
"captures" application's data to be introduced into TAPI data stream. See 
function CreateCaptureMediaStreamingTerminal.)

Once a terminal is constructed, the application opens the input file (see 
CAVIFileReader class for details on media file handling) to get the format 
of the audio stream that we are about to start sending.

The data format is then communicated to the terminal by calling 
ITAMMediaFormat::put_MediaFormat(). A failure to set format on the terminal 
may mean that the underlying MSP used for the call requires a specific wave 
format (as is the case with H323 MSP which requires 16-bit mono 8000 samples 
per second PCM data).

TAPISend uses terminal's ITAllocatorProperties interface to suggest allocator 
properties for the terminal (number and size of data buffers). If the 
application chooses to configure allocator properties, it needs to do this 
before the call is connected and terminal is selected.

The default behavior (and the behavior shown in the sample) is to have MST 
allocate buffers for the data.

The application, however, can do its own memory allocation. In this case the 
application needs to call ITAllocatorProperties::SetAllocateBuffers(FALSE). 
Later, during sample processing, the application would instruct MST's samples 
to use application-allocated buffers by calling IMemoryData::SetBuffer on MST 
samples.

Once audio format is configured and allocator properties are suggested, the 
application selects the terminal on the call's first outgoing audio stream. 

Reading the samples from the file and submitting them to the MST is done in 
ReadFileIntoTerminal.

In a loop, until connection breaks, or exit is requested, we use terminal's
IMediaStream interface to "allocate" samples on the terminal 
(a call to IMediaStream::AllocateSample, which, in effect returns us a pointer
 o IStreamSample interface of a sample. The terminal has a limited number of 
samples. This number can be set and verified via the terminal's 
ITAllocatorProperties interface.

If the application gets ahead of the terminal and all samples have been filled 
with data and submitted to MST but not yet processed, the call to 
AllocateSample will block until the MST completes processing of at least one 
sample and the processed sample becomes available to the application to be 
refilled.

We use the sample's IMemoryData interface to get to its buffer, which is filled
with audio data from the file by AVI file reader. After the data is copied to 
the sample's buffer, the application "returns" the sample to the MST by calling
Update() on its the sample's IStreamSample interface. This tells the Media 
Streaming Terminal that the sample is ready for processing (injecting into 
TAPI media stream and sending to the destination).

Note that if Update() is called while the stream is not yet active, it returns
the VFW_E_NOT_COMMITTED error code. Since TAPISend does not process events, 
and therefore not notified of when the stream is started, the application 
uses this error code as a sign that the stream is not yet active, and keeps
retrying until Update() succeeds. Again, the recommended procedure is to wait
for CME_STREAM_ACTIVE call media event before starting sending samples to 
the MST.

To keep track of samples that we have submitted, we also put the sample into 
a list. This will allow us to insure that, when the whole file is submitted, 
we don't terminate the call until all samples are completely processed by the
MST. This is achieved by calling method IStreamSample::CompletionStatus() on 
all the samples submitted during file read.

Note that the list of samples will have duplicate samples. The correctness of 
the application does not suffer, but the performance does, and this should be 
addressed in a real-life application (by making sure the list only has unique 
entries, for instance).

When all samples are submitted, or the user requested exit, we disconnect the 
call, uninitialize TAPI, and exit. Note that since we don't process events we 
don't know when the receiving side received all the samples we submitted. So 
we wait several seconds before disconnecting the call.


Additional Notes on how to build the sample: 

 How to build the samples with VS.Net or VC6 or VC5:
-	install DirectX9 SDK
-	build the $(MSSDK)\samples\Multimedia\DirectShow\BaseClasses
	DirectShow sample.  Build debug or release (or both) non-UNICODE configurations
	using the Visual studio Envoriment
-	go to the path where you installed the platform SDK 
	(e.g. C:\Program Files\Microsoft SDK) 
	and type SetEnv.Bat.
-	check the following environment variables: 
	PATH, LIB, INCLUDE. You can see their current 
	values by typing "SET" at the command prompt. 
	You should see that they contain first the SDK 
	paths and then the VC6 paths.
-	Set the enviroment variable DXSDK_DIR to point to the 
	root directory of the DirectX9 SDK
	(e.g. set DXSDK_DIR=C:\DXSDK )
        NOTE: With latest Direct X SDK(DirectX 9.0 SDK -August 2005) , DXSDK_DIR is already set
              set it manually only if the env var is not already set

	You can add this to the setenv.bat so DXSDK_DIR will be set 
	whenever you run setenv.bat.  A good place to add this 
	line is right after the set MSSDK= line
-	go to the path where the TapiSend sample is installed
	(Samples\NetDS\Tapi\Tapi3\cpp\TapiSend) and type NMAKE. 

