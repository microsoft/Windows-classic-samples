#pragma once 

class MainDialog : public CBaseDialog
{

public:
	MainDialog();
	~MainDialog();

	HRESULT OnInitDialog(); 
	INT_PTR OnCommand(HWND hControl, WORD idControl, WORD msg);

private:

	void InitializeControls();
    void FillCategoryList();
    void DisplayFullCategorySet();
    void ClearDeviceList();
    void ClearFilterList();
    void SetNumClasses(int nClasses);
    void SetNumFilters(int nFilters);
	void OnSelchangeListDevices();
	void OnSelchangeListFilters();
	void OnCheckShowall();
    
	HRESULT AddFilterCategory(const TCHAR *szCatDesc, const GUID& CatGuid);
    HRESULT AddFilter(const TCHAR *szFilterName, const GUID& CatGuid);
    HRESULT EnumFilters(IEnumMoniker *pEnumCat);


	ICreateDevEnum*  m_pSysDevEnum;

	ListBox		m_FilterList;
	ListBox		m_DeviceList;
	BOOL		m_bShowAllCategories;

};