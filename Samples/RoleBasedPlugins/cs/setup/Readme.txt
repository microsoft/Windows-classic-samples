
Instructions to setup Management OData web service endpoint:

1. Copy the Microsoft.Samples.Management.OData.RoleBasedPlugins.dll to current folder. Or build the sample.

2. RBAC authorizes only those accounts which are provisioned in the RbacConfiguration.xml file. Current file has got provisioned two user accounts localAdmin and localNonAdmin. But the script is not going to add those user accounts to the machine. Please add them to the machine and use them to access service. If you do not want to add these user accounts, please add your current account in the RbacConfiguration.xml file.

2. Execute .\setupEndPoint.ps1

