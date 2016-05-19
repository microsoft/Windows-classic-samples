// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved

#ifndef __TEDVIS__
#define __TEDVIS__

#include "tedutil.h"

class CVisualObject;
class CVisualConnector;
class CVisualComponent;
class CVisualTree;
class CCommandHandler;
class CMoveComponentHandler;
class CConnectPinHandler;
class CVisualPin;
class CVisualContainer;

///////////////////////////////////////////////////////////////////////////////
// all coordinates are normalized between 0.0 and 1.0
//
class CVisualPoint
{
public:
	CVisualPoint() 
		: m_x(0)
		, m_y(0)
	{;
	}

	CVisualPoint(double x, double y) 
		: m_x(x)
		, m_y(y)
	{;
	}

	double x() { return m_x; }
	double y() { return m_y; }

	void Move(double x, double y) { m_x = x; m_y = y; }
	void Add(double x, double y) { m_x += x; m_y += y; }
	void Sub(double x, double y) { m_x -= x; m_y -= y; }

	bool operator!=(const CVisualPoint& rhs) {
		if(m_x != rhs.m_x || m_y != rhs.m_y) return true;

		return false;
	}

private:

	double m_x;
	double m_y;
};

class CVisualSize
{
public:
	double m_cx;
	double m_cy;
};

class CVisualRect
{
public:
    CVisualRect() 
    	: m_x(0)
    	, m_y(0)
    	, m_w(0)
    	, m_h(0)
    {;
    }

    CVisualRect(double x, double y, double w, double h) 
    	: m_x(x)
    	, m_y(y)
    	, m_w(w)
    	, m_h(h)
    {;
    }

    double x() { return m_x; }
    double y() { return m_y; }
    double w() { return m_w; }
    double h() { return m_h; }
    double right() { return m_x + m_w; }
    double bottom() { return m_y + m_h; }

    void Move(double x, double y) { m_x = x; m_y = y; }
    void Add(double x, double y) { m_x += x; m_y += y; }
    void Expand(double w, double h) { m_w += w; m_h += h; }
    BOOL IsIn(CVisualPoint & pt) { return (pt.x() >= m_x && pt.y() >= m_y && pt.x() <= right() && pt.y() <= bottom()); }
    BOOL IsIn(CVisualRect& rect);
	
private:
	double m_x;
	double m_y;
	double m_w;
	double m_h;
};

///////////////////////////////////////////////////////////////////////////////
// 
class CVisualCoordinateTransform
{
public:
    CVisualCoordinateTransform();
    
    POINT VisualToScreen(CVisualPoint & vis);
    RECT VisualToScreen(CVisualRect & vis);

    CVisualPoint ScreenToVisual(POINT & pt);

    void SetPointOffset(double xOffset, double yOffset);
private:
    // conversion between visual and display coordinates
    double m_xScale;
    double m_yScale;
    double m_xOffset;
    double m_yOffset;
};

class CVisualDrawContext
{
public:
    CVisualDrawContext(CVisualCoordinateTransform * pTransform);
    ~CVisualDrawContext();

    BOOL BeginPaint(HWND hWnd);
    void EndPaint();

    void PushState();
    void PopState();

    BOOL SelectSmallFont();
    void DeselectSmallFont();
    
    BOOL SelectPen(COLORREF color, int width);
    void DeselectPen();
    BOOL SelectSolidBrush(COLORREF color);
    
    HDC DC() { return m_hBgBuffer; }

    void MapPoint(CVisualPoint & vis, POINT & disp);
    void MapRect(CVisualRect & vis, RECT & disp);

    class CState
    {
    public:
        CState()
            : m_xCoord(0)
            , m_yCoord(0)
            , m_hNewBrush(NULL)
            , m_hOldBrush(NULL)
        {;
        }
        
        double m_xCoord;
        double m_yCoord;

        HPEN m_hOldPen;
        HPEN m_hNewPen;
        HBRUSH m_hNewBrush;
        HBRUSH m_hOldBrush;
        HGDIOBJ m_hOldFont;
    };

    void ShiftCoordinates(double xOffset, double yOffset);
        
private:
    HDC m_hdc;
    HDC m_hBgBuffer;
    HWND m_hWnd;
    PAINTSTRUCT m_ps;

    CState m_State;

    CAtlList<CState> m_StateStack;

    CVisualCoordinateTransform * m_pTransform;

    HBITMAP m_hOldBitmap;
};

///////////////////////////////////////////////////////////////////////////////
// 
class CCommandHandler
{
public:
    virtual BOOL OnLButtonDown(CVisualObject * pObj, CVisualPoint & pt) { return FALSE; }
    virtual BOOL OnLButtonUp(CVisualObject * pObj, CVisualPoint & pt) { return FALSE; }
    virtual BOOL OnMouseMove(CVisualObject * pObj, CVisualPoint & pt) { return FALSE; }
    virtual BOOL OnLButtonDoubleClick(CVisualObject* pObj, CVisualPoint& pt) { return FALSE; }
    virtual BOOL OnFocus(CVisualObject* pObj) { return FALSE; }
};

///////////////////////////////////////////////////////////////////////////////
//
class CVisualObject 
{
public:
    enum OBJECT_TYPE
    {
        UNDEFINED,
        NODE,
        PIN,
        CONNECTOR,
        CONTAINER,
    };

    enum CONNECTION_TYPE {
        NONE,
        INPUT,
        OUTPUT  
    };

    CVisualObject(CVisualRect & Rect)
        : m_Rect(Rect)
        , m_Type(UNDEFINED)
        , m_pHandler(NULL)
        , m_pData(NULL)
        , m_dwIndex(0)
        , m_bIsSelected(false)
        , m_pContainer(NULL)
    {;
    }

    CVisualObject()
        : m_pHandler(NULL)
        , m_Type(UNDEFINED)
        , m_pData(NULL)
        , m_dwIndex(0)
        , m_bIsSelected(false)
        , m_pContainer(NULL)
    {;
    }
    
    virtual ~CVisualObject() {

    }

    virtual CONNECTION_TYPE GetConnectionType() const = 0;

    virtual void Draw(CVisualDrawContext & Ctx) = 0;
    virtual BOOL HitTest(CVisualPoint & pt, CVisualObject** ppObj) { return FALSE; }
    virtual void NotifyRemoved(CVisualObject* removed) = 0;

    // derived component can overwrite move method to move dependent objects
    virtual void Move(double x, double y) { m_Rect.Move(x, y); }
    virtual void AddPosition(double x, double y) { m_Rect.Add(x, y); }
    
    CVisualRect & Rect() { return m_Rect; }

    // get/set handler
    virtual void SetHandler(CCommandHandler* pHandler) { m_pHandler = pHandler; }
    CCommandHandler * GetHandler() { return m_pHandler; }

    // get/set user data
    void SetData(LONG_PTR pData) { m_pData = pData; }
    LONG_PTR GetData() { return m_pData; }

    void SetIndex(DWORD dwIndex) { m_dwIndex = dwIndex; }
    DWORD GetIndex() { return m_dwIndex; }
    
    // get visual type
    OBJECT_TYPE Type() { return m_Type; }
    
    virtual void Select(bool bIsSelected);
    bool IsSelected() { return m_bIsSelected; }

    virtual bool IsDependent(CVisualObject* pOtherObj) const = 0;
    
    CVisualContainer* GetContainer() { return m_pContainer; }
    void SetContainer(CVisualContainer* pContainer) { m_pContainer = pContainer; }

    virtual bool ContainsVisual(CVisualObject* pVisual);

protected:
    CVisualRect m_Rect;
    OBJECT_TYPE m_Type;
    CCommandHandler * m_pHandler;
    LONG_PTR m_pData;
    DWORD m_dwIndex;
    bool m_bIsSelected;
    CVisualContainer* m_pContainer;
};

///////////////////////////////////////////////////////////////////////////////
//

class CVisualConnector : public CVisualObject
{
public:
    CVisualConnector()
        : CVisualObject()
    {
        m_Type = CONNECTOR;
        m_clrSelected = RGB(0, 0, 200);
        m_clrLine = RGB(0, 0, 0);
    }

    CONNECTION_TYPE GetConnectionType() const;

    void NotifyRemoved(CVisualObject* removed);

    // draw connector
    void Draw(CVisualDrawContext& Ctx);

    CVisualPoint & Left() { return m_Left; }
    CVisualPoint & Right() { return m_Right; }
    
    virtual bool IsDependent(CVisualObject* pOtherObj) const;

    virtual BOOL HitTest(CVisualPoint& pt, CVisualObject** ppObj);
private:
    CVisualPoint m_Left;
    CVisualPoint m_Right;
    COLORREF m_clrLine;
    COLORREF m_clrSelected;
};

///////////////////////////////////////////////////////////////////////////////
//
class CVisualPin : public CVisualObject
{
public:
    CVisualPin(CVisualComponent* pOwner, CVisualRect& Rect, CONNECTION_TYPE connType, const CAtlStringW& strLabel, int nPinId);

    CONNECTION_TYPE GetConnectionType() const;
    int GetPinId() const;

    void Draw(CVisualDrawContext& Ctx);

    void NotifyRemoved(CVisualObject* removed);

    // returns point for connector in screen coordinates
    CVisualPoint GetConnectorPoint();
    
    CVisualConnector * GetConnector() { return m_pConnector; }
    void SetConnector(CVisualConnector* pConnector) { m_pConnector = pConnector; }
    
    virtual void Select(bool selected);

    virtual bool IsDependent(CVisualObject* pOtherObj) const;

    void Highlight(bool fHighlight = true) { m_fHighlight = fHighlight; }
    
private:
    CVisualComponent * m_pOwner;
    CVisualConnector * m_pConnector;
    CONNECTION_TYPE m_ConnType;
    CAtlStringW m_strLabel;
    int m_nPinId;

    const static int LABEL_OFFSET_X;
    const static int LABEL_OFFSET_Y;

    bool m_fHighlight;
};

///////////////////////////////////////////////////////////////////////////////
//
class CVisualComponent : public CVisualObject
{
public:
    CVisualComponent(CVisualRect& Rect);
    virtual ~CVisualComponent();

    virtual CONNECTION_TYPE GetConnectionType() const;

    virtual CVisualPin* GetInputPinByIndex(size_t nIndex) = 0;
    virtual CVisualPin* GetOutputPinByIndex(size_t nIndex) = 0;
    virtual size_t GetInputPinCount() = 0;
    virtual size_t GetOutputPinCount() = 0;
    virtual CVisualPin* GetInputPin(int nPinID) = 0;
    virtual CVisualPin* GetOutputPin(int nPinID) = 0;

    virtual void FlagTopoLoadError(size_t nIndex, bool fError = true) = 0;
};

/////////////////////////////////////////////////////////////////////////////////
//
class CVisualNode : public CVisualComponent
{
public:
     enum
    {
        COMP_DEF_WIDTH = 100,
        COMP_DEF_HEIGHT = 100,
        PIN_WIDTH = 9,
        PIN_HEIGHT = 9,
        PIN_INTERVAL = 20,
    };

    CVisualNode(const CAtlStringW& strLabel, bool fAutoInserted);
    ~CVisualNode();

    void Draw(CVisualDrawContext& Ctx);

    virtual bool IsDependent(CVisualObject* pOtherObj) const;

    BOOL HitTest(CVisualPoint & pt, CVisualObject** ppObj);
    void Move(double x, double y);
    void NotifyRemoved(CVisualObject* removed);
    int GetPinIDWithConnector(CVisualConnector* pConnector);

    static void SetPinHandler(CCommandHandler* pHandler);
    CVisualPin * AddPin(bool fInput, LONG_PTR pData, const CAtlStringW& strLabel, int nPinId);

    CVisualPin* GetInputPinByIndex(size_t nIndex);
    CVisualPin* GetOutputPinByIndex(size_t nIndex);
    size_t GetInputPinCount() { return m_InputPins.GetCount(); }
    size_t GetOutputPinCount() { return m_OutputPins.GetCount(); }
    CVisualPin* GetInputPin(int nPinID);
    CVisualPin* GetOutputPin(int nPinID);

    void FlagTopoLoadError(size_t nIndex, bool fError = true);

protected:        
    void RecalcPins();
    void PositionPins(size_t nPins, CVisualObject::CONNECTION_TYPE Dir);

private:
    static CCommandHandler* ms_pPinHandler;
    
    CAtlArray<CVisualPin*> m_InputPins;
    CAtlArray<CVisualPin*> m_OutputPins;
    bool m_fTopoError;

    CAtlStringW m_strLabel;

    COLORREF m_clrFill;
    COLORREF m_clrLine;
    COLORREF m_clrSelectedBorder;
    COLORREF m_clrErrorText;
};


class CVisualContainer : public CVisualComponent
{
public:
    enum
    {
        COMP_DEF_WIDTH = 100,
        COMP_DEF_HEIGHT = 100,
        TOP_MARGIN = 20,
        BOTTOM_MARGIN = 20,
    };

    CVisualContainer(const CAtlStringW& strLabel);
    ~CVisualContainer();

    void Draw(CVisualDrawContext& Ctx);
    BOOL HitTest(CVisualPoint& pt, CVisualObject** ppObj);
    void Move(double x, double y);
    void AddPosition(double x, double y);
    void NotifyRemoved(CVisualObject* removed);
    
    virtual bool IsDependent(CVisualObject* pOtherObj) const;

    void AddComponent(CVisualComponent* pComponent);
    void AddContainer(CVisualContainer* pContainer);

    DWORD GetComponentCount();
    CVisualComponent* GetComponent(DWORD dwIndex);

    DWORD GetContainerCount();
    CVisualContainer* GetContainer(DWORD dwIndex);

    void SetParent(CVisualContainer* pParent);

    void SetHandler(CCommandHandler* pHandler);
    
    CVisualPin* GetInputPinByIndex(size_t nIndex);
    CVisualPin* GetOutputPinByIndex(size_t nIndex);
    size_t GetInputPinCount();
    size_t GetOutputPinCount();
    CVisualPin* GetInputPin(int nPinID);
    CVisualPin* GetOutputPin(int nPinID);

    void FlagTopoLoadError(size_t nIndex, bool fError = true);

    bool ContainsVisual(CVisualObject* pVisual);

protected:
    void RecalcPositions();

private:
    CAtlStringW m_strLabel;
    CAtlArray<CVisualComponent*> m_arrComponents;
    COLORREF m_clrFill;
    CVisualContainer* m_pParent;
};

///////////////////////////////////////////////////////////////////////////////
//
class CVisualObjectEventHandler
{
public:
    virtual void NotifyObjectDeleted(CVisualObject* pVisualObj) = 0;
};

///////////////////////////////////////////////////////////////////////////////
// 
class CVisualTree
{
public:
    CVisualTree();
    ~CVisualTree();

    HRESULT AddVisual(CVisualObject* pVisual, bool fEnsureOpenSpace = true);
    void RemoveVisual(CVisualObject* pVisual);

    void Draw(CVisualDrawContext & Ctx);	

    BOOL HitTest(CVisualPoint & pt, CVisualObject** ppObj);

    void RouteAllConnectors();

    BOOL MakeConnector(CVisualPin* pSourcePin, CVisualPin* pSinkPin);

    void GetMaxExtent(UINT32& maxXExtent, UINT32& maxYExtent);
    bool IsOccupied(CVisualObject* pObj);
    
    void SetEventHandler(CVisualObjectEventHandler* pEventHandler) { m_pEventHandler = pEventHandler; }

protected:
    CAtlArray<CVisualObject *> m_Objects;
    CVisualObjectEventHandler* m_pEventHandler;
};

#endif

