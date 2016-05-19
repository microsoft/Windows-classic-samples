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
 * NAME: ListMachineAccessACL                                                *
 * --------------------------------------------------------------------------*
 * DESCRIPTION: Displays the Machine Access ACL                              *
 * --------------------------------------------------------------------------*
 * RETURNS: WIN32 Error Code                                                 *
\*---------------------------------------------------------------------------*/
DWORD ListMachineAccessACL()
{
    return ListNamedValueSD (HKEY_LOCAL_MACHINE, 
                             _T("Software\\Microsoft\\Ole"), 
                             _T("MachineAccessRestriction"), 
                             SDTYPE_MACHINE_ACCESS);
}

/*---------------------------------------------------------------------------*\
 * NAME: ListMachineLaunchACL                                                *
 * --------------------------------------------------------------------------*
 * DESCRIPTION: Displays the Machine Launch ACL                              *
 * --------------------------------------------------------------------------*
 * RETURNS: WIN32 Error Code                                                 *
\*---------------------------------------------------------------------------*/
DWORD ListMachineLaunchACL()
{
    return ListNamedValueSD (HKEY_LOCAL_MACHINE, 
                             _T("Software\\Microsoft\\Ole"), 
                             _T("MachineLaunchRestriction"), 
                             SDTYPE_MACHINE_LAUNCH);
}

/*---------------------------------------------------------------------------*\
 * NAME: ListDefaultAccessACL                                                *
 * --------------------------------------------------------------------------*
 * DESCRIPTION: Displays the Default Access ACL                              *
 * --------------------------------------------------------------------------*
 * RETURNS: WIN32 Error Code                                                 *
\*---------------------------------------------------------------------------*/
DWORD ListDefaultAccessACL()
{
    return ListNamedValueSD (HKEY_LOCAL_MACHINE, 
                             _T("Software\\Microsoft\\Ole"), 
                             _T("DefaultAccessPermission"), 
                             SDTYPE_DEFAULT_ACCESS);
}

/*---------------------------------------------------------------------------*\
 * NAME: ListDefaultLaunchACL                                                *
 * --------------------------------------------------------------------------*
 * DESCRIPTION: Displays the Defualt Launch/Activate ACL                     *
 * --------------------------------------------------------------------------*
 * RETURNS: WIN32 Error Code                                                 *
\*---------------------------------------------------------------------------*/
DWORD ListDefaultLaunchACL()
{
    return ListNamedValueSD (HKEY_LOCAL_MACHINE, 
                             _T("Software\\Microsoft\\Ole"), 
                             _T("DefaultLaunchPermission"), 
                             SDTYPE_DEFAULT_LAUNCH);
}

/*---------------------------------------------------------------------------*\
 * NAME: ListAppIDAccessACL                                                  *
 * --------------------------------------------------------------------------*
 * DESCRIPTION: Displays a designated Application Access ACL                 *
 * --------------------------------------------------------------------------*
 * RETURNS: WIN32 Error Code                                                 *
\*---------------------------------------------------------------------------*/
DWORD ListAppIDAccessACL (
    LPTSTR tszAppID
    )
{
    TCHAR   tszKeyName [256] = {0};

    _stprintf_s (tszKeyName, 
                 RTL_NUMBER_OF(tszKeyName),
                 tszAppID [0] == '{' ? _T("APPID\\%s") : _T("APPID\\{%s}"), 
                 tszAppID);

    return ListNamedValueSD (HKEY_CLASSES_ROOT, 
                             tszKeyName, 
                             _T("AccessPermission"), 
                             SDTYPE_APPLICATION_ACCESS);
}

/*---------------------------------------------------------------------------*\
 * NAME: ListAppIDLaunchACL                                                  *
 * --------------------------------------------------------------------------*
 * DESCRIPTION: Displays a designated Application Launch/Activate ACL        *
 * --------------------------------------------------------------------------*
 * RETURNS: WIN32 Error Code                                                 *
\*---------------------------------------------------------------------------*/
DWORD ListAppIDLaunchACL (
    LPTSTR tszAppID
    )
{
    TCHAR   tszKeyName [256] = {0};

    _stprintf_s (tszKeyName, 
                 RTL_NUMBER_OF(tszKeyName),
                 tszAppID [0] == '{' ? _T("APPID\\%s") : _T("APPID\\{%s}"), 
                 tszAppID);

    return ListNamedValueSD (HKEY_CLASSES_ROOT, 
                             tszKeyName, 
                             _T("LaunchPermission"), 
                             SDTYPE_APPLICATION_LAUNCH);
}


/*---------------------------------------------------------------------------*\
 * NAME: ChangeMachineAccessACL                                              *
 * --------------------------------------------------------------------------*
 * DESCRIPTION: Modifies the machine-wide access ACL. The                    *
 * system uses the machine-wide access ACL to determine the type of access   *
 * a principal has to servers on this machine.  This value will override     *
 * any default or application specific settings.                             *
 * --------------------------------------------------------------------------*
 *  ARGUMENTS:                                                               *
 *                                                                           *
 *  tszPrincipal - Name of user or group (e.g. "redmond\johndoe").           *
 *                                                                           *
 *  fSetPrincipal - TRUE if you want to add/update the principal's entry in  *
 *  the ACL, FALSE if you want to remove the principal from the ACL.         *
 *                                                                           *
 *  fSetPrincipal - TRUE if you want to add/update the principal's entry in  *
 *  the ACL, FALSE if you want to remove the principal from the ACL.         *
 *                                                                           *
 *  dwAccessMask - Specifies what type of access the principal is to be      *
 *  given.  Can be any combination of the COM_RIGHTS_* values.               *
 *                                                                           *
 * --------------------------------------------------------------------------*
 * RETURNS: WIN32 Error Code                                                 *
\*---------------------------------------------------------------------------*/
DWORD ChangeMachineAccessACL (
    LPTSTR tszPrincipal,
    BOOL fSetPrincipal,
    BOOL fPermit,
    DWORD dwAccessMask
    )
{

    DWORD dwReturnValue = ERROR_SUCCESS;

    if (fSetPrincipal)
    {
        dwReturnValue = 
            RemovePrincipalFromNamedValueSD (HKEY_LOCAL_MACHINE, 
                                             _T("Software\\Microsoft\\Ole"), 
                                             _T("MachineAccessRestriction"), 
                                             tszPrincipal, 
                                             fPermit ? ACCESS_ALLOWED_ACE_TYPE : ACCESS_DENIED_ACE_TYPE);

        if(dwReturnValue != ERROR_SUCCESS) return dwReturnValue;
        
        dwReturnValue = 
            UpdatePrincipalInNamedValueSD(HKEY HKEY_LOCAL_MACHINE, 
                                          _T("Software\\Microsoft\\Ole"), 
                                          _T("MachineAccessRestriction"), 
                                          tszPrincipal, 
                                          dwAccessMask, 
                                          TRUE, 
                                          fPermit ? ACCESS_DENIED_ACE_TYPE : ACCESS_ALLOWED_ACE_TYPE);

        if(dwReturnValue != ERROR_SUCCESS) return dwReturnValue;

        dwReturnValue = 
            AddPrincipalToNamedValueSD (HKEY_LOCAL_MACHINE, 
                                        _T("Software\\Microsoft\\Ole"), 
                                        _T("MachineAccessRestriction"), 
                                        tszPrincipal, 
                                        fPermit, 
                                        dwAccessMask,
                                        SDTYPE_MACHINE_ACCESS);
    } 
    else
    {
        dwReturnValue =  
            RemovePrincipalFromNamedValueSD (HKEY_LOCAL_MACHINE, 
                                             _T("Software\\Microsoft\\Ole"),  
                                             _T("MachineAccessRestriction"), 
                                             tszPrincipal, 
                                             ACE_TYPE_ALL);
    }

    return dwReturnValue;
}

/*---------------------------------------------------------------------------*\
 * NAME: ChangeMachineLaunchAndActivateACL                                   *
 * --------------------------------------------------------------------------*
 * DESCRIPTION: Modifies the machine-wide launch access control list. The    *
 * system uses the machine-wide launch ACL to determine the type of launch   *
 * and/or activation privileges a principal has for servers on this machine. *
 * This value will override any default or application specific settings.    *
 *---------------------------------------------------------------------------*
 *  ARGUMENTS:                                                               *
 *                                                                           *
 *  tszPrincipal - Name of user or group (e.g. "redmond\johndoe").           *
 *                                                                           *
 *  fSetPrincipal - TRUE if you want to add/update the principal's entry     *
 *  in the ACL, FALSE if you want to remove the principal from the ACL.      *
 *                                                                           *
 *  fPermit - TRUE if you want to allow the principal to access the object,  *
 *  FALSE if you want to prevent the principal from accessing the object.    *
 *                                                                           *
 *  dwAccessMask - Specifies what type of access the principal is to be      *
 *  given.  Can be any combination of the COM_RIGHTS_* values.               *
 *                                                                           *
 * --------------------------------------------------------------------------*
 * RETURNS: WIN32 Error Code                                                 *
\*---------------------------------------------------------------------------*/
DWORD ChangeMachineLaunchAndActivateACL (
    LPTSTR tszPrincipal,
    BOOL fSetPrincipal,
    BOOL fPermit,
    DWORD dwAccessMask
    )
{

    DWORD dwReturnValue = ERROR_SUCCESS;

    if (fSetPrincipal)
    {
        dwReturnValue = 
            RemovePrincipalFromNamedValueSD (HKEY_LOCAL_MACHINE, 
                                             _T("Software\\Microsoft\\Ole"), 
                                             _T("MachineLaunchRestriction"), 
                                             tszPrincipal, 
                                             fPermit ? ACCESS_ALLOWED_ACE_TYPE : 
                                             ACCESS_DENIED_ACE_TYPE);

        if(dwReturnValue != ERROR_SUCCESS) return dwReturnValue;
        
        dwReturnValue = 
            UpdatePrincipalInNamedValueSD(HKEY_LOCAL_MACHINE, 
                                          _T("Software\\Microsoft\\Ole"), 
                                          _T("MachineLaunchRestriction"), 
                                          tszPrincipal, 
                                          dwAccessMask, 
                                          TRUE, 
                                          fPermit ? ACCESS_DENIED_ACE_TYPE : 
                                          ACCESS_ALLOWED_ACE_TYPE);

        if(dwReturnValue != ERROR_SUCCESS) return dwReturnValue;

        dwReturnValue = 
            AddPrincipalToNamedValueSD (HKEY_LOCAL_MACHINE, 
                                        _T("Software\\Microsoft\\Ole"),
                                        _T("MachineLaunchRestriction"), 
                                        tszPrincipal, 
                                        fPermit, 
                                        dwAccessMask,
                                        SDTYPE_MACHINE_LAUNCH);
    } 
    else
    {
        dwReturnValue =  
            RemovePrincipalFromNamedValueSD (HKEY_LOCAL_MACHINE, 
                                             _T("Software\\Microsoft\\Ole"), 
                                             _T("MachineLaunchRestriction"), 
                                             tszPrincipal, 
                                             ACE_TYPE_ALL);
    }

    return dwReturnValue;
}


/*---------------------------------------------------------------------------*\
 * NAME: ChangeDefaultAccessACL                                              *
 * --------------------------------------------------------------------------*
 * DESCRIPTION: Modifies the default access ACL. The system uses the         *
 * default access ACL to determine if a principal is allowed to access the   *
 * COM server if the COM server does not have its own access ACL in the      *
 * AppID section of the registry.                                            *
 * --------------------------------------------------------------------------*
 *  ARGUMENTS:                                                               *
 *                                                                           *
 *  tszPrincipal - Name of user or group (e.g. "redmond\johndoe").           *
 *                                                                           *
 *  fSetPrincipal - TRUE if you want to add/update the principal's entry in  *
 *  the ACL, FALSE if you want to remove the principal from the ACL.         *
 *                                                                           *
 *  fPermit - TRUE if you want to allow the principal to access the object,  *
 *  FALSE if you want to prevent the principal from accessing the object.    *
 *                                                                           *
 *  dwAccessMask - Specifies what type of access the principal is to be      *
 *  given.  Can be any combination of the COM_RIGHTS_* values.               *
 *                                                                           *
 * --------------------------------------------------------------------------*
 * RETURNS: WIN32 Error Code                                                 *
\*---------------------------------------------------------------------------*/
DWORD ChangeDefaultAccessACL (
    LPTSTR tszPrincipal,
    BOOL fSetPrincipal,
    BOOL fPermit,
    DWORD dwAccessMask
    )
{

    DWORD dwReturnValue = ERROR_SUCCESS;

    if (fSetPrincipal)
    {
        dwReturnValue = 
            RemovePrincipalFromNamedValueSD (HKEY_LOCAL_MACHINE, 
                                             _T("Software\\Microsoft\\Ole"), 
                                             _T("DefaultAccessPermission"), 
                                             tszPrincipal, 
                                             fPermit ? ACCESS_ALLOWED_ACE_TYPE : 
                                             ACCESS_DENIED_ACE_TYPE);

        if(dwReturnValue != ERROR_SUCCESS) return dwReturnValue;
        
        dwReturnValue = 
            UpdatePrincipalInNamedValueSD(HKEY_LOCAL_MACHINE, 
                                          _T("Software\\Microsoft\\Ole"), 
                                          _T("DefaultAccessPermission"), 
                                          tszPrincipal, 
                                          dwAccessMask, 
                                          TRUE, 
                                          fPermit ? ACCESS_DENIED_ACE_TYPE : 
                                          ACCESS_ALLOWED_ACE_TYPE);

        if(dwReturnValue != ERROR_SUCCESS) return dwReturnValue;

        dwReturnValue = 
            AddPrincipalToNamedValueSD (HKEY_LOCAL_MACHINE, 
                                        _T("Software\\Microsoft\\Ole"), 
                                        _T("DefaultAccessPermission"), 
                                        tszPrincipal, 
                                        fPermit, 
                                        dwAccessMask,
                                        SDTYPE_DEFAULT_ACCESS);
    } 
    else
    {
        dwReturnValue =  
            RemovePrincipalFromNamedValueSD (HKEY_LOCAL_MACHINE, 
                                             _T("Software\\Microsoft\\Ole"), 
                                             _T("DefaultAccessPermission"), 
                                             tszPrincipal, 
                                             ACE_TYPE_ALL);
    }

    return dwReturnValue;

}

/*---------------------------------------------------------------------------*\
 * NAME: ChangeDefaultLaunchAndActivateACL                                   *
 * --------------------------------------------------------------------------*
 * DESCRIPTION: Modifies the default launch access control list. The         *
 * system uses the default launch ACL to determine if a principal is         *
 * allowed to launch a COM server if the COM server does not have its        *
 * own launch ACL in the AppID section of the registry.                      *
 * --------------------------------------------------------------------------*
 *  ARGUMENTS:                                                               *
 *                                                                           *
 *  tszPrincipal - Name of user or group (e.g. "redmond\johndoe").           *
 *                                                                           *
 *  fSetPrincipal - TRUE if you want to add/update the principal's entry     *
 *  in the ACL, FALSE if you want to remove the principal from the ACL.      *
 *                                                                           *
 *  fPermit - TRUE if you want to allow the principal to launch the object   *
 *  FALSE if you want to prevent the principal from launching the object.    *
 *                                                                           *
 *  dwAccessMask - Specifies what type of access the principal is to be      *
 *  given.  Can be any combination of the COM_RIGHTS_* values.               *
 *                                                                           *
 * --------------------------------------------------------------------------*
 * RETURNS: WIN32 Error Code                                                 *
\*---------------------------------------------------------------------------*/
DWORD ChangeDefaultLaunchAndActivateACL (
    LPTSTR tszPrincipal,
    BOOL fSetPrincipal,
    BOOL fPermit,
    DWORD dwAccessMask
    )
{
    DWORD dwReturnValue = ERROR_SUCCESS;

    if (fSetPrincipal)
    {
        dwReturnValue = 
            RemovePrincipalFromNamedValueSD (HKEY_LOCAL_MACHINE, 
                                             _T("Software\\Microsoft\\Ole"), 
                                             _T("DefaultLaunchPermission"), 
                                             tszPrincipal, 
                                             fPermit ? ACCESS_ALLOWED_ACE_TYPE : 
                                             ACCESS_DENIED_ACE_TYPE);

        if(dwReturnValue != ERROR_SUCCESS) return dwReturnValue;
        
        dwReturnValue = 
            UpdatePrincipalInNamedValueSD(HKEY_LOCAL_MACHINE, 
                                          _T("Software\\Microsoft\\Ole"), 
                                          _T("DefaultLaunchPermission"), 
                                          tszPrincipal, 
                                          dwAccessMask, 
                                          TRUE, 
                                          fPermit ? ACCESS_DENIED_ACE_TYPE : 
                                          ACCESS_ALLOWED_ACE_TYPE);

        if(dwReturnValue != ERROR_SUCCESS) return dwReturnValue;

        dwReturnValue = 
            AddPrincipalToNamedValueSD (HKEY_LOCAL_MACHINE, 
                                        _T("Software\\Microsoft\\Ole"), 
                                        _T("DefaultLaunchPermission"), 
                                        tszPrincipal, 
                                        fPermit, 
                                        dwAccessMask,
                                        SDTYPE_DEFAULT_LAUNCH);
    } 
    else
    {
        dwReturnValue =  
            RemovePrincipalFromNamedValueSD (HKEY_LOCAL_MACHINE, 
                                             _T("Software\\Microsoft\\Ole"), 
                                             _T("DefaultLaunchPermission"), 
                                             tszPrincipal, 
                                             ACE_TYPE_ALL);
    }

    return dwReturnValue;


}

/*---------------------------------------------------------------------------*\
 * NAME: ChangeAppIDAccessACL                                                *
 * --------------------------------------------------------------------------*
 * DESCRIPTION: Modifies an AppID's access ACL. The                          *
 * system uses the AppID access ACL to determine if a principal is           *
 * allowed to access the COM server associated with the AppID.               *
 * --------------------------------------------------------------------------*
 *  ARGUEMENTS:                                                              *
 *                                                                           *
 *  tszAppID - The Application ID you wish to modify                         *
 *  (e.g. "{99999999-9999-9999-9999-00AA00BBF7C7}")                          *
 *                                                                           *
 *  tszPrincipal - Name of user or group (e.g. "redmond\johndoe")            *
 *                                                                           *
 *  fSetPrincipal - TRUE if you want to add/update the principal's entry     *
 *  in the ACL, FALSE if you want to remove the principal from the ACL       *
 *                                                                           *
 *  fPermit - TRUE if you want to allow the principal to access the object,  *
 *  FALSE if you want to prevent the principal from accessing the object     *
 *                                                                           *
 *  dwAccessMask - Specifies what type of access the principal is to be      *
 *  given.  Can be any combination of the COM_RIGHTS_* values                *
 *                                                                           *
 * --------------------------------------------------------------------------*
 * RETURNS: WIN32 Error Code                                                 *
\*---------------------------------------------------------------------------*/
DWORD ChangeAppIDAccessACL (
    LPTSTR tszAppID,
    LPTSTR tszPrincipal,
    BOOL fSetPrincipal,
    BOOL fPermit,
    DWORD dwAccessMask
    )
{
    TCHAR   tszKeyName [256] = {0};
    DWORD dwReturnValue = ERROR_SUCCESS;

    _stprintf_s (tszKeyName, 
                 RTL_NUMBER_OF(tszKeyName),
                 tszAppID [0] == '{' ? _T("APPID\\%s") : _T("APPID\\{%s}"), 
                 tszAppID);

    if (fSetPrincipal)
    {
        dwReturnValue = 
            RemovePrincipalFromNamedValueSD (HKEY_CLASSES_ROOT, 
                                             tszKeyName, 
                                             _T("AccessPermission"), 
                                             tszPrincipal, 
                                             fPermit ? ACCESS_ALLOWED_ACE_TYPE : 
                                             ACCESS_DENIED_ACE_TYPE);

        if(dwReturnValue != ERROR_SUCCESS) return dwReturnValue;
        
        dwReturnValue = 
            UpdatePrincipalInNamedValueSD(HKEY_CLASSES_ROOT, 
                                          tszKeyName, 
                                          _T("AccessPermission"), 
                                          tszPrincipal, 
                                          dwAccessMask, 
                                          TRUE, 
                                          fPermit ? ACCESS_DENIED_ACE_TYPE : 
                                          ACCESS_ALLOWED_ACE_TYPE);

        if(dwReturnValue != ERROR_SUCCESS) return dwReturnValue;

        dwReturnValue = 
            AddPrincipalToNamedValueSD (HKEY_CLASSES_ROOT, 
                                        tszKeyName, 
                                        _T("AccessPermission"), 
                                        tszPrincipal, 
                                        fPermit, 
                                        dwAccessMask,
                                        SDTYPE_APPLICATION_ACCESS);
    } 
    else
    {
        dwReturnValue =  
            RemovePrincipalFromNamedValueSD (HKEY_CLASSES_ROOT, 
                                             tszKeyName, 
                                             _T("AccessPermission"), 
                                             tszPrincipal, 
                                             ACE_TYPE_ALL);
    }

    return dwReturnValue;
}

/*---------------------------------------------------------------------------*\
 * NAME: ChangeAppIDLaunchAndActivateACL                                     *
 * --------------------------------------------------------------------------*
 * DESCRIPTION: Modifies an AppID's launch access control list.              *
 * The system uses the AppID launch ACL to determine if a principal          *
 * (a user or group of users) is allowed to launch the COM server            *
 * associated with the AppID.                                                *
 * --------------------------------------------------------------------------*
 *  ARGUMENTS:                                                               *
 *                                                                           *
 *  tszAppID - The Application ID you wish to modify                         *
 *  (e.g. "{99999999-9999-9999-9999-00AA00BBF7C7}")                          *
 *                                                                           *
 *  tszPrincipal - Name of user or group (e.g. "redmond\johndoe")            *
 *                                                                           *
 *  fSetPrincipal - TRUE if you want to add/update the principal's entry     *
 *  in the ACL, FALSE if you want to remove the principal from the ACL       *
 *                                                                           *
 *  fPermit - TRUE if you want to allow the principal to launch the object   *
 *  FALSE if you want to prevent the principal from launching the object     *
 *                                                                           *
 *  dwAccessMask - Specifies what type of access the principal is to be      *
 *  given.  Can be any combination of the COM_RIGHTS_* values                *
 *                                                                           *
 * --------------------------------------------------------------------------*
 * RETURNS: WIN32 Error Code                                                 *
\*---------------------------------------------------------------------------*/
DWORD ChangeAppIDLaunchAndActivateACL (
    LPTSTR tszAppID,
    LPTSTR tszPrincipal,
    BOOL fSetPrincipal,
    BOOL fPermit,
    DWORD dwAccessMask
    )
{
    TCHAR   tszKeyName [256] = {0};
    DWORD dwReturnValue = ERROR_SUCCESS;

    _stprintf_s (tszKeyName, 
                 RTL_NUMBER_OF(tszKeyName),
                 tszAppID [0] == '{' ? _T("APPID\\%s") : _T("APPID\\{%s}"), 
                 tszAppID);

    if (fSetPrincipal)
    {
        dwReturnValue = 
            RemovePrincipalFromNamedValueSD (HKEY_CLASSES_ROOT, 
                                             tszKeyName, 
                                             _T("LaunchPermission"), 
                                             tszPrincipal, 
                                             fPermit ? ACCESS_ALLOWED_ACE_TYPE : 
                                             ACCESS_DENIED_ACE_TYPE);

        if(dwReturnValue != ERROR_SUCCESS) return dwReturnValue;
        
        dwReturnValue = 
            UpdatePrincipalInNamedValueSD(HKEY_CLASSES_ROOT, 
                                          tszKeyName, 
                                          _T("LaunchPermission"), 
                                          tszPrincipal, 
                                          dwAccessMask, 
                                          TRUE, 
                                          fPermit ? ACCESS_DENIED_ACE_TYPE : 
                                          ACCESS_ALLOWED_ACE_TYPE);

        if(dwReturnValue != ERROR_SUCCESS) return dwReturnValue;

        dwReturnValue = 
            AddPrincipalToNamedValueSD (HKEY_CLASSES_ROOT, 
                                        tszKeyName, 
                                        _T("LaunchPermission"), 
                                        tszPrincipal, 
                                        fPermit, 
                                        dwAccessMask,
                                        SDTYPE_APPLICATION_LAUNCH);
    } 
    else
    {
        dwReturnValue =  
            RemovePrincipalFromNamedValueSD (HKEY_CLASSES_ROOT, 
                                             tszKeyName, 
                                             _T("LaunchPermission"), 
                                             tszPrincipal, 
                                             ACE_TYPE_ALL);
    }

    return dwReturnValue;
}
