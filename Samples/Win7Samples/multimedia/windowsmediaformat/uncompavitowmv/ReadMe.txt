================================================================================
                WIN32 Console Application : UncompAVIToWMV
================================================================================

This sample shows the necessary code to compress an AVI file to a WMV file.

It shows how to merge samples for audio and video streams from several AVI files
and either merge these into similar streams or create a new stream based on the 
source stream profile.  It also shows how to create an arbitrary stream, do
multipass encoding and add SMPTE time codes.

Usage: UncompAVIToWMV -i <INPUT_FILE> -o <OUTPUT_FILE> [-if]
                      [ -ps <SYSTEM_PROFILE_INDEX> | -pf <CUSTOM_PROFILE_FILE> ]
                      [-pe] [-sp <FILE>] [-arb] [-m] [-drm] [-smpte] [-d <TIME>]
                      [-attrib <NUMBER> <NAME> <TYPE> <VALUE>] [-pause]

  -i <INPUT_FILE> = Uncompressed input AVI/WAV file name or if -if specified 
                    ASCII file containing a list of uncompressed AVI/WAV files
  -o <OUTPUT_FILE> = Output file name
  -if = load list of AVI/WAV files from file specified by -i <INPUT_FILE>
  -ps <SYSTEM_PROFILE_INDEX> = load the system profile by index
  -pf <CUSTOM_PROFILE_FILE> = load custom profile by file name
  -pe = expand specified profile to include all input streams
  -sp <FILE> = save the current profile to the file
  -arb = include arbitrary stream containing sample number
  -m = use multipass encoding
  -drm = enable digital rights management
  -smpte = add SMPTE time code to video stream
  -d <TIME> = specify the maximum duration in seconds of the output file
  -attrib <NUMBER> <NAME> <TYPE> <VALUE> = add attribute to the specified stream
      NUMBER = stream number for attribute: 0 - 63, 0 - all streams
      NAME = name of the attribute
      TYPE = data type of the attribute: string/qword/word/dword/binary/bool
      VALUE = value of the attribute
  -pause = wait for keypress when done

  REMARK: only AVI files with uncompressed streams are accepted

To build the sample, open the project file UncompAVIToWMV.sln in Visual Studio and build 
the project.

Important methods used in this sample:
    - IWMInputMediaProps::SetMediaType()
    - IWMWriter::GetInputProps()
    - IWMWriter::SetInputProps()
    - IWMWriter::BeginWriting()
    - IWMWriter::EndWriting()
    - IWMWriter::AllocateSample()
    - IWMWriter::WriteSample()
    - IWMWriterPreprocess::PreprocessSample()
    - IWMWriterPreprocess::BeginPreprocessingPass()
    - IWMWriterPreprocess::EndPreprocessingPass()
    - IWMProfileManager::LoadSystemProfile()
    - IWMProfileManager::LoadProfileByData()
    - IWMProfileManager::CreateEmptyProfile()
    - IWMProfileManager::SaveProfile()
    - IWMProfile::RemoveStreamByNumber()
    - IWMProfile::CreateNewStream() 
    - IWMProfile::AddStream()
    - IWMStreamConfig2::AddDataUnitExtension()
    - INSSBuffer::GetBufferAndLength()
    - INSSBuffer3::SetProperty()
    - IWMHeaderInfo::SetAttribute()