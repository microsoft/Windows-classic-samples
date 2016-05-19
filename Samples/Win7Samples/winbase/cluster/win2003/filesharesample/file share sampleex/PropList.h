/////////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2003 <company name>
//
//  Module Name:
//      PropList.h
//
//  Implementation File:
//      PropList.cpp
//
//  Description:
//      Definition of the CClusPropList class.
//
//  Author:
//      <name> (<e-mail name>) Mmmm DD, 2003
//
//  Revision History:
//
//  Notes:
//
/////////////////////////////////////////////////////////////////////////////

#pragma once

/////////////////////////////////////////////////////////////////////////////
// Include Files
/////////////////////////////////////////////////////////////////////////////
#include <windows.h>
#include <clusapi.h>

/////////////////////////////////////////////////////////////////////////////
// Forward Class Declarations
/////////////////////////////////////////////////////////////////////////////

class CObjectProperty;
class CClusPropValueList;
class CClusPropList;

/////////////////////////////////////////////////////////////////////////////
// External Class Declarations
/////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
// Type Definitions
/////////////////////////////////////////////////////////////////////////////

#if !defined( ASSERT )
#include <crtdbg.h>
#define ASSERT _ASSERTE
#endif // !defined( ASSERT )

#pragma warning( push, 3 )

/////////////////////////////////////////////////////////////////////////////
// Custom string compare function declarations. Used to replace lstrcmp*
/////////////////////////////////////////////////////////////////////////////

//
// Full string compares.
//

int
IBSTRCompareW(
      BSTR  bstrString1In
    , BSTR  bstrString2In
    );

int
BSTRCompareW(
      BSTR  bstrString1In
    , BSTR  bstrString2In
    );

int
IStringCompareW(
      LPCWSTR pcwszString1In
    , LPCWSTR pcwszString2In
    );

int
StringCompareW(
      LPCWSTR pcwszString1In
    , LPCWSTR pcwszString2In
    );

int
IStringCompareW(
      LPCWSTR   pcwszString1In
    , size_t    cch1In
    , LPCWSTR   pcwszString2In
    , size_t    cch2In
    );

int
StringCompareW(
      LPCWSTR   pcwszString1In
    , size_t    cch1In
    , LPCWSTR   pcwszString2In
    , size_t    cch2In
    );

//
// Partial string compares.  They use whichever one's length is shortest
// if both are specified or the NBSTR routines are used.
//

int
NIBSTRCompareW(
      BSTR  bstrString1In
    , BSTR  bstrString2In
    );

int
NBSTRCompareW(
      BSTR  bstrString1In
    , BSTR  bstrString2In
    );

int
NIStringCompareW(
      LPCWSTR   pcwszString1In
    , LPCWSTR   pcwszString2In
    , size_t    cchIn
    );

int
NStringCompareW(
      LPCWSTR   pcwszString1In
    , LPCWSTR   pcwszString2In
    , size_t    cchIn
    );

int
NIStringCompareW(
      LPCWSTR   pcwszString1In
    , size_t    cch1In
    , LPCWSTR   pcwszString2In
    , size_t    cch2In
    );

int
NStringCompareW(
      LPCWSTR   pcwszString1In
    , size_t    cch1In
    , LPCWSTR   pcwszString2In
    , size_t    cch2In
    );

/////////////////////////////////////////////////////////////////////////////
//++
//
//  class CObjectProperty
//
//  Description:
//      Describes a property in a cluster property list.
//
//  Inheritance:
//      CObjectProperty
//
//--
/////////////////////////////////////////////////////////////////////////////

#if defined( __AFX_H__ ) || ( defined( __ATLTMP_H__ ) && !defined( _ATL_TMP_NO_CSTRING ) )

class CObjectProperty
{
public:

    LPCWSTR                 m_pwszName;
    CLUSTER_PROPERTY_FORMAT m_propFormat;

    union CValue
    {
        CString *   pstr;
        LONG *      pl;
        DWORD *     pdw;
        BOOL *      pb;
        struct
        {
            PBYTE * ppb;
            DWORD * pcb;
        };
    };

    CValue  m_value;
    CValue  m_valuePrev;
    CValue  m_valueEx;      // expand_sz value (if any)

    DWORD   m_dwFlags;

    enum ObjPropFlags
    {
        opfNew = 1
    };

    CObjectProperty::CObjectProperty( void )
    {
        m_value.pstr = NULL;
        m_value.pcb = NULL;
        m_valuePrev.pstr = NULL;
        m_valuePrev.pcb = NULL;
        m_valueEx.pstr = NULL;
        m_valueEx.pcb = NULL;
    };

    void
    Set(
          LPCWSTR   pwszNameIn
        , CString & rstrValueInout
        , CString & rstrPrevValueInout
        , DWORD     dwFlagsIn = 0
        )
    {
        m_pwszName = pwszNameIn;
        m_propFormat = CLUSPROP_FORMAT_SZ;
        m_value.pstr = &rstrValueInout;
        m_value.pcb = NULL;
        m_valuePrev.pstr = &rstrPrevValueInout;
        m_valuePrev.pcb = NULL;
        m_valueEx.pstr = NULL;
        m_valueEx.pcb = NULL;
        m_dwFlags = dwFlagsIn;

    } //*** Set( CString & )

    void
    SetExpandSz(
          LPCWSTR   pwszNameIn
        , CString & rstrValueInout
        , CString & rstrPrevValueInout
        , DWORD     dwFlagsIn = 0
        )
    {
        m_pwszName = pwszNameIn;
        m_propFormat = CLUSPROP_FORMAT_EXPAND_SZ;
        m_value.pstr = &rstrValueInout;
        m_value.pcb = NULL;
        m_valuePrev.pstr = &rstrPrevValueInout;
        m_valuePrev.pcb = NULL;
        m_valueEx.pstr= NULL;
        m_valueEx.pcb = NULL;
        m_dwFlags = dwFlagsIn;

    } //*** Set( CString & )

    // Set() to get extra EXPANDED_SZ value
    void
    Set(
          LPCWSTR   pwszNameIn
        , CString & rstrValueInout
        , CString & rstrPrevValueInout
        , CString & rstrValueEx
        , DWORD     dwFlagsIn = 0
        )
    {
        m_pwszName = pwszNameIn;
        m_propFormat = CLUSPROP_FORMAT_SZ;
        m_value.pstr = &rstrValueInout;
        m_value.pcb = NULL;
        m_valuePrev.pstr = &rstrPrevValueInout;
        m_valuePrev.pcb = NULL;
        m_valueEx.pstr = &rstrValueEx;
        m_valueEx.pcb = NULL;
        m_dwFlags = dwFlagsIn;

    } //*** Set( CString & )

    // Set() to get extra EXPANDED_SZ value
    void
    SetExpandSz(
          LPCWSTR   pwszNameIn
        , CString & rstrValueInout
        , CString & rstrPrevValueInout
        , CString & rstrValueEx
        , DWORD     dwFlagsIn = 0
        )
    {
        m_pwszName = pwszNameIn;
        m_propFormat = CLUSPROP_FORMAT_EXPAND_SZ;
        m_value.pstr = &rstrValueInout;
        m_value.pcb = NULL;
        m_valuePrev.pstr = &rstrPrevValueInout;
        m_valuePrev.pcb = NULL;
        m_valueEx.pstr = &rstrValueEx;
        m_valueEx.pcb = NULL;
        m_dwFlags = dwFlagsIn;

    } //*** Set( CString & )

    void
    Set(
          LPCWSTR   pwszNameIn
        , LONG &    rnValueInout
        , LONG &    rnPrevValueInout
        , DWORD     dwFlagsIn = 0
        )
    {
        m_pwszName = pwszNameIn;
        m_propFormat = CLUSPROP_FORMAT_LONG;
        m_value.pl = &rnValueInout;
        m_value.pcb = NULL;
        m_valuePrev.pl = &rnPrevValueInout;
        m_valuePrev.pcb = NULL;
        m_valueEx.pstr = NULL;
        m_valueEx.pcb = NULL;
        m_dwFlags = dwFlagsIn;

    } //*** Set( LONG & )

    void
    Set(
          LPCWSTR   pwszNameIn
        , DWORD &   rdwValueInout
        , DWORD &   rdwPrevValueInout
        , DWORD     dwFlagsIn = 0
        )
    {
        m_pwszName = pwszNameIn;
        m_propFormat = CLUSPROP_FORMAT_DWORD;
        m_value.pdw = &rdwValueInout;
        m_value.pcb = NULL;
        m_valuePrev.pdw = &rdwPrevValueInout;
        m_valuePrev.pcb = NULL;
        m_valueEx.pstr = NULL;
        m_valueEx.pcb = NULL;
        m_dwFlags = dwFlagsIn;

    } //*** Set( DWORD & )

    void
    Set(
          LPCWSTR   pwszNameIn
        , BOOL &    rbValueInout
        , BOOL &    rbPrevValueInout
        , DWORD     dwFlagsIn = 0
        )
    {
        m_pwszName = pwszNameIn;
        m_propFormat = CLUSPROP_FORMAT_DWORD;
        m_value.pb = &rbValueInout;
        m_value.pcb = NULL;
        m_valuePrev.pb = &rbPrevValueInout;
        m_valuePrev.pcb = NULL;
        m_valueEx.pstr = NULL;
        m_valueEx.pcb = NULL;
        m_dwFlags = dwFlagsIn;

    } //*** Set( BOOL & )

    void
    Set(
          LPCWSTR   pwszNameIn
        , PBYTE &   rpbValueInoout
        , DWORD &   rcbValueInout
        , PBYTE &   rpbPrevValueInout
        , DWORD &   rcbPrevValueInout
        , DWORD     dwFlagsIn = 0
        )
    {
        m_pwszName = pwszNameIn;
        m_propFormat = CLUSPROP_FORMAT_BINARY;
        m_value.ppb = &rpbValueInoout;
        m_value.pcb = &rcbValueInout;
        m_valuePrev.ppb = &rpbPrevValueInout;
        m_valuePrev.pcb = &rcbPrevValueInout;
        m_valueEx.pstr = NULL;
        m_valueEx.pcb = NULL;
        m_dwFlags = dwFlagsIn;

    } //*** Set( PBYTE & )

    void
    Set(
          LPCWSTR   pwszNameIn
        , LPWSTR &  rpwszValue
        , DWORD &   rcbValueInout
        , LPWSTR &  rpwszPrevValue
        , DWORD &   rcbPrevValueInout
        , DWORD     dwFlagsIn = 0
        )
    {
        m_pwszName = pwszNameIn;
        m_propFormat = CLUSPROP_FORMAT_MULTI_SZ;
        m_value.ppb = reinterpret_cast< PBYTE * >( &rpwszValue );
        m_value.pcb = &rcbValueInout;
        m_valuePrev.ppb = reinterpret_cast< PBYTE * >( &rpwszPrevValue );
        m_valuePrev.pcb = &rcbPrevValueInout;
        m_valueEx.pstr = NULL;
        m_valueEx.pcb = NULL;
        m_dwFlags = dwFlagsIn;

    } //*** Set( LPWSTR & )

}; //*** class CObjectProperty

#endif  // defined( __AFX_H__ ) || ( defined( __ATLTMP_H__ ) && !defined( _ATL_TMP_NO_CSTRING ) )

/////////////////////////////////////////////////////////////////////////////
//++
//
//  class CClusPropValueList
//
//  Description:
//      Describes a cluster property list.
//
//  Inheritance:
//      CClusPropValueList
//      CObject (MFC only)
//
//--
/////////////////////////////////////////////////////////////////////////////

class CClusPropValueList
#ifdef __AFX_H__
    : public CObject
#endif // __AFX_H__
{
#ifdef __AFX_H__
    DECLARE_DYNAMIC( CClusPropValueList );
#endif // __AFX_H__

public:

    //
    // Construction.
    //

    // Default constructor
    CClusPropValueList( void )
        : m_cbDataSize( 0 )
        , m_cbDataLeft( 0 )
        , m_cbBufferSize( 0 )
        , m_fAtEnd( FALSE )
    {
        m_cbhValueList.pb = NULL;
        m_cbhCurrentValue.pb = NULL;

    } //*** CClusPropValueList

    //  Copy constructor.
    CClusPropValueList(
        const CClusPropValueList & rcpvlInout
        )
        : m_cbBufferSize( 0 )
        , m_fAtEnd( FALSE )
    {
        Init( rcpvlInout );

    } //*** CClusPropValueList

    //  Buffer helper constructor.
    CClusPropValueList(
          CLUSPROP_BUFFER_HELPER    cbhValueListIn
        , size_t                    cbDataSizeIn
        )
        : m_cbBufferSize( 0 )
        , m_fAtEnd( FALSE )
    {
        Init( cbhValueListIn, cbDataSizeIn );

    } //*** CClusPropValueList

    //  Destructor
    ~CClusPropValueList( void )
    {
        DeleteValueList();

    } //*** ~CClusPropValueList

    //  Initialize the value list
    void
    Init(
        const CClusPropValueList & rcpvlInout
        )
    {
        ASSERT( m_cbBufferSize == 0 );

        m_cbhValueList      = rcpvlInout.m_cbhValueList;
        m_cbhCurrentValue   = rcpvlInout.m_cbhCurrentValue;
        m_cbDataSize        = rcpvlInout.m_cbDataSize;
        m_cbDataLeft        = rcpvlInout.m_cbDataLeft;
        m_fAtEnd            = rcpvlInout.m_fAtEnd;

    } //*** Init

    //  Initialize the value list from a buffer helper
    void
    Init(
          const CLUSPROP_BUFFER_HELPER  cbhValueListIn
        , size_t                        cbDataSizeIn
        )
    {
        ASSERT( m_cbBufferSize == 0 );

        m_cbhValueList      = cbhValueListIn;
        m_cbhCurrentValue   = cbhValueListIn;
        m_cbDataSize        = cbDataSizeIn;
        m_cbDataLeft        = cbDataSizeIn;
        m_fAtEnd            = FALSE;

    } //*** Init

    //  Assignment operator
    void operator=( const CClusPropValueList & rcpvlInout )
    {
        ASSERT( m_cbBufferSize == 0 );

        m_cbhValueList      = rcpvlInout.m_cbhValueList;
        m_cbhCurrentValue   = rcpvlInout.m_cbhCurrentValue;
        m_cbDataSize        = rcpvlInout.m_cbDataSize;
        m_cbDataLeft        = rcpvlInout.m_cbDataLeft;
        m_fAtEnd            = rcpvlInout.m_fAtEnd;

    } //*** operator=

    //
    // Accessor methods.
    //

    // Buffer helper cast operator to access the current value
    operator const CLUSPROP_BUFFER_HELPER( void ) const
    {
        return m_cbhCurrentValue;

    } //*** operator CLUSPROP_BUFFER_HELPER

    // Access the value list
    CLUSPROP_BUFFER_HELPER CbhValueList( void ) const
    {
        return m_cbhValueList;

    } //*** CbhValueList

    // Access the current value
    CLUSPROP_BUFFER_HELPER CbhCurrentValue( void ) const
    {
        return m_cbhCurrentValue;

    } //*** CbhCurrentValue

    // Access the format of the current value
    CLUSTER_PROPERTY_FORMAT CpfCurrentValueFormat( void ) const
    {
        return static_cast< CLUSTER_PROPERTY_FORMAT >( m_cbhCurrentValue.pValue->Syntax.wFormat );

    } //*** CpfCurrentValueFormat

    // Access the type of the current value
    CLUSTER_PROPERTY_TYPE CptCurrentValueType( void ) const
    {
        return static_cast< CLUSTER_PROPERTY_TYPE >( m_cbhCurrentValue.pValue->Syntax.wType );

    } //*** CptCurrentValueType

    // Access the syntax of the current value
    CLUSTER_PROPERTY_SYNTAX CpsCurrentValueSyntax( void ) const
    {
        return static_cast< CLUSTER_PROPERTY_SYNTAX >( m_cbhCurrentValue.pValue->Syntax.dw );

    } //*** CpsCurrentValueSyntax

    // Access the length of the data of the current value
    DWORD CbCurrentValueLength( void ) const
    {
        DWORD cbLength = 0;

        if ( m_cbhCurrentValue.pb == NULL )
        {
            cbLength = 0;
        } // if: no value list allocated yet
        else
        {
            cbLength = m_cbhCurrentValue.pValue->cbLength;
        } // else: value list allocated

        return cbLength;

    } //*** CbCurrentValueLength

    // Access size of the data in the buffer.
    size_t CbDataSize( void ) const
    {
        return m_cbDataSize;

    } //*** CbDataSize

    // Access amount of data left in buffer after current value
    size_t CbDataLeft( void ) const
    {
        return m_cbDataLeft;

    } //*** CbDataLeft

    //
    // Parsing methods.
    //

    // Move to the first value in the list
    DWORD ScMoveToFirstValue( void );

    // Move the value after the current one in the list
    DWORD ScMoveToNextValue( void );

    // Query whether we are at the last value in the list or not
    DWORD ScCheckIfAtLastValue( void );

    //
    // Methods for building a value list.
    //

    DWORD ScAllocValueList( size_t cbMinimumIn );

    // Delete the value list buffer and cleanup support variables
    void DeleteValueList( void )
    {
        //
        // If m_cbBufferSize is greater then 0 then we allocated the value list.
        // If it's zero then the value list is a part of the property list in
        // CClusPropList.
        //

        if ( m_cbBufferSize > 0 )
        {
            delete [] m_cbhValueList.pb;

            m_cbhValueList.pb = NULL;
            m_cbhCurrentValue.pb = NULL;
            m_cbBufferSize = 0;
            m_cbDataSize = 0;
            m_cbDataLeft = 0;
            m_fAtEnd = FALSE;
        } // if: we allocated anything

    } //*** DeleteValueList

    // Get a value list from a resource
    DWORD
    ScGetResourceValueList(
          HRESOURCE hResourceIn
        , DWORD     dwControlCodeIn
        , HNODE     hHostNodeIn = NULL
        , LPVOID    lpBufferIn = NULL
        , size_t    cbBufferIn = 0
        );

    // Get a value list from a resource type
    DWORD ScGetResourceTypeValueList(
          HCLUSTER  hClusterIn
        , LPCWSTR   pwszResTypeNameIn
        , DWORD     dwControlCodeIn
        , HNODE     hHostNodeIn = NULL
        , LPVOID    lpBufferIn = NULL
        , size_t    cbBufferIn = 0
        );

private:
    CLUSPROP_BUFFER_HELPER  m_cbhValueList;     // Pointer to the value list for parsing.
    CLUSPROP_BUFFER_HELPER  m_cbhCurrentValue;  // Pointer to the current value for parsing.
    size_t                  m_cbDataSize;       // Amount of data in the buffer.
    size_t                  m_cbDataLeft;       // Amount of data left in buffer after current value.
    size_t                  m_cbBufferSize;     // Size of the buffer if we allocated it.
    BOOL                    m_fAtEnd;           // Indicates whether at last value in list.

}; //*** class CClusPropValueList

/////////////////////////////////////////////////////////////////////////////
//++
//
//  class CClusPropList
//
//  Description:
//      Describes a cluster property list.
//
//  Inheritance:
//      CClusPropList
//      CObject (MFC only)
//
//--
/////////////////////////////////////////////////////////////////////////////
class CClusPropList
#ifdef __AFX_H__
    : public CObject
#endif // __AFX_H__
{
#ifdef __AFX_H__
    DECLARE_DYNAMIC( CClusPropList );
#endif // __AFX_H__

public:

    //
    // Construction.
    //

    // Default constructor
    CClusPropList(
        BOOL bAlwaysAddPropIn = FALSE
        )
        : m_fAlwaysAddProp( bAlwaysAddPropIn )
        , m_cbBufferSize( 0 )
        , m_cbDataSize( 0 )
        , m_cbDataLeft( 0 )
        , m_nPropsRemaining( 0 )
    {
        m_cbhPropList.pList     = NULL;
        m_cbhCurrentProp.pb     = NULL;
        m_cbhCurrentPropName.pb = NULL;

    } //*** CClusPropList

    // Destructor
    ~CClusPropList( void )
    {
        DeletePropList();

    } //*** ~CClusPropList

    // Copy list into this list (like assignment operator)
    DWORD ScCopy( const PCLUSPROP_LIST pcplPropListIn, size_t cbListSizeIn );

    // Append list into this list
    DWORD ScAppend( const CClusPropList & rclPropListIn );

    // Delete the property list buffer and cleanup support variables
    void DeletePropList( void )
    {
        delete [] m_cbhPropList.pb;

        m_cbhPropList.pb = NULL;
        m_cbhCurrentProp.pb = NULL;
        m_cbhCurrentPropName.pb = NULL;
        m_cbBufferSize = 0;
        m_cbDataSize = 0;
        m_cbDataLeft = 0;

    } //*** DeletePropList

protected:

    //
    // Attributes.
    //

    BOOL                    m_fAlwaysAddProp;       // Indicate if properties should be added even if not different.
    CLUSPROP_BUFFER_HELPER  m_cbhPropList;          // Pointer to the beginning of the list.
    CLUSPROP_BUFFER_HELPER  m_cbhCurrentProp;       // Pointer to the current property.
    size_t                  m_cbBufferSize;         // Allocated size of the buffer.
    size_t                  m_cbDataSize;           // Amount of data in the buffer.
    size_t                  m_cbDataLeft;           // Amount of data left in buffer after current value.

private:
    CLUSPROP_BUFFER_HELPER  m_cbhCurrentPropName;   // Pointer to the current name for parsing
    DWORD                   m_nPropsRemaining;      // Used by BMoveToNextProperty() to track end of list.
    CClusPropValueList      m_pvlValues;            // Helper class for value list of current property.

public:

    //
    // Accessor methods.
    //

    // Access the values of the current property
    const CClusPropValueList & RPvlPropertyValue( void )
    {
        return m_pvlValues;

    } //*** RPvlPropertyValue

    // Access the property list
    operator PCLUSPROP_LIST( void ) const
    {
        return m_cbhPropList.pList;

    } //*** operator PCLUSPROP_LIST

    // Access allocated size of the buffer
    size_t CbBufferSize( void ) const
    {
        return m_cbBufferSize;

    } //*** CbBufferSize

    // Access the name of the current property
    LPCWSTR PszCurrentPropertyName( void ) const
    {
        return m_cbhCurrentPropName.pName->sz;

    } //*** PszCurrentPropertyName

    // Access the current property name as a buffer helper
    const CLUSPROP_BUFFER_HELPER CbhCurrentPropertyName( void )
    {
        return m_cbhCurrentPropName;

    } //*** CbhCurrentPropertyName

    // Access value list of the current property as a buffer helper
    const CLUSPROP_BUFFER_HELPER CbhCurrentValueList( void )
    {
        return m_pvlValues.CbhValueList();

    } //*** CbhCurrentValueList

    // Access current value of the current property as a buffer helper
    const CLUSPROP_BUFFER_HELPER CbhCurrentValue( void )
    {
        return m_pvlValues.CbhCurrentValue();

    } //*** CbhCurrentValue

    // Access the format of the current value of the current property
    CLUSTER_PROPERTY_FORMAT CpfCurrentValueFormat( void ) const
    {
        return m_pvlValues.CpfCurrentValueFormat();

    } //*** CpfCurrentValueFormat

    // Access the type of the current value of the current property
    CLUSTER_PROPERTY_TYPE CptCurrentValueType( void ) const
    {
        return m_pvlValues.CptCurrentValueType();

    } //*** CptCurrentValueType

    // Access the syntax of the current value of the current property
    CLUSTER_PROPERTY_SYNTAX CpsCurrentValueSyntax( void ) const
    {
        return m_pvlValues.CpsCurrentValueSyntax();

    } //*** CpsCurrentValueSyntax

    // Access the length of the current value of the current property
    size_t CbCurrentValueLength( void ) const
    {
        return m_pvlValues.CbCurrentValueLength();

    } //*** CbCurrentValueLength

    PCLUSPROP_LIST Plist( void )
    {
        return m_cbhPropList.pList;

    } //*** Plist

    const CLUSPROP_BUFFER_HELPER CbhPropList( void ) const
    {
        return m_cbhPropList;

    } //*** CbhPropList

    PBYTE PbPropList( void ) const
    {
        return m_cbhPropList.pb;

    } //*** PbPropList

    size_t CbPropList( void ) const
    {
        return m_cbDataSize + sizeof( CLUSPROP_SYNTAX ); /*endmark*/

    } //*** CbPropList

    // Access amount of data left in buffer after current value
    size_t CbDataLeft( void ) const
    {
        return m_cbDataLeft;

    } //*** CbDataLeft

    DWORD Cprops( void ) const
    {
        if ( m_cbhPropList.pb == NULL )
        {
            return 0;
        } // if:  no buffer yet

        return m_cbhPropList.pList->nPropertyCount;

    } //*** Cprops

public:
    //
    // Parsing methods.
    //

    // Initialize the size after getting properties from an external source
    void InitSize( size_t cbSizeIn )
    {
        ASSERT( m_cbhPropList.pb != NULL );
        ASSERT( m_cbBufferSize > 0 );

        m_cbDataSize = cbSizeIn;
        m_cbDataLeft = cbSizeIn;

    } //*** InitSize

    // Move to the first property in the list
    DWORD ScMoveToFirstProperty( void );

    // Move the property after the current one in the list
    DWORD ScMoveToNextProperty( void );

    // Move to a property by specifying its name
    DWORD ScMoveToPropertyByName( LPCWSTR pwszPropNameIn );

    // Move to the first value in the current property
    DWORD ScMoveToFirstPropertyValue( void )
    {
        return m_pvlValues.ScMoveToFirstValue();

    } //*** ScMoveToFirstPropertyValue

    // Move the the value after the current on in the current property
    DWORD ScMoveToNextPropertyValue( void )
    {
        return m_pvlValues.ScMoveToNextValue();

    } //*** ScMoveToNextPropertyValue

    // Query whether we are at the last property in the list or not
    DWORD ScCheckIfAtLastProperty( void ) const
    {
        DWORD sc;

        if ( m_nPropsRemaining <= 1 )
        {
            sc = ERROR_NO_MORE_ITEMS;
        } // if:  at the last property
        else
        {
            sc = ERROR_SUCCESS;
        } // else:  not at the last property

        return sc;

    } //*** ScCheckIfAtLastProperty

    // Query whether the list is empty or not
    BOOL BIsListEmpty( void ) const
    {
        ASSERT( m_cbhPropList.pb != NULL );
        ASSERT( m_cbDataSize >= sizeof( m_cbhPropList.pList->nPropertyCount ) );

        return m_cbhPropList.pList->nPropertyCount == 0;

    } //*** BIsListEmpty

public:
    //
    // Methods for building a property list.
    //

    // Allocate a property list
    DWORD ScAllocPropList( size_t cbMinimumIn );

    void ClearPropList( void )
    {
        m_cbDataSize = 0;
        m_cbDataLeft = 0;

        if ( m_cbBufferSize != 0 )
        {
            ZeroMemory( m_cbhPropList.pb, m_cbBufferSize );
            m_cbhCurrentProp.pb = m_cbhPropList.pb + sizeof( m_cbhPropList.pList->nPropertyCount );
            m_cbhCurrentPropName = m_cbhCurrentProp;
        } // if:  buffer already allocated

    } //*** ClearPropList

    DWORD ScAddProp( LPCWSTR pwszNameIn, LPCWSTR pwszValueIn, LPCWSTR pwszPrevValueIn );

    DWORD ScAddExpandSzProp( LPCWSTR pwszNameIn, LPCWSTR pwszValueIn, LPCWSTR pwszPrevValueIn );

    DWORD ScAddMultiSzProp( LPCWSTR pwszNameIn, LPCWSTR pwszValueIn, LPCWSTR pwszPrevValueIn );

    DWORD ScAddProp( LPCWSTR pwszNameIn, DWORD nValueIn, DWORD nPrevValueIn );

    DWORD ScAddProp( LPCWSTR pwszNameIn, LONG nValueIn, LONG nPrevValueIn );

    DWORD ScAddProp( LPCWSTR pwszNameIn, ULONGLONG ullValueIn, ULONGLONG ullPrevValueIn );

    DWORD ScAddProp( LPCWSTR pwszNameIn, LONGLONG llValueIn, LONGLONG llPrevValueIn );

    DWORD ScSetPropToDefault( LPCWSTR pwszNameIn, CLUSTER_PROPERTY_FORMAT propFormatIn );

    DWORD ScAddProp(
              LPCWSTR               pwszNameIn
            , const unsigned char * pbValueIn
            , size_t                cbValueIn
            , const unsigned char * pbPrevValueIn
            , size_t                cbPrevValueIn
            );

    DWORD ScAddProp( LPCWSTR pwszNameIn, LPCWSTR pwszValueIn )
    {
        return ScAddProp( pwszNameIn, pwszValueIn, NULL );

    } //*** ScAddProp

    DWORD ScAddExpandSzProp( LPCWSTR pwszNameIn, LPCWSTR pwszValueIn )
    {
        return ScAddExpandSzProp( pwszNameIn, pwszValueIn, NULL );

    } //*** ScAddExpandSzProp

public:

    //
    // Get Property methods.
    //

    DWORD ScGetNodeProperties(
          HNODE     hNodeIn
        , DWORD     dwControlCodeIn
        , HNODE     hHostNodeIn = NULL
        , LPVOID    lpBufferIn = NULL
        , size_t    cbBufferIn = 0
        );

    DWORD ScGetGroupProperties(
          HGROUP   hGroupIn
        , DWORD    dwControlCodeIn
        , HNODE    hHostNodeIn = NULL
        , LPVOID   lpBufferIn = NULL
        , size_t   cbBufferIn = 0
        );

    DWORD ScGetResourceProperties(
          HRESOURCE    hResourceIn
        , DWORD        dwControlCodeIn
        , HNODE        hHostNodeIn = NULL
        , LPVOID       lpBufferIn = NULL
        , size_t       cbBufferIn = 0
        );

    DWORD ScGetResourceTypeProperties(
          HCLUSTER hClusterIn
        , LPCWSTR  pwszResTypeNameIn
        , DWORD    dwControlCodeIn
        , HNODE    hHostNodeIn = NULL
        , LPVOID   lpBufferIn = NULL
        , size_t   cbBufferIn = 0
        );

    DWORD ScGetNetworkProperties(
          HNETWORK hNetworkIn
        , DWORD    dwControlCodeIn
        , HNODE    hHostNodeIn = NULL
        , LPVOID   lpBufferIn = NULL
        , size_t   cbBufferIn = 0
        );

    DWORD ScGetNetInterfaceProperties(
          HNETINTERFACE    hNetInterfaceIn
        , DWORD            dwControlCodeIn
        , HNODE            hHostNodeIn  = NULL
        , LPVOID           lpBufferIn = NULL
        , size_t           cbBufferIn = 0
        );

    DWORD ScGetClusterProperties(
          HCLUSTER hClusterIn
        , DWORD    dwControlCodeIn
        , HNODE    hHostNodeIn = NULL
        , LPVOID   lpBufferIn = NULL
        , size_t   cbBufferIn = 0
        );

// Implementation
protected:
    void CopyProp(
          PCLUSPROP_SZ             ppropIn
        , CLUSTER_PROPERTY_TYPE    cptPropTypeIn
        , LPCWSTR                  pszIn
        , size_t                   cbszIn = 0
        );

    void CopyExpandSzProp(
          PCLUSPROP_SZ             ppropIn
        , CLUSTER_PROPERTY_TYPE    cptPropTypeIn
        , LPCWSTR                  pszIn
        , size_t                   cbszIn = 0
        );

    void CopyMultiSzProp(
          PCLUSPROP_MULTI_SZ       ppropIn
        , CLUSTER_PROPERTY_TYPE    cptPropTypeIn
        , LPCWSTR                  pszIn
        , size_t                   cbszIn = 0
        );

    void CopyProp(
          PCLUSPROP_DWORD          ppropIn
        , CLUSTER_PROPERTY_TYPE    cptPropTypeIn
        , DWORD                    nValueIn
        );

    void CopyProp(
          PCLUSPROP_LONG           ppropIn
        , CLUSTER_PROPERTY_TYPE    cptPropTypeIn
        , LONG                     nValueIn
        );

    void CopyProp(
          PCLUSPROP_ULARGE_INTEGER  ppropOut
        , CLUSTER_PROPERTY_TYPE     cptPropTypeIn
        , ULONGLONG                 ullValueIn
        );

    void CopyProp(
          PCLUSPROP_LARGE_INTEGER  ppropOut
        , CLUSTER_PROPERTY_TYPE    cptPropTypeIn
        , LONGLONG                 llValueIn
        );

    void CopyProp(
          PCLUSPROP_ULARGE_INTEGER  ppropOut
        , CLUSTER_PROPERTY_TYPE     cptPropTypeIn
        , LONGLONG                  llValueIn
        );
    void CopyProp(
          PCLUSPROP_BINARY      ppropOut
        , CLUSTER_PROPERTY_TYPE cptPropTypeIn
        , const unsigned char * pbValueIn
        , size_t                cbValueIn
        );

    void CopyEmptyProp(
          PCLUSPROP_VALUE           ppropOut
        , CLUSTER_PROPERTY_TYPE     cptPropTypeIn
        , CLUSTER_PROPERTY_FORMAT   propFormatIn
        );

}; //*** class CClusPropList

#pragma warning( pop )

