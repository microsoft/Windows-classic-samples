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
 * NAME: CopyACL                                                             *
 * --------------------------------------------------------------------------*
 * DESCRIPTION: 
 *
\*---------------------------------------------------------------------------*/
DWORD CopyACL (
    PACL paclOld,
    PACL paclNew
    )
{
    ACL_SIZE_INFORMATION  aclSizeInfo = {0};
    LPVOID                pvAce       = NULL;
    ACE_HEADER            *pAceHeader = NULL;
    ULONG                 i;

    

    if(!GetAclInformation (paclOld, 
                           (LPVOID) &aclSizeInfo, 
                           (DWORD) sizeof (aclSizeInfo), 
                           AclSizeInformation))
    {
        return GetLastError();
    }

    // Copy all of the ACEs to the new ACL
    for (i = 0; i < aclSizeInfo.AceCount; i++)
    {
        // Get the ACE and header info
        if (!GetAce (paclOld, i, &pvAce))
            return GetLastError();

        pAceHeader = (ACE_HEADER *) pvAce;

        // Add the ACE to the new list
        if (!AddAce (paclNew, ACL_REVISION, 0xffffffff, pvAce, pAceHeader->AceSize))
            return GetLastError();
    }

    return ERROR_SUCCESS;
}


/*---------------------------------------------------------------------------*\
 * NAME: AddAccessDeniedACEToACL                                             *
 * --------------------------------------------------------------------------*
 * DESCRIPTION: Adds an Access Denied ACE with the designate access mask to 
 * the supplied ACL.
\*---------------------------------------------------------------------------*/
DWORD AddAccessDeniedACEToACL (
    PACL *paclOrig,
    DWORD dwAccessMask,
    LPTSTR tszPrincipal
    )
{
    ACL_SIZE_INFORMATION  aclSizeInfo   = {0};
    int                   cbAclSize     = 0;
    DWORD                 dwReturnValue = ERROR_SUCCESS;
    PSID                  psidPrincipal = NULL;
    PACL                  paclOld       = NULL; 
    PACL                  paclNew       = NULL;

    if(!paclOrig) return ERROR_BAD_ARGUMENTS;

    paclOld = *paclOrig;

    dwReturnValue = GetPrincipalSID (tszPrincipal, &psidPrincipal);
    if (dwReturnValue != ERROR_SUCCESS) goto CLEANUP;

    if(!GetAclInformation (paclOld, 
                           (LPVOID) &aclSizeInfo, 
                           (DWORD) sizeof (ACL_SIZE_INFORMATION), 
                           AclSizeInformation))
    {
        dwReturnValue = GetLastError();
        goto CLEANUP;
    }

    cbAclSize = aclSizeInfo.AclBytesInUse +
              sizeof (ACL) + sizeof (ACCESS_DENIED_ACE) +
              GetLengthSid (psidPrincipal) - sizeof (DWORD);

    paclNew = (PACL) malloc(cbAclSize);
    if(!paclNew)
    {
        dwReturnValue = ERROR_OUTOFMEMORY;
        goto CLEANUP;
    }

    if (!InitializeAcl (paclNew, cbAclSize, ACL_REVISION))
    {
        dwReturnValue = GetLastError();
        goto CLEANUP;
    }

    if (!AddAccessDeniedAce (paclNew, 
                             ACL_REVISION2, 
                             dwAccessMask, 
                             psidPrincipal))
    {
        dwReturnValue = GetLastError();
        goto CLEANUP;
    }

    dwReturnValue = CopyACL (paclOld, paclNew);
    if (dwReturnValue != ERROR_SUCCESS) goto CLEANUP;

    *paclOrig = paclNew;

CLEANUP:

    if(psidPrincipal) free (psidPrincipal);
    
    return dwReturnValue;
}


/*---------------------------------------------------------------------------*\
 * NAME: AddAccessAllowedACEToACL                                            *
 * --------------------------------------------------------------------------*
 * DESCRIPTION: Adds an Access Allowed ACE with the designated mask to the 
 * supplied ACL.
\*---------------------------------------------------------------------------*/
DWORD AddAccessAllowedACEToACL (
    PACL *paclOrig,
    DWORD dwAccessMask,
    LPTSTR tszPrincipal
    )
{
    ACL_SIZE_INFORMATION  aclSizeInfo   = {0};
    int                   cbAclSize     = 0;
    DWORD                 dwReturnValue = ERROR_SUCCESS;
    PSID                  psidPrincipal = NULL;
    PACL                  paclOld       = NULL;
    PACL                  paclNew       = NULL;

    if(!paclOrig) return ERROR_BAD_ARGUMENTS;

    paclOld = *paclOrig;

    dwReturnValue = GetPrincipalSID (tszPrincipal, &psidPrincipal);
    if (dwReturnValue != ERROR_SUCCESS) goto CLEANUP;

    if(!GetAclInformation (paclOld, 
                           (LPVOID) &aclSizeInfo, 
                           (DWORD) sizeof (ACL_SIZE_INFORMATION), 
                           AclSizeInformation))
    {
        dwReturnValue = GetLastError();
        goto CLEANUP;
    }

    cbAclSize = aclSizeInfo.AclBytesInUse +
              sizeof (ACL) + sizeof (ACCESS_ALLOWED_ACE) +
              GetLengthSid (psidPrincipal) - sizeof (DWORD);

    paclNew = (PACL) malloc(cbAclSize);
    if(!paclNew)
    {
        dwReturnValue = ERROR_OUTOFMEMORY;
        goto CLEANUP;
    }

    if (!InitializeAcl (paclNew, cbAclSize, ACL_REVISION))
    {
        dwReturnValue = GetLastError();
        goto CLEANUP;
    }

    dwReturnValue = CopyACL (paclOld, paclNew);
    if (dwReturnValue != ERROR_SUCCESS) goto CLEANUP;


    if (!AddAccessAllowedAce (paclNew, 
                              ACL_REVISION2, 
                              dwAccessMask, 
                              psidPrincipal))
    {
        dwReturnValue = GetLastError();
        goto CLEANUP;
    }

    *paclOrig = paclNew;

CLEANUP:

    if(dwReturnValue != ERROR_SUCCESS && paclNew)
    {
        free(paclNew);
    }

    if(psidPrincipal) free (psidPrincipal);
    
    return dwReturnValue;
}

/*---------------------------------------------------------------------------*\
 * NAME: UpdatePrincipalInACL                                                *
 * --------------------------------------------------------------------------*
 * DESCRIPTION: Updates the access mask in all of the ace types in the
 * designated ACL.
\*---------------------------------------------------------------------------*/
DWORD UpdatePrincipalInACL (
    PACL paclOrig,
    LPTSTR tszPrincipal,
    DWORD dwAccessMask,
    BOOL fRemove,
    DWORD fAceType
    )
{
    ACL_SIZE_INFORMATION    aclSizeInfo        = {0};
    LONG                   i;
    LPVOID                  pvAce              = NULL;
    ACCESS_ALLOWED_ACE      *pAccessAllowedAce = NULL;
    ACCESS_DENIED_ACE       *pAccessDeniedAce  = NULL;
    SYSTEM_AUDIT_ACE        *pSystemAuditAce   = NULL;
    PSID                    psidPrincipal      = NULL;
    DWORD                   dwReturnValue      = ERROR_SUCCESS;
    ACE_HEADER              *pAceHeader        = NULL;
    DWORD                   dwFoundValue       = ERROR_FILE_NOT_FOUND;

    //Construct a SID for this principal
    dwReturnValue = GetPrincipalSID (tszPrincipal, &psidPrincipal);
    if (dwReturnValue != ERROR_SUCCESS) goto CLEANUP;

    if(!GetAclInformation (paclOrig, 
                           (LPVOID) &aclSizeInfo, 
                           (DWORD) sizeof (ACL_SIZE_INFORMATION), 
                           AclSizeInformation))
    {
        dwReturnValue = GetLastError();
        goto CLEANUP;
    }

    //Iterate through all of the ACEs in this ACL looking for a SID that corresponds 
    //to the principal.
    for (i = aclSizeInfo.AceCount-1; i >= 0; i--)
    {
        if (!GetAce (paclOrig, i, &pvAce))
        {
            dwReturnValue = GetLastError();
            goto CLEANUP;
        }

        pAceHeader = (ACE_HEADER *) pvAce;

        if ((fAceType == ACCESS_ALLOWED_ACE_TYPE ||
             fAceType == ACE_TYPE_ALL) && 
            pAceHeader->AceType == ACCESS_ALLOWED_ACE_TYPE)
        {
            pAccessAllowedAce = (ACCESS_ALLOWED_ACE *) pvAce;

            if (EqualSid (psidPrincipal, (PSID) &pAccessAllowedAce->SidStart))
            {
                //We've found an ACE.  Let's modify the mask bits
                if(fRemove && (pAccessAllowedAce->Mask & SPECIFIC_RIGHTS_ALL))
                {
                    pAccessAllowedAce->Mask &= ~dwAccessMask;
                    //If there are any COM rights reamining we must ensure 
                    //COM_RIGHTS_EXECUTE is set before leaving.
                    if(pAccessAllowedAce->Mask & (COM_RIGHTS_ACTIVATE_LOCAL  |
                                                  COM_RIGHTS_ACTIVATE_REMOTE |
                                                  COM_RIGHTS_EXECUTE_LOCAL   |
                                                  COM_RIGHTS_EXECUTE_REMOTE))
                    {
                        pAccessAllowedAce->Mask |= COM_RIGHTS_EXECUTE;
                    }
                    else if((pAccessAllowedAce->Mask & SPECIFIC_RIGHTS_ALL) == 0) 
                    {
                        //If there are no more specific rights on this ACE we
                        //may delete it.
                        DeleteAce (paclOrig, i);
                    }
                                              
                }
                else
                {
                    pAccessAllowedAce->Mask |= dwAccessMask;
                }
                dwFoundValue = ERROR_SUCCESS;
            }
        } 
        else if ((fAceType == ACCESS_DENIED_ACE_TYPE ||
                  fAceType == ACE_TYPE_ALL) && 
                 pAceHeader->AceType == ACCESS_DENIED_ACE_TYPE)
        {
            pAccessDeniedAce = (ACCESS_DENIED_ACE *) pvAce;

            if (EqualSid (psidPrincipal, (PSID) &pAccessDeniedAce->SidStart))
            {
                //We've found an ACE.  Let's modify the mask bits
                if(fRemove && (pAccessDeniedAce->Mask & SPECIFIC_RIGHTS_ALL))
                {
                    pAccessDeniedAce->Mask &= ~dwAccessMask;

                    //If there are any COM rights reamining we must ensure 
                    //COM_RIGHTS_EXECUTE is set before leaving.
                    if(pAccessDeniedAce->Mask & (COM_RIGHTS_ACTIVATE_LOCAL  |
                                                 COM_RIGHTS_ACTIVATE_REMOTE |
                                                 COM_RIGHTS_EXECUTE_LOCAL   |
                                                 COM_RIGHTS_EXECUTE_REMOTE))
                    {
                        pAccessDeniedAce->Mask |= COM_RIGHTS_EXECUTE;
                    }
                    else if((pAccessDeniedAce->Mask & SPECIFIC_RIGHTS_ALL) == 0) 
                    {
                        //If there are no more specific rights on this ACE we
                        //may delete it.
                        DeleteAce (paclOrig, i);
                    }
                }
                else
                {
                    pAccessDeniedAce->Mask |= dwAccessMask;
                }
                dwFoundValue = ERROR_SUCCESS;
            }
        } 
        else if ((fAceType == SYSTEM_AUDIT_ACE_TYPE ||
                  fAceType == ACE_TYPE_ALL) && 
                 pAceHeader->AceType == SYSTEM_AUDIT_ACE_TYPE)
        {
            pSystemAuditAce = (SYSTEM_AUDIT_ACE *) pvAce;

            if (EqualSid (psidPrincipal, (PSID) &pSystemAuditAce->SidStart))
            {
                //We've found an ACE.  Let's modify the mask bits
                if(fRemove && (pSystemAuditAce->Mask & SPECIFIC_RIGHTS_ALL))
                {
                    pSystemAuditAce->Mask &= ~dwAccessMask;

                    //If there are any COM rights reamining we must ensure 
                    //COM_RIGHTS_EXECUTE is set before leaving.
                    if(pSystemAuditAce->Mask & (COM_RIGHTS_ACTIVATE_LOCAL  |
                                                COM_RIGHTS_ACTIVATE_REMOTE |
                                                COM_RIGHTS_EXECUTE_LOCAL   |
                                                COM_RIGHTS_EXECUTE_REMOTE))
                    {
                        pSystemAuditAce->Mask |= COM_RIGHTS_EXECUTE;
                    }
                    else if((pSystemAuditAce->Mask & SPECIFIC_RIGHTS_ALL) == 0) 
                    {
                        //If there are no more specific rights on this ACE we
                        //may delete it.
                        DeleteAce (paclOrig, i);
                    }
                }
                else
                {
                    pSystemAuditAce->Mask |= dwAccessMask;
                }
                dwFoundValue = ERROR_SUCCESS;
            }
        }
    }

CLEANUP:

    if(psidPrincipal) free (psidPrincipal);

    if(dwReturnValue == ERROR_SUCCESS) dwReturnValue = dwFoundValue;
    
    return dwReturnValue;
}


/*---------------------------------------------------------------------------*\
 * NAME: RemovePrincipalFromACL                                              *
 * --------------------------------------------------------------------------*
 * DESCRIPTION: Deletes all of the ACEs belonging to the designated 
 * principal in this ACL.
\*---------------------------------------------------------------------------*/
DWORD RemovePrincipalFromACL (
    PACL paclOrig,
    LPTSTR tszPrincipal,
    DWORD fAceType
    )
{
    ACL_SIZE_INFORMATION    aclSizeInfo        = {0};
    LONG                   i;
    LPVOID                  pvAce              = NULL;
    ACCESS_ALLOWED_ACE      *pAccessAllowedAce = NULL;
    ACCESS_DENIED_ACE       *pAccessDeniedAce  = NULL;
    SYSTEM_AUDIT_ACE        *pSystemAuditAce   = NULL;
    PSID                    psidPrincipal      = NULL;
    DWORD                   dwReturnValue      = ERROR_SUCCESS;
    ACE_HEADER              *pAceHeader        = NULL;
    DWORD                   dwFoundValue       = ERROR_FILE_NOT_FOUND;

    dwReturnValue = GetPrincipalSID (tszPrincipal, &psidPrincipal);
    if (dwReturnValue != ERROR_SUCCESS) goto CLEANUP;

    if(!GetAclInformation (paclOrig, 
                           (LPVOID) &aclSizeInfo, 
                           (DWORD) sizeof (ACL_SIZE_INFORMATION), 
                           AclSizeInformation))
    {
        dwReturnValue = GetLastError();
        goto CLEANUP;
    }

    for (i = aclSizeInfo.AceCount-1; i >= 0; i--)
    {
        if (!GetAce (paclOrig, i, &pvAce))
        {
            dwReturnValue = GetLastError();
            goto CLEANUP;
        }

        pAceHeader = (ACE_HEADER *) pvAce;

        if ((fAceType == ACCESS_ALLOWED_ACE_TYPE ||
             fAceType == ACE_TYPE_ALL) && 
            pAceHeader->AceType == ACCESS_ALLOWED_ACE_TYPE)
        {
            pAccessAllowedAce = (ACCESS_ALLOWED_ACE *) pvAce;

            if (EqualSid (psidPrincipal, (PSID) &pAccessAllowedAce->SidStart))
            {
                if(pAccessAllowedAce->Mask & SPECIFIC_RIGHTS_ALL)
                {
                    DeleteAce (paclOrig, i);
                }
                dwFoundValue = ERROR_SUCCESS;
            }
        } 
        else if ((fAceType == ACCESS_DENIED_ACE_TYPE ||
                  fAceType == ACE_TYPE_ALL) && 
                 pAceHeader->AceType == ACCESS_DENIED_ACE_TYPE)
        {
            pAccessDeniedAce = (ACCESS_DENIED_ACE *) pvAce;

            if (EqualSid (psidPrincipal, (PSID) &pAccessDeniedAce->SidStart))
            {
                if(pAccessDeniedAce->Mask & SPECIFIC_RIGHTS_ALL)
                {
                    DeleteAce (paclOrig, i);
                }
                dwFoundValue = ERROR_SUCCESS;
            }
        } 
        else if ((fAceType == SYSTEM_AUDIT_ACE_TYPE ||
                  fAceType == ACE_TYPE_ALL) && 
                 pAceHeader->AceType == SYSTEM_AUDIT_ACE_TYPE)
        {
            pSystemAuditAce = (SYSTEM_AUDIT_ACE *) pvAce;

            if (EqualSid (psidPrincipal, (PSID) &pSystemAuditAce->SidStart))
            {
                if(pSystemAuditAce->Mask & SPECIFIC_RIGHTS_ALL)
                {
                    DeleteAce (paclOrig, i);
                }
                dwFoundValue = ERROR_SUCCESS;
            }
        }
    }

CLEANUP:

    if(psidPrincipal) free (psidPrincipal);

    if(dwReturnValue == ERROR_SUCCESS) dwReturnValue = dwFoundValue;
    
    return dwReturnValue;
}

