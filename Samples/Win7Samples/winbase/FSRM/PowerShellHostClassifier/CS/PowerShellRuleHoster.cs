// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved

using System;
using System.Collections;
using System.Collections.Generic;
using Microsoft.Storage;
using System.Runtime.InteropServices.ComTypes;
using System.IO;
using System.Collections.ObjectModel;
using System.Management.Automation;
using System.Management.Automation.Runspaces;
using System.Threading;
using System.Runtime.InteropServices;
using System.Diagnostics;
using System.Globalization;

namespace Microsoft.Samples.Fsrm.PowerShellHostClassifier
{
    /// <summary>
    /// This class provides a way of running a FSRM Classification rule inside powershell
    /// Encapsulates creating a powershell pipeline, running it, restarting it on errors
    /// </summary>
    public class PowerShellRuleHoster: IDisposable
    {
        #region statics
        private static string ScriptFileNameString = "ScriptFileName";
        #endregion

        
        // The runspace running powershell inside of, and holds references to global objects such as rule deffinition
        // *Note this does not need to be recreated if the pipeline fails
        private Runspace m_runSpace;

        private Pipeline m_pipeLine;

        private string m_ruleName;

        // The most recent value emitted from the pipeline
        private PSObject m_propertyValue;

        // An enumerator that causes the powershell script to hang until it gets input
        private BlockablePropertyBagEnumerator m_propertyBagWriter;

        // Whether the rule applies to the current property bag 
        private bool m_ruleNoApply;

        // A reference to the event log (log - Application, provider - SRMREPORTS)
        private EventLog m_eventLog;

        // The wrapped text of the script to run
        private string m_scriptText;

        // this is used to add the GetStream method to the propertyBag
        private System.Reflection.MethodInfo m_getStreamMethodInfo;

        // Allows to determine if the pipeline has hanged awaiting input or terminated or emitted data
        private WaitHandle[] m_powershellPaused;

        // The index in m_powershellPaused representing which handle is being accessed
        // Note the enumerator will be the highest index because it is a manual reset event and we only want to know about
        // after the pipe line has been consumed
        private enum m_waitHandleIndex
        {
            PipeLine = 0,
            Enumerator,
            Count
        }

        /// <summary>
        /// Parse strParameters into an easy to consume dictionary
        /// Useful for parsing rule or module definition strParameters
        /// </summary>
        /// <param name="parameters">The strParameters to parse</param>
        /// <returns>A dictionary of parameter and parameter values</returns>
        public static Dictionary<string, string> ParseParameters(
            object[] parameters
            )
        {
            Dictionary<string, string> strParameters = new Dictionary<string, string>(StringComparer.OrdinalIgnoreCase);

            // create the dictionary of the strParameters
            foreach (string param in parameters)
            {
                //split param into string before first =
                // and string after first =
                string[] splitParams = param.Split( "=".ToCharArray(),
                                                    2 );
                strParameters[splitParams[0]] = splitParams[1];
            }
            return strParameters;
        }

        /// <summary>
        /// Gets the text of the powershell script
        /// </summary>
        /// <param name="rule">The rule definition (contains the script to get)</param>
        /// <returns>The text of the powershell script</returns>
        public static string GetScriptText(
            IFsrmClassificationRule rule
            )
        {
            // parse the rule strParameters to determine what type of encoding there is
            // otherwise use default encoding to get the text of the file
            Dictionary<string, string> ruleParameters = ParseParameters(rule.Parameters);

            string scriptFileName;

            try
            {
                scriptFileName = ruleParameters[ScriptFileNameString];
            }
            catch (KeyNotFoundException e)
            {
                string message = String.Format(
                    "PowerShellHostClassifier failed processing parameters for rule {0}. Make sure the rule parameter name is ScriptFileName.",
                    rule.Name);

                EventLog eventLog = new EventLog("Application", ".", "SRM_PS_CLS");
                eventLog.WriteEntry(message, EventLogEntryType.Error);

                throw new COMException(message, e);
            }

            // read the text of the file and return it
            string fileText;
            using (StreamReader fs = new StreamReader(scriptFileName))
            {
                fileText = fs.ReadToEnd();
            }
            
            return fileText;
        }

        /// <summary>
        /// Initialize all values, as well as start the pipeline and await for data
        /// </summary>
        /// <param name="moduleDefinition">The module definition for the classifier</param>
        /// <param name="rule">The rule definition for the rule that this represents</param>
        /// <param name="propertyDefinition">The property definition for the property that this rule modifies</param>
        public PowerShellRuleHoster(
            IFsrmPipelineModuleDefinition moduleDefinition,
            IFsrmClassificationRule rule,
            IFsrmPropertyDefinition propertyDefinition
             )
        {
            m_propertyBagWriter = new BlockablePropertyBagEnumerator();
            m_ruleName = rule.Name;

            // create the waitHandles and initialize them
            // *note the pipeline waitHandle is created in CreateAndBeginPipeline
            m_powershellPaused = new WaitHandle[(int)m_waitHandleIndex.Count];
            m_powershellPaused[(int)m_waitHandleIndex.Enumerator] = m_propertyBagWriter.RequestedDataWaitHandle;
            
            // cache the method info for adding the GetStream to PropertyBag
            Type extensionClassForPropertyBag = typeof(ExtensionClassForPropertyBag);
            m_getStreamMethodInfo = extensionClassForPropertyBag.GetMethod("GetStream");


            string fileText = GetScriptText(rule);

            // this enables users to create a scriptblock rather then enumerate over the PropertyBagList
            m_scriptText = "$PropertyBagList | &{" + fileText + "}\n";

            // construct the runspace and set global proxy values for powershell script to use
            m_runSpace = RunspaceFactory.CreateRunspace();
            m_runSpace.Open();
            m_runSpace.SessionStateProxy.SetVariable("ModuleDefinition", moduleDefinition);
            m_runSpace.SessionStateProxy.SetVariable("PropertyBagList", m_propertyBagWriter);
            m_runSpace.SessionStateProxy.SetVariable("Rule", rule);
            m_runSpace.SessionStateProxy.SetVariable("PropertyDefinition", propertyDefinition);

            //launch the pipeline creation
            CreateAndBeginPipeline();
        }

        /// <summary>
        /// Dispose Method
        /// </summary>
        public void Dispose()
        {
            Dispose(true);
            GC.SuppressFinalize(this);
        }

        /// <summary>
        /// Dispose Method
        /// </summary>
        /// <param name="disposing">If should free managed objects</param>
        protected virtual void Dispose(bool disposing)
        {
            if (disposing)
            {
                if (m_eventLog != null)
                {
                    m_eventLog.Dispose();
                    m_eventLog = null;
                }

                if (m_propertyBagWriter != null)
                {
                    m_propertyBagWriter.Dispose();
                    m_propertyBagWriter = null;
                }
                if (m_pipeLine != null)
                {
                    m_pipeLine.Dispose();
                    m_pipeLine = null;
                }
                if (m_runSpace != null)
                {
                    m_runSpace.Close();
                    m_runSpace = null;
                }
            }
        }

        /// <summary>
        /// Creates/Recreates the powersehll pipeline and ensures that everything is at a virgin state
        /// Starts the powershell pipeline and waits for it to request the first element from the pipeline
        /// The pipeline MUST be shutdown before calling this function! the reason for this is we cannot relaibly reset the BlockableEnum otherwise
        /// </summary>
        public void CreateAndBeginPipeline()
        {
            ResetRuleResults();

            // Create/reset writer into pipeline and the pipeline
            m_propertyBagWriter.ResetBlockableEnum();
            m_pipeLine = m_runSpace.CreatePipeline();

            m_powershellPaused[(int)m_waitHandleIndex.PipeLine] = m_pipeLine.Output.WaitHandle;
            
            // set the text of the pipeline and run it
            m_pipeLine.Commands.AddScript(m_scriptText);
            m_pipeLine.InvokeAsync();

            // wait for the pipeline to fail or request its first element
            m_waitHandleIndex lockIndex = 0;
            do
            {
                lockIndex = (m_waitHandleIndex)WaitHandle.WaitAny(m_powershellPaused);
            } while (lockIndex != m_waitHandleIndex.Enumerator && m_pipeLine.Output.IsOpen);

            // errors are not permitted before processing elements
            if (m_pipeLine.Error.Count > 0)
            {
                Exception error = (Exception)m_pipeLine.Error.Read();
                string message = string.Format( CultureInfo.InvariantCulture,
                                               "Powershell Classifier threw errors before processing files in rule [{0}] - details: [{1}]",
                                               m_ruleName,
                                               error.Message );
                ThrowNonPropertyBagException(message, error);
            }

            // if pipeline terminated try and get termination reason
            if (m_pipeLine.Output.EndOfPipeline)
            {
                string message = string.Format( CultureInfo.InvariantCulture,
                                               "Powershell Classifier terminated abruptly before processing files in rule [{0}] - details: [{1}]",
                                               m_ruleName,
                                               m_pipeLine.PipelineStateInfo.Reason.Message );
                ThrowNonPropertyBagException(message, m_pipeLine.PipelineStateInfo.Reason);
            }
        }

        /// <summary>
        /// Runs one step of the pipeline using propertyBag as the pipeline input
        /// Saves last outputed object
        /// Saves whether the pipeline returned 0 or 1 object
        /// throws exception if pipeline outputed more than 1 object
        /// <param name="propertyBag">The property bag to run the powershell script's pipeline on</param>
        public void StepPipeline(
            IFsrmPropertyBag propertyBag
            )
        {
            ResetRuleResults();

            m_waitHandleIndex lockIndex;
            bool readAValue = false;
            bool tooManyValues = false;
            m_ruleNoApply = false;

            // insert the property bag into the pipeline after adding the GetStream Method
            PSObject psPropertyBag = new PSObject(propertyBag);
            psPropertyBag.Methods.Add(new PSCodeMethod("GetStream", m_getStreamMethodInfo));
            m_propertyBagWriter.InsertData(psPropertyBag);

            // wait for either the pipeline to close or 
            // for another property to be requested from the enumerator
            // If for input a in the script value 1 is emitted, it must be emitted before value b is requested
            // Handle multiple values being emitted but fail the current property bag if it happens
            do
            {
                lockIndex = (m_waitHandleIndex)WaitHandle.WaitAny(m_powershellPaused);

                //pipeline terminated unexpectedly save message and restart it
                if (m_pipeLine.Output.EndOfPipeline)
                {

                    string message;

                    if (m_pipeLine.PipelineStateInfo.State == PipelineState.Failed)
                    {
                        message = string.Format(CultureInfo.InvariantCulture,
                                                   "Powershell Classifier terminated abruptly due to failuer while processing file [{0}] in rule [{1}] - failure details: [{2}]",
                                                   propertyBag.VolumeName + propertyBag.RelativePath + "\\" + propertyBag.Name,
                                                   m_ruleName,
                                                   m_pipeLine.PipelineStateInfo.Reason.Message);
                    }
                    else
                    {
                        message = string.Format(CultureInfo.InvariantCulture,
                                                   "Powershell Classifier exited abruptly without failures while processing file [{0}] in rule [{1}].",
                                                   propertyBag.VolumeName + propertyBag.RelativePath + "\\" + propertyBag.Name,
                                                   m_ruleName);
                    }

                    propertyBag.AddMessage(message);
                    CreateAndBeginPipeline();
                    throw new COMException( message, m_pipeLine.PipelineStateInfo.Reason );
                }

                // if we haven't read a value pop one off and save it
                if (m_pipeLine.Output.Count >= 1 && !readAValue)
                {
                    readAValue = true;
                    m_propertyValue = m_pipeLine.Output.Read();
                }

                // if we have read a value and there are values in the pipeline, then 
                // set the tooManyValues flag and eat everythign in the pipeline
                while (m_pipeLine.Output.Count > 0 && readAValue)
                {
                    // if the m_propertyValue currently points to the first value ouput for the pipeline
                    // the add a message for it, other wise output messages for duplicate values after popping them off
                    if (!tooManyValues)
                    {
                        string message1 = string.Format( CultureInfo.InvariantCulture,
                               "Powershell Classifier returned too many values while processing file [{0}] in rule [{1}] - returned object of type [{2}], and value [{3}]",
                               propertyBag.VolumeName + propertyBag.RelativePath + "\\" + propertyBag.Name,
                               m_ruleName,
                               m_propertyValue.BaseObject.GetType().ToString(),
                               m_propertyValue.BaseObject.ToString() );
                        propertyBag.AddMessage(message1);

                    }

                    // cleanup pipeline
                    m_propertyValue = m_pipeLine.Output.Read();

                    // ouput message for current object
                    string message2 = string.Format( CultureInfo.InvariantCulture,
                               "Powershell Classifier returned too many values while processing file [{0}] in rule [{1}] - returned object of type [{2}], and value [{3}]",
                               propertyBag.VolumeName + propertyBag.RelativePath + "\\" + propertyBag.Name,
                               m_ruleName,
                               m_propertyValue.BaseObject.GetType().ToString(), 
                               m_propertyValue.BaseObject.ToString() );
                    propertyBag.AddMessage(message2);

                    tooManyValues = true;
                }

            } while (lockIndex != m_waitHandleIndex.Enumerator && m_pipeLine.Output.IsOpen);

            // if script didn't output any values for this property bag record it
            if (!readAValue)
            {
                m_ruleNoApply = true;
            }

            // if output too many values finish failing this property
            if (tooManyValues)
            {
                //already added messages for reason why we are failing
                string message3 = string.Format( CultureInfo.InvariantCulture,
                               "Powershell Classifier returned too many values while processing file [{0}] in rule [{1}]",
                               propertyBag.VolumeName + propertyBag.RelativePath + "\\" + propertyBag.Name,
                               m_ruleName );
                throw new COMException( message3, HRESULTS.PS_CLS_E_TOO_MANY_VALUES );
            }

            // if there were errors in the pipeline pop them all off and then fail current property bag
            if (m_pipeLine.Error.Count > 0)
            {

                Exception firstError = null;
                while(m_pipeLine.Error.Count > 0)
                {
                    Exception exception = (Exception)m_pipeLine.Error.Read();
                    if (firstError == null)
                    {
                        firstError = exception;
                    }
                    // add message to pipeline
                    propertyBag.AddMessage(exception.Message);
                }

                throw (Exception)firstError;
            }
        }

        /// <summary>
        /// Reset the rule
        /// </summary>
        private void ResetRuleResults()
        {
            m_propertyValue = null;
            m_ruleNoApply = true;
        }

        /// <summary>
        /// Unload the classifier
        /// </summary>
        public void UnloadRule()
        {
            if (m_pipeLine.Output.IsOpen)
            {
                m_propertyBagWriter.EndInput();
                m_pipeLine.Input.Close();

                //wait for the script to finish
                m_pipeLine.Output.WaitHandle.WaitOne();

                if (m_pipeLine.Output.Count > 0)
                {
                    m_eventLog.WriteEntry("Some output available from pipeline after pipeline closed. This is unexpected and may leave some output unprocessed.",
                                          EventLogEntryType.Error);
                }
            }
            else
            {
                if (m_pipeLine.PipelineStateInfo.State == PipelineState.Failed)
                {
                    ThrowNonPropertyBagException("Pipeline failed and closed unexpectedly.", m_pipeLine.PipelineStateInfo.Reason);
                }
                else
                {
                    ThrowNonPropertyBagException("Pipeline exited unexpectedly without failures.", m_pipeLine.PipelineStateInfo.Reason);
                }
            }

            m_pipeLine.Dispose();
            m_pipeLine = null;
            m_runSpace.Close();
            m_runSpace = null;
            Dispose(true);
        }

        /// <summary>
        /// Throw an error and log it because this classifier CRASHED!
        /// </summary>
        /// <param name="message">The message to save to the log file and throw</param>
        private void ThrowNonPropertyBagException(string message, Exception exception)
        {
            if (m_eventLog == null)
            {
                m_eventLog = new EventLog("Application", ".", "SRM_PS_CLS");
            }

            m_eventLog.WriteEntry( message, EventLogEntryType.Error );
            throw new COMException( message, exception );
        }


        // the property value from powershell
        public object PropertyValue
        {
            get
            {
                return m_propertyValue.BaseObject;
            }
        }

        // Whether the rule applies to the current property bag (if powershell pipeline emitted any objects or not)
        public bool RuleNoApply
        {
            get
            {
                return m_ruleNoApply;
            }
        }

        public string RuleName
        {
            get
            {
                return m_ruleName;
            }
        }
            
    }

    /// <summary>
    /// A class that makes it easy to add methods to the property bag in poweshell
    /// </summary>
    public static class ExtensionClassForPropertyBag
    {
        /// <summary>
        /// A function which returns a Stream object which encapsulates an IStream interface
        /// This makes it much easier to get the file contents in managed code / powershell
        /// </summary>
        /// <param name="parentObject">This will be the PSObject that encapsulates a IFsrmPropertyBag</param>
        /// <returns>A stream which wraps the parentObject's IStream</returns>
        public static StreamWrapperForIStream GetStream(PSObject parentObject)
        {
            IFsrmPropertyBag propertyBag = (IFsrmPropertyBag)parentObject.BaseObject;

            IStream istream = (IStream)propertyBag.GetFileStreamInterface(_FsrmFileStreamingMode.FsrmFileStreamingMode_Read, _FsrmFileStreamingInterfaceType.FsrmFileStreamingInterfaceType_IStream );
            return new StreamWrapperForIStream(istream);

        }
    }
}