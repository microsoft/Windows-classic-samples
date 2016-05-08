===============================================================================
Windows RSS Platform - Screensaver Sample
===============================================================================
Last Updated: March 16, 2006

DESCRIPTION
============
This sample, written in C#, demonstrates the use of the Windows Really Simple 
Syndication (RSS) Platform to scan the Common Feed List for feeds that contain 
enclosed images. The sample demonstrates how to enumerate feeds, search for 
enclosures of the desired file type, and display images and text on the screen. 
The online documentation explains how to install and further customize the 
sample.

The installer creates a folder in "My Documents" called MSDN\RssScreenSaver 
with source code and documentation. Once the project is loaded into the Visual 
C# environment, you must compile the application before running it as your 
Windows screensaver.

When the screensaver is launched, it scans the Common Feed List for RSS feed 
items that contain enclosures (binary attachments) that can be displayed. Then 
it creates a full-screen Windows Form and displays data from the feeds. A timer 
is used to update the currently selected topic and change the background image. 
If the mouse is moved or clicked at any point, or if a key is pressed, the 
program exits immediately.


BROWSER/PLATFORM COMPATIBILITY
===============================
The sample runs on Windows XP Service Pack 2 and/or Windows Server 2003 with
Internet Explorer 7 installed, or on Windows Vista. Microsoft Visual Studio 
2005 with .NET Framework 2.0 is required to compile the application.


USAGE
======
To compile and run the sample, complete the following steps.

  1. Download and install the source files.

  2. Open the solution in Visual Studio 2005 and build the application.

  3. Copy ssNews.scr and Interop.Feeds.dll to the Windows\system32 directory.

  4. Configure the "News" screensaver from the Desktop Properties dialog.


SOURCE FILES
=============
Program.cs 
ScreenSaverForm.cs
OptionsForm.cs 
CommonFeedListUtils.cs 
FeedList.cs
RSSFeed.cs
RSSItem.cs
IItem.cs
ItemDescriptionView.cs
ItemListView.cs
FeedListView.cs

Other files
============
App.ico
DefaultRss.xml
SSaverBackground.jpg
SSaverBackground2.jpg 
Readme.txt


SEE ALSO
============
Windows RSS Platform on MSDN
http://msdn.microsoft.com/library/en-us/FeedsAPI/rss/overviews/msfeeds_ovw.asp

Screensaver Sample (Online Documentation)
http://msdn.microsoft.com/library/en-us/FeedsAPI/rss/howto/samp_screensaver.asp

Microsoft Team RSS Blog
http://blogs.msdn.com/rssteam/archive/2006/02/28/540319.aspx


=================================
Copyright © Microsoft Corporation


