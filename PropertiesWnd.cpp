
#include "stdafx.h"

#include "board.h"
#include "SharePCB.h"
#include "PropertiesWnd.h"
#include "Resource.h"
#include "MainFrm.h"
#include "pcb.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

/////////////////////////////////////////////////////////////////////////////
// CResourceViewBar

CPropertiesWnd::CPropertiesWnd( )
{
}

CPropertiesWnd::~CPropertiesWnd( )
{
}

BEGIN_MESSAGE_MAP( CPropertiesWnd, CDockablePane )
	ON_WM_CREATE( )
	ON_WM_SIZE( )
	ON_COMMAND( ID_EXPAND_ALL, OnExpandAllProperties )
	ON_UPDATE_COMMAND_UI( ID_EXPAND_ALL, OnUpdateExpandAllProperties )
	ON_COMMAND( ID_SORTPROPERTIES, OnSortProperties)
	ON_UPDATE_COMMAND_UI( ID_SORTPROPERTIES, OnUpdateSortProperties )
	ON_COMMAND( ID_PROPERTIES1, OnProperties1)
	ON_UPDATE_COMMAND_UI( ID_PROPERTIES1, OnUpdateProperties1 )
	ON_COMMAND( ID_PROPERTIES2, OnProperties2)
	ON_UPDATE_COMMAND_UI( ID_PROPERTIES2, OnUpdateProperties2 )
	ON_WM_SETFOCUS( )
	ON_WM_SETTINGCHANGE( )
	ON_REGISTERED_MESSAGE( AFX_WM_PROPERTY_CHANGED, OnPropChanged )
END_MESSAGE_MAP( )

/////////////////////////////////////////////////////////////////////////////
//AFX_WM_PROPERTY_CHANGED
//Sent to the owner of the property grid control (CMFCPropertyGridCtrl) when the user changes the value of the selected property.
//The control ID of the property list.
//A pointer to the property (CMFCPropertyGridProperty) that changed.

LRESULT CPropertiesWnd::OnPropChanged( WPARAM wp, LPARAM lp )
{
	using namespace PropertiesSpace;
	if( ! sp_props.get( ) || sp_props.use_count( ) == 1 ) //there is no longer an owner out there!
	{
		//testing.....
		if( ! sp_props.get( ) )
			return FALSE;

		m_wndPropList.RemoveAll( );
		//warn/log as internal error
		return FALSE;
	}
	assert( wp == ID_PROPERTIESWND );
	CMFCPropertyGridProperty* pP= reinterpret_cast< CMFCPropertyGridProperty* >( lp ); //
	ASSERT_KINDOF( CMFCPropertyGridProperty, pP );

	SP_PropertiesVectIt it= std::find_if(
		sp_props->begin( ), sp_props->end( )
		,[ pP ] ( const SP_PropertiesItem& i ) { return i->pGridItem == pP; } );

	if( it == sp_props->end( ) )
		return FALSE;

	switch( it->get( )->GetType( ) )
	{
	case engHeader:
		break;

	case engTrueFalse:
		if( it->get( )->callback( it->get( ) ) )
			pP->SetOriginalValue( pP->GetValue( ) );
		else
			pP->SetValue( pP->GetOriginalValue( ) );
		break;

	case engDropDown://is a drop down
	{
		LPCTSTR psStr= pP->GetValue( ).bstrVal;
		const string_set& set= it->get( )->get_set( );
		vect_pstr_cit sit= std::find_if(
			set.second.begin( ), set.second.end( ), not1(bind1st(std::ptr_fun(_tcscmp), psStr ) ) );
		if( sit != set.second.end( ) )
			it->get( )->SetNewSetSel( sit - set.second.begin( ) );
		break;
	}
	default:
		it->get( )->SetVar( pP->GetValue( ) );

		if( it != sp_props->end( ) && it->get( )->callback )//TODO find out how to change the property highlight
			if( it->get( )->callback( it->get( ) ) )
				pP->SetOriginalValue( pP->GetValue( ) );
			else
				pP->SetValue( pP->GetOriginalValue( ) );

	}//switch(...)

	return FALSE;
}

// ............................................................................
// called via mainframe with: 	SendAppMessage( APP_PROPERTIES_NOTIFY, (LPARAM)&sp_prop, NULL );
void CPropertiesWnd::ChangeContent( SP_Properties sp_in )
{
	using namespace PropertiesSpace;

	m_wndPropList.RemoveAll( );
	if( ! sp_in.get( ) )//was not initialized so just clear properties
		return;

	//keep a local copy
	sp_props= sp_in;
	CMFCPropertyGridProperty* pGroup= NULL;
	for( auto it= sp_in->begin( ); it != sp_in->end( ); ++it )
	{
		PropertiesItem* pi= it->get( );
		switch( pi->GetType( ) )
		{
		case engHeader:
			if( pGroup )
				m_wndPropList.AddProperty( pGroup );
			pGroup= new CMFCPropertyGridProperty( pi->GetLabel( ) );
			pi->pGridItem= pGroup;
			m_wndPropList.AddProperty( pGroup );
			pGroup->Expand( );
			break;

		case engTrueFalse:
			assert( pGroup );
			pi->pGridItem= new CMFCPropertyGridProperty( pi->GetLabel( ), pi->get_truefalse( ), pi->GetHelp( ) );
			pGroup->AddSubItem( pi->pGridItem );
			break;

		case engText:
			assert( pGroup );
			pi->pGridItem= new CMFCPropertyGridProperty( pi->GetLabel( ),  (_variant_t)pi->get_pstr( ), pi->GetHelp( ) );
			pGroup->AddSubItem( pi->pGridItem );
			break;

		case engDropDown:
			assert( pGroup );
			pi->pGridItem= new CMFCPropertyGridProperty( pi->GetLabel( ), pi->get_pstr_set( ), pi->GetHelp( ) );
			pGroup->AddSubItem( pi->pGridItem );
			for( size_t i= 0; i < pi->get_pstr_set_size( ); ++i )
				pi->pGridItem->AddOption( pi->get_pstr_set_at( i ) );
			break;

		}//switch
	}
	m_wndPropList.ExpandAll( );
}

void CPropertiesWnd::AdjustLayout( )
{
	if (GetSafeHwnd() == NULL)
	{
		return;
	}

	CRect rectClient,rectCombo;
	GetClientRect(rectClient);

	propertiesComboSet.GetWindowRect(&rectCombo);

	int cyCmb = rectCombo.Size().cy;
	int cyTlb = m_wndToolBar.CalcFixedLayout(FALSE, TRUE).cy;

	propertiesComboSet.SetWindowPos(NULL, rectClient.left, rectClient.top, rectClient.Width(), 200, SWP_NOACTIVATE | SWP_NOZORDER);
	m_wndToolBar.SetWindowPos(NULL, rectClient.left, rectClient.top + cyCmb, rectClient.Width(), cyTlb, SWP_NOACTIVATE | SWP_NOZORDER);
	m_wndPropList.SetWindowPos(NULL, rectClient.left, rectClient.top + cyCmb + cyTlb, rectClient.Width(), rectClient.Height() -(cyCmb+cyTlb), SWP_NOACTIVATE | SWP_NOZORDER);
}

int CPropertiesWnd::OnCreate( LPCREATESTRUCT lpCreateStruct )
{
	if( CDockablePane::OnCreate(lpCreateStruct ) == -1 )
		return -1;

	CRect rectDummy;
	rectDummy.SetRectEmpty( );

	// Create combo:
	const DWORD dwViewStyle = WS_CHILD | WS_VISIBLE | CBS_DROPDOWNLIST | WS_BORDER | CBS_SORT | WS_CLIPSIBLINGS | WS_CLIPCHILDREN;

	if( ! propertiesComboSet.Create( dwViewStyle, rectDummy, this, 1 ) )
	{
		TRACE0("Failed to create Properties Combo \n");
		return -1;      // fail to create
	}

	propertiesComboSet.AddString(_T("Application"));
	propertiesComboSet.AddString(_T("Properties Window"));
	propertiesComboSet.SetCurSel(0);

	if( ! m_wndPropList.Create( WS_VISIBLE | WS_CHILD, rectDummy, this, ID_PROPERTIESWND ) )
	{
		TRACE0("Failed to create Properties Grid \n");
		return -1;      // fail to create
	}

	InitPropList( );

	m_wndToolBar.Create(this, AFX_DEFAULT_TOOLBAR_STYLE, IDR_PROPERTIES);
	m_wndToolBar.LoadToolBar(IDR_PROPERTIES, 0, 0, TRUE /* Is locked */);
	m_wndToolBar.CleanUpLockedImages();
	m_wndToolBar.LoadBitmap(theApp.m_bHiColorIcons ? IDB_PROPERTIES_HC : IDR_PROPERTIES, 0, 0, TRUE /* Locked */);

	m_wndToolBar.SetPaneStyle(m_wndToolBar.GetPaneStyle() | CBRS_TOOLTIPS | CBRS_FLYBY);
	m_wndToolBar.SetPaneStyle(m_wndToolBar.GetPaneStyle() & ~(CBRS_GRIPPER | CBRS_SIZE_DYNAMIC | CBRS_BORDER_TOP | CBRS_BORDER_BOTTOM | CBRS_BORDER_LEFT | CBRS_BORDER_RIGHT));
	m_wndToolBar.SetOwner(this);

	// All commands will be routed via this control , not via the parent frame:
	m_wndToolBar.SetRouteCommandsViaFrame( FALSE );

	AdjustLayout( );
	return 0;
}

void CPropertiesWnd::OnSize(UINT nType, int cx, int cy)
{
	CDockablePane::OnSize(nType, cx, cy);
	AdjustLayout();
}

void CPropertiesWnd::OnExpandAllProperties()
{
	m_wndPropList.ExpandAll();
}

void CPropertiesWnd::OnUpdateExpandAllProperties(CCmdUI* /* pCmdUI */)
{
}

void CPropertiesWnd::OnSortProperties()
{
	m_wndPropList.SetAlphabeticMode(!m_wndPropList.IsAlphabeticMode());
}

void CPropertiesWnd::OnUpdateSortProperties(CCmdUI* pCmdUI)
{
	pCmdUI->SetCheck(m_wndPropList.IsAlphabeticMode());
}

void CPropertiesWnd::OnProperties1( )
{
	// TODO: Add your command handler code here
}

void CPropertiesWnd::OnUpdateProperties1( CCmdUI* pC )
{
	pC->Enable( FALSE );
}

void CPropertiesWnd::OnProperties2( )
{
	// TODO: Add your command handler code here
}

void CPropertiesWnd::OnUpdateProperties2( CCmdUI* pC )
{
	pC->Enable( FALSE );
}

void CPropertiesWnd::InitPropList( )
{
	SetPropListFont( );

	m_wndPropList.EnableHeaderCtrl( FALSE );
	m_wndPropList.EnableDescriptionArea( );
	m_wndPropList.SetVSDotNetLook( );
	m_wndPropList.MarkModifiedProperties( );

	CMFCPropertyGridProperty* pGroup1 = new CMFCPropertyGridProperty(_T("No Current Properties"));
	m_wndPropList.AddProperty(pGroup1);
	//return;


	pGroup1->AddSubItem(new CMFCPropertyGridProperty(_T("Lib Name"), (_variant_t) false, _T("From Loaded Library\nName of Component")));

	CMFCPropertyGridProperty* pProp = new CMFCPropertyGridProperty(_T("Border"), _T("Dialog Frame"), _T("One of: None, Thin, Resizable, or Dialog Frame"));
	pProp->AddOption(_T("None"));
	pProp->AddOption(_T("Thin"));
	pProp->AddOption(_T("Resizable"));
	pProp->AddOption(_T("Dialog Frame"));
	pProp->AllowEdit(FALSE);

	pGroup1->AddSubItem(pProp);
	pGroup1->AddSubItem(new CMFCPropertyGridProperty(_T("Caption"), (_variant_t) _T("About"), _T("Specifies the text that will be displayed in the window's title bar")));


	CMFCPropertyGridProperty* pSize = new CMFCPropertyGridProperty(_T("Window Size"), 0, TRUE);

	pProp = new CMFCPropertyGridProperty(_T("Height"), (_variant_t) 250l, _T("Specifies the window's height"));
	pProp->EnableSpinControl(TRUE, 50, 300);
	pSize->AddSubItem(pProp);

	pProp = new CMFCPropertyGridProperty( _T("Width"), (_variant_t) 150l, _T("Specifies the window's width"));
	pProp->EnableSpinControl(TRUE, 50, 200);
	pSize->AddSubItem(pProp);

	m_wndPropList.AddProperty(pSize);

	CMFCPropertyGridProperty* pGroup2 = new CMFCPropertyGridProperty(_T("Font"));

	LOGFONT lf;
	CFont* font = CFont::FromHandle((HFONT) GetStockObject(DEFAULT_GUI_FONT));
	font->GetLogFont(&lf);

	lstrcpy(lf.lfFaceName, _T("Arial"));

	pGroup2->AddSubItem(new CMFCPropertyGridFontProperty(_T("Font"), lf, CF_EFFECTS | CF_SCREENFONTS, _T("Specifies the default font for the window")));
	pGroup2->AddSubItem(new CMFCPropertyGridProperty(_T("Use System Font"), (_variant_t) true, _T("Specifies that the window uses MS Shell Dlg font")));

	m_wndPropList.AddProperty(pGroup2);

	CMFCPropertyGridProperty* pGroup3 = new CMFCPropertyGridProperty(_T("Misc"));
	pProp = new CMFCPropertyGridProperty(_T("(Name)"), _T("Application"));
	pProp->Enable(FALSE);
	pGroup3->AddSubItem(pProp);

	CMFCPropertyGridColorProperty* pColorProp = new CMFCPropertyGridColorProperty(_T("Window Color"), RGB(210, 192, 254), NULL, _T("Specifies the default window color"));
	pColorProp->EnableOtherButton(_T("Other..."));
	pColorProp->EnableAutomaticButton(_T("Default"), ::GetSysColor(COLOR_3DFACE));
	pGroup3->AddSubItem(pColorProp);

	static const TCHAR szFilter[] = _T("Icon Files(*.ico)|*.ico|All Files(*.*)|*.*||");
	pGroup3->AddSubItem(new CMFCPropertyGridFileProperty(_T("Icon"), TRUE, _T(""), _T("ico"), 0, szFilter, _T("Specifies the window icon")));

	pGroup3->AddSubItem(new CMFCPropertyGridFileProperty(_T("Folder"), _T("c:\\")));

	m_wndPropList.AddProperty(pGroup3);

	CMFCPropertyGridProperty* pGroup4 = new CMFCPropertyGridProperty(_T("Hierarchy"));

	CMFCPropertyGridProperty* pGroup41 = new CMFCPropertyGridProperty(_T("First sub-level"));
	pGroup4->AddSubItem(pGroup41);

	CMFCPropertyGridProperty* pGroup411 = new CMFCPropertyGridProperty(_T("Second sub-level"));
	pGroup41->AddSubItem(pGroup411);

	pGroup411->AddSubItem(new CMFCPropertyGridProperty(_T("Item 1"), (_variant_t) _T("Value 1"), _T("This is a description")));
	pGroup411->AddSubItem(new CMFCPropertyGridProperty(_T("Item 2"), (_variant_t) _T("Value 2"), _T("This is a description")));
	pGroup411->AddSubItem(new CMFCPropertyGridProperty(_T("Item 3"), (_variant_t) _T("Value 3"), _T("This is a description")));

	pGroup4->Expand(FALSE);
	m_wndPropList.AddProperty(pGroup4);
}

void CPropertiesWnd::OnSetFocus(CWnd* pOldWnd)
{
	CDockablePane::OnSetFocus(pOldWnd);
	m_wndPropList.SetFocus();
}

void CPropertiesWnd::OnSettingChange(UINT uFlags, LPCTSTR lpszSection)
{
	CDockablePane::OnSettingChange(uFlags, lpszSection);
	SetPropListFont();
}

void CPropertiesWnd::SetPropListFont( )
{
	::DeleteObject( m_fntPropList.Detach( ) );

	LOGFONT lf;
	afxGlobalData.fontRegular.GetLogFont( &lf );

	NONCLIENTMETRICS info;
	info.cbSize = sizeof( info );

	afxGlobalData.GetNonClientMetrics( info );

	lf.lfHeight= info.lfMenuFont.lfHeight;
	lf.lfWeight= info.lfMenuFont.lfWeight;
	lf.lfItalic= info.lfMenuFont.lfItalic;

	m_fntPropList.CreateFontIndirect( &lf );

	m_wndPropList.SetFont( &m_fntPropList );
	propertiesComboSet.SetFont( &m_fntPropList );
}
