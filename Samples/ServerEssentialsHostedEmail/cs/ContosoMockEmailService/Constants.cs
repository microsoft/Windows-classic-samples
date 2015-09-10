//
//  <copyright file="Constants.cs" company="Microsoft">
//    Copyright (C) Microsoft. All rights reserved.
//  </copyright>
//

using System;
using System.IO;

namespace Contoso.EmailService
{
    public static class Constants
    {
        private const string configFileName = "MockEmailServiceConfiguration.xml";
        private const string schemaFileName = "MockEmailServiceConfiguration.xsd";
        private const string contosoFolderName = "Contoso";
        private const string accountDataFileName = "ACCOUNTS_DATA_FILE.csv";

        private static string baseDir = Path.Combine(Environment.GetFolderPath(Environment.SpecialFolder.CommonApplicationData), contosoFolderName);
        public static string BaseDirectory
        {

            get
            {
                if (!Directory.Exists(baseDir))
                {
                    LogManager.SingleInstance.Log("Creating directory {0}", baseDir);
                    Directory.CreateDirectory(baseDir);
                }
                return baseDir;
            }
        }

        public static string ConfigrationFilePath
        {
            get
            {
                return Path.Combine(BaseDirectory, configFileName);
            }
        }

        public static string AccountDataFilePath
        {
            get
            {
                return Path.Combine(BaseDirectory, accountDataFileName);
            }
        }

        public static string ConfigurationSchemaPath
        {
            get
            {
                return Path.Combine(BaseDirectory, schemaFileName);
            }
        }

        public const string ExtendedParam_DGs = "ExtendedParam_DistributionGroups";

        public const string KeyForwardEmail = "KeyForwardEmail";
        public const string KeyActiveSync = "KeyActiveSync";

        public const char ExtendedParam_DGs_Delimiter = ';';

        public static readonly Guid AdaptorId = new Guid("3983E9AC-B6D1-4A2A-881C-4B1CEFCA5266");
    }
}
