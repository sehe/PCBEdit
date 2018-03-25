
//This will include needed headers for the board object

#pragma once

#ifndef BOARD_H
#define BOARD_H

#ifndef GEOMETRY_H
#include "Geometry.h"
#endif

/*
Unicode:
strcmp	_tcscmp
atoi	_tstoi
//tstring is defined in "globals.h"
*/
#ifndef TEST_PARSER
#ifndef GLOBALS_H
//#include <globals.h>
#endif
#endif

#ifndef AFX_STATIC
	#define AFX_STATIC
	#define AFX_STATIC_DATA static
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
}
extern AStrType< PCBItemTypes::eItemTypes > typeSet;


// ..........................................................................
enum lineflag
{
	PCLF_NULL= 0,
	PCLF_WIRE,
	PCLF_SILK,
};

// ..........................................................................
enum pad_flag
{
	//these flags can be or'd
	PCSF_NULL= 0,

	//shape
	PCSF_ROUND=	0x0001, //bounding rect determins accual shape
	PCSF_RECT=	0x0002,
	//TODO, we really don't need a trapizoid if rotate is used
	PCSF_TRAP=	0x0004, //rect is shifted 45 degrees

	PCFS_SHAPE_MASK= PCSF_ROUND | PCSF_RECT,

	//type
	PCSF_STD=	0x0010, //through hole pin, drill size should not be zero
	PCSF_SMD=	0x0020, //no hole, drill size should be zero
	PCSF_VIA=	0x0040,

	PCFS_TYPE_MASK= PCSF_STD | PCSF_SMD | PCSF_VIA,
};
//DEFINE_ENUM_FLAG_OPERATORS( pad_flag )
inline pad_flag& operator |= (pad_flag& a, pad_flag b) { return (a= static_cast<pad_flag>(static_cast<int>(a) | static_cast<int>(b))); }
// ..........................................................................
enum layerType
{
	PCLT_NULL= 0,
	PCLT_SIGNAL,
	PCLT_USER,
};

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
struct Connect : public Base
{
	TPCB_STR name; //'1','A23',''...
	TPCB_STR number;
	TPCB_UINT net;
	TPCB_NUM layer;
	//this is the bounding box
	bg_point pt1;
	bg_point pt2;
	TPCB_NUM rotate; //tenths degree
	pad_flag flags;

	//so this is not used
	TPCB_NUM size; //as thickness in opensource layout program “PCB”
	TPCB_NUM clearance;
	TPCB_NUM mask;
	Drill drill;

	Connect( )
		:Base( PCBItemTypes::ptypeConnect )
		,net( 0 )
		,pt1( 0, 0 )
		,pt2( 0, 0 )
		,rotate( 0 )
		,flags( PCSF_NULL )
		,size( 0 )
		,clearance( 0 )
		,mask( 0 )
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
	TPCB_STR libName;
	VSP_Line bname;
	SP_SymbolSet sp_refLabel;//'U1','R2'...
	SP_SymbolSet sp_valueLabel;//other '10K'...
	VSP_SymbolSet vsp_otherLables; //like user pin numbers
	TPCB_ROTATE rotate; //just one
	bg_point mpos;
	bg_point tpos;
	TPCB_NUM scale;
	VSP_Connect vsp_pins;
	SSP_Base ssp_other;
	VSP_SymbolSet otherSymbolSet;

	Component( )
		:Base( PCBItemTypes::ptypeComponent )
		,sp_refLabel( new SymbolSet )
		,sp_valueLabel( new SymbolSet )
		,rotate( 0 )
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
struct op_lib_component_comp
{
	bool operator() ( const SP_Component& a, const SP_Component& b )const { return a->libName < b->libName; }
};
typedef std::set< SP_Component, op_component_comp > SSP_Component;
typedef std::set< SP_Component, op_component_comp >::iterator SSP_Component_it;
typedef std::set< SP_Component, op_component_comp >::const_iterator SSP_Component_cit;
typedef std::set< SP_Component, op_lib_component_comp > SSP_Lib_Component;
typedef std::set< SP_Component, op_lib_component_comp >::iterator SSP_Lib_Component_it;
typedef std::set< SP_Component, op_lib_component_comp >::const_iterator SSP_Lib_Component_cit;

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
	VSP_Netlist& GetNetlist( ) { return netlist.netlist; }
};

// ............................................................................
bg_box GetBounds( SP_SSP_Node nodes );

//...........................................................................
//DRC
// ..........................................................................
typedef bg::model::point< double, 2, bg::cs::cartesian > g_point;
typedef bg::model::linestring< g_point >::iterator g_point_it;

#endif //BOARD_H

