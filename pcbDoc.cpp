
// pcbDoc.cpp : implementation of the PCBDoc class
//

#include "stdafx.h"
#include "resource.h"
#include "globals.h"

// SHARED_HANDLERS can be defined in an ATL project implementing preview, thumbnail
// and search filter handlers and allows sharing of document code with that project.
#ifndef SHARED_HANDLERS
//#include "pcb.h"
#endif

//#include <propkey.h>
#include "globals.h"
#include "Geometry.h"
#include "board.h"
#include "Share.h"
#include "RatList.h"

#define BOARD_H
#include "SharePCB.h"
#include "netlist_import.h"
#include "pcbDoc.h"

// .......................................................................
#include "FreePCBParse.h"
#include "KiCadParse.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

enum fileTypes
{
	ftNull,
	ftFreePCB,
	ftKiCad,
};

// .......................................................................
IMPLEMENT_DYNCREATE( PCBDoc, CDocument )
BEGIN_MESSAGE_MAP( PCBDoc, CDocument )
	ON_COMMAND(ID_TEST_TEST, &PCBDoc::OnTestTest)
END_MESSAGE_MAP( )

// .......................................................................
PCBDoc::PCBDoc( )
	:testProperties( new PropertiesTest )
{
	for( auto it= testProperties->begin( ) + 1; it != testProperties->end( ); ++it )
		it->get( )->callback= std::bind1st( std::mem_fun( &PCBDoc::PropertyChanged ), this );
}

// .......................................................................
PCBDoc::~PCBDoc( )
{
}

// .......................................................................
bool PCBDoc::PropertyChanged( PropertiesItem* pProp )
{
	using namespace PropertiesSpace;
	if( pProp->GetSuperType( ) != enPropTest )
		return false;

	switch( pProp->get_id( ) )
	{
	case 2:
		size_t test= pProp->get_new_set_sel( );
		break;
	}
	if( pProp->GetType( ) == engText )
		AfxMessageBox( pProp->get_pstr( ) );
	return true;
}

// .......................................................................
BOOL PCBDoc::OnOpenDocument( LPCTSTR lpszPathName )
{
	fileTypes type= ftNull;
	bfs::path file_path( lpszPathName );
	if( bfs::extension( file_path ) == ".kicad_pcb" )
		type= ftKiCad;
	else if( bfs::extension( file_path ) == ".pcb" )
		type= ftFreePCB;

	//top level here just because it is easy
	switch( type )
	{

	case ftFreePCB:
	{
		CStdioFile file( lpszPathName, CStdioFile::modeRead );
		FreePCBParser parser( board );
		size_t count= 1; //debug
		CString strT;
		for( ;	file.ReadString( strT ); ++count )
			parser.Parse( strT );
		return TRUE;
	}
	case ftKiCad:
	{
		std::ifstream in( lpszPathName, std::ios_base::in );
		if( ! in.is_open( ) )
			assert( false );

		if( ! KiCadParse( in, &board ) )
			return FALSE;

		LayerContent note;
		note.layers= board.layers;
		SendAppMessage( APP_LAYER_NOTIFY, (WPARAM)&note );
		SendAppMessage( APP_COMP_NOTIFY, (WPARAM)&board );

		LoadRatList( board, ratlist );

		std::wofstream os( "test.txt" );
		for( auto t : ratlist )
			os << t->net << " " << t->name << std::endl;

		os.close( );
		return TRUE;
	}
	} //switch fileTypes
	return FALSE;
}

// .......................................................................
BOOL PCBDoc::OnNewDocument()
{
	if (!CDocument::OnNewDocument())
		return FALSE;

	// TODO: add reinitialization code here
	// (SDI documents will reuse this document)

	return TRUE;
}

// .......................................................................
void PCBDoc::Serialize(CArchive& ar)
{
	if (ar.IsStoring())
	{
		// TODO: add storing code here
	}
	else
	{
		for( ; ar.IsBufferEmpty( ); )
		{
			CString strT;
			ar.ReadString( strT );
		}
	}
}

#ifdef SHARED_HANDLERS

// Support for thumbnails
void PCBDoc::OnDrawThumbnail(CDC& dc, LPRECT lprcBounds)
{
	// Modify this code to draw the document's data
	dc.FillSolidRect(lprcBounds, RGB(255, 255, 255));

	CString strText = _T("TODO: implement thumbnail drawing here");
	LOGFONT lf;

	CFont* pDefaultGUIFont = CFont::FromHandle((HFONT) GetStockObject(DEFAULT_GUI_FONT));
	pDefaultGUIFont->GetLogFont(&lf);
	lf.lfHeight = 36;

	CFont fontDraw;
	fontDraw.CreateFontIndirect(&lf);

	CFont* pOldFont = dc.SelectObject(&fontDraw);
	dc.DrawText(strText, lprcBounds, DT_CENTER | DT_WORDBREAK);
	dc.SelectObject(pOldFont);
}

// .......................................................................
// Support for Search Handlers
void PCBDoc::InitializeSearchContent()
{
	CString strSearchContent;
	// Set search contents from document's data. 
	// The content parts should be separated by ";"

	// For example:  strSearchContent = _T("point;rectangle;circle;ole object;");
	SetSearchContent(strSearchContent);
}

// .......................................................................
void PCBDoc::SetSearchContent(const CString& value)
{
	if (value.IsEmpty())
	{
		RemoveChunk(PKEY_Search_Contents.fmtid, PKEY_Search_Contents.pid);
	}
	else
	{
		CMFCFilterChunkValueImpl *pChunk = NULL;
		ATLTRY(pChunk = new CMFCFilterChunkValueImpl);
		if (pChunk != NULL)
		{
			pChunk->SetTextValue(PKEY_Search_Contents, value, CHUNK_TEXT);
			SetChunkValue(pChunk);
		}
	}
}

#endif // SHARED_HANDLERS

// .......................................................................
int ParseKeyString( CString * str, CString * key_str, CArray<CString> * param_str )
{
	int pos = 0;
	int np = 0;
	for( int ip=0; ip<param_str->GetSize(); ip++ )
		(*param_str)[ip] = "";

	str->Trim();
	TCHAR c = str->GetAt(0);
	if( str->GetLength() == 0 || c == '/' )
		return 0;
	CString keyword = str->Tokenize( _T(" :\n\t"), pos );
	if( keyword == "" )
		return -1;
	// keyword found
	keyword.Trim();
	*key_str = keyword;
	// now get params
	if( keyword.GetLength() == str->GetLength() )
		return 1;
	CString param;
	CString right_str = str->Right( str->GetLength() - keyword.GetLength() - 1);
	right_str.Trim();
	pos = 0;
	int lgth = right_str.GetLength();
	while (pos<lgth)
	{
		if( right_str[pos] == _T('\"') )
			// param starts with ", remove it and tokenize to following "
			if (pos+1<lgth && right_str[pos+1]==_T('\"'))
				// NB Tokenize() is too stupid to do this correctly
				pos+=2,
				param = "";
			else
				param = right_str.Tokenize( _T("\""), pos );
		else
			param = right_str.Tokenize( _T(" \n\t"), pos );
		param.Trim();												// CPT2 I guess...
		param_str->SetAtGrow( np, param );
		np++;
		if (pos==-1) break;
		// Advance pos to the next non-whitespace char
		while (pos<lgth && isspace(right_str[pos]))
			pos++;
	}

	return np+1;
}

// .......................................................................
void PCBDoc::ReadNets( )
{
}

// .......................................................................
#ifdef _DEBUG
void PCBDoc::AssertValid() const
{
	CDocument::AssertValid();
}

void PCBDoc::Dump(CDumpContext& dc) const
{
	CDocument::Dump(dc);
}
#endif //_DEBUG
// .......................................................................

struct cut_base
{
	//enum type {
	//	line,
	//	circle,
	//	other,
	//};

	//bg_line line;

	//double length; // or radius if circle
	//double offset; // tool radius
	
};
typedef boost::shared_ptr< cut_base> SP_cut_base;
typedef std::vector< SP_cut_base > cut_vector_type;

struct cut_line : public cut_base
{
	bg_line line;
};
typedef boost::shared_ptr< cut_line> SP_cut_line;

struct cut_arc : public cut_base
{
	bg_line line;
};

typedef boost::shared_ptr< cut_arc> SP_cut_arc;

void PCBDoc::OnTestTest( )
{
	std::ofstream os( "test.txt" );
	//testing cutting output
	//we know single sided traces are on layer 1

	VSP_Line traces= board.GetLayer( 0 )->vsp_lines;

	//create an overlapping set of cuts from layers and compoents
	for( auto & trace : traces )
	{
		os << trace->pt1.get_x( ) << " " << trace->pt1.get_y( ) << std::endl;
	}
	int test= 0;

}
