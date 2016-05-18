#ifndef __WIA_DATATRANSFER_SAMPLE
//==========================================================================
//
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//
//--------------------------------------------------------------------------

//------------------------------------------------------------
//Please read the ReadME.txt which explains the purpose of the
//sample.
//-------------------------------------------------------------
#define __WIA_DATATRANSFER_SAMPLE

#include "utils.h"

#define FOLDER_TRANSFER TRUE
#define FILE_TRANSFER FALSE
#define MAX_TEMP_PATH 1024
#define MAX_FILENAME_LENGTH 1024
#define CHILDITEM_NAME TEXT("Storage.BMP")
#define UPLOAD_FILENAME TEXT("Upload.BMP")

class CWiaTransferCallback : public IWiaTransferCallback
{
private:
    
    ULONG m_cRef;                        //for reference counting
    long  m_lPageCount;                  //page counting for feeder item
    BSTR  m_bstrFileExtension;           //file extension to be appended to the download file 
    BOOL  m_bFeederTransfer;             //flag indicating whether download is from feeder item
    BSTR  m_bstrDirectoryName;           //download directory 
    TCHAR m_szFileName[MAX_FILENAME_LENGTH];   //download file

public:
    // Constructor and destructor
    CWiaTransferCallback();
    virtual ~CWiaTransferCallback();

    // To Initialize the download directory , file extension and bFeederTransfer(which indicates whether download is from feeder item) 
    HRESULT InitializeCallback(TCHAR* bstrDirectoryName, BSTR bstrExt, BOOL bFeederTransfer);

    // IUnknown functions
    HRESULT CALLBACK QueryInterface( REFIID riid, void **ppvObject );
    ULONG CALLBACK AddRef();
    ULONG CALLBACK Release();
    
    // IWiaTransferCallback functions
    HRESULT STDMETHODCALLTYPE TransferCallback(LONG lFlags,WiaTransferParams  *pWiaTransferParams);
    HRESULT STDMETHODCALLTYPE GetNextStream(LONG lFlags, BSTR bstrItemName, BSTR bstrFullItemName, IStream **ppDestination);

};

HRESULT TransferWiaItem( IWiaItem2 *pIWiaItem2 );

HRESULT UploadToDeviceStorage(IWiaItem2* pWiaParentItem2);

HRESULT DownloadItem(IWiaItem2* pWiaItem2, BOOL bFolderTransfer);

HRESULT EnumerateAndTransferItems( IWiaItem2 *pIWiaItem2Root );

HRESULT EnumerateWiaDevices( IWiaDevMgr2 *pWiaDevMgr2 );

#endif

