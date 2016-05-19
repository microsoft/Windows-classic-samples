// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved
//
// DeviceProp.cpp : implementation file
//

#include "stdafx.h"
#include "genericucp.h"
#include "DeviceProp.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define DATA_BUFSIZE 2048
/////////////////////////////////////////////////////////////////////////////
// DeviceProp dialog


CDeviceProp::CDeviceProp(IUPnPDevice* pDevice, CWnd* pParent)
: CDialog(CDeviceProp::IDD, pParent)
{
	//{{AFX_DATA_INIT(CDeviceProp)
	//}}AFX_DATA_INIT
    pCurrentDevice=pDevice;
	pCurrentDevice->AddRef();
}

CDeviceProp::~CDeviceProp(){

	pCurrentDevice->Release();
}

void CDeviceProp::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CDeviceProp)
	DDX_Control(pDX, IDC_OK, m_OkButton);
	DDX_Control(pDX, IDC_EDITUPC, m_UPC);
	DDX_Control(pDX, IDC_EDITUDN, m_UDN);
	DDX_Control(pDX, IDC_EDITSERIALNUMBER, m_SerialNumber);
	DDX_Control(pDX, IDC_EDITPRESENTATIONURL, m_PresentationUrl);
	DDX_Control(pDX, IDC_EDITMODELURL, m_ModelUrl);
	DDX_Control(pDX, IDC_EDITMODELNUMBER, m_ModelNumber);
	DDX_Control(pDX, IDC_EDITMODELNAME, m_ModelName);
	DDX_Control(pDX, IDC_EDITMODELDESCRIPTION, m_ModelDescription);
	DDX_Control(pDX, IDC_EDITMANUFACTURERURL, m_ManufacturerUrl);
	DDX_Control(pDX, IDC_EDITMANUFACTURER, m_Manufacturer);
	DDX_Control(pDX, IDC_EDITFRIENDLYNAME, m_FriendlyName);
	DDX_Control(pDX, IDC_EDITDEVICETYPE, m_DeviceType);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CDeviceProp, CDialog)
	//{{AFX_MSG_MAP(CDeviceProp)
	ON_BN_CLICKED(IDC_OK, OnOk)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// DeviceProp message handlers

void CDeviceProp::OnOk() 
{
	// TODO: Add your control notification handler code here
	EndDialog(1);
	return;
}

//+---------------------------------------------------------------------------
//
//  Member:		OnInitDialog
//
//  Purpose:    Initialization of the dialog box. 
//
//  Arguments:
//				None
//
//  Returns:    TRUE
//
//  Notes:
//				
//



BOOL CDeviceProp::OnInitDialog()
{
    CDialog::OnInitDialog();

	HRESULT hr=S_OK;
	BSTR  bstrTemp = NULL;

	// Print all the properties to the respective edit boxes. 

        hr=pCurrentDevice->get_UniqueDeviceName(&bstrTemp);
        if (S_OK == hr)
        {
            PrintTextToEditBox(&m_UDN, bstrTemp, hr);
            SysFreeString(bstrTemp);
        }
	
	hr=pCurrentDevice->get_FriendlyName(&bstrTemp);
	if (S_OK == hr)
	{
    	    PrintTextToEditBox(&m_FriendlyName, bstrTemp, hr);
	    SysFreeString(bstrTemp);
        }	    
		
	hr=pCurrentDevice->get_Type(&bstrTemp);
	if (S_OK == hr)
	{
    	    PrintTextToEditBox(&m_DeviceType, bstrTemp, hr);
	    SysFreeString(bstrTemp);
        }	    

	hr=pCurrentDevice->get_PresentationURL(&bstrTemp);
	if (S_OK == hr)
	{
            PrintTextToEditBox(&m_PresentationUrl, bstrTemp, hr);
            SysFreeString(bstrTemp);
        }		
        
	hr=pCurrentDevice->get_ManufacturerName(&bstrTemp);
	if (S_OK == hr)
	{
            PrintTextToEditBox(&m_Manufacturer, bstrTemp, hr);
            SysFreeString(bstrTemp);
        }            
		
	hr=pCurrentDevice->get_ManufacturerURL(&bstrTemp);
	if (S_OK == hr)
	{
            PrintTextToEditBox(&m_ManufacturerUrl, bstrTemp, hr);
            SysFreeString(bstrTemp);
        }            
		
	hr=pCurrentDevice->get_ModelName(&bstrTemp);
	if (S_OK == hr)
	{
            PrintTextToEditBox(&m_ModelName, bstrTemp, hr);
            SysFreeString(bstrTemp);
        }            
			
	hr=pCurrentDevice->get_ModelNumber(&bstrTemp);
	if (S_OK == hr)
	{
            PrintTextToEditBox(&m_ModelNumber, bstrTemp, hr);
            SysFreeString(bstrTemp);
        }            
		
	hr=pCurrentDevice->get_Description(&bstrTemp);
	if (S_OK == hr)
	{
            PrintTextToEditBox(&m_ModelDescription, bstrTemp, hr);
            SysFreeString(bstrTemp);
        }            
	
	
	hr=pCurrentDevice->get_ModelURL(&bstrTemp);
	if (S_OK == hr)
	{
            PrintTextToEditBox(&m_ModelUrl, bstrTemp, hr);
            SysFreeString(bstrTemp);
        }            
	
	
	hr=pCurrentDevice->get_UPC(&bstrTemp);
	if (S_OK == hr)
	{
            PrintTextToEditBox(&m_UPC, bstrTemp, hr);
            SysFreeString(bstrTemp);
        }            
	
	
	hr=pCurrentDevice->get_SerialNumber(&bstrTemp);
	if (S_OK == hr)
	{
            PrintTextToEditBox(&m_SerialNumber, bstrTemp, hr);
            SysFreeString(bstrTemp);
        }            
	
    return TRUE;
}

//+---------------------------------------------------------------------------
//
//  Member:		PrintTextToEditBox
//
//  Purpose:    Helper function to print to appropriate text box
//
//  Arguments:
//				None
//
//  Returns:    
//				None
//
//  Notes:
//				
//

void CDeviceProp::PrintTextToEditBox(CEdit *pEditBox, BSTR bstrPrintText, HRESULT hrCheck)
{
	TCHAR tszPropertyString[DATA_BUFSIZE];
	if( hrCheck != S_OK ){
		PrintErrorText(hrCheck);
	}
	else{
		_sntprintf_s(tszPropertyString, DATA_BUFSIZE, _TRUNCATE, _T("%S"), bstrPrintText);
		(*pEditBox).SetWindowText(tszPropertyString);
	}

}
