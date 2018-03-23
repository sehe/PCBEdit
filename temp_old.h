

#pragma once

/*
Unicode:
strcmp	_tcscmp
atoi	_tstoi
//tstring is defined in "globals.h"
*/
#ifndef TEST_PARSER
#ifndef GLOBALS_H
#include <globals.h>
#endif
#endif

#ifndef AFX_STATIC
	#define AFX_STATIC extern
	#define AFX_STATIC_DATA extern __declspec(selectany)
#endif

// ..........................................................................
typedef std::tstring	TPCB_STR;
typedef uint16_t		TPCB_LAYER;
typedef TPCB_NUM		TPCB_ROTATE; //in tenths of degrees

// ..........................................................................
namespace PCBItemTypes
{
	enum eItemTypes
	{
		ptypeNull= 0,
		ptypeAttribute,
		ptypeNode,	//Drawing object
		ptypePolygon,
		ptypeSymbol,
		ptypeSymbolSet,
		ptypeConnect,//we combine PCBApp Pad, Pin, via
		ptypeLine,	//we combine PCBApp Line, ElementLine
		ptypeComponent,
		ptypeLayer,
		ptypeNetList,
		ptypeNetItem,
		ptypeWirePt,
		ptypeWire,
		ptypeWires,
		ptypeDrill,
	};

	AFX_STATIC_DATA LPCTSTR psPCBItemTypes[]=
	{
		_T("Null"),
		_T("Attribute"),
		_T("Node"),
		_T("Polygon"),
		_T("Symbol"),
		_T("SymbolSet"),
		_T("Connect"),
		_T("Line"),
		_T("Component"),
		_T("Layer"),
		_T("NetList"),
		_T("NetItem"),
		_T("WirePoint"),
		_T("Wire"),
		_T("Wires"),
		_T("Drill"),
	};
};
extern AStrType< PCBItemTypes::eItemTypes > typeSet;

// ..........................................................................

namespace PCBFlagTypes
{
	enum typeflags
	{
		NOFLAG=			0x0000,
		PINFLAG=		0x0001,
		VIAFLAG=		0x0002,
		FOUNDFLAG=		0x0004,
		HOLEFLAG=		0x0008,
		RATFLAG=		0x0010,
		PININPOLYFLAG=	0x0020,
		CLEARPOLYFLAG=	0x0040,
		HIDENAMEFLAG=	0x0080,
		DISPLAYNAMEFLAG=0x0100,
		CLEARLINEFLAG=	0x0200,
		SELECTEDFLAG=	0x0400,
		ONSOLDERFLAG=	0x0800,
		AUTOFLAG=		0x1000,
		SQUAREFLAG=		0x2000,
		RUBBERENDFLAG=	0x4000,
		WARNFLAG=		0x8000,
		USETHERMALFLAG=	0x00010000,
		OCTAGONFLAG=	0x00020000,
		DRCFLAG=		0x00040000,
		LOCKFLAG=		0x00080000,
		EDGE2FLAG=		0x00100000,
		FULLPOLYFLAG=	0x00200000,
		NOPASTEFLAG=	0x00400000,
	};

	AFX_STATIC_DATA LPCTSTR psPCBFlagTypes[]=
	{
	  _T("pin"),
	  _T("via"),
	  _T("found"),
	  _T("hole"),
	  _T("rat"),
	  _T("pininpoly"),
	  _T("clearpoly"),
	  _T("hidename"),
	  _T("showname"),
	  _T("clearline"),
	  _T("selected"),
	  _T("onsolder"),
	  _T("auto"),
	  _T("square"),
	  _T("rubberend"),
	  _T("warn"),
	  _T("usetherm"),
	  _T("octagon"),
	  _T("drc"),
	  _T("lock"),
	  _T("edge2"),
	  _T("fullpoly"),
	  _T("nopaste"),
	};

	//enum enPadShape
	//{
	//	enpNull,
	//	enpCircle,
	//	enpRect,
	//	enpOblong,
	//	enpTrap, //Trapèze
	//};
};

extern AStrTFlag< PCBFlagTypes::typeflags > flagsSet;

// ..........................................................................
enum connectflag
{
	PCCF_NULL= 0,
	PCCF_VIA,
	PCCF_PAD,
	PCCF_PIN,
};

// ..........................................................................
//depretiate and use detrees in tenths
enum rotationflag
{
	//pcb rotates counter clockwise, to be seen...
	PCROT_NONE= 0,
	PCROT_90,
	PCROT_180,
	PCROT_270,
};

// ..........................................................................
enum lineflag
{
	PCLF_NULL= 0,
	PCLF_WIRE,
	PCLF_SILK,
};

// ..........................................................................
enum shapeflag
{
	PCSF_NULL= 0,
	PCSF_ROUND, //bounding rect determins accual shape
	PCSF_RECT,
	PCSF_TRAP, //rect is shifted 45 degrees
};

// ..........................................................................
enum layerType
{
	PCLT_NULL= 0,
	PCLT_SIGNAL,
	PCLT_USER,
};

// ..........................................................................
/*
namespace traits
{
    template <>
    struct access<Line, 0>
    {
        static double get(Line const& p)
        {
            return p.x1;
        }
    };
    // same for 1: p.y
}
*/

// ..........................................................................
struct Base;
typedef boost::shared_ptr< Base > SP_Base;
struct Base //old school way of keeping assorted objects
{
	PCBItemTypes::eItemTypes type;

	Base( PCBItemTypes::eItemTypes intype= PCBItemTypes::ptypeNull )
		:type( intype )
	{ }

	virtual ~Base( ) { }

	virtual bool operator < ( const Base& base ) const
	{
		return  &base < this;
	}
	virtual SP_Base SP_Copy( )= 0;
};

typedef std::set< SP_Base > SSP_Base;
typedef boost::shared_ptr< SSP_Base > SP_SSP_Base;
typedef std::set< SP_Base >::iterator SSP_BaseIt;
typedef std::vector< SP_Base > VSP_Base;
typedef std::stack< SP_Base > StackSP_Base;

// ..........................................................................
//for internal editing 'Node does not exsist in description files
struct Line;
struct Node : public Base
{
	SP_Base sp_obj;
	bg_point& point;
	size_t index;

	Node( SP_Base l, bg_point& p )
		:Base( PCBItemTypes::ptypeNode )
		,sp_obj( l )
		,point( p )
		,index( 0 )
	{ }
	//Node( )
	//	:index( 0 )
	//{ }
	bg_point pt( ) const { return point; }

	//these return bounding points of the sp_obj for dragging
	bg_point pt_min( ) const;
	bg_point pt_max( ) const;

	virtual SP_Base SP_Copy( ) { return SP_Base( new Node( *this ) ); }
};

typedef boost::shared_ptr< Node > SP_Node;
typedef std::set< SP_Node > SSP_Node;
typedef boost::shared_ptr< SSP_Node > SP_SSP_Node;

// ..........................................................................
//Lines of different types will be kept in different containers ?
struct Line : public Base
{
	lineflag	lflags;
	TPCB_UINT	net;
	bg_point	pt1;
	bg_point	pt2;
	TPCB_NUM	size; //as thickness
	TPCB_NUM	clearance;

	Line( )
		:Base( PCBItemTypes::ptypeLine )
		,lflags( PCLF_NULL )
		,net( 0 )
		,size( 0 )
		,clearance( 0 )
	{ }
	bg_point pt_1( ) const { return pt1; }
	bg_point pt_2( ) const { return pt2; }

	TPCB_NUM x1( ) const { return pt1.get< 0 >( ); }
	TPCB_NUM y1( ) const { return pt1.get< 1 >( ); }
	TPCB_NUM x2( ) const { return pt2.get< 0 >( ); }
	TPCB_NUM y2( ) const { return pt2.get< 1 >( ); }

	void set_pt1( const bg_point& in ) { pt1= in; }
	void set_pt2( const bg_point& in ) { pt2= in; }

	virtual SP_Base SP_Copy( ) { return SP_Base( new Line( *this ) ); }
};

typedef boost::shared_ptr< Line > SP_Line;
typedef std::vector< SP_Line > VSP_Line;
typedef boost::shared_ptr< VSP_Line > SP_VSP_Line;

// ..........................................................................
struct Symbol : public Base
{
	TPCB_CHAR chr;
	TPCB_NUM size;
	VSP_Line vsp_lines;

	Symbol( )
		:Base( PCBItemTypes::ptypeSymbol )
	{ }

	bool operator < ( const Symbol& s ) const
	{
		return  s.chr < chr;
	}

	virtual SP_Base SP_Copy( ) { return SP_Base( new Symbol( *this ) ); }
};

typedef boost::shared_ptr< Symbol > SP_Symbol;
typedef std::set< SP_Symbol > SSP_Symbol;

//for one line of text
//from KiCAD: T<field number> <Xpos> <Ypos> <Xsize> <Ysize> <rotation> <penWidth> N <visible> <layer> "text"
struct SymbolSet : public Base
{
	bool bVisable;
	TPCB_STR text;
	bg_point min_ext;
	bg_point max_ext;
	TPCB_ROTATE rotation;
	TPCB_LAYER layer;

	SymbolSet( )
		:Base( PCBItemTypes::ptypeSymbolSet )
		,rotation( 0 )
	{ }

	virtual SP_Base SP_Copy( ) { return SP_Base( new SymbolSet( *this ) ); }
};
typedef boost::shared_ptr< SymbolSet > SP_SymbolSet;
typedef std::vector< SP_SymbolSet > VSP_SymbolSet;

// .........................................................................
struct Drill : public Base
{
	TPCB_NUM size;
	bg_point from;
	bg_point to;

	Drill( )
		:Base( PCBItemTypes::ptypeDrill )
		,size( 0 )
	{ }
	virtual SP_Base SP_Copy( ) { return SP_Base( new Drill( *this ) ); }
};


// ..........................................................................
//Pad [rX1 rY1 rX2 rY2 Thickness Clearance Mask "Name" "Number" SFlags]
//Pin [rX rY Thickness Clearance Mask Drill "Name" "Number" SFlags]
struct Connect : public Base
{
	connectflag cflags;
	TPCB_STR name; //'1','A23',''...
	TPCB_STR number;
	TPCB_UINT net;
	TPCB_NUM layer;
	//this is the bounding box
	bg_point pt1;
	bg_point pt2;
	shapeflag shape;

	// relitive to parent object
	//bg_point pos;
	//so this is not used
	TPCB_NUM size; //as thickness in opensource layout program “PCB”
	TPCB_NUM clearance;
	TPCB_NUM mask;
	Drill drill;

	PCBFlagTypes::typeflags flags;
//	PCBFlagTypes::enPadShape shape;
	//TPCB_ROTATE rotate; //we don't use this internally so would work for post parse if likes of KiCAD

	Connect( )
		:Base( PCBItemTypes::ptypeConnect )
		,cflags( PCCF_NULL )
		,flags( PCBFlagTypes::NOFLAG )
		//,index( -1 )//max uint as null
		,size( 0 )
		,clearance( 0 )
		,mask( 0 )
		,net( 0 )
		//,rotate( 0 )
		,pt1( 0, 0 )
		,pt2( 0, 0 )
	{ }

	bool operator == ( bg_point& in ) { return in == pt1; }

	operator bg_point ( ) const { return pt1; }

	bg_point pt_1( ) const { return pt1; }
	bg_point pt_2( ) const { return pt2; }

	TPCB_NUM x1( ) const { return pt1.get< 0 >( ); }
	TPCB_NUM y1( ) const { return pt1.get< 1 >( ); }
	TPCB_NUM x2( ) const { return pt2.get< 0 >( ); }
	TPCB_NUM y2( ) const { return pt2.get< 1 >( ); }

	void set_pt1( const bg_point& in ) { pt1= in; }
	void set_pt2( const bg_point& in ) { pt2= in; }

	virtual SP_Base SP_Copy( ) { return SP_Base( new Connect( *this ) ); }

};

typedef boost::shared_ptr< Connect > SP_Connect;
typedef std::vector< SP_Connect > VSP_Connect;

inline Connect& get_connect( SP_Base pB )
{
	return *boost::dynamic_pointer_cast< Connect >( pB ).get( );
}

// ..........................................................................
//Components are aggregates
struct Component : public Base
{
	TPCB_STR desc;
	TPCB_STR libName; //as of KiCAD
	SP_SymbolSet sp_refLabel;//'U1','R2'...
	SP_SymbolSet sp_valueLabel;//other
	PCBFlagTypes::typeflags flags; //can be or'd
	TPCB_ROTATE rotate; //just one
	bg_point mpos;
	bg_point tpos;
	TPCB_NUM scale;
	VSP_Connect vsp_pins;
	SSP_Base ssp_other;
	VSP_Line bname;
	VSP_SymbolSet otherSymbolSet;

	Component( )
		:Base( PCBItemTypes::ptypeComponent )
		,rotate( 0 )
		,sp_refLabel( new SymbolSet )
		,sp_valueLabel( new SymbolSet )
	{ }

	virtual bool operator < ( const boost::shared_ptr< Component >& a ) const
	{
		return  a->sp_refLabel->text < sp_refLabel->text;
	}

	virtual SP_Base SP_Copy( ) { return SP_Base( new Component( *this ) ); }
	bg_point const& get_pt( ) const { return mpos; }
	TPCB_NUM Get_mx( ) const { return mpos.get< 0 >( ); }
	TPCB_NUM Get_my( ) const { return mpos.get< 1 >( ); }
	TPCB_NUM Get_tx( ) const { return tpos.get< 0 >( ); }
	TPCB_NUM Get_ty( ) const { return tpos.get< 1 >( ); }
};

typedef boost::shared_ptr< Component > SP_Component;
struct op_component_comp
{
	bool operator() ( const SP_Component& a, const SP_Component& b )const { return a->sp_refLabel->text < b->sp_refLabel->text; }
};
typedef std::set< SP_Component, op_component_comp > SSP_Component;

// ..........................................................................
struct Layer : public Base
{
	TPCB_UINT	id;
	TPCB_STR	name;
	layerType	type;
	//traces will be kept in the layer.
	VSP_Line vsp_lines;

	Layer( )
		:Base( PCBItemTypes::ptypeLayer )
	{ }
	bool operator < ( Layer& a ) { return a.id < id; }
	virtual SP_Base SP_Copy( ) { return SP_Base( new Layer( *this ) ); }
};
typedef boost::shared_ptr< Layer > SP_Layer;
typedef std::set< SP_Layer > VSP_Layer;

// ..........................................................................
//net stuff.
//net items are nodes of the net. with a container of sub items.
//sub items are pointers to other objects that on the net.
struct Net_sub_item : public Base
{
	SP_Component sp_commponent;
	SP_Connect sp_connect;
	std::tstring name;
	TPCB_UINT id;
	//bool operator < ( const Net_item& s ) const
	//{
	//	return  s.id < id;
	//}
	virtual SP_Base SP_Copy( ) { return SP_Base( new Net_sub_item( *this ) ); }
};
typedef boost::shared_ptr< Net_sub_item > SP_Net_sub_item;
typedef std::vector< SP_Net_sub_item > VSP_Net_SubItems;

// ..........................................................................
struct NetItem : public Base
{
	TPCB_STR name;
	TPCB_NUM id; //not used by FreePCB; Not needed if KiCAD is name compliant
	VSP_Net_SubItems items;
	NetItem( )
		:Base( PCBItemTypes::ptypeNetItem )
	{ }

	virtual SP_Base SP_Copy( ) { return SP_Base( new NetItem( *this ) ); }
};
typedef boost::shared_ptr< NetItem > SP_Net;
typedef std::vector< SP_Net > VSP_Netlist;
typedef VSP_Netlist::iterator VSP_NetlistIt;

// ..........................................................................
struct Netlist : public Base
{
	VSP_Netlist netlist;
	Netlist( )
		:Base( PCBItemTypes::ptypeNetList )
	{ }

	void add( SP_Net spNet ) { netlist.push_back( spNet ); }
	VSP_NetlistIt begin( ) { return netlist.begin( ); }
	VSP_NetlistIt end( ) { return netlist.end( ); }
	virtual SP_Base SP_Copy( ) { return SP_Base( new Netlist( *this ) ); }
};

// ..........................................................................
// the board..... Just a repository, does not know how to do much
class PCBoard
{
	bg_box extents;
	bg_box work_area;

public:
	PCBoard( )
		:extents( bg_point( 0, 0 ), bg_point( 0, 0 ) )
	{ }
	bg_box& GetExtents( ) { return extents; }
	TPCB_NUM GetWidth( ) { return extents.max_corner( ).get< 0 >( ) - extents.min_corner( ).get< 0 >( ); }
	TPCB_NUM GetHeight( ) { return extents.max_corner( ).get< 1 >( ) - extents.min_corner( ).get< 1 >( ); }
	bg_point GetSize( ) { return extents.max_corner( ) - extents.min_corner( ); }
	bg_point GetDrawSize( ) { return work_area.max_corner( ) - work_area.min_corner( ); }
	void SetExtents( TPCB_NUM const& x1, TPCB_NUM const& y1, TPCB_NUM const& x2, TPCB_NUM const& y2 )
	{
		extents.min_corner( ).set_x( x1 );
		extents.min_corner( ).set_y( y1 );
		extents.max_corner( ).set_x( x2 );
		extents.max_corner( ).set_y( y2 );
	}
	void SetWorkArea( TPCB_NUM const& x1, TPCB_NUM const& y1, TPCB_NUM const& x2, TPCB_NUM const& y2 )
	{
		work_area.min_corner( ).set_x( x1 );
		work_area.min_corner( ).set_y( y1 );
		work_area.max_corner( ).set_x( x2 );
		work_area.max_corner( ).set_y( y2 );
	}

	SSP_Base pool;
	VSP_Connect vias;
	//could be built in or read like from FreePCB
	SSP_Symbol symbols;
	//other stuff?
	SSP_Base board_pool;

	SSP_Component components;
	SSP_Component org_components;
	VSP_Layer layers;
	Netlist netlist;

	Layer* GetLayer( TPCB_UINT layer_id );
};

// ............................................................................
bg_box GetBounds( SP_SSP_Node nodes );

//...........................................................................
//DRC
// ..........................................................................
typedef bg::model::point< double, 2, bg::cs::cartesian > g_point;
typedef bg::model::linestring< g_point >::iterator g_point_it;


