//
//  <copyright file="AccountManager.cs" company="Microsoft">
//    Copyright (C) Microsoft. All rights reserved.
//  </copyright>
//

using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using Microsoft.WindowsServerSolutions.HostedEmail;

namespace Contoso.EmailService
{
    /// <summary>
    /// AccountManager provides ways to read/write data from disk data file without cache
    /// </summary>
    internal class AccountManager
    {
        private string accountsDataFilePath = null;

        public AccountManager(string datafilePath)
        {
            if (!File.Exists(datafilePath))
            {
                // create an empty data file
                CSVUtil.WriteToFile(datafilePath, null);
            }
            accountsDataFilePath = datafilePath;
        }

        public bool ContainsAccount(string accountId)
        {
            lock (this)
            {
                return LoadAccounts().ContainsKey(accountId);
            }
        }

        public bool ContainsPrimaryEmailAddress(string emailAddress)
        {
            lock (this)
            {
                if (string.IsNullOrEmpty(emailAddress)) return false;
                var accounts = LoadAccounts();
                foreach (EmailAccountInfo account in accounts.Values)
                {
                    if (string.Equals(account.PrimaryEmailAddress, emailAddress, StringComparison.OrdinalIgnoreCase))
                        return true;
                }
                return false;
            }
        }

        public EmailAccountInfo AddAccount(EmailAccountInfo info)
        {
            lock (this)
            {
                if (ContainsPrimaryEmailAddress(info.PrimaryEmailAddress))
                {
                    throw new AccountExistsException();
                }

                var accountId = Guid.NewGuid().ToString();
                info.AccountId = accountId;
                var accounts = LoadAccounts();
                accounts.Add(accountId, info);
                SaveAccounts(accounts);

                return info;
            }
        }

        public void UpdateAccount(EmailAccountInfo info)
        {
            lock (this)
            {
                var accounts = LoadAccounts();
                var accountId = info.AccountId;
                if (!accounts.ContainsKey(accountId))
                {
                    throw new AccountNotExistsException();
                }
                if (!string.IsNullOrEmpty(info.PrimaryEmailAddress))
                {
                    if (accounts.Values.Where((i) => { return string.Equals(i.PrimaryEmailAddress, info.PrimaryEmailAddress, StringComparison.OrdinalIgnoreCase) && !string.Equals(i.AccountId, info.AccountId, StringComparison.OrdinalIgnoreCase); }).Count() > 0)
                    {
                        throw new AccountExistsException();
                    }
                }
                accounts[accountId] = info;
                SaveAccounts(accounts);
            }
        }

        public EmailAccountInfo GetAccountById(string accountId)
        {
            lock (this)
            {
                var accounts = LoadAccounts();
                if (accounts.ContainsKey(accountId))
                {
                    var account = accounts[accountId];
                    LogManager.SingleInstance.Log("Account: {0}, Enabled: {1}", account.PrimaryEmailAddress, account.Enabled);
                    return accounts[accountId];
                }
                else return null;
            }
        }

        public EmailAccountInfo GetAccountByPrimaryEmail(string emailAddress)
        {
            lock (this)
            {
                return (from a in LoadAccounts().Values
                        where string.Equals(a.PrimaryEmailAddress, emailAddress, StringComparison.OrdinalIgnoreCase)
                        select a).FirstOrDefault();
            }
        }

        public void RemoveAccount(string accountId)
        {
            lock (this)
            {
                var accounts = LoadAccounts();
                if (!accounts.ContainsKey(accountId))
                {
                    throw new AccountNotExistsException();
                }
                accounts.Remove(accountId);
                SaveAccounts(accounts);
            }
        }

        public void EnableAccount(string accountId)
        {
            lock (this)
            {
                var accounts = LoadAccounts();
                if (!accounts.ContainsKey(accountId))
                {
                    //throw new HostedEmailAdaptorException(HostedEmailAdaptorErrorCode.AccountNotExists, new AddinErrorRecord()
                    //{
                    //    Message = Resources.ErrMsg_OnlineUserNotExist,
                    //    Title = Resources.ErrTitle_OnlineUserNotExist,
                    //});
                    throw new AccountNotExistsException();
                }

                accounts[accountId].Enabled = true;
                SaveAccounts(accounts);
            }
        }

        public void DisableAccount(string accountId)
        {
            lock (this)
            {
                var accounts = LoadAccounts();
                if (!accounts.ContainsKey(accountId))
                {
                    throw new AccountNotExistsException();
                }

                accounts[accountId].Enabled = false;

                SaveAccounts(accounts);
            }
        }

        public EmailAccountInfo[] GetAllAccounts()
        {
            lock (this)
            {
                return LoadAccounts().Values.ToArray();
            }
        }

        private Dictionary<string, EmailAccountInfo> LoadAccounts()
        {
            Dictionary<string, EmailAccountInfo> accounts = new Dictionary<string, EmailAccountInfo>();
            if (accountsDataFilePath != null)
            {
                List<String[]> ls = CSVUtil.ReadFromFile(this.accountsDataFilePath);

                //i start from 1, skip title
                for (int i = 1; i < ls.Count; i++)
                {
                    if (ls[i].Length != CSVUtil.ColumnCount)
                    {
                        LogManager.SingleInstance.Log("Invalid account record found: {0}, it will be removed", ls[i]);
                        continue;
                    }
                    String[] strArr = ls[i];
                    Dictionary<string, string> extendedProperties = null;

                    extendedProperties = new Dictionary<string, string>();
                    extendedProperties[Constants.ExtendedParam_DGs] = strArr[(int)CSVUtil.AccountPropertyIndex.DistributionLists];
                    extendedProperties[Constants.KeyForwardEmail] = strArr[(int)CSVUtil.AccountPropertyIndex.KeyForwardEmail];
                    extendedProperties[Constants.KeyActiveSync] = strArr[(int)CSVUtil.AccountPropertyIndex.KeyActiveSync];

                    EmailAccountInfo info = new EmailAccountInfo(extendedProperties);
                    if (strArr.Length > (int)CSVUtil.AccountPropertyIndex.AdditionalEmailAddresses)
                    {
                        char[] splitters = {'|'};
                        if (!string.IsNullOrEmpty(strArr[(int)CSVUtil.AccountPropertyIndex.AdditionalEmailAddresses]))
                        {
                            string[] emailAddresses = strArr[(int)CSVUtil.AccountPropertyIndex.AdditionalEmailAddresses].Split(splitters);
                            foreach (string emailAddress in emailAddresses)
                            {
                                info.AdditionalEmailAddresses.Add(emailAddress);
                            }
                        }
                    }

                    info.AccountId = strArr[(int)CSVUtil.AccountPropertyIndex.AccountId];
                    info.DisplayName = strArr[(int)CSVUtil.AccountPropertyIndex.DisplayName];
                    info.PrimaryEmailAddress = strArr[(int)CSVUtil.AccountPropertyIndex.PrimaryEmailAddress];

                    bool parseResult = true;
                    if (Boolean.TryParse(strArr[(int)CSVUtil.AccountPropertyIndex.Enabled], out parseResult))
                    {
                        info.Enabled = parseResult;
                    }

                    info.FirstName = strArr[(int)CSVUtil.AccountPropertyIndex.FirstName];
                    info.LastName = strArr[(int)CSVUtil.AccountPropertyIndex.LastName];
                    LogManager.SingleInstance.Log("Account loaded: {0}", info.ToString());
                    accounts.Add(info.AccountId, info);
                }
            }
            return accounts;
        }

        private void SaveAccounts(Dictionary<string, EmailAccountInfo> accounts)
        {
            if (accounts == null) throw new ArgumentNullException("accounts");
            lock (this)
            {
                if (this.accountsDataFilePath != null)
                {
                    List<String[]> ls = new List<String[]>();
                    foreach (EmailAccountInfo account in accounts.Values)
                    {
                        String[] rowStringArray = new String[CSVUtil.ColumnCount];
                        rowStringArray[(int)CSVUtil.AccountPropertyIndex.AccountId] = account.AccountId;
                        rowStringArray[(int)CSVUtil.AccountPropertyIndex.DisplayName] = account.DisplayName;
                        rowStringArray[(int)CSVUtil.AccountPropertyIndex.PrimaryEmailAddress] = account.PrimaryEmailAddress;
                        rowStringArray[(int)CSVUtil.AccountPropertyIndex.Enabled] = account.Enabled.ToString();
                        rowStringArray[(int)CSVUtil.AccountPropertyIndex.FirstName] = account.FirstName;
                        rowStringArray[(int)CSVUtil.AccountPropertyIndex.LastName] = account.LastName;
                        if (account.ExtendedProperties != null && account.ExtendedProperties.ContainsKey(Constants.ExtendedParam_DGs))
                        {
                            rowStringArray[(int)CSVUtil.AccountPropertyIndex.DistributionLists] = account.ExtendedProperties[Constants.ExtendedParam_DGs];
                        }
                        else
                        {
                            rowStringArray[(int)CSVUtil.AccountPropertyIndex.DistributionLists] = string.Empty;
                        }

                        if (account.ExtendedProperties != null && account.ExtendedProperties.ContainsKey(Constants.KeyActiveSync))
                        {
                            rowStringArray[(int)CSVUtil.AccountPropertyIndex.KeyActiveSync] = account.ExtendedProperties[Constants.KeyActiveSync];
                        }
                        else
                        {
                            rowStringArray[(int)CSVUtil.AccountPropertyIndex.KeyActiveSync] = false.ToString();
                        }

                        if (account.ExtendedProperties != null && account.ExtendedProperties.ContainsKey(Constants.KeyForwardEmail))
                        {
                            rowStringArray[(int)CSVUtil.AccountPropertyIndex.KeyForwardEmail] = account.ExtendedProperties[Constants.KeyForwardEmail];
                        }
                        else
                        {
                            rowStringArray[(int)CSVUtil.AccountPropertyIndex.KeyForwardEmail] = string.Empty;
                        }

                        if (account.AdditionalEmailAddresses != null)
                        {
                            rowStringArray[(int)CSVUtil.AccountPropertyIndex.AdditionalEmailAddresses] = string.Join("|",account.AdditionalEmailAddresses);
                        }
                        else
                        {
                            rowStringArray[(int)CSVUtil.AccountPropertyIndex.AdditionalEmailAddresses] = string.Empty;
                        }

                        ls.Add(rowStringArray);
                    }
                    CSVUtil.WriteToFile(this.accountsDataFilePath, ls);
                }
            }
        }

    }

}
