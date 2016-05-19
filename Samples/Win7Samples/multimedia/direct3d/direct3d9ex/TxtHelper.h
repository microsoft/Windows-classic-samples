//--------------------------------------------------------------------------------------
// File: TxtHelper.h
//
// Text helper class for drawing txt
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//--------------------------------------------------------------------------------------

#include <windows.h>
#include <strsafe.h>

class CTextHelper
{
public:
    CTextHelper( ID3DXFont* pFont, ID3DXSprite* pSprite, int nLineHeight );

    void SetInsertionPos( int x, int y ) { m_pt.x = x; m_pt.y = y; }
    void SetForegroundColor( D3DXCOLOR clr ) { m_clr = clr; }

    void Begin();
    HRESULT DrawFormattedTextLine( const WCHAR* strMsg, ... );
    HRESULT DrawTextLine( const WCHAR* strMsg );
    HRESULT DrawFormattedTextLine( RECT &rc, DWORD dwFlags, const WCHAR* strMsg, ... );
    HRESULT DrawTextLine( RECT &rc, DWORD dwFlags, const WCHAR* strMsg );
    void End();

protected:
    ID3DXFont*   m_pFont;
    ID3DXSprite* m_pSprite;
    D3DXCOLOR    m_clr;
    POINT        m_pt;
    int          m_nLineHeight;
};

//--------------------------------------------------------------------------------------
CTextHelper::CTextHelper( ID3DXFont* pFont, ID3DXSprite* pSprite, int nLineHeight )
{
    m_pFont = pFont;
    m_pSprite = pSprite;
    m_clr = D3DXCOLOR(1,1,1,1);
    m_pt.x = 0; 
    m_pt.y = 0; 
    m_nLineHeight = nLineHeight;
}


//--------------------------------------------------------------------------------------
HRESULT CTextHelper::DrawFormattedTextLine( const WCHAR* strMsg, ... )
{
    WCHAR strBuffer[512];
    
    va_list args;
    va_start(args, strMsg);
    StringCchVPrintf( strBuffer, 512, strMsg, args );
    strBuffer[511] = L'\0';
    va_end(args);

    return DrawTextLine( strBuffer );
}


//--------------------------------------------------------------------------------------
HRESULT CTextHelper::DrawTextLine( const WCHAR* strMsg )
{
    if( NULL == m_pFont ) 
        return E_INVALIDARG;

    HRESULT hr;
    RECT rc;
    SetRect( &rc, m_pt.x, m_pt.y, 0, 0 ); 
    hr = m_pFont->DrawText( m_pSprite, strMsg, -1, &rc, DT_NOCLIP, m_clr );
    if( FAILED(hr) )
        return hr;

    m_pt.y += m_nLineHeight;

    return S_OK;
}


HRESULT CTextHelper::DrawFormattedTextLine( RECT &rc, DWORD dwFlags, const WCHAR* strMsg, ... )
{
    WCHAR strBuffer[512];
    
    va_list args;
    va_start(args, strMsg);
    StringCchVPrintf( strBuffer, 512, strMsg, args );
    strBuffer[511] = L'\0';
    va_end(args);

    return DrawTextLine( rc, dwFlags, strBuffer );
}


HRESULT CTextHelper::DrawTextLine( RECT &rc, DWORD dwFlags, const WCHAR* strMsg )
{
    if( NULL == m_pFont ) 
        return E_INVALIDARG;

    HRESULT hr;
    hr = m_pFont->DrawText( m_pSprite, strMsg, -1, &rc, dwFlags, m_clr );
    if( FAILED(hr) )
        return hr;

    m_pt.y += m_nLineHeight;

    return S_OK;
}


//--------------------------------------------------------------------------------------
void CTextHelper::Begin()
{
    if( m_pSprite )
        m_pSprite->Begin( D3DXSPRITE_ALPHABLEND | D3DXSPRITE_SORT_TEXTURE );
}
void CTextHelper::End()
{
    if( m_pSprite )
        m_pSprite->End();
}