
#include "stdafx.h"
#include <globals.h>
#include "board.h"
#include "lib_comp.h"
#include "netlist_import.h"

bool TinyCADNetlistImport( CXML& xml, NetListImport& import )
{
	//component library
	CompLibrary lib;
	//from here we have to have hard fast rules, like 'number
	//is an unsigned int. should not be a problem...
	//first add part even if we don't have package descriptions
	XMLNODE node= xml.GetNode( _X("parts") );
	for( size_t i= 0; node.SetCurrentChild( i ); ++i )
	{
		XMLNODE child= node.GetCurrentChild( );
		SP_Component comp= SP_Component( new Component );
		comp->sp_refLabel->text= child.GetAttribute( _X("ref") );
		comp->sp_valueLabel->text= child.GetAttribute( _X("name") );
		XMLNODE att= child.SetChildByAttribute( _X("attribute"), _X("name"), _T("Package") );
		if( att.IsValid( ) )
		{
			lib.LoadPackage( comp, att.GetValue( ) );
		}
		import.components.insert( comp );
	}
	//temp for lookups
	SP_Component comp_lookup( new Component );
	node= xml.GetNode( _X("nets") );
	for( size_t i= 0; node.SetCurrentChild( i ); ++i )
	{
		XMLNODE child= node.GetCurrentChild( );
		SP_Net net= SP_Net( new NetItem );
		net->name= child.GetAttribute( _X("name") );
		net->id= _tstoi( child.GetAttribute( _X("number") ) );
		import.netlist.add( net );

		for( size_t j= 0; child.SetCurrentChild( j ); ++j )
		{
			XMLNODE pin= child.GetCurrentChild( );
			SP_Net_sub_item item= SP_Net_sub_item( new Net_sub_item );
			item->name= pin.GetAttribute( _X("part") );
			item->id= _tstoi( pin.GetAttribute( _X("number") ) );

			comp_lookup->sp_refLabel->text= item->name;
			auto sp_c= import.components.find( comp_lookup );
			if( sp_c != import.components.end( ) )
			{
				item->sp_commponent= *sp_c;
				SP_Connect sp_pin= SP_Connect( new Connect );
				item->sp_connect= sp_pin;
				(*sp_c)->vsp_pins.push_back( sp_pin );
			}
			net->items.push_back( item );
		}
	}
	//at this point no real pcb components have been created
	//that, unless we have package information, yet todo...
	// ** there is a 'parts' description in the export, another yet
	//and somewhat well defined in the Tiny export. <attribute name="Package">
	//for sure otherwise, we need to provide selection, as is typical from here
	return true;
}

//bool TinyCADNetlistImport( CXML& xml, PCBoard& board )
//{
//	//from here we have to have hard fast rules, like 'number
//	//is an unsigned int. should not be a problem...
//	//first add part even if we don't have package descriptions
//	XMLNODE node= xml.GetNode( _X("parts") );
//	for( size_t i= 0; node.SetCurrentChild( i ); ++i )
//	{
//		XMLNODE child= node.GetCurrentChild( );
//		SP_Component comp= SP_Component( new Component );
//		comp->sp_refLabel->text= child.GetAttribute( _X("ref") );
//		comp->sp_valueLabel->text= child.GetAttribute( _X("name") );
//		board.components.insert( comp );
//	}
//	//temp for lookups
//	SP_Component comp_lookup( new Component );
//	node= xml.GetNode( _X("nets") );
//	for( size_t i= 0; node.SetCurrentChild( i ); ++i )
//	{
//		XMLNODE child= node.GetCurrentChild( );
//		SP_Net net= SP_Net( new NetItem );
//		net->name= child.GetAttribute( _X("name") );
//		net->id= _tstoi( child.GetAttribute( _X("number") ) );
//		board.netlist.add( net );
//
//		for( size_t j= 0; child.SetCurrentChild( j ); ++j )
//		{
//			XMLNODE pin= child.GetCurrentChild( );
//			SP_Net_sub_item item= SP_Net_sub_item( new Net_sub_item );
//			item->name= pin.GetAttribute( _X("part") );
//			item->id= _tstoi( pin.GetAttribute( _X("number") ) );
//
//			comp_lookup->sp_refLabel->text= item->name;
//			auto sp_c= board.components.find( comp_lookup );
//			if( sp_c != board.components.end( ) )
//			{
//				item->sp_commponent= *sp_c;
//				SP_Connect sp_pin= SP_Connect( new Connect );
//				item->sp_connect= sp_pin;
//				(*sp_c)->vsp_pins.push_back( sp_pin );
//				//item->sp_connect= 
//			}
//			net->items.push_back( item );
//		}
//	}
//	//at this point no real pcb components have been created
//	//that, unless we have package information, yet todo...
//	// ** there is a 'parts' description in the export, another yet
//	//and somewhat well defined in the Tiny export. 'part.'name for sure
//	//otherwise, we need to provide selection, as is typical from here
//	return true;
//}
//
