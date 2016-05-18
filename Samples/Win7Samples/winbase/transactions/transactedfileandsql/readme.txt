Transacted File and SQL Sample
==============================

This sample demonstrates how to create a new transaction using the Distributed Transaction 
Coordinator (DTC) interfaces, and do a transacted file and a SQL database operation using 
that transaction.

The sample makes a connection to the database, creates a new transaction, and in the same transaction completes the 
database and transacted file operations. A command line parameter is used to configure the sample to commit or 
abort the transaction. If the transaction commits, all changes are made permanent. If the transaction aborts, 
all changes are rolled back.



Requirements
============
1. Any edition of 'SQL Server 2000 SP4' or 'SQL Server 2005'
   'pubs' database must exist in the database. A database script file (CreatePubsDB.SQL) is provided with this 
   sample to create the database and the necessary table in it.
2. DTC service must be running on the machine where you are running the sample.
3. DTC service must be running on the machine where you have the SQL server installed.
4. Both DTC services must be configured to allow Network DTC Access for inbound and outbound access.
5. If you have Windows Firewall enabled on either of these machines, you should create a new firewall 
   exception to allow all all network traffic for the MSDTC.exe executable.

Note: 	Read Component Services MMC Snapin help documentation for instructions on how to configure DTC
	You can see the Component services MMC Snapin documentation by following these steps:
	1. Run 'dcomcnfg' at the command line
	2. Click 'Help' from the menu, Click 'Help topics'

Note:	See Windows Firewall documentation for details on how to configure the firewall.




Sample Files
============
Readme.txt			This file
Makefile			Project make file
CreatePubsDB.SQL		SQL script file
TransactedFileandSQL.cpp	Main program
TransactedFileandSQL.sln	Visual Studio Solution File
TransactedFileandSQL.vcproj	Visual C Project File



Remarks
=======
The SQL server can be installed on the local machine or can be remote.

By default the local machine is used as the target SQL server, or you can use a remote server by 
indicating the server name in the '-server' command line parameter.

The sample adds a new row to the 'jobs' table of the 'pubs' database.
This database and the table is included in SQL Server 2000 by default, however it does not exist in
SQL Server 2005. If you don't have this database in your SQL server, you can use the 
CreatePubsDB.SQL script file to create the database.



To create the database
======================
If you don't have the 'pubs' database create it by running the following command at the command line:

	osql -E -I -i CreatePubsDB.sql



Usage
=====
TransactedFileAndSQL.exe [-server <servername>] [-abort]
	-server <servername>
	SQL server to connect to. Default is the local machine.
	
	-abort
	Aborts the transaction at the end. Otherwise by default the
	transaction will be comitted.
	
	-help
	Displays the usage information.



To Run the Sample
=================
1. Run TransactedFileAndSQL.exe from the command line with any of the command line parameters indicated above.
2. If '-abort' is not specified, the sample will 
 	create a new file named "test.txt" if it doesn't exist and append the current time to it,
	and add a new row to the 'jobs' table of the 'pubs' database.
3. Observe the contents of "test.txt" and the database table for updates.




Supported Platforms
===================
Windows Vista
Windows Server 2008