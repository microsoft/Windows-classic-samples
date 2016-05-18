Managing the Audit Status


SUMMARY
=======

The AUDIT sample demonstrates how to use the LSA security API to manage the
audit status on the local machine or a remote machine.

Querying the current audit status is demonstrated, in addition to changing
the audit state of an audit event type. Enabling and disabling all auditing
is also demonstrated.

MORE INFORMATION
================

When targetting a domain controller for an audit update operation, be sure
to target the primary domain controller for the domain. The audit settings
are replicated by the primary domain controller to each backup domain
controller as appropriate. The NetGetDCName function can be used to get the
primary domain controller computer name from a domain name.

This sample targets the local machine if no command line argument is
specified, or the machine specified on argv[1].

The sample relies on the NTSECAPI.H header file found in the Win32 SDK
MSTOOLS\SECURITY directory.
