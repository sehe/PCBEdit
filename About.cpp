
#define USE_WINDOWS_EX
#include "stdafx.h"
#include <globals.h>
#include "resource.h"
#include "About.h"

//TODO this will point to the 'Program Files' folder
CString GetProgramDirectory( ) { return CString( ".\\ProgramFiles\\" ); } // use 'program' in debug;

// ............................................................
//IMPLEMENT_DYNAMIC( CRichBox, CObject )

BEGIN_MESSAGE_MAP( CRichBox, CRichEditCtrl )
	ON_WM_CREATE( )
END_MESSAGE_MAP( )

int CRichBox::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CRichEditCtrl::OnCreate(lpCreateStruct) == -1)
		return -1;

	return 0;
}

BOOL CRichBox::Create( LPCTSTR lpszText, DWORD dwStyle, const RECT& rect, CWnd* pParentWnd, UINT nID )
{
	if( !AfxInitRichEdit2( ) )
		return FALSE;

	CWnd* pWnd = this;
	return pWnd->Create( RICHEDIT_CLASS, NULL, dwStyle, rect, pParentWnd, nID );
}

CAboutDlg::CAboutDlg() : CDialogEx(CAboutDlg::IDD)
, valDouble(0)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_RICHEDIT21, cRichAck);
	DDX_Text(pDX, IDC_EDIT1, valDouble);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()


BOOL CAboutDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	cRichAck.dcTarget.CreateDC( _T("DISPLAY"), NULL, NULL, NULL );
	long lLineWidth= ::MulDiv( cRichAck.dcTarget.GetDeviceCaps( PHYSICALWIDTH ),
		1440, cRichAck.dcTarget.GetDeviceCaps( LOGPIXELSX ) );
	cRichAck.SetTargetDevice( cRichAck.dcTarget, lLineWidth );
	cRichAck.SendMessage( EM_SETTYPOGRAPHYOPTIONS, TO_ADVANCEDTYPOGRAPHY, TO_ADVANCEDTYPOGRAPHY );
	cRichAck.SetReadOnly( );

	bfs::path file( GetProgramDirectory( ) + _T("Ack.rtf") );
	std::ifstream fs( file.string( ), std::ios::in );
	if( ! fs.bad( ) )
	{
		char* p= new char[ ( unsigned int)fs.tellg( ) ];
		fs.read( p, fs.tellg( ) );
		SetRichText( cRichAck, p );
		delete [ ]p;
	}
	return TRUE;
}
