// -----------------------------------------------------------------------
// <copyright file="OdataJsonRequest.cs" company="Microsoft">
//     Copyright (C) 2012 Microsoft Corporation
// </copyright>
// -----------------------------------------------------------------------

namespace Microsoft.Samples.Management.OData.AssociationClient
{
    using System.Collections.Generic;
    using System.IO;
    using System.Net;

        /// <summary>
        /// Represents an HTTP request to the ODATA server.
        /// </summary>
    internal class OdataJsonRequest
    {
        /// <summary>
        /// the HTTP request
        /// </summary>
        private HttpWebRequest request;

        /// <summary>
        /// URL options that should be added to the request
        /// </summary>
        private Dictionary<string, string> options;

        /// <summary>
        /// Headers option value
        /// </summary>
        private Dictionary<string, string> headers;

        /// <summary>
        /// HTTP verb of the request
        /// </summary>
        private string verb;

        /// <summary>
        /// URL of the request
        /// </summary>
        private string url;

        /// <summary>
        /// credentials for the request
        /// </summary>
        private ICredentials credentials;

        /// <summary>
        /// the server response 
        /// </summary>
        private HttpWebResponse response;

        /// <summary>
        /// Initializes a new instance of the OdataJsonRequest class.
        /// </summary>
        /// <param name="verb">HTTP verb</param>
        /// <param name="url">the url</param>
        /// <param name="credentials">credentials, typically a CredentialCache</param>
        public OdataJsonRequest(string verb, string url, ICredentials credentials)
        {
            this.options = new Dictionary<string, string>();
            this.headers = new Dictionary<string, string>();
            this.url = url;
            this.verb = verb;
            this.credentials = credentials;
        }

        /// <summary>
        /// Gets or sets the body of the request.  
        /// </summary>
        public string RequestBody { get; set; }

        /// <summary>
        /// Gets the body of the server response.  Valid only after SendReceive();
        /// </summary>
        public string ResponseBody { get; private set; }

        /// <summary>
        /// Gets the status code of the server response.  Valid only after SendReceive();
        /// </summary>
        public HttpStatusCode ResponseStatus { get; private set; }

        /// <summary>
        /// Gets the decoded response body 
        /// </summary>
        public dynamic Response { get; private set; }

        /// <summary>
        /// Checks whether a reference set contains a given element.
        /// The set and the element are expected to be sub-elements of a ResponseBody 
        /// of a previous request.
        /// </summary>
        /// <param name="set">the reference-set (in a response body)</param>
        /// <param name="element">the reference (in a response body)</param>
        /// <returns>true if the set contains the element</returns>
        public static bool JsonReferenceSetContains(dynamic set, dynamic element)
        {
            return JsonReferenceSetContains(set, element.__metadata.uri);
        }

        /// <summary>
        /// Encodes a single name-value pair in JSON format.
        /// </summary>
        /// <param name="key">the key</param>
        /// <param name="value">the value</param>
        /// <returns>the encoded pair</returns>
        public static string EncodeJsonElement(string key, string value)
        {
            return "\"" + key + "\": \"" + value + "\"";
        }

        /// <summary>
        /// Encodes a single name-value pair in JSON format.
        /// </summary>
        /// <param name="key">the key</param>
        /// <param name="value">the value</param>
        /// <returns>the encoded pair</returns>
        public static string EncodeJsonElement(string key, int value)
        {
            return "\"" + key + "\": " + value.ToString();
        }

        /// <summary>
        ///  Starts JSON encoding of a struct property, writing the field name and open-brace in JSON format.
        /// </summary>
        /// <param name="name">the field name</param>
        /// <returns>the encoded string</returns>
        public static string BeginJsonStruct(string name)
        {
            return "\"" + name + "\": {";
        }

        /// <summary>
        ///  Finishes JSON encoding of a struct property, writing close-brace.
        /// </summary>
        /// <returns>the encoded string</returns>
        public static string EndJsonStruct()
        {
            return "}";
        }

        /// <summary>
        /// Adds a URL option to the request.
        /// </summary>
        /// <param name="name">option name</param>
        /// <param name="value">option value</param>
        public void AddOption(string name, string value)
        {
            this.options.Add(name, value);
        }

        /// <summary>
        /// Adds a header value
        /// </summary>
        /// <param name="name">option name</param>
        /// <param name="value">option value</param>
        public void AddHeader(string name, string value)
        {
            this.headers.Add(name, value);
        }

        /// <summary>
        /// Sends the request and waits for the response.
        /// </summary>
        public void SendReceive()
        {
            // add URL options
            if (this.options.Count > 0)
            {
                string fullOptions = string.Empty;
                foreach (KeyValuePair<string, string> pair in this.options)
                {
                    if (fullOptions.Length == 0)
                    {
                        fullOptions += "?" + pair.Key + "=" + pair.Value;
                    }
                    else
                    {
                        fullOptions += "&" + pair.Key + "=" + pair.Value;
                    }
                }

                this.url += fullOptions;
            }

            // create the request
            this.request = (HttpWebRequest)WebRequest.Create(this.url);

            foreach (KeyValuePair<string, string> header in this.headers)
            {
                this.request.Headers.Add(header.Key, header.Value);
            }

            this.request.Credentials = this.credentials;
            this.request.Method = this.verb;
            this.request.Accept = "application/json;odata=verbose";

            // send the request data
            if (false == string.IsNullOrEmpty(this.RequestBody))
            {
                System.Text.ASCIIEncoding encoding = new System.Text.ASCIIEncoding();
                byte[] requestData = encoding.GetBytes(this.RequestBody);
                this.request.ContentLength = requestData.Length;
                this.request.ContentType = "application/json;odata=verbose";

                Stream requestStream = this.request.GetRequestStream();
                requestStream.Write(requestData, 0, requestData.Length);
                requestStream.Close();
            }

            // wait for the response and parse it
            try
            {
                this.response = (HttpWebResponse)this.request.GetResponse();
            }
            catch (WebException ex)
            {
                this.response = (HttpWebResponse)ex.Response;
            }

            this.ResponseStatus = this.response.StatusCode;

            StreamReader stream = new StreamReader(this.response.GetResponseStream());
            this.ResponseBody = stream.ReadToEnd();

            if (this.ResponseStatus == HttpStatusCode.OK ||
                this.ResponseStatus == HttpStatusCode.Created)
            {
                System.Management.Automation.ErrorRecord error;
                dynamic ret = Microsoft.PowerShell.Commands.JsonObject.ConvertFromJson(this.ResponseBody, out error);
                this.Response = ret;
            }
        }
    }
}
