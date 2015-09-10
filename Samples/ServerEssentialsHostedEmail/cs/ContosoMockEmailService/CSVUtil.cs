//
//  <copyright file="CSVUtil.cs" company="Microsoft">
//    Copyright (C) Microsoft. All rights reserved.
//  </copyright>
//

using System;
using System.Collections.Generic;
using System.IO;
using System.Text;

namespace Contoso.EmailService
{
    internal static class CSVUtil
    {
        public enum AccountPropertyIndex
        {
            AccountId = 0,
            DisplayName = 1,
            PrimaryEmailAddress = 2,
            Enabled = 3,
            FirstName = 4,
            LastName = 5,
            DistributionLists = 6,
            KeyForwardEmail = 7,
            KeyActiveSync = 8,
            AdditionalEmailAddresses = 9
        };

        public static readonly int ColumnCount = Enum.GetValues(typeof(AccountPropertyIndex)).Length;

        private static readonly string titleLine = string.Join(",", Enum.GetNames(typeof(AccountPropertyIndex)));

        public static object lockObj = new Object();

        //overwritten is default option
        public static void WriteToFile(string filePathName, List<String[]> ls)
        {
            lock (lockObj)
            {
                string[] content;
                if (ls == null)
                {
                    content = new string[1];
                    content[0] = titleLine;
                }
                else
                {
                    content = new string[ls.Count + 1];
                    content[0] = titleLine;
                    for (int i = 0; i < ls.Count; ++i)
                    {
                        content[i + 1] = string.Join(",", ls[i]);
                    }
                }
                // File created by using UTF8, File.CreateText creates a text file using UTF8
                File.WriteAllLines(filePathName, content, Encoding.UTF8);
            }
        }

        public static List<String[]> ReadFromFile(string filePathName)
        {
            lock (lockObj)
            {
                if (!File.Exists(filePathName))
                {
                    return new List<string[]>(0);
                }
                string[] contents = File.ReadAllLines(filePathName);
                List<String[]> ls = new List<String[]>(contents.Length);
                foreach (var line in contents)
                {
                    if (string.IsNullOrWhiteSpace(line)) continue;
                    ls.Add(line.Split(','));
                }
                return ls;
            }
        }
    }
}
