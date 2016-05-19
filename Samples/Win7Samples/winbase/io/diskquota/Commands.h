/*----------------------------------------------------------------------------
THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED
TO THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
PARTICULAR PURPOSE.

Copyright (C) 1999 - 2000.  Microsoft Corporation.  All rights reserved.


Commands.h

----------------------------------------------------------------------------*/

#if !defined (_COMMANDS_H_)
#define _COMMANDS_H_

BOOL GetDefaultQuota(IDiskQuotaControl* lpDiskQuotaControl);
BOOL GetQuotaLogFlags(IDiskQuotaControl* lpDiskQuotaControl);
BOOL GetDefaultHardLimit(IDiskQuotaControl* lpDiskQuotaControl);
BOOL GetDefaultThreshold(IDiskQuotaControl* lpDiskQuotaControl);
BOOL SetDefaultHardLimit(IDiskQuotaControl* lpDiskQuotaControl);
BOOL SetDefaultThreshold(IDiskQuotaControl* lpDiskQuotaControl);
BOOL SetUserHardLimit(IDiskQuotaControl* lpDiskQuotaControl);
BOOL SetUserThreshold(IDiskQuotaControl* lpDiskQuotaControl);
BOOL GetUserQuotaInfo(IDiskQuotaControl* lpDiskQuotaControl);
BOOL AddUser(IDiskQuotaControl* lpDiskQuotaControl);
BOOL DeleteUser(IDiskQuotaControl* lpDiskQuotaControl);
BOOL EnumerateUsers(IDiskQuotaControl* lpDiskQuotaControl);
BOOL EnumerateUserQuotas(IDiskQuotaControl* lpDiskQuotaControl);

void PrintError(HRESULT hr);
void LfcrToNull(LPWSTR szString);

#endif /* _COMMANDS_H_ */