
// pcb.cpp : Defines the class behaviors for the application.
//

#include "stdafx.h"
#include "afxdialogex.h"
#include "resource.h"       // main symbols
#include "HEWinApp.h"

#include "globals.h"
#include "CmdArgs.h"

#include "board.h"
#include "Share.h"
#include "RatList.h"
#include "netlist_import.h"

#include "About.h"
#include "pcb.h"

#include "SharePCB.h"
#include "pcbFrame.h"
#include "pcbDoc.h"
#include "pcbView.h"

#include "MainFrm.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


CpcbApp theApp;

// .......................................................................
BEGIN_MESSAGE_MAP( CpcbApp, HEWinApp )
	ON_COMMAND(ID_APP_ABOUT, &CpcbApp::OnAppAbout)
	// Standard file based document commands
	ON_COMMAND(ID_FILE_NEW, &CWinAppEx::OnFileNew)
	ON_COMMAND(ID_FILE_OPEN, &CWinAppEx::OnFileOpen)
	// Standard print setup command
	ON_COMMAND(ID_FILE_PRINT_SETUP, &CWinAppEx::OnFilePrintSetup)
	ON_COMMAND(ID_FILE_NEWFROMNETLIST, &CpcbApp::OnFileNewfromnetlist)
END_MESSAGE_MAP()

// .......................................................................
CpcbApp::CpcbApp( )
{
	// support Restart Manager
	m_dwRestartManagerSupportFlags = AFX_RESTART_MANAGER_SUPPORT_ALL_ASPECTS;
#ifdef _MANAGED
	// If the application is built using Common Language Runtime support (/clr):
	//     1) This additional setting is needed for Restart Manager support to work properly.
	//     2) In your project, you must add a reference to System.Windows.Forms in order to build.
	System::Windows::Forms::Application::SetUnhandledExceptionMode(System::Windows::Forms::UnhandledExceptionMode::ThrowException);
#endif

	// TODO: replace application ID string below with unique ID string; recommended
	// format for string is CompanyName.ProductName.SubProduct.VersionInformation
	SetAppID( _T("pcb.AppID.NoVersion") );

	// TODO: add construction code here,
	// Place all significant initialization in InitInstance
}

// .......................................................................
BOOL CpcbApp::InitInstance()
{
	// Initialize OLE libraries
	if (!AfxOleInit())
	{
		AfxMessageBox(IDP_OLE_INIT_FAILED);
		return FALSE;
	}
	AfxInitRichEdit2( );

	// -u on command line?
	argstate state;
	if( state.count( "user-folder" ) )
	{
		pathUser= state.value< std::string >( "user-folder" );
	}
	//here?
	HEWinApp::InitInstance( );

	if( ! state.init( __argc, __targv ) )
		return state.err( );

	// Register the application's document templates.  Document templates
	//  serve as the connection between documents, frame windows and views
	CMultiDocTemplate* pDocTemplate;
	pDocTemplate = new CMultiDocTemplate(
		IDR_pcbTYPE,
		RUNTIME_CLASS(PCBDoc),
		RUNTIME_CLASS(PCBFrame), // custom MDI child frame
		RUNTIME_CLASS(PCBView));
	if (!pDocTemplate)
		return FALSE;
	AddDocTemplate(pDocTemplate);

	// create main MDI Frame window
	CMainFrame* pMainFrame = new CMainFrame;
	if (!pMainFrame || !pMainFrame->LoadFrame(IDR_MAINFRAME))
	{
		delete pMainFrame;
		return FALSE;
	}
	m_pMainWnd = pMainFrame;
	// call DragAcceptFiles only if there's a suffix
	//  In an MDI app, this should occur immediately after setting m_pMainWnd
	// Enable drag/drop open
	m_pMainWnd->DragAcceptFiles();

	//TODO If command line path/file passed, open that
	bool bCmdFile= false;
	//should be settings optional to open on init
#ifndef _DEBUG
	if( ! bCmdFile && m_pRecentFileList && m_pRecentFileList->m_nSize && (*m_pRecentFileList)[ 0 ].GetLength() > 0 )
		OnOpenRecentFile( ID_FILE_MRU_FIRST );
#else// _DEBUG
//	else
//		OpenDocumentFile( _T("..\\docs\\usb-gpio2.pcb") );
	//OpenDocumentFile( _T("C:\\cpp\\pcb_14\\etc\\sonde xilinx.kicad_pcb") );
	if( ! bCmdFile && m_pRecentFileList && m_pRecentFileList->m_nSize && (*m_pRecentFileList)[ 0 ].GetLength() > 0 )
		OnOpenRecentFile( ID_FILE_MRU_FIRST );
#endif// _DEBUG

	// Enable DDE Execute open
	EnableShellOpen( );
	RegisterShellFileTypes( TRUE );

	// The main window has been initialized, so show and update it
	pMainFrame->ShowWindow(m_nCmdShow);
	pMainFrame->UpdateWindow();

	//for testing
	//sp_xml_settings_store_global->Close( );
	return TRUE;
}

// .......................................................................
int CpcbApp::ExitInstance()
{
	return HEWinApp::ExitInstance( );
}

// .......................................................................
void CpcbApp::PreLoadState( )
{
	BOOL bNameValid;
	CString strName;
	bNameValid = strName.LoadString(IDS_EDIT_MENU);
	ASSERT(bNameValid);
	GetContextMenuManager()->AddMenu(strName, IDR_POPUP_EDIT);
	bNameValid = strName.LoadString(IDS_EXPLORER);
	ASSERT(bNameValid);
	GetContextMenuManager()->AddMenu(strName, IDR_POPUP_EXPLORER);
}

// .......................................................................
void CpcbApp::OnFileNewfromnetlist( )
{
	// TODO: Import from file name
	bfs::path netpath( "C:\\cpp\\pcb\\etc\\NatGas2.net.xml" );
	CXML xml;
	xml.Open( netpath.wstring( ).c_str( ), "netlist", CXML::modeRead );

	CDocTemplate* pTemp= m_pDocManager->GetBestTemplate( netpath.wstring( ).c_str( ) );
	CDocument* pDoc= pTemp->OpenDocumentFile( NULL );

	PCBDoc* pPCDoc= dynamic_cast< PCBDoc* >( pDoc );
	assert( pPCDoc );
	TinyCADNetlistImport( xml, pPCDoc->netlist_import );
}

// .......................................................................
void CpcbApp::OnAppAbout( )
{
	CAboutDlg aboutDlg;
	aboutDlg.DoModal( );
}


