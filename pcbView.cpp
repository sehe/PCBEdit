
// pcbView.cpp : implementation of the PCBView class
//

#include "stdafx.h"
#include "debug.h"
#include "resource.h"       // main symbols
#include "globals.h"

// SHARED_HANDLERS can be defined in an ATL project implementing preview, thumbnail
// and search filter handlers and allows sharing of document code with that project.
#ifndef SHARED_HANDLERS
#include "pcb.h"
#endif

#include "debug.h"
#include "Geometry.h"
#include "board.h"
#include "Drawing.h"
#include "RatList.h"

#include "Share.h"
#include "netlist_import.h"
#define BOARD_H
#include "SharePCB.h"
#include "pcbDoc.h"
#include "pcbView.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#ifdef _DEBUG
//#define DRAW_TEST
#endif

//.. this can be used to send message to parent frame 'PCBFrame
	//GetParent( )->SendMessage( ID_POST_PCVIEW_FRAME, (WPARAM)pointerToInfo );
//..

using namespace PCBItemTypes;
using namespace NSEditMode;

// ............................................................................
IMPLEMENT_DYNCREATE( PCBView, CView )
BEGIN_MESSAGE_MAP( PCBView, CView )
	ON_WM_CONTEXTMENU( )
	ON_WM_RBUTTONUP( )
	ON_WM_MOUSEWHEEL( )
	ON_WM_MOUSEMOVE( )
	ON_WM_LBUTTONDOWN( )
	ON_WM_LBUTTONUP( )
	ON_WM_ERASEBKGND( )
	ON_WM_SYSKEYDOWN( )
	ON_WM_SYSKEYUP( )
	ON_WM_KEYDOWN( )
	ON_WM_KEYUP( )
	ON_WM_SETCURSOR( )
	ON_WM_VSCROLL( )
	ON_WM_HSCROLL( )
	ON_WM_SIZE( )

	//buttons
	ON_COMMAND(ID_S_SELECT, &PCBView::OnSSelect)
	ON_UPDATE_COMMAND_UI(ID_S_SELECT, &PCBView::OnUpdateSSelect)
	ON_COMMAND(ID_S_ADD_NODE, &PCBView::OnSAddNode)
	ON_UPDATE_COMMAND_UI(ID_S_ADD_NODE, &PCBView::OnUpdateSAddNode)
	ON_COMMAND(ID_S_HAND, &PCBView::OnSHand)
	ON_UPDATE_COMMAND_UI(ID_S_HAND, &PCBView::OnUpdateSHand)

	// Standard printing commands
	ON_COMMAND( ID_FILE_PRINT, &CView::OnFilePrint )
	ON_COMMAND( ID_FILE_PRINT_DIRECT, &CView::OnFilePrint )
	ON_COMMAND( ID_FILE_PRINT_PREVIEW, &PCBView::OnFilePrintPreview )
END_MESSAGE_MAP( )

// ............................................................................
PCBView::PCBView( )
	:winScale( 200000 )
	,offsetx( 0 )
	,offsety( 0 )
	,d_ext( rectS, offsetx, offsety, winScale )
	,lastNode( new SSP_Node )
	,curMode( mdSelect )
{
}

// ............................................................................
PCBView::~PCBView()
{
}

// ............................................................................
BOOL PCBView::PreCreateWindow( CREATESTRUCT& cs )
{
#ifdef USING_CONSOLE
	Create_STD_OUT_Console( );
#endif
	cs.style|= WS_HSCROLL | WS_VSCROLL;
	scroller.AttachWnd( this );
	return CView::PreCreateWindow( cs );
}

// ............................................................................
//basic stuff
// ............................................................................
bg_point PCBView::ScreenToBoard( CPoint& inPoint )
{
	bg_point p(
		( inPoint.x - offsetx ) * winScale
		,( inPoint.y - offsety ) * winScale
		);
	return p;
}

// ............................................................................
CPoint PCBView::BoardToScreen( bg_point& inPoint )
{
	CPoint p(
		(long)( inPoint.get< 0 >( ) / winScale + offsetx )
		,(long)( inPoint.get< 1 >( ) / winScale + offsety )
		);
	return p;
}

// ............................................................................
#define SNAP_SIZE 500000
void PCBView::GetSnap( bg_point& in, size_t size )
{
	in.set_x( ( in.get_x( ) / size ) * size + size / 2  );
	in.set_y( ( in.get_y( ) / size ) * size + size / 2  );
}

// ............................................................................
SP_Node PCBView::LocateNearestWire( )
{
	for( auto it= lastNode->begin( ); it != lastNode->end( ); ++it )
		if( it->get( )->sp_obj->type == ptypeLine )
			return *it;
//			return SP_Node( new Node( *it->get( ) ) );

	return SP_Node( );
}

// ............................................................................
void PCBView::DevCall( bg_point& pt )
{
	bg_box box= GetBounds( lastNode );
	rectInvalid.SetRect(
		(long)box.min_corner( ).get< 0 >( )
		,(long)box.min_corner( ).get< 1 >( )
		,(long)box.max_corner( ).get< 0 >( ),
		(long)box.max_corner( ).get< 1 >( ) );
	rectInvalid.TopLeft( )= BoardToScreen( box.min_corner( ) );
	rectInvalid.BottomRight( )= BoardToScreen( box.max_corner( ) );

#ifdef DRAW_TEST
	CClientDC dc( this );
	dc.SelectStockObject( DKGRAY_BRUSH );
	CSize size( 2, 2 );
	dc.DrawDragRect( rectInvalid, size, NULL, size );
#endif

	if( pt == lastSnap )
		return;
	//drag
	lastSnap= pt;
	TRACE( "DragPt: %I64d %I64d\n", pt.x, pt.y );
	for( auto it= lastNode->begin( ); it != lastNode->end( ); ++it )
	{
		it->get( )->point= lastSnap;//This will require a 'set_point member to handle complete object
	}
	InvalidateRect( rectInvalid );
}

// ............................................................................
//mouse stuff
// ............................................................................
BOOL PCBView::OnMouseWheel( UINT nFlags, short zDelta, CPoint pt )
{
	long prevWinScale= winScale;

	if( zDelta < 0 )
		winScale+= winScale / 4;
	else
		winScale-= winScale / 4;

	CSize ratio( winScale, prevWinScale );
	scroller.OnMouseWheel( ratio, pt );

	Invalidate( );

	return FALSE;
//	return CView::OnMouseWheel( nFlags, zDelta, pt );
}

// ............................................................................
void PCBView::OnLButtonDown( UINT nFlags, CPoint point )
{

	//we may be moving objects, get the start
	bg_point at= ScreenToBoard( point );
	switch( curMode )
	{
	case mdSelect:
		GetSnap( at, SNAP_SIZE );
		lastSnap= at;
		TRACE( " FirstSnap: %I64d %I64d \n", at.get_x( ), at.get_y( ) );

		if( nFlags & MK_CONTROL )
			;
		break;

	case mdAddNode:
	{
		SP_Node spN= LocateNearestWire( );
		//todo, handle layers and this needs to be in its own place
		//create a new wire
		Line* pLineOrg= dynamic_cast< Line* >( spN->sp_obj.get( ) );
		SP_Base spNew= pLineOrg->SP_Copy( );
		Line* pLineNew= dynamic_cast< Line* >( spNew.get( ) );
		//connect the wires
		pLineOrg->set_pt1( at );
		pLineNew->set_pt2( at );
		SP_Node spNewNode= SP_Node( new Node( spNew, at ) );
		curMode= mdSelect;
		break;
	}
	case mdHandMove:
		curMode= mdHandDown;
		OnSetCursor( NULL, 0, 0 );
//		return;
		break;

	}//switch

	//TODO figure out how to make this work........
	SHORT key_stat= ::GetAsyncKeyState( VK_LSHIFT );
	MPTRACE( "OnLButtonDown key_stat: %x\n" );

	if( nFlags & MK_ALT )
	{
		int test= 0;
	}

	SetCapture( );
}

// ............................................................................
void PCBView::OnMouseMove(UINT nFlags, CPoint point)
{
	//return;
	//in case of return from other windows. TODO deal
	SetFocus( );

	//CDC* pDC = GetDC( );
	//snapCursor.Draw(pDC, &point);    // MUST ADD FOR SNAPCURSOR

	bool bCapture= GetCapture( ) == this;
	bg_point at= ScreenToBoard( point );
	GetSnap( at, SNAP_SIZE );
	PCBoard& board= GetDocument( )->board;

	switch( curMode )
	{
	case mdSelect:
	{
		if( bCapture )
		{
			DevCall( at );
//			return;
		}
		else
		{
			Locator locate( board );
			SP_SSP_Node nextNode= locate.FindClosestNodes( at );

			if( ! lastNode->size( ) || nextNode->begin( )->get( )->pt( ) != lastNodePoint )
			{
				if( lastNode->size( ) )
				{
					CPoint pt= BoardToScreen( lastNode->begin( )->get( )->pt( ) );
					CRect rect( pt.x - 15, pt.y - 15, pt.x + 15, pt.y + 15 );
					InvalidateRect( rect, FALSE );
				}
				if( nextNode->size( ) )
				{
					CPoint pt= BoardToScreen( nextNode->begin( )->get( )->pt( ) );
					CRect rect( pt.x - 15, pt.y - 15, pt.x + 15, pt.y + 15 );
					InvalidateRect( rect, FALSE );
					lastNodePoint= nextNode->begin( )->get( )->pt( );
				}
				lastNode= nextNode;
				//TRACE( "invalidated %d\n", lastNode.get( ) );
			}
		}
	}
	break;

	case mdHandDown:
		if( bCapture )
		{
			CSize dif= ptLastMouse - point;
//			scroller.GetScrollPos( ).cx+= offsetx;
			scroller.OffsetScrollPos( dif );
			scroller.UpdateScrollInfo(  );
			Invalidate( );
			break;
		}

	}//switch

	if( ptLastMouse != point )
		bStatusMouseChanged= true;
	else
		return;

	ptLastMouse= point;
}

// ............................................................................
void PCBView::OnLButtonUp(UINT nFlags, CPoint point)
{
	ReleaseCapture( );
	switch( curMode )
	{
	case mdHandDown:
		curMode= mdHandMove;
		break;

	}//switch
}

// ............................................................................
void PCBView::OnRButtonUp(UINT /* nFlags */, CPoint point)
{
	ClientToScreen(&point);
	OnContextMenu(this, point);
}

// ............................................................................
//Drawing stuff
// ............................................................................
void PCBView::DrawTest( CDC* pDC, CPoint pt )
{
	DrawArea drawer( pDC, GetDocument( )->board, d_ext );
	//LOGBRUSH penbrush= { 0 };
	//penbrush.lbStyle= BS_SOLID;
	//penbrush.lbColor= RGB( 10, 200, 200 );
	drawer.SetPen( RGB( 10, 200, 200 ), 20, penStylesRnd );
	drawer.DrawCross( pt );
}

// ............................................................................
void PCBView::OnDraw( CDC* poDC )
{
/*
	This is not intended as a base for a finished OnDraw, just a test to check things out.
	There should be a Drawing object(s) for this work as an editor.
*/
	PCBDoc* pDoc = GetDocument( );
	ASSERT_VALID( pDoc );
	if( ! pDoc )
		return;

	bool bCopper= true;
	bool bRat= true;
	bool bCapture= GetCapture( ) == this;

    offsetx= -scroller.GetScrollPos( ).cx;
    offsety= -scroller.GetScrollPos( ).cy;

	GetClientRect( &rectS );
	CNewMemDC pDC( poDC );
	PCBoard& board= GetDocument( )->board;
	DrawArea drawer( pDC, board, d_ext );

//fade to black...
	pDC->FillSolidRect( rectS, RGB( 0, 0, 0 ) );

//draw board outline
	drawer.SetPen( RGB( 240, 240, 0 ), 3, penStylesFlat );
	drawer.DrawRect( board.GetExtents( ) );

	if( ! board.layers.size( ) )
		return;

//bottom copper layer: lines
	if( bCopper && board.GetLayer( 15 ) )
	{
		VSP_Line spL= board.GetLayer( 15 )->vsp_lines;
		for( auto it= spL.begin( ); it != spL.end( ); ++it )
		{
			drawer.SetPen( RGB( 255, 0, 0 ), (long)( it->get( )->size / winScale ), penStylesRnd );
			drawer.DrawLine( dynamic_cast< Line& >( *it->get( ) ) );
		}
	}
//top copper layer: lines
	if( bCopper && board.GetLayer( 0 ) )
	{
		VSP_Line spL= board.GetLayer( 0 )->vsp_lines;
		for( auto it= spL.begin( ); it != spL.end( ); ++it )
		{
			drawer.SetPen( RGB( 0, 255, 0 ), (long)it->get( )->size / winScale, penStylesRnd );
			drawer.DrawLine( dynamic_cast< Line& >( *it->get( ) ) );
		}
	}
//commponent pads and pins
	{
		drawer.SetPen( RGB( 0, 255, 200 ), 3, penStylesFlat );
		drawer.SetDCBrush( RGB( 0, 255, 200 ) );
		for( auto it= board.components.begin( ); it != board.components.end( ); ++it )
		{
			Component& comp= *it->get( );
			DRAWTRACE( " comp: %s %d %d\n", comp.sp_refLabel->text.c_str( ), comp.Get_mx( ), comp.Get_my( ) );
			for( auto sit= comp.vsp_pins.begin( ); sit != comp.vsp_pins.end( ); ++sit )
			{
				Connect& con= *sit->get( );
				drawer.DrawShape( con.pt_1( ) + comp.get_pt( ), con.pt_2( ) + comp.get_pt( ), con.flags );
				if( con.flags & PCSF_STD ) //should have valid drill and this could be more effecient in an outside loop
				{
					drawer.SetPen( RGB( 0, 255, 200 ), 1, penStylesFlat );
					drawer.SetDCBrush( RGB( 20, 55, 20 ) );
					bg_box b( comp.get_pt( ) + con.drill.from - con.drill.size / 2, comp.get_pt( ) + con.drill.from + con.drill.size / 2 );
					drawer.DrawShape( b, PCSF_ROUND );
					//TRACE( " x: %d y: %d\n", (long)( b.max_corner( ).get_x( ) - b.min_corner( ).get_x( ) ), (long)(  b.max_corner( ).get_y( ) - b.min_corner( ).get_y( ) ) );
					drawer.PopPenBrush( );
				}
			}
			//testing show comp origin
//			drawer.DrawBoardCross( comp.get_pt( ) );
		}
		pDC->SelectStockObject( NULL_BRUSH );
	}
//commponent silk
	{
		for( auto it= board.components.begin( ); it != board.components.end( ); ++it )
		{
			Component& comp= *it->get( );
			DRAWTRACE( " comp: %s %d %d\n", comp.sp_refLabel->text.c_str( ), comp.Get_mx( ), comp.Get_my( ) );

			//example of draw symbol text
			for( auto cit= (*it)->sp_refLabel->text.begin( ); cit != (*it)->sp_refLabel->text.end( ); ++cit )
			{

			}
			//
			for( auto sit= comp.ssp_other.begin( ); sit != comp.ssp_other.end( ); ++sit )
			{
				Line* pS= dynamic_cast< Line* >( sit->get( ) );
				if( pS )
				{
					drawer.SetPen( RGB( 240, 240, 100 ), (size_t)( pS->size / winScale ), penStylesFlat );
					drawer.DrawLine( *pS, comp.get_pt( ) );
				}
				else
					assert( false );
			}
		}
	}
	{
//Commponent Text using DrawText
//		penbrush.lbColor= RGB( 255, 255, 0 );
		pDC->SetTextColor( RGB( 255, 255, 255 ) );
		pDC->SetBkMode( TRANSPARENT );
		for( auto it= board.components.begin( ); it != board.components.end( ); ++it )
		{
			Component& c= *it->get( );
			long ox= (long)( (*it)->Get_mx( ) / winScale + offsetx );
			long oy= (long)( (*it)->Get_my( ) / winScale + offsety );
			CRect rect( ox, oy, ox + 100, oy + 100 );
			pDC->DrawText( (*it)->sp_refLabel->text.c_str( ), (*it)->sp_refLabel->text.size( ), rect, DT_LEFT | DT_NOPREFIX );
		}
	}
//Vias
	{
		for( auto it= board.vias.begin( ); it != board.vias.end( ); ++it )
		{
			drawer.SetPen( RGB( 0, 0, 255 ), 4, penStylesFlat );
			drawer.DrawConnect( *it->get( ) );
		}
	}

//Rat lines
	if( bRat )
	{
		drawer.SetPen( RGB( 255, 255, 200 ), 1, penStylesFlat );
		drawer.SetDCBrush( RGB( 255, 55, 200 ) );
		for( auto it : board.netlist )
		{
			DRAWTRACE( L"%s\n", it->name.c_str( ) );
			std::vector< CPoint > temp;
			for( auto item : it->items )
			{
//				MWTRACE( L"  %s %d\n", (*nit)->sp_commponent->sp_nameLabel->text.c_str( ), (*nit)->sp_connect->index );

				long ox= (long)( item->sp_commponent->Get_mx( ) / winScale + offsetx );
				long oy= (long)( item->sp_commponent->Get_my( ) / winScale + offsety );
				long px= (long)( item->sp_connect->x1( ) / winScale + ox );
				long py= (long)( item->sp_connect->y1( ) / winScale + oy );
				temp.push_back( CPoint( px, py ) );
			}
			auto cit= temp.begin( );
			if( cit != temp.end( ) )
			{
				pDC->MoveTo( *cit );
				for( ; cit != temp.end( ); ++cit )
					pDC->LineTo( *cit );
			}
		}
	}
//dragable indicators
	if( lastNode->size( ) )
	{
		Node* pNode= dynamic_cast< Node* >( lastNode->begin( )->get( ) );
		drawer.DrawCross( BoardToScreen( pNode->pt( ) ) );
	}

#ifdef SHOW_CENTER
	{
		CRect trect;
		LOGBRUSH penbrush= { 0 };
		penbrush.lbColor= RGB( 255, 255, 255 );
		CPen pen( PS_SOLID | PS_ENDCAP_FLAT |PS_GEOMETRIC, 4, &penbrush );
		CPen* pT= pDC->SelectObject( &pen );
		GetWindowRect( &trect );
		ScreenToClient( &trect );
		CRect rect( trect.Width(  ) / 2 - 5,
			trect.Height(  ) / 2 - 5,
			trect.Width(  ) / 2 + 5,
			trect.Height(  ) / 2 + 5
		);
		pDC->Ellipse( rect );
	}
#endif
}

// ............................................................................
//the rest stuff
// ............................................................................
void PCBView::OnSSelect( )
{
	curMode= mdSelect;
}

// ............................................................................
void PCBView::OnSAddNode( )
{
	curMode= mdAddNode;
}

// ............................................................................
void PCBView::OnSHand( )
{
	curMode= mdHandMove;
}

// ............................................................................
void PCBView::OnUpdateSAddNode( CCmdUI *pCmdUI )
{
	pCmdUI->SetCheck( curMode == mdAddNode );
}

// ............................................................................
void PCBView::OnUpdateSSelect( CCmdUI *pCmdUI )
{
	pCmdUI->SetCheck( curMode == mdSelect );
}

// ............................................................................
void PCBView::OnUpdateSHand(CCmdUI *pCmdUI)
{
	pCmdUI->SetCheck( curMode == mdHandMove );
}

// ............................................................................
BOOL PCBView::OnSetCursor( CWnd* pWnd, UINT nHitTest, UINT message )
{
	switch( curMode )
	{
	case mdSelect:
		::SetCursor( AfxGetApp( )->LoadStandardCursor( IDC_ARROW ) );
		break;

	case mdAddNode:
		::SetCursor( AfxGetApp( )->LoadStandardCursor( IDC_IBEAM ) );
		break;

	case mdHandMove:
		::SetCursor( AfxGetApp( )->LoadStandardCursor( IDC_HAND ) );
		break;

	case mdHandDown:
		::SetCursor( AfxGetApp( )->LoadStandardCursor( IDC_NO ) );
		break;

	}//switch
	return FALSE;
//	return CView::OnSetCursor( pWnd, nHitTest, message );
}

// ............................................................................
void PCBView::SetDisplaySize( )
{
	PCBoard& board= GetDocument( )->board;
	CPoint pt( BoardToScreen( board.GetDrawSize( ) ) );
	scroller.SetDisplaySize( pt.x, pt.y );
	//testing
	Invalidate( );
}

// ............................................................................
void PCBView::OnUpdate( CView* /*pSender*/, LPARAM /*lHint*/, CObject* /*pHint*/)
{
	//This gets called after the doc has a PCB loaded.
	SetDisplaySize( );
}

// ............................................................................
void PCBView::OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
	VIEWTRACE( "VScroll pos: %d  code: %d\n", nPos, nSBCode );
	scroller.OnVScroll( nSBCode, nPos, pScrollBar );

//	CView::OnVScroll(nSBCode, nPos, pScrollBar);
}

// ............................................................................
void PCBView::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
	scroller.OnHScroll( nSBCode, nPos, pScrollBar );
//	CView::OnHScroll( nSBCode, nPos, pScrollBar );
}

// ............................................................................
void PCBView::OnSize( UINT nType, int cx, int cy )
{
	CView::OnSize( nType, cx, cy );
	scroller.OnSize( nType, cx, cy );
}

// ............................................................................
//When frame needs status info
CString PCBView::GetStatusInfo( )
{
	if( bStatusMouseChanged )
	{
		strStatus.Format( _T("x: %d y: %d  smmx: %.2f smmy: %.2f")
			,ptLastMouse.x
			,ptLastMouse.y
			,(double)( ptLastMouse.x - offsetx ) * winScale / MILL_MULT
			,(double)( ptLastMouse.y - offsety ) * winScale / MILL_MULT
		);
		bStatusMouseChanged= false;
	}
	return strStatus;
}

// PCBView printing
// ............................................................................
void PCBView::OnFilePrintPreview()
{
#ifndef SHARED_HANDLERS
	AFXPrintPreview(this);
#endif
}

// ............................................................................
BOOL PCBView::OnPreparePrinting(CPrintInfo* pInfo)
{
	// default preparation
	return DoPreparePrinting(pInfo);
}

// ............................................................................
void PCBView::OnBeginPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
	// TODO: add extra initialization before printing
}

// ............................................................................
void PCBView::OnEndPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
	// TODO: add cleanup after printing
}

// ............................................................................
void PCBView::OnContextMenu(CWnd* /* pWnd */, CPoint point)
{
#ifndef SHARED_HANDLERS
	theApp.GetContextMenuManager()->ShowPopupMenu(IDR_POPUP_EDIT, point.x, point.y, this, TRUE);
#endif
}

// ............................................................................
// PCBView diagnostics
#ifdef _DEBUG
void PCBView::AssertValid() const
{
	CView::AssertValid();
}

// ............................................................................
void PCBView::Dump(CDumpContext& dc) const
{
	CView::Dump(dc);
}

// ............................................................................
PCBDoc* PCBView::GetDocument() const // non-debug version is inline
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(PCBDoc)));
	return (PCBDoc*)m_pDocument;
}
#endif //_DEBUG

// ............................................................................
BOOL PCBView::OnEraseBkgnd(CDC* pDC)
{
//	snapCursor.Reset( );
	return TRUE;
}

// ............................................................................
//DELETE the following...........
// ............................................................................
void PCBView::OnSysKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	// TODO: Add your message handler code here and/or call default
//	TRACE( "OnSysKeyDown: %x\n", nFlags );
	CView::OnSysKeyDown(nChar, nRepCnt, nFlags);
}


void PCBView::OnSysKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	// TODO: Add your message handler code here and/or call default

//	TRACE( "OnSysKeyUp: %x\n", nFlags );
	CView::OnSysKeyUp(nChar, nRepCnt, nFlags);
}


void PCBView::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	// TODO: Add your message handler code here and/or call default

	//TRACE( "OnKeyDown: %x\n", nFlags );
	CView::OnKeyDown(nChar, nRepCnt, nFlags);
}


void PCBView::OnKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	// TODO: Add your message handler code here and/or call default

//	TRACE( "OnKeyUp: %x\n", nFlags );
	CView::OnKeyUp(nChar, nRepCnt, nFlags);
}


void PCBView::OnActivateView( BOOL bActivate, CView* pActivateView, CView* pDeactiveView )
{
	PCBDoc* pDoc= GetDocument( );
	ASSERT_VALID( pDoc );

	CView::OnActivateView( bActivate, pActivateView, pDeactiveView );
}


	//PCBoard& board= GetDocument( )->board;
	//Locator locate( board );
	//SP_SSP_Node set= locate.FindClosestNodes( lastSnap );
	//Node* pNode= dynamic_cast< Node* >( set->begin( )->get( ) );

/*
	holdEditBases.clear( );
	for( auto it= set->begin( ); it != set->end( ); ++it )
	{
		holdEditBases.insert( it->get( )->sp_obj->SP_Copy( ) );
	}
	for( auto it= holdEditBases.begin( ); it != holdEditBases.end( ); ++it )
	{
		switch( it->get( )->type )
		{
		case ptypeLine:
		{
			Line* pLine= dynamic_cast< Line* >( it->get( ) );
			MPTRACE( "Line\t%d %d %d %d\n", pLine->x1( ), pLine->y1( ), pLine->x2( ), pLine->y2( ) );
			break;
		}
		case ptypeConnect:
		{
			Connect* pLine= dynamic_cast< Connect* >( it->get( ) );
			MPTRACE( "Conct\t%d %d\n", pLine->x1( ), pLine->y1( ) );
		}
		}//switch
	}
*/
