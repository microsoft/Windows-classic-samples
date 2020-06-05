---
page_type: sample
languages:
- cpp
products:
- windows-api-win32
name: WinHttpPost sample
urlFragment: winhttppost-sample
description: Demonstrates how to post with WinHttp perhaps using SSL and perhaps through proxy and/or server which require authentication.
extendedZipContent:
- path: LICENSE
  target: LICENSE
---

# WinHttpPostSample

WinHttpPostSample (*POSTSAMPLE.EXE*) is a sample application showing how to post with WinHttp perhaps using SSL and perhaps through proxy and/or server which require authentication. It takes three parameters, the server URL, the POST contents pointed to by a filename, and the proxy URL. The URLs are of the form

     http[s]://username:password@host:port/path

Where *username*, *password*, and *port* are optional. The proxy url must be http not https, and may not include a path  Technically the *username:password@* part makes this not a valid HTTP url, but its a convenient way to pass credentials to this test app.

## Build the sample  

1. Initialize the sample build environment with *setenv.bat*.
1. Compile the by running `nmake` in this directory. This sample only builds for an NT environment.

An indispensable tool when working with HTTP requests is Microsoft Network Monitor, which can currently be found at http://msdn.microsoft.com/library/default.asp?url=/library/en-us/netmon/netmon/using_network_monitor_2_0.asp.

*DATA.ASP* is a simple ASP page that responds with about the same response body as it saw in the request body. It can be set up on an IIS server to try out this sample. 

Copyright (c) Microsoft Corporation. All rights reserved. 
