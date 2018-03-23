
// pcbDoc.h : interface of the PCBDoc class
//


#pragma once
#include "board.h"

class PCBDoc : public CDocument
{
protected: // create from serialization only
	PCBDoc();
	DECLARE_DYNCREATE(PCBDoc)

	//one board per doc?
	PCBoard board;

	//the ratlist
	ratlist_t ratlist;
	//from shcematic netlist on refresh
	NetListImport netlist_import;

	SP_Properties testProperties;
	SP_Properties GetProperties( PropertiesSpace::viewCom& vc ) { return testProperties; }
	bool PropertyChanged( PropertiesItem* pProp );

public:
	virtual ~PCBDoc( );

public:
	virtual BOOL OnNewDocument();
	virtual BOOL OnOpenDocument( LPCTSTR lpszPathName );
	virtual void Serialize(CArchive& ar);

#ifdef SHARED_HANDLERS
	virtual void InitializeSearchContent();
	virtual void OnDrawThumbnail(CDC& dc, LPRECT lprcBounds);
	//
	void SetSearchContent(const CString& value);
#endif // SHARED_HANDLERS

#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:
	DECLARE_MESSAGE_MAP( )
	void ReadNets( );
public:
	afx_msg void OnTestTest();
};
