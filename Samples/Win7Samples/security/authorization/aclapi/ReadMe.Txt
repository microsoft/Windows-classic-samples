
This module illustrates Acl management API for Windows NT 4.0.

Developers of new software which is to run on Windows NT version 4.0
and above are encouraged to utilize these security API rather than
implementing code which uses lower level security API.  The development
and test time can be drastically reduced by utilizing these new API.

This sample illustrates this point by implementing code which modifies
the security on an existing file using the Windows NT 4.0
Acl management API.


The following new API functions are illustrated in this sample:

GetNamedSecurityInfo()
BuildExplicitAccessWithName()
SetEntriesInAcl()
SetNamedSecurityInfo()

The following lower-level security API would have been used to achieve
the same result:

LookupAccountName()
InitializeSecurityDescriptor()
InitializeAcl()
GetSecurityDescriptorDacl()
GetAclInformation()
GetAce()
SetSecurityDescriptorDacl()
AddAce()
AddAccessAllowedAce() / AddAccessDeniedAce()
GetFileSecurity()
SetFileSecurity()

Less code and less complex code is required to achieve this task using
the new Windows NT 4.0 Acl management API.
