
#include "stdafx.h"
#ifdef TEST_PARSER
#include <loc_globals.h>
#else
#include <globals.h>
#endif
#include "board.h"

// ..........................................................................
using namespace PCBItemTypes;
//using namespace PCBFlagTypes;
AStrType< eItemTypes > typeSet( ASTRTYPE_DEF( psPCBItemTypes ) );
//AStrTFlag< typeflags > flagsSet( ASTRTYPE_DEF( psPCBFlagTypes ) );

bg_point Node::pt_min( ) const
{
	bg_point pt;
	switch( sp_obj.get( )->type )
	{
	case ptypeLine:
	{
		Line* line= dynamic_cast< Line* >( sp_obj.get( ) );
		assert( line );
		pt.set< 0 >( line->x1( ) < line->x2( ) ? line->x1( ) : line->x2( ) );
		pt.set< 1 >( line->y1( ) < line->y2( ) ? line->y1( ) : line->y2( ) );
		pt-= bg_point( line->size, line->size );
	}
	}//switch( ... )
	return pt;
}

bg_point Node::pt_max( ) const
{
	bg_point pt;
	switch( sp_obj.get( )->type )
	{
	case ptypeLine:
	{
		//bg_box box( 
		Line* line= dynamic_cast< Line* >( sp_obj.get( ) );
		assert( line );
		pt.set< 0 >( line->x1( ) > line->x2( ) ? line->x1( ) : line->x2( ) );
		pt.set< 1 >( line->y1( ) > line->y2( ) ? line->y1( ) : line->y2( ) );
		pt+= bg_point( line->size, line->size );
	}
	}//switch( ... )
	return pt;
}

Layer* PCBoard::GetLayer( TPCB_UINT layer_id )
{
	auto it= layers.begin( );
	for( ; it != layers.end( ); ++it )
		if( it->get( )->id == layer_id )
			break;
	if( it != layers.end( ) )
		return it->get( );
	else
		return NULL;
}
