// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved

#include "stdafx.h"

#include "tedvis.h"
#include "tededit.h"
#include "commctrl.h"

#include <assert.h>
#include <math.h>

///////////////////////////////////////////////////////////////////////////////
//

BOOL CVisualRect::IsIn(CVisualRect& rect)
{
        return !(   x() > rect.right()
                    || right() < rect.x()
                    || y() > rect.bottom()
                    || bottom() < rect.y() );
}

///////////////////////////////////////////////////////////////////////////////
//
CVisualCoordinateTransform::CVisualCoordinateTransform()
    : m_xScale(1)
    , m_yScale(1)
    , m_xOffset(0)
    , m_yOffset(0)
{
}

POINT CVisualCoordinateTransform::VisualToScreen(CVisualPoint & vis)
{
    POINT pt;
    
    pt.x = LONG((vis.x() + m_xOffset) * m_xScale);
    pt.y = LONG((vis.y() + m_yOffset) * m_yScale);

    return pt;
}

RECT CVisualCoordinateTransform::VisualToScreen(CVisualRect & vis)
{
    RECT rc;
    
    rc.left = LONG((vis.x() + m_xOffset) * m_xScale);
    rc.top = LONG((vis.y() + m_yOffset) * m_yScale);
    rc.right = LONG((vis.right() + m_xOffset) * m_xScale);
    rc.bottom = LONG((vis.bottom() + m_yOffset) * m_yScale);

    return rc;
}

CVisualPoint CVisualCoordinateTransform::ScreenToVisual(POINT & pt)
{
    CVisualPoint vis(pt.x / m_xScale - m_xOffset, pt.y / m_yScale - m_yOffset);

    return vis;
}

void CVisualCoordinateTransform::SetPointOffset(double xOffset, double yOffset)
{
    m_xOffset = xOffset;
    m_yOffset = yOffset;
}

///////////////////////////////////////////////////////////////////////////////
//
CVisualDrawContext::CVisualDrawContext(CVisualCoordinateTransform * pTransform)
    : m_pTransform(pTransform)
{
}

CVisualDrawContext::~CVisualDrawContext()
{
}

BOOL CVisualDrawContext::BeginPaint(HWND hWnd)
{
    m_hWnd = hWnd;
    m_hdc = ::BeginPaint(hWnd, &m_ps);

    RECT rect;
    ::GetClientRect(m_hWnd, &rect);

    m_hBgBuffer = ::CreateCompatibleDC(m_hdc);
    if(NULL == m_hBgBuffer)
    {
        return FALSE;
    }
    
    HBITMAP hBitmap = ::CreateCompatibleBitmap(m_hdc, rect.right - rect.left, rect.bottom - rect.top);
    if(NULL == hBitmap)
    {
        DeleteObject(m_hBgBuffer);
        return FALSE;
    }

    m_hOldBitmap = (HBITMAP) SelectObject(m_hBgBuffer, hBitmap);
    if(NULL == m_hOldBitmap)
    {
        DeleteObject(hBitmap);
        DeleteObject(m_hBgBuffer);
        return FALSE;
    }

    HBRUSH hWhiteBrush = ::CreateSolidBrush(RGB(255,255,255));
    if(NULL == hWhiteBrush)
    {
        SelectObject(m_hBgBuffer, m_hOldBitmap);
        DeleteObject(hBitmap);
        DeleteObject(m_hBgBuffer);
        return FALSE;
    }
    
    ::FillRect(m_hBgBuffer, &rect, hWhiteBrush);
    DeleteObject(hWhiteBrush);
    
    return TRUE;
}

void CVisualDrawContext::EndPaint()
{
    RECT rect;
    ::GetClientRect(m_hWnd, &rect);

    ::BitBlt(m_hdc, rect.left, rect.top, rect.right - rect.left, rect.bottom - rect.top, m_hBgBuffer, 0, 0, SRCCOPY);
    ::EndPaint(m_hWnd, &m_ps);

    HBITMAP hBitmap = (HBITMAP) SelectObject(m_hBgBuffer, m_hOldBitmap);
    DeleteObject(hBitmap);
    DeleteDC(m_hBgBuffer);
    
}

void CVisualDrawContext::PushState()
{
    m_StateStack.AddHead(m_State);

    // cleanup brush. hdc should have selected brush. 
    // when we pop state and m_hOldBrush is not NULL - select old brush
    m_State.m_hNewBrush = NULL;
    m_State.m_hOldBrush = NULL;
    m_State.m_hNewPen = NULL;
    m_State.m_hOldPen = NULL;
}

void CVisualDrawContext::PopState()
{
    if(m_State.m_hOldBrush)
    {
        SelectObject(m_hBgBuffer, m_State.m_hOldBrush);
    }

    if(m_State.m_hNewBrush)
    {
        DeleteObject(m_State.m_hNewBrush);
    }
    
    if(m_State.m_hOldPen)
    {
        SelectObject(m_hBgBuffer, m_State.m_hOldPen);
    }
    
    if(m_State.m_hNewPen)
    {
        DeleteObject(m_State.m_hNewPen);
    }
    
    m_State = m_StateStack.RemoveHead();
}

BOOL CVisualDrawContext::SelectSmallFont() 
{
    CAtlString strFontSize = LoadAtlString(IDS_FONT_SIZE_12);
    CAtlString strFontFace = LoadAtlString(IDS_FONT_FACE_ARIAL);

    LOGFONT lfLabelFont;
    lfLabelFont.lfHeight = _wtol(strFontSize);
    lfLabelFont.lfWidth = 0;
    lfLabelFont.lfEscapement = 0;
    lfLabelFont.lfOrientation = 0;
    lfLabelFont.lfWeight = FW_DONTCARE;
    lfLabelFont.lfItalic = FALSE;
    lfLabelFont.lfUnderline = FALSE;
    lfLabelFont.lfStrikeOut = FALSE;
    lfLabelFont.lfCharSet = DEFAULT_CHARSET;
    lfLabelFont.lfOutPrecision = OUT_DEFAULT_PRECIS;
    lfLabelFont.lfClipPrecision = CLIP_DEFAULT_PRECIS;
    lfLabelFont.lfQuality = DEFAULT_QUALITY;
    lfLabelFont.lfPitchAndFamily = FF_DONTCARE | DEFAULT_PITCH;
    wcscpy_s(lfLabelFont.lfFaceName, 32, strFontFace);
    
    HFONT hSmallFont = CreateFontIndirect(&lfLabelFont);
    if(NULL == hSmallFont)
    {
        return FALSE;
    }
    
    m_State.m_hOldFont = SelectObject(m_hBgBuffer, hSmallFont);
    if(NULL == m_State.m_hOldFont)
    {
        DeleteObject(hSmallFont);
        return FALSE;
    }
    
    return true;
}

void CVisualDrawContext::DeselectSmallFont() 
{
    HGDIOBJ hFont = SelectObject(m_hBgBuffer, m_State.m_hOldFont);
    DeleteObject(hFont);
}

BOOL CVisualDrawContext::SelectPen(COLORREF color, int width)
{
    HPEN hOldPen;
    
    HPEN hNewPen = CreatePen(PS_SOLID, width, color);
    if(NULL == hNewPen)
    {
        return FALSE;
    }

    hOldPen  = (HPEN)SelectObject(m_hBgBuffer, hNewPen);
    if(NULL == hOldPen)
    {
        DeleteObject(hNewPen);
        return FALSE;
    }

    // if we already have old - keep previous
    if(!m_State.m_hOldPen)
    {
        m_State.m_hOldPen = hOldPen;
    }
    
    if(m_State.m_hNewPen)
    {
        DeleteObject(m_State.m_hNewPen);
    }

    m_State.m_hNewPen = hNewPen;
    
    return TRUE;
}

BOOL CVisualDrawContext::SelectSolidBrush(COLORREF color)
{
    HBRUSH hOldBrush;
    
    HBRUSH hNewBrush = CreateSolidBrush(color);
    if(NULL == hNewBrush)
    {
        return FALSE;
    }
    
    hOldBrush  = (HBRUSH)SelectObject(m_hBgBuffer, hNewBrush);
    if(NULL == hOldBrush)
    {
        DeleteObject(hNewBrush);
        return FALSE;
    }

    // if we already have old - keep previous
    if(!m_State.m_hOldBrush)
    {
        m_State.m_hOldBrush = hOldBrush;
    }
    
    if(m_State.m_hNewBrush)
    {
        DeleteObject(m_State.m_hNewBrush);
    }

    m_State.m_hNewBrush = hNewBrush;
    
    return TRUE;
}

void CVisualDrawContext::MapPoint(CVisualPoint & vis, POINT & disp)
{
    CVisualPoint v(vis);

    v.Add(m_State.m_xCoord, m_State.m_yCoord);
    disp = m_pTransform->VisualToScreen(v);
}

void CVisualDrawContext::MapRect(CVisualRect & vis, RECT & disp)
{
    CVisualRect v(vis);
    
    v.Add(m_State.m_xCoord, m_State.m_yCoord);
    disp = m_pTransform->VisualToScreen(v);
   
}

void CVisualDrawContext::ShiftCoordinates(double xOffset, double yOffset)
{
    m_State.m_xCoord += xOffset;
    m_State.m_yCoord += yOffset;
}

///////////////////////////////////////////////////////////////////////////////
//

void CVisualObject::Select(bool bIsSelected) 
{
    m_bIsSelected = bIsSelected;
}

bool CVisualObject::ContainsVisual(CVisualObject* pVisual)
{
    if(this == pVisual)
    {
        return true;
    }

    return false;
}

///////////////////////////////////////////////////////////////////////////////
//

// draw connector
void CVisualConnector::Draw(CVisualDrawContext & Ctx)
{
    // set color
    if(IsSelected())
    {
        Ctx.SelectPen(m_clrSelected, 2);
    }
    else
    {
        Ctx.SelectPen(m_clrLine, 1);
    }

    POINT left, right;
    Ctx.MapPoint(m_Left, left);
    Ctx.MapPoint(m_Right, right);
    
    MoveToEx(Ctx.DC(), left.x, left.y, NULL);
    LineTo(Ctx.DC(), right.x, right.y);
}

CVisualObject::CONNECTION_TYPE CVisualConnector::GetConnectionType() const 
{
    return CVisualObject::NONE;
}

void CVisualConnector::NotifyRemoved(CVisualObject* removed) 
{

}

bool CVisualConnector::IsDependent(CVisualObject* pOtherObj) const 
{
    return false;
}

BOOL CVisualConnector::HitTest(CVisualPoint & pt, CVisualObject ** ppObj)
{
    double alpha = m_Right.x() - m_Left.x();
    double beta = m_Right.y() - m_Left.y();

    double t1 = (pt.x() - m_Left.x()) / alpha;
    double t2 = (pt.y() - m_Left.y()) / beta;

    if(fabs(t1 - t2) < .1 && t1 >= 0 && t1 <= 1)
    {
        *ppObj = this;
        return true;
    }

    return false;
}

///////////////////////////////////////////////////////////////////////////////
//

const int CVisualPin::LABEL_OFFSET_X = 30;
const int CVisualPin::LABEL_OFFSET_Y = 2;

CVisualPin::CVisualPin(CVisualComponent * pOwner, CVisualRect & Rect, CVisualObject::CONNECTION_TYPE connType, const CAtlStringW& strLabel, int nPinId)
    : CVisualObject(Rect)
    , m_ConnType(connType)
    , m_pOwner(pOwner) 
    , m_pConnector(NULL)
    , m_strLabel(strLabel)
    , m_nPinId(nPinId)
    , m_fHighlight(false)
{
    m_Type = PIN;
}

CVisualPoint CVisualPin::GetConnectorPoint()
{
    CVisualPoint pt(m_pOwner->Rect().x(), m_pOwner->Rect().y());

    if(m_ConnType == INPUT)
    {
        pt.Add(Rect().x(), Rect().y() + Rect().h() / 2);
    }
    else
    {
        pt.Add(Rect().right(), Rect().y() + Rect().h() / 2);
    }

    return pt;
}

void CVisualPin::Draw(CVisualDrawContext & Ctx)
{
    RECT rect;

    if(m_fHighlight)
    {
        Ctx.SelectPen(RGB(0, 0, 200), 2);
    }
    else
    {
        Ctx.SelectPen(RGB(0, 0, 0), 1);
    }
    Ctx.SelectSolidBrush(RGB(0, 0, 0));
        
    // draw rect
    Ctx.MapRect(m_Rect, rect);
    Rectangle(Ctx.DC(), rect.left, rect.top, rect.right, rect.bottom);

    rect.left -= LABEL_OFFSET_X;
    rect.top -= LABEL_OFFSET_Y;
    rect.bottom += LABEL_OFFSET_Y;

    DrawText(Ctx.DC(), m_strLabel, -1, &rect, 0);
}

CVisualObject::CONNECTION_TYPE CVisualPin::GetConnectionType() const 
{
    return m_ConnType;
}

int CVisualPin::GetPinId() const 
{
    return m_nPinId;
}

void CVisualPin::NotifyRemoved(CVisualObject* removed) 
{
    if(removed == m_pConnector) 
    {
        m_pConnector = NULL;
    }
}

void CVisualPin::Select(bool selected) 
{
    CVisualObject::Select(selected);

    if(m_pConnector) 
    {
        m_pConnector->Select(selected);
    }
}

bool CVisualPin::IsDependent(CVisualObject* pOtherObj) const 
{
    if(pOtherObj == m_pConnector) 
    {
        return true;
    }

    return false;
}

///////////////////////////////////////////////////////////////////////////////
//

CVisualComponent::CVisualComponent(CVisualRect& Rect)
    : CVisualObject(Rect)
{
}

CVisualComponent::~CVisualComponent() 
{
}

CVisualObject::CONNECTION_TYPE CVisualComponent::GetConnectionType() const 
{
	return CVisualObject::NONE;
}

////////////////////////////////////////////////////////////////////////////
//

CCommandHandler* CVisualNode::ms_pPinHandler = NULL;

CVisualNode::CVisualNode(const CAtlStringW& strLabel, bool fAutoInserted) 
    : CVisualComponent(CVisualRect(0, 0, COMP_DEF_WIDTH, COMP_DEF_HEIGHT))
	, m_strLabel(strLabel)
	, m_fTopoError(false)
{
    if(fAutoInserted)
    {
        m_clrFill = RGB(153, 153, 243);
    }
    else 
    {
        m_clrFill = RGB(153, 243, 153);
    }
    
    m_clrLine = RGB(0, 0, 0);
    m_clrSelectedBorder = RGB(0, 0, 200);
    m_clrErrorText = RGB(200, 0, 0);
        
    m_Type = CVisualObject::NODE;
}

CVisualNode::~CVisualNode()
{
   for(size_t i = 0; i < m_InputPins.GetCount(); i++)
    {
        delete m_InputPins.GetAt(i);
    }

    for(size_t i = 0; i < m_OutputPins.GetCount(); i++)
    {
        delete m_OutputPins.GetAt(i);
    }
}

void CVisualNode::Draw(CVisualDrawContext & Ctx)
{
    RECT rect;
    size_t n;

    Ctx.SelectSmallFont();

    if(IsSelected())
    {
        Ctx.SelectPen(m_clrSelectedBorder, 2);
    }
    else
    {
        Ctx.SelectPen(m_clrLine, 1);
    }

    // select colors
    Ctx.SelectSolidBrush(m_clrFill);

    // draw rect
    Ctx.MapRect(m_Rect, rect);
    Rectangle(Ctx.DC(), rect.left, rect.top, rect.right, rect.bottom);

    if(!m_strLabel.IsEmpty()) 
    {
        COLORREF oldColor = SetTextColor(Ctx.DC(), m_clrLine);
        Ctx.SelectSolidBrush(RGB(0, 0, 0));
        SetBkColor(Ctx.DC(), m_clrFill);

        rect.left += 5;
        rect.right -= 5;
        rect.top +=5;
        
        DrawText(Ctx.DC(), m_strLabel, m_strLabel.GetLength(), &rect, DT_WORDBREAK);

        SetTextColor(Ctx.DC(), oldColor);
    }

    if(m_fTopoError)
    {
        COLORREF oldColor = SetTextColor(Ctx.DC(), m_clrErrorText);
        rect.top += 10;

        CAtlStringW errString = LoadAtlString(IDS_E_TOPO_RESOLUTION);
        DrawText(Ctx.DC(), errString, errString.GetLength(), &rect, DT_WORDBREAK);

        SetTextColor(Ctx.DC(), oldColor);
    }

    Ctx.PushState();

    Ctx.ShiftCoordinates(m_Rect.x(), m_Rect.y());
    
    // draw pins
    for(n = 0; n < m_InputPins.GetCount(); n++)
    {
        m_InputPins.GetAt(n)->Draw(Ctx);
    }

    for(n = 0; n < m_OutputPins.GetCount(); n++)
    {
        m_OutputPins.GetAt(n)->Draw(Ctx);
    }
    
    Ctx.PopState();

    Ctx.DeselectSmallFont();
}

bool CVisualNode::IsDependent(CVisualObject* pOtherObj) const 
{
    for(size_t i = 0; i < m_InputPins.GetCount(); i++)
    {
        if(m_InputPins.GetAt(i)->IsDependent(pOtherObj)) return true;
    }

    for(size_t i = 0; i < m_OutputPins.GetCount(); i++)
    {
        if(m_OutputPins.GetAt(i)->IsDependent(pOtherObj)) return true;
    }
    return false;
}

void CVisualNode::NotifyRemoved(CVisualObject* removed) 
{
    for(size_t i = 0; i < m_InputPins.GetCount(); i++) 
    {
        m_InputPins.GetAt(i)->NotifyRemoved(removed);
    }

    for(size_t i = 0; i < m_OutputPins.GetCount(); i++) 
    {
        m_OutputPins.GetAt(i)->NotifyRemoved(removed);
    }
}

BOOL CVisualNode::HitTest(CVisualPoint & pt, CVisualObject ** ppObj)
{
    size_t n;
    CVisualPoint ptLocal(pt);

    ptLocal.Sub(m_Rect.x(), m_Rect.y());

    // first check if we hit any pin
    for(n = 0; n < m_InputPins.GetCount(); n++)
    {
        if(m_InputPins.GetAt(n)->Rect().IsIn(ptLocal))
        {
            *ppObj = m_InputPins.GetAt(n);
            return TRUE;
        }
    }

    for(n = 0; n < m_OutputPins.GetCount(); n++)
    {
        if(m_OutputPins.GetAt(n)->Rect().IsIn(ptLocal))
        {
            *ppObj = m_OutputPins.GetAt(n);
            return TRUE;
        }
    }
    
    if(m_Rect.IsIn(pt))
    {   
        (*ppObj) = this;
        return TRUE;
    }

    return FALSE;
}

void CVisualNode::Move(double x, double y)
{
    size_t n;
    CVisualPin * pPin;

    CVisualObject::Move(x, y);

    for(n = 0; n < m_InputPins.GetCount(); n++)
    {
        pPin = m_InputPins.GetAt(n);
        
        if(pPin->GetConnector())
        {
            pPin->GetConnector()->Right() = pPin->GetConnectorPoint();
        }
    }

     for(n = 0; n < m_OutputPins.GetCount(); n++)
    {
        pPin = m_OutputPins.GetAt(n);
        
        if(pPin->GetConnector())
        {
           pPin->GetConnector()->Left() = pPin->GetConnectorPoint();
        }
    }
}

int CVisualNode::GetPinIDWithConnector(CVisualConnector* pConnector)
{
    for(size_t i = 0; i < m_OutputPins.GetCount(); i++)
    {
        CVisualPin* pPin = m_OutputPins.GetAt(i);
        if(pPin->GetConnector() == pConnector)
        {
            return pPin->GetPinId();
        }
    }

    return -1;
}

void CVisualNode::SetPinHandler(CCommandHandler* pHandler)
{
    ms_pPinHandler = pHandler;
}

CVisualPin* CVisualNode::AddPin(bool fInput, LONG_PTR pData, const CAtlStringW& strLabel, int nPinId)
{
    HRESULT hr;
    CVisualPin* pPin = new CVisualPin(this, 
                            CVisualRect(0, 0, PIN_WIDTH, PIN_HEIGHT), 
                            (fInput) ? CVisualObject::INPUT : CVisualObject::OUTPUT, strLabel, nPinId);
    CHECK_ALLOC( pPin );
    
    pPin->SetData(pData);

    if(fInput)
    {
        m_InputPins.Add(pPin);
    }
    else
    {
        m_OutputPins.Add(pPin);
    }

    pPin->SetHandler(ms_pPinHandler);
   
    RecalcPins();

Cleanup:
    return pPin;
}

CVisualPin* CVisualNode::GetInputPinByIndex(size_t nIndex)
{
    return m_InputPins.GetAt(nIndex);
}

CVisualPin* CVisualNode::GetOutputPinByIndex(size_t nIndex)
{
    return m_OutputPins.GetAt(nIndex);
}


CVisualPin* CVisualNode::GetInputPin(int nPinID)
{
    for(size_t i = 0; i < m_InputPins.GetCount(); i++)
    {
        if(m_InputPins.GetAt(i)->GetPinId() == nPinID) return m_InputPins.GetAt(i);
    }

    return NULL;
}

CVisualPin* CVisualNode::GetOutputPin(int nPinID)
{
    for(size_t i = 0; i < m_OutputPins.GetCount(); i++)
    {
        if(m_OutputPins.GetAt(i)->GetPinId() == nPinID) return m_OutputPins.GetAt(i);
    }

    return NULL;
}

void CVisualNode::FlagTopoLoadError(size_t nIndex, bool fError)
{
    assert(nIndex == 0);

    m_fTopoError = fError;
}

void CVisualNode::RecalcPins()
{
    size_t nInputs = 0;
    size_t nOutputs = 0;

    // calculate number of inputs and outputs
    nInputs = m_InputPins.GetCount();
    nOutputs = m_OutputPins.GetCount();

    // position pins
    PositionPins(nInputs, CVisualPin::INPUT);
    PositionPins(nOutputs, CVisualPin::OUTPUT);
}

void CVisualNode::PositionPins(size_t nPins, CVisualObject::CONNECTION_TYPE Dir)
{
    size_t n;

    if(nPins == 0)
    {
        return;
    }

    // TODO: resize if necessary

    double yCur;
    double x;

    yCur = (m_Rect.h() - (nPins * PIN_HEIGHT) - ((nPins - 1) * PIN_INTERVAL)) / 2;
    x = (Dir == CVisualObject::INPUT) ? -PIN_WIDTH : m_Rect.w();

    // calculate number of inputs and outputs
    for(n = 0; n < m_InputPins.GetCount(); n++)
    {
        if(m_InputPins.GetAt(n)->GetConnectionType() == Dir)
        {
            m_InputPins.GetAt(n)->Rect() = CVisualRect(x, yCur, PIN_WIDTH, PIN_HEIGHT);
            yCur += PIN_HEIGHT + PIN_INTERVAL;
        }
    }

    for(n = 0; n < m_OutputPins.GetCount(); n++)
    {
        if(m_OutputPins.GetAt(n)->GetConnectionType() == Dir)
        {
            m_OutputPins.GetAt(n)->Rect() = CVisualRect(x, yCur, PIN_WIDTH, PIN_HEIGHT);
            yCur += PIN_HEIGHT + PIN_INTERVAL;
        }
    }
}

///////////////////////////////////////////////////////////////////////////////
//

CVisualContainer::CVisualContainer(const CAtlStringW& label)
   : CVisualComponent(CVisualRect(0, 0, COMP_DEF_WIDTH, BOTTOM_MARGIN))
   , m_strLabel(label)
   , m_clrFill(RGB(192, 192, 192))
{
    m_Type = CVisualObject::CONTAINER;
}

CVisualContainer::~CVisualContainer()
{
    for(size_t i = 0; i < m_arrComponents.GetCount(); i++)
    {
        delete m_arrComponents.GetAt(i);
    }
}

void CVisualContainer::Draw(CVisualDrawContext& Ctx)
{
    RECT rect;

    Ctx.SelectSmallFont();

    // select colors
    Ctx.SelectSolidBrush(m_clrFill);

    // draw rect
    Ctx.MapRect(m_Rect, rect);
    Rectangle(Ctx.DC(), rect.left, rect.top, rect.right, rect.bottom);

    if(!m_strLabel.IsEmpty()) 
    {
        Ctx.SelectSolidBrush(RGB(0, 0, 0));
        SetBkColor(Ctx.DC(), m_clrFill);

        rect.left += 5;
        rect.right -= 5;
        rect.top +=5;
        
        DrawText(Ctx.DC(), m_strLabel, m_strLabel.GetLength(), &rect, DT_WORDBREAK);
    }

    Ctx.DeselectSmallFont();
    
    for(size_t i = 0; i < m_arrComponents.GetCount(); i++)
    {
        Ctx.PushState();
        m_arrComponents.GetAt(i)->Draw(Ctx);
        Ctx.PopState();
    }
}

BOOL CVisualContainer::HitTest(CVisualPoint& pt, CVisualObject** ppObj)
{
    for(size_t i = 0; i < m_arrComponents.GetCount(); i++)
    {
        if(m_arrComponents.GetAt(i)->HitTest(pt, ppObj))
        {
            return TRUE;
        }
    }

    return FALSE;
}

void CVisualContainer::Move(double x, double y)
{
    CVisualObject::Move(x, y);

    RecalcPositions();
}

void CVisualContainer::AddPosition(double x, double y)
{
    CVisualObject::AddPosition(x, y);

    RecalcPositions();
}

void CVisualContainer::NotifyRemoved(CVisualObject* removed)
{
    for(size_t i = 0; i < m_arrComponents.GetCount(); i++)
    {
        m_arrComponents.GetAt(i)->NotifyRemoved(removed);
    }
}

bool CVisualContainer::IsDependent(CVisualObject* pOtherObj) const
{
    for(size_t i = 0; i < m_arrComponents.GetCount(); i++)
    {
        if(m_arrComponents.GetAt(i) == pOtherObj) return true;
        if(m_arrComponents.GetAt(i)->IsDependent(pOtherObj)) return true;
    }

    return false;
}

void CVisualContainer::AddComponent(CVisualComponent* pComponent)
{
    pComponent->SetContainer(this);

    m_arrComponents.Add(pComponent);

    m_Rect.Expand(0, pComponent->Rect().h() + TOP_MARGIN);

    RecalcPositions();
}

DWORD CVisualContainer::GetComponentCount()
{
    return (DWORD) m_arrComponents.GetCount();
}

CVisualComponent* CVisualContainer::GetComponent(DWORD dwIndex)
{
    return m_arrComponents.GetAt(dwIndex);
}

void CVisualContainer::SetParent(CVisualContainer* pParent)
{
    m_pParent = pParent;
}

void CVisualContainer::SetHandler(CCommandHandler* pHandler)
{
    CVisualObject::SetHandler(pHandler);

    for(size_t i = 0; i < m_arrComponents.GetCount(); i++)
    {
        m_arrComponents.GetAt(i)->SetHandler(pHandler);
    }
}

void CVisualContainer::RecalcPositions()
{
    double dblHeightSoFar = 0;
    for(size_t i = 0; i < m_arrComponents.GetCount(); i++)
    {
        m_arrComponents.GetAt(i)->Move(m_Rect.x(), m_Rect.y() + (TOP_MARGIN * (i + 1)) + (dblHeightSoFar * i));
        dblHeightSoFar = m_arrComponents.GetAt(i)->Rect().h();
    }
}

CVisualPin* CVisualContainer::GetInputPinByIndex(size_t nIndex)
{
    size_t cPinsSoFar = 0;
    
    for(size_t i = 0; i < m_arrComponents.GetCount(); i++)
    {
        size_t cPins = m_arrComponents.GetAt(i)->GetInputPinCount();

        if(nIndex < cPinsSoFar + cPins)
        {
            return m_arrComponents.GetAt(i)->GetInputPinByIndex(nIndex - cPinsSoFar);
        }

        cPinsSoFar += cPins;
    }

    return NULL;
}

CVisualPin* CVisualContainer::GetOutputPinByIndex(size_t nIndex)
{
    size_t cPinsSoFar = 0;
    
    for(size_t i = 0; i < m_arrComponents.GetCount(); i++)
    {
        size_t cPins = m_arrComponents.GetAt(i)->GetOutputPinCount();

        if(nIndex < cPinsSoFar + cPins)
        {
            return m_arrComponents.GetAt(i)->GetOutputPinByIndex(nIndex - cPinsSoFar);
        }

        cPinsSoFar += cPins;
    }

    return NULL;
}

size_t CVisualContainer::GetInputPinCount()
{
    size_t cTotal = 0;

    for(size_t i = 0; i < m_arrComponents.GetCount(); i++)
    {
        cTotal += m_arrComponents.GetAt(i)->GetInputPinCount();
    }

    return cTotal;
}

size_t CVisualContainer::GetOutputPinCount()
{
    size_t cTotal = 0;

    for(size_t i = 0; i < m_arrComponents.GetCount(); i++)
    {
        cTotal += m_arrComponents.GetAt(i)->GetOutputPinCount();
    }

    return cTotal;
}

CVisualPin* CVisualContainer::GetInputPin(int nPinID)
{
    for(size_t i = 0; i < m_arrComponents.GetCount(); i++)
    {
        CVisualPin* pPin = m_arrComponents.GetAt(i)->GetInputPin(nPinID);
        if(pPin)
        {
            return pPin;
        }
    }

    return NULL;
}

CVisualPin* CVisualContainer::GetOutputPin(int nPinID)
{
    for(size_t i = 0; i < m_arrComponents.GetCount(); i++)
    {
        CVisualPin* pPin = m_arrComponents.GetAt(i)->GetOutputPin(nPinID);
        if(pPin)
        {
            return pPin;
        }
    }

    return NULL;
}

void CVisualContainer::FlagTopoLoadError(size_t nIndex, bool fError)
{
    assert(nIndex < m_arrComponents.GetCount());

    m_arrComponents.GetAt(nIndex)->FlagTopoLoadError(0, fError);
}

bool CVisualContainer::ContainsVisual(CVisualObject* pVisual)
{
    if(CVisualComponent::ContainsVisual(pVisual))
    {
        return true;
    }

    for(size_t i = 0; i < m_arrComponents.GetCount(); i++)
    {
        if(m_arrComponents.GetAt(i)->ContainsVisual(pVisual))
        {
            return true;
        }
    }

    return false;
}

///////////////////////////////////////////////////////////////////////////////
//

CVisualTree::CVisualTree()
    : m_pEventHandler(NULL)
{
}
CVisualTree::~CVisualTree() 
{
    for(size_t i = 0; i < m_Objects.GetCount(); i++) {
        delete m_Objects.GetAt(i);
    }
}

HRESULT CVisualTree::AddVisual(CVisualObject * pVisual, bool fEnsureOpenSpace)
{
    assert(pVisual != NULL);

    m_Objects.Add(pVisual);

    if(fEnsureOpenSpace)
    {
        while(IsOccupied(pVisual))
        {
            pVisual->AddPosition(120, 0);

            if(pVisual->Rect().x() > 600)
            {
                pVisual->Move(20, pVisual->Rect().y() + 120);
            }
        }
    }
    
    return S_OK;
}

void CVisualTree::RemoveVisual(CVisualObject * pVisual)
{
    size_t n;
    CVisualObject * pIter;

    for(n = 0; n < m_Objects.GetCount(); n++)
    {
        pIter = m_Objects.GetAt(n);

        if(pVisual->IsDependent(pIter)) 
        {
            RemoveVisual(pIter);
            n--;
        }
        else if(pIter == pVisual)
        {
            m_Objects.RemoveAt(n);
            n--;
        }
        else
        {
            pIter->NotifyRemoved(pVisual);  
        }
    }

    if(m_pEventHandler) 
    {
        m_pEventHandler->NotifyObjectDeleted(pVisual);
    }

    delete pVisual;
}

void CVisualTree::Draw(CVisualDrawContext & Ctx)
{
    size_t n;
    CVisualObject * pObject;

    if(m_Objects.IsEmpty()) return;
    
    for(n = m_Objects.GetCount() - 1; n > 0; n--)
    {
        Ctx.PushState();
        pObject = m_Objects.GetAt(n);
        pObject->Draw(Ctx);
        Ctx.PopState();
    }

    Ctx.PushState();
    m_Objects.GetAt(0)->Draw(Ctx);
    Ctx.PopState();
}

BOOL CVisualTree::HitTest(CVisualPoint & pt, CVisualObject ** ppObj)
{
    size_t n;
    CVisualObject * pObject;

    for(n = 0; n < m_Objects.GetCount(); n++)
    {
        pObject = m_Objects.GetAt(n);
        if(pObject->HitTest(pt, ppObj))
        {
            return TRUE;
        }
    }

    return FALSE;
}

void CVisualTree::RouteAllConnectors()
{
    size_t n;
    CVisualObject * pObject;

    for(n = 0; n < m_Objects.GetCount(); n++)
    {
        pObject = m_Objects.GetAt(n);
        if(pObject->Type() == CVisualObject::NODE)
        {
            CVisualNode* pNode = (CVisualNode*) pObject;
            for(size_t i = 0; i < pNode->GetOutputPinCount(); i++)
            {
                CVisualPin* pPin = pNode->GetOutputPinByIndex((DWORD) i);
                if(pPin->GetConnector())
                {
                    pPin->GetConnector()->Left() = pPin->GetConnectorPoint();
                }
            }

            for(size_t i = 0; i < pNode->GetInputPinCount(); i++)
            {
                CVisualPin* pPin = pNode->GetInputPinByIndex((DWORD) i);
                if(pPin->GetConnector())
                {
                    pPin->GetConnector()->Right() = pPin->GetConnectorPoint();
                }
            }
        }
    }
}

BOOL CVisualTree::MakeConnector(CVisualPin* pSourcePin, CVisualPin* pSinkPin)
{
    HRESULT hr = S_OK;
    
    CVisualConnector* pVisualConnector = new CVisualConnector;
    CHECK_ALLOC( pVisualConnector );
    
    AddVisual(pVisualConnector);
        
    pVisualConnector->Left() = pSourcePin->GetConnectorPoint();
    pVisualConnector->Right() = pSinkPin->GetConnectorPoint();
    pSourcePin->SetConnector(pVisualConnector);
    pSinkPin->SetConnector(pVisualConnector);
    
Cleanup:
    if(FAILED(hr))
    {
        return FALSE;
    }
    
    return TRUE;
}

void CVisualTree::GetMaxExtent(UINT32& maxXExtent, UINT32& maxYExtent)
{
    for(size_t n = 0; n < m_Objects.GetCount(); n++)
    {
        CVisualObject* pObj = m_Objects.GetAt(n);

        UINT32 objXExtent = UINT32(pObj->Rect().x() + pObj->Rect().w() + 10);
        UINT32 objYExtent = UINT32(pObj->Rect().y() + pObj->Rect().h() + 10);

        if(objXExtent > maxXExtent) maxXExtent = objXExtent;
        if(objYExtent > maxYExtent) maxYExtent = objYExtent;
    }
}

bool CVisualTree::IsOccupied(CVisualObject* pObj)
{
    for(size_t n = 0; n < m_Objects.GetCount(); n++)
    {
        CVisualObject* pObj2 = m_Objects.GetAt(n);

        if( pObj != pObj2 && pObj->Rect().IsIn(pObj2->Rect()) ) return true;
    }

    return false;
}

