//---------------------------------------------------------------------
//  This file is part of the Microsoft .NET Framework SDK Code Samples.
// 
//  Copyright (C) Microsoft Corporation.  All rights reserved.
// 
//This source code is intended only as a supplement to Microsoft
//Development Tools and/or on-line documentation.  See these other
//materials for detailed information regarding Microsoft code samples.
// 
//THIS CODE AND INFORMATION ARE PROVIDED AS IS WITHOUT WARRANTY OF ANY
//KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
//IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
//PARTICULAR PURPOSE.
//---------------------------------------------------------------------

using System;
using System.Collections.Generic;
using System.Text;
using CERTENROLLLib;

// Simple template based user enrollment
// Set Subject name and add friendly name
namespace EnrollSimpleUserCert
{
    class EnrollCertificate
    {
        // enroll a certificate based on given template name
        public static void EnrollCert(
            string templateName,
            string subjectName,
            string friendlyName)
        {
            // create a CX509Enrollment object
            // either from CX509EnrollmentClass or CX509Enrollment should work
            //CX509EnrollmentClass objEnroll = new CX509EnrollmentClass();
            CX509Enrollment objEnroll = new CX509Enrollment();

            // initialize the CX509Enrollment object
            objEnroll.InitializeFromTemplateName(
                X509CertificateEnrollmentContext.ContextUser,
                templateName);

            // set up the subject name
            //
            // first get the request
            IX509CertificateRequest iRequest = objEnroll.Request;

            // then get the inner PKCS10 request
            IX509CertificateRequest iInnerRequest = 
                iRequest.GetInnerRequest(InnerRequestLevel.LevelInnermost);
            IX509CertificateRequestPkcs10 iRequestPkcs10 = 
                iInnerRequest as IX509CertificateRequestPkcs10;

            // create CX500DistinguishedName
            CX500DistinguishedName objName = new CX500DistinguishedName();
            objName.Encode(subjectName, X500NameFlags.XCN_CERT_NAME_STR_NONE);

            // set up the subject name
            iRequestPkcs10.Subject = objName;

            // set up friendly name
            objEnroll.CertificateFriendlyName = friendlyName;

            // enroll for the certificate, which should install the certficate
            // in MY store if the certificate is successfully issued by CA
            objEnroll.Enroll();
        }
    }
}
