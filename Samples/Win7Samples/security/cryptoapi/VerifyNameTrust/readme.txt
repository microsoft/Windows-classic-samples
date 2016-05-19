To use VerifyNameTrust code samples with test signed files:

1) Install the Windows Software Development Kit, available from:
   http://msdn.microsoft.com/en-us/windowsvista/bb980924.aspx
   Be sure to include: "Developer Tools > Windows Development Tools > Win32 development tools"

2) Build a chain of test certificates placed in the appropriate certificate stores:

> makecert.exe -pe -r -n "cn=Contoso Root Authority" -ss root -a sha1 contosoroot.cer
> makecert.exe -pe -n "cn=Contoso intermediate" -ss ca -a sha1 -in "Contoso Root Authority" -is root -cy authority contosointermediate.cer
> makecert.exe -pe -n "cn=Contoso, o=Contoso, l=Redmond, s=Washington, c=US" -ss my -a sha1 -in "Contoso intermediate" -is ca -cy end  -eku 1.3.6.1.5.5.7.3.3 contoso.cer


3) Update VerifyNameTrust.cpp in VerifyNameTrust directory so that RootKeyList matches the root certificate Key ID Hash(sha1)

The included  contosoroot.cer sample produces the following results:

> certutil -v contosoroot.cer
...
Key Id Hash(rfc-sha1): 25 f9 24 45 74 bc 03 89 4b 2c 5f ee 92 1b 95 5a e7 a4 9e 76
...

4) Update VerifyNameTrust.cpp in VerifyNameTrust directory so that PublisherNameList matches the publisher certificate subject name fields. 

The included contoso.cer sample produces the following results:

> certutil -v contoso.cer
  X509 Certificate:
  ...
  Subject:
      CN=Contoso
      O=Contoso
      L=Redmond
      S=Washington
      C=US

5) Test sign a sample executable file:

> signtool.exe sign /v /n Contoso /s my targetfile.exe

6) Build updated VerifyNameTrust samples

	To build the samples using Visual Studio 2005 (preferred method):
	================================================
	     1. In the File menu, select Open -> Project / Solution
	     2. Navigate to and select .sln file of target sample
	     3. In the Build menu, select Build Solution. The command line application will be built in the default \Debug or \Release directory.

	To build the samples using Visual Studio 2008:
	================================================
	     1. In the File menu, select Open -> Project / Solution
	     2. Navigate to and select .sln file of target sample
	     3. Click Next in Visual Studio Conversion Wizard, optionally create a backup, click finish and close
	     4. In the Build menu, select Build Solution. The command line application will be built in the default \Debug or \Release directory.

	To build with another compiler:
	================================================
	     Be sure to link crypt32.lib and wintrust.lib

7) Verify test signed file via VerifyNameTrust

> VerifyNameTrust.exe targetfile.exe