//***************************************************************************
//
// Copyright (c)  Microsoft Corporation, All Rights Reserved
//
//	File:  dpdetail.cpp
//
//	Description:
//		This file implements the DiskPerfDetails() routine which 
//		demonstrates how to enumerate properties for the DiskPerf
//		class and instances.
//
//	Part of :	WMIDiskPerf
//
//  History:	
//
//***************************************************************************
#include <wbemidl.h>
#include <stdio.h>
#include <strsafe.h>

void ShowPropInfo( IWbemClassObject *pInst );

char PropListHeader[] =
	"DiskPerf Property Descriptions:\n\n"
	"WMI Data ID\tProperty\tDescription\n"
	"======================================="
	"=======================================\n"
	"";


//==============================================================================
//
//	DiskPerfDetails( IWbemServices * pIWbemServices ) 
//
//==============================================================================
void DiskPerfDetails( IWbemServices * pIWbemServices ) 
{
	HRESULT		hr;
	long		lLower, lUpper, lCount; 
	SAFEARRAY	*psaNames = NULL;
	BSTR		PropName  = NULL;
	VARIANT		vVal;
	ULONG		uReturned;

	IEnumWbemClassObject	*pEnum      = NULL;
	IWbemClassObject		*pPerfData  = NULL;
	IWbemClassObject		*pPerfInst  = NULL;
	IWbemQualifierSet		*pQualSet   = NULL;
	VariantInit( &vVal );
	// Alloc class name string for DiskPerf
	BSTR PerfClass = SysAllocString( L"MSDiskDriver_PerformanceData" );
	if (!PerfClass)
	{
		printf("Out of memory\n");
		return;
	}

	// Get the Descriptions for the WMIDataId properties in the Embedded class.
	// Here the object info for the Class - MSDiskDriver_PerformanceData is
	// retrieved and displayed.
	// Collect object information for PerfClass
    hr = pIWbemServices->GetObject( PerfClass,
	                                  0L,
	                                  NULL,
	                                  &pPerfData,
                                      NULL );
    SysFreeString( PerfClass );

    PerfClass = NULL;
	if ( hr == WBEM_NO_ERROR )
	{
		// show the property description list header
		printf( PropListHeader );

		// Load up a safearray of property names
		if ( ( pPerfData->GetNames( NULL,
		                            WBEM_FLAG_ALWAYS | WBEM_FLAG_NONSYSTEM_ONLY,
		                            NULL, 
		                            &psaNames ) ) == WBEM_NO_ERROR )
		{
			// Get the upper and lower bounds of the Names array
			if ( ( hr = SafeArrayGetLBound( psaNames, 1, &lLower ) ) == S_OK ) 
			{
				hr = SafeArrayGetUBound( psaNames, 1, &lUpper );
			}

			if ( hr != S_OK ) 
			{
				printf( "Problem with property name array.\n" );
			}
			else
			{
				BSTR WmiQual  = SysAllocString( L"WmiDataId" );
				if (!WmiQual)
				{
					pPerfData->Release( );
					SafeArrayDestroy( psaNames );
					printf("Out of memory\n");
					return;
				}
				BSTR DescQual = SysAllocString( L"Description" );
				if (!DescQual)
				{
					pPerfData->Release( );
					SafeArrayDestroy( psaNames );
					SysFreeString(WmiQual);
					printf("Out of memory\n");
					return;
				}
				UINT uWmiId;

				for ( lCount = lLower; lCount <= lUpper; lCount++ ) 
				{
					// get the property name for this element
					if ( ( SafeArrayGetElement( psaNames, 
					                            &lCount, 
					                            &PropName)) == S_OK )
					{
						if ( ( pPerfData->GetPropertyQualifierSet( PropName, &pQualSet ) ) == WBEM_NO_ERROR ) 
						{
							// check to see if the property is a WMI data Item and save its description
							// these are the DiskPerf counters
							if ( ( pQualSet->Get( WmiQual, 0L, &vVal, NULL ) ) == WBEM_NO_ERROR )
							{
								uWmiId = vVal.lVal;

								VariantClear( &vVal );
								if ( ( pQualSet->Get( DescQual, 0L, &vVal, NULL ) ) == WBEM_NO_ERROR )
								{
									wprintf( L"( %d )\t\t%s\t%s\n", uWmiId, PropName, vVal.bstrVal );
									VariantClear( &vVal );
								}
								else // No Description qualifier available
									wprintf( L"( %d )\t\t%s\n", uWmiId, PropName);

							}

							pQualSet->Release( ); 
							pQualSet = NULL;
						}
						SysFreeString( PropName );
                        			PropName  = NULL;
					}
				}
				SysFreeString( WmiQual );
				SysFreeString( DescQual );
			}
			SafeArrayDestroy( psaNames );
		}
		pPerfData->Release( );
	}


	// Now that the data class info is displayed, go get the values for all the
	// disk instances
	PerfClass = SysAllocString( L"MSDiskDriver_Performance" );
	if (!PerfClass)
	{
		printf("Out of memory\n");
		return;
	}

	// Create enumerator for all disk instances
	hr = pIWbemServices->CreateInstanceEnum( PerfClass,
	                                         WBEM_FLAG_SHALLOW,
	                                         NULL,
	                                         &pEnum );
	SysFreeString( PerfClass );
	PerfClass = NULL;

	if ( hr == WBEM_NO_ERROR )
	{
        while ( pEnum->Next( INFINITE,
		                     1,
                             &pPerfInst,
                             &uReturned ) == WBEM_NO_ERROR )
		{
			// Explicitly get the properties of InstanceName and Active state

			// Get the Instance Name string
			PropName = L"InstanceName";
			if ( ( pPerfInst->Get( PropName, 
			                       0L, 
			                       &vVal, 
			                       NULL, NULL ) ) == WBEM_NO_ERROR ) 
			{
				wprintf( L"\n%s\n", vVal.bstrVal );
				VariantClear( &vVal );
			}

			// Get Active status boolean

			PropName = L"Active";
			if ( ( pPerfInst->Get( PropName, 
			                       0L, 
			                       &vVal, 
			                       NULL, NULL ) ) == WBEM_NO_ERROR ) 
			{
				wprintf( L"\t%s\t\t= %s\n", PropName, vVal.boolVal ? L"TRUE" : L"FALSE" );
				VariantClear( &vVal );
			}

			// Get the performance data embedded object
			PropName = L"PerfData" ;
			if ( ( pPerfInst->Get( PropName, 
			                       0L, 
			                       &vVal, 
			                       NULL, NULL ) ) == WBEM_NO_ERROR ) 
			{
				if ( vVal.punkVal->QueryInterface( IID_IWbemClassObject,
				                                  (PVOID *) &pPerfData ) == S_OK )
				{
					// display the WMI data item from this instance
					ShowPropInfo( pPerfData );
					pPerfData->Release( );
				}
				VariantClear( &vVal );
			}

			pPerfInst->Release( );
		}
		pEnum->Release( );
	}
	else
	{
		printf( "Can't enumerate DiskPerf instances!\n" );
	}
}


// Display property names and values for an instance.
void ShowPropInfo( IWbemClassObject *pInst )
{
	HRESULT hr;
	VARIANT vVal;
	BSTR PropName;

	if ( pInst )
	{
		hr = pInst->BeginEnumeration( WBEM_FLAG_LOCAL_ONLY );
		while ( hr == WBEM_NO_ERROR )
		{
			if ( ( hr = pInst->Next( 0, &PropName,
				                       &vVal, 
				                       NULL, NULL ) ) == WBEM_NO_ERROR ) 
			{
				switch( vVal.vt )
				{
					case VT_I4:
					{
						wprintf( L"\t%s\t= %d\n", PropName, vVal.lVal );
					}
					break;
					case VT_BSTR:
					{
						wprintf( L"\t%s\t= %s\n", PropName, vVal.bstrVal );
					}
					break;
					default:
					{
						wprintf( L"\t%s\t= NULL\n", PropName );
					}
					break;
				}
				VariantClear( &vVal );
				SysFreeString( PropName );
			}
		}
		pInst->EndEnumeration(  );
	}
}