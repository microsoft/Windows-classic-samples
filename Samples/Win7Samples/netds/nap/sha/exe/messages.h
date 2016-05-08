//
//  Values are 32 bit values layed out as follows:
//
//   3 3 2 2 2 2 2 2 2 2 2 2 1 1 1 1 1 1 1 1 1 1
//   1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0
//  +---+-+-+-----------------------+-------------------------------+
//  |Sev|C|R|     Facility          |               Code            |
//  +---+-+-+-----------------------+-------------------------------+
//
//  where
//
//      Sev - is the severity code
//
//          00 - Success
//          01 - Informational
//          10 - Warning
//          11 - Error
//
//      C - is the Customer code flag
//
//      R - is a reserved bit
//
//      Facility - is the facility code
//
//      Code - is the facility's status code
//
//
// Define the facility codes
//


//
// Define the severity codes
//


//
// MessageId: SDK_SAMPLE_SHA_WRONG_MSG_ID
//
// MessageText:
//
//  Wrong Message ID.
//
#define SDK_SAMPLE_SHA_WRONG_MSG_ID      ((DWORD)0xC0000001L)

//
// MessageId: SDK_SAMPLE_FRIENDLY_NAME_MSG_ID
//
// MessageText:
//
//  SHA SDK Sample.
//
#define SDK_SAMPLE_FRIENDLY_NAME_MSG_ID  ((DWORD)0x40000002L)

//
// MessageId: SDK_SAMPLE_COMPANY_NAME_MSG_ID
//
// MessageText:
//
//  Microsoft.
//
#define SDK_SAMPLE_COMPANY_NAME_MSG_ID   ((DWORD)0x40000003L)

//
// MessageId: SDK_SAMPLE_VERSION_INFO_MSG_ID
//
// MessageText:
//
//  1.0.0.1.
//
#define SDK_SAMPLE_VERSION_INFO_MSG_ID   ((DWORD)0x40000004L)

//
// MessageId: SDK_SAMPLE_GENERIC_FIXUP_FAILURE_MSG_ID
//
// MessageText:
//
//  Failed to apply fixes.
//
#define SDK_SAMPLE_GENERIC_FIXUP_FAILURE_MSG_ID ((DWORD)0xC0000005L)

//
// MessageId: SDK_SAMPLE_GENERIC_FIXUP_SUCCESS_MSG_ID
//
// MessageText:
//
//  Fixes applied successfully.
//
#define SDK_SAMPLE_GENERIC_FIXUP_SUCCESS_MSG_ID ((DWORD)0x40000006L)

//
// MessageId: SDK_SAMPLE_CLIENT_NOT_PATCHED_MSG_ID
//
// MessageText:
//
//  Client is not patched.
//
#define SDK_SAMPLE_CLIENT_NOT_PATCHED_MSG_ID ((DWORD)0xC0000007L)

//
// MessageId: SDK_SAMPLE_COMPLIANT_CLIENT_MSG_ID
//
// MessageText:
//
//  Client is compliant.
//
#define SDK_SAMPLE_COMPLIANT_CLIENT_MSG_ID ((DWORD)0x40000009L)

//
// MessageId: SDK_SAMPLE_SHA_DESCRIPTION_MSG_ID
//
// MessageText:
//
//  Microsoft SHA SDK Sample.
//
#define SDK_SAMPLE_SHA_DESCRIPTION_MSG_ID ((DWORD)0x4000000AL)

//
// MessageId: SDK_SAMPLE_GENERIC_FIXUP_INPROGRESS_MSG_ID
//
// MessageText:
//
//  Fixes are being applied.
//
#define SDK_SAMPLE_GENERIC_FIXUP_INPROGRESS_MSG_ID ((DWORD)0x4000000BL)

