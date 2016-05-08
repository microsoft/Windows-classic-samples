#include "mapper.h"
#include "MainDialog.h"
#include "types.h"

#include "fil_data.h"

// Forward declares

void FillMajorTypes(ListBox& listbox);
void FillSubType(ListBox& listboxMajor, ListBox& listboxMinor);

void EnableSecondTypePair(
		ListBox& ListMajor, 
		ListBox& ListMajor2, 
		ListBox& ListMinor2
	);



void FillTypeArray(
		ListBox& ListMajor, 
		ListBox& ListMinor,
		GUID *atypes, 
		int nIndex, 
		DWORD *pdwPairs
	);

void AddMeritToString(WCHAR *szInfo, size_t len, DWORD dwMerit);



/// MainDialog

MainDialog::MainDialog() 
  : CBaseDialog(IDD_MAPPER_DIALOG),
    m_pMapper(NULL)
{
}

MainDialog::~MainDialog()
{
	SAFE_RELEASE(m_pMapper);
}


HRESULT MainDialog::OnInitDialog()
{
	HRESULT hr;

    // Create the filter mapper that will be used for all queries    
    hr = CoCreateInstance(
		CLSID_FilterMapper2, 
        NULL, CLSCTX_INPROC, IID_IFilterMapper2, 
        (void **) &m_pMapper);


	if (SUCCEEDED(hr))
	{
		InitializeControls();
		SetDefaults();

	    // Customize list box tab stops for good formatting.
	    int nTabStops[2]={140,230};
		m_ListFilters.SendMessage(LB_SETTABSTOPS, 2, (LPARAM)nTabStops);

	    // Perform a default search with the default parameters.
		OnButtonSearch();
	}

	return hr;
}

INT_PTR MainDialog::OnCommand(HWND hControl, WORD idControl, WORD msg)
{
	switch (idControl)
	{
	case IDC_BUTTON_SEARCH:
		if (msg == BN_CLICKED)
		{
			OnButtonSearch();
		}
		break;

	case IDC_BUTTON_CLEAR:
		if (msg == BN_CLICKED)
		{
			OnButtonClear();
		}
		break;

	case IDC_LIST_INPUT_MAJOR:
		if (msg == LBN_SELCHANGE)
		{
			OnSelchangeListInputMajor();
		}
		break;

	case IDC_LIST_OUTPUT_MAJOR:
		if (msg == LBN_SELCHANGE)
		{
			OnSelchangeListOutputMajor();
		}
		break;

	case IDC_LIST_INPUT_MAJOR2:
		if (msg == LBN_SELCHANGE)
		{
			OnSelchangeListInputMajor2();
		}
		break;

	case IDC_LIST_OUTPUT_MAJOR2:
		if (msg == LBN_SELCHANGE)
		{
			OnSelchangeListOutputMajor2();
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
	m_ListOutputMinor2.SetWindow(GetDlgItem(IDC_LIST_OUTPUT_MINOR2));
	m_ListOutputMinor.SetWindow(GetDlgItem(IDC_LIST_OUTPUT_MINOR));
	m_ListOutputMajor2.SetWindow(GetDlgItem(IDC_LIST_OUTPUT_MAJOR2));
	m_ListOutputMajor.SetWindow(GetDlgItem(IDC_LIST_OUTPUT_MAJOR));
	m_ListInputMinor2.SetWindow(GetDlgItem(IDC_LIST_INPUT_MINOR2));
	m_ListInputMinor.SetWindow(GetDlgItem(IDC_LIST_INPUT_MINOR));
	m_ListInputMajor2.SetWindow(GetDlgItem(IDC_LIST_INPUT_MAJOR2));
	m_ListInputMajor.SetWindow(GetDlgItem(IDC_LIST_INPUT_MAJOR));

	m_bAtLeastOneOutputPin.SetWindow(GetDlgItem(IDC_CHECK_OUTPUT_PIN));
	m_bAtLeastOneInputPin.SetWindow(GetDlgItem(IDC_CHECK_INPUT_PIN));
	m_bIsRenderer.SetWindow(GetDlgItem(IDC_CHECK_RENDERER));
	m_bExactMatch.SetWindow(GetDlgItem(IDC_CHECK_EXACT_MATCH));

	m_ComboMerit.SetWindow(GetDlgItem(IDC_COMBO_MERIT));
	m_ComboOutputCat.SetWindow(GetDlgItem(IDC_COMBO_OUTPUT_CATEGORY));
	m_ComboInputCat.SetWindow(GetDlgItem(IDC_COMBO_INPUT_CATEGORY));

	m_ListFilters.SetWindow(GetDlgItem(IDC_LIST_FILTERS));
}


///////////////////////////////////////////////////////////////////////
// Name: SetDefaults
// Desc: Restore the UI controls to their default settings.
///////////////////////////////////////////////////////////////////////

void MainDialog::SetDefaults()
{
    // The app allows you to specify up to two major/minor type pairs.
    FillMajorTypes(m_ListInputMajor);
    FillMajorTypes(m_ListInputMajor2);
    FillMajorTypes(m_ListOutputMajor);
    FillMajorTypes(m_ListOutputMajor2);

	// Initialize subtype list boxes
    m_ListInputMinor.ClearItems();
    m_ListInputMinor2.ClearItems();
    m_ListOutputMinor.ClearItems();
    m_ListOutputMinor2.ClearItems();

    // Set checkboxes to preferred default values
    m_bAtLeastOneInputPin.SetCheck(1);
    m_bAtLeastOneOutputPin.SetCheck(1);

    // Initialize merit box
    m_ComboMerit.ClearItems();
    for (int i=0; i < NUM_MERIT_TYPES; i++)
    {
        m_ComboMerit.InsertString(i, merittypes[i].szName);
        m_ComboMerit.SetItemData(i, merittypes[i].dwMerit);
    }
    m_ComboMerit.Select(4);  // "Normal" merit
    
    // Initialize pin categories
    m_ComboInputCat.ClearItems();
    m_ComboOutputCat.ClearItems();
    m_ComboInputCat.AddString(L"<Don't care>\0");
    m_ComboOutputCat.AddString(L"<Don't care>\0");

    // Fill pin category lists
    for (int i=1; i < NUM_PIN_TYPES; i++)
    {
        m_ComboInputCat.InsertString(i, pintypes[i].szName);
        m_ComboInputCat.SetItemData(i, (DWORD_PTR)pintypes[i].pGUID);
        m_ComboOutputCat.InsertString(i, pintypes[i].szName);
        m_ComboOutputCat.SetItemData(i, (DWORD_PTR)pintypes[i].pGUID);
    }
    m_ComboInputCat.Select(0);
    m_ComboOutputCat.Select(0);  


    // Clear filter list
    m_ListFilters.ClearItems();  
    SetDlgItemText(IDC_STATIC_NUMFILTERS, L"Filters Matching Query : 0");

    // Select the first item in each type list
    FillSubType(m_ListInputMajor, m_ListInputMinor);
    FillSubType(m_ListInputMajor2, m_ListInputMinor2);
    FillSubType(m_ListOutputMajor, m_ListOutputMinor);
    FillSubType(m_ListOutputMajor2, m_ListOutputMinor2);


    // Disable the second type/subtype listboxes until the user selects
    // something other than "don't care" for the first type/subtype pair.
    OnSelchangeListInputMajor();
    OnSelchangeListOutputMajor();

}


///////////////////////////////////////////////////////////////////////
// Name: OnButtonClear
// Desc: Called when the Clear button is clicked.
///////////////////////////////////////////////////////////////////////

void MainDialog::OnButtonClear() 
{
    // Clear all values and reset dialog
    SetDefaults();  
}


///////////////////////////////////////////////////////////////////////
// Name: OnSelchangeListInputMajor
// Desc: Called when the "Input Media Type" list is selected.
///////////////////////////////////////////////////////////////////////

void MainDialog::OnSelchangeListInputMajor() 
{
    // User has selected a new major type, so refill the subtype list box
    FillSubType(m_ListInputMajor, m_ListInputMinor);

    // Since the second type/subtype pair is irrelevant if the first
    // is a don't care, disable the second set if appropriate.
    EnableSecondTypePair(m_ListInputMajor, m_ListInputMajor2, m_ListInputMinor2);
}

///////////////////////////////////////////////////////////////////////
// Name: OnSelchangeListInputMajor2
// Desc: Called when the "Secondary Input Type" list is selected.
///////////////////////////////////////////////////////////////////////

void MainDialog::OnSelchangeListInputMajor2() 
{
    // User has selected a new major type, so refill the subtype list box
    FillSubType(m_ListInputMajor2, m_ListInputMinor2);
}

///////////////////////////////////////////////////////////////////////
// Name: OnSelchangeListOutputMajor
// Desc: Called when the "Output Media Type" list is selected.
///////////////////////////////////////////////////////////////////////

void MainDialog::OnSelchangeListOutputMajor() 
{
    // User has selected a new major type, so refill the subtype list box
    FillSubType(m_ListOutputMajor, m_ListOutputMinor);

    // Since the second type/subtype pair is irrelevant if the first
    // is a don't care, disable the second set if appropriate.
    EnableSecondTypePair(m_ListOutputMajor, m_ListOutputMajor2, m_ListOutputMinor2);
}

///////////////////////////////////////////////////////////////////////
// Name: OnSelchangeListOutputMajor2
// Desc: Called when the "Secondary Output Type" list is selected.
///////////////////////////////////////////////////////////////////////

void MainDialog::OnSelchangeListOutputMajor2() 
{
    // User has selected a new major type, so refill the subtype list box
    FillSubType(m_ListOutputMajor2, m_ListOutputMinor2);
}


///////////////////////////////////////////////////////////////////////
// Name: OnButtonSearch
// Desc: Called when the "Saerch" button is clicked.
//
// This function enumerates the filters that match the search criteria
// the user has selected. For each matching filter, the name, merit,
// and DLL is listed.
//
///////////////////////////////////////////////////////////////////////

void MainDialog::OnButtonSearch() 
{
    HRESULT hr=S_OK;
    int nFilters = 0;    

	IEnumMoniker *pEnum = NULL;
    IMoniker *pMoniker=0;

    // Clear any previous results
    m_ListFilters.ClearItems();

    // Enumerate filters based on the current dialog box selections.
	hr = EnumerateFilters(&pEnum);
    if (FAILED(hr))
    {
        return;
    }
	
	// Reset the enumerator.
    hr = pEnum->Reset();  // Not strictly necessary but can't hurt.

	// Go through each moniker in the collection.
    while((hr = pEnum->Next(1, &pMoniker, NULL)) == S_OK)
    {
        IPropertyBag *pPropBag = NULL;
        DWORD dwMerit;
        VARIANT varName;
        VARIANT varFilterClsid;

		VariantInit(&varName);
		VariantInit(&varFilterClsid);

		// Initialize variant types 
		varName.vt = VT_BSTR;
        varFilterClsid.vt = VT_BSTR;

        // Get a property bag for this moniker.
        hr = pMoniker->BindToStorage(
			0, 0, IID_IPropertyBag, (void **)&pPropBag);

	    // Read the filter name from the property bag
        if (SUCCEEDED(hr))
		{
		    hr = pPropBag->Read(L"FriendlyName", &varName, 0);
		}

        
        // Read the Merit value from the property bag
		if (SUCCEEDED(hr))
		{
	        hr = GetMerit(pPropBag, &dwMerit);
		}

        // Read filter's CLSID from property bag.  The CLSID is
		// returned as a string (BSTR).

		if (SUCCEEDED(hr))
		{
	        hr = pPropBag->Read(L"CLSID", &varFilterClsid, 0);
		}

         // Add filter name and filename to listbox
         if(SUCCEEDED(hr))
		 {
			 nFilters++;
			 AddFilterToList(varName.bstrVal, varFilterClsid.bstrVal, dwMerit);
		 }

        // Clean up.
        SysFreeString(varFilterClsid.bstrVal);
        SysFreeString(varName.bstrVal);
        SAFE_RELEASE(pPropBag);
        SAFE_RELEASE(pMoniker);
    }   

    // Display number of filters matching query
    WCHAR szNumFilters[48];
    hr = StringCchPrintf(szNumFilters, NUMELMS(szNumFilters), L"Filters Matching Query : %d\0", nFilters);

    SetDlgItemText(IDC_STATIC_NUMFILTERS, szNumFilters);

	if (nFilters == 0)
	{
        m_ListFilters.AddString(L"<No filters matched query>");
	}

    // Clean up enumerator
    SAFE_RELEASE(pEnum);


}


///////////////////////////////////////////////////////////////////////
// Name: EnumerateFilters
// Desc: Enumerates the filters that match the user's search criteria.
///////////////////////////////////////////////////////////////////////

HRESULT MainDialog::EnumerateFilters(IEnumMoniker **ppEnum)
{
	HRESULT hr;

	assert(ppEnum);

    // First set up the boolean values
    BOOL bExactMatch = m_bExactMatch.IsChecked();
    BOOL bAtLeastOneInputPin = m_bAtLeastOneInputPin.IsChecked();
    BOOL bAtLeastOneOutputPin = m_bAtLeastOneOutputPin.IsChecked();
    BOOL bIsRenderer = m_bIsRenderer.IsChecked();

	DWORD_PTR dwMerit = 0;
	GUID *pInCat = NULL;
	GUID *pOutCat = NULL;

    // Merit
	m_ComboMerit.GetCurrentSelectionItemData(&dwMerit);

	// Pin categories
    m_ComboInputCat.GetCurrentSelectionItemData((DWORD_PTR*)&pInCat);
    m_ComboOutputCat.GetCurrentSelectionItemData((DWORD_PTR*)&pOutCat);


    // Major type/subtype pairs
    DWORD dwInputTypes=0, dwOutputTypes=0;
    GUID arrayInput[4], arrayOutput[4];

    // Initialize GUID type/subtype arrays
    for (int i=0; i<4; i++)
    {
        arrayInput[i]  = GUID_NULL;
        arrayOutput[i] = GUID_NULL;
    }

    // Fill each of the four GUID arrays (input/output, major/minor).
    // If the first call doesn't set anything (primary), don't bother to
    // read the secondary value, since it would be invalid anyway due to
    // its position within the array (2) but with a count of only 1.
    FillTypeArray(m_ListInputMajor, m_ListInputMinor, arrayInput, 0, &dwInputTypes);
    if (dwInputTypes)
	{
        FillTypeArray(m_ListInputMajor2, m_ListInputMinor2, arrayInput, 2, &dwInputTypes);
	}

    FillTypeArray(m_ListOutputMajor, m_ListOutputMinor, arrayOutput, 0, &dwOutputTypes);
    
	if (dwOutputTypes)
	{
        FillTypeArray(m_ListOutputMajor2, m_ListOutputMinor2, arrayOutput, 2, &dwOutputTypes);
	}

	hr = m_pMapper->EnumMatchingFilters(
            ppEnum,
            0,                  // Reserved
            bExactMatch,        // Use exact match?
            (DWORD)dwMerit,     // Minimum merit
            bAtLeastOneInputPin,
            dwInputTypes,       // Number of major type/subtype pairs for input
            arrayInput,         // Array of major type/subtype pairs for input
            NULL,               // Input medium
            pInCat,             // Input pin category
            bIsRenderer,        // Must be a renderer?
            bAtLeastOneOutputPin,
            dwOutputTypes,      // Number of major type/subtype pairs for output
            arrayOutput,        // Array of major type/subtype pairs for output
            NULL,               // Output medium
            pOutCat);           // Output pin category

	return hr;
}


///////////////////////////////////////////////////////////////////////
// Name: AddFilterToList
// Desc: Displays information about a filter in the "Filters" list box.
//
// szFilterName: Display name of the filter.
// szGUID: CLSID of the filter, in string format
// dwMerit: Filter merit
///////////////////////////////////////////////////////////////////////

void MainDialog::AddFilterToList(const WCHAR *szFilterName, const WCHAR *szGUID, DWORD dwMerit)
{
	const DWORD STRING_LEN = 300;

    WCHAR szInfo[STRING_LEN], szFilename[STRING_LEN];

    HRESULT hr;

    // First add the friendly name of the filter
    hr = StringCchPrintf(szInfo, STRING_LEN, L"%s\t\0", szFilterName);

    // Add the filter's merit value
	if (SUCCEEDED(hr))
	{
	    AddMeritToString(szInfo, STRING_LEN, dwMerit);
	}

    // Get the filter's server (DLL) name from the registry
	if (SUCCEEDED(hr))
	{
	    hr = GetFilenameByCLSIDString(szGUID, szFilename, STRING_LEN);
	}

    // Add the filter's server name
    if (SUCCEEDED(hr))
    {
        hr = StringCchCat(szInfo, STRING_LEN, L"\t\0");
	}

	if (SUCCEEDED(hr))
	{
        hr = StringCchCat(szInfo, STRING_LEN, szFilename);
    }

    // Add the final string to the listbox
	if (SUCCEEDED(hr))
	{
	    m_ListFilters.AddString(szInfo);
	}
}




// Read merit and version information
HRESULT MainDialog::GetMerit(IPropertyBag *pPropBag, DWORD *pdwMerit)
{
    HRESULT hr;
    IAMFilterData *pData = NULL;

    VARIANT varFilData={0};
    varFilData.vt = VT_UI1 | VT_ARRAY;
    varFilData.parray = 0;     // docs say to zero this

    BYTE *pbFilterData = 0;    // 0 if not read
    DWORD dwcbFilterDAta = 0;  // 0 if not read

    // Read compressed filter data from the property bag with a variant
    hr = pPropBag->Read(L"FilterData", &varFilData, 0);
    if (SUCCEEDED(hr))
    {
        assert(varFilData.vt == (VT_UI1 | VT_ARRAY));
        dwcbFilterDAta = varFilData.parray->rgsabound[0].cElements;

        // Access the filter data
        hr = SafeArrayAccessData(varFilData.parray, (void **)&pbFilterData);
    }

    // Get the IAMFilterData interface for parsing the filter data
	if (SUCCEEDED(hr))
	{
	    hr = m_pMapper->QueryInterface(__uuidof(IAMFilterData), (void **)&pData);
	}

    if (SUCCEEDED(hr))
    {
        BYTE *pb=0;

        // Use a helper method to parse the binary filter data.  Pass in
        // the pointer to the filter data, its size, and a buffer to fill with
        // the resulting data.  The "pb" buffer is allocated with CoTaskMemAlloc,
        // so it must be correspondingly freed by the caller.
        hr = pData->ParseFilterData(pbFilterData, dwcbFilterDAta, &pb);
        if(SUCCEEDED(hr))
        {
            REGFILTER2 *pFil = ((REGFILTER2 **)pb)[0];


            // Assign the merit value from the REGFILTER2 structure
            if (pFil)
			{
                *pdwMerit = pFil->dwMerit;

                 // Free the memory allocated by ParseFilterData
                 CoTaskMemFree(pFil);
			}
        }
    }

	if (pbFilterData)
	{
		SafeArrayUnaccessData(varFilData.parray);
		VariantClear(&varFilData);
	}

	// Release the IAMFilterData interface
	SAFE_RELEASE(pData);

    return hr;
}


////// Helper functions



void FillMajorTypes(ListBox& listbox)
{
    listbox.ClearItems();

    // Fill the specified list box with major type name/GUID
    for (int i=0; i < NUM_MAJOR_TYPES; i++)
    {
        listbox.AddItem(majortypes[i].szName, (void *) majortypes[i].pGUID);
    }

    listbox.Select(0);
}


void FillSubType(ListBox& listboxMajor, ListBox& listboxMinor)
{
    const GUIDINFO *pSubtype;
    UINT nSelection = 0;
	
	listboxMajor.GetCurrentSelection(&nSelection);
    int nMajorType;

    // First clear the subtype list
    listboxMinor.ClearItems();

    // If the "don't care" item was selected, clear and exit
    if (nSelection == 0)
    {
        listboxMinor.AddString(L"<No subtypes>\0");
        listboxMinor.Select(0);
        return;
    }
    else
	{
        nMajorType = nSelection - 1;
	}

    // Determine how to fill the minor type list, based on the
    // currently selected major type.
    pSubtype = pSubTypes[nMajorType];

    // If there's no associated subtype, just add a default
    if (!pSubtype)
    {
        listboxMinor.AddString(L"<No subtypes>\0");
        listboxMinor.Select(0);
        return;
    }
    else
    {
        // Set a default item for "don't care"
        listboxMinor.AddString(L"<Don't care>\0");

        int i=0;

        // Fill the subtype list box.  Enter N item data to the N+1 list slot.
        while (pSubtype[i].pGUID != NULL)
        {
            listboxMinor.AddItem(pSubtype[i].szName, (void *) pSubtype[i].pGUID);
            i++;
        }

        listboxMinor.Select(0);
    }
}




void EnableSecondTypePair(
		ListBox& ListMajor, 
        ListBox& ListMajor2, 
		ListBox& ListMinor2
	)
{
    // If there is no selection in the first major type listbox,
    // clear and disable the second major/minor type listboxes.

	UINT selection = 0;
	ListMajor.GetCurrentSelection(&selection);

    if (selection == 0)
    {
        ListMajor2.Select(0);
        FillSubType(ListMajor2, ListMinor2);

        ListMajor2.Enable(FALSE);
        ListMinor2.Enable(FALSE);
    }
    else
    {
        ListMajor2.Enable(TRUE);
        ListMinor2.Enable(TRUE);
    }
}



void FillTypeArray(
		ListBox& ListMajor, 
		ListBox& ListMinor,
		GUID *atypes, 
		int nIndex, 
		DWORD *pdwPairs
	)
{
    UINT nMajorSel = 0, nMinorSel = 0; 
		
	BOOL result = ListMajor.GetCurrentSelection(&nMajorSel);
    // If no selection ("don't care"), just exit without modifying the array
	if (!result || nMajorSel == 0)
	{
        return;
	}

    // Get GUID for major type
    const GUID *p1 = (const GUID *)ListMajor.GetItem(nMajorSel);

    // Since the FilterMapper interface requires GUIDs (instead of GUID *),
    // copy the specified GUID data into its array slot.
    if (p1)
	{
        memcpy(&atypes[nIndex], p1, sizeof(GUID));
	}
    else
	{
        memset(&atypes[nIndex], 0, sizeof(GUID));
	}

    // Increment number of type/subtype pairs
    (*pdwPairs)++;

	result = ListMinor.GetCurrentSelection(&nMinorSel);

    // If no selection ("don't care"), or uninitialized (returning -1),
    // just exit without modifying the array
    if (!result || nMinorSel == 0)
	{
        return;
	}
    
    // Get GUID for subtype
    const GUID *p2 = (const GUID *)ListMinor.GetItem(nMinorSel);

    if (p2)
	{
        memcpy(&atypes[nIndex+1], p2, sizeof(GUID));
	}
    else
	{
        memset(&atypes[nIndex+1], 0, sizeof(GUID));
	}
}



void AddMeritToString(WCHAR *szInfo, size_t len, DWORD dwMerit)
{
    WCHAR szMerit[32];
    HRESULT hr;

    switch (dwMerit)
    {
        case MERIT_NORMAL:
             hr = StringCchCopy(szMerit, NUMELMS(szMerit), L"MERIT_NORMAL\0");
             break;     // Assume normal
        case MERIT_PREFERRED:
             hr = StringCchCopy(szMerit, NUMELMS(szMerit), L"MERIT_PREFERRED\0");
             break;
        case MERIT_UNLIKELY:
             hr = StringCchCopy(szMerit, NUMELMS(szMerit), L"MERIT_UNLIKELY\0");
             break;
        case MERIT_DO_NOT_USE:
             hr = StringCchCopy(szMerit, NUMELMS(szMerit), L"MERIT_DO_NOT_USE\0");
             break;
        case MERIT_SW_COMPRESSOR:
             hr = StringCchCopy(szMerit, NUMELMS(szMerit), L"MERIT_SW_COMPRESSOR\0");
             break;
        case MERIT_HW_COMPRESSOR:
             hr = StringCchCopy(szMerit, NUMELMS(szMerit), L"MERIT_HW_COMPRESSOR\0");
             break;
        
        default:
             hr = StringCchPrintf(szMerit, NUMELMS(szMerit), L"0x%08x\0", dwMerit);
             break;
    }

    // Add new merit information to string
    hr = StringCchCat(szInfo, len , szMerit);
}

