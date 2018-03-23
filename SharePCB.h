
//share.h interfaces PCB to views
#pragma once

#define APP_LAYER_NOTIFY ( WM_USER + 100 )
#define APP_COMP_NOTIFY ( WM_USER + 101 )
#define APP_PROPERTIES_NOTIFY ( WM_USER + 102 )

CWnd* GetMainWindow( );
class CMainFrame;
CMainFrame* GetMainFrame( );
class PCBDoc;
PCBDoc* GetActiveDocument( );

//we will use this a bit...
inline LRESULT SendAppMessage( UINT message, LPARAM lp, WPARAM wp= NULL )
{
	return ::SendMessage( GetMainWindow( )->GetSafeHwnd( ), message, lp, wp );
}

class PCBDoc;

#ifdef BOARD_H
//....................................................
struct LayerContent
{
	static const int id= 1234;
	VSP_Layer layers;
};

//....................................................
//Properties for 'Properties window', first components...
typedef std::vector< LPCTSTR > vect_pstr;
typedef std::vector< LPCTSTR >::const_iterator vect_pstr_cit;
//string_set.first.pos for choice of string
typedef std::pair< size_t/*pos*/, vect_pstr > string_set;
typedef boost::variant< bool, std::tstring, string_set, TPCB_NUM > prop_var;

//....................................................
namespace PropertiesSpace
{
	enum prop_var_which  // will match prop_var.which( )
	{
		enpvBool,
		enpvText,
		enpvTextSet,
		enpvNumber,
	};

	enum enGridType
	{
		engHeader,
		engTrueFalse,
		engText,
		engDropDown,
		engVal,
		engColor,
	};

	//specialized for PCB, Properties Type
	enum enPropsType
	{
		enPropNull,
		enPropTest,
		enPropComp,
	};

	struct viewCom
	{
		enPropsType type;
		VSP_Base ssp_items;
	};
}//namespace PropertiesSpace

class Properties;
class PropertiesItem;
typedef boost::function< bool ( PropertiesItem* pItem )> prop_callback;
class PropertiesItem
{
protected:
	PropertiesSpace::enGridType type;
	LPCTSTR label;
	LPCTSTR help;
	prop_var var;
	//user can use this to id the item
	size_t new_set_sel;
	size_t id;
	Properties* pParent;

public:
	CMFCPropertyGridProperty* pGridItem;
	//callback
	prop_callback callback;
	PropertiesItem( )
	{ }
	PropertiesItem( PropertiesSpace::enGridType inType, LPCTSTR pstrInLabel, LPCTSTR pstrInHelp= NULL, size_t id= -1 )
		:type( inType )
		,label( pstrInLabel )
		,help( pstrInHelp )
		,id( id )
	{ }
	PropertiesSpace::enGridType GetType( ) const { return type; }
	void SetType( PropertiesSpace::enGridType itype ) { type= itype; }
	void SetLabel( LPCTSTR pIn ) { label= pIn; }
	void SetParent( Properties* pIn ) { pParent= pIn; }
	void SetHelp( LPCTSTR pIn ) { help= pIn; }
	void SetCallback( prop_callback f ) { f= f; }
	void SetVar( bool bIn ) { var= bIn; }
	void SetVar( LPCTSTR pIn ) { var= std::tstring( pIn ); }
	void SetVar( TPCB_NUM in ) { var= in; }
	void SetVar( COleVariant const& vIn );
	void make_string_set( ) { var= string_set( 0, vect_pstr( ) ); }
	void push_back( LPCTSTR pIn ) { boost::get< string_set >( var ).second.push_back( pIn ); }
	void set_pos( size_t ipos ) { boost::get< string_set >( var ).first= ipos; }
	void add_list( const TCHAR** plist, size_t size );
	void SetNewSetSel( size_t in ) { new_set_sel= in; }

	PropertiesSpace::enPropsType GetSuperType( ) const;
	LPCTSTR GetHelp( ) const { return help; }
	LPCTSTR GetLabel( ) const { return label; }
	LPCTSTR get_pstr( ) const { assert( var.which( ) == PropertiesSpace::enpvText ); return boost::get< std::tstring >( var ).c_str( ); }
	_variant_t get_truefalse( ) const { assert( var.which( ) == PropertiesSpace::enpvBool ); return boost::get< bool >( var ); }

	size_t get_pstr_set_size( ) const  { assert( var.which( ) == PropertiesSpace::enpvTextSet ); return boost::get< string_set >( var ).second.size( ); }
	size_t get_pstr_set_pos( ) const  { assert( var.which( ) == PropertiesSpace::enpvTextSet ); return boost::get< string_set >( var ).first; }
	LPCTSTR get_pstr_set_at( size_t pos ) const { assert( var.which( ) == PropertiesSpace::enpvTextSet ); return boost::get< string_set >( var ).second.at( pos ); }
	LPCTSTR get_pstr_set( ) const { assert( var.which( ) == PropertiesSpace::enpvTextSet ); return boost::get< string_set >( var ).second.at( boost::get< string_set >( var ).first ); }
	size_t get_id( ) const { return id; }
	string_set const& get_set( ) const { assert( var.which( ) == PropertiesSpace::enpvTextSet ); return boost::get< string_set >( var ); }
	size_t get_new_set_sel( ) const { return new_set_sel; }
};

inline void PropertiesItem::add_list( const TCHAR** plist, size_t size )
{
	make_string_set( );
	for( size_t i= 0; i < size; ++i )
		push_back( plist[ i ] );
}

typedef boost::shared_ptr< PropertiesItem > SP_PropertiesItem;
typedef std::vector< SP_PropertiesItem > SP_PropertiesVect;
typedef std::vector< SP_PropertiesItem >::iterator SP_PropertiesVectIt;

//....................................................
class Properties
{
protected:
	PropertiesSpace::enPropsType propType;
	SP_PropertiesVect props;

public:
	Properties( )
		:propType( PropertiesSpace::enPropNull )
	{ }

	Properties( PropertiesSpace::enPropsType type )
		:propType( type )
	{ }

	virtual PropertiesSpace::enPropsType GetType( ) { return propType; }

	void Populate( );
	SP_PropertiesVectIt begin( ) { return props.begin( ); }
	SP_PropertiesVectIt end( ) { return props.end( ); }
	void AddProperty( SP_PropertiesItem spItem ) { spItem->SetParent( this ); props.push_back( spItem ); }
};
typedef boost::shared_ptr< Properties >SP_Properties;

inline PropertiesSpace::enPropsType PropertiesItem::GetSuperType( ) const { return pParent->GetType( ); }

//....................................................
	static TCHAR* drops[ ]=
	{
		_T("first"),
		_T("second"),
		_T("third"),
		_T("forth")
	};
class PropertiesTest : public Properties, public PropertiesItem
{

public:
	PropertiesTest( )
		:Properties( PropertiesSpace::enPropTest )
	{
		using namespace PropertiesSpace;
		{
			SP_PropertiesItem pI= SP_PropertiesItem(
				new PropertiesItem(  engHeader, _T("Test Header"), _T("The Test Header\nhelp line...") ) );
			AddProperty( pI );
		}
		{
			SP_PropertiesItem pI= SP_PropertiesItem(
				new PropertiesItem( engTrueFalse, _T("Show"), _T("This is the 'Show'\nSet to true or false"), 0 ) );
			pI->SetVar( true );
			AddProperty( pI );
		}
		{
			SP_PropertiesItem pI= SP_PropertiesItem(
				new PropertiesItem( engText, _T("Text Edit"), _T("This is the Edit Field\nType what you would like"), 1 ) );
			pI->SetVar( _T("the text") );
			AddProperty( pI );
		}
		{
			SP_PropertiesItem pI= SP_PropertiesItem(
				new PropertiesItem( engDropDown, _T("Type"), _T("This is the 'Drop Down'\nJust Pick One!!!!"), 2 ) );
			pI->add_list( ASTRTYPE_DEF( drops ) );
			//or
			//pI->make_string_set( );
			//pI->push_back( _T("test1") );
			//pI->push_back( _T("test2") );
			//pI->push_back( _T("test3") );
			pI->set_pos( 2 );
			AddProperty( pI );
		}
	}
};

//....................................................
class PropertiesComponents : public Properties
{
	//PCBoard& board;
	SSP_Component ssp_comp;

public:
	PropertiesComponents( )
		:Properties( PropertiesSpace::enPropComp )
	{ }

	size_t size( ) const { return ssp_comp.size( ); }
	SSP_Component_cit begin( ) const { return ssp_comp.begin( ); }
};

#endif // BOARD_H
