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

//This sample demonstrates how to use the Windows 7 new http protocol to 
//enroll a certificate by calling the IX509Enrollment2:CreateRequest, 
//ICertRequest3::SetCredential, ICertRequest3::Submit and 
//IX509Enrollment2::InstallResponse2 methods. The purpose of the call to
//the ICertRequest3::SetCredential is to set the authentication credential
//to enrollment server in the object pointed by the interface ICertRequest3.

//This sample does not support certificate authentication type

using System;
using System.Collections.Generic;
using System.Text;
using CERTENROLLLib;

namespace X509Enrollment
{
    class EnrollWithIX509EnrollmentHelper
    {
        static int Main(string[] args)
        {
            EnrollWithIX509EnrollmentHelper objEnrollWithHelper = null;

            objEnrollWithHelper = new EnrollWithIX509EnrollmentHelper();

            //Parse command line arguments
            if (!objEnrollWithHelper.ParseParams(args))
            {
                Usage();
                return 1;
            }

            //Enroll a certificate through http protocol
            if (!objEnrollWithHelper.enrollWithIX509EnrollmentHelper())
                return 1;

            return 0;
        }

        public bool enrollWithIX509EnrollmentHelper()
        {
            bool bRet = true;

            try
            {
                IX509EnrollmentPolicyServer objPolicyServer = null;
                IX509CertificateTemplates objTemplates = null;
                IX509CertificateTemplate objTemplate = null;

                IX509EnrollmentHelper objEnrollHelper = null;

                IX509Enrollment2 objEnroll2 = null;

                objPolicyServer = new CX509EnrollmentPolicyWebService();
                objPolicyServer.Initialize(
                                m_strPolicyServerUrl,
                                null,
                                m_PolicyServerAuthType,
                                true,
                                m_context);
                //This call sets authentication type and authentication credential
                //to policy server to the object referenced by objPolicyServer.
                //This call is necessary even for Kerberos authentication type.
                objPolicyServer.SetCredential(
                                0,
                                m_PolicyServerAuthType,
                                m_strPolicyServerUsername,
                                m_strPolicyServerPassword);

                objPolicyServer.LoadPolicy(X509EnrollmentPolicyLoadOption.LoadOptionDefault);
                objTemplates = objPolicyServer.GetTemplates();
                objTemplate = objTemplates.get_ItemByName(m_strTemplateName);

                //There is no need to cache credential for Kerberos authentication type
                if (m_EnrollmentServerAuthType == X509EnrollmentAuthFlags.X509AuthUsername)
                {
                    objEnrollHelper = new CX509EnrollmentHelper();
                    objEnrollHelper.Initialize(m_context);
                    //This call caches the authentication credential to
                    //enrollment server in Windows vault
                    objEnrollHelper.AddEnrollmentServer(
                                    m_strEnrollmentServerUrl,
                                    m_EnrollmentServerAuthType,
                                    m_strEnrollmentServerUsername,
                                    m_strEnrollmentServerPassword);
                }

                objEnroll2 = new CX509Enrollment();
                objEnroll2.InitializeFromTemplate(
                                m_context,
                                objPolicyServer,
                                objTemplate);
                //This call reads authentication cache to
                //enrollment server from Windows vault
                objEnroll2.Enroll();
            }
            catch (Exception e)
            {
                bRet = false;
                Console.WriteLine("Error: {0}", e.Message);
            }

            if (bRet)
                Console.WriteLine("Certificate enrollment succeeded.");
            else
                Console.WriteLine("Certificate enrollment failed.");

            return bRet;
        }

        public static void Usage()
        {
            Console.WriteLine("Usage:\n");

            Console.WriteLine("enrollWithIX509EnrollmentHelper.exe <-Param> <Value> \n");

            Console.WriteLine("-Param                    Value");
            Console.WriteLine("-Context                  User | Machine");
            Console.WriteLine("-TemplateName             Certificate template name");
            Console.WriteLine("-PolicyServerAuthType     Kerberos | UsernamePassword");
            Console.WriteLine("-PolicyServerUrl          Policy server URL");
            Console.WriteLine("-PolicyServerUsername     Username or auth cert hash for policy server authentication");
            Console.WriteLine("-PolicyServerPassword     Password for policy server authentication");
            Console.WriteLine("-EnrollmentServerAuthType Kerberos | UsernamePassword | Certificate");
            Console.WriteLine("-EnrollmentServerUrl      Enrollment server URL");
            Console.WriteLine("-EnrollmentServerUsername Username or auth cert hash for enrollment server authentication");
            Console.WriteLine("-EnrollmentServerPassword Password for enrollment server authentication \n");

            Console.Write("Example: \n");
            Console.Write("enrollWithIX509EnrollmentHelper.exe ");
            Console.Write("-Context User ");
            Console.Write("-TemplateName User ");
            Console.Write("-PolicyServerAuthType UsernamePassword ");
            Console.Write("-PolicyServerUrl https://policyservermachinename.sampledomain.sample.com/ADPolicyProvider_CEP_UsernamePassword/service.svc/CEP ");
            Console.Write("-PolicyServerUsername sampledomain\\sampleuser ");
            Console.Write("-PolicyServerPassword samplepassword ");
            Console.Write("-EnrollmentServerAuthType UsernamePassword ");
            Console.Write("-EnrollmentServerUrl https://enrollmentservermachinename.sampledomain.sample.com/CaName_CES_UsernamePassword/service.svc/CES ");
            Console.Write("-EnrollmentServerUsername sampledomain\\samleuser ");
            Console.Write("-EnrollmentServerPassword samplepassword \n");
        }

        public bool ParseParams(string[] args)
        {
             try
            {
                for (int i = 0; i < args.Length; i += 2)
                {
                    if (!args[i].StartsWith("-") || (i + 1 == args.Length))
                        return false;

                    if (args[i].Equals("-Context"))
                    {
                        if (args[i + 1].Equals("User"))
                            m_context = X509CertificateEnrollmentContext.ContextUser;
                        else if (args[i + 1].Equals("Machine"))
                            m_context = X509CertificateEnrollmentContext.ContextAdministratorForceMachine;
                        else
                            return false;
                    }
                    else if (args[i].Equals("-TemplateName"))
                        m_strTemplateName = args[i + 1];
                    else if (args[i].Equals("-PolicyServerAuthType"))
                    {
                        if (args[i + 1].Equals("Kerberos"))
                            m_PolicyServerAuthType = X509EnrollmentAuthFlags.X509AuthKerberos;
                        else if (args[i + 1].Equals("UsernamePassword"))
                            m_PolicyServerAuthType = X509EnrollmentAuthFlags.X509AuthUsername;
                        else
                            return false;
                    }
                    else if (args[i].Equals("-PolicyServerUrl"))
                        m_strPolicyServerUrl = args[i + 1];
                    else if (args[i].Equals("-PolicyServerUsername"))
                        m_strPolicyServerUsername = args[i + 1];
                    else if (args[i].Equals("-PolicyServerPassword"))
                        m_strPolicyServerPassword = args[i + 1];
                    else if (args[i].Equals("-EnrollmentServerAuthType"))
                    {
                        if (args[i + 1].Equals("Kerberos"))
                            m_EnrollmentServerAuthType = X509EnrollmentAuthFlags.X509AuthKerberos;
                        else if (args[i + 1].Equals("UsernamePassword"))
                            m_EnrollmentServerAuthType = X509EnrollmentAuthFlags.X509AuthUsername;
                        else
                            return false;
                    }
                    else if (args[i].Equals("-EnrollmentServerUrl"))
                        m_strEnrollmentServerUrl = args[i + 1];
                    else if (args[i].Equals("-EnrollmentServerUsername"))
                        m_strEnrollmentServerUsername = args[i + 1];
                    else if (args[i].Equals("-EnrollmentServerPassword"))
                        m_strEnrollmentServerPassword = args[i + 1];
                    else
                        return false;
                }
             }
            catch(System.ArgumentException e)
            {
                Console.WriteLine("Error: {0}", e.Message);
                return false;
            }

            //Check if necessary members were set
            if (m_strTemplateName == null                                      ||
                m_PolicyServerAuthType == X509EnrollmentAuthFlags.X509AuthNone ||
                m_strPolicyServerUrl == null                                   ||
                m_strEnrollmentServerUrl == null)
                return false;

            //Set enrollment server to the same auth type as policy server
            //if enrollment server auth type was not passed
            if (m_EnrollmentServerAuthType == X509EnrollmentAuthFlags.X509AuthNone)
            {
                m_EnrollmentServerAuthType = m_PolicyServerAuthType;
                m_strEnrollmentServerUsername = m_strPolicyServerUsername;
                m_strEnrollmentServerPassword = m_strPolicyServerPassword;
            }

            return true;
        }

        private X509CertificateEnrollmentContext m_context = X509CertificateEnrollmentContext.ContextUser;
        private string m_strTemplateName = null;
        private X509EnrollmentAuthFlags m_PolicyServerAuthType = X509EnrollmentAuthFlags.X509AuthNone;
        private string m_strPolicyServerUrl = null;
        private string m_strPolicyServerUsername = null;
        private string m_strPolicyServerPassword = null;
        private X509EnrollmentAuthFlags m_EnrollmentServerAuthType = X509EnrollmentAuthFlags.X509AuthNone;
        private string m_strEnrollmentServerUrl = null;
        private string m_strEnrollmentServerUsername = null;
        private string m_strEnrollmentServerPassword = null;
    }
}
