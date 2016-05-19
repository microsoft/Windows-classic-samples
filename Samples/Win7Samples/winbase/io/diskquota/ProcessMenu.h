/*----------------------------------------------------------------------------
THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED
TO THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
PARTICULAR PURPOSE.

Copyright (C) 1999 - 2000.  Microsoft Corporation.  All rights reserved.


ProcessMenu.h

Description:
Prototypes for functions used to process menu selections
----------------------------------------------------------------------------*/

#if !defined (_PROCESS_MENU_H_)
#define _PROCESS_MENU_H_

BOOL ProcessMainMenu(WCHAR wcMenuChoice, IDiskQuotaControl* lpDiskQuotaControl);
BOOL ProcessDefaultQuotaMenu(WCHAR wcMenuChoice, IDiskQuotaControl* lpDiskQuotaControl);
BOOL ProcessQuotaLogFlagMenu(WCHAR wcMenuChoice, IDiskQuotaControl* lpDiskQuotaControl);
BOOL ProcessDefaultLimitsMenu(WCHAR wcMenuChoice, IDiskQuotaControl* lpDiskQuotaControl);
BOOL ProcessUserManagerMenu(WCHAR wcMenuChoice, IDiskQuotaControl* lpDiskQuotaControl);
BOOL ProcessQuotaManagerMenu(WCHAR wcMenuChoice, IDiskQuotaControl* lpDiskQuotaControl);

#endif /* _PROCESS_MENU_H_ */