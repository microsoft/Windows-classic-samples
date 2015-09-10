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
#include "MSFT_WindowsService.h"
#include "MSFT_WindowsServiceManager.h"
#include "CIM_Error.h"

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
** CIM_ManagedElement
**
**==============================================================================
*/

static MI_CONST MI_Char* CIM_ManagedElement_InstanceID_Description_qual_value = MI_T("1");

static MI_CONST MI_Qualifier CIM_ManagedElement_InstanceID_Description_qual =
{
    MI_T("Description"),
    MI_STRING,
    MI_FLAG_ENABLEOVERRIDE|MI_FLAG_TOSUBCLASS|MI_FLAG_TRANSLATABLE,
    &CIM_ManagedElement_InstanceID_Description_qual_value
};

static MI_Qualifier MI_CONST* MI_CONST CIM_ManagedElement_InstanceID_quals[] =
{
    &CIM_ManagedElement_InstanceID_Description_qual,
};

/* property CIM_ManagedElement.InstanceID */
static MI_CONST MI_PropertyDecl CIM_ManagedElement_InstanceID_prop =
{
    MI_FLAG_PROPERTY|MI_FLAG_READONLY, /* flags */
    0x0069640A, /* code */
    MI_T("InstanceID"), /* name */
    CIM_ManagedElement_InstanceID_quals, /* qualifiers */
    MI_COUNT(CIM_ManagedElement_InstanceID_quals), /* numQualifiers */
    MI_STRING, /* type */
    NULL, /* className */
    0, /* subscript */
    offsetof(CIM_ManagedElement, InstanceID), /* offset */
    MI_T("CIM_ManagedElement"), /* origin */
    MI_T("CIM_ManagedElement"), /* propagator */
    NULL,
};

static MI_CONST MI_Char* CIM_ManagedElement_Caption_Description_qual_value = MI_T("2");

static MI_CONST MI_Qualifier CIM_ManagedElement_Caption_Description_qual =
{
    MI_T("Description"),
    MI_STRING,
    MI_FLAG_ENABLEOVERRIDE|MI_FLAG_TOSUBCLASS|MI_FLAG_TRANSLATABLE,
    &CIM_ManagedElement_Caption_Description_qual_value
};

static MI_CONST MI_Uint32 CIM_ManagedElement_Caption_MaxLen_qual_value = 64U;

static MI_CONST MI_Qualifier CIM_ManagedElement_Caption_MaxLen_qual =
{
    MI_T("MaxLen"),
    MI_UINT32,
    MI_FLAG_ENABLEOVERRIDE|MI_FLAG_TOSUBCLASS,
    &CIM_ManagedElement_Caption_MaxLen_qual_value
};

static MI_Qualifier MI_CONST* MI_CONST CIM_ManagedElement_Caption_quals[] =
{
    &CIM_ManagedElement_Caption_Description_qual,
    &CIM_ManagedElement_Caption_MaxLen_qual,
};

/* property CIM_ManagedElement.Caption */
static MI_CONST MI_PropertyDecl CIM_ManagedElement_Caption_prop =
{
    MI_FLAG_PROPERTY|MI_FLAG_READONLY, /* flags */
    0x00636E07, /* code */
    MI_T("Caption"), /* name */
    CIM_ManagedElement_Caption_quals, /* qualifiers */
    MI_COUNT(CIM_ManagedElement_Caption_quals), /* numQualifiers */
    MI_STRING, /* type */
    NULL, /* className */
    0, /* subscript */
    offsetof(CIM_ManagedElement, Caption), /* offset */
    MI_T("CIM_ManagedElement"), /* origin */
    MI_T("CIM_ManagedElement"), /* propagator */
    NULL,
};

static MI_CONST MI_Char* CIM_ManagedElement_Description_Description_qual_value = MI_T("3");

static MI_CONST MI_Qualifier CIM_ManagedElement_Description_Description_qual =
{
    MI_T("Description"),
    MI_STRING,
    MI_FLAG_ENABLEOVERRIDE|MI_FLAG_TOSUBCLASS|MI_FLAG_TRANSLATABLE,
    &CIM_ManagedElement_Description_Description_qual_value
};

static MI_Qualifier MI_CONST* MI_CONST CIM_ManagedElement_Description_quals[] =
{
    &CIM_ManagedElement_Description_Description_qual,
};

/* property CIM_ManagedElement.Description */
static MI_CONST MI_PropertyDecl CIM_ManagedElement_Description_prop =
{
    MI_FLAG_PROPERTY|MI_FLAG_READONLY, /* flags */
    0x00646E0B, /* code */
    MI_T("Description"), /* name */
    CIM_ManagedElement_Description_quals, /* qualifiers */
    MI_COUNT(CIM_ManagedElement_Description_quals), /* numQualifiers */
    MI_STRING, /* type */
    NULL, /* className */
    0, /* subscript */
    offsetof(CIM_ManagedElement, Description), /* offset */
    MI_T("CIM_ManagedElement"), /* origin */
    MI_T("CIM_ManagedElement"), /* propagator */
    NULL,
};

static MI_CONST MI_Char* CIM_ManagedElement_ElementName_Description_qual_value = MI_T("4");

static MI_CONST MI_Qualifier CIM_ManagedElement_ElementName_Description_qual =
{
    MI_T("Description"),
    MI_STRING,
    MI_FLAG_ENABLEOVERRIDE|MI_FLAG_TOSUBCLASS|MI_FLAG_TRANSLATABLE,
    &CIM_ManagedElement_ElementName_Description_qual_value
};

static MI_Qualifier MI_CONST* MI_CONST CIM_ManagedElement_ElementName_quals[] =
{
    &CIM_ManagedElement_ElementName_Description_qual,
};

/* property CIM_ManagedElement.ElementName */
static MI_CONST MI_PropertyDecl CIM_ManagedElement_ElementName_prop =
{
    MI_FLAG_PROPERTY|MI_FLAG_READONLY, /* flags */
    0x0065650B, /* code */
    MI_T("ElementName"), /* name */
    CIM_ManagedElement_ElementName_quals, /* qualifiers */
    MI_COUNT(CIM_ManagedElement_ElementName_quals), /* numQualifiers */
    MI_STRING, /* type */
    NULL, /* className */
    0, /* subscript */
    offsetof(CIM_ManagedElement, ElementName), /* offset */
    MI_T("CIM_ManagedElement"), /* origin */
    MI_T("CIM_ManagedElement"), /* propagator */
    NULL,
};

static MI_PropertyDecl MI_CONST* MI_CONST CIM_ManagedElement_props[] =
{
    &CIM_ManagedElement_InstanceID_prop,
    &CIM_ManagedElement_Caption_prop,
    &CIM_ManagedElement_Description_prop,
    &CIM_ManagedElement_ElementName_prop,
};

static MI_CONST MI_Boolean CIM_ManagedElement_Abstract_qual_value = 1;

static MI_CONST MI_Qualifier CIM_ManagedElement_Abstract_qual =
{
    MI_T("Abstract"),
    MI_BOOLEAN,
    MI_FLAG_ENABLEOVERRIDE|MI_FLAG_RESTRICTED,
    &CIM_ManagedElement_Abstract_qual_value
};

static MI_CONST MI_Char* CIM_ManagedElement_Version_qual_value = MI_T("5");

static MI_CONST MI_Qualifier CIM_ManagedElement_Version_qual =
{
    MI_T("Version"),
    MI_STRING,
    MI_FLAG_ENABLEOVERRIDE|MI_FLAG_TRANSLATABLE|MI_FLAG_RESTRICTED,
    &CIM_ManagedElement_Version_qual_value
};

static MI_CONST MI_Char* CIM_ManagedElement_UMLPackagePath_qual_value = MI_T("CIM::Core::CoreElements");

static MI_CONST MI_Qualifier CIM_ManagedElement_UMLPackagePath_qual =
{
    MI_T("UMLPackagePath"),
    MI_STRING,
    MI_FLAG_ENABLEOVERRIDE|MI_FLAG_TOSUBCLASS,
    &CIM_ManagedElement_UMLPackagePath_qual_value
};

static MI_CONST MI_Char* CIM_ManagedElement_Description_qual_value = MI_T("6");

static MI_CONST MI_Qualifier CIM_ManagedElement_Description_qual =
{
    MI_T("Description"),
    MI_STRING,
    MI_FLAG_ENABLEOVERRIDE|MI_FLAG_TOSUBCLASS|MI_FLAG_TRANSLATABLE,
    &CIM_ManagedElement_Description_qual_value
};

static MI_Qualifier MI_CONST* MI_CONST CIM_ManagedElement_quals[] =
{
    &CIM_ManagedElement_Abstract_qual,
    &CIM_ManagedElement_Version_qual,
    &CIM_ManagedElement_UMLPackagePath_qual,
    &CIM_ManagedElement_Description_qual,
};

/* class CIM_ManagedElement */
MI_CONST MI_ClassDecl CIM_ManagedElement_rtti =
{
    MI_FLAG_CLASS|MI_FLAG_ABSTRACT, /* flags */
    0x00637412, /* code */
    MI_T("CIM_ManagedElement"), /* name */
    CIM_ManagedElement_quals, /* qualifiers */
    MI_COUNT(CIM_ManagedElement_quals), /* numQualifiers */
    CIM_ManagedElement_props, /* properties */
    MI_COUNT(CIM_ManagedElement_props), /* numProperties */
    sizeof(CIM_ManagedElement), /* size */
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
** CIM_ManagedSystemElement
**
**==============================================================================
*/

static MI_CONST MI_Char* CIM_ManagedSystemElement_InstallDate_Description_qual_value = MI_T("7");

static MI_CONST MI_Qualifier CIM_ManagedSystemElement_InstallDate_Description_qual =
{
    MI_T("Description"),
    MI_STRING,
    MI_FLAG_ENABLEOVERRIDE|MI_FLAG_TOSUBCLASS|MI_FLAG_TRANSLATABLE,
    &CIM_ManagedSystemElement_InstallDate_Description_qual_value
};

static MI_Qualifier MI_CONST* MI_CONST CIM_ManagedSystemElement_InstallDate_quals[] =
{
    &CIM_ManagedSystemElement_InstallDate_Description_qual,
};

/* property CIM_ManagedSystemElement.InstallDate */
static MI_CONST MI_PropertyDecl CIM_ManagedSystemElement_InstallDate_prop =
{
    MI_FLAG_PROPERTY|MI_FLAG_READONLY, /* flags */
    0x0069650B, /* code */
    MI_T("InstallDate"), /* name */
    CIM_ManagedSystemElement_InstallDate_quals, /* qualifiers */
    MI_COUNT(CIM_ManagedSystemElement_InstallDate_quals), /* numQualifiers */
    MI_DATETIME, /* type */
    NULL, /* className */
    0, /* subscript */
    offsetof(CIM_ManagedSystemElement, InstallDate), /* offset */
    MI_T("CIM_ManagedSystemElement"), /* origin */
    MI_T("CIM_ManagedSystemElement"), /* propagator */
    NULL,
};

static MI_CONST MI_Char* CIM_ManagedSystemElement_Name_Description_qual_value = MI_T("8");

static MI_CONST MI_Qualifier CIM_ManagedSystemElement_Name_Description_qual =
{
    MI_T("Description"),
    MI_STRING,
    MI_FLAG_ENABLEOVERRIDE|MI_FLAG_TOSUBCLASS|MI_FLAG_TRANSLATABLE,
    &CIM_ManagedSystemElement_Name_Description_qual_value
};

static MI_CONST MI_Uint32 CIM_ManagedSystemElement_Name_MaxLen_qual_value = 1024U;

static MI_CONST MI_Qualifier CIM_ManagedSystemElement_Name_MaxLen_qual =
{
    MI_T("MaxLen"),
    MI_UINT32,
    MI_FLAG_ENABLEOVERRIDE|MI_FLAG_TOSUBCLASS,
    &CIM_ManagedSystemElement_Name_MaxLen_qual_value
};

static MI_Qualifier MI_CONST* MI_CONST CIM_ManagedSystemElement_Name_quals[] =
{
    &CIM_ManagedSystemElement_Name_Description_qual,
    &CIM_ManagedSystemElement_Name_MaxLen_qual,
};

/* property CIM_ManagedSystemElement.Name */
static MI_CONST MI_PropertyDecl CIM_ManagedSystemElement_Name_prop =
{
    MI_FLAG_PROPERTY|MI_FLAG_READONLY, /* flags */
    0x006E6504, /* code */
    MI_T("Name"), /* name */
    CIM_ManagedSystemElement_Name_quals, /* qualifiers */
    MI_COUNT(CIM_ManagedSystemElement_Name_quals), /* numQualifiers */
    MI_STRING, /* type */
    NULL, /* className */
    0, /* subscript */
    offsetof(CIM_ManagedSystemElement, Name), /* offset */
    MI_T("CIM_ManagedSystemElement"), /* origin */
    MI_T("CIM_ManagedSystemElement"), /* propagator */
    NULL,
};

static MI_CONST MI_Char* CIM_ManagedSystemElement_OperationalStatus_Description_qual_value = MI_T("9");

static MI_CONST MI_Qualifier CIM_ManagedSystemElement_OperationalStatus_Description_qual =
{
    MI_T("Description"),
    MI_STRING,
    MI_FLAG_ENABLEOVERRIDE|MI_FLAG_TOSUBCLASS|MI_FLAG_TRANSLATABLE,
    &CIM_ManagedSystemElement_OperationalStatus_Description_qual_value
};

static MI_CONST MI_Char* CIM_ManagedSystemElement_OperationalStatus_ValueMap_qual_data_value[] =
{
    MI_T("0"),
    MI_T("1"),
    MI_T("2"),
    MI_T("3"),
    MI_T("4"),
    MI_T("5"),
    MI_T("6"),
    MI_T("7"),
    MI_T("8"),
    MI_T("9"),
    MI_T("10"),
    MI_T("11"),
    MI_T("12"),
    MI_T("13"),
    MI_T("14"),
    MI_T("15"),
    MI_T("16"),
    MI_T("17"),
    MI_T("18"),
    MI_T(".."),
    MI_T("0x8000.."),
};

static MI_CONST MI_ConstStringA CIM_ManagedSystemElement_OperationalStatus_ValueMap_qual_value =
{
    CIM_ManagedSystemElement_OperationalStatus_ValueMap_qual_data_value,
    MI_COUNT(CIM_ManagedSystemElement_OperationalStatus_ValueMap_qual_data_value),
};

static MI_CONST MI_Qualifier CIM_ManagedSystemElement_OperationalStatus_ValueMap_qual =
{
    MI_T("ValueMap"),
    MI_STRINGA,
    MI_FLAG_ENABLEOVERRIDE|MI_FLAG_TOSUBCLASS,
    &CIM_ManagedSystemElement_OperationalStatus_ValueMap_qual_value
};

static MI_CONST MI_Char* CIM_ManagedSystemElement_OperationalStatus_Values_qual_data_value[] =
{
    MI_T("10"),
    MI_T("11"),
    MI_T("12"),
    MI_T("13"),
    MI_T("14"),
    MI_T("15"),
    MI_T("16"),
    MI_T("17"),
    MI_T("18"),
    MI_T("19"),
    MI_T("20"),
    MI_T("21"),
    MI_T("22"),
    MI_T("23"),
    MI_T("24"),
    MI_T("25"),
    MI_T("26"),
    MI_T("27"),
    MI_T("28"),
    MI_T("29"),
    MI_T("30"),
};

static MI_CONST MI_ConstStringA CIM_ManagedSystemElement_OperationalStatus_Values_qual_value =
{
    CIM_ManagedSystemElement_OperationalStatus_Values_qual_data_value,
    MI_COUNT(CIM_ManagedSystemElement_OperationalStatus_Values_qual_data_value),
};

static MI_CONST MI_Qualifier CIM_ManagedSystemElement_OperationalStatus_Values_qual =
{
    MI_T("Values"),
    MI_STRINGA,
    MI_FLAG_ENABLEOVERRIDE|MI_FLAG_TOSUBCLASS|MI_FLAG_TRANSLATABLE,
    &CIM_ManagedSystemElement_OperationalStatus_Values_qual_value
};

static MI_CONST MI_Char* CIM_ManagedSystemElement_OperationalStatus_ArrayType_qual_value = MI_T("Indexed");

static MI_CONST MI_Qualifier CIM_ManagedSystemElement_OperationalStatus_ArrayType_qual =
{
    MI_T("ArrayType"),
    MI_STRING,
    MI_FLAG_DISABLEOVERRIDE|MI_FLAG_TOSUBCLASS,
    &CIM_ManagedSystemElement_OperationalStatus_ArrayType_qual_value
};

static MI_CONST MI_Char* CIM_ManagedSystemElement_OperationalStatus_ModelCorrespondence_qual_data_value[] =
{
    MI_T("CIM_ManagedSystemElement.StatusDescriptions"),
};

static MI_CONST MI_ConstStringA CIM_ManagedSystemElement_OperationalStatus_ModelCorrespondence_qual_value =
{
    CIM_ManagedSystemElement_OperationalStatus_ModelCorrespondence_qual_data_value,
    MI_COUNT(CIM_ManagedSystemElement_OperationalStatus_ModelCorrespondence_qual_data_value),
};

static MI_CONST MI_Qualifier CIM_ManagedSystemElement_OperationalStatus_ModelCorrespondence_qual =
{
    MI_T("ModelCorrespondence"),
    MI_STRINGA,
    MI_FLAG_ENABLEOVERRIDE|MI_FLAG_TOSUBCLASS,
    &CIM_ManagedSystemElement_OperationalStatus_ModelCorrespondence_qual_value
};

static MI_Qualifier MI_CONST* MI_CONST CIM_ManagedSystemElement_OperationalStatus_quals[] =
{
    &CIM_ManagedSystemElement_OperationalStatus_Description_qual,
    &CIM_ManagedSystemElement_OperationalStatus_ValueMap_qual,
    &CIM_ManagedSystemElement_OperationalStatus_Values_qual,
    &CIM_ManagedSystemElement_OperationalStatus_ArrayType_qual,
    &CIM_ManagedSystemElement_OperationalStatus_ModelCorrespondence_qual,
};

/* property CIM_ManagedSystemElement.OperationalStatus */
static MI_CONST MI_PropertyDecl CIM_ManagedSystemElement_OperationalStatus_prop =
{
    MI_FLAG_PROPERTY|MI_FLAG_READONLY, /* flags */
    0x006F7311, /* code */
    MI_T("OperationalStatus"), /* name */
    CIM_ManagedSystemElement_OperationalStatus_quals, /* qualifiers */
    MI_COUNT(CIM_ManagedSystemElement_OperationalStatus_quals), /* numQualifiers */
    MI_UINT16A, /* type */
    NULL, /* className */
    0, /* subscript */
    offsetof(CIM_ManagedSystemElement, OperationalStatus), /* offset */
    MI_T("CIM_ManagedSystemElement"), /* origin */
    MI_T("CIM_ManagedSystemElement"), /* propagator */
    NULL,
};

static MI_CONST MI_Char* CIM_ManagedSystemElement_StatusDescriptions_Description_qual_value = MI_T("31");

static MI_CONST MI_Qualifier CIM_ManagedSystemElement_StatusDescriptions_Description_qual =
{
    MI_T("Description"),
    MI_STRING,
    MI_FLAG_ENABLEOVERRIDE|MI_FLAG_TOSUBCLASS|MI_FLAG_TRANSLATABLE,
    &CIM_ManagedSystemElement_StatusDescriptions_Description_qual_value
};

static MI_CONST MI_Char* CIM_ManagedSystemElement_StatusDescriptions_ArrayType_qual_value = MI_T("Indexed");

static MI_CONST MI_Qualifier CIM_ManagedSystemElement_StatusDescriptions_ArrayType_qual =
{
    MI_T("ArrayType"),
    MI_STRING,
    MI_FLAG_DISABLEOVERRIDE|MI_FLAG_TOSUBCLASS,
    &CIM_ManagedSystemElement_StatusDescriptions_ArrayType_qual_value
};

static MI_CONST MI_Char* CIM_ManagedSystemElement_StatusDescriptions_ModelCorrespondence_qual_data_value[] =
{
    MI_T("CIM_ManagedSystemElement.OperationalStatus"),
};

static MI_CONST MI_ConstStringA CIM_ManagedSystemElement_StatusDescriptions_ModelCorrespondence_qual_value =
{
    CIM_ManagedSystemElement_StatusDescriptions_ModelCorrespondence_qual_data_value,
    MI_COUNT(CIM_ManagedSystemElement_StatusDescriptions_ModelCorrespondence_qual_data_value),
};

static MI_CONST MI_Qualifier CIM_ManagedSystemElement_StatusDescriptions_ModelCorrespondence_qual =
{
    MI_T("ModelCorrespondence"),
    MI_STRINGA,
    MI_FLAG_ENABLEOVERRIDE|MI_FLAG_TOSUBCLASS,
    &CIM_ManagedSystemElement_StatusDescriptions_ModelCorrespondence_qual_value
};

static MI_Qualifier MI_CONST* MI_CONST CIM_ManagedSystemElement_StatusDescriptions_quals[] =
{
    &CIM_ManagedSystemElement_StatusDescriptions_Description_qual,
    &CIM_ManagedSystemElement_StatusDescriptions_ArrayType_qual,
    &CIM_ManagedSystemElement_StatusDescriptions_ModelCorrespondence_qual,
};

/* property CIM_ManagedSystemElement.StatusDescriptions */
static MI_CONST MI_PropertyDecl CIM_ManagedSystemElement_StatusDescriptions_prop =
{
    MI_FLAG_PROPERTY|MI_FLAG_READONLY, /* flags */
    0x00737312, /* code */
    MI_T("StatusDescriptions"), /* name */
    CIM_ManagedSystemElement_StatusDescriptions_quals, /* qualifiers */
    MI_COUNT(CIM_ManagedSystemElement_StatusDescriptions_quals), /* numQualifiers */
    MI_STRINGA, /* type */
    NULL, /* className */
    0, /* subscript */
    offsetof(CIM_ManagedSystemElement, StatusDescriptions), /* offset */
    MI_T("CIM_ManagedSystemElement"), /* origin */
    MI_T("CIM_ManagedSystemElement"), /* propagator */
    NULL,
};

static MI_CONST MI_Char* CIM_ManagedSystemElement_Status_Deprecated_qual_data_value[] =
{
    MI_T("CIM_ManagedSystemElement.OperationalStatus"),
};

static MI_CONST MI_ConstStringA CIM_ManagedSystemElement_Status_Deprecated_qual_value =
{
    CIM_ManagedSystemElement_Status_Deprecated_qual_data_value,
    MI_COUNT(CIM_ManagedSystemElement_Status_Deprecated_qual_data_value),
};

static MI_CONST MI_Qualifier CIM_ManagedSystemElement_Status_Deprecated_qual =
{
    MI_T("Deprecated"),
    MI_STRINGA,
    MI_FLAG_ENABLEOVERRIDE|MI_FLAG_RESTRICTED,
    &CIM_ManagedSystemElement_Status_Deprecated_qual_value
};

static MI_CONST MI_Char* CIM_ManagedSystemElement_Status_Description_qual_value = MI_T("32");

static MI_CONST MI_Qualifier CIM_ManagedSystemElement_Status_Description_qual =
{
    MI_T("Description"),
    MI_STRING,
    MI_FLAG_ENABLEOVERRIDE|MI_FLAG_TOSUBCLASS|MI_FLAG_TRANSLATABLE,
    &CIM_ManagedSystemElement_Status_Description_qual_value
};

static MI_CONST MI_Char* CIM_ManagedSystemElement_Status_ValueMap_qual_data_value[] =
{
    MI_T("OK"),
    MI_T("Error"),
    MI_T("Degraded"),
    MI_T("Unknown"),
    MI_T("Pred Fail"),
    MI_T("Starting"),
    MI_T("Stopping"),
    MI_T("Service"),
    MI_T("Stressed"),
    MI_T("NonRecover"),
    MI_T("No Contact"),
    MI_T("Lost Comm"),
    MI_T("Stopped"),
};

static MI_CONST MI_ConstStringA CIM_ManagedSystemElement_Status_ValueMap_qual_value =
{
    CIM_ManagedSystemElement_Status_ValueMap_qual_data_value,
    MI_COUNT(CIM_ManagedSystemElement_Status_ValueMap_qual_data_value),
};

static MI_CONST MI_Qualifier CIM_ManagedSystemElement_Status_ValueMap_qual =
{
    MI_T("ValueMap"),
    MI_STRINGA,
    MI_FLAG_ENABLEOVERRIDE|MI_FLAG_TOSUBCLASS,
    &CIM_ManagedSystemElement_Status_ValueMap_qual_value
};

static MI_CONST MI_Uint32 CIM_ManagedSystemElement_Status_MaxLen_qual_value = 10U;

static MI_CONST MI_Qualifier CIM_ManagedSystemElement_Status_MaxLen_qual =
{
    MI_T("MaxLen"),
    MI_UINT32,
    MI_FLAG_ENABLEOVERRIDE|MI_FLAG_TOSUBCLASS,
    &CIM_ManagedSystemElement_Status_MaxLen_qual_value
};

static MI_Qualifier MI_CONST* MI_CONST CIM_ManagedSystemElement_Status_quals[] =
{
    &CIM_ManagedSystemElement_Status_Deprecated_qual,
    &CIM_ManagedSystemElement_Status_Description_qual,
    &CIM_ManagedSystemElement_Status_ValueMap_qual,
    &CIM_ManagedSystemElement_Status_MaxLen_qual,
};

/* property CIM_ManagedSystemElement.Status */
static MI_CONST MI_PropertyDecl CIM_ManagedSystemElement_Status_prop =
{
    MI_FLAG_PROPERTY|MI_FLAG_READONLY, /* flags */
    0x00737306, /* code */
    MI_T("Status"), /* name */
    CIM_ManagedSystemElement_Status_quals, /* qualifiers */
    MI_COUNT(CIM_ManagedSystemElement_Status_quals), /* numQualifiers */
    MI_STRING, /* type */
    NULL, /* className */
    0, /* subscript */
    offsetof(CIM_ManagedSystemElement, Status), /* offset */
    MI_T("CIM_ManagedSystemElement"), /* origin */
    MI_T("CIM_ManagedSystemElement"), /* propagator */
    NULL,
};

static MI_CONST MI_Char* CIM_ManagedSystemElement_HealthState_Description_qual_value = MI_T("33");

static MI_CONST MI_Qualifier CIM_ManagedSystemElement_HealthState_Description_qual =
{
    MI_T("Description"),
    MI_STRING,
    MI_FLAG_ENABLEOVERRIDE|MI_FLAG_TOSUBCLASS|MI_FLAG_TRANSLATABLE,
    &CIM_ManagedSystemElement_HealthState_Description_qual_value
};

static MI_CONST MI_Char* CIM_ManagedSystemElement_HealthState_ValueMap_qual_data_value[] =
{
    MI_T("0"),
    MI_T("5"),
    MI_T("10"),
    MI_T("15"),
    MI_T("20"),
    MI_T("25"),
    MI_T("30"),
    MI_T(".."),
};

static MI_CONST MI_ConstStringA CIM_ManagedSystemElement_HealthState_ValueMap_qual_value =
{
    CIM_ManagedSystemElement_HealthState_ValueMap_qual_data_value,
    MI_COUNT(CIM_ManagedSystemElement_HealthState_ValueMap_qual_data_value),
};

static MI_CONST MI_Qualifier CIM_ManagedSystemElement_HealthState_ValueMap_qual =
{
    MI_T("ValueMap"),
    MI_STRINGA,
    MI_FLAG_ENABLEOVERRIDE|MI_FLAG_TOSUBCLASS,
    &CIM_ManagedSystemElement_HealthState_ValueMap_qual_value
};

static MI_CONST MI_Char* CIM_ManagedSystemElement_HealthState_Values_qual_data_value[] =
{
    MI_T("10"),
    MI_T("12"),
    MI_T("34"),
    MI_T("35"),
    MI_T("36"),
    MI_T("37"),
    MI_T("38"),
    MI_T("29"),
};

static MI_CONST MI_ConstStringA CIM_ManagedSystemElement_HealthState_Values_qual_value =
{
    CIM_ManagedSystemElement_HealthState_Values_qual_data_value,
    MI_COUNT(CIM_ManagedSystemElement_HealthState_Values_qual_data_value),
};

static MI_CONST MI_Qualifier CIM_ManagedSystemElement_HealthState_Values_qual =
{
    MI_T("Values"),
    MI_STRINGA,
    MI_FLAG_ENABLEOVERRIDE|MI_FLAG_TOSUBCLASS|MI_FLAG_TRANSLATABLE,
    &CIM_ManagedSystemElement_HealthState_Values_qual_value
};

static MI_Qualifier MI_CONST* MI_CONST CIM_ManagedSystemElement_HealthState_quals[] =
{
    &CIM_ManagedSystemElement_HealthState_Description_qual,
    &CIM_ManagedSystemElement_HealthState_ValueMap_qual,
    &CIM_ManagedSystemElement_HealthState_Values_qual,
};

/* property CIM_ManagedSystemElement.HealthState */
static MI_CONST MI_PropertyDecl CIM_ManagedSystemElement_HealthState_prop =
{
    MI_FLAG_PROPERTY|MI_FLAG_READONLY, /* flags */
    0x0068650B, /* code */
    MI_T("HealthState"), /* name */
    CIM_ManagedSystemElement_HealthState_quals, /* qualifiers */
    MI_COUNT(CIM_ManagedSystemElement_HealthState_quals), /* numQualifiers */
    MI_UINT16, /* type */
    NULL, /* className */
    0, /* subscript */
    offsetof(CIM_ManagedSystemElement, HealthState), /* offset */
    MI_T("CIM_ManagedSystemElement"), /* origin */
    MI_T("CIM_ManagedSystemElement"), /* propagator */
    NULL,
};

static MI_CONST MI_Char* CIM_ManagedSystemElement_CommunicationStatus_Description_qual_value = MI_T("39");

static MI_CONST MI_Qualifier CIM_ManagedSystemElement_CommunicationStatus_Description_qual =
{
    MI_T("Description"),
    MI_STRING,
    MI_FLAG_ENABLEOVERRIDE|MI_FLAG_TOSUBCLASS|MI_FLAG_TRANSLATABLE,
    &CIM_ManagedSystemElement_CommunicationStatus_Description_qual_value
};

static MI_CONST MI_Char* CIM_ManagedSystemElement_CommunicationStatus_ValueMap_qual_data_value[] =
{
    MI_T("0"),
    MI_T("1"),
    MI_T("2"),
    MI_T("3"),
    MI_T("4"),
    MI_T(".."),
    MI_T("0x8000.."),
};

static MI_CONST MI_ConstStringA CIM_ManagedSystemElement_CommunicationStatus_ValueMap_qual_value =
{
    CIM_ManagedSystemElement_CommunicationStatus_ValueMap_qual_data_value,
    MI_COUNT(CIM_ManagedSystemElement_CommunicationStatus_ValueMap_qual_data_value),
};

static MI_CONST MI_Qualifier CIM_ManagedSystemElement_CommunicationStatus_ValueMap_qual =
{
    MI_T("ValueMap"),
    MI_STRINGA,
    MI_FLAG_ENABLEOVERRIDE|MI_FLAG_TOSUBCLASS,
    &CIM_ManagedSystemElement_CommunicationStatus_ValueMap_qual_value
};

static MI_CONST MI_Char* CIM_ManagedSystemElement_CommunicationStatus_Values_qual_data_value[] =
{
    MI_T("10"),
    MI_T("40"),
    MI_T("41"),
    MI_T("23"),
    MI_T("22"),
    MI_T("29"),
    MI_T("30"),
};

static MI_CONST MI_ConstStringA CIM_ManagedSystemElement_CommunicationStatus_Values_qual_value =
{
    CIM_ManagedSystemElement_CommunicationStatus_Values_qual_data_value,
    MI_COUNT(CIM_ManagedSystemElement_CommunicationStatus_Values_qual_data_value),
};

static MI_CONST MI_Qualifier CIM_ManagedSystemElement_CommunicationStatus_Values_qual =
{
    MI_T("Values"),
    MI_STRINGA,
    MI_FLAG_ENABLEOVERRIDE|MI_FLAG_TOSUBCLASS|MI_FLAG_TRANSLATABLE,
    &CIM_ManagedSystemElement_CommunicationStatus_Values_qual_value
};

static MI_Qualifier MI_CONST* MI_CONST CIM_ManagedSystemElement_CommunicationStatus_quals[] =
{
    &CIM_ManagedSystemElement_CommunicationStatus_Description_qual,
    &CIM_ManagedSystemElement_CommunicationStatus_ValueMap_qual,
    &CIM_ManagedSystemElement_CommunicationStatus_Values_qual,
};

/* property CIM_ManagedSystemElement.CommunicationStatus */
static MI_CONST MI_PropertyDecl CIM_ManagedSystemElement_CommunicationStatus_prop =
{
    MI_FLAG_PROPERTY|MI_FLAG_READONLY, /* flags */
    0x00637313, /* code */
    MI_T("CommunicationStatus"), /* name */
    CIM_ManagedSystemElement_CommunicationStatus_quals, /* qualifiers */
    MI_COUNT(CIM_ManagedSystemElement_CommunicationStatus_quals), /* numQualifiers */
    MI_UINT16, /* type */
    NULL, /* className */
    0, /* subscript */
    offsetof(CIM_ManagedSystemElement, CommunicationStatus), /* offset */
    MI_T("CIM_ManagedSystemElement"), /* origin */
    MI_T("CIM_ManagedSystemElement"), /* propagator */
    NULL,
};

static MI_CONST MI_Char* CIM_ManagedSystemElement_DetailedStatus_Description_qual_value = MI_T("42");

static MI_CONST MI_Qualifier CIM_ManagedSystemElement_DetailedStatus_Description_qual =
{
    MI_T("Description"),
    MI_STRING,
    MI_FLAG_ENABLEOVERRIDE|MI_FLAG_TOSUBCLASS|MI_FLAG_TRANSLATABLE,
    &CIM_ManagedSystemElement_DetailedStatus_Description_qual_value
};

static MI_CONST MI_Char* CIM_ManagedSystemElement_DetailedStatus_ValueMap_qual_data_value[] =
{
    MI_T("0"),
    MI_T("1"),
    MI_T("2"),
    MI_T("3"),
    MI_T("4"),
    MI_T("5"),
    MI_T(".."),
    MI_T("0x8000.."),
};

static MI_CONST MI_ConstStringA CIM_ManagedSystemElement_DetailedStatus_ValueMap_qual_value =
{
    CIM_ManagedSystemElement_DetailedStatus_ValueMap_qual_data_value,
    MI_COUNT(CIM_ManagedSystemElement_DetailedStatus_ValueMap_qual_data_value),
};

static MI_CONST MI_Qualifier CIM_ManagedSystemElement_DetailedStatus_ValueMap_qual =
{
    MI_T("ValueMap"),
    MI_STRINGA,
    MI_FLAG_ENABLEOVERRIDE|MI_FLAG_TOSUBCLASS,
    &CIM_ManagedSystemElement_DetailedStatus_ValueMap_qual_value
};

static MI_CONST MI_Char* CIM_ManagedSystemElement_DetailedStatus_Values_qual_data_value[] =
{
    MI_T("40"),
    MI_T("43"),
    MI_T("14"),
    MI_T("15"),
    MI_T("17"),
    MI_T("26"),
    MI_T("29"),
    MI_T("30"),
};

static MI_CONST MI_ConstStringA CIM_ManagedSystemElement_DetailedStatus_Values_qual_value =
{
    CIM_ManagedSystemElement_DetailedStatus_Values_qual_data_value,
    MI_COUNT(CIM_ManagedSystemElement_DetailedStatus_Values_qual_data_value),
};

static MI_CONST MI_Qualifier CIM_ManagedSystemElement_DetailedStatus_Values_qual =
{
    MI_T("Values"),
    MI_STRINGA,
    MI_FLAG_ENABLEOVERRIDE|MI_FLAG_TOSUBCLASS|MI_FLAG_TRANSLATABLE,
    &CIM_ManagedSystemElement_DetailedStatus_Values_qual_value
};

static MI_CONST MI_Char* CIM_ManagedSystemElement_DetailedStatus_ModelCorrespondence_qual_data_value[] =
{
    MI_T("CIM_EnabledLogicalElement.PrimaryStatus"),
    MI_T("CIM_ManagedSystemElement.HealthState"),
};

static MI_CONST MI_ConstStringA CIM_ManagedSystemElement_DetailedStatus_ModelCorrespondence_qual_value =
{
    CIM_ManagedSystemElement_DetailedStatus_ModelCorrespondence_qual_data_value,
    MI_COUNT(CIM_ManagedSystemElement_DetailedStatus_ModelCorrespondence_qual_data_value),
};

static MI_CONST MI_Qualifier CIM_ManagedSystemElement_DetailedStatus_ModelCorrespondence_qual =
{
    MI_T("ModelCorrespondence"),
    MI_STRINGA,
    MI_FLAG_ENABLEOVERRIDE|MI_FLAG_TOSUBCLASS,
    &CIM_ManagedSystemElement_DetailedStatus_ModelCorrespondence_qual_value
};

static MI_Qualifier MI_CONST* MI_CONST CIM_ManagedSystemElement_DetailedStatus_quals[] =
{
    &CIM_ManagedSystemElement_DetailedStatus_Description_qual,
    &CIM_ManagedSystemElement_DetailedStatus_ValueMap_qual,
    &CIM_ManagedSystemElement_DetailedStatus_Values_qual,
    &CIM_ManagedSystemElement_DetailedStatus_ModelCorrespondence_qual,
};

/* property CIM_ManagedSystemElement.DetailedStatus */
static MI_CONST MI_PropertyDecl CIM_ManagedSystemElement_DetailedStatus_prop =
{
    MI_FLAG_PROPERTY|MI_FLAG_READONLY, /* flags */
    0x0064730E, /* code */
    MI_T("DetailedStatus"), /* name */
    CIM_ManagedSystemElement_DetailedStatus_quals, /* qualifiers */
    MI_COUNT(CIM_ManagedSystemElement_DetailedStatus_quals), /* numQualifiers */
    MI_UINT16, /* type */
    NULL, /* className */
    0, /* subscript */
    offsetof(CIM_ManagedSystemElement, DetailedStatus), /* offset */
    MI_T("CIM_ManagedSystemElement"), /* origin */
    MI_T("CIM_ManagedSystemElement"), /* propagator */
    NULL,
};

static MI_CONST MI_Char* CIM_ManagedSystemElement_OperatingStatus_Description_qual_value = MI_T("44");

static MI_CONST MI_Qualifier CIM_ManagedSystemElement_OperatingStatus_Description_qual =
{
    MI_T("Description"),
    MI_STRING,
    MI_FLAG_ENABLEOVERRIDE|MI_FLAG_TOSUBCLASS|MI_FLAG_TRANSLATABLE,
    &CIM_ManagedSystemElement_OperatingStatus_Description_qual_value
};

static MI_CONST MI_Char* CIM_ManagedSystemElement_OperatingStatus_ValueMap_qual_data_value[] =
{
    MI_T("0"),
    MI_T("1"),
    MI_T("2"),
    MI_T("3"),
    MI_T("4"),
    MI_T("5"),
    MI_T("6"),
    MI_T("7"),
    MI_T("8"),
    MI_T("9"),
    MI_T("10"),
    MI_T("11"),
    MI_T("12"),
    MI_T("13"),
    MI_T("14"),
    MI_T("15"),
    MI_T("16"),
    MI_T(".."),
    MI_T("0x8000.."),
};

static MI_CONST MI_ConstStringA CIM_ManagedSystemElement_OperatingStatus_ValueMap_qual_value =
{
    CIM_ManagedSystemElement_OperatingStatus_ValueMap_qual_data_value,
    MI_COUNT(CIM_ManagedSystemElement_OperatingStatus_ValueMap_qual_data_value),
};

static MI_CONST MI_Qualifier CIM_ManagedSystemElement_OperatingStatus_ValueMap_qual =
{
    MI_T("ValueMap"),
    MI_STRINGA,
    MI_FLAG_ENABLEOVERRIDE|MI_FLAG_TOSUBCLASS,
    &CIM_ManagedSystemElement_OperatingStatus_ValueMap_qual_value
};

static MI_CONST MI_Char* CIM_ManagedSystemElement_OperatingStatus_Values_qual_data_value[] =
{
    MI_T("10"),
    MI_T("40"),
    MI_T("45"),
    MI_T("18"),
    MI_T("19"),
    MI_T("20"),
    MI_T("24"),
    MI_T("25"),
    MI_T("27"),
    MI_T("46"),
    MI_T("47"),
    MI_T("48"),
    MI_T("49"),
    MI_T("50"),
    MI_T("51"),
    MI_T("52"),
    MI_T("21"),
    MI_T("29"),
    MI_T("30"),
};

static MI_CONST MI_ConstStringA CIM_ManagedSystemElement_OperatingStatus_Values_qual_value =
{
    CIM_ManagedSystemElement_OperatingStatus_Values_qual_data_value,
    MI_COUNT(CIM_ManagedSystemElement_OperatingStatus_Values_qual_data_value),
};

static MI_CONST MI_Qualifier CIM_ManagedSystemElement_OperatingStatus_Values_qual =
{
    MI_T("Values"),
    MI_STRINGA,
    MI_FLAG_ENABLEOVERRIDE|MI_FLAG_TOSUBCLASS|MI_FLAG_TRANSLATABLE,
    &CIM_ManagedSystemElement_OperatingStatus_Values_qual_value
};

static MI_CONST MI_Char* CIM_ManagedSystemElement_OperatingStatus_ModelCorrespondence_qual_data_value[] =
{
    MI_T("CIM_EnabledLogicalElement.EnabledState"),
};

static MI_CONST MI_ConstStringA CIM_ManagedSystemElement_OperatingStatus_ModelCorrespondence_qual_value =
{
    CIM_ManagedSystemElement_OperatingStatus_ModelCorrespondence_qual_data_value,
    MI_COUNT(CIM_ManagedSystemElement_OperatingStatus_ModelCorrespondence_qual_data_value),
};

static MI_CONST MI_Qualifier CIM_ManagedSystemElement_OperatingStatus_ModelCorrespondence_qual =
{
    MI_T("ModelCorrespondence"),
    MI_STRINGA,
    MI_FLAG_ENABLEOVERRIDE|MI_FLAG_TOSUBCLASS,
    &CIM_ManagedSystemElement_OperatingStatus_ModelCorrespondence_qual_value
};

static MI_Qualifier MI_CONST* MI_CONST CIM_ManagedSystemElement_OperatingStatus_quals[] =
{
    &CIM_ManagedSystemElement_OperatingStatus_Description_qual,
    &CIM_ManagedSystemElement_OperatingStatus_ValueMap_qual,
    &CIM_ManagedSystemElement_OperatingStatus_Values_qual,
    &CIM_ManagedSystemElement_OperatingStatus_ModelCorrespondence_qual,
};

/* property CIM_ManagedSystemElement.OperatingStatus */
static MI_CONST MI_PropertyDecl CIM_ManagedSystemElement_OperatingStatus_prop =
{
    MI_FLAG_PROPERTY|MI_FLAG_READONLY, /* flags */
    0x006F730F, /* code */
    MI_T("OperatingStatus"), /* name */
    CIM_ManagedSystemElement_OperatingStatus_quals, /* qualifiers */
    MI_COUNT(CIM_ManagedSystemElement_OperatingStatus_quals), /* numQualifiers */
    MI_UINT16, /* type */
    NULL, /* className */
    0, /* subscript */
    offsetof(CIM_ManagedSystemElement, OperatingStatus), /* offset */
    MI_T("CIM_ManagedSystemElement"), /* origin */
    MI_T("CIM_ManagedSystemElement"), /* propagator */
    NULL,
};

static MI_CONST MI_Char* CIM_ManagedSystemElement_PrimaryStatus_Description_qual_value = MI_T("53");

static MI_CONST MI_Qualifier CIM_ManagedSystemElement_PrimaryStatus_Description_qual =
{
    MI_T("Description"),
    MI_STRING,
    MI_FLAG_ENABLEOVERRIDE|MI_FLAG_TOSUBCLASS|MI_FLAG_TRANSLATABLE,
    &CIM_ManagedSystemElement_PrimaryStatus_Description_qual_value
};

static MI_CONST MI_Char* CIM_ManagedSystemElement_PrimaryStatus_ValueMap_qual_data_value[] =
{
    MI_T("0"),
    MI_T("1"),
    MI_T("2"),
    MI_T("3"),
    MI_T(".."),
    MI_T("0x8000.."),
};

static MI_CONST MI_ConstStringA CIM_ManagedSystemElement_PrimaryStatus_ValueMap_qual_value =
{
    CIM_ManagedSystemElement_PrimaryStatus_ValueMap_qual_data_value,
    MI_COUNT(CIM_ManagedSystemElement_PrimaryStatus_ValueMap_qual_data_value),
};

static MI_CONST MI_Qualifier CIM_ManagedSystemElement_PrimaryStatus_ValueMap_qual =
{
    MI_T("ValueMap"),
    MI_STRINGA,
    MI_FLAG_ENABLEOVERRIDE|MI_FLAG_TOSUBCLASS,
    &CIM_ManagedSystemElement_PrimaryStatus_ValueMap_qual_value
};

static MI_CONST MI_Char* CIM_ManagedSystemElement_PrimaryStatus_Values_qual_data_value[] =
{
    MI_T("10"),
    MI_T("12"),
    MI_T("13"),
    MI_T("16"),
    MI_T("29"),
    MI_T("30"),
};

static MI_CONST MI_ConstStringA CIM_ManagedSystemElement_PrimaryStatus_Values_qual_value =
{
    CIM_ManagedSystemElement_PrimaryStatus_Values_qual_data_value,
    MI_COUNT(CIM_ManagedSystemElement_PrimaryStatus_Values_qual_data_value),
};

static MI_CONST MI_Qualifier CIM_ManagedSystemElement_PrimaryStatus_Values_qual =
{
    MI_T("Values"),
    MI_STRINGA,
    MI_FLAG_ENABLEOVERRIDE|MI_FLAG_TOSUBCLASS|MI_FLAG_TRANSLATABLE,
    &CIM_ManagedSystemElement_PrimaryStatus_Values_qual_value
};

static MI_CONST MI_Char* CIM_ManagedSystemElement_PrimaryStatus_ModelCorrespondence_qual_data_value[] =
{
    MI_T("CIM_ManagedSystemElement.DetailedStatus"),
    MI_T("CIM_ManagedSystemElement.HealthState"),
};

static MI_CONST MI_ConstStringA CIM_ManagedSystemElement_PrimaryStatus_ModelCorrespondence_qual_value =
{
    CIM_ManagedSystemElement_PrimaryStatus_ModelCorrespondence_qual_data_value,
    MI_COUNT(CIM_ManagedSystemElement_PrimaryStatus_ModelCorrespondence_qual_data_value),
};

static MI_CONST MI_Qualifier CIM_ManagedSystemElement_PrimaryStatus_ModelCorrespondence_qual =
{
    MI_T("ModelCorrespondence"),
    MI_STRINGA,
    MI_FLAG_ENABLEOVERRIDE|MI_FLAG_TOSUBCLASS,
    &CIM_ManagedSystemElement_PrimaryStatus_ModelCorrespondence_qual_value
};

static MI_Qualifier MI_CONST* MI_CONST CIM_ManagedSystemElement_PrimaryStatus_quals[] =
{
    &CIM_ManagedSystemElement_PrimaryStatus_Description_qual,
    &CIM_ManagedSystemElement_PrimaryStatus_ValueMap_qual,
    &CIM_ManagedSystemElement_PrimaryStatus_Values_qual,
    &CIM_ManagedSystemElement_PrimaryStatus_ModelCorrespondence_qual,
};

/* property CIM_ManagedSystemElement.PrimaryStatus */
static MI_CONST MI_PropertyDecl CIM_ManagedSystemElement_PrimaryStatus_prop =
{
    MI_FLAG_PROPERTY|MI_FLAG_READONLY, /* flags */
    0x0070730D, /* code */
    MI_T("PrimaryStatus"), /* name */
    CIM_ManagedSystemElement_PrimaryStatus_quals, /* qualifiers */
    MI_COUNT(CIM_ManagedSystemElement_PrimaryStatus_quals), /* numQualifiers */
    MI_UINT16, /* type */
    NULL, /* className */
    0, /* subscript */
    offsetof(CIM_ManagedSystemElement, PrimaryStatus), /* offset */
    MI_T("CIM_ManagedSystemElement"), /* origin */
    MI_T("CIM_ManagedSystemElement"), /* propagator */
    NULL,
};

static MI_PropertyDecl MI_CONST* MI_CONST CIM_ManagedSystemElement_props[] =
{
    &CIM_ManagedElement_InstanceID_prop,
    &CIM_ManagedElement_Caption_prop,
    &CIM_ManagedElement_Description_prop,
    &CIM_ManagedElement_ElementName_prop,
    &CIM_ManagedSystemElement_InstallDate_prop,
    &CIM_ManagedSystemElement_Name_prop,
    &CIM_ManagedSystemElement_OperationalStatus_prop,
    &CIM_ManagedSystemElement_StatusDescriptions_prop,
    &CIM_ManagedSystemElement_Status_prop,
    &CIM_ManagedSystemElement_HealthState_prop,
    &CIM_ManagedSystemElement_CommunicationStatus_prop,
    &CIM_ManagedSystemElement_DetailedStatus_prop,
    &CIM_ManagedSystemElement_OperatingStatus_prop,
    &CIM_ManagedSystemElement_PrimaryStatus_prop,
};

static MI_CONST MI_Char* CIM_ManagedSystemElement_UMLPackagePath_qual_value = MI_T("CIM::Core::CoreElements");

static MI_CONST MI_Qualifier CIM_ManagedSystemElement_UMLPackagePath_qual =
{
    MI_T("UMLPackagePath"),
    MI_STRING,
    MI_FLAG_ENABLEOVERRIDE|MI_FLAG_TOSUBCLASS,
    &CIM_ManagedSystemElement_UMLPackagePath_qual_value
};

static MI_CONST MI_Char* CIM_ManagedSystemElement_Description_qual_value = MI_T("54");

static MI_CONST MI_Qualifier CIM_ManagedSystemElement_Description_qual =
{
    MI_T("Description"),
    MI_STRING,
    MI_FLAG_ENABLEOVERRIDE|MI_FLAG_TOSUBCLASS|MI_FLAG_TRANSLATABLE,
    &CIM_ManagedSystemElement_Description_qual_value
};

static MI_CONST MI_Boolean CIM_ManagedSystemElement_Abstract_qual_value = 1;

static MI_CONST MI_Qualifier CIM_ManagedSystemElement_Abstract_qual =
{
    MI_T("Abstract"),
    MI_BOOLEAN,
    MI_FLAG_ENABLEOVERRIDE|MI_FLAG_RESTRICTED,
    &CIM_ManagedSystemElement_Abstract_qual_value
};

static MI_CONST MI_Char* CIM_ManagedSystemElement_Version_qual_value = MI_T("55");

static MI_CONST MI_Qualifier CIM_ManagedSystemElement_Version_qual =
{
    MI_T("Version"),
    MI_STRING,
    MI_FLAG_ENABLEOVERRIDE|MI_FLAG_TRANSLATABLE|MI_FLAG_RESTRICTED,
    &CIM_ManagedSystemElement_Version_qual_value
};

static MI_Qualifier MI_CONST* MI_CONST CIM_ManagedSystemElement_quals[] =
{
    &CIM_ManagedSystemElement_UMLPackagePath_qual,
    &CIM_ManagedSystemElement_Description_qual,
    &CIM_ManagedSystemElement_Abstract_qual,
    &CIM_ManagedSystemElement_Version_qual,
};

/* class CIM_ManagedSystemElement */
MI_CONST MI_ClassDecl CIM_ManagedSystemElement_rtti =
{
    MI_FLAG_CLASS|MI_FLAG_ABSTRACT, /* flags */
    0x00637418, /* code */
    MI_T("CIM_ManagedSystemElement"), /* name */
    CIM_ManagedSystemElement_quals, /* qualifiers */
    MI_COUNT(CIM_ManagedSystemElement_quals), /* numQualifiers */
    CIM_ManagedSystemElement_props, /* properties */
    MI_COUNT(CIM_ManagedSystemElement_props), /* numProperties */
    sizeof(CIM_ManagedSystemElement), /* size */
    MI_T("CIM_ManagedElement"), /* superClass */
    &CIM_ManagedElement_rtti, /* superClassDecl */
    NULL, /* methods */
    0, /* numMethods */
    &schemaDecl, /* schema */
    NULL, /* functions */
    NULL /* owningClass */
};

/*
**==============================================================================
**
** CIM_LogicalElement
**
**==============================================================================
*/

static MI_PropertyDecl MI_CONST* MI_CONST CIM_LogicalElement_props[] =
{
    &CIM_ManagedElement_InstanceID_prop,
    &CIM_ManagedElement_Caption_prop,
    &CIM_ManagedElement_Description_prop,
    &CIM_ManagedElement_ElementName_prop,
    &CIM_ManagedSystemElement_InstallDate_prop,
    &CIM_ManagedSystemElement_Name_prop,
    &CIM_ManagedSystemElement_OperationalStatus_prop,
    &CIM_ManagedSystemElement_StatusDescriptions_prop,
    &CIM_ManagedSystemElement_Status_prop,
    &CIM_ManagedSystemElement_HealthState_prop,
    &CIM_ManagedSystemElement_CommunicationStatus_prop,
    &CIM_ManagedSystemElement_DetailedStatus_prop,
    &CIM_ManagedSystemElement_OperatingStatus_prop,
    &CIM_ManagedSystemElement_PrimaryStatus_prop,
};

static MI_CONST MI_Char* CIM_LogicalElement_UMLPackagePath_qual_value = MI_T("CIM::Core::CoreElements");

static MI_CONST MI_Qualifier CIM_LogicalElement_UMLPackagePath_qual =
{
    MI_T("UMLPackagePath"),
    MI_STRING,
    MI_FLAG_ENABLEOVERRIDE|MI_FLAG_TOSUBCLASS,
    &CIM_LogicalElement_UMLPackagePath_qual_value
};

static MI_CONST MI_Char* CIM_LogicalElement_Description_qual_value = MI_T("56");

static MI_CONST MI_Qualifier CIM_LogicalElement_Description_qual =
{
    MI_T("Description"),
    MI_STRING,
    MI_FLAG_ENABLEOVERRIDE|MI_FLAG_TOSUBCLASS|MI_FLAG_TRANSLATABLE,
    &CIM_LogicalElement_Description_qual_value
};

static MI_CONST MI_Boolean CIM_LogicalElement_Abstract_qual_value = 1;

static MI_CONST MI_Qualifier CIM_LogicalElement_Abstract_qual =
{
    MI_T("Abstract"),
    MI_BOOLEAN,
    MI_FLAG_ENABLEOVERRIDE|MI_FLAG_RESTRICTED,
    &CIM_LogicalElement_Abstract_qual_value
};

static MI_CONST MI_Char* CIM_LogicalElement_Version_qual_value = MI_T("57");

static MI_CONST MI_Qualifier CIM_LogicalElement_Version_qual =
{
    MI_T("Version"),
    MI_STRING,
    MI_FLAG_ENABLEOVERRIDE|MI_FLAG_TRANSLATABLE|MI_FLAG_RESTRICTED,
    &CIM_LogicalElement_Version_qual_value
};

static MI_Qualifier MI_CONST* MI_CONST CIM_LogicalElement_quals[] =
{
    &CIM_LogicalElement_UMLPackagePath_qual,
    &CIM_LogicalElement_Description_qual,
    &CIM_LogicalElement_Abstract_qual,
    &CIM_LogicalElement_Version_qual,
};

/* class CIM_LogicalElement */
MI_CONST MI_ClassDecl CIM_LogicalElement_rtti =
{
    MI_FLAG_CLASS|MI_FLAG_ABSTRACT, /* flags */
    0x00637412, /* code */
    MI_T("CIM_LogicalElement"), /* name */
    CIM_LogicalElement_quals, /* qualifiers */
    MI_COUNT(CIM_LogicalElement_quals), /* numQualifiers */
    CIM_LogicalElement_props, /* properties */
    MI_COUNT(CIM_LogicalElement_props), /* numProperties */
    sizeof(CIM_LogicalElement), /* size */
    MI_T("CIM_ManagedSystemElement"), /* superClass */
    &CIM_ManagedSystemElement_rtti, /* superClassDecl */
    NULL, /* methods */
    0, /* numMethods */
    &schemaDecl, /* schema */
    NULL, /* functions */
    NULL /* owningClass */
};

/*
**==============================================================================
**
** CIM_Job
**
**==============================================================================
*/

static MI_CONST MI_Char* CIM_Job_JobStatus_Description_qual_value = MI_T("58");

static MI_CONST MI_Qualifier CIM_Job_JobStatus_Description_qual =
{
    MI_T("Description"),
    MI_STRING,
    MI_FLAG_ENABLEOVERRIDE|MI_FLAG_TOSUBCLASS|MI_FLAG_TRANSLATABLE,
    &CIM_Job_JobStatus_Description_qual_value
};

static MI_CONST MI_Char* CIM_Job_JobStatus_ModelCorrespondence_qual_data_value[] =
{
    MI_T("CIM_ManagedSystemElement.OperationalStatus"),
};

static MI_CONST MI_ConstStringA CIM_Job_JobStatus_ModelCorrespondence_qual_value =
{
    CIM_Job_JobStatus_ModelCorrespondence_qual_data_value,
    MI_COUNT(CIM_Job_JobStatus_ModelCorrespondence_qual_data_value),
};

static MI_CONST MI_Qualifier CIM_Job_JobStatus_ModelCorrespondence_qual =
{
    MI_T("ModelCorrespondence"),
    MI_STRINGA,
    MI_FLAG_ENABLEOVERRIDE|MI_FLAG_TOSUBCLASS,
    &CIM_Job_JobStatus_ModelCorrespondence_qual_value
};

static MI_Qualifier MI_CONST* MI_CONST CIM_Job_JobStatus_quals[] =
{
    &CIM_Job_JobStatus_Description_qual,
    &CIM_Job_JobStatus_ModelCorrespondence_qual,
};

/* property CIM_Job.JobStatus */
static MI_CONST MI_PropertyDecl CIM_Job_JobStatus_prop =
{
    MI_FLAG_PROPERTY|MI_FLAG_READONLY, /* flags */
    0x006A7309, /* code */
    MI_T("JobStatus"), /* name */
    CIM_Job_JobStatus_quals, /* qualifiers */
    MI_COUNT(CIM_Job_JobStatus_quals), /* numQualifiers */
    MI_STRING, /* type */
    NULL, /* className */
    0, /* subscript */
    offsetof(CIM_Job, JobStatus), /* offset */
    MI_T("CIM_Job"), /* origin */
    MI_T("CIM_Job"), /* propagator */
    NULL,
};

static MI_CONST MI_Char* CIM_Job_TimeSubmitted_Description_qual_value = MI_T("59");

static MI_CONST MI_Qualifier CIM_Job_TimeSubmitted_Description_qual =
{
    MI_T("Description"),
    MI_STRING,
    MI_FLAG_ENABLEOVERRIDE|MI_FLAG_TOSUBCLASS|MI_FLAG_TRANSLATABLE,
    &CIM_Job_TimeSubmitted_Description_qual_value
};

static MI_Qualifier MI_CONST* MI_CONST CIM_Job_TimeSubmitted_quals[] =
{
    &CIM_Job_TimeSubmitted_Description_qual,
};

/* property CIM_Job.TimeSubmitted */
static MI_CONST MI_PropertyDecl CIM_Job_TimeSubmitted_prop =
{
    MI_FLAG_PROPERTY|MI_FLAG_READONLY, /* flags */
    0x0074640D, /* code */
    MI_T("TimeSubmitted"), /* name */
    CIM_Job_TimeSubmitted_quals, /* qualifiers */
    MI_COUNT(CIM_Job_TimeSubmitted_quals), /* numQualifiers */
    MI_DATETIME, /* type */
    NULL, /* className */
    0, /* subscript */
    offsetof(CIM_Job, TimeSubmitted), /* offset */
    MI_T("CIM_Job"), /* origin */
    MI_T("CIM_Job"), /* propagator */
    NULL,
};

static MI_CONST MI_Char* CIM_Job_ScheduledStartTime_Deprecated_qual_data_value[] =
{
    MI_T("CIM_Job.RunMonth"),
    MI_T("CIM_Job.RunDay"),
    MI_T("CIM_Job.RunDayOfWeek"),
    MI_T("CIM_Job.RunStartInterval"),
};

static MI_CONST MI_ConstStringA CIM_Job_ScheduledStartTime_Deprecated_qual_value =
{
    CIM_Job_ScheduledStartTime_Deprecated_qual_data_value,
    MI_COUNT(CIM_Job_ScheduledStartTime_Deprecated_qual_data_value),
};

static MI_CONST MI_Qualifier CIM_Job_ScheduledStartTime_Deprecated_qual =
{
    MI_T("Deprecated"),
    MI_STRINGA,
    MI_FLAG_ENABLEOVERRIDE|MI_FLAG_RESTRICTED,
    &CIM_Job_ScheduledStartTime_Deprecated_qual_value
};

static MI_CONST MI_Boolean CIM_Job_ScheduledStartTime_Write_qual_value = 1;

static MI_CONST MI_Qualifier CIM_Job_ScheduledStartTime_Write_qual =
{
    MI_T("Write"),
    MI_BOOLEAN,
    MI_FLAG_ENABLEOVERRIDE|MI_FLAG_TOSUBCLASS,
    &CIM_Job_ScheduledStartTime_Write_qual_value
};

static MI_CONST MI_Char* CIM_Job_ScheduledStartTime_Description_qual_value = MI_T("60");

static MI_CONST MI_Qualifier CIM_Job_ScheduledStartTime_Description_qual =
{
    MI_T("Description"),
    MI_STRING,
    MI_FLAG_ENABLEOVERRIDE|MI_FLAG_TOSUBCLASS|MI_FLAG_TRANSLATABLE,
    &CIM_Job_ScheduledStartTime_Description_qual_value
};

static MI_Qualifier MI_CONST* MI_CONST CIM_Job_ScheduledStartTime_quals[] =
{
    &CIM_Job_ScheduledStartTime_Deprecated_qual,
    &CIM_Job_ScheduledStartTime_Write_qual,
    &CIM_Job_ScheduledStartTime_Description_qual,
};

/* property CIM_Job.ScheduledStartTime */
static MI_CONST MI_PropertyDecl CIM_Job_ScheduledStartTime_prop =
{
    MI_FLAG_PROPERTY, /* flags */
    0x00736512, /* code */
    MI_T("ScheduledStartTime"), /* name */
    CIM_Job_ScheduledStartTime_quals, /* qualifiers */
    MI_COUNT(CIM_Job_ScheduledStartTime_quals), /* numQualifiers */
    MI_DATETIME, /* type */
    NULL, /* className */
    0, /* subscript */
    offsetof(CIM_Job, ScheduledStartTime), /* offset */
    MI_T("CIM_Job"), /* origin */
    MI_T("CIM_Job"), /* propagator */
    NULL,
};

static MI_CONST MI_Char* CIM_Job_StartTime_Description_qual_value = MI_T("61");

static MI_CONST MI_Qualifier CIM_Job_StartTime_Description_qual =
{
    MI_T("Description"),
    MI_STRING,
    MI_FLAG_ENABLEOVERRIDE|MI_FLAG_TOSUBCLASS|MI_FLAG_TRANSLATABLE,
    &CIM_Job_StartTime_Description_qual_value
};

static MI_Qualifier MI_CONST* MI_CONST CIM_Job_StartTime_quals[] =
{
    &CIM_Job_StartTime_Description_qual,
};

/* property CIM_Job.StartTime */
static MI_CONST MI_PropertyDecl CIM_Job_StartTime_prop =
{
    MI_FLAG_PROPERTY|MI_FLAG_READONLY, /* flags */
    0x00736509, /* code */
    MI_T("StartTime"), /* name */
    CIM_Job_StartTime_quals, /* qualifiers */
    MI_COUNT(CIM_Job_StartTime_quals), /* numQualifiers */
    MI_DATETIME, /* type */
    NULL, /* className */
    0, /* subscript */
    offsetof(CIM_Job, StartTime), /* offset */
    MI_T("CIM_Job"), /* origin */
    MI_T("CIM_Job"), /* propagator */
    NULL,
};

static MI_CONST MI_Char* CIM_Job_ElapsedTime_Description_qual_value = MI_T("62");

static MI_CONST MI_Qualifier CIM_Job_ElapsedTime_Description_qual =
{
    MI_T("Description"),
    MI_STRING,
    MI_FLAG_ENABLEOVERRIDE|MI_FLAG_TOSUBCLASS|MI_FLAG_TRANSLATABLE,
    &CIM_Job_ElapsedTime_Description_qual_value
};

static MI_Qualifier MI_CONST* MI_CONST CIM_Job_ElapsedTime_quals[] =
{
    &CIM_Job_ElapsedTime_Description_qual,
};

/* property CIM_Job.ElapsedTime */
static MI_CONST MI_PropertyDecl CIM_Job_ElapsedTime_prop =
{
    MI_FLAG_PROPERTY|MI_FLAG_READONLY, /* flags */
    0x0065650B, /* code */
    MI_T("ElapsedTime"), /* name */
    CIM_Job_ElapsedTime_quals, /* qualifiers */
    MI_COUNT(CIM_Job_ElapsedTime_quals), /* numQualifiers */
    MI_DATETIME, /* type */
    NULL, /* className */
    0, /* subscript */
    offsetof(CIM_Job, ElapsedTime), /* offset */
    MI_T("CIM_Job"), /* origin */
    MI_T("CIM_Job"), /* propagator */
    NULL,
};

static MI_CONST MI_Boolean CIM_Job_JobRunTimes_Write_qual_value = 1;

static MI_CONST MI_Qualifier CIM_Job_JobRunTimes_Write_qual =
{
    MI_T("Write"),
    MI_BOOLEAN,
    MI_FLAG_ENABLEOVERRIDE|MI_FLAG_TOSUBCLASS,
    &CIM_Job_JobRunTimes_Write_qual_value
};

static MI_CONST MI_Char* CIM_Job_JobRunTimes_Description_qual_value = MI_T("63");

static MI_CONST MI_Qualifier CIM_Job_JobRunTimes_Description_qual =
{
    MI_T("Description"),
    MI_STRING,
    MI_FLAG_ENABLEOVERRIDE|MI_FLAG_TOSUBCLASS|MI_FLAG_TRANSLATABLE,
    &CIM_Job_JobRunTimes_Description_qual_value
};

static MI_Qualifier MI_CONST* MI_CONST CIM_Job_JobRunTimes_quals[] =
{
    &CIM_Job_JobRunTimes_Write_qual,
    &CIM_Job_JobRunTimes_Description_qual,
};

static MI_CONST MI_Uint32 CIM_Job_JobRunTimes_value = 1U;

/* property CIM_Job.JobRunTimes */
static MI_CONST MI_PropertyDecl CIM_Job_JobRunTimes_prop =
{
    MI_FLAG_PROPERTY, /* flags */
    0x006A730B, /* code */
    MI_T("JobRunTimes"), /* name */
    CIM_Job_JobRunTimes_quals, /* qualifiers */
    MI_COUNT(CIM_Job_JobRunTimes_quals), /* numQualifiers */
    MI_UINT32, /* type */
    NULL, /* className */
    0, /* subscript */
    offsetof(CIM_Job, JobRunTimes), /* offset */
    MI_T("CIM_Job"), /* origin */
    MI_T("CIM_Job"), /* propagator */
    &CIM_Job_JobRunTimes_value,
};

static MI_CONST MI_Boolean CIM_Job_RunMonth_Write_qual_value = 1;

static MI_CONST MI_Qualifier CIM_Job_RunMonth_Write_qual =
{
    MI_T("Write"),
    MI_BOOLEAN,
    MI_FLAG_ENABLEOVERRIDE|MI_FLAG_TOSUBCLASS,
    &CIM_Job_RunMonth_Write_qual_value
};

static MI_CONST MI_Char* CIM_Job_RunMonth_Description_qual_value = MI_T("64");

static MI_CONST MI_Qualifier CIM_Job_RunMonth_Description_qual =
{
    MI_T("Description"),
    MI_STRING,
    MI_FLAG_ENABLEOVERRIDE|MI_FLAG_TOSUBCLASS|MI_FLAG_TRANSLATABLE,
    &CIM_Job_RunMonth_Description_qual_value
};

static MI_CONST MI_Char* CIM_Job_RunMonth_ValueMap_qual_data_value[] =
{
    MI_T("0"),
    MI_T("1"),
    MI_T("2"),
    MI_T("3"),
    MI_T("4"),
    MI_T("5"),
    MI_T("6"),
    MI_T("7"),
    MI_T("8"),
    MI_T("9"),
    MI_T("10"),
    MI_T("11"),
};

static MI_CONST MI_ConstStringA CIM_Job_RunMonth_ValueMap_qual_value =
{
    CIM_Job_RunMonth_ValueMap_qual_data_value,
    MI_COUNT(CIM_Job_RunMonth_ValueMap_qual_data_value),
};

static MI_CONST MI_Qualifier CIM_Job_RunMonth_ValueMap_qual =
{
    MI_T("ValueMap"),
    MI_STRINGA,
    MI_FLAG_ENABLEOVERRIDE|MI_FLAG_TOSUBCLASS,
    &CIM_Job_RunMonth_ValueMap_qual_value
};

static MI_CONST MI_Char* CIM_Job_RunMonth_Values_qual_data_value[] =
{
    MI_T("65"),
    MI_T("66"),
    MI_T("67"),
    MI_T("68"),
    MI_T("69"),
    MI_T("70"),
    MI_T("71"),
    MI_T("72"),
    MI_T("73"),
    MI_T("74"),
    MI_T("75"),
    MI_T("76"),
};

static MI_CONST MI_ConstStringA CIM_Job_RunMonth_Values_qual_value =
{
    CIM_Job_RunMonth_Values_qual_data_value,
    MI_COUNT(CIM_Job_RunMonth_Values_qual_data_value),
};

static MI_CONST MI_Qualifier CIM_Job_RunMonth_Values_qual =
{
    MI_T("Values"),
    MI_STRINGA,
    MI_FLAG_ENABLEOVERRIDE|MI_FLAG_TOSUBCLASS|MI_FLAG_TRANSLATABLE,
    &CIM_Job_RunMonth_Values_qual_value
};

static MI_CONST MI_Char* CIM_Job_RunMonth_ModelCorrespondence_qual_data_value[] =
{
    MI_T("CIM_Job.RunDay"),
    MI_T("CIM_Job.RunDayOfWeek"),
    MI_T("CIM_Job.RunStartInterval"),
};

static MI_CONST MI_ConstStringA CIM_Job_RunMonth_ModelCorrespondence_qual_value =
{
    CIM_Job_RunMonth_ModelCorrespondence_qual_data_value,
    MI_COUNT(CIM_Job_RunMonth_ModelCorrespondence_qual_data_value),
};

static MI_CONST MI_Qualifier CIM_Job_RunMonth_ModelCorrespondence_qual =
{
    MI_T("ModelCorrespondence"),
    MI_STRINGA,
    MI_FLAG_ENABLEOVERRIDE|MI_FLAG_TOSUBCLASS,
    &CIM_Job_RunMonth_ModelCorrespondence_qual_value
};

static MI_Qualifier MI_CONST* MI_CONST CIM_Job_RunMonth_quals[] =
{
    &CIM_Job_RunMonth_Write_qual,
    &CIM_Job_RunMonth_Description_qual,
    &CIM_Job_RunMonth_ValueMap_qual,
    &CIM_Job_RunMonth_Values_qual,
    &CIM_Job_RunMonth_ModelCorrespondence_qual,
};

/* property CIM_Job.RunMonth */
static MI_CONST MI_PropertyDecl CIM_Job_RunMonth_prop =
{
    MI_FLAG_PROPERTY, /* flags */
    0x00726808, /* code */
    MI_T("RunMonth"), /* name */
    CIM_Job_RunMonth_quals, /* qualifiers */
    MI_COUNT(CIM_Job_RunMonth_quals), /* numQualifiers */
    MI_UINT8, /* type */
    NULL, /* className */
    0, /* subscript */
    offsetof(CIM_Job, RunMonth), /* offset */
    MI_T("CIM_Job"), /* origin */
    MI_T("CIM_Job"), /* propagator */
    NULL,
};

static MI_CONST MI_Boolean CIM_Job_RunDay_Write_qual_value = 1;

static MI_CONST MI_Qualifier CIM_Job_RunDay_Write_qual =
{
    MI_T("Write"),
    MI_BOOLEAN,
    MI_FLAG_ENABLEOVERRIDE|MI_FLAG_TOSUBCLASS,
    &CIM_Job_RunDay_Write_qual_value
};

static MI_CONST MI_Char* CIM_Job_RunDay_Description_qual_value = MI_T("77");

static MI_CONST MI_Qualifier CIM_Job_RunDay_Description_qual =
{
    MI_T("Description"),
    MI_STRING,
    MI_FLAG_ENABLEOVERRIDE|MI_FLAG_TOSUBCLASS|MI_FLAG_TRANSLATABLE,
    &CIM_Job_RunDay_Description_qual_value
};

static MI_CONST MI_Sint64 CIM_Job_RunDay_MinValue_qual_value = MI_LL(-31);

static MI_CONST MI_Qualifier CIM_Job_RunDay_MinValue_qual =
{
    MI_T("MinValue"),
    MI_SINT64,
    MI_FLAG_ENABLEOVERRIDE|MI_FLAG_TOSUBCLASS,
    &CIM_Job_RunDay_MinValue_qual_value
};

static MI_CONST MI_Sint64 CIM_Job_RunDay_MaxValue_qual_value = MI_LL(31);

static MI_CONST MI_Qualifier CIM_Job_RunDay_MaxValue_qual =
{
    MI_T("MaxValue"),
    MI_SINT64,
    MI_FLAG_ENABLEOVERRIDE|MI_FLAG_TOSUBCLASS,
    &CIM_Job_RunDay_MaxValue_qual_value
};

static MI_CONST MI_Char* CIM_Job_RunDay_ModelCorrespondence_qual_data_value[] =
{
    MI_T("CIM_Job.RunMonth"),
    MI_T("CIM_Job.RunDayOfWeek"),
    MI_T("CIM_Job.RunStartInterval"),
};

static MI_CONST MI_ConstStringA CIM_Job_RunDay_ModelCorrespondence_qual_value =
{
    CIM_Job_RunDay_ModelCorrespondence_qual_data_value,
    MI_COUNT(CIM_Job_RunDay_ModelCorrespondence_qual_data_value),
};

static MI_CONST MI_Qualifier CIM_Job_RunDay_ModelCorrespondence_qual =
{
    MI_T("ModelCorrespondence"),
    MI_STRINGA,
    MI_FLAG_ENABLEOVERRIDE|MI_FLAG_TOSUBCLASS,
    &CIM_Job_RunDay_ModelCorrespondence_qual_value
};

static MI_Qualifier MI_CONST* MI_CONST CIM_Job_RunDay_quals[] =
{
    &CIM_Job_RunDay_Write_qual,
    &CIM_Job_RunDay_Description_qual,
    &CIM_Job_RunDay_MinValue_qual,
    &CIM_Job_RunDay_MaxValue_qual,
    &CIM_Job_RunDay_ModelCorrespondence_qual,
};

/* property CIM_Job.RunDay */
static MI_CONST MI_PropertyDecl CIM_Job_RunDay_prop =
{
    MI_FLAG_PROPERTY, /* flags */
    0x00727906, /* code */
    MI_T("RunDay"), /* name */
    CIM_Job_RunDay_quals, /* qualifiers */
    MI_COUNT(CIM_Job_RunDay_quals), /* numQualifiers */
    MI_SINT8, /* type */
    NULL, /* className */
    0, /* subscript */
    offsetof(CIM_Job, RunDay), /* offset */
    MI_T("CIM_Job"), /* origin */
    MI_T("CIM_Job"), /* propagator */
    NULL,
};

static MI_CONST MI_Boolean CIM_Job_RunDayOfWeek_Write_qual_value = 1;

static MI_CONST MI_Qualifier CIM_Job_RunDayOfWeek_Write_qual =
{
    MI_T("Write"),
    MI_BOOLEAN,
    MI_FLAG_ENABLEOVERRIDE|MI_FLAG_TOSUBCLASS,
    &CIM_Job_RunDayOfWeek_Write_qual_value
};

static MI_CONST MI_Char* CIM_Job_RunDayOfWeek_Description_qual_value = MI_T("78");

static MI_CONST MI_Qualifier CIM_Job_RunDayOfWeek_Description_qual =
{
    MI_T("Description"),
    MI_STRING,
    MI_FLAG_ENABLEOVERRIDE|MI_FLAG_TOSUBCLASS|MI_FLAG_TRANSLATABLE,
    &CIM_Job_RunDayOfWeek_Description_qual_value
};

static MI_CONST MI_Char* CIM_Job_RunDayOfWeek_ValueMap_qual_data_value[] =
{
    MI_T("-7"),
    MI_T("-6"),
    MI_T("-5"),
    MI_T("-4"),
    MI_T("-3"),
    MI_T("-2"),
    MI_T("-1"),
    MI_T("0"),
    MI_T("1"),
    MI_T("2"),
    MI_T("3"),
    MI_T("4"),
    MI_T("5"),
    MI_T("6"),
    MI_T("7"),
};

static MI_CONST MI_ConstStringA CIM_Job_RunDayOfWeek_ValueMap_qual_value =
{
    CIM_Job_RunDayOfWeek_ValueMap_qual_data_value,
    MI_COUNT(CIM_Job_RunDayOfWeek_ValueMap_qual_data_value),
};

static MI_CONST MI_Qualifier CIM_Job_RunDayOfWeek_ValueMap_qual =
{
    MI_T("ValueMap"),
    MI_STRINGA,
    MI_FLAG_ENABLEOVERRIDE|MI_FLAG_TOSUBCLASS,
    &CIM_Job_RunDayOfWeek_ValueMap_qual_value
};

static MI_CONST MI_Char* CIM_Job_RunDayOfWeek_Values_qual_data_value[] =
{
    MI_T("79"),
    MI_T("80"),
    MI_T("81"),
    MI_T("82"),
    MI_T("83"),
    MI_T("84"),
    MI_T("85"),
    MI_T("86"),
    MI_T("87"),
    MI_T("88"),
    MI_T("89"),
    MI_T("90"),
    MI_T("91"),
    MI_T("92"),
    MI_T("93"),
};

static MI_CONST MI_ConstStringA CIM_Job_RunDayOfWeek_Values_qual_value =
{
    CIM_Job_RunDayOfWeek_Values_qual_data_value,
    MI_COUNT(CIM_Job_RunDayOfWeek_Values_qual_data_value),
};

static MI_CONST MI_Qualifier CIM_Job_RunDayOfWeek_Values_qual =
{
    MI_T("Values"),
    MI_STRINGA,
    MI_FLAG_ENABLEOVERRIDE|MI_FLAG_TOSUBCLASS|MI_FLAG_TRANSLATABLE,
    &CIM_Job_RunDayOfWeek_Values_qual_value
};

static MI_CONST MI_Char* CIM_Job_RunDayOfWeek_ModelCorrespondence_qual_data_value[] =
{
    MI_T("CIM_Job.RunMonth"),
    MI_T("CIM_Job.RunDay"),
    MI_T("CIM_Job.RunStartInterval"),
};

static MI_CONST MI_ConstStringA CIM_Job_RunDayOfWeek_ModelCorrespondence_qual_value =
{
    CIM_Job_RunDayOfWeek_ModelCorrespondence_qual_data_value,
    MI_COUNT(CIM_Job_RunDayOfWeek_ModelCorrespondence_qual_data_value),
};

static MI_CONST MI_Qualifier CIM_Job_RunDayOfWeek_ModelCorrespondence_qual =
{
    MI_T("ModelCorrespondence"),
    MI_STRINGA,
    MI_FLAG_ENABLEOVERRIDE|MI_FLAG_TOSUBCLASS,
    &CIM_Job_RunDayOfWeek_ModelCorrespondence_qual_value
};

static MI_Qualifier MI_CONST* MI_CONST CIM_Job_RunDayOfWeek_quals[] =
{
    &CIM_Job_RunDayOfWeek_Write_qual,
    &CIM_Job_RunDayOfWeek_Description_qual,
    &CIM_Job_RunDayOfWeek_ValueMap_qual,
    &CIM_Job_RunDayOfWeek_Values_qual,
    &CIM_Job_RunDayOfWeek_ModelCorrespondence_qual,
};

/* property CIM_Job.RunDayOfWeek */
static MI_CONST MI_PropertyDecl CIM_Job_RunDayOfWeek_prop =
{
    MI_FLAG_PROPERTY, /* flags */
    0x00726B0C, /* code */
    MI_T("RunDayOfWeek"), /* name */
    CIM_Job_RunDayOfWeek_quals, /* qualifiers */
    MI_COUNT(CIM_Job_RunDayOfWeek_quals), /* numQualifiers */
    MI_SINT8, /* type */
    NULL, /* className */
    0, /* subscript */
    offsetof(CIM_Job, RunDayOfWeek), /* offset */
    MI_T("CIM_Job"), /* origin */
    MI_T("CIM_Job"), /* propagator */
    NULL,
};

static MI_CONST MI_Boolean CIM_Job_RunStartInterval_Write_qual_value = 1;

static MI_CONST MI_Qualifier CIM_Job_RunStartInterval_Write_qual =
{
    MI_T("Write"),
    MI_BOOLEAN,
    MI_FLAG_ENABLEOVERRIDE|MI_FLAG_TOSUBCLASS,
    &CIM_Job_RunStartInterval_Write_qual_value
};

static MI_CONST MI_Char* CIM_Job_RunStartInterval_Description_qual_value = MI_T("94");

static MI_CONST MI_Qualifier CIM_Job_RunStartInterval_Description_qual =
{
    MI_T("Description"),
    MI_STRING,
    MI_FLAG_ENABLEOVERRIDE|MI_FLAG_TOSUBCLASS|MI_FLAG_TRANSLATABLE,
    &CIM_Job_RunStartInterval_Description_qual_value
};

static MI_CONST MI_Char* CIM_Job_RunStartInterval_ModelCorrespondence_qual_data_value[] =
{
    MI_T("CIM_Job.RunMonth"),
    MI_T("CIM_Job.RunDay"),
    MI_T("CIM_Job.RunDayOfWeek"),
    MI_T("CIM_Job.RunStartInterval"),
};

static MI_CONST MI_ConstStringA CIM_Job_RunStartInterval_ModelCorrespondence_qual_value =
{
    CIM_Job_RunStartInterval_ModelCorrespondence_qual_data_value,
    MI_COUNT(CIM_Job_RunStartInterval_ModelCorrespondence_qual_data_value),
};

static MI_CONST MI_Qualifier CIM_Job_RunStartInterval_ModelCorrespondence_qual =
{
    MI_T("ModelCorrespondence"),
    MI_STRINGA,
    MI_FLAG_ENABLEOVERRIDE|MI_FLAG_TOSUBCLASS,
    &CIM_Job_RunStartInterval_ModelCorrespondence_qual_value
};

static MI_Qualifier MI_CONST* MI_CONST CIM_Job_RunStartInterval_quals[] =
{
    &CIM_Job_RunStartInterval_Write_qual,
    &CIM_Job_RunStartInterval_Description_qual,
    &CIM_Job_RunStartInterval_ModelCorrespondence_qual,
};

/* property CIM_Job.RunStartInterval */
static MI_CONST MI_PropertyDecl CIM_Job_RunStartInterval_prop =
{
    MI_FLAG_PROPERTY, /* flags */
    0x00726C10, /* code */
    MI_T("RunStartInterval"), /* name */
    CIM_Job_RunStartInterval_quals, /* qualifiers */
    MI_COUNT(CIM_Job_RunStartInterval_quals), /* numQualifiers */
    MI_DATETIME, /* type */
    NULL, /* className */
    0, /* subscript */
    offsetof(CIM_Job, RunStartInterval), /* offset */
    MI_T("CIM_Job"), /* origin */
    MI_T("CIM_Job"), /* propagator */
    NULL,
};

static MI_CONST MI_Boolean CIM_Job_LocalOrUtcTime_Write_qual_value = 1;

static MI_CONST MI_Qualifier CIM_Job_LocalOrUtcTime_Write_qual =
{
    MI_T("Write"),
    MI_BOOLEAN,
    MI_FLAG_ENABLEOVERRIDE|MI_FLAG_TOSUBCLASS,
    &CIM_Job_LocalOrUtcTime_Write_qual_value
};

static MI_CONST MI_Char* CIM_Job_LocalOrUtcTime_Description_qual_value = MI_T("95");

static MI_CONST MI_Qualifier CIM_Job_LocalOrUtcTime_Description_qual =
{
    MI_T("Description"),
    MI_STRING,
    MI_FLAG_ENABLEOVERRIDE|MI_FLAG_TOSUBCLASS|MI_FLAG_TRANSLATABLE,
    &CIM_Job_LocalOrUtcTime_Description_qual_value
};

static MI_CONST MI_Char* CIM_Job_LocalOrUtcTime_ValueMap_qual_data_value[] =
{
    MI_T("1"),
    MI_T("2"),
};

static MI_CONST MI_ConstStringA CIM_Job_LocalOrUtcTime_ValueMap_qual_value =
{
    CIM_Job_LocalOrUtcTime_ValueMap_qual_data_value,
    MI_COUNT(CIM_Job_LocalOrUtcTime_ValueMap_qual_data_value),
};

static MI_CONST MI_Qualifier CIM_Job_LocalOrUtcTime_ValueMap_qual =
{
    MI_T("ValueMap"),
    MI_STRINGA,
    MI_FLAG_ENABLEOVERRIDE|MI_FLAG_TOSUBCLASS,
    &CIM_Job_LocalOrUtcTime_ValueMap_qual_value
};

static MI_CONST MI_Char* CIM_Job_LocalOrUtcTime_Values_qual_data_value[] =
{
    MI_T("96"),
    MI_T("97"),
};

static MI_CONST MI_ConstStringA CIM_Job_LocalOrUtcTime_Values_qual_value =
{
    CIM_Job_LocalOrUtcTime_Values_qual_data_value,
    MI_COUNT(CIM_Job_LocalOrUtcTime_Values_qual_data_value),
};

static MI_CONST MI_Qualifier CIM_Job_LocalOrUtcTime_Values_qual =
{
    MI_T("Values"),
    MI_STRINGA,
    MI_FLAG_ENABLEOVERRIDE|MI_FLAG_TOSUBCLASS|MI_FLAG_TRANSLATABLE,
    &CIM_Job_LocalOrUtcTime_Values_qual_value
};

static MI_Qualifier MI_CONST* MI_CONST CIM_Job_LocalOrUtcTime_quals[] =
{
    &CIM_Job_LocalOrUtcTime_Write_qual,
    &CIM_Job_LocalOrUtcTime_Description_qual,
    &CIM_Job_LocalOrUtcTime_ValueMap_qual,
    &CIM_Job_LocalOrUtcTime_Values_qual,
};

/* property CIM_Job.LocalOrUtcTime */
static MI_CONST MI_PropertyDecl CIM_Job_LocalOrUtcTime_prop =
{
    MI_FLAG_PROPERTY, /* flags */
    0x006C650E, /* code */
    MI_T("LocalOrUtcTime"), /* name */
    CIM_Job_LocalOrUtcTime_quals, /* qualifiers */
    MI_COUNT(CIM_Job_LocalOrUtcTime_quals), /* numQualifiers */
    MI_UINT16, /* type */
    NULL, /* className */
    0, /* subscript */
    offsetof(CIM_Job, LocalOrUtcTime), /* offset */
    MI_T("CIM_Job"), /* origin */
    MI_T("CIM_Job"), /* propagator */
    NULL,
};

static MI_CONST MI_Boolean CIM_Job_UntilTime_Write_qual_value = 1;

static MI_CONST MI_Qualifier CIM_Job_UntilTime_Write_qual =
{
    MI_T("Write"),
    MI_BOOLEAN,
    MI_FLAG_ENABLEOVERRIDE|MI_FLAG_TOSUBCLASS,
    &CIM_Job_UntilTime_Write_qual_value
};

static MI_CONST MI_Char* CIM_Job_UntilTime_Description_qual_value = MI_T("98");

static MI_CONST MI_Qualifier CIM_Job_UntilTime_Description_qual =
{
    MI_T("Description"),
    MI_STRING,
    MI_FLAG_ENABLEOVERRIDE|MI_FLAG_TOSUBCLASS|MI_FLAG_TRANSLATABLE,
    &CIM_Job_UntilTime_Description_qual_value
};

static MI_CONST MI_Char* CIM_Job_UntilTime_ModelCorrespondence_qual_data_value[] =
{
    MI_T("CIM_Job.LocalOrUtcTime"),
};

static MI_CONST MI_ConstStringA CIM_Job_UntilTime_ModelCorrespondence_qual_value =
{
    CIM_Job_UntilTime_ModelCorrespondence_qual_data_value,
    MI_COUNT(CIM_Job_UntilTime_ModelCorrespondence_qual_data_value),
};

static MI_CONST MI_Qualifier CIM_Job_UntilTime_ModelCorrespondence_qual =
{
    MI_T("ModelCorrespondence"),
    MI_STRINGA,
    MI_FLAG_ENABLEOVERRIDE|MI_FLAG_TOSUBCLASS,
    &CIM_Job_UntilTime_ModelCorrespondence_qual_value
};

static MI_Qualifier MI_CONST* MI_CONST CIM_Job_UntilTime_quals[] =
{
    &CIM_Job_UntilTime_Write_qual,
    &CIM_Job_UntilTime_Description_qual,
    &CIM_Job_UntilTime_ModelCorrespondence_qual,
};

/* property CIM_Job.UntilTime */
static MI_CONST MI_PropertyDecl CIM_Job_UntilTime_prop =
{
    MI_FLAG_PROPERTY, /* flags */
    0x00756509, /* code */
    MI_T("UntilTime"), /* name */
    CIM_Job_UntilTime_quals, /* qualifiers */
    MI_COUNT(CIM_Job_UntilTime_quals), /* numQualifiers */
    MI_DATETIME, /* type */
    NULL, /* className */
    0, /* subscript */
    offsetof(CIM_Job, UntilTime), /* offset */
    MI_T("CIM_Job"), /* origin */
    MI_T("CIM_Job"), /* propagator */
    NULL,
};

static MI_CONST MI_Boolean CIM_Job_Notify_Write_qual_value = 1;

static MI_CONST MI_Qualifier CIM_Job_Notify_Write_qual =
{
    MI_T("Write"),
    MI_BOOLEAN,
    MI_FLAG_ENABLEOVERRIDE|MI_FLAG_TOSUBCLASS,
    &CIM_Job_Notify_Write_qual_value
};

static MI_CONST MI_Char* CIM_Job_Notify_Description_qual_value = MI_T("99");

static MI_CONST MI_Qualifier CIM_Job_Notify_Description_qual =
{
    MI_T("Description"),
    MI_STRING,
    MI_FLAG_ENABLEOVERRIDE|MI_FLAG_TOSUBCLASS|MI_FLAG_TRANSLATABLE,
    &CIM_Job_Notify_Description_qual_value
};

static MI_Qualifier MI_CONST* MI_CONST CIM_Job_Notify_quals[] =
{
    &CIM_Job_Notify_Write_qual,
    &CIM_Job_Notify_Description_qual,
};

/* property CIM_Job.Notify */
static MI_CONST MI_PropertyDecl CIM_Job_Notify_prop =
{
    MI_FLAG_PROPERTY, /* flags */
    0x006E7906, /* code */
    MI_T("Notify"), /* name */
    CIM_Job_Notify_quals, /* qualifiers */
    MI_COUNT(CIM_Job_Notify_quals), /* numQualifiers */
    MI_STRING, /* type */
    NULL, /* className */
    0, /* subscript */
    offsetof(CIM_Job, Notify), /* offset */
    MI_T("CIM_Job"), /* origin */
    MI_T("CIM_Job"), /* propagator */
    NULL,
};

static MI_CONST MI_Char* CIM_Job_Owner_Description_qual_value = MI_T("100");

static MI_CONST MI_Qualifier CIM_Job_Owner_Description_qual =
{
    MI_T("Description"),
    MI_STRING,
    MI_FLAG_ENABLEOVERRIDE|MI_FLAG_TOSUBCLASS|MI_FLAG_TRANSLATABLE,
    &CIM_Job_Owner_Description_qual_value
};

static MI_CONST MI_Char* CIM_Job_Owner_ModelCorrespondence_qual_data_value[] =
{
    MI_T("CIM_OwningJobElement"),
};

static MI_CONST MI_ConstStringA CIM_Job_Owner_ModelCorrespondence_qual_value =
{
    CIM_Job_Owner_ModelCorrespondence_qual_data_value,
    MI_COUNT(CIM_Job_Owner_ModelCorrespondence_qual_data_value),
};

static MI_CONST MI_Qualifier CIM_Job_Owner_ModelCorrespondence_qual =
{
    MI_T("ModelCorrespondence"),
    MI_STRINGA,
    MI_FLAG_ENABLEOVERRIDE|MI_FLAG_TOSUBCLASS,
    &CIM_Job_Owner_ModelCorrespondence_qual_value
};

static MI_Qualifier MI_CONST* MI_CONST CIM_Job_Owner_quals[] =
{
    &CIM_Job_Owner_Description_qual,
    &CIM_Job_Owner_ModelCorrespondence_qual,
};

/* property CIM_Job.Owner */
static MI_CONST MI_PropertyDecl CIM_Job_Owner_prop =
{
    MI_FLAG_PROPERTY|MI_FLAG_READONLY, /* flags */
    0x006F7205, /* code */
    MI_T("Owner"), /* name */
    CIM_Job_Owner_quals, /* qualifiers */
    MI_COUNT(CIM_Job_Owner_quals), /* numQualifiers */
    MI_STRING, /* type */
    NULL, /* className */
    0, /* subscript */
    offsetof(CIM_Job, Owner), /* offset */
    MI_T("CIM_Job"), /* origin */
    MI_T("CIM_Job"), /* propagator */
    NULL,
};

static MI_CONST MI_Boolean CIM_Job_Priority_Write_qual_value = 1;

static MI_CONST MI_Qualifier CIM_Job_Priority_Write_qual =
{
    MI_T("Write"),
    MI_BOOLEAN,
    MI_FLAG_ENABLEOVERRIDE|MI_FLAG_TOSUBCLASS,
    &CIM_Job_Priority_Write_qual_value
};

static MI_CONST MI_Char* CIM_Job_Priority_Description_qual_value = MI_T("101");

static MI_CONST MI_Qualifier CIM_Job_Priority_Description_qual =
{
    MI_T("Description"),
    MI_STRING,
    MI_FLAG_ENABLEOVERRIDE|MI_FLAG_TOSUBCLASS|MI_FLAG_TRANSLATABLE,
    &CIM_Job_Priority_Description_qual_value
};

static MI_Qualifier MI_CONST* MI_CONST CIM_Job_Priority_quals[] =
{
    &CIM_Job_Priority_Write_qual,
    &CIM_Job_Priority_Description_qual,
};

/* property CIM_Job.Priority */
static MI_CONST MI_PropertyDecl CIM_Job_Priority_prop =
{
    MI_FLAG_PROPERTY, /* flags */
    0x00707908, /* code */
    MI_T("Priority"), /* name */
    CIM_Job_Priority_quals, /* qualifiers */
    MI_COUNT(CIM_Job_Priority_quals), /* numQualifiers */
    MI_UINT32, /* type */
    NULL, /* className */
    0, /* subscript */
    offsetof(CIM_Job, Priority), /* offset */
    MI_T("CIM_Job"), /* origin */
    MI_T("CIM_Job"), /* propagator */
    NULL,
};

static MI_CONST MI_Char* CIM_Job_PercentComplete_Description_qual_value = MI_T("102");

static MI_CONST MI_Qualifier CIM_Job_PercentComplete_Description_qual =
{
    MI_T("Description"),
    MI_STRING,
    MI_FLAG_ENABLEOVERRIDE|MI_FLAG_TOSUBCLASS|MI_FLAG_TRANSLATABLE,
    &CIM_Job_PercentComplete_Description_qual_value
};

static MI_CONST MI_Char* CIM_Job_PercentComplete_Units_qual_value = MI_T("103");

static MI_CONST MI_Qualifier CIM_Job_PercentComplete_Units_qual =
{
    MI_T("Units"),
    MI_STRING,
    MI_FLAG_ENABLEOVERRIDE|MI_FLAG_TOSUBCLASS|MI_FLAG_TRANSLATABLE,
    &CIM_Job_PercentComplete_Units_qual_value
};

static MI_CONST MI_Sint64 CIM_Job_PercentComplete_MinValue_qual_value = MI_LL(0);

static MI_CONST MI_Qualifier CIM_Job_PercentComplete_MinValue_qual =
{
    MI_T("MinValue"),
    MI_SINT64,
    MI_FLAG_ENABLEOVERRIDE|MI_FLAG_TOSUBCLASS,
    &CIM_Job_PercentComplete_MinValue_qual_value
};

static MI_CONST MI_Sint64 CIM_Job_PercentComplete_MaxValue_qual_value = MI_LL(101);

static MI_CONST MI_Qualifier CIM_Job_PercentComplete_MaxValue_qual =
{
    MI_T("MaxValue"),
    MI_SINT64,
    MI_FLAG_ENABLEOVERRIDE|MI_FLAG_TOSUBCLASS,
    &CIM_Job_PercentComplete_MaxValue_qual_value
};

static MI_CONST MI_Char* CIM_Job_PercentComplete_PUnit_qual_value = MI_T("percent");

static MI_CONST MI_Qualifier CIM_Job_PercentComplete_PUnit_qual =
{
    MI_T("PUnit"),
    MI_STRING,
    MI_FLAG_ENABLEOVERRIDE|MI_FLAG_TOSUBCLASS,
    &CIM_Job_PercentComplete_PUnit_qual_value
};

static MI_Qualifier MI_CONST* MI_CONST CIM_Job_PercentComplete_quals[] =
{
    &CIM_Job_PercentComplete_Description_qual,
    &CIM_Job_PercentComplete_Units_qual,
    &CIM_Job_PercentComplete_MinValue_qual,
    &CIM_Job_PercentComplete_MaxValue_qual,
    &CIM_Job_PercentComplete_PUnit_qual,
};

/* property CIM_Job.PercentComplete */
static MI_CONST MI_PropertyDecl CIM_Job_PercentComplete_prop =
{
    MI_FLAG_PROPERTY|MI_FLAG_READONLY, /* flags */
    0x0070650F, /* code */
    MI_T("PercentComplete"), /* name */
    CIM_Job_PercentComplete_quals, /* qualifiers */
    MI_COUNT(CIM_Job_PercentComplete_quals), /* numQualifiers */
    MI_UINT16, /* type */
    NULL, /* className */
    0, /* subscript */
    offsetof(CIM_Job, PercentComplete), /* offset */
    MI_T("CIM_Job"), /* origin */
    MI_T("CIM_Job"), /* propagator */
    NULL,
};

static MI_CONST MI_Boolean CIM_Job_DeleteOnCompletion_Write_qual_value = 1;

static MI_CONST MI_Qualifier CIM_Job_DeleteOnCompletion_Write_qual =
{
    MI_T("Write"),
    MI_BOOLEAN,
    MI_FLAG_ENABLEOVERRIDE|MI_FLAG_TOSUBCLASS,
    &CIM_Job_DeleteOnCompletion_Write_qual_value
};

static MI_CONST MI_Char* CIM_Job_DeleteOnCompletion_Description_qual_value = MI_T("104");

static MI_CONST MI_Qualifier CIM_Job_DeleteOnCompletion_Description_qual =
{
    MI_T("Description"),
    MI_STRING,
    MI_FLAG_ENABLEOVERRIDE|MI_FLAG_TOSUBCLASS|MI_FLAG_TRANSLATABLE,
    &CIM_Job_DeleteOnCompletion_Description_qual_value
};

static MI_Qualifier MI_CONST* MI_CONST CIM_Job_DeleteOnCompletion_quals[] =
{
    &CIM_Job_DeleteOnCompletion_Write_qual,
    &CIM_Job_DeleteOnCompletion_Description_qual,
};

/* property CIM_Job.DeleteOnCompletion */
static MI_CONST MI_PropertyDecl CIM_Job_DeleteOnCompletion_prop =
{
    MI_FLAG_PROPERTY, /* flags */
    0x00646E12, /* code */
    MI_T("DeleteOnCompletion"), /* name */
    CIM_Job_DeleteOnCompletion_quals, /* qualifiers */
    MI_COUNT(CIM_Job_DeleteOnCompletion_quals), /* numQualifiers */
    MI_BOOLEAN, /* type */
    NULL, /* className */
    0, /* subscript */
    offsetof(CIM_Job, DeleteOnCompletion), /* offset */
    MI_T("CIM_Job"), /* origin */
    MI_T("CIM_Job"), /* propagator */
    NULL,
};

static MI_CONST MI_Char* CIM_Job_ErrorCode_Description_qual_value = MI_T("105");

static MI_CONST MI_Qualifier CIM_Job_ErrorCode_Description_qual =
{
    MI_T("Description"),
    MI_STRING,
    MI_FLAG_ENABLEOVERRIDE|MI_FLAG_TOSUBCLASS|MI_FLAG_TRANSLATABLE,
    &CIM_Job_ErrorCode_Description_qual_value
};

static MI_CONST MI_Char* CIM_Job_ErrorCode_ModelCorrespondence_qual_data_value[] =
{
    MI_T("CIM_Job.ErrorDescription"),
};

static MI_CONST MI_ConstStringA CIM_Job_ErrorCode_ModelCorrespondence_qual_value =
{
    CIM_Job_ErrorCode_ModelCorrespondence_qual_data_value,
    MI_COUNT(CIM_Job_ErrorCode_ModelCorrespondence_qual_data_value),
};

static MI_CONST MI_Qualifier CIM_Job_ErrorCode_ModelCorrespondence_qual =
{
    MI_T("ModelCorrespondence"),
    MI_STRINGA,
    MI_FLAG_ENABLEOVERRIDE|MI_FLAG_TOSUBCLASS,
    &CIM_Job_ErrorCode_ModelCorrespondence_qual_value
};

static MI_Qualifier MI_CONST* MI_CONST CIM_Job_ErrorCode_quals[] =
{
    &CIM_Job_ErrorCode_Description_qual,
    &CIM_Job_ErrorCode_ModelCorrespondence_qual,
};

/* property CIM_Job.ErrorCode */
static MI_CONST MI_PropertyDecl CIM_Job_ErrorCode_prop =
{
    MI_FLAG_PROPERTY|MI_FLAG_READONLY, /* flags */
    0x00656509, /* code */
    MI_T("ErrorCode"), /* name */
    CIM_Job_ErrorCode_quals, /* qualifiers */
    MI_COUNT(CIM_Job_ErrorCode_quals), /* numQualifiers */
    MI_UINT16, /* type */
    NULL, /* className */
    0, /* subscript */
    offsetof(CIM_Job, ErrorCode), /* offset */
    MI_T("CIM_Job"), /* origin */
    MI_T("CIM_Job"), /* propagator */
    NULL,
};

static MI_CONST MI_Char* CIM_Job_ErrorDescription_Description_qual_value = MI_T("106");

static MI_CONST MI_Qualifier CIM_Job_ErrorDescription_Description_qual =
{
    MI_T("Description"),
    MI_STRING,
    MI_FLAG_ENABLEOVERRIDE|MI_FLAG_TOSUBCLASS|MI_FLAG_TRANSLATABLE,
    &CIM_Job_ErrorDescription_Description_qual_value
};

static MI_CONST MI_Char* CIM_Job_ErrorDescription_ModelCorrespondence_qual_data_value[] =
{
    MI_T("CIM_Job.ErrorCode"),
};

static MI_CONST MI_ConstStringA CIM_Job_ErrorDescription_ModelCorrespondence_qual_value =
{
    CIM_Job_ErrorDescription_ModelCorrespondence_qual_data_value,
    MI_COUNT(CIM_Job_ErrorDescription_ModelCorrespondence_qual_data_value),
};

static MI_CONST MI_Qualifier CIM_Job_ErrorDescription_ModelCorrespondence_qual =
{
    MI_T("ModelCorrespondence"),
    MI_STRINGA,
    MI_FLAG_ENABLEOVERRIDE|MI_FLAG_TOSUBCLASS,
    &CIM_Job_ErrorDescription_ModelCorrespondence_qual_value
};

static MI_Qualifier MI_CONST* MI_CONST CIM_Job_ErrorDescription_quals[] =
{
    &CIM_Job_ErrorDescription_Description_qual,
    &CIM_Job_ErrorDescription_ModelCorrespondence_qual,
};

/* property CIM_Job.ErrorDescription */
static MI_CONST MI_PropertyDecl CIM_Job_ErrorDescription_prop =
{
    MI_FLAG_PROPERTY|MI_FLAG_READONLY, /* flags */
    0x00656E10, /* code */
    MI_T("ErrorDescription"), /* name */
    CIM_Job_ErrorDescription_quals, /* qualifiers */
    MI_COUNT(CIM_Job_ErrorDescription_quals), /* numQualifiers */
    MI_STRING, /* type */
    NULL, /* className */
    0, /* subscript */
    offsetof(CIM_Job, ErrorDescription), /* offset */
    MI_T("CIM_Job"), /* origin */
    MI_T("CIM_Job"), /* propagator */
    NULL,
};

static MI_CONST MI_Char* CIM_Job_RecoveryAction_Description_qual_value = MI_T("107");

static MI_CONST MI_Qualifier CIM_Job_RecoveryAction_Description_qual =
{
    MI_T("Description"),
    MI_STRING,
    MI_FLAG_ENABLEOVERRIDE|MI_FLAG_TOSUBCLASS|MI_FLAG_TRANSLATABLE,
    &CIM_Job_RecoveryAction_Description_qual_value
};

static MI_CONST MI_Char* CIM_Job_RecoveryAction_ValueMap_qual_data_value[] =
{
    MI_T("0"),
    MI_T("1"),
    MI_T("2"),
    MI_T("3"),
    MI_T("4"),
    MI_T("5"),
};

static MI_CONST MI_ConstStringA CIM_Job_RecoveryAction_ValueMap_qual_value =
{
    CIM_Job_RecoveryAction_ValueMap_qual_data_value,
    MI_COUNT(CIM_Job_RecoveryAction_ValueMap_qual_data_value),
};

static MI_CONST MI_Qualifier CIM_Job_RecoveryAction_ValueMap_qual =
{
    MI_T("ValueMap"),
    MI_STRINGA,
    MI_FLAG_ENABLEOVERRIDE|MI_FLAG_TOSUBCLASS,
    &CIM_Job_RecoveryAction_ValueMap_qual_value
};

static MI_CONST MI_Char* CIM_Job_RecoveryAction_Values_qual_data_value[] =
{
    MI_T("10"),
    MI_T("11"),
    MI_T("108"),
    MI_T("109"),
    MI_T("110"),
    MI_T("111"),
};

static MI_CONST MI_ConstStringA CIM_Job_RecoveryAction_Values_qual_value =
{
    CIM_Job_RecoveryAction_Values_qual_data_value,
    MI_COUNT(CIM_Job_RecoveryAction_Values_qual_data_value),
};

static MI_CONST MI_Qualifier CIM_Job_RecoveryAction_Values_qual =
{
    MI_T("Values"),
    MI_STRINGA,
    MI_FLAG_ENABLEOVERRIDE|MI_FLAG_TOSUBCLASS|MI_FLAG_TRANSLATABLE,
    &CIM_Job_RecoveryAction_Values_qual_value
};

static MI_CONST MI_Char* CIM_Job_RecoveryAction_ModelCorrespondence_qual_data_value[] =
{
    MI_T("CIM_Job.OtherRecoveryAction"),
};

static MI_CONST MI_ConstStringA CIM_Job_RecoveryAction_ModelCorrespondence_qual_value =
{
    CIM_Job_RecoveryAction_ModelCorrespondence_qual_data_value,
    MI_COUNT(CIM_Job_RecoveryAction_ModelCorrespondence_qual_data_value),
};

static MI_CONST MI_Qualifier CIM_Job_RecoveryAction_ModelCorrespondence_qual =
{
    MI_T("ModelCorrespondence"),
    MI_STRINGA,
    MI_FLAG_ENABLEOVERRIDE|MI_FLAG_TOSUBCLASS,
    &CIM_Job_RecoveryAction_ModelCorrespondence_qual_value
};

static MI_Qualifier MI_CONST* MI_CONST CIM_Job_RecoveryAction_quals[] =
{
    &CIM_Job_RecoveryAction_Description_qual,
    &CIM_Job_RecoveryAction_ValueMap_qual,
    &CIM_Job_RecoveryAction_Values_qual,
    &CIM_Job_RecoveryAction_ModelCorrespondence_qual,
};

/* property CIM_Job.RecoveryAction */
static MI_CONST MI_PropertyDecl CIM_Job_RecoveryAction_prop =
{
    MI_FLAG_PROPERTY|MI_FLAG_READONLY, /* flags */
    0x00726E0E, /* code */
    MI_T("RecoveryAction"), /* name */
    CIM_Job_RecoveryAction_quals, /* qualifiers */
    MI_COUNT(CIM_Job_RecoveryAction_quals), /* numQualifiers */
    MI_UINT16, /* type */
    NULL, /* className */
    0, /* subscript */
    offsetof(CIM_Job, RecoveryAction), /* offset */
    MI_T("CIM_Job"), /* origin */
    MI_T("CIM_Job"), /* propagator */
    NULL,
};

static MI_CONST MI_Char* CIM_Job_OtherRecoveryAction_Description_qual_value = MI_T("112");

static MI_CONST MI_Qualifier CIM_Job_OtherRecoveryAction_Description_qual =
{
    MI_T("Description"),
    MI_STRING,
    MI_FLAG_ENABLEOVERRIDE|MI_FLAG_TOSUBCLASS|MI_FLAG_TRANSLATABLE,
    &CIM_Job_OtherRecoveryAction_Description_qual_value
};

static MI_CONST MI_Char* CIM_Job_OtherRecoveryAction_ModelCorrespondence_qual_data_value[] =
{
    MI_T("CIM_Job.RecoveryAction"),
};

static MI_CONST MI_ConstStringA CIM_Job_OtherRecoveryAction_ModelCorrespondence_qual_value =
{
    CIM_Job_OtherRecoveryAction_ModelCorrespondence_qual_data_value,
    MI_COUNT(CIM_Job_OtherRecoveryAction_ModelCorrespondence_qual_data_value),
};

static MI_CONST MI_Qualifier CIM_Job_OtherRecoveryAction_ModelCorrespondence_qual =
{
    MI_T("ModelCorrespondence"),
    MI_STRINGA,
    MI_FLAG_ENABLEOVERRIDE|MI_FLAG_TOSUBCLASS,
    &CIM_Job_OtherRecoveryAction_ModelCorrespondence_qual_value
};

static MI_Qualifier MI_CONST* MI_CONST CIM_Job_OtherRecoveryAction_quals[] =
{
    &CIM_Job_OtherRecoveryAction_Description_qual,
    &CIM_Job_OtherRecoveryAction_ModelCorrespondence_qual,
};

/* property CIM_Job.OtherRecoveryAction */
static MI_CONST MI_PropertyDecl CIM_Job_OtherRecoveryAction_prop =
{
    MI_FLAG_PROPERTY|MI_FLAG_READONLY, /* flags */
    0x006F6E13, /* code */
    MI_T("OtherRecoveryAction"), /* name */
    CIM_Job_OtherRecoveryAction_quals, /* qualifiers */
    MI_COUNT(CIM_Job_OtherRecoveryAction_quals), /* numQualifiers */
    MI_STRING, /* type */
    NULL, /* className */
    0, /* subscript */
    offsetof(CIM_Job, OtherRecoveryAction), /* offset */
    MI_T("CIM_Job"), /* origin */
    MI_T("CIM_Job"), /* propagator */
    NULL,
};

static MI_PropertyDecl MI_CONST* MI_CONST CIM_Job_props[] =
{
    &CIM_ManagedElement_InstanceID_prop,
    &CIM_ManagedElement_Caption_prop,
    &CIM_ManagedElement_Description_prop,
    &CIM_ManagedElement_ElementName_prop,
    &CIM_ManagedSystemElement_InstallDate_prop,
    &CIM_ManagedSystemElement_Name_prop,
    &CIM_ManagedSystemElement_OperationalStatus_prop,
    &CIM_ManagedSystemElement_StatusDescriptions_prop,
    &CIM_ManagedSystemElement_Status_prop,
    &CIM_ManagedSystemElement_HealthState_prop,
    &CIM_ManagedSystemElement_CommunicationStatus_prop,
    &CIM_ManagedSystemElement_DetailedStatus_prop,
    &CIM_ManagedSystemElement_OperatingStatus_prop,
    &CIM_ManagedSystemElement_PrimaryStatus_prop,
    &CIM_Job_JobStatus_prop,
    &CIM_Job_TimeSubmitted_prop,
    &CIM_Job_ScheduledStartTime_prop,
    &CIM_Job_StartTime_prop,
    &CIM_Job_ElapsedTime_prop,
    &CIM_Job_JobRunTimes_prop,
    &CIM_Job_RunMonth_prop,
    &CIM_Job_RunDay_prop,
    &CIM_Job_RunDayOfWeek_prop,
    &CIM_Job_RunStartInterval_prop,
    &CIM_Job_LocalOrUtcTime_prop,
    &CIM_Job_UntilTime_prop,
    &CIM_Job_Notify_prop,
    &CIM_Job_Owner_prop,
    &CIM_Job_Priority_prop,
    &CIM_Job_PercentComplete_prop,
    &CIM_Job_DeleteOnCompletion_prop,
    &CIM_Job_ErrorCode_prop,
    &CIM_Job_ErrorDescription_prop,
    &CIM_Job_RecoveryAction_prop,
    &CIM_Job_OtherRecoveryAction_prop,
};

static MI_CONST MI_Char* CIM_Job_KillJob_Deprecated_qual_data_value[] =
{
    MI_T("CIM_ConcreteJob.RequestStateChange()"),
};

static MI_CONST MI_ConstStringA CIM_Job_KillJob_Deprecated_qual_value =
{
    CIM_Job_KillJob_Deprecated_qual_data_value,
    MI_COUNT(CIM_Job_KillJob_Deprecated_qual_data_value),
};

static MI_CONST MI_Qualifier CIM_Job_KillJob_Deprecated_qual =
{
    MI_T("Deprecated"),
    MI_STRINGA,
    MI_FLAG_ENABLEOVERRIDE|MI_FLAG_RESTRICTED,
    &CIM_Job_KillJob_Deprecated_qual_value
};

static MI_CONST MI_Char* CIM_Job_KillJob_Description_qual_value = MI_T("113");

static MI_CONST MI_Qualifier CIM_Job_KillJob_Description_qual =
{
    MI_T("Description"),
    MI_STRING,
    MI_FLAG_ENABLEOVERRIDE|MI_FLAG_TOSUBCLASS|MI_FLAG_TRANSLATABLE,
    &CIM_Job_KillJob_Description_qual_value
};

static MI_CONST MI_Char* CIM_Job_KillJob_ValueMap_qual_data_value[] =
{
    MI_T("0"),
    MI_T("1"),
    MI_T("2"),
    MI_T("3"),
    MI_T("4"),
    MI_T("6"),
    MI_T("7"),
    MI_T(".."),
    MI_T("32768..65535"),
};

static MI_CONST MI_ConstStringA CIM_Job_KillJob_ValueMap_qual_value =
{
    CIM_Job_KillJob_ValueMap_qual_data_value,
    MI_COUNT(CIM_Job_KillJob_ValueMap_qual_data_value),
};

static MI_CONST MI_Qualifier CIM_Job_KillJob_ValueMap_qual =
{
    MI_T("ValueMap"),
    MI_STRINGA,
    MI_FLAG_ENABLEOVERRIDE|MI_FLAG_TOSUBCLASS,
    &CIM_Job_KillJob_ValueMap_qual_value
};

static MI_CONST MI_Char* CIM_Job_KillJob_Values_qual_data_value[] =
{
    MI_T("114"),
    MI_T("115"),
    MI_T("10"),
    MI_T("116"),
    MI_T("117"),
    MI_T("118"),
    MI_T("119"),
    MI_T("29"),
    MI_T("120"),
};

static MI_CONST MI_ConstStringA CIM_Job_KillJob_Values_qual_value =
{
    CIM_Job_KillJob_Values_qual_data_value,
    MI_COUNT(CIM_Job_KillJob_Values_qual_data_value),
};

static MI_CONST MI_Qualifier CIM_Job_KillJob_Values_qual =
{
    MI_T("Values"),
    MI_STRINGA,
    MI_FLAG_ENABLEOVERRIDE|MI_FLAG_TOSUBCLASS|MI_FLAG_TRANSLATABLE,
    &CIM_Job_KillJob_Values_qual_value
};

static MI_Qualifier MI_CONST* MI_CONST CIM_Job_KillJob_quals[] =
{
    &CIM_Job_KillJob_Deprecated_qual,
    &CIM_Job_KillJob_Description_qual,
    &CIM_Job_KillJob_ValueMap_qual,
    &CIM_Job_KillJob_Values_qual,
};

static MI_CONST MI_Boolean CIM_Job_KillJob_DeleteOnKill_In_qual_value = 1;

static MI_CONST MI_Qualifier CIM_Job_KillJob_DeleteOnKill_In_qual =
{
    MI_T("In"),
    MI_BOOLEAN,
    MI_FLAG_DISABLEOVERRIDE|MI_FLAG_TOSUBCLASS,
    &CIM_Job_KillJob_DeleteOnKill_In_qual_value
};

static MI_CONST MI_Char* CIM_Job_KillJob_DeleteOnKill_Description_qual_value = MI_T("121");

static MI_CONST MI_Qualifier CIM_Job_KillJob_DeleteOnKill_Description_qual =
{
    MI_T("Description"),
    MI_STRING,
    MI_FLAG_ENABLEOVERRIDE|MI_FLAG_TOSUBCLASS|MI_FLAG_TRANSLATABLE,
    &CIM_Job_KillJob_DeleteOnKill_Description_qual_value
};

static MI_Qualifier MI_CONST* MI_CONST CIM_Job_KillJob_DeleteOnKill_quals[] =
{
    &CIM_Job_KillJob_DeleteOnKill_In_qual,
    &CIM_Job_KillJob_DeleteOnKill_Description_qual,
};

/* parameter CIM_Job.KillJob(): DeleteOnKill */
static MI_CONST MI_ParameterDecl CIM_Job_KillJob_DeleteOnKill_param =
{
    MI_FLAG_PARAMETER|MI_FLAG_IN, /* flags */
    0x00646C0C, /* code */
    MI_T("DeleteOnKill"), /* name */
    CIM_Job_KillJob_DeleteOnKill_quals, /* qualifiers */
    MI_COUNT(CIM_Job_KillJob_DeleteOnKill_quals), /* numQualifiers */
    MI_BOOLEAN, /* type */
    NULL, /* className */
    0, /* subscript */
    offsetof(CIM_Job_KillJob, DeleteOnKill), /* offset */
};

static MI_CONST MI_Char* CIM_Job_KillJob_MIReturn_Deprecated_qual_data_value[] =
{
    MI_T("CIM_ConcreteJob.RequestStateChange()"),
};

static MI_CONST MI_ConstStringA CIM_Job_KillJob_MIReturn_Deprecated_qual_value =
{
    CIM_Job_KillJob_MIReturn_Deprecated_qual_data_value,
    MI_COUNT(CIM_Job_KillJob_MIReturn_Deprecated_qual_data_value),
};

static MI_CONST MI_Qualifier CIM_Job_KillJob_MIReturn_Deprecated_qual =
{
    MI_T("Deprecated"),
    MI_STRINGA,
    MI_FLAG_ENABLEOVERRIDE|MI_FLAG_RESTRICTED,
    &CIM_Job_KillJob_MIReturn_Deprecated_qual_value
};

static MI_CONST MI_Char* CIM_Job_KillJob_MIReturn_Description_qual_value = MI_T("113");

static MI_CONST MI_Qualifier CIM_Job_KillJob_MIReturn_Description_qual =
{
    MI_T("Description"),
    MI_STRING,
    MI_FLAG_ENABLEOVERRIDE|MI_FLAG_TOSUBCLASS|MI_FLAG_TRANSLATABLE,
    &CIM_Job_KillJob_MIReturn_Description_qual_value
};

static MI_CONST MI_Char* CIM_Job_KillJob_MIReturn_ValueMap_qual_data_value[] =
{
    MI_T("0"),
    MI_T("1"),
    MI_T("2"),
    MI_T("3"),
    MI_T("4"),
    MI_T("6"),
    MI_T("7"),
    MI_T(".."),
    MI_T("32768..65535"),
};

static MI_CONST MI_ConstStringA CIM_Job_KillJob_MIReturn_ValueMap_qual_value =
{
    CIM_Job_KillJob_MIReturn_ValueMap_qual_data_value,
    MI_COUNT(CIM_Job_KillJob_MIReturn_ValueMap_qual_data_value),
};

static MI_CONST MI_Qualifier CIM_Job_KillJob_MIReturn_ValueMap_qual =
{
    MI_T("ValueMap"),
    MI_STRINGA,
    MI_FLAG_ENABLEOVERRIDE|MI_FLAG_TOSUBCLASS,
    &CIM_Job_KillJob_MIReturn_ValueMap_qual_value
};

static MI_CONST MI_Char* CIM_Job_KillJob_MIReturn_Values_qual_data_value[] =
{
    MI_T("114"),
    MI_T("115"),
    MI_T("10"),
    MI_T("116"),
    MI_T("117"),
    MI_T("118"),
    MI_T("119"),
    MI_T("29"),
    MI_T("120"),
};

static MI_CONST MI_ConstStringA CIM_Job_KillJob_MIReturn_Values_qual_value =
{
    CIM_Job_KillJob_MIReturn_Values_qual_data_value,
    MI_COUNT(CIM_Job_KillJob_MIReturn_Values_qual_data_value),
};

static MI_CONST MI_Qualifier CIM_Job_KillJob_MIReturn_Values_qual =
{
    MI_T("Values"),
    MI_STRINGA,
    MI_FLAG_ENABLEOVERRIDE|MI_FLAG_TOSUBCLASS|MI_FLAG_TRANSLATABLE,
    &CIM_Job_KillJob_MIReturn_Values_qual_value
};

static MI_Qualifier MI_CONST* MI_CONST CIM_Job_KillJob_MIReturn_quals[] =
{
    &CIM_Job_KillJob_MIReturn_Deprecated_qual,
    &CIM_Job_KillJob_MIReturn_Description_qual,
    &CIM_Job_KillJob_MIReturn_ValueMap_qual,
    &CIM_Job_KillJob_MIReturn_Values_qual,
};

/* parameter CIM_Job.KillJob(): MIReturn */
static MI_CONST MI_ParameterDecl CIM_Job_KillJob_MIReturn_param =
{
    MI_FLAG_PARAMETER|MI_FLAG_OUT, /* flags */
    0x006D6E08, /* code */
    MI_T("MIReturn"), /* name */
    CIM_Job_KillJob_MIReturn_quals, /* qualifiers */
    MI_COUNT(CIM_Job_KillJob_MIReturn_quals), /* numQualifiers */
    MI_UINT32, /* type */
    NULL, /* className */
    0, /* subscript */
    offsetof(CIM_Job_KillJob, MIReturn), /* offset */
};

static MI_ParameterDecl MI_CONST* MI_CONST CIM_Job_KillJob_params[] =
{
    &CIM_Job_KillJob_MIReturn_param,
    &CIM_Job_KillJob_DeleteOnKill_param,
};

/* method CIM_Job.KillJob() */
MI_CONST MI_MethodDecl CIM_Job_KillJob_rtti =
{
    MI_FLAG_METHOD, /* flags */
    0x006B6207, /* code */
    MI_T("KillJob"), /* name */
    CIM_Job_KillJob_quals, /* qualifiers */
    MI_COUNT(CIM_Job_KillJob_quals), /* numQualifiers */
    CIM_Job_KillJob_params, /* parameters */
    MI_COUNT(CIM_Job_KillJob_params), /* numParameters */
    sizeof(CIM_Job_KillJob), /* size */
    MI_UINT32, /* returnType */
    MI_T("CIM_Job"), /* origin */
    MI_T("CIM_Job"), /* propagator */
    &schemaDecl, /* schema */
    NULL, /* method */
};

static MI_MethodDecl MI_CONST* MI_CONST CIM_Job_meths[] =
{
    &CIM_Job_KillJob_rtti,
};

static MI_CONST MI_Char* CIM_Job_UMLPackagePath_qual_value = MI_T("CIM::Core::CoreElements");

static MI_CONST MI_Qualifier CIM_Job_UMLPackagePath_qual =
{
    MI_T("UMLPackagePath"),
    MI_STRING,
    MI_FLAG_ENABLEOVERRIDE|MI_FLAG_TOSUBCLASS,
    &CIM_Job_UMLPackagePath_qual_value
};

static MI_CONST MI_Char* CIM_Job_Description_qual_value = MI_T("122");

static MI_CONST MI_Qualifier CIM_Job_Description_qual =
{
    MI_T("Description"),
    MI_STRING,
    MI_FLAG_ENABLEOVERRIDE|MI_FLAG_TOSUBCLASS|MI_FLAG_TRANSLATABLE,
    &CIM_Job_Description_qual_value
};

static MI_CONST MI_Boolean CIM_Job_Abstract_qual_value = 1;

static MI_CONST MI_Qualifier CIM_Job_Abstract_qual =
{
    MI_T("Abstract"),
    MI_BOOLEAN,
    MI_FLAG_ENABLEOVERRIDE|MI_FLAG_RESTRICTED,
    &CIM_Job_Abstract_qual_value
};

static MI_CONST MI_Char* CIM_Job_Version_qual_value = MI_T("123");

static MI_CONST MI_Qualifier CIM_Job_Version_qual =
{
    MI_T("Version"),
    MI_STRING,
    MI_FLAG_ENABLEOVERRIDE|MI_FLAG_TRANSLATABLE|MI_FLAG_RESTRICTED,
    &CIM_Job_Version_qual_value
};

static MI_Qualifier MI_CONST* MI_CONST CIM_Job_quals[] =
{
    &CIM_Job_UMLPackagePath_qual,
    &CIM_Job_Description_qual,
    &CIM_Job_Abstract_qual,
    &CIM_Job_Version_qual,
};

/* class CIM_Job */
MI_CONST MI_ClassDecl CIM_Job_rtti =
{
    MI_FLAG_CLASS|MI_FLAG_ABSTRACT, /* flags */
    0x00636207, /* code */
    MI_T("CIM_Job"), /* name */
    CIM_Job_quals, /* qualifiers */
    MI_COUNT(CIM_Job_quals), /* numQualifiers */
    CIM_Job_props, /* properties */
    MI_COUNT(CIM_Job_props), /* numProperties */
    sizeof(CIM_Job), /* size */
    MI_T("CIM_LogicalElement"), /* superClass */
    &CIM_LogicalElement_rtti, /* superClassDecl */
    CIM_Job_meths, /* methods */
    MI_COUNT(CIM_Job_meths), /* numMethods */
    &schemaDecl, /* schema */
    NULL, /* functions */
    NULL /* owningClass */
};

/*
**==============================================================================
**
** CIM_Error
**
**==============================================================================
*/

static MI_CONST MI_Char* CIM_Error_ErrorType_Description_qual_value = MI_T("124");

static MI_CONST MI_Qualifier CIM_Error_ErrorType_Description_qual =
{
    MI_T("Description"),
    MI_STRING,
    MI_FLAG_ENABLEOVERRIDE|MI_FLAG_TOSUBCLASS|MI_FLAG_TRANSLATABLE,
    &CIM_Error_ErrorType_Description_qual_value
};

static MI_CONST MI_Char* CIM_Error_ErrorType_ValueMap_qual_data_value[] =
{
    MI_T("0"),
    MI_T("1"),
    MI_T("2"),
    MI_T("3"),
    MI_T("4"),
    MI_T("5"),
    MI_T("6"),
    MI_T("7"),
    MI_T("8"),
    MI_T("9"),
    MI_T("10"),
    MI_T(".."),
};

static MI_CONST MI_ConstStringA CIM_Error_ErrorType_ValueMap_qual_value =
{
    CIM_Error_ErrorType_ValueMap_qual_data_value,
    MI_COUNT(CIM_Error_ErrorType_ValueMap_qual_data_value),
};

static MI_CONST MI_Qualifier CIM_Error_ErrorType_ValueMap_qual =
{
    MI_T("ValueMap"),
    MI_STRINGA,
    MI_FLAG_ENABLEOVERRIDE|MI_FLAG_TOSUBCLASS,
    &CIM_Error_ErrorType_ValueMap_qual_value
};

static MI_CONST MI_Char* CIM_Error_ErrorType_Values_qual_data_value[] =
{
    MI_T("10"),
    MI_T("11"),
    MI_T("125"),
    MI_T("126"),
    MI_T("127"),
    MI_T("128"),
    MI_T("129"),
    MI_T("130"),
    MI_T("131"),
    MI_T("132"),
    MI_T("133"),
    MI_T("29"),
};

static MI_CONST MI_ConstStringA CIM_Error_ErrorType_Values_qual_value =
{
    CIM_Error_ErrorType_Values_qual_data_value,
    MI_COUNT(CIM_Error_ErrorType_Values_qual_data_value),
};

static MI_CONST MI_Qualifier CIM_Error_ErrorType_Values_qual =
{
    MI_T("Values"),
    MI_STRINGA,
    MI_FLAG_ENABLEOVERRIDE|MI_FLAG_TOSUBCLASS|MI_FLAG_TRANSLATABLE,
    &CIM_Error_ErrorType_Values_qual_value
};

static MI_CONST MI_Char* CIM_Error_ErrorType_ModelCorrespondence_qual_data_value[] =
{
    MI_T("CIM_Error.OtherErrorType"),
};

static MI_CONST MI_ConstStringA CIM_Error_ErrorType_ModelCorrespondence_qual_value =
{
    CIM_Error_ErrorType_ModelCorrespondence_qual_data_value,
    MI_COUNT(CIM_Error_ErrorType_ModelCorrespondence_qual_data_value),
};

static MI_CONST MI_Qualifier CIM_Error_ErrorType_ModelCorrespondence_qual =
{
    MI_T("ModelCorrespondence"),
    MI_STRINGA,
    MI_FLAG_ENABLEOVERRIDE|MI_FLAG_TOSUBCLASS,
    &CIM_Error_ErrorType_ModelCorrespondence_qual_value
};

static MI_Qualifier MI_CONST* MI_CONST CIM_Error_ErrorType_quals[] =
{
    &CIM_Error_ErrorType_Description_qual,
    &CIM_Error_ErrorType_ValueMap_qual,
    &CIM_Error_ErrorType_Values_qual,
    &CIM_Error_ErrorType_ModelCorrespondence_qual,
};

/* property CIM_Error.ErrorType */
static MI_CONST MI_PropertyDecl CIM_Error_ErrorType_prop =
{
    MI_FLAG_PROPERTY|MI_FLAG_READONLY, /* flags */
    0x00656509, /* code */
    MI_T("ErrorType"), /* name */
    CIM_Error_ErrorType_quals, /* qualifiers */
    MI_COUNT(CIM_Error_ErrorType_quals), /* numQualifiers */
    MI_UINT16, /* type */
    NULL, /* className */
    0, /* subscript */
    offsetof(CIM_Error, ErrorType), /* offset */
    MI_T("CIM_Error"), /* origin */
    MI_T("CIM_Error"), /* propagator */
    NULL,
};

static MI_CONST MI_Char* CIM_Error_OtherErrorType_Description_qual_value = MI_T("134");

static MI_CONST MI_Qualifier CIM_Error_OtherErrorType_Description_qual =
{
    MI_T("Description"),
    MI_STRING,
    MI_FLAG_ENABLEOVERRIDE|MI_FLAG_TOSUBCLASS|MI_FLAG_TRANSLATABLE,
    &CIM_Error_OtherErrorType_Description_qual_value
};

static MI_CONST MI_Char* CIM_Error_OtherErrorType_ModelCorrespondence_qual_data_value[] =
{
    MI_T("CIM_Error.ErrorType"),
};

static MI_CONST MI_ConstStringA CIM_Error_OtherErrorType_ModelCorrespondence_qual_value =
{
    CIM_Error_OtherErrorType_ModelCorrespondence_qual_data_value,
    MI_COUNT(CIM_Error_OtherErrorType_ModelCorrespondence_qual_data_value),
};

static MI_CONST MI_Qualifier CIM_Error_OtherErrorType_ModelCorrespondence_qual =
{
    MI_T("ModelCorrespondence"),
    MI_STRINGA,
    MI_FLAG_ENABLEOVERRIDE|MI_FLAG_TOSUBCLASS,
    &CIM_Error_OtherErrorType_ModelCorrespondence_qual_value
};

static MI_Qualifier MI_CONST* MI_CONST CIM_Error_OtherErrorType_quals[] =
{
    &CIM_Error_OtherErrorType_Description_qual,
    &CIM_Error_OtherErrorType_ModelCorrespondence_qual,
};

/* property CIM_Error.OtherErrorType */
static MI_CONST MI_PropertyDecl CIM_Error_OtherErrorType_prop =
{
    MI_FLAG_PROPERTY|MI_FLAG_READONLY, /* flags */
    0x006F650E, /* code */
    MI_T("OtherErrorType"), /* name */
    CIM_Error_OtherErrorType_quals, /* qualifiers */
    MI_COUNT(CIM_Error_OtherErrorType_quals), /* numQualifiers */
    MI_STRING, /* type */
    NULL, /* className */
    0, /* subscript */
    offsetof(CIM_Error, OtherErrorType), /* offset */
    MI_T("CIM_Error"), /* origin */
    MI_T("CIM_Error"), /* propagator */
    NULL,
};

static MI_CONST MI_Char* CIM_Error_OwningEntity_Description_qual_value = MI_T("135");

static MI_CONST MI_Qualifier CIM_Error_OwningEntity_Description_qual =
{
    MI_T("Description"),
    MI_STRING,
    MI_FLAG_ENABLEOVERRIDE|MI_FLAG_TOSUBCLASS|MI_FLAG_TRANSLATABLE,
    &CIM_Error_OwningEntity_Description_qual_value
};

static MI_Qualifier MI_CONST* MI_CONST CIM_Error_OwningEntity_quals[] =
{
    &CIM_Error_OwningEntity_Description_qual,
};

/* property CIM_Error.OwningEntity */
static MI_CONST MI_PropertyDecl CIM_Error_OwningEntity_prop =
{
    MI_FLAG_PROPERTY|MI_FLAG_READONLY, /* flags */
    0x006F790C, /* code */
    MI_T("OwningEntity"), /* name */
    CIM_Error_OwningEntity_quals, /* qualifiers */
    MI_COUNT(CIM_Error_OwningEntity_quals), /* numQualifiers */
    MI_STRING, /* type */
    NULL, /* className */
    0, /* subscript */
    offsetof(CIM_Error, OwningEntity), /* offset */
    MI_T("CIM_Error"), /* origin */
    MI_T("CIM_Error"), /* propagator */
    NULL,
};

static MI_CONST MI_Boolean CIM_Error_MessageID_Required_qual_value = 1;

static MI_CONST MI_Qualifier CIM_Error_MessageID_Required_qual =
{
    MI_T("Required"),
    MI_BOOLEAN,
    MI_FLAG_DISABLEOVERRIDE|MI_FLAG_TOSUBCLASS,
    &CIM_Error_MessageID_Required_qual_value
};

static MI_CONST MI_Char* CIM_Error_MessageID_Description_qual_value = MI_T("136");

static MI_CONST MI_Qualifier CIM_Error_MessageID_Description_qual =
{
    MI_T("Description"),
    MI_STRING,
    MI_FLAG_ENABLEOVERRIDE|MI_FLAG_TOSUBCLASS|MI_FLAG_TRANSLATABLE,
    &CIM_Error_MessageID_Description_qual_value
};

static MI_CONST MI_Char* CIM_Error_MessageID_ModelCorrespondence_qual_data_value[] =
{
    MI_T("CIM_Error.Message"),
    MI_T("CIM_Error.MessageArguments"),
};

static MI_CONST MI_ConstStringA CIM_Error_MessageID_ModelCorrespondence_qual_value =
{
    CIM_Error_MessageID_ModelCorrespondence_qual_data_value,
    MI_COUNT(CIM_Error_MessageID_ModelCorrespondence_qual_data_value),
};

static MI_CONST MI_Qualifier CIM_Error_MessageID_ModelCorrespondence_qual =
{
    MI_T("ModelCorrespondence"),
    MI_STRINGA,
    MI_FLAG_ENABLEOVERRIDE|MI_FLAG_TOSUBCLASS,
    &CIM_Error_MessageID_ModelCorrespondence_qual_value
};

static MI_Qualifier MI_CONST* MI_CONST CIM_Error_MessageID_quals[] =
{
    &CIM_Error_MessageID_Required_qual,
    &CIM_Error_MessageID_Description_qual,
    &CIM_Error_MessageID_ModelCorrespondence_qual,
};

/* property CIM_Error.MessageID */
static MI_CONST MI_PropertyDecl CIM_Error_MessageID_prop =
{
    MI_FLAG_PROPERTY|MI_FLAG_REQUIRED|MI_FLAG_READONLY, /* flags */
    0x006D6409, /* code */
    MI_T("MessageID"), /* name */
    CIM_Error_MessageID_quals, /* qualifiers */
    MI_COUNT(CIM_Error_MessageID_quals), /* numQualifiers */
    MI_STRING, /* type */
    NULL, /* className */
    0, /* subscript */
    offsetof(CIM_Error, MessageID), /* offset */
    MI_T("CIM_Error"), /* origin */
    MI_T("CIM_Error"), /* propagator */
    NULL,
};

static MI_CONST MI_Char* CIM_Error_Message_Description_qual_value = MI_T("137");

static MI_CONST MI_Qualifier CIM_Error_Message_Description_qual =
{
    MI_T("Description"),
    MI_STRING,
    MI_FLAG_ENABLEOVERRIDE|MI_FLAG_TOSUBCLASS|MI_FLAG_TRANSLATABLE,
    &CIM_Error_Message_Description_qual_value
};

static MI_CONST MI_Char* CIM_Error_Message_ModelCorrespondence_qual_data_value[] =
{
    MI_T("CIM_Error.MessageID"),
    MI_T("CIM_Error.MessageArguments"),
};

static MI_CONST MI_ConstStringA CIM_Error_Message_ModelCorrespondence_qual_value =
{
    CIM_Error_Message_ModelCorrespondence_qual_data_value,
    MI_COUNT(CIM_Error_Message_ModelCorrespondence_qual_data_value),
};

static MI_CONST MI_Qualifier CIM_Error_Message_ModelCorrespondence_qual =
{
    MI_T("ModelCorrespondence"),
    MI_STRINGA,
    MI_FLAG_ENABLEOVERRIDE|MI_FLAG_TOSUBCLASS,
    &CIM_Error_Message_ModelCorrespondence_qual_value
};

static MI_Qualifier MI_CONST* MI_CONST CIM_Error_Message_quals[] =
{
    &CIM_Error_Message_Description_qual,
    &CIM_Error_Message_ModelCorrespondence_qual,
};

/* property CIM_Error.Message */
static MI_CONST MI_PropertyDecl CIM_Error_Message_prop =
{
    MI_FLAG_PROPERTY|MI_FLAG_READONLY, /* flags */
    0x006D6507, /* code */
    MI_T("Message"), /* name */
    CIM_Error_Message_quals, /* qualifiers */
    MI_COUNT(CIM_Error_Message_quals), /* numQualifiers */
    MI_STRING, /* type */
    NULL, /* className */
    0, /* subscript */
    offsetof(CIM_Error, Message), /* offset */
    MI_T("CIM_Error"), /* origin */
    MI_T("CIM_Error"), /* propagator */
    NULL,
};

static MI_CONST MI_Char* CIM_Error_MessageArguments_Description_qual_value = MI_T("138");

static MI_CONST MI_Qualifier CIM_Error_MessageArguments_Description_qual =
{
    MI_T("Description"),
    MI_STRING,
    MI_FLAG_ENABLEOVERRIDE|MI_FLAG_TOSUBCLASS|MI_FLAG_TRANSLATABLE,
    &CIM_Error_MessageArguments_Description_qual_value
};

static MI_CONST MI_Char* CIM_Error_MessageArguments_ModelCorrespondence_qual_data_value[] =
{
    MI_T("CIM_Error.MessageID"),
    MI_T("CIM_Error.Message"),
};

static MI_CONST MI_ConstStringA CIM_Error_MessageArguments_ModelCorrespondence_qual_value =
{
    CIM_Error_MessageArguments_ModelCorrespondence_qual_data_value,
    MI_COUNT(CIM_Error_MessageArguments_ModelCorrespondence_qual_data_value),
};

static MI_CONST MI_Qualifier CIM_Error_MessageArguments_ModelCorrespondence_qual =
{
    MI_T("ModelCorrespondence"),
    MI_STRINGA,
    MI_FLAG_ENABLEOVERRIDE|MI_FLAG_TOSUBCLASS,
    &CIM_Error_MessageArguments_ModelCorrespondence_qual_value
};

static MI_Qualifier MI_CONST* MI_CONST CIM_Error_MessageArguments_quals[] =
{
    &CIM_Error_MessageArguments_Description_qual,
    &CIM_Error_MessageArguments_ModelCorrespondence_qual,
};

/* property CIM_Error.MessageArguments */
static MI_CONST MI_PropertyDecl CIM_Error_MessageArguments_prop =
{
    MI_FLAG_PROPERTY|MI_FLAG_READONLY, /* flags */
    0x006D7310, /* code */
    MI_T("MessageArguments"), /* name */
    CIM_Error_MessageArguments_quals, /* qualifiers */
    MI_COUNT(CIM_Error_MessageArguments_quals), /* numQualifiers */
    MI_STRINGA, /* type */
    NULL, /* className */
    0, /* subscript */
    offsetof(CIM_Error, MessageArguments), /* offset */
    MI_T("CIM_Error"), /* origin */
    MI_T("CIM_Error"), /* propagator */
    NULL,
};

static MI_CONST MI_Char* CIM_Error_PerceivedSeverity_Description_qual_value = MI_T("139");

static MI_CONST MI_Qualifier CIM_Error_PerceivedSeverity_Description_qual =
{
    MI_T("Description"),
    MI_STRING,
    MI_FLAG_ENABLEOVERRIDE|MI_FLAG_TOSUBCLASS|MI_FLAG_TRANSLATABLE,
    &CIM_Error_PerceivedSeverity_Description_qual_value
};

static MI_CONST MI_Char* CIM_Error_PerceivedSeverity_ValueMap_qual_data_value[] =
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

static MI_CONST MI_ConstStringA CIM_Error_PerceivedSeverity_ValueMap_qual_value =
{
    CIM_Error_PerceivedSeverity_ValueMap_qual_data_value,
    MI_COUNT(CIM_Error_PerceivedSeverity_ValueMap_qual_data_value),
};

static MI_CONST MI_Qualifier CIM_Error_PerceivedSeverity_ValueMap_qual =
{
    MI_T("ValueMap"),
    MI_STRINGA,
    MI_FLAG_ENABLEOVERRIDE|MI_FLAG_TOSUBCLASS,
    &CIM_Error_PerceivedSeverity_ValueMap_qual_value
};

static MI_CONST MI_Char* CIM_Error_PerceivedSeverity_Values_qual_data_value[] =
{
    MI_T("10"),
    MI_T("11"),
    MI_T("140"),
    MI_T("34"),
    MI_T("141"),
    MI_T("142"),
    MI_T("143"),
    MI_T("144"),
    MI_T("29"),
};

static MI_CONST MI_ConstStringA CIM_Error_PerceivedSeverity_Values_qual_value =
{
    CIM_Error_PerceivedSeverity_Values_qual_data_value,
    MI_COUNT(CIM_Error_PerceivedSeverity_Values_qual_data_value),
};

static MI_CONST MI_Qualifier CIM_Error_PerceivedSeverity_Values_qual =
{
    MI_T("Values"),
    MI_STRINGA,
    MI_FLAG_ENABLEOVERRIDE|MI_FLAG_TOSUBCLASS|MI_FLAG_TRANSLATABLE,
    &CIM_Error_PerceivedSeverity_Values_qual_value
};

static MI_Qualifier MI_CONST* MI_CONST CIM_Error_PerceivedSeverity_quals[] =
{
    &CIM_Error_PerceivedSeverity_Description_qual,
    &CIM_Error_PerceivedSeverity_ValueMap_qual,
    &CIM_Error_PerceivedSeverity_Values_qual,
};

/* property CIM_Error.PerceivedSeverity */
static MI_CONST MI_PropertyDecl CIM_Error_PerceivedSeverity_prop =
{
    MI_FLAG_PROPERTY|MI_FLAG_READONLY, /* flags */
    0x00707911, /* code */
    MI_T("PerceivedSeverity"), /* name */
    CIM_Error_PerceivedSeverity_quals, /* qualifiers */
    MI_COUNT(CIM_Error_PerceivedSeverity_quals), /* numQualifiers */
    MI_UINT16, /* type */
    NULL, /* className */
    0, /* subscript */
    offsetof(CIM_Error, PerceivedSeverity), /* offset */
    MI_T("CIM_Error"), /* origin */
    MI_T("CIM_Error"), /* propagator */
    NULL,
};

static MI_CONST MI_Char* CIM_Error_ProbableCause_Description_qual_value = MI_T("145");

static MI_CONST MI_Qualifier CIM_Error_ProbableCause_Description_qual =
{
    MI_T("Description"),
    MI_STRING,
    MI_FLAG_ENABLEOVERRIDE|MI_FLAG_TOSUBCLASS|MI_FLAG_TRANSLATABLE,
    &CIM_Error_ProbableCause_Description_qual_value
};

static MI_CONST MI_Char* CIM_Error_ProbableCause_ValueMap_qual_data_value[] =
{
    MI_T("0"),
    MI_T("1"),
    MI_T("2"),
    MI_T("3"),
    MI_T("4"),
    MI_T("5"),
    MI_T("6"),
    MI_T("7"),
    MI_T("8"),
    MI_T("9"),
    MI_T("10"),
    MI_T("11"),
    MI_T("12"),
    MI_T("13"),
    MI_T("14"),
    MI_T("15"),
    MI_T("16"),
    MI_T("17"),
    MI_T("18"),
    MI_T("19"),
    MI_T("20"),
    MI_T("21"),
    MI_T("22"),
    MI_T("23"),
    MI_T("24"),
    MI_T("25"),
    MI_T("26"),
    MI_T("27"),
    MI_T("28"),
    MI_T("29"),
    MI_T("30"),
    MI_T("31"),
    MI_T("32"),
    MI_T("33"),
    MI_T("34"),
    MI_T("35"),
    MI_T("36"),
    MI_T("37"),
    MI_T("38"),
    MI_T("39"),
    MI_T("40"),
    MI_T("41"),
    MI_T("42"),
    MI_T("43"),
    MI_T("44"),
    MI_T("45"),
    MI_T("46"),
    MI_T("47"),
    MI_T("48"),
    MI_T("49"),
    MI_T("50"),
    MI_T("51"),
    MI_T("52"),
    MI_T("53"),
    MI_T("54"),
    MI_T("55"),
    MI_T("56"),
    MI_T("57"),
    MI_T("58"),
    MI_T("59"),
    MI_T("60"),
    MI_T("61"),
    MI_T("62"),
    MI_T("63"),
    MI_T("64"),
    MI_T("65"),
    MI_T("66"),
    MI_T("67"),
    MI_T("68"),
    MI_T("69"),
    MI_T("70"),
    MI_T("71"),
    MI_T("72"),
    MI_T("73"),
    MI_T("74"),
    MI_T("75"),
    MI_T("76"),
    MI_T("77"),
    MI_T("78"),
    MI_T("79"),
    MI_T("80"),
    MI_T("81"),
    MI_T("82"),
    MI_T("83"),
    MI_T("84"),
    MI_T("85"),
    MI_T("86"),
    MI_T("87"),
    MI_T("88"),
    MI_T("89"),
    MI_T("90"),
    MI_T("91"),
    MI_T("92"),
    MI_T("93"),
    MI_T("94"),
    MI_T("95"),
    MI_T("96"),
    MI_T("97"),
    MI_T("98"),
    MI_T("99"),
    MI_T("100"),
    MI_T("101"),
    MI_T("102"),
    MI_T("103"),
    MI_T("104"),
    MI_T("105"),
    MI_T("106"),
    MI_T("107"),
    MI_T("108"),
    MI_T("109"),
    MI_T("110"),
    MI_T("111"),
    MI_T("112"),
    MI_T("113"),
    MI_T("114"),
    MI_T("115"),
    MI_T("116"),
    MI_T("117"),
    MI_T("118"),
    MI_T("119"),
    MI_T("120"),
    MI_T("121"),
    MI_T("122"),
    MI_T("123"),
    MI_T("124"),
    MI_T("125"),
    MI_T("126"),
    MI_T("127"),
    MI_T("128"),
    MI_T("129"),
    MI_T("130"),
    MI_T(".."),
};

static MI_CONST MI_ConstStringA CIM_Error_ProbableCause_ValueMap_qual_value =
{
    CIM_Error_ProbableCause_ValueMap_qual_data_value,
    MI_COUNT(CIM_Error_ProbableCause_ValueMap_qual_data_value),
};

static MI_CONST MI_Qualifier CIM_Error_ProbableCause_ValueMap_qual =
{
    MI_T("ValueMap"),
    MI_STRINGA,
    MI_FLAG_ENABLEOVERRIDE|MI_FLAG_TOSUBCLASS,
    &CIM_Error_ProbableCause_ValueMap_qual_value
};

static MI_CONST MI_Char* CIM_Error_ProbableCause_Values_qual_data_value[] =
{
    MI_T("10"),
    MI_T("11"),
    MI_T("146"),
    MI_T("147"),
    MI_T("148"),
    MI_T("149"),
    MI_T("150"),
    MI_T("151"),
    MI_T("152"),
    MI_T("153"),
    MI_T("154"),
    MI_T("155"),
    MI_T("156"),
    MI_T("157"),
    MI_T("158"),
    MI_T("159"),
    MI_T("160"),
    MI_T("161"),
    MI_T("162"),
    MI_T("163"),
    MI_T("164"),
    MI_T("165"),
    MI_T("166"),
    MI_T("167"),
    MI_T("168"),
    MI_T("169"),
    MI_T("170"),
    MI_T("171"),
    MI_T("172"),
    MI_T("173"),
    MI_T("174"),
    MI_T("175"),
    MI_T("176"),
    MI_T("177"),
    MI_T("178"),
    MI_T("179"),
    MI_T("180"),
    MI_T("181"),
    MI_T("182"),
    MI_T("183"),
    MI_T("184"),
    MI_T("185"),
    MI_T("186"),
    MI_T("187"),
    MI_T("188"),
    MI_T("189"),
    MI_T("190"),
    MI_T("127"),
    MI_T("191"),
    MI_T("192"),
    MI_T("193"),
    MI_T("194"),
    MI_T("195"),
    MI_T("196"),
    MI_T("197"),
    MI_T("198"),
    MI_T("199"),
    MI_T("200"),
    MI_T("201"),
    MI_T("202"),
    MI_T("203"),
    MI_T("204"),
    MI_T("205"),
    MI_T("206"),
    MI_T("207"),
    MI_T("208"),
    MI_T("209"),
    MI_T("210"),
    MI_T("211"),
    MI_T("212"),
    MI_T("213"),
    MI_T("214"),
    MI_T("215"),
    MI_T("216"),
    MI_T("217"),
    MI_T("218"),
    MI_T("219"),
    MI_T("220"),
    MI_T("221"),
    MI_T("222"),
    MI_T("223"),
    MI_T("224"),
    MI_T("225"),
    MI_T("226"),
    MI_T("227"),
    MI_T("228"),
    MI_T("229"),
    MI_T("230"),
    MI_T("231"),
    MI_T("232"),
    MI_T("233"),
    MI_T("234"),
    MI_T("235"),
    MI_T("236"),
    MI_T("237"),
    MI_T("238"),
    MI_T("239"),
    MI_T("240"),
    MI_T("241"),
    MI_T("242"),
    MI_T("243"),
    MI_T("244"),
    MI_T("245"),
    MI_T("246"),
    MI_T("247"),
    MI_T("248"),
    MI_T("249"),
    MI_T("250"),
    MI_T("251"),
    MI_T("252"),
    MI_T("253"),
    MI_T("116"),
    MI_T("254"),
    MI_T("255"),
    MI_T("256"),
    MI_T("257"),
    MI_T("258"),
    MI_T("259"),
    MI_T("260"),
    MI_T("261"),
    MI_T("262"),
    MI_T("263"),
    MI_T("264"),
    MI_T("265"),
    MI_T("266"),
    MI_T("267"),
    MI_T("268"),
    MI_T("269"),
    MI_T("270"),
    MI_T("271"),
    MI_T("272"),
    MI_T("29"),
};

static MI_CONST MI_ConstStringA CIM_Error_ProbableCause_Values_qual_value =
{
    CIM_Error_ProbableCause_Values_qual_data_value,
    MI_COUNT(CIM_Error_ProbableCause_Values_qual_data_value),
};

static MI_CONST MI_Qualifier CIM_Error_ProbableCause_Values_qual =
{
    MI_T("Values"),
    MI_STRINGA,
    MI_FLAG_ENABLEOVERRIDE|MI_FLAG_TOSUBCLASS|MI_FLAG_TRANSLATABLE,
    &CIM_Error_ProbableCause_Values_qual_value
};

static MI_CONST MI_Char* CIM_Error_ProbableCause_ModelCorrespondence_qual_data_value[] =
{
    MI_T("CIM_Error.ProbableCauseDescription"),
};

static MI_CONST MI_ConstStringA CIM_Error_ProbableCause_ModelCorrespondence_qual_value =
{
    CIM_Error_ProbableCause_ModelCorrespondence_qual_data_value,
    MI_COUNT(CIM_Error_ProbableCause_ModelCorrespondence_qual_data_value),
};

static MI_CONST MI_Qualifier CIM_Error_ProbableCause_ModelCorrespondence_qual =
{
    MI_T("ModelCorrespondence"),
    MI_STRINGA,
    MI_FLAG_ENABLEOVERRIDE|MI_FLAG_TOSUBCLASS,
    &CIM_Error_ProbableCause_ModelCorrespondence_qual_value
};

static MI_Qualifier MI_CONST* MI_CONST CIM_Error_ProbableCause_quals[] =
{
    &CIM_Error_ProbableCause_Description_qual,
    &CIM_Error_ProbableCause_ValueMap_qual,
    &CIM_Error_ProbableCause_Values_qual,
    &CIM_Error_ProbableCause_ModelCorrespondence_qual,
};

/* property CIM_Error.ProbableCause */
static MI_CONST MI_PropertyDecl CIM_Error_ProbableCause_prop =
{
    MI_FLAG_PROPERTY|MI_FLAG_READONLY, /* flags */
    0x0070650D, /* code */
    MI_T("ProbableCause"), /* name */
    CIM_Error_ProbableCause_quals, /* qualifiers */
    MI_COUNT(CIM_Error_ProbableCause_quals), /* numQualifiers */
    MI_UINT16, /* type */
    NULL, /* className */
    0, /* subscript */
    offsetof(CIM_Error, ProbableCause), /* offset */
    MI_T("CIM_Error"), /* origin */
    MI_T("CIM_Error"), /* propagator */
    NULL,
};

static MI_CONST MI_Char* CIM_Error_ProbableCauseDescription_Description_qual_value = MI_T("273");

static MI_CONST MI_Qualifier CIM_Error_ProbableCauseDescription_Description_qual =
{
    MI_T("Description"),
    MI_STRING,
    MI_FLAG_ENABLEOVERRIDE|MI_FLAG_TOSUBCLASS|MI_FLAG_TRANSLATABLE,
    &CIM_Error_ProbableCauseDescription_Description_qual_value
};

static MI_CONST MI_Char* CIM_Error_ProbableCauseDescription_ModelCorrespondence_qual_data_value[] =
{
    MI_T("CIM_Error.ProbableCause"),
};

static MI_CONST MI_ConstStringA CIM_Error_ProbableCauseDescription_ModelCorrespondence_qual_value =
{
    CIM_Error_ProbableCauseDescription_ModelCorrespondence_qual_data_value,
    MI_COUNT(CIM_Error_ProbableCauseDescription_ModelCorrespondence_qual_data_value),
};

static MI_CONST MI_Qualifier CIM_Error_ProbableCauseDescription_ModelCorrespondence_qual =
{
    MI_T("ModelCorrespondence"),
    MI_STRINGA,
    MI_FLAG_ENABLEOVERRIDE|MI_FLAG_TOSUBCLASS,
    &CIM_Error_ProbableCauseDescription_ModelCorrespondence_qual_value
};

static MI_Qualifier MI_CONST* MI_CONST CIM_Error_ProbableCauseDescription_quals[] =
{
    &CIM_Error_ProbableCauseDescription_Description_qual,
    &CIM_Error_ProbableCauseDescription_ModelCorrespondence_qual,
};

/* property CIM_Error.ProbableCauseDescription */
static MI_CONST MI_PropertyDecl CIM_Error_ProbableCauseDescription_prop =
{
    MI_FLAG_PROPERTY|MI_FLAG_READONLY, /* flags */
    0x00706E18, /* code */
    MI_T("ProbableCauseDescription"), /* name */
    CIM_Error_ProbableCauseDescription_quals, /* qualifiers */
    MI_COUNT(CIM_Error_ProbableCauseDescription_quals), /* numQualifiers */
    MI_STRING, /* type */
    NULL, /* className */
    0, /* subscript */
    offsetof(CIM_Error, ProbableCauseDescription), /* offset */
    MI_T("CIM_Error"), /* origin */
    MI_T("CIM_Error"), /* propagator */
    NULL,
};

static MI_CONST MI_Char* CIM_Error_RecommendedActions_Description_qual_value = MI_T("274");

static MI_CONST MI_Qualifier CIM_Error_RecommendedActions_Description_qual =
{
    MI_T("Description"),
    MI_STRING,
    MI_FLAG_ENABLEOVERRIDE|MI_FLAG_TOSUBCLASS|MI_FLAG_TRANSLATABLE,
    &CIM_Error_RecommendedActions_Description_qual_value
};

static MI_Qualifier MI_CONST* MI_CONST CIM_Error_RecommendedActions_quals[] =
{
    &CIM_Error_RecommendedActions_Description_qual,
};

/* property CIM_Error.RecommendedActions */
static MI_CONST MI_PropertyDecl CIM_Error_RecommendedActions_prop =
{
    MI_FLAG_PROPERTY|MI_FLAG_READONLY, /* flags */
    0x00727312, /* code */
    MI_T("RecommendedActions"), /* name */
    CIM_Error_RecommendedActions_quals, /* qualifiers */
    MI_COUNT(CIM_Error_RecommendedActions_quals), /* numQualifiers */
    MI_STRINGA, /* type */
    NULL, /* className */
    0, /* subscript */
    offsetof(CIM_Error, RecommendedActions), /* offset */
    MI_T("CIM_Error"), /* origin */
    MI_T("CIM_Error"), /* propagator */
    NULL,
};

static MI_CONST MI_Char* CIM_Error_ErrorSource_Description_qual_value = MI_T("275");

static MI_CONST MI_Qualifier CIM_Error_ErrorSource_Description_qual =
{
    MI_T("Description"),
    MI_STRING,
    MI_FLAG_ENABLEOVERRIDE|MI_FLAG_TOSUBCLASS|MI_FLAG_TRANSLATABLE,
    &CIM_Error_ErrorSource_Description_qual_value
};

static MI_CONST MI_Char* CIM_Error_ErrorSource_ModelCorrespondence_qual_data_value[] =
{
    MI_T("CIM_Error.ErrorSourceFormat"),
};

static MI_CONST MI_ConstStringA CIM_Error_ErrorSource_ModelCorrespondence_qual_value =
{
    CIM_Error_ErrorSource_ModelCorrespondence_qual_data_value,
    MI_COUNT(CIM_Error_ErrorSource_ModelCorrespondence_qual_data_value),
};

static MI_CONST MI_Qualifier CIM_Error_ErrorSource_ModelCorrespondence_qual =
{
    MI_T("ModelCorrespondence"),
    MI_STRINGA,
    MI_FLAG_ENABLEOVERRIDE|MI_FLAG_TOSUBCLASS,
    &CIM_Error_ErrorSource_ModelCorrespondence_qual_value
};

static MI_Qualifier MI_CONST* MI_CONST CIM_Error_ErrorSource_quals[] =
{
    &CIM_Error_ErrorSource_Description_qual,
    &CIM_Error_ErrorSource_ModelCorrespondence_qual,
};

/* property CIM_Error.ErrorSource */
static MI_CONST MI_PropertyDecl CIM_Error_ErrorSource_prop =
{
    MI_FLAG_PROPERTY|MI_FLAG_READONLY, /* flags */
    0x0065650B, /* code */
    MI_T("ErrorSource"), /* name */
    CIM_Error_ErrorSource_quals, /* qualifiers */
    MI_COUNT(CIM_Error_ErrorSource_quals), /* numQualifiers */
    MI_STRING, /* type */
    NULL, /* className */
    0, /* subscript */
    offsetof(CIM_Error, ErrorSource), /* offset */
    MI_T("CIM_Error"), /* origin */
    MI_T("CIM_Error"), /* propagator */
    NULL,
};

static MI_CONST MI_Char* CIM_Error_ErrorSourceFormat_Description_qual_value = MI_T("276");

static MI_CONST MI_Qualifier CIM_Error_ErrorSourceFormat_Description_qual =
{
    MI_T("Description"),
    MI_STRING,
    MI_FLAG_ENABLEOVERRIDE|MI_FLAG_TOSUBCLASS|MI_FLAG_TRANSLATABLE,
    &CIM_Error_ErrorSourceFormat_Description_qual_value
};

static MI_CONST MI_Char* CIM_Error_ErrorSourceFormat_ValueMap_qual_data_value[] =
{
    MI_T("0"),
    MI_T("1"),
    MI_T("2"),
    MI_T(".."),
};

static MI_CONST MI_ConstStringA CIM_Error_ErrorSourceFormat_ValueMap_qual_value =
{
    CIM_Error_ErrorSourceFormat_ValueMap_qual_data_value,
    MI_COUNT(CIM_Error_ErrorSourceFormat_ValueMap_qual_data_value),
};

static MI_CONST MI_Qualifier CIM_Error_ErrorSourceFormat_ValueMap_qual =
{
    MI_T("ValueMap"),
    MI_STRINGA,
    MI_FLAG_ENABLEOVERRIDE|MI_FLAG_TOSUBCLASS,
    &CIM_Error_ErrorSourceFormat_ValueMap_qual_value
};

static MI_CONST MI_Char* CIM_Error_ErrorSourceFormat_Values_qual_data_value[] =
{
    MI_T("10"),
    MI_T("11"),
    MI_T("277"),
    MI_T("29"),
};

static MI_CONST MI_ConstStringA CIM_Error_ErrorSourceFormat_Values_qual_value =
{
    CIM_Error_ErrorSourceFormat_Values_qual_data_value,
    MI_COUNT(CIM_Error_ErrorSourceFormat_Values_qual_data_value),
};

static MI_CONST MI_Qualifier CIM_Error_ErrorSourceFormat_Values_qual =
{
    MI_T("Values"),
    MI_STRINGA,
    MI_FLAG_ENABLEOVERRIDE|MI_FLAG_TOSUBCLASS|MI_FLAG_TRANSLATABLE,
    &CIM_Error_ErrorSourceFormat_Values_qual_value
};

static MI_CONST MI_Char* CIM_Error_ErrorSourceFormat_ModelCorrespondence_qual_data_value[] =
{
    MI_T("CIM_Error.ErrorSource"),
    MI_T("CIM_Error.OtherErrorSourceFormat"),
};

static MI_CONST MI_ConstStringA CIM_Error_ErrorSourceFormat_ModelCorrespondence_qual_value =
{
    CIM_Error_ErrorSourceFormat_ModelCorrespondence_qual_data_value,
    MI_COUNT(CIM_Error_ErrorSourceFormat_ModelCorrespondence_qual_data_value),
};

static MI_CONST MI_Qualifier CIM_Error_ErrorSourceFormat_ModelCorrespondence_qual =
{
    MI_T("ModelCorrespondence"),
    MI_STRINGA,
    MI_FLAG_ENABLEOVERRIDE|MI_FLAG_TOSUBCLASS,
    &CIM_Error_ErrorSourceFormat_ModelCorrespondence_qual_value
};

static MI_Qualifier MI_CONST* MI_CONST CIM_Error_ErrorSourceFormat_quals[] =
{
    &CIM_Error_ErrorSourceFormat_Description_qual,
    &CIM_Error_ErrorSourceFormat_ValueMap_qual,
    &CIM_Error_ErrorSourceFormat_Values_qual,
    &CIM_Error_ErrorSourceFormat_ModelCorrespondence_qual,
};

static MI_CONST MI_Uint16 CIM_Error_ErrorSourceFormat_value = 0;

/* property CIM_Error.ErrorSourceFormat */
static MI_CONST MI_PropertyDecl CIM_Error_ErrorSourceFormat_prop =
{
    MI_FLAG_PROPERTY|MI_FLAG_READONLY, /* flags */
    0x00657411, /* code */
    MI_T("ErrorSourceFormat"), /* name */
    CIM_Error_ErrorSourceFormat_quals, /* qualifiers */
    MI_COUNT(CIM_Error_ErrorSourceFormat_quals), /* numQualifiers */
    MI_UINT16, /* type */
    NULL, /* className */
    0, /* subscript */
    offsetof(CIM_Error, ErrorSourceFormat), /* offset */
    MI_T("CIM_Error"), /* origin */
    MI_T("CIM_Error"), /* propagator */
    &CIM_Error_ErrorSourceFormat_value,
};

static MI_CONST MI_Char* CIM_Error_OtherErrorSourceFormat_Description_qual_value = MI_T("278");

static MI_CONST MI_Qualifier CIM_Error_OtherErrorSourceFormat_Description_qual =
{
    MI_T("Description"),
    MI_STRING,
    MI_FLAG_ENABLEOVERRIDE|MI_FLAG_TOSUBCLASS|MI_FLAG_TRANSLATABLE,
    &CIM_Error_OtherErrorSourceFormat_Description_qual_value
};

static MI_CONST MI_Char* CIM_Error_OtherErrorSourceFormat_ModelCorrespondence_qual_data_value[] =
{
    MI_T("CIM_Error.ErrorSourceFormat"),
};

static MI_CONST MI_ConstStringA CIM_Error_OtherErrorSourceFormat_ModelCorrespondence_qual_value =
{
    CIM_Error_OtherErrorSourceFormat_ModelCorrespondence_qual_data_value,
    MI_COUNT(CIM_Error_OtherErrorSourceFormat_ModelCorrespondence_qual_data_value),
};

static MI_CONST MI_Qualifier CIM_Error_OtherErrorSourceFormat_ModelCorrespondence_qual =
{
    MI_T("ModelCorrespondence"),
    MI_STRINGA,
    MI_FLAG_ENABLEOVERRIDE|MI_FLAG_TOSUBCLASS,
    &CIM_Error_OtherErrorSourceFormat_ModelCorrespondence_qual_value
};

static MI_Qualifier MI_CONST* MI_CONST CIM_Error_OtherErrorSourceFormat_quals[] =
{
    &CIM_Error_OtherErrorSourceFormat_Description_qual,
    &CIM_Error_OtherErrorSourceFormat_ModelCorrespondence_qual,
};

/* property CIM_Error.OtherErrorSourceFormat */
static MI_CONST MI_PropertyDecl CIM_Error_OtherErrorSourceFormat_prop =
{
    MI_FLAG_PROPERTY|MI_FLAG_READONLY, /* flags */
    0x006F7416, /* code */
    MI_T("OtherErrorSourceFormat"), /* name */
    CIM_Error_OtherErrorSourceFormat_quals, /* qualifiers */
    MI_COUNT(CIM_Error_OtherErrorSourceFormat_quals), /* numQualifiers */
    MI_STRING, /* type */
    NULL, /* className */
    0, /* subscript */
    offsetof(CIM_Error, OtherErrorSourceFormat), /* offset */
    MI_T("CIM_Error"), /* origin */
    MI_T("CIM_Error"), /* propagator */
    NULL,
};

static MI_CONST MI_Char* CIM_Error_CIMStatusCode_Description_qual_value = MI_T("279");

static MI_CONST MI_Qualifier CIM_Error_CIMStatusCode_Description_qual =
{
    MI_T("Description"),
    MI_STRING,
    MI_FLAG_ENABLEOVERRIDE|MI_FLAG_TOSUBCLASS|MI_FLAG_TRANSLATABLE,
    &CIM_Error_CIMStatusCode_Description_qual_value
};

static MI_CONST MI_Char* CIM_Error_CIMStatusCode_ValueMap_qual_data_value[] =
{
    MI_T("1"),
    MI_T("2"),
    MI_T("3"),
    MI_T("4"),
    MI_T("5"),
    MI_T("6"),
    MI_T("7"),
    MI_T("8"),
    MI_T("9"),
    MI_T("10"),
    MI_T("11"),
    MI_T("12"),
    MI_T("13"),
    MI_T("14"),
    MI_T("15"),
    MI_T("16"),
    MI_T("17"),
    MI_T("18"),
    MI_T("19"),
    MI_T("20"),
    MI_T("21"),
    MI_T("22"),
    MI_T("23"),
    MI_T("24"),
    MI_T("25"),
    MI_T("26"),
    MI_T("27"),
    MI_T("28"),
    MI_T("29"),
    MI_T(".."),
};

static MI_CONST MI_ConstStringA CIM_Error_CIMStatusCode_ValueMap_qual_value =
{
    CIM_Error_CIMStatusCode_ValueMap_qual_data_value,
    MI_COUNT(CIM_Error_CIMStatusCode_ValueMap_qual_data_value),
};

static MI_CONST MI_Qualifier CIM_Error_CIMStatusCode_ValueMap_qual =
{
    MI_T("ValueMap"),
    MI_STRINGA,
    MI_FLAG_ENABLEOVERRIDE|MI_FLAG_TOSUBCLASS,
    &CIM_Error_CIMStatusCode_ValueMap_qual_value
};

static MI_CONST MI_Char* CIM_Error_CIMStatusCode_Values_qual_data_value[] =
{
    MI_T("280"),
    MI_T("281"),
    MI_T("282"),
    MI_T("283"),
    MI_T("284"),
    MI_T("285"),
    MI_T("286"),
    MI_T("287"),
    MI_T("288"),
    MI_T("289"),
    MI_T("290"),
    MI_T("291"),
    MI_T("292"),
    MI_T("293"),
    MI_T("294"),
    MI_T("295"),
    MI_T("296"),
    MI_T("297"),
    MI_T("298"),
    MI_T("299"),
    MI_T("300"),
    MI_T("301"),
    MI_T("302"),
    MI_T("303"),
    MI_T("304"),
    MI_T("305"),
    MI_T("306"),
    MI_T("307"),
    MI_T("308"),
    MI_T("29"),
};

static MI_CONST MI_ConstStringA CIM_Error_CIMStatusCode_Values_qual_value =
{
    CIM_Error_CIMStatusCode_Values_qual_data_value,
    MI_COUNT(CIM_Error_CIMStatusCode_Values_qual_data_value),
};

static MI_CONST MI_Qualifier CIM_Error_CIMStatusCode_Values_qual =
{
    MI_T("Values"),
    MI_STRINGA,
    MI_FLAG_ENABLEOVERRIDE|MI_FLAG_TOSUBCLASS|MI_FLAG_TRANSLATABLE,
    &CIM_Error_CIMStatusCode_Values_qual_value
};

static MI_CONST MI_Char* CIM_Error_CIMStatusCode_ModelCorrespondence_qual_data_value[] =
{
    MI_T("CIM_Error.CIMStatusCodeDescription"),
};

static MI_CONST MI_ConstStringA CIM_Error_CIMStatusCode_ModelCorrespondence_qual_value =
{
    CIM_Error_CIMStatusCode_ModelCorrespondence_qual_data_value,
    MI_COUNT(CIM_Error_CIMStatusCode_ModelCorrespondence_qual_data_value),
};

static MI_CONST MI_Qualifier CIM_Error_CIMStatusCode_ModelCorrespondence_qual =
{
    MI_T("ModelCorrespondence"),
    MI_STRINGA,
    MI_FLAG_ENABLEOVERRIDE|MI_FLAG_TOSUBCLASS,
    &CIM_Error_CIMStatusCode_ModelCorrespondence_qual_value
};

static MI_Qualifier MI_CONST* MI_CONST CIM_Error_CIMStatusCode_quals[] =
{
    &CIM_Error_CIMStatusCode_Description_qual,
    &CIM_Error_CIMStatusCode_ValueMap_qual,
    &CIM_Error_CIMStatusCode_Values_qual,
    &CIM_Error_CIMStatusCode_ModelCorrespondence_qual,
};

/* property CIM_Error.CIMStatusCode */
static MI_CONST MI_PropertyDecl CIM_Error_CIMStatusCode_prop =
{
    MI_FLAG_PROPERTY|MI_FLAG_READONLY, /* flags */
    0x0063650D, /* code */
    MI_T("CIMStatusCode"), /* name */
    CIM_Error_CIMStatusCode_quals, /* qualifiers */
    MI_COUNT(CIM_Error_CIMStatusCode_quals), /* numQualifiers */
    MI_UINT32, /* type */
    NULL, /* className */
    0, /* subscript */
    offsetof(CIM_Error, CIMStatusCode), /* offset */
    MI_T("CIM_Error"), /* origin */
    MI_T("CIM_Error"), /* propagator */
    NULL,
};

static MI_CONST MI_Char* CIM_Error_CIMStatusCodeDescription_Description_qual_value = MI_T("309");

static MI_CONST MI_Qualifier CIM_Error_CIMStatusCodeDescription_Description_qual =
{
    MI_T("Description"),
    MI_STRING,
    MI_FLAG_ENABLEOVERRIDE|MI_FLAG_TOSUBCLASS|MI_FLAG_TRANSLATABLE,
    &CIM_Error_CIMStatusCodeDescription_Description_qual_value
};

static MI_CONST MI_Char* CIM_Error_CIMStatusCodeDescription_ModelCorrespondence_qual_data_value[] =
{
    MI_T("CIM_Error.CIMStatusCode"),
};

static MI_CONST MI_ConstStringA CIM_Error_CIMStatusCodeDescription_ModelCorrespondence_qual_value =
{
    CIM_Error_CIMStatusCodeDescription_ModelCorrespondence_qual_data_value,
    MI_COUNT(CIM_Error_CIMStatusCodeDescription_ModelCorrespondence_qual_data_value),
};

static MI_CONST MI_Qualifier CIM_Error_CIMStatusCodeDescription_ModelCorrespondence_qual =
{
    MI_T("ModelCorrespondence"),
    MI_STRINGA,
    MI_FLAG_ENABLEOVERRIDE|MI_FLAG_TOSUBCLASS,
    &CIM_Error_CIMStatusCodeDescription_ModelCorrespondence_qual_value
};

static MI_Qualifier MI_CONST* MI_CONST CIM_Error_CIMStatusCodeDescription_quals[] =
{
    &CIM_Error_CIMStatusCodeDescription_Description_qual,
    &CIM_Error_CIMStatusCodeDescription_ModelCorrespondence_qual,
};

/* property CIM_Error.CIMStatusCodeDescription */
static MI_CONST MI_PropertyDecl CIM_Error_CIMStatusCodeDescription_prop =
{
    MI_FLAG_PROPERTY|MI_FLAG_READONLY, /* flags */
    0x00636E18, /* code */
    MI_T("CIMStatusCodeDescription"), /* name */
    CIM_Error_CIMStatusCodeDescription_quals, /* qualifiers */
    MI_COUNT(CIM_Error_CIMStatusCodeDescription_quals), /* numQualifiers */
    MI_STRING, /* type */
    NULL, /* className */
    0, /* subscript */
    offsetof(CIM_Error, CIMStatusCodeDescription), /* offset */
    MI_T("CIM_Error"), /* origin */
    MI_T("CIM_Error"), /* propagator */
    NULL,
};

static MI_PropertyDecl MI_CONST* MI_CONST CIM_Error_props[] =
{
    &CIM_Error_ErrorType_prop,
    &CIM_Error_OtherErrorType_prop,
    &CIM_Error_OwningEntity_prop,
    &CIM_Error_MessageID_prop,
    &CIM_Error_Message_prop,
    &CIM_Error_MessageArguments_prop,
    &CIM_Error_PerceivedSeverity_prop,
    &CIM_Error_ProbableCause_prop,
    &CIM_Error_ProbableCauseDescription_prop,
    &CIM_Error_RecommendedActions_prop,
    &CIM_Error_ErrorSource_prop,
    &CIM_Error_ErrorSourceFormat_prop,
    &CIM_Error_OtherErrorSourceFormat_prop,
    &CIM_Error_CIMStatusCode_prop,
    &CIM_Error_CIMStatusCodeDescription_prop,
};

static MI_CONST MI_Boolean CIM_Error_Indication_qual_value = 1;

static MI_CONST MI_Qualifier CIM_Error_Indication_qual =
{
    MI_T("Indication"),
    MI_BOOLEAN,
    MI_FLAG_DISABLEOVERRIDE|MI_FLAG_TOSUBCLASS,
    &CIM_Error_Indication_qual_value
};

static MI_CONST MI_Char* CIM_Error_Version_qual_value = MI_T("310");

static MI_CONST MI_Qualifier CIM_Error_Version_qual =
{
    MI_T("Version"),
    MI_STRING,
    MI_FLAG_ENABLEOVERRIDE|MI_FLAG_TRANSLATABLE|MI_FLAG_RESTRICTED,
    &CIM_Error_Version_qual_value
};

static MI_CONST MI_Boolean CIM_Error_Exception_qual_value = 1;

static MI_CONST MI_Qualifier CIM_Error_Exception_qual =
{
    MI_T("Exception"),
    MI_BOOLEAN,
    MI_FLAG_DISABLEOVERRIDE|MI_FLAG_TOSUBCLASS,
    &CIM_Error_Exception_qual_value
};

static MI_CONST MI_Char* CIM_Error_UMLPackagePath_qual_value = MI_T("CIM::Interop");

static MI_CONST MI_Qualifier CIM_Error_UMLPackagePath_qual =
{
    MI_T("UMLPackagePath"),
    MI_STRING,
    MI_FLAG_ENABLEOVERRIDE|MI_FLAG_TOSUBCLASS,
    &CIM_Error_UMLPackagePath_qual_value
};

static MI_CONST MI_Char* CIM_Error_Description_qual_value = MI_T("311");

static MI_CONST MI_Qualifier CIM_Error_Description_qual =
{
    MI_T("Description"),
    MI_STRING,
    MI_FLAG_ENABLEOVERRIDE|MI_FLAG_TOSUBCLASS|MI_FLAG_TRANSLATABLE,
    &CIM_Error_Description_qual_value
};

static MI_Qualifier MI_CONST* MI_CONST CIM_Error_quals[] =
{
    &CIM_Error_Indication_qual,
    &CIM_Error_Version_qual,
    &CIM_Error_Exception_qual,
    &CIM_Error_UMLPackagePath_qual,
    &CIM_Error_Description_qual,
};

/* class CIM_Error */
MI_CONST MI_ClassDecl CIM_Error_rtti =
{
    MI_FLAG_CLASS|MI_FLAG_INDICATION, /* flags */
    0x00637209, /* code */
    MI_T("CIM_Error"), /* name */
    CIM_Error_quals, /* qualifiers */
    MI_COUNT(CIM_Error_quals), /* numQualifiers */
    CIM_Error_props, /* properties */
    MI_COUNT(CIM_Error_props), /* numProperties */
    sizeof(CIM_Error), /* size */
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
** CIM_ConcreteJob
**
**==============================================================================
*/

static MI_CONST MI_Char* CIM_ConcreteJob_InstanceID_Description_qual_value = MI_T("312");

static MI_CONST MI_Qualifier CIM_ConcreteJob_InstanceID_Description_qual =
{
    MI_T("Description"),
    MI_STRING,
    MI_FLAG_ENABLEOVERRIDE|MI_FLAG_TOSUBCLASS|MI_FLAG_TRANSLATABLE,
    &CIM_ConcreteJob_InstanceID_Description_qual_value
};

static MI_CONST MI_Boolean CIM_ConcreteJob_InstanceID_Key_qual_value = 1;

static MI_CONST MI_Qualifier CIM_ConcreteJob_InstanceID_Key_qual =
{
    MI_T("Key"),
    MI_BOOLEAN,
    MI_FLAG_DISABLEOVERRIDE|MI_FLAG_TOSUBCLASS,
    &CIM_ConcreteJob_InstanceID_Key_qual_value
};

static MI_CONST MI_Char* CIM_ConcreteJob_InstanceID_Override_qual_value = MI_T("InstanceID");

static MI_CONST MI_Qualifier CIM_ConcreteJob_InstanceID_Override_qual =
{
    MI_T("Override"),
    MI_STRING,
    MI_FLAG_ENABLEOVERRIDE|MI_FLAG_RESTRICTED,
    &CIM_ConcreteJob_InstanceID_Override_qual_value
};

static MI_Qualifier MI_CONST* MI_CONST CIM_ConcreteJob_InstanceID_quals[] =
{
    &CIM_ConcreteJob_InstanceID_Description_qual,
    &CIM_ConcreteJob_InstanceID_Key_qual,
    &CIM_ConcreteJob_InstanceID_Override_qual,
};

/* property CIM_ConcreteJob.InstanceID */
static MI_CONST MI_PropertyDecl CIM_ConcreteJob_InstanceID_prop =
{
    MI_FLAG_PROPERTY|MI_FLAG_KEY|MI_FLAG_READONLY, /* flags */
    0x0069640A, /* code */
    MI_T("InstanceID"), /* name */
    CIM_ConcreteJob_InstanceID_quals, /* qualifiers */
    MI_COUNT(CIM_ConcreteJob_InstanceID_quals), /* numQualifiers */
    MI_STRING, /* type */
    NULL, /* className */
    0, /* subscript */
    offsetof(CIM_ConcreteJob, InstanceID), /* offset */
    MI_T("CIM_ManagedElement"), /* origin */
    MI_T("CIM_ConcreteJob"), /* propagator */
    NULL,
};

static MI_CONST MI_Char* CIM_ConcreteJob_Name_Description_qual_value = MI_T("313");

static MI_CONST MI_Qualifier CIM_ConcreteJob_Name_Description_qual =
{
    MI_T("Description"),
    MI_STRING,
    MI_FLAG_ENABLEOVERRIDE|MI_FLAG_TOSUBCLASS|MI_FLAG_TRANSLATABLE,
    &CIM_ConcreteJob_Name_Description_qual_value
};

static MI_CONST MI_Uint32 CIM_ConcreteJob_Name_MaxLen_qual_value = 1024U;

static MI_CONST MI_Qualifier CIM_ConcreteJob_Name_MaxLen_qual =
{
    MI_T("MaxLen"),
    MI_UINT32,
    MI_FLAG_ENABLEOVERRIDE|MI_FLAG_TOSUBCLASS,
    &CIM_ConcreteJob_Name_MaxLen_qual_value
};

static MI_CONST MI_Boolean CIM_ConcreteJob_Name_Required_qual_value = 1;

static MI_CONST MI_Qualifier CIM_ConcreteJob_Name_Required_qual =
{
    MI_T("Required"),
    MI_BOOLEAN,
    MI_FLAG_DISABLEOVERRIDE|MI_FLAG_TOSUBCLASS,
    &CIM_ConcreteJob_Name_Required_qual_value
};

static MI_CONST MI_Char* CIM_ConcreteJob_Name_Override_qual_value = MI_T("Name");

static MI_CONST MI_Qualifier CIM_ConcreteJob_Name_Override_qual =
{
    MI_T("Override"),
    MI_STRING,
    MI_FLAG_ENABLEOVERRIDE|MI_FLAG_RESTRICTED,
    &CIM_ConcreteJob_Name_Override_qual_value
};

static MI_Qualifier MI_CONST* MI_CONST CIM_ConcreteJob_Name_quals[] =
{
    &CIM_ConcreteJob_Name_Description_qual,
    &CIM_ConcreteJob_Name_MaxLen_qual,
    &CIM_ConcreteJob_Name_Required_qual,
    &CIM_ConcreteJob_Name_Override_qual,
};

/* property CIM_ConcreteJob.Name */
static MI_CONST MI_PropertyDecl CIM_ConcreteJob_Name_prop =
{
    MI_FLAG_PROPERTY|MI_FLAG_REQUIRED|MI_FLAG_READONLY, /* flags */
    0x006E6504, /* code */
    MI_T("Name"), /* name */
    CIM_ConcreteJob_Name_quals, /* qualifiers */
    MI_COUNT(CIM_ConcreteJob_Name_quals), /* numQualifiers */
    MI_STRING, /* type */
    NULL, /* className */
    0, /* subscript */
    offsetof(CIM_ConcreteJob, Name), /* offset */
    MI_T("CIM_ManagedSystemElement"), /* origin */
    MI_T("CIM_ConcreteJob"), /* propagator */
    NULL,
};

static MI_CONST MI_Char* CIM_ConcreteJob_JobState_Description_qual_value = MI_T("314");

static MI_CONST MI_Qualifier CIM_ConcreteJob_JobState_Description_qual =
{
    MI_T("Description"),
    MI_STRING,
    MI_FLAG_ENABLEOVERRIDE|MI_FLAG_TOSUBCLASS|MI_FLAG_TRANSLATABLE,
    &CIM_ConcreteJob_JobState_Description_qual_value
};

static MI_CONST MI_Char* CIM_ConcreteJob_JobState_ValueMap_qual_data_value[] =
{
    MI_T("2"),
    MI_T("3"),
    MI_T("4"),
    MI_T("5"),
    MI_T("6"),
    MI_T("7"),
    MI_T("8"),
    MI_T("9"),
    MI_T("10"),
    MI_T("11"),
    MI_T("12"),
    MI_T("13..32767"),
    MI_T("32768..65535"),
};

static MI_CONST MI_ConstStringA CIM_ConcreteJob_JobState_ValueMap_qual_value =
{
    CIM_ConcreteJob_JobState_ValueMap_qual_data_value,
    MI_COUNT(CIM_ConcreteJob_JobState_ValueMap_qual_data_value),
};

static MI_CONST MI_Qualifier CIM_ConcreteJob_JobState_ValueMap_qual =
{
    MI_T("ValueMap"),
    MI_STRINGA,
    MI_FLAG_ENABLEOVERRIDE|MI_FLAG_TOSUBCLASS,
    &CIM_ConcreteJob_JobState_ValueMap_qual_value
};

static MI_CONST MI_Char* CIM_ConcreteJob_JobState_Values_qual_data_value[] =
{
    MI_T("315"),
    MI_T("18"),
    MI_T("316"),
    MI_T("317"),
    MI_T("50"),
    MI_T("27"),
    MI_T("318"),
    MI_T("319"),
    MI_T("320"),
    MI_T("321"),
    MI_T("322"),
    MI_T("29"),
    MI_T("30"),
};

static MI_CONST MI_ConstStringA CIM_ConcreteJob_JobState_Values_qual_value =
{
    CIM_ConcreteJob_JobState_Values_qual_data_value,
    MI_COUNT(CIM_ConcreteJob_JobState_Values_qual_data_value),
};

static MI_CONST MI_Qualifier CIM_ConcreteJob_JobState_Values_qual =
{
    MI_T("Values"),
    MI_STRINGA,
    MI_FLAG_ENABLEOVERRIDE|MI_FLAG_TOSUBCLASS|MI_FLAG_TRANSLATABLE,
    &CIM_ConcreteJob_JobState_Values_qual_value
};

static MI_Qualifier MI_CONST* MI_CONST CIM_ConcreteJob_JobState_quals[] =
{
    &CIM_ConcreteJob_JobState_Description_qual,
    &CIM_ConcreteJob_JobState_ValueMap_qual,
    &CIM_ConcreteJob_JobState_Values_qual,
};

/* property CIM_ConcreteJob.JobState */
static MI_CONST MI_PropertyDecl CIM_ConcreteJob_JobState_prop =
{
    MI_FLAG_PROPERTY|MI_FLAG_READONLY, /* flags */
    0x006A6508, /* code */
    MI_T("JobState"), /* name */
    CIM_ConcreteJob_JobState_quals, /* qualifiers */
    MI_COUNT(CIM_ConcreteJob_JobState_quals), /* numQualifiers */
    MI_UINT16, /* type */
    NULL, /* className */
    0, /* subscript */
    offsetof(CIM_ConcreteJob, JobState), /* offset */
    MI_T("CIM_ConcreteJob"), /* origin */
    MI_T("CIM_ConcreteJob"), /* propagator */
    NULL,
};

static MI_CONST MI_Char* CIM_ConcreteJob_TimeOfLastStateChange_Description_qual_value = MI_T("323");

static MI_CONST MI_Qualifier CIM_ConcreteJob_TimeOfLastStateChange_Description_qual =
{
    MI_T("Description"),
    MI_STRING,
    MI_FLAG_ENABLEOVERRIDE|MI_FLAG_TOSUBCLASS|MI_FLAG_TRANSLATABLE,
    &CIM_ConcreteJob_TimeOfLastStateChange_Description_qual_value
};

static MI_Qualifier MI_CONST* MI_CONST CIM_ConcreteJob_TimeOfLastStateChange_quals[] =
{
    &CIM_ConcreteJob_TimeOfLastStateChange_Description_qual,
};

/* property CIM_ConcreteJob.TimeOfLastStateChange */
static MI_CONST MI_PropertyDecl CIM_ConcreteJob_TimeOfLastStateChange_prop =
{
    MI_FLAG_PROPERTY|MI_FLAG_READONLY, /* flags */
    0x00746515, /* code */
    MI_T("TimeOfLastStateChange"), /* name */
    CIM_ConcreteJob_TimeOfLastStateChange_quals, /* qualifiers */
    MI_COUNT(CIM_ConcreteJob_TimeOfLastStateChange_quals), /* numQualifiers */
    MI_DATETIME, /* type */
    NULL, /* className */
    0, /* subscript */
    offsetof(CIM_ConcreteJob, TimeOfLastStateChange), /* offset */
    MI_T("CIM_ConcreteJob"), /* origin */
    MI_T("CIM_ConcreteJob"), /* propagator */
    NULL,
};

static MI_CONST MI_Boolean CIM_ConcreteJob_TimeBeforeRemoval_Required_qual_value = 1;

static MI_CONST MI_Qualifier CIM_ConcreteJob_TimeBeforeRemoval_Required_qual =
{
    MI_T("Required"),
    MI_BOOLEAN,
    MI_FLAG_DISABLEOVERRIDE|MI_FLAG_TOSUBCLASS,
    &CIM_ConcreteJob_TimeBeforeRemoval_Required_qual_value
};

static MI_CONST MI_Boolean CIM_ConcreteJob_TimeBeforeRemoval_Write_qual_value = 1;

static MI_CONST MI_Qualifier CIM_ConcreteJob_TimeBeforeRemoval_Write_qual =
{
    MI_T("Write"),
    MI_BOOLEAN,
    MI_FLAG_ENABLEOVERRIDE|MI_FLAG_TOSUBCLASS,
    &CIM_ConcreteJob_TimeBeforeRemoval_Write_qual_value
};

static MI_CONST MI_Char* CIM_ConcreteJob_TimeBeforeRemoval_Description_qual_value = MI_T("324");

static MI_CONST MI_Qualifier CIM_ConcreteJob_TimeBeforeRemoval_Description_qual =
{
    MI_T("Description"),
    MI_STRING,
    MI_FLAG_ENABLEOVERRIDE|MI_FLAG_TOSUBCLASS|MI_FLAG_TRANSLATABLE,
    &CIM_ConcreteJob_TimeBeforeRemoval_Description_qual_value
};

static MI_Qualifier MI_CONST* MI_CONST CIM_ConcreteJob_TimeBeforeRemoval_quals[] =
{
    &CIM_ConcreteJob_TimeBeforeRemoval_Required_qual,
    &CIM_ConcreteJob_TimeBeforeRemoval_Write_qual,
    &CIM_ConcreteJob_TimeBeforeRemoval_Description_qual,
};

static MI_CONST MI_Datetime CIM_ConcreteJob_TimeBeforeRemoval_value = {0,{{0,0,5,0,0}}};

/* property CIM_ConcreteJob.TimeBeforeRemoval */
static MI_CONST MI_PropertyDecl CIM_ConcreteJob_TimeBeforeRemoval_prop =
{
    MI_FLAG_PROPERTY|MI_FLAG_REQUIRED, /* flags */
    0x00746C11, /* code */
    MI_T("TimeBeforeRemoval"), /* name */
    CIM_ConcreteJob_TimeBeforeRemoval_quals, /* qualifiers */
    MI_COUNT(CIM_ConcreteJob_TimeBeforeRemoval_quals), /* numQualifiers */
    MI_DATETIME, /* type */
    NULL, /* className */
    0, /* subscript */
    offsetof(CIM_ConcreteJob, TimeBeforeRemoval), /* offset */
    MI_T("CIM_ConcreteJob"), /* origin */
    MI_T("CIM_ConcreteJob"), /* propagator */
    &CIM_ConcreteJob_TimeBeforeRemoval_value,
};

static MI_PropertyDecl MI_CONST* MI_CONST CIM_ConcreteJob_props[] =
{
    &CIM_ConcreteJob_InstanceID_prop,
    &CIM_ManagedElement_Caption_prop,
    &CIM_ManagedElement_Description_prop,
    &CIM_ManagedElement_ElementName_prop,
    &CIM_ManagedSystemElement_InstallDate_prop,
    &CIM_ConcreteJob_Name_prop,
    &CIM_ManagedSystemElement_OperationalStatus_prop,
    &CIM_ManagedSystemElement_StatusDescriptions_prop,
    &CIM_ManagedSystemElement_Status_prop,
    &CIM_ManagedSystemElement_HealthState_prop,
    &CIM_ManagedSystemElement_CommunicationStatus_prop,
    &CIM_ManagedSystemElement_DetailedStatus_prop,
    &CIM_ManagedSystemElement_OperatingStatus_prop,
    &CIM_ManagedSystemElement_PrimaryStatus_prop,
    &CIM_Job_JobStatus_prop,
    &CIM_Job_TimeSubmitted_prop,
    &CIM_Job_ScheduledStartTime_prop,
    &CIM_Job_StartTime_prop,
    &CIM_Job_ElapsedTime_prop,
    &CIM_Job_JobRunTimes_prop,
    &CIM_Job_RunMonth_prop,
    &CIM_Job_RunDay_prop,
    &CIM_Job_RunDayOfWeek_prop,
    &CIM_Job_RunStartInterval_prop,
    &CIM_Job_LocalOrUtcTime_prop,
    &CIM_Job_UntilTime_prop,
    &CIM_Job_Notify_prop,
    &CIM_Job_Owner_prop,
    &CIM_Job_Priority_prop,
    &CIM_Job_PercentComplete_prop,
    &CIM_Job_DeleteOnCompletion_prop,
    &CIM_Job_ErrorCode_prop,
    &CIM_Job_ErrorDescription_prop,
    &CIM_Job_RecoveryAction_prop,
    &CIM_Job_OtherRecoveryAction_prop,
    &CIM_ConcreteJob_JobState_prop,
    &CIM_ConcreteJob_TimeOfLastStateChange_prop,
    &CIM_ConcreteJob_TimeBeforeRemoval_prop,
};

static MI_CONST MI_Char* CIM_ConcreteJob_RequestStateChange_Description_qual_value = MI_T("325");

static MI_CONST MI_Qualifier CIM_ConcreteJob_RequestStateChange_Description_qual =
{
    MI_T("Description"),
    MI_STRING,
    MI_FLAG_ENABLEOVERRIDE|MI_FLAG_TOSUBCLASS|MI_FLAG_TRANSLATABLE,
    &CIM_ConcreteJob_RequestStateChange_Description_qual_value
};

static MI_CONST MI_Char* CIM_ConcreteJob_RequestStateChange_ValueMap_qual_data_value[] =
{
    MI_T("0"),
    MI_T("1"),
    MI_T("2"),
    MI_T("3"),
    MI_T("4"),
    MI_T("5"),
    MI_T("6"),
    MI_T(".."),
    MI_T("4096"),
    MI_T("4097"),
    MI_T("4098"),
    MI_T("4099"),
    MI_T("4100..32767"),
    MI_T("32768..65535"),
};

static MI_CONST MI_ConstStringA CIM_ConcreteJob_RequestStateChange_ValueMap_qual_value =
{
    CIM_ConcreteJob_RequestStateChange_ValueMap_qual_data_value,
    MI_COUNT(CIM_ConcreteJob_RequestStateChange_ValueMap_qual_data_value),
};

static MI_CONST MI_Qualifier CIM_ConcreteJob_RequestStateChange_ValueMap_qual =
{
    MI_T("ValueMap"),
    MI_STRINGA,
    MI_FLAG_ENABLEOVERRIDE|MI_FLAG_TOSUBCLASS,
    &CIM_ConcreteJob_RequestStateChange_ValueMap_qual_value
};

static MI_CONST MI_Char* CIM_ConcreteJob_RequestStateChange_Values_qual_data_value[] =
{
    MI_T("326"),
    MI_T("115"),
    MI_T("327"),
    MI_T("328"),
    MI_T("117"),
    MI_T("329"),
    MI_T("330"),
    MI_T("29"),
    MI_T("331"),
    MI_T("332"),
    MI_T("333"),
    MI_T("334"),
    MI_T("335"),
    MI_T("120"),
};

static MI_CONST MI_ConstStringA CIM_ConcreteJob_RequestStateChange_Values_qual_value =
{
    CIM_ConcreteJob_RequestStateChange_Values_qual_data_value,
    MI_COUNT(CIM_ConcreteJob_RequestStateChange_Values_qual_data_value),
};

static MI_CONST MI_Qualifier CIM_ConcreteJob_RequestStateChange_Values_qual =
{
    MI_T("Values"),
    MI_STRINGA,
    MI_FLAG_ENABLEOVERRIDE|MI_FLAG_TOSUBCLASS|MI_FLAG_TRANSLATABLE,
    &CIM_ConcreteJob_RequestStateChange_Values_qual_value
};

static MI_Qualifier MI_CONST* MI_CONST CIM_ConcreteJob_RequestStateChange_quals[] =
{
    &CIM_ConcreteJob_RequestStateChange_Description_qual,
    &CIM_ConcreteJob_RequestStateChange_ValueMap_qual,
    &CIM_ConcreteJob_RequestStateChange_Values_qual,
};

static MI_CONST MI_Boolean CIM_ConcreteJob_RequestStateChange_RequestedState_In_qual_value = 1;

static MI_CONST MI_Qualifier CIM_ConcreteJob_RequestStateChange_RequestedState_In_qual =
{
    MI_T("In"),
    MI_BOOLEAN,
    MI_FLAG_DISABLEOVERRIDE|MI_FLAG_TOSUBCLASS,
    &CIM_ConcreteJob_RequestStateChange_RequestedState_In_qual_value
};

static MI_CONST MI_Char* CIM_ConcreteJob_RequestStateChange_RequestedState_Description_qual_value = MI_T("336");

static MI_CONST MI_Qualifier CIM_ConcreteJob_RequestStateChange_RequestedState_Description_qual =
{
    MI_T("Description"),
    MI_STRING,
    MI_FLAG_ENABLEOVERRIDE|MI_FLAG_TOSUBCLASS|MI_FLAG_TRANSLATABLE,
    &CIM_ConcreteJob_RequestStateChange_RequestedState_Description_qual_value
};

static MI_CONST MI_Char* CIM_ConcreteJob_RequestStateChange_RequestedState_ValueMap_qual_data_value[] =
{
    MI_T("2"),
    MI_T("3"),
    MI_T("4"),
    MI_T("5"),
    MI_T("6"),
    MI_T("7..32767"),
    MI_T("32768..65535"),
};

static MI_CONST MI_ConstStringA CIM_ConcreteJob_RequestStateChange_RequestedState_ValueMap_qual_value =
{
    CIM_ConcreteJob_RequestStateChange_RequestedState_ValueMap_qual_data_value,
    MI_COUNT(CIM_ConcreteJob_RequestStateChange_RequestedState_ValueMap_qual_data_value),
};

static MI_CONST MI_Qualifier CIM_ConcreteJob_RequestStateChange_RequestedState_ValueMap_qual =
{
    MI_T("ValueMap"),
    MI_STRINGA,
    MI_FLAG_ENABLEOVERRIDE|MI_FLAG_TOSUBCLASS,
    &CIM_ConcreteJob_RequestStateChange_RequestedState_ValueMap_qual_value
};

static MI_CONST MI_Char* CIM_ConcreteJob_RequestStateChange_RequestedState_Values_qual_data_value[] =
{
    MI_T("337"),
    MI_T("338"),
    MI_T("339"),
    MI_T("340"),
    MI_T("321"),
    MI_T("29"),
    MI_T("30"),
};

static MI_CONST MI_ConstStringA CIM_ConcreteJob_RequestStateChange_RequestedState_Values_qual_value =
{
    CIM_ConcreteJob_RequestStateChange_RequestedState_Values_qual_data_value,
    MI_COUNT(CIM_ConcreteJob_RequestStateChange_RequestedState_Values_qual_data_value),
};

static MI_CONST MI_Qualifier CIM_ConcreteJob_RequestStateChange_RequestedState_Values_qual =
{
    MI_T("Values"),
    MI_STRINGA,
    MI_FLAG_ENABLEOVERRIDE|MI_FLAG_TOSUBCLASS|MI_FLAG_TRANSLATABLE,
    &CIM_ConcreteJob_RequestStateChange_RequestedState_Values_qual_value
};

static MI_Qualifier MI_CONST* MI_CONST CIM_ConcreteJob_RequestStateChange_RequestedState_quals[] =
{
    &CIM_ConcreteJob_RequestStateChange_RequestedState_In_qual,
    &CIM_ConcreteJob_RequestStateChange_RequestedState_Description_qual,
    &CIM_ConcreteJob_RequestStateChange_RequestedState_ValueMap_qual,
    &CIM_ConcreteJob_RequestStateChange_RequestedState_Values_qual,
};

/* parameter CIM_ConcreteJob.RequestStateChange(): RequestedState */
static MI_CONST MI_ParameterDecl CIM_ConcreteJob_RequestStateChange_RequestedState_param =
{
    MI_FLAG_PARAMETER|MI_FLAG_IN, /* flags */
    0x0072650E, /* code */
    MI_T("RequestedState"), /* name */
    CIM_ConcreteJob_RequestStateChange_RequestedState_quals, /* qualifiers */
    MI_COUNT(CIM_ConcreteJob_RequestStateChange_RequestedState_quals), /* numQualifiers */
    MI_UINT16, /* type */
    NULL, /* className */
    0, /* subscript */
    offsetof(CIM_ConcreteJob_RequestStateChange, RequestedState), /* offset */
};

static MI_CONST MI_Boolean CIM_ConcreteJob_RequestStateChange_TimeoutPeriod_In_qual_value = 1;

static MI_CONST MI_Qualifier CIM_ConcreteJob_RequestStateChange_TimeoutPeriod_In_qual =
{
    MI_T("In"),
    MI_BOOLEAN,
    MI_FLAG_DISABLEOVERRIDE|MI_FLAG_TOSUBCLASS,
    &CIM_ConcreteJob_RequestStateChange_TimeoutPeriod_In_qual_value
};

static MI_CONST MI_Char* CIM_ConcreteJob_RequestStateChange_TimeoutPeriod_Description_qual_value = MI_T("341");

static MI_CONST MI_Qualifier CIM_ConcreteJob_RequestStateChange_TimeoutPeriod_Description_qual =
{
    MI_T("Description"),
    MI_STRING,
    MI_FLAG_ENABLEOVERRIDE|MI_FLAG_TOSUBCLASS|MI_FLAG_TRANSLATABLE,
    &CIM_ConcreteJob_RequestStateChange_TimeoutPeriod_Description_qual_value
};

static MI_Qualifier MI_CONST* MI_CONST CIM_ConcreteJob_RequestStateChange_TimeoutPeriod_quals[] =
{
    &CIM_ConcreteJob_RequestStateChange_TimeoutPeriod_In_qual,
    &CIM_ConcreteJob_RequestStateChange_TimeoutPeriod_Description_qual,
};

/* parameter CIM_ConcreteJob.RequestStateChange(): TimeoutPeriod */
static MI_CONST MI_ParameterDecl CIM_ConcreteJob_RequestStateChange_TimeoutPeriod_param =
{
    MI_FLAG_PARAMETER|MI_FLAG_IN, /* flags */
    0x0074640D, /* code */
    MI_T("TimeoutPeriod"), /* name */
    CIM_ConcreteJob_RequestStateChange_TimeoutPeriod_quals, /* qualifiers */
    MI_COUNT(CIM_ConcreteJob_RequestStateChange_TimeoutPeriod_quals), /* numQualifiers */
    MI_DATETIME, /* type */
    NULL, /* className */
    0, /* subscript */
    offsetof(CIM_ConcreteJob_RequestStateChange, TimeoutPeriod), /* offset */
};

static MI_CONST MI_Char* CIM_ConcreteJob_RequestStateChange_MIReturn_Description_qual_value = MI_T("325");

static MI_CONST MI_Qualifier CIM_ConcreteJob_RequestStateChange_MIReturn_Description_qual =
{
    MI_T("Description"),
    MI_STRING,
    MI_FLAG_ENABLEOVERRIDE|MI_FLAG_TOSUBCLASS|MI_FLAG_TRANSLATABLE,
    &CIM_ConcreteJob_RequestStateChange_MIReturn_Description_qual_value
};

static MI_CONST MI_Char* CIM_ConcreteJob_RequestStateChange_MIReturn_ValueMap_qual_data_value[] =
{
    MI_T("0"),
    MI_T("1"),
    MI_T("2"),
    MI_T("3"),
    MI_T("4"),
    MI_T("5"),
    MI_T("6"),
    MI_T(".."),
    MI_T("4096"),
    MI_T("4097"),
    MI_T("4098"),
    MI_T("4099"),
    MI_T("4100..32767"),
    MI_T("32768..65535"),
};

static MI_CONST MI_ConstStringA CIM_ConcreteJob_RequestStateChange_MIReturn_ValueMap_qual_value =
{
    CIM_ConcreteJob_RequestStateChange_MIReturn_ValueMap_qual_data_value,
    MI_COUNT(CIM_ConcreteJob_RequestStateChange_MIReturn_ValueMap_qual_data_value),
};

static MI_CONST MI_Qualifier CIM_ConcreteJob_RequestStateChange_MIReturn_ValueMap_qual =
{
    MI_T("ValueMap"),
    MI_STRINGA,
    MI_FLAG_ENABLEOVERRIDE|MI_FLAG_TOSUBCLASS,
    &CIM_ConcreteJob_RequestStateChange_MIReturn_ValueMap_qual_value
};

static MI_CONST MI_Char* CIM_ConcreteJob_RequestStateChange_MIReturn_Values_qual_data_value[] =
{
    MI_T("326"),
    MI_T("115"),
    MI_T("327"),
    MI_T("328"),
    MI_T("117"),
    MI_T("329"),
    MI_T("330"),
    MI_T("29"),
    MI_T("331"),
    MI_T("332"),
    MI_T("333"),
    MI_T("334"),
    MI_T("335"),
    MI_T("120"),
};

static MI_CONST MI_ConstStringA CIM_ConcreteJob_RequestStateChange_MIReturn_Values_qual_value =
{
    CIM_ConcreteJob_RequestStateChange_MIReturn_Values_qual_data_value,
    MI_COUNT(CIM_ConcreteJob_RequestStateChange_MIReturn_Values_qual_data_value),
};

static MI_CONST MI_Qualifier CIM_ConcreteJob_RequestStateChange_MIReturn_Values_qual =
{
    MI_T("Values"),
    MI_STRINGA,
    MI_FLAG_ENABLEOVERRIDE|MI_FLAG_TOSUBCLASS|MI_FLAG_TRANSLATABLE,
    &CIM_ConcreteJob_RequestStateChange_MIReturn_Values_qual_value
};

static MI_Qualifier MI_CONST* MI_CONST CIM_ConcreteJob_RequestStateChange_MIReturn_quals[] =
{
    &CIM_ConcreteJob_RequestStateChange_MIReturn_Description_qual,
    &CIM_ConcreteJob_RequestStateChange_MIReturn_ValueMap_qual,
    &CIM_ConcreteJob_RequestStateChange_MIReturn_Values_qual,
};

/* parameter CIM_ConcreteJob.RequestStateChange(): MIReturn */
static MI_CONST MI_ParameterDecl CIM_ConcreteJob_RequestStateChange_MIReturn_param =
{
    MI_FLAG_PARAMETER|MI_FLAG_OUT, /* flags */
    0x006D6E08, /* code */
    MI_T("MIReturn"), /* name */
    CIM_ConcreteJob_RequestStateChange_MIReturn_quals, /* qualifiers */
    MI_COUNT(CIM_ConcreteJob_RequestStateChange_MIReturn_quals), /* numQualifiers */
    MI_UINT32, /* type */
    NULL, /* className */
    0, /* subscript */
    offsetof(CIM_ConcreteJob_RequestStateChange, MIReturn), /* offset */
};

static MI_ParameterDecl MI_CONST* MI_CONST CIM_ConcreteJob_RequestStateChange_params[] =
{
    &CIM_ConcreteJob_RequestStateChange_MIReturn_param,
    &CIM_ConcreteJob_RequestStateChange_RequestedState_param,
    &CIM_ConcreteJob_RequestStateChange_TimeoutPeriod_param,
};

/* method CIM_ConcreteJob.RequestStateChange() */
MI_CONST MI_MethodDecl CIM_ConcreteJob_RequestStateChange_rtti =
{
    MI_FLAG_METHOD, /* flags */
    0x00726512, /* code */
    MI_T("RequestStateChange"), /* name */
    CIM_ConcreteJob_RequestStateChange_quals, /* qualifiers */
    MI_COUNT(CIM_ConcreteJob_RequestStateChange_quals), /* numQualifiers */
    CIM_ConcreteJob_RequestStateChange_params, /* parameters */
    MI_COUNT(CIM_ConcreteJob_RequestStateChange_params), /* numParameters */
    sizeof(CIM_ConcreteJob_RequestStateChange), /* size */
    MI_UINT32, /* returnType */
    MI_T("CIM_ConcreteJob"), /* origin */
    MI_T("CIM_ConcreteJob"), /* propagator */
    &schemaDecl, /* schema */
    NULL, /* method */
};

static MI_CONST MI_Char* CIM_ConcreteJob_GetError_Description_qual_value = MI_T("342");

static MI_CONST MI_Qualifier CIM_ConcreteJob_GetError_Description_qual =
{
    MI_T("Description"),
    MI_STRING,
    MI_FLAG_ENABLEOVERRIDE|MI_FLAG_TOSUBCLASS|MI_FLAG_TRANSLATABLE,
    &CIM_ConcreteJob_GetError_Description_qual_value
};

static MI_CONST MI_Char* CIM_ConcreteJob_GetError_ValueMap_qual_data_value[] =
{
    MI_T("0"),
    MI_T("1"),
    MI_T("2"),
    MI_T("3"),
    MI_T("4"),
    MI_T("5"),
    MI_T("6"),
    MI_T(".."),
    MI_T("32768..65535"),
};

static MI_CONST MI_ConstStringA CIM_ConcreteJob_GetError_ValueMap_qual_value =
{
    CIM_ConcreteJob_GetError_ValueMap_qual_data_value,
    MI_COUNT(CIM_ConcreteJob_GetError_ValueMap_qual_data_value),
};

static MI_CONST MI_Qualifier CIM_ConcreteJob_GetError_ValueMap_qual =
{
    MI_T("ValueMap"),
    MI_STRINGA,
    MI_FLAG_ENABLEOVERRIDE|MI_FLAG_TOSUBCLASS,
    &CIM_ConcreteJob_GetError_ValueMap_qual_value
};

static MI_CONST MI_Char* CIM_ConcreteJob_GetError_Values_qual_data_value[] =
{
    MI_T("114"),
    MI_T("115"),
    MI_T("343"),
    MI_T("116"),
    MI_T("117"),
    MI_T("329"),
    MI_T("118"),
    MI_T("29"),
    MI_T("120"),
};

static MI_CONST MI_ConstStringA CIM_ConcreteJob_GetError_Values_qual_value =
{
    CIM_ConcreteJob_GetError_Values_qual_data_value,
    MI_COUNT(CIM_ConcreteJob_GetError_Values_qual_data_value),
};

static MI_CONST MI_Qualifier CIM_ConcreteJob_GetError_Values_qual =
{
    MI_T("Values"),
    MI_STRINGA,
    MI_FLAG_ENABLEOVERRIDE|MI_FLAG_TOSUBCLASS|MI_FLAG_TRANSLATABLE,
    &CIM_ConcreteJob_GetError_Values_qual_value
};

static MI_Qualifier MI_CONST* MI_CONST CIM_ConcreteJob_GetError_quals[] =
{
    &CIM_ConcreteJob_GetError_Description_qual,
    &CIM_ConcreteJob_GetError_ValueMap_qual,
    &CIM_ConcreteJob_GetError_Values_qual,
};

static MI_CONST MI_Boolean CIM_ConcreteJob_GetError_Error_Out_qual_value = 1;

static MI_CONST MI_Qualifier CIM_ConcreteJob_GetError_Error_Out_qual =
{
    MI_T("Out"),
    MI_BOOLEAN,
    MI_FLAG_DISABLEOVERRIDE|MI_FLAG_TOSUBCLASS,
    &CIM_ConcreteJob_GetError_Error_Out_qual_value
};

static MI_CONST MI_Char* CIM_ConcreteJob_GetError_Error_Description_qual_value = MI_T("344");

static MI_CONST MI_Qualifier CIM_ConcreteJob_GetError_Error_Description_qual =
{
    MI_T("Description"),
    MI_STRING,
    MI_FLAG_ENABLEOVERRIDE|MI_FLAG_TOSUBCLASS|MI_FLAG_TRANSLATABLE,
    &CIM_ConcreteJob_GetError_Error_Description_qual_value
};

static MI_CONST MI_Char* CIM_ConcreteJob_GetError_Error_EmbeddedInstance_qual_value = MI_T("CIM_Error");

static MI_CONST MI_Qualifier CIM_ConcreteJob_GetError_Error_EmbeddedInstance_qual =
{
    MI_T("EmbeddedInstance"),
    MI_STRING,
    MI_FLAG_ENABLEOVERRIDE|MI_FLAG_TOSUBCLASS,
    &CIM_ConcreteJob_GetError_Error_EmbeddedInstance_qual_value
};

static MI_Qualifier MI_CONST* MI_CONST CIM_ConcreteJob_GetError_Error_quals[] =
{
    &CIM_ConcreteJob_GetError_Error_Out_qual,
    &CIM_ConcreteJob_GetError_Error_Description_qual,
    &CIM_ConcreteJob_GetError_Error_EmbeddedInstance_qual,
};

/* parameter CIM_ConcreteJob.GetError(): Error */
static MI_CONST MI_ParameterDecl CIM_ConcreteJob_GetError_Error_param =
{
    MI_FLAG_PARAMETER|MI_FLAG_OUT, /* flags */
    0x00657205, /* code */
    MI_T("Error"), /* name */
    CIM_ConcreteJob_GetError_Error_quals, /* qualifiers */
    MI_COUNT(CIM_ConcreteJob_GetError_Error_quals), /* numQualifiers */
    MI_INSTANCE, /* type */
    MI_T("CIM_Error"), /* className */
    0, /* subscript */
    offsetof(CIM_ConcreteJob_GetError, Error), /* offset */
};

static MI_CONST MI_Char* CIM_ConcreteJob_GetError_MIReturn_Description_qual_value = MI_T("342");

static MI_CONST MI_Qualifier CIM_ConcreteJob_GetError_MIReturn_Description_qual =
{
    MI_T("Description"),
    MI_STRING,
    MI_FLAG_ENABLEOVERRIDE|MI_FLAG_TOSUBCLASS|MI_FLAG_TRANSLATABLE,
    &CIM_ConcreteJob_GetError_MIReturn_Description_qual_value
};

static MI_CONST MI_Char* CIM_ConcreteJob_GetError_MIReturn_ValueMap_qual_data_value[] =
{
    MI_T("0"),
    MI_T("1"),
    MI_T("2"),
    MI_T("3"),
    MI_T("4"),
    MI_T("5"),
    MI_T("6"),
    MI_T(".."),
    MI_T("32768..65535"),
};

static MI_CONST MI_ConstStringA CIM_ConcreteJob_GetError_MIReturn_ValueMap_qual_value =
{
    CIM_ConcreteJob_GetError_MIReturn_ValueMap_qual_data_value,
    MI_COUNT(CIM_ConcreteJob_GetError_MIReturn_ValueMap_qual_data_value),
};

static MI_CONST MI_Qualifier CIM_ConcreteJob_GetError_MIReturn_ValueMap_qual =
{
    MI_T("ValueMap"),
    MI_STRINGA,
    MI_FLAG_ENABLEOVERRIDE|MI_FLAG_TOSUBCLASS,
    &CIM_ConcreteJob_GetError_MIReturn_ValueMap_qual_value
};

static MI_CONST MI_Char* CIM_ConcreteJob_GetError_MIReturn_Values_qual_data_value[] =
{
    MI_T("114"),
    MI_T("115"),
    MI_T("343"),
    MI_T("116"),
    MI_T("117"),
    MI_T("329"),
    MI_T("118"),
    MI_T("29"),
    MI_T("120"),
};

static MI_CONST MI_ConstStringA CIM_ConcreteJob_GetError_MIReturn_Values_qual_value =
{
    CIM_ConcreteJob_GetError_MIReturn_Values_qual_data_value,
    MI_COUNT(CIM_ConcreteJob_GetError_MIReturn_Values_qual_data_value),
};

static MI_CONST MI_Qualifier CIM_ConcreteJob_GetError_MIReturn_Values_qual =
{
    MI_T("Values"),
    MI_STRINGA,
    MI_FLAG_ENABLEOVERRIDE|MI_FLAG_TOSUBCLASS|MI_FLAG_TRANSLATABLE,
    &CIM_ConcreteJob_GetError_MIReturn_Values_qual_value
};

static MI_Qualifier MI_CONST* MI_CONST CIM_ConcreteJob_GetError_MIReturn_quals[] =
{
    &CIM_ConcreteJob_GetError_MIReturn_Description_qual,
    &CIM_ConcreteJob_GetError_MIReturn_ValueMap_qual,
    &CIM_ConcreteJob_GetError_MIReturn_Values_qual,
};

/* parameter CIM_ConcreteJob.GetError(): MIReturn */
static MI_CONST MI_ParameterDecl CIM_ConcreteJob_GetError_MIReturn_param =
{
    MI_FLAG_PARAMETER|MI_FLAG_OUT, /* flags */
    0x006D6E08, /* code */
    MI_T("MIReturn"), /* name */
    CIM_ConcreteJob_GetError_MIReturn_quals, /* qualifiers */
    MI_COUNT(CIM_ConcreteJob_GetError_MIReturn_quals), /* numQualifiers */
    MI_UINT32, /* type */
    NULL, /* className */
    0, /* subscript */
    offsetof(CIM_ConcreteJob_GetError, MIReturn), /* offset */
};

static MI_ParameterDecl MI_CONST* MI_CONST CIM_ConcreteJob_GetError_params[] =
{
    &CIM_ConcreteJob_GetError_MIReturn_param,
    &CIM_ConcreteJob_GetError_Error_param,
};

/* method CIM_ConcreteJob.GetError() */
MI_CONST MI_MethodDecl CIM_ConcreteJob_GetError_rtti =
{
    MI_FLAG_METHOD, /* flags */
    0x00677208, /* code */
    MI_T("GetError"), /* name */
    CIM_ConcreteJob_GetError_quals, /* qualifiers */
    MI_COUNT(CIM_ConcreteJob_GetError_quals), /* numQualifiers */
    CIM_ConcreteJob_GetError_params, /* parameters */
    MI_COUNT(CIM_ConcreteJob_GetError_params), /* numParameters */
    sizeof(CIM_ConcreteJob_GetError), /* size */
    MI_UINT32, /* returnType */
    MI_T("CIM_ConcreteJob"), /* origin */
    MI_T("CIM_ConcreteJob"), /* propagator */
    &schemaDecl, /* schema */
    NULL, /* method */
};

static MI_MethodDecl MI_CONST* MI_CONST CIM_ConcreteJob_meths[] =
{
    &CIM_Job_KillJob_rtti,
    &CIM_ConcreteJob_RequestStateChange_rtti,
    &CIM_ConcreteJob_GetError_rtti,
};

static MI_CONST MI_Char* CIM_ConcreteJob_UMLPackagePath_qual_value = MI_T("CIM::Core::CoreElements");

static MI_CONST MI_Qualifier CIM_ConcreteJob_UMLPackagePath_qual =
{
    MI_T("UMLPackagePath"),
    MI_STRING,
    MI_FLAG_ENABLEOVERRIDE|MI_FLAG_TOSUBCLASS,
    &CIM_ConcreteJob_UMLPackagePath_qual_value
};

static MI_CONST MI_Char* CIM_ConcreteJob_Description_qual_value = MI_T("345");

static MI_CONST MI_Qualifier CIM_ConcreteJob_Description_qual =
{
    MI_T("Description"),
    MI_STRING,
    MI_FLAG_ENABLEOVERRIDE|MI_FLAG_TOSUBCLASS|MI_FLAG_TRANSLATABLE,
    &CIM_ConcreteJob_Description_qual_value
};

static MI_CONST MI_Char* CIM_ConcreteJob_Version_qual_value = MI_T("55");

static MI_CONST MI_Qualifier CIM_ConcreteJob_Version_qual =
{
    MI_T("Version"),
    MI_STRING,
    MI_FLAG_ENABLEOVERRIDE|MI_FLAG_TRANSLATABLE|MI_FLAG_RESTRICTED,
    &CIM_ConcreteJob_Version_qual_value
};

static MI_Qualifier MI_CONST* MI_CONST CIM_ConcreteJob_quals[] =
{
    &CIM_ConcreteJob_UMLPackagePath_qual,
    &CIM_ConcreteJob_Description_qual,
    &CIM_ConcreteJob_Version_qual,
};

/* class CIM_ConcreteJob */
MI_CONST MI_ClassDecl CIM_ConcreteJob_rtti =
{
    MI_FLAG_CLASS, /* flags */
    0x0063620F, /* code */
    MI_T("CIM_ConcreteJob"), /* name */
    CIM_ConcreteJob_quals, /* qualifiers */
    MI_COUNT(CIM_ConcreteJob_quals), /* numQualifiers */
    CIM_ConcreteJob_props, /* properties */
    MI_COUNT(CIM_ConcreteJob_props), /* numProperties */
    sizeof(CIM_ConcreteJob), /* size */
    MI_T("CIM_Job"), /* superClass */
    &CIM_Job_rtti, /* superClassDecl */
    CIM_ConcreteJob_meths, /* methods */
    MI_COUNT(CIM_ConcreteJob_meths), /* numMethods */
    &schemaDecl, /* schema */
    NULL, /* functions */
    NULL /* owningClass */
};

/*
**==============================================================================
**
** CIM_EnabledLogicalElement
**
**==============================================================================
*/

static MI_CONST MI_Char* CIM_EnabledLogicalElement_EnabledState_Description_qual_value = MI_T("346");

static MI_CONST MI_Qualifier CIM_EnabledLogicalElement_EnabledState_Description_qual =
{
    MI_T("Description"),
    MI_STRING,
    MI_FLAG_ENABLEOVERRIDE|MI_FLAG_TOSUBCLASS|MI_FLAG_TRANSLATABLE,
    &CIM_EnabledLogicalElement_EnabledState_Description_qual_value
};

static MI_CONST MI_Char* CIM_EnabledLogicalElement_EnabledState_ValueMap_qual_data_value[] =
{
    MI_T("0"),
    MI_T("1"),
    MI_T("2"),
    MI_T("3"),
    MI_T("4"),
    MI_T("5"),
    MI_T("6"),
    MI_T("7"),
    MI_T("8"),
    MI_T("9"),
    MI_T("10"),
    MI_T("11..32767"),
    MI_T("32768..65535"),
};

static MI_CONST MI_ConstStringA CIM_EnabledLogicalElement_EnabledState_ValueMap_qual_value =
{
    CIM_EnabledLogicalElement_EnabledState_ValueMap_qual_data_value,
    MI_COUNT(CIM_EnabledLogicalElement_EnabledState_ValueMap_qual_data_value),
};

static MI_CONST MI_Qualifier CIM_EnabledLogicalElement_EnabledState_ValueMap_qual =
{
    MI_T("ValueMap"),
    MI_STRINGA,
    MI_FLAG_ENABLEOVERRIDE|MI_FLAG_TOSUBCLASS,
    &CIM_EnabledLogicalElement_EnabledState_ValueMap_qual_value
};

static MI_CONST MI_Char* CIM_EnabledLogicalElement_EnabledState_Values_qual_data_value[] =
{
    MI_T("10"),
    MI_T("11"),
    MI_T("347"),
    MI_T("348"),
    MI_T("50"),
    MI_T("349"),
    MI_T("350"),
    MI_T("51"),
    MI_T("351"),
    MI_T("352"),
    MI_T("18"),
    MI_T("29"),
    MI_T("30"),
};

static MI_CONST MI_ConstStringA CIM_EnabledLogicalElement_EnabledState_Values_qual_value =
{
    CIM_EnabledLogicalElement_EnabledState_Values_qual_data_value,
    MI_COUNT(CIM_EnabledLogicalElement_EnabledState_Values_qual_data_value),
};

static MI_CONST MI_Qualifier CIM_EnabledLogicalElement_EnabledState_Values_qual =
{
    MI_T("Values"),
    MI_STRINGA,
    MI_FLAG_ENABLEOVERRIDE|MI_FLAG_TOSUBCLASS|MI_FLAG_TRANSLATABLE,
    &CIM_EnabledLogicalElement_EnabledState_Values_qual_value
};

static MI_CONST MI_Char* CIM_EnabledLogicalElement_EnabledState_ModelCorrespondence_qual_data_value[] =
{
    MI_T("CIM_EnabledLogicalElement.OtherEnabledState"),
};

static MI_CONST MI_ConstStringA CIM_EnabledLogicalElement_EnabledState_ModelCorrespondence_qual_value =
{
    CIM_EnabledLogicalElement_EnabledState_ModelCorrespondence_qual_data_value,
    MI_COUNT(CIM_EnabledLogicalElement_EnabledState_ModelCorrespondence_qual_data_value),
};

static MI_CONST MI_Qualifier CIM_EnabledLogicalElement_EnabledState_ModelCorrespondence_qual =
{
    MI_T("ModelCorrespondence"),
    MI_STRINGA,
    MI_FLAG_ENABLEOVERRIDE|MI_FLAG_TOSUBCLASS,
    &CIM_EnabledLogicalElement_EnabledState_ModelCorrespondence_qual_value
};

static MI_Qualifier MI_CONST* MI_CONST CIM_EnabledLogicalElement_EnabledState_quals[] =
{
    &CIM_EnabledLogicalElement_EnabledState_Description_qual,
    &CIM_EnabledLogicalElement_EnabledState_ValueMap_qual,
    &CIM_EnabledLogicalElement_EnabledState_Values_qual,
    &CIM_EnabledLogicalElement_EnabledState_ModelCorrespondence_qual,
};

static MI_CONST MI_Uint16 CIM_EnabledLogicalElement_EnabledState_value = 5;

/* property CIM_EnabledLogicalElement.EnabledState */
static MI_CONST MI_PropertyDecl CIM_EnabledLogicalElement_EnabledState_prop =
{
    MI_FLAG_PROPERTY|MI_FLAG_READONLY, /* flags */
    0x0065650C, /* code */
    MI_T("EnabledState"), /* name */
    CIM_EnabledLogicalElement_EnabledState_quals, /* qualifiers */
    MI_COUNT(CIM_EnabledLogicalElement_EnabledState_quals), /* numQualifiers */
    MI_UINT16, /* type */
    NULL, /* className */
    0, /* subscript */
    offsetof(CIM_EnabledLogicalElement, EnabledState), /* offset */
    MI_T("CIM_EnabledLogicalElement"), /* origin */
    MI_T("CIM_EnabledLogicalElement"), /* propagator */
    &CIM_EnabledLogicalElement_EnabledState_value,
};

static MI_CONST MI_Char* CIM_EnabledLogicalElement_OtherEnabledState_Description_qual_value = MI_T("353");

static MI_CONST MI_Qualifier CIM_EnabledLogicalElement_OtherEnabledState_Description_qual =
{
    MI_T("Description"),
    MI_STRING,
    MI_FLAG_ENABLEOVERRIDE|MI_FLAG_TOSUBCLASS|MI_FLAG_TRANSLATABLE,
    &CIM_EnabledLogicalElement_OtherEnabledState_Description_qual_value
};

static MI_CONST MI_Char* CIM_EnabledLogicalElement_OtherEnabledState_ModelCorrespondence_qual_data_value[] =
{
    MI_T("CIM_EnabledLogicalElement.EnabledState"),
};

static MI_CONST MI_ConstStringA CIM_EnabledLogicalElement_OtherEnabledState_ModelCorrespondence_qual_value =
{
    CIM_EnabledLogicalElement_OtherEnabledState_ModelCorrespondence_qual_data_value,
    MI_COUNT(CIM_EnabledLogicalElement_OtherEnabledState_ModelCorrespondence_qual_data_value),
};

static MI_CONST MI_Qualifier CIM_EnabledLogicalElement_OtherEnabledState_ModelCorrespondence_qual =
{
    MI_T("ModelCorrespondence"),
    MI_STRINGA,
    MI_FLAG_ENABLEOVERRIDE|MI_FLAG_TOSUBCLASS,
    &CIM_EnabledLogicalElement_OtherEnabledState_ModelCorrespondence_qual_value
};

static MI_Qualifier MI_CONST* MI_CONST CIM_EnabledLogicalElement_OtherEnabledState_quals[] =
{
    &CIM_EnabledLogicalElement_OtherEnabledState_Description_qual,
    &CIM_EnabledLogicalElement_OtherEnabledState_ModelCorrespondence_qual,
};

/* property CIM_EnabledLogicalElement.OtherEnabledState */
static MI_CONST MI_PropertyDecl CIM_EnabledLogicalElement_OtherEnabledState_prop =
{
    MI_FLAG_PROPERTY|MI_FLAG_READONLY, /* flags */
    0x006F6511, /* code */
    MI_T("OtherEnabledState"), /* name */
    CIM_EnabledLogicalElement_OtherEnabledState_quals, /* qualifiers */
    MI_COUNT(CIM_EnabledLogicalElement_OtherEnabledState_quals), /* numQualifiers */
    MI_STRING, /* type */
    NULL, /* className */
    0, /* subscript */
    offsetof(CIM_EnabledLogicalElement, OtherEnabledState), /* offset */
    MI_T("CIM_EnabledLogicalElement"), /* origin */
    MI_T("CIM_EnabledLogicalElement"), /* propagator */
    NULL,
};

static MI_CONST MI_Char* CIM_EnabledLogicalElement_RequestedState_Description_qual_value = MI_T("354");

static MI_CONST MI_Qualifier CIM_EnabledLogicalElement_RequestedState_Description_qual =
{
    MI_T("Description"),
    MI_STRING,
    MI_FLAG_ENABLEOVERRIDE|MI_FLAG_TOSUBCLASS|MI_FLAG_TRANSLATABLE,
    &CIM_EnabledLogicalElement_RequestedState_Description_qual_value
};

static MI_CONST MI_Char* CIM_EnabledLogicalElement_RequestedState_ValueMap_qual_data_value[] =
{
    MI_T("0"),
    MI_T("2"),
    MI_T("3"),
    MI_T("4"),
    MI_T("5"),
    MI_T("6"),
    MI_T("7"),
    MI_T("8"),
    MI_T("9"),
    MI_T("10"),
    MI_T("11"),
    MI_T("12"),
    MI_T(".."),
    MI_T("32768..65535"),
};

static MI_CONST MI_ConstStringA CIM_EnabledLogicalElement_RequestedState_ValueMap_qual_value =
{
    CIM_EnabledLogicalElement_RequestedState_ValueMap_qual_data_value,
    MI_COUNT(CIM_EnabledLogicalElement_RequestedState_ValueMap_qual_data_value),
};

static MI_CONST MI_Qualifier CIM_EnabledLogicalElement_RequestedState_ValueMap_qual =
{
    MI_T("ValueMap"),
    MI_STRINGA,
    MI_FLAG_ENABLEOVERRIDE|MI_FLAG_TOSUBCLASS,
    &CIM_EnabledLogicalElement_RequestedState_ValueMap_qual_value
};

static MI_CONST MI_Char* CIM_EnabledLogicalElement_RequestedState_Values_qual_data_value[] =
{
    MI_T("10"),
    MI_T("347"),
    MI_T("348"),
    MI_T("355"),
    MI_T("356"),
    MI_T("357"),
    MI_T("358"),
    MI_T("351"),
    MI_T("352"),
    MI_T("359"),
    MI_T("360"),
    MI_T("349"),
    MI_T("29"),
    MI_T("30"),
};

static MI_CONST MI_ConstStringA CIM_EnabledLogicalElement_RequestedState_Values_qual_value =
{
    CIM_EnabledLogicalElement_RequestedState_Values_qual_data_value,
    MI_COUNT(CIM_EnabledLogicalElement_RequestedState_Values_qual_data_value),
};

static MI_CONST MI_Qualifier CIM_EnabledLogicalElement_RequestedState_Values_qual =
{
    MI_T("Values"),
    MI_STRINGA,
    MI_FLAG_ENABLEOVERRIDE|MI_FLAG_TOSUBCLASS|MI_FLAG_TRANSLATABLE,
    &CIM_EnabledLogicalElement_RequestedState_Values_qual_value
};

static MI_CONST MI_Char* CIM_EnabledLogicalElement_RequestedState_ModelCorrespondence_qual_data_value[] =
{
    MI_T("CIM_EnabledLogicalElement.EnabledState"),
};

static MI_CONST MI_ConstStringA CIM_EnabledLogicalElement_RequestedState_ModelCorrespondence_qual_value =
{
    CIM_EnabledLogicalElement_RequestedState_ModelCorrespondence_qual_data_value,
    MI_COUNT(CIM_EnabledLogicalElement_RequestedState_ModelCorrespondence_qual_data_value),
};

static MI_CONST MI_Qualifier CIM_EnabledLogicalElement_RequestedState_ModelCorrespondence_qual =
{
    MI_T("ModelCorrespondence"),
    MI_STRINGA,
    MI_FLAG_ENABLEOVERRIDE|MI_FLAG_TOSUBCLASS,
    &CIM_EnabledLogicalElement_RequestedState_ModelCorrespondence_qual_value
};

static MI_Qualifier MI_CONST* MI_CONST CIM_EnabledLogicalElement_RequestedState_quals[] =
{
    &CIM_EnabledLogicalElement_RequestedState_Description_qual,
    &CIM_EnabledLogicalElement_RequestedState_ValueMap_qual,
    &CIM_EnabledLogicalElement_RequestedState_Values_qual,
    &CIM_EnabledLogicalElement_RequestedState_ModelCorrespondence_qual,
};

static MI_CONST MI_Uint16 CIM_EnabledLogicalElement_RequestedState_value = 12;

/* property CIM_EnabledLogicalElement.RequestedState */
static MI_CONST MI_PropertyDecl CIM_EnabledLogicalElement_RequestedState_prop =
{
    MI_FLAG_PROPERTY|MI_FLAG_READONLY, /* flags */
    0x0072650E, /* code */
    MI_T("RequestedState"), /* name */
    CIM_EnabledLogicalElement_RequestedState_quals, /* qualifiers */
    MI_COUNT(CIM_EnabledLogicalElement_RequestedState_quals), /* numQualifiers */
    MI_UINT16, /* type */
    NULL, /* className */
    0, /* subscript */
    offsetof(CIM_EnabledLogicalElement, RequestedState), /* offset */
    MI_T("CIM_EnabledLogicalElement"), /* origin */
    MI_T("CIM_EnabledLogicalElement"), /* propagator */
    &CIM_EnabledLogicalElement_RequestedState_value,
};

static MI_CONST MI_Boolean CIM_EnabledLogicalElement_EnabledDefault_Write_qual_value = 1;

static MI_CONST MI_Qualifier CIM_EnabledLogicalElement_EnabledDefault_Write_qual =
{
    MI_T("Write"),
    MI_BOOLEAN,
    MI_FLAG_ENABLEOVERRIDE|MI_FLAG_TOSUBCLASS,
    &CIM_EnabledLogicalElement_EnabledDefault_Write_qual_value
};

static MI_CONST MI_Char* CIM_EnabledLogicalElement_EnabledDefault_Description_qual_value = MI_T("361");

static MI_CONST MI_Qualifier CIM_EnabledLogicalElement_EnabledDefault_Description_qual =
{
    MI_T("Description"),
    MI_STRING,
    MI_FLAG_ENABLEOVERRIDE|MI_FLAG_TOSUBCLASS|MI_FLAG_TRANSLATABLE,
    &CIM_EnabledLogicalElement_EnabledDefault_Description_qual_value
};

static MI_CONST MI_Char* CIM_EnabledLogicalElement_EnabledDefault_ValueMap_qual_data_value[] =
{
    MI_T("2"),
    MI_T("3"),
    MI_T("5"),
    MI_T("6"),
    MI_T("7"),
    MI_T("9"),
    MI_T(".."),
    MI_T("32768..65535"),
};

static MI_CONST MI_ConstStringA CIM_EnabledLogicalElement_EnabledDefault_ValueMap_qual_value =
{
    CIM_EnabledLogicalElement_EnabledDefault_ValueMap_qual_data_value,
    MI_COUNT(CIM_EnabledLogicalElement_EnabledDefault_ValueMap_qual_data_value),
};

static MI_CONST MI_Qualifier CIM_EnabledLogicalElement_EnabledDefault_ValueMap_qual =
{
    MI_T("ValueMap"),
    MI_STRINGA,
    MI_FLAG_ENABLEOVERRIDE|MI_FLAG_TOSUBCLASS,
    &CIM_EnabledLogicalElement_EnabledDefault_ValueMap_qual_value
};

static MI_CONST MI_Char* CIM_EnabledLogicalElement_EnabledDefault_Values_qual_data_value[] =
{
    MI_T("347"),
    MI_T("348"),
    MI_T("349"),
    MI_T("350"),
    MI_T("362"),
    MI_T("352"),
    MI_T("29"),
    MI_T("30"),
};

static MI_CONST MI_ConstStringA CIM_EnabledLogicalElement_EnabledDefault_Values_qual_value =
{
    CIM_EnabledLogicalElement_EnabledDefault_Values_qual_data_value,
    MI_COUNT(CIM_EnabledLogicalElement_EnabledDefault_Values_qual_data_value),
};

static MI_CONST MI_Qualifier CIM_EnabledLogicalElement_EnabledDefault_Values_qual =
{
    MI_T("Values"),
    MI_STRINGA,
    MI_FLAG_ENABLEOVERRIDE|MI_FLAG_TOSUBCLASS|MI_FLAG_TRANSLATABLE,
    &CIM_EnabledLogicalElement_EnabledDefault_Values_qual_value
};

static MI_Qualifier MI_CONST* MI_CONST CIM_EnabledLogicalElement_EnabledDefault_quals[] =
{
    &CIM_EnabledLogicalElement_EnabledDefault_Write_qual,
    &CIM_EnabledLogicalElement_EnabledDefault_Description_qual,
    &CIM_EnabledLogicalElement_EnabledDefault_ValueMap_qual,
    &CIM_EnabledLogicalElement_EnabledDefault_Values_qual,
};

static MI_CONST MI_Uint16 CIM_EnabledLogicalElement_EnabledDefault_value = 2;

/* property CIM_EnabledLogicalElement.EnabledDefault */
static MI_CONST MI_PropertyDecl CIM_EnabledLogicalElement_EnabledDefault_prop =
{
    MI_FLAG_PROPERTY, /* flags */
    0x0065740E, /* code */
    MI_T("EnabledDefault"), /* name */
    CIM_EnabledLogicalElement_EnabledDefault_quals, /* qualifiers */
    MI_COUNT(CIM_EnabledLogicalElement_EnabledDefault_quals), /* numQualifiers */
    MI_UINT16, /* type */
    NULL, /* className */
    0, /* subscript */
    offsetof(CIM_EnabledLogicalElement, EnabledDefault), /* offset */
    MI_T("CIM_EnabledLogicalElement"), /* origin */
    MI_T("CIM_EnabledLogicalElement"), /* propagator */
    &CIM_EnabledLogicalElement_EnabledDefault_value,
};

static MI_CONST MI_Char* CIM_EnabledLogicalElement_TimeOfLastStateChange_Description_qual_value = MI_T("363");

static MI_CONST MI_Qualifier CIM_EnabledLogicalElement_TimeOfLastStateChange_Description_qual =
{
    MI_T("Description"),
    MI_STRING,
    MI_FLAG_ENABLEOVERRIDE|MI_FLAG_TOSUBCLASS|MI_FLAG_TRANSLATABLE,
    &CIM_EnabledLogicalElement_TimeOfLastStateChange_Description_qual_value
};

static MI_Qualifier MI_CONST* MI_CONST CIM_EnabledLogicalElement_TimeOfLastStateChange_quals[] =
{
    &CIM_EnabledLogicalElement_TimeOfLastStateChange_Description_qual,
};

/* property CIM_EnabledLogicalElement.TimeOfLastStateChange */
static MI_CONST MI_PropertyDecl CIM_EnabledLogicalElement_TimeOfLastStateChange_prop =
{
    MI_FLAG_PROPERTY|MI_FLAG_READONLY, /* flags */
    0x00746515, /* code */
    MI_T("TimeOfLastStateChange"), /* name */
    CIM_EnabledLogicalElement_TimeOfLastStateChange_quals, /* qualifiers */
    MI_COUNT(CIM_EnabledLogicalElement_TimeOfLastStateChange_quals), /* numQualifiers */
    MI_DATETIME, /* type */
    NULL, /* className */
    0, /* subscript */
    offsetof(CIM_EnabledLogicalElement, TimeOfLastStateChange), /* offset */
    MI_T("CIM_EnabledLogicalElement"), /* origin */
    MI_T("CIM_EnabledLogicalElement"), /* propagator */
    NULL,
};

static MI_CONST MI_Char* CIM_EnabledLogicalElement_AvailableRequestedStates_Description_qual_value = MI_T("364");

static MI_CONST MI_Qualifier CIM_EnabledLogicalElement_AvailableRequestedStates_Description_qual =
{
    MI_T("Description"),
    MI_STRING,
    MI_FLAG_ENABLEOVERRIDE|MI_FLAG_TOSUBCLASS|MI_FLAG_TRANSLATABLE,
    &CIM_EnabledLogicalElement_AvailableRequestedStates_Description_qual_value
};

static MI_CONST MI_Char* CIM_EnabledLogicalElement_AvailableRequestedStates_ValueMap_qual_data_value[] =
{
    MI_T("2"),
    MI_T("3"),
    MI_T("4"),
    MI_T("6"),
    MI_T("7"),
    MI_T("8"),
    MI_T("9"),
    MI_T("10"),
    MI_T("11"),
    MI_T(".."),
};

static MI_CONST MI_ConstStringA CIM_EnabledLogicalElement_AvailableRequestedStates_ValueMap_qual_value =
{
    CIM_EnabledLogicalElement_AvailableRequestedStates_ValueMap_qual_data_value,
    MI_COUNT(CIM_EnabledLogicalElement_AvailableRequestedStates_ValueMap_qual_data_value),
};

static MI_CONST MI_Qualifier CIM_EnabledLogicalElement_AvailableRequestedStates_ValueMap_qual =
{
    MI_T("ValueMap"),
    MI_STRINGA,
    MI_FLAG_ENABLEOVERRIDE|MI_FLAG_TOSUBCLASS,
    &CIM_EnabledLogicalElement_AvailableRequestedStates_ValueMap_qual_value
};

static MI_CONST MI_Char* CIM_EnabledLogicalElement_AvailableRequestedStates_Values_qual_data_value[] =
{
    MI_T("347"),
    MI_T("348"),
    MI_T("355"),
    MI_T("357"),
    MI_T("358"),
    MI_T("365"),
    MI_T("352"),
    MI_T("359"),
    MI_T("360"),
    MI_T("29"),
};

static MI_CONST MI_ConstStringA CIM_EnabledLogicalElement_AvailableRequestedStates_Values_qual_value =
{
    CIM_EnabledLogicalElement_AvailableRequestedStates_Values_qual_data_value,
    MI_COUNT(CIM_EnabledLogicalElement_AvailableRequestedStates_Values_qual_data_value),
};

static MI_CONST MI_Qualifier CIM_EnabledLogicalElement_AvailableRequestedStates_Values_qual =
{
    MI_T("Values"),
    MI_STRINGA,
    MI_FLAG_ENABLEOVERRIDE|MI_FLAG_TOSUBCLASS|MI_FLAG_TRANSLATABLE,
    &CIM_EnabledLogicalElement_AvailableRequestedStates_Values_qual_value
};

static MI_CONST MI_Char* CIM_EnabledLogicalElement_AvailableRequestedStates_ModelCorrespondence_qual_data_value[] =
{
    MI_T("CIM_EnabledLogicalElement.RequestStateChange"),
    MI_T("CIM_EnabledLogicalElementCapabilities.RequestedStatesSupported"),
};

static MI_CONST MI_ConstStringA CIM_EnabledLogicalElement_AvailableRequestedStates_ModelCorrespondence_qual_value =
{
    CIM_EnabledLogicalElement_AvailableRequestedStates_ModelCorrespondence_qual_data_value,
    MI_COUNT(CIM_EnabledLogicalElement_AvailableRequestedStates_ModelCorrespondence_qual_data_value),
};

static MI_CONST MI_Qualifier CIM_EnabledLogicalElement_AvailableRequestedStates_ModelCorrespondence_qual =
{
    MI_T("ModelCorrespondence"),
    MI_STRINGA,
    MI_FLAG_ENABLEOVERRIDE|MI_FLAG_TOSUBCLASS,
    &CIM_EnabledLogicalElement_AvailableRequestedStates_ModelCorrespondence_qual_value
};

static MI_Qualifier MI_CONST* MI_CONST CIM_EnabledLogicalElement_AvailableRequestedStates_quals[] =
{
    &CIM_EnabledLogicalElement_AvailableRequestedStates_Description_qual,
    &CIM_EnabledLogicalElement_AvailableRequestedStates_ValueMap_qual,
    &CIM_EnabledLogicalElement_AvailableRequestedStates_Values_qual,
    &CIM_EnabledLogicalElement_AvailableRequestedStates_ModelCorrespondence_qual,
};

/* property CIM_EnabledLogicalElement.AvailableRequestedStates */
static MI_CONST MI_PropertyDecl CIM_EnabledLogicalElement_AvailableRequestedStates_prop =
{
    MI_FLAG_PROPERTY|MI_FLAG_READONLY, /* flags */
    0x00617318, /* code */
    MI_T("AvailableRequestedStates"), /* name */
    CIM_EnabledLogicalElement_AvailableRequestedStates_quals, /* qualifiers */
    MI_COUNT(CIM_EnabledLogicalElement_AvailableRequestedStates_quals), /* numQualifiers */
    MI_UINT16A, /* type */
    NULL, /* className */
    0, /* subscript */
    offsetof(CIM_EnabledLogicalElement, AvailableRequestedStates), /* offset */
    MI_T("CIM_EnabledLogicalElement"), /* origin */
    MI_T("CIM_EnabledLogicalElement"), /* propagator */
    NULL,
};

static MI_CONST MI_Char* CIM_EnabledLogicalElement_TransitioningToState_Description_qual_value = MI_T("366");

static MI_CONST MI_Qualifier CIM_EnabledLogicalElement_TransitioningToState_Description_qual =
{
    MI_T("Description"),
    MI_STRING,
    MI_FLAG_ENABLEOVERRIDE|MI_FLAG_TOSUBCLASS|MI_FLAG_TRANSLATABLE,
    &CIM_EnabledLogicalElement_TransitioningToState_Description_qual_value
};

static MI_CONST MI_Char* CIM_EnabledLogicalElement_TransitioningToState_ValueMap_qual_data_value[] =
{
    MI_T("0"),
    MI_T("2"),
    MI_T("3"),
    MI_T("4"),
    MI_T("5"),
    MI_T("6"),
    MI_T("7"),
    MI_T("8"),
    MI_T("9"),
    MI_T("10"),
    MI_T("11"),
    MI_T("12"),
    MI_T(".."),
};

static MI_CONST MI_ConstStringA CIM_EnabledLogicalElement_TransitioningToState_ValueMap_qual_value =
{
    CIM_EnabledLogicalElement_TransitioningToState_ValueMap_qual_data_value,
    MI_COUNT(CIM_EnabledLogicalElement_TransitioningToState_ValueMap_qual_data_value),
};

static MI_CONST MI_Qualifier CIM_EnabledLogicalElement_TransitioningToState_ValueMap_qual =
{
    MI_T("ValueMap"),
    MI_STRINGA,
    MI_FLAG_ENABLEOVERRIDE|MI_FLAG_TOSUBCLASS,
    &CIM_EnabledLogicalElement_TransitioningToState_ValueMap_qual_value
};

static MI_CONST MI_Char* CIM_EnabledLogicalElement_TransitioningToState_Values_qual_data_value[] =
{
    MI_T("10"),
    MI_T("347"),
    MI_T("348"),
    MI_T("355"),
    MI_T("356"),
    MI_T("357"),
    MI_T("358"),
    MI_T("365"),
    MI_T("352"),
    MI_T("359"),
    MI_T("360"),
    MI_T("349"),
    MI_T("29"),
};

static MI_CONST MI_ConstStringA CIM_EnabledLogicalElement_TransitioningToState_Values_qual_value =
{
    CIM_EnabledLogicalElement_TransitioningToState_Values_qual_data_value,
    MI_COUNT(CIM_EnabledLogicalElement_TransitioningToState_Values_qual_data_value),
};

static MI_CONST MI_Qualifier CIM_EnabledLogicalElement_TransitioningToState_Values_qual =
{
    MI_T("Values"),
    MI_STRINGA,
    MI_FLAG_ENABLEOVERRIDE|MI_FLAG_TOSUBCLASS|MI_FLAG_TRANSLATABLE,
    &CIM_EnabledLogicalElement_TransitioningToState_Values_qual_value
};

static MI_CONST MI_Char* CIM_EnabledLogicalElement_TransitioningToState_ModelCorrespondence_qual_data_value[] =
{
    MI_T("CIM_EnabledLogicalElement.RequestStateChange"),
    MI_T("CIM_EnabledLogicalElement.RequestedState"),
    MI_T("CIM_EnabledLogicalElement.EnabledState"),
};

static MI_CONST MI_ConstStringA CIM_EnabledLogicalElement_TransitioningToState_ModelCorrespondence_qual_value =
{
    CIM_EnabledLogicalElement_TransitioningToState_ModelCorrespondence_qual_data_value,
    MI_COUNT(CIM_EnabledLogicalElement_TransitioningToState_ModelCorrespondence_qual_data_value),
};

static MI_CONST MI_Qualifier CIM_EnabledLogicalElement_TransitioningToState_ModelCorrespondence_qual =
{
    MI_T("ModelCorrespondence"),
    MI_STRINGA,
    MI_FLAG_ENABLEOVERRIDE|MI_FLAG_TOSUBCLASS,
    &CIM_EnabledLogicalElement_TransitioningToState_ModelCorrespondence_qual_value
};

static MI_Qualifier MI_CONST* MI_CONST CIM_EnabledLogicalElement_TransitioningToState_quals[] =
{
    &CIM_EnabledLogicalElement_TransitioningToState_Description_qual,
    &CIM_EnabledLogicalElement_TransitioningToState_ValueMap_qual,
    &CIM_EnabledLogicalElement_TransitioningToState_Values_qual,
    &CIM_EnabledLogicalElement_TransitioningToState_ModelCorrespondence_qual,
};

static MI_CONST MI_Uint16 CIM_EnabledLogicalElement_TransitioningToState_value = 12;

/* property CIM_EnabledLogicalElement.TransitioningToState */
static MI_CONST MI_PropertyDecl CIM_EnabledLogicalElement_TransitioningToState_prop =
{
    MI_FLAG_PROPERTY|MI_FLAG_READONLY, /* flags */
    0x00746514, /* code */
    MI_T("TransitioningToState"), /* name */
    CIM_EnabledLogicalElement_TransitioningToState_quals, /* qualifiers */
    MI_COUNT(CIM_EnabledLogicalElement_TransitioningToState_quals), /* numQualifiers */
    MI_UINT16, /* type */
    NULL, /* className */
    0, /* subscript */
    offsetof(CIM_EnabledLogicalElement, TransitioningToState), /* offset */
    MI_T("CIM_EnabledLogicalElement"), /* origin */
    MI_T("CIM_EnabledLogicalElement"), /* propagator */
    &CIM_EnabledLogicalElement_TransitioningToState_value,
};

static MI_PropertyDecl MI_CONST* MI_CONST CIM_EnabledLogicalElement_props[] =
{
    &CIM_ManagedElement_InstanceID_prop,
    &CIM_ManagedElement_Caption_prop,
    &CIM_ManagedElement_Description_prop,
    &CIM_ManagedElement_ElementName_prop,
    &CIM_ManagedSystemElement_InstallDate_prop,
    &CIM_ManagedSystemElement_Name_prop,
    &CIM_ManagedSystemElement_OperationalStatus_prop,
    &CIM_ManagedSystemElement_StatusDescriptions_prop,
    &CIM_ManagedSystemElement_Status_prop,
    &CIM_ManagedSystemElement_HealthState_prop,
    &CIM_ManagedSystemElement_CommunicationStatus_prop,
    &CIM_ManagedSystemElement_DetailedStatus_prop,
    &CIM_ManagedSystemElement_OperatingStatus_prop,
    &CIM_ManagedSystemElement_PrimaryStatus_prop,
    &CIM_EnabledLogicalElement_EnabledState_prop,
    &CIM_EnabledLogicalElement_OtherEnabledState_prop,
    &CIM_EnabledLogicalElement_RequestedState_prop,
    &CIM_EnabledLogicalElement_EnabledDefault_prop,
    &CIM_EnabledLogicalElement_TimeOfLastStateChange_prop,
    &CIM_EnabledLogicalElement_AvailableRequestedStates_prop,
    &CIM_EnabledLogicalElement_TransitioningToState_prop,
};

static MI_CONST MI_Char* CIM_EnabledLogicalElement_RequestStateChange_Description_qual_value = MI_T("367");

static MI_CONST MI_Qualifier CIM_EnabledLogicalElement_RequestStateChange_Description_qual =
{
    MI_T("Description"),
    MI_STRING,
    MI_FLAG_ENABLEOVERRIDE|MI_FLAG_TOSUBCLASS|MI_FLAG_TRANSLATABLE,
    &CIM_EnabledLogicalElement_RequestStateChange_Description_qual_value
};

static MI_CONST MI_Char* CIM_EnabledLogicalElement_RequestStateChange_ValueMap_qual_data_value[] =
{
    MI_T("0"),
    MI_T("1"),
    MI_T("2"),
    MI_T("3"),
    MI_T("4"),
    MI_T("5"),
    MI_T("6"),
    MI_T(".."),
    MI_T("4096"),
    MI_T("4097"),
    MI_T("4098"),
    MI_T("4099"),
    MI_T("4100..32767"),
    MI_T("32768..65535"),
};

static MI_CONST MI_ConstStringA CIM_EnabledLogicalElement_RequestStateChange_ValueMap_qual_value =
{
    CIM_EnabledLogicalElement_RequestStateChange_ValueMap_qual_data_value,
    MI_COUNT(CIM_EnabledLogicalElement_RequestStateChange_ValueMap_qual_data_value),
};

static MI_CONST MI_Qualifier CIM_EnabledLogicalElement_RequestStateChange_ValueMap_qual =
{
    MI_T("ValueMap"),
    MI_STRINGA,
    MI_FLAG_ENABLEOVERRIDE|MI_FLAG_TOSUBCLASS,
    &CIM_EnabledLogicalElement_RequestStateChange_ValueMap_qual_value
};

static MI_CONST MI_Char* CIM_EnabledLogicalElement_RequestStateChange_Values_qual_data_value[] =
{
    MI_T("326"),
    MI_T("115"),
    MI_T("368"),
    MI_T("369"),
    MI_T("117"),
    MI_T("329"),
    MI_T("330"),
    MI_T("29"),
    MI_T("370"),
    MI_T("332"),
    MI_T("333"),
    MI_T("334"),
    MI_T("335"),
    MI_T("120"),
};

static MI_CONST MI_ConstStringA CIM_EnabledLogicalElement_RequestStateChange_Values_qual_value =
{
    CIM_EnabledLogicalElement_RequestStateChange_Values_qual_data_value,
    MI_COUNT(CIM_EnabledLogicalElement_RequestStateChange_Values_qual_data_value),
};

static MI_CONST MI_Qualifier CIM_EnabledLogicalElement_RequestStateChange_Values_qual =
{
    MI_T("Values"),
    MI_STRINGA,
    MI_FLAG_ENABLEOVERRIDE|MI_FLAG_TOSUBCLASS|MI_FLAG_TRANSLATABLE,
    &CIM_EnabledLogicalElement_RequestStateChange_Values_qual_value
};

static MI_CONST MI_Char* CIM_EnabledLogicalElement_RequestStateChange_ModelCorrespondence_qual_data_value[] =
{
    MI_T("CIM_EnabledLogicalElement.RequestedState"),
};

static MI_CONST MI_ConstStringA CIM_EnabledLogicalElement_RequestStateChange_ModelCorrespondence_qual_value =
{
    CIM_EnabledLogicalElement_RequestStateChange_ModelCorrespondence_qual_data_value,
    MI_COUNT(CIM_EnabledLogicalElement_RequestStateChange_ModelCorrespondence_qual_data_value),
};

static MI_CONST MI_Qualifier CIM_EnabledLogicalElement_RequestStateChange_ModelCorrespondence_qual =
{
    MI_T("ModelCorrespondence"),
    MI_STRINGA,
    MI_FLAG_ENABLEOVERRIDE|MI_FLAG_TOSUBCLASS,
    &CIM_EnabledLogicalElement_RequestStateChange_ModelCorrespondence_qual_value
};

static MI_Qualifier MI_CONST* MI_CONST CIM_EnabledLogicalElement_RequestStateChange_quals[] =
{
    &CIM_EnabledLogicalElement_RequestStateChange_Description_qual,
    &CIM_EnabledLogicalElement_RequestStateChange_ValueMap_qual,
    &CIM_EnabledLogicalElement_RequestStateChange_Values_qual,
    &CIM_EnabledLogicalElement_RequestStateChange_ModelCorrespondence_qual,
};

static MI_CONST MI_Boolean CIM_EnabledLogicalElement_RequestStateChange_RequestedState_In_qual_value = 1;

static MI_CONST MI_Qualifier CIM_EnabledLogicalElement_RequestStateChange_RequestedState_In_qual =
{
    MI_T("In"),
    MI_BOOLEAN,
    MI_FLAG_DISABLEOVERRIDE|MI_FLAG_TOSUBCLASS,
    &CIM_EnabledLogicalElement_RequestStateChange_RequestedState_In_qual_value
};

static MI_CONST MI_Char* CIM_EnabledLogicalElement_RequestStateChange_RequestedState_Description_qual_value = MI_T("371");

static MI_CONST MI_Qualifier CIM_EnabledLogicalElement_RequestStateChange_RequestedState_Description_qual =
{
    MI_T("Description"),
    MI_STRING,
    MI_FLAG_ENABLEOVERRIDE|MI_FLAG_TOSUBCLASS|MI_FLAG_TRANSLATABLE,
    &CIM_EnabledLogicalElement_RequestStateChange_RequestedState_Description_qual_value
};

static MI_CONST MI_Char* CIM_EnabledLogicalElement_RequestStateChange_RequestedState_ValueMap_qual_data_value[] =
{
    MI_T("2"),
    MI_T("3"),
    MI_T("4"),
    MI_T("6"),
    MI_T("7"),
    MI_T("8"),
    MI_T("9"),
    MI_T("10"),
    MI_T("11"),
    MI_T(".."),
    MI_T("32768..65535"),
};

static MI_CONST MI_ConstStringA CIM_EnabledLogicalElement_RequestStateChange_RequestedState_ValueMap_qual_value =
{
    CIM_EnabledLogicalElement_RequestStateChange_RequestedState_ValueMap_qual_data_value,
    MI_COUNT(CIM_EnabledLogicalElement_RequestStateChange_RequestedState_ValueMap_qual_data_value),
};

static MI_CONST MI_Qualifier CIM_EnabledLogicalElement_RequestStateChange_RequestedState_ValueMap_qual =
{
    MI_T("ValueMap"),
    MI_STRINGA,
    MI_FLAG_ENABLEOVERRIDE|MI_FLAG_TOSUBCLASS,
    &CIM_EnabledLogicalElement_RequestStateChange_RequestedState_ValueMap_qual_value
};

static MI_CONST MI_Char* CIM_EnabledLogicalElement_RequestStateChange_RequestedState_Values_qual_data_value[] =
{
    MI_T("347"),
    MI_T("348"),
    MI_T("355"),
    MI_T("357"),
    MI_T("358"),
    MI_T("365"),
    MI_T("352"),
    MI_T("359"),
    MI_T("360"),
    MI_T("29"),
    MI_T("30"),
};

static MI_CONST MI_ConstStringA CIM_EnabledLogicalElement_RequestStateChange_RequestedState_Values_qual_value =
{
    CIM_EnabledLogicalElement_RequestStateChange_RequestedState_Values_qual_data_value,
    MI_COUNT(CIM_EnabledLogicalElement_RequestStateChange_RequestedState_Values_qual_data_value),
};

static MI_CONST MI_Qualifier CIM_EnabledLogicalElement_RequestStateChange_RequestedState_Values_qual =
{
    MI_T("Values"),
    MI_STRINGA,
    MI_FLAG_ENABLEOVERRIDE|MI_FLAG_TOSUBCLASS|MI_FLAG_TRANSLATABLE,
    &CIM_EnabledLogicalElement_RequestStateChange_RequestedState_Values_qual_value
};

static MI_CONST MI_Char* CIM_EnabledLogicalElement_RequestStateChange_RequestedState_ModelCorrespondence_qual_data_value[] =
{
    MI_T("CIM_EnabledLogicalElement.RequestedState"),
};

static MI_CONST MI_ConstStringA CIM_EnabledLogicalElement_RequestStateChange_RequestedState_ModelCorrespondence_qual_value =
{
    CIM_EnabledLogicalElement_RequestStateChange_RequestedState_ModelCorrespondence_qual_data_value,
    MI_COUNT(CIM_EnabledLogicalElement_RequestStateChange_RequestedState_ModelCorrespondence_qual_data_value),
};

static MI_CONST MI_Qualifier CIM_EnabledLogicalElement_RequestStateChange_RequestedState_ModelCorrespondence_qual =
{
    MI_T("ModelCorrespondence"),
    MI_STRINGA,
    MI_FLAG_ENABLEOVERRIDE|MI_FLAG_TOSUBCLASS,
    &CIM_EnabledLogicalElement_RequestStateChange_RequestedState_ModelCorrespondence_qual_value
};

static MI_Qualifier MI_CONST* MI_CONST CIM_EnabledLogicalElement_RequestStateChange_RequestedState_quals[] =
{
    &CIM_EnabledLogicalElement_RequestStateChange_RequestedState_In_qual,
    &CIM_EnabledLogicalElement_RequestStateChange_RequestedState_Description_qual,
    &CIM_EnabledLogicalElement_RequestStateChange_RequestedState_ValueMap_qual,
    &CIM_EnabledLogicalElement_RequestStateChange_RequestedState_Values_qual,
    &CIM_EnabledLogicalElement_RequestStateChange_RequestedState_ModelCorrespondence_qual,
};

/* parameter CIM_EnabledLogicalElement.RequestStateChange(): RequestedState */
static MI_CONST MI_ParameterDecl CIM_EnabledLogicalElement_RequestStateChange_RequestedState_param =
{
    MI_FLAG_PARAMETER|MI_FLAG_IN, /* flags */
    0x0072650E, /* code */
    MI_T("RequestedState"), /* name */
    CIM_EnabledLogicalElement_RequestStateChange_RequestedState_quals, /* qualifiers */
    MI_COUNT(CIM_EnabledLogicalElement_RequestStateChange_RequestedState_quals), /* numQualifiers */
    MI_UINT16, /* type */
    NULL, /* className */
    0, /* subscript */
    offsetof(CIM_EnabledLogicalElement_RequestStateChange, RequestedState), /* offset */
};

static MI_CONST MI_Boolean CIM_EnabledLogicalElement_RequestStateChange_Job_In_qual_value = 0;

static MI_CONST MI_Qualifier CIM_EnabledLogicalElement_RequestStateChange_Job_In_qual =
{
    MI_T("In"),
    MI_BOOLEAN,
    MI_FLAG_DISABLEOVERRIDE|MI_FLAG_TOSUBCLASS,
    &CIM_EnabledLogicalElement_RequestStateChange_Job_In_qual_value
};

static MI_CONST MI_Boolean CIM_EnabledLogicalElement_RequestStateChange_Job_Out_qual_value = 1;

static MI_CONST MI_Qualifier CIM_EnabledLogicalElement_RequestStateChange_Job_Out_qual =
{
    MI_T("Out"),
    MI_BOOLEAN,
    MI_FLAG_DISABLEOVERRIDE|MI_FLAG_TOSUBCLASS,
    &CIM_EnabledLogicalElement_RequestStateChange_Job_Out_qual_value
};

static MI_CONST MI_Char* CIM_EnabledLogicalElement_RequestStateChange_Job_Description_qual_value = MI_T("372");

static MI_CONST MI_Qualifier CIM_EnabledLogicalElement_RequestStateChange_Job_Description_qual =
{
    MI_T("Description"),
    MI_STRING,
    MI_FLAG_ENABLEOVERRIDE|MI_FLAG_TOSUBCLASS|MI_FLAG_TRANSLATABLE,
    &CIM_EnabledLogicalElement_RequestStateChange_Job_Description_qual_value
};

static MI_Qualifier MI_CONST* MI_CONST CIM_EnabledLogicalElement_RequestStateChange_Job_quals[] =
{
    &CIM_EnabledLogicalElement_RequestStateChange_Job_In_qual,
    &CIM_EnabledLogicalElement_RequestStateChange_Job_Out_qual,
    &CIM_EnabledLogicalElement_RequestStateChange_Job_Description_qual,
};

/* parameter CIM_EnabledLogicalElement.RequestStateChange(): Job */
static MI_CONST MI_ParameterDecl CIM_EnabledLogicalElement_RequestStateChange_Job_param =
{
    MI_FLAG_PARAMETER|MI_FLAG_OUT, /* flags */
    0x006A6203, /* code */
    MI_T("Job"), /* name */
    CIM_EnabledLogicalElement_RequestStateChange_Job_quals, /* qualifiers */
    MI_COUNT(CIM_EnabledLogicalElement_RequestStateChange_Job_quals), /* numQualifiers */
    MI_REFERENCE, /* type */
    MI_T("CIM_ConcreteJob"), /* className */
    0, /* subscript */
    offsetof(CIM_EnabledLogicalElement_RequestStateChange, Job), /* offset */
};

static MI_CONST MI_Boolean CIM_EnabledLogicalElement_RequestStateChange_TimeoutPeriod_In_qual_value = 1;

static MI_CONST MI_Qualifier CIM_EnabledLogicalElement_RequestStateChange_TimeoutPeriod_In_qual =
{
    MI_T("In"),
    MI_BOOLEAN,
    MI_FLAG_DISABLEOVERRIDE|MI_FLAG_TOSUBCLASS,
    &CIM_EnabledLogicalElement_RequestStateChange_TimeoutPeriod_In_qual_value
};

static MI_CONST MI_Char* CIM_EnabledLogicalElement_RequestStateChange_TimeoutPeriod_Description_qual_value = MI_T("373");

static MI_CONST MI_Qualifier CIM_EnabledLogicalElement_RequestStateChange_TimeoutPeriod_Description_qual =
{
    MI_T("Description"),
    MI_STRING,
    MI_FLAG_ENABLEOVERRIDE|MI_FLAG_TOSUBCLASS|MI_FLAG_TRANSLATABLE,
    &CIM_EnabledLogicalElement_RequestStateChange_TimeoutPeriod_Description_qual_value
};

static MI_Qualifier MI_CONST* MI_CONST CIM_EnabledLogicalElement_RequestStateChange_TimeoutPeriod_quals[] =
{
    &CIM_EnabledLogicalElement_RequestStateChange_TimeoutPeriod_In_qual,
    &CIM_EnabledLogicalElement_RequestStateChange_TimeoutPeriod_Description_qual,
};

/* parameter CIM_EnabledLogicalElement.RequestStateChange(): TimeoutPeriod */
static MI_CONST MI_ParameterDecl CIM_EnabledLogicalElement_RequestStateChange_TimeoutPeriod_param =
{
    MI_FLAG_PARAMETER|MI_FLAG_IN, /* flags */
    0x0074640D, /* code */
    MI_T("TimeoutPeriod"), /* name */
    CIM_EnabledLogicalElement_RequestStateChange_TimeoutPeriod_quals, /* qualifiers */
    MI_COUNT(CIM_EnabledLogicalElement_RequestStateChange_TimeoutPeriod_quals), /* numQualifiers */
    MI_DATETIME, /* type */
    NULL, /* className */
    0, /* subscript */
    offsetof(CIM_EnabledLogicalElement_RequestStateChange, TimeoutPeriod), /* offset */
};

static MI_CONST MI_Char* CIM_EnabledLogicalElement_RequestStateChange_MIReturn_Description_qual_value = MI_T("367");

static MI_CONST MI_Qualifier CIM_EnabledLogicalElement_RequestStateChange_MIReturn_Description_qual =
{
    MI_T("Description"),
    MI_STRING,
    MI_FLAG_ENABLEOVERRIDE|MI_FLAG_TOSUBCLASS|MI_FLAG_TRANSLATABLE,
    &CIM_EnabledLogicalElement_RequestStateChange_MIReturn_Description_qual_value
};

static MI_CONST MI_Char* CIM_EnabledLogicalElement_RequestStateChange_MIReturn_ValueMap_qual_data_value[] =
{
    MI_T("0"),
    MI_T("1"),
    MI_T("2"),
    MI_T("3"),
    MI_T("4"),
    MI_T("5"),
    MI_T("6"),
    MI_T(".."),
    MI_T("4096"),
    MI_T("4097"),
    MI_T("4098"),
    MI_T("4099"),
    MI_T("4100..32767"),
    MI_T("32768..65535"),
};

static MI_CONST MI_ConstStringA CIM_EnabledLogicalElement_RequestStateChange_MIReturn_ValueMap_qual_value =
{
    CIM_EnabledLogicalElement_RequestStateChange_MIReturn_ValueMap_qual_data_value,
    MI_COUNT(CIM_EnabledLogicalElement_RequestStateChange_MIReturn_ValueMap_qual_data_value),
};

static MI_CONST MI_Qualifier CIM_EnabledLogicalElement_RequestStateChange_MIReturn_ValueMap_qual =
{
    MI_T("ValueMap"),
    MI_STRINGA,
    MI_FLAG_ENABLEOVERRIDE|MI_FLAG_TOSUBCLASS,
    &CIM_EnabledLogicalElement_RequestStateChange_MIReturn_ValueMap_qual_value
};

static MI_CONST MI_Char* CIM_EnabledLogicalElement_RequestStateChange_MIReturn_Values_qual_data_value[] =
{
    MI_T("326"),
    MI_T("115"),
    MI_T("368"),
    MI_T("369"),
    MI_T("117"),
    MI_T("329"),
    MI_T("330"),
    MI_T("29"),
    MI_T("370"),
    MI_T("332"),
    MI_T("333"),
    MI_T("334"),
    MI_T("335"),
    MI_T("120"),
};

static MI_CONST MI_ConstStringA CIM_EnabledLogicalElement_RequestStateChange_MIReturn_Values_qual_value =
{
    CIM_EnabledLogicalElement_RequestStateChange_MIReturn_Values_qual_data_value,
    MI_COUNT(CIM_EnabledLogicalElement_RequestStateChange_MIReturn_Values_qual_data_value),
};

static MI_CONST MI_Qualifier CIM_EnabledLogicalElement_RequestStateChange_MIReturn_Values_qual =
{
    MI_T("Values"),
    MI_STRINGA,
    MI_FLAG_ENABLEOVERRIDE|MI_FLAG_TOSUBCLASS|MI_FLAG_TRANSLATABLE,
    &CIM_EnabledLogicalElement_RequestStateChange_MIReturn_Values_qual_value
};

static MI_CONST MI_Char* CIM_EnabledLogicalElement_RequestStateChange_MIReturn_ModelCorrespondence_qual_data_value[] =
{
    MI_T("CIM_EnabledLogicalElement.RequestedState"),
};

static MI_CONST MI_ConstStringA CIM_EnabledLogicalElement_RequestStateChange_MIReturn_ModelCorrespondence_qual_value =
{
    CIM_EnabledLogicalElement_RequestStateChange_MIReturn_ModelCorrespondence_qual_data_value,
    MI_COUNT(CIM_EnabledLogicalElement_RequestStateChange_MIReturn_ModelCorrespondence_qual_data_value),
};

static MI_CONST MI_Qualifier CIM_EnabledLogicalElement_RequestStateChange_MIReturn_ModelCorrespondence_qual =
{
    MI_T("ModelCorrespondence"),
    MI_STRINGA,
    MI_FLAG_ENABLEOVERRIDE|MI_FLAG_TOSUBCLASS,
    &CIM_EnabledLogicalElement_RequestStateChange_MIReturn_ModelCorrespondence_qual_value
};

static MI_Qualifier MI_CONST* MI_CONST CIM_EnabledLogicalElement_RequestStateChange_MIReturn_quals[] =
{
    &CIM_EnabledLogicalElement_RequestStateChange_MIReturn_Description_qual,
    &CIM_EnabledLogicalElement_RequestStateChange_MIReturn_ValueMap_qual,
    &CIM_EnabledLogicalElement_RequestStateChange_MIReturn_Values_qual,
    &CIM_EnabledLogicalElement_RequestStateChange_MIReturn_ModelCorrespondence_qual,
};

/* parameter CIM_EnabledLogicalElement.RequestStateChange(): MIReturn */
static MI_CONST MI_ParameterDecl CIM_EnabledLogicalElement_RequestStateChange_MIReturn_param =
{
    MI_FLAG_PARAMETER|MI_FLAG_OUT, /* flags */
    0x006D6E08, /* code */
    MI_T("MIReturn"), /* name */
    CIM_EnabledLogicalElement_RequestStateChange_MIReturn_quals, /* qualifiers */
    MI_COUNT(CIM_EnabledLogicalElement_RequestStateChange_MIReturn_quals), /* numQualifiers */
    MI_UINT32, /* type */
    NULL, /* className */
    0, /* subscript */
    offsetof(CIM_EnabledLogicalElement_RequestStateChange, MIReturn), /* offset */
};

static MI_ParameterDecl MI_CONST* MI_CONST CIM_EnabledLogicalElement_RequestStateChange_params[] =
{
    &CIM_EnabledLogicalElement_RequestStateChange_MIReturn_param,
    &CIM_EnabledLogicalElement_RequestStateChange_RequestedState_param,
    &CIM_EnabledLogicalElement_RequestStateChange_Job_param,
    &CIM_EnabledLogicalElement_RequestStateChange_TimeoutPeriod_param,
};

/* method CIM_EnabledLogicalElement.RequestStateChange() */
MI_CONST MI_MethodDecl CIM_EnabledLogicalElement_RequestStateChange_rtti =
{
    MI_FLAG_METHOD, /* flags */
    0x00726512, /* code */
    MI_T("RequestStateChange"), /* name */
    CIM_EnabledLogicalElement_RequestStateChange_quals, /* qualifiers */
    MI_COUNT(CIM_EnabledLogicalElement_RequestStateChange_quals), /* numQualifiers */
    CIM_EnabledLogicalElement_RequestStateChange_params, /* parameters */
    MI_COUNT(CIM_EnabledLogicalElement_RequestStateChange_params), /* numParameters */
    sizeof(CIM_EnabledLogicalElement_RequestStateChange), /* size */
    MI_UINT32, /* returnType */
    MI_T("CIM_EnabledLogicalElement"), /* origin */
    MI_T("CIM_EnabledLogicalElement"), /* propagator */
    &schemaDecl, /* schema */
    NULL, /* method */
};

static MI_MethodDecl MI_CONST* MI_CONST CIM_EnabledLogicalElement_meths[] =
{
    &CIM_EnabledLogicalElement_RequestStateChange_rtti,
};

static MI_CONST MI_Char* CIM_EnabledLogicalElement_UMLPackagePath_qual_value = MI_T("CIM::Core::CoreElements");

static MI_CONST MI_Qualifier CIM_EnabledLogicalElement_UMLPackagePath_qual =
{
    MI_T("UMLPackagePath"),
    MI_STRING,
    MI_FLAG_ENABLEOVERRIDE|MI_FLAG_TOSUBCLASS,
    &CIM_EnabledLogicalElement_UMLPackagePath_qual_value
};

static MI_CONST MI_Char* CIM_EnabledLogicalElement_Description_qual_value = MI_T("374");

static MI_CONST MI_Qualifier CIM_EnabledLogicalElement_Description_qual =
{
    MI_T("Description"),
    MI_STRING,
    MI_FLAG_ENABLEOVERRIDE|MI_FLAG_TOSUBCLASS|MI_FLAG_TRANSLATABLE,
    &CIM_EnabledLogicalElement_Description_qual_value
};

static MI_CONST MI_Boolean CIM_EnabledLogicalElement_Abstract_qual_value = 1;

static MI_CONST MI_Qualifier CIM_EnabledLogicalElement_Abstract_qual =
{
    MI_T("Abstract"),
    MI_BOOLEAN,
    MI_FLAG_ENABLEOVERRIDE|MI_FLAG_RESTRICTED,
    &CIM_EnabledLogicalElement_Abstract_qual_value
};

static MI_CONST MI_Char* CIM_EnabledLogicalElement_Version_qual_value = MI_T("55");

static MI_CONST MI_Qualifier CIM_EnabledLogicalElement_Version_qual =
{
    MI_T("Version"),
    MI_STRING,
    MI_FLAG_ENABLEOVERRIDE|MI_FLAG_TRANSLATABLE|MI_FLAG_RESTRICTED,
    &CIM_EnabledLogicalElement_Version_qual_value
};

static MI_Qualifier MI_CONST* MI_CONST CIM_EnabledLogicalElement_quals[] =
{
    &CIM_EnabledLogicalElement_UMLPackagePath_qual,
    &CIM_EnabledLogicalElement_Description_qual,
    &CIM_EnabledLogicalElement_Abstract_qual,
    &CIM_EnabledLogicalElement_Version_qual,
};

/* class CIM_EnabledLogicalElement */
MI_CONST MI_ClassDecl CIM_EnabledLogicalElement_rtti =
{
    MI_FLAG_CLASS|MI_FLAG_ABSTRACT, /* flags */
    0x00637419, /* code */
    MI_T("CIM_EnabledLogicalElement"), /* name */
    CIM_EnabledLogicalElement_quals, /* qualifiers */
    MI_COUNT(CIM_EnabledLogicalElement_quals), /* numQualifiers */
    CIM_EnabledLogicalElement_props, /* properties */
    MI_COUNT(CIM_EnabledLogicalElement_props), /* numProperties */
    sizeof(CIM_EnabledLogicalElement), /* size */
    MI_T("CIM_LogicalElement"), /* superClass */
    &CIM_LogicalElement_rtti, /* superClassDecl */
    CIM_EnabledLogicalElement_meths, /* methods */
    MI_COUNT(CIM_EnabledLogicalElement_meths), /* numMethods */
    &schemaDecl, /* schema */
    NULL, /* functions */
    NULL /* owningClass */
};

/*
**==============================================================================
**
** CIM_Service
**
**==============================================================================
*/

static MI_CONST MI_Char* CIM_Service_Name_Description_qual_value = MI_T("375");

static MI_CONST MI_Qualifier CIM_Service_Name_Description_qual =
{
    MI_T("Description"),
    MI_STRING,
    MI_FLAG_ENABLEOVERRIDE|MI_FLAG_TOSUBCLASS|MI_FLAG_TRANSLATABLE,
    &CIM_Service_Name_Description_qual_value
};

static MI_CONST MI_Uint32 CIM_Service_Name_MaxLen_qual_value = 256U;

static MI_CONST MI_Qualifier CIM_Service_Name_MaxLen_qual =
{
    MI_T("MaxLen"),
    MI_UINT32,
    MI_FLAG_ENABLEOVERRIDE|MI_FLAG_TOSUBCLASS,
    &CIM_Service_Name_MaxLen_qual_value
};

static MI_CONST MI_Boolean CIM_Service_Name_Key_qual_value = 1;

static MI_CONST MI_Qualifier CIM_Service_Name_Key_qual =
{
    MI_T("Key"),
    MI_BOOLEAN,
    MI_FLAG_DISABLEOVERRIDE|MI_FLAG_TOSUBCLASS,
    &CIM_Service_Name_Key_qual_value
};

static MI_CONST MI_Char* CIM_Service_Name_Override_qual_value = MI_T("Name");

static MI_CONST MI_Qualifier CIM_Service_Name_Override_qual =
{
    MI_T("Override"),
    MI_STRING,
    MI_FLAG_ENABLEOVERRIDE|MI_FLAG_RESTRICTED,
    &CIM_Service_Name_Override_qual_value
};

static MI_Qualifier MI_CONST* MI_CONST CIM_Service_Name_quals[] =
{
    &CIM_Service_Name_Description_qual,
    &CIM_Service_Name_MaxLen_qual,
    &CIM_Service_Name_Key_qual,
    &CIM_Service_Name_Override_qual,
};

/* property CIM_Service.Name */
static MI_CONST MI_PropertyDecl CIM_Service_Name_prop =
{
    MI_FLAG_PROPERTY|MI_FLAG_KEY|MI_FLAG_READONLY, /* flags */
    0x006E6504, /* code */
    MI_T("Name"), /* name */
    CIM_Service_Name_quals, /* qualifiers */
    MI_COUNT(CIM_Service_Name_quals), /* numQualifiers */
    MI_STRING, /* type */
    NULL, /* className */
    0, /* subscript */
    offsetof(CIM_Service, Name), /* offset */
    MI_T("CIM_ManagedSystemElement"), /* origin */
    MI_T("CIM_Service"), /* propagator */
    NULL,
};

static MI_CONST MI_Boolean CIM_Service_SystemCreationClassName_Key_qual_value = 1;

static MI_CONST MI_Qualifier CIM_Service_SystemCreationClassName_Key_qual =
{
    MI_T("Key"),
    MI_BOOLEAN,
    MI_FLAG_DISABLEOVERRIDE|MI_FLAG_TOSUBCLASS,
    &CIM_Service_SystemCreationClassName_Key_qual_value
};

static MI_CONST MI_Char* CIM_Service_SystemCreationClassName_Description_qual_value = MI_T("376");

static MI_CONST MI_Qualifier CIM_Service_SystemCreationClassName_Description_qual =
{
    MI_T("Description"),
    MI_STRING,
    MI_FLAG_ENABLEOVERRIDE|MI_FLAG_TOSUBCLASS|MI_FLAG_TRANSLATABLE,
    &CIM_Service_SystemCreationClassName_Description_qual_value
};

static MI_CONST MI_Uint32 CIM_Service_SystemCreationClassName_MaxLen_qual_value = 256U;

static MI_CONST MI_Qualifier CIM_Service_SystemCreationClassName_MaxLen_qual =
{
    MI_T("MaxLen"),
    MI_UINT32,
    MI_FLAG_ENABLEOVERRIDE|MI_FLAG_TOSUBCLASS,
    &CIM_Service_SystemCreationClassName_MaxLen_qual_value
};

static MI_CONST MI_Char* CIM_Service_SystemCreationClassName_Propagated_qual_value = MI_T("CIM_System.CreationClassName");

static MI_CONST MI_Qualifier CIM_Service_SystemCreationClassName_Propagated_qual =
{
    MI_T("Propagated"),
    MI_STRING,
    MI_FLAG_DISABLEOVERRIDE|MI_FLAG_TOSUBCLASS,
    &CIM_Service_SystemCreationClassName_Propagated_qual_value
};

static MI_Qualifier MI_CONST* MI_CONST CIM_Service_SystemCreationClassName_quals[] =
{
    &CIM_Service_SystemCreationClassName_Key_qual,
    &CIM_Service_SystemCreationClassName_Description_qual,
    &CIM_Service_SystemCreationClassName_MaxLen_qual,
    &CIM_Service_SystemCreationClassName_Propagated_qual,
};

/* property CIM_Service.SystemCreationClassName */
static MI_CONST MI_PropertyDecl CIM_Service_SystemCreationClassName_prop =
{
    MI_FLAG_PROPERTY|MI_FLAG_KEY|MI_FLAG_READONLY, /* flags */
    0x00736517, /* code */
    MI_T("SystemCreationClassName"), /* name */
    CIM_Service_SystemCreationClassName_quals, /* qualifiers */
    MI_COUNT(CIM_Service_SystemCreationClassName_quals), /* numQualifiers */
    MI_STRING, /* type */
    NULL, /* className */
    0, /* subscript */
    offsetof(CIM_Service, SystemCreationClassName), /* offset */
    MI_T("CIM_Service"), /* origin */
    MI_T("CIM_Service"), /* propagator */
    NULL,
};

static MI_CONST MI_Boolean CIM_Service_SystemName_Key_qual_value = 1;

static MI_CONST MI_Qualifier CIM_Service_SystemName_Key_qual =
{
    MI_T("Key"),
    MI_BOOLEAN,
    MI_FLAG_DISABLEOVERRIDE|MI_FLAG_TOSUBCLASS,
    &CIM_Service_SystemName_Key_qual_value
};

static MI_CONST MI_Char* CIM_Service_SystemName_Description_qual_value = MI_T("377");

static MI_CONST MI_Qualifier CIM_Service_SystemName_Description_qual =
{
    MI_T("Description"),
    MI_STRING,
    MI_FLAG_ENABLEOVERRIDE|MI_FLAG_TOSUBCLASS|MI_FLAG_TRANSLATABLE,
    &CIM_Service_SystemName_Description_qual_value
};

static MI_CONST MI_Uint32 CIM_Service_SystemName_MaxLen_qual_value = 256U;

static MI_CONST MI_Qualifier CIM_Service_SystemName_MaxLen_qual =
{
    MI_T("MaxLen"),
    MI_UINT32,
    MI_FLAG_ENABLEOVERRIDE|MI_FLAG_TOSUBCLASS,
    &CIM_Service_SystemName_MaxLen_qual_value
};

static MI_CONST MI_Char* CIM_Service_SystemName_Propagated_qual_value = MI_T("CIM_System.Name");

static MI_CONST MI_Qualifier CIM_Service_SystemName_Propagated_qual =
{
    MI_T("Propagated"),
    MI_STRING,
    MI_FLAG_DISABLEOVERRIDE|MI_FLAG_TOSUBCLASS,
    &CIM_Service_SystemName_Propagated_qual_value
};

static MI_Qualifier MI_CONST* MI_CONST CIM_Service_SystemName_quals[] =
{
    &CIM_Service_SystemName_Key_qual,
    &CIM_Service_SystemName_Description_qual,
    &CIM_Service_SystemName_MaxLen_qual,
    &CIM_Service_SystemName_Propagated_qual,
};

/* property CIM_Service.SystemName */
static MI_CONST MI_PropertyDecl CIM_Service_SystemName_prop =
{
    MI_FLAG_PROPERTY|MI_FLAG_KEY|MI_FLAG_READONLY, /* flags */
    0x0073650A, /* code */
    MI_T("SystemName"), /* name */
    CIM_Service_SystemName_quals, /* qualifiers */
    MI_COUNT(CIM_Service_SystemName_quals), /* numQualifiers */
    MI_STRING, /* type */
    NULL, /* className */
    0, /* subscript */
    offsetof(CIM_Service, SystemName), /* offset */
    MI_T("CIM_Service"), /* origin */
    MI_T("CIM_Service"), /* propagator */
    NULL,
};

static MI_CONST MI_Boolean CIM_Service_CreationClassName_Key_qual_value = 1;

static MI_CONST MI_Qualifier CIM_Service_CreationClassName_Key_qual =
{
    MI_T("Key"),
    MI_BOOLEAN,
    MI_FLAG_DISABLEOVERRIDE|MI_FLAG_TOSUBCLASS,
    &CIM_Service_CreationClassName_Key_qual_value
};

static MI_CONST MI_Char* CIM_Service_CreationClassName_Description_qual_value = MI_T("378");

static MI_CONST MI_Qualifier CIM_Service_CreationClassName_Description_qual =
{
    MI_T("Description"),
    MI_STRING,
    MI_FLAG_ENABLEOVERRIDE|MI_FLAG_TOSUBCLASS|MI_FLAG_TRANSLATABLE,
    &CIM_Service_CreationClassName_Description_qual_value
};

static MI_CONST MI_Uint32 CIM_Service_CreationClassName_MaxLen_qual_value = 256U;

static MI_CONST MI_Qualifier CIM_Service_CreationClassName_MaxLen_qual =
{
    MI_T("MaxLen"),
    MI_UINT32,
    MI_FLAG_ENABLEOVERRIDE|MI_FLAG_TOSUBCLASS,
    &CIM_Service_CreationClassName_MaxLen_qual_value
};

static MI_Qualifier MI_CONST* MI_CONST CIM_Service_CreationClassName_quals[] =
{
    &CIM_Service_CreationClassName_Key_qual,
    &CIM_Service_CreationClassName_Description_qual,
    &CIM_Service_CreationClassName_MaxLen_qual,
};

/* property CIM_Service.CreationClassName */
static MI_CONST MI_PropertyDecl CIM_Service_CreationClassName_prop =
{
    MI_FLAG_PROPERTY|MI_FLAG_KEY|MI_FLAG_READONLY, /* flags */
    0x00636511, /* code */
    MI_T("CreationClassName"), /* name */
    CIM_Service_CreationClassName_quals, /* qualifiers */
    MI_COUNT(CIM_Service_CreationClassName_quals), /* numQualifiers */
    MI_STRING, /* type */
    NULL, /* className */
    0, /* subscript */
    offsetof(CIM_Service, CreationClassName), /* offset */
    MI_T("CIM_Service"), /* origin */
    MI_T("CIM_Service"), /* propagator */
    NULL,
};

static MI_CONST MI_Boolean CIM_Service_PrimaryOwnerName_Write_qual_value = 1;

static MI_CONST MI_Qualifier CIM_Service_PrimaryOwnerName_Write_qual =
{
    MI_T("Write"),
    MI_BOOLEAN,
    MI_FLAG_ENABLEOVERRIDE|MI_FLAG_TOSUBCLASS,
    &CIM_Service_PrimaryOwnerName_Write_qual_value
};

static MI_CONST MI_Char* CIM_Service_PrimaryOwnerName_Description_qual_value = MI_T("379");

static MI_CONST MI_Qualifier CIM_Service_PrimaryOwnerName_Description_qual =
{
    MI_T("Description"),
    MI_STRING,
    MI_FLAG_ENABLEOVERRIDE|MI_FLAG_TOSUBCLASS|MI_FLAG_TRANSLATABLE,
    &CIM_Service_PrimaryOwnerName_Description_qual_value
};

static MI_CONST MI_Uint32 CIM_Service_PrimaryOwnerName_MaxLen_qual_value = 64U;

static MI_CONST MI_Qualifier CIM_Service_PrimaryOwnerName_MaxLen_qual =
{
    MI_T("MaxLen"),
    MI_UINT32,
    MI_FLAG_ENABLEOVERRIDE|MI_FLAG_TOSUBCLASS,
    &CIM_Service_PrimaryOwnerName_MaxLen_qual_value
};

static MI_Qualifier MI_CONST* MI_CONST CIM_Service_PrimaryOwnerName_quals[] =
{
    &CIM_Service_PrimaryOwnerName_Write_qual,
    &CIM_Service_PrimaryOwnerName_Description_qual,
    &CIM_Service_PrimaryOwnerName_MaxLen_qual,
};

/* property CIM_Service.PrimaryOwnerName */
static MI_CONST MI_PropertyDecl CIM_Service_PrimaryOwnerName_prop =
{
    MI_FLAG_PROPERTY, /* flags */
    0x00706510, /* code */
    MI_T("PrimaryOwnerName"), /* name */
    CIM_Service_PrimaryOwnerName_quals, /* qualifiers */
    MI_COUNT(CIM_Service_PrimaryOwnerName_quals), /* numQualifiers */
    MI_STRING, /* type */
    NULL, /* className */
    0, /* subscript */
    offsetof(CIM_Service, PrimaryOwnerName), /* offset */
    MI_T("CIM_Service"), /* origin */
    MI_T("CIM_Service"), /* propagator */
    NULL,
};

static MI_CONST MI_Boolean CIM_Service_PrimaryOwnerContact_Write_qual_value = 1;

static MI_CONST MI_Qualifier CIM_Service_PrimaryOwnerContact_Write_qual =
{
    MI_T("Write"),
    MI_BOOLEAN,
    MI_FLAG_ENABLEOVERRIDE|MI_FLAG_TOSUBCLASS,
    &CIM_Service_PrimaryOwnerContact_Write_qual_value
};

static MI_CONST MI_Char* CIM_Service_PrimaryOwnerContact_Description_qual_value = MI_T("380");

static MI_CONST MI_Qualifier CIM_Service_PrimaryOwnerContact_Description_qual =
{
    MI_T("Description"),
    MI_STRING,
    MI_FLAG_ENABLEOVERRIDE|MI_FLAG_TOSUBCLASS|MI_FLAG_TRANSLATABLE,
    &CIM_Service_PrimaryOwnerContact_Description_qual_value
};

static MI_CONST MI_Uint32 CIM_Service_PrimaryOwnerContact_MaxLen_qual_value = 256U;

static MI_CONST MI_Qualifier CIM_Service_PrimaryOwnerContact_MaxLen_qual =
{
    MI_T("MaxLen"),
    MI_UINT32,
    MI_FLAG_ENABLEOVERRIDE|MI_FLAG_TOSUBCLASS,
    &CIM_Service_PrimaryOwnerContact_MaxLen_qual_value
};

static MI_Qualifier MI_CONST* MI_CONST CIM_Service_PrimaryOwnerContact_quals[] =
{
    &CIM_Service_PrimaryOwnerContact_Write_qual,
    &CIM_Service_PrimaryOwnerContact_Description_qual,
    &CIM_Service_PrimaryOwnerContact_MaxLen_qual,
};

/* property CIM_Service.PrimaryOwnerContact */
static MI_CONST MI_PropertyDecl CIM_Service_PrimaryOwnerContact_prop =
{
    MI_FLAG_PROPERTY, /* flags */
    0x00707413, /* code */
    MI_T("PrimaryOwnerContact"), /* name */
    CIM_Service_PrimaryOwnerContact_quals, /* qualifiers */
    MI_COUNT(CIM_Service_PrimaryOwnerContact_quals), /* numQualifiers */
    MI_STRING, /* type */
    NULL, /* className */
    0, /* subscript */
    offsetof(CIM_Service, PrimaryOwnerContact), /* offset */
    MI_T("CIM_Service"), /* origin */
    MI_T("CIM_Service"), /* propagator */
    NULL,
};

static MI_CONST MI_Char* CIM_Service_StartMode_Deprecated_qual_data_value[] =
{
    MI_T("CIM_Service.EnabledDefault"),
};

static MI_CONST MI_ConstStringA CIM_Service_StartMode_Deprecated_qual_value =
{
    CIM_Service_StartMode_Deprecated_qual_data_value,
    MI_COUNT(CIM_Service_StartMode_Deprecated_qual_data_value),
};

static MI_CONST MI_Qualifier CIM_Service_StartMode_Deprecated_qual =
{
    MI_T("Deprecated"),
    MI_STRINGA,
    MI_FLAG_ENABLEOVERRIDE|MI_FLAG_RESTRICTED,
    &CIM_Service_StartMode_Deprecated_qual_value
};

static MI_CONST MI_Char* CIM_Service_StartMode_Description_qual_value = MI_T("381");

static MI_CONST MI_Qualifier CIM_Service_StartMode_Description_qual =
{
    MI_T("Description"),
    MI_STRING,
    MI_FLAG_ENABLEOVERRIDE|MI_FLAG_TOSUBCLASS|MI_FLAG_TRANSLATABLE,
    &CIM_Service_StartMode_Description_qual_value
};

static MI_CONST MI_Char* CIM_Service_StartMode_ValueMap_qual_data_value[] =
{
    MI_T("Automatic"),
    MI_T("Manual"),
};

static MI_CONST MI_ConstStringA CIM_Service_StartMode_ValueMap_qual_value =
{
    CIM_Service_StartMode_ValueMap_qual_data_value,
    MI_COUNT(CIM_Service_StartMode_ValueMap_qual_data_value),
};

static MI_CONST MI_Qualifier CIM_Service_StartMode_ValueMap_qual =
{
    MI_T("ValueMap"),
    MI_STRINGA,
    MI_FLAG_ENABLEOVERRIDE|MI_FLAG_TOSUBCLASS,
    &CIM_Service_StartMode_ValueMap_qual_value
};

static MI_CONST MI_Uint32 CIM_Service_StartMode_MaxLen_qual_value = 10U;

static MI_CONST MI_Qualifier CIM_Service_StartMode_MaxLen_qual =
{
    MI_T("MaxLen"),
    MI_UINT32,
    MI_FLAG_ENABLEOVERRIDE|MI_FLAG_TOSUBCLASS,
    &CIM_Service_StartMode_MaxLen_qual_value
};

static MI_Qualifier MI_CONST* MI_CONST CIM_Service_StartMode_quals[] =
{
    &CIM_Service_StartMode_Deprecated_qual,
    &CIM_Service_StartMode_Description_qual,
    &CIM_Service_StartMode_ValueMap_qual,
    &CIM_Service_StartMode_MaxLen_qual,
};

/* property CIM_Service.StartMode */
static MI_CONST MI_PropertyDecl CIM_Service_StartMode_prop =
{
    MI_FLAG_PROPERTY|MI_FLAG_READONLY, /* flags */
    0x00736509, /* code */
    MI_T("StartMode"), /* name */
    CIM_Service_StartMode_quals, /* qualifiers */
    MI_COUNT(CIM_Service_StartMode_quals), /* numQualifiers */
    MI_STRING, /* type */
    NULL, /* className */
    0, /* subscript */
    offsetof(CIM_Service, StartMode), /* offset */
    MI_T("CIM_Service"), /* origin */
    MI_T("CIM_Service"), /* propagator */
    NULL,
};

static MI_CONST MI_Char* CIM_Service_Started_Description_qual_value = MI_T("382");

static MI_CONST MI_Qualifier CIM_Service_Started_Description_qual =
{
    MI_T("Description"),
    MI_STRING,
    MI_FLAG_ENABLEOVERRIDE|MI_FLAG_TOSUBCLASS|MI_FLAG_TRANSLATABLE,
    &CIM_Service_Started_Description_qual_value
};

static MI_Qualifier MI_CONST* MI_CONST CIM_Service_Started_quals[] =
{
    &CIM_Service_Started_Description_qual,
};

/* property CIM_Service.Started */
static MI_CONST MI_PropertyDecl CIM_Service_Started_prop =
{
    MI_FLAG_PROPERTY|MI_FLAG_READONLY, /* flags */
    0x00736407, /* code */
    MI_T("Started"), /* name */
    CIM_Service_Started_quals, /* qualifiers */
    MI_COUNT(CIM_Service_Started_quals), /* numQualifiers */
    MI_BOOLEAN, /* type */
    NULL, /* className */
    0, /* subscript */
    offsetof(CIM_Service, Started), /* offset */
    MI_T("CIM_Service"), /* origin */
    MI_T("CIM_Service"), /* propagator */
    NULL,
};

static MI_PropertyDecl MI_CONST* MI_CONST CIM_Service_props[] =
{
    &CIM_ManagedElement_InstanceID_prop,
    &CIM_ManagedElement_Caption_prop,
    &CIM_ManagedElement_Description_prop,
    &CIM_ManagedElement_ElementName_prop,
    &CIM_ManagedSystemElement_InstallDate_prop,
    &CIM_Service_Name_prop,
    &CIM_ManagedSystemElement_OperationalStatus_prop,
    &CIM_ManagedSystemElement_StatusDescriptions_prop,
    &CIM_ManagedSystemElement_Status_prop,
    &CIM_ManagedSystemElement_HealthState_prop,
    &CIM_ManagedSystemElement_CommunicationStatus_prop,
    &CIM_ManagedSystemElement_DetailedStatus_prop,
    &CIM_ManagedSystemElement_OperatingStatus_prop,
    &CIM_ManagedSystemElement_PrimaryStatus_prop,
    &CIM_EnabledLogicalElement_EnabledState_prop,
    &CIM_EnabledLogicalElement_OtherEnabledState_prop,
    &CIM_EnabledLogicalElement_RequestedState_prop,
    &CIM_EnabledLogicalElement_EnabledDefault_prop,
    &CIM_EnabledLogicalElement_TimeOfLastStateChange_prop,
    &CIM_EnabledLogicalElement_AvailableRequestedStates_prop,
    &CIM_EnabledLogicalElement_TransitioningToState_prop,
    &CIM_Service_SystemCreationClassName_prop,
    &CIM_Service_SystemName_prop,
    &CIM_Service_CreationClassName_prop,
    &CIM_Service_PrimaryOwnerName_prop,
    &CIM_Service_PrimaryOwnerContact_prop,
    &CIM_Service_StartMode_prop,
    &CIM_Service_Started_prop,
};

static MI_CONST MI_Char* CIM_Service_StartService_Description_qual_value = MI_T("383");

static MI_CONST MI_Qualifier CIM_Service_StartService_Description_qual =
{
    MI_T("Description"),
    MI_STRING,
    MI_FLAG_ENABLEOVERRIDE|MI_FLAG_TOSUBCLASS|MI_FLAG_TRANSLATABLE,
    &CIM_Service_StartService_Description_qual_value
};

static MI_Qualifier MI_CONST* MI_CONST CIM_Service_StartService_quals[] =
{
    &CIM_Service_StartService_Description_qual,
};

static MI_CONST MI_Char* CIM_Service_StartService_MIReturn_Description_qual_value = MI_T("383");

static MI_CONST MI_Qualifier CIM_Service_StartService_MIReturn_Description_qual =
{
    MI_T("Description"),
    MI_STRING,
    MI_FLAG_ENABLEOVERRIDE|MI_FLAG_TOSUBCLASS|MI_FLAG_TRANSLATABLE,
    &CIM_Service_StartService_MIReturn_Description_qual_value
};

static MI_Qualifier MI_CONST* MI_CONST CIM_Service_StartService_MIReturn_quals[] =
{
    &CIM_Service_StartService_MIReturn_Description_qual,
};

/* parameter CIM_Service.StartService(): MIReturn */
static MI_CONST MI_ParameterDecl CIM_Service_StartService_MIReturn_param =
{
    MI_FLAG_PARAMETER|MI_FLAG_OUT, /* flags */
    0x006D6E08, /* code */
    MI_T("MIReturn"), /* name */
    CIM_Service_StartService_MIReturn_quals, /* qualifiers */
    MI_COUNT(CIM_Service_StartService_MIReturn_quals), /* numQualifiers */
    MI_UINT32, /* type */
    NULL, /* className */
    0, /* subscript */
    offsetof(CIM_Service_StartService, MIReturn), /* offset */
};

static MI_ParameterDecl MI_CONST* MI_CONST CIM_Service_StartService_params[] =
{
    &CIM_Service_StartService_MIReturn_param,
};

/* method CIM_Service.StartService() */
MI_CONST MI_MethodDecl CIM_Service_StartService_rtti =
{
    MI_FLAG_METHOD, /* flags */
    0x0073650C, /* code */
    MI_T("StartService"), /* name */
    CIM_Service_StartService_quals, /* qualifiers */
    MI_COUNT(CIM_Service_StartService_quals), /* numQualifiers */
    CIM_Service_StartService_params, /* parameters */
    MI_COUNT(CIM_Service_StartService_params), /* numParameters */
    sizeof(CIM_Service_StartService), /* size */
    MI_UINT32, /* returnType */
    MI_T("CIM_Service"), /* origin */
    MI_T("CIM_Service"), /* propagator */
    &schemaDecl, /* schema */
    NULL, /* method */
};

static MI_CONST MI_Char* CIM_Service_StopService_Description_qual_value = MI_T("384");

static MI_CONST MI_Qualifier CIM_Service_StopService_Description_qual =
{
    MI_T("Description"),
    MI_STRING,
    MI_FLAG_ENABLEOVERRIDE|MI_FLAG_TOSUBCLASS|MI_FLAG_TRANSLATABLE,
    &CIM_Service_StopService_Description_qual_value
};

static MI_Qualifier MI_CONST* MI_CONST CIM_Service_StopService_quals[] =
{
    &CIM_Service_StopService_Description_qual,
};

static MI_CONST MI_Char* CIM_Service_StopService_MIReturn_Description_qual_value = MI_T("384");

static MI_CONST MI_Qualifier CIM_Service_StopService_MIReturn_Description_qual =
{
    MI_T("Description"),
    MI_STRING,
    MI_FLAG_ENABLEOVERRIDE|MI_FLAG_TOSUBCLASS|MI_FLAG_TRANSLATABLE,
    &CIM_Service_StopService_MIReturn_Description_qual_value
};

static MI_Qualifier MI_CONST* MI_CONST CIM_Service_StopService_MIReturn_quals[] =
{
    &CIM_Service_StopService_MIReturn_Description_qual,
};

/* parameter CIM_Service.StopService(): MIReturn */
static MI_CONST MI_ParameterDecl CIM_Service_StopService_MIReturn_param =
{
    MI_FLAG_PARAMETER|MI_FLAG_OUT, /* flags */
    0x006D6E08, /* code */
    MI_T("MIReturn"), /* name */
    CIM_Service_StopService_MIReturn_quals, /* qualifiers */
    MI_COUNT(CIM_Service_StopService_MIReturn_quals), /* numQualifiers */
    MI_UINT32, /* type */
    NULL, /* className */
    0, /* subscript */
    offsetof(CIM_Service_StopService, MIReturn), /* offset */
};

static MI_ParameterDecl MI_CONST* MI_CONST CIM_Service_StopService_params[] =
{
    &CIM_Service_StopService_MIReturn_param,
};

/* method CIM_Service.StopService() */
MI_CONST MI_MethodDecl CIM_Service_StopService_rtti =
{
    MI_FLAG_METHOD, /* flags */
    0x0073650B, /* code */
    MI_T("StopService"), /* name */
    CIM_Service_StopService_quals, /* qualifiers */
    MI_COUNT(CIM_Service_StopService_quals), /* numQualifiers */
    CIM_Service_StopService_params, /* parameters */
    MI_COUNT(CIM_Service_StopService_params), /* numParameters */
    sizeof(CIM_Service_StopService), /* size */
    MI_UINT32, /* returnType */
    MI_T("CIM_Service"), /* origin */
    MI_T("CIM_Service"), /* propagator */
    &schemaDecl, /* schema */
    NULL, /* method */
};

static MI_MethodDecl MI_CONST* MI_CONST CIM_Service_meths[] =
{
    &CIM_EnabledLogicalElement_RequestStateChange_rtti,
    &CIM_Service_StartService_rtti,
    &CIM_Service_StopService_rtti,
};

static MI_CONST MI_Char* CIM_Service_UMLPackagePath_qual_value = MI_T("CIM::Core::Service");

static MI_CONST MI_Qualifier CIM_Service_UMLPackagePath_qual =
{
    MI_T("UMLPackagePath"),
    MI_STRING,
    MI_FLAG_ENABLEOVERRIDE|MI_FLAG_TOSUBCLASS,
    &CIM_Service_UMLPackagePath_qual_value
};

static MI_CONST MI_Char* CIM_Service_Description_qual_value = MI_T("385");

static MI_CONST MI_Qualifier CIM_Service_Description_qual =
{
    MI_T("Description"),
    MI_STRING,
    MI_FLAG_ENABLEOVERRIDE|MI_FLAG_TOSUBCLASS|MI_FLAG_TRANSLATABLE,
    &CIM_Service_Description_qual_value
};

static MI_CONST MI_Boolean CIM_Service_Abstract_qual_value = 1;

static MI_CONST MI_Qualifier CIM_Service_Abstract_qual =
{
    MI_T("Abstract"),
    MI_BOOLEAN,
    MI_FLAG_ENABLEOVERRIDE|MI_FLAG_RESTRICTED,
    &CIM_Service_Abstract_qual_value
};

static MI_CONST MI_Char* CIM_Service_Version_qual_value = MI_T("386");

static MI_CONST MI_Qualifier CIM_Service_Version_qual =
{
    MI_T("Version"),
    MI_STRING,
    MI_FLAG_ENABLEOVERRIDE|MI_FLAG_TRANSLATABLE|MI_FLAG_RESTRICTED,
    &CIM_Service_Version_qual_value
};

static MI_Qualifier MI_CONST* MI_CONST CIM_Service_quals[] =
{
    &CIM_Service_UMLPackagePath_qual,
    &CIM_Service_Description_qual,
    &CIM_Service_Abstract_qual,
    &CIM_Service_Version_qual,
};

/* class CIM_Service */
MI_CONST MI_ClassDecl CIM_Service_rtti =
{
    MI_FLAG_CLASS|MI_FLAG_ABSTRACT, /* flags */
    0x0063650B, /* code */
    MI_T("CIM_Service"), /* name */
    CIM_Service_quals, /* qualifiers */
    MI_COUNT(CIM_Service_quals), /* numQualifiers */
    CIM_Service_props, /* properties */
    MI_COUNT(CIM_Service_props), /* numProperties */
    sizeof(CIM_Service), /* size */
    MI_T("CIM_EnabledLogicalElement"), /* superClass */
    &CIM_EnabledLogicalElement_rtti, /* superClassDecl */
    CIM_Service_meths, /* methods */
    MI_COUNT(CIM_Service_meths), /* numMethods */
    &schemaDecl, /* schema */
    NULL, /* functions */
    NULL /* owningClass */
};

/*
**==============================================================================
**
** MSFT_WindowsService
**
**==============================================================================
*/

static MI_PropertyDecl MI_CONST* MI_CONST MSFT_WindowsService_props[] =
{
    &CIM_ManagedElement_InstanceID_prop,
    &CIM_ManagedElement_Caption_prop,
    &CIM_ManagedElement_Description_prop,
    &CIM_ManagedElement_ElementName_prop,
    &CIM_ManagedSystemElement_InstallDate_prop,
    &CIM_Service_Name_prop,
    &CIM_ManagedSystemElement_OperationalStatus_prop,
    &CIM_ManagedSystemElement_StatusDescriptions_prop,
    &CIM_ManagedSystemElement_Status_prop,
    &CIM_ManagedSystemElement_HealthState_prop,
    &CIM_ManagedSystemElement_CommunicationStatus_prop,
    &CIM_ManagedSystemElement_DetailedStatus_prop,
    &CIM_ManagedSystemElement_OperatingStatus_prop,
    &CIM_ManagedSystemElement_PrimaryStatus_prop,
    &CIM_EnabledLogicalElement_EnabledState_prop,
    &CIM_EnabledLogicalElement_OtherEnabledState_prop,
    &CIM_EnabledLogicalElement_RequestedState_prop,
    &CIM_EnabledLogicalElement_EnabledDefault_prop,
    &CIM_EnabledLogicalElement_TimeOfLastStateChange_prop,
    &CIM_EnabledLogicalElement_AvailableRequestedStates_prop,
    &CIM_EnabledLogicalElement_TransitioningToState_prop,
    &CIM_Service_SystemCreationClassName_prop,
    &CIM_Service_SystemName_prop,
    &CIM_Service_CreationClassName_prop,
    &CIM_Service_PrimaryOwnerName_prop,
    &CIM_Service_PrimaryOwnerContact_prop,
    &CIM_Service_StartMode_prop,
    &CIM_Service_Started_prop,
};

static MI_CONST MI_Char* MSFT_WindowsService_RequestStateChange_Description_qual_value = MI_T("367");

static MI_CONST MI_Qualifier MSFT_WindowsService_RequestStateChange_Description_qual =
{
    MI_T("Description"),
    MI_STRING,
    MI_FLAG_ENABLEOVERRIDE|MI_FLAG_TOSUBCLASS|MI_FLAG_TRANSLATABLE,
    &MSFT_WindowsService_RequestStateChange_Description_qual_value
};

static MI_CONST MI_Char* MSFT_WindowsService_RequestStateChange_ValueMap_qual_data_value[] =
{
    MI_T("0"),
    MI_T("1"),
    MI_T("2"),
    MI_T("3"),
    MI_T("4"),
    MI_T("5"),
    MI_T("6"),
    MI_T(".."),
    MI_T("4096"),
    MI_T("4097"),
    MI_T("4098"),
    MI_T("4099"),
    MI_T("4100..32767"),
    MI_T("32768..65535"),
};

static MI_CONST MI_ConstStringA MSFT_WindowsService_RequestStateChange_ValueMap_qual_value =
{
    MSFT_WindowsService_RequestStateChange_ValueMap_qual_data_value,
    MI_COUNT(MSFT_WindowsService_RequestStateChange_ValueMap_qual_data_value),
};

static MI_CONST MI_Qualifier MSFT_WindowsService_RequestStateChange_ValueMap_qual =
{
    MI_T("ValueMap"),
    MI_STRINGA,
    MI_FLAG_ENABLEOVERRIDE|MI_FLAG_TOSUBCLASS,
    &MSFT_WindowsService_RequestStateChange_ValueMap_qual_value
};

static MI_CONST MI_Char* MSFT_WindowsService_RequestStateChange_Values_qual_data_value[] =
{
    MI_T("326"),
    MI_T("115"),
    MI_T("368"),
    MI_T("369"),
    MI_T("117"),
    MI_T("329"),
    MI_T("330"),
    MI_T("29"),
    MI_T("370"),
    MI_T("332"),
    MI_T("333"),
    MI_T("334"),
    MI_T("335"),
    MI_T("120"),
};

static MI_CONST MI_ConstStringA MSFT_WindowsService_RequestStateChange_Values_qual_value =
{
    MSFT_WindowsService_RequestStateChange_Values_qual_data_value,
    MI_COUNT(MSFT_WindowsService_RequestStateChange_Values_qual_data_value),
};

static MI_CONST MI_Qualifier MSFT_WindowsService_RequestStateChange_Values_qual =
{
    MI_T("Values"),
    MI_STRINGA,
    MI_FLAG_ENABLEOVERRIDE|MI_FLAG_TOSUBCLASS|MI_FLAG_TRANSLATABLE,
    &MSFT_WindowsService_RequestStateChange_Values_qual_value
};

static MI_CONST MI_Char* MSFT_WindowsService_RequestStateChange_ModelCorrespondence_qual_data_value[] =
{
    MI_T("CIM_EnabledLogicalElement.RequestedState"),
};

static MI_CONST MI_ConstStringA MSFT_WindowsService_RequestStateChange_ModelCorrespondence_qual_value =
{
    MSFT_WindowsService_RequestStateChange_ModelCorrespondence_qual_data_value,
    MI_COUNT(MSFT_WindowsService_RequestStateChange_ModelCorrespondence_qual_data_value),
};

static MI_CONST MI_Qualifier MSFT_WindowsService_RequestStateChange_ModelCorrespondence_qual =
{
    MI_T("ModelCorrespondence"),
    MI_STRINGA,
    MI_FLAG_ENABLEOVERRIDE|MI_FLAG_TOSUBCLASS,
    &MSFT_WindowsService_RequestStateChange_ModelCorrespondence_qual_value
};

static MI_Qualifier MI_CONST* MI_CONST MSFT_WindowsService_RequestStateChange_quals[] =
{
    &MSFT_WindowsService_RequestStateChange_Description_qual,
    &MSFT_WindowsService_RequestStateChange_ValueMap_qual,
    &MSFT_WindowsService_RequestStateChange_Values_qual,
    &MSFT_WindowsService_RequestStateChange_ModelCorrespondence_qual,
};

static MI_CONST MI_Boolean MSFT_WindowsService_RequestStateChange_RequestedState_In_qual_value = 1;

static MI_CONST MI_Qualifier MSFT_WindowsService_RequestStateChange_RequestedState_In_qual =
{
    MI_T("In"),
    MI_BOOLEAN,
    MI_FLAG_DISABLEOVERRIDE|MI_FLAG_TOSUBCLASS,
    &MSFT_WindowsService_RequestStateChange_RequestedState_In_qual_value
};

static MI_CONST MI_Char* MSFT_WindowsService_RequestStateChange_RequestedState_Description_qual_value = MI_T("371");

static MI_CONST MI_Qualifier MSFT_WindowsService_RequestStateChange_RequestedState_Description_qual =
{
    MI_T("Description"),
    MI_STRING,
    MI_FLAG_ENABLEOVERRIDE|MI_FLAG_TOSUBCLASS|MI_FLAG_TRANSLATABLE,
    &MSFT_WindowsService_RequestStateChange_RequestedState_Description_qual_value
};

static MI_CONST MI_Char* MSFT_WindowsService_RequestStateChange_RequestedState_ValueMap_qual_data_value[] =
{
    MI_T("2"),
    MI_T("3"),
    MI_T("4"),
    MI_T("6"),
    MI_T("7"),
    MI_T("8"),
    MI_T("9"),
    MI_T("10"),
    MI_T("11"),
    MI_T(".."),
    MI_T("32768..65535"),
};

static MI_CONST MI_ConstStringA MSFT_WindowsService_RequestStateChange_RequestedState_ValueMap_qual_value =
{
    MSFT_WindowsService_RequestStateChange_RequestedState_ValueMap_qual_data_value,
    MI_COUNT(MSFT_WindowsService_RequestStateChange_RequestedState_ValueMap_qual_data_value),
};

static MI_CONST MI_Qualifier MSFT_WindowsService_RequestStateChange_RequestedState_ValueMap_qual =
{
    MI_T("ValueMap"),
    MI_STRINGA,
    MI_FLAG_ENABLEOVERRIDE|MI_FLAG_TOSUBCLASS,
    &MSFT_WindowsService_RequestStateChange_RequestedState_ValueMap_qual_value
};

static MI_CONST MI_Char* MSFT_WindowsService_RequestStateChange_RequestedState_Values_qual_data_value[] =
{
    MI_T("347"),
    MI_T("348"),
    MI_T("355"),
    MI_T("357"),
    MI_T("358"),
    MI_T("365"),
    MI_T("352"),
    MI_T("359"),
    MI_T("360"),
    MI_T("29"),
    MI_T("30"),
};

static MI_CONST MI_ConstStringA MSFT_WindowsService_RequestStateChange_RequestedState_Values_qual_value =
{
    MSFT_WindowsService_RequestStateChange_RequestedState_Values_qual_data_value,
    MI_COUNT(MSFT_WindowsService_RequestStateChange_RequestedState_Values_qual_data_value),
};

static MI_CONST MI_Qualifier MSFT_WindowsService_RequestStateChange_RequestedState_Values_qual =
{
    MI_T("Values"),
    MI_STRINGA,
    MI_FLAG_ENABLEOVERRIDE|MI_FLAG_TOSUBCLASS|MI_FLAG_TRANSLATABLE,
    &MSFT_WindowsService_RequestStateChange_RequestedState_Values_qual_value
};

static MI_CONST MI_Char* MSFT_WindowsService_RequestStateChange_RequestedState_ModelCorrespondence_qual_data_value[] =
{
    MI_T("CIM_EnabledLogicalElement.RequestedState"),
};

static MI_CONST MI_ConstStringA MSFT_WindowsService_RequestStateChange_RequestedState_ModelCorrespondence_qual_value =
{
    MSFT_WindowsService_RequestStateChange_RequestedState_ModelCorrespondence_qual_data_value,
    MI_COUNT(MSFT_WindowsService_RequestStateChange_RequestedState_ModelCorrespondence_qual_data_value),
};

static MI_CONST MI_Qualifier MSFT_WindowsService_RequestStateChange_RequestedState_ModelCorrespondence_qual =
{
    MI_T("ModelCorrespondence"),
    MI_STRINGA,
    MI_FLAG_ENABLEOVERRIDE|MI_FLAG_TOSUBCLASS,
    &MSFT_WindowsService_RequestStateChange_RequestedState_ModelCorrespondence_qual_value
};

static MI_Qualifier MI_CONST* MI_CONST MSFT_WindowsService_RequestStateChange_RequestedState_quals[] =
{
    &MSFT_WindowsService_RequestStateChange_RequestedState_In_qual,
    &MSFT_WindowsService_RequestStateChange_RequestedState_Description_qual,
    &MSFT_WindowsService_RequestStateChange_RequestedState_ValueMap_qual,
    &MSFT_WindowsService_RequestStateChange_RequestedState_Values_qual,
    &MSFT_WindowsService_RequestStateChange_RequestedState_ModelCorrespondence_qual,
};

/* parameter MSFT_WindowsService.RequestStateChange(): RequestedState */
static MI_CONST MI_ParameterDecl MSFT_WindowsService_RequestStateChange_RequestedState_param =
{
    MI_FLAG_PARAMETER|MI_FLAG_IN, /* flags */
    0x0072650E, /* code */
    MI_T("RequestedState"), /* name */
    MSFT_WindowsService_RequestStateChange_RequestedState_quals, /* qualifiers */
    MI_COUNT(MSFT_WindowsService_RequestStateChange_RequestedState_quals), /* numQualifiers */
    MI_UINT16, /* type */
    NULL, /* className */
    0, /* subscript */
    offsetof(MSFT_WindowsService_RequestStateChange, RequestedState), /* offset */
};

static MI_CONST MI_Boolean MSFT_WindowsService_RequestStateChange_Job_In_qual_value = 0;

static MI_CONST MI_Qualifier MSFT_WindowsService_RequestStateChange_Job_In_qual =
{
    MI_T("In"),
    MI_BOOLEAN,
    MI_FLAG_DISABLEOVERRIDE|MI_FLAG_TOSUBCLASS,
    &MSFT_WindowsService_RequestStateChange_Job_In_qual_value
};

static MI_CONST MI_Boolean MSFT_WindowsService_RequestStateChange_Job_Out_qual_value = 1;

static MI_CONST MI_Qualifier MSFT_WindowsService_RequestStateChange_Job_Out_qual =
{
    MI_T("Out"),
    MI_BOOLEAN,
    MI_FLAG_DISABLEOVERRIDE|MI_FLAG_TOSUBCLASS,
    &MSFT_WindowsService_RequestStateChange_Job_Out_qual_value
};

static MI_CONST MI_Char* MSFT_WindowsService_RequestStateChange_Job_Description_qual_value = MI_T("372");

static MI_CONST MI_Qualifier MSFT_WindowsService_RequestStateChange_Job_Description_qual =
{
    MI_T("Description"),
    MI_STRING,
    MI_FLAG_ENABLEOVERRIDE|MI_FLAG_TOSUBCLASS|MI_FLAG_TRANSLATABLE,
    &MSFT_WindowsService_RequestStateChange_Job_Description_qual_value
};

static MI_Qualifier MI_CONST* MI_CONST MSFT_WindowsService_RequestStateChange_Job_quals[] =
{
    &MSFT_WindowsService_RequestStateChange_Job_In_qual,
    &MSFT_WindowsService_RequestStateChange_Job_Out_qual,
    &MSFT_WindowsService_RequestStateChange_Job_Description_qual,
};

/* parameter MSFT_WindowsService.RequestStateChange(): Job */
static MI_CONST MI_ParameterDecl MSFT_WindowsService_RequestStateChange_Job_param =
{
    MI_FLAG_PARAMETER|MI_FLAG_OUT, /* flags */
    0x006A6203, /* code */
    MI_T("Job"), /* name */
    MSFT_WindowsService_RequestStateChange_Job_quals, /* qualifiers */
    MI_COUNT(MSFT_WindowsService_RequestStateChange_Job_quals), /* numQualifiers */
    MI_REFERENCE, /* type */
    MI_T("CIM_ConcreteJob"), /* className */
    0, /* subscript */
    offsetof(MSFT_WindowsService_RequestStateChange, Job), /* offset */
};

static MI_CONST MI_Boolean MSFT_WindowsService_RequestStateChange_TimeoutPeriod_In_qual_value = 1;

static MI_CONST MI_Qualifier MSFT_WindowsService_RequestStateChange_TimeoutPeriod_In_qual =
{
    MI_T("In"),
    MI_BOOLEAN,
    MI_FLAG_DISABLEOVERRIDE|MI_FLAG_TOSUBCLASS,
    &MSFT_WindowsService_RequestStateChange_TimeoutPeriod_In_qual_value
};

static MI_CONST MI_Char* MSFT_WindowsService_RequestStateChange_TimeoutPeriod_Description_qual_value = MI_T("373");

static MI_CONST MI_Qualifier MSFT_WindowsService_RequestStateChange_TimeoutPeriod_Description_qual =
{
    MI_T("Description"),
    MI_STRING,
    MI_FLAG_ENABLEOVERRIDE|MI_FLAG_TOSUBCLASS|MI_FLAG_TRANSLATABLE,
    &MSFT_WindowsService_RequestStateChange_TimeoutPeriod_Description_qual_value
};

static MI_Qualifier MI_CONST* MI_CONST MSFT_WindowsService_RequestStateChange_TimeoutPeriod_quals[] =
{
    &MSFT_WindowsService_RequestStateChange_TimeoutPeriod_In_qual,
    &MSFT_WindowsService_RequestStateChange_TimeoutPeriod_Description_qual,
};

/* parameter MSFT_WindowsService.RequestStateChange(): TimeoutPeriod */
static MI_CONST MI_ParameterDecl MSFT_WindowsService_RequestStateChange_TimeoutPeriod_param =
{
    MI_FLAG_PARAMETER|MI_FLAG_IN, /* flags */
    0x0074640D, /* code */
    MI_T("TimeoutPeriod"), /* name */
    MSFT_WindowsService_RequestStateChange_TimeoutPeriod_quals, /* qualifiers */
    MI_COUNT(MSFT_WindowsService_RequestStateChange_TimeoutPeriod_quals), /* numQualifiers */
    MI_DATETIME, /* type */
    NULL, /* className */
    0, /* subscript */
    offsetof(MSFT_WindowsService_RequestStateChange, TimeoutPeriod), /* offset */
};

static MI_CONST MI_Char* MSFT_WindowsService_RequestStateChange_MIReturn_Description_qual_value = MI_T("367");

static MI_CONST MI_Qualifier MSFT_WindowsService_RequestStateChange_MIReturn_Description_qual =
{
    MI_T("Description"),
    MI_STRING,
    MI_FLAG_ENABLEOVERRIDE|MI_FLAG_TOSUBCLASS|MI_FLAG_TRANSLATABLE,
    &MSFT_WindowsService_RequestStateChange_MIReturn_Description_qual_value
};

static MI_CONST MI_Char* MSFT_WindowsService_RequestStateChange_MIReturn_ValueMap_qual_data_value[] =
{
    MI_T("0"),
    MI_T("1"),
    MI_T("2"),
    MI_T("3"),
    MI_T("4"),
    MI_T("5"),
    MI_T("6"),
    MI_T(".."),
    MI_T("4096"),
    MI_T("4097"),
    MI_T("4098"),
    MI_T("4099"),
    MI_T("4100..32767"),
    MI_T("32768..65535"),
};

static MI_CONST MI_ConstStringA MSFT_WindowsService_RequestStateChange_MIReturn_ValueMap_qual_value =
{
    MSFT_WindowsService_RequestStateChange_MIReturn_ValueMap_qual_data_value,
    MI_COUNT(MSFT_WindowsService_RequestStateChange_MIReturn_ValueMap_qual_data_value),
};

static MI_CONST MI_Qualifier MSFT_WindowsService_RequestStateChange_MIReturn_ValueMap_qual =
{
    MI_T("ValueMap"),
    MI_STRINGA,
    MI_FLAG_ENABLEOVERRIDE|MI_FLAG_TOSUBCLASS,
    &MSFT_WindowsService_RequestStateChange_MIReturn_ValueMap_qual_value
};

static MI_CONST MI_Char* MSFT_WindowsService_RequestStateChange_MIReturn_Values_qual_data_value[] =
{
    MI_T("326"),
    MI_T("115"),
    MI_T("368"),
    MI_T("369"),
    MI_T("117"),
    MI_T("329"),
    MI_T("330"),
    MI_T("29"),
    MI_T("370"),
    MI_T("332"),
    MI_T("333"),
    MI_T("334"),
    MI_T("335"),
    MI_T("120"),
};

static MI_CONST MI_ConstStringA MSFT_WindowsService_RequestStateChange_MIReturn_Values_qual_value =
{
    MSFT_WindowsService_RequestStateChange_MIReturn_Values_qual_data_value,
    MI_COUNT(MSFT_WindowsService_RequestStateChange_MIReturn_Values_qual_data_value),
};

static MI_CONST MI_Qualifier MSFT_WindowsService_RequestStateChange_MIReturn_Values_qual =
{
    MI_T("Values"),
    MI_STRINGA,
    MI_FLAG_ENABLEOVERRIDE|MI_FLAG_TOSUBCLASS|MI_FLAG_TRANSLATABLE,
    &MSFT_WindowsService_RequestStateChange_MIReturn_Values_qual_value
};

static MI_CONST MI_Char* MSFT_WindowsService_RequestStateChange_MIReturn_ModelCorrespondence_qual_data_value[] =
{
    MI_T("CIM_EnabledLogicalElement.RequestedState"),
};

static MI_CONST MI_ConstStringA MSFT_WindowsService_RequestStateChange_MIReturn_ModelCorrespondence_qual_value =
{
    MSFT_WindowsService_RequestStateChange_MIReturn_ModelCorrespondence_qual_data_value,
    MI_COUNT(MSFT_WindowsService_RequestStateChange_MIReturn_ModelCorrespondence_qual_data_value),
};

static MI_CONST MI_Qualifier MSFT_WindowsService_RequestStateChange_MIReturn_ModelCorrespondence_qual =
{
    MI_T("ModelCorrespondence"),
    MI_STRINGA,
    MI_FLAG_ENABLEOVERRIDE|MI_FLAG_TOSUBCLASS,
    &MSFT_WindowsService_RequestStateChange_MIReturn_ModelCorrespondence_qual_value
};

static MI_Qualifier MI_CONST* MI_CONST MSFT_WindowsService_RequestStateChange_MIReturn_quals[] =
{
    &MSFT_WindowsService_RequestStateChange_MIReturn_Description_qual,
    &MSFT_WindowsService_RequestStateChange_MIReturn_ValueMap_qual,
    &MSFT_WindowsService_RequestStateChange_MIReturn_Values_qual,
    &MSFT_WindowsService_RequestStateChange_MIReturn_ModelCorrespondence_qual,
};

/* parameter MSFT_WindowsService.RequestStateChange(): MIReturn */
static MI_CONST MI_ParameterDecl MSFT_WindowsService_RequestStateChange_MIReturn_param =
{
    MI_FLAG_PARAMETER|MI_FLAG_OUT, /* flags */
    0x006D6E08, /* code */
    MI_T("MIReturn"), /* name */
    MSFT_WindowsService_RequestStateChange_MIReturn_quals, /* qualifiers */
    MI_COUNT(MSFT_WindowsService_RequestStateChange_MIReturn_quals), /* numQualifiers */
    MI_UINT32, /* type */
    NULL, /* className */
    0, /* subscript */
    offsetof(MSFT_WindowsService_RequestStateChange, MIReturn), /* offset */
};

static MI_ParameterDecl MI_CONST* MI_CONST MSFT_WindowsService_RequestStateChange_params[] =
{
    &MSFT_WindowsService_RequestStateChange_MIReturn_param,
    &MSFT_WindowsService_RequestStateChange_RequestedState_param,
    &MSFT_WindowsService_RequestStateChange_Job_param,
    &MSFT_WindowsService_RequestStateChange_TimeoutPeriod_param,
};

/* method MSFT_WindowsService.RequestStateChange() */
MI_CONST MI_MethodDecl MSFT_WindowsService_RequestStateChange_rtti =
{
    MI_FLAG_METHOD, /* flags */
    0x00726512, /* code */
    MI_T("RequestStateChange"), /* name */
    MSFT_WindowsService_RequestStateChange_quals, /* qualifiers */
    MI_COUNT(MSFT_WindowsService_RequestStateChange_quals), /* numQualifiers */
    MSFT_WindowsService_RequestStateChange_params, /* parameters */
    MI_COUNT(MSFT_WindowsService_RequestStateChange_params), /* numParameters */
    sizeof(MSFT_WindowsService_RequestStateChange), /* size */
    MI_UINT32, /* returnType */
    MI_T("CIM_EnabledLogicalElement"), /* origin */
    MI_T("CIM_EnabledLogicalElement"), /* propagator */
    &schemaDecl, /* schema */
    (MI_ProviderFT_Invoke)MSFT_WindowsService_Invoke_RequestStateChange, /* method */
};

static MI_CONST MI_Char* MSFT_WindowsService_StartService_Description_qual_value = MI_T("383");

static MI_CONST MI_Qualifier MSFT_WindowsService_StartService_Description_qual =
{
    MI_T("Description"),
    MI_STRING,
    MI_FLAG_ENABLEOVERRIDE|MI_FLAG_TOSUBCLASS|MI_FLAG_TRANSLATABLE,
    &MSFT_WindowsService_StartService_Description_qual_value
};

static MI_Qualifier MI_CONST* MI_CONST MSFT_WindowsService_StartService_quals[] =
{
    &MSFT_WindowsService_StartService_Description_qual,
};

static MI_CONST MI_Char* MSFT_WindowsService_StartService_MIReturn_Description_qual_value = MI_T("383");

static MI_CONST MI_Qualifier MSFT_WindowsService_StartService_MIReturn_Description_qual =
{
    MI_T("Description"),
    MI_STRING,
    MI_FLAG_ENABLEOVERRIDE|MI_FLAG_TOSUBCLASS|MI_FLAG_TRANSLATABLE,
    &MSFT_WindowsService_StartService_MIReturn_Description_qual_value
};

static MI_Qualifier MI_CONST* MI_CONST MSFT_WindowsService_StartService_MIReturn_quals[] =
{
    &MSFT_WindowsService_StartService_MIReturn_Description_qual,
};

/* parameter MSFT_WindowsService.StartService(): MIReturn */
static MI_CONST MI_ParameterDecl MSFT_WindowsService_StartService_MIReturn_param =
{
    MI_FLAG_PARAMETER|MI_FLAG_OUT, /* flags */
    0x006D6E08, /* code */
    MI_T("MIReturn"), /* name */
    MSFT_WindowsService_StartService_MIReturn_quals, /* qualifiers */
    MI_COUNT(MSFT_WindowsService_StartService_MIReturn_quals), /* numQualifiers */
    MI_UINT32, /* type */
    NULL, /* className */
    0, /* subscript */
    offsetof(MSFT_WindowsService_StartService, MIReturn), /* offset */
};

static MI_ParameterDecl MI_CONST* MI_CONST MSFT_WindowsService_StartService_params[] =
{
    &MSFT_WindowsService_StartService_MIReturn_param,
};

/* method MSFT_WindowsService.StartService() */
MI_CONST MI_MethodDecl MSFT_WindowsService_StartService_rtti =
{
    MI_FLAG_METHOD, /* flags */
    0x0073650C, /* code */
    MI_T("StartService"), /* name */
    MSFT_WindowsService_StartService_quals, /* qualifiers */
    MI_COUNT(MSFT_WindowsService_StartService_quals), /* numQualifiers */
    MSFT_WindowsService_StartService_params, /* parameters */
    MI_COUNT(MSFT_WindowsService_StartService_params), /* numParameters */
    sizeof(MSFT_WindowsService_StartService), /* size */
    MI_UINT32, /* returnType */
    MI_T("CIM_Service"), /* origin */
    MI_T("MSFT_WindowsService"), /* propagator */
    &schemaDecl, /* schema */
    (MI_ProviderFT_Invoke)MSFT_WindowsService_Invoke_StartService, /* method */
};

static MI_CONST MI_Char* MSFT_WindowsService_StopService_Description_qual_value = MI_T("384");

static MI_CONST MI_Qualifier MSFT_WindowsService_StopService_Description_qual =
{
    MI_T("Description"),
    MI_STRING,
    MI_FLAG_ENABLEOVERRIDE|MI_FLAG_TOSUBCLASS|MI_FLAG_TRANSLATABLE,
    &MSFT_WindowsService_StopService_Description_qual_value
};

static MI_Qualifier MI_CONST* MI_CONST MSFT_WindowsService_StopService_quals[] =
{
    &MSFT_WindowsService_StopService_Description_qual,
};

static MI_CONST MI_Char* MSFT_WindowsService_StopService_MIReturn_Description_qual_value = MI_T("384");

static MI_CONST MI_Qualifier MSFT_WindowsService_StopService_MIReturn_Description_qual =
{
    MI_T("Description"),
    MI_STRING,
    MI_FLAG_ENABLEOVERRIDE|MI_FLAG_TOSUBCLASS|MI_FLAG_TRANSLATABLE,
    &MSFT_WindowsService_StopService_MIReturn_Description_qual_value
};

static MI_Qualifier MI_CONST* MI_CONST MSFT_WindowsService_StopService_MIReturn_quals[] =
{
    &MSFT_WindowsService_StopService_MIReturn_Description_qual,
};

/* parameter MSFT_WindowsService.StopService(): MIReturn */
static MI_CONST MI_ParameterDecl MSFT_WindowsService_StopService_MIReturn_param =
{
    MI_FLAG_PARAMETER|MI_FLAG_OUT, /* flags */
    0x006D6E08, /* code */
    MI_T("MIReturn"), /* name */
    MSFT_WindowsService_StopService_MIReturn_quals, /* qualifiers */
    MI_COUNT(MSFT_WindowsService_StopService_MIReturn_quals), /* numQualifiers */
    MI_UINT32, /* type */
    NULL, /* className */
    0, /* subscript */
    offsetof(MSFT_WindowsService_StopService, MIReturn), /* offset */
};

static MI_ParameterDecl MI_CONST* MI_CONST MSFT_WindowsService_StopService_params[] =
{
    &MSFT_WindowsService_StopService_MIReturn_param,
};

/* method MSFT_WindowsService.StopService() */
MI_CONST MI_MethodDecl MSFT_WindowsService_StopService_rtti =
{
    MI_FLAG_METHOD, /* flags */
    0x0073650B, /* code */
    MI_T("StopService"), /* name */
    MSFT_WindowsService_StopService_quals, /* qualifiers */
    MI_COUNT(MSFT_WindowsService_StopService_quals), /* numQualifiers */
    MSFT_WindowsService_StopService_params, /* parameters */
    MI_COUNT(MSFT_WindowsService_StopService_params), /* numParameters */
    sizeof(MSFT_WindowsService_StopService), /* size */
    MI_UINT32, /* returnType */
    MI_T("CIM_Service"), /* origin */
    MI_T("MSFT_WindowsService"), /* propagator */
    &schemaDecl, /* schema */
    (MI_ProviderFT_Invoke)MSFT_WindowsService_Invoke_StopService, /* method */
};

static MI_MethodDecl MI_CONST* MI_CONST MSFT_WindowsService_meths[] =
{
    &MSFT_WindowsService_RequestStateChange_rtti,
    &MSFT_WindowsService_StartService_rtti,
    &MSFT_WindowsService_StopService_rtti,
};

static MI_CONST MI_ProviderFT MSFT_WindowsService_funcs =
{
  (MI_ProviderFT_Load)MSFT_WindowsService_Load,
  (MI_ProviderFT_Unload)MSFT_WindowsService_Unload,
  (MI_ProviderFT_GetInstance)MSFT_WindowsService_GetInstance,
  (MI_ProviderFT_EnumerateInstances)MSFT_WindowsService_EnumerateInstances,
  (MI_ProviderFT_CreateInstance)MSFT_WindowsService_CreateInstance,
  (MI_ProviderFT_ModifyInstance)MSFT_WindowsService_ModifyInstance,
  (MI_ProviderFT_DeleteInstance)MSFT_WindowsService_DeleteInstance,
  (MI_ProviderFT_AssociatorInstances)NULL,
  (MI_ProviderFT_ReferenceInstances)NULL,
  (MI_ProviderFT_EnableIndications)NULL,
  (MI_ProviderFT_DisableIndications)NULL,
  (MI_ProviderFT_Subscribe)NULL,
  (MI_ProviderFT_Unsubscribe)NULL,
  (MI_ProviderFT_Invoke)NULL,
};

static MI_CONST MI_Char* MSFT_WindowsService_UMLPackagePath_qual_value = MI_T("CIM::Core::Service");

static MI_CONST MI_Qualifier MSFT_WindowsService_UMLPackagePath_qual =
{
    MI_T("UMLPackagePath"),
    MI_STRING,
    MI_FLAG_ENABLEOVERRIDE|MI_FLAG_TOSUBCLASS,
    &MSFT_WindowsService_UMLPackagePath_qual_value
};

static MI_CONST MI_Char* MSFT_WindowsService_Description_qual_value = MI_T("385");

static MI_CONST MI_Qualifier MSFT_WindowsService_Description_qual =
{
    MI_T("Description"),
    MI_STRING,
    MI_FLAG_ENABLEOVERRIDE|MI_FLAG_TOSUBCLASS|MI_FLAG_TRANSLATABLE,
    &MSFT_WindowsService_Description_qual_value
};

static MI_CONST MI_Char* MSFT_WindowsService_ClassVersion_qual_value = MI_T("1.0.0");

static MI_CONST MI_Qualifier MSFT_WindowsService_ClassVersion_qual =
{
    MI_T("ClassVersion"),
    MI_STRING,
    MI_FLAG_ENABLEOVERRIDE|MI_FLAG_RESTRICTED,
    &MSFT_WindowsService_ClassVersion_qual_value
};

static MI_Qualifier MI_CONST* MI_CONST MSFT_WindowsService_quals[] =
{
    &MSFT_WindowsService_UMLPackagePath_qual,
    &MSFT_WindowsService_Description_qual,
    &MSFT_WindowsService_ClassVersion_qual,
};

/* class MSFT_WindowsService */
MI_CONST MI_ClassDecl MSFT_WindowsService_rtti =
{
    MI_FLAG_CLASS, /* flags */
    0x006D6513, /* code */
    MI_T("MSFT_WindowsService"), /* name */
    MSFT_WindowsService_quals, /* qualifiers */
    MI_COUNT(MSFT_WindowsService_quals), /* numQualifiers */
    MSFT_WindowsService_props, /* properties */
    MI_COUNT(MSFT_WindowsService_props), /* numProperties */
    sizeof(MSFT_WindowsService), /* size */
    MI_T("CIM_Service"), /* superClass */
    &CIM_Service_rtti, /* superClassDecl */
    MSFT_WindowsService_meths, /* methods */
    MI_COUNT(MSFT_WindowsService_meths), /* numMethods */
    &schemaDecl, /* schema */
    &MSFT_WindowsService_funcs, /* functions */
    NULL /* owningClass */
};

/*
**==============================================================================
**
** MSFT_WindowsServiceManager
**
**==============================================================================
*/


static MI_CONST MI_Boolean MSFT_WindowsServiceManager_GetWindowsServices_Static_qual_value = 1;

static MI_CONST MI_Qualifier MSFT_WindowsServiceManager_GetWindowsServices_Static_qual =
{
    MI_T("Static"),
    MI_BOOLEAN,
    MI_FLAG_DISABLEOVERRIDE|MI_FLAG_TOSUBCLASS,
    &MSFT_WindowsServiceManager_GetWindowsServices_Static_qual_value
};

static MI_Qualifier MI_CONST* MI_CONST MSFT_WindowsServiceManager_GetWindowsServices_quals[] =
{
    &MSFT_WindowsServiceManager_GetWindowsServices_Static_qual,
};

static MI_CONST MI_Boolean MSFT_WindowsServiceManager_GetWindowsServices_status_In_qual_value = 1;

static MI_CONST MI_Qualifier MSFT_WindowsServiceManager_GetWindowsServices_status_In_qual =
{
    MI_T("In"),
    MI_BOOLEAN,
    MI_FLAG_DISABLEOVERRIDE|MI_FLAG_TOSUBCLASS,
    &MSFT_WindowsServiceManager_GetWindowsServices_status_In_qual_value
};

static MI_CONST MI_Char* MSFT_WindowsServiceManager_GetWindowsServices_status_ValueMap_qual_data_value[] =
{
    MI_T("0"),
    MI_T("1"),
    MI_T(".."),
};

static MI_CONST MI_ConstStringA MSFT_WindowsServiceManager_GetWindowsServices_status_ValueMap_qual_value =
{
    MSFT_WindowsServiceManager_GetWindowsServices_status_ValueMap_qual_data_value,
    MI_COUNT(MSFT_WindowsServiceManager_GetWindowsServices_status_ValueMap_qual_data_value),
};

static MI_CONST MI_Qualifier MSFT_WindowsServiceManager_GetWindowsServices_status_ValueMap_qual =
{
    MI_T("ValueMap"),
    MI_STRINGA,
    MI_FLAG_ENABLEOVERRIDE|MI_FLAG_TOSUBCLASS,
    &MSFT_WindowsServiceManager_GetWindowsServices_status_ValueMap_qual_value
};

static MI_CONST MI_Char* MSFT_WindowsServiceManager_GetWindowsServices_status_Values_qual_data_value[] =
{
    MI_T("316"),
    MI_T("20"),
    MI_T("387"),
};

static MI_CONST MI_ConstStringA MSFT_WindowsServiceManager_GetWindowsServices_status_Values_qual_value =
{
    MSFT_WindowsServiceManager_GetWindowsServices_status_Values_qual_data_value,
    MI_COUNT(MSFT_WindowsServiceManager_GetWindowsServices_status_Values_qual_data_value),
};

static MI_CONST MI_Qualifier MSFT_WindowsServiceManager_GetWindowsServices_status_Values_qual =
{
    MI_T("Values"),
    MI_STRINGA,
    MI_FLAG_ENABLEOVERRIDE|MI_FLAG_TOSUBCLASS|MI_FLAG_TRANSLATABLE,
    &MSFT_WindowsServiceManager_GetWindowsServices_status_Values_qual_value
};

static MI_Qualifier MI_CONST* MI_CONST MSFT_WindowsServiceManager_GetWindowsServices_status_quals[] =
{
    &MSFT_WindowsServiceManager_GetWindowsServices_status_In_qual,
    &MSFT_WindowsServiceManager_GetWindowsServices_status_ValueMap_qual,
    &MSFT_WindowsServiceManager_GetWindowsServices_status_Values_qual,
};

/* parameter MSFT_WindowsServiceManager.GetWindowsServices(): status */
static MI_CONST MI_ParameterDecl MSFT_WindowsServiceManager_GetWindowsServices_status_param =
{
    MI_FLAG_PARAMETER|MI_FLAG_IN, /* flags */
    0x00737306, /* code */
    MI_T("status"), /* name */
    MSFT_WindowsServiceManager_GetWindowsServices_status_quals, /* qualifiers */
    MI_COUNT(MSFT_WindowsServiceManager_GetWindowsServices_status_quals), /* numQualifiers */
    MI_UINT32, /* type */
    NULL, /* className */
    0, /* subscript */
    offsetof(MSFT_WindowsServiceManager_GetWindowsServices, status), /* offset */
};

static MI_CONST MI_Boolean MSFT_WindowsServiceManager_GetWindowsServices_services_Out_qual_value = 1;

static MI_CONST MI_Qualifier MSFT_WindowsServiceManager_GetWindowsServices_services_Out_qual =
{
    MI_T("Out"),
    MI_BOOLEAN,
    MI_FLAG_DISABLEOVERRIDE|MI_FLAG_TOSUBCLASS,
    &MSFT_WindowsServiceManager_GetWindowsServices_services_Out_qual_value
};

static MI_CONST MI_Boolean MSFT_WindowsServiceManager_GetWindowsServices_services_Stream_qual_value = 1;

static MI_CONST MI_Qualifier MSFT_WindowsServiceManager_GetWindowsServices_services_Stream_qual =
{
    MI_T("Stream"),
    MI_BOOLEAN,
    MI_FLAG_DISABLEOVERRIDE|MI_FLAG_TOSUBCLASS,
    &MSFT_WindowsServiceManager_GetWindowsServices_services_Stream_qual_value
};

static MI_CONST MI_Char* MSFT_WindowsServiceManager_GetWindowsServices_services_EmbeddedInstance_qual_value = MI_T("MSFT_WindowsService");

static MI_CONST MI_Qualifier MSFT_WindowsServiceManager_GetWindowsServices_services_EmbeddedInstance_qual =
{
    MI_T("EmbeddedInstance"),
    MI_STRING,
    MI_FLAG_ENABLEOVERRIDE|MI_FLAG_TOSUBCLASS,
    &MSFT_WindowsServiceManager_GetWindowsServices_services_EmbeddedInstance_qual_value
};

static MI_Qualifier MI_CONST* MI_CONST MSFT_WindowsServiceManager_GetWindowsServices_services_quals[] =
{
    &MSFT_WindowsServiceManager_GetWindowsServices_services_Out_qual,
    &MSFT_WindowsServiceManager_GetWindowsServices_services_Stream_qual,
    &MSFT_WindowsServiceManager_GetWindowsServices_services_EmbeddedInstance_qual,
};

/* parameter MSFT_WindowsServiceManager.GetWindowsServices(): services */
static MI_CONST MI_ParameterDecl MSFT_WindowsServiceManager_GetWindowsServices_services_param =
{
    MI_FLAG_PARAMETER|MI_FLAG_OUT|MI_FLAG_STREAM, /* flags */
    0x00737308, /* code */
    MI_T("services"), /* name */
    MSFT_WindowsServiceManager_GetWindowsServices_services_quals, /* qualifiers */
    MI_COUNT(MSFT_WindowsServiceManager_GetWindowsServices_services_quals), /* numQualifiers */
    MI_INSTANCEA, /* type */
    MI_T("MSFT_WindowsService"), /* className */
    0, /* subscript */
    0xFFFFFFFF, /* offset */
};

static MI_CONST MI_Boolean MSFT_WindowsServiceManager_GetWindowsServices_MIReturn_Static_qual_value = 1;

static MI_CONST MI_Qualifier MSFT_WindowsServiceManager_GetWindowsServices_MIReturn_Static_qual =
{
    MI_T("Static"),
    MI_BOOLEAN,
    MI_FLAG_DISABLEOVERRIDE|MI_FLAG_TOSUBCLASS,
    &MSFT_WindowsServiceManager_GetWindowsServices_MIReturn_Static_qual_value
};

static MI_Qualifier MI_CONST* MI_CONST MSFT_WindowsServiceManager_GetWindowsServices_MIReturn_quals[] =
{
    &MSFT_WindowsServiceManager_GetWindowsServices_MIReturn_Static_qual,
};

/* parameter MSFT_WindowsServiceManager.GetWindowsServices(): MIReturn */
static MI_CONST MI_ParameterDecl MSFT_WindowsServiceManager_GetWindowsServices_MIReturn_param =
{
    MI_FLAG_PARAMETER|MI_FLAG_OUT, /* flags */
    0x006D6E08, /* code */
    MI_T("MIReturn"), /* name */
    MSFT_WindowsServiceManager_GetWindowsServices_MIReturn_quals, /* qualifiers */
    MI_COUNT(MSFT_WindowsServiceManager_GetWindowsServices_MIReturn_quals), /* numQualifiers */
    MI_UINT32, /* type */
    NULL, /* className */
    0, /* subscript */
    offsetof(MSFT_WindowsServiceManager_GetWindowsServices, MIReturn), /* offset */
};

static MI_ParameterDecl MI_CONST* MI_CONST MSFT_WindowsServiceManager_GetWindowsServices_params[] =
{
    &MSFT_WindowsServiceManager_GetWindowsServices_MIReturn_param,
    &MSFT_WindowsServiceManager_GetWindowsServices_status_param,
    &MSFT_WindowsServiceManager_GetWindowsServices_services_param,
};

/* method MSFT_WindowsServiceManager.GetWindowsServices() */
MI_CONST MI_MethodDecl MSFT_WindowsServiceManager_GetWindowsServices_rtti =
{
    MI_FLAG_METHOD|MI_FLAG_STATIC, /* flags */
    0x00677312, /* code */
    MI_T("GetWindowsServices"), /* name */
    MSFT_WindowsServiceManager_GetWindowsServices_quals, /* qualifiers */
    MI_COUNT(MSFT_WindowsServiceManager_GetWindowsServices_quals), /* numQualifiers */
    MSFT_WindowsServiceManager_GetWindowsServices_params, /* parameters */
    MI_COUNT(MSFT_WindowsServiceManager_GetWindowsServices_params), /* numParameters */
    sizeof(MSFT_WindowsServiceManager_GetWindowsServices), /* size */
    MI_UINT32, /* returnType */
    MI_T("MSFT_WindowsServiceManager"), /* origin */
    MI_T("MSFT_WindowsServiceManager"), /* propagator */
    &schemaDecl, /* schema */
    (MI_ProviderFT_Invoke)MSFT_WindowsServiceManager_Invoke_GetWindowsServices, /* method */
};

static MI_MethodDecl MI_CONST* MI_CONST MSFT_WindowsServiceManager_meths[] =
{
    &MSFT_WindowsServiceManager_GetWindowsServices_rtti,
};

static MI_CONST MI_ProviderFT MSFT_WindowsServiceManager_funcs =
{
  (MI_ProviderFT_Load)MSFT_WindowsServiceManager_Load,
  (MI_ProviderFT_Unload)MSFT_WindowsServiceManager_Unload,
  (MI_ProviderFT_GetInstance)NULL,
  (MI_ProviderFT_EnumerateInstances)NULL,
  (MI_ProviderFT_CreateInstance)NULL,
  (MI_ProviderFT_ModifyInstance)NULL,
  (MI_ProviderFT_DeleteInstance)NULL,
  (MI_ProviderFT_AssociatorInstances)NULL,
  (MI_ProviderFT_ReferenceInstances)NULL,
  (MI_ProviderFT_EnableIndications)NULL,
  (MI_ProviderFT_DisableIndications)NULL,
  (MI_ProviderFT_Subscribe)NULL,
  (MI_ProviderFT_Unsubscribe)NULL,
  (MI_ProviderFT_Invoke)NULL,
};

static MI_CONST MI_Char* MSFT_WindowsServiceManager_Description_qual_value = MI_T("388");

static MI_CONST MI_Qualifier MSFT_WindowsServiceManager_Description_qual =
{
    MI_T("Description"),
    MI_STRING,
    MI_FLAG_ENABLEOVERRIDE|MI_FLAG_TOSUBCLASS|MI_FLAG_TRANSLATABLE,
    &MSFT_WindowsServiceManager_Description_qual_value
};

static MI_CONST MI_Char* MSFT_WindowsServiceManager_ClassVersion_qual_value = MI_T("1.0.0");

static MI_CONST MI_Qualifier MSFT_WindowsServiceManager_ClassVersion_qual =
{
    MI_T("ClassVersion"),
    MI_STRING,
    MI_FLAG_ENABLEOVERRIDE|MI_FLAG_RESTRICTED,
    &MSFT_WindowsServiceManager_ClassVersion_qual_value
};

static MI_Qualifier MI_CONST* MI_CONST MSFT_WindowsServiceManager_quals[] =
{
    &MSFT_WindowsServiceManager_Description_qual,
    &MSFT_WindowsServiceManager_ClassVersion_qual,
};

/* class MSFT_WindowsServiceManager */
MI_CONST MI_ClassDecl MSFT_WindowsServiceManager_rtti =
{
    MI_FLAG_CLASS, /* flags */
    0x006D721A, /* code */
    MI_T("MSFT_WindowsServiceManager"), /* name */
    MSFT_WindowsServiceManager_quals, /* qualifiers */
    MI_COUNT(MSFT_WindowsServiceManager_quals), /* numQualifiers */
    NULL, /* properties */
    0, /* numProperties */
    sizeof(MSFT_WindowsServiceManager), /* size */
    NULL, /* superClass */
    NULL, /* superClassDecl */
    MSFT_WindowsServiceManager_meths, /* methods */
    MI_COUNT(MSFT_WindowsServiceManager_meths), /* numMethods */
    &schemaDecl, /* schema */
    &MSFT_WindowsServiceManager_funcs, /* functions */
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
    &CIM_ConcreteJob_rtti,
    &CIM_EnabledLogicalElement_rtti,
    &CIM_Error_rtti,
    &CIM_Job_rtti,
    &CIM_LogicalElement_rtti,
    &CIM_ManagedElement_rtti,
    &CIM_ManagedSystemElement_rtti,
    &CIM_Service_rtti,
    &MSFT_WindowsService_rtti,
    &MSFT_WindowsServiceManager_rtti,
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

