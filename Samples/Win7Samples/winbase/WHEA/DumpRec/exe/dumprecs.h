// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved

/*++

Module Name:

    dumprecs.h

Abstract:

    This module contains the identifiers for string resources used in the
    Dumprec SDK sample.

--*/

#pragma once

#define IDS_WHEA_UNKNOWN                                        0x1

//
// Label strings
//

#define IDS_LABEL_INVALID_ERROR_RECORD                          0x10
#define IDS_LABEL_HEADER                                        0x11
#define IDS_LABEL_SEVERITY                                      0x12
#define IDS_LABEL_SEVERITY_WITH_FLAGS                           0x13
#define IDS_LABEL_TIMESTAMP                                     0x14
#define IDS_LABEL_PLATFORM_ID                                   0x15
#define IDS_LABEL_PARTITION_ID                                  0x16
#define IDS_LABEL_NOTIFICATION_TYPE                             0x17

//
// Error severity strings. The order of the string identifiers is significant.
// The string identifier must be a linear function of the severity value.
//

#define IDS_WHEA_SEVERITY_BASE                                  IDS_WHEA_SEVERITY_RECOVERABLE
#define IDS_WHEA_SEVERITY_RECOVERABLE                           0x102
#define IDS_WHEA_SEVERITY_FATAL                                 0x103
#define IDS_WHEA_SEVERITY_CORRECTED                             0x104
#define IDS_WHEA_SEVERITY_INFORMATIONAL                         0x105
#define IDS_WHEA_SEVERITY_MAX                                   IDS_WHEA_SEVERITY_INFORMATIONAL

//
// Friendly strings to describe the error record header flags.
//

#define IDS_WHEA_RECORD_FLAGS_RECOVERED                         0x106
#define IDS_WHEA_RECORD_FLAGS_PREVIOUSERROR                     0x107
#define IDS_WHEA_RECORD_FLAGS_SIMULATED                         0x108

//
// Friendly strings to describe the well-know notification types.
//

#define IDS_WHEA_GUID_CMC_NOTIFY_TYPE                           0x109
#define IDS_WHEA_GUID_CPE_NOTIFY_TYPE                           0x10a
#define IDS_WHEA_GUID_MCE_NOTIFY_TYPE                           0x10b
#define IDS_WHEA_GUID_PCIE_NOTIFY_TYPE                          0x10c
#define IDS_WHEA_GUID_INIT_NOTIFY_TYPE                          0x10d
#define IDS_WHEA_GUID_NMI_NOTIFY_TYPE                           0x10e
#define IDS_WHEA_GUID_BOOT_NOTIFY_TYPE                          0x10f

//
// Friendly strings to describe the well-known error record section types.
//

#define IDS_WHEA_GUID_PROCESSOR_GENERIC_ERROR_SECTION           0x110
#define IDS_WHEA_GUID_XPF_PROCESSOR_ERROR_SECTION               0x111
#define IDS_WHEA_GUID_IPF_PROCESSOR_ERROR_SECTION               0x112
#define IDS_WHEA_GUID_MEMORY_ERROR_SECTION                      0x113
#define IDS_WHEA_GUID_PCIEXPRESS_ERROR_SECTION                  0x114
#define IDS_WHEA_GUID_PCIXBUS_ERROR_SECTION                     0x115
#define IDS_WHEA_GUID_PCIXDEVICE_ERROR_SECTION                  0x116
#define IDS_WHEA_GUID_FIRMWARE_ERROR_RECORD_REFERENCE_SECTION   0x117

//
// Friendly strings to describe the various processor generic error types.
//

#define IDS_WHEA_GUID_CACHE_CHECK                               0x118
#define IDS_WHEA_GUID_TLB_CHECK                                 0x119
#define IDS_WHEA_GUID_BUS_CHECK                                 0x11a
#define IDS_WHEA_GUID_MS_CHECK                                  0x11b

//
// Friendly string used to describe the Microsoft WHEA error record creator.
//

#define IDS_WHEA_GUID_WHEA_RECORD_CREATOR                       0x11c

//
// Friendly strings to describe the Windows defined notification types.
//

#define IDS_WHEA_GUID_GENERIC_NOTIFY_TYPE                       0x11d

//
// Friendly strings to describe the Windows defined record section types.
//

#define IDS_WHEA_GUID_IPF_SAL_RECORD_SECTION                    0x11e
#define IDS_WHEA_GUID_XPF_MCA_SECTION                           0x11f
#define IDS_WHEA_GUID_NMI_SECTION                               0x120
#define IDS_WHEA_GUID_GENERIC_SECTION                           0x121
#define IDS_WHEA_GUID_ERROR_PACKET_SECTION                      0x122

