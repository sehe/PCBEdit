
#include "stdafx.h"
#include "board.h"
#include "Drawing.h"

// ............................................................................
void DrawArea::SetPen( COLORREF const& color, size_t width, int inStyle )
{
	width= width ? width : 1;
	pen_logbrush.lbColor= color;
	PCPEN pen( color, width, inStyle );
	PenSetIt it= pen_set.find( pen );
	if( it == pen_set.end( ) )
	{
		pen.sp_Pen= SP_CPen( new CPen(inStyle, width, &pen_logbrush ) );
		pDC->SelectObject( const_cast< CPen* >( pen.sp_Pen.get( ) ) );
		pen_set.insert( pen );
		spLastPen= pen.sp_Pen;
		//TRACE( "%3.0d NewPen: %I64d\n", pen_set.size( ), (__int64)lb.lbColor | ( (__int64)width << 32 ) );
	}
	else if( it->sp_Pen.get( ) != spLastPen.get( ) )
	{
		pDC->SelectObject( const_cast< CPen* >( it->sp_Pen.get( ) ) );
		spLastPen= it->sp_Pen;
	}
}

//// ............................................................................
void DrawArea::PopPenBrush( )
{
}

//// ............................................................................
void DrawArea::SetDCBrush( COLORREF const& color )
{
	pDC->SelectStockObject( DC_BRUSH );
	pDC->SetDCBrushColor( color );
}

//// ............................................................................
//void DrawArea::SetBrush( LOGBRUSH& lb, int inStyle )
//{
//	PCBRUSH brush( lb.lbColor, inStyle );
//	BrushSetIt it= brush_set.find( brush );
//	if( it == brush_set.end( ) )
//	{
//		brush.sp_Brush= SP_CBrush( new CBrush( &lb,  ) );
//		pDC->SelectObject( const_cast< CPen* >( brush.sp_Pen.get( ) ) );
//		pen_set.insert( pen );
//		pLastPen= pen.sp_Pen.get( );
//		//TRACE( "%3.0d NewPen: %I64d\n", pen_set.size( ), (__int64)lb.lbColor | ( (__int64)width << 32 ) );
//	}
//	else if( it->sp_Brush.get( ) != pLastPen )
//	{
//		pDC->SelectObject( const_cast< CPen* >( it->sp_Brush.get( ) ) );
//		pLastPen= const_cast< CBrush* >( it->sp_Brush.get( ) );
//	}
//}
//
// ............................................................................
void DrawArea::DrawRect( bg_box& rect )
{
	CRect srect(
		(long)( rect.min_corner( ).get_x( ) / draw.zoom + draw.of_x ),
		(long)( rect.min_corner( ).get_y( ) / draw.zoom + draw.of_y ),
		(long)( rect.max_corner( ).get_x( ) / draw.zoom + draw.of_x ),
		(long)( rect.max_corner( ).get_y( ) / draw.zoom + draw.of_y )
	);
	pDC->Rectangle( &srect );
}

// ............................................................................
void DrawArea::DrawShape( bg_point const& bound_min, bg_point const& bound_max, const pad_flag flag )
{
	CRect srect(
		(long)( bound_min.get_x( ) / draw.zoom + draw.of_x ),
		(long)( bound_min.get_y( ) / draw.zoom + draw.of_y ),
		(long)( bound_max.get_x( ) / draw.zoom + draw.of_x ),
		(long)( bound_max.get_y( ) / draw.zoom + draw.of_y )
	);
	switch( flag & PCFS_SHAPE_MASK )
	{
	case PCSF_ROUND:
		pDC->Ellipse( srect );
		break;

	case PCSF_RECT:
		pDC->Rectangle( srect );
		break;

	default:
		TRACE( "DrawShape: flag not valid\n" );
	}
}

// ............................................................................
void DrawArea::DrawLine( Line const& line )
{
	long t1x= (long)( line.x1( ) / draw.zoom + draw.of_x );
	long t1y= (long)( line.y1( ) / draw.zoom + draw.of_y );
	pDC->MoveTo( t1x, t1y );
	long t2x= (long)( line.x2( ) / draw.zoom + draw.of_x );
	long t2y= (long)( line.y2( ) / draw.zoom + draw.of_y );
	pDC->LineTo( t2x, t2y );
}

// ............................................................................
void DrawArea::DrawLine( Line const& line, bg_point const& offset )
{
	long t1x= (long)( ( line.x1( ) + offset.get_x( ) ) / draw.zoom + draw.of_x );
	long t1y= (long)( ( line.y1( ) + offset.get_y( ) ) / draw.zoom + draw.of_y );
	pDC->MoveTo( t1x, t1y );
	long t2x= (long)( ( line.x2( ) + offset.get_x( ) ) / draw.zoom + draw.of_x );
	long t2y= (long)( ( line.y2( ) + offset.get_y( ) ) / draw.zoom + draw.of_y );
	pDC->LineTo( t2x, t2y );
}

// ............................................................................
void DrawArea::DrawConnect( Connect& con )
{
	long thick= (long)( con.size / 2 );
	CRect rect(
		(long)( ( con.x1( ) - thick ) / draw.zoom + draw.of_x )
		,(long)( ( con.y1( ) - thick ) / draw.zoom + draw.of_y )
		,(long)( ( con.x1( ) + thick ) / draw.zoom + draw.of_x )
		,(long)( ( con.y1( ) + thick ) / draw.zoom + draw.of_y )
	);
	pDC->Ellipse( rect );
}

// ............................................................................
void DrawArea::DrawCross( CPoint pt )
{
	CPen pen( PS_SOLID, 2, RGB( 100, 100, 100 ) );
	pDC->SelectObject( &pen );
	pDC->MoveTo( pt.x - 10, pt.y );
	pDC->LineTo( pt.x + 10, pt.y );
	pDC->MoveTo( pt.x, pt.y - 10 );
	pDC->LineTo( pt.x, pt.y + 10  );
}

// ............................................................................
void DrawArea::DrawBoardCross( bg_point const& pt )
{
	CPen pen( PS_SOLID, 2, RGB( 230, 230, 230 ) );
	CPen* op= pDC->SelectObject( &pen );
	long x= (long)pt.get_x( ) / draw.zoom + draw.of_x;
	long y= (long)pt.get_y( ) / draw.zoom + draw.of_y;
	pDC->MoveTo( x - 10, y - 10 );
	pDC->LineTo( x + 10, y + 10 );
	pDC->MoveTo( x + 10, y - 10 );
	pDC->LineTo( x - 10, y + 10  );
	pDC->SelectObject( op );
}

// ............................................................................
// ............................................................................
// ............................................................................
SP_SSP_Node Locator::FindVia( bg_point in )
{
	SP_SSP_Node nodes( new SSP_Node );
	VSP_Connect vias( board.vias );

	UINT64 last= -1;
	auto it= vias.begin( );
	for( ; it != vias.end( ); ++it )
	{
		if( *it->get( ) == in )
		{
			SP_Node spN= SP_Node( new Node( *it, it->get( )->pt1 ) );
			nodes->insert( spN );
			break;
		}
	}
	return nodes;
}

// ............................................................................
UINT64 Locator::FindLines( bg_point in, SP_SSP_Node spNodes, VSP_Line spLines, UINT64 dist )
{
	UINT64 last= dist;
	auto it= spLines.begin( );
	for( ; it != spLines.end( ); ++it )
	{
		SP_Node spN1= SP_Node( new Node( *it, it->get( )->pt1 ) );
		SP_Node spN2= SP_Node( new Node( *it, it->get( )->pt2 ) );

		UINT64 dist1= (UINT64)bg::comparable_distance( spN1->pt( ), in );
		UINT64 dist2= (UINT64)bg::comparable_distance( spN2->pt( ), in );

		if( dist1 <= last )// || dist1 < last )
		{
			if( dist1 != last )
			{
				spNodes->clear( );
				last= dist1;
			}
			spNodes->insert( spN1 );
//			TRACE( "less1 dx: %d dy: %d \tdist: %I64u\n", bg::get< 0 >( spN1->pt( ) ), bg::get< 1 >( spN1->pt( ) ), dist1 );
		}
		if( dist2 <= last )// || dist1 < last )
		{
			if( dist2 != last )
			{
				spNodes->clear( );
				last= dist2;
			}
			spNodes->insert( spN2 );
		}
	}
	return last;
}

// ............................................................................
SP_SSP_Node Locator::FindClosestNodes( bg_point in )
{
	if( ! board.layers.size( ) )
		return SP_SSP_Node( new SSP_Node );

	SP_SSP_Node nodes( new SSP_Node );
	Layer* pLayer= board.GetLayer( 0 );
	if( ! pLayer )
		return SP_SSP_Node( new SSP_Node );

	VSP_Line lines= pLayer->vsp_lines;
	UINT64 top= FindLines( in, nodes, lines, boost::numeric::bounds< UINT64 >::highest( ) );

	auto tit= board.layers.begin( );
	++tit;
	SP_Layer sp_layerb( *tit );
	lines= sp_layerb->vsp_lines;
	UINT64 bottom= FindLines( in, nodes, lines, top );

	if( ! nodes->size( ) )
		return nodes;

	//VSP_Connect sp_con( board.vias );
	//for( auto it= sp_con.begin( ); it != sp_con.end( ); ++it )
	//	if( it->get( )->pt_1( ) == nodes->begin( )->get( )->pt( ) )
	//		const_cast< SP_SSP_Node >( nodes )->insert( new Node( *it, it->get( )->pt1 ) );

	return nodes;
}


