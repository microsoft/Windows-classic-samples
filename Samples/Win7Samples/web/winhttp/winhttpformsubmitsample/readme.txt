WinHttpFormSubmitSample
Copyright (c) Microsoft Corporation. All rights reserved. 


  WinHttpFormSubmitSample (FORMSUBMIT.EXE) is a sample application showing how to submit POST-style form requests with WinHTTP.  The key thing is to submit a "POST" request with the proper Content-Type header and a properly formatted POST body.  FORMSUBMIT takes two parameters, a target URL and a source filename.  FORMSUBMIT.EXE sends a POST form request to the target URL with a request body consisting of the source file's contents.  SAMPLE.TXT contains a properly formatted form submit body which can be sent to FORMSUBMIT.ASP set up as described below
  WinHttpFormSubmitSample also demonstrates how to use WinHttpCrackUrl to process URLs.
  
  To build this sample, initialize the sample build environment with "setenv.bat" and then compile by running nmake in this directory.  This sample only builds for an NT environment.

  An indispensable tool when working with HTTP requests is "Microsoft Network Monitor", which can currently be found at http://msdn.microsoft.com/library/default.asp?url=/library/en-us/netmon/netmon/using_network_monitor_2_0.asp.

FORMSUBMIT.ASP is a sample ASP page that can be set up on an IIS server to process requests sent from PostSample.  FORMSUBMIT.HTML is a sample HTML page which can be used to send a form submit request to FORMSUBMIT.ASP from your web browser.  To use these, put these on some server supporting ASP pages in the same directory.  Use Network Monitor (described below) to watch how a typical HTTP client makes such requests.