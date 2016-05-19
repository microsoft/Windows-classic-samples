//*****************************************************************************
//
// Microsoft Windows Media
// Copyright (C) Microsoft Corporation. All rights reserved.
//
// FileName:            ExtensionData.h
//
// Abstract:            Definition for CExtensionData class 
//
//*****************************************************************************

#if !defined(AFX_CExtensionData_H__21A57EE3_8C94_4E10_8092_4C171BCBA68E__INCLUDED_)
#define AFX_CExtensionData_H__21A57EE3_8C94_4E10_8092_4C171BCBA68E__INCLUDED_
class CExtensionData 
{
public :

    CExtensionData( GUID guidDUExt, BYTE *pbExtensionSystemInfo,
                    WORD cbExtensionDataSize, DWORD cbExtensionSystemInfo, WORD wStreamNum );
	~CExtensionData();

    GUID            m_guidDUExt;
    BYTE            *m_pbExtensionSystemInfo;
    WORD            m_cbExtensionDataSize;
    DWORD           m_cbExtensionSystemInfo;
    WORD            m_wStreamNum;
    void            *m_pValue;
    CExtensionData  *m_pNext;

    

    HRESULT DisplayData();

};



///////////////////////////////////////////////////////////////
// List of data extensions
///////////////////////////////////////////////////////////////

class CExtDataList  
{

private :
    CExtensionData *m_pStart, *m_pEnd, *m_pCur, *m_pIter;
    WORD m_wSize;
    WORD m_wSearchStreamNum;

public:
	CExtDataList()
    {
        m_pStart = m_pCur = m_pIter = m_pEnd = NULL;
        m_wSize = 0;
        m_wSearchStreamNum = 0;
    }
    ~CExtDataList()
    {
        while( NULL != m_pStart )
        {
            m_pCur = m_pStart;
            m_pStart = m_pStart->m_pNext;
            delete m_pCur;
        }
    }

public :
    WORD Size() { return m_wSize; };
    HRESULT Create( IWMProfile*    pProfile );
    bool Find( WORD wStreamNum, CExtensionData **pExtensionData );
private :
    bool Append( CExtensionData *pCExtensionData );
    CExtensionData *GetStart() { return m_pStart; };

};

HRESULT SaveProfileToMemory( IWMProfile *pIWMProfile, __deref_out_ecount(*pdwLen) WCHAR ** ppwszBuffer, DWORD *pdwLen );
HRESULT SaveProfileToFile( const TCHAR *pszFileName, IWMProfile *pIWMProfile );

#endif // !defined(AFX_CExtensionData_H__21A57EE3_8C94_4E10_8092_4C171BCBA68E__INCLUDED_)
