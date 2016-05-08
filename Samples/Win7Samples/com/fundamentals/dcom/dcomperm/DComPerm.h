// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.


#define GUIDSTR_MAX 38

#ifndef STR2UNI

#define STR2UNI(unistr, regstr) \
        mbstowcs_s (NULL, unistr, RTL_NUMBER_OF(unistr), regstr, strlen (regstr)+1);

#define UNI2STR(regstr, unistr) \
        wcstombs_s (NULL, regstr, RTL_NUMBER_OF(regstr), unistr, wcslen (unistr)+1);

#endif

#define ACE_TYPE_ALL ((DWORD)-1L)

#define SIZE_NAME_BUFFER 256
#define SIZE_MSG_BUFFER 256

#define SDTYPE_MACHINE_LAUNCH     (0x1L)
#define SDTYPE_MACHINE_ACCESS     (0x2L) 
#define SDTYPE_DEFAULT_LAUNCH     (0x4L) 
#define SDTYPE_DEFAULT_ACCESS     (0x8L)
#define SDTYPE_APPLICATION_LAUNCH (0x10L)
#define SDTYPE_APPLICATION_ACCESS (0x20L)

#define SDTYPE_ACCESS (SDTYPE_MACHINE_ACCESS|SDTYPE_DEFAULT_ACCESS|SDTYPE_APPLICATION_ACCESS) 

#ifndef COM_RIGHTS_EXECUTE_LOCAL
#define COM_RIGHTS_EXECUTE_LOCAL  0x2
#endif
#ifndef COM_RIGHTS_EXECUTE_REMOTE
#define COM_RIGHTS_EXECUTE_REMOTE 0x4
#endif
#ifndef COM_RIGHTS_ACTIVATE_LOCAL
#define COM_RIGHTS_ACTIVATE_LOCAL 0x8
#endif
#ifndef COM_RIGHTS_ACTIVATE_REMOTE
#define COM_RIGHTS_ACTIVATE_REMOTE 0x10
#endif


//
// Wrappers
//
DWORD
ListMachineAccessACL();

DWORD
ListMachineLaunchACL();

DWORD
ListDefaultAccessACL();

DWORD
ListDefaultLaunchACL();

DWORD
ListAppIDAccessACL (
    LPTSTR AppID
    );

DWORD
ListAppIDLaunchACL (
    LPTSTR AppID
    );

DWORD
ChangeMachineAccessACL (
    LPTSTR tszPrincipal,
    BOOL fSetPrincipal,
    BOOL fPermit,
    DWORD dwAccessMask
    );

DWORD
ChangeMachineLaunchAndActivateACL (
    LPTSTR tszPrincipal,
    BOOL fSetPrincipal,
    BOOL fPermit,
    DWORD dwAccessMask
    );

DWORD
ChangeDefaultAccessACL (
    LPTSTR Principal,
    BOOL SetPrincipal,
    BOOL Permit,
    DWORD dwAccessMask
    );

DWORD
ChangeDefaultLaunchAndActivateACL (
    LPTSTR Principal,
    BOOL SetPrincipal,
    BOOL Permit,
    DWORD dwAccessMask
    );

DWORD
ChangeAppIDAccessACL (
    LPTSTR AppID,
    LPTSTR Principal,
    BOOL SetPrincipal,
    BOOL Permit,
    DWORD dwAccessMask    
    );

DWORD
ChangeAppIDLaunchAndActivateACL (
    LPTSTR AppID,
    LPTSTR Principal,
    BOOL SetPrincipal,
    BOOL Permit,
    DWORD dwAccessMask
    );

DWORD GetRunAsPassword (
    LPTSTR AppID,
    LPTSTR Password
    );

DWORD SetRunAsPassword (
    LPTSTR AppID,
    LPTSTR Principal,
    LPTSTR Password
    );

DWORD GetRunAsPassword (
    LPTSTR AppID,
    LPTSTR Password
    );

DWORD SetRunAsPassword (
    LPTSTR AppID,
    LPTSTR Password
    );

//
// Internal functions
//

DWORD
CreateNewSD (
    SECURITY_DESCRIPTOR **SD
    );

DWORD
SetAclDefaults(
    PACL pDacl, 
    DWORD dwSDType
    );

DWORD
MakeSDAbsolute (
    PSECURITY_DESCRIPTOR OldSD,
    PSECURITY_DESCRIPTOR *NewSD
    );

DWORD
SetNamedValueSD (
    HKEY RootKey,
    LPTSTR KeyName,
    LPTSTR ValueName,
    SECURITY_DESCRIPTOR *SD
    );

DWORD
GetNamedValueSD (
    HKEY RootKey,
    LPTSTR KeyName,
    LPTSTR ValueName,
    SECURITY_DESCRIPTOR **SD,
    BOOL *NewSD
    );

DWORD
ListNamedValueSD (
    HKEY hkeyRoot,
    LPTSTR tszKeyName,
    LPTSTR tszValueName,
    DWORD dwSDType
    );

DWORD
AddPrincipalToNamedValueSD (
    HKEY RootKey,
    LPTSTR KeyName,
    LPTSTR ValueName,
    LPTSTR Principal,
    BOOL Permit,
    DWORD dwAccessMask,
    DWORD dwSDType
    );

DWORD
UpdatePrincipalInNamedValueSD (
    HKEY hkeyRoot,
    LPTSTR tszKeyName,
    LPTSTR tszValueName,
    LPTSTR tszPrincipal,
    DWORD dwAccessMask,
    BOOL fRemove,
    DWORD fAceType
    );

DWORD
RemovePrincipalFromNamedValueSD (
    HKEY RootKey,
    LPTSTR KeyName,
    LPTSTR ValueName,
    LPTSTR Principal,
    DWORD fAceType
    );

BOOL
IsLegacySecurityModel ();

DWORD
GetCurrentUserSID (
    PSID *Sid
    );

DWORD
GetPrincipalSID (
    LPTSTR Principal,
    PSID *Sid
    );

DWORD
CopyACL (
    PACL paclOld,
    PACL paclNew
    );

DWORD
AddAccessDeniedACEToACL (
    PACL *paclOrig,
    DWORD dwPermissionMask,
    LPTSTR tszPrincipal
    );

DWORD
AddAccessAllowedACEToACL (
    PACL *paclOrig,
    DWORD dwAccessMask,
    LPTSTR tszPrincipal
    );

DWORD
UpdatePrincipalInACL (
    PACL paclOrig,
    LPTSTR tszPrincipal,
    DWORD dwAccessMask,
    BOOL fRemove,
    DWORD fAceType
    );

DWORD
RemovePrincipalFromACL (
    PACL paclOrig,
    LPTSTR tszPrincipal,
    DWORD fAceType
    );

void
ListACL (
    PACL Acl,
    DWORD dwSDType
    );

DWORD
SetAccountRights (
    LPTSTR User,
    LPTSTR Privilege
    );

//
// Utility Functions
//

LPTSTR
SystemMessage (
    LPTSTR szBuffer,
    DWORD cbBuffer,
    HRESULT hr
    );

