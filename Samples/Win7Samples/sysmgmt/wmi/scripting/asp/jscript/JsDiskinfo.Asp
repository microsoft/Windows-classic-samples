<!- Copyright (c)  Microsoft Corporation->
<%@ LANGUAGE="JSCRIPT"%>
<html>
<!-------------------------------------------------------------------------
 
	This sample illustrates the use of the WMI Scripting API within
   an ASP, using JScript.  It displays information in a table
   for each disk on the local host.

   To run this sample, 

   1. Place it in a directory accessible to your web server
   2. If running on NT4, ensure that the registry value:-

        HKEY_LOCAL_MACHINE\Software\Microsoft\WBEM\Scripting\Enable for ASP

      is set to 1.

      However please consult the WMI SDK documentation for the other 
      affects of setting this value before proceeding.

      For Windows 2000, ensure that Anonymous Access is disabled and Windows
      Integrated Authentication is enabled for this file before running this 
      ASP (this can be done by configuring the file properties using the IIS 
      configuration snap-in).
 
---------------------------------------------------------------------------->

<head>
<title>WMI Scripting ASP Sample - Disk Information (JScript Version)</title>
</head>

<body>
<%
	// Connect to the default namespace (root/cimv2)
	// on the local host
	var Locator = new ActiveXObject ("WbemScripting.SWbemLocator");
	var Service = Locator.connectserver ();
	
	// Retrieve the logical disk class
	var Disk = Service.Get ("Win32_LogicalDisk");
	var Path = Disk.Path_;

	// Retrieve the Disk instances
	var Disks = new Enumerator (Disk.Instances_());
%>

<p>Path of object is <%=Path.DisplayName%></p>

<table BORDER="1">
  <tr>
    <th>Name</th>
    <th>Volume Name</th>
    <th>Volume Serial Number</th>
  </tr>
<%
	for (;!Disks.atEnd ();Disks.moveNext ())
	{
		var DiskInstance = Disks.item();
%>
  <tr>
    <td><%=DiskInstance.Name%>
</td>
    <td><%=DiskInstance.VolumeName%>
</td>
    <td><%=DiskInstance.VolumeSerialNumber%>
</td>
  </tr>
<%
	}
%>
</table>
</body>
</html>
