// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.


#include <windows.h>
#include <stdio.h>
#include <conio.h>
#include <tchar.h>
#include "ntsecapi.h"
#include "dcomperm.h"


/*---------------------------------------------------------------------------*\
 * NAME: 
 * --------------------------------------------------------------------------*
 * DESCRIPTION: 
\*---------------------------------------------------------------------------*/
void WarnIfGlobalPolicySet()
{

    HKEY  hregPolicy    = NULL;
    DWORD dwType        = 0;
    DWORD cbData        = 0;
    BYTE  bArray[512]   = {0};
    DWORD dwReturnValue = ERROR_SUCCESS;

    if(RegOpenKeyEx(HKEY_LOCAL_MACHINE, 
                    _T("SOFTWARE\\Policies\\Microsoft\\Windows NT\\DCOM"), 
                    0, 
                    KEY_READ, 
                    &hregPolicy) == ERROR_SUCCESS)
    {
        cbData = sizeof(bArray);

        dwReturnValue = RegQueryValueEx(hregPolicy, 
                                        _T("MachineAccessRestriction"), 
                                        0, 
                                        &dwType, 
                                        bArray, 
                                        &cbData);
        
        if(dwReturnValue != ERROR_FILE_NOT_FOUND)
        {

            _tprintf (_T("WARNING: Machine-wide Access ")
                      _T("Group Policy may override these settings! \n"));

        }

        cbData = sizeof(bArray);

        dwReturnValue = RegQueryValueEx(hregPolicy, _T("MachineLaunchRestriction"), 0, &dwType, bArray, &cbData);
        if(dwReturnValue != ERROR_FILE_NOT_FOUND)
        {

            _tprintf (_T("WARNING: Machine-wide Launch/Activate ")
                      _T("Group Policy may override these settings! \n"));

        }

        RegCloseKey(hregPolicy);
    }
}


/*---------------------------------------------------------------------------*\
 * NAME: DisplayAccess                                                       *
 * --------------------------------------------------------------------------*
 * DESCRIPTION: 
\*---------------------------------------------------------------------------*/
void DisplayAccess(
    DWORD dwAccessMask, 
    DWORD dwSDType, 
    LPCTSTR tszAccessType, 
    LPCTSTR tszUser, 
    LPCTSTR tszDomain
    )
{

    BOOL fLegacy = IsLegacySecurityModel();

    if(!fLegacy)
    {
        WarnIfGlobalPolicySet();
    }

    if(dwSDType & SDTYPE_ACCESS)
    {
        BOOL fLocalAccess = (dwAccessMask & COM_RIGHTS_EXECUTE_LOCAL  ) ||
                             ((dwAccessMask & COM_RIGHTS_EXECUTE) && 
                              !(dwAccessMask & COM_RIGHTS_EXECUTE_REMOTE));

        BOOL fRemoteAccess = (dwAccessMask & COM_RIGHTS_EXECUTE_REMOTE  ) ||
                             ((dwAccessMask & COM_RIGHTS_EXECUTE) && 
                              !(dwAccessMask & COM_RIGHTS_EXECUTE_LOCAL));

        if(fLegacy && dwAccessMask & COM_RIGHTS_EXECUTE)
        {
            _tprintf (_T("Access %s to %s\\%s.\n"), 
                      tszAccessType,
                      tszDomain, tszUser);

        }
        else if(!fLegacy && (fLocalAccess || fRemoteAccess))
        {

            _tprintf (_T("%s access %s to %s\\%s.\n"), 
                      fLocalAccess ? 
                      fRemoteAccess ? 
                      _T("Remote and Local") : _T("Local") : _T("Remote") ,
                      tszAccessType,
                      tszDomain, tszUser);
         }
    }
    else
    {

        BOOL fLocalLaunchAccess = (dwAccessMask & COM_RIGHTS_EXECUTE_LOCAL  ) ||
                             ((dwAccessMask & COM_RIGHTS_EXECUTE) && 
                             !(dwAccessMask & (COM_RIGHTS_EXECUTE_REMOTE  |
                                               COM_RIGHTS_ACTIVATE_REMOTE |
                                               COM_RIGHTS_ACTIVATE_LOCAL)));

        BOOL fRemoteLaunchAccess = (dwAccessMask & COM_RIGHTS_EXECUTE_REMOTE  ) ||
                             ((dwAccessMask & COM_RIGHTS_EXECUTE) && 
                             !(dwAccessMask & (COM_RIGHTS_EXECUTE_LOCAL   |
                                               COM_RIGHTS_ACTIVATE_REMOTE |
                                               COM_RIGHTS_ACTIVATE_LOCAL)));

        BOOL fLocalActivateAccess = (dwAccessMask & COM_RIGHTS_ACTIVATE_LOCAL) ||
                             ((dwAccessMask & COM_RIGHTS_EXECUTE) && 
                             !(dwAccessMask & (COM_RIGHTS_EXECUTE_LOCAL  |
                                               COM_RIGHTS_EXECUTE_REMOTE |
                                               COM_RIGHTS_ACTIVATE_REMOTE)));

        BOOL fRemoteActivateAccess = (dwAccessMask & COM_RIGHTS_ACTIVATE_REMOTE) ||
                             ((dwAccessMask & COM_RIGHTS_EXECUTE) && 
                             !(dwAccessMask & (COM_RIGHTS_EXECUTE_LOCAL  |
                                               COM_RIGHTS_EXECUTE_REMOTE |
                                               COM_RIGHTS_ACTIVATE_LOCAL)));

        if(fLegacy && dwAccessMask & COM_RIGHTS_EXECUTE)
        {
            _tprintf (_T("Launch %s to %s\\%s.\n"), 
                      tszAccessType,
                      tszDomain, tszUser);

        }
        else 
        {
            if(!fLegacy && (fLocalLaunchAccess || fRemoteLaunchAccess))
            {

                _tprintf (_T("%s launch %s to %s\\%s.\n"), 
                          fLocalLaunchAccess ? 
                          fRemoteLaunchAccess ? 
                          _T("Remote and Local") : _T("Local") : _T("Remote") ,
                          tszAccessType,
                          tszDomain, tszUser);
             }
             if(!fLegacy && (fLocalActivateAccess || fRemoteActivateAccess))
             {

                _tprintf (_T("%s activation %s to %s\\%s.\n"), 
                          fLocalActivateAccess ? 
                          fRemoteActivateAccess ? 
                          _T("Remote and Local") : _T("Local") : _T("Remote"),
                          tszAccessType,
                          tszDomain, tszUser);
             }

         }

    }

}

/*---------------------------------------------------------------------------*\
 * NAME: ListACL                                                             *
 * --------------------------------------------------------------------------*
 * DESCRIPTION: 
\*---------------------------------------------------------------------------*/
void ListACL (
    PACL Acl,
    DWORD dwSDType
    )
{
    ACL_SIZE_INFORMATION     aclSizeInfo                      = {0};
    ACL_REVISION_INFORMATION aclRevInfo                       = {0};
    ULONG                    i;
    LPVOID                   pvAce                            = NULL;
    ACE_HEADER               *pAceHeader                      = NULL;
    ACCESS_ALLOWED_ACE       *pAccessAllowedAce               = NULL;
    ACCESS_DENIED_ACE        *pAccessDeniedAce                = NULL;
    TCHAR                    tszDomainName [SIZE_NAME_BUFFER] = {0};
    TCHAR                    tszUserName [SIZE_NAME_BUFFER]   = {0};
    DWORD                    cchName = 0;
    SID_NAME_USE             snu;

    if (!GetAclInformation (Acl,
                            &aclSizeInfo,
                            sizeof (ACL_SIZE_INFORMATION),
                            AclSizeInformation))
    {
        _tprintf (_T("Could not get AclSizeInformation"));
        return;
    }

    if (!GetAclInformation (Acl,
                            &aclRevInfo,
                            sizeof (ACL_REVISION_INFORMATION),
                            AclRevisionInformation))
    {
        _tprintf (_T("Could not get AclRevisionInformation"));
        return;
    }

    for (i = 0; i < aclSizeInfo.AceCount; i++)
    {
        if (!GetAce (Acl, i, &pvAce))
            return;

        pAceHeader = (ACE_HEADER *) pvAce;

        if (pAceHeader->AceType == ACCESS_ALLOWED_ACE_TYPE)
        {
            pAccessAllowedAce = (ACCESS_ALLOWED_ACE *) pvAce;
            cchName = SIZE_NAME_BUFFER-1;
            LookupAccountSid (NULL,
                              &pAccessAllowedAce->SidStart,
                              tszUserName,
                              &cchName,
                              tszDomainName,
                              &cchName,
                              &snu);


            DisplayAccess(pAccessAllowedAce->Mask, 
                          dwSDType, 
                          _T("permitted"), 
                          tszUserName, 
                          tszDomainName);

        } 
        else if (pAceHeader->AceType == ACCESS_DENIED_ACE_TYPE)
        {
            pAccessDeniedAce = (ACCESS_DENIED_ACE *) pvAce;
            cchName = SIZE_NAME_BUFFER-1;
            LookupAccountSid (NULL,
                              &pAccessDeniedAce->SidStart,
                              tszUserName,
                              &cchName,
                              tszDomainName,
                              &cchName,
                              &snu);

            DisplayAccess(pAccessDeniedAce->Mask, 
                          dwSDType, 
                          _T("denied"), 
                          tszUserName, 
                          tszDomainName);
  
        }
   }
}
