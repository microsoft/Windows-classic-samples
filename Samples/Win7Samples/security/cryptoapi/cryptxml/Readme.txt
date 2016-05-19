Copyright (c) Microsoft Corporation. All rights reserved.

Using CryptXML API to create and verify XML signatures

This sample code shows how to create and verify XML signatures using X.509 certificates.
For the purposes of the sample, we look for a test certificate in the user's personal certificate store and build a chain for it.

=======================================================
This example illustrates the use of the following APIs:

- CryptXmlOpenToDecode
- CryptXmlVerifySignature
- CryptXmlOpenToEncode
- CryptXmlCreateReference
- CryptXmlSign
- CryptXmlEncode
- CryptXmlDigestReference
- CryptXmlGetStatus
- CryptXmlFindAlgorithmInfo


===================
Build requirements:

To run this sample, compile and link it with CryptXml.lib, crypt32.lib, ncrypt32.lib, bcrypt32.lib.

======
Usage:

1. To verify XML signature:
    
	CryptXml.exe VERIFY File.xml

2. To create detached XML signature:

	CryptXml.exe SIGN FileOut.xml #Id1 file1 #id2 file2

3. To create enveloping XML signature on the element with Id=Order1 in fileIn.xml 
   and add it to the top <Signatures> element:

	CryptXml.exe SIGN FileOut.xml fileIn.xml /Signatures #Order1

4. To create enveloped XML signature on the fileIn.xml and add it to the top <Signatures> element:

	CryptXml.exe SIGN FileOut.xml fileIn.xml /Signatures #

=====================
Advanced information:

To create a test certificate used to sign XML document, 
makecert.exe tool can be used.
1 ) Be sure that the following Windows SDK component is installed: 
  "Developer Tools > Windows Development Tools > Win32 development tools"

2) Build a chain of test certificates placed in the appropriate certificate stores:

    makecert.exe -pe -r -n "cn=Contoso Root Authority" -ss root -a sha1 contosoroot.cer
    makecert.exe -pe -n "cn=Contoso intermediate" -ss ca -a sha1 -in "Contoso Root Authority" -is root -cy authority contosointermediate.cer
    makecert.exe -pe -n "cn=Contoso" -ss my -a sha1 -in "Contoso intermediate" -is ca -cy end  -eku 1.3.6.1.4.1.311.10.3.12 contoso.cer

3) Use the following command line syntax to sign a target file:

   IMPORTANT: Create backup of target file signed, as any failure will cause file content to be deleted.

   To create detached XML signature, use the following syntax: 

                CryptXml.exe -n Contoso SIGN FileOut.xml #ReferenceId1 file1 #ReferenceId2 file2

        Notes: 
                     FileOut.xml contains the signature element
                     #ReferenceId1 specifies the ID attribute of the reference element that digests file1
                     #ReferenceId2 specifies the ID attribute of the reference element that digests file2 

   To create enveloped XML signature on the element with Id=TargetID1 in fileIn.xml and 
   add the signature element as a child of the element specified by the /Signatures XPath location, use:  
                
                CryptXml.exe -n Contoso SIGN FileOut.xml fileIn.xml /Signatures #TargetID1

        Notes: 
                     FileOut.xml contains the XML nodes from Filein.xml and the new signature element
                     #TargetID1 specifies the ID attribute of the element to be referenced 
                     /Signatures is an example XPath location path specifying the parent element of the signature element. This parameter is case sensitive.

   To create enveloped XML signature on the element with Id=TargetID1 in fileIn.xml and 
   add the signature element as a child of the element where ID = SignatureLocation, use the following syntax:  
                
                CryptXml.exe -n Contoso SIGN FileOut.xml fileIn.xml #SignatureLocation #TargetID1

        Notes: 
                     FileOut.xml contains the XML nodes from Filein.xml and the new Signature element
                     #TargetID1 specifies the ID attribute of the element to be referenced 
                     #SignatureLocation is the ID attribute of the parent element that will contain the Signature element

   To create enveloped XML signature on the fileIn.xml and  add the signature element as a child 
   of the element specified by the /Signatures XPath location, use:

                CryptXml.exe -n Contoso SIGN FileOut.xml fileIn.xml /Signatures #

        Notes: 
                    Use of ‘#’ string sets the signature’s Reference URI attribute to ""

4) Use the following command line syntax to verify either detached or enveloped signatures:

                 IMPORTANT: Create backup of target file, as any failure will cause file content to be deleted.

                CryptXml.exe VERIFY File.xml
