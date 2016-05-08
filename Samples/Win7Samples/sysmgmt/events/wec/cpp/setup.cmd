@ECHO OFF

REM THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
REM ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
REM THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
REM PARTICULAR PURPOSE.
REM
REM Copyright (c) Microsoft Corporation. All rights reserved.

pushd %~dp0


REM Configure the winrm Service

    call winrm quickconfig -quiet 
    timeout /T 5

REM Configure the wecsvc Service 
    
    call wecutil qc /q 
    timeout /T 5


:NormalExit
    Echo. Setup completed.
    Echo.
popd



