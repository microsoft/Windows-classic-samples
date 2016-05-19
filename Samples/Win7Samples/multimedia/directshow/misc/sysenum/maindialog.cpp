#include "SysEnum.h"
#include "MainDialog.h"


//
//  Global data structure for storing CLSIDs and friendly strings
//
typedef struct _category_info
{
    const CLSID *pclsid;
    TCHAR szName[128];

} CATEGORY_INFO_DS;

// The DirectShow reference documentation lists a set of filter categories
// for which you can enumerate corresponding filters.  See 'Filter Categories'
// under 'DirectShow->Reference->Constants and GUIDS' in the DirectX docs.
const CATEGORY_INFO_DS categories[] = {
    
    &CLSID_AudioInputDeviceCategory, TEXT("Audio Capture Devices\0"),
    &CLSID_AudioCompressorCategory,  TEXT("Audio Compressors\0"),
    &CLSID_AudioRendererCategory,    TEXT("Audio Renderers\0"),
    &CLSID_LegacyAmFilterCategory,   TEXT("DirectShow Filters\0"),
    &CLSID_MidiRendererCategory,     TEXT("Midi Renderers\0"),
    &CLSID_VideoInputDeviceCategory, TEXT("Video Capture Devices\0"),
    &CLSID_VideoCompressorCategory,  TEXT("Video Compressors\0"),
};

#define NUM_CATEGORIES  (sizeof(categories) / sizeof(CATEGORY_INFO_DS))



/// MainDialog

MainDialog::MainDialog() 
  : CBaseDialog(IDD_SYSENUM_DIALOG),
    m_pSysDevEnum(NULL),
	m_bShowAllCategories(FALSE)
{
}

MainDialog::~MainDialog()
{
	SAFE_RELEASE(m_pSysDevEnum);
}


HRESULT MainDialog::OnInitDialog()
{
	HRESULT hr;

    // Create the filter mapper that will be used for all queries    
    hr = CoCreateInstance(
		CLSID_SystemDeviceEnum, 
        NULL, CLSCTX_INPROC, IID_ICreateDevEnum, 
        (void **) &m_pSysDevEnum);


	if (SUCCEEDED(hr))
	{
		InitializeControls();
	}

	FillCategoryList();

	return hr;
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

	case IDC_CHECK_SHOWALL:
		if (msg == BN_CLICKED)
		{
			OnCheckShowall();
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
	m_FilterList.SetWindow(GetDlgItem(IDC_LIST_FILTERS));
	m_DeviceList.SetWindow(GetDlgItem(IDC_LIST_DEVICES));
}


void MainDialog::FillCategoryList()
{
    // Clear listboxes
    ClearDeviceList();
    ClearFilterList();

    if (m_bShowAllCategories)
    {
        // Emulate the behavior of GraphEdit by enumerating all 
        // categories in the system
        DisplayFullCategorySet();
    }
    else
    {
        // Fill the category list box with the categories to display,
        // using the names stored in the CATEGORY_INFO array.
        // See SysEnumDlg.H for a category description.
        for (int i=0; i < NUM_CATEGORIES; i++)
        {
            m_DeviceList.AddString(categories[i].szName);
        }

        // Update listbox title with number of classes
        SetNumClasses(NUM_CATEGORIES);
    }
}


void MainDialog::OnSelchangeListDevices()
{
    HRESULT hr;    
    IEnumMoniker *pEnumCat = NULL;

    // Get the currently selected category name
    UINT nItem;
    const CLSID *clsid;
    
	m_DeviceList.GetCurrentSelection(&nItem);

    if (m_bShowAllCategories)
    {
        // Read the CLSID pointer from the list box's item data
        clsid = (CLSID *) m_DeviceList.GetItem(nItem);
    }
    else
    {
        // Read the CLSID pointer from our hard-coded array of
        // documented filter categories
        clsid = categories[nItem].pclsid;
    }

    // If the CLSID wasn't allocated earlier, then fail
    if (clsid != NULL)
    {
		//
		// WARNING!
		//
		// Some third-party filters throw an exception (int 3) during enumeration
		// on Debug builds, often due to heap corruption in RtlFreeHeap().
		// Sometimes it is done intentionally to prevent reverse engineering.
		// 
		// This is not an issue on Release builds.
		//

		// Enumerate all filters of the selected category  
		hr = m_pSysDevEnum->CreateClassEnumerator(*clsid, &pEnumCat, 0);

		// Enumerate all filters using the category enumerator
		if (SUCCEEDED(hr))
		{
			hr = EnumFilters(pEnumCat);
		}

		// Go ahead and select the first filter in the list, if it exists
		if (SUCCEEDED(hr) && m_FilterList.Count() > 0)
		{
			m_FilterList.Select(0);
			OnSelchangeListFilters();
		}
	}
    SAFE_RELEASE(pEnumCat);
}


void MainDialog::OnSelchangeListFilters()
{
	const size_t FILE_NAME_LEN = 512;

	TCHAR szFileName[FILE_NAME_LEN];
    const CLSID *clsid;

    // Get the currently selected category name
    UINT nItem;
	m_FilterList.GetCurrentSelection(&nItem);
    
    // Read the CLSID pointer from the list box's item data
    clsid = (CLSID *) m_FilterList.GetItem(nItem);

    // Find the filter filename in the registry (by CLSID)
    if (clsid != 0)
	{
		HRESULT hr = GetFilenameByCLSID(clsid, szFileName, FILE_NAME_LEN);
		if (SUCCEEDED(hr))
		{
			SetDlgItemText(IDC_STATIC_FILENAME, szFileName);
		}
		else
		{
			SetDlgItemText(IDC_STATIC_FILENAME, TEXT("<Unknown>"));
		}
	}
}


void MainDialog::OnCheckShowall()
{
    // Toggle category type and redraw the category list
    m_bShowAllCategories ^= 1;
    FillCategoryList();
    SetNumFilters(0);
}


void MainDialog::DisplayFullCategorySet()
{
    HRESULT hr;
    int nClasses=0;

    IEnumMoniker *pEmCat = 0;
    ICreateDevEnum *pCreateDevEnum = NULL;
	IPropertyBag *pPropBag = NULL;

    // Create the system device enumerator.
    hr = CoCreateInstance(CLSID_SystemDeviceEnum, NULL, CLSCTX_INPROC_SERVER,
                          IID_ICreateDevEnum, (void**)&pCreateDevEnum);

	if (SUCCEEDED(hr))
	{
		// Use the meta-category that contains a list of all categories.
		// This emulates the behavior of GraphEdit.
		hr = pCreateDevEnum->CreateClassEnumerator(
							CLSID_ActiveMovieCategories, &pEmCat, 0);

		if (hr == S_FALSE)
		{
			// If CreateClassEnumerator returns S_OK, it means this
			// category does not exist or is empty. For the meta-category,
			// that would qualify as a failure!
			hr = E_FAIL;
		}
	}

	// Enumerate over every category and get the category CLSID and description.
    if (SUCCEEDED(hr))
    {
        IMoniker *pMCat = NULL;

        while (S_OK == pEmCat->Next(1, &pMCat, NULL))
        {
			VARIANT varCatName;
			VariantInit(&varCatName);

			VARIANT varCatClsid;
			VariantInit(&varCatClsid);

			CLSID clsidCat;

            // Associate moniker with a file
            hr = pMCat->BindToStorage(0, 0, IID_IPropertyBag, (void **)&pPropBag);
			
			// Read CLSID string from property bag
			if (SUCCEEDED(hr))
			{
				hr = pPropBag->Read(L"CLSID", &varCatClsid, 0);
			}

			// Convert to a CLSID
			if (SUCCEEDED(hr))
			{
				hr = CLSIDFromString(varCatClsid.bstrVal, &clsidCat);
			}

			// Read the category name. If that fails, use the CLSID string.
			if (SUCCEEDED(hr))
			{
				HRESULT hrTmp = pPropBag->Read(L"FriendlyName", &varCatName, 0);

				// Add category name and CLSID to list box
				if(SUCCEEDED(hrTmp))
				{
					hr = AddFilterCategory(varCatName.bstrVal, clsidCat);
				}
				else
				{
					hr = AddFilterCategory(varCatClsid.bstrVal, clsidCat);
				}
				nClasses++;
			}

			SAFE_RELEASE(pMCat);
			SAFE_RELEASE(pPropBag);
			VariantClear(&varCatName);
			VariantClear(&varCatClsid);

			if (FAILED(hr))
			{
				break;
			}
        } // for loop
    }

    // Update listbox title with number of classes
	if (SUCCEEDED(hr))
	{
	    SetNumClasses(nClasses);
	}

	SAFE_RELEASE(pEmCat);
    SAFE_RELEASE(pCreateDevEnum);

}


HRESULT MainDialog::EnumFilters(IEnumMoniker *pEnumCat)
{
    HRESULT hr=S_OK;
    int nFilters=0;

    IMoniker *pMoniker = NULL;

    // Clear the current filter list
    ClearFilterList();

    // If there are no filters of a requested type, show default string
    if (!pEnumCat)
    {
        m_FilterList.AddString(TEXT("<< No entries >>"));
        SetNumFilters(nFilters);
        return S_OK;
    }

    // Enumerate all items associated with the moniker
    while (pEnumCat->Next(1, &pMoniker, NULL) == S_OK)
    {
        IPropertyBag *pPropBag = NULL;
		CLSID clsidFilter;

		VARIANT varName;
        VARIANT varFilterClsid;

		VariantInit(&varName);
		VariantInit(&varFilterClsid);

        // Associate moniker with a file
        hr = pMoniker->BindToStorage(0, 0, IID_IPropertyBag, 
                                    (void **)&pPropBag);

		// Read filter name from property bag
		if (SUCCEEDED(hr))
		{
			hr = pPropBag->Read(L"FriendlyName", &varName, 0);
		}

        // Read filter's CLSID from property bag.  This CLSID string will be
        // converted to a binary CLSID and passed to AddFilter(), which will
        // add the filter's name to the listbox and its CLSID to the listbox
        // item's DataPtr item.  When the user clicks on a filter name in
        // the listbox, we'll read the stored CLSID, convert it to a string,
        // and use it to find the filter's filename in the registry.

		if (SUCCEEDED(hr))
		{
			// Read CLSID string from property bag
			hr = pPropBag->Read(L"CLSID", &varFilterClsid, 0);

            // Add filter name and CLSID to listbox
            if (SUCCEEDED(hr))
            {
                hr = CLSIDFromString(varFilterClsid.bstrVal, &clsidFilter);
            }
            else if (hr == E_PROP_ID_UNSUPPORTED)
            {
                clsidFilter = GUID_NULL; // No CLSID is listed. 
                hr = S_OK;
            }
        }

		// Add filter name and filename to listbox
		if(SUCCEEDED(hr))
		{
	        nFilters++;
			AddFilter(varName.bstrVal, clsidFilter);
		}

		VariantClear(&varName);
		VariantClear(&varFilterClsid);

        // Cleanup interfaces
        SAFE_RELEASE(pPropBag);
        SAFE_RELEASE(pMoniker);
    }

    // Update count of enumerated filters
    SetNumFilters(nFilters);
    return hr;
}

void MainDialog::SetNumClasses(int nClasses)
{
    TCHAR szClasses[64];

    HRESULT hr = StringCchPrintf(
		szClasses, 
		NUMELMS(szClasses), 
		TEXT("System Device Classes (%d found)\0"), 
		nClasses
	);

	SetDlgItemText(IDC_STATIC_CLASSES, szClasses);
}

void MainDialog::SetNumFilters(int nFilters)
{
    TCHAR szFilters[64]={0};

    HRESULT hr;
	hr = StringCchPrintf(szFilters, NUMELMS(szFilters), 
			TEXT("Registered Filters (%d found)\0"), nFilters);

	if (SUCCEEDED(hr))
	{
		SetDlgItemText(IDC_STATIC_FILTERS, szFilters);
	}
}


void MainDialog::ClearDeviceList(void)
{
    CLSID *pStoredId = NULL;
    
    UINT nCount = m_DeviceList.Count();

    // Delete any CLSID pointers that were stored in the listbox item data
    for (UINT i=0; i < nCount; i++)
    {
        pStoredId = (CLSID *) m_DeviceList.GetItem(i);
        if (pStoredId != 0)
        {
            delete pStoredId;
            pStoredId = NULL;
        }
    }

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
    SetDlgItemText(IDC_STATIC_FILENAME, TEXT("<No filter selected>"));
}


HRESULT MainDialog::AddFilterCategory(const TCHAR *szCatDesc, const GUID& CatGuid)
{
    // Allocate a new CLSID, whose pointer will be stored in 
    // the listbox.  When the listbox is cleared, these will be deleted.
    CLSID *pclsid = new CLSID;

    if (!pclsid)
	{
		return E_OUTOFMEMORY;
	}

	*pclsid = CatGuid;
	

    // Add the category name and a pointer to its CLSID to the list box
	BOOL result = m_DeviceList.AddItem(szCatDesc, pclsid);

	return (result ? S_OK : E_FAIL);
}


HRESULT MainDialog::AddFilter(const TCHAR *szFilterName, const GUID& CatGuid)
{
    // Allocate a new CLSID, whose pointer will be stored in 
    // the listbox.  When the listbox is cleared, these will be deleted.
    CLSID *pclsid = new CLSID;

	if (!pclsid)
	{
		return E_OUTOFMEMORY;
	}

    // Copy the CLSID.
    memcpy(pclsid, &CatGuid, sizeof(GUID));

    // Add the category name and a pointer to its CLSID to the list box
	BOOL result = m_FilterList.AddItem(szFilterName, pclsid);

	return (result ? S_OK : E_FAIL);
}


