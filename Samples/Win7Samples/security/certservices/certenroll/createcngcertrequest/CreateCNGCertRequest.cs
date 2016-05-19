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
//  Sample to demonstrate how to create a CNG (Suite-B based) simple 
//  certificate request using CertEnroll classes.
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

namespace CreateCNGCertRequest
{
    class Program
    {
        static void Main()
        {
                //  Create all the objects that will be required
            CX509CertificateRequestPkcs10 p10 = new CX509CertificateRequestPkcs10Class();
            CX509PrivateKey pri = new CX509PrivateKeyClass();
            CX500DistinguishedName dn = new CX500DistinguishedNameClass();
            CObjectId objecc = new CObjectIdClass();
            CObjectId objhash = new CObjectId();

            string base64p10;

            //  Initialize the object ID class for the ECC algorithm (ECDSA_P256)
//            objecc.InitializeFromAlgorithmName(ObjectIdGroupId.XCN_CRYPT_PUBKEY_ALG_OID_GROUP_ID, ObjectIdPublicKeyFlags.XCN_CRYPT_OID_INFO_PUBKEY_ANY, "ECDSA_P256");
            objecc.InitializeFromAlgorithmName(ObjectIdGroupId.XCN_CRYPT_PUBKEY_ALG_OID_GROUP_ID, ObjectIdPublicKeyFlags.XCN_CRYPT_OID_INFO_PUBKEY_ANY, AlgorithmFlags.AlgorithmFlagsNone, "ECDSA_P256");

            //  Initialize the object ID class for the hashing algorithm (SHA384)
            objhash.InitializeFromAlgorithmName(ObjectIdGroupId.XCN_CRYPT_HASH_ALG_OID_GROUP_ID, ObjectIdPublicKeyFlags.XCN_CRYPT_OID_INFO_PUBKEY_ANY, AlgorithmFlags.AlgorithmFlagsNone, "SHA384");

                //  Provide provider name and the object id to the private key object
            pri.ProviderName = "Microsoft Software Key Storage Provider";
            pri.Algorithm = objecc;

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

            p10.HashAlgorithm = objhash;

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
