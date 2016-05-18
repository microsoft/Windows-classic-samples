/*++
THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED
TO THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
PARTICULAR PURPOSE.

Copyright (C) 1996 - 2002.  Microsoft Corporation.  All rights reserved.
*/

/*   
    Module: MYTOKEN.CPP

    When you log on to a Microsoft Windows NT based platform, NT generates
    an Access Token that describes who you are, what groups you belong to,
    and what privileges you have on that computer.

    The following sample code demonstrates how to extract this interesting
    information from the current thread/process access token. When I look
    at this information I tend to seperate it into three categories

        1. User identification and miscellaneous info
        2. Group information
        3. Privileges

    Notes about the group information:

        1) You will see a group sid with the form NONE_MAPPED. This is 
        the login SID generated for this particular logon session. It is 
        unique until the server is rebooted.

        2) Many of the group SIDS are well-known SIDs and RIDs. Consult the 
        documentation for information about these well-known Identifiers.

    Notes about the privileges information:

        The attributes number is simply a bit flag. 1 indicates that the 
        privilege is enabled and 2 indicates that the privilege is enabled
        by default. 3, of course, indicates that it is enabled by default
        and currently enabled.
  

    This code sample requires the following import libraries:       
  
        advapi32.lib
        user32.lib
*/
