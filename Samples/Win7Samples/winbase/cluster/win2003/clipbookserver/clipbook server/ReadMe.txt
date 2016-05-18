========================================================================
       CLUSTER RESOURCE DLL : ClipBook Server (ClipBook Server)
========================================================================


AppWizard has created this ClipBook Server resource DLL for you.  This
DLL not only demonstrates the basics of using the Cluster API but is
also a starting point for writing your DLL.

This file contains a summary of what you will find in each of the files
that make up your ClipBook Server resource DLL.

ClipBook Server.cpp
    This is the main DLL source file.  It contains implementations for
    all required resource DLL entry points along with some helper
    functions.

ClipBook Server.rc
    This is a listing of all of the Microsoft Windows resources that
    the program uses.  This file can be directly edited in Microsoft
    Developer Studio.

ClipBook Server.def
    This file contains information about the DLL that must be
    provided to run with the cluster software.  It defines parameters
    such as the name and description of the DLL.  It also exports
    functions from the DLL.

/////////////////////////////////////////////////////////////////////////////
Adding your resource type to the cluster:

To add your resource type to the cluster, use cluster.exe.

    cluster resourcetype "ClipBook Server" /create /dllname:"ClipBook Server.dll"

If you don't specify a path to your resource DLL, Cluster Server will search
for it first in the cluster directory and then on the system path.

You can now create resources of type "ClipBook Server" using either
Cluster Administrator or cluster.exe.  To use Cluster Administrator to
create resources, create a Cluster Administrator Extension DLL for your
resource.  This is highly recommended.  To create a resource using cluster.exe,
use a command like the following:

    cluster resource "My ClipBook Server" /create /group:"My Group" /type:"ClipBook Server"

/////////////////////////////////////////////////////////////////////////////
Bringing your resource online:

This sample controls the ClipBook service which depends on Network DDE and
Network DDE DSDM services in Windows Server 2003.  Before the resource can start
the ClipBook service all three services need to be set to "manual" startup
This can be done through the Services MMC Snap-in (found under Control Panel's
Administrative Tools directory).

/////////////////////////////////////////////////////////////////////////////
Other notes:

"TODO:" indicates a part of the source code you should add to or customize.

"ADDPARAM:" indicate parts of the source code you should
modify if you add or remove parameters.

"NOTE:" indicates a point of interest in the source code.

/////////////////////////////////////////////////////////////////////////////
