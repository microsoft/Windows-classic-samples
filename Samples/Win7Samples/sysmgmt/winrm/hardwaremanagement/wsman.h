#ifndef __wsman_h__
#define __wsman_h__

class Wsman
{
private:
	CLSID _wsmanOleClsid;
	IDispatch * _pWsmanDispatch;
	IDispatch * _pWsmanSessionDispatch;
	DISPID _getDispid;
	DISPID _putDispid;
	DISPID _createSessionDispid;
	BSTR _tmpBuf;

public:
	Wsman(void);
	~Wsman(void);
	void CreateSession(const LPWSTR connectionString = L"");
	BSTR Get(const LPWSTR resourceUri);
	BSTR Put(const LPWSTR resourceUri, const LPWSTR resource);
};

#endif
