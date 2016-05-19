///////////////////////////////////////////////////////////////////////////////
//
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//
///////////////////////////////////////////////////////////////////////////////
//
//  Sample to demonstrate how to create a simple certificate request
//  using CertEnroll classes.
//
//  NOTE: This sample requires Visual Studio 2005. Create a project and
//  in the menu click on Project -> Add Reference...
//  this will pop a dialog. Click on the COM tab
//  Select 'CertEnroll 1.0 Type Library' and click OK.
//
//  This will create an interop library which will be used by the C# code.
//
///////////////////////////////////////////////////////////////////////////////

using System;
using System.Collections.Generic;
using System.Text;

    //  Add the CertEnroll namespace
using CERTENROLLLib;

namespace CreateSimpleCertRequest
{
    class CreateSimpleCertRequest
    {
        static void Main()
        {
                //  Create all the objects that will be required
            CX509CertificateRequestPkcs10 p10 = new CX509CertificateRequestPkcs10Class();
            CX509PrivateKey pri = new CX509PrivateKeyClass();
            CCspInformation csp = new CCspInformationClass();
            CCspInformations csps = new CCspInformationsClass();
            CX500DistinguishedName dn = new CX500DistinguishedNameClass();

            string base64p10;

                //  Initialize the csp object using the desired Cryptograhic Service Provider (CSP)
            csp.InitializeFromName("Microsoft Enhanced Cryptographic Provider v1.0");

                //  Add this CSP object to the CSP collection object
            csps.Add(csp);

                //  Provide key container name, key length and key spec to the private key object
            pri.ContainerName = "PutYourContainerName";
            pri.Length = 1024;
            pri.KeySpec = X509KeySpec.XCN_AT_KEYEXCHANGE;

                //  Provide the CSP collection object (in this case containing only 1 CSP object)
                //  to the private key object
            pri.CspInformations = csps;

                //  Create the actual key pair
            pri.Create();

                //  Encode the name in using the Distinguished Name object
            dn.Encode("CN=YourName", X500NameFlags.XCN_CERT_NAME_STR_NONE);

                //  Initialize the PKCS#10 certificate request object based on the private key.
                //  Using the context, indicate that this is a user certificate request and don't
                //  provide a template name
            p10.InitializeFromPrivateKey(X509CertificateEnrollmentContext.ContextUser, pri, "");

                //  The newly created certificate request object will contain some default extensions.
                //  Suppress these defaults by setting the SuppressDefaults flag
            p10.SuppressDefaults = true;

                //  Assing the subject name by using the Distinguished Name object initialized above
            p10.Subject = dn;

                //  Encode the certificate request
            p10.Encode();

                //  Get the certificate request in form of a base 64 encoded string
            base64p10 = p10.get_RawData(EncodingType.XCN_CRYPT_STRING_BASE64);

                //  print the certificate request on the console
            Console.Write(base64p10);
            Console.ReadKey();

            return;
        }
    }
}
