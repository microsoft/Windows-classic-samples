/* Copyright (c) Microsoft Corporation. All rights reserved. */

#include "RawWriter2Event.h"

// DDiscFormat2RawCDEvents methods
STDMETHODIMP_(VOID) CTestRawWriter2Event::Update(IDispatch* objectDispatch, IDispatch* progressDispatch)
{
    UNREFERENCED_PARAMETER(objectDispatch);
    HRESULT hr = S_OK;
    LONG elapsedTime = 0;
    LONG remainingTime = 0;
    LONG totalTime = 0;
    IMAPI_FORMAT2_RAW_CD_WRITE_ACTION currentAction = IMAPI_FORMAT2_RAW_CD_WRITE_ACTION_UNKNOWN;
    LONG startLba = 0;
    LONG sectorCount = 0;
    LONG lastReadLba = 0;
    LONG lastWrittenLba = 0;
    LONG totalSystemBuffer = 0;
    LONG usedSystemBuffer = 0;
    LONG freeSystemBuffer = 0;
    IDiscFormat2RawCDEventArgs* progress = NULL;

    if ((SUCCEEDED(progressDispatch->QueryInterface(IID_PPV_ARGS(&progress)))) &&
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
        totalTime = elapsedTime + remainingTime;

        if (currentAction == IMAPI_FORMAT2_RAW_CD_WRITE_ACTION_PREPARING)
        {
            DeleteCurrentLine();
            printf("Preparing hardware and media...      ");
        }
        else if (currentAction == IMAPI_FORMAT2_RAW_CD_WRITE_ACTION_WRITING)
        {
            DeleteCurrentLine();
            printf("[%08x..%08x] ", startLba, startLba + sectorCount);
            UpdatePercentageDisplay(lastWrittenLba - startLba, sectorCount);
        }
        else if (currentAction == IMAPI_FORMAT2_RAW_CD_WRITE_ACTION_FINISHING)
        {
            DeleteCurrentLine();
            OverwriteCurrentLine();
            DeleteCurrentLine();
            printf("Finishing writing operation...");
        }
        //else if (currentAction == IMAPI_FORMAT2_RAW_CD_WRITE_ACTION_COMPLETED)
        //{
        //    printf("Write has completed!\n");
        //}
    }
    return;
}
