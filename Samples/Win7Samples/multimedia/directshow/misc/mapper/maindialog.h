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
	void SetDefaults();
	void AddFilterToList(const WCHAR *szFilterName, const WCHAR *szGUID, DWORD dwMerit);

	HRESULT EnumerateFilters(IEnumMoniker **ppEnum);
	HRESULT GetMerit(IPropertyBag *pPropBag, DWORD *pdwMerit);

	// Message handlers
	void OnButtonSearch();
	void OnButtonClear();
	void OnSelchangeListInputMajor();
	void OnSelchangeListOutputMajor();
	void OnSelchangeListInputMajor2();
	void OnSelchangeListOutputMajor2();


	IFilterMapper2 *m_pMapper;

	// Controls
	ListBox		m_ListOutputMinor2;
	ListBox		m_ListOutputMinor;
	ListBox		m_ListOutputMajor2;
	ListBox		m_ListOutputMajor;
	ListBox		m_ListInputMinor2;
	ListBox		m_ListInputMinor;
	ListBox		m_ListInputMajor2;
	ListBox		m_ListInputMajor;
	ListBox		m_ListFilters;

	Button		m_bAtLeastOneOutputPin;
	Button		m_bAtLeastOneInputPin;
	Button		m_bIsRenderer;
	Button		m_bExactMatch;

	ComboBox	m_ComboMerit;
	ComboBox	m_ComboOutputCat;
	ComboBox	m_ComboInputCat;



};