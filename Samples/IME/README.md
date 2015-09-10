Input Method Editor (IME) sample
================================

This sample shows how to create an Input Method Editor (IME) that works in Windows Store apps and Windows 8.1 desktop apps.

The sample IME has the following features:

-   Uses the Text Services Framework (TSF)
-   Runs in base trust
-   Compatible with Windows Store apps
-   Compatible with Systray and desktop
-   Interacts with touch keyboard
-   Integrates with Search contract
-   Interacts with light-dismiss

The IME sample uses the following code to obtain the parent window:

`pView->GetWnd(&parentWndHandle);`

This implementation works only for Windows Store apps that use the built-in edit controls. This implementation won't work if the app uses the custom edit control from the subset of Text Service Framework (TSF) APIs available in the Windows Runtime. To ensure that the IME gets the proper parent window, so that the owned window is set correctly and works for desktop and Windows Store apps, replace the previous code with the following code:

`        if (FAILED(pView->GetWnd(&parentWndHandle)) || (parentWndHandle == nullptr))         {             parentWndHandle = GetFocus();         }`

To get a copy of Windows, go to [Downloads and tools](http://go.microsoft.com/fwlink/p/?linkid=301696).

To get a copy of Visual Studio, go to [Visual Studio Downloads](http://go.microsoft.com/fwlink/p/?linkid=301697).

Related topics
--------------

[Guidelines and checklist for IME development](http://go.microsoft.com/fwlink/p/?linkid=262401)

Related technologies
--------------------

[Text Services Framework](http://go.microsoft.com/fwlink/p/?linkid=262402)

Operating system requirements
-----------------------------

Client

Windows 8.1

Server

Windows Server 2012 R2

Build the sample
----------------

1.  Start Visual Studio and select **File** \> **Open** \> **Project/Solution**.

2.  Go to the directory named for the sample, and double-click the Microsoft Visual Studio Solution (.sln) file titled SampleIME.sln.

3.  Press F7 (or F6 for Visual Studio 2013) or use **Build** \> **Build Solution** to build the sample.

Run the sample
--------------

If you build the IME sample by using Visual Studio 2013, create an installation experience for the IME by using a third-party installer that supports Windows 8.1, like InstallShield from Flexera Software.

The following steps show how to use InstallShield to create a setup project for your IME DLL.

-   Install Visual Studio 2013.
-   Start Visual Studio 2013.
-   On the **File** menu, point to **New** and select **Project**. The **New Project** dialog opens.
-   In the left pane, navigate to **Templates \> Other Project Types \> Setup and Deployment**, click **Enable InstallShield Limited Edition**, and click **OK**. Follow the installation instructions.
-   Restart Visual Studio 2013.
-   Open the IME solution (.sln) file.
-   Press F6 to build the solution.
-   In Solution Explorer, right-click the solution, point to **Add**, and select **New Project**. The **Add New Project** dialog opens.
-   In the left tree view control, navigate to **Templates \> Other Project Types \> InstallShield Limited Edition**.
-   In the center window, click **InstallShield Limited Edition Project**.
-   In the **Name** text box, type "SetupIME" and click **OK**.
-   In the **Project Assistant** dialog, click **Application Information**.
-   Fill in your company name and the other fields.
-   Click **Application Files**.
-   In the left pane, right-click the **[INSTALLDIR]** folder, and select **New Folder**. Name the folder "Plugins".
-   Click **Add Files**. Navigate to SampleIME.dll, which is in the C++\\Debug folder, and add it to the **Plugins** folder. Repeat this step for the IME dictionary, which is in the C++\\SampleIME\\Dictionary folder.
-   Right-click the IME DLL and select **Properties**. The **Properties** dialog opens.
-   In the **Properties** dialog, click the **COM & .NET Settings** tab.
-   Under **Registration Type**, select **Self-registration** and click **OK**.
-   Build the solution. The IME DLL is built, and InstallShield creates a setup.exe file that enables users to install your IME on Windows 8.1. The setup.exe file is located in the SetupIME\\SetupIME\\Express\\DVD-5\\DiskImages\\DISK1 folder.

