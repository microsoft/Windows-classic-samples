// --------------------------------------------------------------------
//
//  Copyright (c) Microsoft Corporation.  All rights reserved
//
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// --------------------------------------------------------------------
namespace Microsoft.Samples.MessageQueuing.LargeMessageQueue
{
    using System;
    using System.Text; // for Stringbuilder
    using System.IO; // for Stream
    using System.Globalization;

    internal class Utilities
    {
        // Private Constructor as all members are static at present
        private Utilities()
        {
        }

        // Creates a byte array from hexadecimal string. Two hex are combined to create one byte. 
        public static byte[] GetBytesFromHexaDecimal(string hexaDecimalToConvert)
        {
            char ch;
            StringBuilder sb = new StringBuilder();
            for (int i = 0; i < hexaDecimalToConvert.Length; i++)
            {
                ch = hexaDecimalToConvert[i];
                if (!RemoveNonHexaDecimalChar(ch))
                {
                    sb.Append(ch);
                }
            }

            string stringToConvert = sb.ToString();
            byte[] b = new byte[(stringToConvert.Length / 2)];
            for (int i = 0; i < stringToConvert.Length; i += 2)
            {
                string s = new string(new char[] { stringToConvert[i], stringToConvert[i + 1] });
                b[(i / 2)] = byte.Parse(s, System.Globalization.NumberStyles.HexNumber, CultureInfo.InvariantCulture);
            }

            return b;
        }

        // convert stream to byte array
        internal static byte[] GetBytesFromStream(Stream s)
        {
            byte[] b = new byte[s.Length];
            s.Read(b, 0, b.Length);
            return b;
        }

        // Obtain sub-queue name from a given identifier
        internal static string GetSubQueueName(string correlationId)
        {
            string[] splitId = correlationId.Split('\\');
            byte[] guid = GetBytesFromHexaDecimal(splitId[0]);
            char[] guidChar = new char[guid.Length];

            for (int i = 0; i < guid.Length; i++)
            {
                guidChar[i] = (char)guid[i];
            }

            StringBuilder sb = new StringBuilder();
            for (int i = 0; i < guidChar.Length; i++)
            {
                sb.Append(guidChar[i]);
            }

            sb.Append(splitId[1]);
            return (sb.ToString());
        }

        // On correlationId NOT null, the application is looking for the response message in request-response 
        internal static string GetLargeMessageIdStr(bool delimiter, string correlationId)
        {
            if (correlationId != null)
            {
                return correlationId;
            }

            StringBuilder sb = new StringBuilder();
            sb.Append(Guid.Empty.ToString());
            if (delimiter)
            {
                sb.Append('\\');
            }

            sb.Append(Parameters.LARGE_MESSAGE_ID_DEFAULT);
            return (sb.ToString());
        }

        // Return correlation id that represents empty guid and zero message id
        internal static string GetEmptyCorrelationIdStr()
        {
            StringBuilder sb = new StringBuilder();
            sb.Append(Guid.Empty.ToString());
            sb.Append('\\');
            sb.Append(0);
            return (sb.ToString());
        }

        // convert byte array to stream
        internal static Stream GetStreamFromBytes(byte[] b)
        {
            Stream s = new MemoryStream();
            s.Write(b, 0, b.Length);
            s.Position = 0;
            return s;
        }

        // return true if 'ch' is within hexadecimal range [0,F]
        private static bool RemoveNonHexaDecimalChar(char ch)
        {
            ch = char.ToUpper(ch, CultureInfo.InvariantCulture);

            // ch is between A and F
            if (ch >= 'A' && ch < ('A' + 6))
            {
                return false;
            }

            // c is between 0 and 9
            if (ch >= '0' && ch < ('0' + 10))
            {
                return false;
            }

            return true;
        }
    }
}
