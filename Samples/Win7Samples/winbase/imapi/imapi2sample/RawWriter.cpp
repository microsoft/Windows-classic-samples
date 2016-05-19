//
// Simple raw creator test
// 
// Burns a.pcm, b.pcm, c.pcm to recorder #2 (imapi index)
// 
HRESULT SimpleTestPass()
{
    HRESULT hr = S_OK;

    ::ATL::CComPtr<IRawCDImageCreator> raw;
    ::ATL::CComPtr<IStream> audioStreams[3];
    ::ATL::CComPtr<IStream> resultStream;

    // create a raw image creator
    EXPECT_SUCCESS(raw.CoCreateInstance(CLSID_MsftRawCDImageCreator));

    // create streams for the 3 audio files
    EXPECT_SUCCESS(SHCreateStreamOnFileW(L"a.pcm", STGM_READ | STGM_SHARE_DENY_WRITE, &(audioStreams[0])));
    EXPECT_SUCCESS(SHCreateStreamOnFileW(L"b.pcm", STGM_READ | STGM_SHARE_DENY_WRITE, &(audioStreams[1])));
    EXPECT_SUCCESS(SHCreateStreamOnFileW(L"c.pcm", STGM_READ | STGM_SHARE_DENY_WRITE, &(audioStreams[2])));
    
    // set the image type
    // NOTE: this will change later to put a different mode when it's fully implemented
    EXPECT_SUCCESS(raw->put_ResultingImageType(IMAPI_FORMAT2_RAW_CD_SUBCODE_PQ_ONLY));

    // add the 3 tracks to the disc image
    LONG index = 0; 
    EXPECT_SUCCESS(raw->AddTrack(IMAPI_CD_SECTOR_AUDIO, audioStreams[0], &index));
    EXPECT_SUCCESS(raw->AddTrack(IMAPI_CD_SECTOR_AUDIO, audioStreams[1], &index));
    EXPECT_SUCCESS(raw->AddTrack(IMAPI_CD_SECTOR_AUDIO, audioStreams[2], &index));

    // create the disc image    
    EXPECT_SUCCESS(raw->CreateResultImage(&resultStream));

    //
    // Burn the image to disc
    //
    ATL::CComPtr<IDiscMaster2> iDiscMaster;
    ATL::CComPtr<IDiscRecorder2> iDiscRecorder;
    ATL::CComPtr<IDiscFormat2RawCD> iDiscFormatRaw;
    ULONG recorderNumber = 2;
    BSTR recorderId;
    BSTR clientName = ::SysAllocString(L"RawTestBurn");
    
    // cocreate all burning classes
    EXPECT_SUCCESS(iDiscMaster.CoCreateInstance(CLSID_MsftDiscMaster2));
    EXPECT_SUCCESS(iDiscRecorder.CoCreateInstance(CLSID_MsftDiscRecorder2));
    EXPECT_SUCCESS(iDiscFormatRaw.CoCreateInstance(CLSID_MsftDiscFormat2RawCD));

    // get and initialize disc recorder
    EXPECT_SUCCESS(iDiscMaster->get_Item(recorderNumber, &recorderId));
    EXPECT_SUCCESS(iDiscRecorder->InitializeDiscRecorder(recorderId));

    // attach disc recorder and disc format
    EXPECT_SUCCESS(iDiscFormatRaw->put_Recorder(iDiscRecorder));
    EXPECT_SUCCESS(iDiscFormatRaw->put_ClientName(clientName));

    // prepare media
    EXPECT_SUCCESS(iDiscFormatRaw->PrepareMedia());

    // set up options on disc format
    // NOTE: this will change later to put a different mode when it's fully implemented
    EXPECT_SUCCESS(iDiscFormatRaw->put_RequestedSectorType(IMAPI_FORMAT2_RAW_CD_SUBCODE_PQ_ONLY));

    // burn stream
    EXPECT_SUCCESS(iDiscFormatRaw->WriteMedia(resultStream));

    // release media (even if the burn failed)
    hr = iDiscFormatRaw->ReleaseMedia();

    return hr;
}