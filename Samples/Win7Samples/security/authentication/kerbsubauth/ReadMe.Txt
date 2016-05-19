Kerberos SubAuthentication Filter Sample

The Kerberos authentication package is the standard LSA authentication
package for Windows 2000.

Windows 2000 allows SubAuthentication DLLs to be used in conjunction
with the Kerberos authentication package.  A SubAuthentication DLL
allows the authentication and validation criteria stored in AD to be
replaced for particular subsystems that use the Kerberos authentication
package.  For instance, a particular server might supply a SubAuthentication
DLL that uses a different granularity of logon hours, and/or specifies
workstation restrictions in a different format.

All of this can be accomplished using a SubAuthentication filter DLL without
sacrificing use of the Active Directory (and losing its administration tools).

For more information regarding authentication services in Windows 2000,
please see the SDK documentation.

