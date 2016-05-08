//==========================================================================
//
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//
//--------------------------------------------------------------------------


#include "FaxSecurity.h"
#include <faxcomex_i.c>

//+---------------------------------------------------------------------------
//
//  function:   GiveUsage
//
//  Synopsis:   prints the usage of the application
//
//  Arguments:  [AppName] - Name of the application whose usage has to be printed
//
//  Returns:    void
//----------------------------------------------------------------------------
void GiveUsage(LPTSTR AppName)
{
        _tprintf( TEXT("Usage : %s \n \
                       /s Fax Server Name \n \
                       /o print/add/delete \n \
                       /a AccountName E.g. testmachine\\administrator \n \
                       /d Deny ACE can be 0 or 1. If 1 then Deny \n \
                       /m AccessMask For permissible values, refer to ReadMe.txt "),AppName);
        _tprintf( TEXT("Usage : %s /? -- this message\n"),AppName);
}

//+---------------------------------------------------------------------------
//
//  function:   IsOSVersionCompatible
//
//  Synopsis:   finds whether the target OS supports this functionality.
//
//  Arguments:  [dwVersion] - Minimum Version of the OS required for the Sample to run.
//
//  Returns:    bool - true if the Sample can run on this OS
//
//----------------------------------------------------------------------------
bool IsOSVersionCompatible(DWORD dwVersion)
{
        OSVERSIONINFOEX osvi;
        BOOL bOsVersionInfoEx;

        ZeroMemory(&osvi, sizeof(OSVERSIONINFOEX));
        osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);
        bOsVersionInfoEx = GetVersionEx ((OSVERSIONINFO *) &osvi);
        if( !bOsVersionInfoEx  )
        {
                osvi.dwOSVersionInfoSize = sizeof (OSVERSIONINFO);
                if (! GetVersionEx ( (OSVERSIONINFO *) &osvi) ) 
                        return false;
        }
        bOsVersionInfoEx = (osvi.dwMajorVersion >= dwVersion );
        return (bOsVersionInfoEx == TRUE);
}

//+---------------------------------------------------------------------------
//
//  function:   getDWordAccess
//
//  Synopsis:   Finds the Access Mask from the User Input
//
//  Arguments:  [lptstrAccessMask] -  The Access Mask String
//
//  Returns:    DWORD - Access Mask to be used
//
//----------------------------------------------------------------------------
DWORD getDWordAccess(TCHAR* lptstrAccessMask)
{
        _tprintf(_T("Access Mask is %s \n"), lptstrAccessMask);
        DWORD dwAccess =0 ;
        if(_tcsicmp(lptstrAccessMask,L"submit_low") == 0)
                dwAccess = 0x001;
        if(_tcsicmp(lptstrAccessMask,L"submit_normal") == 0)
                dwAccess = 0x002;
        if(_tcsicmp(lptstrAccessMask,L"submit_high") == 0)
                dwAccess = 0x004;
        if(_tcsicmp(lptstrAccessMask,L"query_jobs") == 0)
                dwAccess = 0x008;
        if(_tcsicmp(lptstrAccessMask,L"manage_jobs") == 0)
                dwAccess = 0x010;
        if(_tcsicmp(lptstrAccessMask,L"query_config") == 0)
                dwAccess = 0x020;
        if(_tcsicmp(lptstrAccessMask,L"manage_config") == 0)
                dwAccess = 0x040;
        if(_tcsicmp(lptstrAccessMask,L"query_archives") == 0)
                dwAccess = 0x080;
        if(_tcsicmp(lptstrAccessMask,L"manage_archives") == 0)
                dwAccess = 0x100;
        if(_tcsicmp(lptstrAccessMask,L"manage_receive_folder") == 0)
                dwAccess = 0x0200;
        _tprintf(_T("Access Mask is %s \n"), lptstrAccessMask);
        return dwAccess;    
}

//+---------------------------------------------------------------------------
//
//  function:   GetSDParts
//
//  Synopsis:   Gets the Security Descriptor parts: DACL and stores it in a global variable
//
//  Arguments:  [pFaxSecurity] : FaxSecurity Object
//
//  Returns:    HRESULT - S_OK if successful
//
//----------------------------------------------------------------------------
HRESULT GetSDParts(IFaxSecurity2* pFaxSecurity)
{
        BOOL fFlag;
        BOOL fDaclPresent = FALSE;
        BOOL fDaclDefaulted = TRUE;
        DWORD dwErrorCode = 0;
        VARIANT vDescriptor;
        HRESULT hr = S_OK;
        VariantInit(&vDescriptor);

        _tprintf(_T("Entering  GetSDParts() \n"));
        //Get the security descriptor from FaxSecurity
        hr = pFaxSecurity->put_InformationType(DACL_SECURITY_INFORMATION);
        if(FAILED(hr))
        {
                _tprintf(_T("put_InformationType failed. Error %x \n"), hr);
                goto Exit;    
        }
        hr = pFaxSecurity->get_Descriptor(&vDescriptor);    
        if(FAILED(hr))
        {
                _tprintf(_T("get_Descriptor failed. Error %x \n"), hr);
                goto Exit;    
        }
        hr = SafeArrayAccessData(vDescriptor.parray, (void **) &g_pSecurityDescriptor);
        if(FAILED(hr))
        {
                _tprintf(_T("SafeArrayAccessData failed. Error %x \n"), hr);
                goto Exit;    
        }
        //Get the DACL
        fFlag = GetSecurityDescriptorDacl(g_pSecurityDescriptor,&fDaclPresent, &g_pDACL, &fDaclDefaulted);
        if(fFlag == FALSE)
        {
                dwErrorCode = GetLastError();       
                hr = HRESULT_FROM_WIN32(dwErrorCode);
                _tprintf(_T("GetSecurityDescriptorDacl failed. Error %x \n"), hr);        
                goto Exit;    
        }
        if (fDaclPresent == FALSE || g_pDACL == NULL)
        {
                _tprintf(_T("No DACL was found (all access is denied), or a NULL DACL (unrestricted access) was found.\n"));
                hr = S_OK;
                goto Exit;
        }
        _tprintf(_T("DACL Found \n"));
        hr = S_OK;
Exit:
        VariantClear(&vDescriptor);
        return hr;
}

//+---------------------------------------------------------------------------
//
//  function:   PrintACEs
//
//  Synopsis:   Print the ACEs present in the DACL
//
//  Arguments:  None
//
//  Returns:    HRESULT - S_OK if successful
//
//----------------------------------------------------------------------------
HRESULT PrintACEs()
{
        HRESULT hr = S_OK;
        BYTE bSIDBuffer[SD_LENGTH1] = {0};
        PSID pSID = &bSIDBuffer;
        DWORD dwDomainBufferSize = SD_LENGTH1;
        LPTSTR lptstrDomainName = NULL;
        DWORD dwAccNameBufferSize = SD_LENGTH1;
        LPTSTR lptstrAccName = NULL;
        SID_NAME_USE eSidType;
        ACL_SIZE_INFORMATION aclSizeInfo;
        DWORD dwErrorCode = 0;
        ACCESS_ALLOWED_ACE* pAce = NULL;    

        _tprintf(_T("Entering PrintACEs ... \n"));
        //Get ACL info    
        BOOL fFlag = GetAclInformation(g_pDACL, &aclSizeInfo, sizeof(aclSizeInfo),AclSizeInformation);            
        if(fFlag == FALSE)
        {
                dwErrorCode = GetLastError();            
                hr = HRESULT_FROM_WIN32(dwErrorCode);
                _tprintf(_T("GetAclInformation failed. hr = %x \n"), hr);        
                goto Exit;
        }

        // Iterate thru the ACE list
        _tprintf(_T("\n\nACE Count %d \n"), aclSizeInfo.AceCount);
        for (UINT cAce = 0; cAce < aclSizeInfo.AceCount; cAce++)
        {
                if (!GetAce(g_pDACL, cAce, (LPVOID*)&pAce))
                {
                        dwErrorCode = GetLastError();
                        hr =HRESULT_FROM_WIN32(dwErrorCode);
                        _tprintf(_T("GetAce failed. hr = %x \n"), hr);
                        goto Exit;        
                }
                dwAccNameBufferSize = SD_LENGTH1;
                dwDomainBufferSize = SD_LENGTH1;
                lptstrDomainName = new TCHAR[dwDomainBufferSize];
                if (lptstrDomainName == NULL)
                {
                        hr = E_OUTOFMEMORY;
                        _tprintf(_T("malloc for lptstrDomainName failed. hr = %x \n"), hr);
                        goto Exit;
                }
                memset(lptstrDomainName, 0, dwDomainBufferSize*sizeof(TCHAR));

                lptstrAccName = new TCHAR[dwAccNameBufferSize];
                if (lptstrAccName == NULL)
                {
                        hr = E_OUTOFMEMORY;
                        _tprintf(_T("malloc for lptstrAccName failed. hr = %x \n"), hr);
                        goto Exit;
                }
                memset(lptstrAccName, 0, dwAccNameBufferSize*sizeof(TCHAR));
                //Get the AccountNAme from the SID
                if(!LookupAccountSid(NULL, &pAce->SidStart, lptstrAccName, &dwAccNameBufferSize, lptstrDomainName, &dwDomainBufferSize, &eSidType))
                {
                        LPTSTR szSID = NULL;
                        // Convert SID to string.
                        ConvertSidToStringSid(&pAce->SidStart, &szSID);
                        _tprintf(_T("\n Couldn't Resolve to account Name. Printing Account SID %s \n"), szSID);
                        LocalFree(szSID);                        
                }   
                else
                {
                        //Print the ACE info        
                        _tprintf(_T("\n\nAccount Name %s \n"), lptstrAccName);
                }
                if( pAce->Header.AceType == ACCESS_DENIED_ACE_TYPE)
                {
                        _tprintf(_T("Access Denied Ace \n"));
                }
                if( pAce->Header.AceType == ACCESS_ALLOWED_ACE_TYPE)
                {
                        _tprintf(_T("Access Allowed Ace \n"));
                }
                _tprintf(_T("Access Mask 0x%x \n"), pAce->Mask);        
                delete lptstrAccName;
                lptstrAccName = NULL;
                delete lptstrDomainName;
                lptstrDomainName = NULL;
        }
Exit:
        _tprintf(_T("\n\n"), pAce->Mask);        
        if(lptstrAccName)
                delete lptstrAccName;
        if(lptstrDomainName)
                delete lptstrDomainName;
        return hr;
}

//+---------------------------------------------------------------------------
//
//  function:   IsAcePresent
//
//  Synopsis:   Check for Duplicate Ace
//
//  Arguments:  None
//
//  Returns:    BOOL - true if duplicate is found
//
//----------------------------------------------------------------------------
BOOL IsAcePresent(PSID pSID, BOOL bDeny, DWORD dwAccessMask, UINT iAceCount)
{
        HRESULT hr = S_OK;
        DWORD dwErrorCode =0;
        ACCESS_ALLOWED_ACE* pAce = NULL;
        //Iterate through the Ace List
        _tprintf(_T("Entering IsAcePresent ...\n"));
        for (UINT cAce = 0; cAce < iAceCount; cAce++)
        {
                if (!GetAce(g_pDACL, cAce, (LPVOID*)&pAce))
                {
                        _tprintf(_T("GetAce failed. GetLastError returned: %d\n"), dwErrorCode);
                        dwErrorCode = GetLastError();
                        hr =HRESULT_FROM_WIN32(dwErrorCode);
                        goto Exit;        
                }

                if (EqualSid((PSID)&pAce->SidStart, pSID))
                {
                        if(pAce->Mask & dwAccessMask)
                        {
                                if((bDeny && (pAce->Header.AceType == ACCESS_DENIED_ACE_TYPE))|| (!bDeny && (pAce->Header.AceType == ACCESS_ALLOWED_ACE_TYPE)))
                                {
                                        //found one
                                        _tprintf(_T("Found Duplicate ACE \n"));
                                        return TRUE;
                                }
                        }
                }
        }
Exit:
        return FALSE;
}

//+---------------------------------------------------------------------------
//
//  function:   PrintACEsFax
//
//  Synopsis:   Prints the Fax ACEs
//
//  Arguments:  [pFaxSecurity] : FaxSecurity Object
//
//  Returns:    BOOL - true if successful
//
//----------------------------------------------------------------------------
bool PrintACEsFax(IFaxSecurity2* pFaxSecurity) 
{
        bool bRetVal = false;
        //Get the SD Parts and from the DACL print the ACEs
        HRESULT hr = S_OK;
        hr = GetSDParts(pFaxSecurity);
        if(FAILED(hr))
        {
                _tprintf(_T("GetSDParts failed. Error %x \n"), hr);
                bRetVal = false;
                goto Exit;
        }
        hr = PrintACEs();
        if(FAILED(hr))
        {
                _tprintf(_T("PrintACEs failed. Error %x \n"), hr);
                bRetVal = false;
                goto Exit;            
        }
        bRetVal = true;
Exit:
        return bRetVal;
}

//+---------------------------------------------------------------------------
//
//  function:   DelAce
//
//  Synopsis:   Delete the ACE from the DACL
//
//  Arguments:  [bDeny] - is it a Deny Ace?
//                [lptstrAccName] : Account from where to delete the ACE
//                [dwAccessMask] : The Access Mask to delete
//
//  Returns:    HRESULT S_OK if successful
//----------------------------------------------------------------------------
HRESULT DelAce(BOOL bDeny, LPTSTR lptstrAccName, DWORD dwAccessMask)
{
        HRESULT hr = S_OK;
        BYTE bSIDBuffer[SD_LENGTH1] = {0};
        PSID pSID = &bSIDBuffer;
        DWORD dwSIDBufferSize = SD_LENGTH1;
        DWORD dwDomainBufferSize = SD_LENGTH1;
        LPTSTR lptstrDomainName = NULL;
        SID_NAME_USE eSidType;
        ACL_SIZE_INFORMATION aclSizeInfo;
        DWORD dwErrorCode = 0;
        ACCESS_ALLOWED_ACE* pAce = NULL;    
        bool bFound = false;

        _tprintf(_T("Entering DelAce ...\n"));
        //Get Acl Info
        BOOL fFlag = GetAclInformation(g_pDACL, &aclSizeInfo, sizeof(aclSizeInfo),AclSizeInformation);            
        if(fFlag == FALSE)
        {
                dwErrorCode = GetLastError();
                hr = HRESULT_FROM_WIN32(dwErrorCode);
                _tprintf(_T("GetAclInformation failed. hr = %x \n"), hr);
                goto Exit;
        }

        lptstrDomainName = new TCHAR[dwDomainBufferSize];
        if (lptstrDomainName == NULL)
        {
                hr = E_OUTOFMEMORY;
                _tprintf(_T("malloc for lptstrDomainName failed. hr = %x \n"), hr);
                goto Exit;
        }
        memset(lptstrDomainName, 0, dwDomainBufferSize*sizeof(TCHAR));
        //Get SID from AccountName
        if(!LookupAccountName(NULL, lptstrAccName, pSID, &dwSIDBufferSize, lptstrDomainName, &dwDomainBufferSize, &eSidType))
        {
                dwErrorCode = GetLastError();
                hr = HRESULT_FROM_WIN32(dwErrorCode);        
                _tprintf(_T("LookupAccountName failed for Account name %s\\%s. hr = %x \n"), lptstrDomainName, lptstrAccName, hr);        
                goto Exit;
        }
        //Is valid SID?
        if (IsValidSid(pSID) == FALSE)
        {
                _tprintf(_T("The SID for %s is invalid. \n"),lptstrAccName);
                hr = MQ_ERROR;
                goto Exit;
        }

        // Iterate thru the ACE list to get the correct ACE
        for (UINT cAce = 0; cAce < aclSizeInfo.AceCount; cAce++)
        {
                if (!GetAce(g_pDACL, cAce, (LPVOID*)&pAce))
                {
                        dwErrorCode = GetLastError();
                        hr =HRESULT_FROM_WIN32(dwErrorCode);
                        _tprintf(_T("GetAce failed. hr = %x \n"), hr);        
                        goto Exit;        
                }
                //Compare the ACE SID with the account SID
                if (EqualSid((PSID)&pAce->SidStart, pSID))
                {
                        //Match the Access Mask
                        if(pAce->Mask & dwAccessMask)
                        {
                                //Compare the ACE type 
                                if((bDeny && (pAce->Header.AceType == ACCESS_DENIED_ACE_TYPE))|| (!bDeny && (pAce->Header.AceType == ACCESS_ALLOWED_ACE_TYPE)))
                                {
                                        _tprintf(_T("Found the Ace. Now Deleting \n"));
                                        //Found the correct ACE. Delete it.
                                        if (DeleteAce(g_pDACL, cAce ) == FALSE)
                                        {
                                                dwErrorCode = GetLastError();                            
                                                hr = HRESULT_FROM_WIN32(dwErrorCode);
                                                _tprintf(_T("DeleteAce failed. hr = %x \n"), hr);        
                                                goto Exit;
                                        }
                                        bFound = true;
                                        break;
                                }
                        }
                }
        }
        if(bFound == false)
                _tprintf(_T("Couldn't find ACE to delete \n"));
Exit:
        if(lptstrDomainName)
                delete lptstrDomainName;
        return hr;
}

//+---------------------------------------------------------------------------
//
//  function:   CreateNewSD
//
//  Synopsis:   Create a NEW SD using the New DACL 
//
//  Arguments:  None
//
//  Returns:    HRESULT S_OK if successful
//----------------------------------------------------------------------------
HRESULT CreateNewSD()
{
        DWORD dwErrorCode = 0;
        HRESULT hr = S_OK;
        PSECURITY_DESCRIPTOR pSDNew = NULL;
        _tprintf(_T("Entering CreateNewSD...\n"));
        pSDNew = new SECURITY_DESCRIPTOR;
        if (pSDNew == NULL)
        {
                hr = E_OUTOFMEMORY;
                _tprintf(_T("new for new Security Descriptor failed. hr = %x \n"), hr);
                goto Exit;
        }
        //Initialize the new SD
        if (!InitializeSecurityDescriptor(pSDNew, SECURITY_DESCRIPTOR_REVISION))
        {
                dwErrorCode = GetLastError();
                hr = HRESULT_FROM_WIN32(dwErrorCode);
                _tprintf(_T("InitializeSecurityDescriptor failed. hr = %x \n"), hr);
                goto Exit;
        }
        //Set the new DACL
        if(!SetSecurityDescriptorDacl(pSDNew,true,g_pDACL,false))
        {
                dwErrorCode = GetLastError();
                hr = HRESULT_FROM_WIN32(dwErrorCode);
                _tprintf(_T("SetSecurityDescriptorDacl failed. hr = %x \n"), hr);
                goto Exit;
        }

        //Is Valid SD?
        if(!IsValidSecurityDescriptor(pSDNew))
        {
                _tprintf(_T("SecurityDescriptor not valid failed. \n "));
                goto Exit;
        }

        SECURITY_DESCRIPTOR_CONTROL sdControl = NULL;
        DWORD dwRevision = NULL;

        //Check if the SD is relative or absolute
        if(!GetSecurityDescriptorControl(pSDNew, &sdControl, &dwRevision))
        {
                dwErrorCode = GetLastError();
                hr = HRESULT_FROM_WIN32(dwErrorCode);
                _tprintf(_T("GetSecurityDescriptorControl failed. hr = %x \n"), hr);
                goto Exit;
        }

        if( sdControl& SE_SELF_RELATIVE )
        {
                _tprintf(_T("Security Descriptor is relative \n"));
                g_pSecurityDescriptorNew = pSDNew;
        }
        else
        {
                //if absolute convert to relative
                _tprintf(_T("Security Descriptor is aboslute. Converting to relative. \n"));
                PSECURITY_DESCRIPTOR    pSDRelative    = g_bSecurityDescriptor;
                g_dwLength    = SD_LENGTH;
                //convert to relative
                MakeSelfRelativeSD(pSDNew, &g_bSecurityDescriptor, &g_dwLength);
                //verify if it is relative?
                GetSecurityDescriptorControl(&g_bSecurityDescriptor, &sdControl, &dwRevision);
                if(sdControl & SE_SELF_RELATIVE )
                {
                        _tprintf(_T("Security Descriptor is relative. \n"));
                }
                else
                {
                        _tprintf(_T("Security Descriptor is absolute.\n"));
                }
                //is valid SD?
                if(!IsValidSecurityDescriptor(pSDRelative))
                {
                        _tprintf(_T("SecurityDescriptor not valid failed. \n"));
                        goto Exit;
                }
                if(pSDNew)
                        delete pSDNew;
                g_pSecurityDescriptorNew = pSDRelative;
        }
Exit:
        if((hr != S_OK) && pSDNew)
                delete pSDNew;      
        return hr;
}

//+---------------------------------------------------------------------------
//
//  function:   CopyRemainingACL
//
//  Synopsis:   Copies the old ACEs from FaxSecurity object to the new SD
//
//  Arguments:  [pDest] : destination PACL
//                [pSrc] :  source PACL
//
//  Returns:    HRESULT S_OK if successful
//+----------------------------------------------------------------------------
HRESULT CopyRemainingACL(PACL pDest, PACL pSrc)
{
        HRESULT hr = S_OK;
        ACL_SIZE_INFORMATION aclSizeInfo;
        LPVOID pAce;    

        _tprintf(_T("Entering CopyRemainingACL ...\n"));
        if (pSrc == NULL)
        {
                _tprintf(_T("Source is NULL \n"));
                return hr;
        }

        if (pDest == NULL)
                return E_POINTER;

        if (!GetAclInformation(pSrc, (LPVOID) &aclSizeInfo, sizeof(ACL_SIZE_INFORMATION), AclSizeInformation))
                return HRESULT_FROM_WIN32(GetLastError());

        // Copy all of the ACEs to the new ACL
        for (UINT i = 0; i < aclSizeInfo.AceCount; i++)
        {
                if (!GetAce(pSrc, i, &pAce))
                {
                        hr = HRESULT_FROM_WIN32(GetLastError());
                        break;
                }

                if (!AddAce(pDest, ACL_REVISION, MAXDWORD, pAce, ((PACE_HEADER)pAce)->AceSize))
                {
                        hr = HRESULT_FROM_WIN32(GetLastError());
                        break;
                }
        }
        return hr;
}


//+---------------------------------------------------------------------------
//
//  function:   DeleteAceFax
//
//  Synopsis:   Delete the ACE from the SD. Calls DelAce to delete from DACL
//
//  Arguments:  [pFaxSecurity] : FaxSecurity Object
//                [bDeny] - is it a Deny Ace?
//                [lptstrAccName] : Account from where to delete the ACE
//                [dwAccessMask] : The Access Mask to delete
//
//  Returns:    BOOL True if successful
//----------------------------------------------------------------------------
bool DeleteAceFax(IFaxSecurity2* pFaxSecurity,BOOL bDeny, LPTSTR lptstrAccName, DWORD dwAccessMask)
{
        //Get the SD parts, delete the ACE and create new DACL and then create SD from it
        HRESULT hr = S_OK;
        bool bRetVal = false;
        _tprintf(_T(" Entering DeleteAceFax ...\n")); 
        hr = GetSDParts(pFaxSecurity);
        if(FAILED(hr))
        {
                _tprintf(_T("GetSDParts failed. Error %x \n"), hr);
                bRetVal = false;
                goto Exit;        
        }    
        hr = DelAce(bDeny, lptstrAccName, dwAccessMask);
        if(FAILED(hr))
        {
                _tprintf(_T("DelAce failed. Error %x \n"), hr);
                bRetVal = false;
                goto Exit;        
        }    
        hr = CreateNewSD();
        if(FAILED(hr))
        {
                _tprintf(_T("CreateNewSD failed. Error %x \n"), hr);
                bRetVal = false;
                goto Exit;        
        }    
        bRetVal = true;
Exit:
        return bRetVal;
}


//+---------------------------------------------------------------------------
//
//  function:   AddDenyAce
//
//  Synopsis:   Add Deny ACE and then copy the old ACEs to the DACL
//
//  Arguments:  [lptstrAccName] : Account from where to delete the ACE
//                [dwAccessMask] : The Access Mask to delete
//
//  Returns:    HRESULT S_OK if successful
//----------------------------------------------------------------------------
HRESULT AddDenyAce(LPTSTR lptstrAccName, DWORD dwAccessMask)
{
        HRESULT hr = S_OK;
        BYTE bSIDBuffer[SD_LENGTH1] = {0};
        PSID pSID = &bSIDBuffer;
        DWORD dwSIDBufferSize = SD_LENGTH1;
        DWORD dwDomainBufferSize = SD_LENGTH1;
        LPTSTR lptstrDomainName = NULL;
        SID_NAME_USE eSidType;
        ACL_SIZE_INFORMATION aclSizeInfo;
        DWORD dwErrorCode = 0;
        PACL pNewACL = NULL;

        _tprintf(_T("Entering AddDenyAce...\n"));
        //Get ACL Info
        BOOL fFlag = GetAclInformation(g_pDACL, &aclSizeInfo, sizeof(aclSizeInfo),AclSizeInformation);            
        if(fFlag == FALSE)
        {
                dwErrorCode = GetLastError();          
                hr = HRESULT_FROM_WIN32(dwErrorCode);
                _tprintf(_T("GetAclInformation failed. hr = %x \n"), hr);
                goto Exit;
        }

        lptstrDomainName = new TCHAR[dwDomainBufferSize];
        if (lptstrDomainName == NULL)
        {
                hr = E_OUTOFMEMORY;
                _tprintf(_T("malloc for lptstrDomainName failed. hr = %x \n"), hr);
                goto Exit;
        }
        memset(lptstrDomainName, 0, dwDomainBufferSize*sizeof(TCHAR));
        //Get the SID for the account name
        if(!LookupAccountName(NULL, lptstrAccName, pSID, &dwSIDBufferSize, lptstrDomainName, &dwDomainBufferSize, &eSidType))
        {
                dwErrorCode = GetLastError();          
                hr = HRESULT_FROM_WIN32(dwErrorCode);
                _tprintf(_T("LookupAccountName failed for Account name %s\\%s. hr = %x \n"), lptstrDomainName, lptstrAccName, hr);                        
                goto Exit;
        }

        if (IsValidSid(pSID) == FALSE)
        {                
                hr = MQ_ERROR;       
                _tprintf(_T("The SID for %s is invalid. hr = %x \n"),lptstrAccName, hr);
                goto Exit;
        }

        //Check Duplicate ACE
        if(IsAcePresent(pSID, true, dwAccessMask,  aclSizeInfo.AceCount))
        {
                hr = S_OK;         
                _tprintf(_T("The ACE is already present. hr = %x \n"), hr);
                goto Exit;
        }
        //Allocate memory for new ACL
        int aclSize = aclSizeInfo.AclBytesInUse + sizeof(ACL) + sizeof(ACCESS_DENIED_ACE) + GetLengthSid(pSID) - sizeof(DWORD);
        pNewACL = (PACL) new BYTE[aclSize];
        if (pNewACL == NULL)
        {
                dwErrorCode = GetLastError();
                hr = HRESULT_FROM_WIN32(dwErrorCode);        
                _tprintf(_T("malloc for new ACL failed. hr = %x \n"), hr);
                goto Exit;
        }        
        if (!InitializeAcl(pNewACL, aclSize, ACL_REVISION))
        {
                dwErrorCode = GetLastError();
                hr = HRESULT_FROM_WIN32(dwErrorCode);
                _tprintf(_T("InitializeAcl failed. hr = %x \n"), hr);
                goto Exit;
        }
        //Add access denied ace
        if(!AddAccessDeniedAce(pNewACL, ACL_REVISION , dwAccessMask, pSID))
        {
                dwErrorCode = GetLastError();
                hr = HRESULT_FROM_WIN32(dwErrorCode);
                _tprintf(_T("AddAccessDeniedAce failed. hr = %x \n"), hr);
                goto Exit;
        }
        //copy the remaining from the old ACL
        hr = CopyRemainingACL(pNewACL, g_pDACL);
        if(hr != S_OK)
        {
                _tprintf(_T("CopyRemainingACL failed. HRESULT returned: %x \n"), hr);
                goto Exit;
        }
        g_pDACL = pNewACL;

Exit:
        if((hr != S_OK) && pNewACL)
                free(pNewACL);
        if(lptstrDomainName)
                delete lptstrDomainName;    
        return hr;


}
//+---------------------------------------------------------------------------
//
//  function:   AddAllowAce
//
//  Synopsis:   Add Allow ACE and then copy the old ACEs to the DACL
//
//  Arguments:  [lptstrAccName] : Account from where to delete the ACE
//                [dwAccessMask] : The Access Mask to delete
//
//  Returns:    HRESULT S_OK if successful
//----------------------------------------------------------------------------
HRESULT AddAllowAce(LPTSTR lptstrAccName, DWORD dwAccessMask)
{
        HRESULT hr = S_OK;
        BYTE bSIDBuffer[SD_LENGTH1] = {0};
        PSID pSID = &bSIDBuffer;
        DWORD dwSIDBufferSize = SD_LENGTH1;
        DWORD dwDomainBufferSize = SD_LENGTH1;
        LPTSTR lptstrDomainName = NULL;
        SID_NAME_USE eSidType;
        ACL_SIZE_INFORMATION aclSizeInfo;
        DWORD dwErrorCode = 0;

        _tprintf(_T("Entering AddAllowAce...\n"));
        //Get Acl Info
        BOOL fFlag = GetAclInformation(g_pDACL, &aclSizeInfo, sizeof(aclSizeInfo),AclSizeInformation);            
        if(fFlag == FALSE)
        {
                dwErrorCode = GetLastError();
                hr = HRESULT_FROM_WIN32(dwErrorCode);
                _tprintf(_T("GetAclInformation failed. hr = %x \n"), hr);
                goto Exit;
        }

        lptstrDomainName = new TCHAR[dwDomainBufferSize];
        if (lptstrDomainName == NULL)
        {
                hr = E_OUTOFMEMORY;
                _tprintf(_T("malloc for lptstrDomainName failed. hr = %x \n"), hr);
                goto Exit;
        }
        memset(lptstrDomainName, 0, dwDomainBufferSize*sizeof(TCHAR));
        //Get the SID from the acc name
        if(!LookupAccountName(NULL, lptstrAccName, pSID, &dwSIDBufferSize, lptstrDomainName, &dwDomainBufferSize, &eSidType))
        {
                dwErrorCode = GetLastError();
                hr = HRESULT_FROM_WIN32(dwErrorCode);     
                _tprintf(_T("LookupAccountName failed for Account name %s\\%s. hr = %x \n"), lptstrDomainName, lptstrAccName, hr);           goto Exit;
        }

        if (IsValidSid(pSID) == FALSE)
        {
                _tprintf(L"The SID for %s is invalid.\n", lptstrAccName);
                hr = MQ_ERROR;
                _tprintf(_T("IsValidSid failed. hr = %x \n"), hr);
                goto Exit;
        }

        //Check Duplicate ACE
        if(IsAcePresent(pSID, false, dwAccessMask,  aclSizeInfo.AceCount))
        {
                hr = S_OK;         
                _tprintf(_T("The ACE is already present. hr = %x \n"), hr);
                goto Exit;
        }
        //allocate memory for new ACL
        PACL pNewACL = NULL;
        int aclSize = aclSizeInfo.AclBytesInUse + sizeof(ACL) + sizeof(ACCESS_ALLOWED_ACE) + GetLengthSid(pSID) - sizeof(DWORD);
        pNewACL = (PACL) new BYTE[aclSize];
        if (pNewACL == NULL)
        {
                hr = E_OUTOFMEMORY;
                _tprintf(_T("malloc for new ACL failed. hr = %x \n"), hr);
                goto Exit;
        }        
        if (!InitializeAcl(pNewACL, aclSize, ACL_REVISION))
        {
                dwErrorCode = GetLastError();

                hr = HRESULT_FROM_WIN32(dwErrorCode);
                _tprintf(_T("InitializeAcl failed. hr = %x \n"), hr);
                goto Exit;
        }
        //add access allowed ace
        if(!AddAccessAllowedAce(pNewACL, ACL_REVISION , dwAccessMask, pSID))
        {
                dwErrorCode = GetLastError();
                hr = HRESULT_FROM_WIN32(dwErrorCode);
                _tprintf(_T("AddAccessAllowedAce failed. hr = %x \n"), hr);
                goto Exit;
        }
        //copy the remaining ACEs from the old ACL
        hr = CopyRemainingACL(pNewACL, g_pDACL);
        if(hr != S_OK)
        {
                _tprintf(_T("CopyRemainingACL failed. hr = %x\n"), hr);
                goto Exit;
        }
        g_pDACL = pNewACL;
Exit:
        if((hr != S_OK) && pNewACL)
                free(pNewACL);
        delete lptstrDomainName;
        return hr;
}

//+---------------------------------------------------------------------------
//
//  function:   AddAllowAceFax
//
//  Synopsis:   Add Allow Ace
//
//  Arguments:  [pFaxSecurity] : FaxSecurity Object
//                [lptstrAccName] : Account from where to delete the ACE
//                [dwAccessMask] : The Access Mask to delete
//
//  Returns:    bool true if successful
//----------------------------------------------------------------------------
bool AddAllowAceFax(IFaxSecurity2* pFaxSecurity, LPTSTR lptstrAccName, DWORD dwAccessMask)
{
        //Get the SD parts, create new DACL and then create SD from it
        HRESULT hr = S_OK;
        bool bRetVal = false;
        _tprintf(_T("Entering AddAllowAceFax ...\n"));
        hr = GetSDParts(pFaxSecurity);
        if(FAILED(hr))
        {
                _tprintf(_T("GetSDParts failed. Error %x \n"), hr);
                bRetVal = false;
                goto Exit;        
        }    
        hr = AddAllowAce(lptstrAccName, dwAccessMask);
        if(FAILED(hr))
        {
                _tprintf(_T("AddAllowAce failed. Error %x \n"), hr);
                bRetVal = false;
                goto Exit;        
        }    
        hr = CreateNewSD();
        if(FAILED(hr))
        {
                _tprintf(_T("CreateNewSD failed. Error %x \n"), hr);
                bRetVal = false;
                goto Exit;        
        }    
        bRetVal = false;
Exit:
        return bRetVal;
}

//+---------------------------------------------------------------------------
//
//  function:   AddDenyAceFax
//
//  Synopsis:   Add Deny Ace
//
//  Arguments:  [pFaxSecurity] : FaxSecurity Object
//                [lptstrAccName] : Account from where to delete the ACE
//                [dwAccessMask] : The Access Mask to delete
//
//  Returns:    bool: true if successful
//----------------------------------------------------------------------------
bool AddDenyAceFax(IFaxSecurity2* pFaxSecurity, LPTSTR lptstrAccName, DWORD dwAccessMask)
{
        //Get the SD parts, create new DACL and then create SD from it
        HRESULT hr = S_OK;
        bool bRetVal = false;
        hr = GetSDParts(pFaxSecurity);
        if(FAILED(hr))
        {
                _tprintf(_T("GetSDParts failed. Error %x \n"), hr);
                bRetVal = false;
                goto Exit;        
        }    
        hr = AddDenyAce(lptstrAccName, dwAccessMask);
        if(FAILED(hr))
        {
                _tprintf(_T("AddDenyAce failed. Error %x \n"), hr);
                bRetVal = false;
                goto Exit;        
        }    
        hr = CreateNewSD();
        if(FAILED(hr))
        {
                _tprintf(_T("CreateNewSD failed. Error %x \n"), hr);
                bRetVal = false;
                goto Exit;        
        }    
        bRetVal = true;
Exit:
        return bRetVal;
}

//+---------------------------------------------------------------------------
//
//  function:   SetFaxSecurity
//
//  Synopsis:   Set the Fax Security Object
//
//  Arguments:  [pFaxSecurity] : FaxSecurity Object
//
//  Returns:    bool true if successful
//----------------------------------------------------------------------------
bool SetFaxSecurity(IFaxSecurity2* pFaxSecurity)
{
        HRESULT hr = S_OK;
        //Cnvert to Byte array
        BYTE* pbDataSD = (BYTE*) &g_bSecurityDescriptor;
        SAFEARRAY *pSafeArray;
        //Create a VARIANT object from SD
        pSafeArray = SafeArrayCreateVector(VT_UI1, 0, g_dwLength);
        if (pSafeArray == NULL)
        {
                hr = E_OUTOFMEMORY;
                _tprintf(_T("SafeArrayCreateVector failed. hr = %x \n"), hr);        
                goto Exit;
        }

        //get Access to the elements of the Safe Array
        BYTE *pbElement;
        hr = SafeArrayAccessData(pSafeArray, (void **) &pbElement);
        if (FAILED(hr))
        {
                //Failed to access safearray
                hr = E_FAIL;
                SafeArrayDestroy(pSafeArray);
                _tprintf(_T("SafeArrayAccessData failed. hr = %x \n"), hr);        
                goto Exit;
        }

        //  Fill the Safe Array with the bytes from pbDataSD
        memcpy(pbElement, pbDataSD, g_dwLength);
        hr = SafeArrayUnaccessData(pSafeArray);
        if (FAILED(hr))
        {
                //Failed to unaccess safearray
                hr = E_FAIL;
                SafeArrayDestroy(pSafeArray);
                _tprintf(_T("SafeArrayUnaccessData failed. hr = %x \n"), hr);
                goto Exit;
        }

        VARIANT pvarNewSD;
        VariantInit(&pvarNewSD);
        pvarNewSD.vt = VT_UI1 | VT_ARRAY;
        pvarNewSD.parray = pSafeArray;
        //Set the new SD
        hr = pFaxSecurity->put_InformationType(DACL_SECURITY_INFORMATION);
        if(FAILED(hr))
        {
                _tprintf(_T("put_InformationType failed. hr = %x \n"), hr);
                goto Exit;
        }
        hr = pFaxSecurity->put_Descriptor(pvarNewSD);    
        if(FAILED(hr))
        {
                _tprintf(_T("put_Descriptor failed. hr = %x \n"), hr);
                goto Exit;
        }
        hr = pFaxSecurity->Save();
        if(FAILED(hr))
        {
                _tprintf(_T("g_pFaxSecurity->Save failed. hr = %x \n"), hr);
                goto Exit;
        }
Exit:
        VariantClear(&pvarNewSD);
        return (hr == S_OK);
}

int __cdecl _tmain(int argc, _TCHAR* argv[])
{
        HRESULT hr = S_OK;
        bool bRetVal = true;
        TCHAR* lptstrServerName = NULL;
        TCHAR* lptstrOption = NULL;
        TCHAR* lptstrAccessMask = NULL;
        TCHAR* lptstrDeny = NULL;
        TCHAR* lptstrAccName = NULL;
        BSTR bstrServerName = NULL;
        BOOL bDeny = TRUE;
        DWORD dwAccessMask = 0;
        bool bConnected = false;
        size_t argSize = 0;

        bool bVersion = IsOSVersionCompatible(VISTA);

        //Check is OS is Vista
        if(bVersion == false)
        {
                _tprintf(_T("OS Version does not support this feature"));
                bRetVal = false;
                goto Exit1;
        }

        //introducing an artifical scope here so that the COM objects are destroyed before CoInitialize is called
        { 
                //COM objects
                IFaxServer2* pFaxServer =NULL;
                IFaxSecurity2* pFaxSecurity =NULL;
                int argcount = 0;
                int count = 0;

#ifdef UNICODE
                argv = CommandLineToArgvW( GetCommandLine(), &argc );
#else
                argv = argvA;
#endif

                if (argc == 1)
                {
                        _tprintf( TEXT("Missing args.\n") );
                        GiveUsage(argv[0]);
                        bRetVal = false;
                        goto Exit;
                }
                // check for commandline switches
                for (argcount=1; argcount<argc; argcount++)
                {
                        if(argcount + 1 < argc)
                        {
                                hr = StringCbLength(argv[argcount + 1],1024 * sizeof(TCHAR),&argSize);
                                if(!FAILED(hr))
                                {
                                        if ((argv[argcount][0] == L'/') || (argv[argcount][0] == L'-'))
                                        {                
                                                switch (towlower(argv[argcount][1]))
                                                {
                                                        case 's':
                                                                if(lptstrServerName == NULL)
                                                                {

                                                                        //servername parameter
                                                                        lptstrServerName = (TCHAR*) malloc((argSize +1)* sizeof(TCHAR));
                                                                        if(lptstrServerName == NULL)
                                                                        {
                                                                                _tprintf(_T("lptstrServerName: malloc failed. Error %d \n"), GetLastError());
                                                                                bRetVal = false;
                                                                                goto Exit;
                                                                        }
                                                                        memset(lptstrServerName, 0, (argSize+1) * sizeof(TCHAR));
                                                                        hr = StringCchCopyN(lptstrServerName,argSize+1, argv[argcount+1],argSize);
                                                                        if(FAILED(hr))
                                                                        {
                                                                                _tprintf(_T("lptstrServerName: StringCchCopyN failed. Error %x \n"), hr);
                                                                                bRetVal = false;
                                                                                goto Exit;
                                                                        }
                                                                }
                                                                else
                                                                {
                                                                        GiveUsage(argv[0]);
                                                                        bRetVal = false;
                                                                        goto Exit;
                                                                }
                                                                break;
                                                        case 'o':
                                                                if(lptstrOption == NULL)
                                                                {
                                                                        //option parameter (list or reassign)
                                                                        lptstrOption = (TCHAR*) malloc((argSize +1)* sizeof(TCHAR));                       
                                                                        if(lptstrOption == NULL)
                                                                        {
                                                                                _tprintf(_T("lptstrOption: malloc failed. Error %d \n"), GetLastError());
                                                                                bRetVal = false;
                                                                                goto Exit;
                                                                        }
                                                                        memset(lptstrOption, 0, (argSize+1)* sizeof(TCHAR));
                                                                        hr = StringCchCopyN(lptstrOption,argSize+1, argv[argcount+1], argSize);
                                                                        if(FAILED(hr))
                                                                        {
                                                                                _tprintf(_T("lptstrOption: StringCchCopyN failed. Error %x \n"), hr);
                                                                                bRetVal = false;
                                                                                goto Exit;
                                                                        }
                                                                }
                                                                else
                                                                {
                                                                        GiveUsage(argv[0]);
                                                                        bRetVal = false;
                                                                        goto Exit;
                                                                }
                                                                break;
                                                        case 'a':
                                                                if(lptstrAccName == NULL)
                                                                {
                                                                        //message id parameter
                                                                        lptstrAccName = (TCHAR*) malloc((argSize +1)* sizeof(TCHAR));
                                                                        if(lptstrAccName == NULL)
                                                                        {
                                                                                _tprintf(_T("lptstrAccName: malloc failed. Error %d \n"), GetLastError());
                                                                                bRetVal = false;
                                                                                goto Exit;
                                                                        }
                                                                        memset(lptstrAccName, 0, (argSize+1)* sizeof(TCHAR));
                                                                        hr = StringCchCopyN(lptstrAccName,argSize+1, argv[argcount+1],argSize);                   
                                                                        if(FAILED(hr))
                                                                        {
                                                                                _tprintf(_T("lptstrAccName: StringCchCopyN failed. Error %x \n"), hr);
                                                                                bRetVal = false;
                                                                                goto Exit;
                                                                        }
                                                                }
                                                                else
                                                                {
                                                                        GiveUsage(argv[0]);
                                                                        bRetVal = false;
                                                                        goto Exit;
                                                                }
                                                                break;                        
                                                        case 'd':
                                                                if(lptstrDeny == NULL)
                                                                {
                                                                        //recipient parameter
                                                                        lptstrDeny = (TCHAR*) malloc((argSize +1)* sizeof(TCHAR));
                                                                        if(lptstrDeny == NULL)
                                                                        {
                                                                                _tprintf(_T("lptstrDeny: malloc failed. Error %x \n"), GetLastError());
                                                                                bRetVal = false;
                                                                                goto Exit;
                                                                        }
                                                                        memset(lptstrDeny, 0, argSize+1* sizeof(TCHAR));

                                                                        hr = StringCchCopyN(lptstrDeny,argSize+1, argv[argcount+1],argSize);
                                                                        if(FAILED(hr))
                                                                        {
                                                                                _tprintf(_T("lptstrDeny: StringCchCopyN failed. Error %x \n"), hr);
                                                                                bRetVal = false;
                                                                                goto Exit;
                                                                        }
                                                                }
                                                                else
                                                                {
                                                                        GiveUsage(argv[0]);
                                                                        bRetVal = false;
                                                                        goto Exit;
                                                                }
                                                                break;
                                                        case 'm':
                                                                if(lptstrAccessMask == NULL)
                                                                {
                                                                        //recipient parameter
                                                                        lptstrAccessMask = (TCHAR*) malloc((argSize +1)* sizeof(TCHAR));
                                                                        if(lptstrAccessMask == NULL)
                                                                        {
                                                                                _tprintf(_T("lptstrAccessMask: malloc failed. Error %x \n"), GetLastError());
                                                                                bRetVal = false;
                                                                                goto Exit;
                                                                        }
                                                                        memset(lptstrAccessMask, 0, argSize+1* sizeof(TCHAR));

                                                                        hr = StringCchCopyN(lptstrAccessMask,argSize+1, argv[argcount+1],argSize);
                                                                        if(FAILED(hr))
                                                                        {
                                                                                _tprintf(_T("lptstrAccessMask: StringCchCopyN failed. Error %x \n"), hr);
                                                                                bRetVal = false;
                                                                                goto Exit;
                                                                        }
                                                                }
                                                                else
                                                                {
                                                                        GiveUsage(argv[0]);
                                                                        bRetVal = false;
                                                                        goto Exit;
                                                                }
                                                                break;
                                                        case '?':
                                                                GiveUsage(argv[0]);
                                                                bRetVal = false;
                                                                goto Exit;                
                                                        default:
                                                                break;
                                                }//switch
                                        }//if
                                }//if failed
                        }//(argcount + 1 < argc)
                }//for

                if ((lptstrOption == NULL) || (( _tcscmp(_T("print"), CharLower(lptstrOption)) != 0 ) && (!lptstrAccName || !lptstrAccessMask  || !lptstrDeny)))
                {
                        _tprintf( TEXT("Missing args.\n") );
                        GiveUsage(argv[0]);
                        bRetVal = false;
                        goto Exit;
                }

                if(lptstrAccessMask)
                {
                        dwAccessMask = getDWordAccess(lptstrAccessMask);
                        if(0 != _ttoi(lptstrDeny))
                                bDeny = true;
                        else
                                bDeny = false;
                }

                //initialize COM
                hr = CoInitialize(NULL);
                if(FAILED(hr))
                {
                        //failed to init com
                        _tprintf(_T("Failed to init com. Error %x \n"), hr);
                        bRetVal = false;
                        goto Exit;
                }

                hr = CoCreateInstance (CLSID_FaxServer, 
                            NULL, 
                            CLSCTX_ALL, 
                            __uuidof(IFaxServer), 
                            (void **)&pFaxServer);
                if(FAILED(hr))
                {
                        //CoCreateInstance failed.
                        _tprintf(_T("CoCreateInstance failed. Error 0x%x \n"), hr);
                        bRetVal = false;
                        goto Exit;
                }
                //connect to fax server.
                bstrServerName = SysAllocString(lptstrServerName);
                hr = pFaxServer->Connect(bstrServerName);
                if(FAILED(hr))
                {
                        _tprintf(_T("Connect failed. Error 0x%x \n"), hr);
                        bRetVal = false;
                        goto Exit;
                }
                bConnected = true;

                FAX_SERVER_APIVERSION_ENUM enumFaxAPIVersion;
                hr = pFaxServer->get_APIVersion(&enumFaxAPIVersion);
                if(FAILED(hr))
                {
                        //get_APIVersion failed.
                        _tprintf(_T("get_APIVersion failed. Error 0x%x \n"), hr);
                        bRetVal = false;
                        goto Exit;
                }

                if (enumFaxAPIVersion < fsAPI_VERSION_3) 
                {
                        bRetVal = false;
                        _tprintf(_T("OS Version does not support this feature"));
                        goto Exit;
                }         

                hr = pFaxServer->get_Security2(&pFaxSecurity);
                if(FAILED(hr))
                {
                        _tprintf(_T("get_Security failed. Error %x \n"), hr);
                        bRetVal = false;
                        goto Exit;
                }        

                if(_tcscmp(_T("print"), CharLower(lptstrOption)) == 0)
                {
                        //print aces
                        bRetVal = PrintACEsFax(pFaxSecurity);
                        if(bRetVal == false)
                        {
                                _tprintf(_T("PrintACEsFax Failed"));
                                goto Exit;
                        }
                        _tprintf(_T("\n \nDone with printing of ACE \n"));
                }

                if(_tcscmp(_T("add"), CharLower(lptstrOption)) == 0)
                {

                        //Add the appropiate ACE and create a new SD
                        if(bDeny)
                        {
                                bRetVal = AddDenyAceFax(pFaxSecurity, lptstrAccName,dwAccessMask);
                                if(bRetVal == false)
                                {
                                        _tprintf(_T("AddDenyAceFax Failed"));
                                        goto Exit;
                                }
                        }
                        else
                        {
                                bRetVal = AddAllowAceFax(pFaxSecurity, lptstrAccName,dwAccessMask);
                                if(bRetVal == false)
                                {
                                        _tprintf(_T("AddAllowAceFax Failed"));
                                        goto Exit;
                                }
                        }
                        if(bRetVal == true )
                        {
                                //Set the FaxSecurity with new SD
                                bRetVal = SetFaxSecurity(pFaxSecurity);
                                if(bRetVal == false)
                                {
                                        _tprintf(_T("SetFaxSecurity Failed"));
                                        goto Exit;
                                }
                        }
                        _tprintf(_T("\n \n Done with addition of ACE \n"));

                }

                if(_tcscmp(_T("delete"), CharLower(lptstrOption)) == 0)
                {
                        //Delete the appropiate ACE and create a new SD
                        bRetVal = DeleteAceFax(pFaxSecurity, bDeny, lptstrAccName, dwAccessMask);
                        if(bRetVal == false)
                        {
                                _tprintf(_T("DeleteAceFax Failed"));
                                goto Exit;
                        }
                        if(bRetVal == true)
                        {
                                //Set the FaxSecurity with new SD
                                bRetVal = SetFaxSecurity(pFaxSecurity);
                                if(bRetVal == false)
                                {
                                        _tprintf(_T("SetFaxSecurity Failed"));
                                        goto Exit;
                                }
                        }
                        _tprintf(_T("\n \n Done with deletion of ACE \n"));
                }

Exit:
                if(bConnected)
                {
                        pFaxServer->Disconnect();
                }
                if(lptstrServerName)
                        free(lptstrServerName);
                if(lptstrOption)
                        free(lptstrOption);
                if(lptstrAccessMask)
                        free(lptstrAccessMask);
                if(lptstrAccName)
                        free(lptstrAccName);
                if(lptstrDeny)
                        free(lptstrDeny);
        }
        CoUninitialize();
Exit1:
        return bRetVal;
}
