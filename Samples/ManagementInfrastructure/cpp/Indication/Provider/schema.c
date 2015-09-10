//
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved
//

/*
**==============================================================================
**
** WARNING: THIS FILE WAS AUTOMATICALLY GENERATED. PLEASE DO NOT EDIT.
**
**==============================================================================
*/
#include <ctype.h>
#include <MI.h>
#include "MSFT_WindowsServiceStarted.h"
#include "MSFT_WindowsServiceStopped.h"

/*
**==============================================================================
**
** Schema Declaration
**
**==============================================================================
*/

MI_EXTERN_C MI_SchemaDecl schemaDecl;

/*
**==============================================================================
**
** Qualifier declarations
**
**==============================================================================
*/

static MI_CONST MI_Boolean Abstract_qual_decl_value = 0;

static MI_CONST MI_QualifierDecl Abstract_qual_decl =
{
    MI_T("Abstract"), /* name */
    MI_BOOLEAN, /* type */
    MI_FLAG_ASSOCIATION|MI_FLAG_CLASS|MI_FLAG_INDICATION, /* scope */
    MI_FLAG_ENABLEOVERRIDE|MI_FLAG_RESTRICTED, /* flavor */
    0, /* subscript */
    &Abstract_qual_decl_value, /* value */
};

static MI_CONST MI_Boolean Aggregate_qual_decl_value = 0;

static MI_CONST MI_QualifierDecl Aggregate_qual_decl =
{
    MI_T("Aggregate"), /* name */
    MI_BOOLEAN, /* type */
    MI_FLAG_REFERENCE, /* scope */
    MI_FLAG_DISABLEOVERRIDE|MI_FLAG_TOSUBCLASS, /* flavor */
    0, /* subscript */
    &Aggregate_qual_decl_value, /* value */
};

static MI_CONST MI_Boolean Aggregation_qual_decl_value = 0;

static MI_CONST MI_QualifierDecl Aggregation_qual_decl =
{
    MI_T("Aggregation"), /* name */
    MI_BOOLEAN, /* type */
    MI_FLAG_ASSOCIATION, /* scope */
    MI_FLAG_DISABLEOVERRIDE|MI_FLAG_TOSUBCLASS, /* flavor */
    0, /* subscript */
    &Aggregation_qual_decl_value, /* value */
};

static MI_CONST MI_QualifierDecl Alias_qual_decl =
{
    MI_T("Alias"), /* name */
    MI_STRING, /* type */
    MI_FLAG_METHOD|MI_FLAG_PROPERTY|MI_FLAG_REFERENCE, /* scope */
    MI_FLAG_ENABLEOVERRIDE|MI_FLAG_TOSUBCLASS|MI_FLAG_TRANSLATABLE, /* flavor */
    0, /* subscript */
    NULL, /* value */
};

static MI_CONST MI_Char* ArrayType_qual_decl_value = MI_T("Bag");

static MI_CONST MI_QualifierDecl ArrayType_qual_decl =
{
    MI_T("ArrayType"), /* name */
    MI_STRING, /* type */
    MI_FLAG_PARAMETER|MI_FLAG_PROPERTY, /* scope */
    MI_FLAG_DISABLEOVERRIDE|MI_FLAG_TOSUBCLASS, /* flavor */
    0, /* subscript */
    &ArrayType_qual_decl_value, /* value */
};

static MI_CONST MI_Boolean Association_qual_decl_value = 0;

static MI_CONST MI_QualifierDecl Association_qual_decl =
{
    MI_T("Association"), /* name */
    MI_BOOLEAN, /* type */
    MI_FLAG_ASSOCIATION, /* scope */
    MI_FLAG_DISABLEOVERRIDE|MI_FLAG_TOSUBCLASS, /* flavor */
    0, /* subscript */
    &Association_qual_decl_value, /* value */
};

static MI_CONST MI_QualifierDecl BitMap_qual_decl =
{
    MI_T("BitMap"), /* name */
    MI_STRINGA, /* type */
    MI_FLAG_METHOD|MI_FLAG_PARAMETER|MI_FLAG_PROPERTY, /* scope */
    MI_FLAG_ENABLEOVERRIDE|MI_FLAG_TOSUBCLASS, /* flavor */
    0, /* subscript */
    NULL, /* value */
};

static MI_CONST MI_QualifierDecl BitValues_qual_decl =
{
    MI_T("BitValues"), /* name */
    MI_STRINGA, /* type */
    MI_FLAG_METHOD|MI_FLAG_PARAMETER|MI_FLAG_PROPERTY, /* scope */
    MI_FLAG_ENABLEOVERRIDE|MI_FLAG_TOSUBCLASS|MI_FLAG_TRANSLATABLE, /* flavor */
    0, /* subscript */
    NULL, /* value */
};

static MI_CONST MI_QualifierDecl ClassConstraint_qual_decl =
{
    MI_T("ClassConstraint"), /* name */
    MI_STRINGA, /* type */
    MI_FLAG_ASSOCIATION|MI_FLAG_CLASS|MI_FLAG_INDICATION, /* scope */
    MI_FLAG_ENABLEOVERRIDE|MI_FLAG_TOSUBCLASS, /* flavor */
    0, /* subscript */
    NULL, /* value */
};

static MI_CONST MI_QualifierDecl ClassVersion_qual_decl =
{
    MI_T("ClassVersion"), /* name */
    MI_STRING, /* type */
    MI_FLAG_ASSOCIATION|MI_FLAG_CLASS|MI_FLAG_INDICATION, /* scope */
    MI_FLAG_ENABLEOVERRIDE|MI_FLAG_RESTRICTED, /* flavor */
    0, /* subscript */
    NULL, /* value */
};

static MI_CONST MI_Boolean Composition_qual_decl_value = 0;

static MI_CONST MI_QualifierDecl Composition_qual_decl =
{
    MI_T("Composition"), /* name */
    MI_BOOLEAN, /* type */
    MI_FLAG_ASSOCIATION, /* scope */
    MI_FLAG_DISABLEOVERRIDE|MI_FLAG_TOSUBCLASS, /* flavor */
    0, /* subscript */
    &Composition_qual_decl_value, /* value */
};

static MI_CONST MI_QualifierDecl Correlatable_qual_decl =
{
    MI_T("Correlatable"), /* name */
    MI_STRINGA, /* type */
    MI_FLAG_PROPERTY, /* scope */
    MI_FLAG_ENABLEOVERRIDE|MI_FLAG_TOSUBCLASS, /* flavor */
    0, /* subscript */
    NULL, /* value */
};

static MI_CONST MI_Boolean Counter_qual_decl_value = 0;

static MI_CONST MI_QualifierDecl Counter_qual_decl =
{
    MI_T("Counter"), /* name */
    MI_BOOLEAN, /* type */
    MI_FLAG_METHOD|MI_FLAG_PARAMETER|MI_FLAG_PROPERTY, /* scope */
    MI_FLAG_ENABLEOVERRIDE|MI_FLAG_TOSUBCLASS, /* flavor */
    0, /* subscript */
    &Counter_qual_decl_value, /* value */
};

static MI_CONST MI_Boolean Delete_qual_decl_value = 0;

static MI_CONST MI_QualifierDecl Delete_qual_decl =
{
    MI_T("Delete"), /* name */
    MI_BOOLEAN, /* type */
    MI_FLAG_ASSOCIATION|MI_FLAG_REFERENCE, /* scope */
    MI_FLAG_ENABLEOVERRIDE|MI_FLAG_TOSUBCLASS, /* flavor */
    0, /* subscript */
    &Delete_qual_decl_value, /* value */
};

static MI_CONST MI_QualifierDecl Deprecated_qual_decl =
{
    MI_T("Deprecated"), /* name */
    MI_STRINGA, /* type */
    MI_FLAG_ANY, /* scope */
    MI_FLAG_ENABLEOVERRIDE|MI_FLAG_RESTRICTED, /* flavor */
    0, /* subscript */
    NULL, /* value */
};

static MI_CONST MI_QualifierDecl Description_qual_decl =
{
    MI_T("Description"), /* name */
    MI_STRING, /* type */
    MI_FLAG_ANY, /* scope */
    MI_FLAG_ENABLEOVERRIDE|MI_FLAG_TOSUBCLASS|MI_FLAG_TRANSLATABLE, /* flavor */
    0, /* subscript */
    NULL, /* value */
};

static MI_CONST MI_QualifierDecl DisplayDescription_qual_decl =
{
    MI_T("DisplayDescription"), /* name */
    MI_STRING, /* type */
    MI_FLAG_ANY, /* scope */
    MI_FLAG_ENABLEOVERRIDE|MI_FLAG_TOSUBCLASS|MI_FLAG_TRANSLATABLE, /* flavor */
    0, /* subscript */
    NULL, /* value */
};

static MI_CONST MI_QualifierDecl DisplayName_qual_decl =
{
    MI_T("DisplayName"), /* name */
    MI_STRING, /* type */
    MI_FLAG_ANY, /* scope */
    MI_FLAG_ENABLEOVERRIDE|MI_FLAG_TOSUBCLASS|MI_FLAG_TRANSLATABLE, /* flavor */
    0, /* subscript */
    NULL, /* value */
};

static MI_CONST MI_Boolean DN_qual_decl_value = 0;

static MI_CONST MI_QualifierDecl DN_qual_decl =
{
    MI_T("DN"), /* name */
    MI_BOOLEAN, /* type */
    MI_FLAG_METHOD|MI_FLAG_PARAMETER|MI_FLAG_PROPERTY, /* scope */
    MI_FLAG_DISABLEOVERRIDE|MI_FLAG_TOSUBCLASS, /* flavor */
    0, /* subscript */
    &DN_qual_decl_value, /* value */
};

static MI_CONST MI_QualifierDecl EmbeddedInstance_qual_decl =
{
    MI_T("EmbeddedInstance"), /* name */
    MI_STRING, /* type */
    MI_FLAG_METHOD|MI_FLAG_PARAMETER|MI_FLAG_PROPERTY, /* scope */
    MI_FLAG_ENABLEOVERRIDE|MI_FLAG_TOSUBCLASS, /* flavor */
    0, /* subscript */
    NULL, /* value */
};

static MI_CONST MI_Boolean EmbeddedObject_qual_decl_value = 0;

static MI_CONST MI_QualifierDecl EmbeddedObject_qual_decl =
{
    MI_T("EmbeddedObject"), /* name */
    MI_BOOLEAN, /* type */
    MI_FLAG_METHOD|MI_FLAG_PARAMETER|MI_FLAG_PROPERTY, /* scope */
    MI_FLAG_DISABLEOVERRIDE|MI_FLAG_TOSUBCLASS, /* flavor */
    0, /* subscript */
    &EmbeddedObject_qual_decl_value, /* value */
};

static MI_CONST MI_Boolean Exception_qual_decl_value = 0;

static MI_CONST MI_QualifierDecl Exception_qual_decl =
{
    MI_T("Exception"), /* name */
    MI_BOOLEAN, /* type */
    MI_FLAG_CLASS|MI_FLAG_INDICATION, /* scope */
    MI_FLAG_DISABLEOVERRIDE|MI_FLAG_TOSUBCLASS, /* flavor */
    0, /* subscript */
    &Exception_qual_decl_value, /* value */
};

static MI_CONST MI_Boolean Expensive_qual_decl_value = 0;

static MI_CONST MI_QualifierDecl Expensive_qual_decl =
{
    MI_T("Expensive"), /* name */
    MI_BOOLEAN, /* type */
    MI_FLAG_ANY, /* scope */
    MI_FLAG_ENABLEOVERRIDE|MI_FLAG_TOSUBCLASS, /* flavor */
    0, /* subscript */
    &Expensive_qual_decl_value, /* value */
};

static MI_CONST MI_Boolean Experimental_qual_decl_value = 0;

static MI_CONST MI_QualifierDecl Experimental_qual_decl =
{
    MI_T("Experimental"), /* name */
    MI_BOOLEAN, /* type */
    MI_FLAG_ANY, /* scope */
    MI_FLAG_ENABLEOVERRIDE|MI_FLAG_RESTRICTED, /* flavor */
    0, /* subscript */
    &Experimental_qual_decl_value, /* value */
};

static MI_CONST MI_Boolean Gauge_qual_decl_value = 0;

static MI_CONST MI_QualifierDecl Gauge_qual_decl =
{
    MI_T("Gauge"), /* name */
    MI_BOOLEAN, /* type */
    MI_FLAG_METHOD|MI_FLAG_PARAMETER|MI_FLAG_PROPERTY, /* scope */
    MI_FLAG_ENABLEOVERRIDE|MI_FLAG_TOSUBCLASS, /* flavor */
    0, /* subscript */
    &Gauge_qual_decl_value, /* value */
};

static MI_CONST MI_Boolean Ifdeleted_qual_decl_value = 0;

static MI_CONST MI_QualifierDecl Ifdeleted_qual_decl =
{
    MI_T("Ifdeleted"), /* name */
    MI_BOOLEAN, /* type */
    MI_FLAG_ASSOCIATION|MI_FLAG_REFERENCE, /* scope */
    MI_FLAG_ENABLEOVERRIDE|MI_FLAG_TOSUBCLASS, /* flavor */
    0, /* subscript */
    &Ifdeleted_qual_decl_value, /* value */
};

static MI_CONST MI_Boolean In_qual_decl_value = 1;

static MI_CONST MI_QualifierDecl In_qual_decl =
{
    MI_T("In"), /* name */
    MI_BOOLEAN, /* type */
    MI_FLAG_PARAMETER, /* scope */
    MI_FLAG_DISABLEOVERRIDE|MI_FLAG_TOSUBCLASS, /* flavor */
    0, /* subscript */
    &In_qual_decl_value, /* value */
};

static MI_CONST MI_Boolean Indication_qual_decl_value = 0;

static MI_CONST MI_QualifierDecl Indication_qual_decl =
{
    MI_T("Indication"), /* name */
    MI_BOOLEAN, /* type */
    MI_FLAG_CLASS|MI_FLAG_INDICATION, /* scope */
    MI_FLAG_DISABLEOVERRIDE|MI_FLAG_TOSUBCLASS, /* flavor */
    0, /* subscript */
    &Indication_qual_decl_value, /* value */
};

static MI_CONST MI_Boolean Invisible_qual_decl_value = 0;

static MI_CONST MI_QualifierDecl Invisible_qual_decl =
{
    MI_T("Invisible"), /* name */
    MI_BOOLEAN, /* type */
    MI_FLAG_ASSOCIATION|MI_FLAG_CLASS|MI_FLAG_METHOD|MI_FLAG_PROPERTY|MI_FLAG_REFERENCE, /* scope */
    MI_FLAG_ENABLEOVERRIDE|MI_FLAG_TOSUBCLASS, /* flavor */
    0, /* subscript */
    &Invisible_qual_decl_value, /* value */
};

static MI_CONST MI_Boolean IsPUnit_qual_decl_value = 0;

static MI_CONST MI_QualifierDecl IsPUnit_qual_decl =
{
    MI_T("IsPUnit"), /* name */
    MI_BOOLEAN, /* type */
    MI_FLAG_METHOD|MI_FLAG_PARAMETER|MI_FLAG_PROPERTY, /* scope */
    MI_FLAG_ENABLEOVERRIDE|MI_FLAG_TOSUBCLASS, /* flavor */
    0, /* subscript */
    &IsPUnit_qual_decl_value, /* value */
};

static MI_CONST MI_Boolean Key_qual_decl_value = 0;

static MI_CONST MI_QualifierDecl Key_qual_decl =
{
    MI_T("Key"), /* name */
    MI_BOOLEAN, /* type */
    MI_FLAG_PROPERTY|MI_FLAG_REFERENCE, /* scope */
    MI_FLAG_DISABLEOVERRIDE|MI_FLAG_TOSUBCLASS, /* flavor */
    0, /* subscript */
    &Key_qual_decl_value, /* value */
};

static MI_CONST MI_Boolean Large_qual_decl_value = 0;

static MI_CONST MI_QualifierDecl Large_qual_decl =
{
    MI_T("Large"), /* name */
    MI_BOOLEAN, /* type */
    MI_FLAG_CLASS|MI_FLAG_PROPERTY, /* scope */
    MI_FLAG_ENABLEOVERRIDE|MI_FLAG_TOSUBCLASS, /* flavor */
    0, /* subscript */
    &Large_qual_decl_value, /* value */
};

static MI_CONST MI_QualifierDecl MappingStrings_qual_decl =
{
    MI_T("MappingStrings"), /* name */
    MI_STRINGA, /* type */
    MI_FLAG_ANY, /* scope */
    MI_FLAG_ENABLEOVERRIDE|MI_FLAG_TOSUBCLASS, /* flavor */
    0, /* subscript */
    NULL, /* value */
};

static MI_CONST MI_QualifierDecl Max_qual_decl =
{
    MI_T("Max"), /* name */
    MI_UINT32, /* type */
    MI_FLAG_REFERENCE, /* scope */
    MI_FLAG_ENABLEOVERRIDE|MI_FLAG_TOSUBCLASS, /* flavor */
    0, /* subscript */
    NULL, /* value */
};

static MI_CONST MI_QualifierDecl MaxLen_qual_decl =
{
    MI_T("MaxLen"), /* name */
    MI_UINT32, /* type */
    MI_FLAG_METHOD|MI_FLAG_PARAMETER|MI_FLAG_PROPERTY, /* scope */
    MI_FLAG_ENABLEOVERRIDE|MI_FLAG_TOSUBCLASS, /* flavor */
    0, /* subscript */
    NULL, /* value */
};

static MI_CONST MI_QualifierDecl MaxValue_qual_decl =
{
    MI_T("MaxValue"), /* name */
    MI_SINT64, /* type */
    MI_FLAG_METHOD|MI_FLAG_PARAMETER|MI_FLAG_PROPERTY, /* scope */
    MI_FLAG_ENABLEOVERRIDE|MI_FLAG_TOSUBCLASS, /* flavor */
    0, /* subscript */
    NULL, /* value */
};

static MI_CONST MI_QualifierDecl MethodConstraint_qual_decl =
{
    MI_T("MethodConstraint"), /* name */
    MI_STRINGA, /* type */
    MI_FLAG_METHOD, /* scope */
    MI_FLAG_ENABLEOVERRIDE|MI_FLAG_TOSUBCLASS, /* flavor */
    0, /* subscript */
    NULL, /* value */
};

static MI_CONST MI_Uint32 Min_qual_decl_value = 0U;

static MI_CONST MI_QualifierDecl Min_qual_decl =
{
    MI_T("Min"), /* name */
    MI_UINT32, /* type */
    MI_FLAG_REFERENCE, /* scope */
    MI_FLAG_ENABLEOVERRIDE|MI_FLAG_TOSUBCLASS, /* flavor */
    0, /* subscript */
    &Min_qual_decl_value, /* value */
};

static MI_CONST MI_Uint32 MinLen_qual_decl_value = 0U;

static MI_CONST MI_QualifierDecl MinLen_qual_decl =
{
    MI_T("MinLen"), /* name */
    MI_UINT32, /* type */
    MI_FLAG_METHOD|MI_FLAG_PARAMETER|MI_FLAG_PROPERTY, /* scope */
    MI_FLAG_ENABLEOVERRIDE|MI_FLAG_TOSUBCLASS, /* flavor */
    0, /* subscript */
    &MinLen_qual_decl_value, /* value */
};

static MI_CONST MI_QualifierDecl MinValue_qual_decl =
{
    MI_T("MinValue"), /* name */
    MI_SINT64, /* type */
    MI_FLAG_METHOD|MI_FLAG_PARAMETER|MI_FLAG_PROPERTY, /* scope */
    MI_FLAG_ENABLEOVERRIDE|MI_FLAG_TOSUBCLASS, /* flavor */
    0, /* subscript */
    NULL, /* value */
};

static MI_CONST MI_QualifierDecl ModelCorrespondence_qual_decl =
{
    MI_T("ModelCorrespondence"), /* name */
    MI_STRINGA, /* type */
    MI_FLAG_ANY, /* scope */
    MI_FLAG_ENABLEOVERRIDE|MI_FLAG_TOSUBCLASS, /* flavor */
    0, /* subscript */
    NULL, /* value */
};

static MI_CONST MI_QualifierDecl Nonlocal_qual_decl =
{
    MI_T("Nonlocal"), /* name */
    MI_STRING, /* type */
    MI_FLAG_REFERENCE, /* scope */
    MI_FLAG_ENABLEOVERRIDE|MI_FLAG_TOSUBCLASS, /* flavor */
    0, /* subscript */
    NULL, /* value */
};

static MI_CONST MI_QualifierDecl NonlocalType_qual_decl =
{
    MI_T("NonlocalType"), /* name */
    MI_STRING, /* type */
    MI_FLAG_REFERENCE, /* scope */
    MI_FLAG_ENABLEOVERRIDE|MI_FLAG_TOSUBCLASS, /* flavor */
    0, /* subscript */
    NULL, /* value */
};

static MI_CONST MI_QualifierDecl NullValue_qual_decl =
{
    MI_T("NullValue"), /* name */
    MI_STRING, /* type */
    MI_FLAG_PROPERTY, /* scope */
    MI_FLAG_DISABLEOVERRIDE|MI_FLAG_TOSUBCLASS, /* flavor */
    0, /* subscript */
    NULL, /* value */
};

static MI_CONST MI_Boolean Octetstring_qual_decl_value = 0;

static MI_CONST MI_QualifierDecl Octetstring_qual_decl =
{
    MI_T("Octetstring"), /* name */
    MI_BOOLEAN, /* type */
    MI_FLAG_METHOD|MI_FLAG_PARAMETER|MI_FLAG_PROPERTY, /* scope */
    MI_FLAG_DISABLEOVERRIDE|MI_FLAG_TOSUBCLASS, /* flavor */
    0, /* subscript */
    &Octetstring_qual_decl_value, /* value */
};

static MI_CONST MI_Boolean Out_qual_decl_value = 0;

static MI_CONST MI_QualifierDecl Out_qual_decl =
{
    MI_T("Out"), /* name */
    MI_BOOLEAN, /* type */
    MI_FLAG_PARAMETER, /* scope */
    MI_FLAG_DISABLEOVERRIDE|MI_FLAG_TOSUBCLASS, /* flavor */
    0, /* subscript */
    &Out_qual_decl_value, /* value */
};

static MI_CONST MI_QualifierDecl Override_qual_decl =
{
    MI_T("Override"), /* name */
    MI_STRING, /* type */
    MI_FLAG_METHOD|MI_FLAG_PROPERTY|MI_FLAG_REFERENCE, /* scope */
    MI_FLAG_ENABLEOVERRIDE|MI_FLAG_RESTRICTED, /* flavor */
    0, /* subscript */
    NULL, /* value */
};

static MI_CONST MI_QualifierDecl Propagated_qual_decl =
{
    MI_T("Propagated"), /* name */
    MI_STRING, /* type */
    MI_FLAG_PROPERTY, /* scope */
    MI_FLAG_DISABLEOVERRIDE|MI_FLAG_TOSUBCLASS, /* flavor */
    0, /* subscript */
    NULL, /* value */
};

static MI_CONST MI_QualifierDecl PropertyConstraint_qual_decl =
{
    MI_T("PropertyConstraint"), /* name */
    MI_STRINGA, /* type */
    MI_FLAG_PROPERTY|MI_FLAG_REFERENCE, /* scope */
    MI_FLAG_ENABLEOVERRIDE|MI_FLAG_TOSUBCLASS, /* flavor */
    0, /* subscript */
    NULL, /* value */
};

static MI_CONST MI_Char* PropertyUsage_qual_decl_value = MI_T("CurrentContext");

static MI_CONST MI_QualifierDecl PropertyUsage_qual_decl =
{
    MI_T("PropertyUsage"), /* name */
    MI_STRING, /* type */
    MI_FLAG_PROPERTY, /* scope */
    MI_FLAG_ENABLEOVERRIDE|MI_FLAG_TOSUBCLASS, /* flavor */
    0, /* subscript */
    &PropertyUsage_qual_decl_value, /* value */
};

static MI_CONST MI_QualifierDecl Provider_qual_decl =
{
    MI_T("Provider"), /* name */
    MI_STRING, /* type */
    MI_FLAG_ANY, /* scope */
    MI_FLAG_ENABLEOVERRIDE|MI_FLAG_TOSUBCLASS, /* flavor */
    0, /* subscript */
    NULL, /* value */
};

static MI_CONST MI_QualifierDecl PUnit_qual_decl =
{
    MI_T("PUnit"), /* name */
    MI_STRING, /* type */
    MI_FLAG_METHOD|MI_FLAG_PARAMETER|MI_FLAG_PROPERTY, /* scope */
    MI_FLAG_ENABLEOVERRIDE|MI_FLAG_TOSUBCLASS, /* flavor */
    0, /* subscript */
    NULL, /* value */
};

static MI_CONST MI_Boolean Read_qual_decl_value = 1;

static MI_CONST MI_QualifierDecl Read_qual_decl =
{
    MI_T("Read"), /* name */
    MI_BOOLEAN, /* type */
    MI_FLAG_PROPERTY, /* scope */
    MI_FLAG_ENABLEOVERRIDE|MI_FLAG_TOSUBCLASS, /* flavor */
    0, /* subscript */
    &Read_qual_decl_value, /* value */
};

static MI_CONST MI_Boolean Required_qual_decl_value = 0;

static MI_CONST MI_QualifierDecl Required_qual_decl =
{
    MI_T("Required"), /* name */
    MI_BOOLEAN, /* type */
    MI_FLAG_METHOD|MI_FLAG_PARAMETER|MI_FLAG_PROPERTY|MI_FLAG_REFERENCE, /* scope */
    MI_FLAG_DISABLEOVERRIDE|MI_FLAG_TOSUBCLASS, /* flavor */
    0, /* subscript */
    &Required_qual_decl_value, /* value */
};

static MI_CONST MI_QualifierDecl Revision_qual_decl =
{
    MI_T("Revision"), /* name */
    MI_STRING, /* type */
    MI_FLAG_ASSOCIATION|MI_FLAG_CLASS|MI_FLAG_INDICATION, /* scope */
    MI_FLAG_ENABLEOVERRIDE|MI_FLAG_TOSUBCLASS|MI_FLAG_TRANSLATABLE, /* flavor */
    0, /* subscript */
    NULL, /* value */
};

static MI_CONST MI_QualifierDecl Schema_qual_decl =
{
    MI_T("Schema"), /* name */
    MI_STRING, /* type */
    MI_FLAG_METHOD|MI_FLAG_PROPERTY, /* scope */
    MI_FLAG_DISABLEOVERRIDE|MI_FLAG_TOSUBCLASS|MI_FLAG_TRANSLATABLE, /* flavor */
    0, /* subscript */
    NULL, /* value */
};

static MI_CONST MI_QualifierDecl Source_qual_decl =
{
    MI_T("Source"), /* name */
    MI_STRING, /* type */
    MI_FLAG_ASSOCIATION|MI_FLAG_CLASS|MI_FLAG_INDICATION, /* scope */
    MI_FLAG_ENABLEOVERRIDE|MI_FLAG_TOSUBCLASS, /* flavor */
    0, /* subscript */
    NULL, /* value */
};

static MI_CONST MI_QualifierDecl SourceType_qual_decl =
{
    MI_T("SourceType"), /* name */
    MI_STRING, /* type */
    MI_FLAG_ASSOCIATION|MI_FLAG_CLASS|MI_FLAG_INDICATION|MI_FLAG_REFERENCE, /* scope */
    MI_FLAG_ENABLEOVERRIDE|MI_FLAG_TOSUBCLASS, /* flavor */
    0, /* subscript */
    NULL, /* value */
};

static MI_CONST MI_Boolean Static_qual_decl_value = 0;

static MI_CONST MI_QualifierDecl Static_qual_decl =
{
    MI_T("Static"), /* name */
    MI_BOOLEAN, /* type */
    MI_FLAG_METHOD|MI_FLAG_PROPERTY, /* scope */
    MI_FLAG_DISABLEOVERRIDE|MI_FLAG_TOSUBCLASS, /* flavor */
    0, /* subscript */
    &Static_qual_decl_value, /* value */
};

static MI_CONST MI_Boolean Stream_qual_decl_value = 0;

static MI_CONST MI_QualifierDecl Stream_qual_decl =
{
    MI_T("Stream"), /* name */
    MI_BOOLEAN, /* type */
    MI_FLAG_METHOD|MI_FLAG_PARAMETER, /* scope */
    MI_FLAG_DISABLEOVERRIDE|MI_FLAG_TOSUBCLASS, /* flavor */
    0, /* subscript */
    &Stream_qual_decl_value, /* value */
};

static MI_CONST MI_QualifierDecl Syntax_qual_decl =
{
    MI_T("Syntax"), /* name */
    MI_STRING, /* type */
    MI_FLAG_METHOD|MI_FLAG_PARAMETER|MI_FLAG_PROPERTY|MI_FLAG_REFERENCE, /* scope */
    MI_FLAG_ENABLEOVERRIDE|MI_FLAG_TOSUBCLASS, /* flavor */
    0, /* subscript */
    NULL, /* value */
};

static MI_CONST MI_QualifierDecl SyntaxType_qual_decl =
{
    MI_T("SyntaxType"), /* name */
    MI_STRING, /* type */
    MI_FLAG_METHOD|MI_FLAG_PARAMETER|MI_FLAG_PROPERTY|MI_FLAG_REFERENCE, /* scope */
    MI_FLAG_ENABLEOVERRIDE|MI_FLAG_TOSUBCLASS, /* flavor */
    0, /* subscript */
    NULL, /* value */
};

static MI_CONST MI_Boolean Terminal_qual_decl_value = 0;

static MI_CONST MI_QualifierDecl Terminal_qual_decl =
{
    MI_T("Terminal"), /* name */
    MI_BOOLEAN, /* type */
    MI_FLAG_ASSOCIATION|MI_FLAG_CLASS|MI_FLAG_INDICATION, /* scope */
    MI_FLAG_ENABLEOVERRIDE|MI_FLAG_TOSUBCLASS, /* flavor */
    0, /* subscript */
    &Terminal_qual_decl_value, /* value */
};

static MI_CONST MI_QualifierDecl TriggerType_qual_decl =
{
    MI_T("TriggerType"), /* name */
    MI_STRING, /* type */
    MI_FLAG_ASSOCIATION|MI_FLAG_CLASS|MI_FLAG_INDICATION|MI_FLAG_METHOD|MI_FLAG_PROPERTY|MI_FLAG_REFERENCE, /* scope */
    MI_FLAG_ENABLEOVERRIDE|MI_FLAG_TOSUBCLASS, /* flavor */
    0, /* subscript */
    NULL, /* value */
};

static MI_CONST MI_QualifierDecl UMLPackagePath_qual_decl =
{
    MI_T("UMLPackagePath"), /* name */
    MI_STRING, /* type */
    MI_FLAG_ASSOCIATION|MI_FLAG_CLASS|MI_FLAG_INDICATION, /* scope */
    MI_FLAG_ENABLEOVERRIDE|MI_FLAG_TOSUBCLASS, /* flavor */
    0, /* subscript */
    NULL, /* value */
};

static MI_CONST MI_QualifierDecl Units_qual_decl =
{
    MI_T("Units"), /* name */
    MI_STRING, /* type */
    MI_FLAG_METHOD|MI_FLAG_PARAMETER|MI_FLAG_PROPERTY, /* scope */
    MI_FLAG_ENABLEOVERRIDE|MI_FLAG_TOSUBCLASS|MI_FLAG_TRANSLATABLE, /* flavor */
    0, /* subscript */
    NULL, /* value */
};

static MI_CONST MI_QualifierDecl UnknownValues_qual_decl =
{
    MI_T("UnknownValues"), /* name */
    MI_STRINGA, /* type */
    MI_FLAG_PROPERTY, /* scope */
    MI_FLAG_DISABLEOVERRIDE|MI_FLAG_TOSUBCLASS, /* flavor */
    0, /* subscript */
    NULL, /* value */
};

static MI_CONST MI_QualifierDecl UnsupportedValues_qual_decl =
{
    MI_T("UnsupportedValues"), /* name */
    MI_STRINGA, /* type */
    MI_FLAG_PROPERTY, /* scope */
    MI_FLAG_DISABLEOVERRIDE|MI_FLAG_TOSUBCLASS, /* flavor */
    0, /* subscript */
    NULL, /* value */
};

static MI_CONST MI_QualifierDecl ValueMap_qual_decl =
{
    MI_T("ValueMap"), /* name */
    MI_STRINGA, /* type */
    MI_FLAG_METHOD|MI_FLAG_PARAMETER|MI_FLAG_PROPERTY, /* scope */
    MI_FLAG_ENABLEOVERRIDE|MI_FLAG_TOSUBCLASS, /* flavor */
    0, /* subscript */
    NULL, /* value */
};

static MI_CONST MI_QualifierDecl Values_qual_decl =
{
    MI_T("Values"), /* name */
    MI_STRINGA, /* type */
    MI_FLAG_METHOD|MI_FLAG_PARAMETER|MI_FLAG_PROPERTY, /* scope */
    MI_FLAG_ENABLEOVERRIDE|MI_FLAG_TOSUBCLASS|MI_FLAG_TRANSLATABLE, /* flavor */
    0, /* subscript */
    NULL, /* value */
};

static MI_CONST MI_QualifierDecl Version_qual_decl =
{
    MI_T("Version"), /* name */
    MI_STRING, /* type */
    MI_FLAG_ASSOCIATION|MI_FLAG_CLASS|MI_FLAG_INDICATION, /* scope */
    MI_FLAG_ENABLEOVERRIDE|MI_FLAG_TRANSLATABLE|MI_FLAG_RESTRICTED, /* flavor */
    0, /* subscript */
    NULL, /* value */
};

static MI_CONST MI_Boolean Weak_qual_decl_value = 0;

static MI_CONST MI_QualifierDecl Weak_qual_decl =
{
    MI_T("Weak"), /* name */
    MI_BOOLEAN, /* type */
    MI_FLAG_REFERENCE, /* scope */
    MI_FLAG_DISABLEOVERRIDE|MI_FLAG_TOSUBCLASS, /* flavor */
    0, /* subscript */
    &Weak_qual_decl_value, /* value */
};

static MI_CONST MI_Boolean Write_qual_decl_value = 0;

static MI_CONST MI_QualifierDecl Write_qual_decl =
{
    MI_T("Write"), /* name */
    MI_BOOLEAN, /* type */
    MI_FLAG_PROPERTY, /* scope */
    MI_FLAG_ENABLEOVERRIDE|MI_FLAG_TOSUBCLASS, /* flavor */
    0, /* subscript */
    &Write_qual_decl_value, /* value */
};

static MI_QualifierDecl MI_CONST* MI_CONST qualifierDecls[] =
{
    &Abstract_qual_decl,
    &Aggregate_qual_decl,
    &Aggregation_qual_decl,
    &Alias_qual_decl,
    &ArrayType_qual_decl,
    &Association_qual_decl,
    &BitMap_qual_decl,
    &BitValues_qual_decl,
    &ClassConstraint_qual_decl,
    &ClassVersion_qual_decl,
    &Composition_qual_decl,
    &Correlatable_qual_decl,
    &Counter_qual_decl,
    &Delete_qual_decl,
    &Deprecated_qual_decl,
    &Description_qual_decl,
    &DisplayDescription_qual_decl,
    &DisplayName_qual_decl,
    &DN_qual_decl,
    &EmbeddedInstance_qual_decl,
    &EmbeddedObject_qual_decl,
    &Exception_qual_decl,
    &Expensive_qual_decl,
    &Experimental_qual_decl,
    &Gauge_qual_decl,
    &Ifdeleted_qual_decl,
    &In_qual_decl,
    &Indication_qual_decl,
    &Invisible_qual_decl,
    &IsPUnit_qual_decl,
    &Key_qual_decl,
    &Large_qual_decl,
    &MappingStrings_qual_decl,
    &Max_qual_decl,
    &MaxLen_qual_decl,
    &MaxValue_qual_decl,
    &MethodConstraint_qual_decl,
    &Min_qual_decl,
    &MinLen_qual_decl,
    &MinValue_qual_decl,
    &ModelCorrespondence_qual_decl,
    &Nonlocal_qual_decl,
    &NonlocalType_qual_decl,
    &NullValue_qual_decl,
    &Octetstring_qual_decl,
    &Out_qual_decl,
    &Override_qual_decl,
    &Propagated_qual_decl,
    &PropertyConstraint_qual_decl,
    &PropertyUsage_qual_decl,
    &Provider_qual_decl,
    &PUnit_qual_decl,
    &Read_qual_decl,
    &Required_qual_decl,
    &Revision_qual_decl,
    &Schema_qual_decl,
    &Source_qual_decl,
    &SourceType_qual_decl,
    &Static_qual_decl,
    &Stream_qual_decl,
    &Syntax_qual_decl,
    &SyntaxType_qual_decl,
    &Terminal_qual_decl,
    &TriggerType_qual_decl,
    &UMLPackagePath_qual_decl,
    &Units_qual_decl,
    &UnknownValues_qual_decl,
    &UnsupportedValues_qual_decl,
    &ValueMap_qual_decl,
    &Values_qual_decl,
    &Version_qual_decl,
    &Weak_qual_decl,
    &Write_qual_decl,
};

/*
**==============================================================================
**
** CIM_Indication
**
**==============================================================================
*/

static MI_CONST MI_Char* CIM_Indication_IndicationIdentifier_Description_qual_value = MI_T("An identifier for the Indication. This property is similar to a key value in that it can be used for identification, when correlating Indications (see the CorrelatedIndications array). Its value SHOULD be unique as long as correlations are reported, but MAY be reused or left NULL if no future Indications will reference it in their CorrelatedIndications array.To ensure uniqueness, the value of IndicationIdentifier should be constructed using the following \"preferred\" algorithm: \n<OrgID>:<LocalID> \nWhere <OrgID> and <LocalID> are separated by a colon (:), and where <OrgID> must include a copyrighted, trademarked, or otherwise unique name that is owned by the business entity that is creating or defining the IndicationIdentifier or that is a recognized ID that is assigned to the business entity by a recognized global authority. (This requirement is similar to the <Schema Name>_<Class Name> structure of Schema class names.) In addition, to ensure uniqueness <OrgID> must not contain a colon (:). When using this algorithm, the first colon to appear in IndicationIdentifier must appear between <OrgID> and <LocalID>. \n<LocalID> is chosen by the business entity and should not be re-used to identify different underlying (real-world) elements. \nIf the above \"preferred\" algorithm is not used, the defining entity should assure that the resulting IndicationIdentifier is not re-used across any IndicationIdentifiers that are produced by this or other providers for the NameSpace of this instance. \nFor DMTF-defined instances, the \"preferred\" algorithm should be used with the <OrgID> set to CIM.");

static MI_CONST MI_Qualifier CIM_Indication_IndicationIdentifier_Description_qual =
{
    MI_T("Description"),
    MI_STRING,
    MI_FLAG_ENABLEOVERRIDE|MI_FLAG_TOSUBCLASS|MI_FLAG_TRANSLATABLE,
    &CIM_Indication_IndicationIdentifier_Description_qual_value
};

static MI_Qualifier MI_CONST* MI_CONST CIM_Indication_IndicationIdentifier_quals[] =
{
    &CIM_Indication_IndicationIdentifier_Description_qual,
};

/* property CIM_Indication.IndicationIdentifier */
static MI_CONST MI_PropertyDecl CIM_Indication_IndicationIdentifier_prop =
{
    MI_FLAG_PROPERTY|MI_FLAG_READONLY, /* flags */
    0x00697214, /* code */
    MI_T("IndicationIdentifier"), /* name */
    CIM_Indication_IndicationIdentifier_quals, /* qualifiers */
    MI_COUNT(CIM_Indication_IndicationIdentifier_quals), /* numQualifiers */
    MI_STRING, /* type */
    NULL, /* className */
    0, /* subscript */
    offsetof(CIM_Indication, IndicationIdentifier), /* offset */
    MI_T("CIM_Indication"), /* origin */
    MI_T("CIM_Indication"), /* propagator */
    NULL,
};

static MI_CONST MI_Char* CIM_Indication_CorrelatedIndications_Description_qual_value = MI_T("A list of IndicationIdentifiers whose notifications are correlated with (related to) this one.");

static MI_CONST MI_Qualifier CIM_Indication_CorrelatedIndications_Description_qual =
{
    MI_T("Description"),
    MI_STRING,
    MI_FLAG_ENABLEOVERRIDE|MI_FLAG_TOSUBCLASS|MI_FLAG_TRANSLATABLE,
    &CIM_Indication_CorrelatedIndications_Description_qual_value
};

static MI_CONST MI_Char* CIM_Indication_CorrelatedIndications_ModelCorrespondence_qual_data_value[] =
{
    MI_T("CIM_Indication.IndicationIdentifier"),
};

static MI_CONST MI_ConstStringA CIM_Indication_CorrelatedIndications_ModelCorrespondence_qual_value =
{
    CIM_Indication_CorrelatedIndications_ModelCorrespondence_qual_data_value,
    MI_COUNT(CIM_Indication_CorrelatedIndications_ModelCorrespondence_qual_data_value),
};

static MI_CONST MI_Qualifier CIM_Indication_CorrelatedIndications_ModelCorrespondence_qual =
{
    MI_T("ModelCorrespondence"),
    MI_STRINGA,
    MI_FLAG_ENABLEOVERRIDE|MI_FLAG_TOSUBCLASS,
    &CIM_Indication_CorrelatedIndications_ModelCorrespondence_qual_value
};

static MI_Qualifier MI_CONST* MI_CONST CIM_Indication_CorrelatedIndications_quals[] =
{
    &CIM_Indication_CorrelatedIndications_Description_qual,
    &CIM_Indication_CorrelatedIndications_ModelCorrespondence_qual,
};

/* property CIM_Indication.CorrelatedIndications */
static MI_CONST MI_PropertyDecl CIM_Indication_CorrelatedIndications_prop =
{
    MI_FLAG_PROPERTY|MI_FLAG_READONLY, /* flags */
    0x00637315, /* code */
    MI_T("CorrelatedIndications"), /* name */
    CIM_Indication_CorrelatedIndications_quals, /* qualifiers */
    MI_COUNT(CIM_Indication_CorrelatedIndications_quals), /* numQualifiers */
    MI_STRINGA, /* type */
    NULL, /* className */
    0, /* subscript */
    offsetof(CIM_Indication, CorrelatedIndications), /* offset */
    MI_T("CIM_Indication"), /* origin */
    MI_T("CIM_Indication"), /* propagator */
    NULL,
};

static MI_CONST MI_Char* CIM_Indication_IndicationTime_Description_qual_value = MI_T("The time and date of creation of the Indication. The property may be set to NULL if the entity creating the Indication is not capable of determining this information. Note that IndicationTime may be the same for two Indications that are generated in rapid succession.");

static MI_CONST MI_Qualifier CIM_Indication_IndicationTime_Description_qual =
{
    MI_T("Description"),
    MI_STRING,
    MI_FLAG_ENABLEOVERRIDE|MI_FLAG_TOSUBCLASS|MI_FLAG_TRANSLATABLE,
    &CIM_Indication_IndicationTime_Description_qual_value
};

static MI_Qualifier MI_CONST* MI_CONST CIM_Indication_IndicationTime_quals[] =
{
    &CIM_Indication_IndicationTime_Description_qual,
};

/* property CIM_Indication.IndicationTime */
static MI_CONST MI_PropertyDecl CIM_Indication_IndicationTime_prop =
{
    MI_FLAG_PROPERTY|MI_FLAG_READONLY, /* flags */
    0x0069650E, /* code */
    MI_T("IndicationTime"), /* name */
    CIM_Indication_IndicationTime_quals, /* qualifiers */
    MI_COUNT(CIM_Indication_IndicationTime_quals), /* numQualifiers */
    MI_DATETIME, /* type */
    NULL, /* className */
    0, /* subscript */
    offsetof(CIM_Indication, IndicationTime), /* offset */
    MI_T("CIM_Indication"), /* origin */
    MI_T("CIM_Indication"), /* propagator */
    NULL,
};

static MI_CONST MI_Char* CIM_Indication_PerceivedSeverity_Description_qual_value = MI_T("An enumerated value that describes the severity of the Indication from the notifier\'s point of view: \n1 - Other, by CIM convention, is used to indicate that the Severity\'s value can be found in the OtherSeverity property. \n3 - Degraded/Warning should be used when its appropriate to let the user decide if action is needed. \n4 - Minor should be used to indicate action is needed, but the situation is not serious at this time. \n5 - Major should be used to indicate action is needed NOW. \n6 - Critical should be used to indicate action is needed NOW and the scope is broad (perhaps an imminent outage to a critical resource will result). \n7 - Fatal/NonRecoverable should be used to indicate an error occurred, but it\'s too late to take remedial action. \n2 and 0 - Information and Unknown (respectively) follow common usage. Literally, the Indication is purely informational or its severity is simply unknown.");

static MI_CONST MI_Qualifier CIM_Indication_PerceivedSeverity_Description_qual =
{
    MI_T("Description"),
    MI_STRING,
    MI_FLAG_ENABLEOVERRIDE|MI_FLAG_TOSUBCLASS|MI_FLAG_TRANSLATABLE,
    &CIM_Indication_PerceivedSeverity_Description_qual_value
};

static MI_CONST MI_Char* CIM_Indication_PerceivedSeverity_ValueMap_qual_data_value[] =
{
    MI_T("0"),
    MI_T("1"),
    MI_T("2"),
    MI_T("3"),
    MI_T("4"),
    MI_T("5"),
    MI_T("6"),
    MI_T("7"),
    MI_T(".."),
};

static MI_CONST MI_ConstStringA CIM_Indication_PerceivedSeverity_ValueMap_qual_value =
{
    CIM_Indication_PerceivedSeverity_ValueMap_qual_data_value,
    MI_COUNT(CIM_Indication_PerceivedSeverity_ValueMap_qual_data_value),
};

static MI_CONST MI_Qualifier CIM_Indication_PerceivedSeverity_ValueMap_qual =
{
    MI_T("ValueMap"),
    MI_STRINGA,
    MI_FLAG_ENABLEOVERRIDE|MI_FLAG_TOSUBCLASS,
    &CIM_Indication_PerceivedSeverity_ValueMap_qual_value
};

static MI_CONST MI_Char* CIM_Indication_PerceivedSeverity_Values_qual_data_value[] =
{
    MI_T("Unknown"),
    MI_T("Other"),
    MI_T("Information"),
    MI_T("Degraded/Warning"),
    MI_T("Minor"),
    MI_T("Major"),
    MI_T("Critical"),
    MI_T("Fatal/NonRecoverable"),
    MI_T("DMTF Reserved"),
};

static MI_CONST MI_ConstStringA CIM_Indication_PerceivedSeverity_Values_qual_value =
{
    CIM_Indication_PerceivedSeverity_Values_qual_data_value,
    MI_COUNT(CIM_Indication_PerceivedSeverity_Values_qual_data_value),
};

static MI_CONST MI_Qualifier CIM_Indication_PerceivedSeverity_Values_qual =
{
    MI_T("Values"),
    MI_STRINGA,
    MI_FLAG_ENABLEOVERRIDE|MI_FLAG_TOSUBCLASS|MI_FLAG_TRANSLATABLE,
    &CIM_Indication_PerceivedSeverity_Values_qual_value
};

static MI_Qualifier MI_CONST* MI_CONST CIM_Indication_PerceivedSeverity_quals[] =
{
    &CIM_Indication_PerceivedSeverity_Description_qual,
    &CIM_Indication_PerceivedSeverity_ValueMap_qual,
    &CIM_Indication_PerceivedSeverity_Values_qual,
};

/* property CIM_Indication.PerceivedSeverity */
static MI_CONST MI_PropertyDecl CIM_Indication_PerceivedSeverity_prop =
{
    MI_FLAG_PROPERTY|MI_FLAG_READONLY, /* flags */
    0x00707911, /* code */
    MI_T("PerceivedSeverity"), /* name */
    CIM_Indication_PerceivedSeverity_quals, /* qualifiers */
    MI_COUNT(CIM_Indication_PerceivedSeverity_quals), /* numQualifiers */
    MI_UINT16, /* type */
    NULL, /* className */
    0, /* subscript */
    offsetof(CIM_Indication, PerceivedSeverity), /* offset */
    MI_T("CIM_Indication"), /* origin */
    MI_T("CIM_Indication"), /* propagator */
    NULL,
};

static MI_CONST MI_Char* CIM_Indication_OtherSeverity_Description_qual_value = MI_T("Holds the value of the user defined severity value when \'PerceivedSeverity\' is 1 (\"Other\").");

static MI_CONST MI_Qualifier CIM_Indication_OtherSeverity_Description_qual =
{
    MI_T("Description"),
    MI_STRING,
    MI_FLAG_ENABLEOVERRIDE|MI_FLAG_TOSUBCLASS|MI_FLAG_TRANSLATABLE,
    &CIM_Indication_OtherSeverity_Description_qual_value
};

static MI_CONST MI_Char* CIM_Indication_OtherSeverity_ModelCorrespondence_qual_data_value[] =
{
    MI_T("CIM_AlertIndication.PerceivedSeverity"),
};

static MI_CONST MI_ConstStringA CIM_Indication_OtherSeverity_ModelCorrespondence_qual_value =
{
    CIM_Indication_OtherSeverity_ModelCorrespondence_qual_data_value,
    MI_COUNT(CIM_Indication_OtherSeverity_ModelCorrespondence_qual_data_value),
};

static MI_CONST MI_Qualifier CIM_Indication_OtherSeverity_ModelCorrespondence_qual =
{
    MI_T("ModelCorrespondence"),
    MI_STRINGA,
    MI_FLAG_ENABLEOVERRIDE|MI_FLAG_TOSUBCLASS,
    &CIM_Indication_OtherSeverity_ModelCorrespondence_qual_value
};

static MI_Qualifier MI_CONST* MI_CONST CIM_Indication_OtherSeverity_quals[] =
{
    &CIM_Indication_OtherSeverity_Description_qual,
    &CIM_Indication_OtherSeverity_ModelCorrespondence_qual,
};

/* property CIM_Indication.OtherSeverity */
static MI_CONST MI_PropertyDecl CIM_Indication_OtherSeverity_prop =
{
    MI_FLAG_PROPERTY|MI_FLAG_READONLY, /* flags */
    0x006F790D, /* code */
    MI_T("OtherSeverity"), /* name */
    CIM_Indication_OtherSeverity_quals, /* qualifiers */
    MI_COUNT(CIM_Indication_OtherSeverity_quals), /* numQualifiers */
    MI_STRING, /* type */
    NULL, /* className */
    0, /* subscript */
    offsetof(CIM_Indication, OtherSeverity), /* offset */
    MI_T("CIM_Indication"), /* origin */
    MI_T("CIM_Indication"), /* propagator */
    NULL,
};

static MI_CONST MI_Char* CIM_Indication_IndicationFilterName_Description_qual_value = MI_T("An identifier for the indication filter that selects this indication and causes it to be sent. This property is to be filled out by the indication sending service. The value shall be correlatable with the Name property of the instance of CIM_IndicationFilter describing the criteria of the indication. The value of the IndicationFilterName should be formatted using the following algorithm: < OrgID > : < LocalID >, where < OrgID > and < LocalID > are separated by a colon (:) and < OrgID > shall include a copyrighted, trademarked, or otherwise unique name that is owned by the business entity that is creating or defining the value or that is a registered ID assigned to the business entity by a recognized global authority. In addition, to ensure uniqueness, < OrgID > shall not contain a colon (:).When using this algorithm, the first colon to appear in the value shall appear between < OrgID > and < LocalID >. < LocalID > is chosen by the business entity and shall be used uniquely.");

static MI_CONST MI_Qualifier CIM_Indication_IndicationFilterName_Description_qual =
{
    MI_T("Description"),
    MI_STRING,
    MI_FLAG_ENABLEOVERRIDE|MI_FLAG_TOSUBCLASS|MI_FLAG_TRANSLATABLE,
    &CIM_Indication_IndicationFilterName_Description_qual_value
};

static MI_CONST MI_Char* CIM_Indication_IndicationFilterName_ModelCorrespondence_qual_data_value[] =
{
    MI_T("CIM_IndicationFilter.Name"),
};

static MI_CONST MI_ConstStringA CIM_Indication_IndicationFilterName_ModelCorrespondence_qual_value =
{
    CIM_Indication_IndicationFilterName_ModelCorrespondence_qual_data_value,
    MI_COUNT(CIM_Indication_IndicationFilterName_ModelCorrespondence_qual_data_value),
};

static MI_CONST MI_Qualifier CIM_Indication_IndicationFilterName_ModelCorrespondence_qual =
{
    MI_T("ModelCorrespondence"),
    MI_STRINGA,
    MI_FLAG_ENABLEOVERRIDE|MI_FLAG_TOSUBCLASS,
    &CIM_Indication_IndicationFilterName_ModelCorrespondence_qual_value
};

static MI_Qualifier MI_CONST* MI_CONST CIM_Indication_IndicationFilterName_quals[] =
{
    &CIM_Indication_IndicationFilterName_Description_qual,
    &CIM_Indication_IndicationFilterName_ModelCorrespondence_qual,
};

/* property CIM_Indication.IndicationFilterName */
static MI_CONST MI_PropertyDecl CIM_Indication_IndicationFilterName_prop =
{
    MI_FLAG_PROPERTY|MI_FLAG_READONLY, /* flags */
    0x00696514, /* code */
    MI_T("IndicationFilterName"), /* name */
    CIM_Indication_IndicationFilterName_quals, /* qualifiers */
    MI_COUNT(CIM_Indication_IndicationFilterName_quals), /* numQualifiers */
    MI_STRING, /* type */
    NULL, /* className */
    0, /* subscript */
    offsetof(CIM_Indication, IndicationFilterName), /* offset */
    MI_T("CIM_Indication"), /* origin */
    MI_T("CIM_Indication"), /* propagator */
    NULL,
};

static MI_CONST MI_Char* CIM_Indication_SequenceContext_Description_qual_value = MI_T("The sequence context portion of a sequence identifier for the indication. The sequence number portion of the sequence identifier is provided by the SequenceNumber property. The combination of both property values represents the sequence identifier for the indication.\nThe sequence identifier for the indication enables a CIM listener to identify duplicate indications when the CIM service attempts the delivery retry of indications, to reorder indications that arrive out-of-order, and to detect lost indications.\nIf a CIM service does not support sequence identifiers for indications, this property shall be NULL.\nIf a CIM service supports sequence identifiers for indications, this property shall be maintained by the CIM service for each registered listener destination, and its value shall uniquely identify the CIM service and the indication service within the CIM service such that restarts of the CIM service and deregistration of listener destinations to the CIM service cause the value to change, without reusing earlier values for a sufficiently long time.\nWhen retrying the delivery of an indication, this property shall have the same value as in the original delivery.\nTo guarantee this uniqueness, the property value should be constructed using the following format (defined in ABNF): sequence-context = indication-service-name \"#\" cim-service-start-id \"#\" listener-destination-creation-time\nWhere: indication-service-name is the value of the Name property of the CIM_IndicationService instance responsible for delivering the indication. cim-service-start-id is an identifier that uniquely identifies the CIM service start, for example via a timestamp of the start time, or via a counter that increases for each start or restart. listener-destination-creation-time is a timestamp of the creation time of the CIM_ListenerDestination instance representing the listener destination.\nSince this format is only a recommendation, CIM clients shall treat the value as an opaque identifier for the sequence context and shall not rely on this format.");

static MI_CONST MI_Qualifier CIM_Indication_SequenceContext_Description_qual =
{
    MI_T("Description"),
    MI_STRING,
    MI_FLAG_ENABLEOVERRIDE|MI_FLAG_TOSUBCLASS|MI_FLAG_TRANSLATABLE,
    &CIM_Indication_SequenceContext_Description_qual_value
};

static MI_CONST MI_Char* CIM_Indication_SequenceContext_ModelCorrespondence_qual_data_value[] =
{
    MI_T("CIM_Indication.SequenceNumber"),
};

static MI_CONST MI_ConstStringA CIM_Indication_SequenceContext_ModelCorrespondence_qual_value =
{
    CIM_Indication_SequenceContext_ModelCorrespondence_qual_data_value,
    MI_COUNT(CIM_Indication_SequenceContext_ModelCorrespondence_qual_data_value),
};

static MI_CONST MI_Qualifier CIM_Indication_SequenceContext_ModelCorrespondence_qual =
{
    MI_T("ModelCorrespondence"),
    MI_STRINGA,
    MI_FLAG_ENABLEOVERRIDE|MI_FLAG_TOSUBCLASS,
    &CIM_Indication_SequenceContext_ModelCorrespondence_qual_value
};

static MI_Qualifier MI_CONST* MI_CONST CIM_Indication_SequenceContext_quals[] =
{
    &CIM_Indication_SequenceContext_Description_qual,
    &CIM_Indication_SequenceContext_ModelCorrespondence_qual,
};

/* property CIM_Indication.SequenceContext */
static MI_CONST MI_PropertyDecl CIM_Indication_SequenceContext_prop =
{
    MI_FLAG_PROPERTY|MI_FLAG_READONLY, /* flags */
    0x0073740F, /* code */
    MI_T("SequenceContext"), /* name */
    CIM_Indication_SequenceContext_quals, /* qualifiers */
    MI_COUNT(CIM_Indication_SequenceContext_quals), /* numQualifiers */
    MI_STRING, /* type */
    NULL, /* className */
    0, /* subscript */
    offsetof(CIM_Indication, SequenceContext), /* offset */
    MI_T("CIM_Indication"), /* origin */
    MI_T("CIM_Indication"), /* propagator */
    NULL,
};

static MI_CONST MI_Char* CIM_Indication_SequenceNumber_Description_qual_value = MI_T("The sequence number portion of a sequence identifier for the indication. The sequence context portion of the sequence identifier is provided by the SequenceContext property. The combination of both property values represents the sequence identifier for the indication.\nThe sequence identifier for the indication enables a CIM listener to identify duplicate indications when the CIM service attempts the delivery retry of indications, to reorder indications that arrive out-of-order, and to detect lost indications.\nIf a CIM service does not support sequence identifiers for indications, this property shall be NULL.\nIf a CIM service supports sequence identifiers for indications, this property shall be maintained by the CIM service for each registered listener destination, and its value shall uniquely identify the indication within the sequence context provided by SequenceContext. It shall start at 0 whenever the sequence context string changes. Otherwise, it shall be increased by 1 for every new indication to that listener destination, and it shall wrap to 0 when the value range is exceeded.\nWhen retrying the delivery of an indication, this property shall have the same value as in the original delivery.");

static MI_CONST MI_Qualifier CIM_Indication_SequenceNumber_Description_qual =
{
    MI_T("Description"),
    MI_STRING,
    MI_FLAG_ENABLEOVERRIDE|MI_FLAG_TOSUBCLASS|MI_FLAG_TRANSLATABLE,
    &CIM_Indication_SequenceNumber_Description_qual_value
};

static MI_CONST MI_Char* CIM_Indication_SequenceNumber_ModelCorrespondence_qual_data_value[] =
{
    MI_T("CIM_Indication.SequenceContext"),
};

static MI_CONST MI_ConstStringA CIM_Indication_SequenceNumber_ModelCorrespondence_qual_value =
{
    CIM_Indication_SequenceNumber_ModelCorrespondence_qual_data_value,
    MI_COUNT(CIM_Indication_SequenceNumber_ModelCorrespondence_qual_data_value),
};

static MI_CONST MI_Qualifier CIM_Indication_SequenceNumber_ModelCorrespondence_qual =
{
    MI_T("ModelCorrespondence"),
    MI_STRINGA,
    MI_FLAG_ENABLEOVERRIDE|MI_FLAG_TOSUBCLASS,
    &CIM_Indication_SequenceNumber_ModelCorrespondence_qual_value
};

static MI_Qualifier MI_CONST* MI_CONST CIM_Indication_SequenceNumber_quals[] =
{
    &CIM_Indication_SequenceNumber_Description_qual,
    &CIM_Indication_SequenceNumber_ModelCorrespondence_qual,
};

/* property CIM_Indication.SequenceNumber */
static MI_CONST MI_PropertyDecl CIM_Indication_SequenceNumber_prop =
{
    MI_FLAG_PROPERTY|MI_FLAG_READONLY, /* flags */
    0x0073720E, /* code */
    MI_T("SequenceNumber"), /* name */
    CIM_Indication_SequenceNumber_quals, /* qualifiers */
    MI_COUNT(CIM_Indication_SequenceNumber_quals), /* numQualifiers */
    MI_SINT64, /* type */
    NULL, /* className */
    0, /* subscript */
    offsetof(CIM_Indication, SequenceNumber), /* offset */
    MI_T("CIM_Indication"), /* origin */
    MI_T("CIM_Indication"), /* propagator */
    NULL,
};

static MI_PropertyDecl MI_CONST* MI_CONST CIM_Indication_props[] =
{
    &CIM_Indication_IndicationIdentifier_prop,
    &CIM_Indication_CorrelatedIndications_prop,
    &CIM_Indication_IndicationTime_prop,
    &CIM_Indication_PerceivedSeverity_prop,
    &CIM_Indication_OtherSeverity_prop,
    &CIM_Indication_IndicationFilterName_prop,
    &CIM_Indication_SequenceContext_prop,
    &CIM_Indication_SequenceNumber_prop,
};

static MI_CONST MI_Boolean CIM_Indication_Indication_qual_value = 1;

static MI_CONST MI_Qualifier CIM_Indication_Indication_qual =
{
    MI_T("Indication"),
    MI_BOOLEAN,
    MI_FLAG_DISABLEOVERRIDE|MI_FLAG_TOSUBCLASS,
    &CIM_Indication_Indication_qual_value
};

static MI_CONST MI_Boolean CIM_Indication_Abstract_qual_value = 1;

static MI_CONST MI_Qualifier CIM_Indication_Abstract_qual =
{
    MI_T("Abstract"),
    MI_BOOLEAN,
    MI_FLAG_ENABLEOVERRIDE|MI_FLAG_RESTRICTED,
    &CIM_Indication_Abstract_qual_value
};

static MI_CONST MI_Char* CIM_Indication_Version_qual_value = MI_T("2.24.0");

static MI_CONST MI_Qualifier CIM_Indication_Version_qual =
{
    MI_T("Version"),
    MI_STRING,
    MI_FLAG_ENABLEOVERRIDE|MI_FLAG_TRANSLATABLE|MI_FLAG_RESTRICTED,
    &CIM_Indication_Version_qual_value
};

static MI_CONST MI_Char* CIM_Indication_UMLPackagePath_qual_value = MI_T("CIM::Event");

static MI_CONST MI_Qualifier CIM_Indication_UMLPackagePath_qual =
{
    MI_T("UMLPackagePath"),
    MI_STRING,
    MI_FLAG_ENABLEOVERRIDE|MI_FLAG_TOSUBCLASS,
    &CIM_Indication_UMLPackagePath_qual_value
};

static MI_CONST MI_Char* CIM_Indication_Description_qual_value = MI_T("CIM_Indication is the abstract root class for all notifications about changes in schema, objects and their data, and about events detected by providers and instrumentation. Subclasses represent specific types of notifications. \n\nTo receive an Indication, a consumer (or subscriber) must create an instance of CIM_IndicationFilter describing the criteria of the notification, an instance of CIM_ListenerDestination describing the delivery of the notification, and an instance of CIM_IndicationSubscription associating the Filter and Handler.");

static MI_CONST MI_Qualifier CIM_Indication_Description_qual =
{
    MI_T("Description"),
    MI_STRING,
    MI_FLAG_ENABLEOVERRIDE|MI_FLAG_TOSUBCLASS|MI_FLAG_TRANSLATABLE,
    &CIM_Indication_Description_qual_value
};

static MI_Qualifier MI_CONST* MI_CONST CIM_Indication_quals[] =
{
    &CIM_Indication_Indication_qual,
    &CIM_Indication_Abstract_qual,
    &CIM_Indication_Version_qual,
    &CIM_Indication_UMLPackagePath_qual,
    &CIM_Indication_Description_qual,
};

/* class CIM_Indication */
MI_CONST MI_ClassDecl CIM_Indication_rtti =
{
    MI_FLAG_CLASS|MI_FLAG_INDICATION|MI_FLAG_ABSTRACT, /* flags */
    0x00636E0E, /* code */
    MI_T("CIM_Indication"), /* name */
    CIM_Indication_quals, /* qualifiers */
    MI_COUNT(CIM_Indication_quals), /* numQualifiers */
    CIM_Indication_props, /* properties */
    MI_COUNT(CIM_Indication_props), /* numProperties */
    sizeof(CIM_Indication), /* size */
    NULL, /* superClass */
    NULL, /* superClassDecl */
    NULL, /* methods */
    0, /* numMethods */
    &schemaDecl, /* schema */
    NULL, /* functions */
    NULL /* owningClass */
};

/*
**==============================================================================
**
** CIM_InstIndication
**
**==============================================================================
*/

static MI_CONST MI_Boolean CIM_InstIndication_SourceInstance_Required_qual_value = 1;

static MI_CONST MI_Qualifier CIM_InstIndication_SourceInstance_Required_qual =
{
    MI_T("Required"),
    MI_BOOLEAN,
    MI_FLAG_DISABLEOVERRIDE|MI_FLAG_TOSUBCLASS,
    &CIM_InstIndication_SourceInstance_Required_qual_value
};

static MI_CONST MI_Char* CIM_InstIndication_SourceInstance_Description_qual_value = MI_T("A copy of the instance that changed to generate the Indication. SourceInstance contains the current values of the properties selected by the Indication Filter\'s Query. In the case of CIM_InstDeletion, the property values are copied before the instance is deleted.");

static MI_CONST MI_Qualifier CIM_InstIndication_SourceInstance_Description_qual =
{
    MI_T("Description"),
    MI_STRING,
    MI_FLAG_ENABLEOVERRIDE|MI_FLAG_TOSUBCLASS|MI_FLAG_TRANSLATABLE,
    &CIM_InstIndication_SourceInstance_Description_qual_value
};

static MI_CONST MI_Boolean CIM_InstIndication_SourceInstance_EmbeddedObject_qual_value = 1;

static MI_CONST MI_Qualifier CIM_InstIndication_SourceInstance_EmbeddedObject_qual =
{
    MI_T("EmbeddedObject"),
    MI_BOOLEAN,
    MI_FLAG_DISABLEOVERRIDE|MI_FLAG_TOSUBCLASS,
    &CIM_InstIndication_SourceInstance_EmbeddedObject_qual_value
};

static MI_Qualifier MI_CONST* MI_CONST CIM_InstIndication_SourceInstance_quals[] =
{
    &CIM_InstIndication_SourceInstance_Required_qual,
    &CIM_InstIndication_SourceInstance_Description_qual,
    &CIM_InstIndication_SourceInstance_EmbeddedObject_qual,
};

/* property CIM_InstIndication.SourceInstance */
static MI_CONST MI_PropertyDecl CIM_InstIndication_SourceInstance_prop =
{
    MI_FLAG_PROPERTY|MI_FLAG_REQUIRED|MI_FLAG_READONLY, /* flags */
    0x0073650E, /* code */
    MI_T("SourceInstance"), /* name */
    CIM_InstIndication_SourceInstance_quals, /* qualifiers */
    MI_COUNT(CIM_InstIndication_SourceInstance_quals), /* numQualifiers */
    MI_INSTANCE, /* type */
    NULL, /* className */
    0, /* subscript */
    offsetof(CIM_InstIndication, SourceInstance), /* offset */
    MI_T("CIM_InstIndication"), /* origin */
    MI_T("CIM_InstIndication"), /* propagator */
    NULL,
};

static MI_CONST MI_Char* CIM_InstIndication_SourceInstanceModelPath_Description_qual_value = MI_T("The Model Path of the SourceInstance. The following format MUST be used to encode the Model Path: \n<NamespacePath>:<ClassName>.<Prop1>=\"<Value1>\", \n<Prop2>=\"<Value2>\", ...");

static MI_CONST MI_Qualifier CIM_InstIndication_SourceInstanceModelPath_Description_qual =
{
    MI_T("Description"),
    MI_STRING,
    MI_FLAG_ENABLEOVERRIDE|MI_FLAG_TOSUBCLASS|MI_FLAG_TRANSLATABLE,
    &CIM_InstIndication_SourceInstanceModelPath_Description_qual_value
};

static MI_CONST MI_Char* CIM_InstIndication_SourceInstanceModelPath_ModelCorrespondence_qual_data_value[] =
{
    MI_T("CIM_InstIndication.SourceInstance"),
};

static MI_CONST MI_ConstStringA CIM_InstIndication_SourceInstanceModelPath_ModelCorrespondence_qual_value =
{
    CIM_InstIndication_SourceInstanceModelPath_ModelCorrespondence_qual_data_value,
    MI_COUNT(CIM_InstIndication_SourceInstanceModelPath_ModelCorrespondence_qual_data_value),
};

static MI_CONST MI_Qualifier CIM_InstIndication_SourceInstanceModelPath_ModelCorrespondence_qual =
{
    MI_T("ModelCorrespondence"),
    MI_STRINGA,
    MI_FLAG_ENABLEOVERRIDE|MI_FLAG_TOSUBCLASS,
    &CIM_InstIndication_SourceInstanceModelPath_ModelCorrespondence_qual_value
};

static MI_Qualifier MI_CONST* MI_CONST CIM_InstIndication_SourceInstanceModelPath_quals[] =
{
    &CIM_InstIndication_SourceInstanceModelPath_Description_qual,
    &CIM_InstIndication_SourceInstanceModelPath_ModelCorrespondence_qual,
};

/* property CIM_InstIndication.SourceInstanceModelPath */
static MI_CONST MI_PropertyDecl CIM_InstIndication_SourceInstanceModelPath_prop =
{
    MI_FLAG_PROPERTY|MI_FLAG_READONLY, /* flags */
    0x00736817, /* code */
    MI_T("SourceInstanceModelPath"), /* name */
    CIM_InstIndication_SourceInstanceModelPath_quals, /* qualifiers */
    MI_COUNT(CIM_InstIndication_SourceInstanceModelPath_quals), /* numQualifiers */
    MI_STRING, /* type */
    NULL, /* className */
    0, /* subscript */
    offsetof(CIM_InstIndication, SourceInstanceModelPath), /* offset */
    MI_T("CIM_InstIndication"), /* origin */
    MI_T("CIM_InstIndication"), /* propagator */
    NULL,
};

static MI_CONST MI_Char* CIM_InstIndication_SourceInstanceHost_Description_qual_value = MI_T("The host name or IP address of the SourceInstance.");

static MI_CONST MI_Qualifier CIM_InstIndication_SourceInstanceHost_Description_qual =
{
    MI_T("Description"),
    MI_STRING,
    MI_FLAG_ENABLEOVERRIDE|MI_FLAG_TOSUBCLASS|MI_FLAG_TRANSLATABLE,
    &CIM_InstIndication_SourceInstanceHost_Description_qual_value
};

static MI_CONST MI_Char* CIM_InstIndication_SourceInstanceHost_ModelCorrespondence_qual_data_value[] =
{
    MI_T("CIM_InstIndication.SourceInstance"),
};

static MI_CONST MI_ConstStringA CIM_InstIndication_SourceInstanceHost_ModelCorrespondence_qual_value =
{
    CIM_InstIndication_SourceInstanceHost_ModelCorrespondence_qual_data_value,
    MI_COUNT(CIM_InstIndication_SourceInstanceHost_ModelCorrespondence_qual_data_value),
};

static MI_CONST MI_Qualifier CIM_InstIndication_SourceInstanceHost_ModelCorrespondence_qual =
{
    MI_T("ModelCorrespondence"),
    MI_STRINGA,
    MI_FLAG_ENABLEOVERRIDE|MI_FLAG_TOSUBCLASS,
    &CIM_InstIndication_SourceInstanceHost_ModelCorrespondence_qual_value
};

static MI_Qualifier MI_CONST* MI_CONST CIM_InstIndication_SourceInstanceHost_quals[] =
{
    &CIM_InstIndication_SourceInstanceHost_Description_qual,
    &CIM_InstIndication_SourceInstanceHost_ModelCorrespondence_qual,
};

/* property CIM_InstIndication.SourceInstanceHost */
static MI_CONST MI_PropertyDecl CIM_InstIndication_SourceInstanceHost_prop =
{
    MI_FLAG_PROPERTY|MI_FLAG_READONLY, /* flags */
    0x00737412, /* code */
    MI_T("SourceInstanceHost"), /* name */
    CIM_InstIndication_SourceInstanceHost_quals, /* qualifiers */
    MI_COUNT(CIM_InstIndication_SourceInstanceHost_quals), /* numQualifiers */
    MI_STRING, /* type */
    NULL, /* className */
    0, /* subscript */
    offsetof(CIM_InstIndication, SourceInstanceHost), /* offset */
    MI_T("CIM_InstIndication"), /* origin */
    MI_T("CIM_InstIndication"), /* propagator */
    NULL,
};

static MI_PropertyDecl MI_CONST* MI_CONST CIM_InstIndication_props[] =
{
    &CIM_Indication_IndicationIdentifier_prop,
    &CIM_Indication_CorrelatedIndications_prop,
    &CIM_Indication_IndicationTime_prop,
    &CIM_Indication_PerceivedSeverity_prop,
    &CIM_Indication_OtherSeverity_prop,
    &CIM_Indication_IndicationFilterName_prop,
    &CIM_Indication_SequenceContext_prop,
    &CIM_Indication_SequenceNumber_prop,
    &CIM_InstIndication_SourceInstance_prop,
    &CIM_InstIndication_SourceInstanceModelPath_prop,
    &CIM_InstIndication_SourceInstanceHost_prop,
};

static MI_CONST MI_Boolean CIM_InstIndication_Indication_qual_value = 1;

static MI_CONST MI_Qualifier CIM_InstIndication_Indication_qual =
{
    MI_T("Indication"),
    MI_BOOLEAN,
    MI_FLAG_DISABLEOVERRIDE|MI_FLAG_TOSUBCLASS,
    &CIM_InstIndication_Indication_qual_value
};

static MI_CONST MI_Char* CIM_InstIndication_UMLPackagePath_qual_value = MI_T("CIM::Event");

static MI_CONST MI_Qualifier CIM_InstIndication_UMLPackagePath_qual =
{
    MI_T("UMLPackagePath"),
    MI_STRING,
    MI_FLAG_ENABLEOVERRIDE|MI_FLAG_TOSUBCLASS,
    &CIM_InstIndication_UMLPackagePath_qual_value
};

static MI_CONST MI_Char* CIM_InstIndication_Description_qual_value = MI_T("CIM_InstIndication is an abstract superclass describing changes to instances. Subclasses represent specific types of change notifications, such as instance creation, deletion and modification.");

static MI_CONST MI_Qualifier CIM_InstIndication_Description_qual =
{
    MI_T("Description"),
    MI_STRING,
    MI_FLAG_ENABLEOVERRIDE|MI_FLAG_TOSUBCLASS|MI_FLAG_TRANSLATABLE,
    &CIM_InstIndication_Description_qual_value
};

static MI_CONST MI_Boolean CIM_InstIndication_Abstract_qual_value = 1;

static MI_CONST MI_Qualifier CIM_InstIndication_Abstract_qual =
{
    MI_T("Abstract"),
    MI_BOOLEAN,
    MI_FLAG_ENABLEOVERRIDE|MI_FLAG_RESTRICTED,
    &CIM_InstIndication_Abstract_qual_value
};

static MI_CONST MI_Char* CIM_InstIndication_Version_qual_value = MI_T("2.9.0");

static MI_CONST MI_Qualifier CIM_InstIndication_Version_qual =
{
    MI_T("Version"),
    MI_STRING,
    MI_FLAG_ENABLEOVERRIDE|MI_FLAG_TRANSLATABLE|MI_FLAG_RESTRICTED,
    &CIM_InstIndication_Version_qual_value
};

static MI_Qualifier MI_CONST* MI_CONST CIM_InstIndication_quals[] =
{
    &CIM_InstIndication_Indication_qual,
    &CIM_InstIndication_UMLPackagePath_qual,
    &CIM_InstIndication_Description_qual,
    &CIM_InstIndication_Abstract_qual,
    &CIM_InstIndication_Version_qual,
};

/* class CIM_InstIndication */
MI_CONST MI_ClassDecl CIM_InstIndication_rtti =
{
    MI_FLAG_CLASS|MI_FLAG_INDICATION|MI_FLAG_ABSTRACT, /* flags */
    0x00636E12, /* code */
    MI_T("CIM_InstIndication"), /* name */
    CIM_InstIndication_quals, /* qualifiers */
    MI_COUNT(CIM_InstIndication_quals), /* numQualifiers */
    CIM_InstIndication_props, /* properties */
    MI_COUNT(CIM_InstIndication_props), /* numProperties */
    sizeof(CIM_InstIndication), /* size */
    MI_T("CIM_Indication"), /* superClass */
    &CIM_Indication_rtti, /* superClassDecl */
    NULL, /* methods */
    0, /* numMethods */
    &schemaDecl, /* schema */
    NULL, /* functions */
    NULL /* owningClass */
};

/*
**==============================================================================
**
** CIM_InstModification
**
**==============================================================================
*/

static MI_CONST MI_Char* CIM_InstModification_PreviousInstance_Description_qual_value = MI_T("A copy of the \'previous\' instance whose change generated the Indication. PreviousInstance contains \'older\' values of an instance\'s properties (as compared to SourceInstance), selected by the IndicationFilter\'s Query.");

static MI_CONST MI_Qualifier CIM_InstModification_PreviousInstance_Description_qual =
{
    MI_T("Description"),
    MI_STRING,
    MI_FLAG_ENABLEOVERRIDE|MI_FLAG_TOSUBCLASS|MI_FLAG_TRANSLATABLE,
    &CIM_InstModification_PreviousInstance_Description_qual_value
};

static MI_CONST MI_Boolean CIM_InstModification_PreviousInstance_EmbeddedObject_qual_value = 1;

static MI_CONST MI_Qualifier CIM_InstModification_PreviousInstance_EmbeddedObject_qual =
{
    MI_T("EmbeddedObject"),
    MI_BOOLEAN,
    MI_FLAG_DISABLEOVERRIDE|MI_FLAG_TOSUBCLASS,
    &CIM_InstModification_PreviousInstance_EmbeddedObject_qual_value
};

static MI_Qualifier MI_CONST* MI_CONST CIM_InstModification_PreviousInstance_quals[] =
{
    &CIM_InstModification_PreviousInstance_Description_qual,
    &CIM_InstModification_PreviousInstance_EmbeddedObject_qual,
};

/* property CIM_InstModification.PreviousInstance */
static MI_CONST MI_PropertyDecl CIM_InstModification_PreviousInstance_prop =
{
    MI_FLAG_PROPERTY|MI_FLAG_READONLY, /* flags */
    0x00706510, /* code */
    MI_T("PreviousInstance"), /* name */
    CIM_InstModification_PreviousInstance_quals, /* qualifiers */
    MI_COUNT(CIM_InstModification_PreviousInstance_quals), /* numQualifiers */
    MI_INSTANCE, /* type */
    NULL, /* className */
    0, /* subscript */
    offsetof(CIM_InstModification, PreviousInstance), /* offset */
    MI_T("CIM_InstModification"), /* origin */
    MI_T("CIM_InstModification"), /* propagator */
    NULL,
};

static MI_PropertyDecl MI_CONST* MI_CONST CIM_InstModification_props[] =
{
    &CIM_Indication_IndicationIdentifier_prop,
    &CIM_Indication_CorrelatedIndications_prop,
    &CIM_Indication_IndicationTime_prop,
    &CIM_Indication_PerceivedSeverity_prop,
    &CIM_Indication_OtherSeverity_prop,
    &CIM_Indication_IndicationFilterName_prop,
    &CIM_Indication_SequenceContext_prop,
    &CIM_Indication_SequenceNumber_prop,
    &CIM_InstIndication_SourceInstance_prop,
    &CIM_InstIndication_SourceInstanceModelPath_prop,
    &CIM_InstIndication_SourceInstanceHost_prop,
    &CIM_InstModification_PreviousInstance_prop,
};

static MI_CONST MI_Boolean CIM_InstModification_Indication_qual_value = 1;

static MI_CONST MI_Qualifier CIM_InstModification_Indication_qual =
{
    MI_T("Indication"),
    MI_BOOLEAN,
    MI_FLAG_DISABLEOVERRIDE|MI_FLAG_TOSUBCLASS,
    &CIM_InstModification_Indication_qual_value
};

static MI_CONST MI_Char* CIM_InstModification_UMLPackagePath_qual_value = MI_T("CIM::Event");

static MI_CONST MI_Qualifier CIM_InstModification_UMLPackagePath_qual =
{
    MI_T("UMLPackagePath"),
    MI_STRING,
    MI_FLAG_ENABLEOVERRIDE|MI_FLAG_TOSUBCLASS,
    &CIM_InstModification_UMLPackagePath_qual_value
};

static MI_CONST MI_Char* CIM_InstModification_Description_qual_value = MI_T("CIM_InstModification notifies when an instance is modified.");

static MI_CONST MI_Qualifier CIM_InstModification_Description_qual =
{
    MI_T("Description"),
    MI_STRING,
    MI_FLAG_ENABLEOVERRIDE|MI_FLAG_TOSUBCLASS|MI_FLAG_TRANSLATABLE,
    &CIM_InstModification_Description_qual_value
};

static MI_CONST MI_Char* CIM_InstModification_Version_qual_value = MI_T("2.10.0");

static MI_CONST MI_Qualifier CIM_InstModification_Version_qual =
{
    MI_T("Version"),
    MI_STRING,
    MI_FLAG_ENABLEOVERRIDE|MI_FLAG_TRANSLATABLE|MI_FLAG_RESTRICTED,
    &CIM_InstModification_Version_qual_value
};

static MI_Qualifier MI_CONST* MI_CONST CIM_InstModification_quals[] =
{
    &CIM_InstModification_Indication_qual,
    &CIM_InstModification_UMLPackagePath_qual,
    &CIM_InstModification_Description_qual,
    &CIM_InstModification_Version_qual,
};

/* class CIM_InstModification */
MI_CONST MI_ClassDecl CIM_InstModification_rtti =
{
    MI_FLAG_CLASS|MI_FLAG_INDICATION, /* flags */
    0x00636E14, /* code */
    MI_T("CIM_InstModification"), /* name */
    CIM_InstModification_quals, /* qualifiers */
    MI_COUNT(CIM_InstModification_quals), /* numQualifiers */
    CIM_InstModification_props, /* properties */
    MI_COUNT(CIM_InstModification_props), /* numProperties */
    sizeof(CIM_InstModification), /* size */
    MI_T("CIM_InstIndication"), /* superClass */
    &CIM_InstIndication_rtti, /* superClassDecl */
    NULL, /* methods */
    0, /* numMethods */
    &schemaDecl, /* schema */
    NULL, /* functions */
    NULL /* owningClass */
};

/*
**==============================================================================
**
** MSFT_WindowsServiceStarted
**
**==============================================================================
*/

static MI_PropertyDecl MI_CONST* MI_CONST MSFT_WindowsServiceStarted_props[] =
{
    &CIM_Indication_IndicationIdentifier_prop,
    &CIM_Indication_CorrelatedIndications_prop,
    &CIM_Indication_IndicationTime_prop,
    &CIM_Indication_PerceivedSeverity_prop,
    &CIM_Indication_OtherSeverity_prop,
    &CIM_Indication_IndicationFilterName_prop,
    &CIM_Indication_SequenceContext_prop,
    &CIM_Indication_SequenceNumber_prop,
    &CIM_InstIndication_SourceInstance_prop,
    &CIM_InstIndication_SourceInstanceModelPath_prop,
    &CIM_InstIndication_SourceInstanceHost_prop,
    &CIM_InstModification_PreviousInstance_prop,
};

static MI_CONST MI_ProviderFT MSFT_WindowsServiceStarted_funcs =
{
  (MI_ProviderFT_Load)MSFT_WindowsServiceStarted_Load,
  (MI_ProviderFT_Unload)MSFT_WindowsServiceStarted_Unload,
  (MI_ProviderFT_GetInstance)NULL,
  (MI_ProviderFT_EnumerateInstances)NULL,
  (MI_ProviderFT_CreateInstance)NULL,
  (MI_ProviderFT_ModifyInstance)NULL,
  (MI_ProviderFT_DeleteInstance)NULL,
  (MI_ProviderFT_AssociatorInstances)NULL,
  (MI_ProviderFT_ReferenceInstances)NULL,
  (MI_ProviderFT_EnableIndications)MSFT_WindowsServiceStarted_EnableIndications,
  (MI_ProviderFT_DisableIndications)MSFT_WindowsServiceStarted_DisableIndications,
  (MI_ProviderFT_Subscribe)MSFT_WindowsServiceStarted_Subscribe,
  (MI_ProviderFT_Unsubscribe)MSFT_WindowsServiceStarted_Unsubscribe,
  (MI_ProviderFT_Invoke)NULL,
};

static MI_CONST MI_Boolean MSFT_WindowsServiceStarted_Indication_qual_value = 1;

static MI_CONST MI_Qualifier MSFT_WindowsServiceStarted_Indication_qual =
{
    MI_T("Indication"),
    MI_BOOLEAN,
    MI_FLAG_DISABLEOVERRIDE|MI_FLAG_TOSUBCLASS,
    &MSFT_WindowsServiceStarted_Indication_qual_value
};

static MI_CONST MI_Char* MSFT_WindowsServiceStarted_UMLPackagePath_qual_value = MI_T("CIM::Event");

static MI_CONST MI_Qualifier MSFT_WindowsServiceStarted_UMLPackagePath_qual =
{
    MI_T("UMLPackagePath"),
    MI_STRING,
    MI_FLAG_ENABLEOVERRIDE|MI_FLAG_TOSUBCLASS,
    &MSFT_WindowsServiceStarted_UMLPackagePath_qual_value
};

static MI_CONST MI_Char* MSFT_WindowsServiceStarted_Description_qual_value = MI_T("CIM_InstModification notifies when an instance is modified.");

static MI_CONST MI_Qualifier MSFT_WindowsServiceStarted_Description_qual =
{
    MI_T("Description"),
    MI_STRING,
    MI_FLAG_ENABLEOVERRIDE|MI_FLAG_TOSUBCLASS|MI_FLAG_TRANSLATABLE,
    &MSFT_WindowsServiceStarted_Description_qual_value
};

static MI_CONST MI_Char* MSFT_WindowsServiceStarted_ClassVersion_qual_value = MI_T("1.0.0");

static MI_CONST MI_Qualifier MSFT_WindowsServiceStarted_ClassVersion_qual =
{
    MI_T("ClassVersion"),
    MI_STRING,
    MI_FLAG_ENABLEOVERRIDE|MI_FLAG_RESTRICTED,
    &MSFT_WindowsServiceStarted_ClassVersion_qual_value
};

static MI_Qualifier MI_CONST* MI_CONST MSFT_WindowsServiceStarted_quals[] =
{
    &MSFT_WindowsServiceStarted_Indication_qual,
    &MSFT_WindowsServiceStarted_UMLPackagePath_qual,
    &MSFT_WindowsServiceStarted_Description_qual,
    &MSFT_WindowsServiceStarted_ClassVersion_qual,
};

/* class MSFT_WindowsServiceStarted */
MI_CONST MI_ClassDecl MSFT_WindowsServiceStarted_rtti =
{
    MI_FLAG_CLASS|MI_FLAG_INDICATION, /* flags */
    0x006D641A, /* code */
    MI_T("MSFT_WindowsServiceStarted"), /* name */
    MSFT_WindowsServiceStarted_quals, /* qualifiers */
    MI_COUNT(MSFT_WindowsServiceStarted_quals), /* numQualifiers */
    MSFT_WindowsServiceStarted_props, /* properties */
    MI_COUNT(MSFT_WindowsServiceStarted_props), /* numProperties */
    sizeof(MSFT_WindowsServiceStarted), /* size */
    MI_T("CIM_InstModification"), /* superClass */
    &CIM_InstModification_rtti, /* superClassDecl */
    NULL, /* methods */
    0, /* numMethods */
    &schemaDecl, /* schema */
    &MSFT_WindowsServiceStarted_funcs, /* functions */
    NULL /* owningClass */
};

/*
**==============================================================================
**
** MSFT_WindowsServiceStopped
**
**==============================================================================
*/

static MI_PropertyDecl MI_CONST* MI_CONST MSFT_WindowsServiceStopped_props[] =
{
    &CIM_Indication_IndicationIdentifier_prop,
    &CIM_Indication_CorrelatedIndications_prop,
    &CIM_Indication_IndicationTime_prop,
    &CIM_Indication_PerceivedSeverity_prop,
    &CIM_Indication_OtherSeverity_prop,
    &CIM_Indication_IndicationFilterName_prop,
    &CIM_Indication_SequenceContext_prop,
    &CIM_Indication_SequenceNumber_prop,
    &CIM_InstIndication_SourceInstance_prop,
    &CIM_InstIndication_SourceInstanceModelPath_prop,
    &CIM_InstIndication_SourceInstanceHost_prop,
    &CIM_InstModification_PreviousInstance_prop,
};

static MI_CONST MI_ProviderFT MSFT_WindowsServiceStopped_funcs =
{
  (MI_ProviderFT_Load)MSFT_WindowsServiceStopped_Load,
  (MI_ProviderFT_Unload)MSFT_WindowsServiceStopped_Unload,
  (MI_ProviderFT_GetInstance)NULL,
  (MI_ProviderFT_EnumerateInstances)NULL,
  (MI_ProviderFT_CreateInstance)NULL,
  (MI_ProviderFT_ModifyInstance)NULL,
  (MI_ProviderFT_DeleteInstance)NULL,
  (MI_ProviderFT_AssociatorInstances)NULL,
  (MI_ProviderFT_ReferenceInstances)NULL,
  (MI_ProviderFT_EnableIndications)MSFT_WindowsServiceStopped_EnableIndications,
  (MI_ProviderFT_DisableIndications)MSFT_WindowsServiceStopped_DisableIndications,
  (MI_ProviderFT_Subscribe)MSFT_WindowsServiceStopped_Subscribe,
  (MI_ProviderFT_Unsubscribe)MSFT_WindowsServiceStopped_Unsubscribe,
  (MI_ProviderFT_Invoke)NULL,
};

static MI_CONST MI_Boolean MSFT_WindowsServiceStopped_Indication_qual_value = 1;

static MI_CONST MI_Qualifier MSFT_WindowsServiceStopped_Indication_qual =
{
    MI_T("Indication"),
    MI_BOOLEAN,
    MI_FLAG_DISABLEOVERRIDE|MI_FLAG_TOSUBCLASS,
    &MSFT_WindowsServiceStopped_Indication_qual_value
};

static MI_CONST MI_Char* MSFT_WindowsServiceStopped_UMLPackagePath_qual_value = MI_T("CIM::Event");

static MI_CONST MI_Qualifier MSFT_WindowsServiceStopped_UMLPackagePath_qual =
{
    MI_T("UMLPackagePath"),
    MI_STRING,
    MI_FLAG_ENABLEOVERRIDE|MI_FLAG_TOSUBCLASS,
    &MSFT_WindowsServiceStopped_UMLPackagePath_qual_value
};

static MI_CONST MI_Char* MSFT_WindowsServiceStopped_Description_qual_value = MI_T("CIM_InstModification notifies when an instance is modified.");

static MI_CONST MI_Qualifier MSFT_WindowsServiceStopped_Description_qual =
{
    MI_T("Description"),
    MI_STRING,
    MI_FLAG_ENABLEOVERRIDE|MI_FLAG_TOSUBCLASS|MI_FLAG_TRANSLATABLE,
    &MSFT_WindowsServiceStopped_Description_qual_value
};

static MI_CONST MI_Char* MSFT_WindowsServiceStopped_ClassVersion_qual_value = MI_T("1.0.0");

static MI_CONST MI_Qualifier MSFT_WindowsServiceStopped_ClassVersion_qual =
{
    MI_T("ClassVersion"),
    MI_STRING,
    MI_FLAG_ENABLEOVERRIDE|MI_FLAG_RESTRICTED,
    &MSFT_WindowsServiceStopped_ClassVersion_qual_value
};

static MI_Qualifier MI_CONST* MI_CONST MSFT_WindowsServiceStopped_quals[] =
{
    &MSFT_WindowsServiceStopped_Indication_qual,
    &MSFT_WindowsServiceStopped_UMLPackagePath_qual,
    &MSFT_WindowsServiceStopped_Description_qual,
    &MSFT_WindowsServiceStopped_ClassVersion_qual,
};

/* class MSFT_WindowsServiceStopped */
MI_CONST MI_ClassDecl MSFT_WindowsServiceStopped_rtti =
{
    MI_FLAG_CLASS|MI_FLAG_INDICATION, /* flags */
    0x006D641A, /* code */
    MI_T("MSFT_WindowsServiceStopped"), /* name */
    MSFT_WindowsServiceStopped_quals, /* qualifiers */
    MI_COUNT(MSFT_WindowsServiceStopped_quals), /* numQualifiers */
    MSFT_WindowsServiceStopped_props, /* properties */
    MI_COUNT(MSFT_WindowsServiceStopped_props), /* numProperties */
    sizeof(MSFT_WindowsServiceStopped), /* size */
    MI_T("CIM_InstModification"), /* superClass */
    &CIM_InstModification_rtti, /* superClassDecl */
    NULL, /* methods */
    0, /* numMethods */
    &schemaDecl, /* schema */
    &MSFT_WindowsServiceStopped_funcs, /* functions */
    NULL /* owningClass */
};

/*
**==============================================================================
**
** __mi_server
**
**==============================================================================
*/

MI_Server* __mi_server;
/*
**==============================================================================
**
** Schema
**
**==============================================================================
*/

static MI_ClassDecl MI_CONST* MI_CONST classes[] =
{
    &CIM_Indication_rtti,
    &CIM_InstIndication_rtti,
    &CIM_InstModification_rtti,
    &MSFT_WindowsServiceStarted_rtti,
    &MSFT_WindowsServiceStopped_rtti,
};

MI_SchemaDecl schemaDecl =
{
    qualifierDecls, /* qualifierDecls */
    MI_COUNT(qualifierDecls), /* numQualifierDecls */
    classes, /* classDecls */
    MI_COUNT(classes), /* classDecls */
};

/*
**==============================================================================
**
** MI_Server Methods
**
**==============================================================================
*/

MI_Result MI_CALL MI_Server_GetVersion(
    MI_Uint32* version){
    return __mi_server->serverFT->GetVersion(version);
}

MI_Result MI_CALL MI_Server_GetSystemName(
    const MI_Char** systemName)
{
    return __mi_server->serverFT->GetSystemName(systemName);
}

