//
//  <copyright file="MockEmailService.cs" company="Microsoft">
//    Copyright (C) Microsoft. All rights reserved.
//  </copyright>
//

using Microsoft.WindowsServerSolutions.HostedEmail;
using System;
using System.IO;
using System.Reflection;
using System.Text.RegularExpressions;
using System.Threading;

namespace Contoso.EmailService
{
    public class MockEmailService
    {
        private LogManager logManager = LogManager.SingleInstance;

        #region Singleton
        private static object singletonLock = new object();
        private static MockEmailService singleInstance = null;
        public static MockEmailService SingleInstance
        {
            get
            {
                if (singleInstance == null)
                {
                    lock (singletonLock)
                    {
                        if (singleInstance == null)
                        {
                            singleInstance = new MockEmailService();
                        }
                    }
                }
                return singleInstance;
            }
        }

        private MockEmailService()
        {
        }

        #endregion

        #region Configuration
        private EmailServiceConfiguration config = null;
        private EmailServiceConfiguration Configuration
        {
            get
            {
                if (config == null)
                {
                    lock (this)
                    {
                        if (config == null)
                        {
                            string filePath = Constants.ConfigrationFilePath;
                            if (!File.Exists(filePath))
                            {
                                logManager.Log("Cannot find configuration file {0}, just create a configuration file use default settings", filePath);
                                File.WriteAllText(filePath, Resources.DefaultConfgiuration);
                            }
                            var schemaPath = Constants.ConfigurationSchemaPath;
                            if (!File.Exists(schemaPath))
                            {
                                logManager.Log("Cannot find configuration schema file {0}, just create it", schemaPath);
                                File.WriteAllText(schemaPath, Resources.ConfigurationSchema);
                            }
                            config = new EmailServiceConfiguration(filePath);
                        }
                    }
                }
                return config;
            }
        }
        private T SimulateOnlineCall<T>(string operationName, Func<T> func)
        {
            try
            {
                logManager.Log("[{0}] starts...", operationName);
                int duration = Configuration.GetOperationDuration(operationName);
                logManager.Log("operation duration is {0} seconds", duration);
                if (duration > 0) Thread.Sleep(duration * 1000);
                var exception = Configuration.GetOperationException(operationName);
                if (exception != null)
                {
                    logManager.Log("Arbitary Exception thrown");
                    throw exception;
                }
                var retVal = func();
                logManager.Log("[{0}] is done", operationName);
                return retVal;
            }
            catch (Exception e)
            {
                logManager.Log("ERROR: {0}", e.ToString());
                throw;
            }
        }

        private void SimulateOnlineCall(string operationName, Action action)
        {
            SimulateOnlineCall<bool>(operationName, () => { action(); return true; });
        }

        #endregion

        #region Admin Account Management
        public bool VerifyAdminAccount(string admin, string password)
        {
            return SimulateOnlineCall(MethodBase.GetCurrentMethod().Name, () =>
                {
                    // verify the admin account here
                    if (string.IsNullOrEmpty(admin) || string.IsNullOrEmpty(password))
                    {
                        logManager.Log("Administrator account is not valid.");
                        return false;
                    }
                    logManager.Log("Administrator account verified.");
                    return true;
                });
        }

        public bool Logon(string admin, string password)
        {
            return SimulateOnlineCall(MethodBase.GetCurrentMethod().Name, () =>
                {
                    // verify the admin account here
                    if (string.IsNullOrEmpty(admin) || string.IsNullOrEmpty(password))
                    {
                        logManager.Log("Administrator account is not valid.");
                        return false;
                    }
                    logManager.Log("Administrator account verified.");
                    return true;
                });
        }

        public void LogOff(string admin)
        {
            SimulateOnlineCall(MethodBase.GetCurrentMethod().Name, () =>
                {

                    if (string.IsNullOrEmpty(admin)) return;
                    logManager.Log("{0} logged off", admin);
                });
        }

        #endregion

        #region Email address validation
        // Please replace the following regex in a real case  
        private static readonly Regex emailAddressPattern = new Regex(@"^([\w\.\-]+)@([\w\-]+)((\.(\w){2,3})+)$", RegexOptions.Compiled | RegexOptions.CultureInvariant | RegexOptions.IgnoreCase);
        public void ValidateEmailAddress(string email)
        {
            // email domain might be checked as well but this is bypassed in this example
            if (string.IsNullOrWhiteSpace(email) || !emailAddressPattern.Match(email).Success)
            {
                throw new HostedEmailAdaptorException(HostedEmailAdaptorErrorCode.InvalidEmailAddress, null);
            }
        }
        #endregion


        #region Email Account Management
        private AccountManager accountManager = new AccountManager(Constants.AccountDataFilePath);
        public bool ContainsAccount(string accountId)
        {
            return SimulateOnlineCall(MethodBase.GetCurrentMethod().Name, () =>
                {
                    return accountManager.ContainsAccount(accountId);
                });
        }

        public bool ContainsPrimaryEmailAddress(string emailAddress)
        {
            return SimulateOnlineCall(MethodBase.GetCurrentMethod().Name, () =>
               {
                   return accountManager.ContainsPrimaryEmailAddress(emailAddress);
               });
        }

        public EmailAccountInfo AddAccount(EmailAccountInfo info)
        {
            if (info == null) throw new ArgumentNullException("info");
            return SimulateOnlineCall(MethodBase.GetCurrentMethod().Name, () =>
                {
                    ValidateEmailAddress(info.PrimaryEmailAddress);
                    if (info.AdditionalEmailAddresses != null)
                    {
                        foreach (string emailAddress in info.AdditionalEmailAddresses)
                        {
                            ValidateEmailAddress(emailAddress);
                        }  
                    }
                    return accountManager.AddAccount(info);
                });
        }

        public void UpdateAccount(EmailAccountInfo info)
        {
            if (info == null) throw new ArgumentNullException("info");
            if (info.PrimaryEmailAddress != null)
            {
                ValidateEmailAddress(info.PrimaryEmailAddress);
            }
            SimulateOnlineCall(MethodBase.GetCurrentMethod().Name, () =>
            {
                var original = accountManager.GetAccountById(info.AccountId);
                if (original == null) throw new AccountNotExistsException();
                EmailAccountInfo updated = null;
                if (info.ExtendedProperties == null)
                {// No extended properties to update, just copy the changed properties to original one
                    if (info.PrimaryEmailAddress != null) original.PrimaryEmailAddress = info.PrimaryEmailAddress;
                    if (info.DisplayName != null) original.DisplayName = info.DisplayName;
                    if (info.FirstName != null) original.FirstName = info.FirstName;
                    if (info.LastName != null) original.LastName = info.LastName;
                    updated = original;
                }
                else
                {// There is some extended properties to update, copy original properties to non-changed ones
                    if (info.PrimaryEmailAddress == null) info.PrimaryEmailAddress = original.PrimaryEmailAddress;
                    if (info.DisplayName == null) info.DisplayName = original.DisplayName;
                    if (info.FirstName == null) info.FirstName = original.FirstName;
                    if (info.LastName == null) info.LastName = original.LastName;
                    if (original.ExtendedProperties != null)
                    {
                        foreach (var pair in original.ExtendedProperties)
                        {
                            if (!info.ExtendedProperties.ContainsKey(pair.Key)) info.ExtendedProperties.Add(pair.Key, pair.Value);
                        }
                    }
                    // this should not be changed by this method, there are specific methods for enable/disable
                    info.Enabled = original.Enabled;
                    updated = info;
                }
                ValidateEmailAddress(info.PrimaryEmailAddress);
                if (info.AdditionalEmailAddresses != null)
                {
                    foreach (string emailAddress in info.AdditionalEmailAddresses)
                    {
                        ValidateEmailAddress(emailAddress);
                    }
                }
                accountManager.UpdateAccount(info);
            });
        }

        public EmailAccountInfo GetAccountById(string accountId)
        {
            return SimulateOnlineCall(MethodBase.GetCurrentMethod().Name, () =>
                {
                    return accountManager.GetAccountById(accountId);
                });
        }

        public EmailAccountInfo GetAccountByPrimaryEmail(string emailAddress)
        {
            return SimulateOnlineCall(MethodBase.GetCurrentMethod().Name, () =>
               {
                   return accountManager.GetAccountByPrimaryEmail(emailAddress);
               });
        }

        public void RemoveAccount(string accountId)
        {
            SimulateOnlineCall(MethodBase.GetCurrentMethod().Name, () =>
                {
                    accountManager.RemoveAccount(accountId);
                });
        }

        public void EnableAccount(string accountId)
        {
            SimulateOnlineCall(MethodBase.GetCurrentMethod().Name, () =>
            {
                accountManager.EnableAccount(accountId);
            });

        }

        public void DisableAccount(string accountId)
        {
            SimulateOnlineCall(MethodBase.GetCurrentMethod().Name, () =>
            {
                accountManager.DisableAccount(accountId);
            });

        }

        public EmailAccountInfo[] GetAllAccounts()
        {
            return SimulateOnlineCall(MethodBase.GetCurrentMethod().Name, () =>
            {
                return accountManager.GetAllAccounts();
            });

        }

        public void ResetPassword(string accountId, string password)
        {
            SimulateOnlineCall(MethodBase.GetCurrentMethod().Name, () => { });
        }

        #endregion

        public string[] GetDomains()
        {
            return SimulateOnlineCall(MethodBase.GetCurrentMethod().Name, () =>
            {
                //return (string[])null;
                return new string[]{"contoso.com", "contosocontoso.com", "contosocontoso1.com"};
            });
        }

        #region Extended features
        public DistributionGroup[] GetDistributionGroups()
        {
            return Configuration.DistributionGroups;
        }
        #endregion
    }
}
