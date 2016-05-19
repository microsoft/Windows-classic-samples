// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved

#pragma once

class CDock 
    : public CWindowImpl<CDock>
{
public:
    CDock();
    ~CDock();
    
    enum AREA_MOVE_TYPE
    {
        MOVE_NO,
        MOVE_HOR,
        MOVE_VERTICAL,
    };

    enum STOCK_AREA        
    {
        STOCK_AREA_LEFT,
        STOCK_AREA_RIGHT,
        STOCK_AREA_TOP,
        STOCK_AREA_BOTTOM,
    };
    
    class CFixedPos
    {
    public:        
        double left;
        double top;
        LONG width;
        LONG height;
    };
    
    class CArea
    {
    public:
        CArea(AREA_MOVE_TYPE Type);

        RECT m_rc;
        CFixedPos m_posFixed;

        struct ATTACH
        {
            CArea* pLeft;
            CArea* pRight;
            CArea* pTop;
            CArea* pBottom;
        } m_Attach;

        // user defined id for the area
        DWORD m_dwId;

        // flag to indicate if user can move area
        AREA_MOVE_TYPE m_MoveType;
        
        // optional window associated with
        CWindow * m_pWindow;

        // valid from position modification until next resize
        BOOL m_fLocked;
        
        // runtime only. iteration for resize
        DWORD m_nResizeIter;
    };

    DWORD GetAreaCount();
    CArea* GetArea(DWORD n);
    CArea* AddArea(AREA_MOVE_TYPE Type);
    CArea* GetStockArea(STOCK_AREA AreaType);

    void RemoveAllAreas();
    void UpdateDock();
    void MoveSplitter(CWindow* pSplitter, LONG x, LONG y);

protected:    
    void ResizeDock(long nWidth, long nHeight);
    void ResizePane(CDock::CArea* pArea);
    BOOL CanResize(CDock::CArea* pArea);
        
    LRESULT OnCreate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
    LRESULT OnSize(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
    
    BEGIN_MSG_MAP(CDock)
        MESSAGE_HANDLER(WM_CREATE, OnCreate)
        MESSAGE_HANDLER(WM_SIZE, OnSize)
    END_MSG_MAP()

private:    
    CAtlArray<CArea*> m_Areas;

    LONG m_nHeight;
    LONG m_nWidth;
    
    CArea m_StockLeft;
    CArea m_StockRight;
    CArea m_StockTop;
    CArea m_StockBottom;
};
