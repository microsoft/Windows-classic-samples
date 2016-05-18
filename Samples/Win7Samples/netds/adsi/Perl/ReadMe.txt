//+-------------------------------------------------------------------------
//
//  THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
//  ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
//  THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
//  PARTICULAR PURPOSE.
//
//  Copyright (c) Microsoft Corporation. All rights reserved.
//
//  Perl Samples: Using ADSI with Perl
//
//--------------------------------------------------------------------------

Description
===========
The three sample Perl scripts demonstrate how to use ADSI with the Perl
scripting language.
  * Binding.Pl shows how to bind to a Directory Service object.
  * Create.Pl shows how to create and delete objects.
  * Modify.Pl shows how to retrieve and set properties of an object,
    including both single and multivalue properties.

These samples use the LDAP: provider and are suitable for Windows 2000 and
later networks running Active Directory.

Sample Files
============
  *  Binding.Pl
  *  Create.Pl
  *  Modify.Pl

Running the Sample
==================
To run these samples, you must install a script engine for the Perl language.

To run one of the samples
  1.  Open a command window and change to the directory that contains the
      Perl scripts.
  2.  Make appropriate changes to a script using a text editor before running.
      Details for each script appear in the following How the Sample Works
      section.
  3.  At the command prompt, type the command
        cscript <script>.pl
      where <script> is one of Binding, Create, or Modify.

How the Sample Works
====================
Each Perl script needs modification for appropriate domain, user name,
password, and so forth before you run it.  The individual scripts and
the necessary changes are as follows.

  Binding.Pl
    The Binding script uses the LDAP: provider to bind to a domain using
    credentials for a specified user.  To specify the domain and user, change
    the following line appropriately.
      $myDSObject = $ldapNameSpace->OpenDSObject("LDAP://myServer/DC=MyDomain,DC=Com",
                     "cn=MyUser,cn=users,dc=MyDomain,dc=com", "UserPassword", 1);
  Create.Pl
    The Create script creates then deletes a new user in a domain using the
    credentials for a specified user.  To specify the domain and user, change
    the following line appropriately.
      $myDSObject = $ldapNameSpace->OpenDSObject("LDAP://myServer/DC=MyDomain,DC=Com",
                     "cn=MyUser,cn=users,dc=MyDomain,dc=com", "UserPassword", 1);
    The script creates a user named "LdapTestUser"; change the following lines if you
    want to create a different user.
      $newObj = $myDSObject->create("User", "cn=LdapTestUser");
      -and-
      $newObj->put("samAccountName", "LdapTestUser");
    If you change the name of the user to create, you'll also need to change the
    following line so that the user is deleted.
      $myDSObject->delete("User", "cn=LdapTestUser");

  Modify.Pl
    The Modify script creates (and eventually deletes) a new user using the
    credentials for a specified user and then illustrates how to get and set
    properties of that new user.  You'll need to make the same changes that
    you made for the Create.Pl script to adapt the script to your situation.

See Also
========
LDAP ADsPath
