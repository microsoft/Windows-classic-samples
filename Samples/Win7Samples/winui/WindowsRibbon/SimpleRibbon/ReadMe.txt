Windows Ribbon: SimpleRibbon Sample

Demonstrates the markup and code required to implement a very simple Win32 application with a Ribbon.


Noteworthy Files:
.\ReadMe.txt           - Describes the sample's purpose, and how to build and run the sample.
.\CPP\SimpleRibbon.sln - Visual Studio solution file.  Load this file to view and compile the sample.
.\CPP\SimpleRibbon.xml - Markup file containing the Ribbon UI command and layout definitions.
.\CPP\res\*.bmp        - Bitmap images used for the sample.


Prerequisites:
     1. Microsoft Windows operating system capable of displaying the Windows Ribbon (Windows 7; Vista support to be added)
     2. Microsoft Windows SDK v7.0 (minimum)
     3. Visual Studio 2008 (Express Edition supported)


Building the Sample:

To build the sample using the command prompt:
==============================================
     1. Open an SDK CMD Shell window and navigate to the .\CPP directory of the sample.
     2. Type msbuild SimpleRibbon.sln /P:Platform=win32


To build the sample using Visual Studio (preferred method):
==============================================
     1. Open Windows Explorer and navigate to the .\CPP directory of the sample.
     2. Double-click the icon for SimpleRibbon.sln (solution) file to open the file in Visual Studio.
     3. In the Build menu, select Build Solution. The application will be built in the "Debug" or "Release" directory.


To run the sample:
=================
     1. Navigate to the directory that contains the newly built executable, using the command prompt or Windows Explorer.
     2. Type SimpleRibbon.exe at the command line, or double-click on SimpleRibbon.exe to launch it from Windows Explorer.


Comments:
The source code contains comments inline that help to describe what the code is doing.  Read these for more information.


-----------------
Legal notice
Information in this document, including URL and other Internet Web site references, is subject to change without notice. Unless otherwise noted, the example companies, organizations, products, domain names, e-mail addresses, logos, people, places and events depicted herein are fictitious, and no association with any real company, organization, product, domain name, e-mail address, logo, person, place or event is intended or should be inferred. Complying with all applicable copyright laws is the responsibility of the user. Without limiting the rights under copyright, no part of this document may be reproduced, stored in or introduced into a retrieval system, or transmitted in any form or by any means (electronic, mechanical, photocopying, recording, or otherwise), or for any purpose, without the express written permission of Microsoft Corporation. 
Microsoft may have patents, patent applications, trademarks, copyrights, or other intellectual property rights covering subject matter in this document. Except as expressly provided in any written license agreement from Microsoft, the furnishing of this document does not give you any license to these patents, trademarks, copyrights, or other intellectual property.
© 2009 Microsoft Corporation. All rights reserved. 
Microsoft, MS-DOS, Windows, Windows NT, Windows Server, Windows Vista, Active Directory, ActiveSync, ActiveX, Direct3D, DirectDraw, DirectInput, DirectMusic, DirectPlay, DirectShow, DirectSound, DirectX, Expression, FrontPage, HighMAT, Internet Explorer, JScript, Microsoft Press, MSN, Outlook, PowerPoint, SideShow, Silverlight, Visual Basic, Visual C++, Visual InterDev, Visual J++, Visual Studio, WebTV, Windows Media, Win32, Win32s, and Zune are either registered trademarks or trademarks of Microsoft Corporation in the U.S.A. and/or other countries.
All other trademarks are property of their respective owners.
