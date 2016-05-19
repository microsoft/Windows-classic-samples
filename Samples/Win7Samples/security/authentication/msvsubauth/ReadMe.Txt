MSV1_0 SubAuthentication Sample

The MSV1_0 authentication package is the standard LSA authentication
package for Windows NT.  It provides or supports:

  Authentication of users in the SAM database.
  Pass-Thru authentication of users in trusted domains.

Windows NT allows SubAuthentication DLLs to be used in conjunction
with the MSV1_0 authentication package.  A SubAuthentication DLL
allows the authentication and validation criteria stored in SAM to be
replaced for particular subsystems that use the MSV1_0 authentication
package.  For instance, a particular server might supply a SubAuthentication
DLL that validates a user’s password via a different algorithm, uses a
different granularity of logon hours, and/or specifies workstation restrictions
in a different format.

All of this can be accomplished using SubAuthentication DLLs without
sacrificing use of the SAM database (and losing its administration tools) or
losing pass-thru authentication.

For more information regarding authentication services in Windows NT and
Windows 2000, please see the SDK documentation.

