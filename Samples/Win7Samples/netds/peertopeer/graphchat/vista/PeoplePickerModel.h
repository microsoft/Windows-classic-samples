/********************************************************************++
THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED
TO THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
PARTICULAR PURPOSE.

Copyright (c) Microsoft Corporation. All Rights Reserved.

Module Name:

    PeoplePickerModel.h

Abstract:

    This C header file declares functions for use in the
    with the People Picker Dialog sample code.

--********************************************************************/

#pragma once

#include "PeoplePickerDialog.h"

HRESULT InitPeoplePickerModel(HWND hDlg);
HRESULT DuplicatePeerPeopleNearMe(PEER_PEOPLE_NEAR_ME ** ppPersonDestination, PEER_PEOPLE_NEAR_ME * pPersonSource);
void PeoplePickerModelFreePerson(PEER_PEOPLE_NEAR_ME * pPersonNearMe);
void PeoplePickerModelDestroy();
