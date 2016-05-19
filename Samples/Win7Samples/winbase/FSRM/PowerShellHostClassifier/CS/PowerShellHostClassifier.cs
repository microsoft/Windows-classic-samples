// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved

using System;
using System.Globalization;
using System.Collections;
using System.Collections.Generic;
using System.Runtime.InteropServices;
using Microsoft.Storage;
using System.Text;
using System.Text.RegularExpressions;
using System.Runtime.InteropServices.ComTypes;
using System.IO;
using System.Collections.ObjectModel;
using System.Management.Automation;
using System.Management.Automation.Runspaces;
using System.Threading;
using System.Reflection;


[assembly: CLSCompliant(false)]
namespace Microsoft.Samples.Fsrm.PowerShellHostClassifier
{

    /// <summary>
    /// The exposed class that implements the interface of a FSRM classifier
    /// The classifier allows a powershell scripts to be run per rule
    /// </summary>
    [ComVisible( true ), Guid( "2573306a-2519-4e82-9617-5f99c6137c51" )]
    public class HostingClassifier : IFsrmClassifierModuleImplementation
    {
        public const int FSRM_E_NO_PROPERTY_VALUE = unchecked((int) 0x80045351);

        //cached module definitions
        private IFsrmPipelineModuleDefinition m_moduleDefinition;

        //cached property bag
        private IFsrmPropertyBag m_propertyBag;

        //cached dictionary of guids to PowerShellRuleHosters
        private Dictionary<Guid, PowerShellRuleHoster> m_dictAllRules;

        //cached dictionary of guids to IFsrmPropertyDefinitions
        private Dictionary<Guid, IFsrmPropertyDefinition> m_dictAllProperties;

        /// <summary>
        /// Constructor
        /// </summary>
        public HostingClassifier()
        {
        }


        /// <summary>
        /// Destructor
        /// </summary>
        ~HostingClassifier()
        {
        }

        /// <summary>
        /// Called once when loading the module, will not necessarily be called once per classificaiton run
        /// </summary>
        /// <param name="moduleDefinition">The definition for this module</param>
        /// <param name="moduleConnector"></param>
        public void OnLoad(
            IFsrmPipelineModuleDefinition moduleDefinition,
            out FsrmPipelineModuleConnector moduleConnector
            )
        {
            moduleConnector = new FsrmPipelineModuleConnector();
            moduleConnector.Bind( moduleDefinition, this );
            m_moduleDefinition = moduleDefinition;

        }

        /// <summary>
        /// Called during module unload
        /// </summary>
        public void OnUnload()
        {
            foreach (PowerShellRuleHoster rule in m_dictAllRules.Values) {
                rule.UnloadRule();
            }
            m_dictAllRules.Clear();
        }

        public object LastModified
        {
            get
            {
                return Decimal.Zero;
            }
        }

        /// <summary>
        /// Called once per classification run with the rule list and the property definition list
        /// </summary>
        /// <param name="Rules"></param>
        /// <param name="propertyDefinitions"></param>
        public void UseRulesAndDefinitions(
            IFsrmCollection Rules,
            IFsrmCollection propertyDefinitions
            )
        {
            m_dictAllRules = new Dictionary<Guid, PowerShellRuleHoster>();
            m_dictAllProperties = new Dictionary<Guid,IFsrmPropertyDefinition>();
            Dictionary<string,IFsrmPropertyDefinition> dictPropertyName = new Dictionary<string,IFsrmPropertyDefinition>();
            foreach (IFsrmPropertyDefinition fsrmPropertyDefinition in propertyDefinitions)
            {
                m_dictAllProperties.Add(fsrmPropertyDefinition.id, fsrmPropertyDefinition);
                dictPropertyName[fsrmPropertyDefinition.Name] = fsrmPropertyDefinition;
            }

            foreach (IFsrmClassificationRule fsrmClsRule in Rules) {

                if (fsrmClsRule.RuleType == _FsrmRuleType.FsrmRuleType_Classification) 
                {
                    Guid ruleId = fsrmClsRule.id;
                    PowerShellRuleHoster newRule = new PowerShellRuleHoster(
                                                        m_moduleDefinition, 
                                                        fsrmClsRule, 
                                                        dictPropertyName[fsrmClsRule.PropertyAffected]);
                    
                    m_dictAllRules.Add( ruleId, newRule );
                }
            }
        }

        /// <summary>
        /// This is called on the begining of each new file
        /// 
        /// It should be noted that if rules are being processed in this function and if each rule can modify the propertyBag and affect other rules
        /// then the rules should be processed synchronously such that the first rule processed contains the longest scope that matches the file's path
        /// To avoid trying to do this on your own you can just process the rules in the DoesPropertyValueApply / GetPropertyValueToApply
        /// </summary>
        /// <param name="propertyBag">The fsrm property bag representing the current file</param>
        /// <param name="arrayRuleIds">An array of strings containg the string form of the guids that apply to this file, to convert them to guid's do new Guid(string)</param>
        public void OnBeginFile(
            IFsrmPropertyBag propertyBag,
            object[] arrayRuleIds
            )
        {
            m_propertyBag = propertyBag;            
        }

        /// <summary>
        /// States whether or not this rule should apply the predefined value to the predefined property
        /// The caller will handle aggregation if multiple rules 
        /// 
        /// *Note
        /// this need not be implemented if not supporting yes no classifiers
        /// </summary>
        /// <param name="property">The name of the property that this rule can modify</param>
        /// <param name="Value">The value to set that will be applied to the property if applyValue is true</param>
        /// <param name="applyValue">True if the caller should set Value to property, False if the caller should not set Value to property</param>
        /// <param name="idRule">The id of the rule to process</param>
        /// <param name="idPropDef">The id of the property definition that this rule can modify</param>
        public void DoesPropertyValueApply(
            string property,
            string Value,
            out bool applyValue,
            Guid idRule,
            Guid idPropDef
            )
        {
            PowerShellRuleHoster rule = m_dictAllRules[idRule];

            // run the powershell pipeline for this value
            rule.StepPipeline(m_propertyBag);
            
            // if the rule doesn't apply force the value to false
            if (rule.RuleNoApply)
            {
                applyValue = false;
            }
            else
            {
                try 
                {
                    applyValue = (bool)rule.PropertyValue;
                }
                catch (Exception ex) 
                {
                    //Assuming this error is due to casting

                    String message = String.Format( CultureInfo.InvariantCulture,
                                                    "PowerShell Classifier for rule [{0}] and file [{1}], failed to convert the property recieved from powershell of type [{2}] and value [{3}] into the a boolean value for yes/no classification, with error [{4}]",
                                                    rule.RuleName,
                                                    m_propertyBag.VolumeName + m_propertyBag.RelativePath + "\\" + m_propertyBag.Name,
                                                    rule.PropertyValue.GetType().ToString(),
                                                    rule.PropertyValue.ToString(),
                                                    ex.Message );

                    m_propertyBag.AddMessage( message );

                    throw new COMException( message, ex.InnerException );
                }
            }

        }

        /// <summary>
        /// Gets the value for the property and rule to apply
        /// The caller will handle aggregating multiple values for a single property
        /// 
        /// *Note
        /// This function need not be implemented if only supporting yes no classifiers
        /// </summary>
        /// <param name="property">The name of the property that this rule can modify</param>
        /// <param name="Value">The value to set the property to</param>
        /// <param name="idRule">The id of the rule to process</param>
        /// <param name="idPropDef">The id of the definition that this rule can modify</param>
        public void GetPropertyValueToApply(
            string property,
            out string Value,
            Guid idRule,
            Guid idPropDef
            )
        {
            PowerShellRuleHoster rule = m_dictAllRules[idRule];
            
            // run the powershell script over the propertyBag
            rule.StepPipeline(m_propertyBag);

            // If the rule does not modify the property value throw an error that specifies this
            // You must specify the HR that is being specified below if you wish to not modify the rule's value
            if (rule.RuleNoApply)
            {
                throw new COMException("Value does not apply", FSRM_E_NO_PROPERTY_VALUE );//, FSRM_E_NO_PROPERTY_VALUE);
            }

            //convert the powershell value into the approrpiate value
            try
            {
                IFsrmPropertyDefinition propertyDefinition = m_dictAllProperties[idPropDef];

                if (propertyDefinition.Type == _FsrmPropertyDefinitionType.FsrmPropertyDefinitionType_Bool)
                {
                    //convert from true/false to "1","0"
                    Value = (bool)rule.PropertyValue ? "1" : "0";
                }
                else if (propertyDefinition.Type == _FsrmPropertyDefinitionType.FsrmPropertyDefinitionType_Date)
                {
                    // convert from datetime to str
                    DateTime time = (DateTime)rule.PropertyValue;
                    long filetime = time.ToFileTimeUtc();
                    Value = filetime.ToString(CultureInfo.InvariantCulture);
                }
                else if (propertyDefinition.Type == _FsrmPropertyDefinitionType.FsrmPropertyDefinitionType_Int)
                {
                    Value = rule.PropertyValue.ToString();
                }
                else
                {
                    Value = (string)rule.PropertyValue;
                }
            }
            catch (Exception ex)
            {
                String message = "";

                if (rule.PropertyValue == null)
                {
                    message = String.Format(CultureInfo.InvariantCulture,
                                                    "PowerShell Classifier for rule [{0}] and file [{1}], received null value from powershell, with error [{2}]",
                                                    rule.RuleName,
                                                    m_propertyBag.VolumeName + m_propertyBag.RelativePath + "\\" + m_propertyBag.Name,
                                                    ex.Message);
                }
                else
                {
                    //Assuming this error is due to casting
                    message = String.Format(CultureInfo.InvariantCulture,
                                                    "PowerShell Classifier for rule [{0}] and file [{1}], failed to convert the property recieved from powershell of type [{2}] and value [{3}] into the property [{4}], with error [{5}]",
                                                    rule.RuleName,
                                                    m_propertyBag.VolumeName + m_propertyBag.RelativePath + "\\" + m_propertyBag.Name,
                                                    rule.PropertyValue.GetType().ToString(),
                                                    rule.PropertyValue.ToString(),
                                                    property,
                                                    ex.Message);
                }

                m_propertyBag.AddMessage(message);

                throw new COMException( message, ex.InnerException );
            }
        }

        /// <summary>
        /// Sample code does nothing here but you can put any per file cleanup code here
        /// </summary>
        public void OnEndFile()
        {
            m_propertyBag = null;
        }
    }


    public static class HRESULTS
    {
        public const int PS_CLS_E_TOO_MANY_VALUES = unchecked( (int)0x80045401 );
    }
}