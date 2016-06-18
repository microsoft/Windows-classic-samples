// <copyright file="ObjectCommandComparer.cs" company="Microsoft Corporation">
// Copyright (c) 2009 Microsoft Corporation. All rights reserved.
// </copyright>
// DISCLAIMER OF WARRANTY: The software is licensed “as-is.” You 
// bear the risk of using it. Microsoft gives no express warranties, 
// guarantees or conditions. You may have additional consumer rights 
// under your local laws which this agreement cannot change. To the extent 
// permitted under your local laws, Microsoft excludes the implied warranties 
// of merchantability, fitness for a particular purpose and non-infringement.


// Description: Helper class to support select-obj cmdlet's unique functionality.
//
// System namespaces needed
using System;
using System.Collections.Generic;
using System.Collections;
using System.Text;
using System.Globalization;
using System.Threading;


// Windows PowerShell namespaces needed
using System.Management.Automation;

namespace Microsoft.Samples.PowerShell.Commands
{
    #region PSObject Comparer

    /// <summary>
    /// Keeps the property value of inputObject. Because the value of a non-existing property is null,
    ///  isExistingProperty is needed to distinguish whether a property exists and its value is null or
    ///  the property does not exist at all.
    /// </summary>
    internal class ObjectCommandPropertyValue
    {
        private ObjectCommandPropertyValue() { }

        internal ObjectCommandPropertyValue(object propVal)
        {
            propertyValue = propVal;
            isExistingProperty = true;
        }
        internal object PropertyValue
        {
            get
            {
                return propertyValue;
            }
        }
        internal bool IsExistingProperty
        {
            get
            {
                return isExistingProperty;
            }
        }
        private object propertyValue;
        private bool isExistingProperty;
        internal readonly static ObjectCommandPropertyValue NonExistingProperty = new ObjectCommandPropertyValue();
        internal readonly static ObjectCommandPropertyValue ExistingNullProperty = new ObjectCommandPropertyValue(null);
    }

    /// <summary>
    /// 
    /// </summary>
    internal class ObjectCommandComparer : IComparer
    {
        /// <summary>
        /// Constructor that doesn't set any private field.
        /// Necessary because compareTo can compare two objects by calling
        /// ((ICompare)obj1).CompareTo(obj2) without using a key.
        /// </summary>
        internal ObjectCommandComparer(bool ascending, CultureInfo cultureInfo, bool caseSensitive)
        {
            this.ascendingOrder = ascending;
            this.cultureInfo = cultureInfo;
            if (this.cultureInfo == null)
            {
                this.cultureInfo = Thread.CurrentThread.CurrentCulture;
            }
            this.caseSensitive = caseSensitive;
        }

        /// <summary>
        /// Check whether <paramref name="obj"/> can be converted to an
        /// PSObject and then check whehter the PSObject is null.
        /// </summary>
        /// <param name="obj"></param>
        /// <returns></returns>
        private static bool IsValueNull(object obj)
        {
            PSObject psObj = obj as PSObject;
            if (null == psObj)
            {
                return (null == obj);
            }

            // obj is converted to an psobject and is found to be null..
            if (psObj == null)
                return true;

            if (null == psObj.ImmediateBaseObject)
            {
                return (null == obj);
            }

            object returnValue = null;
            do
            {
                returnValue = psObj.ImmediateBaseObject;
                psObj = returnValue as PSObject;
            } while ((psObj != null) && (! (null == psObj.ImmediateBaseObject)));

            return (null == returnValue);
        }


        internal int Compare(ObjectCommandPropertyValue first, ObjectCommandPropertyValue second)
        {
            if (first.IsExistingProperty && second.IsExistingProperty)
            {
                return Compare(first.PropertyValue, second.PropertyValue);
            }
            // if first.IsExistingProperty, !second.IsExistingProperty; otherwise the
            // first branch if would return. Regardless of key orders non existing property 
            // will be considered greater than others
            if (first.IsExistingProperty)
            {
                return -1;
            }
            // viceversa for the first.IsExistingProperty
            if (second.IsExistingProperty)
            {
                return 1;
            }
            //both are nonexisting
            return 0;
        }

        /// <summary>
        /// Main method that will compare first and second by
        /// their keys considering case and order
        /// </summary>
        /// <param name="first">
        /// first object to extract value
        /// </param>
        /// <param name="second">
        /// second object to extract value
        /// </param>
        /// <returns> 
        /// 0 if they are the same, less than 0 if first is smaller, more than 0 if first is greater
        ///</returns>
        public int Compare(object first, object second)
        {
            // This method will never throw exceptions, two null
            // objects are considered the same
            if (IsValueNull(first) && IsValueNull(second)) return 0;


            PSObject firstPS = first as PSObject;
            if (firstPS != null)
            {
                first = firstPS.BaseObject;
            }

            PSObject secondPS = second as PSObject;
            if (secondPS != null)
            {
                second = secondPS.BaseObject;
            }

            try
            {
                return LanguagePrimitives.Compare(first, second, !caseSensitive, cultureInfo) * (ascendingOrder ? 1 : -1);
            }
            catch (InvalidCastException)
            {
            }
            catch (ArgumentException)
            {
                // Note that this will occur if the objects do not support
                // IComparable.  We fall back to comparing as strings.
            }

            // being here means the first object doesn't support ICompare
            // or an Exception was raised with Compare
            string firstString = PSObject.AsPSObject(first).ToString();
            string secondString = PSObject.AsPSObject(second).ToString();
            return string.Compare(firstString, secondString, !caseSensitive, cultureInfo) * (ascendingOrder ? 1 : -1);
        }

        private CultureInfo cultureInfo = null;

        private bool ascendingOrder = true;

        private bool caseSensitive = false;

    }

    #endregion
}


