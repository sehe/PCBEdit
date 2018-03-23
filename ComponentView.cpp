
#include "stdafx.h"
#include "board.h"
#include "Share.h"
#include "RatList.h"
#include "MainFrm.h"
#include "ComponentView.h"
#include "Resource.h"
#include "Resource.h"
#include "pcb.h"

#include "netlist_import.h"
#include "pcbDoc.h"

// .......................................................................
class CClassViewMenuButton : public CMFCToolBarMenuButton
{
	friend class CComponentView;

	DECLARE_SERIAL(CClassViewMenuButton)

public:
	CClassViewMenuButton(HMENU hMenu = NULL) : CMFCToolBarMenuButton((UINT)-1, hMenu, -1)
	{
	}

	virtual void OnDraw(CDC* pDC, const CRect& rect, CMFCToolBarImages* pImages, BOOL bHorz = TRUE,
		BOOL bCustomizeMode = FALSE, BOOL bHighlight = FALSE, BOOL bDrawBorder = TRUE, BOOL bGrayDisabledButtons = TRUE)
	{
		pImages = CMFCToolBar::GetImages();

		CAfxDrawState ds;
		pImages->PrepareDrawImage(ds);

		CMFCToolBarMenuButton::OnDraw(pDC, rect, pImages, bHorz, bCustomizeMode, bHighlight, bDrawBorder, bGrayDisabledButtons);

		pImages->EndDrawImage(ds);
	}
};

IMPLEMENT_SERIAL( CClassViewMenuButton, CMFCToolBarMenuButton, 1 )

// .......................................................................
//////////////////////////////////////////////////////////////////////
BEGIN_MESSAGE_MAP( CComponentView, CDockablePane )
	ON_WM_CREATE( )
	ON_WM_SIZE( )
	ON_WM_CONTEXTMENU( )
	ON_COMMAND( ID_CLASS_ADD_MEMBER_FUNCTION, OnClassAddMemberFunction )
	ON_COMMAND( ID_CLASS_ADD_MEMBER_VARIABLE, OnClassAddMemberVariable )
	ON_COMMAND( ID_CLASS_DEFINITION, OnClassDefinition )
	ON_COMMAND( ID_CLASS_PROPERTIES, OnClassProperties )
	ON_COMMAND( ID_NEW_FOLDER, OnNewFolder )
//	ON_NOTIFY( NM_CLICK, ID_COMPTREECTRL, OnClick )
	ON_NOTIFY( TVN_SELCHANGED, ID_COMPTREECTRL, OnClick )
	ON_WM_PAINT( )
	ON_WM_SETFOCUS( )
	ON_COMMAND_RANGE( ID_SORTING_GROUPBYTYPE, ID_SORTING_SORTBYACCESS, OnSort )
	ON_UPDATE_COMMAND_UI_RANGE( ID_SORTING_GROUPBYTYPE, ID_SORTING_SORTBYACCESS, OnUpdateSort )
END_MESSAGE_MAP( )

// .......................................................................
CComponentView::CComponentView( )
	:m_nCurrSort( ID_SORTING_GROUPBYTYPE )
{
}

// .......................................................................
CComponentView::~CComponentView()
{
}

// .......................................................................
void CComponentView::OnClick( NMHDR * pNotifyStruct, LRESULT * result )
{
	if( ! pNotifyStruct )
	{
		SendAppMessage( APP_PROPERTIES_NOTIFY, (LPARAM)NULL );
		return;
	}
	NMITEMACTIVATE* pS= reinterpret_cast< NMITEMACTIVATE* >( pNotifyStruct );
	CTreeCtrlEx& tree= componentTreeViewCtrl;
	CTreeCursor cur= tree.GetSelectedItem( );
	//testing
	using namespace PropertiesSpace;
	viewCom vc;
	vc.type= enPropTest;
	CTreeCursor tc= componentTreeViewCtrl.GetRootItem( );
	for(  tc= tc.GetFirstSelectedItem( ); (HTREEITEM)tc; tc= tc.GetNextSelectedItem( ) )
		vc.ssp_items.push_back( *(SP_Base*)tc.GetData( ) );

	SP_Properties sp_prop= GetActiveDocument( )->GetProperties( vc );
	SendAppMessage( APP_PROPERTIES_NOTIFY, (LPARAM)&sp_prop, NULL );
}

// .......................................................................
void CComponentView::ChangeContent( WPARAM wp, PCBDoc* pDoc )
{
	PCBoard* pL= reinterpret_cast< PCBoard* >( wp );

//	assert( pL == PcBoard_Class );

	CTreeCtrlEx& tctl= componentTreeViewCtrl;
	CTreeCursor root= componentTreeViewCtrl.GetRootItem( ).AddTail(  _T("Components"), 0, 0 );
	root.SetState( TVIS_BOLD, TVIS_BOLD );
	root.SetState( INDEXTOSTATEIMAGEMASK(0) , TVIS_STATEIMAGEMASK );

	for( auto it= pL->components.begin( ); it != pL->components.end( ); ++it )
	{
		Component& comp= *it->get( );
		CTreeCursor cur= root.AddTail( comp.sp_refLabel->text.c_str( ), 1 );
		cur.SetData( (DWORD)&*it );
	}
	//TODO keep selected items as such
	//clear properties for now
//	LRESULT r;
	//OnClick( (NMHDR*)NULL, &r );
	root.Expand( );
}

// .......................................................................
int CComponentView::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CDockablePane::OnCreate(lpCreateStruct) == -1)
		return -1;

	CRect rectDummy;
	rectDummy.SetRectEmpty();

	// Create views:
	const DWORD dwViewStyle = WS_CHILD | WS_VISIBLE | TVS_HASLINES | TVS_LINESATROOT | TVS_HASBUTTONS | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | TVS_SHOWSELALWAYS;

	if( ! componentTreeViewCtrl.Create( dwViewStyle, rectDummy, this, ID_COMPTREECTRL ) )
	{
		TRACE0("Failed to create Class View\n");
		return -1;      // fail to create
	}

	// Load images:
	m_wndToolBar.Create(this, AFX_DEFAULT_TOOLBAR_STYLE, IDR_SORT);
	m_wndToolBar.LoadToolBar(IDR_SORT, 0, 0, TRUE /* Is locked */);

	OnChangeVisualStyle();

	m_wndToolBar.SetPaneStyle(m_wndToolBar.GetPaneStyle() | CBRS_TOOLTIPS | CBRS_FLYBY);
	m_wndToolBar.SetPaneStyle(m_wndToolBar.GetPaneStyle() & ~(CBRS_GRIPPER | CBRS_SIZE_DYNAMIC | CBRS_BORDER_TOP | CBRS_BORDER_BOTTOM | CBRS_BORDER_LEFT | CBRS_BORDER_RIGHT));

	m_wndToolBar.SetOwner(this);

	// All commands will be routed via this control , not via the parent frame:
	m_wndToolBar.SetRouteCommandsViaFrame(FALSE);

	CMenu menuSort;
	menuSort.LoadMenu(IDR_POPUP_SORT);

	m_wndToolBar.ReplaceButton(ID_SORT_MENU, CClassViewMenuButton(menuSort.GetSubMenu(0)->GetSafeHmenu()));

	CClassViewMenuButton* pButton =  DYNAMIC_DOWNCAST(CClassViewMenuButton, m_wndToolBar.GetButton(0));

	if (pButton != NULL)
	{
		pButton->m_bText = FALSE;
		pButton->m_bImage = TRUE;
		pButton->SetImage(GetCmdMgr()->GetCmdImage(m_nCurrSort));
		pButton->SetMessageWnd(this);
	}

	// Fill in some static tree view data (dummy code, nothing magic here)
	FillClassView( );
	//TODO select last

	//if not selected


	return 0;
}

// .......................................................................
void CComponentView::OnSize(UINT nType, int cx, int cy)
{
	CDockablePane::OnSize(nType, cx, cy);
	AdjustLayout();
}

// .......................................................................
void CComponentView::FillClassView( )
{
	//todo: delete
}

// .......................................................................
void CComponentView::OnContextMenu( CWnd* pWnd, CPoint point )
{
	CTreeCtrl* pWndTree = (CTreeCtrl*)&componentTreeViewCtrl;
	ASSERT_VALID(pWndTree);

	if (pWnd != pWndTree)
	{
		CDockablePane::OnContextMenu(pWnd, point);
		return;
	}

	if (point != CPoint(-1, -1))
	{
		// Select clicked item:
		CPoint ptTree = point;
		pWndTree->ScreenToClient(&ptTree);

		UINT flags = 0;
		HTREEITEM hTreeItem = pWndTree->HitTest(ptTree, &flags);
		if (hTreeItem != NULL)
		{
			pWndTree->SelectItem(hTreeItem);
		}
	}

	pWndTree->SetFocus();
	CMenu menu;
	menu.LoadMenu(IDR_POPUP_SORT);

	CMenu* pSumMenu = menu.GetSubMenu(0);

	if (AfxGetMainWnd()->IsKindOf(RUNTIME_CLASS(CMDIFrameWndEx)))
	{
		CMFCPopupMenu* pPopupMenu = new CMFCPopupMenu;

		if (!pPopupMenu->Create(this, point.x, point.y, (HMENU)pSumMenu->m_hMenu, FALSE, TRUE))
			return;

		((CMDIFrameWndEx*)AfxGetMainWnd())->OnShowPopupMenu(pPopupMenu);
		UpdateDialogControls(this, FALSE);
	}
}

// .......................................................................
void CComponentView::AdjustLayout()
{
	if (GetSafeHwnd() == NULL)
	{
		return;
	}

	CRect rectClient;
	GetClientRect(rectClient);

	int cyTlb = m_wndToolBar.CalcFixedLayout(FALSE, TRUE).cy;

	m_wndToolBar.SetWindowPos(NULL, rectClient.left, rectClient.top, rectClient.Width(), cyTlb, SWP_NOACTIVATE | SWP_NOZORDER);
	componentTreeViewCtrl.SetWindowPos(NULL, rectClient.left + 1, rectClient.top + cyTlb + 1, rectClient.Width() - 2, rectClient.Height() - cyTlb - 2, SWP_NOACTIVATE | SWP_NOZORDER);
}

// .......................................................................
BOOL CComponentView::PreTranslateMessage(MSG* pMsg)
{
	return CDockablePane::PreTranslateMessage(pMsg);
}

// .......................................................................
void CComponentView::OnSort(UINT id)
{
	if (m_nCurrSort == id)
	{
		return;
	}

	m_nCurrSort = id;

	CClassViewMenuButton* pButton =  DYNAMIC_DOWNCAST(CClassViewMenuButton, m_wndToolBar.GetButton(0));

	if (pButton != NULL)
	{
		pButton->SetImage(GetCmdMgr()->GetCmdImage(id));
		m_wndToolBar.Invalidate();
		m_wndToolBar.UpdateWindow();
	}
}

// .......................................................................
void CComponentView::OnUpdateSort(CCmdUI* pCmdUI)
{
	pCmdUI->SetCheck(pCmdUI->m_nID == m_nCurrSort);
}

// .......................................................................
void CComponentView::OnClassAddMemberFunction( )
{
	AfxMessageBox(_T("Add member function..."));
}

// .......................................................................
void CComponentView::OnClassAddMemberVariable( )
{
	// TODO: Add your command handler code here
}

// .......................................................................
void CComponentView::OnClassDefinition( )
{
	// TODO: Add your command handler code here
}

// .......................................................................
void CComponentView::OnClassProperties( )
{
	// TODO: Add your command handler code here
}

// .......................................................................
void CComponentView::OnNewFolder( )
{
	AfxMessageBox(_T("New Folder..."));
}

// .......................................................................
void CComponentView::OnPaint( )
{
	CPaintDC dc(this); // device context for painting

	CRect rectTree;
	componentTreeViewCtrl.GetWindowRect(rectTree);
	ScreenToClient(rectTree);

	rectTree.InflateRect(1, 1);
	dc.Draw3dRect(rectTree, ::GetSysColor(COLOR_3DSHADOW), ::GetSysColor(COLOR_3DSHADOW));
}

// .......................................................................
void CComponentView::OnSetFocus(CWnd* pOldWnd)
{
	CDockablePane::OnSetFocus(pOldWnd);

	componentTreeViewCtrl.SetFocus();
}

// .......................................................................
void CComponentView::OnChangeVisualStyle()
{
	m_ClassViewImages.DeleteImageList();

	UINT uiBmpId = theApp.m_bHiColorIcons ? IDB_CLASS_VIEW_24 : IDB_CLASS_VIEW;

	CBitmap bmp;
	if (!bmp.LoadBitmap(uiBmpId))
	{
		TRACE(_T("Can't load bitmap: %x\n"), uiBmpId);
		ASSERT(FALSE);
		return;
	}

	BITMAP bmpObj;
	bmp.GetBitmap(&bmpObj);

	UINT nFlags = ILC_MASK;

	nFlags |= (theApp.m_bHiColorIcons) ? ILC_COLOR24 : ILC_COLOR4;

	m_ClassViewImages.Create(16, bmpObj.bmHeight, nFlags, 0, 0);
	m_ClassViewImages.Add(&bmp, RGB(255, 0, 0));

	componentTreeViewCtrl.SetImageList(&m_ClassViewImages, TVSIL_NORMAL);

	m_wndToolBar.CleanUpLockedImages();
	m_wndToolBar.LoadBitmap(theApp.m_bHiColorIcons ? IDB_SORT_24 : IDR_SORT, 0, 0, TRUE /* Locked */);
}
