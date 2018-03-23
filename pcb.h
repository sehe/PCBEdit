
// pcb.h : main header file for the pcb application
//
#pragma once

#ifndef __AFXWIN_H__
	#error "include 'stdafx.h' before including this file for PCH"
#endif

#include "HEWinApp.h"
#include "CmdArgs.h"

// .....................................................................
class CpcbApp : public HEWinApp
{
public:
	CpcbApp();

	friend class CFrameImpl;
	virtual BOOL InitInstance( );
	virtual int ExitInstance( );
	virtual void PreLoadState( );

protected:
DECLARE_MESSAGE_MAP( )
	afx_msg void OnAppAbout( );
	afx_msg void OnFileNewfromnetlist( );
};

extern CpcbApp theApp;
inline CWnd* GetMainWindow( ) { return theApp.m_pMainWnd; }
