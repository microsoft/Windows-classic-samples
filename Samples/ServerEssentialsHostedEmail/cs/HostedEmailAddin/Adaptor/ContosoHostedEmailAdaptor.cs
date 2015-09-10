//
//  <copyright file="ContosoHostedEmailAdaptor.cs" company="Microsoft">
//    Copyright (C) Microsoft. All rights reserved.
//  </copyright>
//

using System.Collections.Generic;
using Contoso.EmailService;
using Microsoft.WindowsServerSolutions.Administration.ObjectModel.Adorners;
using Microsoft.WindowsServerSolutions.HostedEmail;

namespace Contoso.HostedEmail.Adaptor
{
    public class ContosoHostedEmailAdaptor : IHostedEmailAdaptor
    {
        #region

        private MockEmailService EmailService
        {
            get
            {
                return MockEmailService.SingleInstance;
            }
        }

        private LogManager logManager = LogManager.SingleInstance;

        public ContosoHostedEmailAdaptor()
        {
        }


        private bool IsActivated
        {
            get
            {
                return !string.IsNullOrEmpty(CredentialManager.AdminUserName) && !string.IsNullOrEmpty(CredentialManager.AdminPassword);
            }
        }
        #endregion

        #region IHostedEmailAdaptor
        public void Activate(string admin, string password, IDictionary<string, string> extendedParameters)
        {
            lock (this)
            {
                if (IsActivated)
                {
                    logManager.Log("Already activated");
                    throw new HostedEmailAdaptorException(HostedEmailAdaptorErrorCode.Custom, new AddinErrorRecord()
                     {
                         ErrorCode = 0,
                         Message = Resources.ErrMsg_AlreadyActivated,
                         Title = Resources.ErrTitle_AlreadyActivated,
                     });
                }
                logManager.Log("admin: {0}, password should not be logged", admin);
                if (!EmailService.VerifyAdminAccount(admin, password))
                {
                    throw new HostedEmailAdaptorException(HostedEmailAdaptorErrorCode.AuthenticationFailure, null);
                }
                CredentialManager.ClearAll();
                CredentialManager.AdminUserName = admin;
                CredentialManager.AdminPassword = password;
            }
        }

        public void Deactivate(IDictionary<string, string> extendedParameters)
        {
            lock (this)
            {
                CredentialManager.ClearAll();
            }
        }

        private bool connected = false;
        public void Connect()
        {
            if (!IsActivated)
            {
                connected = false;
            }
            connected = EmailService.Logon(CredentialManager.AdminUserName, CredentialManager.AdminPassword);
        }

        public void Disconnect()
        {
            EmailService.LogOff(CredentialManager.AdminUserName);
            connected = false;
        }

        public bool Connected
        {
            get
            {
                return connected;
            }
        }

        public void ResetAdminCredential(string admin, string password, IDictionary<string, string> extendedParameters)
        {
            if (!EmailService.VerifyAdminAccount(admin, password))
            {
                throw new HostedEmailAdaptorException(HostedEmailAdaptorErrorCode.AuthenticationFailure, null);
            }
            logManager.Log("ResetAdminCredential to admin: {0}", admin);
            CredentialManager.AdminUserName = admin;
            CredentialManager.AdminPassword = password;
        }

        public EmailAccountInfo CreateAccount(EmailAccountInfo info, string password)
        {
            try
            {
                return EmailService.AddAccount(info);
            }
            catch (AccountExistsException)
            {
                throw new HostedEmailAdaptorException(HostedEmailAdaptorErrorCode.AccountAlreadyExists, new AddinErrorRecord()
                {
                    Message = Resources.ErrMsg_OnlineUserAlreadyExist,
                    Title = Resources.ErrTitle_OnlineUserAlreadyExist,
                });
            }
        }

        public void DeleteAccount(string accountId)
        {
            try
            {
                EmailService.RemoveAccount(accountId);
            }
            catch (AccountNotExistsException)
            {
                throw new HostedEmailAdaptorException(HostedEmailAdaptorErrorCode.AccountNotExists, new AddinErrorRecord()
                {
                    Message = Resources.ErrMsg_OnlineUserNotExist,
                    Title = Resources.ErrTitle_OnlineUserNotExist,
                });
            }
        }

        public void DisableAccount(string accountId)
        {
            try
            {
                EmailService.DisableAccount(accountId);
            }
            catch (AccountNotExistsException)
            {
                throw new HostedEmailAdaptorException(HostedEmailAdaptorErrorCode.AccountNotExists, new AddinErrorRecord()
                {
                    Message = Resources.ErrMsg_OnlineUserNotExist,
                    Title = Resources.ErrTitle_OnlineUserNotExist,
                });
            }
        }

        public void EnableAccount(string accountId)
        {
            try
            {
                EmailService.EnableAccount(accountId);
            }
            catch (AccountNotExistsException)
            {
                throw new HostedEmailAdaptorException(HostedEmailAdaptorErrorCode.AccountNotExists, new AddinErrorRecord()
                {
                    Message = Resources.ErrMsg_OnlineUserNotExist,
                    Title = Resources.ErrTitle_OnlineUserNotExist,
                });
            }
        }

        public EmailAccountInfo GetAccount(string accountId)
        {
            try
            {
                return EmailService.GetAccountById(accountId);
            }
            catch (AccountNotExistsException)
            {
                throw new HostedEmailAdaptorException(HostedEmailAdaptorErrorCode.AccountNotExists, new AddinErrorRecord()
                {
                    Message = Resources.ErrMsg_OnlineUserNotExist,
                    Title = Resources.ErrTitle_OnlineUserNotExist,
                });
            }
        }

        public EmailAccountInfo[] GetAllAccounts()
        {
            return EmailService.GetAllAccounts();
        }

        public string[] GetDomains()
        {
            return EmailService.GetDomains();
        }

        public void ResetPassword(string accountId, string password)
        {
            try
            {
                EmailService.GetAccountById(accountId);
                EmailService.ResetPassword(accountId, password);
            }
            catch (AccountNotExistsException)
            {
                throw new HostedEmailAdaptorException(HostedEmailAdaptorErrorCode.AccountNotExists, new AddinErrorRecord()
                {
                    Message = Resources.ErrMsg_OnlineUserNotExist,
                    Title = Resources.ErrTitle_OnlineUserNotExist,
                });
            }
        }

        public void UpdateAccount(EmailAccountInfo info)
        {
            try
            {
                EmailService.UpdateAccount(info);
            }
            catch (AccountNotExistsException)
            {
                throw new HostedEmailAdaptorException(HostedEmailAdaptorErrorCode.AccountNotExists, null);
            }
            catch (AccountExistsException)
            {
                throw new HostedEmailAdaptorException(HostedEmailAdaptorErrorCode.AccountAlreadyExists, null);
            }
            catch (InvalidEmailAddressException)
            {
                throw new HostedEmailAdaptorException(HostedEmailAdaptorErrorCode.InvalidEmailAddress, null);
            }
        }
        #endregion
    }
}