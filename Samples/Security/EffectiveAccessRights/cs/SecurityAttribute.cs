// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved

using System;
using System.Collections.Generic;
using System.Collections.Specialized;
using System.Globalization;
using System.Diagnostics;
using System.Diagnostics.CodeAnalysis;
using System.Linq;
using System.Runtime.InteropServices;
using System.Runtime.Serialization;
using System.Security.Principal;
using System.Text.RegularExpressions;

namespace Microsoft.Samples.DynamicAccessControl
{
    using Utility;

    using USHORT = System.UInt16;
    using ULONG = System.UInt32;
    using ULONG64 = System.UInt64;

    using AUTHZ_CLIENT_CONTEXT_HANDLE = System.IntPtr;
    using PAUTHZ_SECURITY_ATTRIBUTE_V1 = System.IntPtr;

    /// <summary>
    /// Exception raised when value(s) of a claim value type is invalid.
    /// </summary>
    [Serializable]
    public class BadValueException : Exception, ISerializable
    {
        #region Constructors
        public BadValueException()
        { }

        public BadValueException(string message)
            : base(message)
        { }

        public BadValueException(string message, Exception innerException)
            : base(message, innerException)
        { }

        protected BadValueException(SerializationInfo info, StreamingContext context)
            : base(info, context)
        { }
        #endregion
    }

    /// <summary>
    /// Possible types of values supported in claims.
    /// </summary>
    /// <remarks>These maps to the value type that can be specified from
    /// a claim type </remarks>
    internal enum ClaimValueType
    {
        Integer,
        Boolean,
        String,
        MultiValuedString
    }

    /// <summary>
    /// Class to represent the type of claims values held, the value(s) and
    /// obtain native (unmanaged) pointers to the value as they are stored in
    /// the union members of AUTHZ_SECURITY_ATTRIBUTE_V1 structure's 'Values'
    /// field.
    /// </summary>
    internal class ClaimValue
    {
        #region Constructor
        /// <summary>
        /// Constructor to initialize a object given the ValueType and a list of
        /// string, each representing one value in string form.
        /// </summary>
        /// <param name="valType">Type of the claim value</param>
        /// <param name="vals"></param>
        public ClaimValue(ClaimValueType valueType, string value)
        {
            this.valueType = valueType;

            //
            // Attempt to get the raw values so that any ill-formated value would throw an exception
            // and get rejected right away.
            //
            rawValues = GetRawValues(valueType, value, out valueCount);
        }
        #endregion

        #region Public properties
        /// <summary>
        /// Gets the Microsoft.Samples.Cbac.ClaimValueType of the current
        /// instance.
        /// </summary>
        public ClaimValueType ValueType
        {
            get { return valueType; }
        }

        /// <summary>
        /// Get the number of values contained in the
        /// Microsoft.Samples.Cbac.ClaimValue
        /// </summary>
        public ULONG ValueCount
        {
            get { return valueCount; }
        }

        /// <summary>
        /// Get the native (unmanaged) representation of the values.
        /// </summary>
        /// <remarks>The returned native (unmanaged) representation is meant for
        /// use in 'Values' field of AUTHZ_SECURITY_ATTRUBUTE_V1.</remarks>
        public SafeHGlobalHandle RawValues
        {
            get { return rawValues; }
        }
        #endregion

        #region Private implementation
        /// <summary>
        /// Get the native (unmanaged) representation of the list of values in
        /// the Microsoft.Samples.Cbac.ClaimValue instance.
        /// </summary>
        /// <param name="valueType">Type of the value(s) in this instance</param>
        /// <param name="values">The collection of values each in string format</param>
        /// <param name="valueCount">Return the count of unique values</param>
        /// <returns>SafeHGlobalHandle that references a native (unmanaged)
        /// pointer to values that can be used in the 'Values' field of the
        /// CLAIM_SECURITY_ATTRIBUTE_V1 structure.</returns>
        static SafeHGlobalHandle GetRawValues(ClaimValueType valueType,
                                              string value,
                                              out ULONG valueCount)
        {
            const int BASE_OCTAL = 8;
            const int BASE_DECIMAL = 10;
            const int BASE_HEX = 16;

            const string OCTAL_REGEX = "^[+]?0[0-7]+$";
            const string HEX_REGEX = "^[+]?0[xX][0-9a-fA-f]+$";

            var stringValues = new StringCollection();

            valueCount = 1;

            //
            // As part of formulating the values in native format, verify that
            // we do not have duplicates. AuthzModifyClaims fails with
            // ERROR_ALREADY_EXISTS when duplicate values are specified.
            //
            switch (valueType)
            {
                case ClaimValueType.Integer:
                    {
                        long[] values = new long[1];
                        try
                        {
                            int fromBase = BASE_DECIMAL;

                            if (Regex.Match(value, OCTAL_REGEX).Success)
                            {
                                fromBase = BASE_OCTAL;
                            }
                            else if (Regex.Match(value, HEX_REGEX).Success)
                            {
                                fromBase = BASE_HEX;
                            }

                            values[0] = Convert.ToInt64(value, fromBase);

                            return SafeHGlobalHandle.AllocHGlobal(values);
                        }
                        catch (Exception e)
                        {
                            throw new BadValueException(string.Format(CultureInfo.CurrentCulture,
                                                                      "Invalid Int value - {0}",
                                                                      value),
                                                        e);
                        }
                    }

                case ClaimValueType.Boolean:
                    {
                        long[] values = new long[1];

                        try
                        {
                            string strValue = value;

                            if (string.Compare(value, "true", StringComparison.OrdinalIgnoreCase) == 0)
                            {
                                strValue = "1";
                            }
                            else if (string.Compare(value, "false", StringComparison.OrdinalIgnoreCase) == 0)
                            {
                                strValue = "0";
                            }

                            values[0] = Convert.ToInt64(strValue, CultureInfo.InvariantCulture);

                            return SafeHGlobalHandle.AllocHGlobal(values);
                        }
                        catch (Exception e)
                        {
                            throw new BadValueException(string.Format(CultureInfo.CurrentCulture,
                                                        "Invalid Boolean value - {0}",
                                                        value), e);
                        }
                    }
                case ClaimValueType.MultiValuedString:
                    {
                        char[] bracketChars = { '[', ']' };
                        const string CSV_REGEX = @"# Parse CVS line. Capture next value in named group: 'val'
                                                   \s*                      # Ignore leading whitespace.
                                                   (?:                      # Group of value alternatives.
                                                       ""                   # Either a double quoted string,
                                                       (?<val>              # Capture contents between quotes.
                                                       [^""]*(""""[^""]*)*  # Zero or more non-quotes, allowing 
                                                       )                    # doubled "" quotes within string.
                                                       ""\s*                # Ignore whitespace following quote.
                                                   |  (?<val>[^,]+)         # Or... One or more non-commas.
                                                   )                        # End value alternatives group.
                                                   (?:,|$)                  # Match end is comma or EOS";

                        if (!value.StartsWith("[", StringComparison.Ordinal) ||
                            !value.EndsWith("]", StringComparison.Ordinal))
                        {
                            throw new BadValueException(string.Format(CultureInfo.CurrentCulture,
                                                        "Multi-valued String is not enclosed within square brackets: '{0}'",
                                                        value));
                        }

                        MatchCollection splitResult = Regex.Matches(value.Trim(bracketChars),
                                                                    CSV_REGEX,
                                                                      RegexOptions.Multiline
                                                                    | RegexOptions.IgnorePatternWhitespace);

                        if (splitResult.Count == 0)
                        {
                            throw new BadValueException(string.Format(CultureInfo.CurrentCulture,
                                                        "Ill-formed Multi-valued String: '{0}'",
                                                        value));
                        }
                        else
                        {
                            foreach (Match literal in splitResult)
                            {
                                string strVal = literal.Groups["val"].Value.Trim();
                                if (!stringValues.Contains(strVal))
                                {
                                    if (!string.IsNullOrEmpty(strVal))
                                    {
                                        stringValues.Add(strVal);
                                    }
                                }
                                else
                                {
                                    Helper.ReportDuplicateValue(ClaimValueType.MultiValuedString,
                                                                literal.Groups["val"].Value);
                                }
                            }
                        }

                        if (stringValues.Count == 0)
                        {
                            throw new BadValueException(string.Format(CultureInfo.CurrentCulture,
                                                        "No non-empty strings in : '{0}'",
                                                        value));
                        }

                        valueCount = (ULONG)stringValues.Count;

                        goto case ClaimValueType.String;
                    }
                case ClaimValueType.String:
                    {
                        if (stringValues.Count == 0)
                        {
                            string strVal = value.Trim();

                            if (!string.IsNullOrEmpty(strVal))
                            {
                                stringValues.Add(strVal);
                            }
                        }

                        var strings = new List<SafeHGlobalHandle>(stringValues.Count);

                        foreach (var stringValue in stringValues)
                        {
                            SafeHGlobalHandle nativeString = SafeHGlobalHandle.AllocHGlobal(stringValue);

                            strings.Add(nativeString);
                        }

                        SafeHGlobalHandle result = SafeHGlobalHandle.AllocHGlobal(
                                                                        strings.Select(n => n.ToIntPtr())
                                                                               .ToArray());

                        //
                        // Since the native (managed) representation is an array
                        // of pointers to strings, ensure that these pointers
                        // are being referenced in the uber SafeHGlobalHandle
                        // that represents the array of pointers.
                        //
                        result.AddSubReference(strings);

                        valueCount = (ULONG)strings.Count;

                        return result;
                    }
                default:
                    {
                        valueCount = 0;
                        break;
                    }
            }

            return SafeHGlobalHandle.InvalidHandle;
        }
        #endregion

        #region Private members
        ClaimValueType valueType;
        ULONG valueCount;

        /// <summary>
        /// Holds the native (unmanaged) representation of the values
        /// </summary>
        SafeHGlobalHandle rawValues;
        #endregion
    }

    /// <summary>
    /// Enumeration used to identify if a ClaimValueDictionary comprised of user
    /// or device claims.
    /// </summary>
    internal enum ClaimDefinitionType
    {
        User,
        Device
    }

    /// <summary>
    /// Class to represent a set of claim values(s) and to facilitate applying
    /// these to an Authz client context
    /// </summary>
    [Serializable]
    internal class ClaimValueDictionary : Dictionary<string, ClaimValue>, ISerializable
    {
        #region Constructor
        /// <summary>
        /// Identifies if this instance represents user's claims or device's
        /// claims
        /// </summary>
        /// <param name="type">ClaimDefinitionType.User to indicate user's
        /// claims and ClaimDefinitionType.Device to indicate device's claims.
        /// </param>
        /// <remarks>When ClaimDefinitionType.User, AithzModifyClaims in invoked
        /// with SidClass AuthzContextInfoUserClaims and when
        /// ClaimDefinitionType.Device with SidClass
        /// AuthzContextInfoDeviceClaims.</remarks>
        public ClaimValueDictionary(ClaimDefinitionType type)
        {
            claimDefnType = type;
        }
        #endregion

        #region Public methods
        /// <summary>
        /// Adds or replaces claims in the specified Authz Client Context.
        /// </summary>
        /// <remarks>This method invokes AuthzModifyClaims, modifying the claims
        /// using AUTHZ_SECURITY_ATTRIBUTE_OPERATION_REPLACE. This ensures that
        /// the values of a claims that already exists are replaces and the ones
        /// not present are added.</remarks>
        /// <param name="handleClientContext">Handle to the Authz Client Context to be modified</param>
        /// <returns>Win32Error.ERROR_SUCCESS on success and Win32 error code otherwise.</returns>
        public int ApplyClaims(AUTHZ_CLIENT_CONTEXT_HANDLE handleClientContext)
        {
            NativeMethods.AuthzSecurityAttributeOperation[] claimOps = null;
            var claims = new List<NativeMethods.AUTHZ_SECURITY_ATTRIBUTE_V1>(this.Count);

            foreach (var claim in this)
            {
                //
                // If all of the value specified turned out invalid, ignore the claim altogether.
                //
                if (claim.Value.ValueCount == 0)
                {
                    continue;
                }

                var attribute = new NativeMethods.AUTHZ_SECURITY_ATTRIBUTE_V1();

                attribute.Name = claim.Key;
                attribute.Flags = 0;
                attribute.Values = claim.Value.RawValues.ToIntPtr();
                attribute.ValueCount = claim.Value.ValueCount;

                switch(claim.Value.ValueType)
                {
                    case ClaimValueType.Integer:
                        {
                            Debug.Assert(attribute.ValueCount == 1);
                            attribute.Type = NativeMethods.AuthzSecurityAttributeValueType.Int;
                            break;
                        }
                    case ClaimValueType.Boolean:
                        {
                            Debug.Assert(attribute.ValueCount == 1);
                            attribute.Type = NativeMethods.AuthzSecurityAttributeValueType.Boolean;
                            break;
                        }
                    case ClaimValueType.String:
                        {
                            Debug.Assert(attribute.ValueCount == 1);
                            goto case ClaimValueType.MultiValuedString;
                        }
                    case ClaimValueType.MultiValuedString:
                        {
                            attribute.Type = NativeMethods.AuthzSecurityAttributeValueType.String;
                            break;
                        }
                }

                claims.Add(attribute);
            }

            var claimInfo = new NativeMethods.AUTHZ_SECURITY_ATTRIBUTES_INFORMATION();

            claimInfo.Version = 1; // AUTHZ_SECURITY_ATTRIBUTES_INFORMATION_VERSION_V1
            claimInfo.Reserved = 0;
            claimInfo.AttributeCount = (ULONG)claims.Count;

            SafeHGlobalHandle v1Attributes = SafeHGlobalHandle.InvalidHandle;

            if (claimInfo.AttributeCount != 0)
            {
                v1Attributes = SafeHGlobalHandle.AllocHGlobal(claims);

                claimOps = new NativeMethods.AuthzSecurityAttributeOperation[claimInfo.AttributeCount];
                for (ULONG Idx = 0; Idx < claimInfo.AttributeCount; ++Idx)
                {
                    claimOps[Idx] = NativeMethods.AuthzSecurityAttributeOperation.Replace;
                }
            }

            claimInfo.pAttributeV1 = v1Attributes.ToIntPtr();

            if (!NativeMethods.AuthzModifyClaims(handleClientContext,
                                                 claimDefnType == ClaimDefinitionType.User
                                                 ? NativeMethods.AuthzContextInformationClass.AuthzContextInfoUserClaims
                                                 : NativeMethods.AuthzContextInformationClass.AuthzContextInfoDeviceClaims,
                                                 claimOps,
                                                 ref claimInfo))
            {
                return Marshal.GetLastWin32Error();
            }

            return Win32Error.ERROR_SUCCESS;
        }
        #endregion

        #region ISerialization implementation
        protected ClaimValueDictionary(SerializationInfo info, StreamingContext context)
            : base(info, context)
        { }

        public override void GetObjectData(SerializationInfo info, StreamingContext context)
        {
            base.GetObjectData(info, context);

            if (info != null)
            {
                info.AddValue("claimDefnType", this.claimDefnType);
            }
        }
        #endregion

        #region Private members
        ClaimDefinitionType claimDefnType;
        #endregion

        #region Nested class for P/Invokes and native (Win32) structures
        static class NativeMethods
        {
            [StructLayout(LayoutKind.Sequential)]
            public struct AUTHZ_SECURITY_ATTRIBUTES_INFORMATION
            {
                public USHORT Version;
                public USHORT Reserved;
                public ULONG AttributeCount;
                public PAUTHZ_SECURITY_ATTRIBUTE_V1 pAttributeV1;
            }

            public enum AuthzSecurityAttributeValueType : ushort
            {
                Invalid = 0x0,
                Int     = 0x1,
                String  = 0x3,
                Boolean = 0x6,
            }

            [Flags]
            public enum AuthzSecurityAttributeFlags : uint // ULONG
            {
                None = 0x0,
                NonInheritable = 0x1,
                ValueCaseSensitive = 0x2,
            }

            [StructLayout(LayoutKind.Sequential)]
            public struct AUTHZ_SECURITY_ATTRIBUTE_V1
            {
                [MarshalAs(UnmanagedType.LPWStr)]
                public string Name;
                public AuthzSecurityAttributeValueType Type;
                public USHORT Reserved;
                public AuthzSecurityAttributeFlags Flags;
                public ULONG ValueCount;
                public IntPtr Values;
            }

            public enum AuthzContextInformationClass : uint
            {
                AuthzContextInfoUserClaims = 13,
                AuthzContextInfoDeviceClaims,
            };

            public enum AuthzSecurityAttributeOperation : uint
            {
                None = 0,
                ReplaceAll,
                Add,
                Delete,
                Replace
            }

            [DllImport(Win32.AUTHZ_DLL, CharSet = CharSet.Unicode, SetLastError = true)]
            [return: MarshalAs(UnmanagedType.Bool)]
            public static extern bool AuthzModifyClaims(
                AUTHZ_CLIENT_CONTEXT_HANDLE handleClientContext,
                AuthzContextInformationClass infoClass,
                AuthzSecurityAttributeOperation[] claimOperation,
                ref AUTHZ_SECURITY_ATTRIBUTES_INFORMATION claims);
        }
        #endregion
    }
}
