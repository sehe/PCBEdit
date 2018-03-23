
#include "stdafx.h"

#include "board.h"
#include "SharePCB.h"
#include "RatList.h"

#include "MainFrm.h"
#include "netlist_import.h"
#include "pcbDoc.h"

// .......................................................................
void Properties::Populate( )
{
}

// .......................................................................
CMainFrame* GetMainFrame( )
{
	return dynamic_cast< CMainFrame* >( GetMainWindow( ) );
}

// .......................................................................
PCBDoc* GetActiveDocument( )
{
	CMDIChildWnd* pFrmView= GetMainFrame( )->MDIGetActive( );
	return dynamic_cast< PCBDoc* >( pFrmView->GetActiveDocument( ) );
}

void PropertiesItem::SetVar( COleVariant const& vIn )
{
	switch( vIn.vt )
	{
	case VT_BOOL:
		SetVar( !! vIn.boolVal );
		break;

	case VT_BSTR:
		SetVar( vIn.bstrVal );
		break;

	case VT_I4:
		SetVar( vIn.llVal );
		break;

	}
}
