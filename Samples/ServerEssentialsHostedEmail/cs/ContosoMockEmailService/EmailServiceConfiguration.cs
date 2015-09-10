//
//  <copyright file="EmailServiceConfiguration.cs" company="Microsoft">
//    Copyright (C) Microsoft. All rights reserved.
//  </copyright>
//

using System;
using System.Collections.Generic;
using System.Globalization;
using System.IO;
using System.Xml.Serialization;
using Microsoft.WindowsServerSolutions.HostedEmail;

namespace Contoso.EmailService
{
    internal class EmailServiceConfiguration
    {
        private Dictionary<OperationEnum, OperationsOperation> opTable;

        private Configuration config;

        public EmailServiceConfiguration(string filePath)
        {
            if (string.IsNullOrEmpty(filePath)) throw new ArgumentNullException("filePath");
            if (!File.Exists(filePath)) throw new ArgumentException(
                 String.Format(CultureInfo.InvariantCulture, "Cannot find configuration file {0}", filePath), "filePath");
            LoadConfiguration(filePath);
        }

        private int defaultOperationDuration = 0;

        public int GetOperationDuration(string opName)
        {
            OperationEnum op;
            if (!Enum.TryParse<OperationEnum>(opName, out op)) return defaultOperationDuration;
            if (!opTable.ContainsKey(op)) return defaultOperationDuration;
            int retVal = 0;
            if (!int.TryParse(opTable[op].DurationInSeconds, out retVal)) return defaultOperationDuration;
            return retVal;
        }

        public HostedEmailAdaptorException GetOperationException(string opName)
        {
            OperationEnum op;
            if (!Enum.TryParse<OperationEnum>(opName, out op)) return null;
            if (!opTable.ContainsKey(op)) return null;
            var operation = opTable[op];
            if (operation.Exception == null) return null;
            return new HostedEmailAdaptorException(
                operation.Exception.ErrorCode.ToHEAE_ErrorCode(),
                operation.Exception.ErrorRecord.ToAddinErrorRecord());
        }

        public DistributionGroup[] DistributionGroups
        {
            get
            {
                return (config == null) ? null : config.DistributionGroups;
            }
        }

        private void LoadConfiguration(string filePath)
        {
            using (TextReader tr = new StreamReader(filePath))
            {
                try
                {
                    XmlSerializer serializer = new XmlSerializer(typeof(Configuration));
                    config = (Configuration)serializer.Deserialize(tr);
                    if (config == null || config.DefaultOperationDuration == null)
                    {
                        throw new InvalidOperationException(string.Format(CultureInfo.InvariantCulture, "{0} is not a valid configuration", filePath));
                    }

                    if (!int.TryParse(config.DefaultOperationDuration, out defaultOperationDuration)) defaultOperationDuration = 0;

                    opTable = new Dictionary<OperationEnum, OperationsOperation>(config.Operations.Length);
                    if (config.Operations == null) return;
                    foreach (var op in config.Operations)
                    {
                        opTable[op.Name] = op;
                    }
                }
                catch (InvalidOperationException)
                {
                    //failed to deserialze the configuration file, just throw the exception
                    throw;
                }
            }
        }
    }
}
