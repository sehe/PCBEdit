

#pragma once
//This stuff is to parse FreePCB project files. I picked this to parse first as it was pretty well documented
//parsing tools .............................................................
// ..........................................................................
typedef __int64 PCB_HANDLE;
typedef std::stack< PCB_HANDLE > HandleStack;
typedef boost::split_iterator< std::tstring::iterator > split_it;

// ..........................................................................
typedef std::tstring::iterator STR_IT;
typedef std::pair< STR_IT, STR_IT > STR_RANGE;

// ..........................................................................
//tokenizer does not play well with unicode, or, I'm missing something...
typedef boost::tokenizer<
	boost::escaped_list_separator< wchar_t, std::tstring::traits_type >
	, std::tstring::const_iterator, std::tstring
	> tokenizer;
typedef tokenizer::iterator tokenizer_iterator;

// ..........................................................................
typedef boost::variant< TPCB_NUM, TPCB_STR, TPCB_CHAR, double > vars;
typedef std::vector< vars > v_vars;
#define TPCB_NUM_POS 0
#define TPCB_STR_POS 1
#define TPCB_FLG_POS 0
#define TPCB_CHAR_POS 2
#define TPCB_DOUBLE_POS 3

// ..........................................................................
enum PCBdtypes { ptnull= -1, ptnum= 0, ptstr= 1, ptchar= 2, ptdouble= 3 };

// ..........................................................................
//I only see white space delimiters in the PCB project files
class value_set
{
	STR_RANGE params;
	split_it it;
	v_vars tvars;

public:
	value_set( )
	{ }

	value_set( STR_RANGE instr, PCBdtypes* vars );
	size_t size( ) const { return tvars.size( ); }
	TPCB_NUM get_num( size_t at ) { assert( at < tvars.size( ) ); return boost::get<TPCB_NUM>( tvars.at( at ) ); }
	TPCB_STR get_str( size_t at ) { assert( at < tvars.size( ) && tvars.at( at ).which( ) == TPCB_STR_POS ); return boost::get<TPCB_STR>( tvars.at( at ) ); }
	TPCB_CHAR get_char( size_t at ) { assert( at < tvars.size( ) && tvars.at( at ).which( ) == TPCB_CHAR_POS ); return boost::get<TPCB_CHAR>( tvars.at( at ) ); }
};


// ..........................................................................
namespace FreePCBItemTypes
{
	enum eaItemTypes
	{
		ptypeNull= 0,
		ptypePCB,
		ptypeAttribute,
		ptypePolygon,
		ptypeSymbol,
		ptypePad,
		ptypePin,
		ptypeLine,
		ptypeVia,
		ptypeElement,
		ptypeElementLine,
		ptypeLayer,
		ptypeNetList,
		ptypeNet,
	};

	AFX_STATIC_DATA LPCTSTR psPCBAItemTypes[]=
	{
		_T("Null"),
		_T("PCB"),
		_T("Attribute"),
		_T("Polygon"),
		_T("Symbol"),
		_T("Pad"),
		_T("Pin"),
		_T("Line"),
		_T("Via"),
		_T("Element"),
		_T("ElementLine"),
		_T("Layer"),
		_T("NetList"),
		_T("Connect"),//net
	};
}

extern AStrType< FreePCBItemTypes::eaItemTypes > FreePCBtypeSet;

struct int_pair { int from; int to; };
AFX_STATIC_DATA int_pair translate_type[]=
{
	{ FreePCBItemTypes::ptypeAttribute,	PCBItemTypes::ptypeAttribute },
	{ FreePCBItemTypes::ptypePolygon,	PCBItemTypes::ptypePolygon },
	{ FreePCBItemTypes::ptypeSymbol,		PCBItemTypes::ptypeSymbol },
	{ FreePCBItemTypes::ptypePad,		PCBItemTypes::ptypeConnect },
	{ FreePCBItemTypes::ptypePin,		PCBItemTypes::ptypeConnect },
	{ FreePCBItemTypes::ptypeVia,		PCBItemTypes::ptypeConnect },
	{ FreePCBItemTypes::ptypeElement,	PCBItemTypes::ptypeComponent },
	{ FreePCBItemTypes::ptypeLine,		PCBItemTypes::ptypeLine },
	{ FreePCBItemTypes::ptypeElementLine,PCBItemTypes::ptypeLine },
	{ FreePCBItemTypes::ptypeLayer,		PCBItemTypes::ptypeLayer },
	{ FreePCBItemTypes::ptypeNetList,	PCBItemTypes::ptypeNetList },
};


//PCB[ "" x y ]
AFX_STATIC_DATA PCBdtypes PCB_vars[]= { ptnull, ptnum, ptnum };

//Element[ SFlags "Desc" "Name" "Value" MX MY TX TY TDir TScale TSFlags ]
AFX_STATIC_DATA PCBdtypes element_vars[]= { ptstr, ptstr, ptstr, ptstr, ptnum, ptnum, ptnum, ptnum, ptnum, ptnum, ptstr };

//Pad[ rX1 rY1 rX2 rY2 Thickness Clearance Mask "Name" "Number" SFlags ]
AFX_STATIC_DATA PCBdtypes connect_pad_vars[]= { ptnum, ptnum, ptnum, ptnum, ptnum, ptnum, ptnull, ptstr, ptstr, ptstr };

//Pin[ rX rY Thickness Clearance Mask Drill "Name" "Number" SFlags ]
AFX_STATIC_DATA PCBdtypes connect_pin_vars[]= { ptnum, ptnum, ptnum, ptnum, ptnum, ptnum, ptstr, ptstr, ptstr };

//Via[ 2210.00mil 2621.00mil 50.00mil 40.00mil 0.0000 30.00mil "" "thermal(5S)" ]
AFX_STATIC_DATA PCBdtypes connect_via_vars[]= { ptnum, ptnum, ptnum, ptnum, ptnum, ptnum, ptstr, ptstr };

//Line [ rX1	rY1		rX2		rY2		Thickness	Clearance		TSFlags ]
AFX_STATIC_DATA PCBdtypes connect_line_vars[]= { ptnum, ptnum, ptnum, ptnum, ptnum, ptnum, ptstr };

//Symbol [ chr size ]	
AFX_STATIC_DATA PCBdtypes symbol_vars[]= { ptchar, ptnum };

//SymbolLine [ x1	y1	x2	y2	width ]	
AFX_STATIC_DATA PCBdtypes symbol_line_vars[]= { ptnum, ptnum, ptnum, ptnum, ptnum };

//Net [ name unknown ]	
AFX_STATIC_DATA PCBdtypes net_vars[]= { ptstr, ptstr };

// ..........................................................................
class FreePCBParser
{
protected:
	PCBoard* pTarget;
	HandleStack h_stack;

public:
	FreePCBParser( PCBoard& in_board )
		:pTarget( & in_board )
	{ }

	void Parse( const TCHAR* line );

protected:
	PCB_HANDLE loadItemStr( STR_RANGE name, STR_RANGE params );
	PCB_HANDLE loadItemStr( FreePCBItemTypes::eaItemTypes type, STR_RANGE params );
	PCB_HANDLE loadItemStr( PCB_HANDLE hnd, STR_RANGE name, STR_RANGE params );

	void ElementParse( Component* pE, STR_RANGE params );
	void ConnectParse( Connect* pC, FreePCBItemTypes::eaItemTypes type, STR_RANGE params );
	void LineParse( Line* pL, FreePCBItemTypes::eaItemTypes type, STR_RANGE params );
};

