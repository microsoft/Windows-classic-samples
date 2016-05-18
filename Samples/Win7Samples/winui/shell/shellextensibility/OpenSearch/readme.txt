OpenSearch Sample
=====================
Demonstrates a custom federated search provider with Visual Studio 2008 that connects to the AdventureWorks database. Uses ASP.NET to create a custom federated search provider. Queries are federated from Windows to the search provider using the OpenSearch standard. Results from this federated search provider are enumerated in the Windows Explorer as rich items in the view. Windows 7 understands search results that are returned in the RSS format. Other systems that support OpenSearch are compatible with Windows 7 federated search providers like Microsoft Search Server 2008 and other RSS-enabled clients. 

Sample Language Implementations
===============================
C#

Prerequisites:
=============================================
This sample requires Visual Studio 2008 with the Web Development features installed including ASP.NET 2.0.

This sample requires Microsoft SQL Server 2005 Express edition.
   
This sample requires the Microsoft SQL Server 2005 sample databases AdventureWorks sample database (adventureworksDB.msi) which can be found here:
	http://www.codeplex.com/MSFTDBProdSamples/Release/ProjectReleases.aspx?ReleaseId=4004

Information on how to install this sample database can be found here:
	http://msdn.microsoft.com/en-us/library/aa992075.aspx


To build the sample using Visual Studio 2008 (preferred method):
===========================================================
     1. Open Windows Explorer and navigate to the OpenSearch directory.
     2. Double-click the icon for the AdventureSearch.sln file to open the file in Visual Studio.
     3. In the Build menu, select Build Website.

To run the sample:
=================
     1. Run the ASP.NET debugging web server by running the project in Visual Studio by selecting Debug Website in the debugging menu. Make sure default.aspx is opened.
     2. Open AdventureSearch.osdx using Windows Explorer on Windows 7
     3. Search for "touring" in the Windows Explorer search box


