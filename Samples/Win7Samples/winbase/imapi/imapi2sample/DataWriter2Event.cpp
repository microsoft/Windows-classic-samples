/* Copyright (c) Microsoft Corporation. All rights reserved. */

#include "DataWriter2Event.h"

STDMETHODIMP_(VOID) CTestDataWriter2Event::Update(IDispatch* objectDispatch, IDispatch* progressDispatch)
{
    //UNREFERENCED_PARAMETER (objectDispatch);
    HRESULT hr = S_OK;

    IDiscFormat2DataEventArgs* progress = NULL;
    IDiscFormat2Data* object = NULL;
    LONG elapsedTime = 0;
    LONG remainingTime = 0;
    LONG totalTime = 0;
    IMAPI_FORMAT2_DATA_WRITE_ACTION currentAction = IMAPI_FORMAT2_DATA_WRITE_ACTION_VALIDATING_MEDIA;
    LONG startLba = 0;
    LONG sectorCount = 0;
    LONG lastReadLba = 0;
    LONG lastWrittenLba = 0;
    LONG totalSystemBuffer = 0;
    LONG usedSystemBuffer = 0;
    LONG freeSystemBuffer = 0;

    hr = progressDispatch->QueryInterface(IID_PPV_ARGS(&progress));
    hr = objectDispatch->QueryInterface(IID_PPV_ARGS(&object));


    if ((SUCCEEDED(progress->get_ElapsedTime(&elapsedTime)            )) &&
        (SUCCEEDED(progress->get_RemainingTime(&remainingTime)        )) &&
        (SUCCEEDED(progress->get_TotalTime(&totalTime)                )) &&
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
        if (currentAction == IMAPI_FORMAT2_DATA_WRITE_ACTION_VALIDATING_MEDIA)
        {
            DeleteCurrentLine();
            printf("Validating media...      ");
        }
        else if (currentAction == IMAPI_FORMAT2_DATA_WRITE_ACTION_FORMATTING_MEDIA)
        {
            DeleteCurrentLine();
            printf("Formatting media...      ");
        }
        else if (currentAction == IMAPI_FORMAT2_DATA_WRITE_ACTION_INITIALIZING_HARDWARE)
        {
            DeleteCurrentLine();
            printf("Initializing hardware... ");
        }
        else if (currentAction == IMAPI_FORMAT2_DATA_WRITE_ACTION_CALIBRATING_POWER)
        {
            DeleteCurrentLine();
            printf("Calibrating power...     ");
        }
        else if (currentAction == IMAPI_FORMAT2_DATA_WRITE_ACTION_WRITING_DATA)
        {
            DeleteCurrentLine();
            printf("[%08x..%08x] ", startLba, startLba + sectorCount);
            UpdatePercentageDisplay(lastWrittenLba - startLba, sectorCount);
        }
        else if (currentAction == IMAPI_FORMAT2_DATA_WRITE_ACTION_FINALIZATION)
        {
            DeleteCurrentLine();
            OverwriteCurrentLine();
            DeleteCurrentLine();
            printf("Finishing writing operation...");
        }
        else if (currentAction == IMAPI_FORMAT2_DATA_WRITE_ACTION_COMPLETED)
        {
            printf("Write has completed!\n");
        }
    }
    return;
}
