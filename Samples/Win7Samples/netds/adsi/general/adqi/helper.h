

HRESULT  VariantToStringList(  VARIANT& refvar, CStringList& refstringlist);
HRESULT StringListToVariant( VARIANT& refvar, const CStringList& refstringlist);
void StringToStringList( CString s, CStringList &sList );
void StringListToString( CStringList &sList, CString &s );
void PopulateListBoxFromStringList( CListBox &list, CStringList &sList );
void PopulateComboBoxFromStringList( CComboBox &list, CStringList &sList );
void ADsToStringList(ADSVALUE *pValue, DWORD dwNumber, CStringList &sList );
HRESULT  VariantToPtrList(  VARIANT& refvar, CPtrList & refptrlist);
HRESULT PropertyValueToString( LONG lADsType, IADsPropertyValue *pValue, CString &sValue );
CString GetErrorMessage( HRESULT hr );
HRESULT ReportError( HRESULT hr );
CString GetADSIError( HRESULT hr );
HRESULT OctetVariantToString(  VARIANT& refvar, CString &sResult);


#define RETURN_ON_FAILURE(hr) if (!SUCCEEDED(hr)) { ReportError(hr); return; } 