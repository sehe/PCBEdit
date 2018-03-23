

1/1/2017 this file is depreciated
// This is a part of the Microsoft Foundation Classes C++ library.
// Copyright (C) Microsoft Corporation
// All rights reserved.
//
// This source code is only intended as a supplement to the
// Microsoft Foundation Classes Reference and related
// electronic documentation provided with the library.
// See these sources for detailed information regarding the
// Microsoft Foundation Classes product.

/*
	CSettingsStoreSP is a singleton class that manages CSettingsStore

*/
#pragma once

#include <winreg.h>
#include <atlbase.h>

#include "afxcontrolbarutil.h"

#ifdef _AFX_PACKING
#pragma pack(push, _AFX_PACKING)
#endif

#ifdef _AFX_MINREBUILD
#pragma component(minrebuild, off)
#endif

extern SP_XML sp_xml_settings_store_global;
extern SP_XMLNODE sp_currentNode_settings_store_global;
extern 	CString strSysRegPath;

class XMLSettingsStore : public CSettingsStore
{
	friend class XMLSettingsStoreSP;
	DECLARE_DYNCREATE( XMLSettingsStore )

	SP_XML sp_xml;
	SP_XMLNODE sp_currentNode;
//	CString strOpen;
	LPCTSTR psRegPath;

public:
	XMLSettingsStore( BOOL bAdmin, BOOL bReadOnly );
	virtual ~XMLSettingsStore( );

protected:
	XMLSettingsStore( );
	bool MakePath( LPCTSTR psIn, CString& strIn );

// Operations
public:
	bool OpenXML( LPCTSTR psPathFile );
	bool OpenXML( bfs::path pathPathFile ) { return OpenXML( pathPathFile.wstring( ).c_str( ) ); }
	void SetRegPath( LPCTSTR psIn ) { psRegPath= psIn; }

	//the app will pick these up to keep them persistant
	SP_XML GetXML( ) { return sp_xml; }
	SP_XMLNODE GetCurrentNode( ) { return sp_currentNode; }

	void SetCurrentNode( LPCTSTR psNode );

	virtual BOOL CreateKey(LPCTSTR lpszPath);
	virtual BOOL Open(LPCTSTR lpszPath);
	virtual void Close();

	virtual BOOL DeleteValue(LPCTSTR lpszValue);
	virtual BOOL DeleteKey(LPCTSTR lpszPath, BOOL bAdmin = FALSE);

	virtual BOOL Write(LPCTSTR lpszValueName, int nValue);
	virtual BOOL Write(LPCTSTR lpszValueName, DWORD dwVal);
	virtual BOOL Write(LPCTSTR lpszValueName, LPCTSTR lpszVal);
	virtual BOOL Write(LPCTSTR lpszValueName, const CRect& rect);
	virtual BOOL Write(LPCTSTR lpszValueName, LPBYTE pData, UINT nBytes);
	virtual BOOL Write(LPCTSTR lpszValueName, CObject& obj);
	virtual BOOL Write(LPCTSTR lpszValueName, CObject* pObj);

	virtual BOOL Read(LPCTSTR lpszValueName, int& nValue);
	virtual BOOL Read(LPCTSTR lpszValueName, DWORD& dwValue);
	virtual BOOL Read(LPCTSTR lpszValueName, CString& strValue);
	virtual BOOL Read(LPCTSTR lpszValueName, CRect& rect);
	virtual BOOL Read(LPCTSTR lpszValueName, BYTE** ppData, UINT* pcbData);
	virtual BOOL Read(LPCTSTR lpszValueName, CObject& obj);
	virtual BOOL Read(LPCTSTR lpszValueName, CObject*& pObj);

protected:
	ATL::CRegKey m_reg;
	CString m_strPath;
	BOOL    m_bReadOnly;
	BOOL    m_bAdmin;
	DWORD   m_dwUserData;
};

//////////////////////////////////////////////////////////////////////////////
// XMLSettingsStoreSP - Helper class that manages "safe" XMLSettingsStore pointer
/*
class XMLSettingsStoreSP
{
public:
	static BOOL __stdcall SetRuntimeClass(CRuntimeClass* pRTI);

	XMLSettingsStoreSP(DWORD dwUserData = 0) : m_pRegistry(NULL), m_dwUserData(dwUserData)
	{
	}

	~XMLSettingsStoreSP()
	{
		if (m_pRegistry != NULL)
		{
			ASSERT_VALID(m_pRegistry);
			delete m_pRegistry;
		}
	}

	XMLSettingsStore& Create(BOOL bAdmin, BOOL bReadOnly);

protected:
	XMLSettingsStore* m_pRegistry;
	DWORD      m_dwUserData;

//	AFX_IMPORT_DATA static CRuntimeClass* m_pRTIDefault;
	static CRuntimeClass* m_pRTIDefault;
};
*/

#ifdef _AFX_MINREBUILD
#pragma component(minrebuild, on)
#endif

#ifdef _AFX_PACKING
#pragma pack(pop)
#endif
