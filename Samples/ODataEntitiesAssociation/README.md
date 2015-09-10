Association sample
==================

This sample demonstrates associations between OData entities in a Management OData server. It shows how to associate two entities with one another by editing the **Schema.mof** file, and how the association is mapped to the supporting cmdlets. It also demonstrates how a client can use JSON and standard OData URL conventions to send requests involving associations. Examples of requests involving associations include listing the set of associated entities, adding and removing entities from a reference set, and getting the details of an entity and its associated entities in a single network request.

To get a copy of Windows, go to [Downloads and tools](http://go.microsoft.com/fwlink/p/?linkid=301696).

To get a copy of Visual Studio, go to [Visual Studio Downloads](http://go.microsoft.com/fwlink/p/?linkid=301697).

Operating system requirements
-----------------------------

Client

Windows 8.1

Server

Windows Server 2012 R2

Build the sample
----------------

1.  Start Visual Studio and select **File** \> **Open** \> **Project/Solution**.

2.  Navigate to the sample folder, and open **AssociationClient.sln**

3.  Press F7 (or F6 for Visual Studio 2013) or use **Build** \> **Build Solution** to build the sample.

Run the sample
--------------

1. Setup Management OData endpoint by using the [PSWSRoleBasedPlugin](http://go.microsoft.com/fwlink/p/?linkid=243041) sample. 2. Copy **VMSystem.mof**, **VMSystem.xml**, **VMSystem.psm1**, **HostVMSystem.xml** into the virtual directory where you installed the endpoint. 3. Replace **web.config** in the virtual directory with the **web.config** file from this sample. 4. Add a **Module** element with the path to the **VMSystem.psm1** module as it's text nested within the **Modules** tag for the **AdminGroup** in the **RbacConfiguration.xml** file. 5. Make sure that the value of the **\$SystemFileName** variable in the **VMSystem.psm1** setup script points to the path of the **HostVMSystem.xml** file on the web server.

