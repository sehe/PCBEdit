

// Drawing.h

#pragma once
#define DRAWING_H

// ..........................................................................
// ..........................................................................
//drawing.....
typedef boost::shared_ptr< CPen > SP_CPen;
typedef std::vector< SP_CPen > VSP_CPen;
typedef boost::shared_ptr< CBrush > SP_CBrush;
typedef std::vector< SP_CBrush > VSP_CBrush;

AFX_STATIC_DATA	int penStylesRnd=  PS_SOLID | PS_ENDCAP_ROUND | PS_GEOMETRIC;
AFX_STATIC_DATA	int penStylesFlat=  PS_SOLID | PS_ENDCAP_FLAT |PS_GEOMETRIC;

struct PCPEN
{
	COLORREF color;
	int styles;
	size_t width;
	//enum of style...
	SP_CPen sp_Pen;
	PCPEN( COLORREF icolor, size_t iwidth, int inStyles ) //for comp
		:color( icolor )
		,width( iwidth )
		,styles( inStyles )
	{ }

	bool operator < ( const PCPEN& a ) const
	{
		if( styles != a.styles )
			return a.styles < styles;
		return ( (__int64)a.color | ( (__int64)a.width << 32 ) )
			< ( (__int64)color | ( (__int64)width << 32 ) );
	}
	operator CPen* ( ) { return sp_Pen.get( ); }
};

typedef std::set< PCPEN > PenSet;
typedef std::set< PCPEN >::iterator PenSetIt;

//struct PCBRUSH
//{
//	LOGBRUSH lb;
//	int styles;
//	size_t width;
//	//enum of style...
//	SP_CBrush sp_Brush;
//	PCBRUSH( COLORREF icolor, int inStyles ) //for comp
//		//:width( iwidth )
//	{ lb.lbColor= icolor; }
//
//	bool operator < ( const PCBRUSH& a ) const
//	{
//		if( styles != a.styles )
//			return a.styles < styles;
//		return ( (__int64)a.lb.lbColor | ( (__int64)a.width << 32 ) )
//			< ( (__int64)lb.lbColor | ( (__int64)width << 32 ) );
//	}
//	operator CBrush* ( ) { return sp_Brush.get( ); }
//};
//
//typedef std::set< PCBRUSH > BrushSet;
//typedef std::set< PCBRUSH >::iterator BrushSetIt;
//

struct BrushInfo
{
	COLORREF color;
	UINT	type;
};

struct DrawExtent
{
	CRect& rectClient;
	long& of_x;
	long& of_y;
	long& zoom;

	DrawExtent( CRect& rect, long& ix, long& iy, long& izoom )
		:rectClient( rect )
		,of_x( ix )
		,of_y( iy )
		,zoom( izoom )
	{ }
};

class DrawArea
{
	PCBoard& board;
	CDC* pDC;
	DrawExtent draw;
	PenSet pen_set;
//	BrushSet brush_set;
	SP_CPen spLastPen;
	BrushInfo lastBrush;
	LOGBRUSH pen_logbrush;

public:
	DrawArea( CDC* pInDC, PCBoard& inBoard, DrawExtent& iDraw )
		:board( inBoard )
		,pDC( pInDC )
		,draw( iDraw )
		//,pen_set( (CPen*)NULL )
	{
		pDC->SelectStockObject( NULL_BRUSH );
		pen_logbrush.lbColor= RGB( 255, 255, 255 );
		pen_logbrush.lbHatch= 0;
		pen_logbrush.lbStyle= BS_SOLID;
	}
	void DrawItem( Base& item );
	void SetPen( COLORREF const& lb, size_t width, int inStyle );
	void SetDCBrush( COLORREF const& lb );//  { pDC->SelectStockObject( DC_BRUSH ); pDC->SetDCBrushColor( lb ); }
	void SetNullBrush( ) { pDC->SelectStockObject( NULL_BRUSH ); }
	void DrawLine( Line const& line );
	//draw line with offset
	void DrawLine( Line const& line, bg_point const& offset );
	void DrawConnect( Connect& con );
	void DrawCross( CPoint pt );
	void DrawBoardCross( bg_point const& pt );
	void DrawRect( bg_box& rect );
	void DrawShape( bg_point const& bound_min, bg_point const& bound_max, const pad_flag flag );
	void DrawShape( bg_box const& box, const pad_flag flag ) { DrawShape( box.min_corner( ), box.max_corner( ), flag ); }
	void PopPenBrush( );
};

// ............................................................................
class Locator
{
	PCBoard& board;
public:
	Locator( PCBoard& inBoard )
		:board( inBoard )
	{ }
protected:
	UINT64 FindLines( bg_point in, SP_SSP_Node spNodes, VSP_Line spLines, UINT64 dist );

public:
	//in does not have to be an exact hit
	SP_SSP_Node FindClosestNodes( bg_point in );
	//in as via exactly
	SP_SSP_Node FindVia( bg_point in );
};

