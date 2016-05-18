================================================================================
                    WIN32 Console Application : WMVRecompress
================================================================================

This sample shows the necessary code to recompress a WMV file.

It shows reading uncompressed samples, writing uncompressed samples, multi-pass 
encoding, multi-channel output, and smart recompression.

Usage: WMVRecompress -i <INPUT_FILE> -o <OUTPUT_FILE>
                     { -ps <SYSTEM_PROFILE_INDEX> | -pf <CUSTOM_PROFILE_FILE> }
                     [-m] [-c] [-s]
  -i <INPUT_FILE>  = input ASF/WMV/WMA file name
  -o <OUTPUT_FILE>  = output file name
  -ps <SYSTEM_PROFILE_INDEX> = system profile index
  -pf <CUSTOM_PROFILE_FILE> = custom profile file name
  -m = use multi-pass encoding
  -c = enable multi-channel output for the source file (for Windows XP only)
  -s = enable smart recompression for audio stream

To build the sample, open the project file WMVRecompress.sln in Visual Studio and build 
the project.

Important methods used in this sample:

    - IWMOutputMediaProps::GetMediaType
    - IWMReader::GetOutputProps
    - IWMReaderAdvanced::DeliverTime
    - IWMReaderAdvanced::SetUserProvidedClock
    - IWMReaderAdvanced2::SetOutputSetting
    - IWMInputMediaProps::SetMediaType
    - IWMWriter::GetInputProps
    - IWMWriter::SetInputProps
    - IWMWriter::BeginWriting
    - IWMWriter::EndWriting
    - IWMWriter::WriteSample
    - IWMPropertyVault::SetProperty
    - IWMWriterPreprocess::PreprocessSample
    - IWMWriterPreprocess::BeginPreprocessingPass
    - IWMWriterPreprocess::EndPreprocessingPass
    - IWMReaderCallback::OnSample
    - IWMReaderCallbackAdvanced::OnTime
    - IWMReaderCallbackAdvanced::OnStatus
    - IWMProfileManager::LoadSystemProfile
    - IWMProfileManager::LoadProfileByData
