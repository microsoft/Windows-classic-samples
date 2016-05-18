Transacted File Sample
======================

This sample demonstrates how to perform a transacted file operation inside the scope of a distributed transaction.
The transaction is created using the DTC interfaces directly.

In a single transaction the sample opens a text file, or creates it if it does not exist, and appends the current
local time stamp to it. A command line parameter is used to configure the sample to commit or 
abort the transaction. If the transaction commits, all changes are made permanent. If the transaction aborts, 
all changes are rolled back.



Requirements
============
1. DTC service must be running.

Note: 	Read Component Services MMC Snapin help documentation for instructions on how to configure DTC
	You can see the Component services MMC Snapin documentation by following these steps:
	1. Run 'dcomcnfg' at the command line
	2. Click 'Help' from the menu, Click 'Help topics'



Sample Source Files
===================
Readme.txt		This file
TransactedFile.cpp	Main program
TransactedFile.sln	Visual Studio Solution File
TransactedFile.vcproj	Visual C Project File



To build the sample
===================

1. Copy the sample files to a seperate folder
2. Run the following command at the command line in the same folder as the sample files
	vcbuild /platform:<str>
   where <str> is x64 or Win32 indicating the platform for which you want to build



To clean the sample
===================

1. Run the following command at the command line in the same folder as the sample files
	vcbuild /clean



Usage
=====
Usage of the sample executable is as below:

    TransactedFile.exe [-abort]
	-abort
	Aborts the transaction at the end. Otherwise by default the
	transaction will be comitted.
	-help
	Displays the usage information.



To run the sample
=================

1. Run TransactedFile.exe from the command line with or without using the '-abort' parameter.
   If '-abort' is not specified, the sample will 
 	create a new file named "test.txt" if it doesn't exist and append the current time to it,
	and add a new row to the 'jobs' table of the 'pubs' database.
   If '-abort' is specified, then sample will abort the transaction.
2. Observe the contents of "test.txt"

Run the sample a few times with or without the -abort option and observe the contents of 'test.txt'.



Supported Platforms
===================
Windows Vista
Windows Server 2008
