#include "DmoEnum.h"
#include "MainDialog.h"
#include "NamedGuid.h"


//
//  Global data structure for storing GUIDs and friendly strings
//
typedef struct _dmo_category_info
{
    const GUID *pclsid;
    TCHAR szName[128];

} DMO_CATEGORY_INFO;


// The DirectShow reference documentation lists a set of DMO categories
// for which you can enumerate corresponding filters.
const DMO_CATEGORY_INFO dmo_categories[] = {
    
    &GUID_NULL,                  TEXT("All categories\0"),
    &DMOCATEGORY_AUDIO_DECODER,  TEXT("Audio Decoder\0"),
    &DMOCATEGORY_AUDIO_EFFECT,   TEXT("Audio Effect\0"),
    &DMOCATEGORY_AUDIO_ENCODER,  TEXT("Audio Encoder\0"),
    &DMOCATEGORY_VIDEO_DECODER,  TEXT("Video Decoder\0"),
    &DMOCATEGORY_VIDEO_EFFECT,   TEXT("Video Effect\0"),
    &DMOCATEGORY_VIDEO_ENCODER,  TEXT("Video Encoder\0"),
    &DMOCATEGORY_AUDIO_CAPTURE_EFFECT, TEXT("Audio Capture Effect\0"),
};

//
// Constants
//
#define NUM_CATEGORIES  (sizeof(dmo_categories) / sizeof(DMO_CATEGORY_INFO))

/// MainDialog

MainDialog::MainDialog() 
  : CBaseDialog(IDD_DMOENUM_DIALOG),
    m_pDMO(NULL)
{
}

MainDialog::~MainDialog()
{
	SAFE_RELEASE(m_pDMO);
}


HRESULT MainDialog::OnInitDialog()
{
	InitializeControls();

    // Include keyed DMOs in our search by default
    m_bCheckKeyed.SetCheck(TRUE);

    // Enumerate and display the DMO category list
    FillCategoryList();

	return S_OK;
}

INT_PTR MainDialog::OnCommand(HWND hControl, WORD idControl, WORD msg)
{
	switch (idControl)
	{
	case IDC_LIST_DEVICES:
		if (msg == LBN_SELCHANGE)
		{
			OnSelchangeListDevices();
		}
		break;

	case IDC_LIST_FILTERS:
		if (msg == LBN_SELCHANGE)
		{
			OnSelchangeListFilters();
		}
		break;

	case IDC_CHECK_KEYED:
		if (msg == BN_CLICKED)
		{
			OnCheckKeyed();
		}
		break;

	case IDC_LIST_INPUT_STREAMS:
		if (msg == LBN_SELCHANGE)
		{
			OnSelchangeListInputStreams();
		}
		break;

	case IDC_LIST_OUTPUT_STREAMS:
		if (msg == LBN_SELCHANGE)
		{
			OnSelchangeListOutputStreams();
		}
		break;
	}

	return 1;
}



///////////////////////////////////////////////////////////////////////
// Name: InitializeControls
// Desc: Associates the control wrapper classes with the 
//       correct windows for the control.
///////////////////////////////////////////////////////////////////////

void MainDialog::InitializeControls()
{

	m_CheckOutQC.SetWindow(GetDlgItem(IDC_CHECK_OUT_SUPPORTS_QC));
	m_CheckInQC.SetWindow(GetDlgItem(IDC_CHECK_IN_SUPPORTS_QC));
	m_CheckOutFixedSize.SetWindow(GetDlgItem(IDC_CHECK_OUT_FIXED));
	m_CheckOutWholeSamples.SetWindow(GetDlgItem(IDC_CHECK_OUT_WHOLE_SAMPLES));
	m_CheckOutOptional.SetWindow(GetDlgItem(IDC_CHECK_OUT_OPTIONAL));
	m_CheckOutOneSample.SetWindow(GetDlgItem(IDC_CHECK_OUT_ONESAMPLE));
	m_CheckOutDiscardable.SetWindow(GetDlgItem(IDC_CHECK_OUT_DISCARDABLE));
	m_CheckInWholeSamples.SetWindow(GetDlgItem(IDC_CHECK_IN_WHOLE_SAMPLES));
	m_CheckInOneSample.SetWindow(GetDlgItem(IDC_CHECK_IN_ONESAMPLE));
	m_CheckInHoldsBuffers.SetWindow(GetDlgItem(IDC_CHECK_IN_HOLDSBUFFERS));
	m_CheckInFixedSize.SetWindow(GetDlgItem(IDC_CHECK_IN_FIXED));
	m_ListOutputStreams.SetWindow(GetDlgItem(IDC_LIST_OUTPUT_STREAMS));
	m_ListInputStreams.SetWindow(GetDlgItem(IDC_LIST_INPUT_STREAMS));
	m_ListOutputTypes.SetWindow(GetDlgItem(IDC_LIST_OUTPUT_TYPES));
	m_ListInputTypes.SetWindow(GetDlgItem(IDC_LIST_INPUT_TYPES));
	m_bCheckKeyed.SetWindow(GetDlgItem(IDC_CHECK_KEYED));
	m_FilterList.SetWindow(GetDlgItem(IDC_LIST_FILTERS));
	m_DeviceList.SetWindow(GetDlgItem(IDC_LIST_DEVICES));
}


void MainDialog::FillCategoryList()
{
    // Clear listboxes
    ClearDeviceList();
    ClearFilterList();

    // Fill the category list box with the categories to display,
    // using the names stored in the DMO_CATEGORY_INFO array.
    // See DMOEnumDlg.H for a category description.
    for (int i=0; i < NUM_CATEGORIES; i++)
	{
        m_DeviceList.AddString(dmo_categories[i].szName);
	}

    // Update listbox title with number of classes
    SetNumClasses(NUM_CATEGORIES);

    // Select the first category to show useful information
    m_DeviceList.Select(0);
    OnSelchangeListDevices();
}


void MainDialog::ClearDeviceList(void)
{
    // Clean up
    m_DeviceList.ClearItems();
    SetNumClasses(0);
}

void MainDialog::ClearFilterList(void)
{
    CLSID *pStoredId = NULL;
    
    UINT nCount = m_FilterList.Count();

    // Delete any CLSID pointers that were stored in the listbox item data
    for (UINT i=0; i < nCount; i++)
    {
        pStoredId = (CLSID *) m_FilterList.GetItem(i);
        if (pStoredId != 0)
        {
            delete pStoredId;
            pStoredId = NULL;
        }
    }

    // Clean up
    m_FilterList.ClearItems();
    SetNumFilters(0);
	
	SetDlgItemText(IDC_STATIC_FILENAME, TEXT("<No DMO selected>"));
}


void MainDialog::SetNumClasses(int nClasses)
{
    TCHAR szClasses[64];

	HRESULT hr = StringCchPrintf(szClasses, 64, TEXT("DMO Categories (%d found)\0"), nClasses);
	if (SUCCEEDED(hr))
	{
		SetDlgItemText(IDC_STATIC_CLASSES, szClasses);
	}
}


void MainDialog::SetNumFilters(int nFilters)
{
    TCHAR szFilters[64];

	HRESULT hr = StringCchPrintf(szFilters, 64, TEXT("Registered DMOs (%d found)\0"), nFilters);
	if (SUCCEEDED(hr))
	{
		SetDlgItemText(IDC_STATIC_FILTERS, szFilters);
	}
}


void MainDialog::OnSelchangeListDevices() 
{
    HRESULT hr;    
    IEnumDMO *pEnum = NULL;
    const GUID *clsid;

	DWORD dwFlags = m_bCheckKeyed.IsChecked() ? DMO_ENUMF_INCLUDE_KEYED : 0;

    // Get the currently selected category name
    UINT nItem;
	m_DeviceList.GetCurrentSelection(&nItem);
	
    // Read the CLSID pointer from our hard-coded array of
    // documented filter categories
    clsid = dmo_categories[nItem].pclsid;

    ClearTypeLists();
    ReleaseDMO();

    // Enumerate all DMOs of the selected category  
    hr = DMOEnum(*clsid, dwFlags, 0, NULL, 0, NULL, &pEnum);
	
    // Enumerate all filters using the new category enumerator
	if (SUCCEEDED(hr))
	{
	    hr = EnumDMOs(pEnum);
	}

    // Now that the DMOs (if any) are enumerated and added 
    // to the list, go ahead and select the first one.
	if (SUCCEEDED(hr))
	{
	    m_FilterList.Select(0);
		OnSelchangeListFilters();
	}

    SAFE_RELEASE(pEnum);
}

void MainDialog::OnCheckKeyed() 
{
    // Reenumerate the related filters, since the list may
    // have changed to add or removed keyed filters.
    OnSelchangeListDevices();
}


void MainDialog::ClearTypeLists()
{
    // Clear the type boxes
    m_ListInputTypes.ClearItems();
    m_ListOutputTypes.ClearItems();

	SetDlgItemText(IDC_STATIC_NUM_ITYPES, TEXT("0\0"));
	SetDlgItemText(IDC_STATIC_NUM_OTYPES, TEXT("0\0"));
}

void MainDialog::ReleaseDMO()
{
    SAFE_RELEASE(m_pDMO);

    // Clear any information set on the dialog for the last DMO
    ClearDMOInfo();
}


HRESULT MainDialog::EnumDMOs(IEnumDMO *pEnumCat)
{
    HRESULT hr=S_OK;
    int nFilters=0;
    WCHAR *wszName=0;
    CLSID clsid;

    // Clear the current filter list
    ClearFilterList();

    // If there are no filters of a requested type, show default string
    if (!pEnumCat)
    {
        m_FilterList.AddString(TEXT("<< No entries >>"));
        SetNumFilters(0);
        return S_OK;
    }

    // Enumerate all items associated with the moniker
    while (pEnumCat->Next(1, &clsid, &wszName, NULL) == S_OK)
    {
        nFilters++;

        // As a precaution, verify that the DMO has a name.  If not,
        // give it a temporary name so that it can be added to the list box
        // and still have an associated CLSID.

		size_t len;
		hr = StringCchLength(wszName, STRSAFE_MAX_CCH, &len);
		if (SUCCEEDED(hr))
		{
	        // Add this DMO's name and CLSID to the listbox
		    AddFilter(wszName, &clsid);
		}
		else
		{
		    AddFilter(TEXT("<< Invalid DMO name >>\0"), &clsid);
		}

        CoTaskMemFree(wszName);
        wszName = 0;
    }

    // If no DMOs matched the query, show a default item
    if (nFilters == 0)
	{
        m_FilterList.AddString(TEXT("<< No entries >>"));
	}

    // Update count of enumerated filters
    SetNumFilters(nFilters);
    return hr;
}



void MainDialog::OnSelchangeListFilters() 
{
	const size_t FILE_NAME_LEN = 512;

	const CLSID *clsid;

    // Get the currently selected category name
    UINT nItem;
	
	m_FilterList.GetCurrentSelection(&nItem);
	
    // Read the CLSID pointer from the list box's item data
    clsid = (CLSID *) m_FilterList.GetItem(nItem);

    // Find the filter filename in the registry (by CLSID)
    if (clsid != 0)
    {
        // Display the DMO's server filename
		TCHAR szFileName[FILE_NAME_LEN];

		HRESULT hr = GetFilenameByCLSID(clsid, szFileName, FILE_NAME_LEN);
		if (SUCCEEDED(hr))
		{
			SetDlgItemText(IDC_STATIC_FILENAME, szFileName);
		}
		else
		{
			SetDlgItemText(IDC_STATIC_FILENAME, TEXT("<Unknown>"));
		}

        // Show media types/subtypes used by this DMO
        AddTypeInfo(clsid);

        // Display DMO-specific information        
        ReleaseDMO();
        ShowSelectedDMOInfo(clsid);
    }
}



void MainDialog::ClearDMOInfo()
{

	const TCHAR *STR_UNKNOWN = TEXT("<?>");

    // Clear DMO-specific information
    m_ListInputStreams.ClearItems();
    m_ListOutputStreams.ClearItems();
	SetDlgItemText(IDC_STATIC_INPUT_STREAMS, TEXT("In Streams: 0\0"));
	SetDlgItemText(IDC_LIST_OUTPUT_STREAMS, TEXT("Out Streams: 0\0"));

    // Input settings
    m_CheckInWholeSamples.SetCheck(FALSE);
    m_CheckInOneSample.SetCheck(FALSE);
    m_CheckInFixedSize.SetCheck(FALSE);
    m_CheckInHoldsBuffers.SetCheck(FALSE);

	SetDlgItemText(IDC_STATIC_IN_FORMAT, STR_UNKNOWN);
	SetDlgItemText(IDC_STATIC_IN_TYPE, STR_UNKNOWN);
	SetDlgItemText(IDC_STATIC_IN_SUBTYPE, STR_UNKNOWN);
    m_CheckInQC.SetCheck(FALSE);

    // Output settings
    m_CheckOutWholeSamples.SetCheck(FALSE);
    m_CheckOutOneSample.SetCheck(FALSE);
    m_CheckOutFixedSize.SetCheck(FALSE);
    m_CheckOutDiscardable.SetCheck(FALSE);
    m_CheckOutOptional.SetCheck(FALSE);
	SetDlgItemText(IDC_STATIC_OUT_FORMAT, STR_UNKNOWN);
	SetDlgItemText(IDC_STATIC_OUT_TYPE, STR_UNKNOWN);
	SetDlgItemText(IDC_STATIC_OUT_SUBYTPE, STR_UNKNOWN);
    m_CheckOutQC.SetCheck(FALSE);
}


void MainDialog::AddFilter(
    const TCHAR *szFilterName,
    const GUID *pCatGuid)
{
    // Add the category name and a pointer to its CLSID to the list box

    // Allocate a new CLSID, whose pointer will be stored in 
    // the listbox.  When the listbox is cleared, these will be deleted.
    CLSID *pclsid = new CLSID;
    if (pclsid)
    {
        *pclsid = *pCatGuid;
		m_FilterList.AddItem(szFilterName, pclsid);
    }
}



//// HELPERS

void MainDialog::AddTypeInfo(const GUID *pCLSID)
{
    const int NUM_PAIRS=20;
    HRESULT hr;
    DMO_PARTIAL_MEDIATYPE aInputTypes[NUM_PAIRS]={0}, 
                          aOutputTypes[NUM_PAIRS]={0};
    ULONG ulNumInputsSupplied, ulNumOutputsSupplied;
    ULONG ulNumInputTypes=NUM_PAIRS, ulNumOutputTypes=NUM_PAIRS, i;
    TCHAR szCLSID[128];
    TCHAR szNum[10];

    // Clear the type/subtype GUID boxes
    ClearTypeLists();

    // Read type/subtype information
    hr = DMOGetTypes(
        *pCLSID,
        ulNumInputTypes,
        &ulNumInputsSupplied,
        aInputTypes,
        ulNumOutputTypes,
        &ulNumOutputsSupplied,
        aOutputTypes);

	// Display the number of input/output type/subtype pairs found
    if (SUCCEEDED(hr))
	{
		hr = StringCchPrintf(szNum, NUMELMS(szNum), TEXT("%d\0"), ulNumInputsSupplied);
	}

	if (SUCCEEDED(hr))
	{
		SetDlgItemText(IDC_STATIC_NUM_ITYPES, szNum);
	}

	if (SUCCEEDED(hr))
	{
		hr = StringCchPrintf(szNum, NUMELMS(szNum), TEXT("%d\0"), ulNumOutputsSupplied);
	}
   
	if (SUCCEEDED(hr))
	{
		SetDlgItemText(IDC_STATIC_NUM_OTYPES, szNum);
	}

	if (SUCCEEDED(hr))
	{
	    // Show input type/subtype pairs
		for (i=0; i<ulNumInputsSupplied; i++)
		{
			hr = GetTypeSubtypeString(szCLSID, NUMELMS(szCLSID), aInputTypes[i]);
			if (SUCCEEDED(hr))
			{
				m_ListInputTypes.AddString(szCLSID);
			}
		}

		// Show output type/subtype pairs
		for (i=0; i<ulNumOutputsSupplied; i++)
		{
			hr = GetTypeSubtypeString(szCLSID, NUMELMS(szCLSID), aOutputTypes[i]);
			if (SUCCEEDED(hr))
			{
				m_ListOutputTypes.AddString(szCLSID);
			}
		}
	}
}

void MainDialog::ShowSelectedDMOInfo(const GUID *pCLSID)
{
    HRESULT hr;
    TCHAR sz[64];

    // Create an instance of the selected DMO
    hr = CoCreateInstance(*pCLSID, NULL, 
                          CLSCTX_INPROC, IID_IMediaObject, 
                          (void **)&m_pDMO);
    if (FAILED(hr))
    {
        MessageBeep(0);
        return;
    }
       
    // Read and display information about its input/output streams
    DWORD dwInStream=0, dwOutStream=0, dwCount;

    hr = m_pDMO->GetStreamCount(&dwInStream, &dwOutStream);
    if (FAILED(hr))
    {
        MessageBeep(0);
        return;
    }

    // Display the number of streams used by this DMO
    hr = StringCchPrintf(sz, NUMELMS(sz), TEXT("In Streams: %d\0"), dwInStream);
	if (SUCCEEDED(hr))
	{
		SetDlgItemText(IDC_STATIC_INPUT_STREAMS, sz);
	}

    hr = StringCchPrintf(sz, NUMELMS(sz), TEXT("Out Streams: %d\0"), dwOutStream);
	if (SUCCEEDED(hr))
	{
		SetDlgItemText(IDC_STATIC_OUTPUT_STREAMS, sz);
	}

    // Fill stream list boxes
    for (dwCount=0; dwCount < dwInStream; dwCount++)
    {
        hr = StringCchPrintf(sz, NUMELMS(sz), TEXT("In %d\0"), dwCount);
		if (SUCCEEDED(hr))
		{
	        m_ListInputStreams.AddString(sz);
		}
    }
    for (dwCount=0; dwCount < dwOutStream; dwCount++)
    {
        hr = StringCchPrintf(sz, NUMELMS(sz), TEXT("Out %d\0"), dwCount);
		if (SUCCEEDED(hr))
		{
	        m_ListOutputStreams.AddString(sz);
		}
    }

    // Select the first item in each list, if it exists
    if (dwInStream > 0)
    {
        m_ListInputStreams.Select(0);
        OnSelchangeListInputStreams();
    }
    if (dwOutStream > 0)
    {
        m_ListOutputStreams.Select(0);
        OnSelchangeListOutputStreams();
    }
}

void MainDialog::OnSelchangeListInputStreams()
{
    HRESULT hr;
    TCHAR sz[64];
    UINT nSel;
    DWORD dwFlags=0;

	m_ListInputStreams.GetCurrentSelection(&nSel);

    //
    // Display relevant information about input stream
    //
    if (!m_pDMO)
	{
        return;
	}

    // Read input stream information flags
    hr = m_pDMO->GetInputStreamInfo(nSel, &dwFlags);
    if (FAILED(hr))
    {
        MessageBeep(0);
        return;
    }

    // Set stream info checkboxes
    m_CheckInWholeSamples.SetCheck(dwFlags & DMO_INPUT_STREAMF_WHOLE_SAMPLES);
    m_CheckInOneSample.SetCheck(dwFlags & DMO_INPUT_STREAMF_SINGLE_SAMPLE_PER_BUFFER);
    m_CheckInFixedSize.SetCheck(dwFlags & DMO_INPUT_STREAMF_FIXED_SAMPLE_SIZE);
    m_CheckInHoldsBuffers.SetCheck(dwFlags & DMO_INPUT_STREAMF_HOLDS_BUFFERS);

    // Read preferred input type information.  The media type/subtypes
    // are arranged in order of preference, starting from zero.
    DMO_MEDIA_TYPE dmt={0};
    hr = m_pDMO->GetInputType(nSel, 0, &dmt);

    if (SUCCEEDED(hr))
    {
        hr = GetFormatString(sz, NUMELMS(sz), &dmt);
	}

	if (SUCCEEDED(hr))
	{
		SetDlgItemText(IDC_STATIC_IN_FORMAT, sz);
	}

	if (SUCCEEDED(hr))
	{
        hr = GetGUIDString(sz, NUMELMS(sz), &dmt.majortype);
	}

	if (SUCCEEDED(hr))
	{
		SetDlgItemText(IDC_STATIC_IN_TYPE, sz);
	}

	if (SUCCEEDED(hr))
	{
        hr = GetGUIDString(sz, NUMELMS(sz), &dmt.subtype);
	}

	if (SUCCEEDED(hr))
	{
		SetDlgItemText(IDC_STATIC_IN_SUBTYPE, sz);
	}
    
	MoFreeMediaType(&dmt);

    // Does this DMO support quality control?
    IDMOQualityControl *pQC=0;
    hr = m_pDMO->QueryInterface(IID_IDMOQualityControl, (void **) &pQC);
    if (SUCCEEDED(hr))
    {
        m_CheckInQC.SetCheck(TRUE);
        pQC->Release();
    }
}

void MainDialog::OnSelchangeListOutputStreams() 
{
    HRESULT hr;
    TCHAR sz[64];
    UINT nSel;
    DWORD dwFlags=0;

	m_ListInputStreams.GetCurrentSelection(&nSel);

    //
    // Display relevant information about output stream
    //
    if (!m_pDMO)
	{
        return;
	}

    // Read output stream information flags
    hr = m_pDMO->GetOutputStreamInfo(nSel, &dwFlags);
    if (FAILED(hr))
    {
        MessageBeep(0);
        return;
    }

    // Set stream info checkboxes
    m_CheckOutWholeSamples.SetCheck(dwFlags & DMO_OUTPUT_STREAMF_WHOLE_SAMPLES);
    m_CheckOutOneSample.SetCheck(dwFlags & DMO_OUTPUT_STREAMF_SINGLE_SAMPLE_PER_BUFFER);
    m_CheckOutFixedSize.SetCheck(dwFlags & DMO_OUTPUT_STREAMF_FIXED_SAMPLE_SIZE);
    m_CheckOutDiscardable.SetCheck(dwFlags & DMO_OUTPUT_STREAMF_DISCARDABLE);
    m_CheckOutOptional.SetCheck(dwFlags & DMO_OUTPUT_STREAMF_OPTIONAL);

    // Read preferred output type information
    DMO_MEDIA_TYPE dmt={0};
    hr = m_pDMO->GetOutputType(nSel, 0, &dmt);

    if (SUCCEEDED(hr))
    {
        hr = GetFormatString(sz, NUMELMS(sz), &dmt);
	}

	if (SUCCEEDED(hr))
    {
		SetDlgItemText(IDC_STATIC_OUT_FORMAT, sz);
	}

	if (SUCCEEDED(hr))
    {
        hr = GetGUIDString(sz, NUMELMS(sz), &dmt.majortype);
	}

	if (SUCCEEDED(hr))
    {
		SetDlgItemText(IDC_STATIC_OUT_TYPE, sz);
	}

	if (SUCCEEDED(hr))
    {
        hr = GetGUIDString(sz, NUMELMS(sz), &dmt.subtype);
	}

	if (SUCCEEDED(hr))
    {
		SetDlgItemText(IDC_STATIC_OUT_SUBYTPE, sz);
	}

	MoFreeMediaType(&dmt);

    // Does this DMO support quality control?
    IDMOQualityControl *pQC=0;
    hr = m_pDMO->QueryInterface(IID_IDMOQualityControl, (void **) &pQC);
    if (SUCCEEDED(hr))
    {
        m_CheckOutQC.SetCheck(TRUE);
        pQC->Release();
    }
}
