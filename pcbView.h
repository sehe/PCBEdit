
// pcbView.h : interface of the PCBView class
//

#pragma once

#ifndef DRAWING_H
#include "Drawing.h"
#endif

#ifndef SCROLLER_H
#include "Scroller.h"
#endif


// ...................................................................................
//By Keith Rule, 25 Mar 2002 on CodeProject
class CNewMemDC : public CDC {
private:       
    CBitmap    m_bitmap;        // Offscreen bitmap
    CBitmap*       m_oldBitmap; // bitmap originally found in CMemDC
    CDC*       m_pDC;           // Saves CDC passed in constructor
    CRect      m_rect;          // Rectangle of drawing area.
    BOOL       m_bMemDC;        // TRUE if CDC really is a Memory DC.
public:
    
    CNewMemDC(CDC* pDC, const CRect* pRect = NULL) : CDC()
    {
        ASSERT(pDC != NULL); 
 
        // Some initialization
        m_pDC = pDC;
        m_oldBitmap = NULL;
        m_bMemDC = !pDC->IsPrinting();
 
        // Get the rectangle to draw
        if (pRect == NULL) {
             pDC->GetClipBox(&m_rect);
        } else {
             m_rect = *pRect;
        }
 
        if (m_bMemDC) {
             // Create a Memory DC
             CreateCompatibleDC(pDC);
             pDC->LPtoDP(&m_rect);
 
             m_bitmap.CreateCompatibleBitmap(pDC, m_rect.Width(), 
                                                  m_rect.Height());
             m_oldBitmap = SelectObject(&m_bitmap);
 
             SetMapMode(pDC->GetMapMode());
 
             SetWindowExt(pDC->GetWindowExt());
             SetViewportExt(pDC->GetViewportExt());
 
             pDC->DPtoLP(&m_rect);
             SetWindowOrg(m_rect.left, m_rect.top);
        } else {
             // Make a copy of the relevent parts of the current 
             // DC for printing
             m_bPrinting = pDC->m_bPrinting;
             m_hDC       = pDC->m_hDC;
             m_hAttribDC = pDC->m_hAttribDC;
        }
 
        // Fill background 
        FillSolidRect(m_rect, pDC->GetBkColor());
    }
    
    ~CNewMemDC()      
    {          
        if (m_bMemDC) {
             // Copy the offscreen bitmap onto the screen.
             m_pDC->BitBlt(m_rect.left, m_rect.top, 
                           m_rect.Width(),  m_rect.Height(),
                  this, m_rect.left, m_rect.top, SRCCOPY);            
             
             //Swap back the original bitmap.
             SelectObject(m_oldBitmap);        
        } else {
             // All we need to do is replace the DC with an illegal
             // value, this keeps us from accidentally deleting the 
             // handles associated with the CDC that was passed to 
             // the constructor.              
             m_hDC = m_hAttribDC = NULL;
        }       
    }
    
    // Allow usage as a pointer    
    CNewMemDC* operator->() 
    {
        return this;
    }       
 
    // Allow usage as a pointer    
    operator CNewMemDC*() 
    {
        return this;
    }
};

// ...................................................................................
namespace NSEditMode
{
	enum modes {
		mdSelect,
		mdAddNode,
		mdHandMove,
		mdHandDown,
	};
};

// ...................................................................................
class PCBDoc;
class PCBView : public CView
{
protected: // create from serialization only
	PCBView( );
	DECLARE_DYNCREATE( PCBView )

	bg_point lastSnap;

	//screen params
	CRect rectS;
	long winScale;
	long offsetx;
	long offsety;
	//referance to screen params for Drawer
	DrawExtent d_ext;

	//Design support
	SSP_Node nodes;
	//Draging
	CRect rectInvalid;
	//scroller
	WinScroller scroller;

	//edit mode
	NSEditMode::modes curMode;

	//was stuff
	CPoint ptLastMouse;
	SP_SSP_Node lastNode;
	bg_point lastNodePoint;
	//origanal objects before move, and for undo?
	SSP_Base holdEditBases;

	//frame status bar
	bool bStatusMouseChanged;
	CString strStatus;

	//development
	void DevCall( bg_point& pt );
	SP_Node LocateNearestWire( );
	//use when board size or zoom changes
	void SetDisplaySize( );
	//snap to grid
	void GetSnap( bg_point& in, size_t size );

public:
	PCBDoc* GetDocument( ) const;
	CString GetStatusInfo( );
	void DrawTest( CDC* pDC, CPoint pt );
	bg_point ScreenToBoard( CPoint& inPoint );
	CPoint BoardToScreen( bg_point& inPoint );


public:
	virtual ~PCBView( );
	virtual void OnDraw( CDC* pDC );  // overridden to draw this view
	virtual BOOL PreCreateWindow( CREATESTRUCT& cs );
protected:
	virtual BOOL OnPreparePrinting( CPrintInfo* pInfo );
	virtual void OnBeginPrinting( CDC* pDC, CPrintInfo* pInfo );
	virtual void OnEndPrinting( CDC* pDC, CPrintInfo* pInfo );
	virtual void OnActivateView( BOOL bActivate, CView* pActivateView, CView* pDeactiveView );
	virtual void OnUpdate( CView* /*pSender*/, LPARAM /*lHint*/, CObject* /*pHint*/ );

public:
#ifdef _DEBUG
	virtual void AssertValid( ) const;
	virtual void Dump( CDumpContext& dc ) const;
#endif

protected:
	DECLARE_MESSAGE_MAP( )
	afx_msg void OnFilePrintPreview();
	afx_msg void OnRButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
	afx_msg BOOL OnMouseWheel(UINT nFlags, short zDelta, CPoint pt);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnSysKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnSysKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnSSelect();
	afx_msg void OnSAddNode();
	afx_msg void OnUpdateSAddNode(CCmdUI *pCmdUI);
	afx_msg void OnUpdateSSelect(CCmdUI *pCmdUI);
	afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);
	afx_msg void OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnSHand();
	afx_msg void OnUpdateSHand(CCmdUI *pCmdUI);
};

#ifndef _DEBUG  // debug version in pcbView.cpp
inline PCBDoc* PCBView::GetDocument() const
   { return reinterpret_cast<PCBDoc*>(m_pDocument); }
#endif

