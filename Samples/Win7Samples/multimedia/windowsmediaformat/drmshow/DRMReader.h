//*****************************************************************************
//
// Microsoft Windows Media
// Copyright (C) Microsoft Corporation. All rights reserved.
//
// FileName:            DRMReader.h
//
// Abstract:            Header file for the CLicenseViewer class, which
//                      does all the work for printing the DRM-related
//                      properties of the file.
//
//*****************************************************************************

class CLicenseViewer: public IWMReaderCallback
{
public:

    CLicenseViewer();
    ~CLicenseViewer();

	//
	// Methods of IUnknown
	//
	HRESULT STDMETHODCALLTYPE QueryInterface( REFIID riid, void **ppvObject );
	ULONG STDMETHODCALLTYPE AddRef( void );
	ULONG STDMETHODCALLTYPE Release( void );

	//
	// Methods of IWMReaderCallback
	//
	HRESULT STDMETHODCALLTYPE OnSample( DWORD dwOutputNum, QWORD cnsSampleTime, QWORD cnsSampleDuration, DWORD dwFlags, INSSBuffer *pSample, void *pvContext);
	HRESULT STDMETHODCALLTYPE OnStatus( WMT_STATUS Status, HRESULT hr, WMT_ATTR_DATATYPE dwType, BYTE *pValue, void *pvContext);

    //
    // Opens the file from which to get the rights
    //
    HRESULT Open( __in LPWSTR pwszInFile );
    
    //
    // Prints the rights to the console
    //
    HRESULT ShowRights();
    
    //
    // Releases all member variables
    //
    HRESULT Cleanup();
    
    //
    // Closes the currently opened file
    //
    HRESULT Close();

protected:
    //
    // Initializes the class's internal state, creating the reader and callback event
    //
    HRESULT STDMETHODCALLTYPE Initialize();
    
    //
    // Prints the contents of a DRM_LICENSE_STATE_DATA structure to the console window
    //
    HRESULT STDMETHODCALLTYPE PrintLicenseStateData( LPCTSTR tszOutputPrefix, DRM_LICENSE_STATE_DATA* pDRMLicenseStateData );
    
    IWMReader*              m_pIWMReader;
    IWMDRMReader*           m_pIWMDRMReader;
    HANDLE                  m_hEvent;
    HRESULT                 m_hr;
    LONG                    m_cRef;
};
