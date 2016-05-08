/* Copyright (c) Microsoft Corporation. All rights reserved. */

#include "AudioEvent.h"

// ICDWriteEngine2Event methods


STDMETHODIMP_(VOID) CAudioEvent::Update(IDispatch* objectDispatch, IDispatch* progressDispatch)
{
    //UNREFERENCED_PARAMETER(objectDispatch);
    HRESULT hr = S_OK;
    LONG elapsedTime = 0;
    LONG remainingTime = 0;
    IMAPI_FORMAT2_TAO_WRITE_ACTION currentAction = IMAPI_FORMAT2_TAO_WRITE_ACTION_UNKNOWN;
    LONG currentTrack = 0;
    LONG startLba = 0;
    LONG sectorCount = 0;
    LONG lastReadLba = 0;
    LONG lastWrittenLba = 0;
    LONG totalSystemBuffer = 0;
    LONG usedSystemBuffer = 0;
    LONG freeSystemBuffer = 0;
    IDiscFormat2TrackAtOnceEventArgs* progress = NULL;
    IDiscFormat2TrackAtOnce* object = NULL;

    hr = objectDispatch->QueryInterface(IID_PPV_ARGS(&object));

    if (FAILED(hr))
    {
        printf("FAILED to QI in event\n");
        return;
    }

    if ((SUCCEEDED(progressDispatch->QueryInterface(IID_PPV_ARGS(&progress)))) &&
        (SUCCEEDED(progress->get_CurrentTrackNumber(&currentTrack)    )) &&
        (SUCCEEDED(progress->get_ElapsedTime(&elapsedTime)            )) &&
        (SUCCEEDED(progress->get_RemainingTime(&remainingTime)        )) &&
        (SUCCEEDED(progress->get_CurrentAction(&currentAction)        )) &&
        (SUCCEEDED(progress->get_StartLba(&startLba)                  )) &&
        (SUCCEEDED(progress->get_SectorCount(&sectorCount)            )) &&
        (SUCCEEDED(progress->get_LastReadLba(&lastReadLba)            )) &&
        (SUCCEEDED(progress->get_LastWrittenLba(&lastWrittenLba)      )) &&
        (SUCCEEDED(progress->get_TotalSystemBuffer(&totalSystemBuffer))) &&
        (SUCCEEDED(progress->get_UsedSystemBuffer(&usedSystemBuffer)  )) &&
        (SUCCEEDED(progress->get_FreeSystemBuffer(&freeSystemBuffer)  ))
        )
    {
        if (currentAction == IMAPI_FORMAT2_TAO_WRITE_ACTION_PREPARING)
        {
            DeleteCurrentLine();
            printf("Preparing ...      ");
        }
        else if (currentAction == IMAPI_FORMAT2_TAO_WRITE_ACTION_FINISHING)
        {
            DeleteCurrentLine();
            printf("Finishing ...      ");
        }
        else if (currentAction == IMAPI_FORMAT2_TAO_WRITE_ACTION_WRITING)
        {
            DeleteCurrentLine();
            printf("T %02d [%08x..%08x] ", currentTrack, startLba, startLba + sectorCount);
            UpdatePercentageDisplay(lastWrittenLba - startLba, sectorCount);            
        }
    }

    ReleaseAndNull(progress);
    ReleaseAndNull(object);
    return; // hr;
}
