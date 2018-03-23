

#include "stdafx.h"
#include <globals.h>

#include "board.h"
#include "Share.h"
#include "FreePCBParse.h"
// ..........................................................................
//Pad [rX1 rY1 rX2 rY2 Thickness Clearance Mask "Name" "Number" SFlags]
//Pin [rX rY Thickness Clearance Mask Drill "Name" "Number" SFlags]
//Element [SFlags "Desc" "Name" "Value" MX MY TX TY TDir TScale TSFlags]
//ElementLine[rX1 rY1 rX2 rY2 Thickness]
//ElementArc [rX rY Width Height StartAngle DeltaAngle Thickness]
//Element [SFlags "Desc" "Name" "Value" MX MY TX TY TDir TScale TSFlags]
//Line[3375.00mil 15.0150mm 3375.00mil 631.25mil 10.00mil 20.00mil ""]
//ElementLine [-70.00mil -38.00mil 70.00mil -38.00mil 6.00mil]

// ..........................................................................
//simple parsing
value_set::value_set( STR_RANGE instr, PCBdtypes* vars )
	:params( instr )
{
	static boost::escaped_list_separator< wchar_t > escape_list( _T(""), _T(" "), _T("\"\'") );
	tokenizer tok( instr.first, instr.second, escape_list );
	tokenizer_iterator it= tok.begin( );
	//TRACE( "%s\n", std::string(  it->begin( ), it->end( ) ).c_str( ) );
	for( size_t index= 0; it != tok.end( ); ++it, ++index )
		switch( vars[ index ] )
	{
		case ptdouble:
		{
			std::tstring ts( it->begin( ), it->end( ) );
			tvars.push_back( (TPCB_NUM)( _tstof( ts.c_str( ) ) * MILL_MULT ) );
			break;
		}
		case ptnum:
		{
			std::tstring ts( it->begin( ), it->end( ) );
			if( ts.rfind( _T("mil") ) != std::tstring::npos
				|| ts.rfind( _T(".") ) != std::tstring::npos )
			//)//old school
				tvars.push_back( (TPCB_NUM)( _tstof( ts.c_str( ) ) * MILL_MULT ) );

			else
				tvars.push_back( (TPCB_NUM)( _tstof( ts.c_str( ) ) ) );

			break;
		}
		case ptstr:
			tvars.push_back( TPCB_STR( it->begin( ), it->end( ) ) );

			break;

		case ptchar:
			tvars.push_back( (TPCB_CHAR)*it->begin( ) );

			break;

		default:
			tvars.push_back( (TPCB_NUM)0 );

	}
}

// ..........................................................................
using namespace FreePCBItemTypes;
AStrType< eaItemTypes > PCBApptypeSet( ASTRTYPE_DEF( psPCBAItemTypes ) );

//typedef boost::weak_ptr< Base > Visitor;
// ..........................................................................
void FreePCBParser::Parse( const TCHAR* line )
{
	//TODO simplify & remove CString artifact
	//should really use spirit
	CString strT( line );
	static size_t count= 1;
	++count;
	//the likes of CString out of here and portable......
	CString strN;
	CString strB;
	static PCB_HANDLE last_handle= NULL;

	strT.Trim( _T(" \t") );
	if( strT.IsEmpty( ) || strT[ 0 ] == '#' )
		return;

	if( strT == _T("(") )
	{
		if( ! last_handle )//testing and ??
			return;

		h_stack.push( last_handle );
	
		return;
	}
	else if( strT == _T(")") )
	{
		if( ! h_stack.size( ) )//testing
			return;

		assert( h_stack.size( ) );
		h_stack.pop( );
		last_handle= NULL;
	}

	std::tstring str= strT;
	STR_IT strf;
	strf= std::find( str.begin( ), str.end( ), _T('[') );
	if( strf == str.end( ) )
		strf= std::find( str.begin( ), str.end( ), _T('(') );

	if( strf == str.end( ) )
		return;

	bool bNoName= false;
	if( strf == str.begin( ) )
		bNoName= true;

	*strf= _T('\0');
	*( str.end( ) - 1 )= _T('\0');
	STR_RANGE name( str.begin( ),  bNoName ? strf : strf - 1 );
	STR_RANGE params( strf + 1, str.end( ) );

	if( h_stack.size( ) )
		last_handle= loadItemStr( h_stack.top( ), name, params );
	else
		last_handle= loadItemStr( name, params );
}

// ..........................................................................
//top tier
PCB_HANDLE FreePCBParser::loadItemStr( STR_RANGE name, STR_RANGE params )
{
	//just get the job done....
	eaItemTypes type= PCBApptypeSet.stot( &*name.first );
	if( type == (eaItemTypes)-1 )//not found
		return NULL;

	switch( type )
	{
	//top tier containers here
	case ptypeElement:
	{
		SP_Component e( new Component );
		ElementParse( e.get( ), params );
		pTarget->components.insert( e );
		pTarget->pool.insert( e );
		return (PCB_HANDLE)e.get( );
	}
	case ptypeLayer:
	{
		SP_Layer l( new Layer );
		pTarget->layers.insert( l );
		pTarget->pool.insert( l );
		return (PCB_HANDLE)l.get( );
	}
	case ptypeVia:
	{
		SP_Connect c( new Connect );
		ConnectParse( c.get( ), ptypeVia, params );
		pTarget->vias.push_back( c );
		return (PCB_HANDLE)NULL;
	}
	case ptypeSymbol:
	{
		SP_Symbol s( new Symbol );
		value_set vs( params, symbol_vars );
		//TODO, for '"'
		if( vs.size( ) < 2 )
			return (PCB_HANDLE)NULL;
		s->chr= vs.get_char( 0 );
		s->size= vs.get_num( 1 );
		pTarget->symbols.insert( s );
		return (PCB_HANDLE)s.get( );
	}
	case ptypeNetList:
	{
		return (PCB_HANDLE)&pTarget->netlist;
	}
	case ptypePCB:
	{
		value_set vs( params, PCB_vars );
		pTarget->GetExtents( ).max_corner( ).set< 0 >( vs.get_num( 1 ) );
		pTarget->GetExtents( ).max_corner( ).set< 1 >( vs.get_num( 2 ) );
		return NULL;
	}
	default:
		//do nothing
		break;

	}//switch

	return (PCB_HANDLE)NULL;
}

// ..........................................................................
//sub tier
PCB_HANDLE FreePCBParser::loadItemStr( PCB_HANDLE hnd, STR_RANGE name, STR_RANGE params )
{
	eaItemTypes type= PCBApptypeSet.stot( &*name.first );
	if( type == (eaItemTypes)-1 )//not found
		return NULL;

	Base* pBase= (Base*)hnd;

	switch( pBase->type )
	{
	case PCBItemTypes::ptypeComponent:
	{
		Component* pe= dynamic_cast< Component* >( pBase );
		if( type == ptypePad || type == ptypePin )
		{
			SP_Connect sp_connect( new Connect );
			ConnectParse( sp_connect.get( ), type, params );
			pe->vsp_pins.push_back( sp_connect );
		}
		else if( type == ptypeElementLine )
		{
			SP_Line sp_line( new Line );
			sp_line->lflags= PCLF_SILK;
			LineParse( sp_line.get( ), ptypeElementLine, params );
			pe->ssp_other.insert( sp_line );
		}
		break;
	}
	case PCBItemTypes::ptypeLayer:
	{
		Layer* pe= dynamic_cast< Layer* >( pBase );
		if( ! _tcscmp( &*name.first, _T("Line") ) )
		{
			SP_Line sp_line( new Line );
			sp_line->lflags= PCLF_WIRE;
			LineParse( sp_line.get( ), ptypeLine, params );
			pe->vsp_lines.push_back( sp_line );
		}
		break;
	}
	case PCBItemTypes::ptypeNetList:
	{
		Netlist* pn= dynamic_cast< Netlist* >( pBase );
		assert( pn );
		SP_Net net( new NetItem );
		value_set vs( params, net_vars );
		if( vs.size( ) != 2 )
			return (PCB_HANDLE)NULL;
		net->name= vs.get_str( 0 );
		//net->other= vs.get_str( 1 );
		pn->add( net );
		return (PCB_HANDLE)net.get( );
	}
	case PCBItemTypes::ptypeNetItem:
	{
		NetItem* pn= dynamic_cast< NetItem* >( pBase );
		assert( pn );
		SP_Net net( new NetItem );
		STR_IT sit= std::find( params.first, params.second, _T('-') );
		if( sit != params.second ) //sanity
		{
			*sit= _T('\0');
			SP_Component c( new Component );
			c->sp_refLabel->text= &*params.first + 1;
			auto sp_c= pTarget->components.find( c );
			if( sp_c != pTarget->components.end( ) )
			{
				SP_Net_sub_item sp_si= SP_Net_sub_item( new Net_sub_item );
				sp_si->sp_commponent= *sp_c;
				size_t num= _tstoi( &*( sit + 1 ) );
				sp_si->sp_connect= (*sp_c)->vsp_pins.at( num - 1 );
				pn->items.push_back( sp_si );
			}
		}
		return (PCB_HANDLE)NULL;
	}
	}//switch
	return (PCB_HANDLE)NULL;
}

// ..........................................................................
//Element [SFlags		"Desc"			"Name"	"Value"		MX			MY			TX		TY		TDir	TScale		TSFlags]
//Element["hidename"	"screw-4-40.fp"	"MH3"	"unknown"	200.00mil	200.00mil	0.0000	0.0000	0		100			""]
void FreePCBParser::ElementParse( Component* pE, STR_RANGE params )
{
	value_set vs( params, element_vars );
//	pE->flags= flagsSet.GetFlags( vs.get_str( 0 ).c_str( ) );
	pE->desc= vs.get_str( 1 );
	pE->sp_refLabel->text= vs.get_str( 2 );
	pE->sp_valueLabel->text= vs.get_str( 3 );
	pE->mpos.set< 0 >( vs.get_num( 4 ) );
	pE->mpos.set< 1 >( vs.get_num( 5 ) );
	pE->tpos.set< 0 >( vs.get_num( 6 ) );
	pE->tpos.set< 1 >( vs.get_num( 7 ) );
//	pE->rflag= (rotationflag)vs.get_num( 8 );
	pE->scale= vs.get_num( 9 );
}

// ..........................................................................
//	Pad[ rX1			rY1			rX2			rY2		Thickness	Clearance	Mask		"Name"	"Number"	SFlags ]
//	Pad[ 36.00mil	-4.00mil	36.00mil	4.00mil		48.00mil	12.00mil	54.00mil	"1"		"1"			"square" ]

//	Pin[ rX			rY			Thickness	Clearance	Mask		Drill		"Name"		"Number"	SFlags ]
//	Pin[ 350.00mil	75.00mil	65.00mil	20.00mil	71.00mil	35.00mil	"I5"		"6"			""]

//	Via[ 2210.00mil	2621.00mil	50.00mil	40.00mil	0.0000		30.00mil	"			"thermal(5S)" ]

void FreePCBParser::ConnectParse( Connect* pC, eaItemTypes type, STR_RANGE params )
{
	PCBdtypes* dt= NULL;
	if( type == ptypePad )
	{
		dt= connect_pad_vars;
		pC->flags|= PCSF_SMD;
	}
	else if( type == ptypePin )
	{
		dt= connect_pin_vars;
		pC->flags|= PCSF_STD;
	}
	else if( type == ptypeVia )
	{
		dt= connect_via_vars;
		pC->flags|= PCSF_VIA;
	}
	else
	{
		assert( false );
		return;//release sanity
	}
	value_set vs( params, dt );
	TPCB_NUM vs0= vs.get_num( 0 );
	TPCB_NUM vs1= vs.get_num( 1 );
	TPCB_NUM vs2= vs.get_num( 2 );
	TPCB_NUM vs3= vs.get_num( 3 );
	TPCB_NUM vs4= vs.get_num( 4 );
	TPCB_NUM vs5= vs.get_num( 5 );

	size_t index;
	if( type == ptypePad )
	{
		//convert 'PCB' line to rect
		TPCB_NUM vssd= vs4 / 2;
		pC->set_pt1( bg_point( vs0 - vssd, vs1 - vssd ) );
		pC->set_pt2( bg_point( vs2 + vssd, vs3 + vssd ) );
		pC->clearance= vs5;
		pC->mask= vs.get_num( 6 );
		index= 7;
	}
	else //pin/via
	{
		pC->set_pt1( bg_point( vs0, vs1 ) );
		pC->size= vs2;
		pC->clearance= vs3;
		pC->mask= vs4;
		pC->drill.size= vs5;
		index= 6;
	}
	if( type != ptypeVia )
	{
		pC->name= vs.get_str( index );
		//not used and Free does not designate layer
		//pC->index= _tstoi( vs.get_str( index++ ).c_str( ) );
		pC->number= vs.get_str(index++ );
	}
//	pC->flags= flagsSet.GetFlags( vs.get_str( index++ ).c_str( ) );
	//net value to be added when netlist is read...
}

// ..........................................................................
//Assuming, found no documentation
//Line [ rX1	rY1		rX2		rY2		Thickness	Clearance		TSFlags ]
//Line [ 199250	90500	202500	90500	800			2000			"clearline" ]
//ElementLine [ rX1		rY1		rX2		rY2		Thickness ]
//ElementLine [ -6250	-2500	-6250	2500	600 ]

void FreePCBParser::LineParse( Line* pL, FreePCBItemTypes::eaItemTypes type, STR_RANGE params )
{
	value_set vs( params, connect_line_vars );
	pL->set_pt1( bg_point( vs.get_num( 0 ), vs.get_num( 1 ) ) );
	pL->set_pt2( bg_point( vs.get_num( 2 ), vs.get_num( 3 ) ) );
	pL->size= vs.get_num( 4 );
	if( type == ptypeLine )
	{
		pL->clearance= vs.get_num( 5 );
		//pL->flags= flagsSet.GetFlags( vs.get_str( 6 ).c_str( ) );
	}
}




