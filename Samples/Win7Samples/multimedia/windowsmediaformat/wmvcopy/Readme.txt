================================================================================
                    WIN32 Console Application : WMVCopy
================================================================================

This sample shows the necessary code to copy a WMV file.

It demonstrates how to read compressed samples, write compressed samples, read 
header attributes and scripts, and modify header attributes.

Usage: wmvcopy  -i <INPUT_FILE> -o <OUTPUT_FILE> -d <TIME> [-s]
  -i <INPUT_FILE> = Input Windows Media file name
  -o <OUTPUT_FILE> = Output Windows Media file name
  -d <TIME> = specify the max duration in seconds of the output file
  -s = Move scripts in script stream to header

  Note: this program will not copy the image stream in the source file.

To build the sample, open the project file WMVCopy.dsp in Visual Studio and build 
the project.

Important methods used in this sample:

    - IWMReaderAdvanced::SetReceiveStreamSamples
    - IWMReaderAdvanced::SetManualStreamSelection
    - IWMReaderAdvanced::DeliverTime
    - IWMReaderAdvanced::SetUserProvidedClock
    - IWMReaderCallbackAdvanced::OnStreamSample
    - IWMReaderCallbackAdvanced::OnTime
    - IWMReaderCallbackAdvanced::OnStatus
    - IWMWriter::BeginWriting
    - IWMWriter::EndWriting
    - IWMWriterAdvanced::WriteStreamSample
    - IWMHeaderInfo::GetScript
    - IWMHeaderInfo::AddScript
    - IWMHeaderInfo::GetMarker
    - IWMHeaderInfo::AddMarker
    - IWMProfileManager::LoadSystemProfile
    - IWMProfileManager::LoadProfileByData
