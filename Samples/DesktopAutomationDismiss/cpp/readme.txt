Desktop Automation
=============================
Automating desktop applications may require an automated dismissal of the Start menu. This sample shows how to create 
a program that can be used as part of automation to dismiss the Start menu and apps. This binary should 
be run only for automated testing, and should be ran prior to starting an automation run.

In order for this program to work, you must give it UIAccess privileges. See "Special build instructions"

Special build instructions
=============================
These steps are meant only for test machines running UI automation. It will require the creation of a self-signed root
 certificate

1. Make sure that the UIAccess is requested in the application manifest. The sample already has this set, and can be 
   changed under Solution Explorer -> Properties -> Linker -> Manifest Tool -> UAC Bypass UI Protection.

2. Create a self-signed root authority certificate and export the private key. Use the MakeCert tool to do this.
   For example, the following command creates a self-signed certificate with a subject name of "CN=TempCA." 
     makecert -n "CN=TempCA" -r -sv TempCA.pvk TempCA.cer

   You will be prompted to provide a password to protect the private key. This password is required when creating a certificate 
   signed by this root certificate.

   It's important to keep this PVK in a safe place! Binaries signed with this certificate can circumvent a number
   of security restrictions.

3. Add the self-signed certificate to the trusted root store on the machines running automation. CertUtil can be used to do this.
   For example, run this command with administrator privileges:

   certutil -addstore root TempCA.cer

4. Create a certificate signed by the self-signed root authority certificate. Again, use the MakeCert tool.
   For example, the following command creates a certificate signed by the TempCA root authority certificate with a 
   subject name of "CN=SignedByCA" using the private key of the issuer.
      makecert -sk SignedByCA -iv TempCA.pvk -n "CN=SignedByCA" -ic TempCA.cer SignedByCA.cer -sr currentuser -ss My

5. Sign the created binary(in this example, "DesktopAutomation.exe"). SignTool can do this. For example, the following command
   signs "DesktopAutomation.exe"
     signtool sign /f SignedByCA.cer DesktopAutomation.exe

6. Install the binary to "Program Files" or "Windows" directories

Troubleshooting
=============================
Q: The sample fails with error code 5 (ERROR_ACCESS_DENIED)
A: The sample is failing to get UIAccess privileges.
   a. Make sure the executable is placed in “Windows” or “Program Files” directories
   b. Make sure that UIAccess=true is set in the application manifest (see step 1)
   c. If you have disabled UAC (LUA) via group policy, re-enable it. Set it to "Never Notify" mode if you must run your tests with UAC disabled.
      i. Set HKLM\SOFTWARE\Microsoft\Windows\CurrentVersion\Policies\System\EnableLUA = 1 to turn UAC on
      ii. Set HKLM\SOFTWARE\Microsoft\Windows\CurrentVersion\Policies\System\ConsentPromptBehaviorAdmin = 0 to turn on "Never Notify" mode.

Q: When running the sample, a dialog appears with “A referral was returned from the server”
A: The executable is not signed with a certificate, or the certificate does not chain to a trusted root authority
   See steps 2-5