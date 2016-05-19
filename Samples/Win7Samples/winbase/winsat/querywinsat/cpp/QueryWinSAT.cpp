//-----------------------------------------------------------------------
// This file is part of the Windows SDK Code Samples.
// 
// Copyright (C) Microsoft Corporation.  All rights reserved.
// 
// This source code is intended only as a supplement to Microsoft
// Development Tools and/or on-line documentation.  See these other
// materials for detailed information regarding Microsoft code samples.
// 
// THIS CODE AND INFORMATION ARE PROVIDED AS IS WITHOUT WARRANTY OF ANY
// KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//-----------------------------------------------------------------------

#pragma once

#define WIN32_LEAN_AND_MEAN		// Exclude rarely-used stuff from Windows headers
#include <stdio.h>
#include <tchar.h>

#include <atlbase.h>
#include <atlcom.h>

#include "WinSATCOMInterfaceI.h"

HRESULT CreateQueryAssessment( IQueryRecentWinSATAssessment **querywinsat,
                               IQueryAllWinSATAssessments **queryallwinsat)
{
	HRESULT hr;

    hr = CoCreateInstance( __uuidof(CQueryWinSAT), 
                           NULL, 
                           CLSCTX_INPROC_SERVER, 
                           __uuidof(IQueryRecentWinSATAssessment), 
                           (void **)querywinsat);
    if (FAILED(hr))
    {
        return hr;
    }       

    hr = CoCreateInstance( __uuidof(CQueryAllWinSAT), 
                           NULL, 
                           CLSCTX_INPROC_SERVER, 
                           __uuidof(IQueryAllWinSATAssessments), 
                           (void **)queryallwinsat);

    if (FAILED(hr)) 
    {      
        return hr;
    }

	return S_OK;
}


//
// a helper method to dump a node list to the console
//
HRESULT DumpNode( IXMLDOMNode *node )
{
    CComBSTR bstr;
    HRESULT hr;

    if (FAILED(hr = node->get_xml( &bstr )))
    {		
        wprintf(L"Failed to dump XML Node, error is %08.8lx", hr);
        return hr;
    }   
    wprintf (L"%ws\n", (LPCWSTR)bstr );
    return S_OK;
}


//
// a helper method to dump a node list to the console
//
HRESULT DumpNodeList(IXMLDOMNodeList *domNodeList)
{  
    CComPtr<IXMLDOMNode> nodeCur;
    LONG listLength;
    HRESULT hr;   

    if (domNodeList == NULL) 
    {
        return E_INVALIDARG;
    }

    if (FAILED(hr = domNodeList->get_length( &listLength ))) 
    {
        wprintf(L"Failed to get length from node list, error is %08.8lx", hr);
        return hr;
    }

    if (FAILED(hr = domNodeList->reset()))
    {
        wprintf(L"Failed to reset node list, error is %08.8lx", hr);
        return hr;
    }

    for (int i = 0; i < listLength; i++) 
    {
        if (FAILED(hr = domNodeList->nextNode(&nodeCur))) 
        {
            wprintf (L"Failed to get next node in list, error is %08.8lx", hr);
            return hr;
        }
        if (FAILED(hr = DumpNode(nodeCur))) 
        {
            wprintf (L"Failed to dump node, error is %08.8lx", hr);
            return hr;
        }
        wprintf (L"\n");
        nodeCur.Release();
    }

    return S_OK;
}


//
// A class to initialize COM.  The destructor guarantees that COM
// is cleaned up on all error paths.
//
class InitializeCOM
{
private:
   HRESULT  hr;
   bool bInitSucceeded;
public:
   InitializeCOM()
   {
       if (FAILED(hr = CoInitialize(NULL)))
       {
            wprintf(L"Failed to initialize COM, error is %08.8lx", hr);
            bInitSucceeded = false;
       }
       else
       {
            bInitSucceeded = true;
       }
   }

   ~InitializeCOM()
   {
       if (bInitSucceeded) 
       {
            CoUninitialize();
       }
   }

   bool Succeeded()
   {
       return bInitSucceeded;
   }

};

//
//  A routine to write out the information for a specific assessment
//
HRESULT WriteAssessmentInfo(WINSAT_ASSESSMENT_TYPE type,
                     IProvideWinSATResultsInfo* pWinsatInfo)
{  
    CComPtr<IProvideWinSATAssessmentInfo> winsatAssessmentInfo;
    float    score;
    CComBSTR title;
    CComBSTR description;

    HRESULT hr;

    if (FAILED(hr = pWinsatInfo->GetAssessmentInfo( 
                                                   type, 
                                                   &winsatAssessmentInfo)))
    {
        wprintf(L"Failed to get Assessment for %i, error is %08.8lx", 
             (int)type, hr);
        return hr;
    }    

    if (FAILED(hr = winsatAssessmentInfo->get_Score(&score)))
    {
        wprintf( L"Failed to get score, error is %08.8lx", hr);    
        return hr;
    }

    if (FAILED(hr = winsatAssessmentInfo->get_Title(&title)))
    {
        wprintf( L"Failed to get title, error is %08.8lx", hr);    
        return hr;
    }

    if (FAILED(hr = winsatAssessmentInfo->get_Description(&description)))
    {
        wprintf( L"Failed to get title, error is %08.8lx", hr);    
        return hr;
    }    

    wprintf (L"%s %s = %f\n", 
        (LPWSTR)description.m_str, 
        (LPWSTR)title.m_str, 
        score );
    return S_OK;
}


//
//  A routine to write out all information from the most recent assessment
//
HRESULT QueryWinsat(IQueryRecentWinSATAssessment *queryWinsat)
{
    HRESULT hr;  
    CComPtr<IProvideWinSATResultsInfo> winsatInfo;
    if (FAILED(hr = queryWinsat->get_Info( &winsatInfo )))
    {
        wprintf(L"Failed to get winsat info, error is %08.8lx", hr);    
        return hr;
    }

    //
    // Write out the state of Winsat
    //
    WINSAT_ASSESSMENT_STATE state;
    if (FAILED(hr = winsatInfo->get_AssessmentState( &state )))
    {
        wprintf(
            L"Failed to get rating state description, error is %08.8lx", hr);    
        return hr;
    }  
    wprintf (L"State value: %i\n", (int)state);

    //
    // Write out the description of the state
    //
    CComBSTR ratingStateDesc;
    hr = winsatInfo->get_RatingStateDesc( &ratingStateDesc);
    if (FAILED(hr)) 
    {
        wprintf(
            L"Failed ot get rating state description, error is %08.8lx", hr);
        return hr;
    }
    wprintf (L"Description: %ws\n", (LPWSTR)ratingStateDesc.m_str);

    //
    // Write out the rating
    //
    float rating;
    hr = winsatInfo->get_SystemRating( &rating);
    if (FAILED(hr)) 
    {
        wprintf(L"Failed to get rating, error is %08.8lx", hr);
        return hr;
    }
    wprintf(L"Rating: %f\n",rating);

    //
    // Write out the date of the assessment
    //
    VARIANT varDate;
    hr = winsatInfo->get_AssessmentDateTime( &varDate );
    if (FAILED(hr)) 
    {
        wprintf(L"Failed to get date/time, error is %08.8lx", hr);
        return hr;
    }

    SYSTEMTIME systemtime;
    int result = VariantTimeToSystemTime(varDate.date, &systemtime );
    if (result == 0) 
    {
        wprintf(L"Invalid date/time returned from API, error is %08.8lx", hr);
    } 
    else 
    {
        wprintf(L"%i/%i/%i  %i:%i:%i and %i ms\n",
            (int)systemtime.wMonth,
            (int)systemtime.wDay,
            (int)systemtime.wYear,
            (int)systemtime.wHour,
            (int)systemtime.wMinute,
            (int)systemtime.wSecond,
            (int)systemtime.wMilliseconds );
    }        
    

    //
    // Write out the details of the memory assessment   
    //
    if (FAILED(hr = WriteAssessmentInfo(WINSAT_ASSESSMENT_MEMORY, 
                                        winsatInfo)))
    {      
      return hr;
    }    
    
    //
    // Write out the details of the CPU assessment   
    //
    if (FAILED(hr = WriteAssessmentInfo(WINSAT_ASSESSMENT_CPU, 
                                        winsatInfo)))
    {      
      return hr;
    }    
    //
    // Write out the details of the DISK assessment   
    //
    if (FAILED(hr = WriteAssessmentInfo(WINSAT_ASSESSMENT_DISK, 
                                        winsatInfo)))
    {      
      return hr;
    }    
    //
    // Write out the details of the D3D assessment   
    //
    if (FAILED(hr = WriteAssessmentInfo(WINSAT_ASSESSMENT_D3D, 
                                        winsatInfo)))
    {      
      return hr;
    }    
    //
    // Write out the details of the DWM assessment   
    //
    if (FAILED(hr = WriteAssessmentInfo(WINSAT_ASSESSMENT_GRAPHICS, 
                                        winsatInfo)))
    {      
      return hr;
    }    
    return S_OK;  
}

HRESULT QueryXML( IQueryRecentWinSATAssessment *queryWinsat, LPCWSTR xpath )
{
    HRESULT hr;
    CComBSTR bstrXpath(xpath);
    CComBSTR bstrNamespaces(L"");
    CComPtr<IXMLDOMNodeList> resultList;
    if (FAILED( hr = queryWinsat->get_XML( bstrXpath,
                                           bstrNamespaces,
                                           &resultList ) )) 
    {
         wprintf(L"Unable to get XML, error is %08.8lx", hr);
         return hr;
    }
    
    if (FAILED( hr = DumpNodeList (resultList) ) ) 
    {
         wprintf(L"Unable to dump node list, error is %08.8lx", hr);
         return hr;
    }
    return S_OK;
}

HRESULT QueryAllXML( IQueryAllWinSATAssessments *queryAllWinsat, LPCWSTR xpath )
{
    HRESULT hr;
    CComBSTR bstrXpath(xpath);
    CComBSTR bstrNamespaces(L"");
    CComPtr<IXMLDOMNodeList> resultList;
    if (FAILED( hr = queryAllWinsat->get_AllXML( bstrXpath,
                                                 bstrNamespaces,
                                                 &resultList ) )) 
    {
         wprintf(L"Unable to get XML, error is %08.8lx", hr);
         return hr;
    }
    
    if (FAILED( hr = DumpNodeList (resultList) ) ) 
    {
         wprintf(L"Unable to dump node list, error is %08.8lx", hr);
         return hr;
    }
    return S_OK;
}

//
// Main function
//
int _tmain(int argc, TCHAR* argv[])
{
    InitializeCOM initializeCOM;
    CComPtr<IQueryRecentWinSATAssessment> pQueryRecentWinSAT;
    CComPtr<IQueryAllWinSATAssessments> pQueryAllWinSAT;	
    LPCWSTR pStrCommand = NULL;
    LPCWSTR pStrXPath = NULL;
    HRESULT hr;     

    // check for arguments
    if (argc == 2)
    {
        pStrCommand = argv[1];
        pStrXPath = NULL;        
    }
    else if (argc == 3)
    {
        pStrCommand = argv[1];
        pStrXPath = argv[2];        
    } 
    else 
    {
        wprintf(
L"Usage: QueryWinSAT.exe (query | queryxml | queryallxml [xpath]) \n");
        return 0;
    }    

    if (!initializeCOM.Succeeded()) 
    {
        return -1;
    }
    
	if (FAILED(hr = CreateQueryAssessment(&pQueryRecentWinSAT,
                                          &pQueryAllWinSAT)))
	{
		wprintf(L"Error creating Winsat Query objects, error is %08.8lx", hr);
		return -1;
	}

    if (_wcsicmp(L"query", pStrCommand) == 0 )
    {
        if (FAILED(hr = QueryWinsat(pQueryRecentWinSAT)))
	    {
		    wprintf(L"Error querying Winsat, error is %08.8lx", hr);
		    return -1;
	    }
    }
    else if (_wcsicmp(L"queryallxml", pStrCommand) == 0 )
    {
        if (FAILED(hr = QueryAllXML(pQueryAllWinSAT, pStrXPath))) 
        {
            wprintf(L"Failed to query WinSAT XML, error is %08.8lx", hr);
		    return -1;
        }
    }
    else if (_wcsicmp(L"queryxml", pStrCommand) == 0 )
    {
        if (FAILED(hr = QueryXML(pQueryRecentWinSAT, pStrXPath))) 
        {
            wprintf(L"Failed to query WinSAT XML, error is %08.8lx", hr);
		    return -1;
        }
    }
    else
    {
        wprintf(L"Unrecognized command %ws", pStrCommand);
    }
    
    return 0;
}
