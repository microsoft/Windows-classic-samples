To use VerifyKeyTrust code samples with test signed files:

1) Install the Windows Software Development Kit, available from:
   http://msdn.microsoft.com/en-us/windowsvista/bb980924.aspx
   Be sure to include: "Developer Tools > Windows Development Tools > Win32 development tools"

2) Build a chain of test certificates placed in the appropriate certificate stores:

> makecert.exe -pe -r -n "cn=Contoso Root Authority" -ss root -a sha1 contosoroot.cer
> makecert.exe -pe -n "cn=Contoso intermediate" -ss ca -a sha1 -in "Contoso Root Authority" -is root -cy authority contosointermediate.cer
> makecert.exe -pe -n "cn=Contoso, o=Contoso, l=Redmond, s=Washington, c=US" -ss my -a sha1 -in "Contoso intermediate" -is ca -cy end  -eku 1.3.6.1.5.5.7.3.3 contoso.cer


3) Update VerifyKeyTrust.cpp in VerifyKeyTrust directory so that PublisherKeyList matches the Key ID Hash(sha1) value of publisher certificate.

The included contoso.cer sample produces the following results:

> certutil -v contoso.cer
  ...
  Key Id Hash(sha1): d9 48 12 73 f8 4e 89 90 64 47 bf 6b 85 5f b6 cb e2 3d 63 d6
  ...


4) Test sign a sample executable file:

> signtool.exe sign /v /n Contoso /s my targetfile.exe

5) Build updated VerifyKeyTrust sample

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

6) Verify test signed file via VerifyKeyTrust

> VerifyKeyTrust.exe targetfile.exe
