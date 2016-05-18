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

	void	AddFilter(const TCHAR *szFilterName, const GUID *pCatGuid);
	void	AddTypeInfo(const GUID *pCLSID);
	void	ClearDeviceList();
	void	ClearDMOInfo();
	void	ClearFilterList();
	void	ClearTypeLists();
	void	FillCategoryList();
	void	OnCheckKeyed();
	void	OnSelchangeListDevices();
	void	OnSelchangeListFilters();
	void	OnSelchangeListInputStreams();
	void	OnSelchangeListOutputStreams(); 
	void	ReleaseDMO();
	void	SetNumClasses(int nClasses);
	void	SetNumFilters(int nFilters);
	void	ShowSelectedDMOInfo(const GUID *pCLSID);

	HRESULT EnumDMOs(IEnumDMO *pEnumCat);


    IMediaObject  *m_pDMO;

	// Controls
	Button		m_CheckOutQC;
	Button		m_CheckInQC;
	Button		m_CheckOutFixedSize;
	Button		m_CheckOutWholeSamples;
	Button		m_CheckOutOptional;
	Button		m_CheckOutOneSample;
	Button		m_CheckOutDiscardable;
	Button		m_CheckInWholeSamples;
	Button		m_CheckInOneSample;
	Button		m_CheckInHoldsBuffers;
	Button		m_CheckInFixedSize;
	ListBox		m_ListOutputStreams;
	ListBox		m_ListInputStreams;
	ListBox		m_ListOutputTypes;
	ListBox		m_ListInputTypes;
	Button		m_bCheckKeyed;
	ListBox		m_FilterList;
	ListBox		m_DeviceList;
	


};