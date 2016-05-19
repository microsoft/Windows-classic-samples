#pragma once

// The following macros define the minimum required platform.  The minimum required platform
// is the earliest version of Windows, Internet Explorer etc. that has the necessary features to run 
// your application.  The macros work by enabling all features available on platform versions up to and 
// including the version specified.

// This file has been updated with the values found in WinSDKVer.h.  In order to target earlier 
// platforms, change the the following values to the targeted platform.

#ifndef WINVER
#define WINVER _WIN32_MAXVER  // Use value found in WinSDkVer.h
#endif

#ifndef _WIN32_WINNT
#define _WIN32_WINNT _WIN32_WINNT_MAXVER  // Use value found in WinSDkVer.h
#endif

#ifndef _WIN32_WINDOWS
#define _WIN32_WINDOWS _WIN32_WINDOWS_MAXVER  // Use value found in WinSDkVer.h
#endif

#ifndef _WIN32_IE
#define _WIN32_IE _WIN32_IE_MAXVER  // Use value found in WinSDkVer.h
#endif
