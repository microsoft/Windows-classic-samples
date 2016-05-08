// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.

========================================================================
    CONSOLE APPLICATION : NamedNlsFunctions Project Overview
========================================================================

This sample application demonstrates some of the "named" NLS API
functionality.  Applications should use locale names instead of LCIDs
whenever possible, as this example demonstrates.

This application uses EnumSystemLocalesEx() to enumerate all of the
locales on the system (including custom supplemental locales).

The application can also take one or more locale names as arguments,
which are passed to the enumeration callback in the optional LPARAM.
The callback will then only display the specified locales instead of
displaying all of them.

For each displayed locale we report if it is the system locale, we
print the current date using that locale's default formats, and we
display all of the data for that locale in each of the LCTYPES that
are new to Windows Vista, such as LOCALE_SSCRIPTS. 

We also parse the input locales (if any) to see if they are valid
by using IsValidLocaleName

This sample demonstrates the following APIs:

    CompareStringEx
    EnumSystemLocalesEx
    GetDateFormatEx
    GetLocaleInfoEx
    GetSystemDefaultLocaleName
    IsValidLocaleName    
    LocaleNameToLCID
