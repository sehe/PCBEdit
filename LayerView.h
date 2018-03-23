
#pragma once

#include "ViewTree.h"

class CFileViewToolBar : public CMFCToolBar
{
	virtual void OnUpdateCmdUI(CFrameWnd* /*pTarget*/, BOOL bDisableIfNoHndler)
	{
		CMFCToolBar::OnUpdateCmdUI((CFrameWnd*) GetOwner(), bDisableIfNoHndler);
	}

	virtual BOOL AllowShowOnList() const { return FALSE; }
};

class PCBDoc;
class CLayerView : public CDockablePane
{
// Construction
public:
	CLayerView();

	void AdjustLayout();
	void OnChangeVisualStyle();

	void ChangeContent( WPARAM wp, PCBDoc* lp );
// Attributes
protected:

	CViewTree boardTreeViewCtrl;
	CImageList m_FileViewImages;
	CFileViewToolBar m_wndToolBar;

protected:
	void FillFileView();
	void OnLvnItemchangedEventlist( NMHDR *pNMHDR, LRESULT *pResult );

// Implementation
public:
	virtual ~CLayerView();

protected:
DECLARE_MESSAGE_MAP( )
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
	afx_msg void OnProperties();
	afx_msg void OnFileOpen();
	afx_msg void OnFileOpenWith();
	afx_msg void OnDummyCompile();
	afx_msg void OnEditCut();
	afx_msg void OnEditCopy();
	afx_msg void OnEditClear();
	afx_msg void OnPaint();
	afx_msg void OnSetFocus(CWnd* pOldWnd);
	afx_msg LRESULT OnCheckChanged( WPARAM, LPARAM );
};

