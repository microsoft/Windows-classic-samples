/*----------------------------------------------------------------------------
THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED
TO THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
PARTICULAR PURPOSE.

Copyright (C) 1999 - 2000.  Microsoft Corporation.  All rights reserved.


PrintMenu.h

Description:
Prototypes for functions used to print out the menus

----------------------------------------------------------------------------*/

#if !defined (_PRINT_MENU_H_)
#define _PRINT_MENU_H_

WCHAR PrintMainMenu();
WCHAR PrintDefaultQuotaMenu();
WCHAR PrintQuotaLogFlagMenu();
WCHAR PrintDefaultLimitsMenu();
WCHAR PrintUserManagerMenu();
WCHAR PrintQuotaManagerMenu();

#endif /* _PRINT_MENU_H_ */