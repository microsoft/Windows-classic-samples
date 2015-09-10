Desktop Automation sample
=========================

Automating desktop applications may require programmatic dismissal of the Start menu. This sample shows how to create a program that can be used as part of automation to dismiss the Start menu and apps. This binary should be run only for automated testing, and should be run prior to starting an automation run.

**Note**  This sample requires Microsoft Visual Studio Ultimate 2013 or later to build.

**Note**  The Windows-classic-samples repo contains a variety of code samples that exercise the various programming models, platforms, features, and components available in Windows and/or Windows Server. This repo provides a Visual Studio solution (SLN) file for each sample, along with the source files, assets, resources, and metadata needed to compile and run the sample. For more info about the programming models, platforms, languages, and APIs demonstrated in these samples, check out the documentation on the [Windows Dev Center](https://dev.windows.com). This sample is provided as-is in order to indicate or demonstrate the functionality of the programming models and feature APIs for Windows and/or Windows Server. This sample was created for Windows 8.1 and/or Windows Server 2012 R2 using Visual Studio 2013, but in many cases it will run unaltered using later versions. This sample was created for Windows 8.1 and/or Windows Server 2012 R2 using Visual Studio 2013, but in many cases it will run unaltered using later versions. Please provide feedback on this sample!

To get a copy of Windows, go to [Downloads and tools](http://go.microsoft.com/fwlink/p/?linkid=301696).

To get a copy of Visual Studio, go to [Visual Studio Downloads](http://go.microsoft.com/fwlink/p/?linkid=301697).

Related topics
--------------

[Using MakeCert](http://msdn.microsoft.com/en-us/library/windows/desktop/aa388165)

Operating system requirements
-----------------------------

Client

Windows 8.1

Server

Windows Server 2012 R2

Build the sample
----------------

These steps are meant only for test machines running UI automation. It will require the creation of a self-signed root certificate.

1.  Make sure that the UIAccess is requested in the application manifest. The sample already has this set, and can be changed under **Solution Explorer -\> Properties -\> Linker -\> Manifest Tool -\> UAC Bypass UI Protection**.
2.  Create a self-signed root authority certificate and export the private key. Use the MakeCert tool to do this. For example, the following command creates a self-signed certificate with a subject name of "CN=TempCA": **makecert -n "CN=TempCA" -r -sv TempCA.pvk TempCA.cer** You will be prompted to provide a password to protect the private key. This password is required when creating a certificate signed by this root certificate. It's important to keep this PVK in a safe place! Binaries signed with this certificate can circumvent a number of security restrictions.
3.  Add the self-signed certificate to the trusted root store on the machines running automation. CertUtil can be used to do this. For example, run this command with administrator privileges: **certutil -addstore root TempCA.cer**
4.  Create a certificate signed by the self-signed root authority certificate. Again, use the MakeCert tool. For example, the following command creates a certificate signed by the TempCA root authority certificate with a subject name of "CN=SignedByCA" using the private key of the issuer: **makecert -sk SignedByCA -iv TempCA.pvk -n "CN=SignedByCA" -ic TempCA.cer SignedByCA.cer -sr currentuser -ss MyCertStore**
5.  Sign the created binary(in this example, DesktopAutomation.exe). SignTool can do this. For example, the following command signs **DesktopAutomation.exe**: **signtool sign /f SignedByCA.cer DesktopAutomation.exe**
6.  Install the binary to the Program Files or Windows directory.

Run the sample
--------------

Troubleshooting
---------------

Q: The sample fails with error code 5 (**ERROR\_ACCESS\_DENIED**)

A: The sample is failing to get UIAccess privileges.

-   Make sure the executable is placed in either the Program Files or Windows directory.
-   Make sure that UIAccess=true is set in the application manifest (see step 1).
-   If you have disabled UAC (LUA) via group policy, re-enable it. Set it to "Never Notify" mode if you must run your tests with UAC disabled. Set "HKLM\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Policies\\System\\EnableLUA = 1" to turn UAC on. Set "HKLM\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Policies\\System\\ConsentPromptBehaviorAdmin = 0" to turn on "Never Notify" mode.

Q: When running the sample, a dialog appears with "A referral was returned from the server".

A: The executable is not signed with a certificate, or the certificate does not chain to a trusted root authority See steps 2-5 above under Building the Sample.

