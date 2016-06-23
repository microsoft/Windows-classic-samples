// <copyright file="SelectObj.cs" company="Microsoft Corporation">
// Copyright (c) 2009 Microsoft Corporation. All rights reserved.
// </copyright>
// DISCLAIMER OF WARRANTY: The software is licensed “as-is.” You 
// bear the risk of using it. Microsoft gives no express warranties, 
// guarantees or conditions. You may have additional consumer rights 
// under your local laws which this agreement cannot change. To the extent 
// permitted under your local laws, Microsoft excludes the implied warranties 
// of merchantability, fitness for a particular purpose and non-infringement.

// System namespaces needed
using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Globalization;

// Windows PowerShell namespaces needed
using System.Management.Automation;

namespace Microsoft.Samples.PowerShell.Commands
{
    /// <summary>
    /// A cmdlet to select properties from objects.
    /// </summary>
    /// <remarks>
    /// Making this a sealed class. Change this if you need to derrive from this class.
    /// </remarks>
    [Cmdlet("Select", "Obj")]
    public sealed class SelectObjCommand : Cmdlet
    {
        #region Parameters

        /// <summary>
        /// Object that this cmdlet will work on.
        /// Can be either from pipeline or directly specified 
        /// as an argument to the cmdlet.
        /// </summary>
        /// <value></value>
        [Parameter(ValueFromPipeline = true)]
        public PSObject InputObject
        {
            set { inputObject = value; }
            get { return inputObject; }
        }
        private PSObject inputObject;

        /// <summary>
        /// Property parameter for the cmdlet.
        /// Takes an array of strings as input.
        /// </summary>
        [Parameter(Position = 0)]
        public string[] Property
        {
            get { return properties; }
            set { properties = value; }
        }
        private string[] properties;

        /// <summary>
        /// Helps to select specific number of objects from the
        /// last object.
        /// </summary>
        [Parameter]
        [ValidateRange(1, int.MaxValue)]
        public int Last
        {
            get { return last; }
            set { last = value; }
        }
        private int last = 0;

        /// <summary>
        /// Helps to select specific number of objects from the
        /// first object.
        /// </summary>
        [Parameter]
        [ValidateRange(1, int.MaxValue)]
        public int First
        {
            get { return first; }
            set { first = value; }
        }
        private int first = 0;

        /// <summary>
        /// Expand specific properties from the selected object.
        /// </summary>
        /// <value></value>
        [Parameter]
        public string Expand
        {
            get { return expand; }
            set { expand = value; }
        }
        private string expand = null;

        /// <summary>
        /// Specifies whether to select unique objects.
        /// </summary>
        /// <value>true or false</value>
        [Parameter]
        public bool Unique
        {
            get { return unique; }
            set { unique = value; }
        }
        private bool unique = false;

        #endregion

        #region Internal Properties/Data

        private SelectObjQueue inputQueue;
        private List<UniquePSObjectHelper> uniques = null;

        #endregion

        #region Cmdlet Overrides

        /// <summary>
        /// Initialize cmdlet data here.
        /// This method is called only once even when the cmdlet
        /// is used in the middle a pipe.
        /// </summary>
        protected override void BeginProcessing()
        {
            WriteVerbose("Initializing Object Queue");
            
            if (unique)
            {
                uniques = new List<UniquePSObjectHelper>();
            }

            inputQueue = new SelectObjQueue(first, last);
        }

        /// <summary>
        /// Do per object related work here.
        /// This method is called for every object that is coming
        /// from the input pipe.
        /// </summary>
        protected override void ProcessRecord()
        {
            if (null == inputObject)
            {
                return;
            }

            WriteVerbose("Received an input Object. Processing...");

            inputQueue.Enqueue(inputObject);

            // Determine whether we can process this object right away.
            // We can process the object
            // (a) if no first or last is specified.
            // (b) first is specified and the limit is not reached.
            PSObject streamingInputObject = inputQueue.StreamingDequeue();

            if (streamingInputObject != null)
            {
                ProcessObject(streamingInputObject);
                
                WriteVerbose("Processed InputObject");             
            }
        }

        /// <summary>
        /// This method is called after all the objects are processed.
        /// This method is called as the last instruction in a pipe.
        /// 
        /// For select-obj, we will do all the object manipulation 
        /// in this method.
        /// </summary>
        protected override void EndProcessing()
        {
            WriteVerbose("Processing remaining Input objects");
            foreach (PSObject targetObject in inputQueue)
            {
                ProcessObject(targetObject);
            }

            if (uniques != null)
            {
                foreach (UniquePSObjectHelper obj in uniques)
                {
                    if (obj.WrittenObject == null)
                    {
                        continue;
                    }
                    WriteObject(obj.WrittenObject);
                }
            }
        }

        #endregion

        #region Internal Methods

        private void ProcessObject(PSObject inputObject)
        {
            if ((properties == null || properties.Length == 0) && string.IsNullOrEmpty(expand))
            {
                SendToOutputPipe(inputObject, new List<PSNoteProperty>());
                return;
            }
            // Collect property values for the properties specified by
            // the user.
            List<PSNoteProperty> matchedProperties = new List<PSNoteProperty>();

            if (properties != null && properties.Length > 0)
            {
                WriteVerbose("Collecting property values for the user specified properties");
                // Create a collection of property values.
                foreach (string property in properties)
                {
                    ProcessParameter(property, inputObject, matchedProperties);
                }
            }

            if (!string.IsNullOrEmpty(expand))
            {
                WriteVerbose("Expanding a property");
                ProcessExpandParameter(inputObject, matchedProperties);
            }
            else
            {
                // Create a new PSObject and embed properties that the user requested.
                PSObject outputObject = new PSObject();

                if (matchedProperties.Count != 0)
                {
                    foreach (PSNoteProperty noteProperty in matchedProperties)
                    {
                        try
                        {
                            outputObject.Properties.Add(noteProperty);
                        }
                        catch (ExtendedTypeSystemException)
                        {
                            WriteAlreadyExistingPropertyError(noteProperty.Name, inputObject);
                        }
                    }
                }

                SendToOutputPipe(outputObject, matchedProperties);
            }            
        }

        /// <summary>
        /// Gets the value for the property specified by <paramref name="propertyName"/> from
        /// <paramref name="inputObject"/> and adds the value to <paramref name="matchedProperties"/>
        /// </summary>
        /// <param name="propertyName">Property to get the value for.</param>
        /// <param name="inputObject">Input object to get the value from.</param>
        /// <param name="matchedProperties">Collection to add the value to.</param>
        /// <remarks>
        /// <paramref name="matchedProperties"/> is assumed to be initialized.
        /// </remarks>
        private void ProcessParameter(string propertyName, 
            PSObject inputObject,
            List<PSNoteProperty> matchedProperties)
        {
            object propValue = GetPropValue(inputObject, propertyName);
            
            // propValue can be null.
            PSNoteProperty newProperty = new PSNoteProperty(propertyName, propValue);
            matchedProperties.Add(newProperty);
        }

        private void ProcessExpandParameter(PSObject inputObject, List<PSNoteProperty> matchedProperties)
        {
            // expand parameter is used
            // Expand the property value from the inputObject
            object propValue = null;

            try
            {
                propValue = GetPropValue(inputObject, expand);
            }
            catch (Exception e)
            {
                WriteNonTerminatingError(expand, "PropertyAccessException",
                    e, ErrorCategory.InvalidData);

                return;
            }

            if (null == propValue)
            {
                string message = string.Format(CultureInfo.CurrentCulture,GetErrorMessage(PROPERTYNOTFOUND),
                    expand);

                Exception e = new ArgumentException(message);
                WriteNonTerminatingError(expand, "PropertyNotFound",
                    e, ErrorCategory.InvalidData);

                return;
            }
            
            // Expand property value using an enumerator.
            System.Collections.IEnumerable results = LanguagePrimitives.GetEnumerable(propValue);
            if (results == null)
            {
                string message = string.Format(CultureInfo.CurrentCulture,GetErrorMessage(NOTHINGTOEXPAND),
                        expand);

                Exception e = new ArgumentException(message);
                WriteNonTerminatingError(expand, "NothingToExpand",
                    e, ErrorCategory.InvalidArgument);

                return;
            }

            foreach (object expandedValue in results)
            {
                if (expandedValue == null)
                {
                    WriteVerbose("One of the expanded values is null");
                    continue;
                }

                PSObject expandedObject = PSObject.AsPSObject(expandedValue);
                foreach (PSNoteProperty noteProperty in matchedProperties)
                {
                    try
                    {
                        if (expandedObject.Properties[noteProperty.Name] != null)
                        {
                            WriteAlreadyExistingPropertyError(noteProperty.Name, inputObject);
                        }
                        else
                        {
                            expandedObject.Properties.Add(noteProperty);
                        }
                    }
                    catch (ExtendedTypeSystemException)
                    {
                        WriteAlreadyExistingPropertyError(noteProperty.Name, inputObject);
                    }
                }

                SendToOutputPipe(expandedObject, matchedProperties);
            }
        }

        private void SendToOutputPipe(PSObject objectToWrite, List<PSNoteProperty> addedNoteProperties)
        {
            if (!unique)
            {
                if (objectToWrite != null)
                {
                    WriteObject(objectToWrite);
                }

                return;
            }

            bool isObjUnique = true;

            foreach (UniquePSObjectHelper uniqueObj in uniques)
            {
                ObjectCommandComparer comparer = new ObjectCommandComparer(true, System.Threading.Thread.CurrentThread.CurrentCulture, true);
                if ((comparer.Compare(objectToWrite.BaseObject, uniqueObj.WrittenObject.BaseObject) == 0) &&
                    (uniqueObj.NotePropertyCount == addedNoteProperties.Count))
                {
                    bool found = true;
                    foreach (PSNoteProperty note in addedNoteProperties)
                    {
                        PSMemberInfo prop = uniqueObj.WrittenObject.Properties[note.Name];
                        if (prop == null || comparer.Compare(prop.Value, note.Value) != 0)
                        {
                            found = false;
                            break;
                        }
                    }
                    if (found)
                    {
                        isObjUnique = false;
                        break;
                    }
                }
                else
                {
                    continue;
                }
            }
            if (isObjUnique)
            {
                uniques.Add(new UniquePSObjectHelper(objectToWrite, addedNoteProperties.Count));
            }
        }

        /// <summary>
        /// Returns the value of property from the inputObject.
        /// </summary>
        /// <param name="inputObject">Object for which the property value is needed.</param>
        /// <param name="property">Name of the property for which the value is returned.</param>
        /// <returns>
        /// null if property is not found,
        /// value of the property otherwise.
        /// </returns>
        private object GetPropValue(PSObject inputObject, string property)
        {
            // This is an internal method. Validate input passed to this method.
            Debug.Assert(inputObject != null, "Cannot work with a null object.");
            Debug.Assert(!string.IsNullOrEmpty(property), "Property should not be null or Empty");

            // We have no globbing: try an exact match, because this is quicker.
            PSMemberInfo x = inputObject.Members[property];
            return null == x ? null : x.Value;           
        }

        #endregion

        #region Error Handling / Error Messages

        private const int PROPERTYALREADYEXISTS = 0;
        private const int PROPERTYNOTFOUND = 1;
        private const int NOTHINGTOEXPAND = 2;

        private string[] errors =
            new string[] { 
                "Property cannot be processed because property {0} already exists.",
                "Property \"{0}\" cannot be found",
                "Cannot expand property \"{0}\" because it has nothing to expand."
           };

        /// <summary>
        /// Gets error message string corresponding to <paramref name="errorID"/>
        /// </summary>
        /// <param name="errorID">Error ID of the error message</param>
        /// <returns>A string representing the error.</returns>
        /// <remarks>
        /// You can this error subsystem to return locale specific error messages.
        /// </remarks>
        private string GetErrorMessage(int errorID)
        {
            return errors[errorID];
        }

        /// <summary>
        /// Writes a non-terminating error onto the pipeline.
        /// </summary>
        /// <param name="targetObject">Object which caused this exception.</param>
        /// <param name="errorId">ErrorId for this error.</param>
        /// <param name="innerException">Complete exception object.</param>
        /// <param name="category">ErrorCategory for this exception.</param>
        internal void WriteNonTerminatingError(
            Object targetObject,
            string errorId,
            Exception innerException,
            ErrorCategory category)
        {
            WriteError(new ErrorRecord(innerException, errorId, category, targetObject));
        }

        internal void WriteAlreadyExistingPropertyError(string propertyName, object inputObject)
        {
            string message = string.Format(CultureInfo.CurrentCulture, GetErrorMessage(PROPERTYALREADYEXISTS),
                                propertyName);

            Exception e = new ArgumentException(message);
            WriteNonTerminatingError(inputObject, "PropertyAlreadyExists",
                e, ErrorCategory.InvalidData);
        }

        #endregion

        #region SelectObj Queue

        private class SelectObjQueue : Queue<PSObject>
        {
            #region SelectObjQueue Data
            
            int headLimit = 0;
            int tailLimit = 0;
            int streamedObjectCount = 0;

            #endregion

            #region SelectObjQueue Construction

            public SelectObjQueue(int first, int last)
            {
                headLimit = first;
                tailLimit = last;
            }

            #endregion

            #region SelectObjQueue Methods

            new public void Enqueue(PSObject obj)
            {
                if (tailLimit > 0 && this.Count >= tailLimit)
                {
                    base.Dequeue();
                }
                base.Enqueue(obj);
            }

            public PSObject StreamingDequeue()
            {
                if ((headLimit == 0 && tailLimit == 0) || streamedObjectCount < headLimit)
                {
                    Debug.Assert(this.Count > 0, "Streaming an empty queue");
                    streamedObjectCount++;
                    return Dequeue();
                }
                if (tailLimit == 0)
                {
                    Dequeue();
                }

                return null;
            }

            #endregion
        }

        #endregion

        #region UniquePSObjectHelper

        private class UniquePSObjectHelper
        {
            internal UniquePSObjectHelper(PSObject o, int notePropertyCount)
            {
                WrittenObject = o;
                this.notePropertyCount = notePropertyCount;
            }
            internal readonly PSObject WrittenObject;
            internal int NotePropertyCount
            {
                get { return notePropertyCount; }
            }
            private int notePropertyCount;
        }

        #endregion
    }
}

