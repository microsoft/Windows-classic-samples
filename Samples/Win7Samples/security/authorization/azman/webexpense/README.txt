Role-Based Access Control With Authorization Manager

This is a sample ASP.Net (C#) application that uses Authorization Manager.
Authorization Manager is a role-based access control framework which is 
included in Windows Vista, Windows Server 2003 and is available on Windows
2000 and Windows XP via web downloads at Microsoft Download Center.

This sample is a web expense application that will render a UI based upon the
user’s role membership.  Roles are defined for submitting expenses, approving
expenses and administering application settings.

This sample also contains a sample script (GetRoles.vbs) to enumerate a given
user's roles

System Requirements:

Windows Vista or Better

Additional requirements for using Active Directory as a Policy Store
Windows Server 2003 Active Directory
and
Domain Controller must be at Windows Server 2003 functionality level


Installation

1. Copy the WebExpense directory onto a web server.

2. If using XML as the as a policy store.  Copy AzStore.xml to the root of the
   C:/ drive, e.g. C:\AzStore.xml.  Skip steps 3-5 and go to step 6.
   Note: If you wish to change the location of the store make sure to open the
   Install.vbs file and change the following line:
    pAzManStore.Initialize 1+2, "msxml://C:\AzStore.xml"
   to match the desired path to the AzStore.xml file.

3. If you wish to use Active Directory or ADAM as a policy store.  Uncomment
   the line below the declaration of the variable: 
    HttpContext.Current.Appliction[“AZMAN_STORE”]
   which will contain a sample AD or ADAM connection string.  

4. Open the Install.vbs file and change the following line:
   pAzManStore.Initialize 1+2, "msxml://C:\AzStore.xml"
   to the from "msldap://<AzStore dn>". Make sure the specified container for
   the store exists in AD or ADAM (the store is created in the next step.)

5. Run the script file Install.vbs to create the policy store.

6. In IIS, browse to the WebExpense directory and open the properties window.

7. In IIS select the WebExpense folder (probably in the default web site) and
   right-click and select "convert to applicaion". For this Sample make sure
   the WebExpense applicaion uses the "Classic .Net Web Pool"

8. In order to use Integrated Authorization, open the Authentication settings 
   Icon for the application disable Anonymous Access and enable Windows
   Authentication (this may required installing Windows Authentication from
   the control panel Programs and Features manager under IIS, WWW, Security)

9. Open a web browser and browse to the directory where the sample was
   installed. If this is done on the local machine then you can use:
    http://localhost/WebExpense/index.aspx.

10. If the page loads without errors the sample is installed correctly.


Configuration

A user in the Administer role can modify the following properties through the
web expense application.  Note that these properties are reset when the
application is rebuilt or restarted.
* Max Transactions – Maximum number of expenses that can be submitted before
the application resets the transaction queue.
* Self Approval – If this is checked, users who are both a submitter and an
approver can approve their own expenses.

In addition, the policy store location and the method used to initialize the
client context can be modified in Global.asax.cs.


Troubleshooting

Using Active Directory as an Authorization Store:
1. In order to use Active Directory as an Authorization Store, you must raise
   your Domain Controller to Windows Server 2003 functionality level.  You
   cannot use Active Directory as an Authorization Store on Windows 2000 
   Server unless it is connected to a Windows Server 2003 native domain.

2. If the default permissions on the Active Directory domain have been changed
   in a way such that the security context of the application calling AzMan API
   does not have read access to user account object attributes, then LDAP
   queries used in this sample may fail. An example of this is when a domain
   administrator removes the Authenticated Users group from the
   “Pre-Windows 2000 compatibility Access” group. To enable the application
   account to query user attributes, make the application account a member of
   the “Windows Authorization Access” group or the 
   “Pre-Windows 2000 Compatibility Access” group.
   See Microsoft Knowledge Base article ID: 331951.



“Access to the path C:\WINDOWS\Microsoft.NET\Framework
                 \vx.x.xxx\Temporary ASP.NET Files\WebExpense\... is denied.”

This is due to the restricted permissions of IIS on a domain controller. 
On the first build of an ASP.Net project, files for the project must be created
in the Temporary ASP.NET Files folder which the IIS_WPG does not have write
access to by default.  To solve this give the IIS_WPG read/write permissions
for the Temporary ASP.NET Files folder.


If you experience problems with a newly created user, verify that the user has
the right to logon locally (if you test from a local account.)

Start-> Run -> Type in secpol.msc
In the “security settings” tree expand
 Local Policies
  User Rights Assignment
 
Then give the new user the “Allow Logon Locally” privilege.

If integrated authentication isn’t functioning properly, i.e. the new user is
prompted for a username and password when the application is opened, you 
should logoff and log on as the new user.

CS0234: The type or namespace name 'Interop' does not exist in the namespace
'Microsoft'
This may be because the Microsoft.Interop.Security.AzRoles.dll interop assembly
is not in the GAC. This can happen if you are using the 1.2.0.0 version of the
GAC. Due to a bug in WS03 SP1 the 1.2.0.0 version of the interop assembly is
not copied to the GAC. To fix this search on microsoft.com for a hotfix. Or use
the GacUtil.exe tool that is included in the .Net Framework SDK. 
The following command line will register the PIA into the GAC:
  GacUtil.exe -i Microsoft.Interop.Security.AzRoles.dll
