Managing Account Privileges


The LSAPRIVS sample demonstrates how to use the LSA security API to manage
account privileges on the local or a remote machine.

When you target a domain controller for privilege update operations, be sure
to target the primary domain controller for the domain. The privilege settings
are replicated by the primary domain controller to each backup domain
controller as appropriate. The NetGetDCName function can be used to get the
primary domain controller computer name from a domain name.

Notes
-----

For a list of privilges, consult WINNT.H. The privilege
SE_ASSIGNPRIMARYTOKEN_NAME is of interest.

For a list of logon rights, which can also be assigned using this
sample code, consult NTSECAPI.H. The right SE_BATCH_LOGON_NAME is of interest.

You can use domain\account as argv[1]. For instance, mydomain\scott will
grant the privilege to the mydomain domain account scott.

The optional target machine is specified as argv[2], otherwise, the
account database is updated on the local machine.

The LSA API functions used by this sample are Unicode only.

Use LsaRemoveAccountRights to remove account rights.
