//
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved
//


#include <mi.h>
#include <conio.h>
#include <strsafe.h>
#include <stdlib.h>
#include <ctype.h>
#include "utilities.h"

/* Query the user for a single character selection of a specific set of characters */
wchar_t GetUserSelection(const wchar_t *displayString, const wchar_t *validOptions)
{
    wchar_t selection;

    do
    {
        wprintf(displayString);
        selection = _getwch();

    } while (wcschr(validOptions, towlower(selection)) == NULL);
    wprintf(L"%c\n", selection);
    return selection;
}

/* Input a string from a user, allowing the caller to pass in a default value if the input string was zero length */
void GetUserInputString(_In_z_ const wchar_t *displayString, _Out_writes_z_(outputStringLength) wchar_t *outputString, size_t outputStringLength, _In_z_ const wchar_t *defaultString)
{
    size_t finalLength = 0;

    if ((displayString[0] != L'\0') && (defaultString[0] != L'\0'))
    {
        wprintf(L"%s [%s]: ", displayString, defaultString);
    }
    else if (displayString[0] != L'\0' && (defaultString[0] == L'\0'))
    {
        wprintf(L"%s: ", displayString);
    }
    else if (displayString[0] == L'\0' && (defaultString[0] != L'\0'))
    {
        wprintf(L"[%s]: ", defaultString);
    }

    if (outputStringLength == 0)
        return;

    _cgetws_s(outputString, outputStringLength, &finalLength);
    outputString[outputStringLength-1] = L'\0';

    if ((outputString[0] == L'\0') && (defaultString[0] != L'\0'))
    {
        if (StringCchCopyW(outputString, outputStringLength, defaultString) != S_OK)
        {
            outputString[0] = L'\0';
        }
    }
    wprintf(L"<%s>\n", outputString);
}

/* Helper function to convert an MI_Result into a string */
const wchar_t *MI_Result_To_String(MI_Result miResult)
{
    static const wchar_t *miResultStrings[] = 
    {
        L"MI_RESULT_OK",
        L"MI_RESULT_FAILED",
        L"MI_RESULT_ACCESS_DENIED",
        L"MI_RESULT_INVALID_NAMESPACE",
        L"MI_RESULT_INVALID_PARAMETER",
        L"MI_RESULT_INVALID_CLASS",
        L"MI_RESULT_NOT_FOUND",
        L"MI_RESULT_NOT_SUPPORTED",
        L"MI_RESULT_CLASS_HAS_CHILDREN",
        L"MI_RESULT_CLASS_HAS_INSTANCES",
        L"MI_RESULT_INVALID_SUPERCLASS",
        L"MI_RESULT_ALREADY_EXISTS",
        L"MI_RESULT_NO_SUCH_PROPERTY",
        L"MI_RESULT_TYPE_MISMATCH",
        L"MI_RESULT_QUERY_LANGUAGE_NOT_SUPPORTED",
        L"MI_RESULT_INVALID_QUERY",
        L"MI_RESULT_METHOD_NOT_AVAILABLE",
        L"MI_RESULT_METHOD_NOT_FOUND",
        L"MI_RESULT_NAMESPACE_NOT_EMPTY",
        L"MI_RESULT_INVALID_ENUMERATION_CONTEXT",
        L"MI_RESULT_INVALID_OPERATION_TIMEOUT",
        L"MI_RESULT_PULL_HAS_BEEN_ABANDONED",
        L"MI_RESULT_PULL_CANNOT_BE_ABANDONED",
        L"MI_RESULT_FILTERED_ENUMERATION_NOT_SUPPORTED",
        L"MI_RESULT_CONTINUATION_ON_ERROR_NOT_SUPPORTED",
        L"MI_RESULT_SERVER_LIMITS_EXCEEDED",
        L"MI_RESULT_SERVER_IS_SHUTTING_DOWN"
    };

    if (miResult < sizeof(miResultStrings)/sizeof(miResultStrings[0]))
    {
        return miResultStrings[miResult];
    }
    else
    {
        return L"Unknown MI_Result";
    }
}

/* Helper function to convert an MI_Type into a string */
const MI_Char *MI_Type_To_String(MI_Type type)
{
    static const MI_Char *stringTable[] = 
    {
        L"MI_BOOLEAN", 
        L"MI_UINT8",
        L"MI_SINT8",
        L"MI_UINT16",
        L"MI_SINT16",
        L"MI_UINT32",
        L"MI_SINT32",
        L"MI_UINT64",
        L"MI_SINT64",
        L"MI_REAL32",
        L"MI_REAL64",
        L"MI_CHAR16",
        L"MI_DATETIME",
        L"MI_STRING",
        L"MI_REFERENCE",
        L"MI_INSTANCE",
        L"MI_BOOLEANA",
        L"MI_UINT8A",
        L"MI_SINT8A",
        L"MI_UINT16A",
        L"MI_SINT16A",
        L"MI_UINT32A",
        L"MI_SINT32A",
        L"MI_UINT64A",
        L"MI_SINT64A",
        L"MI_REAL32A",
        L"MI_REAL64A",
        L"MI_CHAR16A",
        L"MI_DATETIMEA",
        L"MI_STRINGA",
        L"MI_REFERENCEA",
        L"MI_INSTANCEA"
    };

    if (type >= sizeof(stringTable)/sizeof(stringTable[0]))
    {
        return L"unknown";
    }
    else
    {
        return stringTable[type];
    }
}

/* Helper function to print a number of spaces used to make the instance printing a little easier to read */
void Indent(size_t level)
{
    size_t n = level * 4;
    while (n--)
        putwchar(L' ');
}

/* Convert a MI_DateTime to a string */
void DatetimeToStr(const MI_Datetime* x, _Out_writes_z_(26) MI_Char buf[26])
{
    if (x->isTimestamp)
    {
        const MI_Char FMT[] =  L"%04d%02d%02d%02d%02d%02d.%06d%c%03d";
        MI_Sint32 utc = x->u.timestamp.utc;
        swprintf_s(buf, 26, FMT,
            x->u.timestamp.year,
            x->u.timestamp.month,
            x->u.timestamp.day,
            x->u.timestamp.hour,
            x->u.timestamp.minute,
            x->u.timestamp.second,
            x->u.timestamp.microseconds,
            utc < 0 ? '-' : '+',
            utc < 0 ? -utc : utc);
    }
    else
    {
        const MI_Char FMT[] = L"%08u%02u%02u%02u.%06u:000";
        swprintf_s(buf, 26, FMT,
            x->u.interval.days,
            x->u.interval.hours,
            x->u.interval.minutes,
            x->u.interval.seconds,
            x->u.interval.microseconds);
    }
}

/* Print an element value */
void Print_Element_Value(
    const MI_Value *elementValue, 
    MI_Type elementType, 
    size_t level)
{
        switch (elementType)
        {
            case MI_BOOLEAN:
            {
                wprintf(L"%s", elementValue->boolean ? L"true" : L"false");
                break;
            }
            case MI_SINT8:
            {
                wprintf(L"%hd", elementValue->sint8);
                break;
            }
            case MI_UINT8:
            {
                wprintf(L"%hu", elementValue->uint8);
                break;
            }
            case MI_SINT16:
            {
                wprintf(L"%d", elementValue->sint16);
                break;
            }
            case MI_UINT16:
            {
                wprintf(L"%u", elementValue->uint16);
                break;
            }
            case MI_SINT32:
            {
                wprintf(L"%I32d", elementValue->sint32);
                break;
            }
            case MI_UINT32:
            {
                wprintf(L"%I32u", elementValue->uint32);
                break;
            }
            case MI_SINT64:
            {
                wprintf(L"%I64i", elementValue->sint64);
                break;
            }
            case MI_UINT64:
            {
                wprintf(L"%I64u", elementValue->uint64);
                break;
            }
            case MI_REAL32:
            {
                wprintf(L"%g", elementValue->real32);
                break;
            }
            case MI_REAL64:
            {
                wprintf(L"%lg", elementValue->real64);
                break;
            }
            case MI_CHAR16:
            {
                wprintf(L"%u", elementValue->char16);
                break;
            }
            case MI_DATETIME:
            {
                MI_Char buf[26];
                DatetimeToStr(&elementValue->datetime, buf);
                wprintf(L"%s", buf);
                break;
            }
            case MI_STRING:
            {
                wprintf(L"%s", elementValue->string);
                break;
            }
            case MI_BOOLEANA:
            {
                MI_Uint32 i;
                MI_Type nonArrayType = (MI_Type) (elementType & (~MI_ARRAY));

                wprintf(L"{");

                for (i = 0; i < elementValue->booleana.size; i++)
                {
                    MI_Value value;
                    value.boolean = elementValue->booleana.data[i];
                    Print_Element_Value(&value, nonArrayType, level);
                    if (i + 1 != elementValue->booleana.size)
                        wprintf(L", ");
                }
                wprintf(L"}");
                break;
            }
            case MI_SINT8A:
            {
                MI_Uint32 i;
                MI_Type nonArrayType = (MI_Type) (elementType & (~MI_ARRAY));

                wprintf(L"{");

                for (i = 0; i < elementValue->sint8a.size; i++)
                {
                    MI_Value value;
                    value.sint8 = elementValue->sint8a.data[i];
                    Print_Element_Value(&value, nonArrayType, level);
                    if (i + 1 != elementValue->sint8a.size)
                        wprintf(L", ");
                }
                wprintf(L"}");
                break;
            }
            case MI_UINT8A:
            {
                MI_Uint32 i;
                MI_Type nonArrayType = (MI_Type) (elementType & (~MI_ARRAY));

                wprintf(L"{");

                for (i = 0; i < elementValue->uint8a.size; i++)
                {
                    MI_Value value;
                    value.uint8 = elementValue->uint8a.data[i];
                    Print_Element_Value(&value, nonArrayType, level);
                    if (i + 1 != elementValue->uint8a.size)
                        wprintf(L", ");
                }
                wprintf(L"}");
                break;
            }
            case MI_SINT16A:
            {
                MI_Uint32 i;
                MI_Type nonArrayType = (MI_Type) (elementType & (~MI_ARRAY));

                wprintf(L"{");

                for (i = 0; i < elementValue->sint16a.size; i++)
                {
                    MI_Value value;
                    value.sint16 = elementValue->sint16a.data[i];
                    Print_Element_Value(&value, nonArrayType, level);
                    if (i + 1 != elementValue->sint16a.size)
                        wprintf(L", ");
                }
                wprintf(L"}");
                break;
            }
            case MI_UINT16A:
            {
                MI_Uint32 i;
                MI_Type nonArrayType = (MI_Type) (elementType & (~MI_ARRAY));

                wprintf(L"{");

                for (i = 0; i < elementValue->uint16a.size; i++)
                {
                    MI_Value value;
                    value.uint16 = elementValue->uint16a.data[i];
                    Print_Element_Value(&value, nonArrayType, level);
                    if (i + 1 != elementValue->uint16a.size)
                        wprintf(L", ");
                }
                wprintf(L"}");
                break;
            }
            case MI_SINT32A:
            {
                MI_Uint32 i;
                MI_Type nonArrayType = (MI_Type) (elementType & (~MI_ARRAY));

                wprintf(L"{");

                for (i = 0; i < elementValue->sint32a.size; i++)
                {
                    MI_Value value;
                    value.sint32 = elementValue->sint32a.data[i];
                    Print_Element_Value(&value, nonArrayType, level);
                    if (i + 1 != elementValue->sint32a.size)
                        wprintf(L", ");
                }
                wprintf(L"}");
                break;
            }
            case MI_UINT32A:
            {
                MI_Uint32 i;
                MI_Type nonArrayType = (MI_Type) (elementType & (~MI_ARRAY));

                wprintf(L"{");

                for (i = 0; i < elementValue->uint32a.size; i++)
                {
                    MI_Value value;
                    value.uint32 = elementValue->uint32a.data[i];
                    Print_Element_Value(&value, nonArrayType, level);
                    if (i + 1 != elementValue->uint32a.size)
                        wprintf(L", ");
                }
                wprintf(L"}");
                break;
            }
            case MI_SINT64A:
            {
                MI_Uint32 i;
                MI_Type nonArrayType = (MI_Type) (elementType & (~MI_ARRAY));

                wprintf(L"{");

                for (i = 0; i < elementValue->sint64a.size; i++)
                {
                    MI_Value value;
                    value.sint64 = elementValue->sint64a.data[i];
                    Print_Element_Value(&value, nonArrayType, level);
                    if (i + 1 != elementValue->sint64a.size)
                        wprintf(L", ");
                }
                wprintf(L"}");
                break;
            }
            case MI_UINT64A:
            {
                MI_Uint32 i;
                MI_Type nonArrayType = (MI_Type) (elementType & (~MI_ARRAY));

                wprintf(L"{");

                for (i = 0; i < elementValue->uint64a.size; i++)
                {
                    MI_Value value;
                    value.uint64 = elementValue->uint64a.data[i];
                    Print_Element_Value(&value, nonArrayType, level);
                    if (i + 1 != elementValue->uint64a.size)
                        wprintf(L", ");
                }
                wprintf(L"}");
                break;
            }
            case MI_REAL32A:
            {
                MI_Uint32 i;
                MI_Type nonArrayType = (MI_Type) (elementType & (~MI_ARRAY));

                wprintf(L"{");

                for (i = 0; i < elementValue->real32a.size; i++)
                {
                    MI_Value value;
                    value.real32 = elementValue->real32a.data[i];
                    Print_Element_Value(&value, nonArrayType, level);
                    if (i + 1 != elementValue->real32a.size)
                        wprintf(L", ");
                }
                wprintf(L"}");
                break;
            }
            case MI_REAL64A:
            {
                MI_Uint32 i;
                MI_Type nonArrayType = (MI_Type) (elementType & (~MI_ARRAY));

                wprintf(L"{");

                for (i = 0; i < elementValue->real64a.size; i++)
                {
                    MI_Value value;
                    value.real64 = elementValue->real64a.data[i];
                    Print_Element_Value(&value, nonArrayType, level);
                    if (i + 1 != elementValue->real64a.size)
                        wprintf(L", ");
                }
                wprintf(L"}");
                break;
            }
            case MI_CHAR16A:
            {
                MI_Uint32 i;
                MI_Type nonArrayType = (MI_Type) (elementType & (~MI_ARRAY));

                wprintf(L"{");

                for (i = 0; i < elementValue->char16a.size; i++)
                {
                    MI_Value value;
                    value.char16 = elementValue->char16a.data[i];
                    Print_Element_Value(&value, nonArrayType, level);
                    if (i + 1 != elementValue->char16a.size)
                        wprintf(L", ");
                }
                wprintf(L"}");
                break;
            }
            case MI_DATETIMEA:
            {
                MI_Uint32 i;
                MI_Type nonArrayType = (MI_Type) (elementType & (~MI_ARRAY));

                wprintf(L"{");

                for (i = 0; i < elementValue->datetimea.size; i++)
                {
                    MI_Value value;
                    value.datetime = elementValue->datetimea.data[i];
                    Print_Element_Value(&value, nonArrayType, level);
                    if (i + 1 != elementValue->datetimea.size)
                        wprintf(L", ");
                }
                wprintf(L"}");
                break;
            }
            case MI_STRINGA:
            {
                MI_Uint32 i;

                wprintf(L"{");

                for (i = 0; i < elementValue->stringa.size; i++)
                {
                    MI_Char *value;
                    value = elementValue->stringa.data[i];
                    wprintf(value);
                    if (i + 1 != elementValue->stringa.size)
                        wprintf(L", ");
                }
                wprintf(L"}");
                break;
            }
            case MI_INSTANCE:
            {
                MI_Instance* inst = elementValue->instance;

                Dump_MI_Instance(inst, MI_FALSE, level);
                break;
            }
            case MI_REFERENCE:
            {
                MI_Instance* inst = elementValue->reference;

                wprintf(L" REF ");

                Dump_MI_Instance(inst, MI_TRUE, level);
                break;
            }
            case MI_INSTANCEA:
            {
                const MI_InstanceA* inst = &elementValue->instancea;
                MI_Uint32 i;

                wprintf(L"\n");

                Indent(level);
                wprintf(L"{\n");

                for (i = 0; i < inst->size; i++)
                {
                    Dump_MI_Instance(inst->data[i], MI_TRUE, level + 1);
                }

                Indent(level);
                wprintf(L"}");

                break;
            }
            case MI_REFERENCEA:
            {
                const MI_InstanceA* inst = &elementValue->instancea;
                MI_Uint32 i;

                wprintf(L" REF ");

                wprintf(L"\n");

                Indent(level);
                wprintf(L"{\n");

                for (i = 0; i < inst->size; i++)
                {
                    Dump_MI_Instance(inst->data[i], MI_FALSE, level + 1);
                }

                Indent(level);
                wprintf(L"}");

                break;
            }
            default:
                break;
        }
}

/* Print an element */
void Print_Element(
    const MI_Char *elementName, 
    const MI_Value *elementValue, 
    MI_Type elementType, 
    MI_Uint32 elementFlags, 
    size_t level)
{
    Indent(level);
    wprintf(L"%s [%s", elementName, MI_Type_To_String(elementType));

    if (elementFlags & MI_FLAG_KEY)
    {
        wprintf(L", MI_FLAG_KEY");
    }

    if ((elementFlags & MI_FLAG_NULL) || (elementValue == NULL))
    {
        wprintf(L", NULL]");
        return;
    }
    else 
    {
        wprintf(L"] ");

        Print_Element_Value(elementValue, elementType, level);
    }
}

/* Prints an entire instance.  Note this function is recursive as an instance may contain an embedded instance */
void Dump_MI_Instance(_In_opt_ const MI_Instance *miInstance, MI_Boolean keysOnly, size_t level)
{
    MI_Uint32 elementCount;
    MI_Result miResult;
    MI_Uint32 elementIndex;

    if (miInstance == NULL)
    {
        wprintf(L"<null>\n");
        return;
    }

    miResult = MI_Instance_GetElementCount(miInstance, &elementCount);
    
    if (miResult != MI_RESULT_OK)
    {
        wprintf(L"MI_Instance_GetElementCount failed, error = %s\n", MI_Result_To_String(miResult));
        return;
    }
    Indent(level);
    wprintf(L"Class %s\n", miInstance->classDecl->name);
    Indent(level);
	wprintf(L"{\n");

    for (elementIndex = 0; elementIndex != elementCount; elementIndex++)
    {
        MI_Char *elementName;
        MI_Value elementValue;
        MI_Type elementType;
        MI_Uint32 elementFlags;

        miResult = MI_Instance_GetElementAt(miInstance, elementIndex, &elementName, &elementValue, &elementType, &elementFlags);
        if (miResult != MI_RESULT_OK)
        {
            wprintf(L"MI_Instance_GetElementCount failed on element index %u, error = %s\n", elementIndex, MI_Result_To_String(miResult));
            return;
        }
        if ((keysOnly && (elementFlags & MI_FLAG_KEY)) || 
            !keysOnly)
        {
            Print_Element(elementName, &elementValue, elementType, elementFlags, level+1);
            wprintf(L"\n");
        }
    }
    Indent(level);
    wprintf(L"}\n");
}

/* Retrieve an element value from the user and set it in the given instance */
MI_Result SetInstanceProperty(MI_Instance *miInstance, const MI_Char *elementName, MI_Type elementType, MI_Uint32 elementFlags, _In_opt_ MI_Value *elementValue, _Out_writes_z_(tmpBufferLength) MI_Char *tmpBuffer, MI_Uint32 tmpBufferLength, MI_Boolean useAddElement)
{
    MI_Result miResult = MI_RESULT_OK;
    const wchar_t *keyString = L"";
    MI_Value miValue;
    MI_Uint32 miFlags = 0;

    MI_UNREFERENCED_PARAMETER(elementValue);

    if (elementFlags & MI_FLAG_KEY)
    {
        keyString = L", KEY";
    }

    wprintf(L"%s [%s%s]: ", elementName, MI_Type_To_String(elementType), keyString);

    switch (elementType)
    {
    case MI_STRING:
    {
        miValue.string = L"";
        if (elementValue && elementValue->string)
        {
            miValue.string = elementValue->string;
        }

        GetUserInputString(L"", tmpBuffer, tmpBufferLength, miValue.string);
        miValue.string = tmpBuffer;
        break;
    }
    case MI_BOOLEAN:
    {
        MI_Boolean defaultValue = MI_FALSE;
        if (elementValue)
        {
            defaultValue = elementValue->boolean;
        }
        if (StringCchPrintfW(tmpBuffer, tmpBufferLength, L"%s", defaultValue?L"true":L"false") != S_OK)
        {
            wprintf(L"StringCchPrintfW failed\n");
            miResult = MI_RESULT_FAILED;
            break;
        }

        GetUserInputString(L"", tmpBuffer, tmpBufferLength, tmpBuffer);
        if ((wcscmp(tmpBuffer, L"1") == 0) || (_wcsicmp(tmpBuffer, L"true") == 0))
        {
            miValue.boolean = MI_TRUE;
        }
        else if ((wcscmp(tmpBuffer, L"0") == 0) || (_wcsicmp(tmpBuffer, L"false") == 0))
        {
            miValue.boolean = MI_FALSE;
        }
        else
        {
            wprintf(L"Invalid value.  MI_Boolean values should be 'true' or 'false'\n");
            miResult = MI_RESULT_INVALID_PARAMETER;
            break;
        }
        break;
    }
    case MI_CHAR16:
    {
        MI_Char defaultValue = L'0';
        if (elementValue)
        {
            defaultValue = elementValue->char16;
        }
        if (StringCchPrintfW(tmpBuffer, tmpBufferLength, L"%c", defaultValue) != S_OK)
        {
            wprintf(L"StringCchPrintfW failed\n");
            miResult = MI_RESULT_FAILED;
            break;
        }
        GetUserInputString(L"", tmpBuffer, tmpBufferLength, tmpBuffer);

        miValue.char16 = tmpBuffer[0];
        break;
    }
    case MI_UINT8:
    case MI_SINT8:
    {
        MI_Sint8 defaultValue = 0;
        if (elementValue)
        {
            defaultValue = elementValue->sint8;
        }
        if (StringCchPrintfW(tmpBuffer, tmpBufferLength, L"%hd", (short) defaultValue) != S_OK)
        {
            wprintf(L"StringCchPrintfW failed\n");
            miResult = MI_RESULT_FAILED;
            break;
        }
        GetUserInputString(L"", tmpBuffer, tmpBufferLength, tmpBuffer);

        miValue.sint8 = (char) _wtoi(tmpBuffer);
        break;
    }
    case MI_UINT16:
    case MI_SINT16:
    {
        MI_Sint16 defaultValue = 0;
        if (elementValue)
        {
            defaultValue = elementValue->sint16;
        }
        if (StringCchPrintfW(tmpBuffer, tmpBufferLength, L"%hd", defaultValue) != S_OK)
        {
            wprintf(L"StringCchPrintfW failed\n");
            miResult = MI_RESULT_FAILED;
            break;
        }
        GetUserInputString(L"", tmpBuffer, tmpBufferLength, tmpBuffer);

        miValue.sint16 = (short) _wtoi(tmpBuffer);
        break;
    }
    case MI_UINT32:
    case MI_SINT32:
    {
        MI_Sint32 defaultValue = 0;
        if (elementValue)
        {
            defaultValue = elementValue->sint32;
        }
        if (StringCchPrintfW(tmpBuffer, tmpBufferLength, L"%i", defaultValue) != S_OK)
        {
            wprintf(L"StringCchPrintfW failed\n");
            miResult = MI_RESULT_FAILED;
            break;
        }
        GetUserInputString(L"", tmpBuffer, tmpBufferLength, tmpBuffer);

        miValue.sint32 = _wtol(tmpBuffer);
        break;
    }
    case MI_SINT64:
    case MI_UINT64:
    {
        MI_Sint64 defaultValue = 0;
        if (elementValue)
        {
            defaultValue = elementValue->sint64;
        }
        if (StringCchPrintfW(tmpBuffer, tmpBufferLength, L"%I64i", defaultValue) != S_OK)
        {
            wprintf(L"StringCchPrintfW failed\n");
            miResult = MI_RESULT_FAILED;
            break;
        }
        GetUserInputString(L"", tmpBuffer, tmpBufferLength, tmpBuffer);

        miValue.sint64 = _wtoi64(tmpBuffer);
        break;
    }
    case MI_REAL32:
    {
        MI_Real32 defaultValue = 0;
        if (elementValue)
        {
            defaultValue = elementValue->real32;
        }
        if (StringCchPrintfW(tmpBuffer, tmpBufferLength, L"%f", (double)defaultValue) != S_OK)
        {
            wprintf(L"StringCchPrintfW failed\n");
            miResult = MI_RESULT_FAILED;
            break;
        }
        GetUserInputString(L"", tmpBuffer, tmpBufferLength, tmpBuffer);

        miValue.real32 = (float) _wtof(tmpBuffer);
        break;
    }
    case MI_REAL64:
    {
        MI_Real64 defaultValue = 0;
        if (elementValue)
        {
            defaultValue = elementValue->real64;
        }
        if (StringCchPrintfW(tmpBuffer, tmpBufferLength, L"%f", defaultValue) != S_OK)
        {
            wprintf(L"StringCchPrintfW failed\n");
            miResult = MI_RESULT_FAILED;
            break;
        }
        GetUserInputString(L"", tmpBuffer, tmpBufferLength, tmpBuffer);

        miValue.real64 = _wtof(tmpBuffer);
        break;
    }
    default:
    {
        wprintf(L"Sample does not allow adding a value of this type, leaving it as NULL\n");
        miFlags = MI_FLAG_NULL;
        if (tmpBufferLength)
        {
            tmpBuffer[0] = L'\0';
        }
        break;
    }
    }

    if (miResult != MI_RESULT_OK)
    {
        wprintf(L"Failed to set instance key, error %s\n", MI_Result_To_String(miResult));
    }
    else
    {
        if (useAddElement)
        {
            miResult = MI_Instance_AddElement(miInstance, elementName, miFlags==MI_FLAG_NULL?NULL:&miValue, elementType, miFlags);
        }
        else if (miFlags == 0)
        {
            miResult = MI_Instance_SetElement(miInstance, elementName, &miValue, elementType, 0);
        }
    }

    return miResult;
}

/* Create an instance based on a given class name.  The instance will be strongly typed as it will be based on an actual class declaration,
 * rather than a dynamic instance that is built on the fly.
 * For simplicity, the class declaration is retrieved from the server synchronously.
 */
MI_Result CreateInboundInstance(MI_Session *miSession, _In_z_ const wchar_t *namespaceName, _In_z_ const wchar_t *className, MI_Boolean keysOnly, MI_Instance **inboundInstance)
{
    MI_Operation miOperation = MI_OPERATION_NULL;
    MI_Class *miClass = NULL;
    MI_Boolean moreResults;
    MI_Result miResult;
    MI_Char *errorMessage = NULL;
    MI_Instance *completionDetails = NULL;
    MI_Application miApplication = MI_APPLICATION_NULL;
    MI_Instance *miInstance = NULL;
    MI_Uint32 numberElements = 0;
    MI_Uint32 elementIndex;

    *inboundInstance = NULL;

    MI_Session_GetClass(miSession, 0, NULL, namespaceName, className, NULL, &miOperation);
    MI_Operation_GetClass(&miOperation, &miClass, &moreResults, &miResult, &errorMessage, &completionDetails);

    if (miResult != MI_RESULT_OK)
    {
        wprintf(L"Failed to retrieve class declaration for %s, error %s\n", className, MI_Result_To_String(miResult));
        wprintf(L"Error message: %s\n", errorMessage);
        Dump_MI_Instance(completionDetails, MI_FALSE, 0);
        goto failedGetClass;
    }

    miResult = MI_Session_GetApplication(miSession, &miApplication);
    if (miResult != MI_RESULT_OK)
    {
        wprintf(L"Failed to get the parent application of the session, error %s\n", MI_Result_To_String(miResult));
        goto failedGetClass;
    }

    miResult = MI_Application_NewInstanceFromClass(&miApplication, className, miClass, &miInstance);
    if (miResult != MI_RESULT_OK)
    {
        wprintf(L"Failed to create an instance, error %s\n", MI_Result_To_String(miResult));
        goto failedGetClass;
    }

    miResult = MI_Class_GetElementCount(miClass, &numberElements);
    if (miResult != MI_RESULT_OK)
    {
        wprintf(L"Failed to get element count from class, error %s\n", MI_Result_To_String(miResult));
        goto failedGetClass;
    }

    wprintf(L"Enter %sproperties for inbound instance:\n", keysOnly?L"key " : L"");

    for (elementIndex = 0; elementIndex != numberElements; elementIndex++)
    {
        const MI_Char *elementName;
        MI_Type elementType;
        MI_Uint32 elementFlags;
        MI_Char tmpBuffer[50];

        miResult = MI_Class_GetElementAt(miClass, elementIndex, &elementName, NULL, NULL, &elementType, NULL, NULL, &elementFlags);
        if (miResult != MI_RESULT_OK)
        {
            wprintf(L"Failed to get element details from class, error %s\n", MI_Result_To_String(miResult));
            goto failedGetClass;
        }

        if (keysOnly && ((elementFlags & MI_FLAG_KEY) == 0))
        {
            continue;
        }

        miResult = SetInstanceProperty(miInstance, elementName, elementType, keysOnly?0:elementFlags, NULL, tmpBuffer, sizeof(tmpBuffer)/sizeof(tmpBuffer[0]), MI_FALSE);
        if (miResult != MI_RESULT_OK)
        {
            break;
        }
    }

failedGetClass:


    if ((miResult != MI_RESULT_OK) && miInstance)
    {
        MI_Result _tmpResult = MI_Instance_Delete(miInstance);
        if (_tmpResult != MI_RESULT_OK)
        {
            wprintf(L"Failed to delete key instance, error %s\n", MI_Result_To_String(_tmpResult));
        }
    }

    {
        MI_Result _tmpResult = MI_Operation_Close(&miOperation);
        if (_tmpResult != MI_RESULT_OK)
        {
            wprintf(L"MI_Operation_Close failed, error %s\n", MI_Result_To_String(_tmpResult));
        }
    }
        
    if (miResult == MI_RESULT_OK)
    {
        *inboundInstance = miInstance;
    }
    return miResult;
}

