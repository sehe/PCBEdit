
// MainFrm.h : interface of the CMainFrame class
//

#pragma once
#include "SharePCB.h"
#include "LayerView.h"
#include "ComponentView.h"
#include "OutputWnd.h"
#include "PropertiesWnd.h"

class CMainFrame : public CMDIFrameWndEx
{
	DECLARE_DYNAMIC(CMainFrame)
public:
	CMainFrame();

// Attributes
public:
	CDocument* GetTopDocument( );

// Operations
public:

// Overrides
public:
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	virtual BOOL LoadFrame(UINT nIDResource, DWORD dwDefaultStyle = WS_OVERLAPPEDWINDOW | FWS_ADDTOTITLE, CWnd* pParentWnd = NULL, CCreateContext* pContext = NULL);

// Implementation
public:
	virtual ~CMainFrame();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:  // control bar embedded members
	CMFCMenuBar       m_wndMenuBar;
	CMFCToolBar       m_wndToolBar;
	CMFCToolBar       modeToolBar;
	CMFCStatusBar     m_wndStatusBar;
	CMFCToolBarImages m_UserImages;
	CLayerView        boardTreeViewCtrl;
	CComponentView    componentTreeViewCtrl;
	COutputWnd        m_wndOutput;
	CPropertiesWnd    propertiesWnd;

// Generated message map functions
protected:
	afx_msg LRESULT OnLayerNotify( WPARAM wp, LPARAM lp );
	afx_msg LRESULT OnComponentNotify( WPARAM wp, LPARAM lp );
	afx_msg LRESULT OnPropertiesNotify( WPARAM wp, LPARAM lp );
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnWindowManager();
	afx_msg void OnViewCustomize();
	afx_msg LRESULT OnToolbarCreateNew(WPARAM wp, LPARAM lp);
	afx_msg void OnApplicationLook(UINT id);
	afx_msg void OnUpdateApplicationLook(CCmdUI* pCmdUI);
	afx_msg void OnSettingChange(UINT uFlags, LPCTSTR lpszSection);
	DECLARE_MESSAGE_MAP()

	BOOL CreateDockingWindows();
	void SetDockingWindowIcons(BOOL bHiColorIcons);
	virtual BOOL OnShowTooltips(UINT id, NMHDR *pNMHDR, LRESULT *pResult);

};


