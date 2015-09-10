//
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved
//


#pragma once

/* Query the user for a single character selection of a specific set of characters */
wchar_t GetUserSelection(const wchar_t *displayString, const wchar_t *validOptions);

/* Input a string from a user, allowing the caller to pass in a default value if the input string was zero length */
void GetUserInputString(_In_z_ const wchar_t *displayString, _Out_writes_z_(outputBufferLength) wchar_t *outputString, size_t outputBufferLength, _In_z_ const wchar_t *defaultString);

/* Helper function to convert an MI_Result into a string */
const wchar_t *MI_Result_To_String(MI_Result miResult);

/* Helper function to convert an MI_Type into a string */
const MI_Char *MI_Type_To_String(MI_Type type);

/* Prints an instance element on the screen */
void Print_Element(const MI_Char *elementName, const MI_Value *elementValue, MI_Type elementType, MI_Uint32 elementFlags, size_t level);

/* Dumps all instance elements on the screen */
void Dump_MI_Instance(_In_opt_ const MI_Instance *miInstance, MI_Boolean keysOnly, size_t level);

/* Creates an strongly typed instance based on the class name, and query the user to input the values of the elements.  If keysOnly is MI_TRUE then only key properties are populated. */
MI_Result CreateInboundInstance(MI_Session *miSession, _In_z_ const wchar_t *namespaceName, _In_z_ const wchar_t *className, MI_Boolean keysOnly, MI_Instance **inboundInstance);

/* Given an instance this function queries the user to enter a value for the specified element.  If useAddElement=MI_TRUE then MI_Instance_AddElement is used,
 * otherwise MI_Instance_SetElement is used to set the element in the instance. AddElement is needed for dynamic instances, SetElement is needed for stronly
 * typed instances that were created from a class declaration.
 */
MI_Result SetInstanceProperty(MI_Instance *miInstance, const MI_Char *elementName, MI_Type elementType, MI_Uint32 elementFlags, _In_opt_ MI_Value *elementValue, _Out_writes_z_(tmpBufferLength) MI_Char *tmpBuffer, MI_Uint32 tmpBufferLength, MI_Boolean useAddElement);