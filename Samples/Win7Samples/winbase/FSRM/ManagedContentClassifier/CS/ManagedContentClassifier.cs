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
using System.Diagnostics.CodeAnalysis;


[module: SuppressMessage("Microsoft.Naming", "CA1704:IdentifiersShouldBeSpelledCorrectly", Scope = "namespace", Target = "Microsoft.Samples.Fsrm.ManagedContentClassifier", MessageId = "Fsrm")]
[module: SuppressMessage("Microsoft.Naming", "CA1704:IdentifiersShouldBeSpelledCorrectly", Scope = "type", Target = "Microsoft.Samples.Fsrm.ManagedContentClassifier.FsrmClassificationRule", MessageId = "Fsrm")]
[module: SuppressMessage("Microsoft.Usage", "CA2201:DoNotRaiseReservedExceptionTypes", Scope = "member", Target = "Microsoft.Samples.Fsrm.ManagedContentClassifier.StreamWrapperForIStream.#Read(System.Byte[],System.Int32,System.Int32)")]
[module: SuppressMessage("Microsoft.Usage", "CA2201:DoNotRaiseReservedExceptionTypes", Scope = "member", Target = "Microsoft.Samples.Fsrm.ManagedContentClassifier.StreamWrapperForIStream.#set_Position(System.Int64)")]
[module: SuppressMessage("Microsoft.Naming", "CA1704:IdentifiersShouldBeSpelledCorrectly", Scope = "member", Target = "Microsoft.Samples.Fsrm.ManagedContentClassifier.StreamWrapperForIStream.#.ctor(System.Runtime.InteropServices.ComTypes.IStream)", MessageId = "istream")]

/*++

    namespace Microsoft.Samples.Fsrm.ManagedContentClassifier

Description:

    This namespace contains the managed sample content classifier.

--*/
[assembly: CLSCompliant( false )]
namespace Microsoft.Samples.Fsrm.ManagedContentClassifier
{

    /*++

        class FsrmClassificationRule

    Description:

        This class encapsulates an FSRM rule for this classifier.
        It holds the regular expression object used to evaluate the classifier's rule 
        and result of the classification for the current FSRM property bag (i.e. current file).

    --*/
    public class FsrmClassificationRule
    {
        //bool to store state of rule match/unmatch for the current file
        private bool m_hasMatched;

        //Regular expression object
        private Regex Regexp;

        /*++

            Routine FsrmClassificationRule

        Description:

            This is the constructor for the FsrmClassificationRule
            
        Arguments:

            contents - The rule's search string
            options   - The options for the Regex regular expression object
            
        Return value:

            void
            
        Notes:
            

        --*/
        
        public FsrmClassificationRule( string contents, RegexOptions options )
        {
            Regexp = new Regex( contents, options );
        }


        /*++

            Routine HasMatched

        Description:

            This routine returns the match/not-matched state of the rule for the current file
            
        Arguments:

            void
            
        Return value:

            bool
            
        Notes:
            

        --*/
        
        public bool HasMatched
        {
            get
            {
                return m_hasMatched;
            }
        }


        /*++

            Routine ResetRule

        Description:

            This routine resets the rule so the next file can be processed.
            
        Arguments:

            void
            
        Return value:

            void
            
        Notes:
            

        --*/
        
        public void ResetRule()
        {
            m_hasMatched = false;
        }



        /*++

            Routine DoesRegexSatisfy

        Description:

            This routine determines if the current portion of text from the file 
            matches the regular expression for this rule.
            
        Arguments:

            text - A string containing text from the file.
            
        Return value:

            bool
            
        Notes:
            

        --*/
        
        public bool DoesRegexSatisfy( string text )
        {
            if (m_hasMatched) {
                return m_hasMatched;
            }

            if (Regexp.IsMatch( text )) {
                m_hasMatched = true;
            }
            return m_hasMatched;
        }

    }


    /*++

        class RegexClassifier

    Description:

        This class implements the managed sample classifier.
        This classifier matches regular expressions defined in its rules 
        against the contents of the file.

    --*/

    [ComVisible( true ), Guid( "4d292ba9-366b-42ff-ab6b-274953475166" )]
    public class RegexClassifier : IFsrmClassifierModuleImplementation
    {
        //Delimiter used while parsing the classifier's module parameters
        private static string ModuleParamDelimiter = "=";

        //String key value for regular expression options in the module parameters
        private static string ModuleParamRegexOptions = "RegexOptions";

        private System.Text.Encoding m_encoder;

        //Regex regular expression object's options to be read from the module parameters
        private int m_iValRegexOptions;
        
        //Dictionary for looking up rules
        private Dictionary<Guid, FsrmClassificationRule> m_dictAllRules;


        /*++

            Routine RegexClassifier::OnLoad

        Description:

            This routine implements the IFsrmClassifierModuleImplementation interface OnLoad.
            It is called when the classifier module is loaded by the FSRM classification manager.
            
        Arguments:

            moduleDefinition - The module definition for this object passed by the FSRM classification manager
            moduleConnector - Out parameter for the FSRM pipeline connector module that this method creates and returns.
            
        Return value:

            void
            
        Notes:
            

        --*/
        public void OnLoad(
            IFsrmPipelineModuleDefinition moduleDefinition,
            out FsrmPipelineModuleConnector moduleConnector
            )
        {
            //Default value for the Regex options
            m_iValRegexOptions = (int)(RegexOptions.Compiled | RegexOptions.Multiline | RegexOptions.CultureInvariant);

            //Create the FSRM pipeline module connector
            moduleConnector = new FsrmPipelineModuleConnector();

            //Bind the connector to this object
            moduleConnector.Bind( moduleDefinition, this );

            //Retrieve the classifier's module parameters from the module definitions
            object[] modParams = moduleDefinition.Parameters;

            //loop through all module parameters and save appropriate ones
            foreach (object param in modParams) {
                string strParam = (string)param;
                string[] splitParams =
                    strParam.Split(
                        RegexClassifier.ModuleParamDelimiter.ToCharArray(),
                        2
                        );

                string key = splitParams[0];
                string val = splitParams[1];
                if (val != null) {
                    if (key.Equals(
                        RegexClassifier.ModuleParamRegexOptions,
                        StringComparison.OrdinalIgnoreCase )) {
                        m_iValRegexOptions = Convert.ToInt32( val, CultureInfo.InvariantCulture );
                    } else if (key.Equals( "encoding", StringComparison.OrdinalIgnoreCase )) {
                        if (val.Equals( "ascii", StringComparison.OrdinalIgnoreCase )) {
                            m_encoder = new System.Text.ASCIIEncoding();
                        } else if (val.Equals( "unicode", StringComparison.OrdinalIgnoreCase )) {
                            m_encoder = new System.Text.UnicodeEncoding();
                        } else if (val.Equals( "utf32", StringComparison.OrdinalIgnoreCase )) {
                            m_encoder = new System.Text.UTF32Encoding();
                        } else if (val.Equals( "utf7", StringComparison.OrdinalIgnoreCase )) {
                            m_encoder = new System.Text.UTF7Encoding();
                        } else if (val.Equals( "utf8", StringComparison.OrdinalIgnoreCase )) {
                            m_encoder = new System.Text.UTF8Encoding();
                        }
                    }
                }
            }
            if (m_encoder == null) {
                throw new ArgumentException( "Classifier needs a valid encoding" );
            }

        }

        /*++

            Routine RegexClassifier::OnUnload

        Description:

            This routine implements the IFsrmClassifierModuleImplementation interface OnUnload.
            It is called when the classifier module is unloaded by the FSRM classification manager.
            
        Arguments:

            void
            
        Return value:

            void
            
        Notes:

            This classifier does not do anything in this method.

        --*/
        
        public void OnUnload()
        {

        }


        /*++

            Routine RegexClassifier::OnUnload

        Description:

            This routine implements the IFsrmClassifierModuleImplementation interface LastModified.
            It is called by the pipeline to determine when the classifier's rules were last modified.
            
        Arguments:

            void
            
        Return value:

            last modified time
            
        Notes:

            This classifier indicates to the pipeline that its internal rules were never modified.

        --*/
        
        public object LastModified
        {
            get
            {
                Decimal lastModified = Decimal.Zero;
                return lastModified;
            }
        }


        /*++

            Routine RegexClassifier::UseRulesAndDefinitions

        Description:

            This routine implements the IFsrmClassifierModuleImplementation interface UseRulesAndDefinitions.
            It is called by the pipeline to indicate to the classifier that the following rules apply to it for this run.
            
        Arguments:

            Rules - The FSRM classification rules that apply to this classifier for this run.
            propertyDefinitions - The property definitions affected by the above rules.
            
        Return value:

            void
            
        Notes:
        

        --*/
        
        public void UseRulesAndDefinitions(
            IFsrmCollection Rules,
            IFsrmCollection propertyDefinitions
            )
        {

            m_dictAllRules = new Dictionary<Guid, FsrmClassificationRule>();

            // loop through all rules and save them along with thier
            // parameter descriptions
            foreach (IFsrmClassificationRule fsrmClsRule in Rules) {

                if (fsrmClsRule.RuleType == _FsrmRuleType.FsrmRuleType_Classification) {
                    Guid ruleId = fsrmClsRule.id;

                    foreach (string param in fsrmClsRule.Parameters) {
                        string[] splitParams = param.Split( "=".ToCharArray(), 2 );

                        string paramKey = splitParams[0];
                        string paramVal = splitParams[1];

                        if (paramVal != null) {
                            if (paramKey.Equals(
                                "RegularExpression",
                                StringComparison.OrdinalIgnoreCase )
                                ) {

                                //Create a new internal rule object and store it 
                                FsrmClassificationRule newRuleToAdd = new FsrmClassificationRule(
                                                                                paramVal,
                                                                                (RegexOptions)m_iValRegexOptions
                                                                                );
                                m_dictAllRules.Add( ruleId, newRuleToAdd );
                            }
                        }
                    }
                }
            }
        }


        /*++

            Routine RegexClassifier::OnBeginFile

        Description:

            This routine implements the IFsrmClassifierModuleImplementation interface OnBeginFile.
            It is called by the pipeline for each file so that the classifier can process its rules for this file.
            
        Arguments:

            propertyBag - The FSRM property bag object for this file.
            arrayRuleIds - The list of rules that are applicable for this file.
            
        Return value:

            void
            
        Notes:
        

        --*/
        
        public void OnBeginFile(
            IFsrmPropertyBag propertyBag,
            object[] arrayRuleIds
            )
        {
            //reset all rules
            foreach (KeyValuePair<Guid, FsrmClassificationRule> kvp in m_dictAllRules) {
                FsrmClassificationRule rule = (FsrmClassificationRule)kvp.Value;
                rule.ResetRule();
            }

            // create guid form of each id
            Guid[] ruleIdGuids = new Guid[arrayRuleIds.Length];
            for (int i = 0; i < arrayRuleIds.Length; ++i) {
                ruleIdGuids[i] = new Guid( (string)arrayRuleIds[i] );
            }

            // wrap the istream in a stream, and use a streamreader to get each line
            // match each line against each rule's regexp
            // quit when all data has been read or all rules have been matched
            using (StreamWrapperForIStream stream = new StreamWrapperForIStream( (IStream)propertyBag.GetFileStreamInterface( _FsrmFileStreamingMode.FsrmFileStreamingMode_Read, _FsrmFileStreamingInterfaceType.FsrmFileStreamingInterfaceType_IStream ) )) {

                StreamReader streamReader = new StreamReader( stream, m_encoder );
                bool matchedAllRules = false;

                do {
                    string line = streamReader.ReadLine();

                    matchedAllRules = true;
                    foreach (Guid ruleId in ruleIdGuids) {
                        matchedAllRules &= m_dictAllRules[ruleId].DoesRegexSatisfy( line );
                    }

                } while (!streamReader.EndOfStream && !matchedAllRules);

                streamReader.Dispose();
            }
        }

        /*++

            Routine RegexClassifier::DoesPropertyValueApply

        Description:

            This routine implements the IFsrmClassifierModuleImplementation interface DoesPropertyValueApply.
            It is called by the pipeline for each file for each rule to determine if this classifier 
            wants to apply a that rule for this file.
            This method is called for yes/no classifiers.
            
        Arguments:

            property       - The FSRM property that will be applied.
            Value           - The value that will be set for this property.
            applyValue   - Out parameter to be used by the classifier to 
                                  indicate to the pipeline whether to apply the rule
            idRule          - The guid identifying the rule that is being applied
            idPropDef     - The guid identifying the property definition for the property being applied.
            
        Return value:

            void
            
        Notes:
        

        --*/
        
        public void DoesPropertyValueApply(
            string property,
            string Value,
            out bool applyValue,
            Guid idRule,
            Guid idPropDef
            )
        {
            bool bResult = false;
            FsrmClassificationRule rule;
            if (m_dictAllRules.TryGetValue( idRule, out rule )) {
                bResult = rule.HasMatched;
            } else {
                throw new ArgumentException( "Invalid rule {0}", idRule.ToString() );
            }
            applyValue = bResult;
        }


        /*++

            Routine RegexClassifier::GetPropertyValueToApply

        Description:

            This routine implements the IFsrmClassifierModuleImplementation interface GetPropertyValueToApply.
            It is called by the pipeline for each file for each rule to determine if this classifier 
            wants to apply a that rule for this file. 
            This method is called for classifiers that require an explicit value for the property
            as opposed to yes/no classifiers.
            
        Arguments:

            property       - The FSRM property that will be applied.
            Value           - The value that will be set for this property.
            applyValue   - Out parameter to be used by the classifier to 
                                  indicate to the pipeline whether to apply the rule
            idRule          - The guid identifying the rule that is being applied
            idPropDef     - The guid identifying the property definition for the property being applied.
            
        Return value:

            void
            
        Notes:

            This classifier is a yes/no classifier so this method is not implemented.

        --*/
        
        public void GetPropertyValueToApply(
            string property,
            out string Value,
            Guid idRule,
            Guid idPropDef
            )
        {
            throw new NotImplementedException( "GetPropertyValueToApply" );
        }


        /*++

            Routine RegexClassifier::OnEndFile

        Description:

            This routine implements the IFsrmClassifierModuleImplementation interface OnEndFile.
            It is called by the pipeline for each file at the end of the processing for that file 
            so that the classifier can perform cleanup.
            
        Arguments:

            void
            
        Return value:

            void
            
        Notes:

            This classifier does not need to clean up at the end of a file.

        --*/
        
        public void OnEndFile()
        {
        }

    }

}



