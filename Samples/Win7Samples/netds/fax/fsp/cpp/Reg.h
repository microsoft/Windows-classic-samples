//==========================================================================
//
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//
//--------------------------------------------------------------------------

#ifndef _REG_H
#define _REG_H

// FAX_PROVIDERS_REGKEY is the providers registry key under the fax registry key
#define FAX_PROVIDERS_REGKEY           L"Software\\Microsoft\\Fax\\Device Providers"

// NEWFSP_PROVIDER is the newfsp registry key under the providers registry key
#define NEWFSP_PROVIDER                L"{A5CE0FBD-731D-4d1a-8222-81C6C2CD393C}"
// NEWFSP_PROVIDER_FRIENDLYNAME is the friendly name of the newfsp service provider
#define NEWFSP_PROVIDER_FRIENDLYNAME   L"SampleFSP: Sample Windows Fax Service Provider"
// NEWFSP_PROVIDER_IMAGENAME is the image name of the newfsp service provider
#define NEWFSP_PROVIDER_IMAGENAME      L"%SystemRoot%\\system32\\SampleFSP.dll"
// NEWFSP_PROVIDER_PROVIDERNAME is the provider name of the newfsp service provider
#define NEWFSP_PROVIDER_PROVIDERNAME   L"SampleFSP: Sample Windows Fax Service Provider"

// NEWFSP_LOGGING_ENABLED is the registry value indicating if logging is enabled
#define NEWFSP_LOGGING_ENABLED         L"LoggingEnabled"
// NEWFSP_LOGGING_DIRECTORY is the registry value indicating the logging directory
#define NEWFSP_LOGGING_DIRECTORY       L"LoggingDirectory"

// NEWFSP_DEVICES is the virtual fax devices registry key under the newfsp registry key
#define NEWFSP_DEVICES                 L"Devices"

// NEWFSP_DEVICE_DIRECTORY is the registry value indicating the incoming fax directory for the virtual fax device
#define NEWFSP_DEVICE_DIRECTORY        L"IncomingFaxDirectory"

#endif
