//-----------------------------------------------------------------------------
// Microsoft OLE DB RowsetViewer
// Copyright (C) 1994 - 1999 By Microsoft Corporation.
//
// @doc
//
// @module COPTIONS.H
//
//-----------------------------------------------------------------------------------

#ifndef _COPTIONS_H_
#define _COPTIONS_H_
										   

/////////////////////////////////////////////////////////////////
// Defines 
//
/////////////////////////////////////////////////////////////////
enum CREATE_OPTIONS
{
	CREATE_INITIALIZE			= 0x00000001,
	CREATE_SETPROPERTIES		= 0x00000002,

	CREATE_USESERVICECOMP		= 0x00000010,
//	CREATE_USETRACEPROVIDER		= 0x00000020,
	
	CREATE_NEWWINDOW			= 0x00000100,
	CREATE_NEWWINDOW_IFEXISTS	= 0x00000200,
	CREATE_FINDWINDOW			= 0x00000400,
	CREATE_NODISPLAY			= 0x00000800,

	CREATE_CURSORENGINE			= 0x00001000,
	CREATE_RESTARTPOSITION		= 0x00002000,
	CREATE_DETERMINE_TYPE		= 0x00004000,

	CREATE_NO_AUTOSAVE			= 0x00010000,
	CREATE_QI_MANDATORY			= 0x00100000,
	CREATE_QI_OPTIONAL			= 0x00200000
};

enum COMMAND_OPTIONS
{
	COMMAND_ROWSAFFECTED		= 0x00000001,
	COMMAND_RELEASE_OPENOBJECTS	= 0x00000002
};

enum ACCESSOR_OPTIONS
{
	ACCESSOR_BIND_VALUE			= 0x00000001, //DBPART_VALUE
	ACCESSOR_BIND_LENGTH		= 0x00000002, //DBPART_LENGTH
	ACCESSOR_BIND_STATUS		= 0x00000004, //DBPART_STATUS
	ACCESSOR_BIND_BOOKMARK		= 0x00000008,

	ACCESSOR_BLOB_ISEQSTREAM	= 0x00000010,
	ACCESSOR_BLOB_ILOCKBYTES	= 0x00000020,
	ACCESSOR_BLOB_ISTORAGE		= 0x00000040,
	ACCESSOR_BLOB_ISTREAM		= 0x00000080,

	ACCESSOR_OWNED_PROVIDER		= 0x00001000
};

enum ROWSET_OPTIONS
{
	ROWSET_SETDEFAULTPROPS		= 0x00000004,
	ROWSET_ROWSTOFETCH			= 0x00000100,
	ROWSET_NODATA				= 0x00000200,

	ROWSET_HIDDENCOLUMNS		= 0x00001000,

	ROWSET_ALWAYSRELEASEROWS	= 0x00010000
};

enum NOTIFY_OPTIONS
{
	NOTIFY_OLEDB				= 0x00000001,
	NOTIFY_ADDREF				= 0x00000008,
	NOTIFY_RELEASE				= 0x00000010,
	NOTIFY_QI					= 0x00000020,

	//Trace Provider
	NOTIFY_TRACEPROVIDER		= 0x01000000,

	//Notifications
	NOTIFY_IDBASYNCHNOTIFY		= 0x00000100,
	NOTIFY_IROWSETNOTIFY		= 0x00000200,
	NOTIFY_IROWPOSITIONCHANGE	= 0x00000400,
	
	NOTIFY_PROMPTVETO			= 0x00010000,
	NOTIFY_IPOOLNOTIFY			= 0x00100000,
};


/////////////////////////////////////////////////////////////////////////////
// COptionsDataSource
//
/////////////////////////////////////////////////////////////////////////////
class COptionsDataSource : public CPropPageLite
{
public:
	COptionsDataSource();

	//Messages
	virtual BOOL OnInitDialog();
	virtual BOOL OnApply();

	virtual BOOL OnCommand(UINT iID, HWND hWndCtrl);
};


/////////////////////////////////////////////////////////////////////////////
// COptionsCommand
//
/////////////////////////////////////////////////////////////////////////////
class COptionsCommand : public CPropPageLite
{
public:
	COptionsCommand();

	//Messages
	virtual BOOL OnInitDialog();
	virtual BOOL OnApply();
};


/////////////////////////////////////////////////////////////////////////////
// COptionsAccessor
//
/////////////////////////////////////////////////////////////////////////////
class COptionsAccessor : public CPropPageLite
{
public:
	COptionsAccessor();

	//Messages
	virtual BOOL OnInitDialog();
	virtual BOOL OnApply();

protected:
	//data
	CComboBoxLite	m_comboBindingTypes;
};


/////////////////////////////////////////////////////////////////////////////
// COptionsRowset
//
/////////////////////////////////////////////////////////////////////////////
class COptionsRowset : public CPropPageLite
{
public:
	COptionsRowset();

	//Messages
	virtual BOOL OnInitDialog();
	virtual BOOL OnApply();

	virtual BOOL OnCommand(UINT iID, HWND hWndCtrl);
};


/////////////////////////////////////////////////////////////////////////////
// COptionsNotify
//
/////////////////////////////////////////////////////////////////////////////
class COptionsNotify : public CPropPageLite
{
public:
	COptionsNotify();

	//Messages
	virtual BOOL OnInitDialog();
	virtual BOOL OnApply();
};


/////////////////////////////////////////////////////////////////////////////
// COptionsError
//
/////////////////////////////////////////////////////////////////////////////
class COptionsError : public CPropPageLite
{
public:
	COptionsError();

	//Messages
	virtual BOOL OnInitDialog();
	virtual BOOL OnApply();

	virtual BOOL OnCommand(UINT iID, HWND hWndCtrl);
};


//////////////////////////////////////////////////////////////////////////
// COptionsSheet
//
//////////////////////////////////////////////////////////////////////////
class COptionsSheet : public CPropSheetLite
{
public:
	//Constructors
	COptionsSheet();

	//Helpers
	BOOL	LoadOptions();
	BOOL	SaveOptions();

	//Accessor
	DBTYPE	GetBindingType(DBTYPE wNativeType = DBTYPE_UDT);
	void	SetBindingType(DBTYPE wType)	{ m_wBindingType = wType; }

//protected:
	//Data
	COptionsDataSource	m_CDataSource;
	COptionsCommand		m_CCommand;
	COptionsAccessor	m_CAccessor;
	COptionsRowset		m_CRowset;
	COptionsNotify		m_CNotify;
	COptionsError		m_CError;

	//DataSource
	DWORD				m_dwCLSCTX;
	WCHAR				m_wszRemoteServer[MAX_NAME_LEN+1];
	DWORD				m_dwCreateOpts;

	//Command
	DWORD				m_dwCommandOpts;

	//Accessor
	DWORD				m_dwMaxLength;
	DWORD				m_dwAccessorOpts;

	//Rowset
	DWORD				m_dwConvFlags;
	DWORD				m_dwRowsetOpts;
	ULONG				m_cRowsToFetch;

	//Notify
	DWORD				m_dwNotifyOpts;

protected:
	//Accessor
	DBTYPE				m_wBindingType;
};


#endif	//_COPTIONS_H_
