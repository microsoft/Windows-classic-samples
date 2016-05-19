//+-------------------------------------------------------------------------
//
//  THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
//  ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
//  THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
//  PARTICULAR PURPOSE.
//
//  Copyright (c) Microsoft Corporation. All rights reserved.
//
//  DistQuery SQL Sample: ADSI Query from SQL
//
//--------------------------------------------------------------------------

Description
===========
The DistQuery SQL sample illustrates how to call ADSI using SQL Server.
It creates an employee performace review table, adds records, and then
joins the two tables before printing out a report.

This sample uses the LDAP: provider and is suitable for Windows 2000 and
later networks running Active Directory.

Sample Files
============
  *  Dq.Sql

Running the Sample
==================
This sample requires SQL Server 7.0 with Service Pack 2 or later.  You must
install the ADSI Runtime on the SQL Server computer.

To run this sample
  1.  Open the file Dq.Sql in a text editor such as Notepad.
  2.  Run the Query Analyzer by selecting in the Start menu:
      Programs, Microsoft SQL Server 7.0.
  3.  Logon to the SQL Server computer.
  4.  One at a time, Cut and Paste the SQL commands, substituting appropriate
      domains and users for those given in the file.  The following examples
      assume the domain is Fabrikam.Com and the user is the Administrator.

How the Sample Works
====================
The Dq.Sql file contains the following commands.

  1.  These commands tell SQL Server to associate the word "ADSI" with the
      ADSI OLE DB Provider, "ADSDSOObject".

        sp_addlinkedserver 'ADSI', 'Active Directory Services 2.5',
                           'ADSDSOObject', 'adsdatasource'
        go

  2.  This command queries Active Directory for all users in the Fabrikam.Com
      domain from SQL Server.

        SELECT * FROM OpenQuery( ADSI,'<LDAP://DC=fabrikam,DC=com>;
                                 (objectClass=user);adspath;subtree')
        -or-
        '-- Using the ADSI SQL Dialect
        SELECT * FROM OpenQuery( ADSI, 'SELECT name, adsPath
                                        FROM ''LDAP://DC=Fabrikam,DC=com''
                                        WHERE objectCategory = ''Person''
                                          AND objectClass= ''user''')

  3.  The following commands first create and then execute a view.
      Only the view definition is stored in SQL Server, not the actual result
      set.  Hence, you may get a different result when you execute a view
      again later.

        '-- Create the view
        CREATE VIEW viewADUsers AS
        SELECT *
        FROM OpenQuery( ADSI,'<LDAP://DC=Fabrikam,DC=com>;
                        (&(objectCategory=Person)(objectClass=user));
                        name, adspath;subtree')
        '-- Execute the view
        SELECT * from viewADUsers

  4.  The following commands create a heterogeneous Join between the SQL
      Server and Active Directory.

        '-- Create an employee performance review table
        CREATE TABLE EMP_REVIEW
        (
        userName varChar(40),
        reviewDate datetime,
        rating decimal 
        )
        '-- Insert two records
        INSERT EMP_REVIEW VALUES('Administrator', '2/15/1998', 4.5 )
        INSERT EMP_REVIEW VALUES('Administrator', '7/15/1998', 4.0 )
        '-- Now join the view and the review table
        SELECT ADsPath, userName, ReviewDate, Rating 
        FROM EMP_REVIEW, viewADUsers
        WHERE userName = Name
        '-- Create a report for the join
        CREATE VIEW reviewReport
        SELECT ADsPath, userName, ReviewDate, Rating 
        FROM EMP_REVIEW, viewADUsers
        WHERE userName = Name

  5.  These commands allow you to logon as a different user when connecting
      to Active Directory by specifying alternative credentials.

        '-- Maps 'FABRIKAM\Administrator' to the OLE DB user name
        '-- (in this case the Active Directory user
        '-- 'CN=Administrator,CN=Users,DC=fabrikam,DC=com')
        sp_addlinkedsrvlogin ADSI, false, 'FABRIKAM\Administrator',
          'CN=Administrator,CN=Users,DC=fabrikam,DC=com', 'passwordHere'
        go

  6.  This command stops using the alternative credentials.

        sp_droplinkedsrvlogin ADSI,'FABRIKAM\Administrator'

