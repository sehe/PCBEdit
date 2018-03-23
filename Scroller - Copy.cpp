// Filename: ScrollHelper.cpp
// 2005-07-01 nschan Initial revision.
// 2005-09-08 nschan Added GetClientRectSB() function.

#include "stdafx.h"
#include "debug.h"
#include "Scroller.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// ............................................................................
BOOL WinScroller::OnMouseWheel( CSize ratio, CPoint ptMouse )
{
	if ( m_attachWnd == NULL )
		return FALSE;

	//we will use move mouse cursor method
	//TODO, should be otional to zoom on current mouse position
	::ScreenToClient( m_attachWnd->GetSafeHwnd( ), &ptMouse );

	CRect rect;
	GetClientRectSB( m_attachWnd, rect );
	//center of screen
	CPoint np( rect.Width( ) / 2, rect.Height( ) / 2 );
	//mouse display pos
	CPoint mouse_display( ::MulDiv( ratio.cy, ptMouse.x + m_scrollPos.cx, ratio.cx ), ::MulDiv( ratio.cy, ptMouse.y + m_scrollPos.cx, ratio.cx ) );
	//scale display size
	CSize nsize( ::MulDiv( ratio.cy, m_displaySize.cx, ratio.cx ), ::MulDiv(  ratio.cy, m_displaySize.cy, ratio.cx ) );
	MWTRACE( "OnMouseWheel displaySize x: %d y: %d\n", nsize.cx, nsize.cy );

	//	CPoint ptDisAt( rect.left - m_scrollPos.cx, rect.top - m_scrollPos.cy );
	CPoint ptDisAt( ptMouse + m_scrollPos );
	CPoint msize( ::MulDiv( ratio.cy, ptDisAt.x, ratio.cx ), ::MulDiv(  ratio.cy, ptDisAt.y, ratio.cx ) );

	m_scrollPos.SetSize( ::MulDiv( ratio.cy, m_scrollPos.cx, ratio.cx ), ::MulDiv(  ratio.cy, m_scrollPos.cy, ratio.cx ) );

	CSize move( ptMouse - np );
	MWTRACE( "  OnMouseWheel move x: %d y: %d\n", move.cx, move.cy );
	m_scrollPos+= move;
	if( m_scrollPos.cx < 0 )
		m_scrollPos.cx= 0;
	if( m_scrollPos.cy < 0 )
		m_scrollPos.cy= 0;

	m_displaySize= nsize;

	MWTRACE( "     scroll_pos x: %d y: %d\n\n", m_scrollPos.cx, m_scrollPos.cy );
	CRect wrect;
	GetClientRectSB( m_attachWnd, rect );
    CSize deltaPos( 0, 0 );

	//UpdateScrollBar( SB_HORZ, wrect.Width( ), m_displaySize.cx, m_pageSize.cx, m_scrollPos.cx, deltaPos.cx );
	//UpdateScrollBar( SB_VERT, wrect.Height( ), m_displaySize.cy, m_pageSize.cy, m_scrollPos.cy, deltaPos.cy );

	::ClientToScreen( m_attachWnd->GetSafeHwnd( ), &np );
	SetCursorPos( np.x, np.y );

	UpdateScrollInfo( );
	return TRUE;
}

// ............................................................................
bool WinScroller::UpdateScrollInfo( )
{
    if( m_attachWnd == NULL )
        return false;

    // Get the width/height of the attached wnd that includes the area
    // covered by the scrollbars (if any). The reason we need this is
    // because when scrollbars are present, both cx/cy and GetClientRect()
    // when accessed from OnSize() do not include the scrollbar covered
    // areas. In other words, their values are smaller than what you would
    // expect.
    CRect rect;
    GetClientRectSB( m_attachWnd, rect );
    CSize windowSize( rect.Width( ), rect.Height( ) );

    // Update horizontal scrollbar.
    CSize deltaPos( 0, 0 );
    UpdateScrollBar( SB_HORZ, windowSize.cx, m_displaySize.cx, m_pageSize.cx, m_scrollPos.cx, deltaPos.cx );

    // Update vertical scrollbar.
    UpdateScrollBar( SB_VERT, windowSize.cy, m_displaySize.cy, m_pageSize.cy, m_scrollPos.cy, deltaPos.cy );

	//TRACE( "\ndelta x: %d y: %d\n", deltaPos.cx, deltaPos.cy );
	//TRACE( "display x: %d y: %d\n", m_displaySize.cx, m_displaySize.cy );
	//TRACE( "windowSize x: %d y: %d\n", windowSize.cx, windowSize.cy );
	//TRACE( "m_pageSize x: %d y: %d\n", m_pageSize.cx, m_pageSize.cy );
	//TRACE( "m_scrollPos x: %d y: %d\n", m_scrollPos.cx, m_scrollPos.cy );
    // See if we need to scroll the window back in place.
    // This is needed to handle the case where the scrollbar is
    // moved all the way to the right for example, and controls
    // at the left side disappear from the view. Then the user
    // resizes the window wider until scrollbars disappear. Without
    // this code below, the controls off the page will be gone forever.
  //  if( deltaPos.cx != 0 || deltaPos.cy != 0 )
  //  {
  //      //m_attachWnd->ScrollWindow( deltaPos.cx, deltaPos.cy );
		//TRACE( "ScrollWindow x: %d y: %d\n\n", deltaPos.cx, deltaPos.cy );
		//return true;
  //  }
	return false;
}

// ............................................................................
void WinScroller::UpdateScrollBar(int bar, int windowSize, int displaySize,
                                    LONG& pageSize, LONG& scrollPos, LONG& deltaPos)
{
    int scrollMax= 0;
    deltaPos= 0;
    if ( windowSize < displaySize )
    {
        scrollMax= displaySize - 1;
        if ( pageSize > 0 && scrollPos > 0 )
        {
            // Adjust the scroll position when the window size is changed.
            //scrollPos= (LONG)( 1.0 * scrollPos * windowSize / pageSize );
        }
        pageSize= windowSize;
        scrollPos= min( scrollPos, displaySize - pageSize - 1 );
        deltaPos= m_attachWnd->GetScrollPos( bar ) - scrollPos;
    }
    else
    {
        // Force the scrollbar to go away.
        pageSize= 0;
        scrollPos= 0;
        deltaPos= m_attachWnd->GetScrollPos( bar );
    }

	SCROLLINFO si= { 0 };
    si.cbSize=	sizeof( SCROLLINFO );
    si.fMask=	SIF_ALL;    // SIF_ALL = SIF_PAGE | SIF_RANGE | SIF_POS;
    si.nMin=	0;
    si.nMax=	scrollMax;
    si.nPage=	pageSize;
    si.nPos=	scrollPos;
    m_attachWnd->SetScrollInfo( bar, &si, TRUE );
}

// ............................................................................
int WinScroller::Get32BitScrollPos(int bar, CScrollBar* pScrollBar)
{
    // Code below is from MSDN Article ID 152252, "How To Get
    // 32-bit Scroll Position During Scroll Messages".

    // First determine if the user scrolled a scroll bar control
    // on the window or scrolled the window itself.
    ASSERT( m_attachWnd != NULL );
    HWND hWndScroll;
    if ( pScrollBar == NULL )
        hWndScroll = m_attachWnd->m_hWnd;
    else
        hWndScroll = pScrollBar->m_hWnd;

    SCROLLINFO si;
    si.cbSize = sizeof(SCROLLINFO);
    si.fMask = SIF_TRACKPOS;
    ::GetScrollInfo(hWndScroll, bar, &si);

    int scrollPos = si.nTrackPos;

    return scrollPos;
}

// ............................................................................
void WinScroller::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
    if ( m_attachWnd == NULL )
        return;

    const int lineOffset = 60;

    // Compute the desired change or delta in scroll position.
    int deltaPos = 0;
    switch( nSBCode )
    {
    case SB_LINELEFT:
        // Left scroll arrow was pressed.
        deltaPos = -lineOffset;
        break;

    case SB_LINERIGHT:
        // Right scroll arrow was pressed.
        deltaPos = lineOffset;
        break;

    case SB_PAGELEFT:
        // User clicked inbetween left arrow and thumb.
        deltaPos = -m_pageSize.cx;
        break;

    case SB_PAGERIGHT:
        // User clicked inbetween thumb and right arrow.
        deltaPos = m_pageSize.cx;
        break;

    case SB_THUMBTRACK:
        // Scrollbar thumb is being dragged.
        deltaPos = Get32BitScrollPos(SB_HORZ, pScrollBar) - m_scrollPos.cx;
        break;

    case SB_THUMBPOSITION:
        // Scrollbar thumb was released.
        deltaPos = Get32BitScrollPos(SB_HORZ, pScrollBar) - m_scrollPos.cx;
        break;

    default:
        // We don't process other scrollbar messages.
        return;
    }

    // Compute the new scroll position.
    int newScrollPos = m_scrollPos.cx + deltaPos;

    // If the new scroll position is negative, we adjust
    // deltaPos in order to scroll the window back to origin.
    if ( newScrollPos < 0 )
        deltaPos = -m_scrollPos.cx;

    // If the new scroll position is greater than the max scroll position,
    // we adjust deltaPos in order to scroll the window precisely to the
    // maximum position.
    int maxScrollPos = m_displaySize.cx - m_pageSize.cx;
    if ( newScrollPos > maxScrollPos )
        deltaPos = maxScrollPos - m_scrollPos.cx;

    // Scroll the window if needed.
    if ( deltaPos != 0 )
    {
        m_scrollPos.cx += deltaPos;
        m_attachWnd->SetScrollPos(SB_HORZ, m_scrollPos.cx, TRUE);
        m_attachWnd->ScrollWindow(-deltaPos, 0);
    }
}

// ............................................................................
void WinScroller::OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
    if ( m_attachWnd == NULL )
        return;

    const int lineOffset = 60;

    // Compute the desired change or delta in scroll position.
    int deltaPos = 0;
    switch( nSBCode )
    {
    case SB_LINEUP:
        // Up arrow button on scrollbar was pressed.
        deltaPos = -lineOffset;
        break;

    case SB_LINEDOWN:
        // Down arrow button on scrollbar was pressed.
        deltaPos = lineOffset;
        break;

    case SB_PAGEUP:
        // User clicked inbetween up arrow and thumb.
        deltaPos = -m_pageSize.cy;
        break;

    case SB_PAGEDOWN:
        // User clicked inbetween thumb and down arrow.
        deltaPos = m_pageSize.cy;
        break;

    case SB_THUMBTRACK:
        // Scrollbar thumb is being dragged.
        deltaPos = Get32BitScrollPos(SB_VERT, pScrollBar) - m_scrollPos.cy;
        break;

    case SB_THUMBPOSITION:
        // Scrollbar thumb was released.
        deltaPos = Get32BitScrollPos(SB_VERT, pScrollBar) - m_scrollPos.cy;
        break;

    default:
        // We don't process other scrollbar messages.
        return;
    }

    // Compute the new scroll position.
    int newScrollPos = m_scrollPos.cy + deltaPos;

    // If the new scroll position is negative, we adjust
    // deltaPos in order to scroll the window back to origin.
    if ( newScrollPos < 0 )
        deltaPos = -m_scrollPos.cy;

    // If the new scroll position is greater than the max scroll position,
    // we adjust deltaPos in order to scroll the window precisely to the
    // maximum position.
    int maxScrollPos = m_displaySize.cy - m_pageSize.cy;
    if ( newScrollPos > maxScrollPos )
        deltaPos = maxScrollPos - m_scrollPos.cy;

    // Scroll the window if needed.
    if ( deltaPos != 0 )
    {
        m_scrollPos.cy += deltaPos;
        m_attachWnd->SetScrollPos(SB_VERT, m_scrollPos.cy, TRUE);
        m_attachWnd->ScrollWindow(0, -deltaPos);
    }
}

// ............................................................................
// Helper function to get client rect with possible
// modification by adding scrollbar width/height.
void GetClientRectSB( CWnd* pWnd, CRect& rect )
{
    ASSERT( pWnd != NULL );

    CRect winRect;
    pWnd->GetWindowRect( &winRect );
    pWnd->ScreenToClient( &winRect );

    pWnd->GetClientRect( &rect );

    int cxSB = ::GetSystemMetrics( SM_CXVSCROLL );
    int cySB = ::GetSystemMetrics( SM_CYHSCROLL );

    if( winRect.right >= rect.right + cxSB )
        rect.right+= cxSB;
    if( winRect.bottom >= rect.bottom + cySB )
        rect.bottom+= cySB;
}

// ............................................................................
void WinScroller::SetDisplaySize( int displayWidth, int displayHeight )
{
    m_displaySize= CSize( displayWidth, displayHeight );

    if ( m_attachWnd != NULL && ::IsWindow( m_attachWnd->m_hWnd ) )
        UpdateScrollInfo( );
}

// ............................................................................
void WinScroller::ScrollToOrigin( bool scrollLeft, bool scrollTop )
{
    if ( m_attachWnd == NULL )
        return;

    if ( scrollLeft )
    {
        if ( m_displaySize.cx > 0 && m_pageSize.cx > 0 && m_scrollPos.cx > 0 )
        {
            int deltaPos = -m_scrollPos.cx;
            m_scrollPos.cx += deltaPos;
            m_attachWnd->SetScrollPos(SB_HORZ, m_scrollPos.cx, TRUE);
            m_attachWnd->ScrollWindow(-deltaPos, 0);
        }
    }

    if ( scrollTop )
    {
        if ( m_displaySize.cy > 0 && m_pageSize.cy > 0 && m_scrollPos.cy > 0 )
        {
            int deltaPos = -m_scrollPos.cy;
            m_scrollPos.cy += deltaPos;
            m_attachWnd->SetScrollPos(SB_VERT, m_scrollPos.cy, TRUE);
            m_attachWnd->ScrollWindow(0, -deltaPos);
        }
    }
}

// END

