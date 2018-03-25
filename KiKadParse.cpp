
#include "stdafx.h"
#include "debug.h"
//#include "globals.h"
#include "board.h"
//#include "Share.h"
#include "KiCadParse.h"

// ...........................................................................
using namespace debug_;

//#define D_KICAD D_WOUT
#define D_KICAD( x ) (std::clog << x)

// variant helpers ...................................................................
enum variant_pos
{
	var_uint_pos= 0,
	var_double_pos,
	var_str_pos,
};

inline const unsigned int& get_uint( values const& v, size_t pos ) { return boost::get< unsigned int >( v.at( pos ) ); }
//some strs will parse as ints
inline const std::tstring& get_str( values const& v, size_t pos )
{
	if( v.at( pos ).which( ) == var_uint_pos )
	{
		//TODO !!
		static std::tstring t= boost::lexical_cast< std::tstring >( boost::get< unsigned int >( v.at( pos ) ) );
		return t;
	}

	return boost::get< std::tstring >( v.at( pos ) );
}

//some doubles will parse as ints
inline const double& get_double( values const& v, size_t pos )
{
	if( v.at( pos ).which( ) == var_uint_pos )
	{
		unsigned int t= boost::get< unsigned int >( v.at( pos ) );
		value& tv=  const_cast< value& >( v.at( pos ) );
		tv= (double)( t * MILL_MULT );
		return boost::get< double >( v.at( pos ) );
	}
	else // is double
	{
		double& t= const_cast< double& >( boost::get< double >( v.at( pos ) ) );
		t*= MILL_MULT;
		return t;
	}
}

// ...........................................................................
namespace KiCadParser
{
	enum en_main
	{
		e_null,
		e_general,
		e_layers,
		e_setup,
		e_net,
		e_net_class,
		e_module,
		e_gr_text,
		e_gr_line,
		e_segment,
		e_via,
		e_zone,
		e_fp_text,
		e_fp_line,
		e_pad,
	};
	enum en_general
	{
		eg_links,
		eg_no_connects,
		eg_area,
		eg_thickness,
		eg_tracks,
		eg_modules,
		eg_nets,
		eg_work_area,
	};
	enum en_module
	{
		em_self,
		em_layer,
		em_fp_text,
		em_fp_line,
		em_pad,
		em_model,
		em_gr_text,
	};
	enum es_type
	{
		es_null,
		es_at,
		es_size,
		es_start,
		es_end,
		es_net,
		es_layer,
		es_width,
		es_drill,
	};
	enum en_text
	{
		et_effects,
		et_font,
	};
	void pcnew_wrapper::Init( )
	{
		using namespace boost::assign;
		insert( main )
			( _T("general"), e_general )
			( _T("general"), e_general )
			( _T("layers"), e_layers )
			( _T("setup"), e_setup )
			( _T("net"), e_net )
			( _T("net_class"), e_net_class )
			( _T("module"), e_module )
			( _T("gr_text"), e_gr_text )
			( _T("gr_line"), e_gr_line )
			( _T("segment"), e_segment )
			( _T("via"), e_via )
			( _T("zone"), e_zone )
			( _T("fp_line"), e_fp_line )
			( _T("fp_text"), e_fp_text );

		insert( general )
			( _T("links"), eg_links )
			( _T("no_connects"), eg_no_connects )
			( _T("area"), eg_area )
			( _T("tracks"), eg_tracks )
			( _T("modules"), eg_modules )
			( _T("work_area"), eg_work_area )
			( _T("nets"), eg_nets );

		insert( module )
			( _T("layer"), em_layer )
			( _T("fp_text"), em_fp_text )
			( _T("fp_line"), em_fp_line )
			( _T("pad"), em_pad )
			( _T("model"), em_model )
			( _T("gr_text"), em_gr_text );

		insert( subs )
			( _T("at"), es_at )
			( _T("size"), es_size )
			( _T("start"), es_start )
			( _T("end"), es_end )
			( _T("net"), es_net )
			( _T("layer"), es_layer )
			( _T("drill"), es_drill )
			( _T("width"), es_width );
	}

// sub helpers ...........................................................................

	TPCB_NUM perm_num= 0;
	TPCB_UINT perm_uint= 0;
	bg_point perm_pt( 0, 0 );
	Component* get_commponent( target& t )
	{
		Component* pC= dynamic_cast< Component* >( t.sp_target.get( ) );
		assert( pC );
		return pC;
	}

	SymbolSet* get_symbol( target& t )
	{
		SymbolSet* pS= dynamic_cast< SymbolSet* >( t.sp_target.get( ) );
		assert( pS );
		return pS;
	}

	Connect* get_connect( target& t )
	{
		Connect* pS= dynamic_cast< Connect* >( t.sp_target.get( ) );
		assert( pS );
		return pS;
	}

	Line* get_line( target& t )
	{
		Line* pS= dynamic_cast< Line* >( t.sp_target.get( ) );
		assert( pS );
		return pS;
	}

	bg_point& get_at( s_target& t )
	{
		switch( t.top( ).id )
		{
		case e_module:
			return get_commponent( t.top( ) )->mpos;

		case e_fp_text:
			return get_symbol( t.down( 1 ) )->min_ext;

		case e_pad:
			return get_connect( t.top( ) )->pt1;

		case e_gr_text:
			return get_symbol( t.top( ) )->min_ext;

		case e_segment:
		case e_gr_line:
		case e_fp_line:
			return get_line( t.top( ) )->pt1;

		case e_via:
			return get_connect( t.top( ) )->pt1;

		default:
			assert( false );
			return perm_pt;
		}
	}

	bg_point& get_to( s_target& t )
	{
		switch( t.top( ).id )
		{
		case e_module:
			return get_commponent( t.top( ) )->tpos;

		case e_fp_text:
			return get_symbol( t.down( 1 ) )->max_ext;

		case e_pad:
			return get_connect( t.top( ) )->pt2;

		case e_gr_text:
			return get_symbol( t.top( ) )->max_ext;

		case e_fp_line:
		case e_gr_line:
		case e_segment:
			return get_line( t.top( ) )->pt2;

		case e_via:
			return get_connect( t.top( ) )->pt2;

		default:
			assert( false );
			return perm_pt;
		}
	}

	TPCB_NUM& get_rotate( target& t )
	{
		switch( t.id )
		{
		case e_module:
			return get_commponent( t )->rotate;

		default:
			return perm_num;
		}
	}

	TPCB_NUM& get_size( target& t )
	{
		switch( t.id )
		{
		case e_fp_line:
		case e_segment:
			return get_line( t )->size;

		default:
			return perm_num;
		}
	}

	TPCB_UINT& get_net( target& t )
	{
		switch( t.id )
		{
		case e_segment:
			return get_line( t )->net;

		case e_pad:
			return get_connect( t )->net;

		default:
			return perm_uint;
		}
	}

	TPCB_NUM& get_drill( target& t )
	{
		switch( t.id )
		{
		case e_pad:
			return get_connect( t )->drill.size;

		default:
			return perm_num;
		}
	}

	pad_flag get_pad_type( pair_type const& vals, size_t index )
	{
		const std::tstring& tag( get_str( vals.second, index ) );
		//switch?
		if( tag == _T("thru_hole") )
			return PCSF_STD;

		else// TODO !
			return PCSF_SMD;
	}

	pad_flag get_pad_shape( pair_type const& vals, size_t index )
	{
		const std::tstring& tag( get_str( vals.second, index ) );
		//switch?
		if( tag == _T("oval") || tag == _T("circle") )
			return PCSF_ROUND;
		else if( tag == _T("rect") )
			return PCSF_RECT;
		else// TODO !
			return PCSF_NULL;
	}

	size_t pcnew_wrapper::GetLayerID( std::tstring const& str )
	{
		for( auto it : pcboard->layers )
			if( it->name == str )
				return it->id;
		//else
		return 0;
	}

// ...........................................................................
//top end, from parser, 4 calls
// ...........................................................................
	//a push means there is more object data on the way, use the stack
	void pcnew_wrapper::kipcb_push_dest( pair_type const& pair )
	{
		D_KICAD( "push: " << "vsize: " << targets.size( ) << " " << pair << std::endl );
		if( targets.size( ) ) // ??
		{
			push_obj( pair );
			return;
		}
		//else
		target t( pair.first );
		auto it= main.find( pair.first );
		if( it != main.end( ) )
		{
			t.id= it->second;
			push_values( t, pair.second );
		}
		targets.push( t );
	}

// ...........................................................................
	//a post means a complete object is avaliable, no stack needed
	void pcnew_wrapper::kipcb_post_dest( pair_type const&pair )
	{
		if( ! targets.size( ) )
		{
			auto it= main.find( pair.first );
			top_post_obj( pair );
		}
		else
			post_obj( pair );
		D_KICAD( " post: " << pair << std::endl );
	}

// ...........................................................................
	//right prin ')' notification, check out the object
	void pcnew_wrapper::kipcb_pop_dest( )
	{
		if( ! targets.size( ) ) //should be eof
		{
			return;
		}

		if( targets.size( ) == 1 ) //some stuff needs delayed insert for uniqueness attribute valid (std::set).
		{
			bool bTest= false;
			switch( targets.top( ).id )
			{
			case e_module:
			{
				SP_Component spComp= boost::dynamic_pointer_cast< Component >( targets.top( ).sp_target );
				pcboard->components.insert( spComp );
				break;
			}
			case e_gr_text:
			{
				//pool is type Base so doesn't need cast but we may change this later
				SP_SymbolSet spS= boost::dynamic_pointer_cast< SymbolSet >( targets.top( ).sp_target );
				pcboard->pool.insert( spS );
				break;
			}
			case e_segment:
				bTest= true;
                [[fallthrough]];
			case e_gr_line:
			{
				SP_Line spLine= boost::dynamic_pointer_cast< Line >( targets.top( ).sp_target );
				Layer* pL= pcboard->GetLayer( targets.top( ).layer );
				spLine->lflags= bTest ? PCLF_WIRE : PCLF_SILK;
				if( pL )
				{
					pL->vsp_lines.push_back( spLine );
				}
				if( bTest )
				{
					//D_WOUT( "wire: " << spLine->pt1 << " " << spLine->pt2 << " s: " << spLine->size << std::endl );
				}
				break;
			}
			case e_via:
			{
				SP_Connect sp_via= boost::dynamic_pointer_cast< Connect >( targets.top( ).sp_target );
				pcboard->vias.push_back( sp_via );
				break;
			}
			}//switch
		}
		else //an object of an object. i.e. pad belongs to module
		{
			switch( targets.top( ).id )
			{
			case e_pad:
			{
				SP_Connect pC= boost::dynamic_pointer_cast< Connect >( targets.top( ).sp_target );
				pC->layer= targets.top( ).layer;
			}

			}//switch
		}
		D_KICAD(  "pop: " << "tsize: " << targets.size( ) << " " << targets.top( ).name << " " << targets.top( ).id << std::endl );

		targets.pop( );
	}

// ...........................................................................
	void pcnew_wrapper::kipcb_word_dest( const std::tstring& s )
	{
		//todo, there is a dangling word in pcnew...
		D_KICAD( "dangling word: " << s << std::endl );
	}
// ...........................................................................
	//end top four calls
// ...........................................................................

// ...........................................................................
	//when push_dest has values, targ is position 1
	//New objects need to be created
	void pcnew_wrapper::push_values( target& targ, values const& vals )
	{
		switch( targ.id )
		{
		case e_module:
		{
			SP_Component spC= SP_Component( new Component );
			targ.sp_target= spC;
			spC->libName= get_str( vals, 0 );
			break;
		}
		case e_gr_text:
		{
			SP_SymbolSet pS= SP_SymbolSet( new SymbolSet );
			targ.sp_target= pS;
			pS->text= get_str( vals, 0 );
			break;
		}
		case e_segment:
		case e_gr_line:
		case e_fp_line:
		{
			SP_Line pS= SP_Line( new Line );
			targ.sp_target= pS;
			break;
		}
		case e_via:
		{
			SP_Connect pS= SP_Connect( new Connect );
			targ.sp_target= pS;
			break;
		}

		}//switch
	}

// ...........................................................................
	//call when stack has valid target and start of next compound object
	void pcnew_wrapper::push_obj( const pair_type& pair )
	{
		switch( targets.top( ).id )
		{
		case e_module:
		{
			auto it= module.find( pair.first );
			if( it == module.end( ) )
			{
				push_target( target( pair.first ) );
				return;
			}

			Component* pC= dynamic_cast< Component* >( targets.bottom( ).sp_target.get( ) );
			assert( pC );
			bool bTest= false;
			switch( it->second )
			{
			case em_fp_text:
				bTest= true;
                [[fallthrough]];
			case em_gr_text:
			{
				//only two dests, if more, they can be pushed to a container
				target t;
				t.name= pair.first;
				t.id= bTest ? static_cast<int>(e_fp_text) : static_cast<int>(em_gr_text);
				if( bTest )
				{
					SP_SymbolSet spP;
					if( get_str( pair.second, 0 ) == _T("reference") )
						spP= pC->sp_refLabel;

					else if( get_str( pair.second, 0 ) == _T("value") )
						spP= pC->sp_valueLabel;

					else
					{
						spP= SP_SymbolSet( new SymbolSet );
						pC->vsp_otherLables.push_back( spP );
					}
					spP->text= get_str( pair.second, 1 );
					t.sp_target= spP;

#ifdef USING_CONSOLE
					if( get_str( pair.second, 0 ) == _T("reference") )
						D_WOUT( "COMP: " << spP->text << std::endl );
#endif
				}
				else
				{
					SP_SymbolSet pS= SP_SymbolSet( new SymbolSet );
					pC->otherSymbolSet.push_back( pS );
					t.sp_target= pS;
				}
				push_target( t );
				break;
			}

			case em_pad:
			{
				SP_Connect pP= SP_Connect( new Connect );
				pP->name= get_str( pair.second, 0 );
				pP->flags|= get_pad_type( pair, 1 );
				pP->flags|= get_pad_shape( pair, 2 );
				pC->vsp_pins.push_back( pP );
				push_target( target( pair.first, e_pad, 0, pP ) );
				break;
			}
			case em_fp_line:
			{
				SP_Line pL= SP_Line( new Line );
				pC->ssp_other.insert( pL );
				push_target( target( pair.first, e_fp_line, 0, pL ) );
				break;
			}

			default:
				D_KICAD( "component target not found: " << pair );
				push_target( target( pair.first ) );

			}//switch module
			break;
		}//case module

		default: //should not get here
			D_KICAD( "top target not found: " << pair << std::endl );
			push_target( target( pair.first ) );

		}//switch top
	}

	// ...........................................................................
	//call when stack has valid target and complete object
	void pcnew_wrapper::post_obj( const pair_type& pair )
	{
		auto sit= subs.find( pair.first );
		if( sit != subs.end( ) )
		{
			if( ! targets.top( ).id )//while not all is parsed
				return;

			switch( sit->second )
			{
			case es_start:
			case es_at:
			{
				//bg_point& pt= get_at( targets );
				bg::assign_values(
					get_at( targets ), get_double( pair.second, 0 ), get_double( pair.second, 1 ) );

				if( pair.second.size( ) > 2 )
					get_rotate( targets.top( ) )= (TPCB_NUM)get_double( pair.second, 2 );
				break;
			}
			case es_end:
			case es_size:
			{
				if( targets.bottom( ).id == e_via ) //special case as 'size' is one unit in KiCAD via
				{
					get_connect( targets.bottom( ) )->size= (TPCB_NUM)get_double( pair.second, 0 );
				}
				else
				{
//					bg_point& pt= get_to( targets );
					bg::assign_values( get_to( targets ), get_double( pair.second, 0 ), get_double( pair.second, 1 ) );
					//if( sit->second == es_size ) //translate to dimensions
					//{
					//	bg_point& ps= get_at( targets );
					//	pt+= ps;
					//}
				}
				break;
			}
			case es_drill:
				get_drill( targets.top( ) )= (TPCB_NUM)get_double( pair.second, 0 );
				break;

			case es_width:
			{
				get_size( targets.top( ) )= (TPCB_NUM)get_double( pair.second, 0 );
				break;
			}
			case es_layer: //set temp as we don't keep this id, is by container
				targets.top( ).layer= GetLayerID( get_str( pair.second, 0 ) );
				break;

			case es_net:
				get_net( targets.top( ) )= get_uint( pair.second, 0 );
				break;

			default:
				D_KICAD( " target not found\n" );
			}//switch
		}
		else
		switch( targets.top( ).id )
		{
		case e_general:
			post_general( pair );
			break;

		case e_layers:
			SP_Layer l= SP_Layer( new Layer );
			l->id= std::stoi( pair.first.c_str( ) );
			if( get_str( pair.second, 1 ) == _T("signal") )
				l->type= PCLT_SIGNAL;
			else
				l->type= PCLT_USER;
			l->name= get_str( pair.second, 0 );
			pcboard->layers.insert( l );
			D_KICAD( "layer: " << l->id << " " << l->name << " s: " << l->type << std::endl );
			break;

		}//switch top.id
	}

// ...........................................................................
	//case when complete object is posted to top like 'net'
	void pcnew_wrapper::top_post_obj( const pair_type& pair )
	{
		//first tier
		auto it= main.find( pair.first );
		if( it == main.end( ) )
			return;

		switch( it->second )
		{
		case e_net: //could do a 'don't push' for complete expressions: (word attr)
		{
			SP_Net sp_net= SP_Net( new NetItem );
			sp_net->id= get_uint( pair.second, 0 );
			sp_net->name=  get_str( pair.second, 1 );
			pcboard->netlist.add( sp_net );
			break;
		}
		}//switch
	}

// ...........................................................................
	void pcnew_wrapper::post_general( const pair_type& pair )
	{
		auto it= general.find( pair.first );
		if( it == general.end( ) )
			return;
		switch( it->second )
		{
		case eg_links:
			info.links= (long)get_uint( pair.second, 0 );
			break;

		case eg_area:
			pcboard->SetExtents(
				(TPCB_NUM)get_double( pair.second, 0 ),
				(TPCB_NUM)get_double( pair.second, 1 ),
				(TPCB_NUM)get_double( pair.second, 2 ),
				(TPCB_NUM)get_double( pair.second, 3 )
			);
			break;

		case eg_no_connects:
			info.no_connects= get_uint( pair.second, 0 );
			break;

		case eg_tracks:
			info.tracks= get_uint( pair.second, 0 );
			break;

		case eg_modules:
			info.modules= get_uint( pair.second, 0 );
			break;

		case eg_nets:
			info.links= get_uint( pair.second, 0 );
			break;

		case eg_work_area:
			pcboard->SetWorkArea(
				(TPCB_NUM)get_double( pair.second, 0 ),
				(TPCB_NUM)get_double( pair.second, 1 ),
				(TPCB_NUM)get_double( pair.second, 2 ),
				(TPCB_NUM)get_double( pair.second, 3 )
			);
			break;
		}//switch
	}

}//namespace KiCadParser

//warning C4244
bool PostKicadParse( PCBoard* pBoard )
{
	//TODO ??
	pBoard->SetWorkArea( 0, 0, pBoard->GetExtents( ).max_corner( ).get_x( ) + 500000, pBoard->GetExtents( ).max_corner( ).get_y( ) + 500000 );

	//we keep components in layout coordinates
	for( auto it= pBoard->components.begin( ); it != pBoard->components.end( ); ++it )
	{
		SP_Component pc= boost::dynamic_pointer_cast< Component >( it->get( )->SP_Copy( ) );
		bgstrat::transform::rotate_transformer< bg::degree, double, 2, 2 > rotate( (double)pc->rotate / MILL_MULT );

		//keep an origanal copy
		pBoard->org_components.insert( pc );

		if( pc->rotate )
		{
			for( auto lit= pc->ssp_other.begin( ); lit != pc->ssp_other.end( ); ++ lit )
			{
				Line* pline= boost::dynamic_pointer_cast< Line >( *lit ).get( );
				assert( pline );
				bg::transform( pline->pt1, pline->pt1, rotate );
				bg::transform( pline->pt2, pline->pt2, rotate );
			}
		}

		for( auto pit= pc->vsp_pins.begin( ); pit != pc->vsp_pins.end( ); ++pit )
		{
			Connect& pin= *pit->get( );
			//translate from gerber to bounding rect
			if( pin.flags & PCSF_STD )
			{
				pin.drill.from= pin.pt_1( );
				bg::transform( pin.drill.from, pin.drill.from, rotate );
			}
			pin.pt1-= pin.pt2 / 2;
			pin.pt2+= pin.pt1;
			if( pc->rotate )
			{
				bg::transform( pin.pt1, pin.pt1, rotate );
				bg::transform( pin.pt2, pin.pt2, rotate );
			}
		}
	}
	for( auto vit : pBoard->vias )//.begin( ); vit != pBoard->vias.end( ); ++ vit )
	{
		Connect& via= get_connect( vit );
		via.pt1;
	}
	return true;
}

		//bg::strategy::transform::translate_transformer< double, 2, 2 > translate( (double)pc->get_pt( ).get_x( ), (double)pc->get_pt( ).get_y( ) );
		//bg::strategy::transform::rotate_transformer< bg::degree, double, 2, 2 > rotate( pc->rotate );
		//bg::strategy::transform::ublas_transformer< double, 2, 2> translateRotate( prod( rotate.matrix( ), translate.matrix( ) ) );
		//translateRotate.apply( bg_point( 0, 0 ), pin.pt1 );
		//translateRotate.apply( bg_point( 0, 0 ), pin.pt2 );
