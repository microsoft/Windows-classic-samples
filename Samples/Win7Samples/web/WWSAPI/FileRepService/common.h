//------------------------------------------------------------
// Copyright (c) Microsoft Corporation.  All rights reserved.
//------------------------------------------------------------

// This file contains definitions that are shared among client and server.
#include "WebServices.h"
#include "stdio.h"

#define CountOf(a) (sizeof(a) / sizeof(*a))
#define ERROR_EXIT ErrorExit:
#define EXIT ErrorExit:
#define EXIT_FUNCTION goto ErrorExit;

#pragma warning(disable : 4127) // Disable warning for the while (0)
#define IfFailedExit(EXPR1) do { hr = (EXPR1); if (FAILED(hr)) { EXIT_FUNCTION } } while (0)
#define IfNullExit(EXPR1) do { if (NULL == EXPR1) { hr = E_OUTOFMEMORY; EXIT_FUNCTION } } while (0)

//
// defines the request message and all related structures
//
struct FileRequest
{
    LONGLONG filePosition; //Starting position of the requested chunk; -1 indicates request for file info.
    LPWSTR fileName; // Fully qualified local file name on the server machine
};

// It is recommended to use a dictionary when dealing with a collection of XML strings.
#pragma warning(disable : 4211) // Disable warning for changing from extern to static
extern WS_XML_DICTIONARY fileRequestDictionary;

static WS_XML_STRING fileRequestDictionaryStrings[] =
{
    WS_XML_STRING_DICTIONARY_VALUE("FilePosition", &fileRequestDictionary, 0),
    WS_XML_STRING_DICTIONARY_VALUE("FileName", &fileRequestDictionary, 1),
    WS_XML_STRING_DICTIONARY_VALUE("FileRequest", &fileRequestDictionary, 2),
    WS_XML_STRING_DICTIONARY_VALUE("http://tempuri.org/FileRep", &fileRequestDictionary, 3),
    WS_XML_STRING_DICTIONARY_VALUE("FileRequest", &fileRequestDictionary, 4),
};

static WS_XML_DICTIONARY fileRequestDictionary =
{
    { /* 615a0125-2a70-4159-bd28-080480339f04 */
    0x615a0125,
    0x2a70,
    0x4159,
    {0xbd, 0x28, 0x08, 0x04, 0x80, 0x33, 0x9f, 0x04}
    },
    fileRequestDictionaryStrings,
    WsCountOf(fileRequestDictionaryStrings),
    true,
};

#define filePositionLocalName fileRequestDictionaryStrings[0]
#define fileNameLocalName fileRequestDictionaryStrings[1]
#define fileRequestLocalName fileRequestDictionaryStrings[2]
#define fileRequestNamespace fileRequestDictionaryStrings[3]
#define fileRequestTypeName fileRequestDictionaryStrings[4]

static WS_FIELD_DESCRIPTION filePositionField = 
{
    WS_ELEMENT_FIELD_MAPPING,
    &filePositionLocalName,
    &fileRequestNamespace,
    WS_INT64_TYPE,
    NULL,
    WsOffsetOf(FileRequest, filePosition),
};

static WS_FIELD_DESCRIPTION fileNameField = 
{ 
    WS_ELEMENT_FIELD_MAPPING,
    &fileNameLocalName,
    &fileRequestNamespace,
    WS_WSZ_TYPE,
    NULL,
    WsOffsetOf(FileRequest, fileName),
};

static WS_FIELD_DESCRIPTION* fileRequestFields[] = 
{ 
    &filePositionField,
    &fileNameField,
};

static WS_STRUCT_DESCRIPTION fileRequestType =
{
    sizeof(FileRequest),
    __alignof(FileRequest),
    fileRequestFields,
    WsCountOf(fileRequestFields),
    &fileRequestTypeName,
    &fileRequestNamespace,
};

static WS_ELEMENT_DESCRIPTION fileRequestElement = 
{
    &fileRequestLocalName,
    &fileRequestNamespace,
    WS_STRUCT_TYPE,
    &fileRequestType,
};

//
// defines the file description message and all related structures
//

struct FileInfo
{
    LPWSTR fileName;
    LONGLONG fileLength;
    DWORD chunkSize;    
};

extern WS_XML_DICTIONARY fileInfoDictionary;

static WS_XML_STRING fileInfoDictionaryStrings[] =
{
    WS_XML_STRING_DICTIONARY_VALUE("FileName", &fileInfoDictionary, 0),
    WS_XML_STRING_DICTIONARY_VALUE("FileLength", &fileInfoDictionary, 1),
    WS_XML_STRING_DICTIONARY_VALUE("ChunkSize", &fileInfoDictionary, 2),
    WS_XML_STRING_DICTIONARY_VALUE("FileInfo", &fileInfoDictionary, 3),
    WS_XML_STRING_DICTIONARY_VALUE("http://tempuri.org/FileRep", &fileInfoDictionary, 4),
    WS_XML_STRING_DICTIONARY_VALUE("FileInfo", &fileInfoDictionary, 5),
};

static WS_XML_DICTIONARY fileInfoDictionary =
{
    { /* 22b4b751-eede-4d35-9307-50a2fcba3d4e */
    0x22b4b751,
    0xeede,
    0x4d35,
    {0x93, 0x07, 0x50, 0xa2, 0xfc, 0xba, 0x3d, 0x4e}
    },
    fileInfoDictionaryStrings,
    WsCountOf(fileInfoDictionaryStrings),
    true,
};

#define fileNameInfoLocalName fileInfoDictionaryStrings[0]
#define fileLengthLocalName fileInfoDictionaryStrings[1]
#define chunkSizeLocalName fileInfoDictionaryStrings[2]
#define fileInfoLocalName fileInfoDictionaryStrings[3]
#define fileInfoNamespace fileInfoDictionaryStrings[4]
#define fileInfoElementName fileInfoDictionaryStrings[5]

static WS_FIELD_DESCRIPTION fileNameInfoField = 
{
    WS_ELEMENT_FIELD_MAPPING,
    &fileNameInfoLocalName,
    &fileInfoNamespace,
    WS_WSZ_TYPE,
    NULL,
    WsOffsetOf(FileInfo, fileName),
};

static WS_FIELD_DESCRIPTION fileLengthField = 
{
    WS_ELEMENT_FIELD_MAPPING,
    &fileLengthLocalName,
    &fileInfoNamespace,
    WS_INT64_TYPE,
    NULL,
    WsOffsetOf(FileInfo, fileLength),
};

static WS_FIELD_DESCRIPTION chunkSizeField = 
{
    WS_ELEMENT_FIELD_MAPPING,
    &chunkSizeLocalName,
    &fileInfoNamespace,
    WS_UINT32_TYPE,
    NULL,
    WsOffsetOf(FileInfo, chunkSize),
};

static WS_FIELD_DESCRIPTION* fileInfoFields[] = 
{ 
    &fileNameInfoField,
    &fileLengthField,
    &chunkSizeField,
};

static WS_STRUCT_DESCRIPTION fileInfoType =
{
    sizeof(FileInfo),
    __alignof(FileInfo),
    fileInfoFields,
    WsCountOf(fileInfoFields),
    &fileInfoElementName,
    &fileInfoNamespace,
};

static WS_ELEMENT_DESCRIPTION fileInfoElement = 
{
    &fileInfoLocalName,
    &fileInfoNamespace,
    WS_STRUCT_TYPE,
    &fileInfoType,
};


//
// defines the file message and all related structures
//
struct FileChunk
{
    LONGLONG chunkPosition; // Starting position of the transmitted chunk. Must match request. Undefined in error case. 
    
    // Size of the content must match the defined chunk length or be last chunk, 
    // in which case size+chunkPosition must match the length of the file.
    WS_BYTES fileContent;  
    LPWSTR error;  // Contains "http://tempuri.org/FileRep/NoError" in the success case.
};

extern WS_XML_DICTIONARY fileChunkDictionary;

static WS_XML_STRING fileChunkDictionaryStrings[] =
{
    WS_XML_STRING_DICTIONARY_VALUE("ChunkPosition", &fileChunkDictionary, 0),
    WS_XML_STRING_DICTIONARY_VALUE("FileContent", &fileChunkDictionary, 1),
    WS_XML_STRING_DICTIONARY_VALUE("Error", &fileChunkDictionary, 2),
    WS_XML_STRING_DICTIONARY_VALUE("FileChunk", &fileChunkDictionary, 3),
    WS_XML_STRING_DICTIONARY_VALUE("http://tempuri.org/FileRep", &fileChunkDictionary, 4),
    WS_XML_STRING_DICTIONARY_VALUE("FileChunk", &fileChunkDictionary, 5),
};

static WS_XML_DICTIONARY fileChunkDictionary =
{
    { /* e99f3780-7a2a-449a-a40f-bf9f4a5db648 */
    0xe99f3780,
    0x7a2a,
    0x449a,
    {0xa4, 0x0f, 0xbf, 0x9f, 0x4a, 0x5d, 0xb6, 0x48}
    },
    fileChunkDictionaryStrings,
    WsCountOf(fileChunkDictionaryStrings),
    true,
};

#define chunkPositionLocalName fileChunkDictionaryStrings[0]
#define fileContentLocalName fileChunkDictionaryStrings[1]
#define errorLocalName fileChunkDictionaryStrings[2]
#define fileChunkLocalName fileChunkDictionaryStrings[3]
#define fileChunkNamespace fileChunkDictionaryStrings[4]
#define fileChunkElementName fileChunkDictionaryStrings[5]

static WS_FIELD_DESCRIPTION chunkPositionField = 
{
    WS_ELEMENT_FIELD_MAPPING,
    &chunkPositionLocalName,
    &fileChunkNamespace,
    WS_INT64_TYPE,
    NULL,
    WsOffsetOf(FileChunk, chunkPosition),
};


static WS_FIELD_DESCRIPTION fileContentField = 
{
    WS_ELEMENT_FIELD_MAPPING,
    &fileContentLocalName,
    &fileChunkNamespace,
    WS_BYTES_TYPE,
    NULL,
    WsOffsetOf(FileChunk, fileContent),
};

static WS_FIELD_DESCRIPTION errorField = 
{
    WS_ELEMENT_FIELD_MAPPING,
    &errorLocalName,
    &fileChunkNamespace,
    WS_WSZ_TYPE,
    NULL,
    WsOffsetOf(FileChunk, error),
};

static WS_FIELD_DESCRIPTION* fileChunkFields[] = 
{ 
    &chunkPositionField,
    &fileContentField,
    &errorField,    
};

static WS_STRUCT_DESCRIPTION fileChunkType =
{
    sizeof(FileChunk),
    __alignof(FileChunk),
    fileChunkFields,
    WsCountOf(fileChunkFields),
    &fileChunkElementName,
    &fileChunkNamespace,
};

static WS_ELEMENT_DESCRIPTION fileChunkElement = 
{
    &fileChunkLocalName,
    &fileChunkNamespace,
    WS_STRUCT_TYPE,
    &fileChunkType,
};

typedef enum
{
    HTTP_TRANSPORT = 1,
    TCP_TRANSPORT = 2,
} TRANSPORT_MODE;

typedef enum
{
    NO_SECURITY = 1,
    SSL_SECURITY = 2,   
} SECURITY_MODE;

typedef enum
{
    BINARY_ENCODING = 1,
    TEXT_ENCODING = 2,
    MTOM_ENCODING = 3,
    DEFAULT_ENCODING = 4,
} MESSAGE_ENCODING;

typedef enum
{
    ASYNC_REQUEST = 1,
    SYNC_REQUEST = 2,   
} REQUEST_TYPE;

//
// defines the user request message and all related structures
//
struct UserRequest
{
    REQUEST_TYPE requestType; // If true the request completes synchronously
    LPWSTR serverUri; // Uri of the server service
    TRANSPORT_MODE serverProtocol;
    SECURITY_MODE securityMode;
    MESSAGE_ENCODING messageEncoding; 
    LPWSTR sourcePath; // Fully qualified local file name of the source file
    LPWSTR destinationPath;// Fully qualified local file name of the destination file
};

extern WS_XML_DICTIONARY userRequestDictionary;

static WS_XML_STRING userRequestDictionaryStrings[] =
{
    WS_XML_STRING_DICTIONARY_VALUE("RequestType", &userRequestDictionary, 0),
    WS_XML_STRING_DICTIONARY_VALUE("ServerUri", &userRequestDictionary, 1),
    WS_XML_STRING_DICTIONARY_VALUE("ServerProtocol", &userRequestDictionary, 2),
    WS_XML_STRING_DICTIONARY_VALUE("SecurityMode", &userRequestDictionary, 3),
    WS_XML_STRING_DICTIONARY_VALUE("MessageEncoding", &userRequestDictionary, 4),
    WS_XML_STRING_DICTIONARY_VALUE("SourcePath", &userRequestDictionary, 5),
    WS_XML_STRING_DICTIONARY_VALUE("DestinationPath", &userRequestDictionary, 6),
    WS_XML_STRING_DICTIONARY_VALUE("UserRequest", &userRequestDictionary, 7),
    WS_XML_STRING_DICTIONARY_VALUE("http://tempuri.org/FileRep", &userRequestDictionary, 8),
    WS_XML_STRING_DICTIONARY_VALUE("UserRequest", &userRequestDictionary, 9),
};

static WS_XML_DICTIONARY userRequestDictionary =
{
    { /* 98e77255-271b-4434-abb6-3ab6d3b5a6a4 */
    0x98e77255,
    0x271b,
    0x4434,
    {0xab, 0xb6, 0x3a, 0xb6, 0xd3, 0xb5, 0xa6, 0xa4}
    },
    userRequestDictionaryStrings,
    WsCountOf(userRequestDictionaryStrings),
    true,
};


#define requestTypeLocalName userRequestDictionaryStrings[0]
#define serverUriLocalName userRequestDictionaryStrings[1]
#define serverProtocolLocalName userRequestDictionaryStrings[2]
#define securityModeLocalName userRequestDictionaryStrings[3]
#define messageEncodingLocalName userRequestDictionaryStrings[4]
#define sourcePathLocalName userRequestDictionaryStrings[5]
#define destinationPathLocalName userRequestDictionaryStrings[6]
#define userRequestLocalName userRequestDictionaryStrings[7]
#define userRequestNamespace userRequestDictionaryStrings[8]
#define userRequestTypeName userRequestDictionaryStrings[9]

static WS_FIELD_DESCRIPTION requestTypeField = 
{
    WS_ELEMENT_FIELD_MAPPING,
    &requestTypeLocalName,
    &userRequestNamespace,
    WS_INT32_TYPE,
    NULL,
    WsOffsetOf(UserRequest, requestType),
};

static WS_FIELD_DESCRIPTION serverUriField = 
{ 
    WS_ELEMENT_FIELD_MAPPING,
    &serverUriLocalName,
    &userRequestNamespace,
    WS_WSZ_TYPE,
    NULL,
    WsOffsetOf(UserRequest, serverUri),
};

static WS_FIELD_DESCRIPTION serverProtocolField = 
{ 
    WS_ELEMENT_FIELD_MAPPING,
    &serverProtocolLocalName,
    &userRequestNamespace,
    WS_INT32_TYPE,
    NULL,
    WsOffsetOf(UserRequest, serverProtocol),
};

static WS_FIELD_DESCRIPTION securityModeField = 
{ 
    WS_ELEMENT_FIELD_MAPPING,
    &securityModeLocalName,
    &userRequestNamespace,
    WS_INT32_TYPE,
    NULL,
    WsOffsetOf(UserRequest, securityMode),
};

static WS_FIELD_DESCRIPTION messageEncodingField = 
{ 
    WS_ELEMENT_FIELD_MAPPING,
    &messageEncodingLocalName,
    &userRequestNamespace,
    WS_INT32_TYPE,
    NULL,
    WsOffsetOf(UserRequest, messageEncoding),
};

static WS_FIELD_DESCRIPTION sourcePathField = 
{ 
    WS_ELEMENT_FIELD_MAPPING,
    &sourcePathLocalName,
    &userRequestNamespace,
    WS_WSZ_TYPE,
    NULL,
    WsOffsetOf(UserRequest, sourcePath),
};

static WS_FIELD_DESCRIPTION destinationPathField = 
{ 
    WS_ELEMENT_FIELD_MAPPING,
    &destinationPathLocalName,
    &userRequestNamespace,
    WS_WSZ_TYPE,
    NULL,
    WsOffsetOf(UserRequest, destinationPath),
};

static WS_FIELD_DESCRIPTION* userRequestFields[] = 
{ 
    &requestTypeField,
    &serverUriField,
    &serverProtocolField,
    &securityModeField,
    &messageEncodingField,
    &sourcePathField,
    &destinationPathField,
};

static WS_STRUCT_DESCRIPTION userRequestType =
{
    sizeof(UserRequest),
    __alignof(UserRequest),
    userRequestFields,
    WsCountOf(userRequestFields),
    &userRequestTypeName,
    &userRequestNamespace,
};

static WS_ELEMENT_DESCRIPTION userRequestElement = 
{
    &userRequestLocalName,
    &userRequestNamespace,
    WS_STRUCT_TYPE,
    &userRequestType,
};


//
// defines the user response message and all related structures
//

typedef enum
{
    TRANSFER_SUCCESS = 1, // Failures are returned as faults.
    TRANSFER_ASYNC = 2, // Request completes asynchronously. Client will not be notified of success or failure.
} TRANSFER_RESULTS;

struct UserResponse
{
    TRANSFER_RESULTS returnValue; 
};

extern WS_XML_DICTIONARY userResponseDictionary;

static WS_XML_STRING userResponseDictionaryStrings[] =
{
    WS_XML_STRING_DICTIONARY_VALUE("ReturnValue", &userResponseDictionary, 0),
    WS_XML_STRING_DICTIONARY_VALUE("UserResponse", &userResponseDictionary, 1),
    WS_XML_STRING_DICTIONARY_VALUE("http://tempuri.org/FileRep", &userResponseDictionary, 2),
    WS_XML_STRING_DICTIONARY_VALUE("UserResponse", &userResponseDictionary, 3),
};

static WS_XML_DICTIONARY userResponseDictionary =
{
    { /* 3c13293c-665f-4586-85eb-954a3279a500 */
    0x3c13293c,
    0x665f,
    0x4586,
    {0x85, 0xeb, 0x95, 0x4a, 0x32, 0x79, 0xa5, 0x00}
    },
    userResponseDictionaryStrings,
    WsCountOf(userResponseDictionaryStrings),
    true,
};


#define returnValueLocalName userResponseDictionaryStrings[0]
#define userResponseLocalName userResponseDictionaryStrings[1]
#define userResponseNamespace userResponseDictionaryStrings[2]
#define userResponseTypeName userResponseDictionaryStrings[3]

static WS_FIELD_DESCRIPTION returnValueField = 
{
    WS_ELEMENT_FIELD_MAPPING,
    &returnValueLocalName,
    &userResponseNamespace,
    WS_INT32_TYPE,
    NULL,
    WsOffsetOf(UserResponse, returnValue),
};


static WS_FIELD_DESCRIPTION* userResponseFields[] = 
{ 
    &returnValueField,
};

static WS_STRUCT_DESCRIPTION userResponseType =
{
    sizeof(UserResponse),
    __alignof(UserResponse),
    userResponseFields,
    WsCountOf(userResponseFields),
    &userResponseTypeName,
    &userResponseNamespace,
};

static WS_ELEMENT_DESCRIPTION userResponseElement = 
{
    &userResponseLocalName,
    &userResponseNamespace,
    WS_STRUCT_TYPE,
    &userResponseType,
};


// Set up the action value for the file request message
static WS_XML_STRING fileRequestAction = WS_XML_STRING_VALUE("http://tempuri.org/FileRep/filerequest");

// Set up the action value for the reply message
static WS_XML_STRING fileReplyAction = WS_XML_STRING_VALUE("http://tempuri.org/FileRep/filereply");

// Set up the action value for the info message
static WS_XML_STRING fileInfoAction = WS_XML_STRING_VALUE("http://tempuri.org/FileRep/fileinfo");


// Set up the action value for the user request message
static WS_XML_STRING userRequestAction = WS_XML_STRING_VALUE("http://tempuri.org/FileRep/userrequest");

// Set up the action value for the user reply message
static WS_XML_STRING userResponseAction = WS_XML_STRING_VALUE("http://tempuri.org/FileRep/userresponse");

// Set up the action value for the fault message
static WS_XML_STRING faultAction = WS_XML_STRING_VALUE("http://tempuri.org/FileRep/fault");


// We are allowing very large messages so that the server can chose the optimal
// message size. This could be improved by allowing the client and server to negotiate
// this value but that is beyond the scope of this version of the sample.
#define MAXMESSAGESIZE 536870912 //half gigabyte 
